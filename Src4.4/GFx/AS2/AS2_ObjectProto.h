/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_ObjectProto.h
Content     :   ActionScript Object prototype implementation classes
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_OBJECTPROTO_H
#define INC_SF_GFX_OBJECTPROTO_H

#include "GFx/AS2/AS2_Object.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class GlobalContext;

// The base class for Prototype template class. This class contains common
// functionality that does not depend on the template parameters.
class GASPrototypeBase
{
protected:
#ifdef GFX_AS_ENABLE_GC
    // refcount collection version
    class InterfacesArray : public ArrayLH<GASWeakPtr<Object> >
    {
    public:
        typedef ArrayLH<GASWeakPtr<Object> > BaseType;

        InterfacesArray() : BaseType() {}
        InterfacesArray(int size) : BaseType(size) {}
        ~InterfacesArray()
        {
            SF_ASSERT(0); // should never be executed! Finalize_GC should!
        }
        void Finalize_GC()
        {
            AllocatorType::Free(Data.Data);
        }
    };
    typedef FunctionRef ConstructorFuncRef;
#else
    typedef ArrayLH<GASWeakPtr<Object> > InterfacesArray;
    typedef FunctionWeakRef ConstructorFuncRef;
#endif

    // Flash creates a "natural" circular reference by using standard
    // properties "constructor" (in prototype object) and "prototype" (in constructor
    // function object). The "constructor" prototype's property points on a constructor
    // function object and its "prototype" property points back to prototype.
    // This is not a big problem, if garbage collection is in use, the biggest problem
    // exists if just refcounting is used. However, even with GC, using strong pointers for
    // both properties causes extra collection calls and extra memory used.
    // To avoid this problem we use weak reference for the "constructor" property. 
    // Function object has both - weak and strong pointer to prototype. Strong pointer
    // is stored as a member in hash table, weak - just a pointer. This week ptr is necessary
    // to perform "constructor" property cleanup, when Function object dies.
    // "constructor" property in prototype might be strong as well. It becomes strong
    // when this property is assigned by ActionScript (see Prototype::GetMemberRaw).
    // The only known limitation of this approach is the following very rare situation:
    //  function Func() {}   
    //  var p = Func.prototype;
    //  Func = null;
    //  trace ("p.constructor           = "+p.constructor);
    // This trace will print [type Function] in Flash and "undefined" in GFx.
    ConstructorFuncRef Constructor;     // Weak reference, constructor has strong reference
    // to this prototype instance. If GC is in use, this is 
    // a strong reference.
    FunctionRef __Constructor__;     // Strong reference, __constructor__, a reference
    // to this prototype instance

    InterfacesArray*   pInterfaces; // array of implemented interfaces

public:
    GASPrototypeBase(): pInterfaces(0) {}

    virtual ~GASPrototypeBase();

    SF_INLINE FunctionRef  GetConstructor() { return Constructor; }
protected:
    void InitFunctionMembers(
        Object* pthis,
        ASStringContext *psc, 
        const NameFunction* funcTable, 
        const PropFlags& flags = PropFlags::PropFlag_DontEnum);

    // 'Init' method is used from the Prototype ancestor ctor to init the GASPrototypeBase instance.
    // It is actually a part of GASPrototypeBase ctor, but due to GASPrototypeBase is not Object we need
    // to do this in separate function to avoid warning ("usage of 'this' in initializer list).
    void Init(Object* pthis, ASStringContext *psc, const FunctionRef& constructor);

    bool                    SetConstructor(Object* pthis, ASStringContext *psc, const Value& ctor);
    void                    AddInterface(ASStringContext *psc, int index, FunctionObject* pinterface);

    bool                    DoesImplement(Environment* penv, const Object* prototype) const;
    bool                    GetMemberRawConstructor
        (Object* pthis, ASStringContext *psc, const ASString& name, Value* val, bool isConstructor2);
};

// This class contains common definition for prototype classes. It is template since
// basically all prototype objects are the instances of the same class (for example,
// String.prototype is an instance of String).
template <typename BaseClass, typename Environment> 
class Prototype : public BaseClass, public GASPrototypeBase
{
#ifdef GFX_AS_ENABLE_GC
    // refcount collection version
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    typedef RefCountBaseGC<StatMV_ActionScript_Mem>::Collector Collector;

