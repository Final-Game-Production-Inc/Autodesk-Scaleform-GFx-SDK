/**************************************************************************

Filename    :   HeapMH_FreeBin.cpp
Content     :   
Created     :   2010
Authors     :   Maxim Shemanarev, Boris Rayskiy

Notes       :   Containers to store information about free memory areas

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapMH_FreeBin.h"
#include "../SF_Alg.h"

namespace Scaleform { namespace HeapMH {


//------------------------------------------------------------------------
ListBinMH::ListBinMH()
{
    Reset();
}

void ListBinMH::Reset()
{
    Mask = 0;
    memset(Roots, 0, sizeof(Roots));
}

//--------------------------------------------------------------------
SF_INLINE void ListBinMH::pushNode(UPInt idx, BinNodeMH* node)
{
    BinNodeMH* root = Roots[idx];
    if (root)
    {
        node->SetPrev(root);                // node->pPrev = root;
        node->SetNext(root->GetNext());     // node->pNext = root->pNext;
        root->GetNext()->SetPrev(node);     // root->pNext->pPrev = node;
        root->SetNext(node);                // root->pNext = node;
    }
    else
    {
        node->SetPrev(node);                // node->pPrev = node;
        node->SetNext(node);                // node->pNext = node;
    }
    Roots[idx] = node;
    Mask |= UPInt(1) << idx;
}


//--------------------------------------------------------------------
SF_INLINE void ListBinMH::pullNode(UPInt idx, BinNodeMH* node)
{
    BinNodeMH* root = Roots[idx];
    if (node != root)
    {
        node->GetPrev()->SetNext(node->GetNext());  // node->pPrev->pNext = node->pNext;
        node->GetNext()->SetPrev(node->GetPrev());  // node->pNext->pPrev = node->pPrev;
        return;
    }
    if (root != root->GetNext())
    {
        Roots[idx] = root->GetNext();
        node->GetPrev()->SetNext(node->GetNext());  // node->pPrev->pNext = node->pNext;
        node->GetNext()->SetPrev(node->GetPrev());  // node->pNext->pPrev = node->pPrev;
        return;
    }
    Roots[idx] = 0;
    Mask &= ~(UPInt(1) << idx);
}


UByte* ListBinMH::GetAlignedPtr(UByte* start, UPInt alignMask)
{
    UPInt aligned = (UPInt(start) + alignMask) & ~alignMask;
    return (UByte*)aligned;
}

//--------------------------------------------------------------------
bool ListBinMH::AlignmentIsOK(const BinNodeMH* node, UPInt blocks, UPInt alignMask)
{
    UPInt  bytes   = blocks << PageMH::UnitShift;
    UPInt  aligned = UPInt(GetAlignedPtr((UByte*)node, alignMask));
    return aligned + bytes <= UPInt(node) + node->GetBytes();
}

//--------------------------------------------------------------------
BinNodeMH* ListBinMH::findAligned(BinNodeMH* root, UPInt blocks, UPInt alignMask)
{
    BinNodeMH* node = root;
    if (node) do
    {
        if (AlignmentIsOK(node, blocks, alignMask))
        {
            return node;
        }
        node = node->GetNext();
    }
    while(node != root);
    return 0;
}


//------------------------------------------------------------------------
BinNodeMH* ListBinMH::PullBest(UPInt blocks)
{
    UPInt idx = Alg::Min<UPInt>(blocks-1, BinSize-1);
    BinNodeMH* best = 0;
    UPInt bits = Mask >> idx;
    if (bits)
    {
        // Go straight to the first not empty list. No searching loops.
        //----------------------
        UPInt i = idx + Alg::LowerBit(bits);
        best = Roots[i];
        SF_ASSERT(best);
        pullNode(i, best);
    }
    return best;
}


//--------------------------------------------------------------------
BinNodeMH* ListBinMH::PullBest(UPInt blocks, UPInt alignMask)
{
    UPInt idx = Alg::Min<UPInt>(blocks-1, BinSize-1);
    BinNodeMH* best = 0;
    UPInt bits = Mask >> idx;
    if (bits)
    {
        // Go straight to the first not empty list. No searching loops.
        //----------------------
        UPInt i = idx + Alg::LowerBit(bits);
        SF_ASSERT(Roots[i]);

        // TO DO: Can be optimized by skipping empty roots 
        // according to the Mask.
        do
        {
            best = findAligned(Roots[i], blocks, alignMask);
            if (best)
            {
                pullNode(i, best);
                break;
            }
        }
        while(++i < BinSize);
    }
    return best;
}


// The node must be a fully constructed BinLNode
//------------------------------------------------------------------------
void ListBinMH::Push(UByte* node)
{
    BinNodeMH* tn = (BinNodeMH*)node;
    UPInt blocks = tn->GetBlocks();
    pushNode(Alg::Min<UPInt>(blocks-1, BinSize-1), tn);
}


//------------------------------------------------------------------------
void ListBinMH::Pull(UByte* node)
{
    BinNodeMH* tn = (BinNodeMH*)node;
    UPInt blocks = tn->GetBlocks();
    pullNode(Alg::Min<UPInt>(blocks-1, BinSize-1), tn);
}


//--------------------------------------------------------------------
SF_INLINE BinNodeMH* ListBinMH::getPrevAdjacent(UByte* node) const
{
    // Get the previous adjacent free block. If there's 
    // an empty space in the bit-set it means that the previous
    // adjacent block is empty and valid. This statement simply
    // reads the length from the end of the previous adjacent 
    // block and subtracts the respective size from the "node" pointer.
    //------------------------
    return (BinNodeMH*)(node - ((BinNodeMH*)node)->GetPrevBytes());
}

//--------------------------------------------------------------------
SF_INLINE BinNodeMH* ListBinMH::getNextAdjacent(UByte* node) const
{ 
    // Get the next adjacent free block. If there's 
    // an empty space in the bit-set it means that the next
    // adjacent block is empty and valid. It just takes
    // the size of the node and adds the respective size to 
    // the "node" pointer.
    //------------------------
    return (BinNodeMH*)(node + ((BinNodeMH*)node)->GetBytes());
}

// The "node" must be a fully constructed BinLNode. Use MakeNode().
//------------------------------------------------------------------------
void ListBinMH::Merge(UByte* node, UPInt bytes, bool left, bool right, PageMH* page)
{
    BinNodeMH* mergeNode = (BinNodeMH*)node;
    UPInt      mergeSize = bytes >> PageMH::UnitShift;

    mergeNode->SetBlocks(mergeSize);
    if (left)
    {
        mergeNode  = getPrevAdjacent(node);
        mergeSize += mergeNode->GetBlocks();
        Pull((UByte*)mergeNode);
    }
    if (right)
    {
        BinNodeMH* nextAdjacent = getNextAdjacent(node);
        mergeSize += nextAdjacent->GetBlocks();
        Pull((UByte*)nextAdjacent);
    }
    mergeNode->SetBlocks(mergeSize);
    mergeNode->SetPage(page);
    pushNode(Alg::Min<UPInt>(mergeSize-1, BinSize-1), mergeNode);
}



}} // Scaleform::HeapMH

