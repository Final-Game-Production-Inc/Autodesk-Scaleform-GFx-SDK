//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Number.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Number.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "Kernel/SF_MsgFormat.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
extern UPInt SF_ECMA_dtostr(char* buffer, int bufflen, Double val);
//##protect##"methods"

namespace InstanceTraits { namespace fl
{
    // const UInt16 Number::tito[Number::ThunkInfoNum] = {
    //    0, 2, 4, 6, 8, 10, 
    // };
    const TypeInfo* Number::tit[11] = {
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
    };
    const Abc::ConstValue Number::dva[2] = {
        {Abc::CONSTANT_Double, 1}, 
        {Abc::CONSTANT_Double, 1}, 
    };
    const ThunkInfo Number::ti[Number::ThunkInfoNum] = {
        {&InstanceTraits::fl::Number::AS3toExponential, &Number::tit[0], "toExponential", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::Number::AS3toFixed, &Number::tit[2], "toFixed", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::Number::AS3toPrecision, &Number::tit[4], "toPrecision", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::Number::AS3toString, &Number::tit[6], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Number::dva[0]},
        {&InstanceTraits::fl::Number::AS3toLocaleString, &Number::tit[8], "toLocaleString", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Number::dva[1]},
        {&InstanceTraits::fl::Number::AS3valueOf, &Number::tit[10], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Number::Number(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Number::Number()"
        SetTraitsType(Traits_Number);
//##protect##"InstanceTraits::Number::Number()"

    }

    void Number::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t); SF_ASSERT(false);
    }

    void Number::AS3toExponential(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toExponential()"
        SF_UNUSED1(cti);

        UInt32 fractionDigits = 0;

        if (argc > 0)
            if (!argv[0].Convert2UInt32(fractionDigits))
                // Exception
                return;

        if (fractionDigits > 20)
            return vm.ThrowRangeError(VM::Error(VM::eInvalidPrecisionError, vm));

        DoubleFormatter f(static_cast<double>(_this));
        f.SetType(DoubleFormatter::FmtScientific).SetPrecision(fractionDigits).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toExponential()"
    }
    void Number::AS3toFixed(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toFixed()"
        SF_UNUSED1(cti);

        UInt32 fractionDigits = 0;

        if (argc > 0)
            if (!argv[0].Convert2UInt32(fractionDigits))
                // Exception
                return;

        if (fractionDigits > 20)
            return vm.ThrowRangeError(VM::Error(VM::eInvalidPrecisionError, vm));

        DoubleFormatter f(static_cast<double>(_this));
        f.SetType(DoubleFormatter::FmtDecimal).SetPrecision(fractionDigits).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toFixed()"
    }
    void Number::AS3toPrecision(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toPrecision()"
        SF_UNUSED1(cti);

        UInt32 precision = 0;

        if (argc > 0)
            if (!argv[0].Convert2UInt32(precision))
                // Exception
                return;

        if (precision == 0 || precision > 21)
            return vm.ThrowRangeError(VM::Error(VM::eInvalidPrecisionError, vm));

        DoubleFormatter f(static_cast<Double>(_this));
        f.SetType(DoubleFormatter::FmtSignificant).SetPrecision(precision).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toPrecision()"
    }
    void Number::AS3toString(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toString()"
        SF_UNUSED1(cti);

        UInt32 radix = 10;

        // The toString function is not generic; it throws a TypeError exception 
        // if its this value is not a Number object.
        // Therefore, it cannot be transferred to other kinds of objects for use as a method.
        if (!_this.IsNumber())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        // If radix is the number 10 or undefined ...
        // Save undefined.
        if (argc > 0 && !argv[0].IsUndefined())
            if (!argv[0].Convert2UInt32(radix))
                // Exception
                return;

        if (radix < 2 || radix > 36)
            return vm.ThrowRangeError(VM::Error(VM::eInvalidRadixError, vm SF_DEBUG_ARG(radix)));

        const Value::Number num = _this.AsNumber();

        if (radix != 10 && !NumberUtil::IsNaNOrInfinity(num)) 
        { 
            SInt32 v; 
            if (!_this.Convert2Int32(v)) 
                // exception 
                return; 
            LongFormatter f(v); 
            f.SetBase(radix).SetBigLetters(false).Convert(); 
            StringDataPtr r = f.GetResult(); 
            result = vm.GetStringManager().CreateString(r.ToCStr(), r.GetSize()); 
            return; 
        } 

        char  buffer[40];
        UPInt length = SF_ECMA_dtostr(buffer, sizeof(buffer), num);
        result = vm.GetStringManager().CreateString(buffer, length);
//##protect##"InstanceTraits::AS3toString()"
    }
    void Number::AS3toLocaleString(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toLocaleString()"
        AS3toString(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::AS3toLocaleString()"
    }
    void Number::AS3valueOf(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3valueOf()"
        SF_UNUSED4(cti, vm, argc, argv);

        // The valueOf function is not generic; it throws a TypeError exception 
        // if its this value is not a Number object.
        // Therefore, it cannot be transferred to other kinds of objects for use as a method.
        if (!_this.IsNumber())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this;
//##protect##"InstanceTraits::AS3valueOf()"
    }
    void Number::toStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toStringProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassNumber().GetPrototype())
            // This method is called on a prototype object.
            result = vm.GetStringManager().GetBuiltin(AS3Builtin_zero);
        else
            AS3toString(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::toStringProto()"
    }
    void Number::toLocaleStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toLocaleStringProto()"
        toStringProto(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::toLocaleStringProto()"
    }
    void Number::valueOfProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::valueOfProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassNumber().GetPrototype())
            // This method is called on a prototype object.
            result.SetNumber(0);
        else
            AS3valueOf(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::valueOfProto()"
    }
    void Number::toExponentialProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toExponentialProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsNumber().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsNumber().GetName().ToCStr())
                ));
        }

        AS3toExponential(cti, vm, coerced_this, result, argc, argv);
