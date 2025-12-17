//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Namespace.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Namespace.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_QName.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

Value::Value(Instances::fl::Namespace* v)
: Flags(kNamespace)
, value(v)
{
    if (v)
        v->AddRef();
}

void Value::Assign(Instances::fl::Namespace* v)
{
    Release();

    SetKind(kNamespace);
    value = v;
    if (v)
        v->AddRef();
}

void Value::AssignUnsafe(Instances::fl::Namespace* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kNamespace);
    value = v;
    if (v)
        v->AddRef();
}

void Value::Pick(Instances::fl::Namespace* v)
{
    Release();

    SetKind(kNamespace);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Instances::fl::Namespace* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kNamespace);
    value = v;
    // No AddRef() is necessary.
}


// ***** NamespaceInstanceFactory implementation

// Alternative non-instance hash lookup key used for Instances::fl::Namespace hash.
struct NamespaceKey
{      
    Abc::NamespaceKind Kind;
    ASString           URI;
    NamespaceKey(const NamespaceKey &src) : Kind(src.Kind), URI(src.URI) { }
    NamespaceKey(Abc::NamespaceKind kind, const ASString& uri) : Kind(kind), URI(uri) { }
};

bool operator == (Instances::fl::Namespace* ns, const NamespaceKey &key)
{
    return (ns->GetKind() == key.Kind) && (ns->GetUri() == key.URI);
}

// NamespaceInstanceFactory is responsible for creating interned Instances::fl::Namespace objects.
// Factory maintains a hash-table of weak references, which are removed when Namespace objects
// are garbage collected (i.e. their destructor is called).
// NamespaceInstanceFactory uses non-GCs reference counting on purpose, to avoid being
// a part of collection cycle.

// By SS:
// There is one problem with the NamespaceInstanceFactory class. If a namespace is created by calling
// to MakeNamespace() and not stored anywhere (and as a result not visited by a garbage collector)
// it will prevent garbage collector from cleaning up of namespace object and cause memory
// leaks.
class NamespaceInstanceFactory : public RefCountBase<NamespaceInstanceFactory, StatMV_VM_ITraits_Mem>
{
    friend class InstanceTraits::fl::Namespace;

public:
    NamespaceInstanceFactory(InstanceTraits::fl::Namespace * nsInstanceTraits)
        : pNamespaceInstanceTraits(nsInstanceTraits)
    { }    
    ~NamespaceInstanceFactory()
    {
    }

    Pickable<Instances::fl::Namespace> MakeNamespace(Abc::NamespaceKind kind, const ASString& uri,
        const Value& prefix)
    {
        NamespaceKey key(kind, uri);
        Instances::fl::Namespace*   ns = 0;

        if (!NamespaceSet.GetAlt(key, &ns))
        {
            // MakeNamespace call not allowed after Traits are destroyed.
            SF_ASSERT(pNamespaceInstanceTraits); 

            Pickable<Instances::fl::Namespace> result = pNamespaceInstanceTraits->MakeInstance(kind, uri, prefix);
            NamespaceSet.Add(result.GetPtr());
            return result;
        }

        return Pickable<Instances::fl::Namespace>(ns, PickValue);
    }

    Instances::fl::Namespace& GetNamespace(Abc::NamespaceKind kind, const ASString& uri,
        const Value& prefix)
    {
        SF_UNUSED(prefix);
        NamespaceKey key(kind, uri);
        Instances::fl::Namespace*   ns = 0;

        if (!NamespaceSet.GetAlt(key, &ns))
        {
            SF_ASSERT(false);
            if (pNamespaceInstanceTraits)
                return pNamespaceInstanceTraits->GetVM().GetPublicNamespace();
        }

        return *ns;
    }

    void AddNamespace(Instances::fl::Namespace& ns)
    {
        NamespaceSet.Add(&ns);
    }
    void RemoveNamespace(Instances::fl::Namespace& ns)
    {
        NamespaceSet.Remove(&ns);
    }

private:
    // Debugging-only.
    void onNamespaceTraitsDestroy() { pNamespaceInstanceTraits = 0; }

private:
    class NamespaceHashFunc
    {
    public:        
        // Hash code is stored right in the node
        UPInt  operator() (const Instances::fl::Namespace*const& ns) const { return ns->GetHash(); }
        UPInt  operator() (const NamespaceKey &str) const
        {
            return Instances::fl::Namespace::CalcHash(str.Kind, str.URI);
        }
        // Hash update - unused.
        static UPInt    GetCachedHash(const Instances::fl::Namespace*& ns)            { return ns->GetHash(); }
        static void     SetCachedHash(Instances::fl::Namespace*& ns, UPInt hashValue) { SF_UNUSED2(ns, hashValue); }
        static Instances::fl::Namespace*&       GetValue(Instances::fl::Namespace*& ns)              { return ns; }
        static const Instances::fl::Namespace*& GetValue(const Instances::fl::Namespace*& ns)        { return ns; }
    };

