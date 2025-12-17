/**************************************************************************

Filename    :   AS3_Object.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Object.h"
#include "AS3_VM.h"
#include "AS3_VTable.h"
#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Error.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_Global.h"
#include "AS3_AsString.h" // For logs.
#include "GFx/GFx_ASUtils.h" // for ASNumberUtil::IsNaN, etc
#include "Obj/AS3_Obj_QName.h" // for Multiname::PickRTNameUnsafe().

#ifdef GFX_AS_ENABLE_USERDATA
    #include "GFx/GFx_Player.h"
    #include "GFx/GFx_PlayerImpl.h"
#endif


#ifdef si_ptr
#undef si_ptr
#endif

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_REFCOUNTED_TRAITS
inline void Value::AddRefTraits() const
{
    if (value.VS._1.ITr)
        value.VS._1.ITr->AddRef();
}

void Value::ReleaseTraits()
{
    InstanceTraits::Traits* itr = value.VS._1.ITr;

    if ((UPInt)itr & 1)
        value.VS._1.ITr = (InstanceTraits::Traits*)((UPInt)itr - 1);
    else if (itr)
        itr->Release();
}
#endif

///////////////////////////////////////////////////////////////////////////
const ClassTraits::Traits* FindGOTraits(VM& vm, const TGlobalObjects& go, const Multiname& mn, VMAppDomain& appDomain)
{
    const ClassTraits::Traits* result = NULL;

    const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(mn, appDomain);
    const VMAbcFile* file = NULL;

    if (ctr)
        file = ctr->GetFilePtr();

    for (UPInt i = go.GetSize(); i > 0; --i)
    {
        Instances::fl::GlobalObject* v = go[i - 1];
        const Traits& tr = v->GetTraits();

        if (ctr && file != tr.GetFilePtr())
            continue;

        result = FindFixedTraits(vm, tr, mn, appDomain);

        if (result)
            // For RVO sake ...
            break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
// ??? Can be a method ...
bool HasPublicNamespace(const Abc::ConstPool& cp, const Abc::Multiname& mn)
{
    // QName case ...
    if (mn.IsQName())
        return mn.GetNamespace(cp).IsPublic();
    
    // Multiname case ...
    const Abc::NamespaceSetInfo& nss = mn.GetNamespaceSetInfo(cp);
    const UInt8* ptr = nss.GetData();
    UInt32 size;
    UInt32 ns_ind;
    Abc::Read(ptr, size);

    for(UInt32 i = 0; i < size; ++i)
    {
        Abc::Read(ptr, ns_ind);
        if (cp.GetNamespace(ns_ind).IsPublic())
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Value related section.

void Value::AddRefInternal() const
{
    KindType k = GetKind();
    switch(k)
    {
#ifdef SF_AS3_REFCOUNTED_TRAITS
    case kInstanceTraits:
    case kClassTraits:
        AddRefTraits();
        break;
#endif
#ifdef SF_AS3_AOTC
    case kSNodeIT:
    case kSNodeCT:
        AddRefSNode();
        break;
#endif
    case kString:
        // VStr cannot be NULL because Null string has type Object.
        SF_ASSERT(value.VS._1.VStr);
        AddRefString();
        break;
    case kClass:
    case kFunction:
    case kThunkFunction:
    case kObject:
    case kNamespace:
        AddRefObject();
        break;
    case kThunkClosure:
    case kVTableIndClosure:
        AddRefClosure();
        break;
    default:
        break;
    }
}

void Value::AddRefWeakRef() const
{
    SF_ASSERT(Bonus.pWeakProxy);
    Bonus.pWeakProxy->AddRef();
}

void Value::ReleaseWeakRef()
{
    SF_ASSERT(Bonus.pWeakProxy);
    Bonus.pWeakProxy->Release();

    // Clean up for the safety sake.
    Bonus.pWeakProxy = NULL;
    value.VS._1.VObj = NULL;
    value.VS._2.VObj = NULL;

    SetWeakRef(false);
    SetKind(kUndefined);
}

void Value::ReleaseInternal()
{
    const Value::KindType kind = GetKind();
    switch(kind)
    {
#ifdef SF_AS3_REFCOUNTED_TRAITS
    case kInstanceTraits:
    case kClassTraits:
        ReleaseTraits();
        break;
#endif
#ifdef SF_AS3_AOTC
    case kSNodeIT:
    case kSNodeCT:
        ReleaseSNode();
        break;
#endif
    case kString:
        // VStr cannot be NULL because Null string has type Object.
        SF_ASSERT(value.VS._1.VStr);
        value.VS._1.VStr->Release();
        break;
    case kNamespace:
        {
            Instances::fl::Namespace* ns = value.VS._1.VNs;

            if ((UPInt)ns & 1)
                value.VS._1.VNs = (Instances::fl::Namespace*)((UPInt)ns - 1);
            else if (ns)
                ns->Release();
        }

        break;
    case kClass:
    case kFunction:
    case kThunkFunction:
    case kObject:
        {
            // Object.
            Object* obj = value.VS._1.VObj;

            if ((UPInt)obj & 1)
                value.VS._1.VObj = (Object*)((UPInt)obj - 1);
            else if (obj)
                obj->Release();
        }

        break;
    case kThunkClosure:
    case kVTableIndClosure:
        {
            // Object.
            Object* obj = value.VS._2.VObj;

            if ((UPInt)obj & 1)
                value.VS._2.VObj = (Object*)((UPInt)obj - 1);
            else if (obj)
                obj->Release();
        }

        break;
    default:
        SF_ASSERT(false);
        break;
    }
}

CheckResult Value::Convert2PrimitiveValueUnsafe(ASStringManager& sm, Value& result, Hint hint) const
{
	if (IsPrimitive())
        result = *this;
    else
    {
        // Functions get converted to ZERO.
        switch (GetKind())
        {
        case kNamespace:
            result = AsNamespace().GetUri();
            return true;
        /* DO NOT delete this.
        // Cases below represent dynamic objects, which may have altered
        // toString() and valueOf() methods.
        case kFunction:
        case kThunkFunction:
        case kMethodClosure:
        */

        case kThunk:
        case kThunkClosure:
        case kVTableInd:
        case kVTableIndClosure:
            result.SetNumber(0.0);

            // No exceptions.
            return true;
        default:
            break;
        }

        Object* obj = GetObject();
        if (obj)
        {
            if (!obj->GetDefaultValueUnsafe(result, hint))
                return false;
        }
        else
        {
            // Null.

#if 0
            // !!! DO NOT delete this code !!!
            // Theoretically, we are supposed to run code below.
            // But because obj == NULL we cannot do that. (We cannot get object's Traits)
            BuiltinTraitsType tt = obj->GetTraitsType();

            // Unlike, the [[DefaultValue]] method defined by the Object type, 
            // the XML and XMLList [[DefaultValue]] methods always return a string. 
            // The hint parameter is ignored
            if (tt == Traits_XML || tt == Traits_XMLList)
                hint = Value::hintString;

            if (hint == Value::hintNone)
            {
                if (tt == Traits_Date)
                    hint = Value::hintString;
                else
                    hint = Value::hintNumber;
            }
#endif

            switch (hint)
            {
            case Value::hintNone:
            case Value::hintNumber:
                result.SetNumberUnsafe(0.0);
                break;
            case Value::hintString:
                // Should be string.
                result.AssignUnsafe(sm.CreateConstStringNode("null", 4, 0));
                break;
            }
        }
    }

    // No exceptions.
    return true;
}

