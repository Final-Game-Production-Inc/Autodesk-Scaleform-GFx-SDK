/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_MemoryHeap.h
Content     :   Public memory heap class declaration.
Created     :   October 1, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_MemoryHeap_H
#define INC_SF_Kernel_MemoryHeap_H

#include "SF_Types.h"
#include "SF_Atomic.h"
#include "SF_Stats.h"
#include "SF_List.h"
#include "SF_SysAlloc.h"
#include "SF_AllocInfo.h"

namespace Scaleform {

// *** Heap-Specific Statistics identifiers
//------------------------------------------------------------------------
enum StatHeap
{    
    // StatHeap_Summary is a root stat group for all of heap-custom statistics returned by StatHeap.
    StatHeap_Summary = StatHeap_Start,
        StatHeap_TotalFootprint,       // Total memory allocated for heap and children.
        StatHeap_LocalFootprint,       // Total memory allocated for heap.
        StatHeap_ChildFootprint,       // Total footprint of all child heaps.
        StatHeap_ChildHeaps,           // The number of child heaps.
        StatHeap_LocalUsedSpace,       // Used memory within footprint.
        StatHeap_SysDirectSpace,       // Memory allocated directly from SysAlloc.
        StatHeap_Bookkeeping,          // Amount of memory used for heap bookkeeping.
        StatHeap_DebugInfo,            // Amount of memory used for heap debug information.
        StatHeap_Segments,             // Number of allocated segments.
        StatHeap_Granularity,          // Specified heap granularity.
        StatHeap_DynamicGranularity,   // Current dynamic granularity.
        StatHeap_Reserve               // Heap reserve.
        // 3 slots left. Check/modify StatGroup in "Stat.h" for more.
};

namespace HeapPT { class HeapRoot; }
namespace HeapMH { class RootMH; }
namespace Heap   { class MemVisitor; class SegVisitor; }

// ***** MemoryHeap
//------------------------------------------------------------------------
//
// Memory heap from which allocations take place. This interface supports
// creation of child heaps for dedicated purposes, such that when child
// heaps are released all of their memory is released as well. The benefit
// of child heaps is that they grab memory in large chunks of Granularity
// from the root, such that all of heap-local allocations take place in
// those chunks. This strategy reduces overall fragmentation, since once
// child heap is released it will make all of its large chunks available
// for reuse.
//
// The child memory heap use in Scaleform relies on ability to determine 
// target heap based on the address within allocation in it, done by 
// AllocAutoHeap function. This function takes an address that must be 
// within some other allocation of the desired heap and allocated new memory 
// within the same heap as that allocation. Note that the passed address 
// does NOT have to point to the head of previous allocation, it can also 
// be in the middle of it. This functionality allows convenient initialization 
// of aggregate structures and arrays located within other objects, their 
// allocations will automatically take place within the same heap as the 
// containing object.
//
// Internally, heaps rely on SysAlloc interface for allocating large blocks 
// of system memory; this interface is specified in Scaleform::System constructor.
// Developers can subsitute Scaleform allocator by re-implementing that interface. 

// *** Predeclarations
//------------------------------------------------------------------------

class MemoryHeap : public ListNode<MemoryHeap>
{
    friend class MemoryHeapPT;
    friend class MemoryHeapMH;
public:
    enum MemReportType
    {
        MemReportBrief,
        MemReportFull,
        MemReportFileSummary,
        MemReportHeapDetailed
    };

    enum HeapFlags
    {        
        Heap_ThreadUnsafe       = 0x0001,

        // Significantly speeds up small allocations, up to 8*MinAlign,
        // but may result in higher memory overhead. The smallest heap
        // size will typically increase by about 8*PageSize. For the 
        // PageSize=4K it's 32K. If there are thousands of small 
        // allocations existing simultaneously, the extra memory 
        // overhead is low.
        Heap_FastTinyBlocks     = 0x0002,

        // Fixed Granularity means that memory segments will have as small
        // size as possible. It slightly reduces the total footprint and 
        // eases returning memory to the system, but may slow down the
        // allocations. It also makes the heap less realloc-friendly.
        Heap_FixedGranularity   = 0x0004,

