/**************************************************************************

PublicHeader:   None
Filename    :   SF_ArrayUnsafe.h
Content     :   Template implementation for GArrayUnsafe
Created     :   2008
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_ArrayUnsafe_H
#define INC_SF_Kernel_ArrayUnsafe_H

#include "SF_Allocator.h"

namespace Scaleform {

// ***** ArrayUnsafeBase
//
// A simple and unsafe modification of the array. It DOES NOT automatically
// resizes the array on PushBack, Append and so on! 
//
// Functions Reserve() and Resize() always result in losing all data!
// It's used in performance critical algorithms, such as tessellation,
// where the maximal number of elements is known in advance. There's no 
// check for memory overrun, but the address persistence is guaranteed.
// Use with CAUTION! 
//
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
//------------------------------------------------------------------------
template<class T, class Allocator> class ArrayUnsafeBase
{
public:
    typedef ArrayUnsafeBase<T, Allocator>   SelfType;
    typedef T                               ValueType;
    typedef Allocator                       AllocatorType;

    ~ArrayUnsafeBase() 
    { 
        Allocator::DestructArray(Data, Size);
        Allocator::Free(Data);
    }

    ArrayUnsafeBase() : Data(0), Size(0), Capacity(0) {}

    ArrayUnsafeBase(UPInt capacity) :
        Data((T*)Allocator::Alloc(this, Capacity * sizeof(T), __FILE__, __LINE__)),
        Size(0), 
        Capacity(capacity)
    {}

    ArrayUnsafeBase(const SelfType& v) : 
        Data(Capacity ? (T*)Allocator::Alloc(this, Capacity * sizeof(T), __FILE__, __LINE__) : 0),
        Size(v.Size), 
        Capacity(v.Capacity)
    {
        if(Size)
            Allocator::CopyArrayForward(Data, v.Data, Size);
    }

    void Clear()
    { 
        Allocator::DestructArray(Data, Size);
        Size = 0; 
    }

    void ClearAndRelease()
    {
        Allocator::DestructArray(Data, Size);
        Allocator::Free(Data);
        Size = Capacity = 0; 
        Data = 0;
    }

    void CutAt(UPInt newSize) 
    { 
        if(newSize < Size) 
        {
            Allocator::DestructArray(Data + newSize, Size - newSize);
            Size = newSize; 
        }
    }

    // Set new capacity. All data is lost, size is set to zero.
    void Reserve(UPInt cap, UPInt extraTail=0)
    {
        Allocator::DestructArray(Data, Size);
        if(cap > Capacity)
        {
            Allocator::Free(Data);
            Capacity = cap + extraTail;
            Data = Capacity ? (T*)Allocator::Alloc(this, Capacity * sizeof(T), __FILE__, __LINE__) : 0;
        }
        Size = 0;
    }

    UPInt GetCapacity() const 
    { 
        return Capacity; 
    }

    UPInt GetNumBytes() const
    { 
        return GetCapacity() * sizeof(ValueType); 
    }

    // Allocate n elements. All data is lost, 
    // but elements can be accessed in range 0...size-1. 
    void Resize(UPInt size, UPInt extraTail=0)
    {
        Reserve(size, extraTail);
        Size = size;
        Allocator::ConstructArray(Data, size);
    }

    void Zero()
    {
        memset(Data, 0, sizeof(T) * Size);
    }

    void PushBack(const T& v)
    { 
        Allocator::Construct(&Data[Size++], v);
    }

    template<class S>
    void PushBackAlt(const S& val)
    {
        Allocator::ConstructAlt(&Data[Size++], val);
    }

    void InsertAt(UPInt pos, const T& val)
    {
        if(pos >= Size) 
        {
            PushBack(val);
        }
        else
        {
            Allocator::Construct(Data + Size);
            Allocator::CopyArrayBackward(Data + pos + 1, Data + pos, Size - pos);
            Allocator::Construct(Data + pos, val);
            ++Size;
        }
    }

    // Removing an element from the array is an expensive operation!
    // It compacts only after removing the last element.
    void RemoveAt(UPInt pos)
    {
        if (Size == 1)
        {
            Clear();
        }
        else
        {
            Allocator::Destruct(Data + pos);
            Allocator::CopyArrayForward(
                Data + pos, 
                Data + pos + 1,
                Size - 1 - pos);
            --Size;
        }
    }

    UPInt GetSize() const 
    { 
        return Size; 
    }

    const SelfType& operator = (const SelfType& v)
    {
        if(&v != this)
        {
            Resize(v.Size);
            if(Size) Allocator::CopyArrayForward(Data, v.Data, Size);
        }
        return *this;
    }

    const T& operator [] (UPInt i) const { return Data[i]; }
          T& operator [] (UPInt i)       { return Data[i]; }
    const T& At(UPInt i) const           { return Data[i]; }
          T& At(UPInt i)                 { return Data[i]; }
    T   ValueAt(UPInt i) const           { return Data[i]; }

    const T* GetDataPtr() const { return Data; }
          T* GetDataPtr()       { return Data; }

private:
    T*    Data;
    UPInt Size;
    UPInt Capacity;
};



// ***** ArrayUnsafePOD
//
// General purpose "unsafe" array for movable objects that DOES NOT require 
// construction/destruction. Constructors and destructors are not called! 
// Global heap is in use.
//------------------------------------------------------------------------
template<class T, int SID=Stat_Default_Mem>
class ArrayUnsafePOD : 
    public ArrayUnsafeBase<T, AllocatorGH_POD<T, SID> >
{
public:
    typedef T                                  ValueType;
    typedef AllocatorGH_POD<T, SID>            AllocatorType;
    typedef ArrayUnsafeBase<T, AllocatorType>  BaseType;
    typedef ArrayUnsafePOD<T, SID>             SelfType;
    ArrayUnsafePOD()                   : BaseType() {}
    ArrayUnsafePOD(UPInt capacity)     : BaseType(capacity) {}
    ArrayUnsafePOD(const SelfType& v)  : BaseType(v) {}
    const SelfType& operator=(const SelfType& a) { BaseType::operator=(a); return *this; }
};


// ***** ArrayUnsafe
//
// General purpose "unsafe" array for movable objects that require explicit 
// construction/destruction. Global heap is in use.
//------------------------------------------------------------------------
template<class T, int SID=Stat_Default_Mem>
class ArrayUnsafe : 
    public ArrayUnsafeBase<T, AllocatorGH<T, SID> >
{
public:
    typedef T                                  ValueType;
    typedef AllocatorGH<T, SID>                AllocatorType;
    typedef ArrayUnsafeBase<T, AllocatorType>  BaseType;
    typedef ArrayUnsafe<T, SID>                SelfType;
    ArrayUnsafe()                   : BaseType() {}
    ArrayUnsafe(UPInt capacity)     : BaseType(capacity) {}
    ArrayUnsafe(const SelfType& v)  : BaseType(v) {}
    const SelfType& operator=(const SelfType& a) { BaseType::operator=(a); return *this; }
};


// ***** ArrayUnsafeBase
//
// General purpose "unsafe" array for movable objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// Local heap is in use.
//------------------------------------------------------------------------
template<class T, int SID=Stat_Default_Mem>
class ArrayUnsafeLH_POD : 
    public ArrayUnsafeBase<T, AllocatorLH_POD<T, SID> >
{
public:
    typedef T                                  ValueType;
    typedef AllocatorLH_POD<T, SID>            AllocatorType;
    typedef ArrayUnsafeBase<T, AllocatorType>  BaseType;
    typedef ArrayUnsafeLH_POD<T, SID>          SelfType;
    ArrayUnsafeLH_POD()                   : BaseType() {}
    ArrayUnsafeLH_POD(UPInt capacity)     : BaseType(capacity) {}
    ArrayUnsafeLH_POD(const SelfType& v)  : BaseType(v) {}
    const SelfType& operator=(const SelfType& a) { BaseType::operator=(a); return *this; }
};


// ***** ArrayUnsafeLH
//
// General purpose "unsafe" array for movable objects that require explicit 
// construction/destruction. Local heap is in use.
//------------------------------------------------------------------------
template<class T, int SID=Stat_Default_Mem>
class ArrayUnsafeLH : 
    public ArrayUnsafeBase<T, AllocatorLH<T, SID> >
{
public:
    typedef T                                  ValueType;
    typedef AllocatorLH<T, SID>                AllocatorType;
    typedef ArrayUnsafeBase<T, AllocatorType>  BaseType;
    typedef ArrayUnsafeLH<T, SID>              SelfType;
    ArrayUnsafeLH()                   : BaseType() {}
    ArrayUnsafeLH(UPInt capacity)     : BaseType(capacity) {}
    ArrayUnsafeLH(const SelfType& v)  : BaseType(v) {}
    const SelfType& operator=(const SelfType& a) { BaseType::operator=(a); return *this; }
};

} // Scaleform

#endif
