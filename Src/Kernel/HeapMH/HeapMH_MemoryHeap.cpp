/**************************************************************************

Filename    :   HeapMH_MemoryHeap.cpp
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

#include "HeapMH_Root.h"
#include "HeapMH_AllocEngine.h"
#include "HeapMH_DebugInfo.h"
#include "HeapMH_MemoryHeap.h"
#include "../SF_Debug.h"
#include "../SF_Memory.h"

#undef new

#ifdef SN_TARGET_PS3
#include <stdlib.h>	// for reallocalign()
#endif

#if defined(SF_OS_WIN32) && defined(SF_BUILD_DEBUG)
#define SF_MEMORY_CHECK_CURRENT_THREAD(h) SF_ASSERT((h->OwnerThreadId == 0) || (h->OwnerThreadId == ::GetCurrentThreadId()))
#else
#define SF_MEMORY_CHECK_CURRENT_THREAD(h)
#endif

namespace Scaleform {

namespace HeapMH 
{
    static UPInt HeapRootMemMH[(sizeof(RootMH) + sizeof(UPInt) - 1) / sizeof(UPInt)];
}

//------------------------------------------------------------------------
void MemoryHeap::InitMH(SysAlloc* sysAlloc) // static
{
    if (HeapMH::GlobalRootMH)
    {
        SF_DEBUG_MESSAGE(1, "MemoryHeapMH::Init: Memory system has been already initialized");
        SF_ASSERT(0); return;
    }

    // Function HeapPageTable::Init() does not allocate any memory;
    // it just initializes the root page table with zeros. So, the 
    // sequence of calls must remain as below: First init the page table,
    // then create the root.
    ::new(HeapMH::HeapRootMemMH) HeapMH::RootMH(sysAlloc);
}


//------------------------------------------------------------------------
void MemoryHeap::CleanUpMH() // static
{
    if (HeapMH::GlobalRootMH == 0)
    {
        SF_DEBUG_MESSAGE(1, "MemoryHeapMH::Finalize: Memory system was not initialized");
        SF_ASSERT(0); return;
    }
    HeapMH::GlobalRootMH->~RootMH();
    HeapMH::GlobalRootMH = 0;
}


//------------------------------------------------------------------------
MemoryHeap* MemoryHeap::CreateRootHeapMH(const HeapDesc& desc) // static
{
    if (HeapMH::GlobalRootMH == 0)
    {
        SF_DEBUG_MESSAGE(1, "CreateRootHeap: Memory System is not initialized");
        SF_ASSERT(0); return 0;
    }
    {
        LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
        if (Memory::pGlobalHeap)
        {
            SF_DEBUG_MESSAGE(1, "CreateRootHeap: Root Memory Heap is already created");
            SF_ASSERT(0); return 0;
        }
        HeapDesc d2 = desc;
        d2.HeapId = HeapId_Global;
        Memory::pGlobalHeap = HeapMH::GlobalRootMH->CreateHeap("Global", 0, d2);
        SF_ASSERT(Memory::pGlobalHeap);
    }
    return Memory::pGlobalHeap;
}

//------------------------------------------------------------------------
MemoryHeap* MemoryHeap::CreateRootHeapMH() // static
{
    HeapDesc desc;
    desc.Flags         = Heap_Root;
    desc.MinAlign      = RootHeap_MinAlign;
    desc.Granularity   = RootHeap_Granularity;
    desc.Reserve       = RootHeap_Reserve;
    desc.Threshold     = RootHeap_Threshold;
    desc.Limit         = RootHeap_Limit;
    desc.HeapId        = HeapId_Global;
    return CreateRootHeapMH(desc);
}

//------------------------------------------------------------------------
bool MemoryHeap::ReleaseRootHeapMH() // static
{
    bool hasNoLeaks;
    // No locks needed because this function can be called
    // only after all the threads have shut down.
    if (HeapMH::GlobalRootMH == 0)
    {
        SF_DEBUG_MESSAGE(1, "ReleaseRootHeap: Memory System is not initialized");
        SF_ASSERT(0); return true;
    }
    LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
    if (Memory::pGlobalHeap == 0)
    {
        SF_DEBUG_MESSAGE(1, "ReleaseRootHeap:: Root Memory Heap was not created");
        SF_ASSERT(0); return true;
    }
    hasNoLeaks = !Memory::pGlobalHeap->dumpMemoryLeaks();
    Memory::pGlobalHeap->destroyItself();
    Memory::pGlobalHeap = 0;
    return hasNoLeaks;
}


//------------------------------------------------------------------------
MemoryHeapMH::MemoryHeapMH() : MemoryHeap(), pEngine(0), pDebugStorage(0)
{
}



// *** Operations with memory arenas
//
//------------------------------------------------------------------------
void MemoryHeapMH::CreateArena(UPInt, SysAllocPaged*)
{
}

void MemoryHeapMH::DestroyArena(UPInt)
{
}

bool MemoryHeapMH::ArenaIsEmpty(UPInt)
{
    return true;
}


//------------------------------------------------------------------------
MemoryHeap* MemoryHeapMH::CreateHeap(const char* name, 
                                     const HeapDesc& desc)
{
    MemoryHeap* heap = 0;
    {
        LockSafe::Locker lock(HeapMH::GlobalRootMH->GetLock());
        heap = HeapMH::GlobalRootMH->CreateHeap(name, this, desc);
        if (heap)
        {
            RefCount++;
            // printf("Created heap %p: %s\n", heap, name); // DBG
        }
    }
    if (heap)
    {
        Lock::Locker lock(&HeapLock);
        ChildHeaps.PushBack(heap);
    }
    return heap;
}

//------------------------------------------------------------------------
UPInt MemoryHeapMH::GetFootprint() const
{
    Lock::Locker lock(&HeapLock);
    return pEngine->GetFootprint();
}

//------------------------------------------------------------------------
UPInt MemoryHeapMH::GetTotalFootprint() const
{
    Lock::Locker lock(&HeapLock);
    UPInt footprint = 0;
    if (!(Info.Desc.Flags & MemoryHeap::Heap_UserDebug))
    {
        footprint = pEngine->GetFootprint();
    }
    const MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        footprint += heap->GetTotalFootprint();
        heap = heap->pNext;
    }
    return footprint;
}

//------------------------------------------------------------------------
UPInt MemoryHeapMH::GetUsedSpace() const
{
    Lock::Locker lock(&HeapLock);
    return pEngine->GetUsedSpace();
}

//------------------------------------------------------------------------
UPInt MemoryHeapMH::GetTotalUsedSpace() const
{
    Lock::Locker lock(&HeapLock);
    UPInt space = 0;
    if (!(Info.Desc.Flags & MemoryHeap::Heap_UserDebug))
    {
        space = pEngine->GetUsedSpace();
    }
    const MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        space += heap->GetTotalUsedSpace();
        heap = heap->pNext;
    }
    return space;
}

//------------------------------------------------------------------------
void MemoryHeapMH::SetLimitHandler(LimitHandler* handler)
{
    Lock::Locker locker(&HeapLock);
    pEngine->SetLimitHandler(handler);
}

//------------------------------------------------------------------------
void MemoryHeapMH::SetLimit(UPInt newLimit)
{
    Lock::Locker locker(&HeapLock);
    if (newLimit < Info.Desc.Limit)
    {
        if (newLimit < pEngine->GetFootprint())
            newLimit = pEngine->GetFootprint();
    }
    Info.Desc.Limit = pEngine->SetLimit(newLimit);
}

//------------------------------------------------------------------------
void MemoryHeapMH::AddRef()
{
    // We use lock instead of atomic ops here to avoid accidental
    // heap release during VisitChildHeaps iteration.
    LockSafe::Locker lock(HeapMH::GlobalRootMH->GetLock());
    RefCount++;
}

//------------------------------------------------------------------------
void MemoryHeapMH::Release()
{
    MemoryHeap* parent = Info.pParent;
    if (parent)
    {
        // Lock scope.
        Lock::Locker localLock(&parent->HeapLock);
        LockSafe::Locker globalLock(HeapMH::GlobalRootMH->GetLock());

        if (--RefCount == 0)
        {
            // printf("Releasing heap %p: %s\n", this, GetName()); // DBG

            SF_ASSERT((Info.Desc.Flags & Heap_Root) == 0); // Cannot free root.

            dumpMemoryLeaks();
            SF_ASSERT(ChildHeaps.IsEmpty());

            parent->ChildHeaps.Remove(this);
            HeapMH::GlobalRootMH->DestroyHeap(this);
        }
        else
        {
            parent = 0;
        }
    }
    else
    {
        LockSafe::Locker lock(HeapMH::GlobalRootMH->GetLock());
        RefCount--;

        // The global heap can be destroyed only from ReleaseRootHeap().
        SF_ASSERT(RefCount);
    }

    // This Release() must be performed out of the lock scopes.
    if (parent)
        parent->Release();
}

//------------------------------------------------------------------------
bool MemoryHeapMH::dumpMemoryLeaks()
{
    bool ret = false;

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    ret = pDebugStorage->DumpMemoryLeaks(Info.pName);
#endif

    // TO DO: May use something else instead of SF_DEBUG_MESSAGE 
    // to report unreleased heaps. Or turn on SF_DEBUG_MESSAGE in
    // Release mode.
#ifdef SF_BUILD_DEBUG
    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        SF_DEBUG_MESSAGE2(1, "Heap '%s' has unreleased child '%s'", 
                          Info.pName, 
                          heap->Info.pName);
        heap->dumpMemoryLeaks();
        heap = heap->pNext;
        ret  = true;
    }
#endif

    return ret;
}


//------------------------------------------------------------------------
void MemoryHeapMH::destroyItself()
{
    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        MemoryHeap* next = heap->pNext;
        heap->destroyItself();
        heap = next;
    }
    HeapMH::GlobalRootMH->DestroyHeap(this);
}

//------------------------------------------------------------------------
void MemoryHeapMH::freeLocked(void* ptr, bool globalLocked)
{
    Lock::Locker heapLocker(&HeapLock);
    if (globalLocked)
    {
        pEngine->Free(ptr, true);
    }
    else
    {
        LockSafe::Locker rootLocker(HeapMH::GlobalRootMH->GetLock());
        pEngine->Free(ptr, true);
    }
}

//------------------------------------------------------------------------
inline void* MemoryHeapMH::allocMem(UPInt size, const AllocInfo* info, bool globalLocked)
{
    HeapMH::PageInfoMH pageInfo;
    void* ptr = pEngine->Alloc(size, &pageInfo, globalLocked);

#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnAlloc(this, size, 0, 0, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

#ifdef SF_MEMORY_MEMSET_ALL
    if (ptr)
        memset(ptr, 0xCC, pageInfo.UsableSize);
#endif

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(UPInt(ptr), size, &pageInfo, info, globalLocked))
        {
            freeLocked(ptr, globalLocked);
            ptr = 0;
        }
    }
#else
    SF_UNUSED(info);
#endif
    return ptr;
}


//------------------------------------------------------------------------
inline void* MemoryHeapMH::allocMem(UPInt size, UPInt align, const AllocInfo* info, bool globalLocked)
{
    HeapMH::PageInfoMH pageInfo;
    void* ptr = pEngine->Alloc(size, align, &pageInfo, globalLocked);

#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnAlloc(this, size, align, 0, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
#ifdef SF_MEMORY_MEMSET_ALL
    if (ptr)
        memset(ptr, 0xCC, pageInfo.UsableSize);
#endif

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(UPInt(ptr), size, &pageInfo, info, globalLocked))
        {
            freeLocked(ptr, globalLocked);
            ptr = 0;
        }
    }
#else
    SF_UNUSED(info);
#endif
    return ptr;
}

//------------------------------------------------------------------------
inline void* MemoryHeapMH::allocMem(HeapMH::PageInfoMH* parentInfo, const void *thisPtr, 
                                    UPInt size, const AllocInfo* info, bool globalLocked)
{
    HeapMH::PageInfoMH pageInfo;
    void* ptr = pEngine->Alloc(size, &pageInfo, globalLocked);

#ifdef SF_MEMORY_TRACE_ALL
    unsigned sid = 0;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (ptr && TrackDebugInfo)
    {
        sid = pDebugStorage->GetStatId(&pageInfo, UPInt(thisPtr), info);
    }
#endif
    HeapMH::GlobalRootMH->OnAlloc(this, size, 0, sid, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
#ifdef SF_MEMORY_MEMSET_ALL
    if (ptr)
        memset(ptr, 0xCC, pageInfo.UsableSize);
#endif

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(parentInfo, UPInt(thisPtr), 
                                     UPInt(ptr), size, 
                                     &pageInfo, info, globalLocked))
        {
            freeLocked(ptr, globalLocked);
            ptr = 0;
        }
    }
#else
    SF_UNUSED3(parentInfo, thisPtr, info);
#endif
    return ptr;
}

//------------------------------------------------------------------------
inline void* MemoryHeapMH::allocMem(HeapMH::PageInfoMH* parentInfo, 
                                    const void *thisPtr, UPInt size, UPInt align,
                                    const AllocInfo* info, bool globalLocked)
{
    HeapMH::PageInfoMH pageInfo;
    void* ptr = pEngine->Alloc(size, align, &pageInfo, globalLocked);

#ifdef SF_MEMORY_TRACE_ALL
    unsigned sid = 0;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (ptr && TrackDebugInfo)
    {
        sid = pDebugStorage->GetStatId(&pageInfo, UPInt(thisPtr), info);
    }
#endif
    HeapMH::GlobalRootMH->OnAlloc(this, size, align, sid, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
#ifdef SF_MEMORY_MEMSET_ALL
    if (ptr)
        memset(ptr, 0xCC, pageInfo.UsableSize);
#endif

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(parentInfo, UPInt(thisPtr), 
                                     UPInt(ptr), size, 
                                     &pageInfo, info, globalLocked))
        {
            freeLocked(ptr, globalLocked);
            ptr = 0;
        }
    }
#else
    SF_UNUSED3(parentInfo, thisPtr, info);
#endif
    return ptr;
}


//------------------------------------------------------------------------
void* MemoryHeapMH::Alloc(UPInt size, const AllocInfo* info)
{
    if (UseLocks)
    {
        Lock::Locker locker(&HeapLock);
        return allocMem(size, info, false);
    }
    SF_MEMORY_CHECK_CURRENT_THREAD(this);
    return allocMem(size, info, false);
}

//------------------------------------------------------------------------
void* MemoryHeapMH::Alloc(UPInt size, UPInt align, const AllocInfo* info)
{
    if (UseLocks)
    {
        Lock::Locker locker(&HeapLock);
        return allocMem(size, align, info, false);
    }
    SF_MEMORY_CHECK_CURRENT_THREAD(this);
    return allocMem(size, align, info, false);
}


//------------------------------------------------------------------------
void* MemoryHeapMH::AllocAutoHeap(const void *thisPtr, UPInt size,
                                  const AllocInfo* info)
{
    MemoryHeapMH* heap;
    HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(thisPtr));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH parentInfo;
#endif
    void* ptr = 0;
    if (page)
    {
        heap = page->pHeap;
        if (heap->UseLocks)
        {
            Lock::Locker locker(&heap->HeapLock);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(page, &parentInfo);
            return heap->allocMem(&parentInfo, thisPtr, size, info, false);
#else
            return heap->allocMem(0, thisPtr, size, info, false);
#endif
        }
        else
        {
            SF_MEMORY_CHECK_CURRENT_THREAD(heap);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(page, &parentInfo);
            return heap->allocMem(&parentInfo, thisPtr, size, info, false);
#else
            return heap->allocMem(0, thisPtr, size, info, false);
#endif
        }
    }
    else
    {
        HeapMH::NodeMH* node = 0;
        {
            LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
            node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)thisPtr);
            SF_ASSERT(node);
            heap = node->GetHeap();
        }
        if (heap->UseLocks)
        {
            Lock::Locker locker1(&heap->HeapLock);
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(node, &parentInfo);
            ptr = heap->allocMem(&parentInfo, thisPtr, size, info, true);
#else
            ptr = heap->allocMem(0, thisPtr, size, info, true);
#endif
        }
        else
        {
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(node, &parentInfo);
            ptr = heap->allocMem(&parentInfo, thisPtr, size, info, true);
#else
            ptr = heap->allocMem(0, thisPtr, size, info, true);
#endif
        }
    }
    return ptr;
}

//------------------------------------------------------------------------
void* MemoryHeapMH::AllocAutoHeap(const void *thisPtr, UPInt size, UPInt align,
                                  const AllocInfo* info)
{
    MemoryHeapMH* heap;
    HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(thisPtr));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH parentInfo;
#endif
    void* ptr = 0;
    if (page)
    {
        heap = page->pHeap;
        if (heap->UseLocks)
        {
            Lock::Locker locker(&heap->HeapLock);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(page, &parentInfo);
            return heap->allocMem(&parentInfo, thisPtr, size, align, info, false);
#else
            return heap->allocMem(0, thisPtr, size, align, info, false);
#endif
        }
        else
        {
            SF_MEMORY_CHECK_CURRENT_THREAD(heap);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(page, &parentInfo);
            return heap->allocMem(&parentInfo, thisPtr, size, align, info, false);
#else
            return heap->allocMem(0, thisPtr, size, align, info, false);
#endif
        }
    }
    else
    {
        HeapMH::NodeMH* node = 0;
        {
            LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
            node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)thisPtr);
            SF_ASSERT(node);
            heap = node->GetHeap();
        }
        if (heap->UseLocks)
        {
            Lock::Locker locker1(&heap->HeapLock);
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(node, &parentInfo);
            ptr = heap->allocMem(&parentInfo, thisPtr, size, align, info, true);
#else
            ptr = heap->allocMem(0, thisPtr, size, align, info, true);
#endif
        }
        else
        {
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->GetPageInfo(node, &parentInfo);
            ptr = heap->allocMem(&parentInfo, thisPtr, size, align, info, true);
#else
            ptr = heap->allocMem(0, thisPtr, size, align, info, true);
#endif
        }
    }
    return ptr;
}



//------------------------------------------------------------------------
void  MemoryHeapMH::freeMem(HeapMH::PageMH* page, void* ptr, bool globalLocked)
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH pageInfo;
    pEngine->GetPageInfoWithSize(page, ptr, &pageInfo);
    pDebugStorage->RemoveAlloc(UPInt(ptr), &pageInfo, globalLocked);
#ifdef SF_MEMORY_MEMSET_ALL
    memset(ptr, 0xFE, pageInfo.UsableSize);
#endif
#endif
    pEngine->Free(page, ptr, globalLocked);
#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnFree(this, ptr);
#endif
}

//------------------------------------------------------------------------
void  MemoryHeapMH::freeMem(HeapMH::NodeMH* node, void* ptr, bool globalLocked)
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH pageInfo;
    pEngine->GetPageInfoWithSize(node, ptr, &pageInfo);
    pDebugStorage->RemoveAlloc(UPInt(ptr), &pageInfo, globalLocked);
#ifdef SF_MEMORY_MEMSET_ALL
    memset(ptr, 0xFE, pageInfo.UsableSize);
#endif
#endif
    pEngine->Free(node, ptr, globalLocked);
#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnFree(this, ptr);
#endif
}

//------------------------------------------------------------------------
void  MemoryHeapMH::Free(void* ptr)
{
//UltimateCheck(); // DBG
//static int c; if(++c % 500 == 0) return; // DBG Simulate memory leaks
    if (ptr)
    {
        MemoryHeapMH* heap = 0;
        HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(ptr));
        if (page)
        {
            heap = page->pHeap;
            if (heap->UseLocks)
            {
                Lock::Locker locker(&heap->HeapLock);
                heap->freeMem(page, ptr, false);
            }
            else
            {
                SF_MEMORY_CHECK_CURRENT_THREAD(heap);
                heap->freeMem(page, ptr, false);
            }
        }
        else
        {
            HeapMH::NodeMH* node = 0;
            {
                LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
                node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)ptr);
                SF_ASSERT(node);
                heap = node->GetHeap();
            }
            if (heap->UseLocks)
            {
                Lock::Locker locker1(&heap->HeapLock);
                LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
                heap->freeMem(node, ptr, true);
            }
            else
            {
                LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
                heap->freeMem(node, ptr, true);
            }
        }
        SF_ASSERT(heap);
        if (ptr == heap->pAutoRelease)
            heap->Release();
    }
}


//------------------------------------------------------------------------
void* MemoryHeapMH::reallocMem(HeapMH::PageMH* page, void* oldPtr, UPInt newSize, bool globalLocked)
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH oldPageInfo;
    pEngine->GetPageInfoWithSize(page, oldPtr, &oldPageInfo);
    HeapMH::DebugStorageMH::DebugDataPtr debugData;
    if (TrackDebugInfo)
    {
        pDebugStorage->UnlinkAlloc(UPInt(oldPtr), &oldPageInfo, &debugData);
        pDebugStorage->CheckDataTail(&debugData, oldPageInfo.UsableSize);
    }
#ifdef SF_MEMORY_MEMSET_ALL
    if (newSize < oldPageInfo.UsableSize)
    {
        memset((UByte*)oldPtr + newSize, 0xFE, oldPageInfo.UsableSize - newSize);
    }
#endif //SF_MEMORY_MEMSET_ALL
#endif //SF_MEMORY_ENABLE_DEBUG_INFO

    HeapMH::PageInfoMH newPageInfo;
    void* newPtr = pEngine->ReallocInPage(page, oldPtr, newSize, &newPageInfo, globalLocked);
    if (newPtr == 0)
    {
        if (globalLocked)
        {
            newPtr = pEngine->ReallocGeneral(page, oldPtr, newSize, &newPageInfo, true);
        }
        else
        {
            LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
            newPtr = pEngine->ReallocGeneral(page, oldPtr, newSize, &newPageInfo, true);
        }
    }

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (newPtr)
    {
#ifdef SF_MEMORY_MEMSET_ALL
        if (newPtr && newSize > newPageInfo.UsableSize)
        {
            memset((UByte*)newPtr + newPageInfo.UsableSize, 0xCC, newSize - newPageInfo.UsableSize);
        }
#endif //SF_MEMORY_MEMSET_ALL
        if (TrackDebugInfo && newPtr)
        {
            pDebugStorage->RelinkAlloc(&debugData, UPInt(oldPtr), 
                                       UPInt(newPtr), newSize,
                                       &newPageInfo);
        }
    }
#endif //SF_MEMORY_ENABLE_DEBUG_INFO

#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnRealloc(this, oldPtr, newSize, newPtr);
#endif

    return newPtr;
}


//------------------------------------------------------------------------
void* MemoryHeapMH::reallocMem(HeapMH::NodeMH* node, void* oldPtr, UPInt newSize, bool globalLocked)
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    HeapMH::PageInfoMH oldPageInfo;
    pEngine->GetPageInfoWithSize(node, oldPtr, &oldPageInfo);
    HeapMH::DebugStorageMH::DebugDataPtr debugData;
    if (TrackDebugInfo)
    {
        pDebugStorage->UnlinkAlloc(UPInt(oldPtr), &oldPageInfo, &debugData);
        pDebugStorage->CheckDataTail(&debugData, oldPageInfo.UsableSize);
    }
#ifdef SF_MEMORY_MEMSET_ALL
    if (newSize < oldPageInfo.UsableSize)
    {
        memset((UByte*)oldPtr + newSize, 0xFE, oldPageInfo.UsableSize - newSize);
    }
#endif //SF_MEMORY_MEMSET_ALL
#endif //SF_MEMORY_ENABLE_DEBUG_INFO

    HeapMH::PageInfoMH newPageInfo;
    void* newPtr = pEngine->ReallocInNode(node, oldPtr, newSize, &newPageInfo, globalLocked);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (newPtr)
    {
#ifdef SF_MEMORY_MEMSET_ALL
        if (newPtr && newSize > newPageInfo.UsableSize)
        {
            memset((UByte*)newPtr + newPageInfo.UsableSize, 0xCC, newSize - newPageInfo.UsableSize);
        }
#endif //SF_MEMORY_MEMSET_ALL
        if (TrackDebugInfo && newPtr)
        {
            pDebugStorage->RelinkAlloc(&debugData, UPInt(oldPtr), 
                                       UPInt(newPtr), newSize, 
                                       &newPageInfo);
        }
    }
#endif //SF_MEMORY_ENABLE_DEBUG_INFO

#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->OnRealloc(this, oldPtr, newSize, newPtr);
#endif

    return newPtr;
}


//------------------------------------------------------------------------
void* MemoryHeapMH::Realloc(void* oldPtr, UPInt newSize)
{
    // Realloc cannot work as Alloc in case oldPtr is 0. It's done in 
    // order not to pass a lot of parameters, such as DebugInfo, alignment, 
    // parent allocation, etc.
    //------------------------
    SF_ASSERT(oldPtr != 0);

    MemoryHeapMH* heap;
    HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(oldPtr));
    void* newPtr = 0;
    if (page)
    {
        heap = page->pHeap;
        if (heap->UseLocks)
        {
            Lock::Locker locker(&heap->HeapLock);
            newPtr = heap->reallocMem(page, oldPtr, newSize, false);
        }
        else
        {
            SF_MEMORY_CHECK_CURRENT_THREAD(heap);
            newPtr = heap->reallocMem(page, oldPtr, newSize, false);
        }
    }
    else
    {
        HeapMH::NodeMH* node = 0;
        {
            LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
            node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)oldPtr);
            SF_ASSERT(node);
            heap = node->GetHeap();
        }
        if (heap->UseLocks)
        {
            Lock::Locker locker1(&heap->HeapLock);
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
            newPtr = heap->reallocMem(node, oldPtr, newSize, true);
        }
        else
        {
            LockSafe::Locker locker2(HeapMH::GlobalRootMH->GetLock());
            newPtr = heap->reallocMem(node, oldPtr, newSize, true);
        }
    }
    return newPtr;
}

//------------------------------------------------------------------------
MemoryHeap* MemoryHeapMH::GetAllocHeap(const void *thisPtr)
{
    HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(thisPtr));
    if (page) return page->pHeap;
    LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
    HeapMH::NodeMH* node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)thisPtr);
    SF_ASSERT(node);
    return node->GetHeap();
}

//------------------------------------------------------------------------
UPInt MemoryHeapMH::GetUsableSize(const void* ptr)
{
    HeapMH::PageMH* page = HeapMH::GlobalRootMH->ResolveAddress(UPInt(ptr));
    HeapMH::PageInfoMH pageInfo;
    if (page) 
    {
        page->pHeap->pEngine->GetPageInfoWithSize(page, ptr, &pageInfo);
        return pageInfo.UsableSize;
    }
    LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
    HeapMH::NodeMH* node = HeapMH::GlobalRootMH->FindNodeInGlobalTree((UByte*)ptr);
    SF_ASSERT(node);
    node->GetHeap()->pEngine->GetPageInfoWithSize(node, ptr, &pageInfo);
    return pageInfo.UsableSize;
}

//------------------------------------------------------------------------
void* MemoryHeapMH::AllocSysDirect(UPInt size)
{
    LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
    return HeapMH::GlobalRootMH->GetSysAlloc()->Alloc(size, sizeof(void*));
}

void MemoryHeapMH::FreeSysDirect(void* ptr, UPInt size)
{
    LockSafe::Locker locker(HeapMH::GlobalRootMH->GetLock());
    return HeapMH::GlobalRootMH->GetSysAlloc()->Free(ptr, size, sizeof(void*));
}

//------------------------------------------------------------------------
bool MemoryHeapMH::GetStats(StatBag* bag)
{
    Lock::Locker locker(&HeapLock);

    UPInt footprint = pEngine->GetFootprint();
    
    bag->AddStat(StatHeap_LocalFootprint,   CounterStat(footprint));
    bag->AddStat(StatHeap_LocalUsedSpace,   CounterStat(pEngine->GetUsedSpace()));
    bag->AddStat(StatHeap_Granularity,      CounterStat(0));
    bag->AddStat(StatHeap_Reserve,          CounterStat(0));

    bag->AddStat(StatHeap_Bookkeeping,         CounterStat(0));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    bag->AddStat(StatHeap_DebugInfo,           CounterStat(pDebugStorage->GetUsedSpace()));
#endif
    bag->AddStat(StatHeap_Segments,            CounterStat(0));
    bag->AddStat(StatHeap_DynamicGranularity,  CounterStat(0));
    bag->AddStat(StatHeap_SysDirectSpace,      CounterStat(0));

    // Collect child footprint.
    UPInt childHeapCount = 0;
    UPInt childFootprint = 0;

    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        // TBD: Should we have more detailed logic here?
        if (!(heap->Info.Desc.Flags & MemoryHeap::Heap_UserDebug))
        {
            childHeapCount++;
            // Thread TBD: Need to make sure that child heap locks are not obtained before parent.
            childFootprint += heap->GetTotalFootprint();
        }       
        heap = heap->pNext;
    }

    if (childHeapCount)
    {
        bag->AddStat(StatHeap_ChildHeaps,    CounterStat(childHeapCount));
        bag->AddStat(StatHeap_ChildFootprint,CounterStat(childFootprint));
    }
    bag->AddStat(StatHeap_TotalFootprint,   CounterStat(childFootprint + footprint));

    // Collect detailed memory summary information in debug builds.
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    pDebugStorage->GetStats(pEngine, bag);
#endif    
    return true;
}


//------------------------------------------------------------------------
void MemoryHeapMH::VisitMem(Heap::MemVisitor*, unsigned)
{
}

//------------------------------------------------------------------------
void MemoryHeapMH::VisitHeapSegments(Heap::SegVisitor*) const
{
}

//------------------------------------------------------------------------
void MemoryHeapMH::VisitRootSegments(Heap::SegVisitor*)
{
}

//------------------------------------------------------------------------
void MemoryHeapMH::getUserDebugStats(RootStats* stats) const
{
    Lock::Locker lock(&HeapLock);
    if (Info.Desc.Flags & MemoryHeap::Heap_UserDebug)
    {
        stats->UserDebugFootprint += pEngine->GetFootprint();
        stats->UserDebugUsedSpace += pEngine->GetUsedSpace();
    }
    const MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        heap->getUserDebugStats(stats);
        heap = heap->pNext;
    }
}

//------------------------------------------------------------------------
void MemoryHeapMH::GetRootStats(RootStats* stats)
{
    stats->UserDebugFootprint = 0;
    stats->UserDebugUsedSpace = 0;
    getUserDebugStats(stats);

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    stats->DebugInfoFootprint   = pDebugStorage->GetUsedSpace();
    stats->DebugInfoUsedSpace   = pDebugStorage->GetUsedSpace();
#else
    stats->DebugInfoFootprint   = 0;
    stats->DebugInfoUsedSpace   = 0;
#endif
    stats->SysMemFootprint      = GetTotalFootprint() + stats->UserDebugFootprint + stats->DebugInfoFootprint;
    stats->SysMemUsedSpace      = GetTotalUsedSpace() + stats->UserDebugFootprint + stats->DebugInfoFootprint;
    stats->PageMapFootprint     = 0;
    stats->PageMapUsedSpace     = 0;
    stats->BookkeepingFootprint = 0;
    stats->BookkeepingUsedSpace = 0;
}

//------------------------------------------------------------------------
void MemoryHeapMH::SetTracer(HeapTracer* tracer) 
{
#ifdef SF_MEMORY_TRACE_ALL
    HeapMH::GlobalRootMH->pTracer = tracer;
#else
    SF_UNUSED(tracer);
#endif
}

//------------------------------------------------------------------------
void MemoryHeapMH::ultimateCheck()
{
}

//------------------------------------------------------------------------
void MemoryHeapMH::checkIntegrity() const
{
}

//------------------------------------------------------------------------
void MemoryHeapMH::releaseCachedMem()
{
}
    

//------------------------------------------------------------------------
// Implementation of SysAllocBase based on MemoryHeapMH.
// Initializes heap system, creating and initializing GlobalHeap.
bool SysAlloc::initHeapEngine(const void* heapDesc)
{
    MemoryHeap::InitMH(this);
    const MemoryHeap::HeapDesc& rootHeapDesc = *(const MemoryHeap::HeapDesc*)heapDesc;
    MemoryHeap* p = MemoryHeap::CreateRootHeapMH(rootHeapDesc);
    return p != 0;
}

bool SysAlloc::shutdownHeapEngine()
{
    bool hasNoLeaks = MemoryHeap::ReleaseRootHeapMH();
    MemoryHeap::CleanUpMH();
    return hasNoLeaks;
}

#ifdef SF_OS_3DS

static nn::os::MemoryBlock mb;

SysAllocMalloc::SysAllocMalloc(UByte* base, UPInt size)
{
    if (base == 0)
    {
        mb.Initialize(size);
        base = (UByte*)mb.GetAddress();
    }
    ::new(PrivateData) SysAllocStatic(base, size);
}
#endif

} // Scaleform


