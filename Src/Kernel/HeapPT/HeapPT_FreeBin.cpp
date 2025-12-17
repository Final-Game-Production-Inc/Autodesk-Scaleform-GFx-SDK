/**************************************************************************

Filename    :   HeapPT_FreeBin.cpp
Content     :   
Created     :   2009
Authors     :   Maxim Shemanarev, Boris Rayskiy

Notes       :   Containers to store information about free memory areas

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_FreeBin.h"
#include "HeapPT_MemoryHeap.h"
#include "../SF_HeapBitSet2.h"
#include "../SF_Alg.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
ListBin::ListBin()
{
    Reset();
}
void ListBin::Reset()
{
    Mask = 0;
    memset(Roots, 0, sizeof(Roots));
}


//------------------------------------------------------------------------
TreeBin::TreeBin()
{
    Reset();
}
void TreeBin::Reset()
{
    Mask = 0;
    memset(Roots, 0, sizeof(Roots));
}

//--------------------------------------------------------------------
UByte* ListBin::GetAlignedPtr(UByte* start, UPInt alignMask)
{
    UPInt aligned = (UPInt(start) + alignMask) & ~alignMask;
    UPInt head = aligned - UPInt(start);
    while (head && head < Heap_MinSize)
    {
        aligned += alignMask+1;
        head    += alignMask+1;
    }
    return (UByte*)aligned;
}

//--------------------------------------------------------------------
bool ListBin::AlignmentIsOK(const BinLNode* node, UPInt blocks,
                            UPInt shift, UPInt alignMask)
{
    UPInt  bytes   = blocks << shift;
    UPInt  aligned = UPInt(GetAlignedPtr((UByte*)node, alignMask));
    return aligned + bytes <= UPInt(node) + FreeBin::GetBytes(node, shift);
}


//--------------------------------------------------------------------
SF_INLINE void ListBin::PushNode(UPInt idx, BinLNode* node)
{
    BinLNode* root = Roots[idx];
    if (root)
    {
        node->pPrev = root;
        node->pNext = root->pNext;
        root->pNext->pPrev = node;
        root->pNext = node;
    }
    else
    {
        node->pPrev = node->pNext = node;
    }
    Roots[idx] = node;
    Mask |= UPInt(1) << idx;
}

//--------------------------------------------------------------------
SF_INLINE void ListBin::PullNode(UPInt idx, BinLNode* node)
{
    BinLNode* root = Roots[idx];
    if (node != root)
    {
        node->pPrev->pNext = node->pNext;
        node->pNext->pPrev = node->pPrev;
        return;
    }
    if (root != root->pNext)
    {
        Roots[idx] = root->pNext;
        node->pPrev->pNext = node->pNext;
        node->pNext->pPrev = node->pPrev;
        return;
    }
    Roots[idx] = 0;
    Mask &= ~(UPInt(1) << idx);
}

//--------------------------------------------------------------------
SF_INLINE BinLNode* ListBin::PullBest(UPInt idx)
{
    BinLNode* best = 0;
    UPInt bits = Mask >> idx;
    if (bits)
    {
        // Go straight to the first not empty list. No searching loops.
        //----------------------
        UPInt i = idx + Alg::LowerBit(bits);
        best = Roots[i];
        SF_HEAP_ASSERT(best);
        PullNode(i, best);
    }
    return best;
}

//--------------------------------------------------------------------
BinLNode* ListBin::FindAligned(BinLNode* root, UPInt blocks, 
                               UPInt shift, UPInt alignMask)
{
    BinLNode* node = root;
    if (node) do
    {
        if (AlignmentIsOK(node, blocks, shift, alignMask))
        {
            return node;
        }
        node = node->pNext;
    }
    while(node != root);
    return 0;
}

//--------------------------------------------------------------------
BinLNode* ListBin::PullBest(UPInt idx, UPInt blocks, UPInt shift, UPInt alignMask)
{
    BinLNode* best = 0;
    UPInt bits = Mask >> idx;
    if (bits)
    {
        // Go straight to the first not empty list. No searching loops.
        //----------------------
        UPInt i = idx + Alg::LowerBit(bits);
        SF_HEAP_ASSERT(Roots[i]);

        // TO DO: Can be optimized by skipping empty roots 
        // according to the Mask.
        do
        {
            best = FindAligned(Roots[i], blocks, shift, alignMask);
            if (best)
            {
                PullNode(i, best);
                break;
            }
        }
        while(++i < BinSize);
    }
    return best;
}




// The TreeBin uses ideas from Doug Lea malloc (aka dlmalloc). 
// dlmalloc has been released to the public domain, as explained at
// http://creativecommons.org/licenses/publicdomain. The tree used 
// there is well proven and tested during years. It's compact and fast.
// Dlmalloc is great masterpiece of public domain software, so, there 
// is the acknowlwgwment: http://g.oswego.edu/ 
// Or just google for "Doug Lea malloc".
//--------------------------------------------------------------------




//--------------------------------------------------------------------
SF_INLINE UPInt TreeBin::ShiftForTreeIndex(UPInt i)
{
    // Computes a left shift for the tree.
    return (i >= BinSize-1) ? 0 : (BinSize-1) - ((i >> 1) + SizeShift - 2);
}


//--------------------------------------------------------------------
SF_INLINE UPInt TreeBin::ComputeTreeIndex(UPInt blocks)
{
    UPInt piles = blocks >> SizeShift;

    if (piles == 0)
        return 0;

    if (piles > SizeLimit)
        return BinSize-1;

    // This is a Doug Lea voodoo that most probably computes the index
    // according to geometric progression with the sqrt(2) coefficient.
    //------------------------
    UPInt   pow2 = Alg::UpperBit(piles);
    return (pow2 << 1) + ((blocks >> (pow2 + (SizeShift - 1))) & 1);
}

//------------------------------------------------------------------------
void TreeBin::PushNode(BinTNode* node)
{
    UPInt size = node->Size;
    UPInt idx  = ComputeTreeIndex(size);

    BinTNode** root = Roots + idx;

    node->Index = idx;
    node->Child[0] = node->Child[1] = 0;

    UPInt mask = UPInt(1) << idx;
    if ((Mask & mask) == 0) 
    {
        Mask |= mask;
        *root = node;
        node->Parent = (BinTNode*)root;
        node->pNext  = node->pPrev = node;
    }
    else 
    {
        BinTNode* head = *root;
        UPInt bits = size << ShiftForTreeIndex(idx);

        for (;;) 
        {
            if (head->Size != size) 
            {
                BinTNode** link = &(head->Child[(bits >> (BinSize-1)) & 1]);
                bits <<= 1;

                if (*link != 0)
                {
                    head = *link;
                }
                else
                {
                    *link = node;
                    node->Parent = head;
                    node->pNext  = node->pPrev = node;
                    break;
                }
            }
            else 
            {
                BinTNode* next = (BinTNode*)head->pNext;
                head->pNext  = next->pPrev = node;
                node->pNext  = next;
                node->pPrev  = head;
                node->Parent = 0;
                break;
            }
        }
    }
}


//------------------------------------------------------------------------
void TreeBin::PullNode(BinTNode* node)
{
    BinTNode* parent = node->Parent;
    BinTNode* rotor;

    if (node->pPrev != node) 
    {
        BinTNode* f = (BinTNode*)node->pNext;
        rotor = (BinTNode*)node->pPrev;

        f->pPrev = rotor;
        rotor->pNext = f;
    }
    else 
    {
        BinTNode** rp;

        if (((rotor = *(rp = &(node->Child[1]))) != 0) || 
            ((rotor = *(rp = &(node->Child[0]))) != 0)) 
        {
            BinTNode** cp;
            while ((*(cp = &(rotor->Child[1])) != 0) || 
                   (*(cp = &(rotor->Child[0])) != 0)) 
            {  
                rotor = *(rp = cp);
            }
            *rp = 0;
        }
    }

    if (parent != 0) 
    {
        BinTNode** root = Roots + node->Index;

        if (node == *root) 
        {
            if ((*root = rotor) == 0)
            {
                Mask &= ~(UPInt(1) << node->Index);
            }
        }    
        else
        {
            parent->Child[parent->Child[0] != node] = rotor;
        }

        if (rotor != 0)
        {
            BinTNode* child;

            rotor->Parent = parent;

            if ((child = node->Child[0]) != 0)
            {
                rotor->Child[0] = child;
                child->Parent   = rotor;
            }

            if ((child = node->Child[1]) != 0)
            {
                rotor->Child[1] = child;
                child->Parent   = rotor;
            }
        }
    }
}


//--------------------------------------------------------------------
BinTNode* TreeBin::FindBest(UPInt size)
{
    BinTNode* best = 0;

    UPInt rsize = -SPInt(size); // Unsigned negation
    UPInt idx   = ComputeTreeIndex(size);
    UPInt tail;

    BinTNode* node = Roots[idx];

    if (node != 0)
    {
        // Traverse tree for this bin looking for node with node->Size == size
        //--------------------------
        BinTNode* rst = 0;  // The deepest untaken right subtree
        UPInt bits = size << ShiftForTreeIndex(idx);
        for (;;)
        {
            BinTNode* rt;
            tail = node->Size - size;
            if (tail < rsize)
            {
                best = node;
                if ((rsize = tail) == 0)
                {
                    break;
                }
            }
            rt   = node->Child[1];
            node = node->Child[(bits >> (BinSize-1)) & 1];
            if (rt != 0 && rt != node)
            {
                rst = rt;
            }
            if (node == 0)
            {
                node = rst; // set node to least subtree holding sizes > size
                break;
            }
            bits <<= 1;
        }
    }

    if (node == 0 && best == 0)
    { 
        // Set node to root of next non-empty bin
        //------------------------
        UPInt x = UPInt(1) << (idx + 1);

        // Mask with all bits to left of least bit of x on,
        // and then mask it with "Mask" (Doug Lea voodoo magic).
        //------------------------
        x = (x | -SPInt(x)) & Mask;
        if (x)
        {
            node = Roots[Alg::LowerBit(x)];
        }
    }

    while (node)
    { 
        // Find smallest of tree or subtree.
        //------------------------
        tail = node->Size - size;
        if (tail < rsize)
        {
            rsize = tail;
            best  = node;
        }
        node = node->Child[node->Child[0] == 0]; // leftmost child
    }

    SF_HEAP_ASSERT(best == 0 || best->Size == rsize + size);
    return best;
}


//--------------------------------------------------------------------
SF_INLINE BinTNode* TreeBin::PullBest(UPInt size)
{
    BinTNode* best = FindBest(size);
    if (best)
    {
        best = (BinTNode*)best->pNext; // May potentially speed-up slightly.
        PullNode(best);
    }
    return best;
}


//--------------------------------------------------------------------
const BinTNode* TreeBin::FindExactSize(UPInt size) const
{
    UPInt idx = ComputeTreeIndex(size);
    const BinTNode* node = Roots[idx];

    // Traverse tree for this bin looking for node with node->Size == size
    //--------------------------
    UPInt bits = size << ShiftForTreeIndex(idx);
    while (node && node->Size != size)
    {
        node = node->Child[(bits >> (BinSize-1)) & 1];
        bits <<= 1;
    }
    return node;
}





//------------------------------------------------------------------------
FreeBin::FreeBin() : FreeBlocks(0) {}

void FreeBin::Reset()
{
    ListBin1.Reset();
    ListBin2.Reset();
    TreeBin1.Reset();
    FreeBlocks = 0;
}


//------------------------------------------------------------------------
void FreeBin::compilerAsserts()
{
    SF_COMPILER_ASSERT(Heap_MinSize >= 4*sizeof(void*));
}

//--------------------------------------------------------------------
SF_INLINE BinLNode* FreeBin::getPrevAdjacent(UByte* node, UPInt shift)
{
    // Get the previous adjacent free block. If there's 
    // an empty space in the bit-set it means that the previous
    // adjacent block is empty and valid. This statement simply
    // reads the length from the end of the previous adjacent 
    // block and subtracts the respective size from the "node" pointer.
    //------------------------
    UPInt size = *(((const UPIntHalf*)node) - 1);
    if (size >= BinSize+1)
    {
        size = *(((const UPInt*)node) - 2);
    }
    return (BinLNode*)(node - (size << shift));
}

//--------------------------------------------------------------------
SF_INLINE BinLNode* FreeBin::getNextAdjacent(UByte* node, UPInt shift)
{ 
    // Get the next adjacent free block. If there's 
    // an empty space in the bit-set it means that the next
    // adjacent block is empty and valid. It just takes
    // the size of the node and adds the respective size to 
    // the "node" pointer.
    //------------------------
    return (BinLNode*)(node + (GetSize(node) << shift));
}









// The node must be a fully constructed BinLNode
//------------------------------------------------------------------------
void FreeBin::Push(UByte* node)
{
    UPInt blocks = GetSize(node);
    FreeBlocks += blocks;
    if (blocks <= 1*BinSize)
    {
        ListBin1.PushNode(blocks-1, (BinLNode*)node);
        return;
    }
    if (blocks <= 2*BinSize)
    {
        ListBin2.PushNode(blocks-BinSize-1, (BinLNode*)node);
        return;
    }
    TreeBin1.PushNode((BinTNode*)node);
}


//------------------------------------------------------------------------
void FreeBin::Pull(UByte* node)
{
    UPInt blocks = GetSize(node);
    FreeBlocks -= blocks;
    if (blocks <= 1*BinSize)
    {
        ListBin1.PullNode(blocks-1, (BinLNode*)node);
        return;
    }
    if (blocks <= 2*BinSize)
    {
        ListBin2.PullNode(blocks-BinSize-1, (BinLNode*)node);
        return;
    }
    TreeBin1.PullNode((BinTNode*)node);
}


//------------------------------------------------------------------------
UByte* FreeBin::PullBest(UPInt blocks)
{
    BinLNode* best = 0;
    if (blocks <= 2*BinSize)
    {
        if (blocks <= 1*BinSize && 
           (best = ListBin1.PullBest(blocks-1)) != 0)
        {
            FreeBlocks -= UPInt(best->ShortSize);
            return (UByte*)best;
        }
        UPInt idx = (blocks <= BinSize) ? 0 : blocks-BinSize-1;
        if ((best = ListBin2.PullBest(idx)) != 0)
        {
            FreeBlocks -= ((BinTNode*)best)->Size;
            return (UByte*)best;
        }
    }
    best = TreeBin1.PullBest(blocks);
    if (best)
    {
        FreeBlocks -= ((BinTNode*)best)->Size;
    }
    return (UByte*)best;
}



// Best fit with the given alignment
//------------------------------------------------------------------------
UByte* FreeBin::PullBest(UPInt blocks, UPInt shift, UPInt alignMask)
{
    if (blocks <= 2*BinSize)
    {
        BinLNode* best = 0;
        if (blocks <= 1*BinSize && 
           (best = ListBin1.PullBest(blocks-1, blocks, shift, alignMask)) != 0)
        {
            FreeBlocks -= UPInt(best->ShortSize);
            return (UByte*)best;
        }

        UPInt idx = (blocks <= BinSize) ? 0 : blocks-BinSize-1;
        if ((best = ListBin2.PullBest(idx, blocks, shift, alignMask)) != 0)
        {
            FreeBlocks -= ((BinTNode*)best)->Size;
            return (UByte*)best;
        }
    }

    // Slightly slower but more precise version
    BinTNode* head;
    BinTNode* best;
    UPInt search = blocks;
    while((head = best = TreeBin1.FindBest(search)) != 0)
    {
        do
        {
            if (ListBin::AlignmentIsOK(best, blocks, shift, alignMask))
            {
                TreeBin1.PullNode(best);
                FreeBlocks -= best->Size;
                return (UByte*)best;
            }
            best = (BinTNode*)best->pNext;
        }
        while(best != head);
        search = best->Size + 1;
    }
    return 0;

    //// Slightly faster but less precise version
    //BinTNode* root = TreeBin1.FindBest(blocks);
    //BinTNode* node = root;
    //if (node) do
    //{
    //    if (BinListBin::AlignmentIsOK(node, blocks, shift, alignMask))
    //    {
    //        TreeBin1.PullNode(node);
    //        FreeBlocks -= node->Size;
    //        return (UByte*)node;
    //    }
    //    node = (BinTNode*)node->pNext;
    //}
    //while(node != root);
    //
    //UPInt extra = (alignMask + 1) >> shift;
    //node = TreeBin1.FindBest(blocks + extra);
    //
    //// At this point the alignment might be not OK because 
    //// of a small heading block before the aligned memory starts. 
    //// If this block is smaller Heap_MinSize, we cannot return it.
    ////-------------------------
    //if (node && ListBin::AlignmentIsOK(node, blocks, shift, alignMask))
    //{
    //    TreeBin1.PullNode(node);
    //    FreeBlocks -= node->Size;
    //    return (UByte*)node;
    //}
    //
    //// 2*extra must satisfy the alignment requirements for sure. 
    //// Code below has practically zero probability of execution.
    //// Still, theoretically the situation may occur.
    ////-------------------------
    //node = TreeBin1.FindBest(blocks + 2*extra);
    //if (node)
    //{
    //    SF_HEAP_ASSERT(ListBin::AlignmentIsOK(node, blocks, shift, alignMask));
    //    TreeBin1.PullNode(node);
    //    FreeBlocks -= node->Size;
    //    return (UByte*)node;
    //}
    //return 0;
}



// The "node" must be a fully constructed BinLNode. Use MakeNode().
//------------------------------------------------------------------------
void FreeBin::Merge(UByte* node, UPInt shift, bool left, bool right)
{
    UPInt     mergeSize = GetSize(node);
    BinLNode* mergeNode = (BinLNode*)node;
    if (left)
    {
        mergeNode  = getPrevAdjacent(node, shift);
        mergeSize += GetSize(mergeNode);
        Pull((UByte*)mergeNode);
#ifdef SF_MEMORY_CHECK_CORRUPTION
        CheckNode(mergeNode, shift);
#endif
    }
    if (right)
    {
        BinLNode* nextAdjacent = getNextAdjacent(node, shift);
        mergeSize += GetSize(nextAdjacent);
        Pull((UByte*)nextAdjacent);
#ifdef SF_MEMORY_CHECK_CORRUPTION
        CheckNode(nextAdjacent, shift);
#endif
    }
    SetSize((UByte*)mergeNode, mergeSize, shift);
#ifdef SF_MEMORY_CHECK_CORRUPTION
    MemsetNode(mergeNode, shift);
#endif
    Push((UByte*)mergeNode);
}


//------------------------------------------------------------------------
void FreeBin::visitUnusedNode(const BinLNode* node, 
                              SegVisitor* visitor, 
                              UPInt shift, unsigned cat) const
{
    UPInt start = (UPInt(node) + Heap_PageMask)         & ~Heap_PageMask;
    UPInt end   = (UPInt(node) + GetBytes(node, shift)) & ~Heap_PageMask;
    if (start + Heap_PageSize <= end)
    {
        visitor->Visit(cat, node->pSegment->pHeap, start, end - start);
    }
}

//------------------------------------------------------------------------
void FreeBin::visitUnusedInTree(const BinTNode* root, 
                                SegVisitor* visitor, 
                                UPInt shift, unsigned cat) const
{
    if (root)
    {
        visitUnusedInTree(root->Child[0], visitor, shift, cat);
        const BinLNode* node = root;
        do
        {
            visitUnusedNode(node, visitor, shift, cat);
            node = node->pNext;
        }
        while(node != root);
        visitUnusedInTree(root->Child[1], visitor, shift, cat);
    }
}

//------------------------------------------------------------------------
void FreeBin::VisitUnused(SegVisitor* visitor, UPInt shift, unsigned cat) const
{
    const BinLNode* node;
    const BinLNode* root;
    for(UPInt i = 0; i < BinSize; ++i)
    {
        node = root = ListBin1.Roots[i];
        if (root && GetBytes(root, shift) >= Heap_PageSize) do
        {
            visitUnusedNode(node, visitor, shift, cat);
            node = node->pNext;
        }
        while(node != root);

        node = root = ListBin2.Roots[i];
        if (root && GetBytes(root, shift) >= Heap_PageSize) do
        {
            visitUnusedNode(node, visitor, shift, cat);
            node = node->pNext;
        }
        while(node != root);

        visitUnusedInTree(TreeBin1.Roots[i], visitor, shift, cat);
    }
}


//------------------------------------------------------------------------
void FreeBin::visitTree(const BinTNode* root, MemVisitor* visitor,
                        UPInt shift, MemVisitor::Category cat) const
{
    if (root)
    {
        visitTree(root->Child[0], visitor, shift, cat);
        const BinTNode* node = root;
        do
        {
            visitor->Visit(node->pSegment, UPInt(node), 
                           node->Size << shift, cat);
            node = (BinTNode*)node->pNext;
        }
        while(node != root);
        visitTree(root->Child[1], visitor, shift, cat);
    }
}


//------------------------------------------------------------------------
void FreeBin::VisitMem(MemVisitor* visitor, UPInt shift, MemVisitor::Category cat) const
{
    UPInt i;
    const BinLNode* node;
    const BinLNode* root;
    for(i = 0; i < BinSize; ++i)
    {
        node = root = ListBin1.Roots[i];
        if (root) do
        {
            visitor->Visit(node->pSegment, UPInt(node), 
                           UPInt(node->ShortSize) << shift, cat);
            node = node->pNext;
        }
        while(node != root);

        node = root = ListBin2.Roots[i];
        if (root) do
        {
            visitor->Visit(node->pSegment, UPInt(node), 
                           GetSize(node) << shift, cat);
            node = node->pNext;
        }
        while(node != root);

        visitTree(TreeBin1.Roots[i], visitor, shift, cat);
    }
}


#ifdef SF_BUILD_DEBUG

//------------------------------------------------------------------------
void FreeBin::checkBlockIntegrity(const BinLNode* node, UPInt shift) const
{
#ifdef SF_MEMORY_CHECK_CORRUPTION
    CheckNode(node, shift);
#endif
    const Heap::HeapSegment* seg = node->pSegment;
    UPInt   blocks = GetSize(node);
    SF_HEAP_ASSERT(blocks);
    UInt32* bitSet = GetBitSet(seg);
    UPInt   start  = (((UByte*)node) - seg->pData) >> shift;
    SF_HEAP_ASSERT(BitSet2::GetValue(bitSet, start) == 0);
    SF_HEAP_ASSERT(BitSet2::GetValue(bitSet, start + blocks - 1) == 0);
}


//------------------------------------------------------------------------
void FreeBin::checkTreeIntegrity(const BinTNode* root, UPInt shift) const
{
    if (root)
    {
        checkTreeIntegrity(root->Child[0], shift);
        const BinTNode* node = root;
        do
        {
            checkBlockIntegrity(node, shift);
            node = (BinTNode*)node->pNext;
        }
        while(node != root);
        checkTreeIntegrity(root->Child[1], shift);
    }
}


// The function is very slow, but in some extreme cases may be handy. 
//------------------------------------------------------------------------
void FreeBin::CheckIntegrity(UPInt shift) const
{
    UPInt i;
    const BinLNode* node;
    const BinLNode* root;
    for(i = 0; i < BinSize; ++i)
    {
        node = root = ListBin1.Roots[i];
        if (root) do
        {
            checkBlockIntegrity(node, shift);
            node = node->pNext;
        }
        while(node != root);

        node = root = ListBin2.Roots[i];
        if (root) do
        {
            checkBlockIntegrity(node, shift);
            node = node->pNext;
        }
        while(node != root);

        checkTreeIntegrity(TreeBin1.Roots[i], shift);
    }
}

#else // SF_BUILD_DEBUG

void FreeBin::checkBlockIntegrity(const BinLNode*, UPInt) const {}
void FreeBin::checkTreeIntegrity(const BinTNode*, UPInt) const {}
void FreeBin::CheckIntegrity(UPInt) const {}

#endif // SF_BUILD_DEBUG


#ifdef SF_MEMORY_CHECK_CORRUPTION

//------------------------------------------------------------------------
void FreeBin::CheckArea(const void* area, UPInt bytes) // static
{
    for(UPInt i = 0; i < bytes; ++i)
    {
        SF_HEAP_ASSERT(((const UByte*)area)[i] == 0xFE);
    }
}

//------------------------------------------------------------------------
void FreeBin::MemsetNode(BinLNode* node, UPInt shift) // static
{
    HeapSegment* seg = node->pSegment;
    UPInt blocks = GetSize(node);
    memset(node, 0xFE, blocks << shift);
    SetSize((UByte*)node, blocks, shift);
    node->pSegment = seg;
}

//------------------------------------------------------------------------
void FreeBin::CheckNode(const BinLNode* node, UPInt shift) // static
{
    UPInt blocks = GetSize(node);
    UPInt bytes  = blocks << shift;
    const UByte* area  = (const UByte*)node;
    SF_HEAP_ASSERT(bytes >= Heap_MinSize);
    if (bytes == Heap_MinSize)
    {
        SF_HEAP_ASSERT(node->ShortSize == Heap_MinSize / (UPInt(1) << shift));
        SF_HEAP_ASSERT(node->Filler == Heap_MinSize / (UPInt(1) << shift));
        return;
    }
    if (blocks <= 1*BinSize)
    {
        SF_HEAP_ASSERT(blocks == *(const UPIntHalf*)(area + bytes - sizeof(UPIntHalf)));
        CheckArea(&node->Filler, bytes - 4*sizeof(void*));
        return;
    }
    const BinTNode* tnode = (const BinTNode*)node;
    SF_HEAP_ASSERT(tnode->ShortSize == BinSize+1);
    SF_HEAP_ASSERT(*(const UPIntHalf*)(area + bytes - sizeof(UPIntHalf)) == BinSize+1);
    SF_HEAP_ASSERT(blocks == *(const UPInt*)(area + bytes - 2*sizeof(UPInt)));
    CheckArea(&tnode->Filler, sizeof(tnode->Filler));
    CheckArea(area + bytes - sizeof(UPInt), sizeof(tnode->Filler));
    if (blocks <= 2*BinSize)
    {
        CheckArea(&tnode->Parent, bytes - 7*sizeof(void*));
    }
    else
    {
        CheckArea(area + sizeof(BinTNode), bytes - 11*sizeof(void*));
    }
}

#endif //SF_HEAP_CHECK_CORRUPTION


}} // Scaleform::Heap

