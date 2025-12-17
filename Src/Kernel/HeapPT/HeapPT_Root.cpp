/**************************************************************************

Filename    :   HeapPT_Root.cpp
Content     :   Heap root used for bootstrapping and bookkeeping.
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Debug.h"
#include "HeapPT_Root.h"
#include "HeapPT_DebugInfo.h"
#include "HeapPT_AllocEngine.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

//------------------------------------------------------------------------
HeapRoot* GlobalRoot = 0;

//------------------------------------------------------------------------
SysAllocGranulator::SysAllocGranulator() : 
    pGranulator(0),
    SysDirectThreshold(0),
    MinAlign(0),
    MaxAlign(0),
    SysDirectFootprint(0)
{}

SysAllocGranulator::SysAllocGranulator(SysAllocPaged* sysAlloc) : 
    pGranulator(0),
    SysDirectThreshold(0),
    MaxHeapGranularity(0),
    MinAlign(0),
    MaxAlign(0),
    SysDirectFootprint(0)
{
    SF_COMPILER_ASSERT(sizeof(PrivateData) >= sizeof(Granulator));
    Init(sysAlloc);
}

//------------------------------------------------------------------------
void SysAllocGranulator::Init(SysAllocPaged* sysAlloc)
{
    pGranulator = ::new(PrivateData) 
                        Granulator(sysAlloc, 
                                   Heap_PageSize,
                                   Heap_PageSize, 
                                   Heap_GranulatorHeaderPageSize);
    Info i;
    memset(&i, 0, sizeof(Info));
    pGranulator->GetSysAlloc()->GetInfo(&i); // For the sake of SysDirectThreshold
    SysDirectThreshold = i.SysDirectThreshold;
    MaxHeapGranularity = i.MaxHeapGranularity;
    MinAlign = i.MinAlign;
    MaxAlign = i.MaxAlign;

    // Allow MinAlign and MaxAlign to have zero values, which means
    // "any alignment is supported".
    if (MinAlign == 0)
        MinAlign = 1;

    if (MaxAlign == 0)
        MaxAlign = UPInt(1) << (8*sizeof(UPInt)-1);
}

//------------------------------------------------------------------------
void SysAllocGranulator::GetInfo(Info* i) const
{
    i->MinAlign             = 0; // Any alignment
    i->MaxAlign             = 0; // Any alignment
    i->Granularity          = 0; // No Granularity
    i->SysDirectThreshold   = SysDirectThreshold;
    i->MaxHeapGranularity   = MaxHeapGranularity;
    i->HasRealloc           = true;
}


//------------------------------------------------------------------------
void* SysAllocGranulator::Alloc(UPInt size, UPInt alignment)
{
    void* ptr = pGranulator->Alloc(size, alignment);

    //if (Memory::pGlobalHeap)             // DBG
    //    printf("+%uK fp:%uK used:%f%%\n",
    //        size/1024, pGranulator->GetUsedSpace()/1024, 
    //        Memory::pGlobalHeap->GetTotalUsedSpace()/double(pGranulator->GetUsedSpace())*100.0);

    return ptr;
}


//------------------------------------------------------------------------
bool SysAllocGranulator::ReallocInPlace(void* oldPtr, UPInt oldSize, 
                                        UPInt newSize, UPInt alignment)
{
    return pGranulator->ReallocInPlace((UByte*)oldPtr, oldSize, 
                                       newSize, alignment);
}

//------------------------------------------------------------------------
bool SysAllocGranulator::Free(void* ptr, UPInt size, UPInt alignment)
{
    bool ret = pGranulator->Free(ptr, size, alignment);

    //// With this DBG GFx crashes at exit. The crash is expected and normal.
    //if (Memory::pGlobalHeap)             // DBG
    //    printf("-%uK fp:%uK used:%f%%\n",
    //        size/1024, pGranulator->GetUsedSpace()/1024, 
    //        Memory::pGlobalHeap->GetTotalUsedSpace()/double(pGranulator->GetUsedSpace())*100.0);

    return ret;
}


//------------------------------------------------------------------------
void* SysAllocGranulator::AllocSysDirect(UPInt size, UPInt alignment, 
                                         UPInt* actualSize, UPInt* actualAlign)
{
    UPInt pageAlignment = alignment;
    if (pageAlignment < Heap_PageSize)
        pageAlignment = Heap_PageSize;

    if (alignment < MinAlign)
        alignment = MinAlign;

    if (alignment > MaxAlign)
        alignment = MaxAlign;

    if (alignment < pageAlignment)
        size += pageAlignment;

    *actualSize  = size;
    *actualAlign = alignment;
    SysDirectFootprint += size;
    void* ptr = pGranulator->GetSysAlloc()->Alloc(size, alignment);
    return ptr;
}

//------------------------------------------------------------------------
bool SysAllocGranulator::FreeSysDirect(void* ptr, UPInt size, UPInt alignment)
{
    SysDirectFootprint -= size;
    return pGranulator->GetSysAlloc()->Free(ptr, size, alignment);
}


//------------------------------------------------------------------------
void SysAllocGranulator::VisitMem(MemVisitor* visitor) const
{ 
    pGranulator->VisitMem(visitor, 
                          MemVisitor::Cat_SysAlloc, 
                          MemVisitor::Cat_SysAllocFree);
}

//------------------------------------------------------------------------
void SysAllocGranulator::VisitSegments(SegVisitor* visitor, UPInt catSeg, UPInt catUnused) const
{
    pGranulator->VisitSegments(visitor, (unsigned)catSeg, (unsigned)catUnused);
}

//------------------------------------------------------------------------
UPInt SysAllocGranulator::GetFootprint() const { return pGranulator->GetFootprint() + SysDirectFootprint; }
UPInt SysAllocGranulator::GetUsedSpace() const { return pGranulator->GetUsedSpace() + SysDirectFootprint; }

//------------------------------------------------------------------------
UPInt SysAllocGranulator::GetBase() const { return pGranulator->GetBase(); } // DBG
UPInt SysAllocGranulator::GetSize() const { return pGranulator->GetSize(); } // DBG



//------------------------------------------------------------------------
SysAllocWrapper::SysAllocWrapper(SysAllocPaged* sysAlloc) :
    Allocator(),
    pSrcAlloc(sysAlloc),
    pSysAlloc(sysAlloc),
    SysGranularity(Heap_PageSize),
    MinAlign(1),
    UsedSpace(0)
{
    Info i;
    memset(&i, 0, sizeof(Info));
    sysAlloc->GetInfo(&i);
    if (i.Granularity)
    {
        Allocator.Init(sysAlloc);
        pSysAlloc = &Allocator;
        SysGranularity = i.Granularity;
    }
    if (MinAlign < i.MinAlign)
        MinAlign = i.MinAlign;
}

//------------------------------------------------------------------------
void SysAllocWrapper::GetInfo(Info* i) const
{
    memset(i, 0, sizeof(Info));
    pSysAlloc->GetInfo(i);
}

void* SysAllocWrapper::Alloc(UPInt size, UPInt align)
{
    UsedSpace += size;
    return pSysAlloc->Alloc(size, align);
}

bool SysAllocWrapper::Free(void* ptr, UPInt size, UPInt align)
{
    UsedSpace -= size;
    return pSysAlloc->Free(ptr, size, align);
}

bool SysAllocWrapper::ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
{
    UsedSpace -= oldSize;
    UsedSpace += newSize;
    return pSysAlloc->ReallocInPlace(oldPtr, oldSize, newSize, align);
}

void* SysAllocWrapper::AllocSysDirect(UPInt size, UPInt alignment, UPInt* actualSize, UPInt* actualAlign)
{
    UsedSpace += size;
    return pSysAlloc->AllocSysDirect(size, alignment, actualSize, actualAlign);
}

bool  SysAllocWrapper::FreeSysDirect(void* ptr, UPInt size, UPInt alignment)
{
    UsedSpace -= size;
    return pSysAlloc->FreeSysDirect(ptr, size, alignment);
}

void SysAllocWrapper::VisitMem(Heap::MemVisitor* visitor) const
{
    pSysAlloc->VisitMem(visitor);
}

void SysAllocWrapper::VisitSegments(Heap::SegVisitor* visitor, 
                                    UPInt catSeg, UPInt catUnused) const
{
    pSysAlloc->VisitSegments(visitor, catSeg, catUnused);
}

//------------------------------------------------------------------------
void* SysAllocWrapper::AllocSysDirect(UPInt size)
{
    size = (size + SysGranularity - 1) / SysGranularity * SysGranularity;
    UsedSpace += size;
    return pSrcAlloc->Alloc(size, MinAlign);
}

//------------------------------------------------------------------------
void SysAllocWrapper::FreeSysDirect(void* ptr, UPInt size)
{
    size = (size + SysGranularity - 1) / SysGranularity * SysGranularity;
    UsedSpace -= size;
    pSrcAlloc->Free(ptr, size, MinAlign);
}

//------------------------------------------------------------------------
UPInt SysAllocWrapper::GetFootprint() const { return pSysAlloc->GetFootprint(); }
UPInt SysAllocWrapper::GetUsedSpace() const { return UsedSpace; }

//------------------------------------------------------------------------
#ifdef SF_MEMORY_TRACE_ALL
MemoryHeap::HeapTracer* HeapRoot::pTracer = 0;
#endif


//------------------------------------------------------------------------
HeapRoot::HeapRoot(SysAllocPaged* sysAlloc) :
    AllocWrapper(sysAlloc),
    AllocStarter(&AllocWrapper, Heap_StarterGranularity, Heap_StarterHeaderPageSize),
    AllocBookkeeper(&AllocWrapper, Heap_BookkeeperGranularity),
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    DebugAlloc(&AllocWrapper, 
               256,
               Heap_DebugAllocGranularity, 
               Heap_DebugAllocHeaderPageSize),
#endif
    RootLock(),
    pArenas(0),
    NumArenas(0)
{
    HeapPT::GlobalPageTable->SetStarter(&AllocStarter);
    GlobalRoot = this;
}

HeapRoot::~HeapRoot()
{
}

// *** Memory Arenas
//
//------------------------------------------------------------------------
void HeapRoot::CreateArena(UPInt arena, SysAllocPaged* sysAlloc)
{
    SF_HEAP_ASSERT(arena);
    LockSafe::Locker lock(&RootLock);
    if (NumArenas < arena)
    {
        UPInt newNumArenas = (arena + 15) & ~15;
        SysAllocPaged** newArenas = (SysAllocPaged**)AllocBookkeeper.Alloc(newNumArenas * sizeof(void*));
        SF_ASSERT(newArenas);
        memset(newArenas, 0, newNumArenas * sizeof(void*));
        if (pArenas) 
        {
            memcpy(newArenas, pArenas, NumArenas * sizeof(void*));
            AllocBookkeeper.Free(pArenas, NumArenas * sizeof(void*));
        }
        pArenas = newArenas;
        NumArenas = newNumArenas;
    }
    SF_HEAP_ASSERT(arena <= NumArenas);
    SF_HEAP_ASSERT(pArenas[arena-1] == 0);
    pArenas[arena-1] = (SysAllocPaged*)AllocBookkeeper.Alloc(sizeof(SysAllocWrapper));
    SF_ASSERT(pArenas[arena-1]);
    pArenas[arena-1] = ::new(pArenas[arena-1]) SysAllocWrapper(sysAlloc);
}

//------------------------------------------------------------------------
int HeapRoot::DestroyArena(UPInt arena)
{
    SF_HEAP_ASSERT(arena);
    SF_HEAP_ASSERT(pArenas[arena-1]);
    LockSafe::Locker lock(&RootLock);
    bool empty = ArenaIsEmpty(arena);
    if (empty)
    {
        pArenas[arena-1]->~SysAllocPaged();
        AllocBookkeeper.Free(pArenas[arena-1], sizeof(SysAllocWrapper));
        pArenas[arena-1] = 0;
    }
    else
    {
        SF_DEBUG_MESSAGE1(!empty, "MemoryHeap Arena %ld is not empty!", (long)arena);
#ifdef SF_BUILD_DEBUG
        SF_HEAP_ASSERT(empty);
        return 0;
#else
        return *(int*)0; // "Safe crash" in release mode
#endif
    }
    return 0;
}

//------------------------------------------------------------------------
void HeapRoot::DestroyAllArenas()
{
    LockSafe::Locker lock(&RootLock);
    if (pArenas)
    {
        for (UPInt i = NumArenas; i; --i)
        {
            if (pArenas[i-1])
                DestroyArena(i);
        }
        AllocBookkeeper.Free(pArenas, NumArenas * sizeof(void*));
        pArenas = 0;
        NumArenas = 0;
    }
}

//------------------------------------------------------------------------
bool HeapRoot::ArenaIsEmpty(UPInt arena)
{
    LockSafe::Locker lock(&RootLock);
    return GetSysAlloc(arena)->GetUsedSpace() == 0;
}

//------------------------------------------------------------------------
SysAllocPaged* HeapRoot::GetSysAlloc(UPInt arena) 
{
    LockSafe::Locker lock(&RootLock);
    if(arena)
    {
        SF_HEAP_ASSERT(arena <= NumArenas);
        SF_HEAP_ASSERT(pArenas[arena-1]);
        return pArenas[arena-1];
    }
    return &AllocWrapper; 
}


//------------------------------------------------------------------------
MemoryHeapPT* HeapRoot::CreateHeap(const char* name, 
                                   MemoryHeapPT* parent,
                                   const MemoryHeap::HeapDesc& desc)
{
    // Locked by Root::Lock in MemoryHeap::CreateHeap
    if (GetSysAlloc(0) == 0)
    {
        SF_DEBUG_MESSAGE(1, "CreateHeap: Memory System is not initialized");
        SF_HEAP_ASSERT(0);
    }

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt debugStorageSize = sizeof(DebugStorage);
#else
    UPInt debugStorageSize = 0;
#endif

    UPInt selfSize = sizeof(MemoryHeapPT) + 
                     sizeof(AllocEngine) + 
                     debugStorageSize +
                     strlen(name) + 1;

    selfSize = (selfSize + Heap_MinMask) & ~UPInt(Heap_MinMask);

    UByte* heapBuf = (UByte*)AllocBookkeeper.Alloc(selfSize);
    if (heapBuf == 0)
    {
        return 0;
    }

    unsigned engineFlags = 0;

    if (desc.Flags & MemoryHeap::Heap_FastTinyBlocks) 
        engineFlags |= Heap_AllowTinyBlocks;

    if ((desc.Flags & MemoryHeap::Heap_FixedGranularity) == 0) 
        engineFlags |= Heap_AllowDynaSize;

    MemoryHeapPT* heap = ::new(heapBuf) MemoryHeapPT;

    AllocEngine* engine = ::new (heapBuf + sizeof(MemoryHeapPT))
        AllocEngine(GetSysAlloc(desc.Arena),
                    heap,
                    engineFlags, 
                    desc.MinAlign, 
                    desc.Granularity, 
#ifdef SF_MEMORY_FORCE_MALLOC
                         0,
#else
                         desc.Reserve, 
#endif
                         desc.Threshold,
                         desc.Limit);

    if (!engine->IsValid())
    {
        AllocBookkeeper.Free(heapBuf, selfSize);
        return 0;
    }

    heap->SelfSize          = selfSize;
    heap->RefCount          = 1;
    heap->pAutoRelease      = 0;
    heap->Info.Desc         = desc;
    heap->Info.pParent      = parent;
    heap->Info.pName        = (char*)heapBuf + 
                                     sizeof(MemoryHeapPT) + 
                                     sizeof(AllocEngine) + 
                                     debugStorageSize;
    heap->UseLocks          = (desc.Flags & MemoryHeap::Heap_ThreadUnsafe) == 0;
    heap->TrackDebugInfo    = (desc.Flags & MemoryHeap::Heap_NoDebugInfo)  == 0;
    heap->pEngine           = engine;

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    heap->pDebugStorage = ::new(heapBuf + sizeof(MemoryHeapPT) + sizeof(AllocEngine))
        DebugStorage(GlobalRoot->GetDebugAlloc(), GlobalRoot->GetLock()),
#endif
    memcpy(heap->Info.pName, name, strlen(name) + 1);

#ifdef SF_MEMORY_TRACE_ALL
    if (pTracer)
        pTracer->OnCreateHeap(heap);
#endif

    return heap;
}


//------------------------------------------------------------------------
void HeapRoot::DestroyHeap(MemoryHeapPT* heap)
{
    // Locked by Root::Lock in MemoryHeap::Release
#ifdef SF_MEMORY_TRACE_ALL
    if (pTracer)
        pTracer->OnDestroyHeap(heap);
#endif

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    heap->pDebugStorage->FreeAll();
#endif
    heap->pEngine->FreeAll();
    heap->~MemoryHeapPT();
    AllocBookkeeper.Free(heap, heap->SelfSize);
}


//------------------------------------------------------------------------
#ifdef SF_MEMORY_TRACE_ALL
void HeapRoot::OnAlloc(const MemoryHeap* heap, UPInt size, UPInt align, unsigned sid, const void* ptr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnAlloc(heap, size, align, sid, ptr);
    }
}

void HeapRoot::OnRealloc(const MemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnRealloc(heap, oldPtr, newSize, newPtr);
    }
}

void HeapRoot::OnFree(const MemoryHeap* heap, const void* ptr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnFree(heap, ptr);
    }
}
#endif


//------------------------------------------------------------------------
void HeapRoot::VisitSegments(Heap::SegVisitor* visitor) const
{
    LockSafe::Locker lock(&RootLock);
    AllocWrapper.VisitSegments(visitor, 
                               Heap::SegVisitor::Seg_SysMem, 
                               Heap::SegVisitor::Seg_SysMemUnused);
    AllocStarter.VisitSegments(visitor);
    AllocBookkeeper.VisitSegments(visitor);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    DebugAlloc.VisitSegments(visitor, 
                             Heap::SegVisitor::Seg_DebugInfo,
                             Heap::SegVisitor::Seg_DebugInfoUnused);
#endif
}


//------------------------------------------------------------------------
void HeapRoot::GetStats(MemoryHeap::RootStats* stats) const
{
    LockSafe::Locker lock(&RootLock);
    stats->SysMemFootprint      = AllocWrapper.GetFootprint();
    stats->SysMemUsedSpace      = AllocWrapper.GetUsedSpace();
    stats->PageMapFootprint     = AllocStarter.GetFootprint();
    stats->PageMapUsedSpace     = AllocStarter.GetUsedSpace();
    stats->BookkeepingFootprint = AllocBookkeeper.GetFootprint();
    stats->BookkeepingUsedSpace = AllocBookkeeper.GetUsedSpace();
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    stats->DebugInfoFootprint   = DebugAlloc.GetFootprint();
    stats->DebugInfoUsedSpace   = DebugAlloc.GetUsedSpace();
#else
    stats->DebugInfoFootprint   = 0;
    stats->DebugInfoUsedSpace   = 0;
#endif
}


//------------------------------------------------------------------------
void* HeapRoot::AllocSysDirect(UPInt size)
{
    LockSafe::Locker lock(&RootLock);
    return AllocWrapper.AllocSysDirect(size);
}
void HeapRoot::FreeSysDirect(void* ptr, UPInt size)
{
    LockSafe::Locker lock(&RootLock);
    AllocWrapper.FreeSysDirect(ptr, size);
}

}} // Scaleform::Heap


