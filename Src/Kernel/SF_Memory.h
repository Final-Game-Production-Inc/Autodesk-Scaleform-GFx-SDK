/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Memory.h
Content     :   General purpose memory allocation functions
Created     :   December 28, 1998 - 2009
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Memory_H
#define INC_SF_Kernel_Memory_H

#include "SF_Types.h"
#include "SF_MemoryHeap.h"

// Unified SysAlloc. New malloc-friendly allocator
#include "HeapMH/HeapMH_SysAllocMalloc.h"
#define SF_SYSALLOC_DEFAULT_CLASS Scaleform::SysAllocMalloc

/*
// Include the right allocation header for the platform involved.
// This is ok even if users use custom allocators, as having the headers
// included here does NOT force the code to be linked.
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_OS_WINCE)
    #include "HeapPT/HeapPT_SysAllocWinAPI.h"
    #define  SF_SYSALLOC_DEFAULT_CLASS SysAllocWinAPI
    #include "HeapPT/HeapPT_SysAllocMalloc.h"
    //#define  SF_SYSALLOC_DEFAULT_CLASS SysAllocMalloc
#elif defined(SF_OS_PS3)
    #include "HeapPT/HeapPT_SysAllocPS3.h"
    #define  SF_SYSALLOC_DEFAULT_CLASS SysAllocPS3
#elif defined(SF_OS_WII)
    #include "HeapPT/HeapPT_SysAllocWii.h"
    #define  SF_SYSALLOC_DEFAULT_CLASS SysAllocWii
#elif defined(SF_OS_3DS)
    #include "HeapPT/HeapPT_SysAlloc3DS.h"
    #define  SF_SYSALLOC_DEFAULT_CLASS SysAlloc3DS
#else
    #include "HeapMH/HeapMH_SysAllocMalloc.h"
    #define  SF_SYSALLOC_DEFAULT_CLASS SysAllocPagedMalloc
#endif
*/


// Define this macro to enable memory tracking; since tracking is not free
// this variable probably should not be defined for a release build.
//#define   SF_MEMORY_TRACKSIZES

// This file requires operator 'new' to NOT be defined; its definition
// is restored in the bottom of the header based on SF_DEFINE_NEW.
#undef new

// ***** Memory Allocation Macros

// These macros are used for allocation to ensure that the allocation location
// filename and line numbers are recorded in debug builds. The heap-based
// operator new versions are declared in a separate header file "HeapNew.h".

#if !defined(SF_MEMORY_ENABLE_DEBUG_INFO)

    // Allocate from global heap.
    #define SF_ALLOC(s,id)                    Scaleform::Memory::Alloc((s))
    #define SF_MEMALIGN(s,a,id)               Scaleform::Memory::Alloc((s),(a))
    #define SF_REALLOC(p,s,id)                Scaleform::Memory::Realloc((p),(s))
    #define SF_FREE(p)                        Scaleform::Memory::Free((p))
    #define SF_FREE_ALIGN(s)                  Scaleform::Memory::Free((s))
    // Heap versions.
    #define SF_HEAP_ALLOC(heap,s,id)          Scaleform::Memory::AllocInHeap((heap),(s))
    #define SF_HEAP_MEMALIGN(heap,s,a,id)     Scaleform::Memory::AllocInHeap((heap),(s),(a))
    #define SF_HEAP_AUTO_ALLOC(addr,s)        Scaleform::Memory::AllocAutoHeap((addr),(s))
    #define SF_HEAP_AUTO_ALLOC_ID(addr,s,id)  Scaleform::Memory::AllocAutoHeap((addr),(s))
    #define SF_HEAP_FREE(heap, p)             Scaleform::Memory::Free((p))
#else
    #define SF_ALLOC(s,id)                    Scaleform::Memory::Alloc((s),     Scaleform::AllocInfo((id),__FILE__,__LINE__))
    #define SF_MEMALIGN(s,a,id)               Scaleform::Memory::Alloc((s),(a), Scaleform::AllocInfo((id),__FILE__,__LINE__))
    #define SF_REALLOC(p,s,id)                Scaleform::Memory::Realloc((p),(s))
    #define SF_FREE(p)                        Scaleform::Memory::Free((p))
    #define SF_FREE_ALIGN(s)                  Scaleform::Memory::Free((s))
    // Heap versions.
    #define SF_HEAP_ALLOC(heap,s,id)          Scaleform::Memory::AllocInHeap((heap),(s),    Scaleform::AllocInfo((id),__FILE__,__LINE__))
    #define SF_HEAP_MEMALIGN(heap,s,a,id)     Scaleform::Memory::AllocInHeap((heap),(s),(a),Scaleform::AllocInfo((id),__FILE__,__LINE__))
    #define SF_HEAP_AUTO_ALLOC(addr,s)        Scaleform::Memory::AllocAutoHeap((addr),(s),  Scaleform::AllocInfo(Stat_Default_Mem,__FILE__,__LINE__))
    #define SF_HEAP_AUTO_ALLOC_ID(addr,s,id)  Scaleform::Memory::AllocAutoHeap((addr),(s),  Scaleform::AllocInfo((id),__FILE__,__LINE__))
    #define SF_HEAP_FREE(heap, p)             Scaleform::Memory::Free((p))

