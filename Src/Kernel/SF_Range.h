/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Range.h
Content     :   Range template types and functions
Created     :   April 17, 2007
Authors     :   Artyom Bolgar, Maxim Shemanarev 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Range_H
#define INC_SF_Kernel_Range_H

#include "SF_Types.h"
#include "SF_Debug.h"
#include "SF_Memory.h"
#include "SF_Math.h"
#include "SF_Array.h"

namespace Scaleform {

class Range
{
public:
    SPInt    Index;
    UPInt    Length;

    enum BoundsType { Min, Max };

    Range():Index(0), Length(0) {}
    explicit Range(UPInt count)        { SetRange(count); }
    Range(SPInt index, UPInt length)   { SetRange(index, length); }
    Range(const Range &r)              { SetRange(r); }
    Range(BoundsType bt)               { SetRange(bt); }

    // *** Initialization

    void    SetRange(UPInt count)   { Index = 0; Length = count; }
    // first and last inclusive
    void    SetRange(SPInt index, UPInt length)     
    { 
        Index = index;
        Length = length;
    }
    void    SetRange(const Range &r)           { SetRange(r.Index, r.Length); }
    void    SetRange(BoundsType bt)
    {
        switch (bt)
        {
        case Min:   SetRange(0, 0); break;
        case Max:   SetRange(0, SF_MAX_UPINT); break;
        }
    }

    // moves index and decrease the length, thus NextIndex remains same. 
    // Positive delta shows direct direction, negative - reverse.
    SPInt MoveIndex(SPInt delta) 
    {
        delta = (delta > SPInt(Length)) ? SPInt(Length) : delta;
        Index += delta; 
        Length = (UPInt)(Length - delta);
        return Index; 
    }
    // moves whole range keeping the original length
    SPInt MoveRange(SPInt delta) 
    { 
        Index += delta; 
        return Index; 
    }

    // Shrink range's length
    UPInt ShrinkRange(UPInt lengthDelta) 
    {
        if (lengthDelta > Length) 
            Length = 0;
        else
            Length -= lengthDelta;
        return Length;
    }
    UPInt ExpandRange(UPInt lengthDelta)
    {
        Length += lengthDelta;
        return Length;
    }

    // returns the next available index after the range
    SPInt NextIndex() const { return Index + Length; }
    // returns the last included index in the range
    SPInt LastIndex() const { return Index + Length - 1; }
    SPInt FirstIndex() const { return Index; }
    bool IsEmpty() const { return Length == 0; }

    // Reset the range to first = 0 and count = 0       
    void    Clear()                             { SetRange(Min); }

    // *** Intersection

    // Tests whether a value is inside the range
    bool    Contains(SPInt index) const         { return (index >= Index && index <= LastIndex()); }    
    // Tests whether the range is completely inside this one
    bool    Contains(const Range &r) const      { return (r.Index >= Index && r.LastIndex() <= LastIndex()); }

    // Tests whether the range intersects this one
    bool    Intersects(const Range &r) const    { return (r.LastIndex() >= FirstIndex()) && (LastIndex() >= r.FirstIndex()); }

    int     CompareTo(SPInt index) const
    {
        if (Contains(index)) 
            return 0;
        if (index < Index) 
            return int(Index - index);
        else
            return int(LastIndex() - index);
    }
};

template <class T>
class RangePrimData : public Range
{
    T       Data;
public:
    RangePrimData():Range() {}
    // reference dowsn't work for primitive types' constants
    //??RangeData(SPInt index, UPInt length, const T& data):Range(index, length),Data(data) {}
    RangePrimData(SPInt index, UPInt length, const T data):Range(index, length),Data(data) {}

    T GetData() const               { return Data; }
    void SetData(T data)            { Data = data; }
    bool IsDataEqual(T data) const  { return Data == data; }
};

template <class T>
class RangeData : public Range
{
    T       Data;
public:
    RangeData():Range() {}
    RangeData(SPInt index, UPInt length, const T& data):Range(index, length),Data(data) {}