CheckResult Value::Convert2PrimitiveValueUnsafeHintNumber(Value& result) const
{
	if (IsPrimitive())
        result = *this;
    else
    {
        // Functions get converted to ZERO.
        switch (GetKind())
        {
        case kNamespace:
            result = AsNamespace().GetUri();
            return true;
        /* DO NOT delete this.
        // Cases below represent dynamic objects, which may have altered
        // toString() and valueOf() methods.
        case kFunction:
        case kThunkFunction:
        case kMethodClosure:
        */

        case kThunk:
        case kThunkClosure:
        case kVTableInd:
        case kVTableIndClosure:
            result.SetNumber(0.0);

            // No exceptions.
            return true;
        default:
            break;
        }

        Object* obj = GetObject();
        if (obj)
        {
            if (!obj->GetDefaultValueUnsafe(result, Value::hintNumber))
                return false;
        }
        else
            // Null.
            result.SetNumberUnsafe(0.0);
    }

    // No exceptions.
    return true;
}

///////////////////////////////////////////////////////////////////////////
Pickable<WeakProxy> GASRefCountBase::CreateWeakProxy()
{
    RefCountCollector<Mem_Stat>* rcc = GetCollector();
    WeakProxy* wp = NULL;

    if (!rcc->WProxyHash.Get(this, &wp))
    {
        if ((wp = SF_HEAP_AUTO_NEW(this) WeakProxy(this)) == 0)
            return Pickable<WeakProxy>(0);

        SetWeakRef();
        rcc->WProxyHash.Add(this, wp);
    }

    wp->AddRef();
    return Pickable<WeakProxy>(wp);
}

///////////////////////////////////////////////////////////////////////////
NamespaceSet::NamespaceSet(VM& vm)
: GASRefCountBase(&vm.GetGC())
{
}

void NamespaceSet::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    const UPInt size = Namespaces.GetSize();
    for (UPInt i = 0; i < size; ++i)
        AS3::ForEachChild_GC(prcc, Namespaces[i], op SF_DEBUG_ARG(*this));
}
    
bool NamespaceSet::Contains(const Instances::fl::Namespace& ns) const
{
    const UPInt size = Namespaces.GetSize();
    for(UPInt i = 0; i < size; ++i)
    {
        if (*Namespaces[i].GetPtr() == ns)
            return true;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
Multiname::Multiname(VM& vm)
: Kind(Abc::MN_QName)
{
    Name = vm.GetStringManager().CreateEmptyString();

    // Set up namespace.
    Obj = vm.GetDefXMLNamespace();

    if (Obj == NULL)
        Obj = &vm.GetPublicNamespace();
}

Multiname::Multiname(VM& vm, const Value& v)
: Kind(Abc::MN_QName)
{
    // Set up name
    const Value::KindType k = v.GetKind();

    if (v.IsNumeric(k) || v.IsString(k))
    {
        Name = v;
        PostProcessName(false);
    }
    else if (IsQNameObject(v))
    {
        SetFromQName(v);
        return;
    }
    else if (v.IsObject())
    {
        if (v.GetObject() == NULL)
        {
            vm.ThrowTypeError(VM::Error(VM::eNotImplementedError, vm SF_DEBUG_ARG("Multiname")));
            return;
        }

        Name = v;

        if (!Name.ToStringValue(vm.GetStringManager()))
            // Exception.
            return;

        PostProcessName(false);
    }
    else
    {
        vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("Multiname")));
        return;
    }

    // Set up namespace.
    Obj = vm.GetDefXMLNamespace();

    if (Obj == NULL)
        Obj = &vm.GetPublicNamespace();
}

Multiname::Multiname(Instances::fl::Namespace& ns, const Value& name)
: Kind(Abc::MN_QName)
, Obj(&ns)
{
    SetRTNameUnsafe(name);
    SF_ASSERT(IsQName());
}

Multiname::Multiname(NamespaceSet& nss, const Value& name)
: Kind(Abc::MN_Multiname)
, Obj(&nss)
, Name(name)
{
    // ???
    // SetRTNameUnsafe(name);
    SF_ASSERT(IsMultiname());
}

Multiname::Multiname(VMFile& file, const Abc::Multiname& mn)
: Kind(mn.GetKind())
{
    if (!mn.IsAnyType())
    {
        Name = file.GetInternedString(mn.GetNameInd());
        PostProcessName(false);
    }

    // Ignore run-time multinames.
    if (mn.IsQName() && !mn.IsRunTime())
    {
        if (!mn.IsAnyNamespace())
            Obj = &file.GetInternedNamespace(mn.GetNamespaceInd());
    }
    else if (mn.IsMultiname())
        Obj = &file.GetInternedNamespaceSet(mn.GetNamespaceSetInd());
    else
    {
        // RTQName receives null object.
    }
}

Multiname::Multiname(const VM& vm, const StringDataPtr& qname)
: Kind(Abc::MN_QName)
{
    // Parse "flash.display::Stage" or "flash.display.Stage" into namespace + typename.
    bool          colon = true;
    SPInt         dotIndex = qname.FindLastChar(':');

    if (dotIndex < 0)
    {
        colon = false;
        dotIndex = qname.FindLastChar('.');
    }

    const StringDataPtr name = qname.GetTrimLeft(dotIndex + 1);

    if (dotIndex > 0)
    {
        Obj = vm.MakeInternedNamespace(Abc::NS_Public,
            vm.GetStringManager().CreateString(qname.ToCStr(), 
            (colon ? Alg::Max(SPInt(0), (SPInt)dotIndex - 1) : (SPInt)dotIndex)));
    }
    else
        Obj = &vm.GetPublicNamespace();

    Name = vm.GetStringManager().CreateString(name.ToCStr(), name.GetSize());
    PostProcessName(false);
}


Multiname::Multiname(const VM& vm, const TypeInfo& ti)
: Kind(Abc::MN_QName)
{
    StringManager& sm = vm.GetStringManager();

    Obj = vm.MakeInternedNamespace(
        Abc::NS_Public,
        sm.CreateConstString(ti.PkgName)
        );

    Name = sm.CreateConstString(ti.Name);
    PostProcessName(false);
}

Multiname::Multiname(const Multiname& other)
: Kind(other.Kind)
, Obj(other.Obj)
, Name(other.Name)
{
}

bool Multiname::IsAnyType() const
{
    bool result = false;

    if (Name.IsNullOrUndefined())
        result = true;
    else if (Name.IsString())
    {
        const ASString str = Name.AsString();

        result = str.IsEmpty() || str == "*";
    }

    return result;
}