    typedef HashSetUncachedLH<Instances::fl::Namespace*, NamespaceHashFunc, 
        NamespaceHashFunc, Stat_Default_Mem>  NamespacePtrHashType;

    NamespacePtrHashType            NamespaceSet;
    InstanceTraits::fl::Namespace*  pNamespaceInstanceTraits;
};

///////////////////////////////////////////////////////////////////////////////
namespace Instances { namespace fl
{
    Namespace::Namespace(VM& vm, Abc::NamespaceKind kind, const char* uri)
    : GASRefCountBase(&vm.GetGC())
    , Kind(kind)
    , IsInterface(false)
    , VMRef(vm)
    , Uri(vm.GetStringManager().CreateConstString(uri))
    {
    }

    Namespace::Namespace(
        VM& vm,
        NamespaceInstanceFactory* factory,
        Abc::NamespaceKind kind,
        const ASString& uri,
        const Value& prefix
        )
    : GASRefCountBase(&vm.GetGC())
    , Kind(kind)
    , IsInterface(false)
    , VMRef(vm)
    , Uri(uri)
    , pFactory(factory)
    , Prefix(prefix)
    {
    }

    Namespace::~Namespace()
    {
        // Remove weak reference to us from factory, if we were added there.
        if (pFactory)
            pFactory->RemoveNamespace(*this);
    }

    void Namespace::SetUri(const Value& value)
    {
        VM& vm = GetVM();

        const Traits& tr = vm.GetValueTraits(value);
        if (tr.GetTraitsType() == Traits_QName && tr.IsInstanceTraits())
        {
            // If the value is a QName object, the uri property is set to the uri property of the QName object.
            if (value.GetObject())
            {
                Instances::fl::QName& qname = static_cast<Instances::fl::QName&>(*value.GetObject());

#ifdef SF_URI_AS_NAMESPACE
                if (qname.GetNamespace() == NULL)
                    SetUri(vm.GetStringManager().CreateConstString("*"));
                else
                    SetUri(qname.GetNamespace()->GetUri());
#else
                SetUri(qname.GetUri());
#endif
            }
        } else
        {
            // Otherwise, the uriValue parameter is converted to a string and assigned to the uri property.
            value.Convert2String(Uri).DoNotCheck();
        }
    }

    void Namespace::AS3Constructor(unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();

        // If no value is passed, the prefix and uri properties are set to an empty string.
        switch(argc)
        {
        case 0:
            SetPrefix(vm.GetStringManager().CreateEmptyString());
            break;
        case 1:
            {
                //SetPrefix(vm.GetStringManager().CreateEmptyString());
                //SetPrefix(Value::GetUndefined());

                const Value& v = argv[0];
                if (v.IsNamespace())
                    // If the value is a Namespace object, a copy of the object is created.
                    *this = v.AsNamespace();
                else if (IsQNameObject(v))
                {
                    const Instances::fl::QName* qname = static_cast<const Instances::fl::QName*>(v.GetObject());

#ifdef SF_URI_AS_NAMESPACE
                    if (qname->GetNamespace() == NULL)
                        SetUri(vm.GetStringManager().CreateConstString("*"));
                    else
                        SetUri(qname->GetNamespace()->GetUri());
#else
                    SetUri(qname->GetUri());
#endif
                }
                else
                {
                    SetUri(v);

                    if (v.IsString() && v.AsString().IsEmpty())
                        SetPrefix(v);
                    else
                        SetPrefix(Value::GetUndefined());
                }
            }
            break;
        default:
            // Two and more arguments ...
            {
                const Value& vp = argv[0];
                const Value& vu = argv[1];
                StringManager& sm = vm.GetStringManager();
                ASString p = sm.CreateEmptyString();
                ASString u = sm.CreateEmptyString();

                if (!vp.Convert2String(p))
                    // Exception.
                    return;

                if (!vu.Convert2String(u))
                    // Exception.
                    return;

                if (!p.IsEmpty() && u.IsEmpty())
                    return vm.ThrowTypeError(VM::Error(VM::eXMLNamespaceWithPrefixAndNoURI, vm SF_DEBUG_ARG(p)));

                // First argument prefixValue ...
                SetPrefix(vp);

                // Second argument uriValue ...
                SetUri(vu);
            }
            break;
        }
    }

