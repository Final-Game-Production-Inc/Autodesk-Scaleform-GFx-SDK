//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Proxy.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_Proxy.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_VTable.h"
#include "../AS3_Obj_Array.h"
#include "../AS3_Obj_QName.h"
#ifdef GFX_ENABLE_XML
    #include "../AS3_Obj_XMLList.h"
#endif
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc2<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxycallProperty, Value, unsigned, const Value*> TFunc_Instances_Proxy_flash_proxycallProperty;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxydeleteProperty, bool, const Value&> TFunc_Instances_Proxy_flash_proxydeleteProperty;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxygetDescendants, Value, const Value&> TFunc_Instances_Proxy_flash_proxygetDescendants;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxygetProperty, Value, const Value&> TFunc_Instances_Proxy_flash_proxygetProperty;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxyhasProperty, bool, const Value&> TFunc_Instances_Proxy_flash_proxyhasProperty;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxyisAttribute, bool, const Value&> TFunc_Instances_Proxy_flash_proxyisAttribute;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxynextName, ASString, SInt32> TFunc_Instances_Proxy_flash_proxynextName;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxynextNameIndex, SInt32, SInt32> TFunc_Instances_Proxy_flash_proxynextNameIndex;
typedef ThunkFunc1<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxynextValue, Value, SInt32> TFunc_Instances_Proxy_flash_proxynextValue;
typedef ThunkFunc2<Instances::fl_utils::Proxy, Instances::fl_utils::Proxy::mid_flash_proxysetProperty, const Value, const Value&, const Value&> TFunc_Instances_Proxy_flash_proxysetProperty;

template <> const TFunc_Instances_Proxy_flash_proxycallProperty::TMethod TFunc_Instances_Proxy_flash_proxycallProperty::Method = &Instances::fl_utils::Proxy::flash_proxycallProperty;
template <> const TFunc_Instances_Proxy_flash_proxydeleteProperty::TMethod TFunc_Instances_Proxy_flash_proxydeleteProperty::Method = &Instances::fl_utils::Proxy::flash_proxydeleteProperty;
template <> const TFunc_Instances_Proxy_flash_proxygetDescendants::TMethod TFunc_Instances_Proxy_flash_proxygetDescendants::Method = &Instances::fl_utils::Proxy::flash_proxygetDescendants;
template <> const TFunc_Instances_Proxy_flash_proxygetProperty::TMethod TFunc_Instances_Proxy_flash_proxygetProperty::Method = &Instances::fl_utils::Proxy::flash_proxygetProperty;
template <> const TFunc_Instances_Proxy_flash_proxyhasProperty::TMethod TFunc_Instances_Proxy_flash_proxyhasProperty::Method = &Instances::fl_utils::Proxy::flash_proxyhasProperty;
template <> const TFunc_Instances_Proxy_flash_proxyisAttribute::TMethod TFunc_Instances_Proxy_flash_proxyisAttribute::Method = &Instances::fl_utils::Proxy::flash_proxyisAttribute;
template <> const TFunc_Instances_Proxy_flash_proxynextName::TMethod TFunc_Instances_Proxy_flash_proxynextName::Method = &Instances::fl_utils::Proxy::flash_proxynextName;
template <> const TFunc_Instances_Proxy_flash_proxynextNameIndex::TMethod TFunc_Instances_Proxy_flash_proxynextNameIndex::Method = &Instances::fl_utils::Proxy::flash_proxynextNameIndex;
template <> const TFunc_Instances_Proxy_flash_proxynextValue::TMethod TFunc_Instances_Proxy_flash_proxynextValue::Method = &Instances::fl_utils::Proxy::flash_proxynextValue;
template <> const TFunc_Instances_Proxy_flash_proxysetProperty::TMethod TFunc_Instances_Proxy_flash_proxysetProperty::Method = &Instances::fl_utils::Proxy::flash_proxysetProperty;

