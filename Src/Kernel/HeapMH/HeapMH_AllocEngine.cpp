/**************************************************************************

Filename    :   HeapMH_AllocEngine.cpp
Content     :   The main allocation engine
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_HeapTypes.h"
#include "HeapMH_AllocEngine.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
AllocEngineMH::AllocEngineMH(SysAlloc*     sysAlloc,
                             MemoryHeapMH* heap,
                             UPInt         minAlignSize,
                             UPInt         limit) : 
    pSysAlloc(sysAlloc),
    pHeap(heap),
    MinAlignSize((minAlignSize < sizeof(void*)) ? sizeof(void*) : minAlignSize),
    Allocator(),
    Pages(),
    Footprint(0),
    UsedSpace(0),
    Limit(limit),
    pLimHandler(0),
    UseCount(0)
{
}

//------------------------------------------------------------------------
AllocEngineMH::~AllocEngineMH()
{
    FreeAll();
}

void AllocEngineMH::FreeAll()
{
    // TO DO: Implement
}

//------------------------------------------------------------------------
PageMH* AllocEngineMH::allocPageUnlocked(bool* limHandlerOK)
{
    if (Limit && Footprint + PageMH::PageSize > Limit && pLimHandler)
    {
        ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        *limHandlerOK = 
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + PageMH::PageSize - Limit);
        --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
    }

    *limHandlerOK = false;

    LockSafe::Locker rl(GlobalRootMH->GetLock());
    PageMH* page = GlobalRootMH->AllocPage(pHeap);
    if (page)
    {
        UInt32 index = GlobalRootMH->GetPageIndex(page);
        Allocator.InitPage(page, index);
        Footprint += PageMH::PageSize;
        Pages.PushFront(page);
        *limHandlerOK = true;
    }
    return page;
}


//------------------------------------------------------------------------
PageMH* AllocEngineMH::allocPageLocked(bool* limHandlerOK)
{
    if (Limit && Footprint + PageMH::PageSize > Limit && pLimHandler)
    {
        LockSafe::TmpUnlocker unlocker(GlobalRootMH->GetLock());
        ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        *limHandlerOK = 
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + PageMH::PageSize - Limit);
        --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
    }

    *limHandlerOK = false;
    PageMH* page = GlobalRootMH->AllocPage(pHeap);
    if (page)
    {
        UInt32 index = GlobalRootMH->GetPageIndex(page);
        Allocator.InitPage(page, index);
        Footprint += PageMH::PageSize;
        Pages.PushFront(page);
        *limHandlerOK = true;
    }
    return page;
}


//------------------------------------------------------------------------
void AllocEngineMH::freePage(PageMH* page, bool globalLocked)
{
    if (globalLocked)
    {
        Allocator.ReleasePage(page->Start);
        PageListType::Remove(page);
        GlobalRootMH->FreePage(page);
    }
    else
    {
        LockSafe::Locker rl(GlobalRootMH->GetLock());
        Allocator.ReleasePage(page->Start);
        PageListType::Remove(page);
        GlobalRootMH->FreePage(page);
    }
    Footprint -= PageMH::PageSize;
}

//------------------------------------------------------------------------
void* AllocEngineMH::allocFromPage(UPInt size, PageInfoMH* info, bool globalLocked)
{
    void* ptr;
    MagicHeadersInfo headers;
    bool limHandlerOK = false;
    do
    {
        ptr = Allocator.Alloc(size, &headers);
        if (ptr)
        {
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            info->DebugHeaders[0] = info->DebugHeaders[1] = info->DebugHeaders[2] = 0;
#endif
            if (headers.Header1) 
            {
                headers.Header1->UseCount++;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
                info->DebugHeaders[0] = &(headers.Header1->DebugHeader);
#endif
            }
            if (headers.Header2) 
            {
                headers.Header2->UseCount++;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
                info->DebugHeaders[1] = &(headers.Header2->DebugHeader);
#endif
            }
            info->UsableSize = size;
            info->Page = headers.Page;
            info->Node = 0;
            ++UseCount;
            UsedSpace += size;
            return ptr;
        }
        if (globalLocked) allocPageLocked(&limHandlerOK);
        else              allocPageUnlocked(&limHandlerOK);
    }
    while(limHandlerOK);
    return 0;
}

//------------------------------------------------------------------------
void* AllocEngineMH::allocFromPage(UPInt size, UPInt alignSize, PageInfoMH* info, bool globalLocked)
{
    void* ptr;
    MagicHeadersInfo headers;
    bool limHandlerOK = false;
    do
    {
        ptr = Allocator.Alloc(size, alignSize, &headers);
        if (ptr)
        {
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            info->DebugHeaders[0] = info->DebugHeaders[1] = info->DebugHeaders[2] = 0;
#endif
            if (headers.Header1) 
            {
                headers.Header1->UseCount++;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
                info->DebugHeaders[0] = &(headers.Header1->DebugHeader);
#endif
            }
            if (headers.Header2) 
            {
                headers.Header2->UseCount++;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
                info->DebugHeaders[1] = &(headers.Header2->DebugHeader);
#endif
            }
            info->UsableSize = size;
            info->Page = headers.Page;
            info->Node = 0;
            ++UseCount;
            UsedSpace += size;
            return ptr;
        }
        if (globalLocked) allocPageLocked(&limHandlerOK);
        else              allocPageUnlocked(&limHandlerOK);
    }
    while(limHandlerOK);
    return 0;
}

//------------------------------------------------------------------------
void AllocEngineMH::Free(PageMH* page, void* ptr, bool globalLocked)
{
    SF_ASSERT(page && page->pHeap == pHeap);
    MagicHeadersInfo headers;
    UPInt oldSize;
    Allocator.Free(page, ptr, &headers, &oldSize);
    unsigned useCount = 0;
    UsedSpace -= oldSize;
    if (headers.Header1) useCount = --headers.Header1->UseCount;
    if (headers.Header2) useCount = --headers.Header2->UseCount;
    if (useCount == 0)
    {
        freePage(page, globalLocked);
    }
    --UseCount;
}


//------------------------------------------------------------------------
void AllocEngineMH::Free(NodeMH* node, void* ptr, bool)
{
    SF_ASSERT(node && node->GetHeap() == pHeap);
    GlobalRootMH->RemoveFromGlobalTree(node);
    UPInt align = node->GetAlign();
    UPInt nodeSize = NodeMH::GetNodeSize(align);
    UPInt size = (UByte*)node - (UByte*)ptr + nodeSize;
    --UseCount;
    Footprint -= size;
    UsedSpace -= size - nodeSize;
    pSysAlloc->Free(ptr, size, align);
}


//------------------------------------------------------------------------
// For easy use only. Not called from MemoryHeap
void AllocEngineMH::Free(void* ptr, bool globalLocked)
{
    PageMH* page = GlobalRootMH->ResolveAddress(UPInt(ptr));
    if (page)
    {
        Free(page, ptr, globalLocked);
    }
    else
    {
        if (globalLocked)
        {
            NodeMH* node = GlobalRootMH->FindNodeInGlobalTree((UByte*)ptr);
            Free(node, ptr, true);
        }
        else
        {
            LockSafe::Locker rl(GlobalRootMH->GetLock());
            NodeMH* node = GlobalRootMH->FindNodeInGlobalTree((UByte*)ptr);
            Free(node, ptr, false);
        }
    }
}

//------------------------------------------------------------------------
void AllocEngineMH::GetPageInfo(PageMH* page, PageInfoMH* info) const
{
    memset(info, 0, sizeof(PageInfoMH));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(page->Start), &headers);
    if(headers.Header1) info->DebugHeaders[0] = &(headers.Header1->DebugHeader);
    if(headers.Header2) info->DebugHeaders[1] = &(headers.Header2->DebugHeader);
#endif
    info->UsableSize = 0;
    info->Page = page;
}

//------------------------------------------------------------------------
void AllocEngineMH::GetPageInfo(NodeMH* node, PageInfoMH* info) const
{
    memset(info, 0, sizeof(PageInfoMH));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    info->DebugHeaders[2] = &(node->DebugHeader);
#endif
    info->UsableSize = 0;
    info->Node = node;
}

//------------------------------------------------------------------------
void AllocEngineMH::GetPageInfoWithSize(PageMH* page, const void* ptr, PageInfoMH* info) const
{
    memset(info, 0, sizeof(PageInfoMH));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(page->Start), &headers);
    if(headers.Header1) info->DebugHeaders[0] = &(headers.Header1->DebugHeader);
    if(headers.Header2) info->DebugHeaders[1] = &(headers.Header2->DebugHeader);
#endif
    info->UsableSize = Allocator.GetUsableSize(page, ptr);
    info->Page = page;
}

//------------------------------------------------------------------------
void AllocEngineMH::GetPageInfoWithSize(NodeMH* node, const void* ptr, PageInfoMH* info) const
{
    memset(info, 0, sizeof(PageInfoMH));
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    info->DebugHeaders[2] = &(node->DebugHeader);
#endif
    info->UsableSize = (UByte*)node - (UByte*)ptr;
    info->Node = node;
}

//------------------------------------------------------------------------
void* AllocEngineMH::allocDirect(UPInt size, UPInt alignSize, bool* limHandlerOK, PageInfoMH* info)
{
    size = (size + sizeof(void*) - 1) & ~UPInt(sizeof(void*) - 1);
    UPInt nodeSize = NodeMH::GetNodeSize(alignSize);

    if (Limit && Footprint + size + nodeSize > Limit && pLimHandler)
    {
        LockSafe::TmpUnlocker unlocker(GlobalRootMH->GetLock());
        ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        *limHandlerOK = 
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + size + nodeSize - Limit);
        --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
    }

    *limHandlerOK = false;
    void* ptr = pSysAlloc->Alloc(size + nodeSize, alignSize);
    if (ptr)
    {
        NodeMH* node = GlobalRootMH->AddToGlobalTree((UByte*)ptr, size, alignSize, pHeap);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        info->DebugHeaders[0] = info->DebugHeaders[1] = 0;
        info->DebugHeaders[2] = &(node->DebugHeader);
#endif
        info->UsableSize = size;
        info->Page = 0;
        info->Node = node;
        ++UseCount;
        Footprint += size + nodeSize;
        UsedSpace += size;
        *limHandlerOK = true;
    }
    return ptr;
}


//------------------------------------------------------------------------
void* AllocEngineMH::Alloc(UPInt size, PageInfoMH* info, bool globalLocked)
{
    if (MinAlignSize > PageMH::UnitSize)
        return Alloc(size, MinAlignSize, info, globalLocked);

    if (size <= PageMH::MaxBytes)
    {
        size = (size + PageMH::UnitMask) & ~UPInt(PageMH::UnitMask);
        return allocFromPage(size, info, globalLocked);
    }

    void* ptr = 0;
    {
        if (globalLocked)
        {
            bool limHandlerOK = false;
            do
            {
                ptr = allocDirect(size, MinAlignSize, &limHandlerOK, info);
                if (ptr)
                    return ptr;
            }
            while(limHandlerOK);
        }
        else
        {
            LockSafe::Locker rl(GlobalRootMH->GetLock());
            bool limHandlerOK = false;
            do
            {
                ptr = allocDirect(size, MinAlignSize, &limHandlerOK, info);
                if (ptr)
                    return ptr;
            }
            while(limHandlerOK);
        }
    }
    return ptr;
}


//------------------------------------------------------------------------
void* AllocEngineMH::Alloc(UPInt size, UPInt alignSize, PageInfoMH* info, bool globalLocked)
{
    SF_ASSERT(((alignSize - 1) & alignSize) == 0); // Must be a power of two

    if (size <= PageMH::MaxBytes)
    {
        if (alignSize < PageMH::UnitSize)
            alignSize = PageMH::UnitSize;

        size = (size + PageMH::UnitMask) & ~UPInt(PageMH::UnitMask);
        return allocFromPage(size, alignSize, info, globalLocked);
    }

    void* ptr = 0;  // 外部ptr声明
    {
        if (alignSize < sizeof(void*))
            alignSize = sizeof(void*);

        if (size < alignSize)
            size = alignSize;

        size = (size + sizeof(void*) - 1) & ~UPInt(sizeof(void*) - 1);
        // 删除内部的 void* ptr = 0; 声明，直接使用外部ptr
        {
            if (globalLocked)
            {
                bool limHandlerOK = false;
                do
                {
                    ptr = allocDirect(size, alignSize, &limHandlerOK, info);  // 使用外部ptr
                    if (ptr)
                        return ptr;
                }
                while (limHandlerOK);
            }
            else
            {
                LockSafe::Locker rl(GlobalRootMH->GetLock());
                bool limHandlerOK = false;
                do
                {
                    ptr = allocDirect(size, alignSize, &limHandlerOK, info);  // 使用外部ptr
                    if (ptr)
                        return ptr;
                }
                while (limHandlerOK);
            }
        }
    }
    return ptr;
}


//------------------------------------------------------------------------
void* AllocEngineMH::ReallocInPage(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool)
{
    SF_ASSERT(page && page->pHeap == pHeap);
    void* newPtr = 0;
    if (newSize < PageMH::PageSize/2)
    {
        newSize = (newSize + PageMH::UnitMask) & ~UPInt(PageMH::UnitMask);
        MagicHeadersInfo headers;
        UPInt oldSize;
        newPtr = Allocator.ReallocInPlace(page, oldPtr, newSize, &oldSize, &headers);
        if (newPtr)
        {
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            newInfo->DebugHeaders[0] = newInfo->DebugHeaders[1] = newInfo->DebugHeaders[2] = 0;
            if (headers.Header1) newInfo->DebugHeaders[0] = &(headers.Header1->DebugHeader);
            if (headers.Header2) newInfo->DebugHeaders[1] = &(headers.Header2->DebugHeader);
#endif
            newInfo->UsableSize = newSize;
            newInfo->Page = headers.Page;
            newInfo->Node = 0;
            UsedSpace -= oldSize;
            UsedSpace += newSize;
        }
    }
    return newPtr;
}


//------------------------------------------------------------------------
void* AllocEngineMH::reallocInNodeNoLock(NodeMH* node, void* oldPtr, UPInt newSize, PageInfoMH* newInfo)
{
    void* newPtr = 0;
    UPInt oldAlign = node->GetAlign();
    UPInt nodeSize = NodeMH::GetNodeSize(oldAlign);
    UPInt oldSize  = (UByte*)node - (UByte*)oldPtr + nodeSize;
    newSize = (newSize + sizeof(void*)-1) & ~UPInt(sizeof(void*)-1);
    newSize += nodeSize;
    if (newSize > oldSize)
    {
        if (Limit && Footprint + newSize - oldSize > Limit && pLimHandler)
        {
            LockSafe::TmpUnlocker unlocker(GlobalRootMH->GetLock());
            // We CAN'T perform full OnExceedLimit from realloc:
            // full OnExceedLimit may cause the 'node' to become invalid (by freeing
            // or reallocating the very same memory block). Therefore, all what we 
            // can do is to increment LimHandler.AllocCount, thus, it will just 
            // schedule the collection and won't do collection right away.
            ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + newSize - oldSize - Limit);
            --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        }
    }
    GlobalRootMH->RemoveFromGlobalTree(node);
    newPtr = pSysAlloc->Realloc(oldPtr, oldSize, newSize, oldAlign);
    if (newPtr)
    {
        SF_ASSERT((UPInt(newPtr) & (oldAlign-1)) == 0);
        // 重命名内部node为newNode，避免隐藏参数
        NodeMH* newNode =
            GlobalRootMH->AddToGlobalTree((UByte*)newPtr, newSize - nodeSize, oldAlign, pHeap);
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        newInfo->DebugHeaders[0] = newInfo->DebugHeaders[1] = 0;
        newInfo->DebugHeaders[2] = &(node->DebugHeader);
#endif
        newInfo->UsableSize = newSize-nodeSize;
        newInfo->Page = 0;
        newInfo->Node = newNode;

        Footprint -= oldSize;
        Footprint += newSize;
        UsedSpace -= oldSize-nodeSize;
        UsedSpace += newSize-nodeSize;
    }
    else
    {
        GlobalRootMH->AddToGlobalTree((UByte*)oldPtr, oldSize-nodeSize, oldAlign, pHeap);
    }
    return newPtr;
}


//------------------------------------------------------------------------
void* AllocEngineMH::ReallocInNode(NodeMH* node, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool globalLocked)
{
    SF_ASSERT(node && node->GetHeap() == pHeap);
    newSize = (newSize + sizeof(void*) - 1) & ~UPInt(sizeof(void*) - 1);

    if (!globalLocked)
    {
        LockSafe::Locker rl(GlobalRootMH->GetLock());
        return reallocInNodeNoLock(node, oldPtr, newSize, newInfo);
    }
    return reallocInNodeNoLock(node, oldPtr, newSize, newInfo);
}

//------------------------------------------------------------------------
void* AllocEngineMH::ReallocGeneral(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool globalLocked)
{
    PageInfoMH oldInfo;
    void* newPtr = Alloc(newSize, newInfo, globalLocked);
    if (newPtr)
    {
        GetPageInfoWithSize(page, oldPtr, &oldInfo);
        memcpy(newPtr, oldPtr, (oldInfo.UsableSize < newInfo->UsableSize) ? 
                                oldInfo.UsableSize : newInfo->UsableSize);
        Free(page, oldPtr, globalLocked);
    }
    return newPtr;
}

//------------------------------------------------------------------------
void* AllocEngineMH::Realloc(void* oldPtr, UPInt newSize, bool globalLocked)
{
    PageInfoMH newInfo;
    PageMH* page = GlobalRootMH->ResolveAddress(UPInt(oldPtr));
    if (page)
    {
        return ReallocGeneral(page, oldPtr, newSize, &newInfo, globalLocked);
    }
    LockSafe::Locker rl(GlobalRootMH->GetLock());
    NodeMH* node = GlobalRootMH->FindNodeInGlobalTree((UByte*)oldPtr);
    return ReallocInNode(node, oldPtr, newSize, &newInfo, globalLocked);
}

//------------------------------------------------------------------------
UPInt AllocEngineMH::GetUsableSize(void* ptr)
{
    PageMH* page = GlobalRootMH->ResolveAddress(UPInt(ptr));
    if (page)
    {
        return Allocator.GetUsableSize(page, ptr);
    }
    LockSafe::Locker rl(GlobalRootMH->GetLock());
    NodeMH* node = GlobalRootMH->FindNodeInGlobalTree((UByte*)ptr);
    SF_ASSERT(node);
    return (UByte*)node - (UByte*)ptr;
}

}} // Scaleform::HeapMH
