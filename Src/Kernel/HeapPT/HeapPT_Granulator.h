/**************************************************************************

Filename    :   HeapPT_Granulator.h
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

#ifndef INC_SF_Kernel_HeapPT_Granulator_H
#define INC_SF_Kernel_HeapPT_Granulator_H

#include "../SF_SysAlloc.h"
#include "HeapPT_AllocLite.h"

namespace Scaleform { namespace HeapPT {

// ***** Granulator
//
// The granulator requests the system allocator for large memory blocks 
// and granulates them as necessary, providing smaller granularity to 
// the consumer.
//
// Internally the granulator allocates a page of "headerPageSize" for 
// the headers, and pages of "granularity" size (or bigger) for the payload. 
// There can be many header pages linked in a circular list. When allocating
// it sometimes (very rarely) has to iterate through all header pages
// and all headers. Most of the time requests will be served successfully
// by using the last successful page and header.
//
// When freeing the granulator has to find the page that corresponds to 
// the pointer. The granulator uses a binary search within every header
// page and iterates through all header pages in the linked list.
// 
// Don't worry if it sounds scary in terms of the performance. 
// The request are rare, and besides, there's no large variety of 
// sizes.
//
// The only restriction is: HeaderPageSize and Granularity cannot be
// less than Heap_PageSize.
//------------------------------------------------------------------------


//------------------------------------------------------------------------
struct HdrPage : ListNode<HdrPage>
{
    UPInt   UseCount;
    UPInt   Filler[5]; // Just for better alignment to sizeof(TreeSeg)
//  TreeSeg Segments[PageCapacity];
};


//------------------------------------------------------------------------
class Granulator
{
    typedef TreeSeg Segment;

public:
    //--------------------------------------------------------------------
    struct SegListAccessor
    {
        static void  SetPrev(Segment* self, Segment* what) { self->AddrChild[0] = what; }
        static void  SetNext(Segment* self, Segment* what) { self->AddrChild[1] = what; }
        static const Segment* GetPrev(const Segment* self) { return self->AddrChild[0]; }
        static const Segment* GetNext(const Segment* self) { return self->AddrChild[1]; }
        static       Segment* GetPrev(Segment* self)       { return self->AddrChild[0]; }
        static       Segment* GetNext(Segment* self)       { return self->AddrChild[1]; }
    };

    //--------------------------------------------------------------------
    struct SegTreeAccessor
    {
        static       UPInt     GetKey     (const Segment* n)          { return  UPInt(n->Buffer); }
        static       Segment*  GetChild   (      Segment* n, UPInt i) { return  n->AddrChild[i];  }
        static const Segment*  GetChild   (const Segment* n, UPInt i) { return  n->AddrChild[i];  }
        static       Segment** GetChildPtr(      Segment* n, UPInt i) { return &n->AddrChild[i];  }

        static       Segment*  GetParent  (      Segment* n)          { return n->AddrParent; }
        static const Segment*  GetParent  (const Segment* n)          { return n->AddrParent; }

        static void SetParent (Segment* n, Segment* p)                { n->AddrParent   = p; }
        static void SetChild  (Segment* n, UPInt i, Segment* c)       { n->AddrChild[i] = c; }
        static void ClearLinks(Segment* n) { n->AddrParent = n->AddrChild[0] = n->AddrChild[1] = 0; }
    };


    //--------------------------------------------------------------------
    typedef List<HdrPage>                           HdrListType;
    typedef List2<Segment, SegListAccessor>         SegListType;
    typedef RadixTree<Segment, SegTreeAccessor>     SegTreeType;


public:
    // Public interface functions
    //--------------------------------------------------------------------
    Granulator(SysAllocPaged* sysAlloc, 
               UPInt minSize     = 256,
               UPInt granularity = 64*1024,
               UPInt hdrPageSize = Heap_PageSize);

    SysAllocPaged* GetSysAlloc() { return pSysAlloc;   }

    UPInt   GetGranularity() { return Granularity; }

    void*   Alloc(UPInt size, UPInt alignSize);
    bool    ReallocInPlace(void* oldPtr, UPInt oldSize, 
                           UPInt newSize, UPInt alignSize);

    bool    Free(void* ptr, UPInt size, UPInt alignSize); // Size is required!

    UPInt   GetFootprint() const { return Footprint; }
    UPInt   GetUsedSpace() const { return Footprint - Allocator.GetFreeBytes(); }

    UPInt   GetBase() const { return pSysAlloc->GetBase(); } // DBG
    UPInt   GetSize() const { return pSysAlloc->GetSize(); } // DBG

    const TreeSeg* GetAllocSegment(const void* ptr) const;

    void    VisitMem(MemVisitor* visitor, 
                     MemVisitor::Category catSegm,
                     MemVisitor::Category catFree) const;

    void    VisitSegments(SegVisitor* visitor, unsigned catSeg, unsigned catUnused) const;

    UPInt   GetMinBase() const; // DBG

private:
    //--------------------------------------------------------------------
    bool    allocSegment(UPInt size, UPInt alignSize);
    bool    freeSegment(Segment* seg);
    void*   alloc(UPInt size, UPInt alignSize);
    bool    hasHeaders(const Segment* seg) const
    { 
        return (UByte*)seg->Headers + HdrPageSize == seg->Buffer; 
    }
    UByte*  getSegBase(Segment* seg)
    {
        UPInt  hdrSize = hasHeaders(seg) ? HdrPageSize : 0;
        return seg->Buffer - hdrSize - seg->HeadBytes;
    }
    UPInt   getSegSize(const Segment* seg) const
    {
        UPInt  hdrSize = hasHeaders(seg) ? HdrPageSize : 0;
        UPInt  tailBytes = seg->HeadBytes ? Allocator.GetMinSize() - seg->HeadBytes : 0;
        return seg->Size + hdrSize + seg->HeadBytes + tailBytes;
    }
    void visitSegments(const Segment* node, SegVisitor* visitor, unsigned cat) const;

    //--------------------------------------------------------------------
    SysAllocPaged*      pSysAlloc;
    UPInt               Granularity;
    UPInt               HdrPageSize;
    UPInt               HdrCapacity;
    UPInt               MinAlign;
    UPInt               MaxAlign;
    bool                HasRealloc;
    HdrListType         HdrList;
    SegListType         FreeSeg;
    SegTreeType         UsedSeg;
    UPInt               Footprint;
    AllocLite           Allocator;
};

}} // Scaleform::Heap

#endif