        // This flag is set if this is the root (global) memory heap.
        Heap_Root               = 0x0008,

        // Set if this heap keeps debug data in an alternative heap.
        // If not, then this heap does not track debug information.
        Heap_NoDebugInfo        = 0x0010,

        // This flag can be set by the user for debug tool allocations, such as the
        // player HUD.  Scaleform tools can choose to omit these heaps from information
        // reporting or report them separately. This flag is for user information only,
        // it has no effect on heap implementation.
        Heap_UserDebug          = 0x1000
    };

    enum RootHeapParameters
    {
        RootHeap_MinAlign       = 16,
        RootHeap_Granularity    = 16*1024,
        RootHeap_Reserve        = 16*1024,
        RootHeap_Threshold      = 256*1024,
        RootHeap_Limit          = 0
    };

    struct HeapDesc
    {
        // Capability flags such as thread safety, debug support, etc.
        unsigned        Flags;

        // Minimum alignment that will be enforced for all allocations from heap.
        UPInt           MinAlign;

        // Granularity represents the smallest block size that the heap
        // will request from the "root" heap memory system, which ultimately
        // routes to SysAlloc. In other words, this is the size multiple by
        // which the heap will grow. For example, with the default value of 16K,
        // the first allocation of 10 bytes on the heap will allocate a 16K block;
        // from that point on all of the further heap allocations will be serviced from
        // that block until it is exhausted. Once exhausted, an additional block of
        // granularity will be allocated. If all of the allocation within a block
        // are free, it will similarly be returned to root heap manager for reuse
        // in other heaps or return to SysAlloc.
        UPInt           Granularity;

        // Smallest maintained reserve, or pre-allocated reserve. Data within
        // the reserve will not be given back to parent heap or system until
        // the heap is released. Must be multiple of Granularity.
        UPInt           Reserve;

        // If the allocation size exceeds this threshold the allocation
        // is being redirected directly to SysAlloc.
        UPInt           Threshold;

        // If not 0, specifies the limit on the amount of memory used by heap.
        // Any attempt to allocate more will fail returning null. Must be
        // multiple of Granularity.
        UPInt           Limit;

        // An arbitrary integer to associate the heap with some additional
        // information. In particular, the Id is associated with the color to
        // be displayed in the memory monitor. If the Id is 0 there is no 
        // associations, but the heap will be displayed anyway. The only 
        // difference is persistence. Heaps with IDs will have guaranteed 
        // persistent colors to be easily distinguishable from other ones.
        UPInt           HeapId;

        // Memory arena used for this heap. The arena must be created, see
        // MemoryHeap::CreateArena. Arena 0 is reserved for the use by default.
        UPInt           Arena;

        HeapDesc(unsigned  flags = 0,
                 UPInt minAlign = 16,
                 UPInt granularity = 8*1024,
                 UPInt reserve = 8*1024, 
                 UPInt threshold=~UPInt(0),
                 UPInt limit = 0,
                 UPInt heapId = 0,
                 UPInt arena = 0)
            : Flags(flags), MinAlign(minAlign),
              Granularity(granularity), Reserve(reserve),
              Threshold(threshold), Limit(limit), 
              HeapId(heapId), Arena(arena)
        { }

        void Clear()
        {
            Flags       = 0;
            Granularity = 0;
            MinAlign    = 16;
            Reserve     = 0;
            Threshold   = ~UPInt(0);
            Limit       = 0;
            HeapId      = 0;
            Arena       = 0;
        }
    };

    // Root heap descriptor. This class exists for the sole purpose of using
    // its default constructor for initializing a root heap object, as root default
    // arguments are different from those of child heaps.
    struct RootHeapDesc : public HeapDesc
    {
        RootHeapDesc()
            : HeapDesc(0, RootHeap_MinAlign,
                          RootHeap_Granularity, RootHeap_Reserve,
                          RootHeap_Threshold, RootHeap_Limit, HeapId_Global)
        { }
    };

