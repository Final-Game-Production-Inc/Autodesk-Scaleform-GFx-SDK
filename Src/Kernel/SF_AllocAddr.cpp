/**************************************************************************

Filename    :   SF_AllocAddr.cpp
Content     :   Abstract address space allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   An abstract address space allocator based on binary 
                trees. Used when the memory itself is not available
                (address space, video memory, etc).

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_AllocAddr.h"
#include "SF_MemoryHeap.h"

//#include <stdio.h> // DBG

namespace Scaleform {

//------------------------------------------------------------------------
AllocAddr::AllocAddr(MemoryHeap* nodeHeap) :
    pNodeHeap(nodeHeap)
{
}

AllocAddr::AllocAddr(MemoryHeap* nodeHeap, UPInt start, UPInt size) :
    pNodeHeap(nodeHeap)
{
    FreeNode* node = (FreeNode*)pNodeHeap->Alloc(sizeof(FreeNode));
    pushNode(node, start, size);
}

//------------------------------------------------------------------------
AllocAddr::~AllocAddr()
{
    destroyAll();
}

//------------------------------------------------------------------------
static void linearizeTree(AllocAddrNode* root, List<AllocAddrNode>* nodes)
{
    if (root)
    {
        linearizeTree(root->AddrChild[0], nodes);
        nodes->PushBack(root);
        linearizeTree(root->AddrChild[1], nodes);
    }
}

//------------------------------------------------------------------------
void AllocAddr::destroyAll()
{
    List<AllocAddrNode> nodes;
    linearizeTree(AddrTree.Root, &nodes);
    AddrTree.Clear();
    SizeTree.Tree.Clear();
    while(!nodes.IsEmpty())
    {
        AllocAddrNode* node = nodes.GetFirst();
        nodes.Remove(node);
        pNodeHeap->Free(node);
    }
}


//------------------------------------------------------------------------
inline void AllocAddr::pushNode(FreeNode* node, UPInt addr, UPInt size)
{
    node->Addr = addr;
    node->Size = size;
    SizeTree.Insert(node);
    AddrTree.Insert(node);
}


//------------------------------------------------------------------------
inline void AllocAddr::pullNode(FreeNode* node)
{
    SizeTree.Remove(node);
    AddrTree.Remove(node);
}


//------------------------------------------------------------------------
AllocAddr::FreeNode* AllocAddr::pullBest(UPInt size)
{
    FreeNode* best = SizeTree.PullBestGrEq(size);
    if (best)
    {
        AddrTree.Remove(best);
    }
    return best;
}


//------------------------------------------------------------------------
void AllocAddr::splitNode(FreeNode* node, UPInt addr, UPInt size)
{
    UPInt headSize = addr - node->Addr;
    UPInt tailSize = (node->Addr + node->Size) - (addr + size);
    if (headSize && tailSize)
    {
        pushNode(node, node->Addr, headSize);
        node = (FreeNode*)pNodeHeap->Alloc(sizeof(FreeNode));
        pushNode(node, addr + size, tailSize);
    }
    else if (headSize)
    {
        pushNode(node, node->Addr, headSize);
    }
    else if (tailSize)
    {
        pushNode(node, addr + size, tailSize);
    }
    else
    {
        pNodeHeap->Free(node);
    }
}


//------------------------------------------------------------------------
UPInt AllocAddr::mergeNodes(FreeNode* prev, FreeNode* next, UPInt addr, UPInt size)
{
    // Merge nodes
    //-----------------
    UPInt freeSize;
    if (prev && next)
    {
        freeSize = prev->Size + size + next->Size;
        pullNode(prev);
        pullNode(next);        
        pushNode(prev, prev->Addr, freeSize);
        pNodeHeap->Free(next);
    }
    else if (prev)
    {
        freeSize = prev->Size + size;
        pullNode(prev);
        pushNode(prev, prev->Addr, freeSize);
    }
    else if (next)
    {
        freeSize = next->Size + size;
        pullNode(next);
        pushNode(next, addr, freeSize);
    }
    else
    {
        freeSize = size;
        next = (FreeNode*)pNodeHeap->Alloc(sizeof(FreeNode));
        pushNode(next, addr, size);
    }
    return freeSize;
}

//static void traverseAddrTree(AllocAddrNode* node) // DBG
//{
//    if (node)
//    {
//        traverseAddrTree(node->AddrChild[0]);
//        printf("%u ", node->Size);
//        traverseAddrTree(node->AddrChild[1]);
//    }
//}

//------------------------------------------------------------------------
UPInt AllocAddr::Alloc(UPInt size)
{
    SF_ASSERT(size);
    FreeNode* best = pullBest(size);
    if (best)
    {
        UPInt addr = best->Addr;
        splitNode(best, best->Addr, size);
        return addr;
    }
    return ~UPInt(0);
}


//------------------------------------------------------------------------
UPInt AllocAddr::Alloc(UPInt size, UPInt align)
{
    SF_ASSERT(size);
    if (align < 2)
        return Alloc(size);

    SF_ASSERT((align & (align-1)) == 0);

    // The algorithm performs search in the best-fit nodes.
    // The tree node may have a list of nodes of the same size.
    // If the node conforms with the alignment it splits the node and returns the address.
    // If the there's no suitable nodes in the list, we just increase the size in order
    // to find a bigger node.
    //size = (size + align-1) & ~(align-1);
    UPInt pullSize = size;
    for(;;)
    {
        FreeNode* best = (FreeNode*)SizeTree.FindBestGrEq(pullSize);
        if (best == 0)
            break;

        // Increase pull size in order to find next best in AddrTree.
        pullSize = best->Size + 1;

        FreeNode* stop = best;
        do
        {
            UPInt aligned = (best->Addr + align-1) & ~(align-1);
            if (aligned + size < best->Addr + best->Size)
            {
                SizeTree.Remove(best);
                AddrTree.Remove(best);
                splitNode(best, aligned, size);
                return aligned;
            }
            best = best->pNext;
        } while(best != stop);
    }
    return ~UPInt(0);
}



//------------------------------------------------------------------------
UPInt AllocAddr::Free(UPInt addr, UPInt size)
{
    // This class does not handle freeing of zero properly.
    if ( size == 0 )
        return 0;

    FreeNode* prev = (FreeNode*)AddrTree.FindLeEq(addr);
    FreeNode* next = (FreeNode*)AddrTree.FindGrEq(addr + size);

    // Reset prev if not adjacent
    //-----------------
    if (prev == 0 || prev->Addr + prev->Size != addr)
    {
        prev = 0;
    }

    // Reset next if not adjacent
    //-----------------
    if (next == 0 || next->Addr != addr + size)
    {
        next = 0;
    }

    return mergeNodes(prev, next, addr, size);

    //traverseAddrTree(AddrTree.Root);   // DBG
    //printf("\n");                      // DBG
}

//------------------------------------------------------------------------
void AllocAddr::AddSegment(UPInt addr, UPInt size)
{
    Free(addr, size);
}

//------------------------------------------------------------------------
void AllocAddr::RemoveSegment(UPInt addr, UPInt size)
{
    FreeNode* node = (FreeNode*)AddrTree.FindLeEq(addr);
    SF_ASSERT(node && node->Addr <= addr);
    SF_ASSERT(node->Addr + node->Size >= addr + size);
    pullNode(node);
    splitNode(node, addr, size);
}


static void calcTreeSize(const AllocAddrNode* node, UPInt* size)
{
    if (node)
    {
        *size += node->Size;
        calcTreeSize(node->AddrChild[0], size);
        calcTreeSize(node->AddrChild[1], size);
    }
}

//------------------------------------------------------------------------
UPInt AllocAddr::GetFreeSize() const
{
    UPInt size = 0;
    calcTreeSize(AddrTree.Root, &size);
    return size;
}


} // Scaleform