bool Multiname::ContainsNamespace(const Instances::fl::Namespace& ns) const
{
    if (!IsMultiname())
        return *static_cast<Instances::fl::Namespace*>(Obj.GetPtr()) == ns;
    return static_cast<NamespaceSet*>(Obj.GetPtr())->Contains(ns);
}

// Used by op-stack Read() to resolve name components.
void Multiname::SetRTNamespace(Instances::fl::Namespace& ns)
{
    SF_ASSERT(IsRunTime());
    Obj = &ns;
    setNamespaceBits(Abc::MN_NS_Qualified);
}

void Multiname::PickRTNameUnsafe(VSBase& vs)
{
    if (IsQNameObject(vs.Back()))
    {
        SetFromQName(vs.Back());
        vs.PopBack();
    }
    else
    {
        SF_ASSERT(IsNameLate());
        Name.PickUnsafe(vs);

        // Keep MN_NameLate flag. We will use it during code generation.
        // setKind((MultinameKind)(GetKind() & ~Abc::MN_NameLate));

        PostProcessName(false);
    }
}

void Multiname::SetFromQName(const Value& nameVal)
{
    SF_ASSERT(IsQNameObject(nameVal));

    Instances::fl::QName* qn = static_cast<Instances::fl::QName*>(nameVal.GetObject());

    Name = qn->GetLocalName();
    PostProcessName(true);

#ifdef SF_URI_AS_NAMESPACE
    //SF_ASSERT(qn->Ns.GetPtr());
    // It is legal to have qn->Ns == NULL;
    // This means "any name".
    Obj = qn->Ns;
#else
    if (qn->GetUri())
        Obj = qn->GetVM().MakeInternedNamespace(Abc::NS_Public, qn->GetUri()->pData);
#endif

    // Convert kind to QName.
    setKind((Abc::MultinameKind)(GetKind() & ~Abc::MN_NameLate & ~Abc::MN_NS_Multi));
}

void Multiname::SetRTNameUnsafe(const Value& nameVal)
{
    if (IsQNameObject(nameVal))
        SetFromQName(nameVal);
    else
    {
        Name.AssignUnsafe(nameVal);
        PostProcessName(false);
    }
}

Instances::fl::Namespace& Multiname::GetNamespace() const
{
    SF_ASSERT(IsQName());
    SF_ASSERT(Obj.GetPtr());
    return *static_cast<Instances::fl::Namespace*>(Obj.GetPtr());
}

void Multiname::PostProcessName(const bool fromQName)
{
    if (Name.IsString() && !fromQName)
    {
        // '@' in case of QName is just a regular character.
        const ASString name = Name.AsString();

        if (!name.IsEmpty() && name[0] == '@')
        {
            SetAttr();
            Name = name.Substring(1, name.GetSize());
        }
    }
}

void Multiname::ForEachChild_GC(Multiname::Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const
{
    AS3::ForEachChild_GC(prcc, Obj, op SF_DEBUG_ARG(owner));
    AS3::ForEachChild_GC(prcc, Name, op SF_DEBUG_ARG(owner));
}

///////////////////////////////////////////////////////////////////////////
void FindScopeProperty(
    PropRef& result,
    VM& vm,
    const UPInt baseSSInd,
    const ScopeStackType& scope_stack,
    const Multiname& mn
    )
{
    // [2.3.6]
    // Typically, the order of the search for resolving multinames is the object’s declared traits, 
    // its dynamic properties, and finally the prototype chain.
    // The dynamic properties and prototype chain search will only happen if the
    // multiname contains the public namespace (dynamic properties are always in the public namespace in
    // ActionScript 3.0; a run-time error is signaled if an attempt is add a non-public property).
    
    // Look into the scope stack ...
    for (UPInt i = scope_stack.GetSize(); i > baseSSInd ; --i)
    {
        const Value& v = scope_stack[i - 1];

        /* For maniacs
        if (v.IsNullOrUndefined())
            continue;
        */

        FindPropertyWith(result, vm, v, mn);
        
        if (result)
            break;
    }
}

void FindGOProperty(
    PropRef& result, 
    VM& vm, 
    const TGlobalObjects& go, 
    const Multiname& mn,
    const ClassTraits::Traits* ctr
    )
{
    const VMAbcFile* file = NULL;

    if (ctr)
        file = ctr->GetFilePtr();

    const UPInt size = go.GetSize();
    for (UPInt i = 0; i < size; ++i)
    {
        Instances::fl::GlobalObject* obj = go[i];
        Object* v = obj;

        if (ctr && file != v->GetTraits().GetFilePtr())
            continue;

        FindPropertyWith(result, vm, Value(v), mn);
        
        if (result)
            break;
    }
}

const SlotInfo* FindDynamicSlot(const SlotContainerType& sc, 
                                const ASString& name, const Instances::fl::Namespace& ns)
{
    const SlotInfo* sli_ptr = NULL;

    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);
    if (values != NULL)
    {
        for (SPInt ind = *values; ind >= 0; ind = sc.GetPrevInd(ind))
        {
            const SlotInfo& si = sc[AbsoluteIndex(ind)];
            const Instances::fl::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns == ns)
            {
                sli_ptr = &si;
                break;
            }
        }
    }

    return sli_ptr;
}

const SlotInfo* FindFixedSlot(const Traits& t, const ASString& name,
                              const Instances::fl::Namespace& ns, UPInt& index, Object* obj)
{
    const SlotInfo* sli_ptr = NULL;

    const Slots::SetType::ValueType* values = t.FindSlotValues(name);

    if (values != NULL)
    {
        // Find a property with the same name and check namespace after that ...
        for (SPInt ind = *values; ind >= 0; ind = t.GetPrevSlotIndex(ind))
        {
            // DO NOT forget to set index.
            index = static_cast<UPInt>(ind);
            const SlotInfo& si = t.GetSlotInfo(AbsoluteIndex(ind));
            const Instances::fl::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns.GetKind() == ns.GetKind())
            {
                bool found = false;

                switch (ns.GetKind())
                {
                case Abc::NS_Protected:
                    found = true;
                    break;
                case Abc::NS_Private:
                    found = (&ns == &cur_ns);
                    break;
                default:
                    found = (cur_ns.GetUri() == ns.GetUri());
                    break;
                }

                if (found)
                {
                    sli_ptr = &si;
                    break;
                }
            }
        }
    }

    // if (!sli_ptr && obj) will break VM.
    if (obj)
        sli_ptr = obj->InitializeOnDemand(sli_ptr, name, ns, index);

    return sli_ptr;
}

const ClassTraits::Traits* 
FindFixedTraits(const Traits& t, const ASString& name, const Instances::fl::Namespace& ns, UPInt& index)
{
    const ClassTraits::Traits* result = NULL;

    const SlotInfo* si = FindFixedSlot(t, name, ns, index, NULL);
    if (si)
        result = const_cast<SlotInfo*>(si)->GetDataType(t.GetVM());

    return result;
}