    //--------------------------------------------------------------------
    struct HeapInfo
    {
        HeapDesc        Desc;

        // Parent heap, if this describes a nested heap. Null for root heap.
        MemoryHeap*     pParent;

        // Heap name. Can be in UTF-8.
        char*           pName;
    };

    //--------------------------------------------------------------------
    struct HeapVisitor
    {
        virtual ~HeapVisitor() { }

        // Called for each child heap within parent; the child heap is
        // guaranteed to stay alive during the call as long as parent is alive too.
        // Implementation of Visit is not allowed to allocate memory
        // from childHeap due to a risk of deadlock (it can allocate from
        // parent or other heaps).
        virtual void Visit(MemoryHeap* parentHeap, MemoryHeap *childHeap) = 0;
    };

    //--------------------------------------------------------------------
    struct LimitHandler
    {
        LimitHandler():AllocCount(0) {}

    	virtual ~LimitHandler() {}
        // The handler that is called when the limit is reached. The handler
        // can try to free memory of at least "overLimit" size in summary 
        // (release cached elements, invoke GC, etc).
        // If freeing is not possible or there's not enough size of elements
        // being released the function can increase the heap limit 
        // (MemoryHeap::SetLimit). In both cases it should return true.
        // If neither is possible the function returns false and then the 
        // allocation call fails (returns 0).
        virtual bool OnExceedLimit(MemoryHeap* heap, UPInt overLimit) = 0;

        // The function is called when the segment is freeing. It allows the
        // application algorithm to decrease the limit when necessary.
        virtual void OnFreeSegment(MemoryHeap* heap, UPInt freeingSize) = 0;

        bool IsInsideAlloc() const { return AllocCount != 0; }

        int AllocCount;
    };

    //--------------------------------------------------------------------
    struct HeapTracer
    {
    	virtual ~HeapTracer() {}
        virtual void OnCreateHeap(const MemoryHeap* heap) = 0;
        virtual void OnDestroyHeap(const MemoryHeap* heap) = 0;
        virtual void OnAlloc(const MemoryHeap* heap, UPInt size, UPInt align, unsigned sid, const void* ptr) = 0;
        virtual void OnRealloc(const MemoryHeap* heap, const void* oldPtr, UPInt newSize, const void* newPtr) = 0;
        virtual void OnFree(const MemoryHeap* heap, const void* ptr) = 0;
    };

    //--------------------------------------------------------------------
    struct RootStats
    {
        UPInt SysMemFootprint;
        UPInt SysMemUsedSpace;
        UPInt PageMapFootprint;
        UPInt PageMapUsedSpace;
        UPInt BookkeepingFootprint;
        UPInt BookkeepingUsedSpace;
        UPInt DebugInfoFootprint;
        UPInt DebugInfoUsedSpace;
        UPInt UserDebugFootprint;
        UPInt UserDebugUsedSpace;
    };

protected:
    MemoryHeap();  // Explicit creation and destruction is prohibited
    virtual ~MemoryHeap() {}

public:

    // *** Initial bootstrapping and final clean-up class factory functions
    //
    //--------------------------------------------------------------------
    static void SF_STDCALL InitPT(SysAllocPaged* sysAlloc);
    static void SF_STDCALL InitMH(SysAlloc* sysAllocMH);
    static void SF_STDCALL CleanUpPT();
    static void SF_STDCALL CleanUpMH();

    // Creates the root heap. The function can be called only once. 
    // The second call will return 0. Call ReleaseRootHeap before creating 
    // another root heap.
    static MemoryHeap* SF_STDCALL CreateRootHeapPT();
    static MemoryHeap* SF_STDCALL CreateRootHeapMH();
    static MemoryHeap* SF_STDCALL CreateRootHeapPT(const HeapDesc& desc);
    static MemoryHeap* SF_STDCALL CreateRootHeapMH(const HeapDesc& desc);

    // Releases root heap and/or de-initializes it; intended to be the opposite
    // of the heap-specific CreateRootHeap() static function. ReleaseRootHeap is
    // called during System::Destroy before the global heap is reset.
    // Its implementation may or may not include ref-counting semantics.
    // Returns true for clean shutdown; false if there were memory leaks.
    static bool SF_STDCALL ReleaseRootHeapPT();
    static bool SF_STDCALL ReleaseRootHeapMH();


    // *** Operations with memory arenas
    //
    //--------------------------------------------------------------------
    virtual void CreateArena(UPInt arena, SysAllocPaged* sysAlloc) = 0;
    virtual void DestroyArena(UPInt arena) = 0;
    virtual bool ArenaIsEmpty(UPInt arena) = 0;

    // *** Initialization
    //
    // Creates a nested child heap; The heap should be destroyed 
    // by calling release. If child heap creation failed due to 
    // out-of-memory condition, returns 0. If child heap creation 
    // is not supported a pointer to the same parent heap will be returned.
    //--------------------------------------------------------------------
    virtual MemoryHeap* CreateHeap(const char* name, 
                                   const HeapDesc& desc) = 0;

            MemoryHeap* CreateHeap(const char* name, 
                                   unsigned  flags = 0,
                                   UPInt minAlign = 16,
                                   UPInt granularity = 16*1024,
                                   UPInt reserve = 16*1024, 
                                   UPInt threshold=~UPInt(0), 
                                   UPInt limit = 0, 
                                   UPInt heapId = 0,
                                   UPInt arena = 0)
    {
        HeapDesc desc(flags, minAlign, granularity, reserve, threshold, limit, heapId, arena);
        return CreateHeap(name, desc);
    }

    // *** Service functions
    //--------------------------------------------------------------------

    // Fills in heap descriptor with information.
    void            GetHeapInfo(HeapInfo* infoPtr) const;

    const char*     GetName()       const { return Info.pName;      }
    UPInt           GetId()         const { return Info.Desc.HeapId;}
    MemoryHeap*     GetParentHeap() const { return Info.pParent;    }
    unsigned        GetFlags()      const { return Info.Desc.Flags; }
    UPInt           GetGranularity()const { return Info.Desc.Granularity; }

    virtual void    SetLimitHandler(LimitHandler* handler) = 0;
    virtual void    SetLimit(UPInt newLimit) = 0;
            UPInt   GetLimit() const { return Info.Desc.Limit; }

    // Determines if the heap is thread safe or not. One benefit of thread safe heap
    // is that its stats can be obtained from another thread during child heap iteration.
    inline  bool IsThreadSafe() const 
    { 
        return (Info.Desc.Flags & Heap_ThreadUnsafe) == 0; 
    }

    // Child heap lifetime is reference counted.
    //  These function need to be implemented when child heaps are supported.

    // Increments heap reference count; this function is primarily useful
    // when enumerating heaps.
    virtual void    AddRef() = 0;

    // Releases this heap, deallocating it unless there are other references.
    // Other references can come either from child heaps or multiple external
    // references to the heap.
    // Internal allocations are NOT considered references for heap lifetime.
    // Release should only be called for heaps created with CreateHeap.
    virtual void    Release() = 0;

    // Marks allocation for automatic cleanup of heap. When this allocation
    // is freed, the entire heap will be released. Allocation must belong
    // to this heap, and should usually be the last item to be freed.
    void            ReleaseOnFree(void *ptr);

    // Assign heap to current thread causing ASSERTs if called for other thread
    void            AssignToCurrentThread();

    // *** Allocation API
    //--------------------------------------------------------------------
    virtual void*   Alloc(UPInt size, const AllocInfo* info = 0) = 0;
    virtual void*   Alloc(UPInt size, UPInt align, const AllocInfo* info = 0) = 0;

    // Reallocates memory; this call does not maintain custom alignment specified
    // during allocation.
    virtual void*   Realloc(void* oldPtr, UPInt newSize) = 0; 
    virtual void    Free(void* ptr) = 0;

    // Allocate while automatically identifying heap and allocation id based on 
    // the specified address.
    virtual void*   AllocAutoHeap(const void *thisPtr, UPInt size,
                                  const AllocInfo* info = 0) = 0;


