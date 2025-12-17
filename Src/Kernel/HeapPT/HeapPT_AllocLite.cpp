/**************************************************************************

Filename    :   HeapPT_AllocLite.cpp
Content     :   Allocator based on binary search trees
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   The granulator requests the system allocator for 
                large memory blocks and granulates them as necessary,
                providing smaller granularity to the consumer.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Alg.h"
#include "HeapPT_AllocLite.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

//------------------------------------------------------------------------
AllocLite::AllocLite(UPInt minSize) :
    MinShift(Alg::UpperBit(minSize)),
    MinSize(UPInt(1) << MinShift),
    MinMask(MinSize - 1),
    SizeTree(),
    AddrTree(),
    FreeBlocks(0)
{
    // Restrictions for minSize: at least 16 pointers; at most the 
    // capacity of UPIntHalf. The latest is required because the 
    // number of unaligned head bytes of the segment is stored in 
    // UPIntHalf; see struct Segment.
    //----------------------
    SF_HEAP_ASSERT(minSize >= sizeof(DualTNode));
    SF_HEAP_ASSERT(minSize <= (UPInt(1) << (8*sizeof(UPIntHalf))));
    SF_HEAP_ASSERT(minSize && ((minSize-1) & minSize) == 0);
}


// TO DO: Optimization is possible. Avoid extra Remove/Insert from/to AddrTree
// Consider using a single Size tree instead of ListBin and TreeBin.

//------------------------------------------------------------------------
inline void AllocLite::pushNode(FreeNode* node, TreeSeg* seg, UPInt blocks)
{
    node->Size = blocks;
    node->ParentSeg = seg;
    SizeTree.Insert(node);
    AddrTree.Insert(node);
    FreeBlocks += blocks;
}


//------------------------------------------------------------------------
inline void AllocLite::pullNode(FreeNode* node)
{
    FreeBlocks -= node->Size;
    SizeTree.Remove(node);
    AddrTree.Remove(node);
}


//------------------------------------------------------------------------
AllocLite::FreeNode* AllocLite::pullBest(UPInt blocks)
{
    FreeNode* best = SizeTree.PullBestGrEq(blocks);
    if (best)
    {
        AddrTree.Remove(best);
        FreeBlocks -= best->Size;
    }
    return best;
}


// Best fit with the given alignment
//------------------------------------------------------------------------
AllocLite::FreeNode* AllocLite::pullBest(UPInt blocks, UPInt alignMask)
{
    if (alignMask <= MinMask)
    {
        return pullBest(blocks);
    }

    // Slightly slower but more precise version
    FreeNode* best;
    FreeNode* head;
    UPInt search = blocks;
    while((head = best = (FreeNode*)SizeTree.FindBestGrEq(search)) != 0)
    {
        do
        {
            if (best->AlignmentIsOK(blocks, MinShift, alignMask))
            {
                SizeTree.Remove(best);
                AddrTree.Remove(best);
                FreeBlocks -= best->Size;
                return best;
            }
            best = best->pNext;
        }
        while(best != head);
        search = best->Size + 1;
    }
    return best;

    //// Slightly faster but less precise version
    //FreeNode* best;
    //FreeNode* head = best = (FreeNode*)SizeTree.FindBestGrEq(blocks);
    //if (best) do
    //{
    //    if (best->AlignmentIsOK(blocks, MinShift, alignMask))
    //    {
    //        SizeTree.Remove(best);
    //        AddrTree.Remove(best);
    //        FreeBlocks -= best->Size;
    //        return best;
    //    }
    //    best = best->pNext;
    //}
    //while(best != head);

    //UPInt extra = (alignMask + 1) >> MinShift;
    //best = (FreeNode*)SizeTree.FindBestGrEq(blocks + extra);

    //// At this point the alignment must be OK because we add
    //// "extra" to the size for search. 
    ////-------------------------
    //if (best)
    //{
    //    SF_HEAP_ASSERT(best->AlignmentIsOK(blocks, MinShift, alignMask));
    //    SizeTree.Remove(best);
    //    AddrTree.Remove(best);
    //    FreeBlocks -= best->Size;
    //}
    //return best;
}


//------------------------------------------------------------------------
void AllocLite::splitNode(FreeNode* node, UByte* start, UPInt size)
{
    TreeSeg* seg = node->ParentSeg;
    UPInt  headBytes = start - (UByte*)node;
    UPInt  tailBytes = (UPInt(node) + (node->Size << MinShift)) - 
                       (UPInt(start) + size);
    if (headBytes)
    {
        pushNode(node, seg, headBytes >> MinShift);
    }
    if (tailBytes)
    {
        pushNode((FreeNode*)(start + size), seg, tailBytes >> MinShift);
    }
}


//------------------------------------------------------------------------
void AllocLite::InitSegment(TreeSeg* seg)
{
    pushNode((FreeNode*)seg->Buffer, seg, seg->Size >> MinShift);
}


//------------------------------------------------------------------------
void AllocLite::ReleaseSegment(TreeSeg* seg)
{
    pullNode((FreeNode*)seg->Buffer);
}

//#include <stdio.h> // DBG
//static void traverseSizeTree(DualTNode* node, UPInt shift) // DBG
//{
//    if (node)
//    {
//        traverseSizeTree(node->Child[0], shift);
//        const DualTNode* n2 = node;
//        do
//        {
//            n2 = n2->pNext;
//            printf("%u ", node->Size << shift);
//        }
//        while(n2 != node);
//        traverseSizeTree(node->Child[1], shift);
//    }
//}

//------------------------------------------------------------------------
void* AllocLite::Alloc(UPInt size, UPInt alignSize, TreeSeg** allocSeg)
{
    if (size < MinSize) 
        size = MinSize;

    if (alignSize < MinSize) 
        alignSize = MinSize;

    SF_HEAP_ASSERT(((alignSize-1) & alignSize) == 0);

    size = (size + alignSize-1) & ~(alignSize-1);

    FreeNode* best = pullBest(size >> MinShift, alignSize-1);
    if (best)
    {
        SF_HEAP_ASSERT(best->AlignmentIsOK(size >> MinShift, MinShift, alignSize-1));
        UByte* aligned = DualTNode::GetAlignedPtr((UByte*)best, alignSize-1);
        *allocSeg = best->ParentSeg;
        splitNode(best, aligned, size);
        return aligned;
    }
    //printf("%u %u: ", size, alignSize);               // DBG
    //traverseSizeTree(SizeTree.Tree.Root, MinShift);   // DBG
    //printf("\n");                                     // DBG
    return 0;
}

//------------------------------------------------------------------------
void AllocLite::Free(TreeSeg* seg, void* ptr, UPInt size, UPInt alignSize)
{
    if (size < MinSize) 
        size = MinSize;

    if (alignSize < MinSize) 
        alignSize = MinSize;

    SF_HEAP_ASSERT(((alignSize-1) & alignSize) == 0);

    size = (size + alignSize-1) & ~(alignSize-1);

    FreeNode* prev = (FreeNode*)AddrTree.FindLeEq(UPInt(ptr));
    FreeNode* next = (FreeNode*)AddrTree.FindGrEq(UPInt(ptr) + size);

    // Reset prev if not adjacent
    //-----------------
    if ((UByte*)prev == 0 || prev->ParentSeg != seg || 
        (UByte*)prev + (prev->Size << MinShift) != (UByte*)ptr)
    {
        prev = 0;
    }

    // Reset next if not adjacent
    //-----------------
    if ((UByte*)next == 0 || next->ParentSeg != seg ||
        (UByte*)next != (UByte*)ptr + size)
    {
        next = 0;
    }

    // The prev and next blocks (if exist) must be within the segment
    //-----------------
    SF_HEAP_ASSERT(prev == 0 || (UByte*)prev >= seg->Buffer);
    SF_HEAP_ASSERT(next == 0 || (UByte*)next <  seg->Buffer + seg->Size);

    // Merge
    //-----------------
    FreeNode* node = (FreeNode*)ptr;
    node->Size = size >> MinShift;
    node->ParentSeg = seg;

    UPInt     mergeSize = node->Size;
    FreeNode* mergeNode = node;
    if (prev)
    {
        mergeNode  = prev;
        mergeSize += prev->Size;
        pullNode(prev);
    }
    if (next)
    {
        mergeSize += next->Size;
        pullNode(next);
    }
    pushNode(mergeNode, seg, mergeSize);
}


//------------------------------------------------------------------------
AllocLite::ReallocResult 
AllocLite::ReallocInPlace(TreeSeg* seg, void* ptr, UPInt oldSize, 
                          UPInt newSize, UPInt alignSize)
{
    SF_HEAP_ASSERT(ptr && oldSize && newSize);

    if (alignSize < MinSize) 
        alignSize = MinSize;

    SF_HEAP_ASSERT(((alignSize-1) & alignSize) == 0);

    oldSize = (oldSize + alignSize-1) & ~(alignSize-1);
    newSize = (newSize + alignSize-1) & ~(alignSize-1);

    if (newSize == oldSize)
    {
        return ReallocSucceeded;
    }

    if (newSize > oldSize)
    {
        FreeNode* next = (FreeNode*)AddrTree.FindGrEq(UPInt(ptr) + oldSize);
        if (next && next->ParentSeg == seg && 
            (UByte*)next == (UByte*)ptr + oldSize)
        {
            // The next block must be within the segment
            SF_HEAP_ASSERT((UByte*)next < seg->Buffer + seg->Size);

            UPInt freeSize = next->Size << MinShift;
            if ((UByte*)ptr + newSize <= (UByte*)next + freeSize)
            {
                pullNode(next);
                splitNode(next, (UByte*)next, newSize - oldSize);
                return ReallocSucceeded;
            }
            if ((UByte*)next + freeSize == seg->Buffer + seg->Size)
            {
                return ReallocFailedAtTail;
            }
        }
        if ((UByte*)ptr + oldSize == seg->Buffer + seg->Size)
        {
            return ReallocFailedAtTail;
        }
        return ReallocFailed;
    }

    // oldSize < newSize
    Free(seg, (UByte*)ptr + newSize, oldSize - newSize, alignSize);

    UPInt  freeTail = ((FreeNode*)((UByte*)ptr + newSize))->Size << MinShift;
    UByte* endPtr   = seg->Buffer + seg->Size;

    return ((UByte*)ptr + newSize + freeTail == endPtr) ?
        ReallocShrinkedAtTail : ReallocSucceeded;
}


//------------------------------------------------------------------------
void AllocLite::Extend(TreeSeg* seg, UPInt incSize)
{
    FreeNode* prev = (FreeNode*)AddrTree.FindLeEq(UPInt(seg->Buffer + seg->Size)-1);
    if (prev && prev->ParentSeg == seg && 
        (UByte*)prev + (prev->Size << MinShift) == seg->Buffer + seg->Size)
    {
        // Merge with the previous adjacent node
        //-----------------
        pullNode(prev);
        pushNode(prev, seg, prev->Size + (incSize >> MinShift));
    }
    else
    {
        // Nothing to merge with
        //-----------------
        pushNode((FreeNode*)(seg->Buffer + seg->Size), seg, incSize >> MinShift);
    }
    seg->Size += incSize;
}

//------------------------------------------------------------------------
bool AllocLite::TrimAt(TreeSeg* seg, UByte* ptrAt)
{
    DualTNode* node = (DualTNode*)AddrTree.FindLeEq(UPInt(ptrAt));
    if (node && node->ParentSeg == seg &&
        ptrAt >= (UByte*)node && 
        ptrAt <  (UByte*)node + (node->Size << MinShift))
    {
        pullNode(node);
        UPInt tailBytes = (UByte*)node + (node->Size << MinShift) - ptrAt;
        SF_HEAP_ASSERT(tailBytes);
        if ((UByte*)node < ptrAt)
        {
            pushNode(node, seg, (ptrAt - (UByte*)node) >> MinShift);
            node = (DualTNode*)ptrAt;
            node->ParentSeg = seg;
            node->Size      = tailBytes >> MinShift;
        }
        seg->Size -= tailBytes;
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void AllocLite::visitTree(const FreeNode* root, 
                          HeapSegment* seg,
                          MemVisitor* visitor,
                          MemVisitor::Category cat) const
{
    if (root)
    {
        visitTree(root->Child[0], seg, visitor, cat);
        const FreeNode* node = root;
        do
        {
            seg->DataSize = node->ParentSeg->Size;
            seg->pData    = node->ParentSeg->Buffer;
            visitor->Visit(seg, UPInt(node), node->Size << MinShift, cat);
            node = node->pNext;
        }
        while(node != root);
        visitTree(root->Child[1], seg, visitor, cat);
    }
}


//------------------------------------------------------------------------
void AllocLite::VisitMem(MemVisitor* visitor, MemVisitor::Category cat) const
{
    HeapSegment seg; // Fake segment
    seg.SelfSize  = 0;
    seg.SegType   = Heap_SegmentUndefined;
    seg.Alignment = 0;
    seg.UseCount  = 0;
    seg.pHeap     = 0;
    seg.DataSize  = 0;
    seg.pData     = 0;
    visitTree(SizeTree.Tree.Root, &seg, visitor, cat);
}


//------------------------------------------------------------------------
void AllocLite::visitUnusedNode(const FreeNode* node, 
                                SegVisitor* visitor, 
                                unsigned cat) const
{
    UPInt start = (UPInt(node) + Heap_PageMask)            & ~Heap_PageMask;
    UPInt end   = (UPInt(node) + (node->Size << MinShift)) & ~Heap_PageMask;
    if (start + Heap_PageSize <= end)
    {
        visitor->Visit(cat, 0, start, end - start);
    }
}

//------------------------------------------------------------------------
void AllocLite::visitUnusedInTree(const FreeNode* root, 
                                  SegVisitor* visitor, 
                                  unsigned cat) const
{
    if (root)
    {
        visitUnusedInTree(root->Child[0], visitor, cat);
        const FreeNode* node = root;
        do
        {
            visitUnusedNode(node, visitor, cat);
            node = node->pNext;
        }
        while(node != root);
        visitUnusedInTree(root->Child[1], visitor, cat);
    }
}


//------------------------------------------------------------------------
void AllocLite::VisitUnused(SegVisitor* visitor, unsigned cat) const
{
    visitUnusedInTree(SizeTree.Tree.Root, visitor, cat);
}

}} // Scaleform::Heap
