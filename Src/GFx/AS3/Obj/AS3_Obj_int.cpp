//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_int.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_int.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "Kernel/SF_MsgFormat.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl
{
    // const UInt16 int_::tito[int_::ThunkInfoNum] = {
    //    0, 2, 4, 6, 8, 
    // };
    const TypeInfo* int_::tit[9] = {
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::int_TI, 
    };
    const Abc::ConstValue int_::dva[1] = {
        {Abc::CONSTANT_UInt, 1}, 
    };
    const ThunkInfo int_::ti[int_::ThunkInfoNum] = {
        {&InstanceTraits::fl::int_::AS3toExponential, &int_::tit[0], "toExponential", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::int_::AS3toFixed, &int_::tit[2], "toFixed", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::int_::AS3toPrecision, &int_::tit[4], "toPrecision", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::int_::AS3toString, &int_::tit[6], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &int_::dva[0]},
        {&InstanceTraits::fl::int_::AS3valueOf, &int_::tit[8], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    int_::int_(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::int_::int_()"
        SetTraitsType(Traits_SInt);
//##protect##"InstanceTraits::int_::int_()"

    }

    void int_::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t); SF_ASSERT(false);
    }

    void int_::AS3toExponential(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
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

        DoubleFormatter f(_this.AsInt());
        f.SetType(DoubleFormatter::FmtScientific).SetPrecision(fractionDigits).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toExponential()"
    }
    void int_::AS3toFixed(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
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

        DoubleFormatter f(_this.AsInt());
        f.SetType(DoubleFormatter::FmtDecimal).SetPrecision(fractionDigits).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toFixed()"
    }
    void int_::AS3toPrecision(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
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

        DoubleFormatter f(_this.AsInt());
        f.SetType(DoubleFormatter::FmtSignificant).SetPrecision(precision).Convert();
        result = vm.GetStringManager().CreateString(f.GetResult().ToCStr(), f.GetSize());
//##protect##"InstanceTraits::AS3toPrecision()"
    }
    void int_::AS3toString(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toString()"
        SF_UNUSED3(cti, argc, argv);

        // This method is not generic.
        if (!_this.IsInt())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        // If radix is the number 10 or undefined ...
        // Save undefined.
        UInt32 radix = 10;
        if (argc > 0 && !argv[0].IsUndefined())
            if (!argv[0].Convert2UInt32(radix))
                // Exception
                return;

        if (radix < 2 || radix > 16)
            return vm.ThrowRangeError(VM::Error(VM::eInvalidRadixError, vm SF_DEBUG_ARG(radix)));

        LongFormatter f(_this.AsInt());
        f.SetBase(radix).SetBigLetters(false).Convert();
        StringDataPtr r = f.GetResult();

        result = vm.GetStringManager().CreateString(r.ToCStr(), r.GetSize());
//##protect##"InstanceTraits::AS3toString()"
    }
    void int_::AS3valueOf(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3valueOf()"
        SF_UNUSED4(cti, vm, argc, argv);

        // This method is not generic.
        if (!_this.IsInt())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result.SetSInt32(_this.AsInt());
//##protect##"InstanceTraits::AS3valueOf()"
    }
    void int_::toStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toStringProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassSInt().GetPrototype())
            // This method is called on a prototype object.
            result = vm.GetStringManager().GetBuiltin(AS3Builtin_zero);
        else
            AS3toString(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::toStringProto()"
    }
    void int_::toLocaleStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toLocaleStringProto()"
        toStringProto(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::toLocaleStringProto()"
    }
    void int_::valueOfProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::valueOfProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassSInt().GetPrototype())
            // This method is called on a prototype object.
            result.SetSInt32(0);
        else
            AS3valueOf(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::valueOfProto()"
    }
    void int_::toExponentialProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toExponentialProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsSInt().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsSInt().GetName().ToCStr())
                ));
        }

        AS3toExponential(cti, vm, coerced_this, result, argc, argv);
//##protect##"InstanceTraits::toExponentialProto()"
    }
    void int_::toFixedProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toFixedProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsSInt().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsSInt().GetName().ToCStr())
                ));
        }

        AS3toFixed(cti, vm, coerced_this, result, argc, argv);
//##protect##"InstanceTraits::toFixedProto()"
    }
    void int_::toPrecisionProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toPrecisionProto()"
        Value coerced_this;

        if (!vm.GetClassTraitsSInt().CoerceValue(_this, coerced_this))
        {
            return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(vm.GetClassTraitsSInt().GetName().ToCStr())
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
    // const UInt16 int_::tito[int_::ThunkInfoNum] = {
    //    0, 2, 4, 5, 7, 9, 
    // };
    const TypeInfo* int_::tit[11] = {
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
    };
    const Abc::ConstValue int_::dva[2] = {
        {Abc::CONSTANT_UInt, 1}, 
        {Abc::CONSTANT_UInt, 1}, 
    };
    const ThunkInfo int_::ti[int_::ThunkInfoNum] = {
        {&InstanceTraits::fl::int_::toStringProto, &int_::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &int_::dva[0]},
        {&InstanceTraits::fl::int_::toLocaleStringProto, &int_::tit[2], "toLocaleString", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &int_::dva[1]},
        {&InstanceTraits::fl::int_::valueOfProto, &int_::tit[4], "valueOf", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::int_::toExponentialProto, &int_::tit[5], "toExponential", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::int_::toFixedProto, &int_::tit[7], "toFixed", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {&InstanceTraits::fl::int_::toPrecisionProto, &int_::tit[9], "toPrecision", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
    };

    int_::int_(ClassTraits::Traits& t)
    : Class(t)
    , MIN_VALUE(SF_MIN_SINT32)
    , MAX_VALUE(SF_MAX_SINT32)
    {
//##protect##"class_::int_::int_()"
//##protect##"class_::int_::int_()"
    }

    void int_::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void int_::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void int_::Construct(Value& result, unsigned argc, const Value* argv, bool /*extCall*/)
    {
        if (argc > 0)
        {
            SInt32 v;
            if (argv[0].Convert2Int32(v))
                result.SetSInt32(v);
        }
        else
            result.SetSInt32(0);
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{
    const MemberInfo int_::mi[int_::MemberInfoNum] = {
        {"MIN_VALUE", NULL, OFFSETOF(Classes::fl::int_, MIN_VALUE), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"MAX_VALUE", NULL, OFFSETOF(Classes::fl::int_, MAX_VALUE), Abc::NS_Public, SlotInfo::BT_Int, 1},
    };


    int_::int_(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::int_::int_()"
        SetTraitsType(Traits_SInt);
//##protect##"ClassTraits::int_::int_()"

    }

    Pickable<Traits> int_::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) int_(vm, AS3::fl::int_CI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::int_CI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
    bool int_::Coerce(const Value& value, Value& result) const
    {
        return CoerceValue(value, result);
    }
    bool int_::CoerceValue(const Value& value, Value& result) const
    {
        SInt32 r;

        if (value.Convert2Int32(r))
        {
            result.SetSInt32(r);
            return true;
        }

        return false;
    }
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo int_TI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::int_::InstanceType),
        0,
        ClassTraits::fl::int_::MemberInfoNum,
        InstanceTraits::fl::int_::ThunkInfoNum,
        0,
        "int", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo int_CI = {
        &int_TI,
        ClassTraits::fl::int_::MakeClassTraits,
        NULL,
        ClassTraits::fl::int_::mi,
        InstanceTraits::fl::int_::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

