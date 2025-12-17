/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Allocator.h
Content     :   Template allocators, constructors and functions.
Created     :   2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Allocator_H
#define INC_SF_Kernel_Allocator_H

#include "SF_Types.h"
#include "SF_Memory.h"
#include <string.h>


// ***** Disable template-unfriendly MS VC++ warnings
#if defined(SF_CC_MSVC)
#pragma warning(disable : 4786)
// In MSVC 7.1, warning about placement new POD default initializer
#pragma warning(disable : 4345)
#endif


// ***** Add support for placement new

// Calls constructor on own memory created with "new(ptr) type"
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

#if defined(SF_CC_MWERKS) || defined(SF_CC_BORLAND)
#include <new>
#else

#ifndef SF_CC_GNU
SF_INLINE void*   operator new        (UPInt n, void *ptr)   { return ptr; }
SF_INLINE void    operator delete     (void *ptr, void *ptr2) { }
#else
#include <new>
#endif // SF_CC_GNU

#endif // SF_CC_MWERKS | SF_CC_BORLAND

#endif // __PLACEMENT_NEW_INLINE


#undef new

namespace Scaleform {

// ***** Construct / Destruct

template <class T>
SF_INLINE T*  Construct(void *p)
{
    return ::new(p) T;
}

template <class T>
SF_INLINE T*  Construct(void *p, const T& source)
{
    return ::new(p) T(source);
}

// Same as above, but allows for a different type of constructor.
template <class T, class S>
SF_INLINE T*  ConstructAlt(void *p, const S& source)
{
    return ::new(p) T(source);
}

template <class T, class S1, class S2>
SF_INLINE T*  ConstructAlt(void *p, const S1& src1, const S2& src2)
{
    return ::new(p) T(src1, src2);
}

template <class T>
SF_INLINE void ConstructArray(void *p, UPInt count)
{
    UByte *pdata = (UByte*)p;
    for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
    {
        Construct<T>(pdata);
    }
}

template <class T>
SF_INLINE void ConstructArray(void *p, UPInt count, const T& source)
{
    UByte *pdata = (UByte*)p;
    for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
    {
        Construct<T>(pdata, source);
    }
}

template <class T>
SF_INLINE void Destruct(T *pobj)
{
    SF_UNUSED(pobj);
    pobj->~T();
}

template <class T>
SF_INLINE void DestructArray(T *pobj, UPInt count)
{   
    for (UPInt i=0; i<count; ++i, ++pobj)
        pobj->~T();
}



// ***** Local Allocator

// Local allocator allocates memory for containers which are a part
// of other data structures. This allocator determines the allocation
// heap and StatId based on the argument address; due to heap limitation
// it can not be used for stack-allocated or global values.
//  - If SID is Stat_Default_Mem, the allocator also inherits the SID
//------------------------------------------------------------------------
template<int SID = Stat_Default_Mem>
class AllocatorBaseLH
{
public:
    enum { StatId = SID };

#ifdef SF_BUILD_DEBUG
    static void*   Alloc(const void* pheapAddr, UPInt size, const char* pfile, unsigned line) 
    {         
        return Memory::AllocAutoHeap(pheapAddr, size, AllocInfo(StatId, pfile, line));
    }
    static void*   Realloc(void* p, UPInt newSize, const char* pfile, unsigned line) 
    { 
        SF_UNUSED2(pfile, line);
        return Memory::Realloc(p, newSize);
    }

#else

    static void*   Alloc(const void* pheapAddr, UPInt size, const char*, unsigned) 
    {         
        return Memory::AllocAutoHeap(pheapAddr, size, AllocInfo(StatId, 0, 0));
    }
    static void*   Realloc(void* p, UPInt newSize, const char*, unsigned) 
    { 
        return Memory::Realloc(p, newSize);
    }

#endif

    static void Free(void *p) 
    { 
        Memory::Free(p);
    }
};


// ***** Dynamic-Heap Allocator

// Dynamic-heap allocator allocates memory for containers created on 
// the stack, or anywhere else, where the heap pointer is explicitly
// specified. The difference is that AllocatorBaseLH takes any 
// pointer inside the heap, while AllocatorBaseDH requires a pointer
// to the heap itself.
//  - If SID is Stat_Default_Mem, the allocator also inherits the SID
//------------------------------------------------------------------------
template<int SID = Stat_Default_Mem>
class AllocatorBaseDH
{
public:
    enum { StatId = SID };

#ifdef SF_BUILD_DEBUG
    static void*   Alloc(const void* pheap, UPInt size, const char* pfile, unsigned line) 
    {         
        return Memory::AllocInHeap((MemoryHeap*)pheap, size, AllocInfo(StatId, pfile, line));
    }
    static void*   Realloc(void* p, UPInt newSize, const char* pfile, unsigned line) 
    { 
        SF_UNUSED2(pfile, line);
        return Memory::Realloc(p, newSize);
    }

#else

    static void*   Alloc(const void* pheap, UPInt size, const char*, unsigned) 
    {         
        return Memory::AllocInHeap((MemoryHeap*)pheap, size, AllocInfo(StatId, 0, 0));
    }
    static void*   Realloc(void* p, UPInt newSize, const char*, unsigned) 
    { 
        return Memory::Realloc(p, newSize);
    }

#endif

