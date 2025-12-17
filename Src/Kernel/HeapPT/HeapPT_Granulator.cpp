/**************************************************************************

Filename    :   HeapPT_Granulator.cpp
Content     :   Allocation granulator
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

#include "HeapPT_Granulator.h"
#include "../SF_Debug.h"
#include "../SF_Alg.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
Granulator::Granulator(SysAllocPaged* sysAlloc, 
                       UPInt minSize,
                       UPInt granularity,
                       UPInt hdrPageSize) :
    pSysAlloc(sysAlloc),
    Granularity(granularity),
    HdrPageSize(hdrPageSize),
    HdrCapacity((hdrPageSize - sizeof(HdrPage)) / sizeof(Segment)),
    MinAlign(0),
    MaxAlign(0),
    HasRealloc(false),
    HdrList(),
    FreeSeg(),
    UsedSeg(),
    Footprint(0),
    Allocator(minSize)
{
    SysAllocPaged::Info i;
    memset(&i, 0, sizeof(SysAllocPaged::Info));
    pSysAlloc->GetInfo(&i);

    if (i.Granularity < Heap_PageSize)
        i.Granularity = Heap_PageSize;

    MinAlign = i.MinAlign;
    MaxAlign = i.MaxAlign;

    // Allow MinAlign and MaxAlign to have zero values, which means
    // "any alignment is supported".
    if (MinAlign == 0)
        MinAlign = 1;

    if (MaxAlign == 0)
        MaxAlign = UPInt(1) << (8*sizeof(UPInt)-1);


    // If the system allocator isn't realloc-in-place friendly the granulator
    // will not call SysAllocPaged::ReallocInPlace() in order to eliminate some
    // unnecessary operations in this->ReallocInPlace.
    //--------------------
    HasRealloc = i.HasRealloc;// && MaxAlign >= Allocator.GetMinSize();

    SF_HEAP_ASSERT(((MinAlign-1) & MinAlign) == 0);
    SF_HEAP_ASSERT(((MaxAlign-1) & MaxAlign) == 0);
    SF_HEAP_ASSERT(MinAlign <= MaxAlign);
    SF_HEAP_ASSERT((i.Granularity & Heap_PageMask) == 0);

    Granularity = (Granularity + i.Granularity-1) / i.Granularity * i.Granularity;
    SF_HEAP_ASSERT(Granularity >= 2*HdrPageSize);
}

//#include <stdio.h> // DBG
//------------------------------------------------------------------------
bool Granulator::allocSegment(UPInt size, UPInt alignSize)
{
    if (alignSize < MinAlign)
        alignSize = MinAlign;

    UPInt hdrPageSize = FreeSeg.IsEmpty() ? HdrPageSize : 0;
    UPInt segAlign    = alignSize;
    UPInt minSize     = Allocator.GetMinSize();

    if (segAlign < minSize)
        segAlign = minSize;

    if (segAlign > MaxAlign)
        segAlign = MaxAlign;

    // Calculation of the segment size is tricky.
    // It's required to ensure that the allocation will succeed with the 
    // worst possible conditions, even if the SysAllocPaged does not provide
    // any alignment. 
    //------------------------
    UPInt segSize;
    UPInt extraTail = (alignSize > MaxAlign) ? alignSize : 0;
    UPInt headAlign = (segAlign  < minSize ) ? minSize   : 0;

    if (extraTail <= headAlign)
        extraTail = 0;

    segSize = (size + hdrPageSize + extraTail + alignSize-1) & ~(alignSize-1);
    segSize = (segSize + Granularity-1) / Granularity * Granularity + headAlign;
    segSize = (segSize + minSize-1) & ~(minSize-1);

    UByte* newSpacePtr = (UByte*)pSysAlloc->Alloc(segSize, segAlign);

    SF_DEBUG_ERROR((UPInt(newSpacePtr) & (segAlign-1)) != 0, 
        "SysAllocPaged implementation has returned improperly aligned address.");

    SF_HEAP_ASSERT((UPInt(newSpacePtr) & (segAlign-1)) == 0);

    if (newSpacePtr)
    {
        UByte* newSpaceEnd = newSpacePtr + segSize;
        UByte* alignedPtr = (UByte*)((UPInt(newSpacePtr) + minSize-1) & ~(minSize-1));
        UByte* alignedEnd = (UByte*)((UPInt(newSpaceEnd)            ) & ~(minSize-1));

        SF_HEAP_ASSERT(
            newSpaceEnd == alignedEnd ||
            UPInt(newSpaceEnd-alignedEnd) == minSize - UPInt(alignedPtr-newSpacePtr));

        Segment* seg;

        if (hdrPageSize)
        {
            memset(alignedPtr, 0, hdrPageSize);
            HdrPage* hdr = (HdrPage*)alignedPtr;
            hdr->UseCount = 0;
            for(UPInt i = 0; i < HdrCapacity; ++i)
            {
                seg = (Segment*)(alignedPtr + sizeof(HdrPage) + i * sizeof(Segment));
                seg->Headers = hdr;
                FreeSeg.PushBack(seg);
            }
            HdrList.PushBack(hdr);
        }

        seg = FreeSeg.GetFirst();
        SegListType::Remove(seg);

        ++seg->Headers->UseCount;

        seg->Buffer     = alignedPtr + hdrPageSize;
        seg->Size       = alignedEnd - seg->Buffer;
        seg->UseCount   = 0;
        seg->AlignShift = Alg::UpperBit(segAlign);
        seg->HeadBytes  = UPIntHalf(alignedPtr - newSpacePtr);

        UsedSeg.Insert(seg);
        Allocator.InitSegment(seg);

        //if (DualTNode::GetAlignedPtr(seg->Buffer, alignSize-1) +  // DBG
        //   ((size + alignSize-1) & ~(alignSize-1)) > alignedEnd)
        //{
        //    printf("*");
        //}

        SF_HEAP_ASSERT(hdrPageSize || 
                       DualTNode::GetAlignedPtr(seg->Buffer, alignSize-1) + 
                       ((size + alignSize-1) & ~(alignSize-1)) <= alignedEnd);

        Footprint += segSize;
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
bool Granulator::freeSegment(Segment* seg)
{
    SF_HEAP_ASSERT(seg->UseCount == 0);

    HdrPage* hdrPage = seg->Headers;

    // Theoretically, if SysAllocPaged::Free() has failed it's nice to restore 
    // the previous state. But it cannot be guaranteed that pSysAlloc->Free() 
    // keeps memory on fail. So, if a failure occurs function freeSegment() 
    // behaves as if SysAllocPaged freed memory anyway. If SysAllocPaged::Free() can 
    // really fail it means that there something really bad has happened to 
    // the entire system. So, correct restoring of the previous state on 
    // failure involves too much hassle for too miserable result. 
    //-------------------
    bool ret = true;
    UPInt segSize;
    if (!hasHeaders(seg))
    {
        // This is not a HeaderPage; can be released.
        //-------------------
        UsedSeg.Remove(seg);
        FreeSeg.PushFront(seg);
        --hdrPage->UseCount;

        segSize = getSegSize(seg);
        Allocator.ReleaseSegment(seg);
        Footprint -= segSize;
        ret = pSysAlloc->Free(getSegBase(seg), segSize, UPInt(1) << UPInt(seg->AlignShift));
    }

    if (hdrPage->UseCount == 1)
    {
        // Release segment with the header page.
        //-------------------
        Segment* hdrSeg = (Segment*)UsedSeg.FindEqual(UPInt(hdrPage) + HdrPageSize);
        SF_HEAP_ASSERT(hdrSeg);

        if (hdrSeg->UseCount == 0)
        {
            UsedSeg.Remove(hdrSeg);
            FreeSeg.PushFront(hdrSeg);

            UByte* hptr = (UByte*)hdrPage;
            for(UPInt i = 0; i < HdrCapacity; ++i)
            {
                seg = (Segment*)(hptr + sizeof(HdrPage) + i * sizeof(Segment));
                FreeSeg.Remove(seg);
            }

            HdrList.Remove(hdrPage);
            segSize = getSegSize(hdrSeg);
            Allocator.ReleaseSegment(hdrSeg);
            Footprint -= segSize;
            ret = pSysAlloc->Free(getSegBase(hdrSeg), segSize, UPInt(1) << UPInt(hdrSeg->AlignShift));
        }
    }
    return ret;
}


//------------------------------------------------------------------------
void* Granulator::Alloc(UPInt size, UPInt alignSize)
{
    Segment* seg;
    void* ptr = Allocator.Alloc(size, alignSize, &seg);
    if (ptr)
    {
        ++seg->UseCount;
        return ptr;
    }

    // The attempt to allocate the segment is performed twice. 
    // It's necessary because of different logic for the segments with 
    // headers. These segments request the system for exactly Granularity 
    // size, which can be not enough for the actual allocation (in case)
    // the allocation size is bigger than Granularity. In this case
    // allocSegment() may be called second time; the second call guarantees 
    // that the requested size will be provided.
    //-------------------------
    if (!allocSegment(size, alignSize))
    {
        return 0;
    }
    ptr = Allocator.Alloc(size, alignSize, &seg);
    if (ptr)
    {
        ++seg->UseCount;
        return ptr;
    }
    if (!allocSegment(size, alignSize))
    {
        return 0;
    }
    ptr = Allocator.Alloc(size, alignSize, &seg);
    if (ptr)
    {
        ++seg->UseCount;
    }
    return ptr;
}


//------------------------------------------------------------------------
bool Granulator::Free(void* ptr, UPInt size, UPInt alignSize)
{
    Segment* seg = (Segment*)UsedSeg.FindLeEq(UPInt(ptr));
    SF_HEAP_ASSERT(seg && 
                  (UByte*)ptr >= seg->Buffer && 
                  (UByte*)ptr <  seg->Buffer + seg->Size);
    Allocator.Free(seg, ptr, size, alignSize);
    if (--seg->UseCount == 0)
    {
        return freeSegment(seg);
    }

    return true;
}


//------------------------------------------------------------------------
bool Granulator::ReallocInPlace(void* oldPtr, UPInt oldSize, 
                                UPInt newSize, UPInt alignSize)
{
    SF_HEAP_ASSERT(oldPtr && oldSize && newSize);

    if (alignSize < Allocator.GetMinSize())
        alignSize = Allocator.GetMinSize();

    SF_HEAP_ASSERT(((alignSize-1) & alignSize) == 0);

    bool ret = false;
    Segment* seg = (Segment*)UsedSeg.FindLeEq(UPInt(oldPtr));
    SF_HEAP_ASSERT(seg && 
                  (UByte*)oldPtr >= seg->Buffer && 
                  (UByte*)oldPtr <  seg->Buffer + seg->Size);

    AllocLite::ReallocResult res = 
        Allocator.ReallocInPlace(seg, (UByte*)oldPtr, oldSize, newSize, alignSize);

    if (!HasRealloc)
    {
        return res < AllocLite::ReallocFailed;
    }

    if (res == AllocLite::ReallocSucceeded)
    {
        return true;
    }

    UByte* segBase    = getSegBase(seg);
    UPInt  oldSegSize = getSegSize(seg);
    UPInt  newSegSize;

    UPInt buffer = UPInt(segBase);
    UPInt oldEnd = UPInt(oldPtr) - buffer + oldSize;
    UPInt newEnd = UPInt(oldPtr) - buffer + newSize;
    oldEnd = (oldEnd + alignSize-1) & ~(alignSize-1);
    newEnd = (newEnd + alignSize-1) & ~(alignSize-1);
    oldEnd = (oldEnd + Granularity-1) / Granularity * Granularity;
    newEnd = (newEnd + Granularity-1) / Granularity * Granularity;

    // Try to shrink the allocation in the system.
    // Segments with a header page are excluded from the process.
    //------------------------
    if (res == AllocLite::ReallocShrinkedAtTail)
    {
        if (newEnd < oldEnd)
        {
            // Trimming at the tail is tricky. If the system ReallocInPlace()
            // succeeded we can't do anything with this memory. So, we have 
            // to trim the allocation space in advance and then, if the system 
            // ReallocInPlace() fails extend it back to the previous state.
            //---------------------------
            newSegSize = buffer + newEnd - UPInt(segBase);
            Allocator.TrimAt(seg, (UByte*)(buffer + newEnd));
            if (pSysAlloc->ReallocInPlace(segBase, oldSegSize, newSegSize, 
                                          UPInt(1) << seg->AlignShift))
            {
                Footprint -= oldSegSize - newSegSize;
            }
            else
            {
                Allocator.Extend(seg, oldSegSize - newSegSize);
            }
        }
        return true;
    }

    // Try to extend the allocation in the system.
    // Segments with a header page are excluded from the process.
    //-----------------------
    if (res == AllocLite::ReallocFailedAtTail)
    {
        while (newEnd <= oldEnd)
        {
            newEnd += Granularity;
        }
        newSegSize = buffer + newEnd - UPInt(segBase);
        if (pSysAlloc->ReallocInPlace(segBase, oldSegSize, newSegSize,
                                      UPInt(1) << seg->AlignShift))
        {
            Footprint += newSegSize - oldSegSize;
            Allocator.Extend(seg, newSegSize - oldSegSize);
            res = Allocator.ReallocInPlace(seg, (UByte*)oldPtr, oldSize, newSize, alignSize);
            SF_HEAP_ASSERT(res < AllocLite::ReallocFailed); // Must succeed!
            return true;
        }
    }

    return ret;
}


//------------------------------------------------------------------------
const TreeSeg* Granulator::GetAllocSegment(const void* ptr) const
{
    const TreeSeg* seg = UsedSeg.FindLeEq(UPInt(ptr));
    SF_HEAP_ASSERT(seg && 
                   (UByte*)ptr >= seg->Buffer && 
                   (UByte*)ptr <  seg->Buffer + seg->Size);
    return seg;
}

//------------------------------------------------------------------------
void Granulator::VisitMem(MemVisitor* visitor,
                          MemVisitor::Category catSegm,
                          MemVisitor::Category catFree) const
{
    SF_UNUSED(catSegm);
    Allocator.VisitMem(visitor, catFree);
}


//------------------------------------------------------------------------
void Granulator::visitSegments(const Segment* node, 
                               SegVisitor* visitor, 
                               unsigned cat) const
{
    if (node)
    {
        visitSegments(node->AddrChild[0], visitor, cat);
        UPInt hdrPage = hasHeaders(node) ? HdrPageSize : 0;
        UPInt buffer = (UPInt(node->Buffer) - hdrPage + Heap_PageMask) & ~Heap_PageMask;
        UPInt size   = (node->Size + hdrPage) & ~Heap_PageMask;
        visitor->Visit(cat, 0, buffer, size);
        visitSegments(node->AddrChild[1], visitor, cat);
    }
}

//------------------------------------------------------------------------
void Granulator::VisitSegments(SegVisitor* visitor, unsigned catSeg, unsigned catUnused) const
{
    visitSegments(UsedSeg.Root, visitor, catSeg);
    Allocator.VisitUnused(visitor, catUnused);
}

}} // Scaleform::Heap
