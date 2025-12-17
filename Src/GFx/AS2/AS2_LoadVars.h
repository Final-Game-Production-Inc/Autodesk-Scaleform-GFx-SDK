/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_LoadVars.h
Content     :   LoadVars reference class for ActionScript 2.0
Created     :   3/28/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_LOADVARS_H
#define INC_SF_GFX_LOADVARS_H

#include "GFxConfig.h"
#include "GFx/AS2/AS2_Object.h"
#ifdef GFX_AS2_ENABLE_LOADVARS
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// **** Declared Classes
class LoadVarsObject;
class LoadVarsProto;

// ****************************************************************************
// GAS LoadVars class
// 
class LoadVarsObject : public Object
{
    friend class LoadVarsProto;
    Number BytesLoadedCurrent;
    Number BytesLoadedTotal;
    
    void CommonInit()
    {
        BytesLoadedCurrent = -1;    // undefined
        BytesLoadedTotal = -1;      // undefined
    };


protected:
    LoadVarsObject(ASStringContext *psc, Object* pprototype)
        : Object (psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
        CommonInit();
    }
public:
    LoadVarsObject(Environment* penv);

    virtual ObjectType GetObjectType() const { return Object_LoadVars; }

    virtual bool SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags = PropFlags());

    void NotifyOnData(Environment* penv, ASString& src);
    void NotifyOnHTTPStatus(Environment* penv, Number httpStatus);
    void NotifyOnLoad(Environment* penv, bool success);

    void SetLoadedBytes(Number bytes)
    {
        if (BytesLoadedTotal < 0) BytesLoadedTotal = 0; // set to 0 if undefined
        BytesLoadedCurrent = bytes;
        BytesLoadedTotal += bytes;
    }

};

// ****************************************************************************
// GAS LoadVars prototype class
//
class LoadVarsProto : public Prototype<LoadVarsObject>
{
public:
    LoadVarsProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    static void AddRequestHeader(const FnCall& fn);
    static void Decode(const FnCall& fn);
    static void GetBytesLoaded(const FnCall& fn);
    static void GetBytesTotal(const FnCall& fn);
    static void Load(const FnCall& fn);
    static void Send(const FnCall& fn);
    static void SendAndLoad(const FnCall& fn);
    static void ToString(const FnCall& fn);

    static void DefaultOnData(const FnCall& fn);

    static bool LoadVariables(Environment *penv, ObjectInterface* pchar, const String& data);
};

class GASLoadVarsLoaderCtorFunction : public CFunctionObject
{
public:
    GASLoadVarsLoaderCtorFunction (ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};
}}} // SF::GFx::AS2
#else
namespace Scaleform { namespace GFx { namespace AS2 {
class LoadVarsObject : public Object
{
public:
    LoadVarsObject (ASStringContext *psc) : Object(psc) {}
    void NotifyOnData(Environment* , ASString& ) {}
    void NotifyOnHTTPStatus(Environment* , Number ) {}
    void NotifyOnLoad(Environment* , bool ) {}
    void SetLoadedBytes(Number) {}
};
}}} // SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS

#endif  // INC_SF_GFX_LOADVARS_H