    static void Free(void *p) 
    { 
        Memory::Free(p);
    }
};


// ***** Global Allocator

// Allocator for items coming from GlobalHeap. StatId must be specified for useful
// memory tracking to be performed.
//------------------------------------------------------------------------
template<int SID = Stat_Default_Mem>
class AllocatorBaseGH
{
public:
    enum { StatId = SID };

#ifdef SF_BUILD_DEBUG
    static void*   Alloc(const void*, UPInt size, const char* pfile, unsigned line) 
    {         
        return Memory::Alloc(size, AllocInfo(StatId, pfile, line));
    }
    static void*   Realloc(void *p, UPInt newSize, const char* pfile, unsigned line) 
    { 
        SF_UNUSED2(pfile, line);
        return Memory::Realloc(p, newSize);
    }

#else

    static void*   Alloc(const void*, UPInt size, const char*, unsigned) 
    {         
        return Memory::Alloc(size, AllocInfo(StatId, 0, 0));
    }
    static void*   Realloc(void *p, UPInt newSize, const char*, unsigned) 
    { 
        return Memory::Realloc(p, newSize);
    }

#endif

    static void Free(void *p) 
    { 
        if (p) // allow dtor for empty static arrays after ~SF::System
            Memory::Free(p);
    }
};


// ***** Constructors, Destructors, Copiers

// Plain Old Data
//------------------------------------------------------------------------
template<class T> 
class ConstructorPOD
{
public:
    static void Construct(void *) {}
    static void Construct(void *p, const T& source) 
    { 
        *(T*)p = source;
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void *p, const S& source)
    {
        *(T*)p = source;
    }

    static void ConstructArray(void*, UPInt) {}

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte *pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            *(T*)pdata = source;
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        memcpy(p, psource, sizeof(T) * count);
    }

    static void Destruct(T*) {}
    static void DestructArray(T*, UPInt) {}

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static bool IsMovable() { return true; }
};


// ***** ConstructorMov
//
// Correct C++ construction and destruction for movable objects
//------------------------------------------------------------------------
template<class T> 
class ConstructorMov
{
public:
    static void Construct(void* p) 
    { 
        ::new(p) T;
    }

    static void Construct(void* p, const T& source) 
    { 
        ::new(p) T(source); 
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void* p, const S& source)
    {
        ::new(p) T(source);
    }

    static void ConstructArray(void* p, UPInt count)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata);
    }

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, source);
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, *psource++);
    }

    static void Destruct(T* p)
    {
        p->~T();
        SF_UNUSED(p); // Suppress silly MSVC warning
    }

    static void DestructArray(T* p, UPInt count)
    {   
        p += count - 1;
        for (UPInt i=0; i<count; ++i, --p)
            p->~T();
    }

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static bool IsMovable() { return true; }
};


// ***** ConstructorCPP
//
// Correct C++ construction and destruction for movable objects
//------------------------------------------------------------------------
template<class T> 
class ConstructorCPP
{
public:
    static void Construct(void* p) 
    { 
        ::new(p) T;
    }

    static void Construct(void* p, const T& source) 
    { 
        ::new(p) T(source); 
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void* p, const S& source)
    {
        ::new(p) T(source);
    }

    static void ConstructArray(void* p, UPInt count)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata);
    }

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, source);
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, *psource++);
    }

    static void Destruct(T* p)
    {
        p->~T();
        SF_UNUSED(p); // Suppress silly MSVC warning
    }

    static void DestructArray(T* p, UPInt count)
    {   
        p += count - 1;
        for (UPInt i=0; i<count; ++i, --p)
            p->~T();
    }

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        for(UPInt i = 0; i < count; ++i)
            dst[i] = src[i];
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        for(UPInt i = count; i; --i)
            dst[i-1] = src[i-1];
    }

    static bool IsMovable() { return false; }
};



// ***** Allocator*
//
// Simple wraps as specialized allocators
//------------------------------------------------------------------------
template<class T, int SID = Stat_Default_Mem> struct AllocatorGH_POD : AllocatorBaseGH<SID>, ConstructorPOD<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorGH     : AllocatorBaseGH<SID>, ConstructorMov<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorGH_CPP : AllocatorBaseGH<SID>, ConstructorCPP<T> {};

template<class T, int SID = Stat_Default_Mem> struct AllocatorLH_POD : AllocatorBaseLH<SID>, ConstructorPOD<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorLH     : AllocatorBaseLH<SID>, ConstructorMov<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorLH_CPP : AllocatorBaseLH<SID>, ConstructorCPP<T> {};

template<class T, int SID = Stat_Default_Mem> struct AllocatorDH_POD : AllocatorBaseDH<SID>, ConstructorPOD<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorDH     : AllocatorBaseDH<SID>, ConstructorMov<T> {};
template<class T, int SID = Stat_Default_Mem> struct AllocatorDH_CPP : AllocatorBaseDH<SID>, ConstructorCPP<T> {};

} // Scaleform

// Redefine operator 'new' if necessary.
#if defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#endif