#endif //!defined(SF_BUILD_DEBUG)

namespace Scaleform {

// ***** Memory Class

// Maintains current heap and the global allocator, wrapping heap access functions.
// The main purpose of wrapping is to allow GAllocDebugInfo temporary to be converted
// from a constant reference to an optional pointer argument for the allocation
// functions in MemoryHeap.

class Memory
{
public:
    static MemoryHeap *pGlobalHeap;


    // May need to export this later and check allocation status.
    static void         SF_STDCALL SetGlobalHeap(MemoryHeap *heap) { pGlobalHeap = heap; }
    static MemoryHeap*  SF_STDCALL GetGlobalHeap()                 { return pGlobalHeap; }

    // *** Operations with memory arenas
    //--------------------------------------------------------------------
    static void SF_STDCALL CreateArena(UPInt arena, SysAllocPaged* sysAlloc) { pGlobalHeap->CreateArena(arena, sysAlloc); }
    static void SF_STDCALL DestroyArena(UPInt arena)                         { pGlobalHeap->DestroyArena(arena); }
    static bool SF_STDCALL ArenaIsEmpty(UPInt arena)                         { return pGlobalHeap->ArenaIsEmpty(arena); }

    // *** Memory Allocation
    // Memory::Alloc of size==0 will allocate a tiny block & return a valid pointer;
    // this makes it suitable for new operator.
    static void* SF_STDCALL Alloc(UPInt size)                                       { return pGlobalHeap->Alloc(size); }
    static void* SF_STDCALL Alloc(UPInt size, UPInt align)                          { return pGlobalHeap->Alloc(size, align); }
    static void* SF_STDCALL Alloc(UPInt size, const AllocInfo& info)                { return pGlobalHeap->Alloc(size, &info); }
    static void* SF_STDCALL Alloc(UPInt size, UPInt align, const AllocInfo& info)   { return pGlobalHeap->Alloc(size, align, &info); }

    // Allocate while automatically identifying heap and allocation id based on the specified address.
    static void* SF_STDCALL AllocAutoHeap(const void *p, UPInt size)                                     { return pGlobalHeap->AllocAutoHeap(p, size); }
    static void* SF_STDCALL AllocAutoHeap(const void *p, UPInt size, UPInt align)                        { return pGlobalHeap->AllocAutoHeap(p, size, align); }
    static void* SF_STDCALL AllocAutoHeap(const void *p, UPInt size, const AllocInfo& info)              { return pGlobalHeap->AllocAutoHeap(p, size, &info); }
    static void* SF_STDCALL AllocAutoHeap(const void *p, UPInt size, UPInt align, const AllocInfo& info) { return pGlobalHeap->AllocAutoHeap(p, size, align, &info); }

    // Allocate in a specified heap. The later functions are provided to ensure that 
    // AllocInfo can be passed by reference with extended lifetime.
    static void* SF_STDCALL AllocInHeap(MemoryHeap* heap, UPInt size)                                       { return heap->Alloc(size); }
    static void* SF_STDCALL AllocInHeap(MemoryHeap* heap, UPInt size, UPInt align)                          { return heap->Alloc(size, align); }
    static void* SF_STDCALL AllocInHeap(MemoryHeap* heap, UPInt size, const AllocInfo& info)                { return heap->Alloc(size, &info); }
    static void* SF_STDCALL AllocInHeap(MemoryHeap* heap, UPInt size, UPInt align, const AllocInfo& info)   { return heap->Alloc(size, align, &info); }

    
    // Reallocate to a new size, 0 return == can't reallocate, previous memory is still valid
    // 
    // Realloc to decrease size will never fail
    // Realloc of pointer == 0 is equivalent to Memory::Alloc
    // Realloc of size == 0, shrinks to the minimal size, pointer remains valid and requires Free().
    // NOTE: Realloc guarantees the alignment specified in Alloc() 
    static void* SF_STDCALL Realloc(void *p, UPInt newSize)         { return pGlobalHeap->Realloc(p, newSize); }
   
    // Free allocated/reallocated block
    static void         SF_STDCALL Free(void *p)                    { return pGlobalHeap->Free(p); }

