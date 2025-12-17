/**************************************************************************

Filename    :   AS3_CodeExecute.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_CodeExecute_H
#define INC_AS3_CodeExecute_H

#include "AS3_Value.h"
#include "AS3_ValueStack.h"

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// ToType is doing conversion in place.

template <typename T> 
SF_INLINE
T& ToType(Value& v, bool& /*success*/)
{
    return v;
}

template <> 
SF_INLINE 
Value& ToType<Value>(Value& v, bool& /*success*/) { return v; }

template <> 
SF_INLINE 
bool& ToType<bool>(Value& v, bool& success) 
{
    if (success)
    {
        v.ToBooleanValue();
        return v;
    }

    static bool tmp;
    return tmp;
}

template <> 
SF_INLINE 
SInt32& ToType<SInt32>(Value& v, bool& success) 
{
    static SInt32 tmp;
    success = success && v.ToInt32Value();
    return success ? v.AsInt() : tmp;
}

template <> 
SF_INLINE 
UInt32& ToType<UInt32>(Value& v, bool& success) 
{
    static UInt32 tmp;
    success = success && v.ToUInt32Value();
    return success ? v.AsUInt() : tmp;
}

template <> 
SF_INLINE 
Value::Number& ToType<Value::Number>(Value& v, bool& success) 
{
    static Value::Number tmp;
    // Replacing with ToNumberValueInline() seems to make performance worse
    success = success && v.ToNumberValue();
    return success ? v.AsNumber() : tmp;
}

///////////////////////////////////////////////////////////////////////////
// Stack helper, which reads ONE argument form stack ...

///
// This stack helper will convert value on stack to the T1 type.
// Conversion happens "in place".
template <int N, typename T1 = Value>
struct SH1
{
    typedef SH1<N, T1> SelfType;

    SF_INLINE
    SH1(VSBase& _stack)
    : Success(true)
    , _1(ToType<T1>(_stack.Top0(), Success))
    , stack(_stack)
    {
    }
    SF_INLINE
        ~SH1()
    {
        stack.PopBack(N);
    }

private:
    bool Success;

public:
    T1& _1;
    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);

private:
    VSBase& stack;
};

// Value will not be deleted from stack.
template <typename T1>
struct SH1<0, T1>
{
    typedef SH1<0, T1> SelfType;

    SF_INLINE
    SH1(VSBase& _stack)
    : Success(true)
    , _1(ToType<T1>(_stack.Top0(), Success))
    {
    }

private:
    bool Success;

public:
    T1& _1;
    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

// Value will not be deleted from stack.
template <>
struct SH1<0, Value>
{
    typedef SH1<0, Value> SelfType;

    SF_INLINE
    SH1(VSBase& _stack)
    : _1(_stack.Top0())
    {
    }

public:
    Value& _1;
    bool IsOK() const { return true; }
    bool IsException() const { return false; }

private:
    SelfType& operator =(const SelfType&);
};

// Value will be deleted from stack.
template <typename T1>
struct SH1<1, T1>
{
    typedef SH1<1, T1> SelfType;

    SF_INLINE
    SH1(VSBase& _stack)
    : Success(true)
    , _1(ToType<T1>(_stack.Top0(), Success))
    {
        _stack.Release0();
    }

private:
    bool Success;

public:
    T1 _1;
    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

template <>
struct SH1<1, Value>
{
    typedef SH1<1, Value> SelfType;

    SF_INLINE
    SH1(VSBase& _stack)
    : _1(_stack)
    {
    }

public:
    Value _1;
    bool IsOK() const { return true; }
    bool IsException() const { return false; }

private:
    SelfType& operator =(const SelfType&);
};

///
// This stack helper expects value of type T1 on stack.
// No conversion is necessary.
template <int N, typename T1 = Value>
struct TSH1
{
    typedef TSH1<N, T1> SelfType;

    SF_INLINE
    TSH1(VSBase& _stack)
    : _1(_stack.Top0())
    , stack(_stack)
    {
    }
    SF_INLINE
        ~TSH1()
    {
        stack.PopBack(N);
    }

public:
    T1& _1;

private:
    SelfType& operator =(const SelfType&);

private:
    VSBase& stack;
};

// This specialization should be used only with primitive types.
template <typename T1>
struct TSH1<1, T1>
{
    typedef TSH1<1, T1> SelfType;

    SF_INLINE
    TSH1(VSBase& _stack)
    : _1(_stack.Top0())
    {
        // We can just forget a value on stack because it is primitive.
        _stack.Release0();
    }

    T1 _1;

private:
    SelfType& operator =(const SelfType&);
};

template <>
struct TSH1<1, Value>
{
    typedef TSH1<1, Value> SelfType;

    SF_INLINE
    TSH1(VSBase& _stack)
    : _1(_stack)
    {
    }

