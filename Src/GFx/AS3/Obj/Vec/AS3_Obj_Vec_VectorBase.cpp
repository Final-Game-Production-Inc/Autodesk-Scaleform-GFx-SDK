/**************************************************************************

Filename    :   AS3_Obj_Vec_VectorBase.cpp
Content     :   
Created     :   Apr, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Obj_Vec_VectorBase.h"
#include "AS3_Obj_Vec_Vector_int.h"
#include "AS3_Obj_Vec_Vector_uint.h"
#include "AS3_Obj_Vec_Vector_double.h"
#include "AS3_Obj_Vec_Vector_String.h"
#include "AS3_Obj_Vec_Vector_object.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
int Impl::CompareAsStringInd::Compare(const ASString& a, const ASString& b) const
{
    if (UseLocale)
    {
        if (Desc)
            return b.LocaleCompare_CaseCheck(a, !CaseInsensitive);

        return a.LocaleCompare_CaseCheck(b, !CaseInsensitive);
    }

    int result = 0;
    if (CaseInsensitive)
        result = String::CompareNoCase(a.ToCStr(), b.ToCStr());
    else
        result = SFstrcmp(a.ToCStr(), b.ToCStr());

    if (Desc)
        result = -result;

    return result;
}

///////////////////////////////////////////////////////////////////////////
int Impl::CompareFunct(VM& vm, const Value& Funct, const Value& a, const Value& b)
{
    Value result;
    Value argv[2] = {a, b};

    vm.ExecuteUnsafe(Funct, Value::GetUndefined(), result, 2, argv);

    if (vm.IsException())
        return 0;

    Value::Number n;
    if (!result.Convert2Number(n))
        // exception
        return 0;

    if (NumberUtil::IsNEGATIVE_INFINITY(n))
        return -1;

    if (NumberUtil::IsPOSITIVE_INFINITY(n))
        return 1;

    if (NumberUtil::IsNEGATIVE_ZERO(n))
        return -1;

    if (NumberUtil::IsPOSITIVE_ZERO(n))
        return 1;

    return (n == 0.0 ? 0 : (n < 0.0 ? -1 : 1));
}

///////////////////////////////////////////////////////////////////////////
ArrayBase::ArrayBase(VM& vm)
: Fixed(false)
, VMRef(vm)
{
}

ArrayBase::~ArrayBase()
{
}

///////////////////////////////////////////////////////////////////////////
template<>
void VectorBase<Value>::Value2NumberCollector::operator()(UInt32 ind, const Value& v)
{
    Value::Number num;
    if (!v.Convert2Number(num))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(NumberIndPair(num, ind));
}

template<>
void VectorBase<Ptr<ASStringNode> >::Value2NumberCollector::operator()(UInt32 ind, const Ptr<ASStringNode> & v)
{
    Value::Number num;
    Value value(v);
    if (!value.Convert2Number(num))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(NumberIndPair(num, ind));
}

template<>
void VectorBase<double>::Value2NumberCollector::operator()(UInt32 ind, const double& v)
{
    Value::Number num = v;

    // Store a pair
    Pairs.PushBack(NumberIndPair(num, ind));
}

template<>
void VectorBase<UInt32>::Value2NumberCollector::operator()(UInt32 ind, const UInt32& v)
{
    Value::Number num = v;

    // Store a pair
    Pairs.PushBack(NumberIndPair(num, ind));
}

template<>
void VectorBase<SInt32>::Value2NumberCollector::operator()(UInt32 ind, const SInt32& v)
{
    Value::Number num = v;

    // Store a pair
    Pairs.PushBack(NumberIndPair(num, ind));
}

///////////////////////////////////////////////////////////////////////////
template<>
void VectorBase<Value>::Value2StrCollector::operator()(UInt32 ind, const Value& v)
{
    // Convert to string.
    StringManager& sm = GetVM().GetStringManager();
    ASString str = sm.CreateEmptyString();
    if (!v.Convert2String(str))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(StringIndPair(str, ind));
}

template<>
void VectorBase<Ptr<ASStringNode> >::Value2StrCollector::operator()(UInt32 ind, const Ptr<ASStringNode> & v)
{
    ASString str(v);

    // Store a pair
    Pairs.PushBack(StringIndPair(str, ind));
}

template<>
void VectorBase<double>::Value2StrCollector::operator()(UInt32 ind, const double& v)
{
    // Convert to string.
    StringManager& sm = GetVM().GetStringManager();
    ASString str = sm.CreateEmptyString();
    if (!Value(v).Convert2String(str))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(StringIndPair(str, ind));
}

template<>
void VectorBase<UInt32>::Value2StrCollector::operator()(UInt32 ind, const UInt32& v)
{
    // Convert to string.
    StringManager& sm = GetVM().GetStringManager();
    ASString str = sm.CreateEmptyString();
    if (!Value(v).Convert2String(str))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(StringIndPair(str, ind));
}

template<>
void VectorBase<SInt32>::Value2StrCollector::operator()(UInt32 ind, const SInt32& v)
{
    // Convert to string.
    StringManager& sm = GetVM().GetStringManager();
    ASString str = sm.CreateEmptyString();
    if (!Value(v).Convert2String(str))
        // Exception.
        return;

    // Store a pair
    Pairs.PushBack(StringIndPair(str, ind));
}

///////////////////////////////////////////////////////////////////////////
CheckResult ArrayBase::OfCorrectType(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr) const
{
    for (unsigned i = 0; i < argc; ++i)
    {
        const Value& v = argv[i];

        if (!OfCorrectType(v, tr))
            return false;
    }

    return true;
}

CheckResult ArrayBase::OfCorrectType(const Instances::fl::Array& arr, const ClassTraits::Traits& tr) const
{
    for (unsigned i = 0; i < arr.GetSize(); ++i)
    {
        const Value& v = arr.At(i);

        if (!OfCorrectType(v, tr))
            return false;
    }

    return true;
}

CheckResult ArrayBase::CheckFixed() const
{
    if (GetFixed())
        GetVM().ThrowRangeError(VM::Error(VM::eVectorFixedError, GetVM() SF_DEBUG_ARG("Vector")));

    return !GetFixed();
}

CheckResult ArrayBase::CheckCorrectType(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr) const
{
    bool result = true;

    if (!OfCorrectType(argc, argv, tr))
    {
        VM& vm = GetVM();

        vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
            SF_DEBUG_ARG("arguments")
            SF_DEBUG_ARG(tr.GetName().ToCStr())
            ));

        result = false;
    }

    return result;
}

CheckResult ArrayBase::CheckCorrectType(const Instances::fl::Array& arr, const ClassTraits::Traits& tr) const
{
    bool result = true;

    if (!OfCorrectType(arr, tr))
    {
        VM& vm = GetVM();

        vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
            SF_DEBUG_ARG(arr.GetTraits().GetName().ToCStr())
            SF_DEBUG_ARG(tr.GetName().ToCStr())
            ));

        result = false;
    }

    return result;
}

CheckResult ArrayBase::CheckCallable(const Value& callback) const
{
    bool result = true;

    if (!callback.IsCallable())
    {
        VM& vm = GetVM();

        vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
            SF_DEBUG_ARG(GetVM().GetValueTraits(callback).GetName().ToCStr())
            SF_DEBUG_ARG("callable")
            ));

        result = false;
    }

    return result;
}

CheckResult ArrayBase::CheckCoerce(const ClassTraits::Traits& tr, const Value& v, Value& coerced) const
{
    bool result = true;

    if (!tr.Coerce(v, coerced))
    {
        VM& vm = GetVM();

        vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
            SF_DEBUG_ARG(GetVM().GetValueTraits(v).GetName().ToCStr())
            SF_DEBUG_ARG(tr.GetName().ToCStr())
            ));

        result = false;
    }

    return result;
}

ASString ArrayBase::ToString(const ASString& sep) const
{
    StringBuffer buff(GetVM().GetMemoryHeap());

    const UInt32 size = GetArraySize();
    for(UInt32 i = 0; i < size; ++i)
    {
        if (i > 0)
            buff += sep.ToCStr();

        Value v;
        GetValueUnsafe(i, v);

        if (!v.Convert2String(buff))
            // Exception.
            break;
    }

    return GetVM().GetStringManager().CreateString(buff.ToCStr(), buff.GetSize());
}

AbsoluteIndex ArrayBase::GetNextArrayIndex(AbsoluteIndex ind) const
{
    const UInt32 size = GetArraySize();

    if (!ind.IsValid())
    {
        // Get very first index.
        if (size > 0)
            return AbsoluteIndex(0);
    }
    else
    {
        if (ind < size - 1)
            return ind + 1;
    }

    return AbsoluteIndex(-1);
}

void ArrayBase::Append(unsigned argc, const Value* const argv, const ClassTraits::Traits& tr)
{
    if (!CheckFixed() || !CheckCorrectType(argc, argv, tr))
        return;

    for (UPInt i = 0; i < argc; ++i)
        PushBackValueUnsafe(argv[i]);
}

void ArrayBase::Append(const Instances::fl::Array& arr, const ClassTraits::Traits& tr)
{
    if (!CheckFixed() || !CheckCorrectType(arr, tr))
        return;

    for (UPInt i = 0; i < arr.GetSize(); ++i)
        PushBackValueUnsafe(arr.At(i));
}

void ArrayBase::AppendCoerce(const Instances::fl::Array& arr, const ClassTraits::Traits& tr)
{
    if (!CheckFixed())
        return;

    const UPInt size = arr.GetSize();
    for (UPInt i = 0; i < size; ++i)
    {
        Value r;

        if (!CheckCoerce(tr, arr.At(i), r))
            return;

        PushBackValueUnsafe(r);
    }
}

void ArrayBase::AppendCoerce(const ArrayBase& arr, const ClassTraits::Traits& tr)
{
    if (!CheckFixed())
        return;

    const UInt32 size = arr.GetArraySize();
    for (UInt32 i = 0; i < size; ++i)
    {
        Value r;
        Value v;

        arr.GetValueUnsafe(i, v);
        if (!CheckCoerce(tr, v, r))
            return;

        PushBackValueUnsafe(r);
    }
}

bool ArrayBase::AppendCoerce(const Value& value, const ClassTraits::Traits& tr)
{
    VM& vm = GetVM();
    const Traits& vtr = vm.GetValueTraits(value);
    const BuiltinTraitsType tt = vtr.GetTraitsType();

    if (vtr.IsInstanceTraits())
    {
        const ArrayBase* ab = NULL;

        // TODO: Vector.<Class>
        switch (tt)
        {
        case Traits_Array:
            {
                // Value is an Array.
                const Instances::fl::Array& arr = *static_cast<Instances::fl::Array*>(value.GetObject());

                AppendCoerce(arr, tr);
            }
            return !GetVM().IsException();
        case Traits_Vector_int:
            ab = &static_cast<Instances::fl_vec::Vector_int*>(value.GetObject())->GetArrayBase();
            AppendCoerce(*ab, tr);
            return !vm.IsException();
        case Traits_Vector_uint:
            ab = &static_cast<Instances::fl_vec::Vector_uint*>(value.GetObject())->GetArrayBase();
            AppendCoerce(*ab, tr);
            return !vm.IsException();
        case Traits_Vector_double:
            ab = &static_cast<Instances::fl_vec::Vector_double*>(value.GetObject())->GetArrayBase();
            AppendCoerce(*ab, tr);
            return !vm.IsException();
        case Traits_Vector_String:
            ab = &static_cast<Instances::fl_vec::Vector_String*>(value.GetObject())->GetArrayBase();
            AppendCoerce(*ab, tr);
            return !vm.IsException();
        case Traits_Vector_object:
            ab = &static_cast<Instances::fl_vec::Vector_object*>(value.GetObject())->GetArrayBase();
            AppendCoerce(*ab, tr);
            return !vm.IsException();
        default:
            break;
        }
    }

    return false;
}

bool ArrayBase::Every(const Value& callback, const Value& thisObject, AS3::Object& currObj)
{
    if (callback.IsNullOrUndefined())
        return false;

    if (!CheckCallable(callback))
        return false;

    UInt32 i = 0;
    Value _this(!thisObject.IsNullOrUndefined() ? thisObject : callback);
    const UInt32 size = GetArraySize();

    for(; i < size; ++i)
    {
        Value argv[3] = {Value::GetUndefined(), Value(i), Value(&currObj)};
        Value result;

        GetValueUnsafe(i, argv[0]);
        GetVM().ExecuteUnsafe(callback, _this, result, 3, argv);

        if (GetVM().IsException())
            break;

        if (!result.IsBool() || result.AsBool() == false)
            break;
    }

    // function returns true when called on all items in the Vector; otherwise, false.
    if (i == size)
        return true;

    return false;
}

void ArrayBase::ForEach(const Value& callback, const Value& thisObject, AS3::Object& currObj)
{
    if (callback.IsNullOrUndefined())
        return;

    if (!CheckCallable(callback))
        return;

    Value _this(!thisObject.IsNullOrUndefined() ? thisObject : callback);
    const UInt32 size = GetArraySize();

    for(UInt32 i = 0; i < size; ++i)
    {
        Value argv[3] = {Value::GetUndefined(), Value(i), Value(&currObj)};
        Value result;

        GetValueUnsafe(i, argv[0]);
        GetVM().ExecuteUnsafe(callback, _this, result, 3, argv);

        if (GetVM().IsException())
            break;
    }
}

bool ArrayBase::Some(const Value& callback, const Value& thisObject, AS3::Object& currObj)
{
    if (callback.IsNullOrUndefined())
        return false;

    if (!CheckCallable(callback))
        return false;

    Value _this(!thisObject.IsNullOrUndefined() ? thisObject : callback);
    const UInt32 size = GetArraySize();

    for(UInt32 i = 0; i < size; ++i)
    {
        Value argv[3] = {Value::GetUndefined(), Value(i), Value(&currObj)};
        Value result;

        GetValueUnsafe(i, argv[0]);
        GetVM().ExecuteUnsafe(callback, _this, result, 3, argv);

        if (GetVM().IsException())
            break;

        if (!result.IsBool())
            return false;

        if (result.AsBool() == true)
            return true;
    }

    return false;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3