//##protect##"InstanceTraits::toExponentialProto()"
    }
    void Number::toFixedProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toFixedProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsNumber().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsNumber().GetName().ToCStr())
                ));
        }

        AS3toFixed(cti, vm, coerced_this, result, argc, argv);
//##protect##"InstanceTraits::toFixedProto()"
    }
    void Number::toPrecisionProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toPrecisionProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsNumber().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsNumber().GetName().ToCStr())
                ));
        }

        AS3toPrecision(cti, vm, coerced_this, result, argc, argv);
//##protect##"InstanceTraits::toPrecisionProto()"
    }
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    // const UInt16 Number::tito[Number::ThunkInfoNum] = {
    //    0, 2, 4, 5, 7, 9, 
    // };
    const TypeInfo* Number::tit[11] = {
        &AS3::fl::StringTI, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
    };
    const Abc::ConstValue Number::dva[2] = {
        {Abc::CONSTANT_Double, 1}, 
        {Abc::CONSTANT_Double, 1}, 
    };
    const ThunkInfo Number::ti[Number::ThunkInfoNum] = {
        {&InstanceTraits::fl::Number::toStringProto, &Number::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Number::dva[0]},
        {&InstanceTraits::fl::Number::toLocaleStringProto, &Number::tit[2], "toLocaleString", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Number::dva[1]},
        {&InstanceTraits::fl::Number::valueOfProto, &Number::tit[4], "valueOf", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Number::toExponentialProto, &Number::tit[5], "toExponential", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::Number::toFixedProto, &Number::tit[7], "toFixed", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::Number::toPrecisionProto, &Number::tit[9], "toPrecision", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
    };

    Number::Number(ClassTraits::Traits& t)
    : Class(t)
    , NaN(NumberUtil::NaN())
    , NEGATIVE_INFINITY(NumberUtil::NEGATIVE_INFINITY())
    , POSITIVE_INFINITY(NumberUtil::POSITIVE_INFINITY())
    , MIN_VALUE(2.2250738585072014e-308)
    , MAX_VALUE(1.7976931348623158e+308)
    {
//##protect##"class_::Number::Number()"
//##protect##"class_::Number::Number()"
    }

    void Number::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void Number::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void Number::Construct(Value& result, unsigned argc, const Value* argv, bool /*extCall*/)
    {
        if (argc == 0)
            result.SetNumber(0.0);
        else if (argc == 1)
        {
            Value::Number v;
            if (argv[0].Convert2Number(v))
                result.SetNumber(v);
        }
        else
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("Number::Construct") SF_DEBUG_ARG(0) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{
    const MemberInfo Number::mi[Number::MemberInfoNum] = {
        {"NaN", NULL, OFFSETOF(Classes::fl::Number, NaN), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"NEGATIVE_INFINITY", NULL, OFFSETOF(Classes::fl::Number, NEGATIVE_INFINITY), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"POSITIVE_INFINITY", NULL, OFFSETOF(Classes::fl::Number, POSITIVE_INFINITY), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"MIN_VALUE", NULL, OFFSETOF(Classes::fl::Number, MIN_VALUE), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"MAX_VALUE", NULL, OFFSETOF(Classes::fl::Number, MAX_VALUE), Abc::NS_Public, SlotInfo::BT_Number, 1},
    };


    Number::Number(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Number::Number()"
        SetTraitsType(Traits_Number);
//##protect##"ClassTraits::Number::Number()"

    }

    Pickable<Traits> Number::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Number(vm, AS3::fl::NumberCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::NumberCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
    bool Number::Coerce(const Value& value, Value& result) const
    {
        return CoerceValue(value, result);
    }

    bool Number::CoerceValue(const Value& value, Value& result) const
    {
        Value::Number r;

        if (value.Convert2Number(r))
        {
            result.SetNumber(r);
            return true;
        }

        return false;
    }
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo NumberTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::Number::InstanceType),
        0,
        ClassTraits::fl::Number::MemberInfoNum,
        InstanceTraits::fl::Number::ThunkInfoNum,
        0,
        "Number", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NumberCI = {
        &NumberTI,
        ClassTraits::fl::Number::MakeClassTraits,
        NULL,
        ClassTraits::fl::Number::mi,
        InstanceTraits::fl::Number::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

