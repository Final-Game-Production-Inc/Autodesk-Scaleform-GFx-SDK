/**************************************************************************

Filename    :   HeapPT_Root.h
Content     :   Heap root used for bootstrapping and bookkeeping.
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_Root_H
#define INC_SF_Kernel_HeapPT_Root_H

#include "../SF_Atomic.h"
#include "HeapPT_MemoryHeap.h"
#include "HeapPT_Bookkeeper.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

// ***** SysAllocGranulator
//
// An intermediate system allocator that allows to have as large system
// granularity as wanted. The granulator requests the system allocator for 
// large memory blocks and granulates them as necessary, providing smaller 
// granularity to the allocation engine.
//
// If the system provides an advanced allocation mechanism, the granulator 
// typically isn't necessary. However, it may speed up some cases, when 
// the system calls are expensive (like VirtualAlloc on Win platforms).
// 
// The granulator is really useful when the allocation system calls are 
// "alignment unfriendly". A typical case of it is regular malloc/free
// (or dlmalloc as well).
//
// The allocator may request the system for small blocks of memory, but not
// less than Heap_PageSize, which is 4KB on 32-bit systems. The allocation
// data must be aligned to the "alignment" parameter, which is in most
// cases equals to Heap_PageSize. The alignment requirement can be less 
// in some rare cases, when the allocator requests for small blocks of 
// for the initial bootstrapping.
// But for the user payload the alignment is at least Heap_PageSize. It 
// can be bigger than that only in cases of direct system allocations with
// bigger than 4K alignment requirements.
//
// If you create a heap with a very small granularity, like 4K, it's very
// likely that there will be many requests for those 4KB, aligned to 4KB 
// too. With regular malloc/free it may result in up to 2x memory overhead.
// SysAllocGranulator can smooth this "injustice of nature." It 
// requests the system for large blocks, (64K by default), aligned to 4K, 
// and handles all the smaller requests from the allocator. Obviously, in 
// case of 64K granularity and Heap_PageSize = 4K, the memory overhead 
// for the alignment will be in the worst case 1/16, or 6.25%. In average 
// it will be about two times less.
//------------------------------------------------------------------------
class SysAllocGranulator : public SysAllocPaged
{
public:
    SysAllocGranulator();
    SysAllocGranulator(SysAllocPaged* sysAlloc);

    void Init(SysAllocPaged* sysAlloc);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt alignment);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt alignment);
    virtual bool    Free(void* ptr, UPInt size, UPInt alignment);

    virtual void*   AllocSysDirect(UPInt size, UPInt alignment, UPInt* actualSize, UPInt* actualAlign);
    virtual bool    FreeSysDirect(void* ptr, UPInt size, UPInt alignment);

    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetUsedSpace() const;

    virtual void    VisitMem(MemVisitor* visitor) const;
    virtual void    VisitSegments(SegVisitor* visitor, UPInt catSeg, UPInt catUnused) const;

    virtual UPInt   GetBase() const; // DBG
    virtual UPInt   GetSize() const; // DBG

private:
    Granulator* pGranulator;
    UPInt       SysDirectThreshold;
    UPInt       MaxHeapGranularity;
    UPInt       MinAlign;
    UPInt       MaxAlign;
    UPInt       SysDirectFootprint;
    UPInt       PrivateData[32];
};


//------------------------------------------------------------------------
class SysAllocWrapper : public SysAllocPaged
{
public:
    SysAllocWrapper(SysAllocPaged* sysAlloc);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);

    virtual void*   AllocSysDirect(UPInt size, UPInt alignment, UPInt* actualSize, UPInt* actualAlign);
    virtual bool    FreeSysDirect(void* ptr, UPInt size, UPInt alignment);

    virtual void    VisitMem(MemVisitor*) const;
    virtual void    VisitSegments(Heap::SegVisitor* visitor, UPInt catSeg, UPInt catUnused) const;

    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetUsedSpace() const;

    void*           AllocSysDirect(UPInt size);
    void            FreeSysDirect(void* ptr, UPInt size);

private:
    SysAllocGranulator  Allocator;
    SysAllocPaged*      pSrcAlloc;
    SysAllocPaged*      pSysAlloc;
    UPInt               SysGranularity; // For AllocSysDirect
    UPInt               MinAlign;       // For AllocSysDirect
    UPInt               UsedSpace;
};

//------------------------------------------------------------------------
class HeapRoot
{
public:
    HeapRoot(SysAllocPaged* sysAlloc);
    ~HeapRoot();

    void                CreateArena(UPInt arena, SysAllocPaged* sysAlloc);
    int                 DestroyArena(UPInt arena);
    void                DestroyAllArenas();
    bool                ArenaIsEmpty(UPInt arena);
    SysAllocPaged*      GetSysAlloc(UPInt arena);

    Starter*            GetStarter()    { return &AllocStarter; }
    Bookkeeper*         GetBookkeeper() { return &AllocBookkeeper;  }

    MemoryHeapPT*       CreateHeap(const char*  name, 
                                   MemoryHeapPT* parent,
                                   const MemoryHeap::HeapDesc& desc);

    void                DestroyHeap(MemoryHeapPT* heap);

    LockSafe*           GetLock()     { return &RootLock; }

    void                VisitSegments(Heap::SegVisitor* visitor) const;

    void*               AllocSysDirect(UPInt size);
    void                FreeSysDirect(void* ptr, UPInt size);

    void                GetStats(MemoryHeap::RootStats* stats) const;

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    Granulator*         GetDebugAlloc() { return &DebugAlloc; }
#endif

#ifdef SF_MEMORY_TRACE_ALL
    void OnAlloc(const MemoryHeap* heap, UPInt size, UPInt align, unsigned sid, const void* ptr);
    void OnRealloc(const MemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr);
    void OnFree(const MemoryHeap* heap, const void* ptr);

    static MemoryHeap::HeapTracer* pTracer;
#endif


private:
    SysAllocWrapper     AllocWrapper;
    Starter             AllocStarter;
    Bookkeeper          AllocBookkeeper;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    Granulator          DebugAlloc;
#endif
    mutable LockSafe    RootLock;
    SysAllocPaged**     pArenas;
    UPInt               NumArenas;
};

extern HeapRoot* GlobalRoot;

}} // Scaleform::HeapPT

#endif
