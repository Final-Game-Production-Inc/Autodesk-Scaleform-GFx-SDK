/**************************************************************************

Filename    :   AS3_Obj_Vec_VectorBase.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Obj_VectorBase_H
#define INC_AS3_Obj_VectorBase_H

#include "../AS3_Obj_Array.h"
#include "../../AS3_VM.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline
bool IsNullOrUndefined(const T&)
{
    return false;
}

template <>
inline
bool IsNullOrUndefined<SPtr<Object> >(const SPtr<Object>& v)
{
    return v.GetPtr() == NULL;
}

template <>
inline
bool IsNullOrUndefined<Value>(const Value& v)
{
    return v.IsNullOrUndefined();
}

///////////////////////////////////////////////////////////////////////////////
typedef Pair<ASString, UInt32> StringIndPair;
typedef Pair<Value::Number, UInt32> NumberIndPair;

///////////////////////////////////////////////////////////////////////////////
namespace Impl 
{

class CompareAsNumberInd
{
public:
    typedef NumberIndPair DataType;
    typedef DataType::FirstType KeyType;

    CompareAsNumberInd(bool desc) : Desc(desc) {}

public:
    bool operator ()(const DataType& a, const DataType& b) const
    {
        if (Desc)
            return b.First < a.First;

        return a.First < b.First;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return a.First == b.First;
    }

private:
    CompareAsNumberInd& operator = (const CompareAsNumberInd&);

private:
    const bool Desc;
};

class CompareAsStringInd
{
public:
    typedef StringIndPair DataType;
    typedef DataType::FirstType KeyType;

    CompareAsStringInd(bool desc, bool ci, bool use_locale) 
        : Desc(desc)
        , CaseInsensitive(ci) 
        , UseLocale(use_locale)
    {

    }

public:
    bool operator ()(const DataType& a, const DataType& b) const
    {
        return Compare(a.First, b.First) < 0;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return Compare(a.First, b.First) == 0;
    }

private:
    CompareAsStringInd& operator = (const CompareAsStringInd&);

    int Compare(const ASString& a, const ASString& b) const;

private:
    const bool Desc;
    const bool CaseInsensitive;
    const bool UseLocale;
};

int CompareFunct(VM& vm, const Value& Funct, const Value& a, const Value& b);

} // namespace Impl

///////////////////////////////////////////////////////////////////////////////
class ArrayBase
{
public:
    ArrayBase(VM& vm);
    // We do not need a virtual destructor, but compilers complain about a missing one.
    virtual ~ArrayBase();

public:
    ///
    VM& GetVM() const { return VMRef; }

    ///
    StringManager& GetStringManager() const
    {
        return GetVM().GetStringManager();
    }

    ///
    bool GetFixed() const { return Fixed; }
    void SetFixed(bool flag = true) { Fixed = flag; }

    ///
    // Check data type compatibility.
    CheckResult OfCorrectType(const Value& v, const ClassTraits::Traits& tr) const
    {
        return tr.IsParentTypeOf(GetVM().GetClassTraits(v));
    }

    CheckResult OfCorrectType(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr) const;
    CheckResult OfCorrectType(const Instances::fl::Array& arr, const ClassTraits::Traits& tr) const;

public:
    // All CheckXXX() methods return true if we can proceed further or
    // false if we need to stop.

    // Return false and throw an exception if fixed.
    CheckResult CheckFixed() const;
    CheckResult CheckCorrectType(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr) const;
    CheckResult CheckCorrectType(const Instances::fl::Array& arr, const ClassTraits::Traits& tr) const;
    CheckResult CheckCallable(const Value& callback) const;
    CheckResult CheckCoerce(const ClassTraits::Traits& tr, const Value& v, Value& coerced) const;

public:
    virtual UInt32  GetArraySize() const = 0;
    virtual void    GetValue(UInt32 ind, Value& v) const = 0;
    virtual void    GetValueUnsafe(UInt32 ind, Value& v) const = 0;

public:
    ASString ToString(const ASString& sep) const;

    AbsoluteIndex GetNextArrayIndex(AbsoluteIndex ind) const;
    void GetNextValue(Value& value, GlobalSlotIndex ind) const
    {
        if (!ind.IsValid())
            return;

        GetValue(AbsoluteIndex(ind).Get(), value);
    }

    // Append() methods expect all values to be of a correct type.
    void Append(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr);
    void Append(const Instances::fl::Array& arr, const ClassTraits::Traits& tr);

    // Coerce value to a given type and append coerced value.
    void AppendCoerce(const Instances::fl::Array& arr, const ClassTraits::Traits& tr);
    void AppendCoerce(const ArrayBase& arr, const ClassTraits::Traits& tr);
    bool AppendCoerce(const Value& value, const ClassTraits::Traits& tr);

    bool Every(const Value& callback, const Value& thisObject, AS3::Object& currObj);
    void ForEach(const Value& callback, const Value& thisObject, AS3::Object& currObj);
    bool Some(const Value& callback, const Value& thisObject, AS3::Object& currObj);

protected:
    virtual void PushBackValueUnsafe(const Value& v) = 0;

private:
    ArrayBase& operator=(const ArrayBase&);

private:
    bool    Fixed;
    VM&     VMRef;
}; // class ArrayBase

///////////////////////////////////////////////////////////////////////////////
template <typename T>
class VectorBase : public ArrayBase
{
public:
    typedef ArrayBase           ParentType;
    typedef VectorBase<T>       SelfType;
    typedef ArrayDH<const T*>   ValuePtrArray;

public:
    class ArrayFunc
    {
    public:
        virtual ~ArrayFunc() {}

    public:
        virtual void operator()(UInt32 ind, const T& v) = 0;
    };

    // This function cannot be moved into the parent because it depends
    // on ArrayFunc, which is template based.
    void ForEach(ArrayFunc& f) const
    {
        const UPInt size = ValueA.GetSize();
        for (UInt32 i = 0; i < size; ++i)
            f(i, ValueA[i]);
    }
    void ForEach(const Value& callback, const Value& thisObject, AS3::Object& currObj)
    {
        ArrayBase::ForEach(callback, thisObject, currObj);
    }

public:
    class CompareValuePtr
    {
    public:
        typedef Value* DataType;
        typedef DataType KeyType;

        CompareValuePtr(VM& vm, const Value& f) : Vm(vm), Func(f) {}

    public:
        bool operator ()(const T* a, const T* b) const
        {
            return Impl::CompareFunct(GetVM(), Func, Value(*a), Value(*b)) < 0;
        }
        bool Equal(const T* a, const T* b) const
        {
            return Impl::CompareFunct(GetVM(), Func, Value(*a), Value(*b)) == 0;
        }

    public:
        VM& GetVM() const { return Vm; }

    private:
        CompareValuePtr& operator = (const CompareValuePtr&);

    private:
        VM& Vm;
        const Value& Func;
    };

public:
    class Value2NumberCollector : public ArrayFunc
    {
    public:
        Value2NumberCollector(ArrayDH<NumberIndPair>& p)
            : Pairs(p)
        {
        }

    public:
        virtual void operator()(UInt32 ind, const T& v);

    private:
        Value2NumberCollector& operator =(const Value2NumberCollector&);

    private:
        ArrayDH<NumberIndPair>& Pairs;
    };

    class Value2StrCollector : public ArrayFunc
    {
    public:
        Value2StrCollector(VM& vm, ArrayDH<StringIndPair>& p)
            : Vm(vm)
            , Pairs(p)
        {
        }

    public:
        virtual void operator()(UInt32 /*ind*/, const T& v);

    private:
        Value2StrCollector& operator =(const Value2StrCollector&);

        VM& GetVM()
        {
            return Vm;
        }

    private:
        VM& Vm;
        ArrayDH<StringIndPair>& Pairs;
    };

    class ValuePtrCollector : public ArrayFunc
    {
    public:
        ValuePtrCollector(ValuePtrArray& p) : Ptrs(p) {}

    public:
        virtual void operator()(UInt32 /*ind*/, const T& v)
        {
            // Store a value
            Ptrs.PushBack(&v);
        }

    private:
        ValuePtrCollector& operator =(const ValuePtrCollector&);

    private:
        ValuePtrArray& Ptrs;
    };