    Namespace& Namespace::operator =(const Namespace& other)
    {
        if (this != &other)
        {
            Prefix = other.Prefix;
            Uri = other.Uri;
            Kind = other.Kind;
            pFactory = other.pFactory;
        }

        return *this;
    }

    bool Namespace::IsVMPublic() const
    {
        return (this == &GetVM().GetPublicNamespace());
    }

    void Namespace::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        AS3::ForEachChild_GC(prcc, Prefix, op SF_DEBUG_ARG(*this));
#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
        AS3::ForEachChild_GC<AS3::Traits, Mem_Stat>(prcc, pOwner, op SF_DEBUG_ARG(*this));
#endif
    }

    void Namespace::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
    {
        VM& vm = GetVM();

        switch (ind.Get())
        {
        case 1:
            name = vm.GetStringManager().CreateConstString("uri");
            break;
        case 2:
            name = vm.GetStringManager().CreateConstString("prefix");
        default:
            break;
        }
    }

    void Namespace::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
    {
        switch (ind.Get())
        {
        case 1:
            value = GetUri();
            break;
        case 2:
            value = GetPrefix();
        default:
            break;
        }
    }

    GlobalSlotIndex Namespace::GetNextPropIndex(GlobalSlotIndex ind) const
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
}} // namespace Instances { namespace fl

//##protect##"methods"

namespace InstanceTraits { namespace fl
{
    // const UInt16 Namespace::tito[Namespace::ThunkInfoNum] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* Namespace::tit[4] = {
        NULL, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo Namespace::ti[Namespace::ThunkInfoNum] = {
        {&InstanceTraits::fl::Namespace::prefixGet, &Namespace::tit[0], "prefix", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Namespace::uriGet, &Namespace::tit[1], "uri", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Namespace::AS3valueOf, &Namespace::tit[2], "valueOf", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Namespace::AS3toString, &Namespace::tit[3], "toString", NS_AS3, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Namespace::Namespace(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Namespace::Namespace()"
        SetTraitsType(Traits_Namespace);
        pNamespaceFactory = *SF_HEAP_NEW(vm.GetMemoryHeap()) NamespaceInstanceFactory(this);        
//##protect##"InstanceTraits::Namespace::Namespace()"

    }

    void Namespace::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t); SF_ASSERT(false);
    }

    void Namespace::prefixGet(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::prefixGet()"
        SF_UNUSED3(cti, argc, argv);

        // This method is not generic.
        if (!_this.IsNamespace())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this.AsNamespace().GetPrefix();
//##protect##"InstanceTraits::prefixGet()"
    }
    void Namespace::uriGet(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::uriGet()"
        SF_UNUSED3(cti, argc, argv);

        // This method is not generic.
        if (!_this.IsNamespace())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this.AsNamespace().GetUri();
//##protect##"InstanceTraits::uriGet()"
    }
    void Namespace::AS3valueOf(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3valueOf()"
        SF_UNUSED3(cti, argc, argv);

        // This method is not generic.
        if (!_this.IsNamespace())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this.AsNamespace().GetUri();
//##protect##"InstanceTraits::AS3valueOf()"
    }
    void Namespace::AS3toString(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::AS3toString()"
        SF_UNUSED3(cti, argc, argv);

        // This method is not generic.
        if (!_this.IsNamespace())
            return vm.ThrowTypeError(VM::Error(VM::eInvokeOnIncompatibleObjectError, vm));

        result = _this.AsNamespace().GetUri();
//##protect##"InstanceTraits::AS3toString()"
    }
    void Namespace::valueOfProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::valueOfProto()"
        toStringProto(cti, vm, _this, result, argc, argv);
//##protect##"InstanceTraits::valueOfProto()"
    }
    void Namespace::toStringProto(const ThunkInfo& cti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
//##protect##"InstanceTraits::toStringProto()"
        SF_UNUSED3(cti, argc, argv);

        if (_this.IsObject() && _this.GetObject())
        {
            const AS3::Object& o = *_this.GetObject();
            if (&o == &vm.GetClassNamespace().GetPrototype())
            {
                // This method is called on a prototype object.
                result = vm.GetStringManager().GetBuiltin(AS3Builtin_empty_);
                return;
            }
        }
        else if (_this.IsNamespace())
        {
            result = _this.AsNamespace().GetUri();
            return;
        }

        vm.ThrowTypeError(VM::Error(VM::eUndefinedVarError, vm));
//##protect##"InstanceTraits::toStringProto()"
    }
//##protect##"instance_traits$methods"