const ClassTraits::Traits* 
FindClassTraits(VM& vm, const Multiname& mn, VMAppDomain& appDomain)
{
    const ClassTraits::Traits* ctr = NULL;

    if (mn.IsQName())
        ctr = vm.Resolve2ClassTraits(mn, appDomain);
    else
    {
        NamespaceSet& nss = mn.GetNamespaceSet();
        const NamespaceSet::TContainer& n = nss.GetNamespaces();
        const UPInt size = n.GetSize();

        for (UPInt i = 0; i < size; ++i)
        {
            ctr = vm.Resolve2ClassTraits(mn.GetName(), *n[i], appDomain);
            if (ctr)
                break;
        }
    }

    return ctr;
}

const ClassTraits::Traits* 
FindClassTraits(VMAbcFile& file, const Abc::Multiname& mn)
{
    const ClassTraits::Traits* ctr = NULL;
    VM& vm = file.GetVM();
    const Abc::ConstPool& cp = file.GetConstPool();

    if (mn.IsQName())
        ctr = vm.Resolve2ClassTraits(file, mn);
    else
    {
        const Abc::NamespaceSetInfo& nss = mn.GetNamespaceSetInfo(cp);
        const UInt8* ptr = nss.GetData();
        UInt32 size;
        SInt32 ns_ind;

        Abc::Read(ptr, size);
        for(UInt32 i = 0; i < size; ++i)
        {
            // Optimization for empty strings ...
            Abc::Read(ptr, ns_ind);

            const Abc::Multiname qmn = {ns_ind, -1, mn.GetNameInd(), Abc::MN_QName};
            ctr = vm.Resolve2ClassTraits(file, qmn);
            if (ctr)
                break;
        }
    }

    return ctr;
}

const SlotInfo* FindDynamicSlot(VM& vm, const SlotContainerType& sc, const Multiname& mn)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name))
        // Exception.
        return NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
        return FindDynamicSlot(sc, name, mn.GetNamespace());

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const NamespaceSet::TContainer& namespaces = mn.GetNamespaceSet().GetNamespaces();
    const SlotInfo* si = NULL;
    const UPInt size = namespaces.GetSize();

    for(UPInt i = 0; i < size; ++i)
    {
        si = FindDynamicSlot(sc, name, *namespaces[i]);

        if (si)
            break;
    }

    return si;
}

const SlotInfo* FindFixedSlot(VM& vm, const Traits& t, const Multiname& mn, UPInt& index, Object* obj)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.GetBuiltin(AS3Builtin_empty_);
    if (!vn.Convert2String(name))
        // Exception.
        return NULL;

    const SlotInfo* si_ptr = NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
        return AS3::FindFixedSlot(t, name, mn.GetNamespace(), index, obj);

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const NamespaceSet::TContainer& namespaces = mn.GetNamespaceSet().GetNamespaces();

    const Slots::SetType::ValueType* values = t.FindSlotValues(name);
    const UPInt size = namespaces.GetSize();

    for(UPInt i = 0; i < size; ++i)
    {
        //si_ptr = AS3::FindFixedSlot(GetTraits(), name, namespaces[i].AsNamespace());
        const Instances::fl::Namespace& ns = *namespaces[i];

        if (values != NULL)
        {
            // Find a property with the same name and check namespace after that ...
            for (SPInt ind = *values; ind >= 0; ind = t.GetPrevSlotIndex(ind))
            {
                // DO NOT forget to set index.
                index = static_cast<UPInt>(ind);
                const SlotInfo& si = t.GetSlotInfo(AbsoluteIndex(ind));
                const Instances::fl::Namespace& cur_ns = si.GetNamespace();

                if (cur_ns.GetKind() == ns.GetKind())
                {
                    bool found = false;

                    switch (ns.GetKind())
                    {
                    case Abc::NS_Protected:
                        found = true;
                        break;
                    case Abc::NS_Private:
                        found = (&ns == &cur_ns);

// Future development.
//                         if (!found && cur_ns.IsParentOf(ns))
//                         {
//                             // Private namespaces always have owners.
//                             vm.ThrowReferenceError(VM::ePropertyNotFoundError SF_DEBUG_ARG("Attempt to access private property in a base class"));
//                             return NULL;
//                         }
                        break;
                    default:
                        found = (cur_ns.GetUri() == ns.GetUri());
                        break;
                    }

                    if (found)
                    {
                        si_ptr = &si;
                        break;
                    }
                }
            }
        }

        // if (!si_ptr && obj) will break VM.
        if (obj)
            si_ptr = obj->InitializeOnDemand(si_ptr, name, ns, index);

        if (si_ptr)
            break;
    }

    return si_ptr;
}

