/**************************************************************************

Filename    :   AS2_NetConnection.h
Content     :   GFx video: AS2 NetConnection class
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_AS2NETCONNECTION_H
#define INC_GFX_VIDEO_AS2NETCONNECTION_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class NetConnection;
class NetConnectionProto;
class NetConnectionCtorFunction;

class Environment;

//////////////////////////////////////////////////////////////////////////
//

class NetConnection : public Object
{
protected:
    void commonInit(Environment* penv);
    
    NetConnection(ASStringContext *psc, Object* pprototype) : Object(psc)
    { 
        Set__proto__(psc, pprototype);
    }

public:
    NetConnection(Environment* penv);

    ObjectType GetObjectType() const { return Object_NetConnection; }
};

//////////////////////////////////////////////////////////////////////////
//

class NetConnectionProto : public Prototype<NetConnection>
{
public:
    NetConnectionProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void Connect(const FnCall& fn);
};

//////////////////////////////////////////////////////////////////////////
//

class NetConnectionCtorFunction : public CFunctionObject
{
public:
    NetConnectionCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment *penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_AS2NETCONNECTION_H
