/**************************************************************************

Filename    :   HeapPT_AllocBitSet2.cpp
Content     :   Bit-set based allocator, 2 bits per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Alg.h"
#include "HeapPT_AllocBitSet2.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
AllocBitSet2::AllocBitSet2(UPInt minAlignShift) :
    MinAlignShift(minAlignShift),
    MinAlignMask((UPInt(1) << minAlignShift) - 1)
{
}


//------------------------------------------------------------------------
void AllocBitSet2::InitSegment(HeapSegment* seg)
{
    BitSet2::MarkFree(GetBitSet(seg), 0, seg->DataSize >> MinAlignShift);
    Bin.Push(seg, seg->pData, seg->DataSize >> MinAlignShift, MinAlignShift);
}


//------------------------------------------------------------------------
void AllocBitSet2::ReleaseSegment(HeapSegment* seg)
{
    Bin.Pull(seg->pData);
}

//------------------------------------------------------------------------
void* AllocBitSet2::Alloc(UPInt bytes, HeapSegment** allocSeg)
{
    // Size must be greater or equal than Heap_MinSize.
    // The conditions must be verified externally.
    SF_HEAP_ASSERT(bytes >= Heap_MinSize && 
                (bytes & MinAlignMask) == 0);

    UByte* best = (UByte*)Bin.PullBest(bytes >> MinAlignShift);
    if (best)
    {
#ifdef SF_MEMORY_CHECK_CORRUPTION
        Bin.CheckNode((BinLNode*)best, MinAlignShift);
#endif
        HeapSegment* seg   = ((BinLNode*)best)->pSegment;
        UPInt   shift      = MinAlignShift;
        UPInt   blocks     = bytes >> shift;
        UInt32* bitSet     = GetBitSet(seg);
        UByte*  base       = seg->pData;
        UPInt   tailBlocks = Bin.GetSize(best) - blocks;
        UPInt   tailBytes  = tailBlocks << shift;

        // Free blocks with size less than Heap_MinSize are not allowed.
        // If it happens, just merge this "tail" with allocated block.
        //--------------------------
        if (tailBytes >= Heap_MinSize)
        {
            Bin.Push(seg, best + bytes, tailBlocks, shift);
            BitSet2::MarkFree(bitSet, (best + bytes - base) >> shift, tailBlocks);
        }
        else
        {
            bytes += tailBytes;
        }
        BitSet2::MarkBusy(bitSet, (best - base) >> shift, bytes >> shift, 0);
        *allocSeg = seg;
    }
    return best;
}



// Alloc with the given alignment.
//------------------------------------------------------------------------
void* AllocBitSet2::Alloc(UPInt bytes, UPInt alignSize, HeapSegment** allocSeg)
{
    // Size and alignSize must meet the requirements below. 
    // The conditions must be verified externally.
    SF_HEAP_ASSERT(bytes     >= alignSize && 
                   bytes     >= Heap_MinSize && 
                   alignSize >= MinAlignMask+1 &&
                   (bytes & (alignSize-1)) == 0);

    UPInt  shift     = MinAlignShift;
    UPInt  blocks    = bytes >> shift;
    UPInt  alignMask = alignSize-1;
    UByte* best      = (UByte*)Bin.PullBest(blocks, shift, alignMask);
    if (best)
    {
#ifdef SF_MEMORY_CHECK_CORRUPTION
        Bin.CheckNode((BinLNode*)best, shift);
#endif
        HeapSegment* seg  = ((BinLNode*)best)->pSegment;
        UInt32* bitSet    = GetBitSet(seg);
        UByte*  base      = seg->pData;
        UByte*  aligned   = ListBin::GetAlignedPtr(best, alignMask);
        UPInt   headBytes = aligned - best;
        UPInt   tailBytes = (best + (Bin.GetSize(best) << shift)) - (aligned + bytes);
        if (headBytes)
        {
            // PullBest with the given alignment guarantees that the 
            // "empty head" cannot be less than Heap_MinSize, so that, 
            // we can add this block for sure.
            //--------------------------
            SF_HEAP_ASSERT(headBytes >= Heap_MinSize);
            Bin.Push(seg, best, headBytes >> shift, shift);
            BitSet2::MarkFree(bitSet, (best - base) >> shift, headBytes >> shift);
        }

        // Free blocks with size less than Heap_MinSize are not allowed.
        // If it happens, just merge this "tail" with allocated block.
        //--------------------------
        if (tailBytes >= Heap_MinSize)
        {
            Bin.Push(seg, aligned + bytes, tailBytes >> shift, shift);
            BitSet2::MarkFree(bitSet, (aligned + bytes - base) >> shift, tailBytes >> shift);
        }
        else
        {
            bytes += tailBytes;
        }
        BitSet2::MarkBusy(bitSet, (aligned - base) >> shift, 
                          bytes >> shift, Alg::UpperBit(alignSize) - shift);
        *allocSeg = seg;
        return aligned;
    }
    return 0;
}


//------------------------------------------------------------------------
void AllocBitSet2::Free(HeapSegment* seg, void* ptr)
{
    bool    left, right, tail;
    UPInt   blocks;
    UPInt   bytes;
    UByte*  base   = seg->pData;
    UByte*  end    = base + seg->DataSize;
    UInt32* bitSet = GetBitSet(seg);
    UPInt   shift  = MinAlignShift;
    UPInt   start  = ((UByte*)ptr - base) >> shift;

    blocks = BitSet2::GetBlockSize(bitSet, start);
    SF_HEAP_ASSERT(blocks);
    bytes  = blocks << shift;

    BitSet2::MarkFree(bitSet, start, blocks);
    tail  = (UByte*)ptr + bytes < end;
    left  = start && BitSet2::GetValue(bitSet, start - 1     ) == 0;
    right = tail  && BitSet2::GetValue(bitSet, start + blocks) == 0;
    Bin.MakeNode(seg, (UByte*)ptr, blocks, shift);
    if (left | right)
    {
        Bin.Merge((UByte*)ptr, shift, left, right);
    }
    else
    {
        Bin.Push((UByte*)ptr);
    }
}

//------------------------------------------------------------------------
void* AllocBitSet2::ReallocInPlace(HeapSegment* seg, void* oldPtr, 
                                   UPInt newSize, UPInt* oldSize)
{
    SF_HEAP_ASSERT(oldPtr && newSize && 
                   newSize >= Heap_MinSize &&
                  (newSize & MinAlignMask) == 0);

    UPInt    newBytes = newSize;
    UByte*   base     = seg->pData;
    UByte*   end      = base + seg->DataSize;
    UInt32*  bitSet   = GetBitSet(seg);
    UPInt    shift    = MinAlignShift;
    UPInt    start    = ((UByte*)oldPtr - base) >> shift;
    UPInt    blocks   = BitSet2::GetBlockSize(bitSet, start);
    unsigned alignSh  = BitSet2::GetAlignShift(bitSet, start, blocks);
    UPInt    alignMsk = (UPInt(1) << (alignSh + MinAlignShift)) - 1;
    UPInt    oldBytes = blocks << shift;
    UByte*   nextAdj;

    *oldSize = oldBytes;
    newBytes = (newBytes + alignMsk) & ~alignMsk;

    if (newBytes > oldBytes)                // Grow
    {
        nextAdj = (UByte*)oldPtr + oldBytes;
        if (nextAdj < end && BitSet2::GetValue(bitSet, start + blocks) == 0)
        {
            UPInt nextBytes = Bin.GetSize(nextAdj) << shift;
            SF_HEAP_ASSERT(nextBytes);
            if (newBytes <= oldBytes + nextBytes)
            {
                Bin.Pull(nextAdj);
                UPInt tailBytes = oldBytes + nextBytes - newBytes;
                if (tailBytes >= Heap_MinSize)
                {
                    UByte* tailBlock  = (UByte*)oldPtr + newBytes;
                    Bin.Push(seg, tailBlock, tailBytes >> shift, shift);

                    BitSet2::MarkFree(bitSet, (tailBlock - base) >> shift, tailBytes >> shift);
                }
                else
                {
                    newBytes += tailBytes;
                }
                BitSet2::MarkBusy(bitSet, start, newBytes >> shift, alignSh);
                return oldPtr;
            }
        }
        return 0;   // Can't grow
    }

    if (newBytes < oldBytes)                // Shrink
    {
        UPInt thisTail = oldBytes - newBytes;
        UPInt nextTail = 0;

        nextAdj = (UByte*)oldPtr + oldBytes;
        if (nextAdj < end && BitSet2::GetValue(bitSet, start + blocks) == 0)
        {
            nextTail = Bin.GetSize(nextAdj) << shift;
            SF_HEAP_ASSERT(nextTail);
        }
        if (thisTail + nextTail >= Heap_MinSize)
        {
            if (nextTail)
            {
                Bin.Pull(nextAdj);
            }
            nextAdj   = (UByte*)oldPtr + newBytes;
            thisTail += nextTail;

            Bin.Push(seg, nextAdj, thisTail >> shift, shift);
            BitSet2::MarkBusy(bitSet, start, newBytes >> shift, alignSh);
            BitSet2::MarkFree(bitSet, (nextAdj - base) >> shift, thisTail >> shift);
        }
    }
    return oldPtr;
}


//------------------------------------------------------------------------
UPInt AllocBitSet2::GetUsableSize(const HeapSegment* seg, const void* ptr) const
{
    UPInt start = ((UByte*)ptr - seg->pData) >> MinAlignShift;
    return BitSet2::GetBlockSize(GetBitSet(seg), start) << MinAlignShift;
}

//------------------------------------------------------------------------
UPInt AllocBitSet2::GetAlignShift(const HeapSegment* seg, const void* ptr, UPInt size) const
{
    UPInt    start = ((UByte*)ptr - seg->pData) >> MinAlignShift;
    unsigned al = BitSet2::GetAlignShift(GetBitSet(seg), start, size >> MinAlignShift);
    return MinAlignShift + al;
}

}} // Scaleform::Heap