const ClassTraits::Traits* FindFixedTraits(VM& vm, const Traits& t, const Multiname& mn, VMAppDomain& appDomain)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name))
        // Exception.
        return NULL;

    const ClassTraits::Traits* result = NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
    {
        UPInt index = 0;
        return AS3::FindFixedTraits(t, name, mn.GetNamespace(), index);
    }

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const NamespaceSet::TContainer& namespaces = mn.GetNamespaceSet().GetNamespaces();

    const Slots::SetType::ValueType* values = t.FindSlotValues(name);
    const UPInt size = namespaces.GetSize();

    for(UPInt i = 0; i < size; ++i)
    {
        //si_ptr = AS3::FindFixedSlot(GetTraits(), name, namespaces[i].AsNamespace());
        const Instances::fl::Namespace& ns = *namespaces[i];

        if (values != NULL)
        {
            // Find a property with the same name and check namespace after that ...
            for (SPInt ind = *values; ind >= 0; ind = t.GetPrevSlotIndex(ind))
            {
                const SlotInfo& si = t.GetSlotInfo(AbsoluteIndex(ind));
                const Instances::fl::Namespace& cur_ns = si.GetNamespace();

                if (cur_ns.GetKind() == ns.GetKind())
                {
                    bool found = false;

                    switch (ns.GetKind())
                    {
                    case Abc::NS_Protected:
                        found = true;
                        break;
                    case Abc::NS_Private:
                        found = (&ns == &cur_ns);

// Future development.
//                         if (!found && cur_ns.IsParentOf(ns))
//                         {
//                             // Private namespaces always have owners.
//                             vm.ThrowReferenceError(VM::ePropertyNotFoundError SF_DEBUG_ARG("Attempt to access private property in a base class"));
//                             return NULL;
//                         }
                        break;
                    default:
                        found = (cur_ns.GetUri() == ns.GetUri());
                        break;
                    }

                    if (found)
                    {
                        result = const_cast<SlotInfo&>(si).GetDataType(vm);
                        break;
                    }
                }
            }
        }

        if (!result)
            result = vm.Resolve2ClassTraits(name, ns, appDomain);

        if (result)
            break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
void FindObjProperty(PropRef& result, VM& vm, const Value& scope, const Multiname& mn, FindPropAttr attr)
{
    //const bool is_object = value.IsObject() && value.GetObject() != NULL;
    //bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsNamespace() || IsMethodClosure() || IsThunkClosure(); }

    // This is different from Value::IsObject().
    bool is_object = false;
    switch (scope.GetKind())
    {
    case Value::kUndefined: // = 0
    case Value::kBoolean:   // = 1
    case Value::kInt:       // = 2
    case Value::kUInt:      // = 3
    case Value::kNumber:    // = 4
    case Value::kString:    // = 5
    case Value::kNamespace: // = 10
        break;
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
        // null should be handled outside of this function. 
        SF_ASSERT(scope.GetObject() != NULL);
        is_object = true;
        break;
    case Value::kThunk:     // = 11
    case Value::kThunkFunction: // = 12
        // kThunkClosure contains object.
    case Value::kThunkClosure:  // = 15
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        break;
    default:
        SF_ASSERT(false);
        break;
    }

    const Traits& t = vm.GetValueTraits(scope);
    const SlotInfo* si;
    UPInt slot_index = 0;

    // Search in fixed slots.
    si = FindFixedSlot(vm, t, mn, slot_index, (is_object ? scope.GetObject() : NULL));

    if (si)
    {
        result = PropRef(scope, si, slot_index);
        return;
    }

    if (!mn.ContainsNamespace(vm.GetPublicNamespace()))
        return;

    // Search in dynamic slots.
    if (is_object && t.IsDynamic())
    {
        // ECMA-357 11.2.2.1
        // The XML and XMLList [[Get]] method is never called for method lookup.
        if (attr != FindCall || !IsXMLObject(scope))
            result = scope.GetObject()->FindDynamicSlot(mn);
    }

    if (result)
        return;

    // In case of FindGet or FindCall
    if (attr != FindSet)
    {
        // Search in a prototype chain.
        {
            const Traits* cur_tr = &t;
            while (cur_tr)
            {
                Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

                proto.FindProperty(result, mn, attr);
                if (result)
                    break;

                cur_tr = cur_tr->GetParent();
            }
        }

#if 1 // This doesn't seems to be necessary.
        // Search in a stored scope.
        // !!!! Handle Closures here !!!!
        if (!result && scope.IsFunction())
        {
            const Instances::Function& f = scope.AsFunction();

            FindScopeProperty(result, vm, 0, f.GetStoredScopeStack(), mn);
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////
/// Very similar to FindProperty2.
void FindPropertyWith(PropRef& result, VM& vm, const Value& scope, const Multiname& mn, FindPropAttr attr)
{
    //const bool is_object = value.IsObject() && value.GetObject() != NULL;
    //bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsNamespace() || IsMethodClosure() || IsThunkClosure(); }

    // This is different from Value::IsObject().
    bool is_object = false;
    switch (scope.GetKind())
    {
    case Value::kUndefined: // = 0
    case Value::kBoolean:   // = 1
    case Value::kInt:       // = 2
    case Value::kUInt:      // = 3
    case Value::kNumber:    // = 4
    case Value::kString:    // = 5
    case Value::kNamespace: // = 10
        break;
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
        // null should be handled outside of this function. 
        SF_ASSERT(scope.GetObject() != NULL);
        is_object = true;
        break;
    case Value::kThunk:     // = 11
    case Value::kThunkFunction: // = 12
        // kThunkClosure contains object ...
    case Value::kThunkClosure:  // = 15
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        break;
    default:
        SF_ASSERT(false);
        break;
    }

    const Traits& t = vm.GetValueTraits(scope);
    const SlotInfo* si;
    UPInt slot_index = 0;

    // Search in fixed slots.
    si = FindFixedSlot(vm, t, mn, slot_index, (is_object ? scope.GetObject() : NULL));

    if (si)
    {
        result = PropRef(scope, si, slot_index);
        return;
    }

    if (!(t.IsGlobal() || scope.GetWith()))
        return;

    // Search in dynamic slots.
    if (is_object && t.IsDynamic())
    {
        // ECMA-357 11.2.2.1
        // The XML and XMLList [[Get]] method is never called for method lookup.
        if (attr != FindCall || !IsXMLObject(scope))
            result = scope.GetObject()->FindDynamicSlot(mn);
    }

    if (result)
        return;

    // In case of FindGet or FindCall
    if (attr != FindSet)
    {
        // Search in a prototype chain.
        {
            PropRef r;
            const Traits* cur_tr = &t;
            while (cur_tr)
            {
                Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

                proto.FindProperty(r, mn, attr);
                if (r)
                {
                    result = PropRef(scope, NULL, 0);
                    break;
                }

                cur_tr = cur_tr->GetParent();
            }
        }

#if 1 // This doesn't seem to be necessary.
        // Search in a stored scope.
        // !!!! Handle Closures here. !!!!
        if (!result && scope.IsFunction())
        {
            const Instances::Function& f = scope.AsFunction();

            FindScopeProperty(result, vm, 0, f.GetStoredScopeStack(), mn);
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////
CheckResult ExecutePropertyUnsafe(VM& vm, const Multiname& prop_name, const Value& _this, Value& result, unsigned argc, const Value* argv)
{
    if (_this.IsObject() && _this.GetObject())
        return _this.GetObject()->ExecutePropertyUnsafe(prop_name, result, argc, argv);

    PropRef prop;

    AS3::FindObjProperty(prop, vm, _this, prop_name, FindCall);

    if (prop)
    {
        Value funct;

        if (prop.GetSlotValueUnsafe(vm, funct, SlotInfo::valExecute))
        {
            if (funct.IsNullOrUndefined())
            {
                // A TypeError is thrown if the property specified by the
                // multiname is null or undefined.
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                    SF_DEBUG_ARG(prop_name.GetName())
                    ));
            }
            else
            {
                // The [[Call]] property is invoked on the value of the resolved property with the arguments 
                // obj, arg1, ..., argn. 
                vm.ExecuteUnsafe(funct, _this, result, argc, argv);
            }
        }
        else
            // We should throw an exception here.
            return false;
    }
    else
    {
        // No property ...
        if (vm.GetValueTraits(_this).IsDynamic() || _this.IsPrimitive())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm
                SF_DEBUG_ARG(prop_name.GetName())
                ));
        }
        else
        {
            vm.ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, vm
                SF_DEBUG_ARG(prop_name.GetName())
                SF_DEBUG_ARG(_this)
                ));
        }
    }

    return !vm.IsException();
}

///////////////////////////////////////////////////////////////////////////
Object::Object(Traits& t)
: GASRefCountBase(&t.GetVM().GetGC())
, pTraits(&t)
#ifdef GFX_AS_ENABLE_USERDATA
, pUserDataHolder(0)
#endif
{
    SF_ASSERT(pTraits);
}

Object::Object(VM& vm)
: GASRefCountBase(&vm.GetGC())
, pTraits(NULL)
#ifdef GFX_AS_ENABLE_USERDATA
, pUserDataHolder(0)
#endif
{
}

Object::~Object()
{
#ifdef GFX_AS_ENABLE_USERDATA
    if (pUserDataHolder)
    {
        pUserDataHolder->NotifyDestroy(this);
        delete pUserDataHolder;
    }
#endif  // GFX_AS_ENABLE_USERDATA
}

