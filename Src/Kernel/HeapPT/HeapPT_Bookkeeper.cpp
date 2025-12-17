/**************************************************************************

Filename    :   HeapPT_Bookkeeper.cpp
Content     :   Bookkeeping allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Internal allocator used to store bookkeeping information.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_Bookkeeper.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
Bookkeeper::Bookkeeper(SysAllocPaged* sysAlloc, UPInt granularity) :
    pSysAlloc(sysAlloc), 
    Granularity(granularity),
    Allocator(Heap_MinShift),
    Footprint(0)
{
    SysAllocPaged::Info i;
    memset(&i, 0, sizeof(SysAllocPaged::Info));
    pSysAlloc->GetInfo(&i);
    if (i.Granularity < Heap_PageSize)
        i.Granularity = Heap_PageSize;
    SF_HEAP_ASSERT((i.Granularity & Heap_PageMask) == 0);
    Granularity = (Granularity + i.Granularity-1) / i.Granularity * i.Granularity;
}


//------------------------------------------------------------------------
UPInt Bookkeeper::getHeaderSize(UPInt dataSize) const
{
    UPInt headerSize;
    headerSize = sizeof(HeapSegment) + Allocator.GetBitSetBytes(dataSize);
    headerSize = (headerSize + Heap_MinMask) & ~UPInt(Heap_MinMask);
    return headerSize;
}

//------------------------------------------------------------------------
HeapSegment* Bookkeeper::allocSegment(UPInt dataSize)
{
    UByte* segMem = (UByte*)pSysAlloc->Alloc(dataSize, Heap_PageSize);
    HeapSegment* seg = (HeapSegment*)segMem;
    UPInt headerSize;
    if (seg)
    {
        seg->SelfSize   = dataSize;
        seg->SegType    = Heap_SegmentBookkeeper;
        seg->Alignment  = Heap_PageShift;
        seg->UseCount   = 0;
        seg->pHeap      = 0;
        seg->DataSize   = 0;
        seg->pData      = 0;
        if (!GlobalPageTable->MapRange(segMem, dataSize))
        {
            pSysAlloc->Free(segMem, dataSize, Heap_PageSize);
            return 0;
        }
        GlobalPageTable->SetSegmentInRange(UPInt(segMem), dataSize, seg);
        headerSize    = getHeaderSize(dataSize);
        seg->DataSize = dataSize - headerSize;
        seg->pData    = segMem + headerSize;
        SegmentList.PushFront(seg);
        Allocator.InitSegment(seg);
        Footprint += seg->SelfSize;
        return seg;
    }
    return 0;
}


//------------------------------------------------------------------------
void Bookkeeper::freeSegment(HeapSegment* seg)
{
    Allocator.ReleaseSegment(seg);
    SegmentListType::Remove(seg);
    GlobalPageTable->UnmapRange(seg, seg->SelfSize);
    Footprint -= seg->SelfSize;
    pSysAlloc->Free(seg, seg->SelfSize, Heap_PageSize);
}


//------------------------------------------------------------------------
void* Bookkeeper::Alloc(UPInt size)
{
    size = Allocator.AlignSize((size < Heap_MinSize) ? Heap_MinSize : size);

    HeapSegment* seg;
    void* ptr = Allocator.Alloc(size, &seg);
    if (ptr)
    {
        seg->UseCount++;
        return ptr;
    }

    UPInt segSize = (size + Granularity - 1) / Granularity * Granularity;
    while (segSize < size + getHeaderSize(segSize))
    {
        // This is an extreme condition, but still, may occur. 
        // Since the the bit-set takes some space in the beginning,
        // the rest of the payload may be not enough for the allocation, 
        // while the allocation must be guaranteed if allocSegment()
        // succeeded.
        //-----------------------
        segSize += Granularity;
    }

    seg = allocSegment(segSize);
    if (seg == 0)
    {
        return 0;
    }

    HeapSegment* t;
    ptr = Allocator.Alloc(size, &t);
    SF_HEAP_ASSERT(ptr && t == seg); // Must succeed and allocate in the new segment!
    seg->UseCount++;
    return ptr;
}


//------------------------------------------------------------------------
void Bookkeeper::Free(void* ptr, UPInt size)
{
    HeapSegment* seg = GlobalPageTable->GetSegmentSafe(UPInt(ptr));
    SF_HEAP_ASSERT(seg != 0 && seg->SegType == Heap_SegmentBookkeeper);

    size = Allocator.AlignSize((size < Heap_MinSize) ? Heap_MinSize : size);

    Allocator.Free(seg, ptr, size);
    if (--seg->UseCount == 0)
    {
        freeSegment(seg);
    }
}


//------------------------------------------------------------------------
void Bookkeeper::VisitMem(MemVisitor* visitor, unsigned flags) const
{
    if (flags & MemVisitor::VF_Bookkeeper)
    {
        const HeapSegment* seg = SegmentList.GetFirst();
        while(!SegmentList.IsNull(seg))
        {
            visitor->Visit(seg, UPInt(seg->pData), 
                           seg->DataSize, MemVisitor::Cat_Bookkeeper);
            seg = seg->pNext;
        }
        if (flags & MemVisitor::VF_BookkeeperFree)
        {
            Allocator.VisitMem(visitor, MemVisitor::Cat_BookkeeperFree);
        }
    }
}

//------------------------------------------------------------------------
void Bookkeeper::VisitSegments(SegVisitor* visitor) const
{
    const HeapSegment* seg = SegmentList.GetFirst();
    while(!SegmentList.IsNull(seg))
    {
        visitor->Visit(SegVisitor::Seg_Bookkeeper, 0, 
                       UPInt(seg), seg->SelfSize);
        seg = seg->pNext;
    }
    Allocator.VisitUnused(visitor, SegVisitor::Seg_BookkeeperUnused);
}

}} // Scaleform::Heap