public:
    VectorBase(MemoryHeap* heap, VM& vm)
        : ArrayBase(vm)
        , ValueA(heap)
    {
    }

public:
    ///
    UInt32 GetSize() const { return static_cast<UInt32>(ValueA.GetSize()); }
    virtual UInt32 GetArraySize() const { return GetSize(); }

    ///
    virtual void  GetValue(UInt32 ind, Value& v) const { v.Assign(ValueA[ind]); }
    virtual void  GetValueUnsafe(UInt32 ind, Value& v) const { v.AssignUnsafe(ValueA[ind]); }

    ///
    const T& Get(UInt32 ind) const { return ValueA[ind]; }

    ///
    T GetDefaultValue() const
    {
        return T();
    }

    ///
    void Append(const SelfType& other)
    {
        if (!CheckFixed())
            return;

        for (UPInt i = 0; i < other.GetSize(); ++i)
            ValueA.PushBack(other.ValueA[i]);
    }
    void Append(const ArrayDH<T>& arr)
    {
        if (!CheckFixed())
            return;

        for (UPInt i = 0; i < arr.GetSize(); ++i)
            ValueA.PushBack(arr.At(i));
    }
    void Append(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr)
    {
        ParentType::Append(argc, argv, tr);
    }
    void Append(const Instances::fl::Array& arr, const ClassTraits::Traits& tr)
    {
        ParentType::Append(arr, tr);
    }

    ///
    void AppendCoerce(const Instances::fl::Array& arr, const ClassTraits::Traits& tr)
    {
        ParentType::AppendCoerce(arr, tr);
    }
    void AppendCoerce(const ArrayBase& arr, const ClassTraits::Traits& tr)
    {
        ParentType::AppendCoerce(arr, tr);
    }
    bool AppendCoerce(const Value& v, const ClassTraits::Traits& tr)
    {
        return ParentType::AppendCoerce(v, tr);
    }

    ///
    CheckResult Resize(UPInt newSise)
    {
        bool result = false;

        if (CheckFixed())
        {
            UPInt i = ValueA.GetSize();

            ValueA.Resize(newSise);

            for (; i < newSise; ++i)
                ValueA[i] = GetDefaultValue();

            result = true;
        }

        return result;
    }

    ///
    CheckResult Set(UInt32 ind, const Value& v, const ClassTraits::Traits& tr)
    {
        if (GetFixed() && ind >= GetSize())
        {
            GetVM().ThrowRangeError(VM::Error(VM::eOutOfRangeError, GetVM() SF_DEBUG_ARG(ind) SF_DEBUG_ARG(static_cast<int>(GetSize() - 1))));
            return false;
        }

        if (ind > GetSize())
        {
            GetVM().ThrowRangeError(VM::Error(VM::eOutOfRangeError, GetVM() SF_DEBUG_ARG(ind) SF_DEBUG_ARG(static_cast<int>(GetSize() - 1))));
            return false;
        }

        if (ind == GetSize())
        {
            if (!Resize(ind + 1))
                return false;
        }

        Value r;
        if (!CheckCoerce(tr, v, r))
            return false;

        SetUnsafe(ind, r);
        return true;
    }

    ///
    void SetUnsafeT(UInt32 ind, const T& v)
    {
        ValueA[ind] = v;
    }

    //
    void Get(UInt32 ind, Value& v) const 
    {
        if (ind < GetSize())
            v.Assign(ValueA[ind]); 
    }

    //
    CheckResult RemoveAt(UInt32 ind) 
    {
        if (ind >= GetSize())
            return false;

        ValueA.RemoveAt(ind);

        return true;
    }

    //
    template <class I>
    void Map(SPtr<I>& result, const Value& callback, const Value& thisObject, I* currObj)
    {
        typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj->GetTraits());
        Pickable<I> r = itr.MakeInstance(itr); 

        result.Pick(r);

        if (callback.IsNullOrUndefined())
            return;

        if (!CheckCallable(callback))
            return;

        Value _this(!thisObject.IsNullOrUndefined() ? thisObject : callback);
        const ClassTraits::Traits& tr = currObj->GetEnclosedClassTraits();

        for(UPInt i = 0; i < GetSize(); ++i)
        {
            const Value argv[3] = {Value(ValueA[i]), Value(i), Value(currObj)};

            Value resultValue;
            GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

            if (GetVM().IsException())
                break;

            Value coerced_result;
            if (!CheckCoerce(tr, resultValue, coerced_result))
                return;

            r->PushBack(coerced_result);
        }
    }

    template <class I>
    void Concat(Value& result, unsigned argc, const Value* const argv, I& currObj)
    {
        VM& vm = GetVM();
        const ClassTraits::Traits& currClassTR = const_cast<const I&>(currObj).GetClass().GetClassTraits();
        typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj.GetTraits());
        Pickable<I> r = itr.MakeInstance(itr);

        result.Pick(r);
        r->Append(currObj);

        for (unsigned i = 0; i < argc; ++i)
        {
            const Value& value = argv[i];
            const AS3::Traits& argTraits = vm.GetValueTraits(value);
            const ClassTraits::Traits& argClassTR = vm.GetClassTraits(value);

            if (argTraits.IsArrayLike())
            {
                // It is an Array/Vector.
                if (argClassTR.IsOfType(vm.GetClassTraitsArray()))
                {
                    // This is an Array. Values can be of any type.
                    NOT_IMPLEMENTED("Vector::concat() for argument of type Array");
                    return;
                }
                else if(&currClassTR == &argClassTR)
                    // We are concatenating objects of same class.
                    r->Append(static_cast<I&>(*value.GetObject()));
                else
                {
                    GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                        SF_DEBUG_ARG(currClassTR.GetName().ToCStr())
                        SF_DEBUG_ARG(argClassTR.GetName().ToCStr())
                        ));
                    return;
                }
            }
            else if (OfCorrectType(value, currClassTR))
                r->PushBack(value);
            else
            {
                GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                    SF_DEBUG_ARG(currClassTR.GetName().ToCStr())
                    SF_DEBUG_ARG(argClassTR.GetName().ToCStr())
                    ));
                return;
            }
        }
    }

    template <class I>
    void Filter(SPtr<I>& result, const Value& callback, const Value& thisObject, I& currObj)
    {
        typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj.GetTraits());
        Pickable<I> r = itr.MakeInstance(itr);

        result.Pick(r);
        if (callback.IsNullOrUndefined())
            return;

        if (!CheckCallable(callback))
            return;

        Value _this(!thisObject.IsNullOrUndefined() ? thisObject : callback);

        for(UPInt i = 0; i < GetSize(); ++i)
        {
            const Value argv[3] = {Value(ValueA[i]), Value(i), Value(&currObj)};
            Value resultValue;

            GetVM().ExecuteUnsafe(callback, _this, resultValue, 3, argv);

            if (GetVM().IsException())
                break;

            if (!resultValue.IsBool() || resultValue.AsBool() == false)
                continue;

            // It looks like this case needs no special treatment.
            r->PushBack(ValueA[i]);
        }
    }

    template <typename R>
    void Pop(R& result)
    {
        if (!CheckFixed())
            return;

        if (GetSize() > 0)
            result = ValueA.Pop();
    }

    void PushBack(const T& v)
    {
        if (!CheckFixed())
            return;

        ValueA.PushBack(v);
    }

    void PushBack(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr)
    {
        if (!CheckFixed())
            return;

        for (unsigned i = 0; i < argc; ++i)
        {
            Value r;

            if (!CheckCoerce(tr, argv[i], r))
                return;

            PushBackUnsafe(r);
        }
    }

    void Unshift(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr)
    {
        if (!CheckFixed() || !CheckCorrectType(argc, argv, tr))
            return;

        // Insert empty values.
        //ValueA.InsertMultipleAt(0, argc, DefaultValue<T>(GetStringManager()));
        ValueA.InsertMultipleAt(0, argc);

        // Assign real values.
        for (unsigned i = 0; i < argc; ++i)
            SetUnsafe(i, argv[i]);
    }

    void Reverse() { Alg::ReverseArray(ValueA); }

    template <typename R>
    void Shift(R& result)
    { 
        if (!CheckFixed())
            return;

        if (ValueA.IsEmpty())
            return;

        result = ValueA[0];
        ValueA.RemoveAt(0);
    }

    template <class I>
    void Sort(SPtr<I>& result, const Value& arg, I& currObj)
    {
        SF_UNUSED2(result, currObj);

        Value compareFunction;
        SInt32 flags = 0;

        if(arg.IsCallable()) 
            compareFunction = arg;
        else if (!arg.Convert2Int32(flags))
        {
            VM& vm = GetVM();
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm 
                SF_DEBUG_ARG(vm.GetValueTraits(arg).GetName().ToCStr())
                SF_DEBUG_ARG(currObj.GetEnclosedClassTraits().GetName().ToCStr())
                ));
        }

        ArrayDH<T> newSA(GetVM().GetMemoryHeap());

        // There are two major branches: with compareFunction and without compareFunction.
        if (compareFunction.IsNullOrUndefined())
        {
            // We can compare as strings or as numbers.
            if ((flags & Instances::fl::Array::SortFlags_Numeric) != 0)
            {
                // Compare as numbers.

                // Convert values to numbers and collect them into an array.
                ArrayDH<NumberIndPair> pairs(GetVM().GetMemoryHeap());
                Value2NumberCollector vc(pairs);
                ForEach(vc);

                // Choose a correct sorting function.
                Impl::CompareAsNumberInd functor((flags & Instances::fl::Array::SortFlags_Descending) != 0);

                // Sort.
                Alg::QuickSortSafe(pairs, functor);

                // Check for UNIQUESORT
                if ((flags & Instances::fl::Array::SortFlags_UniqueSort) != 0)
                {
                    for (UInt32 i = 1; i < pairs.GetSize(); ++i)
                    {
                        if (functor.Equal(pairs[i - 1], pairs[i]))
                        {
                            // two or more elements being sorted have identical sort fields.
                            result = NULL;
                            return;
                        }
                    }
                }

                // Initialize a new SparseArray.
                for (UPInt i = 0; i < pairs.GetSize(); ++i)
                    newSA.PushBack(ValueA[pairs[i].Second]);
            }
            else
            {
                // Compare as strings.

                // Convert values to strings and collect them into an array.
                ArrayDH<StringIndPair> pairs(GetVM().GetMemoryHeap());
                Value2StrCollector vc(GetVM(), pairs);
                ForEach(vc);

                // Choose a correct sorting function.
                Impl::CompareAsStringInd functor (
                    (flags & Instances::fl::Array::SortFlags_Descending) != 0,
                    (flags & Instances::fl::Array::SortFlags_CaseInsensitive) != 0,
                    (flags & Instances::fl::Array::SortFlags_Locale) != 0
                    );


                // Sort.
                Alg::QuickSortSafe(pairs, functor);

                // Check for UNIQUESORT
                if ((flags & Instances::fl::Array::SortFlags_UniqueSort) != 0)
                {
                    for (UInt32 i = 1; i < pairs.GetSize(); ++i)
                    {
                        if (functor.Equal(pairs[i - 1], pairs[i]))
                        {
                            // two or more elements being sorted have identical sort fields.
                            // Documentation says we should return ZERO.
                            //result.SetSInt32(0);
                            result = NULL;
                            return;
                        }
                    }
                }

                // Initialize a new SparseArray.
                for (UPInt i = 0; i < pairs.GetSize(); ++i)
                    newSA.PushBack(ValueA[pairs[i].Second]);
            }

        } else
        {
            // Compare using compareFunction.

            // Collect values into an array.
            ValuePtrArray ptrs(GetVM().GetMemoryHeap());
            ValuePtrCollector vc(ptrs);
            ForEach(vc);

            // Sort.
            CompareValuePtr functor(GetVM(), compareFunction);
            Alg::QuickSortSafe(ptrs, functor);

            // Check for UNIQUESORT (do we really need to do this?)
            if ((flags & Instances::fl::Array::SortFlags_UniqueSort) != 0)
            {
                for (UInt32 i = 1; i < ptrs.GetSize(); ++i)
                {
                    if (functor.Equal(ptrs[i - 1], ptrs[i]))
                    {
                        // two or more elements being sorted have identical sort fields.
                        // Documentation says we should return ZERO.
                        //result.SetSInt32(0);
                        result = NULL;
                        return;
                    }
                }
            }

            // Initialize a new SparseArray.
            for (UPInt i = 0; i < ptrs.GetSize(); ++i)
                newSA.PushBack(typename ArrayDH<T>::ValueType(*ptrs[i]));
        }

        // Make a result (replace current or return a new array).
        if (flags & Instances::fl::Array::SortFlags_ReturnIndexedArray)
        {
            typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj.GetTraits());
            result = itr.MakeInstance(itr);
            result->Append(newSA);
        } else
        {
            // Replace current SparseArray.
            ValueA = newSA;
            result = &currObj;
        }
    }

    template <class I>
    void Slice(Value& result, unsigned argc, const Value* const argv, I& currObj)
    {
        typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj.GetTraits());
        Pickable<I> r = itr.MakeInstance(itr);

        result.Pick(r);
        SInt32 startIndex = 0;
        SInt32 endIndex = 16777215; // 0xFFFFFF;

        if (argc > 0)
            if (!argv[0].Convert2Int32(startIndex))
                // Exception.
                return;

        if (argc > 1)
            if (!argv[1].Convert2Int32(endIndex))
                // Exception.
                return;

        if (startIndex < 0)
            startIndex += static_cast<SInt32>(GetSize());

        startIndex = Alg::Max<SInt32>(0, startIndex);

        if (endIndex < 0)
            endIndex += static_cast<SInt32>(GetSize());

        endIndex = Alg::Min<SInt32>(static_cast<SInt32>(GetSize()), endIndex);

        for (SInt32 i = startIndex; i < endIndex; ++i)
            // No special treatment is necessary
            r->PushBack(ValueA[i]);
    }

    template <class I>
    void Splice(Value& result, unsigned argc, const Value* const argv, I& currObj)
    {
        if (!CheckFixed())
            return;

        SInt32 startIndex = 0;
        if (argc > 0)
            if (!argv[0].Convert2Int32(startIndex))
                // Exception.
                return;

        const UInt32 size = GetSize();
        UInt32 deleteCount = 0;

        if (startIndex < 0)
            startIndex += static_cast<SInt32>(size);

        if (argc > 1)
        {
            if (!argv[1].Convert2UInt32(deleteCount))
                // Exception.
                return;

            const SInt32 n = startIndex + deleteCount;
            if (n < 0 || static_cast<UInt32>(n) > size)
            {
                return GetVM().ThrowRangeError(VM::Error(VM::eOutOfRangeError, GetVM() 
                    SF_DEBUG_ARG(Value(n))
                    SF_DEBUG_ARG(Value(size))
                    ));
            }
        }
        else
        {
            // If you do not specify a value for the deleteCount parameter, 
            // the method deletes all of the values from the startIndex  
            // element to the last element in the array.
            deleteCount = size - startIndex;
        }

        typename I::TraitsType& itr = static_cast<typename I::TraitsType&>(currObj.GetTraits());
        Pickable<I> r = itr.MakeInstance(itr);
        result.Pick(r);

        if (startIndex > static_cast<SInt32>(size) || startIndex < 0)
            return;

        // Delete ...
        DeleteMultipleAt(startIndex, deleteCount, *r);

        // Insert the rest ...
        if (argc > 2)
        {
            startIndex = Alg::Min<SInt32>(static_cast<SInt32>(GetSize()), startIndex);
            Insert(startIndex, argc - 2, argv + 2);
        }
    }

    void IndexOf(SInt32& result, const T& searchElement, SInt32 fromIndex)
    {
        if (fromIndex < 0)
            fromIndex = static_cast<SInt32>(GetSize() + fromIndex);

        for(UPInt i = fromIndex; i < GetSize(); ++i)
        {
            //if (StrictEqual(ValueA.At(i), searchElement))
            if (ValueA.At(i) == searchElement)
            {
                result = static_cast<SInt32>(i);
                return;
            }
        }

        result = -1;
    }

    void LastIndexOf(SInt32& result, const T& searchElement, SInt32 fromIndex)
    {
        if (fromIndex < 0)
            fromIndex = static_cast<SInt32>(GetSize() + fromIndex);

        for(SPInt i = Alg::Min<SPInt>(fromIndex, static_cast<SPInt>(GetSize()) - 1); i >= 0; --i)
        {
            //if (StrictEqual(At(i), searchElement))
            if (ValueA.At(i) == searchElement)
            {
                result = static_cast<SInt32>(i);
                return;
            }
        }

        result = -1;
    }

