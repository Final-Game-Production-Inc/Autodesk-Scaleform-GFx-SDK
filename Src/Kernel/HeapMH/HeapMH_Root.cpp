/**************************************************************************

Filename    :   HeapMH_Root.cpp
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
#include "HeapMH_Root.h"
#include "HeapMH_AllocEngine.h"
#include "HeapMH_DebugInfo.h"

namespace Scaleform { namespace HeapMH {

RootMH*     GlobalRootMH = 0;
PageTableMH GlobalPageTableMH;
PageMH      GlobalEmptyPageMH;

//------------------------------------------------------------------------
#ifdef SF_MEMORY_TRACE_ALL
MemoryHeap::HeapTracer* RootMH::pTracer = 0;
#endif

//------------------------------------------------------------------------
void GetMagicHeaders(UPInt pageStart, MagicHeadersInfo* headers)
{
    headers->Header1 = 0;
    headers->Header2 = 0;
    UPInt start = (pageStart + PageMH::UnitMask) & ~UPInt(PageMH::UnitMask);
    UPInt end   = (pageStart + PageMH::PageSize) & ~UPInt(PageMH::UnitMask);
    UPInt bound = (pageStart + PageMH::PageMask) & ~UPInt(PageMH::PageMask);
    if (bound-start > PageMH::UnitSize)
    {
        headers->Header1 = (MagicHeader*)(bound - sizeof(MagicHeader));
    }
    if (end-bound > PageMH::UnitSize)
    {
        headers->Header2 = (MagicHeader*)bound;
    }
    headers->BitSet = (UInt32*)((bound-start > end-bound)? 
                                (bound - sizeof(MagicHeader) - PageMH::BitSetBytes): 
                                (bound + sizeof(MagicHeader)));
    headers->AlignedStart = (UByte*)start;
    headers->AlignedEnd   = (UByte*)end;
    headers->Bound = (UByte*)bound;
    headers->Page  = 0;
}


//------------------------------------------------------------------------
RootMH::RootMH(SysAlloc* sysAlloc) :
    pSysAlloc(sysAlloc),
//#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
//    DebugAlloc(&AllocWrapper, 
//               256,
//               Heap_DebugAllocGranularity, 
//               Heap_DebugAllocHeaderPageSize),
//#endif
    RootLock(),
    FreePages(),
    TableCount(0)
{
    GlobalEmptyPageMH.pHeap = 0;
    GlobalEmptyPageMH.Start = 0; // Theoretically there must be a 4K static block,
                                 // but 0...4K is never available on any system

    for(int i = 0; i < PageTableMH::TableSize; ++i)
    {
        GlobalPageTableMH.Entries[i].FirstPage = &GlobalEmptyPageMH;
        GlobalPageTableMH.Entries[i].SizeMask  = 0;
    }
    GlobalRootMH = this;
}

//------------------------------------------------------------------------
RootMH::~RootMH()
{
    FreeTables();
    GlobalRootMH = 0;
}

//------------------------------------------------------------------------
void RootMH::FreeTables()
{
    for(int i = 0; i < PageTableMH::TableSize; ++i)
    {
        PageMH* pool = GlobalPageTableMH.Entries[i].FirstPage;
        if (pool != &GlobalEmptyPageMH)
        {
// Took out ASSERT to avoid it happening on shutdown after memory leak report.
//#ifdef SF_BUILD_DEBUG
//            UPInt size = GlobalPageTableMH.Entries[i].SizeMask;
//            SF_ASSERT(size > 1);
//            for (UPInt j = 0; j < size; ++j)
//            {
//                // This assert indicates that the respective PageMH is still use.
//                // Expect memory leaks.
//                //-----------------------
//                //SF_ASSERT(pool[j].pHeap == 0 && pool[j].Start == 0);
//            }
//#endif
            pSysAlloc->Free(GlobalPageTableMH.Entries[i].FirstPage, 
                           (GlobalPageTableMH.Entries[i].SizeMask+1) * sizeof(PageMH),
                            sizeof(void*));
        }
        GlobalPageTableMH.Entries[i].FirstPage = &GlobalEmptyPageMH;
        GlobalPageTableMH.Entries[i].SizeMask  = 0;
    }
}

//------------------------------------------------------------------------
bool RootMH::allocPagePool()
{
    if (TableCount >= PageTableMH::TableSize)
    {
        *(int*)4 = 0; // Perform artificial crash
        return false;
    }

    UPInt tableSize = 128 << (TableCount / 16); //  0...15: 128
                                                // 16...31: 256
                                                // 32...48: 512 ...etc
    PageMH* pool = (PageMH*)pSysAlloc->Alloc(tableSize * sizeof(PageMH), sizeof(void*));
    if (pool == 0)
        return false;

    GlobalPageTableMH.Entries[TableCount].FirstPage = pool;
    GlobalPageTableMH.Entries[TableCount].SizeMask  = tableSize-1;
    for (UPInt i = 0; i < tableSize; ++i)
    {
        pool->pHeap = 0;
        pool->Start = 0;
        FreePages.PushBack(pool);
        ++pool;
    }
    ++TableCount;
    return true;
}

//------------------------------------------------------------------------
PageMH* RootMH::AllocPage(MemoryHeapMH* heap)
{
    if (FreePages.IsEmpty())
        if (!allocPagePool())
            return 0;

    SF_ASSERT(!FreePages.IsEmpty());

    PageMH* page = FreePages.GetFirst();
    page->Start = (UByte*)pSysAlloc->Alloc(PageMH::PageSize, sizeof(void*));
    if (page->Start == 0)
    {
        page->pHeap = 0;
        return 0;
    }
    List<PageMH>::Remove(page);
    page->pHeap = heap;
    setMagic(page->Start, MagicHeader::MagicValue);
    return page;
}

//------------------------------------------------------------------------
void RootMH::FreePage(PageMH* page)
{
    clearMagic(page->Start); // To reduce the number of collisions
    UByte* p = page->Start;
    page->Start = 0;
    page->pHeap = 0;
    pSysAlloc->Free(p, PageMH::PageSize, sizeof(void*));
    FreePages.PushBack(page);
}

//------------------------------------------------------------------------
UInt32 RootMH::GetPageIndex(const PageMH* page) const
{
    for (unsigned i = 0; i < TableCount; ++i)
    {
        const PageTableMH::Level0Entry& l1Table = GlobalPageTableMH.Entries[i];
        SF_ASSERT(l1Table.FirstPage != &GlobalEmptyPageMH);
        UPInt idx1 = page - l1Table.FirstPage;
        if (idx1 <= UPInt(l1Table.SizeMask))
        {
            return UInt32((idx1 << PageTableMH::Index1Shift) | i);
        }
    }
    return UInt32(~0U);
}

//------------------------------------------------------------------------
void RootMH::setMagic(UByte* pageStart, unsigned magicValue)
{
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(pageStart), &headers);
    if (headers.Header1) headers.Header1->Magic = (UInt16)magicValue;
    if (headers.Header2) headers.Header2->Magic = (UInt16)magicValue;
}

//------------------------------------------------------------------------
void RootMH::clearMagic(UByte* pageStart)
{
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(pageStart), &headers);
    if (headers.Header1) 
    {
        headers.Header1->Magic = 0;
        headers.Header1->UseCount = 0;
        headers.Header1->Index = 0;
        headers.Header1->DebugHeader = 0; 
    }
    if (headers.Header2) 
    {
        headers.Header2->Magic = 0;
        headers.Header2->UseCount = 0;
        headers.Header2->Index = 0;
        headers.Header2->DebugHeader = 0; 
    }
}

//------------------------------------------------------------------------
PageMH* RootMH::ResolveAddress(UPInt addr) const
{
    const MagicHeader* header;

    header = (const MagicHeader*)(addr & ~UPInt(PageMH::PageMask));
    if (header->Magic == MagicHeader::MagicValue)
    {      
        const PageTableMH::Level0Entry& l0e = GlobalPageTableMH.Entries[header->GetIndex0()];
        PageMH* page = l0e.FirstPage + (header->GetIndex1() & l0e.SizeMask);
        if ((addr - UPInt(page->Start)) < PageMH::PageSize)
        {
            return page;
        }
    }

    // No match on left
    header = (const MagicHeader*)((addr & ~UPInt(PageMH::PageMask)) + PageMH::PageSize - sizeof(MagicHeader));
    if (header->Magic == MagicHeader::MagicValue)
    {
        const PageTableMH::Level0Entry& l0e = GlobalPageTableMH.Entries[header->GetIndex0()];
        PageMH* page = l0e.FirstPage + (header->GetIndex1() & l0e.SizeMask);
        if ((addr - UPInt(page->Start)) < PageMH::PageSize)
        {
            return page;
        }
    }
    return 0;
}


//------------------------------------------------------------------------
MemoryHeapMH* RootMH::CreateHeap(const char*  name, 
                                 MemoryHeapMH* parent,
                                 const MemoryHeap::HeapDesc& desc)
{
    // Locked by Root::Lock in MemoryHeap::CreateHeap
    if (GetSysAlloc() == 0)
    {
        SF_DEBUG_MESSAGE(1, "CreateHeap: Memory System is not initialized");
        SF_ASSERT(0);
    }

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    UPInt debugStorageSize = sizeof(DebugStorageMH);
#else
    UPInt debugStorageSize = 0;
#endif

    UPInt selfSize = sizeof(MemoryHeapMH) + 
                     sizeof(AllocEngineMH) + 
                     debugStorageSize +
                     strlen(name) + 1;

    selfSize = (selfSize + PageMH::UnitMask) & ~UPInt(PageMH::UnitMask);

    UByte* heapBuf = (UByte*)pSysAlloc->Alloc(selfSize, sizeof(void*));
    if (heapBuf == 0)
    {
        return 0;
    }

    MemoryHeapMH* heap = ::new(heapBuf) MemoryHeapMH;

    AllocEngineMH* engine = ::new (heapBuf + sizeof(MemoryHeapMH))
        AllocEngineMH(pSysAlloc,
                      heap,
                      desc.MinAlign, 
                      desc.Limit);

    if (!engine->IsValid())
    {
        pSysAlloc->Free(heapBuf, selfSize, sizeof(void*));
        return 0;
    }

    heap->SelfSize          = selfSize;
    heap->RefCount          = 1;
    heap->pAutoRelease      = 0;
    heap->Info.Desc         = desc;
    heap->Info.pParent      = parent;
    heap->Info.pName        = (char*)heapBuf + 
                                     sizeof(MemoryHeapMH) + 
                                     sizeof(AllocEngineMH) + 
                                     debugStorageSize;
    heap->UseLocks          = (desc.Flags & MemoryHeap::Heap_ThreadUnsafe) == 0;
    heap->TrackDebugInfo    = (desc.Flags & MemoryHeap::Heap_NoDebugInfo)  == 0;
    heap->pEngine           = engine;

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    heap->pDebugStorage = ::new(heapBuf + sizeof(MemoryHeapMH) + sizeof(AllocEngineMH))
        DebugStorageMH(GlobalRootMH->GetSysAlloc(), GlobalRootMH->GetLock());
#endif
    memcpy(heap->Info.pName, name, strlen(name) + 1);

#ifdef SF_MEMORY_TRACE_ALL
    if (pTracer)
        pTracer->OnCreateHeap(heap);
#endif

    return heap;
}

//------------------------------------------------------------------------
void RootMH::DestroyHeap(MemoryHeapMH* heap)
{
    // Locked by Root::Lock in MemoryHeap::Release
#ifdef SF_MEMORY_TRACE_ALL
    if (pTracer)
        pTracer->OnDestroyHeap(heap);
#endif

    UPInt heapSize = heap->SelfSize;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    heap->pDebugStorage->FreeAll();
#endif
    heap->pEngine->FreeAll();
    heap->~MemoryHeapMH();
    pSysAlloc->Free(heap, heapSize, sizeof(void*));
}


//------------------------------------------------------------------------
#ifdef SF_MEMORY_TRACE_ALL
void RootMH::OnAlloc(const MemoryHeap* heap, UPInt size, UPInt align, unsigned sid, const void* ptr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnAlloc(heap, size, align, sid, ptr);
    }
}

void RootMH::OnRealloc(const MemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnRealloc(heap, oldPtr, newSize, newPtr);
    }
}

void RootMH::OnFree(const MemoryHeap* heap, const void* ptr)
{
    if (pTracer)
    {
        LockSafe::Locker lock(&RootLock);
        pTracer->OnFree(heap, ptr);
    }
}
#endif

}} // Scaleform::HeapMH


