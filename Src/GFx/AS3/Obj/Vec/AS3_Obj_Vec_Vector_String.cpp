//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Vec_Vector_String.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Vec_Vector_String.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    ASString GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3join, 0, const ASString&>(AS3::StringManager& sm)
    {
        return sm.CreateConstString(",");
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3every, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3forEach, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3map, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3some, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3filter, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    SInt32 GetMethodDefArg<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3lastIndexOf, 1, SInt32>(AS3::StringManager&)
    {
        return 0x7fffffff;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_lengthGet, UInt32> TFunc_Instances_Vector_String_lengthGet;
typedef ThunkFunc1<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_lengthSet, const Value, UInt32> TFunc_Instances_Vector_String_lengthSet;
typedef ThunkFunc1<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_fixedSet, const Value, bool> TFunc_Instances_Vector_String_fixedSet;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_fixedGet, bool> TFunc_Instances_Vector_String_fixedGet;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3toString, ASString> TFunc_Instances_Vector_String_AS3toString;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3toLocaleString, ASString> TFunc_Instances_Vector_String_AS3toLocaleString;
typedef ThunkFunc1<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3join, ASString, const ASString&> TFunc_Instances_Vector_String_AS3join;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3every, bool, const Value&, const Value&> TFunc_Instances_Vector_String_AS3every;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3forEach, const Value, const Value&, const Value&> TFunc_Instances_Vector_String_AS3forEach;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3map, SPtr<Instances::fl_vec::Vector_String>, const Value&, const Value&> TFunc_Instances_Vector_String_AS3map;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3push, Value, unsigned, const Value*> TFunc_Instances_Vector_String_AS3push;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3some, bool, const Value&, const Value&> TFunc_Instances_Vector_String_AS3some;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3unshift, Value, unsigned, const Value*> TFunc_Instances_Vector_String_AS3unshift;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3concat, Value, unsigned, const Value*> TFunc_Instances_Vector_String_AS3concat;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3filter, SPtr<Instances::fl_vec::Vector_String>, const Value&, const Value&> TFunc_Instances_Vector_String_AS3filter;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3pop, ASString> TFunc_Instances_Vector_String_AS3pop;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3reverse, SPtr<Instances::fl_vec::Vector_String> > TFunc_Instances_Vector_String_AS3reverse;
typedef ThunkFunc0<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3shift, ASString> TFunc_Instances_Vector_String_AS3shift;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3slice, Value, unsigned, const Value*> TFunc_Instances_Vector_String_AS3slice;
typedef ThunkFunc1<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3sort, SPtr<Instances::fl_vec::Vector_String>, const Value&> TFunc_Instances_Vector_String_AS3sort;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3splice, Value, unsigned, const Value*> TFunc_Instances_Vector_String_AS3splice;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3indexOf, SInt32, const ASString&, SInt32> TFunc_Instances_Vector_String_AS3indexOf;
typedef ThunkFunc2<Instances::fl_vec::Vector_String, Instances::fl_vec::Vector_String::mid_AS3lastIndexOf, SInt32, const ASString&, SInt32> TFunc_Instances_Vector_String_AS3lastIndexOf;

template <> const TFunc_Instances_Vector_String_lengthGet::TMethod TFunc_Instances_Vector_String_lengthGet::Method = &Instances::fl_vec::Vector_String::lengthGet;
template <> const TFunc_Instances_Vector_String_lengthSet::TMethod TFunc_Instances_Vector_String_lengthSet::Method = &Instances::fl_vec::Vector_String::lengthSet;
template <> const TFunc_Instances_Vector_String_fixedSet::TMethod TFunc_Instances_Vector_String_fixedSet::Method = &Instances::fl_vec::Vector_String::fixedSet;
template <> const TFunc_Instances_Vector_String_fixedGet::TMethod TFunc_Instances_Vector_String_fixedGet::Method = &Instances::fl_vec::Vector_String::fixedGet;
template <> const TFunc_Instances_Vector_String_AS3toString::TMethod TFunc_Instances_Vector_String_AS3toString::Method = &Instances::fl_vec::Vector_String::AS3toString;
template <> const TFunc_Instances_Vector_String_AS3toLocaleString::TMethod TFunc_Instances_Vector_String_AS3toLocaleString::Method = &Instances::fl_vec::Vector_String::AS3toLocaleString;
template <> const TFunc_Instances_Vector_String_AS3join::TMethod TFunc_Instances_Vector_String_AS3join::Method = &Instances::fl_vec::Vector_String::AS3join;
template <> const TFunc_Instances_Vector_String_AS3every::TMethod TFunc_Instances_Vector_String_AS3every::Method = &Instances::fl_vec::Vector_String::AS3every;
template <> const TFunc_Instances_Vector_String_AS3forEach::TMethod TFunc_Instances_Vector_String_AS3forEach::Method = &Instances::fl_vec::Vector_String::AS3forEach;
template <> const TFunc_Instances_Vector_String_AS3map::TMethod TFunc_Instances_Vector_String_AS3map::Method = &Instances::fl_vec::Vector_String::AS3map;
template <> const TFunc_Instances_Vector_String_AS3push::TMethod TFunc_Instances_Vector_String_AS3push::Method = &Instances::fl_vec::Vector_String::AS3push;
template <> const TFunc_Instances_Vector_String_AS3some::TMethod TFunc_Instances_Vector_String_AS3some::Method = &Instances::fl_vec::Vector_String::AS3some;
template <> const TFunc_Instances_Vector_String_AS3unshift::TMethod TFunc_Instances_Vector_String_AS3unshift::Method = &Instances::fl_vec::Vector_String::AS3unshift;
template <> const TFunc_Instances_Vector_String_AS3concat::TMethod TFunc_Instances_Vector_String_AS3concat::Method = &Instances::fl_vec::Vector_String::AS3concat;
template <> const TFunc_Instances_Vector_String_AS3filter::TMethod TFunc_Instances_Vector_String_AS3filter::Method = &Instances::fl_vec::Vector_String::AS3filter;
template <> const TFunc_Instances_Vector_String_AS3pop::TMethod TFunc_Instances_Vector_String_AS3pop::Method = &Instances::fl_vec::Vector_String::AS3pop;
template <> const TFunc_Instances_Vector_String_AS3reverse::TMethod TFunc_Instances_Vector_String_AS3reverse::Method = &Instances::fl_vec::Vector_String::AS3reverse;
template <> const TFunc_Instances_Vector_String_AS3shift::TMethod TFunc_Instances_Vector_String_AS3shift::Method = &Instances::fl_vec::Vector_String::AS3shift;
template <> const TFunc_Instances_Vector_String_AS3slice::TMethod TFunc_Instances_Vector_String_AS3slice::Method = &Instances::fl_vec::Vector_String::AS3slice;
template <> const TFunc_Instances_Vector_String_AS3sort::TMethod TFunc_Instances_Vector_String_AS3sort::Method = &Instances::fl_vec::Vector_String::AS3sort;
template <> const TFunc_Instances_Vector_String_AS3splice::TMethod TFunc_Instances_Vector_String_AS3splice::Method = &Instances::fl_vec::Vector_String::AS3splice;
template <> const TFunc_Instances_Vector_String_AS3indexOf::TMethod TFunc_Instances_Vector_String_AS3indexOf::Method = &Instances::fl_vec::Vector_String::AS3indexOf;
template <> const TFunc_Instances_Vector_String_AS3lastIndexOf::TMethod TFunc_Instances_Vector_String_AS3lastIndexOf::Method = &Instances::fl_vec::Vector_String::AS3lastIndexOf;

namespace Instances { namespace fl_vec
{
    Vector_String::Vector_String(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Vector_String::Vector_String()$data"
    , V(t.GetVM().GetMemoryHeap(), t.GetVM())
//##protect##"instance::Vector_String::Vector_String()$data"
    {
//##protect##"instance::Vector_String::Vector_String()$code"
//##protect##"instance::Vector_String::Vector_String()$code"
    }

    void Vector_String::lengthGet(UInt32& result)
    {
//##protect##"instance::Vector_String::lengthGet()"
        result = static_cast<UInt32>(V.GetSize());
//##protect##"instance::Vector_String::lengthGet()"
    }
    void Vector_String::lengthSet(const Value& result, UInt32 value)
    {
//##protect##"instance::Vector_String::lengthSet()"
        SF_UNUSED1(result);
        V.Resize(value).DoNotCheck();
//##protect##"instance::Vector_String::lengthSet()"
    }
    void Vector_String::fixedSet(const Value& result, bool f)
    {
//##protect##"instance::Vector_String::fixedSet()"
        SF_UNUSED1(result);
        V.SetFixed(f);
//##protect##"instance::Vector_String::fixedSet()"
    }
    void Vector_String::fixedGet(bool& result)
    {
//##protect##"instance::Vector_String::fixedGet()"
        result = V.GetFixed();
//##protect##"instance::Vector_String::fixedGet()"
    }
    void Vector_String::AS3toString(ASString& result)
    {
//##protect##"instance::Vector_String::AS3toString()"
        result = V.ToString(GetVM().GetStringManager().GetBuiltin(AS3Builtin_comma));
//##protect##"instance::Vector_String::AS3toString()"
    }
    void Vector_String::AS3toLocaleString(ASString& result)
    {
//##protect##"instance::Vector_String::AS3toLocaleString()"
        AS3toString(result);
//##protect##"instance::Vector_String::AS3toLocaleString()"
    }
    void Vector_String::AS3join(ASString& result, const ASString& separator)
    {
//##protect##"instance::Vector_String::AS3join()"
        result = V.ToString(separator);
//##protect##"instance::Vector_String::AS3join()"
    }
    void Vector_String::AS3every(bool& result, const Value& checker, const Value& thisObj)
    {
//##protect##"instance::Vector_String::AS3every()"
        result = V.Every(checker, thisObj, *this);
//##protect##"instance::Vector_String::AS3every()"
    }
    void Vector_String::AS3forEach(const Value& result, const Value& eacher, const Value& thisObj)
    {
//##protect##"instance::Vector_String::AS3forEach()"
        SF_UNUSED1(result);
        V.ForEach(eacher, thisObj, *this);
//##protect##"instance::Vector_String::AS3forEach()"
    }
    void Vector_String::AS3map(SPtr<Instances::fl_vec::Vector_String>& result, const Value& mapper, const Value& thisObj)
    {
//##protect##"instance::Vector_String::AS3map()"
        V.Map(result, mapper, thisObj, this);
//##protect##"instance::Vector_String::AS3map()"
    }
    void Vector_String::AS3push(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Vector_String::AS3push()"
        V.PushBack(argc, argv, GetEnclosedClassTraits());
        result.SetUInt32(static_cast<UInt32>(V.GetSize()));
//##protect##"instance::Vector_String::AS3push()"
    }
    void Vector_String::AS3some(bool& result, const Value& checker, const Value& thisObj)
    {
//##protect##"instance::Vector_String::AS3some()"
        result = V.Some(checker, thisObj, *this);
//##protect##"instance::Vector_String::AS3some()"
    }
    void Vector_String::AS3unshift(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Vector_String::AS3unshift()"
        V.Unshift(argc, argv, GetEnclosedClassTraits());
        result.SetUInt32(static_cast<UInt32>(V.GetSize()));
//##protect##"instance::Vector_String::AS3unshift()"
    }
    void Vector_String::AS3concat(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Vector_String::AS3concat()"
        V.Concat(result, argc, argv, *this);
//##protect##"instance::Vector_String::AS3concat()"
    }
    void Vector_String::AS3filter(SPtr<Instances::fl_vec::Vector_String>& result, const Value& checker, const Value& thisObj)
    {
//##protect##"instance::Vector_String::AS3filter()"
        V.Filter(result, checker, thisObj, *this);
//##protect##"instance::Vector_String::AS3filter()"
    }
    void Vector_String::AS3pop(ASString& result)
    {
//##protect##"instance::Vector_String::AS3pop()"
        V.Pop(result);
//##protect##"instance::Vector_String::AS3pop()"
    }
    void Vector_String::AS3reverse(SPtr<Instances::fl_vec::Vector_String>& result)
    {
//##protect##"instance::Vector_String::AS3reverse()"
        V.Reverse();
        result = this;
//##protect##"instance::Vector_String::AS3reverse()"
    }
    void Vector_String::AS3shift(ASString& result)
    {
//##protect##"instance::Vector_String::AS3shift()"
        V.Shift(result);
//##protect##"instance::Vector_String::AS3shift()"
    }
    void Vector_String::AS3slice(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Vector_String::AS3slice()"
        V.Slice(result, argc, argv, *this);
//##protect##"instance::Vector_String::AS3slice()"
    }
    void Vector_String::AS3sort(SPtr<Instances::fl_vec::Vector_String>& result, const Value& comparefn)
    {
//##protect##"instance::Vector_String::AS3sort()"
        V.Sort(result, comparefn, *this);
//##protect##"instance::Vector_String::AS3sort()"
    }
    void Vector_String::AS3splice(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Vector_String::AS3splice()"
        V.Splice(result, argc, argv, *this);
//##protect##"instance::Vector_String::AS3splice()"
    }
    void Vector_String::AS3indexOf(SInt32& result, const ASString& value, SInt32 from)
    {
//##protect##"instance::Vector_String::AS3indexOf()"
        V.IndexOf(result, value.GetNode(), from);
//##protect##"instance::Vector_String::AS3indexOf()"
    }
    void Vector_String::AS3lastIndexOf(SInt32& result, const ASString& value, SInt32 from)
    {
//##protect##"instance::Vector_String::AS3lastIndexOf()"
        V.LastIndexOf(result, value.GetNode(), from);
//##protect##"instance::Vector_String::AS3lastIndexOf()"
    }

//##protect##"instance$methods"
    Vector_String::Vector_String(InstanceTraits::Traits& t, UInt32 length, bool fixed)
        : Instances::fl::Object(t)
        , V(t.GetVM().GetMemoryHeap(), t.GetVM())
    {
        if (!V.Resize(length))
            return;

        V.SetFixed(fixed);
    }

    void Vector_String::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 0)
        {
            UInt32 length;
            if (!argv[0].Convert2UInt32(length))
                return;

            if (!V.Resize(length))
                return;
        }

        if (argc > 1)
        {
            V.SetFixed(argv[1].Convert2Boolean());
        }
    }

    void Vector_String::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        if (!ind.IsValid())
            return name.SetUndefined();

        name.SetUInt32(static_cast<UInt32>(AbsoluteIndex(ind).Get()));
    }

    GlobalSlotIndex Vector_String::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
        GlobalSlotIndex gind(0);
        const AbsoluteIndex aind = GetNextArrayIndex(AbsoluteIndex(ind));

        if (aind.IsValid())
            gind = GlobalSlotIndex(aind);

        return gind;
    }

    CheckResult Vector_String::SetProperty(const Multiname& prop_name, const Value& value)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            return Set(ind, value, GetEnclosedClassTraits());

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::SetProperty(prop_name, value);
    }

    CheckResult Vector_String::GetProperty(const Multiname& prop_name, Value& value)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
        {
            if (ind < V.GetSize())
            {
                Get(ind, value);
                return true;
            }

            VM& vm = GetVM();
            vm.ThrowRangeError(VM::Error(VM::eOutOfRangeError, vm
                SF_DEBUG_ARG(ind)
                SF_DEBUG_ARG(V.GetSize())
                ));

            return false;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::GetProperty(prop_name, value);
    }

    void Vector_String::GetDynamicProperty(AbsoluteIndex ind, Value& value)
    {
        V.GetValue(ind.Get(), value);
    }

    CheckResult Vector_String::DeleteProperty(const Multiname& prop_name)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            return RemoveAt(ind);

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::DeleteProperty(prop_name);
    }

    bool Vector_String::HasProperty(const Multiname& prop_name, bool check_prototype)
    {
        UInt32 ind;
        if (GetVectorInd(prop_name, ind))
            return ind < V.GetSize();

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::HasProperty(prop_name, check_prototype);
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_vec
{
    // const UInt16 Vector_String::tito[Vector_String::ThunkInfoNum] = {
    //    0, 1, 3, 5, 6, 7, 8, 10, 13, 16, 19, 20, 23, 24, 25, 28, 29, 30, 31, 32, 34, 37, 40, 
    // };
    const TypeInfo* Vector_String::tit[43] = {
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::FunctionTI, &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::FunctionTI, &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::FunctionTI, &AS3::fl::ObjectTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::uintTI, 
        NULL, 
        NULL, &AS3::fl::FunctionTI, &AS3::fl::ObjectTI, 
        &AS3::fl::StringTI, 
        NULL, 
        &AS3::fl::StringTI, 
        NULL, 
        NULL, NULL, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, &AS3::fl::StringTI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::StringTI, &AS3::fl::int_TI, 
    };
    const Abc::ConstValue Vector_String::dva[2] = {
        {Abc::CONSTANT_Utf8, 1}, 
        {Abc::CONSTANT_Int, 1}, 
    };
    const ThunkInfo Vector_String::ti[Vector_String::ThunkInfoNum] = {
        {TFunc_Instances_Vector_String_lengthGet::Func, &Vector_String::tit[0], "length", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_lengthSet::Func, &Vector_String::tit[1], "length", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Vector_String_fixedSet::Func, &Vector_String::tit[3], "fixed", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Vector_String_fixedGet::Func, &Vector_String::tit[5], "fixed", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3toString::Func, &Vector_String::tit[6], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3toLocaleString::Func, &Vector_String::tit[7], "toLocaleString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3join::Func, &Vector_String::tit[8], "join", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Vector_String::dva[0]},
        {TFunc_Instances_Vector_String_AS3every::Func, &Vector_String::tit[10], "every", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3forEach::Func, &Vector_String::tit[13], "forEach", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3map::Func, &Vector_String::tit[16], "map", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3push::Func, &Vector_String::tit[19], "push", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Vector_String_AS3some::Func, &Vector_String::tit[20], "some", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3unshift::Func, &Vector_String::tit[23], "unshift", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Vector_String_AS3concat::Func, &Vector_String::tit[24], "concat", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Vector_String_AS3filter::Func, &Vector_String::tit[25], "filter", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3pop::Func, &Vector_String::tit[28], "pop", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3reverse::Func, &Vector_String::tit[29], "reverse", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3shift::Func, &Vector_String::tit[30], "shift", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3slice::Func, &Vector_String::tit[31], "slice", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1, 0, NULL},
        {TFunc_Instances_Vector_String_AS3sort::Func, &Vector_String::tit[32], "sort", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3splice::Func, &Vector_String::tit[34], "splice", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Vector_String_AS3indexOf::Func, &Vector_String::tit[37], "indexOf", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Vector_String_AS3lastIndexOf::Func, &Vector_String::tit[40], "lastIndexOf", NS_AS3, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Vector_String::dva[1]},
    };

    Vector_String::Vector_String(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Vector_String::Vector_String()"
        SetArrayLike();
        SetTraitsType(Traits_Vector_String);
//##protect##"InstanceTraits::Vector_String::Vector_String()"

    }

    void Vector_String::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Vector_String&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_vec
{
    Vector_String::Vector_String(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Vector_String::Vector_String()"
//##protect##"class_::Vector_String::Vector_String()"
    }
//##protect##"class_$methods"
    void Vector_String::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        if (argc == 1)
        {
            const Value& value = argv[0];

            if (value.IsNullOrUndefined())
            {
                result = value;
                return;
            }

            InstanceTraits::fl_vec::Vector_String& itr = static_cast<InstanceTraits::fl_vec::Vector_String&>(GetClassTraits().GetInstanceTraits());
            const Traits& value_tr = GetVM().GetValueTraits(value);

            // Class Vector.<X> is final. 
            if (&value_tr == &itr)
            {
                result = value;
                return;
            }

            SPtr<Instances::fl_vec::Vector_String> vstr(itr.MakeInstance(itr));

            if (vstr->AppendCoerce(value))
                result = vstr;
            else
                GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                SF_DEBUG_ARG(value)
                SF_DEBUG_ARG(itr.GetName().ToCStr())
                ));
        }
        else
            return GetVM().ThrowRangeError(VM::Error(VM::eCoerceArgumentCountError, GetVM() SF_DEBUG_ARG(argc)));
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_vec
{

    Vector_String::Vector_String(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Vector_String::Vector_String()"
        SetTraitsType(Traits_Vector_String);
//##protect##"ClassTraits::Vector_String::Vector_String()"

    }

    Pickable<Traits> Vector_String::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Vector_String(vm, AS3::fl_vec::Vector_StringCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_vec::Vector_StringCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_vec
{
    const TypeInfo Vector_StringTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject | TypeInfo::Final,
        sizeof(ClassTraits::fl_vec::Vector_String::InstanceType),
        0,
        0,
        InstanceTraits::fl_vec::Vector_String::ThunkInfoNum,
        0,
        "Vector$String", "__AS3__.vec", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo Vector_StringCI = {
        &Vector_StringTI,
        ClassTraits::fl_vec::Vector_String::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_vec::Vector_String::ti,
        NULL,
    };
}; // namespace fl_vec


}}} // namespace Scaleform { namespace GFx { namespace AS3

