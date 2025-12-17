/**************************************************************************

Filename    :   HeapPT_SysAllocMapper.cpp
Content     :   System Memory allocator based on memory mapping interface
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Alg.h"
#include "../SF_HeapTypes.h"
#include "HeapPT_SysAllocMapper.h"
#include "HeapPT_BitSet1.h"

//#include <stdio.h>  // DBG
//#include "SF_Memory.h" // DBG

namespace Scaleform {


//------------------------------------------------------------------------
SysAllocMapper::SysAllocMapper(SysMemMapper* mapper, UPInt segSize, 
                               UPInt granularity, bool bestFit) :
    pMapper(mapper),
    PageSize(0),
    SegmSize(segSize),
    Granularity(granularity ? granularity : 1),
    Footprint(0),
    NumSegments(0),
    LastSegment(~UPInt(0)),
    BestFit(bestFit),
    LastUsed(0)
{
    PageSize = pMapper->GetPageSize();
    SF_HEAP_ASSERT((PageSize & (PageSize-1)) == 0);

    PageShift= Alg::UpperBit(PageSize);

    Granularity = (Granularity + PageSize-1) & ~(PageSize-1);
    SF_HEAP_ASSERT((Granularity & (Granularity-1)) == 0);

    SF_HEAP_ASSERT(segSize && (segSize & (segSize-1)) == 0);
    SF_HEAP_ASSERT(segSize >= 2*Granularity);

    Footprint   = 0;
    NumSegments = 0;
    LastSegment = ~UPInt(0);

#ifdef SF_MEMORY_FORCE_SYSALLOC
    Granularity = PageSize;
    BestFit     = false;
#endif

}

//------------------------------------------------------------------------
void SysAllocMapper::GetInfo(Info* i) const
{
    i->MinAlign    = PageSize;
    i->MaxAlign    = 0; // Any alignment
    i->Granularity = Granularity <= Heap::Heap_PageSize ? 0 : Granularity;
    i->HasRealloc  = true;
}


//------------------------------------------------------------------------
UPInt SysAllocMapper::getBitSize(UPInt size) const
{
    return (((size + 8*PageSize-1) >> (PageShift + 3)) + PageSize-1) & ~(PageSize-1);
}

//------------------------------------------------------------------------
UInt32* SysAllocMapper::getBitSet(const UByte* start, UPInt size) const
{
    return (UInt32*)(start + size - getBitSize(size));
}

//------------------------------------------------------------------------
UPInt SysAllocMapper::getEndBit(UPInt size) const
{
    return (size - getBitSize(size)) >> PageShift;
}

//------------------------------------------------------------------------
inline bool 
SysAllocMapper::inRange(const UByte* ptr, const Segment* seg) const
{
    return ptr >= seg->Memory && ptr < seg->Memory + SegmSize;
}


//------------------------------------------------------------------------
UPInt SysAllocMapper::binarySearch(const UByte* ptr) const
{
    UPInt found = 0;
    SPInt len   = NumSegments;
    SPInt half;
    SPInt middle;
    
    while(len > 0) 
    {
        half = len >> 1;
        middle = found + half;
        if (Segments[middle].Memory < (UByte*)ptr)
        {
            found = middle + 1;
            len   = len - half - 1;
        }
        else
        {
            len = half;
        }
    }
    return found;
}

//------------------------------------------------------------------------
UPInt SysAllocMapper::findSegment(const UByte* ptr) const
{
    UPInt found = binarySearch(ptr);
    if (found && inRange(ptr, &Segments[found-1]))
    {
        return found - 1;
    }
    if (found < NumSegments && inRange(ptr, &Segments[found]))
    {
        return found;
    }
    return NumSegments;
}


//------------------------------------------------------------------------
bool SysAllocMapper::reserveSegment(UPInt size)
{
    if (NumSegments < MaxSegments)
    {
        UPInt segSize = SegmSize;

        if ((segSize - getBitSize(segSize)) < size)
        {
            segSize = (size + getBitSize(size) + Granularity-1) / Granularity * Granularity;
        }

        while ((segSize - getBitSize(segSize)) < size)
        {
            segSize += Granularity;
        }

        UByte* ptr = 0;
        do
        {
            ptr = (UByte*)pMapper->ReserveAddrSpace(segSize);
            if (ptr)
            {
                break;
            }
            segSize = (segSize / 2 + Granularity-1) / Granularity * Granularity;
        }
        while (segSize >= 2*Granularity && (segSize - getBitSize(segSize)) >= size);

        if (ptr)
        {
            UInt32* bitSet  = getBitSet(ptr, segSize);
            UPInt   bitSize = getBitSize(segSize);

            bitSet = (UInt32*)pMapper->MapPages(bitSet, bitSize);

            if (bitSet == 0)
            {
                pMapper->ReleaseAddrSpace(ptr, segSize);
                return false;
            }
            HeapPT::BitSet1::Clear(bitSet, bitSize/4);
            HeapPT::BitSet1::SetBit(bitSet, getEndBit(segSize));

            UPInt pos = binarySearch(ptr);
            SF_HEAP_ASSERT(pos < MaxSegments);
            if (pos < NumSegments)
            {
                memmove(Segments + pos + 1, 
                        Segments + pos,
                        sizeof(Segment) * (NumSegments - pos));
            }
            NumSegments++;
            Segment* seg = &Segments[pos];
            seg->Memory    = ptr;
            seg->Size      = segSize;
            seg->PageCount = 0;
//for(UPInt i=1; i < NumSegments; ++i) // DBG
//SF_HEAP_ASSERT(Segments[i].Memory > Segments[i-1].Memory);
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------
void SysAllocMapper::releaseSegment(UPInt pos)
{
    bool t;

    Segment* seg = &Segments[pos];

    UInt32* bitSet  = getBitSet(seg->Memory, seg->Size);
    UPInt   bitSize = getBitSize(seg->Size);

    t = pMapper->UnmapPages(bitSet, bitSize);
    SF_HEAP_ASSERT(t); SF_UNUSED(t);

    t = pMapper->ReleaseAddrSpace(seg->Memory, seg->Size);
    SF_ASSERT(t);

    if (pos + 1 < NumSegments)
    {
        memmove(Segments + pos,
                Segments + pos + 1, 
                sizeof(Segment) * (NumSegments - pos - 1));
    }
    NumSegments--;
    LastSegment = ~UPInt(0);
}


//------------------------------------------------------------------------
UByte* SysAllocMapper::getAlignedPtr(const UByte* ptr, UPInt alignment) const
{
    return (UByte*)((UPInt(ptr) + alignment-1) & ~(alignment-1));
}

//------------------------------------------------------------------------
bool SysAllocMapper::alignmentIsOK(const UByte* ptr, 
                                   UPInt size, 
                                   UPInt alignment,
                                   UPInt limit) const
{
    const UByte* aligned = getAlignedPtr(ptr, alignment);
    return aligned + size <= ptr + limit;
}

//------------------------------------------------------------------------
void* SysAllocMapper::allocMem(UPInt pos, UPInt size, UPInt alignment)
{
    UByte* ptr = 0;
    Segment* seg = &Segments[pos];
    UInt32*  bitSet = getBitSet(seg->Memory, seg->Size);
    UPInt pages = size >> PageShift;
    UPInt limit = getEndBit(seg->Size);

#ifdef SF_MEMORY_FORCE_SYSALLOC
    if (LastUsed == 0)
        LastUsed = seg->Memory;
    UPInt start = (LastUsed - seg->Memory) >> PageShift;
    if (start > limit)
        start = 0;
#else
    UPInt start = 0;
#endif

    UPInt rest  = ~UPInt(0);
    UPInt found = ~UPInt(0);
    while(start < limit)
    {
        if (HeapPT::BitSet1::GetValue(bitSet, start))
        {
            start += HeapPT::BitSet1::FindUsedSize(bitSet, start, limit);
        }
        else
        {
            UPInt freeSize = HeapPT::BitSet1::FindFreeSize(bitSet, start);
            if (alignmentIsOK(seg->Memory + start*PageSize, size, alignment, freeSize*PageSize) &&
                freeSize - pages < rest)
            {
                found = start;
                rest  = freeSize - pages;
                if (!BestFit) 
                {
                    break;
                }
            }
            start += freeSize;
        }
    }
    if (found != ~UInt32(0))
    {
        ptr = getAlignedPtr(seg->Memory + found*PageSize, alignment);
        UPInt head = (ptr - (seg->Memory + found*PageSize)) >> PageShift;
        ptr = (UByte*)pMapper->MapPages(seg->Memory + (found + head)*PageSize, size); 

        if (ptr)
        {
            HeapPT::BitSet1::SetUsed(bitSet, found + head, pages);
            seg->PageCount += pages;
            Footprint      += pages << PageShift;
        }
    }
    LastSegment = ptr ? pos : ~UPInt(0);
    LastUsed = (UByte*)ptr + size;
    return ptr;
}

//------------------------------------------------------------------------
void* SysAllocMapper::allocMem(UPInt size, UPInt alignment)
{
    void* ptr = 0;
    if (LastSegment != ~UPInt(0))
    {
        ptr = allocMem(LastSegment, size, alignment);
        if (ptr)
        {
            return ptr;
        }
        LastUsed = 0;
    }
    for (UPInt i = 0; i < NumSegments; ++i)
    {
        if (i != LastSegment)
        {
            ptr = allocMem(i, size, alignment);
            if (ptr)
            {
                return ptr;
            }
            LastUsed = 0;
        }
    }
    return 0;
}

//------------------------------------------------------------------------
UPInt SysAllocMapper::freeMem(void* ptr, UPInt size)
{
    UPInt pos = findSegment((UByte*)ptr);
    SF_HEAP_ASSERT(pos < NumSegments);

    Segment* seg = &Segments[pos];

    bool t = pMapper->UnmapPages(ptr, size);
    SF_HEAP_ASSERT(t); SF_UNUSED(t);

    UPInt   start  = ((UByte*)ptr - seg->Memory) >> PageShift;
    UInt32* bitSet = getBitSet(seg->Memory, seg->Size);
    UPInt   pages  = size >> PageShift;

    HeapPT::BitSet1::SetFree(bitSet, start, pages);

    seg->PageCount -= pages;
    Footprint      -= pages << PageShift;
    return pos;
}

//------------------------------------------------------------------------
void* SysAllocMapper::Alloc(UPInt size, UPInt alignment)
{
    if (alignment < PageSize)
        alignment = PageSize;

    size = (size + alignment-1) & ~(alignment-1);

    void* ptr = allocMem(size, alignment);
    if (ptr)
    {
        return ptr;
    }
    if (!reserveSegment(size))
    {
        return 0;
    }
    return allocMem(size, alignment);
}

//------------------------------------------------------------------------
bool SysAllocMapper::ReallocInPlace(void* oldPtr, UPInt oldSize, 
                                       UPInt newSize, UPInt alignment)
{
    if (alignment < PageSize)
        alignment = PageSize;

    oldSize = (oldSize + alignment-1) & ~(alignment-1);
    newSize = (newSize + alignment-1) & ~(alignment-1);

    if (newSize == oldSize)
    {
        return true;
    }

    if (newSize > oldSize)
    {
        UPInt pos = findSegment((UByte*)oldPtr);
        SF_HEAP_ASSERT(pos < NumSegments);
        Segment* seg      = &Segments[pos];
        UPInt    start    = ((UByte*)oldPtr + oldSize - seg->Memory) >> PageShift;
        UInt32*  bitSet   = getBitSet(seg->Memory, seg->Size);
        UPInt    addPages = (newSize - oldSize) >> PageShift;
        UPInt    freeSize = HeapPT::BitSet1::FindFreeSize(bitSet, start);
        if (freeSize >= addPages)
        {
            if (pMapper->MapPages(seg->Memory + start*PageSize, newSize - oldSize))
            {
                HeapPT::BitSet1::SetUsed(bitSet, start, addPages);
                seg->PageCount += addPages;
                Footprint      += addPages << PageShift;
                return true;
            }
        }
    }
    else
    {
        return Free((UByte*)oldPtr + newSize, oldSize - newSize, alignment);
    }
    return false;
}

//------------------------------------------------------------------------
bool SysAllocMapper::Free(void* ptr, UPInt size, UPInt alignment)
{
    if (alignment < PageSize)
        alignment = PageSize;

    size = (size + alignment-1) & ~(alignment-1);

    UPInt pos = freeMem(ptr, size);
    SF_HEAP_ASSERT(pos < NumSegments);
    if (Segments[pos].PageCount == 0)
    {
        releaseSegment(pos);
    }
    return true;
}

//------------------------------------------------------------------------
UPInt SysAllocMapper::GetBase() const
{
    return NumSegments ? UPInt(Segments[0].Memory) : 0;
}

} // Scaleform
