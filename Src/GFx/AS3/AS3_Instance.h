/**************************************************************************

Filename    :   AS3_Instance.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Instance_H
#define INC_AS3_Instance_H


#include "AS3_Traits.h"

#ifdef GetClassInfo
// There is a define GetClassInfo in winuser.h
#undef GetClassInfo
#endif

// #define SF_AS3_ENABLE_INSTANCE_NAMESPACES

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
extern const char* NS_AS3;
extern const char* NS_flash_proxy;
extern const char* NS_Vector;
extern const char* NS_XML;

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits
{
    class CTraits;
    class UserDefined;

    class Traits : public AS3::Traits
    {
        friend class InstanceTraits::CTraits; // Because of AddInterfaceSlots().
        friend class InstanceTraits::UserDefined; // Because of AddInterfaceSlots().

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Traits"; }
#endif

    public:
        Traits(Pickable<Instances::fl::Namespace> ns, VM& vm, const Traits* pt, bool isDynamic, bool isFinal);
        ~Traits();

    public:
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;
        // This method is not const because it is supposed to pass non-const "this"
        // into a newly created object ...
        // This method returns Value instead of Object* because primitive types 
        // have more compact representations.
        // Object initialization should be done by the AS3Constructor() method.
        // This is a default implementation.
        virtual void MakeObject(Value& result, Traits& t);
        virtual void AS3Constructor(const AS3::Traits& /*ot*/, const Value& _this, unsigned argc, const Value* argv) const;
        virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const;
        // Determines whether these traits implement the specified
        // interface class.
        virtual bool SupportsInterface(const Traits&) const;

    public:
        const Traits* GetParent() const
        {
            return static_cast<const Traits*>(AS3::Traits::GetParent());
        }

        const Instances::fl::Namespace& GetNamespace() const
        { 
            return *Ns; 
        }

#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
        // May be NULL.
        const Instances::fl::Namespace* GetProtectedNamespace() const { return ProtectedNamespace; }
        void SetProtectedNamespace(Pickable<Instances::fl::Namespace> ns);

        // Should never be NULL.
        const Instances::fl::Namespace& GetPrivateNamespace();
#endif

	public:
		// From documentation to the *istype* opcode:
		// let valueType refer to *this* type. If valueType is the same as otherType, result
		// is true. If otherType is a base type of valueType, or an implemented interface of valueType,
		// then result is true. Otherwise result is set to false.
		bool IsParentTypeOf(const Traits& other) const;
		bool IsOfType(const Traits& other) const
		{
			return other.IsParentTypeOf(*this);
		}

    protected:
        // file_ptr is optional. 
        virtual void AddInterfaceSlots2This(VMAbcFile* file_ptr, InstanceTraits::Traits& This);

    private:
        void AddInterfaceSlots(VMAbcFile* file_ptr, Traits& itr);

    private:
#ifdef SF_AS3_ENABLE_INSTANCE_NAMESPACES
        SPtr<Instances::fl::Namespace> ProtectedNamespace;
        SPtr<Instances::fl::Namespace> PrivateNamespace;
#endif

        SPtr<Instances::fl::Namespace> Ns;
    };

    // Compile-time Traits
    class CTraits : public Traits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::CTraits"; }
#endif

    public:
        CTraits(VM& vm, const ClassInfo& ci);
        ~CTraits();

    public:
        const ClassInfo& GetCI() const
        {
            return CI;
        }
        const ArrayLH<Multiname>& GetImplementedInterfaces() const
        {
            return ImplementsInterfaces;
        }

    public:
        // Virtual functions.
        virtual ASString GetName() const;
        virtual bool SupportsInterface(const Traits&) const;
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

    protected:
        void RegisterImplementedInterface(const Multiname& mn)
        {
            return ImplementsInterfaces.PushBack(mn);
        }
        virtual void AddInterfaceSlots2This(VMAbcFile* file_ptr, InstanceTraits::Traits& This);

    private:
        static const Traits* GetParentInstanceTraits(VM& vm, const ClassInfo& ci, VMAppDomain& appDomain);

    private:
        const ClassInfo& CI;
        // This member is used only by C++ Traits. AS3 Traits use Abc::Instance
        // to get info about implemented interfaces.
        ArrayLH<Multiname> ImplementsInterfaces;
    };

    // Run-time Traits
    class RTraits : public Traits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::RTraits"; }