    static MemoryHeap*  SF_STDCALL GetHeapByAddress(const void* p)  { return pGlobalHeap->GetAllocHeap(p); }
    static bool         SF_STDCALL DetectMemoryLeaks()              { return pGlobalHeap->DumpMemoryLeaks(); }
};

// ***** Macros to redefine class new/delete operators

// Types specifically declared to allow disambiguation of address in
// class member operator new. Used in HeapNew.h.

struct MemAddressStub { };
typedef MemAddressStub* MemAddressPtr;

#define SF_MEMORY_REDEFINE_NEW_IMPL(class_name, check_delete, StatType)                                  \
    void*   operator new(UPInt sz)                                                                       \
    { void *p = SF_ALLOC(sz, StatType); return p; }                                                      \
    void*   operator new(UPInt sz, Scaleform::MemoryHeap* heap)                                                     \
    { void *p = SF_HEAP_ALLOC(heap, sz, StatType); return p; }                                           \
    void*   operator new(UPInt sz, Scaleform::MemoryHeap* heap, int blocktype)                                      \
    { SF_UNUSED(blocktype); void *p = SF_HEAP_ALLOC(heap, sz, blocktype); return p; }                    \
    void*   operator new(UPInt sz, Scaleform::MemAddressPtr adr)                                                    \
    { void *p = Scaleform::Memory::AllocAutoHeap(adr, sz, Scaleform::AllocInfo(StatType,__FILE__,__LINE__)); return p; }       \
    void*   operator new(UPInt sz, const char* pfilename, int line)                                      \
    { void* p = Scaleform::Memory::Alloc(sz, Scaleform::AllocInfo(StatType, pfilename, line)); return p; }                     \
    void*   operator new(UPInt sz, Scaleform::MemoryHeap* heap, const char* pfilename, int line)                    \
    { void* p = Scaleform::Memory::AllocInHeap(heap, sz, Scaleform::AllocInfo(StatType, pfilename, line)); return p; }         \
    void*   operator new(UPInt sz, Scaleform::MemAddressPtr adr, const char* pfilename, int line)                   \
    { void* p = Scaleform::Memory::AllocAutoHeap(adr, sz, Scaleform::AllocInfo(StatType, pfilename, line)); return p; }        \
    void*   operator new(UPInt sz, int blocktype, const char* pfilename, int line)                       \
    { void* p = Scaleform::Memory::Alloc(sz, Scaleform::AllocInfo(blocktype, pfilename, line)); return p; }                    \
    void*   operator new(UPInt sz, Scaleform::MemoryHeap* heap, int blocktype, const char* pfilename, int line)     \
    { void* p = Scaleform::Memory::AllocInHeap(heap, sz, Scaleform::AllocInfo(blocktype, pfilename, line)); return p; }        \
    void*   operator new(UPInt sz, Scaleform::MemAddressPtr adr, int blocktype, const char* pfilename, int line)    \
    { void* p = Scaleform::Memory::AllocAutoHeap(adr, sz, Scaleform::AllocInfo(blocktype, pfilename, line)); return p; }       \
    void    operator delete(void *p)                                        \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, const char*, int)                      \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, int, const char*, int)                 \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemoryHeap*)                           \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemoryHeap*, int)                      \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemoryHeap*, const char*, int)         \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemoryHeap*, int,const char*,int)      \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemAddressPtr)                         \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemAddressPtr, int)                    \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemAddressPtr, const char*, int)       \
    { check_delete(class_name, p); SF_FREE(p); }                            \
    void    operator delete(void *p, Scaleform::MemAddressPtr,int,const char*,int)     \
    { check_delete(class_name, p); SF_FREE(p); }

#define SF_MEMORY_DEFINE_PLACEMENT_NEW                                                  \
    void*   operator new        (UPInt n, void *ptr)    { SF_UNUSED(n); return ptr; }   \
    void    operator delete     (void *ptr, void *ptr2) { SF_UNUSED2(ptr,ptr2); }


#define SF_MEMORY_CHECK_DELETE_NONE(class_name, p)

// Redefined all delete/new operators in a class without custom memory initialization
#define SF_MEMORY_REDEFINE_NEW(class_name, StatType) SF_MEMORY_REDEFINE_NEW_IMPL(class_name, SF_MEMORY_CHECK_DELETE_NONE, StatType)

// Base class that overrides the new and delete operators.
// Deriving from this class, even as a multiple base, incurs no space overhead.
template<int Stat>
class NewOverrideBase
{
public:
    enum { StatType = Stat };

    // Redefine all new & delete operators.
    SF_MEMORY_REDEFINE_NEW(NewOverrideBase, Stat)
};


} // Scaleform



// *** Restore operator 'new' Definition

// If users specified SF__BUILD_DEFINE_NEW in preprocessor settings, use that definition.
#if defined(SF_BUILD_DEFINE_NEW) && !defined(SF_DEFINE_NEW)
#define SF_DEFINE_NEW new(__FILE__,__LINE__)
#endif
// Redefine operator 'new' if necessary.
#if defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif


#endif // INC_GMEMORY_H
