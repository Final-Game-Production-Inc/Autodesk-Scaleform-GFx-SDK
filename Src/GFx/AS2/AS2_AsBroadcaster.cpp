/**************************************************************************

Filename    :   AS2_AsBroadcaster.cpp
Content     :   Implementation of AsBroadcaster class
Created     :   October, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "Kernel/SF_UTF8Util.h"

namespace Scaleform { namespace GFx { namespace AS2 {

AsBroadcaster::AsBroadcaster(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

void AsBroadcaster::commonInit (Environment* penv)
{    
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_AsBroadcaster));
}

//////////////////////////////////////////
//
void AsBroadcasterProto::AddListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);

    ObjectInterface* plistener = fn.Arg(0).ToObjectInterface(fn.Env);
    AsBroadcaster::AddListener(fn.Env, fn.ThisPtr, plistener);
    fn.Result->SetBool(true);
}

void AsBroadcasterProto::BroadcastMessage(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);

    ASString eventName(fn.Arg(0).ToString(fn.Env));
    AsBroadcaster::BroadcastMessage(fn.Env, fn.ThisPtr, eventName, fn.NArgs - 1, fn.Env->GetTopIndex() - 4);
    fn.Result->SetUndefined();
}

void AsBroadcasterProto::RemoveListener(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);

    ObjectInterface* plistener = fn.Arg(0).ToObjectInterface(fn.Env);
    fn.Result->SetBool(AsBroadcaster::RemoveListener(fn.Env, fn.ThisPtr, plistener));
}

static const NameFunction GAS_AsBcFunctionTable[] = 
{
    { "addListener",        &AsBroadcasterProto::AddListener        },
    { "broadcastMessage",   &AsBroadcasterProto::BroadcastMessage   },
    { "removeListener",     &AsBroadcasterProto::RemoveListener     },
    { 0, 0 }
};

AsBroadcasterProto::AsBroadcasterProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<AsBroadcaster>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_AsBcFunctionTable);
}

//////////////////
const NameFunction AsBroadcasterCtorFunction::StaticFunctionTable[] = 
{
    { "initialize", &AsBroadcasterCtorFunction::Initialize },
    { 0, 0 }
};

AsBroadcasterCtorFunction::AsBroadcasterCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void AsBroadcasterCtorFunction::Initialize (const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;
    ObjectInterface* ptr = fn.Arg(0).ToObjectInterface(fn.Env);
    fn.Result->SetUndefined();
    AsBroadcaster::Initialize(fn.Env->GetSC(), ptr);
}
//////////////////
bool AsBroadcaster::Initialize(ASStringContext* psc, ObjectInterface* pobj)
{
    InitializeProto(psc, pobj);
    return InitializeInstance(psc, pobj);
}

bool AsBroadcaster::InitializeProto(ASStringContext* psc, ObjectInterface* pobj)
{
    if (!pobj)
        return false;
    NameFunction::AddConstMembers(pobj, psc, GAS_AsBcFunctionTable, PropFlags::PropFlag_DontEnum);    
    return true;
}

bool AsBroadcaster::InitializeInstance(ASStringContext* psc, ObjectInterface* pobj)
{
    if (!pobj)
        return false;

    Ptr<ArrayObject> ao = *SF_HEAP_NEW(psc->GetHeap()) ArrayObject(psc);
    pobj->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin__listeners), Value(ao), PropFlags::PropFlag_DontEnum);
    return true;
}

bool AsBroadcaster::AddListener(Environment* penv, ObjectInterface* pthis, ObjectInterface* plistener)
{
    if (!pthis || !plistener)
        return false;

    Value listenersVal;
    if (pthis->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin__listeners), &listenersVal))
    {
        Object* pobj = listenersVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object_Array)
        {
            Ptr<ArrayObject> parrObj = static_cast<ArrayObject*>(pobj);

            for (unsigned i = 0, n = parrObj->GetSize(); i < n; i++)
            {
                Value* pelem = parrObj->GetElementPtr(i);
                if (pelem && pelem->ToObjectInterface(penv) == plistener)
                {
                    // Already in the list.
                    return false;
                }
            }
            Value val;
            val.SetAsObjectInterface(plistener);
            parrObj->PushBack(val);
        }
    }
    return true;
}

bool AsBroadcaster::RemoveListener(Environment* penv, ObjectInterface* pthis, ObjectInterface* plistener)
{
    if (!pthis || !plistener)
        return false;

    Value listenersVal;
    if (pthis->GetMemberRaw(penv->GetSC(),
        penv->GetBuiltin(ASBuiltin__listeners), &listenersVal))
    {
        Object* pobj = listenersVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object_Array)
        {
            Ptr<ArrayObject> parrObj = static_cast<ArrayObject*>(pobj);

            for (int i = (int)parrObj->GetSize() - 1; i >= 0; --i)
            {
                Value* pelem = parrObj->GetElementPtr(i);
                if (pelem && pelem->ToObjectInterface(penv) == plistener)
                {
                    // Found in the list.
                    parrObj->RemoveElements(i, 1);
                    return true;
                }
            }
        }
    }
    return false;
}

bool AsBroadcaster::BroadcastMessage
    (Environment* penv, ObjectInterface* pthis, const ASString& eventName, int nArgs, int firstArgBottomIndex)
{
    if (!pthis)
        return false;
    SF_ASSERT(penv);
    class LocalInvokeCallback : public InvokeCallback
    {
    public:
        int NArgs;
        int FirstArgBottomIndex;

        LocalInvokeCallback(int na, int fa) : NArgs(na), FirstArgBottomIndex(fa) {}

        virtual void Invoke(Environment* penv, ObjectInterface* pthis, const FunctionRef& method)
        {
            Value result;
            method.Invoke(FnCall(&result, pthis, penv, NArgs, FirstArgBottomIndex));
        }
    } Callback(nArgs, firstArgBottomIndex);
    BroadcastMessageWithCallback(penv, pthis, eventName, &Callback);
    return true;
}

bool AsBroadcaster::BroadcastMessageWithCallback
(Environment* penv, ObjectInterface* pthis, const ASString& eventName, InvokeCallback* pcallback)
{
    if (!pthis)
        return false;
    SF_ASSERT(penv);

    Value listenersVal;
    if (pthis->GetMemberRaw(penv->GetSC(),
        penv->GetBuiltin(ASBuiltin__listeners), &listenersVal))
    {
        Object* pobj = listenersVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object_Array)
        {
            Ptr<ArrayObject> porigArrObj = static_cast<ArrayObject*>(pobj);
            if (porigArrObj->GetSize() > 0)
            {
                // AB: Seems like Flash does copy the _listeners array before broadcasting 
                // and iterate using the copy, since if one of the listeners removes the 
                // entry(ies) then broadcast still invokes the removed handler(s).
                Ptr<ArrayObject> parrObj = *SF_HEAP_NEW(penv->GetHeap()) ArrayObject(penv);
                parrObj->MakeDeepCopyFrom(penv->GetHeap(), *porigArrObj);

                for (unsigned i = 0, n = parrObj->GetSize(); i < n; i++)
                {
                    Value*           pelem = parrObj->GetElementPtr(i);
                    ObjectInterface* plistener;

                    if (pelem && (plistener = pelem->ToObjectInterface(penv)) != NULL)
                    {
                        Ptr<Object> objHolder;
                        Ptr<InteractiveObject> charHolder;
                        if (pelem->IsCharacter())
                            charHolder = pelem->ToCharacter(penv);
                        else
                            objHolder = pelem->ToObject(penv);

                        Value methodVal;
                        if (plistener->GetMemberRaw(penv->GetSC(),
                            eventName, &methodVal))
                        {
                            FunctionRef method = methodVal.ToFunction(penv);
                            if (!method.IsNull())
                                pcallback->Invoke(penv, plistener, method);
                        }
                    }
                }
            }
        }
    }
    return true;
}

void AsBroadcasterCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_AsBroadcaster && 
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        Ptr<AsBroadcaster> ab = static_cast<AsBroadcaster*>(fn.ThisPtr);
        fn.Result->SetAsObject(ab.GetPtr());
    }
    else
        fn.Result->SetNull();
}

Object* AsBroadcasterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) AsBroadcaster(penv);
}


FunctionRef AsBroadcasterCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) AsBroadcasterCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) AsBroadcasterProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_AsBroadcaster, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_AsBroadcaster), Value(ctor));
    return ctor;
}
}}} //SF::GFx::AS2