public:
    void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                         SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
    {
        const UPInt size = ValueA.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, ValueA[i], op SF_DEBUG_ARG(owner));
    }

protected:
    template <class I>
    void DeleteMultipleAt(UPInt pos, UPInt count, I& deleted)
    {
        for (UPInt i = 0; i < count && ((i + pos) < ValueA.GetSize()); ++i)
            deleted.PushBack(ValueA[i + pos]);

        ValueA.RemoveMultipleAt(pos, count);
    }

    void Insert(UPInt pos, unsigned argc, const Value* argv)
    {
        // Check data type compatibility.
        // Do not check it for the time being.
//         if (!OfCorrectType(argc, argv, currObj))
//             return GetVM().ThrowTypeError(VM::eCheckTypeFailedError);

        for (UPInt i = 0 ; i < argc; ++i)
            ValueA.InsertAt(pos + i, typename ArrayDH<T>::ValueType(argv[i]));
    }

    void SetUnsafe(UInt32 ind, const Value& v)
    {
        ValueA[ind] = static_cast<T>(v);
    }

    void PushBackUnsafe(const Value& v)
    {
        ValueA.PushBack(static_cast<T>(v));
    }
    virtual void PushBackValueUnsafe(const Value& v)
    {
        PushBackUnsafe(v);
    }

