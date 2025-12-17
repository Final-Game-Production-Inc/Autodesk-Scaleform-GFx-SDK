//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Object.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Object.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace InstanceTraits
{
    Interface::Interface(VM& vm, const ClassInfo& ci)
        : CTraits(vm, ci)
    {
        SetMemSize(sizeof(Instances::fl::Object));
        SetIsInterface();
    }

    Pickable<Instances::fl::Object> Interface::MakeInstance(Traits& t)
    {
        return Pickable<Instances::fl::Object>(new(t.Alloc()) Instances::fl::Object(t));
    }

    void Interface::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(t);
    }
}
//##protect##"methods"


namespace Instances { namespace fl
{
    Object::Object(InstanceTraits::Traits& t)
    : Instance(t)
//##protect##"instance::Object::Object()$data"
//##protect##"instance::Object::Object()$data"
    {
//##protect##"instance::Object::Object()$code"
//##protect##"instance::Object::Object()$code"
    }

    void Object::AS3isPrototypeOf(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::AS3isPrototypeOf()"
        // ECMA-262 Section 15.2.4.6 

        // Indicates whether an instance of the Object class is in the prototype 
        // chain of the object specified as the parameter. This method returns 
        // true if the object is in the prototype chain of the object specified 
        // by the theClass parameter. The method returns false if the target 
        // object is absent from the prototype chain of the theClass  object, 
        // and also if the theClass parameter is not an object. 

        SF_UNUSED1(ti);

        if (_this.IsNullOrUndefined())
            return vm.ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, vm));

        if (argc < 1)
            // No argument.
            return vm.ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, vm SF_DEBUG_ARG("Object::AS3isPrototypeOf") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));

        const Value& theClass = argv[0];
        if (theClass.IsNullOrUndefined())
            return result.SetBool(false);

        // Search in a prototype chain.
        const AS3::Object* thisObj = _this.GetObject();
        const Traits* cur_tr = &vm.GetValueTraits(theClass);
        while (cur_tr)
        {
            const AS3::Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            if (&proto == thisObj)
                return result.SetBool(true);

            cur_tr = cur_tr->GetParent();
        }

        result.SetBool(false);
//##protect##"instance::Object::AS3isPrototypeOf()"
    }
    void Object::AS3hasOwnProperty(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::AS3hasOwnProperty()"
        SF_UNUSED1(ti);

        if (_this.IsNullOrUndefined())
            return vm.ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, vm));

        if (argc < 1)
            // No argument.
            return vm.ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, vm SF_DEBUG_ARG("Object::AS3hasOwnProperty") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));

        if (_this.IsObject())
        {
            const Value& name = argv[0];
            const Multiname mn(vm.GetPublicNamespace(), name);
            // DO NOT check prototype.
            result.SetBool(_this.GetObject()->HasProperty(mn, false));
        }
        else
        {
            const Value& vn = argv[0];
            ASString name = vm.GetStringManager().CreateEmptyString();
            if (!vn.Convert2String(name))
                return;

            UPInt index = 0;
            const SlotInfo* si = AS3::FindFixedSlot(vm.GetValueTraits(_this), name, vm.GetPublicNamespace(), index, NULL);
            result.SetBool(si != NULL);
        }
//##protect##"instance::Object::AS3hasOwnProperty()"
    }
    void Object::AS3propertyIsEnumerable(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::AS3propertyIsEnumerable()"
        SF_UNUSED1(ti);

        // One argument is required.
        if (argc < 1)
            return vm.ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, vm SF_DEBUG_ARG("Object::AS3propertyIsEnumerable") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));

        if (_this.IsObject() && !_this.IsNull())
        {
            const Value& name = argv[0];
            UInt32 ind = 0;
            const Traits& tr = vm.GetValueTraits(_this);
            if (tr.GetTraitsType() == Traits_Array && tr.IsInstanceTraits() && (name.IsNumeric() || (name.IsString() && GetArrayInd(name.GetStringNode(), ind))))
                return result.SetBool(true);

            StringManager& sm = vm.GetStringManager();
            ASString str_name = sm.CreateEmptyString();
            if (!name.Convert2String(str_name))
                // Exception
                return;

            // Fixed attributes.
            UPInt index = 0;
            const SlotInfo* si = _this.GetObject()->FindFixedSlot(str_name, vm.GetPublicNamespace(), index);
            if (si)
                return result.SetBool(!si->IsDontEnum());

            // Dynamic attributes.
            const Object::DynAttrsType* da = _this.GetObject()->GetDynamicAttrs();
            if (da)
            {
                Object::DynAttrsType::ConstIterator it = da->Find(str_name);
                if (!it.IsEnd())
                    return result.SetBool(!it->First.IsDoNotEnum());
            }

            // Return false if property wasn't found.
        }
        else if (_this.IsNamespace() && argv[0].IsString())
        {
            // Namespace is a primitive type.
            const ASString name = argv[0];
            if (name == "prefix" || name == "uri")
                return result.SetBool(true);
        }

        // Properties of primitive values are not enumerable.
        result.SetBool(false);
