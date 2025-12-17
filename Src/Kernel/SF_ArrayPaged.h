/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_ArrayPaged.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_ArrayPaged_H
#define INC_SF_Kernel_ArrayPaged_H

#include "SF_Allocator.h"

namespace Scaleform {


// ***** ConstructorPagedPOD
//
// A modification of ConstructorPOD with paged construction/destruction
// Used to avoid possible run-time overhead for POD types.
//------------------------------------------------------------------------
template<class T> 
class ConstructorPagedPOD : public ConstructorPOD<T>
{
public:
    static void ConstructArrayPaged(T**, UPInt, UPInt, UPInt, UPInt) {}
    static void DestructArrayPaged (T**, UPInt, UPInt, UPInt, UPInt) {}
};


// ***** ConstructorPagedMov
//
// Correct C++ paged construction and destruction
//------------------------------------------------------------------------
template<class T> 
class ConstructorPagedMov : public ConstructorMov<T>
{
public:
    static void ConstructArrayPaged(T** pages, UPInt start, UPInt end, UPInt pageShift, UPInt pageMask)
    {   
        for (UPInt i = start; i < end; ++i)
        {
            ConstructorMov<T>::Construct(pages[i >> pageShift] + (i & pageMask));
        }
    }

    static void DestructArrayPaged(T** pages, UPInt start, UPInt end, UPInt pageShift, UPInt pageMask)
    {   
        for (UPInt i = end; i > start; --i)
        {
            ConstructorMov<T>::Destruct(pages[(i-1) >> pageShift] + ((i-1) & pageMask));
        }
    }
};


// ***** ConstructorPagedMovCC
//
// Correct C++ paged construction and destruction
//------------------------------------------------------------------------
template<typename T> 
class ConstructorPagedMovCC : public ConstructorMov<T>
{
public:
    const T& GetDefaultValue() const;
    void ConstructArrayPaged(T** pages, UPInt start, UPInt end, UPInt pageShift, UPInt pageMask)
    {   
        for (UPInt i = start; i < end; ++i)
        {
            ConstructorMov<T>::ConstructAlt(pages[i >> pageShift] + (i & pageMask), GetDefaultValue());
        }
    }

    static void DestructArrayPaged(T** pages, UPInt start, UPInt end, UPInt pageShift, UPInt pageMask)
    {   
        for (UPInt i = end; i > start; --i)
        {
            ConstructorMov<T>::Destruct(pages[(i-1) >> pageShift] + ((i-1) & pageMask));
        }
    }
};


// ***** AllocatorPaged*
//
// Simple wraps as specialized allocators
//------------------------------------------------------------------------
template<class T, int SID> struct AllocatorPagedGH_POD : AllocatorBaseGH<SID>, ConstructorPagedPOD<T> {};
template<class T, int SID> struct AllocatorPagedGH     : AllocatorBaseGH<SID>, ConstructorPagedMov<T> {};

template<class T, int SID> struct AllocatorPagedLH_POD : AllocatorBaseLH<SID>, ConstructorPagedPOD<T> {};
template<class T, int SID> struct AllocatorPagedLH     : AllocatorBaseLH<SID>, ConstructorPagedMov<T> {};

template<typename T, int SID> struct AllocatorPagedCC : AllocatorBaseLH<SID>, ConstructorPagedMovCC<T> {};


// ***** ArrayPagedBase
//
// A simple class template to store data similar to std::deque
// It doesn't reallocate memory but instead, uses pages of data of size 
// of (1 << PageSh), that is, power of two. The data is NOT contiguous in memory, 
// so the only valid access methods are operator [], At(), ValueAt(), Front(), Back()
// The container guarantees the persistence of elements' addresses in memory, 
// so the elements can be used in intrusive linked lists. 
// 
// Reallocs occur only when the pool of pointers to pages needs 
// to be extended (happens very rarely). You can control the increment 
// value by PtrPoolInc. 
// 
//-------------------
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
//------------------------------------------------------------------------
template<class T, int PageSh, int PtrPoolInc, class Allocator>
class ArrayPagedBase : public Allocator
{
public:
    enum PageConsts
    {   
        PageShift = PageSh,
        PageSize  = 1 << PageShift,
        PageMask  = PageSize - 1
    };

    typedef ArrayPagedBase<T, PageSh, PtrPoolInc, Allocator>    SelfType;
    typedef T                                                   ValueType;
    typedef Allocator                                           AllocatorType;

    ~ArrayPagedBase()
    {
        ClearAndRelease();
    }

    ArrayPagedBase() : 
        Size(0),
        NumPages(0),
        MaxPages(0),
        Pages(0)
    {}