namespace Instances { namespace fl_utils
{
    Proxy::Proxy(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Proxy::Proxy()$data"
    , callPropertyInd(-1)
    , deletePropertyInd(-1)
    , getDescendantsInd(-1)
    , getPropertyInd(-1)
    , hasPropertyInd(-1)
    , isAttributeInd(-1)
    , nextNameInd(-1)
    , nextNameIndexInd(-1)
    , nextValueInd(-1)
    , setPropertyInd(-1)
//##protect##"instance::Proxy::Proxy()$data"
    {
//##protect##"instance::Proxy::Proxy()$code"
//##protect##"instance::Proxy::Proxy()$code"
    }

    void Proxy::flash_proxycallProperty(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Proxy::flash_proxycallProperty()"
        SF_UNUSED3(result, argc, argv);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyCallPropertyError, vm));
//##protect##"instance::Proxy::flash_proxycallProperty()"
    }
    void Proxy::flash_proxydeleteProperty(bool& result, const Value& name)
    {
//##protect##"instance::Proxy::flash_proxydeleteProperty()"
        SF_UNUSED2(result, name);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyDeletePropertyError, vm));
//##protect##"instance::Proxy::flash_proxydeleteProperty()"
    }
    void Proxy::flash_proxygetDescendants(Value& result, const Value& name)
    {
//##protect##"instance::Proxy::flash_proxygetDescendants()"
        SF_UNUSED2(result, name);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyGetDescendantsError, vm));
//##protect##"instance::Proxy::flash_proxygetDescendants()"
    }
    void Proxy::flash_proxygetProperty(Value& result, const Value& name)
    {
//##protect##"instance::Proxy::flash_proxygetProperty()"
        SF_UNUSED2(result, name);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyGetPropertyError, vm));
//##protect##"instance::Proxy::flash_proxygetProperty()"
    }
    void Proxy::flash_proxyhasProperty(bool& result, const Value& name)
    {
//##protect##"instance::Proxy::flash_proxyhasProperty()"
        SF_UNUSED2(result, name);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyHasPropertyError, vm));
//##protect##"instance::Proxy::flash_proxyhasProperty()"
    }
    void Proxy::flash_proxyisAttribute(bool& result, const Value& name)
    {
//##protect##"instance::Proxy::flash_proxyisAttribute()"
        SF_UNUSED2(result, name);
        WARN_NOT_IMPLEMENTED("instance::Proxy::flash_proxyisAttribute()");
//##protect##"instance::Proxy::flash_proxyisAttribute()"
    }
    void Proxy::flash_proxynextName(ASString& result, SInt32 index)
    {
//##protect##"instance::Proxy::flash_proxynextName()"
        SF_UNUSED2(result, index);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyNextNameError, vm));
//##protect##"instance::Proxy::flash_proxynextName()"
    }
    void Proxy::flash_proxynextNameIndex(SInt32& result, SInt32 index)
    {
//##protect##"instance::Proxy::flash_proxynextNameIndex()"
        SF_UNUSED2(result, index);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyNextNameIndexError, vm));
//##protect##"instance::Proxy::flash_proxynextNameIndex()"
    }
    void Proxy::flash_proxynextValue(Value& result, SInt32 index)
    {
//##protect##"instance::Proxy::flash_proxynextValue()"
        SF_UNUSED2(result, index);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxyNextValueError, vm));
//##protect##"instance::Proxy::flash_proxynextValue()"
    }
    void Proxy::flash_proxysetProperty(const Value& result, const Value& name, const Value& value)
    {
//##protect##"instance::Proxy::flash_proxysetProperty()"
        SF_UNUSED3(result, name, value);
        VM& vm = GetVM();
        vm.ThrowError(VM::Error(VM::eProxySetPropertyError, vm));
//##protect##"instance::Proxy::flash_proxysetProperty()"
    }