//##protect##"instance::Object::AS3propertyIsEnumerable()"
    }
    void Object::hasOwnPropertyProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::hasOwnPropertyProto()"
        // Just delegate a call.
        AS3hasOwnProperty(ti, vm, _this, result, argc, argv);
//##protect##"instance::Object::hasOwnPropertyProto()"
    }
    void Object::propertyIsEnumerableProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::propertyIsEnumerableProto()"
        // Just delegate a call.
        AS3propertyIsEnumerable(ti, vm, _this, result, argc, argv);
//##protect##"instance::Object::propertyIsEnumerableProto()"
    }
    void Object::setPropertyIsEnumerableProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::setPropertyIsEnumerableProto()"
        SF_UNUSED2(ti, result);
        // One argument is required.
        if (argc < 1)
            return vm.ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, vm SF_DEBUG_ARG("Object::setPropertyIsEnumerableProto") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));

        if (_this.IsObject() && !_this.IsNull())
        {
            const Value& name = argv[0];
            bool isEnum = true;

            StringManager& sm = vm.GetStringManager();
            ASString str_name = sm.CreateEmptyString();
            if (!name.Convert2String(str_name))
                // Exception
                return;

            if (argc > 1)
                isEnum = argv[1].Convert2Boolean();

            const DynAttrsType* da = _this.GetObject()->GetDynamicAttrs();
            if (da)
            {
                DynAttrsType::ConstIterator it = da->Find(str_name);

                if (it.IsEnd())
                {
                    return vm.ThrowReferenceError(VM::Error(VM::eWriteSealedError, vm
                        SF_DEBUG_ARG(str_name.ToCStr())
                        SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                        ));
                }
                else
                    it->First.SetDoNotEnum(!isEnum);
            }
        }
        else
        {
            return vm.ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, vm));
            /* DO NOT delete this code.
            vm.ThrowReferenceErrorStrStr(VM::eWriteSealedError 
                SF_DEBUG_ARG(GetNameNode()->pData)
                SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
                );
            */
        }
//##protect##"instance::Object::setPropertyIsEnumerableProto()"
    }
    void Object::isPrototypeOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::isPrototypeOfProto()"
        // Just delegate a call.
        AS3isPrototypeOf(ti, vm, _this, result, argc, argv);
//##protect##"instance::Object::isPrototypeOfProto()"
    }
    void Object::toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::toStringProto()"
        SF_UNUSED3(ti, argc, argv);

        /* DO NOT delete this code.
        // This code will cause recursion.
        if (_this.IsObject())
            result = _this.GetObject()->ToString();
        else
            result = vm.GetStringManager().CreateConstString("[object ") + vm.GetValueTraits(_this).GetClass().GetName() + "]";
        */

        const Traits& vtr = vm.GetValueTraits(_this);
#if 1
        const ClassTraits::Function& ftr = vm.GetClassTraitsFunction();

#if 1
        // This version is slightly faster (3DInventory_AS3.swf).

        ASString name = vm.GetStringManager().GetBuiltin(AS3Builtin_objPrefix);

        // Handle special cases when Tamarin requires object dependent value.
        if (vtr.GetTraitsType() == Traits_Function && vtr.IsInstanceTraits())
        {
            if (&vtr == &ftr.GetThunkTraits())
                name += static_cast<const InstanceTraits::Thunk&>(vtr).GetThunkName(_this);
            else if (&vtr == &ftr.GetThunkFunctionTraits())
                name += static_cast<const InstanceTraits::ThunkFunction&>(vtr).GetThunkName(_this);
            else if (&vtr == &ftr.GetMethodIndTraits())
                name += static_cast<const InstanceTraits::MethodInd&>(vtr).GetMethodIndName(_this);
            else
                // Just a function.
                name += static_cast<const InstanceTraits::Function&>(vtr).GetFunctionName(_this);

        }
        else
            name += vtr.GetName();

        name += vm.GetStringManager().GetBuiltin(AS3Builtin_squareBracketClose);
        result = name;
#else
        // This version is slower.
        StringBuffer buff(vm.GetMemoryHeap());

        buff += "[object ";

        // Handle spacial cases when Tamarin requires object dependent value.
        if (vtr.GetTraitsType() == Traits_Function && vtr.IsInstanceTraits())
        {
            if (&vtr == &ftr.GetThunkTraits())
                buff += static_cast<const InstanceTraits::Thunk&>(vtr).GetName(_this).ToCStr();
            else if (&vtr == &ftr.GetThunkFunctionTraits())
                buff += static_cast<const InstanceTraits::ThunkFunction&>(vtr).GetName(_this).ToCStr();
            else if (&vtr == &ftr.GetMethodIndTraits())
                buff += static_cast<const InstanceTraits::MethodInd&>(vtr).GetName(_this).ToCStr();
            else
                // Just a function.
                buff += static_cast<const InstanceTraits::Function&>(vtr).GetName(_this).ToCStr();

        }
        else
            buff += vtr.GetName().ToCStr();

        buff +=  "]";

        result = vm.GetStringManager().CreateString(buff.ToCStr(), buff.GetSize());
