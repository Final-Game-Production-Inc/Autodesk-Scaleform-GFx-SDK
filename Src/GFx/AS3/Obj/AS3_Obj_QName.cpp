//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_QName.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_QName.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl::QName, Instances::fl::QName::mid_localNameGet, ASString> TFunc_Instances_QName_localNameGet;
typedef ThunkFunc0<Instances::fl::QName, Instances::fl::QName::mid_uriGet, Value> TFunc_Instances_QName_uriGet;
typedef ThunkFunc0<Instances::fl::QName, Instances::fl::QName::mid_AS3valueOf, SPtr<Instances::fl::QName> > TFunc_Instances_QName_AS3valueOf;
typedef ThunkFunc0<Instances::fl::QName, Instances::fl::QName::mid_AS3toString, ASString> TFunc_Instances_QName_AS3toString;

template <> const TFunc_Instances_QName_localNameGet::TMethod TFunc_Instances_QName_localNameGet::Method = &Instances::fl::QName::localNameGet;
template <> const TFunc_Instances_QName_uriGet::TMethod TFunc_Instances_QName_uriGet::Method = &Instances::fl::QName::uriGet;
template <> const TFunc_Instances_QName_AS3valueOf::TMethod TFunc_Instances_QName_AS3valueOf::Method = &Instances::fl::QName::AS3valueOf;
template <> const TFunc_Instances_QName_AS3toString::TMethod TFunc_Instances_QName_AS3toString::Method = &Instances::fl::QName::AS3toString;


namespace Instances { namespace fl
{
    QName::QName(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::QName::QName()$data"
    , LocalName(GetStringManager().CreateEmptyString())
#ifdef SF_URI_AS_NAMESPACE
    , Ns(&t.GetVM().GetPublicNamespace())
#else
    , Uri(GetStringManager().GetEmptyStringNode())
#endif
//##protect##"instance::QName::QName()$data"
    {
//##protect##"instance::QName::QName()$code"
#ifndef SF_URI_AS_NAMESPACE
        Uri->AddRef();
#endif
//##protect##"instance::QName::QName()$code"
    }

    void QName::localNameGet(ASString& result)
    {
//##protect##"instance::QName::localNameGet()"
        result = LocalName;
//##protect##"instance::QName::localNameGet()"
    }
    void QName::uriGet(Value& result)
    {
//##protect##"instance::QName::uriGet()"
#ifdef SF_URI_AS_NAMESPACE
        if (Ns.GetPtr())
            result = Ns->GetUri();
        else
            result.SetNull();
#else
        result = Uri;
#endif
//##protect##"instance::QName::uriGet()"
    }
    void QName::AS3valueOf(SPtr<Instances::fl::QName>& result)
    {
//##protect##"instance::QName::AS3valueOf()"
        result = this;
//##protect##"instance::QName::AS3valueOf()"
    }
    void QName::AS3toString(ASString& result)
    {
//##protect##"instance::QName::AS3toString()"
#ifdef SF_URI_AS_NAMESPACE
        if (Ns.GetPtr() == NULL)
            result = "*::" + LocalName;
        else if (Ns->GetUri().IsEmpty())
            result = LocalName;
        else
        {
            result  = Ns->GetUri();
            result += "::";
            result += LocalName;
        }
#else
        if (Uri == NULL)
            result = "*::" + LocalName;
        else if (Uri->Size == 0)
            result = LocalName;
        else
        {
            result.AssignNode(Uri);
            result += "::";
            result += LocalName;
        }
#endif
//##protect##"instance::QName::AS3toString()"
    }
    void QName::toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"instance::QName::toStringProto()"
        SF_UNUSED3(ti, argc, argv);

        if (_this.IsObject() && _this.GetObject() == &vm.GetClassQName().GetPrototype())
            // This method is called on a prototype object.
            result = vm.GetStringManager().GetBuiltin(AS3Builtin_empty_);
        else 
        {
            const Traits& tr = vm.GetValueTraits(_this);
            const BuiltinTraitsType tt = tr.GetTraitsType();

            if (tt != Traits_QName || tr.IsClassTraits())
                return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

            Instances::fl::QName* obj = static_cast<Instances::fl::QName*>(_this.GetObject());
            ASString str = vm.GetStringManager().CreateEmptyString();

            obj->AS3toString(str);

            // There should be no harm if we do not check ro exception here.
            result = str;
        }
//##protect##"instance::QName::toStringProto()"
    }

//##protect##"instance$methods"
#ifdef SF_URI_AS_NAMESPACE
    QName::QName(InstanceTraits::Traits& t, const ASString& n, Namespace* ns)
#else
    QName::QName(InstanceTraits::Traits& t, const ASString& n, const ASString& u)
#endif
    : Instances::fl::Object(t)
    , LocalName(n)
#ifdef SF_URI_AS_NAMESPACE
    , Ns(ns)
#else
    , Uri(u.GetNode())
#endif
    {
#ifndef SF_URI_AS_NAMESPACE
        if (Uri)
            Uri->AddRef();
#endif
    }

#ifdef SF_URI_AS_NAMESPACE
    void QName::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC(prcc, Ns, op SF_DEBUG_ARG(*this));
    }
#else
    QName::~QName()
    {
        if (Uri)
            Uri->Release();
    }
#endif