    Namespace::~Namespace()
    {
        // Notify factory of our destruction for debug error checking.
        pNamespaceFactory->onNamespaceTraitsDestroy();
    }

    Pickable<Instances::fl::Namespace> Namespace::MakeInternedInstance(Abc::NamespaceKind kind, const ASString& uri,
                                                         const Value& prefix)
    {
        return pNamespaceFactory->MakeNamespace(kind, uri, prefix);
    }

    Instances::fl::Namespace& Namespace::GetInternedInstance(Abc::NamespaceKind kind, const ASString& uri,
        const Value& prefix)
    {
        return pNamespaceFactory->GetNamespace(kind, uri, prefix);
    }

//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    // const UInt16 Namespace::tito[Namespace::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* Namespace::tit[2] = {
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo Namespace::ti[Namespace::ThunkInfoNum] = {
        {&InstanceTraits::fl::Namespace::valueOfProto, &Namespace::tit[0], "valueOf", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {&InstanceTraits::fl::Namespace::toStringProto, &Namespace::tit[1], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Namespace::Namespace(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Namespace::Namespace()"
//##protect##"class_::Namespace::Namespace()"
    }

    void Namespace::InitPrototype(AS3::Object& obj) const
    {
        if (GetParentClass())
            GetParentClass()->InitPrototype(obj);

        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            AddDynamicFunc(obj, ti[i]);
        AddConstructor(obj);
    }
//##protect##"class_$methods"
    void Namespace::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void Namespace::Construct(Value& result, unsigned argc, const Value* argv, bool /*extCall*/)
    {
#if 0
        InstanceTraits::fl::Namespace& itr = static_cast<InstanceTraits::fl::Namespace&>(GetClassTraits().GetInstanceTraits());
        Pickable<Instances::fl::Namespace> inst = itr.MakeInternedInstance(Abc::NS_Public, GetStringManager().CreateEmptyString());
        inst->AS3Constructor(argc, argv);
        result = inst;
#else
        VM& vm = GetVM();

        if (argc == 1)
        {
            const Value& v = argv[0];
            const Traits& tr = vm.GetValueTraits(v);
            const BuiltinTraitsType tt = tr.GetTraitsType();

            if (tt == Traits_Namespace && tr.IsInstanceTraits())
            {
                // Do not create a new instance.
                result = v;
                return;
            }
        }

        InstanceTraits::fl::Namespace& itr = static_cast<InstanceTraits::fl::Namespace&>(GetClassTraits().GetInstanceTraits());
        Pickable<Instances::fl::Namespace> inst = itr.MakeInstance(Abc::NS_Public, GetStringManager().CreateEmptyString(), Value::GetUndefined());
        // Assign right after creating an object to avoid memory leaks in case of exception.
        result = inst;
        inst->AS3Constructor(argc, argv);
        if (vm.IsException())
            return;
        itr.pNamespaceFactory->AddNamespace(*inst);
#endif
    }
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl
{

    Namespace::Namespace(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Namespace::Namespace()"
        SetTraitsType(Traits_Namespace);
//##protect##"ClassTraits::Namespace::Namespace()"

    }

    Pickable<Traits> Namespace::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Namespace(vm, AS3::fl::NamespaceCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::NamespaceCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
    bool Namespace::Coerce(const Value& value, Value& result) const
    {
        InstanceTraits::Traits& itr = GetInstanceTraits();
        itr.GetClass().Construct(result, 1, &value);
        return !GetVM().IsException();
    }
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo NamespaceTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::Namespace::InstanceType),
        0,
        0,
        InstanceTraits::fl::Namespace::ThunkInfoNum,
        0,
        "Namespace", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NamespaceCI = {
        &NamespaceTI,
        ClassTraits::fl::Namespace::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl::Namespace::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

