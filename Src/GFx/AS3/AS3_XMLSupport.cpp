/**************************************************************************

Filename    :   AS3_XMLSupport.h
Content     :   
Created     :   Apr, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_VM.h"

#ifdef GFX_ENABLE_XML
    #include "Kernel/SF_MsgFormat.h"
    #include "AS3_VTable.h"
    #include "Obj/AS3_Obj_QName.h"
    #include "Obj/AS3_Obj_XML.h"
    #include "Obj/AS3_Obj_XMLList.h"
    #include "Obj/AS3_Obj_UserDefined.h"
#endif

namespace Scaleform { namespace GFx { namespace AS3
{

#ifdef GFX_ENABLE_XML
///////////////////////////////////////////////////////////////////////////
class XMLSupportImpl : public XMLSupport
{
    friend class VM;
    typedef XMLSupport Parent;

protected:
    XMLSupportImpl(VM& vm);

public:
    virtual ~XMLSupportImpl();

public:
    virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

public:
    virtual CheckResult EqualsXML(bool& result, Instances::fl::XML& l, Instances::fl::XML& r) const; 
    virtual CheckResult EqualsXML(bool& stop, bool& result, Instances::fl::XML& l, const Value& r) const; 
    virtual CheckResult EqualsXMLList(bool& result, Instances::fl::XMLList& l, const Value& v) const; 
    virtual CheckResult EqualsQName(bool& result, Instances::fl::QName& l, Instances::fl::QName& r) const; 

    virtual bool Add(Value& result, Object& l, Object& r) const; 
    virtual CheckResult ToXMLString(VM& vm, Value& v) const;
    virtual void DescribeType(VM& vm, SPtr<Instances::fl::XML>& result, const Value& value) const;

    // XML
    virtual const ClassTraits::Traits* GetClassTraitsXML() const;
    virtual InstanceTraits::Traits* GetITraitsXML() const;
    // XMLList
    virtual const ClassTraits::Traits* GetClassTraitsXMLList() const;
    virtual InstanceTraits::Traits* GetITraitsXMLList() const;

private:
    XMLSupportImpl& operator =(const XMLSupportImpl&);

    Pickable<Instances::fl::XML>        MakeXML() const;
    Pickable<Instances::fl::XMLList>    MakeXMLList() const;

    static ASString GetQualifiedName(const Instances::fl::Namespace& ns, const ASString& name, Traits::QNameFormat f = Traits::qnfWithColons);

    void DescribeMetaData(VM& vm, Instances::fl::XMLElement& xml, const VMAbcFile& file, const Abc::TraitInfo& ti) const;
    void DescribeTraits(VM& vm, Instances::fl::XMLElement& xml, const Traits& tr) const;

    VM& GetVM() { return VMRef; }

private:
    VM& VMRef;
    SPtr<ClassTraits::fl::XML>      TraitsXML;
    SPtr<ClassTraits::fl::XMLList>  TraitsXMLList;
};

///////////////////////////////////////////////////////////////////////////
XMLSupportImpl::XMLSupportImpl(VM& vm) 
: XMLSupport(vm, true)
, VMRef(vm) 
, TraitsXML(vm.PrepareClassTraits(static_cast<ClassTraits::fl::XML*>(ClassTraits::fl::XML::MakeClassTraits(vm).GetPtr())))
, TraitsXMLList(vm.PrepareClassTraits(static_cast<ClassTraits::fl::XMLList*>(ClassTraits::fl::XMLList::MakeClassTraits(vm).GetPtr())))
{
}

XMLSupportImpl::~XMLSupportImpl()
{
}

void XMLSupportImpl::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    Parent::ForEachChild_GC(prcc, op);
}

CheckResult XMLSupportImpl::EqualsXML(bool& result, Instances::fl::XML& l, Instances::fl::XML& r) const
{
    Instances::fl::XML::Kind lk = l.GetKind();
    Instances::fl::XML::Kind rk = r.GetKind();

    if (((lk == Instances::fl::XML::kText || lk == Instances::fl::XML::kAttr) && r.HasSimpleContent()) ||
        ((rk == Instances::fl::XML::kText || rk == Instances::fl::XML::kAttr) && l.HasSimpleContent()))
    {
        StringBuffer lbuf;
        StringBuffer rbuf;

        l.ToString(lbuf, 0);
        r.ToString(rbuf, 0);

        result = lbuf.GetSize() == rbuf.GetSize() && strncmp(lbuf.ToCStr(), rbuf.ToCStr(), lbuf.GetSize()) == 0;
    }
    else
        result = l.Equals(r);

    return true;
}

CheckResult XMLSupportImpl::EqualsXML(bool& stop, bool& result, Instances::fl::XML& l, const Value& r) const
{
    stop = false;

    if (l.HasSimpleContent())
    {
        StringBuffer l_buf;
        StringBuffer r_buf;

        l.ToString(l_buf, 0);
        if (!r.Convert2String(r_buf))
            return false;

        result = l_buf.GetSize() == r_buf.GetSize() && strncmp(l_buf.ToCStr(), r_buf.ToCStr(), r_buf.GetSize()) == 0;
        stop = true;
    }

    return true;
}

CheckResult XMLSupportImpl::EqualsXMLList(bool& result, Instances::fl::XMLList& l, const Value& v) const
{
    return l.Equals(result, v);
}

CheckResult XMLSupportImpl::EqualsQName(bool& result, Instances::fl::QName& l, Instances::fl::QName& r) const
{
#ifdef SF_URI_AS_NAMESPACE
    result = l.GetNamespace() != NULL && r.GetNamespace() != NULL && *l.GetNamespace() == *r.GetNamespace() && l.GetLocalName() == r.GetLocalName();
#else
    result = l.GetUri() == r.GetUri() && l.GetLocalName() == r.GetLocalName();
#endif

    return true;
}

bool XMLSupportImpl::Add(Value& result, Object& l, Object& r) const
{
    const Traits& ltr = l.GetTraits();
    const Traits& rtr = r.GetTraits();
    const BuiltinTraitsType ltt = ltr.GetTraitsType();
    const BuiltinTraitsType rtt = rtr.GetTraitsType();

    if ((ltt == Traits_XML || ltt == Traits_XMLList) && ltr.IsInstanceTraits() &&
        (rtt == Traits_XML || rtt == Traits_XMLList) && rtr.IsInstanceTraits())
    {
        Pickable<Instances::fl::XMLList> list = MakeXMLList();

        switch (ltt)
        {
        case Traits_XML:
            list->Apppend(static_cast<Instances::fl::XML&>(l));
            break;
        case Traits_XMLList:
            list->Apppend(static_cast<Instances::fl::XMLList&>(l));
            break;
        default:
            break;
        }

        switch (rtt)
        {
        case Traits_XML:
            list->Apppend(static_cast<Instances::fl::XML&>(r));
            break;
        case Traits_XMLList:
            list->Apppend(static_cast<Instances::fl::XMLList&>(r));
            break;
        default:
            break;
        }

        // !!! We must assign to result at the very end because
        // result and l may point to same value.
        result = list;

        return true;
    }

    return false;
}

CheckResult XMLSupportImpl::ToXMLString(VM& vm, Value& v) const
{
    if (v.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::Error(
            v.IsNull() ? VM::eConvertNullToObjectError : VM::eConvertUndefinedToObjectError,
            vm
            ));
        return false;
    }

    StringManager& sm = vm.GetStringManager();
    const Value::KindType k = v.GetKind();

    switch (k)
    {
    case Value::kBoolean:
    case Value::kInt:
    case Value::kUInt:
    case Value::kNumber:
        return v.ToStringValue(sm);
    case Value::kString:
        {
            StringBuffer buf;
            Instances::fl::XML::EscapeElementValue(buf, v.AsString());
            v = sm.CreateString(buf.ToCStr(), buf.GetSize());
        }
    default:
        if (IsXMLObject(v))
        {
            Instances::fl::XML* xml = static_cast<Instances::fl::XML*>(v.GetObject());
            StringBuffer buf;
            xml->ToXMLString(buf, 0, NULL, NULL);
            v = sm.CreateString(buf.ToCStr(), buf.GetSize());
            // It looks like we shouldn't get exception here.
        }
        else if (IsXMLListObject(v))
        {
            Instances::fl::XMLList* xml_list = static_cast<Instances::fl::XMLList*>(v.GetObject());

            v = xml_list->AS3toXMLString();
            // It looks like we shouldn't get exception here.
        }
        else
        {
            if (!v.ToPrimitiveValue(*sm.GetStringManager()) || !v.IsPrimitive())
                return false;

            // We shouldn't get any exception here because we are converting a 
            // primitive value.
            v.ToStringValue(sm).DoNotCheck();

            StringBuffer buf;
            Instances::fl::XML::EscapeElementValue(buf, v.AsString());
            v = sm.CreateString(buf.ToCStr(), buf.GetSize());
        }
        break;
    }

    return true;
}

ASString XMLSupportImpl::GetQualifiedName(const Instances::fl::Namespace& ns, const ASString& name, Traits::QNameFormat f)
{
    const ASString& nsUri = ns.GetUri();

    if (nsUri.IsEmpty())
        return name;

    if (f == Traits::qnfWithColons)
        return nsUri + "::" + name;

    return nsUri + "." + name;
}

void XMLSupportImpl::DescribeMetaData(VM& vm, Instances::fl::XMLElement& xml, const VMAbcFile& file, const Abc::TraitInfo& ti) const
{
    if (!ti.HasMetaData())
        return;

    InstanceTraits::fl::XML& xml_itr = static_cast<InstanceTraits::fl::XML&>(*GetITraitsXML());
    Instances::fl::Namespace& pns = vm.GetPublicNamespace();
    StringManager& sm = vm.GetStringManager();
    const Abc::TraitInfo::MetaInfo& md = ti.GetMetaInfo();
    const UPInt msize = md.GetSize();

    if (msize > 0)
    {
        const ASString _name = sm.CreateConstString("name");
        const ASString _metadata = sm.CreateConstString("metadata");
        const ASString _arg = sm.CreateConstString("arg");
        const ASString _key = sm.CreateConstString("key");
        const ASString _value = sm.CreateConstString("value");
        const Abc::ConstPool& cp = file.GetConstPool();

        for (UPInt k = 0; k < msize; ++k)
        {
            const Abc::MetadataInfo& mdi = md.Get(file.GetMetadata(), AbsoluteIndex(k));
            const StringDataPtr n = mdi.GetName();

            Pickable<Instances::fl::XMLElement> metadata = xml_itr.MakeInstanceElement(xml_itr, pns, _metadata);
            if (!xml.AppendChild(metadata))
                // Exception.
                return;

            metadata->AddAttr(pns, _name, sm.CreateString(n.ToCStr(), n.GetSize()));

            const Abc::MetadataInfo::ItemsT& items = mdi.GetItems();
            const UPInt isize = items.GetSize();
            for (UPInt j = 0; j < isize; ++j)
            {
                Pickable<Instances::fl::XMLElement> arg = xml_itr.MakeInstanceElement(xml_itr, pns, _arg);
                if (!metadata->AppendChild(arg))
                    // Exception.
                    return;

                const Abc::MetadataInfo::Item& item = items[j];

                if (item.GetKeyInd() > 0)
                {
                    const StringDataPtr kPtr = cp.GetString(AbsoluteIndex(item.GetKeyInd()));
                    arg->AddAttr(pns, _key, sm.CreateString(kPtr.ToCStr(), kPtr.GetSize()));
                }

                const StringDataPtr v = cp.GetString(AbsoluteIndex(item.GetValueInd()));
                arg->AddAttr(pns, _value, sm.CreateString(v.ToCStr(), v.GetSize()));
            }
        }
    }
}

void XMLSupportImpl::DescribeTraits(VM& vm, Instances::fl::XMLElement& xml, const Traits& tr) const
{
    StringManager& sm = vm.GetStringManager();
    Instances::fl::Namespace& pns = vm.GetPublicNamespace();
    const ASString _true = sm.GetBuiltin(AS3Builtin_true);
    const ASString _false = sm.GetBuiltin(AS3Builtin_false);
    const ASString _type = sm.CreateConstString("type");
    const ASString _optional = sm.CreateConstString("optional");
    const ASString _parameter = sm.CreateConstString("parameter");
    const ASString _index = sm.CreateConstString("index");
    const Traits* parent = tr.IsClassTraits() ? &vm.GetClassTraitsClassClass() : tr.GetParent();
    InstanceTraits::fl::XML& xml_itr = static_cast<InstanceTraits::fl::XML&>(*GetITraitsXML());

    // Populate "extendsClass"
    if (parent)
    {
        const ASString _ec = sm.CreateConstString("extendsClass");
        for (const Traits* t = parent; t != NULL; t = t->GetParent())
        {
            Pickable<Instances::fl::XMLElement> extends = xml_itr.MakeInstanceElement(xml_itr, pns, _ec);
            if (!xml.AppendChild(extends))
                // Exception.
                return;

            extends->AddAttr(pns, _type, t->GetQualifiedName());
        }
    }

    // Populate "implementsInterface"
    if (tr.IsInstanceTraits())
    {
        const ASString _ii = sm.CreateConstString("implementsInterface");
        HashSetDH<ASString> hii(vm.GetMemoryHeap());
        ASString name = sm.CreateEmptyString();

        for (const Traits* t = &tr; t != NULL; t = t->GetParent())
        {
            const InstanceTraits::Traits& itr = static_cast<const InstanceTraits::Traits&>(*t);

            if (itr.IsAbcObject())
            {
                // ABC Traits.
                const InstanceTraits::UserDefined& ud_itr = static_cast<const InstanceTraits::UserDefined&>(itr);
                const Abc::Instance& inst = ud_itr.GetClassInfo().GetInstanceInfo();
                const Abc::Instance::Interfaces& iint = inst.GetInterfaces();
                const Abc::ConstPool& cp = ud_itr.GetFile().GetConstPool();

                const UPInt size = iint.GetSize();
                for (UPInt i = 0; i < size; ++i)
                {
                    const Abc::Multiname& amn = iint.GetName(cp, AbsoluteIndex(i));
                    Multiname mn(ud_itr.GetFile(), amn);

                    if (mn.GetName().Convert2String(name))
                    {
                        // List implemented interface only once.
                        if (hii.Get(name) == NULL)
                        {
                            hii.Add(name);

                            Pickable<Instances::fl::XMLElement> implements = xml_itr.MakeInstanceElement(xml_itr, pns, _ii);
                            if (!xml.AppendChild(implements))
                                // Exception.
                                return;

                            const Instances::fl::Namespace* ns = NULL;
                            if (mn.IsQName())
                                ns = &mn.GetNamespace();
                            else
                            {
                                // Multiname
                                const NamespaceSet::TContainer& nss = mn.GetNamespaceSet().GetNamespaces();
                                const UPInt UPsize = nss.GetSize();
                                for (UPInt upi = 0; upi < UPsize ; ++upi)
                                {
                                    const Instances::fl::Namespace& cur_ns = *nss[upi];
                                    const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(name, cur_ns, ud_itr.GetFile().GetAppDomain());

                                    if (ctr)
                                    {
                                        ns = &cur_ns;
                                        break;
                                    }
                                }
                            }

                            if (ns == NULL)
                                ns = &pns;

                            implements->AddAttr(pns, _type, GetQualifiedName(*ns, name));

                        }
                    }
                }
            }
            else
            {
                // C++ Traits.
                const ArrayLH<Multiname>& iint = static_cast<const InstanceTraits::CTraits&>(itr).GetImplementedInterfaces();
                const UPInt size = iint.GetSize();

                for (UPInt i = 0; i < size; ++i)
                {
                    if (iint[i].GetName().Convert2String(name))
                    {
                        // List implemented interface only once.
                        if (hii.Get(name) == NULL)
                        {
                            hii.Add(name);

                            Pickable<Instances::fl::XMLElement> implements = xml_itr.MakeInstanceElement(xml_itr, pns, _ii);
                            if (!xml.AppendChild(implements))
                                // Exception.
                                return;

                            // This is a class implemented in C++. There is no special namespace.
                            implements->AddAttr(pns, _type, GetQualifiedName(iint[i].GetNamespace(), name));
                        }
                    }
                }
            }
        }
    }

    // Populate "constructor"
    {
        // Constructor is not in a virtual table. We need to get all info from ABC format.
        if (tr.IsAbcObject())
        {
            if (tr.IsInstanceTraits())
            {
                const InstanceTraits::UserDefined& ud_itr = static_cast<const InstanceTraits::UserDefined&>(tr);
                VMAbcFile& file = ud_itr.GetFile();
                const Abc::Instance& inst = ud_itr.GetClassInfo().GetInstanceInfo();
                const Abc::MethodInfo& mi = inst.GetMethodInfo(file.GetMethods());
                // const Abc::MethodBodyInfo& mbi = mi.GetMethodBodyInfo(file.GetMethodBody());
                const UPInt param_count = mi.GetParamCount();

                if (param_count > 0)
                {
                    const UPInt first_opt_param_num = param_count - mi.GetOptionalParamCount();

                    const ASString _constructor = sm.CreateConstString("constructor");
                    Pickable<Instances::fl::XMLElement> constructor = xml_itr.MakeInstanceElement(xml_itr, pns, _constructor);
                    if (!xml.AppendChild(constructor))
                        // Exception.
                        return;

                    // parameters.
                    for(UPInt i = 0; i < param_count; ++i)
                    {
                        // Resolve multiname ...
                        const Abc::Multiname& abc_type = mi.GetParamType(file.GetConstPool(), i);
                        const Multiname mn(file, abc_type);
                        const ASString type = mn.IsAnyType() ? sm.CreateConstString("*") : GetQualifiedName(mn.GetNamespace(), mn.GetName().AsString());
                        LongFormatter f(i + 1);
                        Pickable<Instances::fl::XMLElement> param = xml_itr.MakeInstanceElement(xml_itr, pns, _parameter);

                        if (!constructor->AppendChild(param))
                            // Exception.
                            return;

                        // index
                        f.Convert();
                        param->AddAttr(pns, _index, vm.GetStringManager().CreateString(f.ToCStr(), f.GetSize()));

                        // type
                        param->AddAttr(pns, _type, type);

                        // optional
                        param->AddAttr(pns, _optional, i >= first_opt_param_num ? _true : _false);
                    }
                }
            }

            // Classes have constructors, but these constructor do not take arguments.
        }

        // Info about "constructor" is not available with classes implemented in C++.
    }

    // Populate with info from Traits.
    {
        const ASString _access = sm.CreateConstString("access");
        const ASString _accessor = sm.CreateConstString("accessor");
        const ASString _method = sm.CreateConstString("method");
        const ASString _declaredBy = sm.CreateConstString("declaredBy");
        const ASString _const = sm.CreateConstString("constant");
        const ASString _var = sm.CreateConstString("variable");
        const ASString _returnType = sm.CreateConstString("returnType");
        const ASString _uri = sm.CreateConstString("uri");
        const ASString _name = sm.CreateConstString("name");

        MultinameHash<bool> handled(vm.GetMemoryHeap());

        // There is no reason to visit parents here because slots already include all info from parents.
        const Slots::CIterator& eit = tr.REnd();
        for (Slots::CIterator it = tr.RBegin(); it > eit; --it)
        {
            const SlotInfo& si = it.GetSlotInfo();

            // Filter out non-public members and methods.
            if (si.GetNamespace() != pns)
                continue;

            const ASString name (it.GetSlotName());

            if (handled.Get(name, si.GetNamespace()) != NULL)
                // This namespace/name combination is already handled.
                continue;

            handled.Add(name, si.GetNamespace(), true);

            const SlotInfo::BindingType bt = si.GetBindingType();

            if (si.IsCode())
            {
                Pickable<Instances::fl::XMLElement> code;

                code = xml_itr.MakeInstanceElement(xml_itr, pns, si.IsMethod() ? _method : _accessor);
                if (!xml.AppendChild(code))
                    // Exception.
                    return;

                code->AddAttr(pns, _name, name);

                if (!si.IsMethod())
                {
                    // Getter/Setter.
                    const char* access = NULL;

                    switch (bt)
                    {
                    case SlotInfo::BT_Get:
                        access = "readonly";
                        break;
                    case SlotInfo::BT_Set:
                        access = "writeonly";
                        break;
                    case SlotInfo::BT_GetSet:
                        access = "readwrite";
                        break;
                    default:
                        break;
                    }

                    const ASString _access_name = sm.CreateConstString(access);
                    code->AddAttr(pns, _access, _access_name);
                }

                if (!si.GetNamespace().GetUri().IsEmpty())
                    code->AddAttr(pns, _uri, si.GetNamespace().GetUri());

                // Retrieve "type", "declaredBy", and method's parameters.
                {
                    const Value func = tr.GetVT().GetValue(si.GetAValueInd() + (bt == SlotInfo::BT_Set ? 1 : 0));
                    const Value::KindType k = func.GetKind();
                    InstanceTraits::Traits& itr = vm.GetFunctReturnType(func, tr.GetAppDomain());

                    // Retrieve "type".
                    ASString type = itr.IsOfType(vm.GetITraitsClass()) ? sm.CreateConstString("*") : itr.GetQualifiedName();

                    if (k == Value::kVTableInd)
                    {
                        const VTable& vt = func.GetTraits().GetVT();
                        const SInt32 ind = func.GetVTableInd();
                        const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));
                        const Abc::MiInd method_ind(real_func.GetMethodInfoInd());
                        const AS3::Traits& real_tr = real_func.GetTraits();
                        SF_ASSERT(real_tr.GetFilePtr());
                        VMAbcFile& file = *real_tr.GetFilePtr();
                        const Abc::ConstPool& cp = file.GetConstPool();
                        const Abc::MethodInfo& mi = file.GetMethodInfo(method_ind);

                        // type
                        if (bt == SlotInfo::BT_Set)
                        {
                            const Abc::Multiname& abc_type = mi.GetParamType(cp, 0);
                            Multiname mn(file, abc_type);
                            type = mn.IsAnyType() ? sm.CreateConstString("*") : GetQualifiedName(mn.GetNamespace(), mn.GetName().AsString());
                        }

                        code->AddAttr(pns, si.IsMethod() ? _returnType : _type, type);

                        // declaredBy
                        code->AddAttr(pns, _declaredBy, real_tr.GetQualifiedName());

                        // Parameters.
                        // We shouldn't retrieve parameters for getters/setters.
                        if (si.IsMethod())
                        {
                            const UPInt size = mi.GetParamCount();
                            const UPInt first_opt_param_num = size - mi.GetOptionalParamCount();

                            for (UPInt i = 0; i < size; ++i)
                            {
                                LongFormatter f(i + 1);
                                Pickable<Instances::fl::XMLElement> param = xml_itr.MakeInstanceElement(xml_itr, pns, _parameter);

                                if (!code->AppendChild(param))
                                    // Exception.
                                    return;

                                // index
                                f.Convert();
                                param->AddAttr(pns, _index, vm.GetStringManager().CreateString(f.ToCStr(), f.GetSize()));

                                // type
                                const Abc::Multiname& abc_type = mi.GetParamType(cp, i);
                                Multiname typeMulti(file, abc_type);
                                if (typeMulti.IsAnyType())
                                    param->AddAttr(pns, _type, sm.CreateConstString("*"));
                                else
                                    param->AddAttr(pns, _type, GetQualifiedName(typeMulti.GetNamespace(), typeMulti.GetName().AsString()));

                                // optional
                                param->AddAttr(pns, _optional, i >= first_opt_param_num ? _true : _false);
                            }
                        }

                        // metadata.
                        {
                            const Abc::TraitInfo* ti = si.GetTraitInfoPtr();
                            const VMAbcFile* abcfile = si.GetFilePtr();
                            if (ti && abcfile)
                                DescribeMetaData(vm, *code, *abcfile, *ti);
                        }
                    }
                    else
                    {
                        // Thunk.
                        SF_ASSERT(k == Value::kThunk);

                        // !!! There is no type info for the time of being.
                        code->AddAttr(pns, si.IsMethod() ? _returnType : _type, type);
                        const ThunkInfo& thunk = func.AsThunk();

                        // Thunk doesn't seem to have information about a class where it 
                        // was originally declared.
                        // code->AddAttr(pns, _declaredBy, tr.GetVT().GetTraits().GetQualifiedName());

                        // We shouldn't retrieve parameters for getters/setters.
                        if (si.IsMethod())
                        {
                            if (!(thunk.MinArgNum_ == 0 && thunk.MaxArgNum_ == SF_AS3_VARARGNUM))
                            {
                                // We have arguments.
                                const unsigned arg_num = static_cast<unsigned>(thunk.MaxArgNum_ == SF_AS3_VARARGNUM ? thunk.MinArgNum_ : thunk.MaxArgNum_);

                                for (unsigned i = 0; i < arg_num; ++i)
                                {
                                    LongFormatter f(i + 1);
                                    Pickable<Instances::fl::XMLElement> param = xml_itr.MakeInstanceElement(xml_itr, pns, _parameter);

                                    if (!code->AppendChild(param))
                                        // Exception.
                                        return;

                                    // index
                                    f.Convert();
                                    param->AddAttr(pns, _index, vm.GetStringManager().CreateString(f.ToCStr(), f.GetSize()));

                                    // !!! There is no type info for the time of being.

                                    // optional
                                    param->AddAttr(pns, _optional, i >= thunk.MinArgNum_ ? _true : _false);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // Members.
                Pickable<Instances::fl::XMLElement> member;

                member = xml_itr.MakeInstanceElement(xml_itr, pns, si.IsConst() ? _const : _var);
                if (!xml.AppendChild(member))
                    // Exception.
                    return;

                member->AddAttr(pns, _name, name);

                const ClassTraits::Traits* dt = si.GetDataType(vm);
                // dt == NULL means error.
                if (dt)
                    member->AddAttr(pns, _type, dt->GetQualifiedName());

                if (!si.GetNamespace().GetUri().IsEmpty())
                    member->AddAttr(pns, _uri, si.GetNamespace().GetUri());

                const Abc::TraitInfo* ti = si.GetTraitInfoPtr();
                if (ti)
                    DescribeMetaData(vm, *member, *si.GetFilePtr(), *ti);
            }
        }
    }

    // factory.
    if (tr.IsClassTraits())
    {
        const ASString _factory = sm.CreateConstString("factory");
        Pickable<Instances::fl::XMLElement> factory = xml_itr.MakeInstanceElement(xml_itr, pns, _factory, &xml);

        if (!xml.AppendChild(factory))
            // Exception.
            return;

        factory->AddAttr(pns, _type, tr.GetQualifiedName());

        DescribeTraits(vm, *factory, static_cast<const ClassTraits::Traits&>(tr).GetInstanceTraits());
    }

#if 0
    // DO NOT delete this code.
    // Do we really need to handle dynamic attributes?
    // 
    // Put dynamic attributes.
    // We already checked for null before.
    if (value.IsObject())
    {
        const AS3::Object& obj = *value.GetObject();
        const AS3::Object::DynAttrsType* da = obj.GetDynamicAttrs();

        if (da)
        {
            AS3::Object::DynAttrsType::ConstIterator it = da->Begin();

            for (; !it.IsEnd(); ++it)
            {
                if (it->First.IsDoNotEnum())
                    continue;
            }
        }
    }
#endif
}

void XMLSupportImpl::DescribeType(VM& vm, SPtr<Instances::fl::XML>& result, const Value& value) const
{
    StringManager& sm = vm.GetStringManager();
    Instances::fl::Namespace& pns = vm.GetPublicNamespace();
    const Traits& tr = vm.GetValueTraits(value);
    const ASString _true = sm.GetBuiltin(AS3Builtin_true);
    const ASString _false = sm.GetBuiltin(AS3Builtin_false);
    const ASString _type = sm.CreateConstString("type");
    const Traits* parent = value.IsClass() ? &vm.GetClassTraitsClassClass() : tr.GetParent();

    // Create XMLElement.
    InstanceTraits::fl::XML& xml_itr = static_cast<InstanceTraits::fl::XML&>(*GetITraitsXML());
    Pickable<Instances::fl::XMLElement> xml = xml_itr.MakeInstanceElement(
        xml_itr, 
        pns,
        _type
        );
    result = xml;

    // Populate attributes of "type".
    if (value.IsNullOrUndefined())
    {
        xml->AddAttr(pns, sm.CreateConstString("name"), value.IsNull() ? sm.CreateConstString("null") : tr.GetQualifiedName());
        xml->AddAttr(pns, sm.CreateConstString("isDynamic"), _false);
        xml->AddAttr(pns, sm.CreateConstString("isFinal"), _true);
        xml->AddAttr(pns, sm.CreateConstString("isStatic"), _false);
    }
    else
    {
        xml->AddAttr(pns, sm.CreateConstString("name"), tr.GetQualifiedName());
        if (parent != NULL)
            xml->AddAttr(pns, sm.CreateConstString("base"), parent->GetQualifiedName());

        xml->AddAttr(pns, sm.CreateConstString("isDynamic"), tr.IsDynamic() ? _true : _false);
        xml->AddAttr(pns, sm.CreateConstString("isFinal"), tr.IsFinal() ? _true : _false);
        xml->AddAttr(pns, sm.CreateConstString("isStatic"), tr.IsClassTraits() ? _true : _false);

        DescribeTraits(vm, *xml, tr);
    }
}

const ClassTraits::Traits* XMLSupportImpl::GetClassTraitsXML() const
{
    return TraitsXML;
}

InstanceTraits::Traits* XMLSupportImpl::GetITraitsXML() const
{
    return &TraitsXML->GetInstanceTraits();
}

const ClassTraits::Traits* XMLSupportImpl::GetClassTraitsXMLList() const
{
    return TraitsXMLList;
}

InstanceTraits::Traits* XMLSupportImpl::GetITraitsXMLList() const
{
    return &TraitsXMLList->GetInstanceTraits();
}

Pickable<Instances::fl::XML> XMLSupportImpl::MakeXML() const
{
    InstanceTraits::fl::XML& itr = static_cast<InstanceTraits::fl::XML&>(*GetITraitsXML());
    return itr.MakeInstance(itr);
}

Pickable<Instances::fl::XMLList> XMLSupportImpl::MakeXMLList() const
{
    InstanceTraits::fl::XMLList& itr = static_cast<InstanceTraits::fl::XMLList&>(*GetITraitsXMLList());
    return itr.MakeInstance(itr);
}

void VM::EnableXMLSupport() 
{ 
    XMLSupport_ = Pickable<XMLSupport>(SF_HEAP_NEW(GetMemoryHeap()) XMLSupportImpl(*this));
}

#endif

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

