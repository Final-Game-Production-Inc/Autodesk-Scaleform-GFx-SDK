/**************************************************************************

Filename    :   AS2_NetConnection.cpp
Content     :   GFx video: AS2 NetConnection class
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/AS2/AS2_NetConnection.h"

#ifdef GFX_ENABLE_VIDEO

namespace Scaleform { namespace GFx { namespace AS2 {

//////////////////////////////////////////////////////////////////////////
//

NetConnection::NetConnection(Environment* penv) : Object(penv)
{
    commonInit(penv);
}

void NetConnection::commonInit(Environment* penv)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_NetConnection));
}

//////////////////////////////////////////////////////////////////////////
//

void NetConnectionProto::Connect(const FnCall& fn)
{
    SF_UNUSED(fn);
}

static const NameFunction AS2_NetConnectionFunctionTable[] = 
{
    { "connect", &NetConnectionProto::Connect },
    { 0, 0 }
};

NetConnectionProto::NetConnectionProto(
    ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<NetConnection>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, AS2_NetConnectionFunctionTable);
}

//////////////////////////////////////////////////////////////////////////
//

NetConnectionCtorFunction::NetConnectionCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

Object* NetConnectionCtorFunction::CreateNewObject(Environment *penv) const
{ 
    Object* obj = SF_HEAP_NEW(penv->GetHeap()) NetConnection(penv); 
    return obj;
}

void NetConnectionCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<NetConnection> pobj;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_NetConnection
                   && !fn.ThisPtr->IsBuiltinPrototype())
        pobj = static_cast<NetConnection*>(fn.ThisPtr);
    else
        pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) NetConnection(fn.Env);

    fn.Result->SetAsObject(pobj.GetPtr());
}

FunctionRef NetConnectionCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) NetConnectionCtorFunction(&sc));
    Ptr<Object> pproto = *SF_HEAP_NEW(pgc->GetHeap()) NetConnectionProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_NetConnection, pproto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_NetConnection), Value(ctor));
    return ctor;
}

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO
