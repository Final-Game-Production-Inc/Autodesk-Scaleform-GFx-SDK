/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_SysAlloc.h
Content     :   System Allocator Interface
Created     :   2009
Authors     :   Maxim Shemanarev, Michael Antonov

Notes       :   Interface to the system allocator.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_SysAlloc_H
#define INC_SF_Kernel_SysAlloc_H

#include "SF_Types.h"

namespace Scaleform {

namespace Heap { class MemVisitor; class SegVisitor; }

// ***** SysAllocBase

// SysAllocBase is a base class for SysAllocPaged and SysAllocBase.
// This class is used to provide link-independent initialization for 
// different memory heap engines in System class and has no public APIs.
class SysAllocBase
{
    friend class System;
protected:
    virtual ~SysAllocBase() { }

    // Initializes heap system, creating and initializing GlobalHeap.
    virtual bool initHeapEngine(const void* heapDesc) { SF_UNUSED(heapDesc); return false; }
    // Shuts down heap system, clearing out global heap.
    // Returns true if there were no memory leaks (success), false in case of leaks.
    virtual bool shutdownHeapEngine() { return true; }
};


//------------------------------------------------------------------------
// ***** SysAlloc

// SysAlloc defines a memory allocation interface that developers can override
// to to provide memory for GFx; an instance of this class is typically created on
// application startup and passed into System or GFx::System constructor.
// 
// This new "malloc-friendly" interface is introduced with GFx 3.3 and replaces
// older SysAlloc implementation (now renamed to SysAllocPaged). SysAlloc
// is more memory efficient when delegating to malloc/dlmalloc based implementation,
// as it doesn't require large alignment and will return more memory blocks
// to the application once content is unloaded.
//
// Users implementing this interface must provide three functions: Alloc, Free,
// and Realloc. Implementations of these functions must honor the requested alignment.
// Although arbitrary alignment requests are possible, requested alignment will
// typically be small, such as 16 bytes or less.
//
// SysAlloc links to the MemoryHeapMH implementation, which groups small
// allocations into 4K-sized blocks belonging to the heap. Allocations larger
// then 512 bytes will delegated directly to SysAlloc and released immediately
// once no longer used, making more memory available to the application.

class SysAlloc : public SysAllocBase
{
public:
    SysAlloc() {}
    virtual ~SysAlloc() {}

    virtual void* Alloc(UPInt size, UPInt align) = 0;
    virtual void  Free(void* ptr, UPInt size, UPInt align) = 0;
    virtual void* Realloc(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align) = 0;

protected:
    // Implementation of SysAllocBase based on MemoryHeapMH.
    virtual bool initHeapEngine(const void* heapDesc);
    virtual bool shutdownHeapEngine();
};



//------------------------------------------------------------------------
// ***** SysAllocPaged

// SysAllocPaged is an abstract interface used to provide allocations to page
// based MemoryHeapPT memory system, first introduced with GFx 3.0. Unlike
// SysAlloc, this system will requires 4K page alignment on allocations and
// will in general keep larger blocks allocated. This system is best for OS-direct
// allocations, being able to take advantage of HW paging, and is also used
// to implement SysAllocStatic. If you plan to implement this interface
// by delegating to malloc/dlmalloc based API, SysAlloc is a better choice.
// 
// users can implement it to delegate allocations to their own memory allocator.
// Developers implementing SysAllocPaged need to define at least three
// functions: GetInfo, Alloc and Free.
//
// When running, GFx will allocate and free large blocks through this interface,
// managing all of the smaller allocations internally. As GFx uses 4K alignment
// for large blocks internally, it is best if this interface maps directly to OS
// implementation. When that is not the case, it is best to use 64K+ granularity
// to make sure that minimal memory is wasted on alignment.
//
// Scaleform provides several default SysAllocPaged implementations:
//  1. Memory Range Based:  SysAllocStatic.
//      - Accepts a memory range in constructor, forcing GFx to work within
//        a block of memory.                    
//  2. System Specific:     SysAllocWin32, etc.
//     - Uses OS specific implementation such as VirtualAlloc, mmap, etc
//       to allocate memory pages. This implementation is most efficient for
//       the target platform. This heap is used by default in GFxSystem if
//       you don't specify a different allocator.
//  3. Malloc version:      SysAllocPagedMalloc.
//     - Uses standard library functions such as memalign() and free(). You can
//       use the source code to these as a reference for your own implementation.
//     - It is better to use SysAlloc derived implementation insead.

class SysAllocPaged : public SysAllocBase
{
public:
    struct Info
    {
        // MinAlign is the minimum alignment that system allocator will always 
        // to apply all of the allocations, independent of whether it is requested
        // or not. For OS allocators, this will often be equal to the page size
        // of the system.
        UPInt   MinAlign;

        // MaxAlign is the maximum supported alignment that will be honored
        // by the allocator. For larger alignment requests, external granulator
        // wrapper will fake alignment by making larger allocations.
        // Set this value to 0 if you allocator supports any alignment request;
        // set this to 1 byte if it doesn't support any alignment.
        UPInt   MaxAlign;

        // Granularity is the allocation granularity that the system can handle 
        // efficiently. On Win32 it's at least 64K and must be a multiple of 64K.
        UPInt   Granularity;

        // When not null SysDirectThreshold defines the global size threshold. 
        // If the allocation size is greater or equal this threshold it's being 
        // redirected to the system, ignoring the granulator layer.
        UPInt   SysDirectThreshold;

        // If not null it MaxHeapGranularity restricts the maximal possible 
        // heap granularity. In most cases it can reduce the system memory 
        // footprint for the price of more frequent segment alloc/free operations, 
        // which slows down the allocator.
        // MaxHeapGranularity must be at least 4096 and a multiple of 4096.
        UPInt   MaxHeapGranularity;

        // HasRealloc flag tells the allocator whether or not ReallocInPlace is 
        // implemented. This is just an optimization that allows the allocator to 
        // eliminate some unnecessary operations. If ReallocInPlace is not 
        // implemented the allocator is still capable to reallocate memory but
        // moving of the data will occur more frequently.
        bool    HasRealloc;
    };
    SysAllocPaged() {}

    // Fills in information about capabilities of this SysAllocPaged implementation.
    // MemoryHeap implementation will take these values into account when making
    // allocation calls.
    virtual void    GetInfo(Info* i) const = 0;
    virtual void*   Alloc(UPInt size, UPInt align) = 0;    
    virtual bool    Free(void* ptr, UPInt size, UPInt align) = 0;

    // ReallocInPlace attempts to reallocate memory to a new size without moving it.
    // If such reallocation succeeds 'true' is returned, otherwise 'false' is returned and the
    // previous allocation remains unchanged. This function is provided as an optimization
    // for internal Realloc implementation for large blocks; it does not need to be implemented.
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
    {
        SF_UNUSED4(oldPtr, oldSize, newSize, align);
        return false;
    }

    // Not mandatory for overriding. May do nothing.
    virtual void*   AllocSysDirect(UPInt, UPInt, UPInt*, UPInt*) { return 0; }
    virtual bool    FreeSysDirect(void*, UPInt, UPInt)           { return false; }

    virtual UPInt   GetBase() const { return 0; } // DBG
    virtual UPInt   GetSize() const { return 0; } // DBG

    virtual UPInt   GetFootprint() const { return 0; }
    virtual UPInt   GetUsedSpace() const { return 0; }
    virtual void    VisitMem(Heap::MemVisitor*) const {}
    virtual void    VisitSegments(Heap::SegVisitor*, UPInt, UPInt) const {}

    virtual ~SysAllocPaged() { }


protected:
    // Implementation of SysAllocBase based on MemoryHeapPT.
    virtual bool initHeapEngine(const void* heapDesc);
    virtual bool shutdownHeapEngine();
};


//------------------------------------------------------------------------
// ***** SysAllocStatic
//
//  System allocator that works entirely in a single block of memory.
namespace HeapPT { class  AllocLite; }
namespace Heap   { class  MemVisitor; }

class SysAllocStatic : public SysAllocPaged
{
public:
    enum { MaxSegments = 4 };

    SF_EXPORT SysAllocStatic(void* mem1=0, UPInt size1=0,
                             void* mem2=0, UPInt size2=0,
                             void* mem3=0, UPInt size3=0,
                             void* mem4=0, UPInt size4=0);

    SF_EXPORT SysAllocStatic(UPInt minSize);

    SF_EXPORT void  AddMemSegment(void* mem, UPInt size);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual void    VisitMem(Heap::MemVisitor* visitor) const;

    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetUsedSpace() const;

    virtual UPInt   GetBase() const; // DBG
    virtual UPInt   GetSize() const; // DBG

private:
    UPInt               MinSize;
    UPInt               NumSegments;
    HeapPT::AllocLite*  pAllocator;
    UPInt               PrivateData[8];
    UPInt               Segments[MaxSegments][8];
    UPInt               TotalSpace;
};



//------------------------------------------------------------------------
// ***** SysAllocBase_SingletonSupport

// SysAllocBase_SingletonSupport is a SysAllocBase class wrapper that implements
// the InitSystemSingleton static function, used to create a global singleton
// used for the GFxSystem default argument initialization.
//
// End users implementing custom SysAlloc/Paged interface don't need to make use of this base
// class; they can just create an instance of their own class on stack and pass it to System.

template<class A, class B>
class SysAllocBase_SingletonSupport : public B
{
    struct SysAllocContainer
    {        
        UPInt Data[(sizeof(A) + sizeof(UPInt)-1) / sizeof(UPInt)];
        bool  Initialized;
        SysAllocContainer() : Initialized(0) { }
    };

    SysAllocContainer* pContainer;

public:
    SysAllocBase_SingletonSupport() : pContainer(0) { }

    // Creates a singleton instance of this SysAllocPaged class used
    // on SF_SYSALLOC_DEFAULT_CLASS during GFxSystem initialization.
    static  B*  InitSystemSingleton()
    {
        static SysAllocContainer Container;
        SF_ASSERT(Container.Initialized == false);

    #undef new

        SysAllocBase_SingletonSupport<A,B> *presult = ::new((void*)Container.Data) A;

    // Redefine operator 'new' if necessary.
    #if defined(SF_DEFINE_NEW)
    #define new SF_DEFINE_NEW
    #endif

        presult->pContainer = &Container;
        Container.Initialized = true;
        return presult;
    }

protected:
    virtual bool shutdownHeapEngine()
    {
        bool hasNoLeaks = B::shutdownHeapEngine();
        if (pContainer)
        {
            pContainer->Initialized = false;
            ((B*)this)->~B();
            pContainer = 0;
        }
        return hasNoLeaks;
    }
};


//------------------------------------------------------------------------
// ***** SysMemMapper
//
// SysMemMap is an abstract interface to the system memory mapping such as
// WinAPI VirtualAlloc/VirtualFree or posix mmap/munmap. It's used with
// SysAllocMemoryMap that takes a full advantage of the system memory mapping
// interface.
class SysMemMapper
{
public:
    virtual UPInt   GetPageSize() const = 0;

    // Reserve and release address space. The functions must not allocate
    // any actual memory. The size is typically very large, such as 128 
    // megabytes. ReserveAddrSpace() returns a pointer to the reserved
    // space; there is no usable memory is allocated.
    //--------------------------------------------------------------------
    virtual void*   ReserveAddrSpace(UPInt size) = 0;
    virtual bool    ReleaseAddrSpace(void* ptr, UPInt size) = 0;

    // Map and unmap memory pages to allocate actual memory. The caller 
    // guarantees the integrity of the calls. That is, the the size is 
    // always a multiple of GetPageSize() and the ptr...ptr+size is always 
    // within the reserved address space. Also it's guaranteed that there 
    // will be no MapPages call for already mapped memory and no UnmapPages 
    // for not mapped memory.
    //--------------------------------------------------------------------
    virtual void*   MapPages(void* ptr, UPInt size) = 0;
    virtual bool    UnmapPages(void* ptr, UPInt size) = 0;

    virtual ~SysMemMapper() {}
};

} // Scaleform

#endif
