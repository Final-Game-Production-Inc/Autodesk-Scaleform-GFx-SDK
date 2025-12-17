/**************************************************************************

Filename    :   AS2_ExternalInterface.h
Content     :   ExternalInterface AS class implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_EXTINTF_H
#define INC_SF_GFX_EXTINTF_H

#include "Kernel/SF_RefCount.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class ExternalInterface;
class ExternalInterfaceProto;
class ExternalInterfaceCtorFunction;

// ***** External Classes
class Value;


// ActionScript Stage objects.

class ExternalInterface : public Object
{
    friend class ExternalInterfaceProto;
protected:
    ExternalInterface (ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:

    ExternalInterface(Environment* penv);

    virtual ObjectType          GetObjectType() const   { return Object_Stage; }
};

class ExternalInterfaceProto : public Prototype<ExternalInterface>
{
public:
    ExternalInterfaceProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

};

class ExternalInterfaceCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];
public:
    ExternalInterfaceCtorFunction(ASStringContext *psc);

    bool    GetMember(Environment *penv, const ASString& name, Value* val);
    
    static void AddCallback(const FnCall& fn);
    static void Call(const FnCall& fn);

    virtual Object* CreateNewObject(Environment* penv) const;    

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_EXTINTF_H

