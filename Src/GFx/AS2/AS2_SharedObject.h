/**************************************************************************

Filename    :   AS2_SharedObject.h
Content     :   AS2 Implementation of SharedObject class
Created     :   January 20, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SharedObject_H
#define INC_SF_GFX_SharedObject_H

#include "GFxConfig.h"
#if defined(GFX_AS2_ENABLE_SHAREDOBJECT)

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_SharedObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {
//
// AS2 Shared Object
//
class SharedObject : public Object
{
protected:
    SharedObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

public:
#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC() 
    { 
        Name.~String(); 
        LocalPath.~String(); 
        Object::Finalize_GC();
    }
#endif

    SharedObject(Environment* penv);

    virtual ObjectType      GetObjectType() const { return Object_SharedObject; }

    const String& GetName() const { return Name; }
    const String& GetLocalPath() const { return LocalPath; }

    // Overloaded to prohibit setting 'data' member from AS runtime
    virtual bool        SetMember(Environment* penv, 
        const ASString& name, const Value& val, 
        const PropFlags& flags = PropFlags());

    // Special setter to initialize the 'data' member
    void    SetDataObject(Environment* penv, Object* pobj);

    bool    SetNameAndLocalPath(const String& name, const String& localPath );
    
    UPInt   ComputeSizeInBytes(Environment* penv);
    void    Flush(Environment* penv, SharedObjectVisitor* writer);

private:
    String     Name;
    String     LocalPath;
};


//
// AS2 Shared Object Prototype
//
class SharedObjectProto : public Prototype<SharedObject>
{
    static const NameFunction FunctionTable[];

public:
    SharedObjectProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);

    static void         Clear(const FnCall& fn);
    static void         Flush(const FnCall& fn);
    static void         GetSize(const FnCall& fn);
};


//
// Proxy for SharedObject pointers (needed for GC)
//
class SharedObjectPtr : public Ptr<SharedObject> 
{
public:
    SF_INLINE SharedObjectPtr(SharedObject *robj) : Ptr<SharedObject>(robj) {}
    SF_INLINE virtual ~SharedObjectPtr() {}

#ifdef GFX_AS_ENABLE_GC
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void   ForEachChild_GC(RefCountBaseGC<StatMV_ActionScript_Mem>::Collector* prcc) const    { Functor::Call(prcc, pObject); }
    virtual void                    Finalize_GC() {}
#endif // SF_NO_GC
};

//
// AS2 SharedObject Constructor Function
//
class SharedObjectCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void         GetLocal(const FnCall& fn);

    typedef ASStringHash_GC<SharedObjectPtr>     SOLookupType;
    SOLookupType        SharedObjects;

#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    virtual                  void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;
    virtual void                  Finalize_GC();
#endif // SF_NO_GC

public:
    SharedObjectCtorFunction (ASStringContext *psc);

    static void             GlobalCtor(const FnCall& fn);
    virtual Object*      CreateNewObject(Environment*) const;

    static FunctionRef   Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_SHAREDOBJECT

#endif // INC_SF_GFX_SharedObject_H