    void QName::AS3Constructor(unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();

        // If the parameter is undefined or unspecified, a new QName object 
        // is created with the localName property set to the empty string.
        // 
        if (argc == 0)
            LocalName = sm.CreateEmptyString();
        else if (argc == 1)
        {
            // If the parameter is not a QName object, the parameter is 
            // converted to a string and assigned to the localName property 
            // of the new QName instance.
            // Situation when parameter is not a QName object is handled by Construct().
            if (!argv[0].IsUndefined())
            {
                if (!argv[0].Convert2String(LocalName))
                    return;
            }

            if (LocalName == "*")
            {
#ifdef SF_URI_AS_NAMESPACE
                Ns = NULL;
#else
                if (Uri)
                    Uri->Release();
                Uri = NULL;
#endif
            }
        }
        else
        {
#ifdef SF_URI_AS_NAMESPACE
            Ns = NULL;
#else
            if (Uri)
            {
                Uri->Release();
                Uri = NULL;
            }
#endif

            if (!argv[0].IsNull())
            {
                // Namespace is a final class.
                if (IsNamespaceObject(argv[0]))
                {
#ifdef SF_URI_AS_NAMESPACE
                    Ns = argv[0].GetNamespace();
#else
                    Instances::fl::Namespace* ns = argv[0].GetNamespace();

                    Uri = ns->GetUri().GetNode();

                    if (Uri)
                        Uri->AddRef();
#endif
                }
                else if (IsQNameObject(argv[0]))
                {
                    QName* other = static_cast<QName*>(argv[0].GetObject());
#ifdef SF_URI_AS_NAMESPACE
                    Ns = other->Ns;
#else
                    Uri = other->Uri;
                    if (Uri)
                        Uri->AddRef();
#endif
                }
                else
                {
                    ASString uri = sm.CreateEmptyString();
                    if (argv[0].Convert2String(uri))
                    {
#ifdef SF_URI_AS_NAMESPACE
                        InstanceTraits::fl::Namespace& itr_ns = static_cast<InstanceTraits::fl::Namespace&>(GetVM().GetITraitsNamespace());
                        Ns = itr_ns.MakeInternedInstance(Abc::NS_Public, uri);
#else
                        Uri = uri.GetNode();
                        Uri->AddRef();
#endif
                    }
                }
            }

            // Set LocalName.
            if (IsNamespaceObject(argv[1]))
            {
                Instances::fl::Namespace* ns = argv[0].GetNamespace();

                ns->GetPrefix().Convert2String(LocalName).DoNotCheck();
            }
            else if (IsQNameObject(argv[1]))
            {
                QName* other = static_cast<QName*>(argv[1].GetObject());
                LocalName = other->LocalName;
            }
            else if (argv[1].IsUndefined())
                LocalName = sm.CreateEmptyString();
            else
                argv[1].Convert2String(LocalName).DoNotCheck();
        }
    }

    void QName::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();

        switch (ind.Get())
        {
        case 1:
            name = sm.CreateConstString("uri");
            break;
        case 2:
            name = sm.CreateConstString("localName");
            break;
        default:
            break;
        }
    }

    GlobalSlotIndex QName::GetNextDynPropIndex(GlobalSlotIndex ind) const
    {
        switch (ind.Get())
        {
        case 0:
        case 1:
            return ++ind;
        default:
            break;
        }

        return GlobalSlotIndex(0);
    }

    void QName::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
    {
        switch (ind.Get())
        {
        case 1:
#ifdef SF_URI_AS_NAMESPACE
            if (Ns.GetPtr())
                value = Ns->GetUri();
            else
                value.SetNull();
#else
            value = GetUri();
#endif
            break;
        case 2:
            value = GetLocalName();
            break;
        default:
            break;
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 QName::tito[QName::ThunkInfoNum] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* QName::tit[4] = {
        &AS3::fl::StringTI, 
        NULL, 
        &AS3::fl::QNameTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo QName::ti[QName::ThunkInfoNum] = {
        {TFunc_Instances_QName_localNameGet::Func, &QName::tit[0], "localName", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_QName_uriGet::Func, &QName::tit[1], "uri", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_QName_AS3valueOf::Func, &QName::tit[2], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_QName_AS3toString::Func, &QName::tit[3], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    QName::QName(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::QName::QName()"
        SetTraitsType(Traits_QName);
//##protect##"InstanceTraits::QName::QName()"

    }

    void QName::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<QName&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    // const UInt16 QName::tito[QName::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* QName::tit[1] = {
        &AS3::fl::StringTI, 
    };
    const ThunkInfo QName::ti[QName::ThunkInfoNum] = {
        {&Instances::fl::QName::toStringProto, &QName::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    QName::QName(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::QName::QName()"
//##protect##"class_::QName::QName()"
    }

    void QName::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void QName::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
    {
        if (argc == 1)
        {
            const Traits& tr = GetVM().GetValueTraits(argv[0]);
            const BuiltinTraitsType tt = tr.GetTraitsType();

            if (tt == Traits_QName && tr.IsInstanceTraits())
            {
                // We shouldn't create new object in this case.
                _this = argv[0];
                return;
            }
        }

        Class::Construct(_this, argc, argv, extCall);
    }

    void QName::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{

    QName::QName(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::QName::QName()"
        SetTraitsType(Traits_QName);
//##protect##"ClassTraits::QName::QName()"

    }

    Pickable<Traits> QName::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) QName(vm, AS3::fl::QNameCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::QNameCI));
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
    const TypeInfo QNameTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::QName::InstanceType),
        0,
        0,
        InstanceTraits::fl::QName::ThunkInfoNum,
        0,
        "QName", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo QNameCI = {
        &QNameTI,
        ClassTraits::fl::QName::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl::QName::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

