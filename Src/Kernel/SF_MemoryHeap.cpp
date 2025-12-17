/**************************************************************************

Filename    :   SF_MemoryHeap.cpp
Content     :   The main allocator interface. Receive data from heap 
                management system and allocate/free memory from the
                current heap. Manipulates with heaps - creation of
                heap chunks, deletes heap chunks, if free.
Created     :   July 14, 2008
Authors     :   Michael Antonov, Boris Rayskiy, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Debug.h"
#include "SF_MemoryHeap.h"
#include "SF_Memory.h" 

#include <string.h>

#undef new

#ifdef SN_TARGET_PS3
#include <stdlib.h>	// for reallocalign()
#endif

namespace Scaleform {

// *** Heap-Specific Statistics identifiers
SF_DECLARE_STAT_GROUP(StatHeap_Summary,              "Heap Summary",     StatGroup_Default)
SF_DECLARE_COUNTER_STAT(StatHeap_TotalFootprint,     "Total Footprint",  StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_LocalFootprint,     "Local Footprint",  StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_ChildFootprint,     "Child Footprint",  StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_ChildHeaps,         "Child Heaps",      StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_LocalUsedSpace,     "Local Used Space", StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_SysDirectSpace,     "Sys Direct Space", StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_Bookkeeping,        "Bookkeeping",      StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_DebugInfo,          "DebugInfo",        StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_Segments,           "Segments",         StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_Granularity,        "Granularity",      StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_DynamicGranularity, "Dynamic Granularity",StatHeap_Summary)
SF_DECLARE_COUNTER_STAT(StatHeap_Reserve,            "Reserve",          StatHeap_Summary)


#if defined(SF_OS_WIN32) && defined(SF_BUILD_DEBUG)
#define SF_MEMORYHEAP_CHECK_CURRENT_THREAD(h) SF_ASSERT((h->OwnerThreadId == 0) || (h->OwnerThreadId == ::GetCurrentThreadId()))
#else
#define SF_MEMORYHEAP_CHECK_CURRENT_THREAD(h)
#endif

MemoryHeap* Memory::pGlobalHeap = 0;

//------------------------------------------------------------------------
MemoryHeap::MemoryHeap() :
    SelfSize(0),
    RefCount(1),
    OwnerThreadId(0),
    pAutoRelease(0),
    //Info(),
    ChildHeaps(),
    HeapLock(),
    UseLocks(true),
    TrackDebugInfo(true)
{
    memset(&Info, 0, sizeof(Info));
}

//------------------------------------------------------------------------
void MemoryHeap::GetHeapInfo(HeapInfo* infoPtr) const
{
    memcpy(infoPtr, &Info, sizeof(HeapInfo));
}

//------------------------------------------------------------------------
void MemoryHeap::ReleaseOnFree(void *ptr)
{
    pAutoRelease = ptr;
}

//------------------------------------------------------------------------
void MemoryHeap::AssignToCurrentThread()
{
#ifdef SF_OS_WIN32
    OwnerThreadId = (UPInt) ::GetCurrentThreadId();
#endif
}

//------------------------------------------------------------------------
void MemoryHeap::VisitChildHeaps(HeapVisitor* visitor)
{
    Lock::Locker locker(&HeapLock);

    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        // AddRef() and Release() are locked with a Global Lock here,
        // to avoid the possibility of heap being released during this iteration.
        // If necessary, this could re rewritten with AddRef_NotZero for efficiency.
        visitor->Visit(this, heap);
        heap = heap->pNext;
    }
}

//------------------------------------------------------------------------
void MemoryHeap::LockAndVisit(HeapVisitor* visitor)
{
    Lock::Locker locker(&HeapLock);
    visitor->Visit(NULL, this);
}


//------------------------------------------------------------------------
bool MemoryHeap::DumpMemoryLeaks()
{
    // This lock is safe because at this point there should be no more
    // external references to this heap.
    Lock::Locker locker(&HeapLock);
    return dumpMemoryLeaks();
}

//------------------------------------------------------------------------
void MemoryHeap::UltimateCheck()
{
    Memory::pGlobalHeap->ultimateCheck();
}

//------------------------------------------------------------------------
void MemoryHeap::CheckIntegrity()
{
    Memory::pGlobalHeap->checkIntegrity();
}

//------------------------------------------------------------------------
void MemoryHeap::ReleaseCachedMem()
{
    Memory::pGlobalHeap->releaseCachedMem();
}

int MemoryHeap::CheckRead(int v) const
{
    return v+1;
}


} // Scaleform