#endif
#else
        // Old version.
        ASString name = vm.GetStringManager().CreateConstString("[object ");

        name += vtr.GetName();
        name +=  "]";
        result = name;
#endif
//##protect##"instance::Object::toStringProto()"
    }
    void Object::toLocaleStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::toLocaleStringProto()"
        // Just delegate a call.
        toStringProto(ti, vm, _this, result, argc, argv);
//##protect##"instance::Object::toLocaleStringProto()"
    }
    void Object::valueOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::Object::valueOfProto()"
        SF_UNUSED4(ti, vm, argc, argv);
        result = _this;
//##protect##"instance::Object::valueOfProto()"
    }

//##protect##"instance$methods"
    Object::Object(VM& vm)
    : Instance(vm)
    {
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 Object::tito[Object::ThunkInfoNum] = {
    //    0, 2, 4, 
    // };
    const TypeInfo* Object::tit[5] = {
        &AS3::fl::BooleanTI, &AS3::fl::ObjectTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Object::ti[Object::ThunkInfoNum] = {
        {&Instances::fl::Object::AS3isPrototypeOf, &Object::tit[0], "isPrototypeOf", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {&Instances::fl::Object::AS3hasOwnProperty, &Object::tit[2], "hasOwnProperty", NS_AS3, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {&Instances::fl::Object::AS3propertyIsEnumerable, &Object::tit[4], "propertyIsEnumerable", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
    };

    Object::Object(VM& vm, const ClassInfo& ci)
    : CTraits(vm, ci)
    {
//##protect##"InstanceTraits::Object::Object()"
//##protect##"InstanceTraits::Object::Object()"

    }

    void Object::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Object&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    // const UInt16 Object::tito[Object::ThunkInfoNum] = {
    //    0, 2, 3, 6, 8, 9, 10, 
    // };
    const TypeInfo* Object::tit[11] = {
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl::ObjectTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::ObjectTI, 
    };
    const Abc::ConstValue Object::dva[1] = {
        {Abc::CONSTANT_True, 0}, 
    };
    const ThunkInfo Object::ti[Object::ThunkInfoNum] = {
        {&Instances::fl::Object::hasOwnPropertyProto, &Object::tit[0], "hasOwnProperty", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {&Instances::fl::Object::propertyIsEnumerableProto, &Object::tit[2], "propertyIsEnumerable", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {&Instances::fl::Object::setPropertyIsEnumerableProto, &Object::tit[3], "setPropertyIsEnumerable", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &Object::dva[0]},
        {&Instances::fl::Object::isPrototypeOfProto, &Object::tit[6], "isPrototypeOf", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {&Instances::fl::Object::toStringProto, &Object::tit[8], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&Instances::fl::Object::toLocaleStringProto, &Object::tit[9], "toLocaleString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&Instances::fl::Object::valueOfProto, &Object::tit[10], "valueOf", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Object::Object(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Object::Object()"
//##protect##"class_::Object::Object()"
    }

    void Object::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void Object::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void Object::Construct(Value& result, unsigned argc, const Value* argv, bool extCall)
    {
        SF_UNUSED(extCall);

        InstanceTraits::Traits& itr = GetClassTraits().GetInstanceTraits();
        if (argc > 0)
        {
            if (argv[0].IsThunk())
            {
                // Thunk is supposed to represent a Function, which is a dynamic object.
                // So, if someone wants to create an object from a Thunk, we need to create
                // a real object because a next operation might be adding of a dynamic property.

                // Make an object instead of a Thunk.
                result = GetVM().GetClassFunction().MakeThunkFunction(argv[0].AsThunk() SF_DEBUG_ARG(itr));
                return;
            }
            else if (!argv[0].IsNullOrUndefined())
            {
                result = argv[0];
                return;
            }
        } 

        itr.MakeObject(result, itr);
    }

    void Object::SetupSlotValues(Object& /*for_obj*/) const
    {
        ; // No slots ...
    }

    void Object::toStringProto(ASString& result)
    {
//         String str = "[class " + String(GetName().ToCStr(), GetName().GetSize()) + "]";
//         result = GetStringManager().CreateConstStringNode(str, str.GetSize(), 0);

        result = GetStringManager().CreateConstString("[class ") + GetTraits().GetName() + "]";
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{

    Object::Object(VM& vm, const ClassInfo& ci)
    : Traits(vm, ci)
    {
//##protect##"ClassTraits::Object::Object()"
//##protect##"ClassTraits::Object::Object()"

    }

    Pickable<Traits> Object::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Object(vm, AS3::fl::ObjectCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::ObjectCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo ObjectTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl::Object::InstanceType),
        0,
        0,
        InstanceTraits::fl::Object::ThunkInfoNum,
        0,
        "Object", "", NULL,
        TypeInfo::None
    };

    const ClassInfo ObjectCI = {
        &ObjectTI,
        ClassTraits::fl::Object::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl::Object::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