#endif

    public:
        RTraits(VM& vm, const ASString& n, Pickable<Instances::fl::Namespace> ns, const Traits* pt, bool isDynamic, bool isFinal);
        ~RTraits();

    public:
        const ASString& GetNameRef() const { return Name; }

    public:
        // Virtual functions.
        virtual ASString GetName() const;
        virtual void MakeObject(Value& result, Traits& t);
        virtual unsigned GetFixedMemSize() const;

    private:
        ASString Name;
    };

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    // Ahead Of Time Traits.
    class AOTraits : public CTraits
    {
    public:
        AOTraits(VM& vm, const ClassInfo& ci);

    public:
//         virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

    public:
        bool IsValid() const
        {
            //return GO.GetPtr() != NULL;
            return GO != NULL;
        }
        void SetGO(Instances::fl::GlobalObject* go);
        Instances::fl::GlobalObject& GetGO() const
        {
            SF_ASSERT(IsValid());
            return *GO;
        }

    private:
        // SPtr<Instances::fl::GlobalObject>   GO;
        Instances::fl::GlobalObject*    GO;
    };

    // Activation Object Traits.
    class TraitsAO : public Traits
    {
    public:
        TraitsAO(VM& vm, const MemberInfo mi[], UInt32 mi_size);

    public:
        virtual ASString GetName() const;
    };
#endif

    // This is Traits, which cannot create objects.
    // Such paradigm is used at several places.
    class Prototype : public CTraits
    {
    public:
        typedef Instances::fl::Object InstanceType;

    public:
        Prototype(VM& vm, const ClassInfo& ci);
        Prototype(VM& vm, const ClassInfo& ci, Class& c);

    public:
        virtual void MakeObject(Value& result, Traits& t);
    };

    // Doesn't belong to any class.
    // It is more like the ClassTraits::Traits.
    class Anonimous : public Prototype
    {
    public:
        Anonimous(VM& vm);
        static const TypeInfo TInfo;
        static const ClassInfo CInfo;
    };

    class Void : public Prototype
    {
    public:
        Void(VM& vm);
        static const TypeInfo TInfo;
        static const ClassInfo CInfo;
    };

    template <typename TR>
    inline Pickable<typename TR::InstanceType> MakeInstance(TR& tr)
    {
        return Pickable<typename TR::InstanceType>(new(tr.Alloc()) typename TR::InstanceType(tr));
    }

    template <typename TR>
    inline SPtr<typename TR::InstanceType> MakeInstanceS(TR& tr)
    {
        return MakeInstance(tr);
    }

} // namespace InstanceTraits

namespace ClassTraits
{
#if defined(SF_AS3_AOTC2)
    inline InstanceTraits::AOTraits& AOTraits::GetInstanceTraits() const
    {
        return static_cast<InstanceTraits::AOTraits&>(Traits::GetInstanceTraits());
    }

    inline Instances::fl::GlobalObject& AOTraits::GetGO() const
    {
        return GetInstanceTraits().GetGO();
    }
#endif
}

///////////////////////////////////////////////////////////////////////////
// Instance of a Class ...

class Instance : public AS3::Object
{
    friend class Instances::fl::Object; // Because of Instance(VM& vm).
    
#ifdef GFX_AS3_VERBOSE
private:
    virtual const char* GetAS3ObjectType() const { return "Instance"; }
#endif

public:
    enum {IsClass = 0, IsInstance = 1};

public:
    Instance(InstanceTraits::Traits& t);
    virtual ~Instance();

    // Regular new/delete.
    SF_MEMORY_REDEFINE_NEW(Instance, StatType)
    // Placement new/delete.
    SF_MEMORY_DEFINE_PLACEMENT_NEW

private:
    Instance(VM& vm)
    : AS3::Object(vm)
    {
    }
    
public:
    virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
    
    InstanceTraits::Traits& GetInstanceTraits()
    {
        return static_cast<InstanceTraits::Traits&>(GetTraits());
    }
    const InstanceTraits::Traits& GetInstanceTraits() const
    {
        return static_cast<const InstanceTraits::Traits&>(GetTraits());
    }
};

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Instance_H