    void ClearAndRelease()
    {
        if(NumPages)
        {
            T** blk = Pages + NumPages - 1;
            // It is actually numDataPages - 1. But this is OK.
            const UPInt numDataPages = (Size > 0 ? Size >> PageShift : 0);
            while(NumPages--)
            {
                const UPInt freeCount = (NumPages < numDataPages ? PageSize : (NumPages == numDataPages ? Size & PageMask : 0));
                Allocator::DestructArray(*blk, freeCount);
                Allocator::Free(*blk);
                --blk;
            }
            Allocator::Free(Pages);
        }
        Size = NumPages = MaxPages = 0;
        Pages = 0;
    }

    void Clear()
    { 
        Allocator::DestructArrayPaged(Pages, 0, Size, PageShift, PageMask);
        Size = 0; 
    }

    void PushBack(const T& val)
    {
        Allocator::Construct(acquireDataPtr(), val);
        ++Size;
    }

    bool PushBackSafe(const T& val)
    {
        T* p = acquireDataPtrSafe();
        if (!p) return false;
        Allocator::Construct(p, val);
        ++Size;
        return true;
    }

    template<class S>
    void PushBackAlt(const S& val)
    {
        Allocator::ConstructAlt(acquireDataPtr(), val);
        ++Size;
    }

    void PopBack()
    {
        if(Size) 
        {
            Allocator::Destruct(&At(Size - 1));
            --Size;
        }
    }
    void PopBack(UPInt count)
    {
        SF_ASSERT(Size >= count);
        while(count) 
        {
            Allocator::Destruct(&At(Size - 1));
            --Size;
            --count;
        }
    }

    T& PushDefault()
    {
        PushBack(T());
        return Back();
    }

    T Pop()
    {
        T t = Back();
        PopBack();
        return t;
    }


    UPInt GetCapacity() const
    {
        return NumPages << PageShift;
    }

    UPInt GetNumBytes() const
    { 
        return GetCapacity() * sizeof(T) + MaxPages * sizeof(T*);
    }

    void Reserve(UPInt newCapacity)
    {
        if(newCapacity > GetCapacity())
        {
            UPInt newNumPages = (newCapacity + PageMask) >> PageShift;
            for(UPInt i = NumPages; i < newNumPages; ++i)
                allocatePage(i);
        }
    }

    void Resize(UPInt newSize)
    {
        if(newSize > Size)
        {
            UPInt newNumPages = (newSize + PageMask) >> PageShift;
            for(UPInt i = NumPages; i < newNumPages; ++i)
                allocatePage(i);
            Allocator::ConstructArrayPaged(Pages, Size, newSize, PageShift, PageMask);
            Size = newSize;
            return;
        }
        if(newSize < Size)
        {
            Allocator::DestructArrayPaged(Pages, newSize, Size, PageShift, PageMask);
            Size = newSize;
        }
    }

    void CutAt(UPInt newSize)
    {
        if(newSize < Size) 
        {
            Allocator::DestructArrayPaged(Pages, newSize, Size, PageShift, PageMask);
            Size = newSize;
        }
    }

    void InsertAt(UPInt pos, const T& val)
    {
        if(pos >= Size) 
        {
            PushBack(val);
        }
        else
        {
            Allocator::Construct(acquireDataPtr());
            ++Size;
            UPInt i;
            // TBD: Optimize page copying
            for(i = Size-1; i > pos; --i)
            {
                At(i) = At(i - 1);
            }
            At(pos) = val;
        }
    }

    void RemoveAt(UPInt pos)
    {
        if(Size)
        {
            // TBD: Optimize page copying
            for(++pos; pos < Size; pos++)
            {
                At(pos-1) = At(pos);
            }
            Allocator::Destruct(&At(Size - 1));
            --Size;
        }
    }

    UPInt GetSize() const 
    { 
        return Size; 
    }

    const T& operator [] (UPInt i) const
    {
        return Pages[i >> PageShift][i & PageMask];
    }

    T& operator [] (UPInt i)
    {
        return Pages[i >> PageShift][i & PageMask];
    }

    const T& At(UPInt i) const
    { 
        return Pages[i >> PageShift][i & PageMask];
    }

    T& At(UPInt i) 
    { 
        return Pages[i >> PageShift][i & PageMask];
    }

    T ValueAt(UPInt i) const
    { 
        return Pages[i >> PageShift][i & PageMask];
    }

    const T& Front() const
    {
        return Pages[0][0];
    }

    T& Front()
    {
        return Pages[0][0];
    }

    const T& Back() const
    {
        return At(Size - 1);
    }

    T& Back()
    {
        return At(Size - 1);
    }

private:
    // Copying is prohibited
    ArrayPagedBase(const SelfType&);
    const SelfType& operator = (const SelfType&);

    void allocatePage(UPInt nb)
    {
        if(nb >= MaxPages) 
        {
            if(Pages)
            {
                Pages = (T**)Allocator::Realloc(
                    Pages, (MaxPages + PtrPoolInc) * sizeof(T*), 
                    __FILE__, __LINE__);
            }
            else
            {
                Pages = (T**)Allocator::Alloc(
                    this, PtrPoolInc * sizeof(T*), 
                    __FILE__, __LINE__);
            }
            MaxPages += PtrPoolInc;
        }
        Pages[nb] = (T*)Allocator::Alloc(this, PageSize * sizeof(T), __FILE__, __LINE__);
        NumPages++;
    }