    template <typename Functor>
    void ForEachChild_GC(Collector* prcc) const
    {
        BaseClass::template ForEachChild_GC<Functor>(prcc);
        Constructor.template ForEachChild_GC<Functor>(prcc);
        __Constructor__.template ForEachChild_GC<Functor>(prcc);
        if (pInterfaces)
        {
            for (UPInt i = 0, n = pInterfaces->GetSize(); i < n; ++i)
            {
                GASWeakPtr<Object>& pintf = (*pInterfaces)[i];
                if (pintf)
                    Functor::Call(prcc, pintf);
            }
        }
    }
    virtual void ExecuteForEachChild_GC(
        Collector* prcc, RefCountBaseGC<StatMV_ActionScript_Mem>::OperationGC operation) const
    {
        ASRefCountBaseType::template CallForEachChild<Prototype<BaseClass, Environment> >(prcc, operation);
    }
    void Finalize_GC()
    {
        if (pInterfaces)
        {
            pInterfaces->Finalize_GC();
            SF_FREE(pInterfaces);
        }
        BaseClass::Finalize_GC();
    }
#else
protected:    
    // no refcount collection version

    virtual void CheckAndResetCtorRef(FunctionObject* pfunc) 
    {
        if (Constructor.GetObjectPtr() == pfunc)
            Constructor = NULL;
    }
#endif //SF_NO_GC

protected:    
    void InitFunctionMembers(
        ASStringContext *psc, 
        const NameFunction* funcTable, 
        const PropFlags& flags = PropFlags::PropFlag_DontEnum)
    {
        GASPrototypeBase::InitFunctionMembers(
            this,
            psc, 
            funcTable, 
            flags);
    }
public:

    // MA: We pass psc to base class in case it may need to have member variables of GASString class,
    // as in GASStringObject. Forces ASStringContext constructors on bases. Any way to avoid this?
    Prototype(ASStringContext *psc, const FunctionRef& constructor)
        : BaseClass(psc), GASPrototypeBase()
    {
        GASPrototypeBase::Init(this, psc, constructor);
    }

    Prototype(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor)
        : BaseClass(psc, pprototype), GASPrototypeBase()
    {
        GASPrototypeBase::Init(this, psc, constructor);
    }

    Prototype(ASStringContext *psc, Object* pprototype)
        : BaseClass(psc, pprototype), GASPrototypeBase()
    {
    }

    ~Prototype()
    {
    }

    inline bool     GetMember(Environment* penv, const ASString& name, Value* val)
    {
        SF_UNUSED(penv);
        return GetMemberRaw(penv->GetSC(), name, val);
    }

    virtual bool    GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
    {
        bool isConstructor2 = psc->GetBuiltin(ASBuiltin___constructor__).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive());
        
        // if (name == "constructor" || name == "__constructor__")
        if ( isConstructor2 ||
             psc->GetBuiltin(ASBuiltin_constructor).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()) )
        {
            return GetMemberRawConstructor(this, psc, name, val, isConstructor2);
        }
        return BaseClass::GetMemberRaw(psc, name, val);
    }

    virtual void    AddInterface(ASStringContext *psc, int index, FunctionObject* ctor)
    {
        GASPrototypeBase::AddInterface(psc, index, ctor);
    }
    virtual bool    DoesImplement(Environment* penv, const Object* prototype) const 
    { 
        if (BaseClass::DoesImplement(penv, prototype)) return true; 
        return GASPrototypeBase::DoesImplement(penv, prototype);
    }

    // is it prototype of built-in class?
    virtual bool            IsBuiltinPrototype() const { return true; }
};

// A prototype for Object class (Object.prototype)
class ObjectProto : public Prototype<Object>
{
public:
    ObjectProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    // This ctor is used by 'extends' operation and does NOT 
    // initialize the function table
    ObjectProto(ASStringContext *psc, Object* pprototype);

    ObjectProto(ASStringContext *psc, const FunctionRef& constructor);

