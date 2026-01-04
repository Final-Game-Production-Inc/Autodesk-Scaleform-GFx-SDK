/**************************************************************************

PublicHeader:   None
Filename    :   SF_Alg.h
Content     :   Simple general purpose algorithms: Sort, Binary Search, etc.
Created     :   
Authors     :   Maxim Shemanarev, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Alg_H
#define INC_SF_Kernel_Alg_H

#include "SF_Types.h"
#include <string.h>

namespace Scaleform { namespace Alg {


// ***** Operator extensions

template <typename T> SF_INLINE void Swap(T &a, T &b) 
    {  T temp(a); a = b; b = temp; }

// Specialization for Pair()
template <typename T1, typename T2> SF_INLINE void Swap(Pair<T1, T2>& l, Pair<T1, T2>& r)
    { Swap(l.First, r.First); Swap(l.Second, r.Second); }

template<typename T> SF_INLINE T Sign(T a)
    { if (a == T(0)) 
        return T(0);
      if (a < 0)
          return T(-1);
      return T(1);
    }

// ***** min/max are not implemented in Visual Studio 6 standard STL

template <typename T> SF_INLINE const T Min(const T a, const T b)
    { return (a < b) ? a : b; }

template <typename T> SF_INLINE const T Max(const T a, const T b)
    { return (b < a) ? a : b; }

template <typename T> SF_INLINE const T Clamp(const T v, const T minVal, const T maxVal)
    { return Max<T>(minVal, Min<T>(v, maxVal)); }

template <typename T> SF_INLINE int     Chop(T f)
    { return (int)f; }

template <typename T> SF_INLINE T       Lerp(T a, T b, T f) 
    { return (b - a) * f + a; }

// Lerp between two objects, based on a third of another type. For example, 
// for Color and UByte, this formula is acceptable. Same for Point/Size<T> and T.
template <typename T, typename S> SF_INLINE T       Lerp(T a, T b, S f) 
{ return (T)((b - a) * f + a); }

template <typename T> SF_INLINE int     IRound(const T a)
    { return (a > 0.0) ? (int)(a + 0.5) : (int)(a - 0.5); }

template <typename T> SF_INLINE unsigned URound(const T a)
    { return (unsigned)(a + 0.5); }

template <typename T> SF_INLINE bool    ToleranceEqual(const T a, const T b, const T tolerance)
{
    return (a + tolerance >= b) && (a - tolerance <= b);
}
// These functions stand to fix a stupid VC++ warning (with /Wp64 on):
// "warning C4267: 'argument' : conversion from 'size_t' to 'const unsigned', possible loss of data"
// Use these functions instead of gmin/gmax if the argument has size
// of the pointer to avoid the warning. Though, functionally they are
// absolutelly the same as regular gmin/gmax.
template <typename T>   SF_INLINE const T PMin(const T a, const T b)
{
    SF_COMPILER_ASSERT(sizeof(T) == sizeof(UPInt));
    return (a < b) ? a : b;
}
template <typename T>   SF_INLINE const T PMax(const T a, const T b)
{
    SF_COMPILER_ASSERT(sizeof(T) == sizeof(UPInt));
    return (b < a) ? a : b;
}

// Do not change the order of lines below, otherwise VC++ will gen warnings in /Wp64 mode
inline bool IsMax(UPInt v) { return v == SF_MAX_UPINT; }
#ifdef SF_64BIT_POINTERS
inline bool IsMax(unsigned v)  { return v == SF_MAX_UINT; }
#endif

// ***** abs

template <typename T>   SF_INLINE const T Abs(const T v)
    { return (v>=0) ? v : -v; }


UInt8 RoundUpToPowerOfTwo(UInt8 x);

inline UInt32 RoundUpToPowerOfTwo(UInt32 x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    return x + 1;
}

// Create a block of bits starting at position p having size n.
inline UInt64 BitBlock(UInt8 p, UInt8 n)
{
    UInt64 x = (1UL << n) - 1;
    return x << p;
}

// 'Aligns' a value (with a literal/const) and other version with a parameter
// *alignment* must be power of two.
template <int alignment, typename T>   SF_INLINE T Align(T v)
{
    SF_COMPILER_ASSERT(alignment > 1);
    const T a = alignment - 1;
    return (v + a) & ~a;
}

template <typename T> SF_INLINE T Align(T v, UPInt alignment)
{
    if (alignment <= 1)
        return v;
    const UPInt a = alignment - 1;
    return static_cast<T>((v + a) & ~a);
}
template <typename T> SF_INLINE T* Align(T* v, UPInt alignment)
{
    if (alignment <= 1)
        return v;
    const UPInt a = alignment - 1;
    return reinterpret_cast<T*>((reinterpret_cast<UPInt>(v) + a) & ~a);
}

// Returns whether val is a power of two.
template<typename T> bool IsPowerOfTwo(T val)
{
    return (val & (val-1)) == 0;
}

template <typename T> SF_INLINE bool IsAligned(T x, int n)
{
    SF_ASSERT(IsPowerOfTwo(n));
    return (x & (n - 1)) == 0;
}

template <typename T> SF_INLINE bool IsAligned(T* x, int n)
{
    return IsAligned(reinterpret_cast<UPInt>(x), n);
}

template <typename T> SF_INLINE T RoundDown(T x, int n)
{
    SF_ASSERT(IsPowerOfTwo(n));
    return (x & -n);
}

template <typename T> SF_INLINE T* RoundDown(T* x, int n)
{
    return reinterpret_cast<T*>(RoundDown(reinterpret_cast<UPInt>(x), n));
}

template <typename T> SF_INLINE T RoundUp(T x, int n)
{
    return RoundDown(x + n - 1, n);
}

template <typename T> SF_INLINE T* RoundUp(T* x, int n)
{
    return reinterpret_cast<T*>(RoundUp(reinterpret_cast<UPInt>(x), n));
}

// Returns the next highest power-of-two of val, or val if it is already a power-of-two.
template <typename T> SF_INLINE const T UpperPowerOfTwo(T val)
{
    T n(1);
    while (n < val)
        n *= T(2);
    return n;
}

// Explicit specialization for UInt32
template<>
SF_INLINE const UInt32 UpperPowerOfTwo(UInt32 val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}


// Returns the next lower power-of-two of val, or val if it is already a power-of-two.
template <typename T> SF_INLINE const T LowerPowerOfTwo(T val)
{
    if (IsPowerOfTwo(val))
        return val;
    return UpperPowerOfTwo(val) >> 1;
}

// ***** OperatorLess
//
//------------------------------------------------------------------------
template<class T> struct OperatorLess
{
    static bool Compare(const T& a, const T& b)
    {
        return a < b;
    }
};


// ***** QuickSortSliced
//
// Sort any part of any array: plain, Array, ArrayPaged, ArrayUnsafe.
// The range is specified with start, end, where "end" is exclusive!
// The comparison predicate must be specified.
//
// The code of these classes was taken from the Anti-Grain Geometry
// Project and modified for the use by Scaleform/Autodesk. 
// Permission to use without restrictions is hereby granted to 
// Scaleform/Autodesk by the author of Anti-Grain Geometry Project.
// See http://antigrain.com for details.
//------------------------------------------------------------------------
template<class Array, class Less> 
void QuickSortSliced(Array& arr, UPInt start, UPInt end, Less less)
{
    enum 
    {
        Threshold = 9
    };

    if(end - start <  2) return;

    SPInt  stack[80];
    SPInt* top   = stack; 
    SPInt  base  = (SPInt)start;
    SPInt  limit = (SPInt)end;

    for(;;)
    {
        SPInt len = limit - base;
        SPInt i, j, pivot;

        if(len > Threshold)
        {
            // we use base + len/2 as the pivot
            pivot = base + len / 2;
            Swap(arr[base], arr[pivot]);

            i = base + 1;
            j = limit - 1;

            // now ensure that *i <= *base <= *j 
            if(less(arr[j],    arr[i])) Swap(arr[j],    arr[i]);
            if(less(arr[base], arr[i])) Swap(arr[base], arr[i]);
            if(less(arr[j], arr[base])) Swap(arr[j], arr[base]);

            for(;;)
            {
                do i++; while( less(arr[i], arr[base]) );
                do j--; while( less(arr[base], arr[j]) );

                if( i > j )
                {
                    break;
                }

                Swap(arr[i], arr[j]);
            }

            Swap(arr[base], arr[j]);

            // now, push the largest sub-array
            if(j - base > limit - i)
            {
                top[0] = base;
                top[1] = j;
                base   = i;
            }
            else
            {
                top[0] = i;
                top[1] = limit;
                limit  = j;
            }
            top += 2;
        }
        else
        {
            // the sub-array is small, perform insertion sort
            j = base;
            i = j + 1;

            for(; i < limit; j = i, i++)
            {
                for(; less(arr[j + 1], arr[j]); j--)
                {
                    Swap(arr[j + 1], arr[j]);
                    if(j == base)
                    {
                        break;
                    }
                }
            }
            if(top > stack)
            {
                top  -= 2;
                base  = top[0];
                limit = top[1];
            }
            else
            {
                break;
            }
        }
    }
}

// ***** QuickSortSliced
//
// Sort any part of any array: plain, Array, ArrayPaged, ArrayUnsafe.
// The range is specified with start, end, where "end" is exclusive!
// The data type must have a defined "<" operator.
//------------------------------------------------------------------------
template<class Array> 
void QuickSortSliced(Array& arr, UPInt start, UPInt end)
{
    typedef typename Array::ValueType ValueType;
    QuickSortSliced(arr, start, end, OperatorLess<ValueType>::Compare);
}

// Same as corresponding G_QuickSortSliced but with checking array limits to avoid
// crash in the case of wrong comparator functor.
template<class Array, class Less> 
bool QuickSortSlicedSafe(Array& arr, UPInt start, UPInt end, Less less)
{
    enum 
    {
        Threshold = 9
    };

    if(end - start <  2) return true;

    SPInt  stack[80];
    SPInt* top   = stack; 
    SPInt  base  = (SPInt)start;
    SPInt  limit = (SPInt)end;

    for(;;)
    {
        SPInt len = limit - base;
        SPInt i, j, pivot;

        if(len > Threshold)
        {
            // we use base + len/2 as the pivot
            pivot = base + len / 2;
            Swap(arr[base], arr[pivot]);

            i = base + 1;
            j = limit - 1;

            // now ensure that *i <= *base <= *j 
            if(less(arr[j],    arr[i])) Swap(arr[j],    arr[i]);
            if(less(arr[base], arr[i])) Swap(arr[base], arr[i]);
            if(less(arr[j], arr[base])) Swap(arr[j], arr[base]);

            for(;;)
            {
                do 
                {   
                    i++; 
                    if (i >= limit)
                        return false;
                } while( less(arr[i], arr[base]) );
                do 
                {
                    j--; 
                    if (j < 0)
                        return false;
                } while( less(arr[base], arr[j]) );

                if( i > j )
                {
                    break;
                }

                Swap(arr[i], arr[j]);
            }

            Swap(arr[base], arr[j]);

            // now, push the largest sub-array
            if(j - base > limit - i)
            {
                top[0] = base;
                top[1] = j;
                base   = i;
            }
            else
            {
                top[0] = i;
                top[1] = limit;
                limit  = j;
            }
            top += 2;
        }
        else
        {
            // the sub-array is small, perform insertion sort
            j = base;
            i = j + 1;

            for(; i < limit; j = i, i++)
            {
                for(; less(arr[j + 1], arr[j]); j--)
                {
                    Swap(arr[j + 1], arr[j]);
                    if(j == base)
                    {
                        break;
                    }
                }
            }
            if(top > stack)
            {
                top  -= 2;
                base  = top[0];
                limit = top[1];
            }
            else
            {
                break;
            }
        }
    }
    return true;
}

template<class Array> 
bool QuickSortSlicedSafe(Array& arr, UPInt start, UPInt end)
{
    typedef typename Array::ValueType ValueType;
    return QuickSortSlicedSafe(arr, start, end, OperatorLess<ValueType>::Compare);
}

//------------------------------------------------------------------------
// ***** QuickSort
//
// Sort an array Array, ArrayPaged, ArrayUnsafe.
// The array must have GetSize() function.
// The comparison predicate must be specified.
//------------------------------------------------------------------------
template<class Array, class Less> 
void QuickSort(Array& arr, Less less)
{
    QuickSortSliced(arr, 0, arr.GetSize(), less);
}

// checks for boundaries
template<class Array, class Less> 
bool QuickSortSafe(Array& arr, Less less)
{
    return QuickSortSlicedSafe(arr, 0, arr.GetSize(), less);
}


//------------------------------------------------------------------------
// ***** QuickSort
//
// Sort an array Array, ArrayPaged, ArrayUnsafe.
// The array must have GetSize() function.
// The data type must have a defined "<" operator.
template<class Array> 
void QuickSort(Array& arr)
{
    typedef typename Array::ValueType ValueType;
    QuickSortSliced(arr, 0, arr.GetSize(), OperatorLess<ValueType>::Compare);
}

template<class Array> 
bool QuickSortSafe(Array& arr)
{
    typedef typename Array::ValueType ValueType;
    return QuickSortSlicedSafe(arr, 0, arr.GetSize(), OperatorLess<ValueType>::Compare);
}

// ***** InsertionSortSliced
//
// Sort any part of any array: plain, Array, ArrayPaged, ArrayUnsafe.
// The range is specified with start, end, where "end" is exclusive!
// The comparison predicate must be specified.
// Unlike Quick Sort, the Insertion Sort works much slower in average, 
// but may be much faster on almost sorted arrays. Besides, it guarantees
// that the elements will not be swapped if not necessary. For example, 
// an array with all equal elements will remain "untouched", while 
// Quick Sort will considerably shuffle the elements in this case.
//------------------------------------------------------------------------
template<class Array, class Less> 
void InsertionSortSliced(Array& arr, UPInt start, UPInt end, Less less)
{
    UPInt j = start;
    UPInt i = j + 1;
    UPInt limit = end;

    for(; i < limit; j = i, i++)
    {
        for(; less(arr[j + 1], arr[j]); j--)
        {
            Swap(arr[j + 1], arr[j]);
            if(j <= start)
            {
                break;
            }
        }
    }
}




// ***** InsertionSortSliced
//
// Sort any part of any array: plain, Array, ArrayPaged, ArrayUnsafe.
// The range is specified with start, end, where "end" is exclusive!
// The data type must have a defined "<" operator.
//------------------------------------------------------------------------
template<class Array> 
void InsertionSortSliced(Array& arr, UPInt start, UPInt end)
{
    typedef typename Array::ValueType ValueType;
    InsertionSortSliced(arr, start, end, OperatorLess<ValueType>::Compare);
}


// ***** InsertionSort
//
// Sort an array Array, ArrayPaged, ArrayUnsafe.
// The array must have GetSize() function.
// The comparison predicate must be specified.
//------------------------------------------------------------------------
template<class Array, class Less> 
void InsertionSort(Array& arr, Less less)
{
    InsertionSortSliced(arr, 0, arr.GetSize(), less);
}


// ***** InsertionSort
//
// Sort an array Array, ArrayPaged, ArrayUnsafe.
// The array must have GetSize() function.
// The data type must have a defined "<" operator.
//------------------------------------------------------------------------
template<class Array> 
void InsertionSort(Array& arr)
{
    typedef typename Array::ValueType ValueType;
    InsertionSortSliced(arr, 0, arr.GetSize(), OperatorLess<ValueType>::Compare);
}




// ***** LowerBoundSliced
//
//-----------------------------------------------------------------------
template<class Array, class Value, class Less>
UPInt LowerBoundSliced(const Array& arr, UPInt start, UPInt end, const Value& val, Less less)
{
    SPInt first = (SPInt)start;
    SPInt len   = (SPInt)(end - start);
    SPInt half;
    SPInt middle;
    
    while(len > 0) 
    {
        half = len >> 1;
        middle = first + half;
        if(less(arr[middle], val)) 
        {
            first = middle + 1;
            len   = len - half - 1;
        }
        else
        {
            len = half;
        }
    }
    return (UPInt)first;
}


// ***** LowerBoundSliced
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt LowerBoundSliced(const Array& arr, UPInt start, UPInt end, const Value& val)
{
    return LowerBoundSliced(arr, start, end, val, OperatorLess<Value>::Compare);
}


// ***** LowerBoundSized
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt LowerBoundSized(const Array& arr, UPInt size, const Value& val)
{
    return LowerBoundSliced(arr, 0, size, val, OperatorLess<Value>::Compare);
}


// ***** LowerBound
//
//-----------------------------------------------------------------------
template<class Array, class Value, class Less>
UPInt LowerBound(const Array& arr, const Value& val, Less less)
{
    return LowerBoundSliced(arr, 0, arr.GetSize(), val, less);
}


// ***** LowerBound
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt LowerBound(const Array& arr, const Value& val)
{
    return LowerBoundSliced(arr, 0, arr.GetSize(), val, OperatorLess<Value>::Compare);
}




// ***** UpperBoundSliced
//
//-----------------------------------------------------------------------
template<class Array, class Value, class Less>
UPInt UpperBoundSliced(const Array& arr, UPInt start, UPInt end, const Value& val, Less less)
{
    SPInt first = (SPInt)start;
    SPInt len   = (SPInt)(end - start);
    SPInt half;
    SPInt middle;
    
    while(len > 0) 
    {
        half = len >> 1;
        middle = first + half;
        if(less(val, arr[middle]))
        {
            len = half;
        }
        else 
        {
            first = middle + 1;
            len   = len - half - 1;
        }
    }
    return (UPInt)first;
}


// ***** UpperBoundSliced
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt UpperBoundSliced(const Array& arr, UPInt start, UPInt end, const Value& val)
{
    return UpperBoundSliced(arr, start, end, val, OperatorLess<Value>::Compare);
}


// ***** UpperBoundSized
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt UpperBoundSized(const Array& arr, UPInt size, const Value& val)
{
    return UpperBoundSliced(arr, 0, size, val, OperatorLess<Value>::Compare);
}


// ***** UpperBound
//
//-----------------------------------------------------------------------
template<class Array, class Value, class Less>
UPInt UpperBound(const Array& arr, const Value& val, Less less)
{
    return UpperBoundSliced(arr, 0, arr.GetSize(), val, less);
}


// ***** UpperBound
//
//-----------------------------------------------------------------------
template<class Array, class Value>
UPInt UpperBound(const Array& arr, const Value& val)
{
    return UpperBoundSliced(arr, 0, arr.GetSize(), val, OperatorLess<Value>::Compare);
}


// ***** ReverseArray
//
//-----------------------------------------------------------------------
template<class Array> void ReverseArray(Array& arr)
{
    SPInt from = 0;
    SPInt to   = arr.GetSize() - 1;
    while(from < to)
    {
        Swap(arr[from], arr[to]);
        ++from;
        --to;
    }
}



// ***** AppendArray
//
template<class CDst, class CSrc> 
void AppendArray(CDst& dst, const CSrc& src)
{
    UPInt i;
    for(i = 0; i < src.GetSize(); i++) 
        dst.PushBack(src[i]);
}


// ***** ArrayAdaptor
//
// A simple adapter that provides the GetSize() method and overloads 
// operator []. Used to wrap plain arrays in QuickSort and such.
//------------------------------------------------------------------------
template<class T> class ArrayAdaptor
{
public:
    typedef T ValueType;
    ArrayAdaptor() : Data(0), Size(0) {}
    ArrayAdaptor(T* ptr, UPInt size) : Data(ptr), Size(size) {}
    UPInt GetSize() const { return Size; }
    const T& operator [] (UPInt i) const { return Data[i]; }
          T& operator [] (UPInt i)       { return Data[i]; }
private:
    T*      Data;
    UPInt   Size;
};


// ***** GConstArrayAdaptor
//
// A simple const adapter that provides the GetSize() method and overloads 
// operator []. Used to wrap plain arrays in LowerBound and such.
//------------------------------------------------------------------------
template<class T> class ConstArrayAdaptor
{
public:
    typedef T ValueType;
    ConstArrayAdaptor() : Data(0), Size(0) {}
    ConstArrayAdaptor(const T* ptr, UPInt size) : Data(ptr), Size(size) {}
    UPInt GetSize() const { return Size; }
    const T& operator [] (UPInt i) const { return Data[i]; }
private:
    const T* Data;
    UPInt    Size;
};



//------------------------------------------------------------------------
extern const UByte UpperBitTable[256];
extern const UByte LowerBitTable[256];



//------------------------------------------------------------------------
inline UByte UpperBit(UPInt val)
{
#ifndef SF_64BIT_POINTERS

    if (val & 0xFFFF0000)
    {
        return (val & 0xFF000000) ? 
            UpperBitTable[(val >> 24)       ] + 24: 
            UpperBitTable[(val >> 16) & 0xFF] + 16;
    }
    return (val & 0xFF00) ?
        UpperBitTable[(val >> 8) & 0xFF] + 8:
        UpperBitTable[(val     ) & 0xFF];

#else

    if (val & 0xFFFFFFFF00000000)
    {
        if (val & 0xFFFF000000000000)
        {
            return (val & 0xFF00000000000000) ?
                UpperBitTable[(val >> 56)       ] + 56: 
                UpperBitTable[(val >> 48) & 0xFF] + 48;
        }
        return (val & 0xFF0000000000) ?
            UpperBitTable[(val >> 40) & 0xFF] + 40:
            UpperBitTable[(val >> 32) & 0xFF] + 32;
    }
    else
    {
        if (val & 0xFFFF0000)
        {
            return (val & 0xFF000000) ? 
                UpperBitTable[(val >> 24)       ] + 24: 
                UpperBitTable[(val >> 16) & 0xFF] + 16;
        }
        return (val & 0xFF00) ?
            UpperBitTable[(val >> 8) & 0xFF] + 8:
            UpperBitTable[(val     ) & 0xFF];
    }

#endif
}

//------------------------------------------------------------------------
inline UByte LowerBit(UPInt val)
{
#ifndef SF_64BIT_POINTERS

    if (val & 0xFFFF)
    {
        return (val & 0xFF) ?
            LowerBitTable[ val & 0xFF]:
            LowerBitTable[(val >> 8) & 0xFF] + 8;
    }
    return (val & 0xFF0000) ?
            LowerBitTable[(val >> 16) & 0xFF] + 16:
            LowerBitTable[(val >> 24) & 0xFF] + 24;

#else

    if (val & 0xFFFFFFFF)
    {
        if (val & 0xFFFF)
        {
            return (val & 0xFF) ?
                LowerBitTable[ val & 0xFF]:
                LowerBitTable[(val >> 8) & 0xFF] + 8;
        }
        return (val & 0xFF0000) ?
                LowerBitTable[(val >> 16) & 0xFF] + 16:
                LowerBitTable[(val >> 24) & 0xFF] + 24;
    }
    else
    {
        if (val & 0xFFFF00000000)
        {
             return (val & 0xFF00000000) ?
                LowerBitTable[(val >> 32) & 0xFF] + 32:
                LowerBitTable[(val >> 40) & 0xFF] + 40;
        }
        return (val & 0xFF000000000000) ?
            LowerBitTable[(val >> 48) & 0xFF] + 48:
            LowerBitTable[(val >> 56) & 0xFF] + 56;
    }

#endif
}



// ******* Special (optimized) memory routines
// Note: null (bad) pointer is not tested
class MemUtil
{
public:
    
    // Memory copy
    static void*    Copy  (void* pdest, const void* psrc, UPInt byteCount)      { return memcpy(pdest, psrc, byteCount); }
    static void*    Copy16(void* pdest, const void* psrc, UPInt int16Count)     { return memcpy(pdest, psrc, int16Count*2); }
    static void*    Copy32(void* pdest, const void* psrc, UPInt int32Count)     { return memcpy(pdest, psrc, int32Count*4); }
    static void*    Copy64(void* pdest, const void* psrc, UPInt int64Count)     { return memcpy(pdest, psrc, int64Count*8); }
                                            
    // Memory move
    static void*    Move  (void* pdest, const void* psrc, UPInt byteCount)      { return memmove(pdest, psrc, byteCount); }
    static void*    Move16(void* pdest, const void* psrc, UPInt int16Count)     { return memmove(pdest, psrc, int16Count*2); }
    static void*    Move32(void* pdest, const void* psrc, UPInt int32Count)     { return memmove(pdest, psrc, int32Count*4); }
    static void*    Move64(void* pdest, const void* psrc, UPInt int64Count)     { return memmove(pdest, psrc, int64Count*8); }
                                    
    // Memory compare
    static int      Cmp  (const void* p1, const void* p2, UPInt byteCount)      { return memcmp(p1, p2, byteCount); }
    static int      Cmp16(const void* p1, const void* p2, UPInt int16Count);
    static int      Cmp32(const void* p1, const void* p2, UPInt int32Count);
    static int      Cmp64(const void* p1, const void* p2, UPInt int64Count);

    // Memory set
    static void     Set  (void* pdest, int value, UPInt byteCount)           { memset(pdest, value, byteCount); }
//  SF_EXPORT static void     SF_STDCALL Set16(void* pdest, SInt16 value, UPInt int16Count);
//  SF_EXPORT static void     SF_STDCALL Set32(void* pdest, SInt32 value, UPInt int32Count);
//  SF_EXPORT static void     SF_STDCALL Set64(void* pdest, SInt64 value, UPInt int64Count);
};

// ** Inline Implementation

inline int     MemUtil::Cmp16(const void* p1, const void* p2, UPInt int16Count)
{
    SInt16*  pa  = (SInt16*)p1; 
    SInt16*  pb  = (SInt16*)p2;
    unsigned ic  = 0;
    if (int16Count == 0)
        return 0;
    while (pa[ic] == pb[ic])
        if (++ic==int16Count)
            return 0;
    return pa[ic] > pb[ic] ? 1 : -1;
}
inline int     MemUtil::Cmp32(const void* p1, const void* p2, UPInt int32Count)
{
    SInt32*  pa  = (SInt32*)p1;
    SInt32*  pb  = (SInt32*)p2;
    unsigned ic  = 0;
    if (int32Count == 0)
        return 0;
    while (pa[ic] == pb[ic])
        if (++ic==int32Count)
            return 0;
    return pa[ic] > pb[ic] ? 1 : -1;
}
inline int     MemUtil::Cmp64(const void* p1, const void* p2, UPInt int64Count)
{
    SInt64*  pa  = (SInt64*)p1;
    SInt64*  pb  = (SInt64*)p2;
    unsigned ic  = 0;
    if (int64Count == 0)
        return 0;
    while (pa[ic] == pb[ic])
        if (++ic==int64Count)
            return 0;
    return pa[ic] > pb[ic] ? 1 : -1;
}

// ** End Inline Implementation



// ******* Byte Order Conversions
namespace ByteUtil {

    // *** Swap Byte Order

    // Swap the byte order of a byte array
    inline void     SwapOrder(void* pv, int size)
    {
        UByte*  pb = (UByte*)pv;
        UByte   temp;
        for (int i = 0; i < size>>1; i++)
        { 
            temp            = pb[size-1-i];
            pb[size-1-i]    = pb[i];
            pb[i]           = temp; 
        }
    }

    // Swap the byte order of primitive types
    inline UInt8    SwapOrder(UInt8 v)      { return v; }
    inline SInt8    SwapOrder(SInt8 v)      { return v; }
    inline UInt16   SwapOrder(UInt16 v)     { return UInt16(v>>8)|UInt16(v<<8); }
    inline SInt16   SwapOrder(SInt16 v)     { return SInt16((UInt16(v)>>8)|(v<<8)); }
    inline UInt32   SwapOrder(UInt32 v)     { return (v>>24)|((v&0x00FF0000)>>8)|((v&0x0000FF00)<<8)|(v<<24); }
    inline SInt32   SwapOrder(SInt32 p)     { return (SInt32)SwapOrder(UInt32(p)); }
    inline UInt64   SwapOrder(UInt64 v)
    { 
        return   (v>>56) |
                 ((v&SF_UINT64(0x00FF000000000000))>>40) |
                 ((v&SF_UINT64(0x0000FF0000000000))>>24) |
                 ((v&SF_UINT64(0x000000FF00000000))>>8)  |
                 ((v&SF_UINT64(0x00000000FF000000))<<8)  |
                 ((v&SF_UINT64(0x0000000000FF0000))<<24) |
                 ((v&SF_UINT64(0x000000000000FF00))<<40) |
                 (v<<56); 
    }
    inline SInt64   SwapOrder(SInt64 v)     { return (SInt64)SwapOrder(UInt64(v)); }
    inline float    SwapOrder(float p)      
    { 
        union {
            float p;
            UInt32 v;
        } u;
        u.p = p;
        u.v = SwapOrder(u.v);
        return u.p;
    }

    inline Double   SwapOrder(Double p)
    { 
        union {
            double p;
            UInt64 v;
        } u;
        u.p = p;
        u.v = SwapOrder(u.v);
        return u.p;
    }
    
    // *** Byte-order conversion

#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
    // Little Endian to System (LE)
    inline UInt8    LEToSystem(UInt8  v)    { return v; }
    inline SInt8    LEToSystem(SInt8  v)    { return v; }
    inline UInt16   LEToSystem(UInt16 v)    { return v; }
    inline SInt16   LEToSystem(SInt16 v)    { return v; }
    inline UInt32   LEToSystem(UInt32 v)    { return v; }
    inline SInt32   LEToSystem(SInt32 v)    { return v; }
    inline UInt64   LEToSystem(UInt64 v)    { return v; }
    inline SInt64   LEToSystem(SInt64 v)    { return v; }
    inline float    LEToSystem(float  v)    { return v; }
    inline Double   LEToSystem(Double v)    { return v; }
    // Big Endian to System (LE)
    inline UInt8    BEToSystem(UInt8  v)    { return SwapOrder(v); }
    inline SInt8    BEToSystem(SInt8  v)    { return SwapOrder(v); }
    inline UInt16   BEToSystem(UInt16 v)    { return SwapOrder(v); }
    inline SInt16   BEToSystem(SInt16 v)    { return SwapOrder(v); }
    inline UInt32   BEToSystem(UInt32 v)    { return SwapOrder(v); }
    inline SInt32   BEToSystem(SInt32 v)    { return SwapOrder(v); }
    inline UInt64   BEToSystem(UInt64 v)    { return SwapOrder(v); }
    inline SInt64   BEToSystem(SInt64 v)    { return SwapOrder(v); }
    inline float    BEToSystem(float  v)    { return SwapOrder(v); }
    inline Double   BEToSystem(Double v)    { return SwapOrder(v); }
    // System (LE) to Little Endian
    inline UInt8    SystemToLE(UInt8  v)    { return v; }
    inline SInt8    SystemToLE(SInt8  v)    { return v; }
    inline UInt16   SystemToLE(UInt16 v)    { return v; }
    inline SInt16   SystemToLE(SInt16 v)    { return v; }
    inline UInt32   SystemToLE(UInt32 v)    { return v; }
    inline SInt32   SystemToLE(SInt32 v)    { return v; }
    inline UInt64   SystemToLE(UInt64 v)    { return v; }
    inline SInt64   SystemToLE(SInt64 v)    { return v; }
    inline float    SystemToLE(float  v)    { return v; }
    inline Double   SystemToLE(Double v)    { return v; }   
    // System (LE) to Big Endian
    inline UInt8    SystemToBE(UInt8  v)    { return SwapOrder(v); }
    inline SInt8    SystemToBE(SInt8  v)    { return SwapOrder(v); }
    inline UInt16   SystemToBE(UInt16 v)    { return SwapOrder(v); }
    inline SInt16   SystemToBE(SInt16 v)    { return SwapOrder(v); }
    inline UInt32   SystemToBE(UInt32 v)    { return SwapOrder(v); }
    inline SInt32   SystemToBE(SInt32 v)    { return SwapOrder(v); }
    inline UInt64   SystemToBE(UInt64 v)    { return SwapOrder(v); }
    inline SInt64   SystemToBE(SInt64 v)    { return SwapOrder(v); }
    inline float    SystemToBE(float  v)    { return SwapOrder(v); }
    inline Double   SystemToBE(Double v)    { return SwapOrder(v); }

#elif (SF_BYTE_ORDER == SF_BIG_ENDIAN)
    // Little Endian to System (BE)
    inline UInt8    LEToSystem(UInt8  v)    { return SwapOrder(v); }
    inline SInt8    LEToSystem(SInt8  v)    { return SwapOrder(v); }
    inline UInt16   LEToSystem(UInt16 v)    { return SwapOrder(v); }
    inline SInt16   LEToSystem(SInt16 v)    { return SwapOrder(v); }
    inline UInt32   LEToSystem(UInt32 v)    { return SwapOrder(v); }
    inline SInt32   LEToSystem(SInt32 v)    { return SwapOrder(v); }
    inline UInt64   LEToSystem(UInt64 v)    { return SwapOrder(v); }
    inline SInt64   LEToSystem(SInt64 v)    { return SwapOrder(v); }
    inline float    LEToSystem(float  v)    { return SwapOrder(v); }
    inline Double   LEToSystem(Double v)    { return SwapOrder(v); }
    // Big Endian to System (BE)
    inline UInt8    BEToSystem(UInt8  v)    { return v; }
    inline SInt8    BEToSystem(SInt8  v)    { return v; }
    inline UInt16   BEToSystem(UInt16 v)    { return v; }
    inline SInt16   BEToSystem(SInt16 v)    { return v; }
    inline UInt32   BEToSystem(UInt32 v)    { return v; }
    inline SInt32   BEToSystem(SInt32 v)    { return v; }
    inline UInt64   BEToSystem(UInt64 v)    { return v; }
    inline SInt64   BEToSystem(SInt64 v)    { return v; }
    inline float    BEToSystem(float  v)    { return v; }
    inline Double   BEToSystem(Double v)    { return v; }
    // System (BE) to Little Endian
    inline UInt8    SystemToLE(UInt8  v)    { return SwapOrder(v); }
    inline SInt8    SystemToLE(SInt8  v)    { return SwapOrder(v); }
    inline UInt16   SystemToLE(UInt16 v)    { return SwapOrder(v); }
    inline SInt16   SystemToLE(SInt16 v)    { return SwapOrder(v); }
    inline UInt32   SystemToLE(UInt32 v)    { return SwapOrder(v); }
    inline SInt32   SystemToLE(SInt32 v)    { return SwapOrder(v); }
    inline UInt64   SystemToLE(UInt64 v)    { return SwapOrder(v); }
    inline SInt64   SystemToLE(SInt64 v)    { return SwapOrder(v); }
    inline float    SystemToLE(float  v)    { return SwapOrder(v); }
    inline Double   SystemToLE(Double v)    { return SwapOrder(v); }
    // System (BE) to Big Endian
    inline UInt8    SystemToBE(UInt8  v)    { return v; }
    inline SInt8    SystemToBE(SInt8  v)    { return v; }
    inline UInt16   SystemToBE(UInt16 v)    { return v; }
    inline SInt16   SystemToBE(SInt16 v)    { return v; }
    inline UInt32   SystemToBE(UInt32 v)    { return v; }
    inline SInt32   SystemToBE(SInt32 v)    { return v; }
    inline UInt64   SystemToBE(UInt64 v)    { return v; }
    inline SInt64   SystemToBE(SInt64 v)    { return v; }
    inline float    SystemToBE(float  v)    { return v; }
    inline Double   SystemToBE(Double v)    { return v; }

#else
    #error "SF_BYTE_ORDER must be defined to SF_LITTLE_ENDIAN or SF_BIG_ENDIAN"
#endif

} // namespace ByteUtil


template <class T>
inline UInt8 Cardinality(T v)
{
    const T MASK_01010101 = (((T)(-1))/3);
    const T MASK_00110011 = (((T)(-1))/5);
    const T MASK_00001111 = (((T)(-1))/17);

    v = (v & MASK_01010101) + ((v >> 1) & MASK_01010101) ; 
    v = (v & MASK_00110011) + ((v >> 2) & MASK_00110011) ; 
    v = (v & MASK_00001111) + ((v >> 4) & MASK_00001111) ; 
    return (UInt8)(v % 255);
}


inline UInt8 BitCount32(UInt32 value)
{
    /* Binary search - decision tree (5 tests, rarely 6) */
    return
        ((value < 1UL<<15) ?
        ((value < 1UL<<7) ?
        ((value < 1UL<<3) ?
        ((value < 1UL<<1) ? ((value < 1UL<<0) ? 0 : 1) : ((value < 1UL<<2) ? 2 : 3)) :
        ((value < 1UL<<5) ? ((value < 1UL<<4) ? 4 : 5) : ((value < 1UL<<6) ? 6 : 7))) :
        ((value < 1UL<<11) ?
        ((value < 1UL<<9) ? ((value < 1UL<<8) ? 8 : 9) : ((value < 1UL<<10) ? 10 : 11)) :
        ((value < 1UL<<13) ? ((value < 1UL<<12) ? 12 : 13) : ((value < 1UL<<14) ? 14 : 15)))) :
        ((value < 1UL<<23) ?
        ((value < 1UL<<19) ?
        ((value < 1UL<<17) ? ((value < 1UL<<16) ? 16 : 17) : ((value < 1UL<<18) ? 18 : 19)) :
        ((value < 1UL<<21) ? ((value < 1UL<<20) ? 20 : 21) : ((value < 1UL<<22) ? 22 : 23))) :
        ((value < 1UL<<27) ?
        ((value < 1UL<<25) ? ((value < 1UL<<24) ? 24 : 25) : ((value < 1UL<<26) ? 26 : 27)) :
        ((value < 1UL<<29) ? ((value < 1UL<<28) ? 28 : 29) : ((value < 1UL<<30) ? 30 : 
        ((value < 1UL<<31) ? 31 : 32))))));
}

}} // Scaleform Alg

#endif