    T& GetData()                           { return Data; }
    const T& GetData() const               { return Data; }
    T& operator->()                        { return Data; }
    const T& operator->() const            { return Data; }
    T& operator*()                         { return Data; }
    const T& operator*() const             { return Data; }
    void SetData(const T& data)            { Data = data; }
    bool IsDataEqual(const T& data) const  { return Data == data; }
};

template <class T, class Array = Scaleform::Array<RangeData<T> > >
class RangeDataArray
{
    UPInt FindRangeIndex(SPInt index) const;
    UPInt FindNearestRangeIndex(SPInt index) const;
public:
    typedef RangeData<T>                            TypedRangeData;
    typedef Array                                   RangeArrayType;

    RangeArrayType Ranges;

    // just returns range at the specified position in Ranges array
    TypedRangeData& operator[](UPInt position)             { return Ranges[position]; }
    const TypedRangeData& operator[](UPInt position) const { return Ranges[position]; }
    UPInt Count() const { return Ranges.GetSize(); }

    void SetRange(SPInt index, UPInt length, const T& data)
    {
        SetRange(TypedRangeData(index, length, data));
    }
    void SetRange(const TypedRangeData& range);
    // just clears the range w/o collapsing the array
    void ClearRange(SPInt index, UPInt length);

    TypedRangeData& Append(UPInt length, const T& data);
    void InsertRange(SPInt startPos, UPInt length, const T& pdata);
    void ExpandRange(SPInt startPos, UPInt length);
    // removes range and collapse the range array, thus no new hole is produced
    void RemoveRange(SPInt startPos, UPInt length);
    void RemoveAll() { Ranges.Resize(0); }

    class Iterator
    {
        friend class RangeDataArray<T, Array>;
        RangeDataArray<T, Array>*   pArray;
        SPInt                       Index;

        explicit Iterator(RangeDataArray<T, Array>& arr) : pArray(&arr), Index(0) {}
        Iterator(RangeDataArray<T, Array>& arr, bool) : pArray(&arr), Index(pArray->Count() - 1) {}

        // inserts a range at the current position
        void InsertBefore(const TypedRangeData& range);
        void InsertAfter(const TypedRangeData& range);
        // remove one range at the current position
        void Remove();

    public:
        Iterator(const Iterator& it) : pArray(it.pArray), Index(it.Index) {}
        Iterator():pArray(0), Index(-1) {}

        Iterator& operator++() 
        { 
            if (Index < (SPInt)pArray->Count())
                ++Index; 
            return *this; 
        }    
        Iterator operator++(int) 
        { 
            Iterator it(*this); 
            if (Index < (SPInt)pArray->Count())
                ++Index; 
            return it; 
        }   
        Iterator& operator--() 
        { 
            if (Index >= 0)
                --Index; 
            return *this; 
        }    
        Iterator operator--(int) 
        { 
            Iterator it(*this); 
            if (Index >= 0)
                --Index; 
            return it; 
        }   
        TypedRangeData& operator*()  { return (*pArray)[Index]; }
        TypedRangeData* operator->() { return &(*pArray)[Index]; }
        TypedRangeData* GetPtr()     { return (Index < (SPInt)pArray->Count()) ? &(*pArray)[Index] : NULL; }
        bool operator==(const Iterator& it) const
        {
            return (pArray == it.pArray && Index == it.Index);
        }
        Iterator operator+(SPInt delta) const
        {
            Iterator it(*this);
            if (Index + delta < 0)
                it.Index = 0;
            else if (UPInt(Index + delta) >= pArray->Count())
                it.Index = pArray->Count() - 1;
            else
                it.Index += delta;
            
            return it;
        }
        bool IsFinished() const { return Index < 0 || UPInt(Index) >= pArray->Count(); }
    };
    Iterator Begin() { return Iterator(*this); }
    Iterator Last()  { return Iterator(*this, true); }
    Iterator GetIteratorAt(SPInt index);
    Iterator GetIteratorByNearestIndex(SPInt index);

    class ConstIterator
    {
        friend class RangeDataArray<T, Array>;
        const RangeDataArray<T, Array>* pArray;
        SPInt                           Index;

        explicit ConstIterator(const RangeDataArray<T, Array>& arr) : pArray(&arr), Index(0) {}
        ConstIterator(const RangeDataArray<T, Array>& arr, bool) : pArray(&arr), Index(pArray->Count() - 1) {}