//##protect##"instance$methods"
    CheckResult Proxy::MakeName(const Multiname& prop_name, Value& name) const
    {
        VM& vm = GetVM();

        if (prop_name.IsQName())
        {
            StringManager& sm = GetStringManager();
            const Value& value_name = prop_name.GetName();
            ASString str_name = sm.CreateEmptyString();

            if (!value_name.Convert2String(str_name))
                // Exception.
                return false;

            InstanceTraits::fl::QName& qn_it = static_cast<InstanceTraits::fl::QName&>(vm.GetClassTraitsQName().GetInstanceTraits());
            name = qn_it.MakeInstance(qn_it, str_name, &prop_name.GetNamespace());
        }
        else
            name = prop_name.GetName();

        return !vm.IsException();
    }

    SInt32 Proxy::GetMethodInd(const char* name) const
    {
        VM& vm = GetVM();
        StringManager& sm = GetStringManager();
        Instances::fl::Namespace& ns = vm.GetProxyNamespace();
        const Traits& tr = GetTraits();
        const SlotInfo* si = tr.FindSlotInfo(sm.CreateConstString(name), ns);
        SF_ASSERT(si);
        return si->GetValueInd();
    }

    CheckResult Proxy::SetProperty(const Multiname& prop_name, const Value& value)
    {
        VM& vm = GetVM();
        UPInt index;
        const SlotInfo* si = FindFixedSlot(prop_name, index);

        if (si)
            return si->SetSlotValue(vm, value, this);

        // No fixed property ...
        if (setPropertyInd < 0)
            setPropertyInd = GetMethodInd("setProperty");

        const Value funct = GetVT().GetValue(AbsoluteIndex(setPropertyInd));

        Value result;
        Value args[2] = {Value(), value};

        if (!MakeName(prop_name, args[0]))
            return false;

        vm.Execute(
            funct,
            Value(this),
            result,
            2,
            args
            );

        return !vm.IsException();
    }

    CheckResult Proxy::GetProperty(const Multiname& prop_name, Value& value)
    {
        VM& vm = GetVM();
        UPInt index;
        const SlotInfo* si = FindFixedSlot(prop_name, index);

        if (si)
        {
            Value v;
            if (si->GetSlotValueUnsafe(v, this))
            {
                Alg::Swap(v, value);
                return true;
            }
        }

        // No fixed property ...
        if (getPropertyInd < 0)
            getPropertyInd = GetMethodInd("getProperty");

        const Value funct = GetVT().GetValue(AbsoluteIndex(getPropertyInd));
        Value name;

        if (!MakeName(prop_name, name))
            return false;

        vm.Execute(
            funct,
            Value(this),
            value,
            1,
            &name
            );

        return !vm.IsException();
    }

    CheckResult Proxy::DeleteProperty(const Multiname& prop_name)
    {
        bool ok = true;

        if (!DeleteDynamicSlotValuePair(prop_name))
        {
            VM& vm = GetVM();

            if (deletePropertyInd < 0)
                deletePropertyInd = GetMethodInd("deleteProperty");

            const Value funct = GetVT().GetValue(AbsoluteIndex(deletePropertyInd));
            Value result;
            Value name;

            if (!MakeName(prop_name, name))
                return false;

            vm.Execute(
                funct,
                Value(this),
                result,
                1,
                &name
                );

            if (vm.IsException())
                return false;

            // Doesn't throw exceptions.
            ok = result.Convert2Boolean();
        }

        return ok;
    }

    CheckResult Proxy::ExecutePropertyUnsafe(const Multiname& prop_name, Value& result, unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();
        const Value _this(this);
        UPInt index;
        const SlotInfo* si = FindFixedSlot(prop_name, index);

        if (si)
        {
            Value funct;
            if (si->GetSlotValueUnsafe(funct, this))
            {
                if (funct.IsNullOrUndefined())
                    // A TypeError is thrown if the property specified by the multiname is null or undefined.
                    vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm SF_DEBUG_ARG(prop_name.GetName())));
                else
                    vm.ExecuteUnsafe(funct, _this, result, argc, argv);
            }
        } else
        {
            // No fixed property ...
            // Instead of throwing an exception we call method flash_proxy::callProperty().
            if (callPropertyInd < 0)
                callPropertyInd = GetMethodInd("callProperty");

            const Value funct = GetVT().GetValue(AbsoluteIndex(callPropertyInd));

            Pickable<Instances::fl::Array> array = vm.MakeArray();
            array->Append(argc, argv);

            Value args[2] = {Value(), array};

            if (!MakeName(prop_name, args[0]))
                return false;

            vm.Execute(
                funct,
                _this,
                result,
                2,
                args
                );
        }

        return !vm.IsException();
    }

    bool Proxy::HasProperty(const Multiname& prop_name, bool check_prototype)
    {
        SF_UNUSED1(check_prototype);
        UPInt index;
        const SlotInfo* si = FindFixedSlot(prop_name, index);

        if (si)
            return true;

        bool result = false;
        VM& vm = GetVM();

        if (hasPropertyInd < 0)
            hasPropertyInd = GetMethodInd("hasProperty");

        const Value funct = GetVT().GetValue(AbsoluteIndex(hasPropertyInd));
        Value r;
        Value name;

        if (!MakeName(prop_name, name))
            return false;

        vm.Execute(
            funct,
            Value(const_cast<Proxy*>(this)),
            r,
            1,
            &name
            );

        result = r.Convert2Boolean();

        return result;
    }

    void Proxy::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
