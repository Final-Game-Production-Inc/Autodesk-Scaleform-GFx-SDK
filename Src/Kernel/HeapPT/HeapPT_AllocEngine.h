/**************************************************************************

Filename    :   HeapPT_AllocEngine.h
Content     :   The main allocation engine
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_AllocEngine_H
#define INC_SF_Kernel_HeapPT_AllocEngine_H

#include "../SF_Atomic.h"
#include "HeapPT_PageTable.h"
#include "HeapPT_Bookkeeper.h"
#include "HeapPT_AllocBitSet2.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
struct HeapOtherStats
{
    UPInt Segments;
    UPInt Bookkeeping;
    UPInt DynamicGranularity;
    UPInt SysDirectSpace;
};

//------------------------------------------------------------------------
class AllocEngine
{
public:
    //-------------------------------------------------------------------
    struct TinyBlock : ListNode<TinyBlock>
    {
        HeapSegment*   pSegment;
    };

private:
    typedef List<HeapSegment> SegmentListType;
    typedef List<TinyBlock>   TinyListType;

public:
    //--------------------------------------------------------------------
    AllocEngine(SysAllocPaged* sysAlloc,
                MemoryHeapPT*  heap,
                unsigned       allocFlags, 
                UPInt          minAlignSize=16,
                UPInt          granularity=8*1024, 
                UPInt          reserve=8*1024,
                UPInt          internalThreshold=128*1024,
                UPInt          limit=0);

    ~AllocEngine();

    bool IsValid() const { return Valid; }

    UPInt SetLimit(UPInt lim);
    void  SetLimitHandler(void* handler) { pLimHandler = handler; }

    // Calling FreeAll() releases all memory the heap holds, 
    // but the heap remains functional.
    void FreeAll();

    // External functions
    //--------------------------------------------------------------------
    void*   Alloc(UPInt size);
    void*   Alloc(UPInt size, UPInt alignSize);
    void*   Realloc(HeapSegment* seg, void* oldPtr, UPInt newSize);
    void    Free(HeapSegment* seg, void* ptr);

    void*   Realloc(void* oldPtr, UPInt newSize);
    void    Free(void* ptr);

    UPInt   GetUsableSize(const void* ptr);
    UPInt   GetUsableSize(HeapSegment* seg, const void* ptr);

    UPInt   GetFootprint() const;
    UPInt   GetUsedSpace() const;

    void    ReleaseCachedMem();

    // See MemVisitor::VisitingFlags for "flags" argument.
    void    VisitMem(MemVisitor* visitor, 
                     unsigned flags = MemVisitor::VF_Heap) const;

    void    VisitSegments(SegVisitor* visitor) const;

    void    GetHeapOtherStats(HeapOtherStats* otherStats) const;

    void    CheckIntegrity() const
    {
        Allocator.CheckIntegrity();
    }

private:
    //--------------------------------------------------------------------
    static void     compilerAsserts(); // A fake function used for SF_COMPILER_ASSERTS only

    HeapSegment*    allocSegment(unsigned  segType, UPInt size, UPInt sysAlignment, 
                                 UPInt bookkeepingSize, bool* limHandlerOK);

    HeapSegment*    allocSegmentNoGranulator(UPInt dataSize, UPInt alignment, bool* limHandlerOK);

    HeapSegment*    allocSegmentLocked(unsigned  segType, UPInt size, UPInt sysAlignment, 
                                       UPInt bookkeepingSize, bool* limHandlerOK);

    void            freeSegment(HeapSegment* seg);
    void            freeSegmentLocked(HeapSegment* seg);

    UPInt           calcDynaSize() const;
    HeapSegment*    allocSegmentBitSet(UPInt size, UPInt alignSize, 
                                       UPInt granularity, 
                                       bool* limHandlerOK);

    HeapSegment*    allocSegmentBitSet(UPInt size, UPInt alignSize, bool* limHandlerOK);

    void            freeSegmentBitSet(HeapSegment* seg);

    void*           allocSysDirect(UPInt dataSize, UPInt alignSize);
    void*           reallocSysDirect(HeapSegment* seg, void* oldPtr, UPInt newSize);

    void*           reallocGeneral(HeapSegment* seg, void* oldPtr, 
                                   UPInt oldSize, UPInt newSize, 
                                   UPInt alignShift);

    void*           allocBitSet(UPInt size);
    void*           allocBitSet(UPInt size, UPInt alignSize);

    void*           allocSegmentTiny(unsigned idx);
    void            releaseSegmentTiny(HeapSegment* seg);
    void            freeSegmentTiny(HeapSegment* seg);
    void*           allocTiny(unsigned sizeIdx);
    void            freeTiny(HeapSegment* seg, TinyBlock* ptr);

private:
    //--------------------------------------------------------------------
    MemoryHeapPT*       pHeap;
    SysAllocPaged*      pSysAlloc;
    Bookkeeper*         pBookkeeper;
    UPInt               MinAlignShift;
    UPInt               MinAlignMask;
    AllocBitSet2        Allocator;
    SegmentListType     SegmentList;
    TinyListType        TinyBlocks[Heap_TinyBinSize];
    bool                AllowTinyBlocks;
    bool                AllowDynaSize;
    bool                Valid;
    bool                HasRealloc;
    UPInt               SysGranularity;
    UPInt               Granularity;
    UPInt               Reserve;
    UPInt               Threshold;
    UPInt               SysDirectThreshold;
    UPInt               Footprint;
    UPInt               TinyFreeSpace;
    UPInt               SysDirectSpace;
    HeapSegment*        pCachedBSeg; // Cached bit-set segment
    HeapSegment*        pCachedTSeg; // Cached tiny blocks segment
    UPInt               Limit;
    void*               pLimHandler;
};

}} // Scaleform::Heap

#endif

