/**************************************************************************

Filename    :   AS3_ValueStack.h
Content     :   
Created     :   Mar, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_ValueArray_H
#define INC_AS3_ValueArray_H

#include "AS3_Value.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// Value Stack Base
// Destructor has to be implemented by a child class.
class VSBase
{
    friend class Value;

public:
    typedef Value ValueType;

public:
    VSBase(ValueType* pst SF_DEBUG_ARG(UPInt n)) 
        : pCurrent(pst - 1)
        , pStack(pst)
        SF_DEBUG_ARG(NumOfReservedElem(n))
    {
    }

public:
    SF_INLINE Value* GetCurrent() const { return pCurrent; }

    SF_INLINE UPInt GetSize() const { return pCurrent + 1 - pStack; }

    // Top0() and Top1() are used most commonly to access the stack.
    // Inline and optimize them as much as possible.

    SF_INLINE Value&   Top0() const
    {        
        SF_ASSERT(pCurrent >= pStack);
        return *pCurrent;
    }

    SF_INLINE Value&   Top1() const
    {
        SF_ASSERT(pCurrent - 1 >= pStack);
        return *(pCurrent - 1);
    }
    SF_INLINE Value&   TopN(UInt16 n) const
    {     
        SF_ASSERT(pCurrent - n >= pStack);
        return *(pCurrent - n);
    }

    // Methods below are for compatibility with GFx Array.
    SF_INLINE const Value& Back() const
    {        
        SF_ASSERT(pCurrent >= pStack);
        return *pCurrent;
    }
    SF_INLINE Value& Back()
    {        
        SF_ASSERT(pCurrent >= pStack);
        return *pCurrent;
    }

    // Get primitive value from the top of the stack
    template <typename T>
    SF_INLINE T PopPrimitive()
    {
        SF_ASSERT(pCurrent >= pStack);
        // Get result and forget the value.
        return *pCurrent--;
    }

    // Push default value.
    SF_INLINE void PushBack()
    {
        ++pCurrent;
        Construct<ValueType>(pCurrent);
    }

    SF_INLINE void PushBack(const ValueType& val)
    {
        ++pCurrent;
        // Use copy constructor to initialize stack top.
        Construct(pCurrent, val);
    }

    SF_INLINE void PickPushBack(ValueType& val)
    {
        ++pCurrent;
        // Use pick constructor to initialize stack top.
        ::new(pCurrent) ValueType(val, PickValue);
    }

    // Push back not reference counted value.
    SF_INLINE void PushBackNRC(ValueType& val)
    {
        ++pCurrent;
        ::new(pCurrent) ValueType(val, ValueType::NotRefCounted);
    }

    // Pop one element from stack. 
    SF_INLINE void PopBack()
    {
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
    }

    SF_INLINE void PopBack2()
    {
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
    }

    void    PopBack3()
    {
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
        Scaleform::Destruct(pCurrent);
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
    }

    // General Pop(number) implementation.
    void    PopBack(unsigned n)
    {
        while(n-- > 0)
            PopBack();
    }

    // General Push(number) implementation.
    // Push default values.
    void    PushBack(unsigned n)
    {
        while(n-- > 0)
            PushBack();
    }

    void    Release0()
    {
        SF_ASSERT(pCurrent >= pStack);
        --pCurrent;
    }

    void    ReleaseN(UPInt n)
    {
        SF_ASSERT(n <= GetSize());
        pCurrent -= n;
    }

    void    Acquire1()
    {
#ifdef SF_BUILD_DEBUG
        SF_ASSERT(GetSize() < NumOfReservedElem);
#endif
        ++pCurrent;
        *(UPInt*)pCurrent = 0;
    }

    void    SwapTop()
    {
        Value _1(Top0(), PickValue);
        Top0().PickUnsafe(Top1());
        Top1().PickUnsafe(_1);
    }

protected:
    // Pointer to the current element.
    // In case of a stack initial value should point to an element previous
    // to a first valid pointer so we can safely increase it in Push().
    ValueType*  pCurrent; 
    ValueType*  pStack;
    SF_DEBUG_CODE(UPInt NumOfReservedElem;) 
};

///////////////////////////////////////////////////////////////////////////
// Fixed size Value Stack.
template <UPInt N>
class FixedVS : public VSBase
{
public:
    FixedVS() : VSBase(Stack SF_DEBUG_ARG(N)) {}
    // ~FixedVS() We do not need an explicit destructor.
    // Default one will be just fine.

private:
    Value   Stack[N];
};

///////////////////////////////////////////////////////////////////////////
class ValueStack : public VSBase
{
public:
    typedef Value ValueType;
    enum {DefPageSize = 64};

    // Page structure. This structure is SF_ALLOC'ed and NOT new'd because 
    // we need to manage construction/cleanup of T values manually.    
    struct Page
    {
        UInt16      ReservationNum; // Number of reservations.
        UInt16      PageSize; // Total number of values
        // Last pFirst value before switching to a new page.
        ValueType*  pFirst;
        // Last pCurrent value before switching to a new page.
        ValueType*  pCurrent;
        Page*       pNext;
        Page*       pPrev;
        ValueType   Values[1]; // Actual values.
    };

public:
    ValueStack();
    ~ValueStack();

public:
    void Reserve(UInt16 n);
    void ReleaseReserved(ValueType* first SF_DEBUG_ARG(UInt16 prevReservNum));
    // Preserve return value in case of retVal == true.
    void PopReserved(ValueType* current, bool retVal);

    // ***** High-performance Stack Access

    SF_DEBUG_CODE(SF_INLINE bool IsValid() const { return GetSize() <= GetNumOfReservedElem(); })

    SF_INLINE Value* GetFirst() const { return pStack; }
    SF_DEBUG_CODE(UInt16 GetNumOfReservedElem() const { return static_cast<UInt16>(NumOfReservedElem); })

private:
    // *** Page allocator.
    Page*   AllocPage(UInt16 pageSize);
    Page*   NewPage(UInt16 pageSize);

private:
    Page*   pCurrentPage; // Pointer to the current page.
    Page*   pReserved; // Already allocated pages in a linked list.
};

///////////////////////////////////////////////////////////////////////////
// Value Register File base.
// Destructor has to be implemented by a child class.
class VRFBase
{
public:
    typedef Value ValueType;

public:
    VRFBase(ValueType* prf, UInt16 n) 
    : ReservedNum(n)
    , pRF(prf)
    {
    }

public:
    // Operators.

    const ValueType& operator [] (UPInt i) const
    {
        SF_ASSERT(i < ReservedNum);
        return pRF[i];
    }

    ValueType& operator [] (UPInt i)
    {
        SF_ASSERT(i < ReservedNum);
        return pRF[i];
    }

    const ValueType& At(UPInt i) const
    { 
        SF_ASSERT(i < ReservedNum);
        return pRF[i];
    }

    ValueType& At(UPInt i) 
    { 
        SF_ASSERT(i < ReservedNum);
        return pRF[i];
    }

protected:
    UInt16      ReservedNum; // Total number of reserved elements in the current page.
    ValueType*  pRF; 
};

///////////////////////////////////////////////////////////////////////////
// Fixed size Value Register File.
template <UInt16 N>
class FixedVRF : public VRFBase
{
public:
    FixedVRF() : VRFBase(RF, N) {}
    // ~FixedVRF() We do not need an explicit destructor.
    // Default one will be just fine.

private:
    ValueType   RF[N];
};

///////////////////////////////////////////////////////////////////////////
class ValueRegisterFile : public VRFBase
{
    friend class Value;

public:
    typedef Value ValueType;
//     enum {DefPageSize = 128};
    enum {DefPageSize = 64};

    // Page structure. This structure is SF_ALLOC'ed and NOT new'd because 
    // we need to manage construction/cleanup of T values manually.    
    struct Page
    {
        UInt16      CurrPos;
        UInt16      PageSize; // Total number of values
        UInt16      ReservedNum; // Number of already reserved values.
        ValueType*  pCurrent; // Stored current value.
        Page*       pNext;
        Page*       pPrev;
        ValueType   Values[1]; // Actual values.

        // We can have up to sizeof(UInt16) registers.
        UInt16 PopSize()
        {
            SF_ASSERT(CurrPos > 0 && CurrPos <= PageSize);
            return *(reinterpret_cast<const UInt16*>(Values + PageSize) + --CurrPos);
        }
        void PushSize(UInt16 size)
        {
            SF_ASSERT(CurrPos < PageSize);
            *(reinterpret_cast<UInt16*>(Values + PageSize) + CurrPos++) = size;
        }
    };

public:
    ValueRegisterFile();
    ~ValueRegisterFile();

public:
    void Reserve(UInt16 n);
    void ReleaseReserved(UInt16 n);

    UInt16 GetSize() const { return static_cast<UInt16>(ReservedNum - (pRF - pCurrentPage->Values)); }

private:
    // *** Page allocator.
    Page*   AllocPage(UInt16 pageSize);
    Page*   NewPage(UInt16 pageSize);

private:
    UInt16      MaxReservedPageSize;
    UInt16      MaxAllocatedPageSize;

    // State stack value pointers for fast access.

    Page*       pCurrentPage; // Pointer to the current page.
    Page*       pReserved; // Already allocated pages in a linked list.
};

///////////////////////////////////////////////////////////////////////////////
SF_INLINE
Value::Value(VSBase& v)
: Flags(v.pCurrent->Flags)
, Bonus(v.pCurrent->Bonus)
, value(v.pCurrent->value)
{
    v.Release0();
}

SF_INLINE
void Value::Pick(VSBase& v)
{
    Release();

    Value& other = v.Top0();
    Flags = other.Flags;
    Bonus = other.Bonus;
    value = other.value;

    v.Release0();
}

SF_INLINE
void Value::PickUnsafe(VSBase& v)
{
    Value& other = v.Top0();
    Flags = other.Flags;
    Bonus = other.Bonus;
    value = other.value;

    v.Release0();
}

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_ValueArray_H