#if 0
        // Code from Object::GetNextPropertyName().
        SF_ASSERT(ind.IsValid());

        const DynAttrsType::ConstIterator bit = DynAttrs.Begin();
        DynAttrsType::ConstIterator it(bit.GetContainer(), AbsoluteIndex(ind).Get());

        name = it->First.GetName();
#endif

        VM& vm = GetVM();

        if (nextNameInd < 0)
            nextNameInd = GetMethodInd("nextName");

        const Value funct = GetVT().GetValue(AbsoluteIndex(nextNameInd));
        const Value args[1] = {Value(ind.Get())};

        vm.Execute(
            funct,
            Value(const_cast<Proxy*>(this)),
            name,
            1,
            args
            );
    }

    GlobalSlotIndex Proxy::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
#if 0
        // Code from Object::GetNextDynPropIndex().
        const DynAttrsType::ConstIterator bit = DynAttrs.Begin();
        DynAttrsType::ConstIterator it(bit.GetContainer(), (ind.IsValid() ? static_cast<SPInt>(AbsoluteIndex(ind).Get()) : -1));

        if (it.IsEnd())
            return GlobalSlotIndex(0);

        ++it;
        // Skip "DontEnum" properties.
        while (!it.IsEnd() && it->First.IsDoNotEnum())
            ++it;

        if (it.IsEnd())
            return GlobalSlotIndex(0);
        else
            return GlobalSlotIndex(AbsoluteIndex(it.GetIndex()));
