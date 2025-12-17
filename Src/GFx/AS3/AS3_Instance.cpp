/**************************************************************************

Filename    :   AS3_Instance.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Instance.h"
#include "AS3_VM.h"

#ifdef SF_AS3_AOTC
#include "Obj/AS3_Obj_Global.h"
#endif

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
const char* NS_XML = "http://www.w3.org/XML/1998/namespace";
const char* NS_AS3 = "http://adobe.com/AS3/2006/builtin";
const char* NS_flash_proxy = "http://www.adobe.com/2006/actionscript/flash/proxy";
const char* NS_Vector = "__AS3__.vec";

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits
{
    Traits::Traits(Pickable<Instances::fl::Namespace> ns, VM& vm, const Traits* pt, bool isDynamic, bool isFinal)
    : AS3::Traits(vm, pt, isDynamic, isFinal)
    , Ns(ns)
    {
    }

    Traits::~Traits()
    {
    }

    void Traits::MakeObject(Value& result, Traits& t)
    {
        result = Pickable<Instances::fl::Object>(new(t.Alloc()) Instances::fl::Object(t));
    }

    void Traits::AS3Constructor(const AS3::Traits& /*ot*/, const Value& _this, unsigned argc, const Value* argv) const
    {
        SF_ASSERT(!_this.IsNull());
        _this.GetObject()->AS3Constructor(argc, argv);
    }

#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
    void Traits::SetProtectedNamespace(Pickable<Instances::fl::Namespace> ns)
    {
        ProtectedNamespace = ns;
        ProtectedNamespace->SetOwner(*this);
    }

    const Instances::fl::Namespace& Traits::GetPrivateNamespace()
    {
        if (!PrivateNamespace)
        {
            PrivateNamespace = GetVM().MakeNamespace(Abc::NS_Private);
            PrivateNamespace->SetOwner(*this);
        }

        return *PrivateNamespace;
    }
#endif

    ASString Traits::GetQualifiedName(QNameFormat f) const
    {
        const ASString& nsUri = GetNamespace().GetUri();
        const ASString name = GetName();

        if (nsUri.IsEmpty())
            return name;

        if (f == qnfWithColons)
            return nsUri + "::" + name;

        return nsUri + "." + name;
    }

    bool Traits::SupportsInterface(const Traits&) const
    {
        return false;
    }

	bool Traits::IsParentTypeOf(const Traits& other) const
	{
		// let valueType refer to *this* type. If valueType is the same as otherType, result
		// is true. If otherType is a base type of valueType, or an implemented interface of valueType,
		// then result is true. Otherwise result is set to false.

		if (this == &other)
			return true;

		const Traits* tr = &other;

		while (tr)
		{
			// Check implemented interfaces.
			if (tr->SupportsInterface(*this))
				return true;

			tr = tr->GetParent();
			if (tr == this)
				return true;
		}

		return false;
	}

    void Traits::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        AS3::Traits::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<Instances::fl::Namespace, Mem_Stat>(prcc, Ns, op SF_DEBUG_ARG(*this));
    }

    void Traits::AddInterfaceSlots2This(VMAbcFile* /*file_ptr*/, InstanceTraits::Traits& /*This*/)
    {
        ; // Do nothing.
    }

    ///////////////////////////////////////////////////////////////////////
    Prototype::Prototype(VM& vm, const ClassInfo& ci) : CTraits(vm, ci)
    {
        SetConstructor(vm.GetClassObject());
    }

    Prototype::Prototype(VM& vm, const ClassInfo& ci, Class& c) : CTraits(vm, ci)
    {
        SetConstructor(c);
    }

    void Prototype::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    Anonimous::Anonimous(VM& vm) : Prototype(vm, CInfo) 
    {

    }

    Void::Void(VM& vm) : Prototype(vm, CInfo)
    {

    }

    ///////////////////////////////////////////////////////////////////////
    CTraits::CTraits(VM& vm, const ClassInfo& ci)
        : Traits(vm.MakeInternedNamespace(Abc::NS_Public, ci.GetPkgName()), vm, GetParentInstanceTraits(vm, ci, vm.GetFrameAppDomain()), ci.IsDynamicObject(), ci.IsFinalType())
    , CI(ci)
    {
        SetMemSize(ci.GetInstanceSize());

        UInt16 size = ci.GetInstanceMemberNum();
        for (UInt8 i = 0; i < size; ++i)
            AddSlot(ci.InstanceMember[i]);

        size = ci.GetInstanceMethodNum();
        for (UInt8 i = 0; i < size; ++i)
            Add2VT(ci, ci.InstanceMethod[i]);

        const TypeInfo& ti = *ci.Type;

        if (ti.IsInterface())
            SetIsInterface();

        if (ti.Implements)
        {
            UPInt ii = 0;
            for (; ti.Implements[ii] != NULL; ++ii)
            {
                Multiname mn(GetVM(), *ti.Implements[ii]);
                RegisterImplementedInterface(mn);
            }

            // AddInterfaceSlots2This() has to go after adding of methods.
            // It is a call to a virtual method inside of a constructor.
            // But this is fine in this case.
            // We should call AddInterfaceSlots2This() AFTER we register implemented interfaces.
            if (ii > 0)
                AddInterfaceSlots2This(NULL, *this);
        }
    }

    CTraits::~CTraits()
    {
    }

    const Traits* CTraits::GetParentInstanceTraits(VM& vm, const ClassInfo& ci, VMAppDomain& appDomain)
    {
        const ClassTraits::Traits* ctr = RetrieveParentClassTraits(vm, ci, appDomain);
        const Traits* itr = NULL;

        if (ctr)
            itr = &ctr->GetInstanceTraits();

        return itr;
    }

    ASString CTraits::GetName() const
    {
        // CI.IsCompileTime() should always be true.
        SF_ASSERT(CI.IsCompileTime());
        return GetVM().GetStringManager().CreateConstString(CI.GetName());
    }

    bool CTraits::SupportsInterface(const Traits& itraits) const
    {
        VM& vm = GetVM();
        const ArrayLH<Multiname>& interfaces = GetImplementedInterfaces();

        for (UPInt i = 0; i < interfaces.GetSize(); ++i)
        {
            const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(interfaces[i], GetAppDomain());

            if (ctr)
            {
                InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

                if (&itr == &itraits)
                    return true;

                // Recursive handling of nested interfaces.
                if (itr.SupportsInterface(itraits))
                    return true;
            }
        }

        return false;
    }

    void CTraits::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        Traits::ForEachChild_GC(prcc, op);

        const UPInt size = ImplementsInterfaces.GetSize();
        for (UPInt i = 0; i < size; ++i)
            ImplementsInterfaces[i].ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));
    }

    void CTraits::AddInterfaceSlots2This(VMAbcFile* file_ptr, InstanceTraits::Traits& This)
    {
        VM& vm = GetVM();
        const UPInt size = ImplementsInterfaces.GetSize();
        VMAppDomain& domain = (file_ptr ? file_ptr->GetAppDomain() : vm.GetSystemAppDomain());

        for (UPInt i = 0; i < size; ++i)
        {
            const Multiname& interfaceMN = ImplementsInterfaces[i];

#if 0
            const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(interfaceMN);
            if (!ctr)
                ctr = AS3::FindGOTraits(GetVM(), GetVM().GetGlobalObjects(), interfaceMN);
#else
            const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(interfaceMN, domain);
#endif

            if (!ctr || !ctr->IsValid())
                return GetVM().ThrowVerifyError(VM::Error(VM::eClassNotFoundError, GetVM() 
                SF_DEBUG_ARG(interfaceMN.GetName())
                ));

            InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

            This.AddInterfaceSlots(file_ptr, itr);
        }
    }

    ///////////////////////////////////////////////////////////////////////
    RTraits::RTraits(VM& vm, const ASString& n, Pickable<Instances::fl::Namespace> ns, const Traits* pt, bool isDynamic, bool isFinal)
    : Traits(ns, vm, pt, isDynamic, isFinal)
    , Name(n)
    {
    }

    RTraits::~RTraits()
    {
    }

    void RTraits::MakeObject(Value& result, Traits& t)
    {
        // This code is similar to the code in InstanceTraits::UserDefined.

        /* DO NOT delete.
        // This code doesn't compile because of constness.
        Traits* parent = GetParent();
        // UserDefined classes always have a parent. It is at least a class Object.
        SF_ASSERT(parent);
        parent->MakeObject(result, t);
        result.GetObject()->SetTraits(*this);
        */

        Traits* parent = const_cast<Traits*>(GetParent());

        if (parent == NULL)
            // Just in case.
            parent = &GetVM().GetITraitsObject();

        parent->MakeObject(result, t);
    }

    unsigned RTraits::GetFixedMemSize() const
    {
        const Traits* parent = GetParent();

        if (parent)
            return parent->GetFixedMemSize();

        // We do not have parent in case of interface.
        return sizeof(Instances::fl::Object);
    }

    ASString RTraits::GetName() const
    {
        return Name;
    }

    ///////////////////////////////////////////////////////////////////////
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    AOTraits::AOTraits(VM& vm, const ClassInfo& ci)
    : CTraits(vm, ci)
    , GO()
    {
    }

    void AOTraits::SetGO(Instances::fl::GlobalObject* go)
    {
        SF_ASSERT(!IsValid());
        GO = go;
    }