// Regular version.
void Object::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    DynAttrsType::ConstIterator it = DynAttrs.Begin();

    for (; !it.IsEnd(); ++it)
        AS3::ForEachChild_GC(prcc, it->Second, op SF_DEBUG_ARG(*this));

    if (pTraits)
        pTraits->ForEachChild_GC_Slot(prcc, *this, op);

    AS3::ForEachChild_GC<Traits, Mem_Stat>(prcc, pTraits, op SF_DEBUG_ARG(*this));
}

void Object::ForEachChild_GC_NoValues(Collector* prcc, GcOp op) const
{
    AS3::ForEachChild_GC<Traits, Mem_Stat>(prcc, pTraits, op SF_DEBUG_ARG(*this));
}

void Object::ValueOfUnsafe(Value& result)
{
    const ASString& name = GetStringManager().GetBuiltin(AS3Builtin_valueOf);
    Instances::fl::Namespace& ns = GetVM().GetPublicNamespace();

	ExecutePropertyUnsafe(Multiname(ns, name), result, 0, NULL).DoNotCheck();
}

CheckResult Object::SetSlotValue(SlotIndex ind, const Value& value)
{
    return GetTraits().SetSlotValue(AbsoluteIndex(ind, GetTraits()), value, this);
}

bool Object::HasProperty(const Multiname& prop_name, bool check_prototype)
{
    PropRef result;
    FindProperty(result, prop_name, check_prototype ? FindGet: FindSet);
    return result;
}

void Object::GetDescendants(Instances::fl::XMLList& /*list*/, const Multiname& /*prop_name*/)
{
}

CheckResult Object::GetDefaultValueUnsafe(Value& result, Value::Hint hint)
{
    StringManager &sm = GetStringManager();
    VM& vm = GetVM();
    Instances::fl::Namespace& ns = vm.GetPublicNamespace();
    BuiltinTraitsType tt = GetTraitsType();

    // Unlike, the [[DefaultValue]] method defined by the Object type, 
    // the XML and XMLList [[DefaultValue]] methods always return a string. 
    // The hint parameter is ignored
    if (tt == Traits_XML || tt == Traits_XMLList)
        hint = Value::hintString;

	if (hint == Value::hintNone)
	{
        if (tt == Traits_Date)
			hint = Value::hintString;
		else
			hint = Value::hintNumber;
	}

	Value value;

	if (hint == Value::hintString)
	{
		// toString part.
		{
            const ASString& name = sm.GetBuiltin(AS3Builtin_toString);
            if (!ExecutePropertyUnsafe(Multiname(ns, name), value, 0, NULL))
                // Exception
                return false;

            result.Swap(value);

			if (result.IsPrimitive())
				return true;
		}

		// valueOf part.
		{
            Value v;
            const ASString& name = sm.GetBuiltin(AS3Builtin_valueOf);
            if (!ExecutePropertyUnsafe(Multiname(ns, name), v, 0, NULL))
                // Exception.
                return false;

            result.Swap(v);

            /*
			if (vm.IsException() || result.IsPrimitive())
				return;
            */
		}
	}
	else
	{
		// valueOf part.
		{
            const ASString& name = sm.GetBuiltin(AS3Builtin_valueOf);
            if (!ExecutePropertyUnsafe(Multiname(ns, name), value, 0, NULL))
                return false;

            result.Swap(value);

			if (result.IsPrimitive())
				return true;
		}

		// toString part.
		{
            Value v;
            const ASString& name = sm.GetBuiltin(AS3Builtin_toString);
            if (!ExecutePropertyUnsafe(Multiname(ns, name), v, 0, NULL))
                return false;

            result.Swap(v);

            /*
			if (vm.IsException() || result.IsPrimitive())
				return;
            */
		}
	}

    return true;
}

void Object::FindProperty(PropRef& result, const Multiname& mn, FindPropAttr attr)
{
    const SlotInfo* si = NULL;
    UPInt slot_index = 0;

    si = AS3::FindFixedSlot(GetVM(), GetTraits(), mn, slot_index, this);
    if (si)
    {
        result = PropRef(this, si, slot_index);
        return;
    }

    // Check dynamic slots.
    if (IsDynamic())
    {
        // ECMA-357 11.2.2.1
        // The XML and XMLList [[Get]] method is never called for method lookup.
        if (attr != FindCall || !IsXMLObject(this))
            result = FindDynamicSlot(mn);
    }

    if (result)
        return;

    // In case of FindGet or FindCall
    if (attr != FindSet)
    {
        // Search in a prototype chain.
        const Traits* cur_tr = &GetTraits();
        while (cur_tr)
        {
            Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            if (&proto == this)
                break;

            proto.FindProperty(result, mn, attr);

            if (result)
                break;

            cur_tr = cur_tr->GetParent();
        }
    }
}

CheckResult Object::ExecutePropertyUnsafe(const Multiname& prop_name, Value& result, unsigned argc, const Value* argv)
{
    PropRef prop;
    const Value _this(this);
    VM& vm = GetVM();

    AS3::FindObjProperty(prop, vm, _this, prop_name, FindCall);
    
    if (prop)
    {
        Value funct;
        
        if (prop.GetSlotValueUnsafe(vm, funct, SlotInfo::valExecute))
        {
            if (funct.IsNullOrUndefined())
                // A TypeError is thrown if the property specified by the multiname is null or undefined.
                vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm SF_DEBUG_ARG(prop_name.GetName())));
            else
                vm.ExecuteUnsafe(funct, _this, result, argc, argv);
        }
        else 
            // We should throw an exception here.
            return false;
    } else
    {
        // No property ...
        if (vm.GetValueTraits(_this).IsDynamic() || _this.IsPrimitive())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw an exception.
            vm.ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, vm SF_DEBUG_ARG(prop_name.GetName())));
        }
        else
        {
            vm.ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, vm
                SF_DEBUG_ARG(prop_name.GetName())
                SF_DEBUG_ARG(_this)
                ));
        }
    }

    return !vm.IsException();
}

void Object::AddDynamicSlotValuePair(const ASString& prop_name, const Value& v, SlotInfo::Attribute a)
{
    UInt32 attrs = a == SlotInfo::aDontEnum ? DynAttrsKey::DoNotEnumMask : 0;

    DynAttrs.Set(DynAttrsKey(prop_name, attrs), v);
}

void Object::AddDynamicSlotValuePair(const Value& prop_name, const Value& v, SlotInfo::Attribute a)
{
    // Name can be passed as any data type.
    // Let's convert it to a string;
    StringManager& sm = GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!prop_name.Convert2String(name))
        // Exception.
        return;

    AddDynamicSlotValuePair(name, v, a);
}

CheckResult Object::DeleteDynamicSlotValuePair(const Multiname& mn)
{
    // Convert to string.
    ASString str_name(GetStringManager().CreateEmptyString());
    if (!mn.GetName().Convert2String(str_name))
        // Exception.
        return false;

    // Remove values
    bool result = true;
    if (DynAttrs.Get(str_name))
        DynAttrs.Remove(str_name);
    else
        result = false;

    return result;
}

void Object::Call( const Value& _this, Value& result, unsigned argc, const Value* const argv )
{
    // Do nothing by default;
    SF_UNUSED4(result, _this, argc, argv);
}