#endif
        
        GlobalSlotIndex gind(0);
        VM& vm = GetVM();

        if (nextNameIndexInd < 0)
            nextNameIndexInd = GetMethodInd("nextNameIndex");

        const Value funct = GetVT().GetValue(AbsoluteIndex(nextNameIndexInd));
        Value next;
        const Value args[1] = {Value(ind.Get())};

        vm.Execute(
            funct,
            Value(const_cast<Proxy*>(this)),
            next,
            1,
            args
            );

        if (!vm.IsException())
        {
            UInt32 next_ind;
            if (next.Convert2UInt32(next_ind))
                gind = GlobalSlotIndex(next_ind);
        }

        return gind;
    }

    void Proxy::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
    {
#if 0
        // Code from Object::GetNextPropertyValue().
        if (!ind.IsValid())
            return;

        GetDynamicProperty(AbsoluteIndex(ind), value);
#endif

        VM& vm = GetVM();

        if (nextValueInd < 0)
            nextValueInd = GetMethodInd("nextValue");

        const Value funct = GetVT().GetValue(AbsoluteIndex(nextValueInd));
        const Value args[1] = {Value(ind.Get())};

        vm.Execute(
            funct,
            Value(this),
            value,
            1,
            args
            );
    }

    void Proxy::GetDescendants(fl::XMLList& list, const Multiname& prop_name)
    {
        VM& vm = GetVM();

#ifdef GFX_ENABLE_XML
        const XMLSupport& xmls = vm.GetXMLSupport();
        if (!xmls.IsEnabled())
            return vm.ThrowVerifyError(VM::Error(VM::eNotImplementedError, vm SF_DEBUG_ARG("Proxy::GetDescendants")));

        if (getDescendantsInd < 0)
            getDescendantsInd = GetMethodInd("getDescendants");

        const Value funct = GetVT().GetValue(AbsoluteIndex(getDescendantsInd));
        Value result;
        Value name;

        if (!MakeName(prop_name, name))
            return;

        vm.Execute(
            funct,
            Value(this),
            result,
            1,
            &name
            );

        if (vm.IsException())
            // Oops. Exception.
            return;

        SPtr<fl::XMLList> new_list;
        if (!vm.ConstructBuiltinObject(new_list, "XMLList", 1, &result))
            return;

        list.Apppend(*new_list);
#else
        if (&list == &list) {;}
        SF_UNUSED1(prop_name);
        vm.ThrowVerifyError(VM::Error(VM::eNotImplementedError, vm SF_DEBUG_ARG("Proxy::GetDescendants")));
#endif
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    // const UInt16 Proxy::tito[Proxy::ThunkInfoNum] = {
    //    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 
    // };
    const TypeInfo* Proxy::tit[21] = {
        NULL, NULL, 
        &AS3::fl::BooleanTI, NULL, 
        NULL, NULL, 
        NULL, NULL, 
        &AS3::fl::BooleanTI, NULL, 
        &AS3::fl::BooleanTI, NULL, 
        &AS3::fl::StringTI, &AS3::fl::int_TI, 
        &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        NULL, NULL, NULL, 
    };
    const ThunkInfo Proxy::ti[Proxy::ThunkInfoNum] = {
        {TFunc_Instances_Proxy_flash_proxycallProperty::Func, &Proxy::tit[0], "callProperty", NS_flash_proxy, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxydeleteProperty::Func, &Proxy::tit[2], "deleteProperty", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxygetDescendants::Func, &Proxy::tit[4], "getDescendants", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxygetProperty::Func, &Proxy::tit[6], "getProperty", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxyhasProperty::Func, &Proxy::tit[8], "hasProperty", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxyisAttribute::Func, &Proxy::tit[10], "isAttribute", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxynextName::Func, &Proxy::tit[12], "nextName", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxynextNameIndex::Func, &Proxy::tit[14], "nextNameIndex", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxynextValue::Func, &Proxy::tit[16], "nextValue", NS_flash_proxy, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Proxy_flash_proxysetProperty::Func, &Proxy::tit[18], "setProperty", NS_flash_proxy, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

    Proxy::Proxy(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Proxy::Proxy()"
//##protect##"InstanceTraits::Proxy::Proxy()"

    }

    void Proxy::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Proxy&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    Proxy::Proxy(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Proxy::Proxy()"
//##protect##"ClassTraits::Proxy::Proxy()"

    }

    Pickable<Traits> Proxy::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Proxy(vm, AS3::fl_utils::ProxyCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::ProxyCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_utils
{
    const TypeInfo ProxyTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_utils::Proxy::InstanceType),
        0,
        0,
        InstanceTraits::fl_utils::Proxy::ThunkInfoNum,
        0,
        "Proxy", "flash.utils", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ProxyCI = {
        &ProxyTI,
        ClassTraits::fl_utils::Proxy::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_utils::Proxy::ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