//     void AOTraits::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
//     {
//         CTraits::ForEachChild_GC(prcc, op);
// 
//         AS3::ForEachChild_GC(prcc, GO, op SF_DEBUG_ARG(*this));
//     }

    ///////////////////////////////////////////////////////////////////////
    TraitsAO::TraitsAO(VM& vm, const MemberInfo mi[], UInt32 mi_size)
        : Traits(vm.MakePublicNamespace(), vm, NULL, false, true)
    {
        for (UInt32 i = 0; i < mi_size; ++i)
            AddSlot(mi[i]);
    }

    ASString TraitsAO::GetName() const
    {
        return GetVM().GetStringManager().CreateConstString("activation");
    }

#endif

    ///////////////////////////////////////////////////////////////////////
    const TypeInfo Anonimous::TInfo = {
        TypeInfo::CompileTime, 
        sizeof(InstanceTraits::Anonimous::InstanceType),
        0, 0, 0, 0,
        "", "", &AS3::fl::ObjectTI,
        TypeInfo::None
    };
    const ClassInfo Anonimous::CInfo = {&Anonimous::TInfo, NULL};

    ///////////////////////////////////////////////////////////////////////
    const TypeInfo Void::TInfo = {
        TypeInfo::CompileTime, 
        sizeof(InstanceTraits::Void::InstanceType),
        0, 0, 0, 0,
        "void", "", &AS3::fl::ObjectTI,
        TypeInfo::None
    };
    const ClassInfo Void::CInfo = {&Void::TInfo, NULL};

} // namespace InstanceTraits

///////////////////////////////////////////////////////////////////////////////
Instance::Instance(InstanceTraits::Traits& t) 
: AS3::Object(t)
{
    t.ConstructTail(*this);
}

Instance::~Instance()
{
    GetTraitsSafe().DestructTail(*this);
}

void Instance::Construct(Value& /*result*/, unsigned /*argc*/, const Value* /*argv*/, bool /*extCall*/)
{
    return GetVM().ThrowTypeError(VM::Error(VM::eNotConstructorError, GetVM()));
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

