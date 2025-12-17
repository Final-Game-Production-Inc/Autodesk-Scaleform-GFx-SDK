/**************************************************************************

PublicHeader:   Kernel
Filename    :   HeapPT_MemoryHeap.h
Content     :   Public memory heap class declaration, PageTable engine.
Created     :   October 1, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_MemoryHeap_H
#define INC_SF_Kernel_HeapPT_MemoryHeap_H

#include "../SF_MemoryHeap.h"

namespace Scaleform {

namespace HeapPT
{
    class  HeapRoot;
    class  AllocEngine;
    class  DebugStorage;
}

namespace Heap
{
    struct HeapSegment;
    class MemVisitor;
    class SegVisitor;
}


// ***** MemoryHeap
//------------------------------------------------------------------------
class MemoryHeapPT : public MemoryHeap
{
private:
    friend class HeapPT::HeapRoot;
    friend class HeapMH::RootMH;

    MemoryHeapPT();  // Explicit creation and destruction is prohibited
    virtual ~MemoryHeapPT() {}

public:

    // *** Operations with memory arenas
    //
    //--------------------------------------------------------------------
    virtual void CreateArena(UPInt arena, SysAllocPaged* sysAlloc);
    virtual void DestroyArena(UPInt arena);
    virtual bool ArenaIsEmpty(UPInt arena);

    // *** Initialization
    //
    // Creates a nested child heap; The heap should be destroyed 
    // by calling release. If child heap creation failed due to 
    // out-of-memory condition, returns 0. If child heap creation 
    // is not supported a pointer to the same parent heap will be returned.
    //--------------------------------------------------------------------
    virtual MemoryHeap* CreateHeap(const char* name, 
                                   const HeapDesc& desc);

    // *** Service functions
    //--------------------------------------------------------------------

    virtual void    SetLimitHandler(LimitHandler* handler);
    virtual void    SetLimit(UPInt newLimit);
    virtual void    AddRef();
    virtual void    Release();

    // *** Allocation API
    //--------------------------------------------------------------------
    virtual void*   Alloc(UPInt size, const AllocInfo* info = 0);
    virtual void*   Alloc(UPInt size, UPInt align, const AllocInfo* info = 0);
    virtual void*   Realloc(void* oldPtr, UPInt newSize); 
    virtual void    Free(void* ptr);
    virtual void*   AllocAutoHeap(const void *thisPtr, UPInt size,
                                  const AllocInfo* info = 0);

    virtual void*   AllocAutoHeap(const void *thisPtr, UPInt size, UPInt align,
                                  const AllocInfo* info = 0);

    virtual MemoryHeap* GetAllocHeap(const void *thisPtr);

    virtual UPInt   GetUsableSize(const void* ptr);


    virtual void*   AllocSysDirect(UPInt size);
    virtual void    FreeSysDirect(void* ptr, UPInt size);

    // *** Statistics
    //--------------------------------------------------------------------
    virtual bool    GetStats(StatBag* bag);
    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetTotalFootprint() const;
    virtual UPInt   GetUsedSpace() const;
    virtual UPInt   GetTotalUsedSpace() const;
    virtual void    GetRootStats(RootStats* stats);
    virtual void    VisitMem(Heap::MemVisitor* visitor, unsigned flags);

    virtual void    VisitRootSegments(Heap::SegVisitor* visitor);
    virtual void    VisitHeapSegments(Heap::SegVisitor* visitor) const;
    virtual void    SetTracer(HeapTracer* tracer);

private:
    //--------------------------------------------------------------------
    virtual void  destroyItself();
    virtual void  ultimateCheck();
    virtual void  releaseCachedMem();
    virtual bool  dumpMemoryLeaks();
    virtual void  checkIntegrity() const;
    virtual void  getUserDebugStats(RootStats* stats) const;

    void* allocMem(UPInt size, const AllocInfo* info);
    void* allocMem(UPInt size, UPInt align, const AllocInfo* info);
    void* allocMem(const void *thisPtr, UPInt size, const AllocInfo* info);
    void* allocMem(const void *thisPtr, UPInt size, UPInt align, const AllocInfo* info);
    void* reallocMem(Heap::HeapSegment* seg, void* oldPtr, UPInt newSize);

    //--------------------------------------------------------------------
    HeapPT::AllocEngine*  pEngine;
    HeapPT::DebugStorage* pDebugStorage;
};

} // Scaleform

#endif
