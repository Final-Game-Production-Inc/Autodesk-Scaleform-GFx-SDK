/**************************************************************************

Filename    :   HeapPT_Bookkeeper.h
Content     :   Bookkeeping allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Internal allocator used to store bookkeeping information.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_Bookkeeper_H
#define INC_SF_Kernel_HeapPT_Bookkeeper_H

#include "../SF_List.h"
#include "../SF_SysAlloc.h"
#include "HeapPT_PageTable.h"
#include "HeapPT_AllocBitSet1.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

// ***** Bookkeeper
//
//  Internal allocator used to store bookkeeping information.
//------------------------------------------------------------------------
class Bookkeeper
{
public:
    typedef List<HeapSegment> SegmentListType;

    Bookkeeper(SysAllocPaged* sysAlloc, UPInt granularity = Heap_PageSize);

    SysAllocPaged* GetSysAlloc() { return pSysAlloc; }

    void*  Alloc(UPInt size);
    void   Free(void* ptr, UPInt size);

    void   VisitMem(MemVisitor* visitor, unsigned flags) const;
    void   VisitSegments(SegVisitor* visitor) const;

    UPInt  GetFootprint() const { return Footprint; }
    UPInt  GetUsedSpace() const { return Footprint - Allocator.GetTotalFreeSpace(); }

private:
    UPInt           getHeaderSize(UPInt dataSize) const;
    HeapSegment*    allocSegment(UPInt size);
    void            freeSegment(HeapSegment* seg);

    SysAllocPaged*      pSysAlloc;
    UPInt               Granularity;
    SegmentListType     SegmentList;
    AllocBitSet1        Allocator;
    UPInt               Footprint;
};

}} //Scaleform::Heap

#endif