    static void GlobalCtor(const FnCall& fn);
    static void AddProperty(const FnCall& fn);
    static void HasOwnProperty(const FnCall& fn);
    static void IsPropertyEnumerable(const FnCall& fn);
    static void IsPrototypeOf(const FnCall& fn);
    static void Watch_(const FnCall& fn);
    static void Unwatch_(const FnCall& fn);
    static void ToString(const FnCall& fn);
    static void ValueOf(const FnCall& fn);
};

// Function Prototype (Function.prototype)
class FunctionProto : public Prototype<Object> 
{
public:
    FunctionProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor, bool initFuncs = true);
    ~FunctionProto() {}

    static void GlobalCtor(const FnCall& fn);
    static void Apply(const FnCall& fn);
    static void Call(const FnCall& fn);
    static void ToString(const FnCall& fn);
    static void ValueOf(const FnCall& fn);
};

// A special implementation for "super" operator
class SuperObject : public Object
{
    Ptr<Object>     SuperProto;
    Ptr<Object>     SavedProto;
    ObjectInterface* RealThis;
    FunctionRef      Constructor;
#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor>
    void ForEachChild_GC(Collector* prcc) const
    {
        Object::template ForEachChild_GC<Functor>(prcc);
        if (SuperProto)
            Functor::Call(prcc, SuperProto);
        if (SavedProto)
            Functor::Call(prcc, SavedProto);
        Constructor.template ForEachChild_GC<Functor>(prcc);
    }
    virtual void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
    {
        ASRefCountBaseType::CallForEachChild<SuperObject>(prcc, operation);
    }
    virtual void Finalize_GC() 
    { 
        Constructor.Finalize_GC(); 
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    SuperObject(Object* superProto, ObjectInterface* _this, const FunctionRef& ctor) :
        Object(superProto->GetCollector()), SuperProto(superProto), RealThis(_this), Constructor(ctor) { pProto = superProto; }

    virtual bool    SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags())
    {
        return SuperProto->SetMember(penv, name, val, flags);
    }
    virtual bool    SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags = PropFlags())
    {
        return SuperProto->SetMemberRaw(psc, name, val, flags);
    }
    virtual bool    GetMember(Environment* penv, const ASString& name, Value* val)
    {
        return SuperProto->GetMember(penv, name, val);
    }
    virtual bool    GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
    {
        return SuperProto->GetMemberRaw(psc, name, val);
    }
    virtual bool    FindMember(ASStringContext *psc, const ASString& name, Member* pmember)
    {
        return SuperProto->FindMember(psc, name, pmember);
    }
    virtual bool    DeleteMember(ASStringContext *psc, const ASString& name)
    {
        return SuperProto->DeleteMember(psc, name);
    }
    virtual bool    SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags)
    {
        return SuperProto->SetMemberFlags(psc, name, flags);
    }
    virtual void    VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface* instance = 0) const
    {
        SuperProto->VisitMembers(psc, pvisitor, visitFlags, instance);
    }
    virtual bool    HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes)
    {
        return SuperProto->HasMember(psc, name, inclPrototypes);
    }

    ObjectInterface* GetRealThis() { return RealThis; }

    /* is it super-class object */
    virtual bool            IsSuper() const { return true; }

    /*virtual Object*      Get__proto__()
    { 
        return SuperProto;
    }*/

    /* gets __constructor__ property */
    virtual FunctionRef   Get__constructor__(ASStringContext *psc)
    { 
        SF_UNUSED(psc);
        return Constructor; 
    }

    void SetAltProto(Object* altProto)
    {
        if (altProto == SuperProto) return;
        SF_ASSERT(!SavedProto);
        SavedProto = SuperProto;
        SuperProto = altProto;
        pProto = SuperProto;
    }
    void ResetAltProto()
    {
        if (SavedProto)
        {
            SuperProto = SavedProto;
            SavedProto = NULL;
            pProto = SuperProto;
        }
    }
    Object* GetSuperProto() { return SuperProto; }
};

}}} // SF::GFx::AS2
#endif //INC_SF_GFX_OBJECTPROTO_H
