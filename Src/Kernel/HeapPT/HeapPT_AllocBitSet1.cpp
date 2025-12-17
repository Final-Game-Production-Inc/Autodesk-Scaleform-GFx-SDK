/**************************************************************************

Filename    :   HeapPT_AllocBitSet1.cpp
Content     :   Bit-set based allocator, 1 bit per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_AllocBitSet1.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
AllocBitSet1::AllocBitSet1(UPInt minAlignShift) :
    MinAlignShift(minAlignShift),
    MinAlignMask((UPInt(1) << minAlignShift) - 1)
{
}

//------------------------------------------------------------------------
void AllocBitSet1::InitSegment(HeapSegment* seg)
{
    UPInt blocks = seg->DataSize >> MinAlignShift;
    markFree(GetBitSet(seg), 0, blocks);
    Bin.Push(seg, seg->pData, blocks, MinAlignShift);
}

//------------------------------------------------------------------------
void AllocBitSet1::ReleaseSegment(HeapSegment* seg)
{
    Bin.Pull(seg->pData);
}

//------------------------------------------------------------------------
void* AllocBitSet1::Alloc(UPInt bytes, HeapSegment** allocSeg)
{
    SF_HEAP_ASSERT(bytes >= Heap_MinSize && (bytes & MinAlignMask) == 0);

    UPInt  minAlignShift = MinAlignShift;
    UPInt  blocks = bytes >> minAlignShift;
    UByte* best   = (UByte*)Bin.PullBest(blocks);
    if (best)
    {
#ifdef SF_MEMORY_CHECK_CORRUPTION
        Bin.CheckNode((BinLNode*)best, minAlignShift);
#endif
        HeapSegment* seg = ((BinLNode*)best)->pSegment;
        UPInt tailBlocks = Bin.GetSize(best) - blocks;
        UPInt tailBytes  = tailBlocks << minAlignShift;

        if (tailBytes >= Heap_MinSize)
        {
            Bin.Push(seg, best + bytes, tailBlocks, minAlignShift);
            markFree(GetBitSet(seg),
                     (best - seg->pData + bytes) >> minAlignShift, 
                     tailBlocks);
        }
        else
        {
            bytes += tailBytes;
        }
        markBusy(GetBitSet(seg),
                 (best - seg->pData) >> minAlignShift, 
                 bytes >> minAlignShift);
        *allocSeg = seg;
    }
    return best;
}

//------------------------------------------------------------------------
void AllocBitSet1::Free(HeapSegment* seg, void* ptr, UPInt bytes)
{
    SF_HEAP_ASSERT(bytes >= Heap_MinSize && (bytes & MinAlignMask) == 0);

    bool    left, right, tail;
    UPInt   blocks;
    UByte*  base   = seg->pData;
    UByte*  end    = base + seg->DataSize;
    UInt32* bitSet = GetBitSet(seg);
    UPInt   start  = ((UByte*)ptr - base) >> MinAlignShift;

    blocks = bytes >> MinAlignShift;
    markFree(bitSet, start, blocks);
    tail  = (UByte*)ptr + bytes < end;
    left  = start && isZero(bitSet, start - 1     );
    right = tail  && isZero(bitSet, start + blocks);
    Bin.MakeNode(seg, (UByte*)ptr, blocks, MinAlignShift);
    if (left | right)
    {
        Bin.Merge((UByte*)ptr, MinAlignShift, left, right);
    }
    else
    {
        Bin.Push((UByte*)ptr);
    }
}

}} // Scaleform::Heap