    SF_INLINE T* acquireDataPtr()
    {
        UPInt np = Size >> PageShift;
        if(np >= NumPages)
        {
            allocatePage(np);
        }
        return Pages[np] + (Size & PageMask);
    }

    bool allocatePageSafe(UPInt nb)
    {
        if(nb >= MaxPages) 
        {
            T** newPages;
            if(Pages)
            {
                newPages = (T**)Allocator::Realloc(
                    Pages, (MaxPages + PtrPoolInc) * sizeof(T*), 
                    __FILE__, __LINE__);
            }
            else
            {
                newPages = (T**)Allocator::Alloc(
                    this, PtrPoolInc * sizeof(T*), 
                    __FILE__, __LINE__);
            }
            if (!newPages)
                return false;
            Pages = newPages;
            MaxPages += PtrPoolInc;
        }
        Pages[nb] = (T*)Allocator::Alloc(this, PageSize * sizeof(T), __FILE__, __LINE__);
        if (!Pages[nb])
            return false;
        NumPages++;
        return true;
    }

    SF_INLINE T* acquireDataPtrSafe()
    {
        UPInt np = Size >> PageShift;
        if(np >= NumPages)
        {
            if (!allocatePageSafe(np))
                return NULL;
        }
        return Pages[np] + (Size & PageMask);
    }

    UPInt Size;
    UPInt NumPages;
    UPInt MaxPages;
    T**   Pages;
};




// ***** ArrayPagedPOD
//
// General purpose paged array for objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// Global heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSh=6, int PtrPoolInc=64, int SID=Stat_Default_Mem>
class ArrayPagedPOD : 
    public ArrayPagedBase<T, PageSh, PtrPoolInc, AllocatorPagedGH_POD<T, SID> >
{
public:
    typedef ArrayPagedPOD<T, PageSh, PtrPoolInc, SID>   SelfType;
    typedef T                                           ValueType;
    typedef AllocatorPagedGH_POD<T, SID>                AllocatorType;
};


// ***** ArrayPaged
//
// General purpose paged array for objects that require 
// explicit construction/destruction. 
// Global heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSh=6, int PtrPoolInc=64, int SID=Stat_Default_Mem>
class ArrayPaged : 
    public ArrayPagedBase<T, PageSh, PtrPoolInc, AllocatorPagedGH<T, SID> >
{
public:
    typedef ArrayPaged<T, PageSh, PtrPoolInc, SID>      SelfType;
    typedef T                                           ValueType;
    typedef AllocatorPagedGH<T, SID>                    AllocatorType;
};


// ***** ArrayPagedLH_POD
//
// General purpose paged array for objects that require 
// explicit construction/destruction. 
// Local heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSh=6, int PtrPoolInc=64, int SID=Stat_Default_Mem>
class ArrayPagedLH_POD : 
    public ArrayPagedBase<T, PageSh, PtrPoolInc, AllocatorPagedLH_POD<T, SID> >
{
public:
    typedef ArrayPagedLH_POD<T, PageSh, PtrPoolInc, SID>    SelfType;
    typedef T                                               ValueType;
    typedef AllocatorPagedLH_POD<T, SID>                    AllocatorType;
};


// ***** ArrayPagedLH
//
// General purpose paged array for objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// Local heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSh=6, int PtrPoolInc=64, int SID=Stat_Default_Mem>
class ArrayPagedLH : 
    public ArrayPagedBase<T, PageSh, PtrPoolInc, AllocatorPagedLH<T, SID> >
{
public:
    typedef ArrayPagedLH<T, PageSh, PtrPoolInc, SID>    SelfType;
    typedef T                                           ValueType;
    typedef AllocatorPagedLH<T, SID>                    AllocatorType;
};

// ***** ArrayPagedCC
//
// A modification of the array that uses the given default value to
// construct the elements. The constructors and destructors are 
// properly called, the objects must be movable.
// Local heap is in use.
//------------------------------------------------------------------------
template<typename T, int PageSh=6, int PtrPoolInc=64, int SID=Stat_Default_Mem>
class ArrayPagedCC : 
    public ArrayPagedBase<T, PageSh, PtrPoolInc, AllocatorPagedCC<T, SID> >
{
public:
    typedef ArrayPagedCC<T, PageSh, PtrPoolInc, SID>    SelfType;
    typedef T                                           ValueType;
    typedef AllocatorPagedCC<T, SID>                    AllocatorType;

public:
    ArrayPagedCC(const ValueType& v) : DefaultValue(v) {}

    const T& GetDefaultValue() const
    {
        return DefaultValue;
    }

private:
    ValueType DefaultValue;
};

} // Scaleform

#endif
