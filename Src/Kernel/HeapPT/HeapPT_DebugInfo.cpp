/**************************************************************************

Filename    :   HeapPT_DebugInfo.cpp
Content     :   Debug and statistics implementation.
Created     :   July 14, 2008
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Alg.h"
#include "../SF_Debug.h"
#include "../SF_Stats.h"
#include "HeapPT_DebugInfo.h"
#include "HeapPT_AllocEngine.h"

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

namespace Scaleform { namespace HeapPT {


#ifdef SF_64BIT_POINTERS

// For x64 the debug info is stored in the radix tree, not using the page table
//------------------------------------------------------------------------


DebugStorage::DebugStorage(Granulator* alloc, LockSafe* rootLocker) : 
    pAllocator(alloc), 
    pRootLocker(rootLocker)
{}


//------------------------------------------------------------------------
bool DebugStorage::allocDataPool()
{
    void* poolBuf = 0; 
    {
        LockSafe::Locker rl(pRootLocker);
        poolBuf = pAllocator->Alloc(PoolSize, Heap_MinSize);
    }

    if (poolBuf)
    {
        DataPoolType* pool = ::new (poolBuf) DataPoolType;
        DataPools.PushFront(pool);

        UPInt size = pool->GetSize(PoolSize);
        for(UPInt i = 0; i < size; ++i)
        {
            DebugData* data = pool->GetElement(i);
            data->pDataPool = pool;
            FreeDataList.PushBack(data);
        }
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void DebugStorage::freeDataPool(DataPoolType* pool)
{
    UPInt size = pool->GetSize(PoolSize);
    for(UPInt i = 0; i < size; ++i)
    {
        FreeDataList.Remove(pool->GetElement(i));
    }
    DataPools.Remove(pool);
    {
        LockSafe::Locker rl(pRootLocker);
        pAllocator->Free(pool, PoolSize, Heap_MinSize);
    }
}


//------------------------------------------------------------------------
DebugData* DebugStorage::allocDebugData()
{
    if (FreeDataList.IsEmpty())
    {
        if (!allocDataPool())
            return 0;
    }
    SF_HEAP_ASSERT(!FreeDataList.IsEmpty());

    DebugData* data = FreeDataList.GetFirst();
    FreeDataList.Remove(data);
    data->Clear();
    data->pDataPool->UseCount++;
    return data;
}


//------------------------------------------------------------------------
void DebugStorage::freeDebugData(DebugData* data)
{
    FreeDataList.PushFront(data);
    if (--data->pDataPool->UseCount == 0)
    {
        freeDataPool(data->pDataPool);
    }
}


//------------------------------------------------------------------------
void DebugStorage::fillDataTail(DebugData* data, UPInt usable)
{
    UByte* p = (UByte*)data->Address;
    for (UPInt i = data->Size; i < usable; ++i)
    {
        p[i] = UByte(129 + i);
    }
}

//------------------------------------------------------------------------
void DebugStorage::checkDataTail(DebugData* data, UPInt usable)
{
    const UByte* p = (const UByte*)data->Address;
    for (UPInt i = data->Size; i < usable; ++i)
    {
        if (p[i] != UByte(129 + i))
        {
            reportViolation(data, " Corruption");
            SF_HEAP_ASSERT(0);
        }
    }
}

//------------------------------------------------------------------------
void DebugStorage::CheckDataTail(UPInt addr, UPInt usable)
{
    DebugData* data = (DebugData*)AllocTree.FindEqual(addr);
    SF_HEAP_ASSERT(data);
    checkDataTail(data, usable);
}

//------------------------------------------------------------------------
unsigned DebugStorage::GetStatId(UPInt parentAddr, const AllocInfo* info)
{
    AllocInfo i2 = info ? *info : AllocInfo();
    if (i2.StatId != Stat_Default_Mem)
    {
        return i2.StatId;
    }
    DebugData* parent = (DebugData*)AllocTree.FindEqual(parentAddr);
    if (parent == NULL)
    {
        return Stat_Default_Mem;
    }
    return parent->Info.StatId;
}

//------------------------------------------------------------------------
bool DebugStorage::AddAlloc(UPInt parentAddr, bool autoHeap, UPInt thisAddr, 
                            UPInt size, UPInt usable, const AllocInfo* info)
{
    if (thisAddr == 0)
    {
        return true;
    }

    DebugData* data = allocDebugData();
    if (data)
    {
        data->RefCount  = 1;
        data->Address   = thisAddr;
        data->Size      = size;
        data->Info      = info ? *info : AllocInfo();

        AllocTree.Insert(data);
        fillDataTail(data, usable);

        if (parentAddr)
        {
            DebugData* upper = (DebugData*)AllocTree.FindLeEq(parentAddr);
            if (autoHeap)
            {
                // The upper allocation must exist and this allocation 
                // must be within the space of the upper one.
                SF_HEAP_ASSERT(upper);
                SF_HEAP_ASSERT(parentAddr < upper->Address + upper->Size);
            }
            else
            {
                if (upper == 0 || parentAddr >= upper->Address + upper->Size)
                {
                    return true;
                }
            }

            if (data->Info.StatId == Stat_Default_Mem)
            {
                data->Info.StatId = upper->Info.StatId;
            }

            // Prevent from creating too long parent chains in 
            // the following scenario:
            //
            //    p1 = heap.Alloc(. . .);
            //    for(. . .)
            //    {
            //        p2 = AllocAutoHeap(p1, . . .);
            //        Free(p1);
            //        p1 = p2;
            //    }
            //
            // When the chain gets longer than ChainLimit, just 
            // connect it to the grandparent, interrupting the chain. 
            // The condition "GHeapDebugNode::ChainLimit+1" means that 
            // we allow one more element in the chain to be able to detect 
            // whether or not the chain was interrupted: 
            // Interruption has occurred if ChainCount > ChainLimit.
            //-----------------------------
            if (upper->UpperCount > UpperChainLimit+1)
            {
                upper = upper->pUpperThis;
                SF_HEAP_ASSERT(upper);
            }
            data->pUpperThis = upper;
            data->UpperCount = upper->UpperCount + 1;
            upper->RefCount++;
        }
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void DebugStorage::UnlinkAlloc(UPInt addr, DebugDataPtr* ptr)
{
    SF_UNUSED2(addr, ptr);
}

//------------------------------------------------------------------------
void DebugStorage::RelinkAlloc(DebugDataPtr* ptr, 
                               UPInt oldAddr, UPInt newAddr, UPInt newSize, UPInt usable)
{
    SF_UNUSED(ptr);
    DebugData* data = (DebugData*)AllocTree.FindEqual(oldAddr);
    SF_HEAP_ASSERT(data);
    data->Size = newSize;
    if (oldAddr != newAddr)
    {
        AllocTree.Remove(data);
        data->Address = newAddr;
        AllocTree.Insert(data);
    }
    fillDataTail(data, usable);
}


//------------------------------------------------------------------------
void DebugStorage::RemoveAlloc(UPInt addr, UPInt usable)
{
    if (addr)
    {
        DebugData* data = (DebugData*)AllocTree.FindEqual(addr);
        SF_HEAP_ASSERT(data);
        checkDataTail(data, usable);
        AllocTree.Remove(data);

        while (data && --data->RefCount == 0)
        {
            DebugData* upper = data->pUpperThis;
            freeDebugData(data);
            data = upper;
        }
    }
}

//------------------------------------------------------------------------
void DebugStorage::FreeAll()
{
    AllocTree.Clear();
    while (!DataPools.IsEmpty())
    {
        DataPoolType* pool = DataPools.GetFirst();
        DataPools.Remove(pool);
        pAllocator->Free(pool, PoolSize, Heap_MinSize);
    }
    FreeDataList.Clear();
}


//------------------------------------------------------------------------
#ifdef SF_ENABLE_STATS
void DebugStorage::getStatNode(const DebugData* node, 
                               AllocEngine* allocator, 
                               StatBag* bag,
                               unsigned& count) const
{
    if (node)
    {
        getStatNode(node->pPrev, allocator, bag, count);
        bag->IncrementMemoryStat(node->Info.StatId,
                                 node->Size,
                                 allocator->GetUsableSize((void*)node->Address));
        ++count;
        getStatNode(node->pNext, allocator, bag, count);
    }
}
#endif

//------------------------------------------------------------------------
void DebugStorage::GetStats(AllocEngine* allocator, StatBag* bag) const
{
#ifdef SF_ENABLE_STATS
    unsigned count = 0;
    // Must be locked in GMemoryHeap.
    getStatNode(AllocTree.Root, allocator, bag, count);
#else
    SF_UNUSED2(allocator, bag);
#endif
}

//------------------------------------------------------------------------
UPInt DebugStorage::GetUsedSpace() const
{
    UPInt usedSpace = 0;
    const DataPoolType* data = DataPools.GetFirst();
    while(!DataPools.IsNull(data))
    {
        usedSpace += PoolSize;
        data = data->pNext;
    }
    return usedSpace;
}

//------------------------------------------------------------------------
void DebugStorage::visitTree(const DebugData* node, 
                             MemVisitor* visitor, 
                             const HeapSegment* seg) const
{
    if (node)
    {
        visitTree(node->pPrev, visitor, seg);
        visitor->Visit(seg, UPInt(node), sizeof(DebugData), 
                       MemVisitor::Cat_DebugDataBusy);
        visitTree(node->pNext, visitor, seg);
    }
}

//------------------------------------------------------------------------
void DebugStorage::VisitMem(MemVisitor* visitor, unsigned flags) const
{
    pAllocator->VisitMem(visitor, 
                         MemVisitor::Cat_DebugInfo,
                         MemVisitor::Cat_DebugInfoFree);

    if (flags & MemVisitor::VF_DebugInfoDetailed)
    {
        HeapSegment seg; // Fake segment
        seg.SelfSize  = 0;
        seg.SegType   = Heap_SegmentUndefined;
        seg.Alignment = 0;
        seg.UseCount  = 0;
        seg.pHeap     = 0;
        seg.DataSize  = 0;
        seg.pData     = 0;

        visitMem(DataPools,    PoolSize,          visitor, MemVisitor::Cat_DebugDataPool, &seg);
        visitMem(FreeDataList, sizeof(DebugData), visitor, MemVisitor::Cat_DebugDataFree, &seg);
        visitTree(AllocTree.Root, visitor, &seg);
    }

}


//------------------------------------------------------------------------
static void MemCpyClean(char* dest, const char* src, int sz)
{
    for (int i=0; i < sz; i++)
    {
        char c = src[i];
        if (c < 32 || c > 126)
            c = '.';
        dest[i] = c;
    }
}


//------------------------------------------------------------------------
void DebugStorage::reportViolation(DebugData* data, const char* msg)
{
    char memdump[33];
    memdump[0] = memdump[32] = 0;

    MemCpyClean(memdump, 
               (const char*)(data->Address), 
               (data->Size < 32) ? int(data->Size) : 32);

    SF_DEBUG_MESSAGE6(data->Info.pFileName,
                      "%s(%d) :%s %lu bytes @0x%8.8lX (contents:'%s')", 
                      data->Info.pFileName, 
                      data->Info.Line,
                      msg,
                      data->Size, 
                      data->Address, 
                      memdump);

    SF_DEBUG_MESSAGE2(!data->Info.pFileName,
                      "%s in unknown file: %lu bytes ",
                      msg,
                      data->Size);

    const DebugData* upper = data->pUpperThis;

    unsigned i2 = 1;
    char indentBuf[256];
    while(upper)
    {
        if (i2 > sizeof(indentBuf) - 1)
            i2 = sizeof(indentBuf) - 1;
        memset(indentBuf, '+', i2);
        indentBuf[i2] = 0;

        SF_DEBUG_MESSAGE5(upper->Info.pFileName,
                          "%s(%d) : %s Parent alloc for (%lu bytes @0x%8.8lX)",
                          upper->Info.pFileName, 
                          upper->Info.Line,
                          indentBuf,
                          data->Size, 
                          data->Address);

        SF_DEBUG_MESSAGE3(!upper->Info.pFileName,
                          "%s Unknown file: Parent alloc for (%lu bytes @0x%8.8lX)", 
                          indentBuf,
                          data->Size,
                          data->Address);

        upper = upper->pUpperThis;
        ++i2;
    }
}


//------------------------------------------------------------------------
void DebugStorage::dumpMemoryLeaks(DebugData* data, MemLeaksSummary* leaks)
{
    if (data)
    {
        dumpMemoryLeaks(data->pPrev, leaks);
        checkDataTail(data, Memory::GetGlobalHeap()->GetUsableSize((const void*)data->Address));
        reportViolation(data, " Leak");
        leaks->LeakedBytes += data->Size;
        leaks->LeakedAllocs++;
        dumpMemoryLeaks(data->pNext, leaks);
    }
}


//------------------------------------------------------------------------
bool DebugStorage::DumpMemoryLeaks(const char* heapName)
{
    DebugData* data = AllocTree.Root;
    SF_DEBUG_ERROR1((data != 0), 
                    "Memory leaks detected in heap '%s'!", heapName);

    MemLeaksSummary leaks = { 0, 0 };
    dumpMemoryLeaks(AllocTree.Root, &leaks);
    SF_DEBUG_ERROR2(leaks.LeakedBytes, 
                    "Total memory leaked: %lu bytes in %lu allocations", 
                    leaks.LeakedBytes, 
                    leaks.LeakedAllocs);
    return leaks.LeakedBytes != 0;
}





#else // !defined SF_64BIT_POINTERS (32 bit mode)
//===============================================================================================

//------------------------------------------------------------------------
DebugStorage::DebugStorage(Granulator* alloc, LockSafe* rootLocker) : 
    pAllocator(alloc), 
    pRootLocker(rootLocker)
{}


//------------------------------------------------------------------------
bool DebugStorage::allocDataPool()
{
    void* poolBuf = 0; 
    {
        LockSafe::Locker rl(pRootLocker);
        poolBuf = pAllocator->Alloc(PoolSize, 0);
    }

    if (poolBuf)
    {
        //memset(poolBuf, 0xCC, PoolSize);
        DataPoolType* pool = ::new (poolBuf) DataPoolType;
        DataPools.PushFront(pool);

        UPInt size = pool->GetSize(PoolSize);
        for(UPInt i = 0; i < size; ++i)
        {
            DebugData* data = pool->GetElement(i);
            data->pDataPool = pool;
            FreeDataList.PushBack(data);
        }
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void DebugStorage::freeDataPool(DataPoolType* pool)
{
    UPInt size = pool->GetSize(PoolSize);
    for(UPInt i = 0; i < size; ++i)
    {
        FreeDataList.Remove(pool->GetElement(i));
    }
    DataPools.Remove(pool);
    {
        LockSafe::Locker rl(pRootLocker);
        //memset(pool, 0xFE, PoolSize);
        pAllocator->Free(pool, PoolSize, 0);
    }
}


//------------------------------------------------------------------------
bool DebugStorage::allocPagePool()
{
    void* poolBuf = 0; 
    {
        LockSafe::Locker rl(pRootLocker);
        poolBuf = pAllocator->Alloc(PoolSize, 0);
    }

    if (poolBuf)
    {
        //memset(poolBuf, 0xCC, PoolSize);
        PagePoolType* pool = ::new (poolBuf) PagePoolType;
        PagePools.PushFront(pool);

        UPInt size = pool->GetSize(PoolSize);
        for(UPInt i = 0; i < size; ++i)
        {
            DebugPage* page = pool->GetElement(i);
            page->pDataPool = pool;
            FreePageList.PushBack(page);
        }
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void DebugStorage::freePagePool(PagePoolType* pool)
{
    UPInt size = pool->GetSize(PoolSize);
    for(UPInt i = 0; i < size; ++i)
    {
        FreePageList.Remove(pool->GetElement(i));
    }
    PagePools.Remove(pool);
    {
        LockSafe::Locker rl(pRootLocker);
        //memset(pool, 0xFE, PoolSize);
        pAllocator->Free(pool, PoolSize, 0);
    }
}




//------------------------------------------------------------------------
DebugPage* DebugStorage::allocDebugPage()
{
    if (FreePageList.IsEmpty())
    {
        if (!allocPagePool())
            return 0;
    }
    SF_HEAP_ASSERT(!FreePageList.IsEmpty());

    DebugPage* page = FreePageList.GetFirst();
    FreePageList.Remove(page);
    page->Clear();
    page->pDataPool->UseCount++;

    return page;
}


//------------------------------------------------------------------------
void DebugStorage::freeDebugPage(DebugPage* page)
{
    //page->Clear();
    FreePageList.PushFront(page);
    if (--page->pDataPool->UseCount == 0)
    {
        freePagePool(page->pDataPool);
    }
}


//------------------------------------------------------------------------
DebugData* DebugStorage::allocDebugData()
{
    if (FreeDataList.IsEmpty())
    {
        if (!allocDataPool())
            return 0;
    }
    SF_HEAP_ASSERT(!FreeDataList.IsEmpty());

    DebugData* data = FreeDataList.GetFirst();
    FreeDataList.Remove(data);
    data->Clear();
    UsedDataList.PushBack(data);
    data->pDataPool->UseCount++;
    return data;
}


//------------------------------------------------------------------------
void DebugStorage::freeDebugData(DebugData* data)
{
    //data->Clear();
    FreeDataList.PushFront(data);
    if (--data->pDataPool->UseCount == 0)
    {
        freeDataPool(data->pDataPool);
    }
}




//------------------------------------------------------------------------
void DebugStorage::linkToPage(DebugPage* page, DebugData* data)
{
    UPInt idx = (data->Address & Heap_PageMask) >> AddrShift;
    SF_HEAP_ASSERT(idx < DebugPage::PageSize);
    data->pNextData = page->pData[idx];
    page->pData[idx] = data;
    page->Mask |= UPInt(1) << idx;
    page->DataCount++;
}


//------------------------------------------------------------------------
bool DebugStorage::convertToPage(DebugData* chain, DebugData* newData)
{
    DebugPage* page = allocDebugPage();
    if (page == 0)
    {
        return false;
    }

    DebugData* node = chain;
    while (node)
    {
        DebugData* next = node->pNextData;
        linkToPage(page, node);
        node = next;
    }
    linkToPage(page, newData);
    GlobalPageTable->SetDebugNode(newData->Address, page);
    return true;
}

//------------------------------------------------------------------------
void DebugStorage::convertToChain(DebugPage* page, DebugData* oldData)
{
    DebugData* chain = 0;
    UPInt mask  = page->Mask;
    UPInt idx   = 0;
    UPInt count = 0;
    while(mask)
    {
        UPInt shift = Alg::LowerBit(mask);
        idx += shift;
        DebugData* data = page->pData[idx];
        while(data)
        {
            DebugData* next = data->pNextData;
            if (data != oldData)
            {
                data->pNextData = chain;
                chain = data;
                ++count;
            }
            data = next;
        }
        ++idx;
        mask >>= shift;
        mask >>= 1; // DON'T DO "mask >>= shift+1"
    }
    SF_HEAP_ASSERT(chain != 0);
    chain->DataCount = UPIntHalf(count);
    GlobalPageTable->SetDebugNode(oldData->Address, chain);
    freeDebugPage(page);
}

//------------------------------------------------------------------------
void DebugStorage::unlinkDebugData(DebugDataPtr* ptr)
{
    if (ptr->pNode->IsPage())
    {
        DebugPage* page = (DebugPage*)ptr->pNode;
        if (page->Decrement() <= PageLowerLimit)
        {
            convertToChain(page, ptr->pSelf);
        }
        else
        {
            if (ptr->pPrev)
            {
                ptr->pPrev->pNextData = ptr->pSelf->pNextData;
            }
            else
            {
                page->pData[ptr->Index] = ptr->pSelf->pNextData;
                if (page->pData[ptr->Index] == 0)
                {
                    page->Mask &= ~(UPInt(1) << ptr->Index);
                    if (page->Mask == 0)
                    {
                        GlobalPageTable->SetDebugNode(ptr->pSelf->Address, 0);
                        freeDebugPage(page);
                    }
                }
            }
        }
    }
    else
    {
        ptr->pNode->DataCount--;
        if (ptr->pPrev)
        {
            ptr->pPrev->pNextData = ptr->pSelf->pNextData;
        }
        else
        {
            if (ptr->pSelf->pNextData)
            {
                ptr->pSelf->pNextData->DataCount = ptr->pNode->DataCount;
            }
#ifdef SF_MEMORY_FORCE_SYSALLOC
            SF_HEAP_ASSERT(ptr->pSelf->pNextData == 0);
#endif
            GlobalPageTable->SetDebugNode(ptr->pSelf->Address, ptr->pSelf->pNextData);
        }
    }
}

//------------------------------------------------------------------------
void DebugStorage::linkDebugData(DebugData* data)
{
    DebugNode* node = GlobalPageTable->GetDebugNode(data->Address);

#ifdef SF_MEMORY_FORCE_SYSALLOC
    SF_HEAP_ASSERT(node == 0);
#endif

    if (node == 0)
    {
        data->pNextData = 0;
        node = data;
    }

    if (node->IsPage())
    {
        linkToPage((DebugPage*)node, data);
    }
    else
    {
        // The following logic guarantees successful linking.
        // Even if it fails to convert the data chain to a page,
        // it continues the chain. This guarantee is necessary
        // for realloc. If realloc worked successfully, we must 
        // guarantee successful relink.
        //-------------------------
        if (node->Increment() >= PageUpperLimit && 
            convertToPage((DebugData*)node, data))
        {
            return;
        }
        if (data != node)
        {
            data->pNextData = (DebugData*)node;
            data->DataCount = node->DataCount;
        }
        GlobalPageTable->SetDebugNode(data->Address, data);
    }
}



//------------------------------------------------------------------------
void DebugStorage::findInChainWithin(DebugData* chain, UPInt addr, DebugDataPtr* ptr)
{
    DebugData* prev = 0;
    while (chain)
    {
        UPInt end = chain->Address + chain->Size;
        if (addr >= chain->Address && addr < end)
        {
            ptr->pSelf = chain;
            ptr->pPrev = prev;
            return;
        }
        prev  = chain;
        chain = chain->pNextData;
    }
}


//------------------------------------------------------------------------
void DebugStorage::findInChainExact(DebugData* chain, UPInt addr, DebugDataPtr* ptr)
{
    DebugData* prev = 0;
    while (chain)
    {
        if (addr == chain->Address)
        {
            ptr->pSelf = chain;
            ptr->pPrev = prev;
            return;
        }
        prev  = chain;
        chain = chain->pNextData;
    }
}


//------------------------------------------------------------------------
void DebugStorage::findInNodeWithin(DebugNode* node, 
                                    UPInt idx, UPInt addr, 
                                    DebugDataPtr* ptr)
{
    if (node->IsPage())
    {
        DebugPage* page = (DebugPage*)node;
        UPInt mask = (((UPInt(1) << idx) - 1) << 1) | 1;
        idx = Alg::UpperBit(page->Mask & mask);
        ptr->pNode = page;
        ptr->Index = idx;
        findInChainWithin(page->pData[idx], addr, ptr);
    }
    else
    {
        findInChainWithin((DebugData*)node, addr, ptr);
    }
}

//------------------------------------------------------------------------
void DebugStorage::findDebugData(UPInt addr, bool autoHeap, DebugDataPtr* ptr)
{
    DebugNode* node = GlobalPageTable->FindDebugNode(addr);

    SF_HEAP_ASSERT(!(autoHeap && node == 0));

    UPInt idx = (addr & Heap_PageMask) >> AddrShift;
    SF_HEAP_ASSERT(idx < DebugPage::PageSize);

    findInNodeWithin(node, idx, addr, ptr);
    if (ptr->pSelf)
    {
        return;
    }

    if (idx)
    {
        findInNodeWithin(node, idx-1, addr, ptr);
        if (ptr->pSelf)
        {
            return;
        }
    }

    node = GlobalPageTable->FindDebugNode(addr-Heap_PageSize);
    if (!autoHeap && node == 0)
    {
        return;
    }
    SF_HEAP_ASSERT(node != 0);
    findInNodeWithin(node, DebugPage::PageSize-1, addr, ptr);
    SF_HEAP_ASSERT(!(autoHeap && ptr->pSelf == 0));
}


//------------------------------------------------------------------------
void DebugStorage::fillDataTail(DebugData* data, UPInt usable)
{
    UByte* p = (UByte*)data->Address;
    for (UPInt i = data->Size; i < usable; ++i)
    {
        p[i] = UByte(129 + i);
    }
}

//------------------------------------------------------------------------
void DebugStorage::CheckDataTail(const DebugDataPtr* ptr, UPInt usable)
{
    const UByte* p = (const UByte*)ptr->pSelf->Address;
    for (UPInt i = ptr->pSelf->Size; i < usable; ++i)
    {
        if (p[i] != UByte(129 + i))
        {
            reportViolation(ptr->pSelf, " Corruption");
            SF_HEAP_ASSERT(0);
        }
    }
}

//------------------------------------------------------------------------
unsigned DebugStorage::GetStatId(UPInt parentAddr, const AllocInfo* info)
{
    AllocInfo i2 = info ? *info : AllocInfo();
    if (i2.StatId != Stat_Default_Mem)
    {
        return i2.StatId;
    }
    DebugDataPtr parent;
    findDebugData(parentAddr, true, &parent);
    if (parent.pSelf == 0)
    {
        return Stat_Default_Mem;
    }
    return parent.pSelf->Info.StatId;
}

//------------------------------------------------------------------------
bool DebugStorage::AddAlloc(UPInt parentAddr, bool autoHeap, UPInt thisAddr, 
                            UPInt size, UPInt usable, const AllocInfo* info)
{
    SF_HEAP_ASSERT(thisAddr);

    DebugData* data = allocDebugData();
    if (data)
    {
        data->RefCount  = 1;
        data->Address   = thisAddr;
        data->Size      = size;
        data->Info      = info ? *info : AllocInfo();

        linkDebugData(data);
        fillDataTail(data, usable);

//if (usable > size)      // DBG Simulate memory corruption
//    ((char*)data->Address)[size] = 0;

        if (parentAddr)
        {
            DebugDataPtr parent;
            findDebugData(parentAddr, autoHeap, &parent);
            if (!autoHeap && parent.pSelf == 0)
            {
                return true;
            }
            SF_HEAP_ASSERT(parent.pSelf != 0);
            if (data->Info.StatId == Stat_Default_Mem)
            {
                data->Info.StatId = parent.pSelf->Info.StatId;
            }

            // Prevent from creating too long parent chains in 
            // the following scenario:
            //
            //    p1 = heap.Alloc(. . .);
            //    for(. . .)
            //    {
            //        p2 = AllocAutoHeap(p1, . . .);
            //        Free(p1);
            //        p1 = p2;
            //    }
            //
            // When the chain gets longer than ChainLimit, just 
            // connect it to the grandparent, interrupting the chain. 
            // The condition "DebugNode::ChainLimit+1" means that 
            // we allow one more element in the chain to be able to detect 
            // whether or not the chain was interrupted: 
            // Interruption has occurred if ChainCount > ChainLimit.
            //-----------------------------
            if (parent.pSelf->ChainCount > DebugNode::ChainLimit+1)
            {
                parent.pSelf = parent.pSelf->pParent;
                SF_HEAP_ASSERT(parent.pSelf);
            }
            data->pParent    = parent.pSelf;
            data->ChainCount = parent.pSelf->ChainCount + 1;
            parent.pSelf->RefCount++;
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
void DebugStorage::RelinkAlloc(DebugDataPtr* ptr, UPInt oldAddr, 
                               UPInt newAddr, UPInt newSize, UPInt usable)
{
    SF_HEAP_ASSERT(ptr->pSelf && oldAddr && newAddr);
    ptr->pSelf->Size = newSize;
    ptr->pSelf->Address   = newAddr;
    ptr->pSelf->DataCount = 0;
    linkDebugData(ptr->pSelf);
    fillDataTail(ptr->pSelf, usable);
}


//------------------------------------------------------------------------
void DebugStorage::UnlinkAlloc(UPInt addr, DebugDataPtr* ptr)
{
    GetDebugData(addr, ptr);
    unlinkDebugData(ptr);
}


//------------------------------------------------------------------------
void DebugStorage::GetDebugData(UPInt addr, DebugDataPtr* ptr)
{
    DebugNode* node = GlobalPageTable->GetDebugNode(addr);
//if(node == 0)
//{
//printf("0 Addr=%08x\n", addr); // DBG
//}
    SF_HEAP_ASSERT(node);
    ptr->pNode = node;

#ifdef SF_MEMORY_FORCE_SYSALLOC
    DebugData* data = (DebugData*)node;
    SF_HEAP_ASSERT(data->pNextData == 0 && data->Address == addr);
    ptr->pSelf = data;
    ptr->pPrev = 0;
#else
    if (node->IsPage())
    {
        DebugPage* page = (DebugPage*)node;
        ptr->Index = (addr & Heap_PageMask) >> AddrShift;
        findInChainExact(page->pData[ptr->Index], addr, ptr);
    }
    else
    {
        findInChainExact((DebugData*)node, addr, ptr);
    }
#endif
    SF_HEAP_ASSERT(ptr->pSelf);
}

//------------------------------------------------------------------------
void DebugStorage::RemoveAlloc(UPInt addr, UPInt usable)
{
    SF_HEAP_ASSERT(addr);
    DebugDataPtr ptr;
    GetDebugData(addr, &ptr);
    CheckDataTail(&ptr, usable);
    SF_HEAP_ASSERT(ptr.pSelf->Address == addr);  // Must be exact address.

    UsedDataList.Remove(ptr.pSelf);
    unlinkDebugData(&ptr);

    DebugData* data = ptr.pSelf;
    while (data && --data->RefCount == 0)
    {
        DebugData* parent = data->pParent;
        freeDebugData(data);
        data = parent;
    }
}

//------------------------------------------------------------------------
void DebugStorage::FreeAll()
{
    const DebugData* data = UsedDataList.GetFirst();
    while (!UsedDataList.IsNull(data))
    {
        GlobalPageTable->SetDebugNode(data->Address, 0);
        data = (const DebugData*)data->pNext;
    }

    while (!DataPools.IsEmpty())
    {
        DataPoolType* pool = DataPools.GetFirst();
        DataPools.Remove(pool);
        pAllocator->Free(pool, PoolSize, 0);
    }

    while (!PagePools.IsEmpty())
    {
        PagePoolType* pool = PagePools.GetFirst();
        PagePools.Remove(pool);
        pAllocator->Free(pool, PoolSize, 0);
    }
    UsedDataList.Clear();
    FreeDataList.Clear();
    FreePageList.Clear();
}

//------------------------------------------------------------------------
void DebugStorage::GetStats(AllocEngine* allocator, StatBag* bag) const
{
#ifdef SF_ENABLE_STATS
    // Must be locked in MemoryHeap.
    const DebugData* data = UsedDataList.GetFirst();
    while (!UsedDataList.IsNull(data))
    {
        bag->IncrementMemoryStat(data->Info.StatId,
                                 data->Size,
                                 allocator->GetUsableSize((void*)data->Address));
        data = (const DebugData*)data->pNext;
    }
#else
    SF_UNUSED2(allocator, bag);
#endif
}

//------------------------------------------------------------------------
const DebugData* DebugStorage::GetFirstEntry() const
{
    return UsedDataList.IsEmpty() ? 0 : UsedDataList.GetFirst();
}


//------------------------------------------------------------------------
const DebugData* DebugStorage::GetNextEntry(const DebugData* entry) const
{
    const DebugData* next = (const DebugData*)entry->pNext;
    return UsedDataList.IsNull(next) ? 0 : next;
}


//------------------------------------------------------------------------
void DebugStorage::VisitMem(MemVisitor* visitor, unsigned flags) const
{
    pAllocator->VisitMem(visitor, 
                         MemVisitor::Cat_DebugInfo,
                         MemVisitor::Cat_DebugInfoFree);

    if (flags & MemVisitor::VF_DebugInfoDetailed)
    {
        HeapSegment seg; // Fake segment
        seg.SelfSize  = 0;
        seg.SegType   = Heap_SegmentUndefined;
        seg.Alignment = 0;
        seg.UseCount  = 0;
        seg.pHeap     = 0;
        seg.DataSize  = 0;
        seg.pData     = 0;

        visitMem(DataPools, PoolSize, visitor, MemVisitor::Cat_DebugDataPool, &seg);
        visitMem(PagePools, PoolSize, visitor, MemVisitor::Cat_DebugPagePool, &seg);

        visitMem(UsedDataList, sizeof(DebugData), visitor, MemVisitor::Cat_DebugDataBusy, &seg);
        visitMem(FreeDataList, sizeof(DebugData), visitor, MemVisitor::Cat_DebugDataFree, &seg);
        visitMem(FreePageList, sizeof(DebugPage), visitor, MemVisitor::Cat_DebugPageFree, &seg);
    }
}


//------------------------------------------------------------------------
static void MemCpyClean(char* dest, const char* src, int sz)
{
    int i = 0;
    for (; i < sz; i++)
    {
        char c = src[i];
        if (c < 32 || c > 126)
            c = '.';
        dest[i] = c;
    }
    dest[i] = 0;
}


//------------------------------------------------------------------------
void DebugStorage::reportViolation(DebugData* data, const char* msg)
{
    char memdump[33];

    MemCpyClean(memdump, 
               (const char*)(data->Address), 
               (data->Size < 32) ? int(data->Size) : 32);

    SF_DEBUG_MESSAGE6(data->Info.pFileName,
                      "%s(%d) :%s %lu bytes @0x%8.8lX (contents:'%s')\n", 
                      data->Info.pFileName, 
                      data->Info.Line,
                      msg,
                      (unsigned long)data->Size, 
                      (unsigned long)data->Address, 
                      memdump);

    SF_DEBUG_MESSAGE2(!data->Info.pFileName,
                      "%s in unknown file: %lu bytes \n",
                      msg,
                      (unsigned long)data->Size);

    const DebugData* parent = data->pParent;

    unsigned int i2 = 1;
    char indentBuf[256];
    while(parent)
    {
        if (i2 > sizeof(indentBuf) - 1)
            i2 = sizeof(indentBuf) - 1;
        memset(indentBuf, '+', i2);
        indentBuf[i2] = 0;

        SF_DEBUG_MESSAGE6(parent->Info.pFileName,
                          "%s(%d) : %s Parent of alloc @0x%8.8lX is (%lu bytes @0x%8.8lX)\n",
                          parent->Info.pFileName, 
                          parent->Info.Line,
                          indentBuf,
                          (unsigned long)data->Address,
                          (unsigned long)parent->Size,
                          (unsigned long)parent->Address);

        SF_DEBUG_MESSAGE4(!parent->Info.pFileName,
                          "%s Unknown file: Parent of alloc @0x%8.8lX is (%lu bytes @0x%8.8lX)\n", 
                          indentBuf,
                          (unsigned long)data->Address,
                          (unsigned long)parent->Size,
                          (unsigned long)parent->Address);

        parent = parent->pParent;
        ++i2;
    }
}


//------------------------------------------------------------------------
bool DebugStorage::DumpMemoryLeaks(const char* heapName)
{
    bool ret = false;
    UPInt leakedBytes = 0;
    UPInt leakedAllocs = 0;

    DebugData* data = UsedDataList.GetFirst();
    SF_DEBUG_ERROR1(!UsedDataList.IsNull(data), 
                    "Memory leaks detected in heap '%s'!", heapName);

    while (!UsedDataList.IsNull(data))
    {
        ret = true;
        reportViolation(data, " Leak");
        leakedBytes += data->Size;
        leakedAllocs++;
        data = (DebugData*)data->pNext;
    }
    SF_DEBUG_ERROR2(ret, "Total memory leaked: %lu bytes in %lu allocations", 
                    (unsigned long)leakedBytes, 
                    (unsigned long)leakedAllocs);
    return ret;
}


//------------------------------------------------------------------------
void DebugStorage::UltimateCheck()
{
    const DebugData* data = UsedDataList.GetFirst();
    while (!UsedDataList.IsNull(data))
    {
        DebugDataPtr ptr;
        GetDebugData(data->Address, &ptr);
        data = (const DebugData*)data->pNext;
    }
}


//------------------------------------------------------------------------
UPInt DebugStorage::GetUsedSpace() const
{
    UPInt usedSpace = 0;
    const DataPoolType* data = DataPools.GetFirst();
    while(!DataPools.IsNull(data))
    {
        usedSpace += PoolSize;
        data = data->pNext;
    }
    const PagePoolType* page = PagePools.GetFirst();
    while(!PagePools.IsNull(page))
    {
        usedSpace += PoolSize;
        page = page->pNext;
    }
    return usedSpace;
}

#endif // SF_64BIT_POINTERS

}} // Scaleform::Heap

#endif