void Object::AS3Constructor(unsigned /*argc*/, const Value* /*argv*/)
{
    // Do nothing by default;
}

void Object::constructor(Value& result)
{
    result = &GetClass();
}

inline
bool IsDeletedKey(ASStringNode* key)
{
    return key == NULL;
}

GlobalSlotIndex Object::GetNextDynPropIndex( GlobalSlotIndex ind ) const
{
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
}

void Object::GetNextPropertyName(Value& name, GlobalSlotIndex ind) const
{
    SF_ASSERT(ind.IsValid());

    const DynAttrsType::ConstIterator bit = DynAttrs.Begin();
    DynAttrsType::ConstIterator it(bit.GetContainer(), AbsoluteIndex(ind).Get());

    name = it->First.GetName();
}

void Object::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
{
    if (!ind.IsValid())
        return;

    GetDynamicProperty(AbsoluteIndex(ind), value);
}

const SlotInfo* Object::InitializeOnDemand(
    const SlotInfo* si,
    const ASString&,
    const Instances::fl::Namespace&,
    UPInt&)
{
    return si;
}

PropRef Object::FindDynamicSlot(const Multiname& mn)
{
    const Value& vn = mn.GetName();
    StringManager& sm = GetVM().GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name))
        // Exception.
        return PropRef();

    // A hack.
    return PropRef(this, DynAttrs.Get(name));
}

#ifdef GFX_AS_ENABLE_USERDATA
void    Object::SetUserData(Movie* pmovieView, ASUserData* puserData, bool isdobj)
{
    if (pUserDataHolder)
    {
        pUserDataHolder->NotifyDestroy(this);
        delete pUserDataHolder;
    }

    pUserDataHolder = SF_HEAP_AUTO_NEW(this) UserDataHolder(pmovieView, puserData);

    if (puserData)
    {
        MovieImpl* pmovieImpl = static_cast<MovieImpl*>(pmovieView);
        puserData->SetLastObjectValue(pmovieImpl->pObjectInterface, this, isdobj);
    }
}

Object::UserDataHolder::~UserDataHolder() 
{ 
}