private:
    SelfType& operator =(const SelfType&);

private:
    ArrayDH<T>  ValueA;
};

///////////////////////////////////////////////////////////////////////////////
// Explicit specializations.

template <>
inline
void VectorBase<Value>::IndexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex)
{
    for(UPInt i = fromIndex; i < GetSize(); ++i)
    {
        if (StrictEqual(ValueA.At(i), searchElement))
        {
            result = static_cast<SInt32>(i);
            return;
        }
    }

    result = -1;
}

template <>
inline
void VectorBase<Value>::LastIndexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex)
{
    for(SPInt i = Alg::Min<SPInt>(fromIndex, static_cast<SPInt>(GetSize()) - 1); i >= 0; --i)
    {
        if (StrictEqual(ValueA.At(i), searchElement))
        {
            result = static_cast<SInt32>(i);
            return;
        }
    }

    result = -1;
}

template <>
inline
void VectorBase<Ptr<ASStringNode> >::SetUnsafe(UInt32 ind, const Value& v)
{
    ValueA[ind] = v.GetStringNode();
}

template <>
inline
void VectorBase<Ptr<ASStringNode> >::PushBackUnsafe(const Value& v)
{
    ValueA.PushBack(v.GetStringNode());
}

template <>
inline
Ptr<ASStringNode> VectorBase<Ptr<ASStringNode> >::GetDefaultValue() const
{
    return GetVM().GetStringManager().GetNullStringNode();
}

template <>
inline
Value VectorBase<Value>::GetDefaultValue() const
{
    return Value::GetNull();
}

///////////////////////////////////////////////////////////////////////////
template<>
inline
bool VectorBase<Value>::CompareValuePtr::operator()(const Value* a, const Value* b) const
{
    return Impl::CompareFunct(GetVM(), Func, *a, *b) < 0;
}

template<>
inline
bool VectorBase<Value>::CompareValuePtr::Equal(const Value* a, const Value* b) const
{
    return Impl::CompareFunct(GetVM(), Func, *a, *b) == 0;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif
