//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Boolean.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Boolean.h"
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
    // const UInt16 Boolean::tito[Boolean::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* Boolean::tit[2] = {
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Boolean::ti[Boolean::ThunkInfoNum] = {
        {&InstanceTraits::fl::Boolean::AS3toString, &Boolean::tit[0], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Boolean::AS3valueOf, &Boolean::tit[1], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Boolean::Boolean(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Boolean::Boolean()"
        SetTraitsType(Traits_Boolean);
//##protect##"InstanceTraits::Boolean::Boolean()"

    }

    void Boolean::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t); SF_ASSERT(false);
    }

    void Boolean::AS3toString(const ThunkInfo& tifo, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toString()"
        SF_UNUSED3(tifo, argc, argv);

        // This method is not generic.
        if (!_this.IsBool())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = vm.GetStringManager().GetBuiltin(_this.AsBool() ? AS3Builtin_true : AS3Builtin_false);
//##protect##"InstanceTraits::AS3toString()"
    }
    void Boolean::AS3valueOf(const ThunkInfo& tifo, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3valueOf()"
        SF_UNUSED3(tifo, argc, argv);

        // This method is not generic.
        if (!_this.IsBool())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this;
//##protect##"InstanceTraits::AS3valueOf()"
    }
    void Boolean::toStringProto(const ThunkInfo& tifo, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toStringProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassBoolean().GetPrototype())
            // This method is called on a prototype object.
            result = vm.GetStringManager().GetBuiltin(AS3Builtin_false);
        else
            AS3toString(tifo, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::toStringProto()"
    }
    void Boolean::valueOfProto(const ThunkInfo& tifo, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::valueOfProto()"
        if (_this.IsObject() && _this.GetObject() == &vm.GetClassBoolean().GetPrototype())
            // This method is called on a prototype object.
            result.SetBool(false);
        else
            AS3valueOf(tifo, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::valueOfProto()"
    }
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    // const UInt16 Boolean::tito[Boolean::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* Boolean::tit[2] = {
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Boolean::ti[Boolean::ThunkInfoNum] = {
        {&InstanceTraits::fl::Boolean::toStringProto, &Boolean::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Boolean::valueOfProto, &Boolean::tit[1], "valueOf", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Boolean::Boolean(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Boolean::Boolean()"
//##protect##"class_::Boolean::Boolean()"
    }

    void Boolean::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void Boolean::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void Boolean::Construct(Value& result, unsigned argc, const Value* argv, bool /*extCall*/)
    {
        if (argc > 0)
            result.SetBool(argv[0].Convert2Boolean());
        else
            result.SetBool(false);
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{

    Boolean::Boolean(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Boolean::Boolean()"
        SetTraitsType(Traits_Boolean);
//##protect##"ClassTraits::Boolean::Boolean()"

    }

    Pickable<Traits> Boolean::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Boolean(vm, AS3::fl::BooleanCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::BooleanCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
    bool Boolean::Coerce(const Value& value, Value& result) const
    {
        return CoerceValue(value, result);
    }
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo BooleanTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::Boolean::InstanceType),
        0,
        0,
        InstanceTraits::fl::Boolean::ThunkInfoNum,
        0,
        "Boolean", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo BooleanCI = {
        &BooleanTI,
        ClassTraits::fl::Boolean::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl::Boolean::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