    virtual void*   AllocAutoHeap(const void *thisPtr, UPInt size, UPInt align,
                                  const AllocInfo* info = 0) = 0;

    // Determine which heap allocation belongs to. 
    // This function will ASSERT internally if the specified
    // address does not come from one of allocated Scaleform heaps.
    virtual MemoryHeap* GetAllocHeap(const void *thisPtr) = 0;

    // Returns the actual allocation size that can be safely used.
    virtual UPInt   GetUsableSize(const void* ptr) = 0;

    // Alloc and free directly from the system allocator. These functions
    // are used only for debugging and/or visualization when it's necessary
    // to allocate some large amount of memory with absolute minimal 
    // interference with the existing memory layout. Address alignment
    // is only guaranteed to be SysAlloc::Info::MinAlign. These functions 
    // must be used with care!
    virtual void* AllocSysDirect(UPInt size) = 0;
    virtual void  FreeSysDirect(void* ptr, UPInt size) = 0;

    // *** Statistics
    //
    //--------------------------------------------------------------------
    // Obtains Memory statistics for the heap. Returns false if the 
    // statistics is not supported.
    virtual bool GetStats(StatBag* bag) = 0;

    // Return the number of bytes allocated from the system
    // and the number of actually allocated bytes in the heap.
    // GetTotalUsedSpace() recursively iterates through all child 
    // heaps and sums up the total used space.
    virtual UPInt   GetFootprint() const = 0;
    virtual UPInt   GetTotalFootprint() const = 0;
    virtual UPInt   GetUsedSpace() const = 0;
    virtual UPInt   GetTotalUsedSpace() const = 0;
    virtual void    GetRootStats(RootStats* stats) = 0;


    // DBG: To be removed in future.
    // See MemVisitor::VisitingFlags for "flags" argument.
    virtual void    VisitMem(Heap::MemVisitor* visitor, unsigned flags) = 0;

    virtual void    VisitRootSegments(Heap::SegVisitor* visitor) = 0;
    virtual void    VisitHeapSegments(Heap::SegVisitor* visitor) const = 0;

    virtual void    SetTracer(HeapTracer* tracer) = 0;

    // Forming the String containing general or detailed information about heaps.
    void  MemReport(class StringBuffer& buffer, MemReportType detailed, bool xmlFormat = false);
    void  MemReport(struct MemItem* rootItem, MemReportType detailed);

    // Traverse all child heaps and dump memory leaks. If no debug info 
    // is present just report alive child heaps. Returns true if memory 
    // leaks have been detected.
    bool    DumpMemoryLeaks();

    void    UltimateCheck();

    // Enumerates all of the child heaps, by calling HeapVisitor::Visit.
    void    VisitChildHeaps(HeapVisitor* visitor);
    void    LockAndVisit(HeapVisitor* visitor);
    void    CheckIntegrity();

    // The heap can hold a memory segment in its cache to reduce system 
    // memory thrashing. This function releases the cached segments from 
    // all heaps. It makes sense to call this function after some big 
    // unloads to return as much memory to the system as possible.
    void    ReleaseCachedMem();

    int     CheckRead(int p) const;

protected:
    virtual void  destroyItself() = 0;
    virtual void  ultimateCheck() = 0;
    virtual void  releaseCachedMem() = 0;
    virtual bool  dumpMemoryLeaks() = 0;
    virtual void  checkIntegrity() const = 0;
    virtual void  getUserDebugStats(RootStats* stats) const = 0;

    typedef List<MemoryHeap> ChildListType;

    UPInt               SelfSize;
    volatile unsigned   RefCount;
    UPInt               OwnerThreadId;

    // Pointer to allocation that will cause this heap
    // to be automatically released when freed.
    void*               pAutoRelease;

    HeapInfo            Info;
    ChildListType       ChildHeaps;
    mutable Lock        HeapLock;
    bool                UseLocks;
    bool                TrackDebugInfo;
};

} // Scaleform

#endif
