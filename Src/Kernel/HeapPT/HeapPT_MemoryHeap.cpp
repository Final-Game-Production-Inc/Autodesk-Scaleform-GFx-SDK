/**************************************************************************

Filename    :   HeapPT_MemoryHeap.cpp
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

#include "HeapPT_Root.h"
#include "HeapPT_AllocEngine.h"
#include "HeapPT_DebugInfo.h"
#include "HeapPT_MemoryHeap.h"
#include "../SF_Debug.h"
#include "../SF_Memory.h"

#ifdef SF_MEMORY_FORCE_MALLOC
#include <malloc.h>
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
#include "HeapPT_SysAllocMalloc.h"
namespace Scaleform {
HeapPT_SysAlloc gSysAlloc;
}
#endif



#undef new

#ifdef SN_TARGET_PS3
#include <stdlib.h>	// for reallocalign()
#endif

#if defined(SF_OS_WIN32) && defined(SF_BUILD_DEBUG)
#define SF_MEMORYHEAP_CHECK_CURRENT_THREAD(h) SF_ASSERT((h->OwnerThreadId == 0) || (h->OwnerThreadId == ::GetCurrentThreadId()))
#else
#define SF_MEMORYHEAP_CHECK_CURRENT_THREAD(h)
#endif

#ifndef SF_OS_WIN32
#define _aligned_malloc(s,a) memalign(a,s)
#define _aligned_realloc(p,s,a) realloc(p,s)
#define _aligned_free free
#endif

namespace Scaleform {

using namespace Heap;

//------------------------------------------------------------------------
static UPInt RootMem[(sizeof(HeapPT::HeapRoot) + sizeof(UPInt) - 1) / sizeof(UPInt)];


//------------------------------------------------------------------------
void MemoryHeap::InitPT(SysAllocPaged* sysAlloc) // static
{
#ifdef SF_MEMORY_FORCE_SYSALLOC
    gSysAlloc.Init();
#endif
    if (HeapPT::GlobalPageTable || HeapPT::GlobalRoot)
    {
        SF_DEBUG_MESSAGE(1, "MemoryHeap::Init: Memory system has been already initialized");
        SF_HEAP_ASSERT(0); return;
    }

    // Function HeapPageTable::Init() does not allocate any memory;
    // it just initializes the root page table with zeros. So, the 
    // sequence of calls must remain as below: First init the page table,
    // then create the root.
    HeapPT::PageTable::Init();
    ::new(RootMem) HeapPT::HeapRoot(sysAlloc);
}


//------------------------------------------------------------------------
void MemoryHeap::CleanUpPT() // static
{
    if (HeapPT::GlobalPageTable == 0 || HeapPT::GlobalRoot == 0)
    {
        SF_DEBUG_MESSAGE(1, "MemoryHeap::Finalize: Memory system was not initialized");
        SF_HEAP_ASSERT(0); return;
    }

    HeapPT::GlobalRoot->DestroyAllArenas();

    HeapPT::GlobalPageTable->~PageTable();
    HeapPT::GlobalPageTable = 0;

    HeapPT::GlobalRoot->~HeapRoot();
    HeapPT::GlobalRoot = 0;
}


//------------------------------------------------------------------------
MemoryHeap* MemoryHeap::CreateRootHeapPT(const HeapDesc& desc) // static
{
    if (HeapPT::GlobalRoot == 0)
    {
        SF_DEBUG_MESSAGE(1, "CreateRootHeap: Memory System is not initialized");
        SF_HEAP_ASSERT(0); return 0;
    }
    {
        LockSafe::Locker locker(HeapPT::GlobalRoot->GetLock());
        if (Memory::pGlobalHeap)
        {
            SF_DEBUG_MESSAGE(1, "CreateRootHeap: Root Memory Heap is already created");
            SF_HEAP_ASSERT(0); return 0;
        }
        HeapDesc d2 = desc;
        d2.HeapId = HeapId_Global;
        Memory::pGlobalHeap = HeapPT::GlobalRoot->CreateHeap("Global", 0, d2);
        SF_HEAP_ASSERT(Memory::pGlobalHeap);
    }
    return Memory::pGlobalHeap;
}

//------------------------------------------------------------------------
MemoryHeap* MemoryHeap::CreateRootHeapPT() // static
{
    HeapDesc desc;
    desc.Flags         = Heap_Root;
    desc.MinAlign      = RootHeap_MinAlign;
    desc.Granularity   = RootHeap_Granularity;
    desc.Reserve       = RootHeap_Reserve;
    desc.Threshold     = RootHeap_Threshold;
    desc.Limit         = RootHeap_Limit;
    desc.HeapId        = HeapId_Global;
    return CreateRootHeapPT(desc);
}

//------------------------------------------------------------------------
bool MemoryHeap::ReleaseRootHeapPT() // static
{
    bool hasNoLeaks;
    // No locks needed because this function can be called
    // only after all the threads have shut down.
    if (HeapPT::GlobalRoot == 0)
    {
        SF_DEBUG_MESSAGE(1, "ReleaseRootHeap: Memory System is not initialized");
        SF_HEAP_ASSERT(0);
        return true;
    }
    LockSafe::Locker locker(HeapPT::GlobalRoot->GetLock());
    if (Memory::pGlobalHeap == 0)
    {
        SF_DEBUG_MESSAGE(1, "ReleaseRootHeap:: Root Memory Heap was not created");
        SF_HEAP_ASSERT(0);
        return true;
    }
    hasNoLeaks = !Memory::pGlobalHeap->dumpMemoryLeaks();
    Memory::pGlobalHeap->destroyItself();
    Memory::pGlobalHeap = 0;
    return hasNoLeaks;
}


//------------------------------------------------------------------------
MemoryHeapPT::MemoryHeapPT() : MemoryHeap(), pEngine(0), pDebugStorage(0)
{
}



// *** Operations with memory arenas
//
//------------------------------------------------------------------------
void MemoryHeapPT::CreateArena(UPInt arena, SysAllocPaged* sysAlloc)
{
    SF_HEAP_ASSERT(HeapPT::GlobalRoot);
    HeapPT::GlobalRoot->CreateArena(arena, sysAlloc);
}

void MemoryHeapPT::DestroyArena(UPInt arena)
{
    SF_HEAP_ASSERT(HeapPT::GlobalRoot);
    HeapPT::GlobalRoot->DestroyArena(arena);
}

bool MemoryHeapPT::ArenaIsEmpty(UPInt arena)
{
    SF_HEAP_ASSERT(HeapPT::GlobalRoot);
    return HeapPT::GlobalRoot->ArenaIsEmpty(arena);
}


//------------------------------------------------------------------------
MemoryHeap* MemoryHeapPT::CreateHeap(const char* name, 
                                     const HeapDesc& desc)
{
#if defined(SF_MEMORY_FORCE_MALLOC) || defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED2(name, desc);
    return this;
#else
    MemoryHeap* heap = 0;
    {
        LockSafe::Locker lock(HeapPT::GlobalRoot->GetLock());
        heap = HeapPT::GlobalRoot->CreateHeap(name, this, desc);
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
#endif
}

//------------------------------------------------------------------------
UPInt MemoryHeapPT::GetFootprint() const
{
    Lock::Locker lock(&HeapLock);
    return pEngine->GetFootprint();
}

//------------------------------------------------------------------------
UPInt MemoryHeapPT::GetTotalFootprint() const
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
UPInt MemoryHeapPT::GetUsedSpace() const
{
    Lock::Locker lock(&HeapLock);
    return pEngine->GetUsedSpace();
}

//------------------------------------------------------------------------
UPInt MemoryHeapPT::GetTotalUsedSpace() const
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
void MemoryHeapPT::SetLimitHandler(LimitHandler* handler)
{
    Lock::Locker locker(&HeapLock);
    pEngine->SetLimitHandler(handler);
}

//------------------------------------------------------------------------
void MemoryHeapPT::SetLimit(UPInt newLimit)
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
void MemoryHeapPT::AddRef()
{
#if defined(SF_MEMORY_FORCE_MALLOC) || defined(SF_MEMORY_FORCE_SYSALLOC)
#else
    // We use lock instead of atomic ops here to avoid accidental
    // heap release during VisitChildHeaps iteration.
    LockSafe::Locker lock(HeapPT::GlobalRoot->GetLock());
    RefCount++;
#endif
}

//------------------------------------------------------------------------
void MemoryHeapPT::Release()
{
#if defined(SF_MEMORY_FORCE_MALLOC) || defined(SF_MEMORY_FORCE_SYSALLOC)
#else
    MemoryHeap* parent = Info.pParent;
    if (parent)
    {
        // Lock scope.
        Lock::Locker localLock(&parent->HeapLock);
        LockSafe::Locker globalLock(HeapPT::GlobalRoot->GetLock());

        if (--RefCount == 0)
        {
            // printf("Releasing heap %p: %s\n", this, GetName()); // DBG

            SF_HEAP_ASSERT((Info.Desc.Flags & Heap_Root) == 0); // Cannot free root.

            dumpMemoryLeaks();
            SF_HEAP_ASSERT(ChildHeaps.IsEmpty());

            parent->ChildHeaps.Remove(this);
            HeapPT::GlobalRoot->DestroyHeap(this);
        }
        else
        {
            parent = 0;
        }
    }
    else
    {
        LockSafe::Locker lock(HeapPT::GlobalRoot->GetLock());
        RefCount--;

        // The global heap can be destroyed only from ReleaseRootHeap().
        SF_HEAP_ASSERT(RefCount);
    }

    // This Release() must be performed out of the lock scopes.
    if (parent)
        parent->Release();
#endif
}

//------------------------------------------------------------------------
bool MemoryHeapPT::dumpMemoryLeaks()
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
void MemoryHeapPT::destroyItself()
{
    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        MemoryHeap* next = heap->pNext;
        heap->destroyItself();
        heap = next;
    }
    HeapPT::GlobalRoot->DestroyHeap(this);
}


//------------------------------------------------------------------------
inline void* MemoryHeapPT::allocMem(UPInt size, const AllocInfo* info)
{
    void* ptr = pEngine->Alloc(size);

#ifdef SF_MEMORY_TRACE_ALL
    HeapPT::GlobalRoot->OnAlloc(this, size, 0, 0, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

    UPInt usable = 0;
    if (ptr)
    {
        usable = pEngine->GetUsableSize(ptr);
#ifdef SF_MEMORY_MEMSET_ALL
        memset(ptr, 0xCC, usable);
#endif
    }

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(0, false, UPInt(ptr), size, usable, info))
        {
            pEngine->Free(HeapPT::GlobalPageTable->GetSegment(UPInt(ptr)), ptr);
            ptr = 0;
        }
    }
#else
    SF_UNUSED(info);
#endif
    return ptr;
}


//------------------------------------------------------------------------
inline void* MemoryHeapPT::allocMem(UPInt size, UPInt align, const AllocInfo* info)
{
    void* ptr = pEngine->Alloc(size, align);

#ifdef SF_MEMORY_TRACE_ALL
    HeapPT::GlobalRoot->OnAlloc(this, size, align, 0, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt usable = 0;
    if (ptr)
    {
        usable = pEngine->GetUsableSize(ptr);
#ifdef SF_MEMORY_MEMSET_ALL
        memset(ptr, 0xCC, usable);
#endif
    }

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(0, false, UPInt(ptr), size, usable, info))
        {
            pEngine->Free(HeapPT::GlobalPageTable->GetSegment(UPInt(ptr)), ptr);
            ptr = 0;
        }
    }
#else
    SF_UNUSED(info);
#endif
    return ptr;
}

//------------------------------------------------------------------------
inline void* MemoryHeapPT::allocMem(const void *thisPtr, UPInt size, const AllocInfo* info)
{
    void* ptr = pEngine->Alloc(size);

#ifdef SF_MEMORY_TRACE_ALL
    unsigned sid = 0;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (ptr && TrackDebugInfo)
    {
        sid = pDebugStorage->GetStatId(UPInt(thisPtr), info);
    }
#endif
    HeapPT::GlobalRoot->OnAlloc(this, size, 0, sid, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt usable = 0;
    if (ptr)
    {
        usable = pEngine->GetUsableSize(ptr);
#ifdef SF_MEMORY_MEMSET_ALL
        memset(ptr, 0xCC, usable);
#endif
    }

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(UPInt(thisPtr), true, UPInt(ptr), size, usable, info))
        {
            pEngine->Free(ptr);
            ptr = 0;
        }
    }
#else
    SF_UNUSED2(thisPtr, info);
#endif
    return ptr;
}

//------------------------------------------------------------------------
inline void* MemoryHeapPT::allocMem(const void *thisPtr, UPInt size, UPInt align,
                                    const AllocInfo* info)
{
    void* ptr = pEngine->Alloc(size, align);

#ifdef SF_MEMORY_TRACE_ALL
    unsigned sid = 0;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    if (ptr && TrackDebugInfo)
    {
        sid = pDebugStorage->GetStatId(UPInt(thisPtr), info);
    }
#endif
    HeapPT::GlobalRoot->OnAlloc(this, size, align, sid, ptr);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt usable = 0;
    if (ptr)
    {
        usable = pEngine->GetUsableSize(ptr);
#ifdef SF_MEMORY_MEMSET_ALL
        memset(ptr, 0xCC, usable);
#endif
    }

    if (ptr && TrackDebugInfo)
    {
        if (!pDebugStorage->AddAlloc(UPInt(thisPtr), true, UPInt(ptr), size, usable, info))
        {
            pEngine->Free(ptr);
            ptr = 0;
        }
    }
#else
    SF_UNUSED2(thisPtr, info);
#endif
    return ptr;
}

//------------------------------------------------------------------------
inline void* MemoryHeapPT::reallocMem(HeapSegment* seg, void* oldPtr, UPInt newSize)
{
//// DBG
//UPInt oldSize = GetUsableSize(oldPtr);
//void* ptr = Alloc(newSize, (AllocInfo*)0);
//      //newSize = GetUsableSize(ptr);
//memcpy(ptr, oldPtr, (oldSize < newSize) ? oldSize : newSize);
//Free(oldPtr);
//return ptr;

    // Realloc cannot work as Alloc in case oldPtr is 0. It's done in 
    // order not to pass a lot of parameters, such as DebugInfo, alignment, 
    // parent allocation, etc.
    //------------------------
    SF_HEAP_ASSERT(oldPtr != 0);

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt usable = pEngine->GetUsableSize(oldPtr);
    HeapPT::DebugStorage::DebugDataPtr debugData;
    if (TrackDebugInfo)
    {
        pDebugStorage->UnlinkAlloc(UPInt(oldPtr), &debugData);
#ifdef SF_64BIT_POINTERS
        pDebugStorage->CheckDataTail(UPInt(oldPtr), usable);
#else
        pDebugStorage->CheckDataTail(&debugData, usable);
#endif
    }
#ifdef SF_MEMORY_MEMSET_ALL
    if (newSize < usable)
    {
        memset((UByte*)oldPtr + newSize, 0xFE, usable - newSize);
    }
#endif //SF_MEMORY_MEMSET_ALL
#endif //SF_MEMORY_ENABLE_DEBUG_INFO

    void* newPtr = pEngine->Realloc(seg, oldPtr, newSize);
    if (newPtr)
    {
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        usable = pEngine->GetUsableSize(newPtr);
#ifdef SF_MEMORY_MEMSET_ALL
        if (newPtr && newSize > usable)
        {
            memset((UByte*)newPtr + usable, 0xCC, newSize - usable);
        }
#endif //SF_MEMORY_MEMSET_ALL
        if (TrackDebugInfo && newPtr)
        {
            pDebugStorage->RelinkAlloc(&debugData, UPInt(oldPtr), 
                                       UPInt(newPtr), newSize, usable);
        }
#endif //SF_MEMORY_ENABLE_DEBUG_INFO
    }

#ifdef SF_MEMORY_TRACE_ALL
    HeapPT::GlobalRoot->OnRealloc(this, oldPtr, newSize, newPtr);
#endif

    return newPtr;
}


//------------------------------------------------------------------------
void* MemoryHeapPT::Alloc(UPInt size, const AllocInfo* info)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED(info);
    return _aligned_malloc(size, 8);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED(info);
    return gSysAlloc.Alloc(size, 8);
#else
    if (UseLocks)
    {
        Lock::Locker locker(&HeapLock);
        return allocMem(size, info);
    }
    SF_MEMORYHEAP_CHECK_CURRENT_THREAD(this);
    return allocMem(size, info);
#endif
}

//------------------------------------------------------------------------
void* MemoryHeapPT::Alloc(UPInt size, UPInt align, const AllocInfo* info)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED(info);
    return _aligned_malloc(size, align);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED(info);
    return gSysAlloc.Alloc(size, align);
#else
    if (UseLocks)
    {
        Lock::Locker locker(&HeapLock);
        return allocMem(size, align, info);
    }
    SF_MEMORYHEAP_CHECK_CURRENT_THREAD(this);
    return allocMem(size, align, info);
#endif
}

//------------------------------------------------------------------------
void* MemoryHeapPT::Realloc(void* oldPtr, UPInt newSize)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    return _aligned_realloc(oldPtr, newSize, 8);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    if (oldPtr)
        CheckRead(*(int*)oldPtr);
    return gSysAlloc.Realloc(oldPtr, newSize);
#else
    // Realloc cannot work as Alloc in case oldPtr is 0. It's done in 
    // order not to pass a lot of parameters, such as DebugInfo, alignment, 
    // parent allocation, etc.
    //------------------------
    SF_HEAP_ASSERT(oldPtr != 0);

    HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(oldPtr));
    SF_HEAP_ASSERT(seg && seg->pHeap);
    MemoryHeapPT* heap = seg->pHeap;
    if (heap->UseLocks)
    {
        Lock::Locker locker(&heap->HeapLock);
        return heap->reallocMem(seg, oldPtr, newSize);
    }
    SF_MEMORYHEAP_CHECK_CURRENT_THREAD(heap);
    return heap->reallocMem(seg, oldPtr, newSize);
#endif
}

//------------------------------------------------------------------------
void  MemoryHeapPT::Free(void* ptr)
{
//UltimateCheck(); // DBG
#ifdef SF_MEMORY_FORCE_MALLOC
    _aligned_free(ptr);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    if (ptr)
        CheckRead(*(int*)ptr);
    gSysAlloc.Free(ptr);
#else

// DBG Simulate memory leaks
//static int c; if(++c % 500 == 0) 
//return; 
//----------------------

    if (ptr)
    {
        HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(ptr));
        SF_HEAP_ASSERT(seg && seg->pHeap);
        MemoryHeapPT* heap = seg->pHeap;

        if (heap->UseLocks)
        {
            {
                Lock::Locker locker(&heap->HeapLock);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
                UPInt usable = heap->pEngine->GetUsableSize(ptr);
                heap->pDebugStorage->RemoveAlloc(UPInt(ptr), usable);
#ifdef SF_MEMORY_MEMSET_ALL
                memset(ptr, 0xFE, usable);
#endif //SF_MEMORY_MEMSET_ALL
#endif //SF_MEMORY_ENABLE_DEBUG_INFO
                heap->pEngine->Free(seg, ptr);
#ifdef SF_MEMORY_TRACE_ALL
                HeapPT::GlobalRoot->OnFree(heap, ptr);
#endif
            }
            // Release is performed outside of the locker scope to 
            // avoid accessing released heap and the lock respectively.
            // This check is OK with threading because "ReleaseOnFree"
            // always takes place with the last Free request.
            if (ptr == heap->pAutoRelease)
            {
                heap->Release();
            }
        }
        else
        {
            SF_MEMORYHEAP_CHECK_CURRENT_THREAD(heap);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            UPInt usable = heap->pEngine->GetUsableSize(ptr);
            heap->pDebugStorage->RemoveAlloc(UPInt(ptr), usable);
#ifdef SF_MEMORY_MEMSET_ALL
            memset(ptr, 0xFE, usable);
#endif //SF_MEMORY_MEMSET_ALL
#endif //SF_MEMORY_ENABLE_DEBUG_INFO
            heap->pEngine->Free(seg, ptr);
#ifdef SF_MEMORY_TRACE_ALL
            HeapPT::GlobalRoot->OnFree(heap, ptr);
#endif
            if (ptr == heap->pAutoRelease)
            {
                heap->Release();
            }
        }
    }
#endif // SF_MEMORY_FORCE_MALLOC
}


//------------------------------------------------------------------------
void* MemoryHeapPT::AllocAutoHeap(const void *thisPtr, UPInt size,
                                  const AllocInfo* info)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED2(thisPtr, info);
    return _aligned_malloc(size, 8);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED2(thisPtr, info);
    CheckRead(*(int*)thisPtr);
    return gSysAlloc.Alloc(size, 8);
#else
    HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(thisPtr));
    SF_HEAP_ASSERT(seg && seg->pHeap);
    MemoryHeapPT* heap = seg->pHeap;
    if (heap->UseLocks)
    {
        Lock::Locker locker(&heap->HeapLock);
        return heap->allocMem(thisPtr, size, info);
    }
    SF_MEMORYHEAP_CHECK_CURRENT_THREAD(heap);
    return heap->allocMem(thisPtr, size, info);
#endif // SF_MEMORY_FORCE_MALLOC
}


//------------------------------------------------------------------------
void* MemoryHeapPT::AllocAutoHeap(const void *thisPtr, UPInt size, UPInt align,
                                  const AllocInfo* info)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED2(thisPtr, info);
    return _aligned_malloc(size, align);
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED2(thisPtr, info);
    CheckRead(*(int*)thisPtr);
    return gSysAlloc.Alloc(size, align);
#else
    HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(thisPtr));
    SF_HEAP_ASSERT(seg && seg->pHeap);
    MemoryHeapPT* heap = seg->pHeap;
    if (heap->UseLocks)
    {
        Lock::Locker locker(&heap->HeapLock);
        return heap->allocMem(thisPtr, size, align, info);
    }
    SF_MEMORYHEAP_CHECK_CURRENT_THREAD(heap);
    return heap->allocMem(thisPtr, size, align, info);
#endif // SF_MEMORY_FORCE_MALLOC
}


//------------------------------------------------------------------------
MemoryHeap* MemoryHeapPT::GetAllocHeap(const void *thisPtr)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED(thisPtr);
    return Memory::pGlobalHeap;
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED(thisPtr);
    CheckRead(*(int*)thisPtr);
    return Memory::pGlobalHeap;
#else
    HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(thisPtr));
    SF_HEAP_ASSERT(seg && seg->pHeap);
    return seg->pHeap;
#endif // SF_MEMORY_FORCE_MALLOC
}

//------------------------------------------------------------------------
UPInt MemoryHeapPT::GetUsableSize(const void* ptr)
{
#ifdef SF_MEMORY_FORCE_MALLOC
    SF_UNUSED(ptr);
    return 0;
#elif defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED(ptr);
    CheckRead(*(int*)ptr);
    return 0;
#else
    HeapSegment* seg = HeapPT::GlobalPageTable->GetSegment(UPInt(ptr));
    SF_HEAP_ASSERT(seg && seg->pHeap);
    MemoryHeapPT* heap = seg->pHeap;
    if (heap->UseLocks)
    {
        Lock::Locker locker(&heap->HeapLock);
        return heap->pEngine->GetUsableSize(seg, ptr);
    }
    return heap->pEngine->GetUsableSize(seg, ptr);
#endif // SF_MEMORY_FORCE_MALLOC
}


//------------------------------------------------------------------------
void MemoryHeapPT::releaseCachedMem()
{
    Lock::Locker locker(&HeapLock);
    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        heap->releaseCachedMem();
        heap = heap->pNext;
    }
    pEngine->ReleaseCachedMem();
}

//------------------------------------------------------------------------
void* MemoryHeapPT::AllocSysDirect(UPInt size)
{
    return HeapPT::GlobalRoot->AllocSysDirect(size);
}
void MemoryHeapPT::FreeSysDirect(void* ptr, UPInt size)
{
    HeapPT::GlobalRoot->FreeSysDirect(ptr, size);
}

//------------------------------------------------------------------------
bool MemoryHeapPT::GetStats(StatBag* bag)
{
#if defined(SF_MEMORY_FORCE_MALLOC) || defined(SF_MEMORY_FORCE_SYSALLOC)
    SF_UNUSED(bag);
    return true;
#else
    Lock::Locker locker(&HeapLock);

    UPInt footprint = pEngine->GetFootprint();
    
    bag->AddStat(StatHeap_LocalFootprint,   CounterStat(footprint));
    bag->AddStat(StatHeap_LocalUsedSpace,   CounterStat(pEngine->GetUsedSpace()));
    bag->AddStat(StatHeap_Granularity,      CounterStat(Info.Desc.Granularity));
    bag->AddStat(StatHeap_Reserve,          CounterStat(Info.Desc.Reserve));

    HeapPT::HeapOtherStats otherStats;
    pEngine->GetHeapOtherStats(&otherStats);

    otherStats.Bookkeeping += SelfSize;

    bag->AddStat(StatHeap_Bookkeeping,        CounterStat(otherStats.Bookkeeping));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    bag->AddStat(StatHeap_DebugInfo,          CounterStat(pDebugStorage->GetUsedSpace()));
#endif
    bag->AddStat(StatHeap_Segments,           CounterStat(otherStats.Segments));
    bag->AddStat(StatHeap_DynamicGranularity, CounterStat(otherStats.DynamicGranularity));
    bag->AddStat(StatHeap_SysDirectSpace,     CounterStat(otherStats.SysDirectSpace));

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
#endif // SF_MEMORY_FORCE_MALLOC
}

//------------------------------------------------------------------------
void MemoryHeapPT::ultimateCheck()
{
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    Lock::Locker locker(&HeapLock);
#ifndef SF_64BIT_POINTERS
    pDebugStorage->UltimateCheck();
#endif
    MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        heap->ultimateCheck();
        heap = heap->pNext;
    }
#endif
}

//------------------------------------------------------------------------
void MemoryHeapPT::VisitMem(Heap::MemVisitor* visitor, unsigned flags)
{
    Lock::Locker locker(&HeapLock);
    pEngine->VisitMem(visitor, flags);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    pDebugStorage->VisitMem(visitor, flags);
#endif
}

//------------------------------------------------------------------------
void MemoryHeapPT::VisitHeapSegments(Heap::SegVisitor* visitor) const
{
    Lock::Locker locker(&HeapLock);
    pEngine->VisitSegments(visitor);
}

//------------------------------------------------------------------------
void MemoryHeapPT::VisitRootSegments(Heap::SegVisitor* visitor)
{
    HeapPT::GlobalRoot->VisitSegments(visitor);
}

//------------------------------------------------------------------------
void MemoryHeapPT::getUserDebugStats(RootStats* stats) const
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
void MemoryHeapPT::GetRootStats(RootStats* stats)
{
    HeapPT::GlobalRoot->GetStats(stats);
    stats->UserDebugFootprint = 0;
    stats->UserDebugUsedSpace = 0;
    Memory::pGlobalHeap->getUserDebugStats(stats);
}

//------------------------------------------------------------------------
void MemoryHeapPT::checkIntegrity() const
{
    Lock::Locker locker(&HeapLock);

    pEngine->CheckIntegrity();
    const MemoryHeap* heap = ChildHeaps.GetFirst();
    while(!ChildHeaps.IsNull(heap))
    {
        // AddRef() and Release() are locked with a Global Lock here,
        // to avoid the possibility of heap being released during this iteration.
        // If necessary, this could re rewritten with AddRef_NotZero for efficiency.
        heap->checkIntegrity();
        heap = heap->pNext;
    }
}

//------------------------------------------------------------------------
void MemoryHeapPT::SetTracer(HeapTracer* tracer) 
{
#ifdef SF_MEMORY_TRACE_ALL
    HeapPT::GlobalRoot->pTracer = tracer;
#else
    SF_UNUSED(tracer);
#endif
}

//------------------------------------------------------------------------

// Implementation of SysAllocBase based on MemoryHeapPT.
// Initializes heap system, creating and initializing GlobalHeap.
bool SysAllocPaged::initHeapEngine(const void* heapDesc)
{
    MemoryHeap::InitPT(this);
    const MemoryHeap::HeapDesc& rootHeapDesc = *(const MemoryHeap::HeapDesc*)heapDesc;
    MemoryHeap* p = MemoryHeap::CreateRootHeapPT(rootHeapDesc);
    return p != 0;
}

bool SysAllocPaged::shutdownHeapEngine()
{
    bool hasNoLeaks = MemoryHeap::ReleaseRootHeapPT();
    MemoryHeap::CleanUpPT();
    return hasNoLeaks;
}

} // Scaleform