    public:
        ConstIterator(const ConstIterator& it) : pArray(it.pArray), Index(it.Index) {}
        ConstIterator():pArray(0), Index(-1) {}

        ConstIterator& operator++() 
        { 
            if (Index < (SPInt)pArray->Count())
                ++Index; 
            return *this; 
        }    
        ConstIterator operator++(int) 
        { 
            ConstIterator it(*this); 
            if (Index < (SPInt)pArray->Count())
                ++Index; 
            return it; 
        }   
        ConstIterator& operator--() 
        { 
            if (Index >= 0)
                --Index; 
            return *this; 
        }    
        ConstIterator operator--(int) 
        { 
            ConstIterator it(*this); 
            if (Index >= 0)
                --Index; 
            return it; 
        }   
        const TypedRangeData& operator*() const  { return (*pArray)[Index]; }
        const TypedRangeData* operator->() const { return &(*pArray)[Index]; }
        const TypedRangeData* GetPtr() const     { return (Index < (SPInt)pArray->Count()) ? &(*pArray)[Index] : NULL; }
        bool operator==(const ConstIterator& it) const
        {
            return (pArray == it.pArray && Index == it.Index);
        }
        ConstIterator operator+(SPInt delta) const
        {
            ConstIterator it(*this);
            if (Index + delta < 0)
                it.Index = 0;
            else if (UPInt(Index + delta) >= pArray->Count())
                it.Index = pArray->Count() - 1;
            else
                it.Index += delta;
            
            return it;
        }
        bool IsFinished() const { return Index < 0 || UPInt(Index) >= pArray->Count(); }
    };
    ConstIterator Begin() const { return ConstIterator(*this); }
    ConstIterator Last() const  { return ConstIterator(*this, true); }
    ConstIterator GetIteratorAt(SPInt index) const;
    ConstIterator GetIteratorByNearestIndex(SPInt index) const;

    class ConstPositionIterator
    {
        friend class RangeDataArray<T, Array>;
        ConstIterator    Iter;
        const TypedRangeData* pCurrentRange; 
        SPInt             CurrentPos;
        SPInt             EndPos;
    public:
        ConstPositionIterator(const RangeDataArray<T, Array>& rangeArray) : Iter(rangeArray.Begin()) 
        {
            if (!Iter.IsFinished())
            {
                CurrentPos = Iter->FirstIndex();
                pCurrentRange = Iter.GetPtr();
                EndPos = rangeArray[rangeArray.Count() - 1].LastIndex();
            }
            else
            {
                CurrentPos = 0;
                pCurrentRange = NULL;
                EndPos = -1;
            }
        }
        ConstPositionIterator(const RangeDataArray<T, Array>& rangeArray, SPInt startIndex, UPInt length) :
            Iter(rangeArray.GetIteratorByNearestIndex(startIndex)), CurrentPos(startIndex),
            EndPos(CurrentPos + length - 1)
        {
            if (!Iter.IsFinished())
            {
                pCurrentRange = Iter.GetPtr();
            }
            else
            {
                pCurrentRange = NULL;
            }
        }

        bool IsFinished() const { return CurrentPos > EndPos; }

        const TypedRangeData* operator*() const  { return GetPtr(); }
        const TypedRangeData* operator->() const 
        { 
            return GetPtr(); 
        }
        const TypedRangeData* GetPtr() const
        { 
            if (pCurrentRange)
            {
                if (pCurrentRange->Contains(CurrentPos))
                    return pCurrentRange;
            }
            return NULL; 
        }

        ConstPositionIterator& operator++() 
        { 
            if (CurrentPos <= EndPos)
            {
                SPInt oldPos = CurrentPos++;
                if (pCurrentRange)
                {
                    if (pCurrentRange->Contains(oldPos) && !pCurrentRange->Contains(CurrentPos))
                    {
                        ++Iter;
                        if (!Iter.IsFinished())
                            pCurrentRange = Iter.GetPtr();
                        else
                            pCurrentRange = NULL;
                    }
                }
            }
            return *this; 
        }    
        ConstPositionIterator operator++(int) 
        { 
            ConstIterator it(*this); 
            operator++();
            return it; 
        }   
    };