void Object::UserDataHolder::NotifyDestroy(Object* pthis)
{
    if (pUserData)  
    {
        // Remove user data weak ref
        pUserData->SetLastObjectValue(NULL, NULL, false);
        pUserData->OnDestroy(pMovieView, pthis);
        
        // Clear user data pointer so destructor doesn't access a bad pointer
        pUserData = NULL;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
CheckResult ToString(VM& vm, const Value& _this, ASString& result)
{
    if (_this.IsNullOrUndefined())
    {
        if (!_this.Convert2String(result))
            return false;
    }
    else
    {
        Value r;
        Multiname name(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("toString"));

        if (!ExecutePropertyUnsafe(vm, name, _this, r, 0, NULL))
            return false;

        if (!r.IsString() && !r.Convert2String(result))
            // exception
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetSuperProperty(VM& vm, const Traits* ot, Value& result, const Value& _this, const Multiname& mn, SlotInfo::ValTarget vtt)
{
    const Traits& tr = (ot ? *ot : vm.GetValueTraits(_this));

    if (tr.GetParent())
    {
        // Search object using base class traits.
        UPInt index = 0;
        const Traits& parent_tr = *tr.GetParent();

        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_tr,
            mn,
            index,
            NULL
            );

        // Retrieve value using base class Virtual Table.
        if (si)
        {
            Value v;
            if (si->GetSlotValueUnsafe(vm, v, _this, &parent_tr.GetVT(), vtt))
            {
                result.Swap(v);
                return true;
            }
        }
    }

    /* Old version/ Let's keep it for historic reason.
    const Class& obj_class = tr.GetClass();
    
    if (obj_class.GetParentClass())
    {
        Class& parent_class = *obj_class.GetParentClass();
        const InstanceTraits::Traits& parent_traits = const_cast<const InstanceTraits::Traits&>(parent_class.GetNewObjectTraits());

        // Search object using base classes instance traits.
        UPInt index = 0;
        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_traits,
            mn,
            index,
            &parent_class
            );

        return si && si->GetSlotValue(vm, result, _this, &parent_traits.GetVT());
    }
    */
    
    return false;
}

CheckResult SetSuperProperty(VM& vm, const Traits* ot, const Value& _this, const Multiname& mn, const Value& value)
{
    const Traits& tr = (ot ? *ot : vm.GetValueTraits(_this));

    if (tr.GetParent())
    {
        // Search object using base class traits.
        UPInt index = 0;
        const Traits& parent_tr = *tr.GetParent();

        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_tr, 
            mn,
            index,
            NULL
            );

        // Set value using base class Virtual Table.
        return si && const_cast<SlotInfo*>(si)->SetSlotValue(vm, value, _this, &parent_tr.GetVT());
    }

    /* Old version/ Let's keep it for historic reason.
    const Class& obj_class = tr.GetClass();
    
    if (obj_class.GetParentClass())
    {
        Class& parent_class = *obj_class.GetParentClass();
        const InstanceTraits::Traits& parent_traits = const_cast<const InstanceTraits::Traits&>(parent_class.GetNewObjectTraits());

        // Search object using base classes instance traits.
        UPInt index = 0;
        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_traits, 
            mn,
            index,
            &parent_class
            );

        return si && const_cast<SlotInfo*>(si)->SetSlotValue(vm, value, _this, &parent_traits.GetVT());
    }
    */
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult PropRef::GetSlotValueUnsafe(VM& vm, Value& value, SlotInfo::ValTarget vtt) const
{
    SF_ASSERT(pSI);
    bool result = true;

    switch ((UPInt)pValue & 3U)
    {
    case SlotInfoFlagMask:
        result = pSI->GetSlotValueUnsafe(vm, value, This, NULL, vtt);
        break;
    case ValueFlagMask:
        value.AssignUnsafe(*GetAsValue());
        break;
    case ObjectFlagMask:
        value.AssignUnsafe(GetAsObject());
        break;
    default:
        break;
    }

    return result;
}

CheckResult PropRef::GetSlotValueUnsafe(VM& vm, Value& value, const VTable& vt) const
{
    SF_ASSERT(pSI);
    bool result = true;

    switch ((UPInt)pValue & 3U)
    {
    case SlotInfoFlagMask:
        result = pSI->GetSlotValueUnsafe(vm, value, This, &vt);
        break;
    case ValueFlagMask:
        value.AssignUnsafe(*GetAsValue());
        break;
    case ObjectFlagMask:
        value.AssignUnsafe(GetAsObject());
        break;
    default:
        break;
    }

    return result;
}

CheckResult PropRef::SetSlotValue(VM& vm, const Value& value) const
{
    SF_ASSERT(pSI);
    bool result = true;

    switch ((UPInt)pValue & 3U)
    {
    case SlotInfoFlagMask:
        result = const_cast<SlotInfo*>(pSI)->SetSlotValue(vm, value, This);
        break;
    case ValueFlagMask:
        GetAsValue()->Assign(value);
        break;
    case ObjectFlagMask:
        SF_ASSERT(false);
        result = false;
        break;
    default:
        break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
Class::Class(ClassTraits::Traits& t)
: Object(t)
, ParentClass(t.GetParent() ? const_cast<Class*>(&t.GetParent()->GetInstanceTraits().GetClass()) : NULL)
{
#ifdef SF_AS3_SETUP_INSTANCE_CONSTRUCTOR_IN_CLASS
    t.GetInstanceTraits().SetConstructor(Pickable<Class>(this));
#endif
}

Class::~Class()
{
    // All prototypes should be initialized. At least in the end.
    // Initialize prototypes on demand.
    //SF_ASSERT(pPrototype.Get());
}

void Class::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
{
    InstanceTraits::Traits& itr = GetInstanceTraits();
    VM& vm = GetVM();

    itr.MakeObject(_this, itr);

    if (_this.IsNull())
        vm.ThrowMemoryError(VM::Error(VM::eOutOfMemoryError, vm));
    if (PreInit(_this))
    {
        _this.GetObject()->InitInstance(extCall);
        PostInit(_this, argc, argv);
        // No errors at this point.
        return;
    }

//     // Exception.
//     // Pop arguments from stack.
//     if (!extCall)
//         vm.OpStack.PopBack(argc);
}

CheckResult Class::PreInit(const Value& /*_this*/) const
{
    // Do nothing by default.
    return true;
}

void Class::PostInit(const Value& _this, unsigned argc, const Value* argv) const
{
    _this.GetObject()->AS3Constructor(argc, argv);
}


// Regular version.
void Class::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    Object::ForEachChild_GC(prcc, op);

    // Instance traits are handled  in parent classes.
    
    AS3::ForEachChild_GC<Class, Mem_Stat>(prcc, ParentClass, op SF_DEBUG_ARG(*this));
    AS3::ForEachChild_GC<Object, Mem_Stat>(prcc, pPrototype, op SF_DEBUG_ARG(*this));
}

void Class::InitPrototypeFromVTable(AS3::Object& obj, ConvertFunc f) const
{
    const InstanceTraits::Traits& itr = GetClassTraits().GetInstanceTraits();
    const VTable& vt = itr.GetVT();

    // Do not use itr.BeginFirstOwnSlot() here. It will skip overrwritten methods.
    for (Slots::CIterator it = itr.Begin(); !it.IsEnd(); ++it)
    {
        const SlotInfo& si = it.GetSlotInfo();

        if (si.GetBindingType() == SlotInfo::BT_Code)
        {
            ASStringNode* sn = it.GetSlotName();

            obj.AddDynamicSlotValuePair(
                ASString(sn),
                // !!! The underlaying callback handles Thunk only !!!
                // This is why we do not convert kMethodInd to kVTableInd here.
                // This can be a problem in the future.
                (this->*f)(vt.GetRaw(si.GetAValueInd()) SF_DEBUG_ARG(itr)), 
                SlotInfo::aDontEnum
                );
        }
    }
}

void Class::InitPrototypeFromVTableCheckType(AS3::Object& obj) const
{
    InitPrototypeFromVTable(obj, &Class::ConvertCheckType);
}

void Class::InitPrototype(AS3::Object& obj) const
{
    if (GetParentClass())
        GetParentClass()->InitPrototype(obj);

    InitPrototypeFromVTableCheckType(obj);
    AddConstructor(obj);
}

void Class::AddConstructor(AS3::Object& obj) const
{
    // Add "constructor" to the prototype object.
    obj.AddDynamicSlotValuePair(
        GetVM().GetStringManager().CreateConstString("constructor"),
        Value(const_cast<Class*>(this)),
        SlotInfo::aDontEnum
        );
}

Pickable<Object> Class::MakePrototype() const
{
    return GetVM().MakeObject();
}

Value Class::ConvertCopy(const Value& v SF_DEBUG_ARG(const Traits&)) const
{
    // Just copy the value;
    return v;
}

Value Class::ConvertCheckType(const Value& v SF_DEBUG_ARG(const Traits& ot)) const
{
    return GetVM().GetClassFunction().MakeCheckTypeInstance(*this, v.AsThunk() SF_DEBUG_ARG(ot));
}

Object& Class::GetPrototype()
{
    if (pPrototype.GetPtr() == NULL)
    {
        pPrototype = MakePrototype();
        InitPrototype(*pPrototype);
    }

    return *pPrototype;
}

void Class::lengthGet(SInt32& result)
{
    result = 1;
}

void Class::prototypeGet(Value& result)
{
    result = &GetPrototype();
}

void Class::toString(ASString& result)
{
    result = GetASString("[class " + GetTraits().GetName() + "]");
}

const ClassTraits::Traits& Class::ApplyTypeArgs(unsigned argc, const Value* argv)
{
    SF_UNUSED2(argc, argv);
    return GetClassTraits();
}

void Class::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
{
    // explicit coercion of a class object.
    if (argc == 1)
    {
        const Value& v = argv[0];
        //const Class& cl = GetValueTraits().GetVM().GetClass(v);
        //const Abc::ClassInfo& ci = GetInstanceTraits().GetClassInfo();

        if (GetClassTraits().Coerce(v, result))
            return;
        else
        {
            return GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                SF_DEBUG_ARG(v)
                SF_DEBUG_ARG(GetClassTraits().GetName().ToCStr())
                ));
        }
    }

    return GetVM().ThrowRangeError(VM::Error(VM::eCoerceArgumentCountError, GetVM() SF_DEBUG_ARG(argc)));
}

///////////////////////////////////////////////////////////////////////////////
void ClassCallConstruct::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
{
    // The same logic as in Construct.
    Construct(result, argc, argv);
}

///////////////////////////////////////////////////////////////////////////////
void NamespaceSet::Add(Instances::fl::Namespace& ns)
{
    Namespaces.PushBack(&ns);
}

///////////////////////////////////////////////////////////////////////////
Value::Value(Object* v)
: Flags(kObject)
, value(v)
{
    if (v)
        v->AddRef();
}

Value::Value(Class* v)
: Flags(kClass)
, value(v)
{
    if (v)
        v->AddRef();
}

void  Value::Assign(Object* v)
{
    Release();
    SetKind(kObject);
    value = v;
    if (v)
        v->AddRef();
}

void  Value::AssignUnsafe(Object* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kObject);
    value = v;
    if (v)
        v->AddRef();
}

void Value::Assign(Class* v)
{
    Release();
    SetKind(kClass);
    value = v;
    if (v)
        v->AddRef();
}

void Value::AssignUnsafe(Class* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kClass);
    value = v;
    if (v)
        v->AddRef();
}

void  Value::Pick(Object* v)
{
    Release();
    SetKind(kObject);
    value = v;
    // No AddRef() is necessary.
}

void  Value::PickUnsafe(Object* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kObject);
    value = v;
    // No AddRef() is necessary.
}

void Value::Pick(Class* v)
{
    Release();
    SetKind(kClass);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Class* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kClass);
    value = v;
    // No AddRef() is necessary.
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