    Value _1;

private:
    SelfType& operator =(const SelfType&);
};


// Stack helper, which reads TWO arguments form stack.
// This stack helper will convert values on stack to T1 and T2 types accordingly.
// Conversion happens "in place".
template <int N, typename T1 = Value, typename T2 = T1>
struct SH2
{
    typedef SH2<N, T1, T2> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : Success(true)
    , _2(ToType<T2>(_stack.Top0(), Success))
    , _1(ToType<T1>(_stack.Top1(), Success))
    , stack(_stack)
    {
    }
    SF_INLINE
        ~SH2()
    {
        stack.PopBack(N);
    }

private:
    bool Success;

public:
    T2& _2;
    T1& _1;
    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);

private:
    VSBase& stack;
};

// None of the values will be deleted from stack.
template <typename T1, typename T2>
struct SH2<0, T1, T2>
{
    typedef SH2<1, T1, T2> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : Success(true)
    , _2(ToType<T2>(_stack.Top0(), Success))
    , _1(ToType<T1>(_stack.Top1(), Success))
    {
    }

private:
    bool Success;

public:
    T2& _2;
    T1& _1;

    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

// None of the values will be deleted from stack.
template <typename T1>
struct SH2<0, T1, Value>
{
    typedef SH2<0, T1, Value> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : Success(true)
    , _2(_stack.Top0())
    , _1(ToType<T1>(_stack.Top1(), Success))
    {
    }

private:
    bool Success;

public:
    Value&  _2;
    T1&     _1;

    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

// None of the values will be deleted from stack.
template <>
struct SH2<0, Value, Value>
{
    typedef SH2<0, Value, Value> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : _2(_stack.Top0())
    , _1(_stack.Top1())
    {
    }

public:
    Value&  _2;
    Value&  _1;

    bool IsOK() const { return true; }
    bool IsException() const { return false; }

private:
    SelfType& operator =(const SelfType&);
};

// Only one value will be deleted from stack.
template <typename T1, typename T2>
struct SH2<1, T1, T2>
{
    typedef SH2<1, T1, T2> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : Success(true)
    , _2(ToType<T2>(_stack.Top0(), Success))
    , _1(ToType<T1>(_stack.Top1(), Success))
    {
        _stack.PopBack();
    }

private:
    bool Success;

public:
    T2  _2;
    T1& _1;

    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

// Only one value will be deleted from stack.
template <typename T1>
struct SH2<1, T1, Value>
{
    typedef SH2<1, T1, Value> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : Success(true)
    , _2(_stack)
    , _1(ToType<T1>(_stack.Top0(), Success))
    {
    }

private:
    bool Success;

public:
    Value   _2;
    T1&     _1;

    bool IsOK() const { return Success; }
    bool IsException() const { return !Success; }

private:
    SelfType& operator =(const SelfType&);
};

// Only one value will be deleted from stack.
template <>
struct SH2<1, Value, Value>
{
    typedef SH2<1, Value, Value> SelfType;

    SF_INLINE
    SH2(VSBase& _stack)
    : _2(_stack)
    , _1(_stack.Top0())
    {
    }

public:
    Value   _2;
    Value&  _1;

    bool IsOK() const { return true; }
    bool IsException() const { return false; }

private:
    SelfType& operator =(const SelfType&);
};

///
// This stack helper expects values of types T1 and T2 on stack.
// No conversion is necessary.
template <int N, typename T1 = Value, typename T2 = T1>
struct TSH2
{
    typedef TSH2<N, T1, T2> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack.Top0())
    , _1(_stack.Top1())
    , stack(_stack)
    {
    }
    SF_INLINE
        ~TSH2()
    {
        stack.PopBack(N);
    }

    T2& _2;
    T1& _1;

private:
    SelfType& operator =(const SelfType&);

private:
    VSBase& stack;
};

template <typename T1, typename T2>
struct TSH2<1, T1, T2>
{
    typedef TSH2<1, T1, T2> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack.Top0())
    , _1(_stack.Top1())
    {
        _stack.Release0();
    }

    T2  _2;
    T1& _1;

private:
    SelfType& operator =(const SelfType&);
};

template <typename T1>
struct TSH2<1, T1, Value>
{
    typedef TSH2<1, T1, Value> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack)
    , _1(_stack.Top1())
    {
        _stack.Release0();
    }

    Value   _2;
    T1&     _1;

private:
    SelfType& operator =(const SelfType&);
};

// Primitive types only.
template <typename T1, typename T2>
struct TSH2<2, T1, T2>
{
    typedef TSH2<2, T1, T2> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack.PopPrimitive<T2>())
    , _1(_stack.PopPrimitive<T1>())
    {
    }

    T2  _2;
    T1  _1;

private:
    SelfType& operator =(const SelfType&);
};

// T1 should be primitive.
template <typename T1>
struct TSH2<2, T1, Value>
{
    typedef TSH2<2, T1, Value> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack)
    , _1(_stack.PopPrimitive<T1>())
    {
    }

    Value   _2;
    T1      _1;

private:
    SelfType& operator =(const SelfType&);
};

// T1 should be primitive.
template <typename T2>
struct TSH2<2, Value, T2>
{
    typedef TSH2<2, Value, T2> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack.PopPrimitive<T2>())
    , _1(_stack)
    {
    }

    T2      _2;
    Value   _1;

private:
    SelfType& operator =(const SelfType&);
};

template <>
struct TSH2<2, Value, Value>
{
    typedef TSH2<2, Value, Value> SelfType;

    SF_INLINE
    TSH2(VSBase& _stack)
    : _2(_stack)
    , _1(_stack)
    {
    }

    Value   _2;
    Value   _1;

private:
    SelfType& operator =(const SelfType&);
};

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_CodeExecute_H