    /*void    set_heap(HeapType& heap)
    {
        Ranges.set_heap(heap);
    }
    void    set_heap(HeapType* pheap)
    {
        Ranges.set_heap(pheap);
    } */

#ifdef SF_BUILD_DEBUG
    void CheckIntegrity();
#else
    void CheckIntegrity() {}
#endif
protected:

};

// inserts a range at the current position
template <class T, class Array>
void RangeDataArray<T, Array>::Iterator::InsertBefore(const TypedRangeData& range)
{
    pArray->Ranges.InsertAt(Index, range);
}

template <class T, class Array>
void RangeDataArray<T, Array>::Iterator::InsertAfter(const TypedRangeData& range)
{
    pArray->Ranges.InsertAt(Index+1, range);
}

// remove one range at the current position
template <class T, class Array>
void RangeDataArray<T, Array>::Iterator::Remove()
{
    if (Index >= 0 && UPInt(Index) < pArray->Count())
        pArray->Ranges.RemoveAt(Index);
}

template <class T, class Array>
UPInt RangeDataArray<T, Array>::FindRangeIndex(SPInt index) const
{
    // do a binary search
    const UPInt count = Count();

    UPInt lower = 0;
    UPInt upper = count - 1;

    while (lower < upper && upper != (UPInt)-1)
    {
        UPInt middle = (lower + upper)/2;
        int   cmpRes = Ranges[middle].CompareTo(index);
        if (cmpRes == 0) { // equal
            return middle;
        }
        if (cmpRes < 0) // *mpArray[middle] < *elem
            lower = middle+1;
        else
            upper = middle-1;
    }

    if (lower == upper && Ranges[lower].CompareTo(index) == 0)
        return lower;
    return SF_MAX_UPINT;
}


template <class T, class Array>
typename RangeDataArray<T, Array>::Iterator RangeDataArray<T, Array>::GetIteratorAt(SPInt index)
{
    UPInt rangeIndex = FindRangeIndex(index);
    if (rangeIndex != SF_MAX_UPINT)
        return Begin() + rangeIndex;
    return Iterator();
}

template <class T, class Array>
typename RangeDataArray<T, Array>::ConstIterator RangeDataArray<T, Array>::GetIteratorAt(SPInt index) const
{
    UPInt rangeIndex = FindRangeIndex(index);
    if (rangeIndex != SF_MAX_UPINT)
        return Begin() + rangeIndex;
    return ConstIterator();
}

template <class T, class Array>
UPInt RangeDataArray<T, Array>::FindNearestRangeIndex(SPInt index) const
{
    // do a binary search
    const UPInt count = Count();
    if (count == 0)
    {
        return 0;
    }

    UPInt lower = 0;
    UPInt upper = count - 1;
    UPInt lowest = 0;

    while (lower < upper && upper != (UPInt)-1) {
        UPInt middle = (lower + upper)/2;
        int cmpRes = Ranges[middle].CompareTo(index);
        if (cmpRes == 0) { // equal
            return middle;
        }
        if (cmpRes < 0) // *mpArray[middle] < *elem
        {
            lowest = lower;
            lower = middle+1;
        }
        else
            upper = middle-1;
    }

    if (lower == upper && Ranges[lower].CompareTo(index) == 0)
        return lower;
    while(lowest < upper && Ranges[lowest + 1].CompareTo(index) < 0)
        ++lowest;
    return lowest;
}

template <class T, class Array>
typename RangeDataArray<T, Array>::Iterator RangeDataArray<T, Array>::GetIteratorByNearestIndex(SPInt index)
{
    return Begin() + FindNearestRangeIndex(index);
}

template <class T, class Array>
typename RangeDataArray<T, Array>::ConstIterator RangeDataArray<T, Array>::GetIteratorByNearestIndex(SPInt index) const
{
    return Begin() + FindNearestRangeIndex(index);
}

template <class T, class Array>
void RangeDataArray<T, Array>::SetRange(const TypedRangeData& range)
{
    if (Count() > 0)
    {
        Iterator it = GetIteratorByNearestIndex(range.Index);
        Iterator insertionPoint;

        // split the first range
        if (it->Contains(range))
        {
            // inserting range is inside the existing range?
            if (it->Index == range.Index)
            {
                // beginnings of ranges are same?
                // |=======||====| - array
                // |+++|           - range
                // |+++|===||====| - result
                it->MoveIndex(range.Length);
                
                if (it->IsEmpty())
                {
                    *it = range;
                }
                else
                    it.InsertBefore(range);
                insertionPoint = it;
                ++it;
            }
            else if (it->NextIndex() > range.NextIndex())
            {
                // |==========||====| - array
                //    |+++|           - range
                // |==|+++|===||====| - result
                // inserting range is completely inside the current range
                // split the current range by 3 parts and replace the middle one by "range"
                TypedRangeData r = *it;
                SPInt endIndex = it->NextIndex();
                it->ShrinkRange(endIndex - range.Index);
                r.MoveIndex(it->Length + range.Length);
                it.InsertAfter(range);
                ++it;
                insertionPoint = it;
                it.InsertAfter(r);
                ++it;
            }
            else
            {
                // |==========||====| - array
                //        |+++|       - range
                // |======|+++||====| - result
                // inserting range intersects the current range
                it->ShrinkRange(range.Length);
                insertionPoint = ++it;
                it.InsertBefore(range);
                ++it;
            }
        }
        else if (it->Contains(range.Index))
        {
            // inserting range intersects the current range
            // |==========||====| - array
            //        |+++++++|   - range
            // |======|   ||====| - result at this stage
            it->ShrinkRange(it->NextIndex() - range.Index);
            insertionPoint = ++it;
            it.InsertBefore(range);
            ++it;
        }
        else
        {
            // beginning of inserting range is on empty space
            // |==========|          |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //            |+++++++|         - range
            if (it->CompareTo(range.Index) > 0)
            {
                it.InsertBefore(range);
                insertionPoint = it;
            }
            else
            {
                it.InsertAfter(range);
                insertionPoint = ++it;
            }
            ++it;
        }
        // remove all ranges in between
        while(!it.IsFinished() && range.Contains(*it))
        {
            it.Remove();
        }
        // shrink the last range
        if (!it.IsFinished() && it->Contains(range.NextIndex() - 1))
        {
            // |==========||======| - array
            //        |+++++++|     - range
            // |==========|   |===| - result
            it->MoveIndex(range.NextIndex() - it->Index);
        }
        // check if is it possible to unite ranges
        Iterator firstRangeIt = insertionPoint;
        --firstRangeIt;
        // check the previous range
        if (!firstRangeIt.IsFinished())
        {
            if (firstRangeIt->IsEmpty())
            {
                // need to correct the insertionPoint accordingly
                --insertionPoint;
                firstRangeIt.Remove();
            }
            else  if (firstRangeIt->NextIndex() == range.Index && firstRangeIt->IsDataEqual(insertionPoint->GetData()))
            {
                // expand first range by the range.Length
                firstRangeIt->ExpandRange(range.Length);
                insertionPoint.Remove();
                insertionPoint = firstRangeIt;
            }
        }
        // check the next range
        Iterator nextIt = insertionPoint;
        ++nextIt;
        if (!nextIt.IsFinished())
        {
            if (nextIt->IsEmpty())
                nextIt.Remove();
            else if (insertionPoint->NextIndex() == nextIt->Index && insertionPoint->IsDataEqual(nextIt->GetData()))
            {
                // expand range by the length of nextIt
                insertionPoint->ExpandRange(nextIt->Length);
                nextIt.Remove();
            }
        }
    }
    else
    {
        Begin().InsertBefore(range);
    }
    CheckIntegrity();
}

template <class T, class Array>
typename RangeDataArray<T, Array>::TypedRangeData& RangeDataArray<T, Array>::Append(UPInt length, const T& data)
{
    // check data at the last element in the array: if same as our, then just expand existing range by length
    UPInt count = Count();
    TypedRangeData* pLastRange = NULL;
    if (count > 0)
    {
        pLastRange = &Ranges[count - 1];
        if (count > 0 && pLastRange->IsDataEqual(data))
        {
            pLastRange->Length += length;
            return *pLastRange;
        }
    }
    // otherwise, create a new range
    Ranges.Resize(count + 1);
    TypedRangeData& newRange = Ranges[count];
    if (pLastRange)
    {
        newRange.Index = pLastRange->Index + pLastRange->Length;
    }
    else
    {
        // insert new range as first one
        newRange.Index = 0;
    }
    newRange.Length = length;
    newRange.SetData(data);
    CheckIntegrity();
    return newRange;
}

template <class T, class Array>
void RangeDataArray<T, Array>::InsertRange(SPInt startPos, UPInt length, const T& data)
{
    ExpandRange(startPos, length);
    SetRange(startPos, length, data);
    CheckIntegrity();
}

template <class T, class Array>
void RangeDataArray<T, Array>::ExpandRange(SPInt startPos, UPInt length)
{
    if (Count() > 0)
    {
        Iterator it = GetIteratorByNearestIndex(startPos);
        TypedRangeData* pnearest = it.GetPtr();
        if (pnearest)
        {
            if (pnearest->Contains(startPos) || pnearest->NextIndex() == startPos)
                pnearest->Length += length;
        }
        // update indices for all following ranges
        for(++it; !it.IsFinished(); ++it)
        {
            it->MoveRange((SPInt)length);
        }
    }
    CheckIntegrity();
}

template <class T, class Array>
void RangeDataArray<T, Array>::RemoveRange(SPInt startPos, UPInt length)
{
    CheckIntegrity();
    if (Count() > 0)
    {
        Iterator it = GetIteratorByNearestIndex(startPos);
        Iterator removalPoint;

        if (length == SF_MAX_UPINT) // special case, remove everything till the end
            length = SF_MAX_SPINT - startPos;

        TypedRangeData range(startPos, length, T());
        TypedRangeData& r = *it;
        // split the first range
        if (r.Contains(range))
        {
            // is inserting range inside the existing range?
            if (r.Index == range.Index)
            {
                // beginnings of ranges are the same?
                // |0123456||====| - array
                // |+++|           - range
                //    |3456||====| - result at this stage
                r.MoveIndex(range.Length);

                if (r.IsEmpty())
                    it.Remove();
                removalPoint = it;
            }
            else if (r.NextIndex() > range.NextIndex())
            {
                // |1234567890||====| - array
                //    |+++|           - range
                // |1237890|   |====| - result  at this stage
                // inserting range is completely inside the current range
                // split the current range by 3 parts and replace the middle one by "range"
                r.ShrinkRange(range.Length);
                if (r.IsEmpty())
                    it.Remove();
                else
                    ++it;
                removalPoint = it;
            }
            else
            {
                // |==========||====| - array
                //        |+++|       - range
                // |======|    |====| - result at this stage
                // inserting range intersects the current range
                r.ShrinkRange(range.Length);
                removalPoint = ++it;
                ++it;
            }
        }
        else if (r.Contains(range.Index))
        {
            // inserting range intersects the current range
            // |==========||====| - array
            //        |+++++++|   - range
            // |======|   ||====| - result at this stage
            r.ShrinkRange(r.NextIndex() - range.Index);
            if (r.IsEmpty())
                it.Remove();
            else
                ++it;
            removalPoint = it;
        }
        else
        {
            // beginning of inserting range is on empty space
            // |==========|          |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //            |+++++++|         - range
            if (r.CompareTo(range.Index) > 0)
            {
                removalPoint = it;
            }
            else
            {
                removalPoint = ++it;
            }
        }
        // remove all ranges in between
        while(!it.IsFinished() && range.Contains(*it))
        {
            it.Remove();
        }
        // shrink the last range
        if (!it.IsFinished() && it->Contains(range.NextIndex() - 1))
        {
            // |==========||======| - array
            //        |+++++++|     - range
            // |==========|   |===| - result
            it->MoveIndex(range.NextIndex() - it->Index);
        }

        Iterator firstRangeIt = removalPoint;
        --firstRangeIt;
        // check, can we unite ranges
        if (!firstRangeIt.IsFinished() && !removalPoint.IsFinished() && 
            firstRangeIt->NextIndex() == removalPoint->Index - SPInt(length) && 
            firstRangeIt->IsDataEqual(removalPoint->GetData()))
        {
            // we can
            firstRangeIt->ExpandRange(removalPoint->Length);
            removalPoint.Remove();
        }

        // update indices for all following ranges
        for(; !removalPoint.IsFinished(); ++removalPoint)
        {
            removalPoint->MoveRange(-((SPInt)length));
        }
    }
    CheckIntegrity();
}

// TODO!
template <class T, class Array>
void RangeDataArray<T, Array>::ClearRange(SPInt startPos, UPInt length)
{
    CheckIntegrity();
    if (Count() > 0)
    {
        Iterator it = GetIteratorByNearestIndex(startPos);
        Iterator removalPoint;

        if (length == SF_MAX_UPINT) // special case, remove everything till the end
            length = SF_MAX_SPINT - startPos;

        TypedRangeData range(startPos, length, T());
        // split the first range
        if (it->Contains(range))
        {
            // is inserting range inside the existing range?
            if (it->Index == range.Index)
            {
                // beginnings of ranges are the same?
                // |=======||====| - array
                // |+++|           - range
                //     |===||====| - result
                it->MoveIndex(range.Length);

                removalPoint = it;
                if (it->IsEmpty())
                    it.Remove();
                else
                    ++it;
            }
            else if (it->NextIndex() > range.NextIndex())
            {
                // |==========||====| - array
                //    |+++|           - range
                // |==|   |===||====| - result
                // inserting range is completely inside the current range
                // split the current range by 3 parts and replace the middle one by "range"
                TypedRangeData r = *it;
                SPInt endIndex = it->NextIndex();
                it->ShrinkRange(endIndex - range.Index);
                r.MoveIndex(it->Length + range.Length);
                ++it;
                removalPoint = it;
                it.InsertBefore(r);
                ++it;
            }
            else
            {
                // |==========||====| - array
                //        |+++|       - range
                // |======|    |====| - result
                // inserting range intersects the current range
                it->ShrinkRange(range.Length);
                removalPoint = ++it;
                ++it;
            }
        }
        else if (it->Contains(range.Index))
        {
            // inserting range intersects the current range
            // |==========||====| - array
            //        |+++++++|   - range
            // |======|   ||====| - result at this stage
            it->ShrinkRange(it->NextIndex() - range.Index);
            removalPoint = ++it;
            ++it;
        }
        else
        {
            // beginning of inserting range is on empty space
            // |==========|          |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //                   |+++++++|   - range
            // or
            //                       |====| - array
            //            |+++++++|         - range
            if (it->CompareTo(range.Index) > 0)
            {
                removalPoint = it;
                ++it;
            }
            else
            {
                removalPoint = ++it;
            }
        }
        // remove all ranges in between
        while(!it.IsFinished() && range.Contains(*it))
        {
            it.Remove();
        }
        // shrink the last range
        if (!it.IsFinished() && it->Contains(range.NextIndex() - 1))
        {
            // |==========||======| - array
            //        |+++++++|     - range
            // |==========|   |===| - result
            it->MoveIndex(range.NextIndex() - it->Index);
        }
    }
    CheckIntegrity();
}

#ifdef SF_BUILD_DEBUG
template <class T, class Array>
void RangeDataArray<T, Array>::CheckIntegrity()
{
    SPInt curIndex = SF_MIN_SPINT;
    TypedRangeData* pprev = NULL;
    for (UPInt i = 0, n = Ranges.GetSize(); i < n; ++i)
    {
        TypedRangeData& r = Ranges[i];
        if (pprev)
        {
            if (pprev->Index + (SPInt)pprev->Length == r.Index)
                SF_ASSERT(pprev->GetData() != r.GetData()); // detect not united ranges
        }
        SF_ASSERT(r.Length); // detect empty ranges
        SF_ASSERT(curIndex <= r.FirstIndex());
        curIndex = r.NextIndex();
        pprev = &r;
    }
}
#endif

} // Scaleform

#endif // INC_SF_Kernel_Range_H

