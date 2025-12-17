/**************************************************************************

Filename    :   AS2_LoadVars.cpp
Content     :   LoadVars reference class for ActionScript 2.0
Created     :   3/28/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_LoadVars.h"
#ifdef GFX_AS2_ENABLE_LOADVARS
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_ASUtils.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ****************************************************************************
// GASLoadVars constructors
//
LoadVarsObject::LoadVarsObject(Environment *penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_LoadVars));
    AsBroadcaster::Initialize(penv->GetSC(), this);

    CommonInit();

    // @TODO
    // NOTE: The following call is REQUIRED.
    // It's causing memory leaks because of circular refs. Look for a fix
    AsBroadcaster::AddListener(penv, this, this);
    
    Object::SetMemberRaw(penv->GetSC(), penv->CreateConstString("contentType"), penv->CreateConstString("application/x-www-form-urlencoded"), PropFlags::PropFlag_DontEnum);
    Object::SetMemberRaw(penv->GetSC(), penv->CreateConstString("loaded"), Value(Value::UNDEFINED), PropFlags::PropFlag_DontEnum);
}

// ****************************************************************************
// Intercept to force enumeration for all properties set 
//
bool LoadVarsObject::SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags)
{
    SF_UNUSED(flags);
    Object::SetMemberFlags(penv->GetSC(), name, 0);
    return Object::SetMember(penv, name, val);
}

// ****************************************************************************
// Invoked when data has completely downloaded from the server or when an 
// error occurs while data is downloading from a server.
//
void LoadVarsObject::NotifyOnData(Environment* penv, ASString& src)
{
    penv->Push(src);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onData"), 1, penv->GetTopIndex());
    penv->Drop1();
}

// ****************************************************************************
// Invoked when Flash Player receives an HTTP status code from the server
//
void LoadVarsObject::NotifyOnHTTPStatus(Environment* penv, Number httpStatus)
{
    penv->Push(httpStatus);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onHTTPStatus"), 1, penv->GetTopIndex());
    penv->Drop1();    
}

// ****************************************************************************
// Invoked when a LoadVars.load() or LoadVars.sendAndLoad() operation has ended
//
void LoadVarsObject::NotifyOnLoad(Environment* penv, bool success)
{
    penv->Push(success);
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onLoad"), 1, penv->GetTopIndex());
    penv->Drop1();    
}

// ****************************************************************************
// AS to GFx function mapping
//
const NameFunction LoadVarsProto::FunctionTable[] = 
{
    { "addRequestHeader", &LoadVarsProto::AddRequestHeader },
    { "decode", &LoadVarsProto::Decode },
    { "getBytesLoaded", &LoadVarsProto::GetBytesLoaded },
    { "getBytesTotal", &LoadVarsProto::GetBytesTotal },
    { "load", &LoadVarsProto::Load },
    { "send", &LoadVarsProto::Send },
    { "sendAndLoad", &LoadVarsProto::SendAndLoad },
    { "toString", &LoadVarsProto::ToString },
    { 0, 0 }
};

// ****************************************************************************
// GASLoadVars prototype constructor
//
LoadVarsProto::LoadVarsProto(ASStringContext *psc, Object *pprototype, const FunctionRef &constructor) : 
Prototype<LoadVarsObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable);
    SetConstMemberRaw(psc, "onData", Value(psc, &LoadVarsProto::DefaultOnData),
                      PropFlags::PropFlag_DontEnum);
}

// ****************************************************************************
// LoadVars.addRequestHeader method
//
void LoadVarsProto::AddRequestHeader(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    // Adds or changes HTTP request headers (such as Content-Type or 
    // SOAPAction) sent with POST actions. In the first usage, you pass two 
    // strings to the method: header and headerValue. In the second usage, you 
    // pass an array of strings, alternating header names and header values. 
    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "LoadVars.addRequestHeader is not implemented.");
}

// ****************************************************************************
// LoadVars.decode method
//
void LoadVarsProto::Decode(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        CHECK_THIS_PTR(fn, LoadVars);
        LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
        ASString urlStr(fn.Arg(0).ToString(fn.Env));
        String data;
        ASUtils::Unescape(urlStr.ToCStr(), urlStr.GetLength(), &data);
        LoadVariables(fn.Env, pthis, data);
    }
}

// ****************************************************************************
// LoadVars.getBytesLoaded method
//
void LoadVarsProto::GetBytesLoaded(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
    if (pthis->BytesLoadedCurrent < 0)
    {
        fn.Result->SetUndefined();
    }
    else
    {
        fn.Result->SetNumber(pthis->BytesLoadedCurrent);
    }
}

// ****************************************************************************
// LoadVars.getBytesLoaded method
//
void LoadVarsProto::GetBytesTotal(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
    if (pthis->BytesLoadedTotal < 0)
    {
        fn.Result->SetUndefined();
    }
    else
    {
        fn.Result->SetNumber(pthis->BytesLoadedTotal);
    }
}

// ****************************************************************************
// LoadVars.load method
//
void LoadVarsProto::Load(const FnCall& fn)
{
    if (fn.NArgs == 0)
    {
        fn.Result->SetBool(false);
        return;
    }
    CHECK_THIS_PTR(fn, LoadVars);
    LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
    ASString urlStr(fn.Arg(0).ToString(fn.Env));
    
    pthis->BytesLoadedCurrent = 0;
    fn.Env->GetAS2Root()->AddVarLoadQueueEntry(pthis, urlStr.ToCStr(), 
        LoadQueueEntry::LM_None);
    fn.Result->SetBool(true);
}

// ****************************************************************************
// LoadVars.send method
//
void LoadVarsProto::Send(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "LoadVars.send is not implemented.");
}

// ****************************************************************************
// LoadVars.sendAndLoad method
//
void LoadVarsProto::SendAndLoad(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    Log* log = fn.GetLog();
    if (log != NULL)
        log->LogMessageById(Log_ScriptError, "LoadVars.sendAndLoad is not implemented.");
}

// ****************************************************************************
// LoadVars.toString method
//
void LoadVarsProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
    SF_ASSERT(pthis);

    String str;
    struct MemberVisitor : ObjectInterface::MemberVisitor
    {
        Environment* pEnvironment;
        String* pString;

        MemberVisitor(Environment *penv, String *str)
        {
            pEnvironment = penv;
            pString = str;
        }

        virtual void Visit(const ASString& name, const Value& val, UByte flags)
        {
            SF_UNUSED(flags);

            StringBuffer buf;
            String tmp;
            ASUtils::Escape(name.ToCStr(), name.GetSize(), &tmp);
            buf += tmp;
            buf += "=";
            tmp.Clear();
            ASString t = val.ToString(pEnvironment);
           // ASUtils::Escape(str.ToCStr(), str.GetLength(), &tmp);
            ASUtils::Escape(t.ToCStr(), t.GetSize(), &tmp);
            buf += tmp;
            buf += "&";
            *pString = buf;
        }
    } visitor(fn.Env, &str);
    pthis->VisitMembers(fn.Env->GetSC(), &visitor, 0);
    str.Remove(str.GetLength()-1); //here the use of GetLength() seems appropriate
    fn.Result->SetString(fn.Env->CreateString(str));
}

// ****************************************************************************
// Default LoadVars.onData method
//
void LoadVarsProto::DefaultOnData(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, LoadVars);
    // calls onLoad
    LoadVarsObject* pthis = static_cast<LoadVarsObject*>(fn.ThisPtr);
    String str( fn.Arg(0).ToString(fn.Env).ToCStr() );
    pthis->NotifyOnLoad(fn.Env, LoadVariables(fn.Env, pthis, fn.Arg(0).ToString(fn.Env).ToCStr()));
}


// ****************************************************************************
// Helper function to load variables from an unescaped string
//
bool LoadVarsProto::LoadVariables(Environment *penv, ObjectInterface* pchar, const String& data)
{
    StringBuffer      name, value;

    if (data.GetSize() == 0)
        return false;

    // parser state machine
    bool parseName = true;

    const char* pstr;
    UInt32 c = data.GetFirstCharAt(0, &pstr);
    while (c)
    {
        if ((UInt32)'\r' == c)
        {
            // flash converts LF to CR, but console printing
            // required LFs
            c = '\n';
        }
        if ((UInt32)'&' == c)
        {
            pchar->SetMember(penv, penv->CreateString(name.ToCStr(), name.GetSize()), 
                                   penv->CreateString(value.ToCStr(),value.GetSize()) );
            name.Clear();
            value.Clear();
            parseName = true;
        }
        else if (parseName)
        {
            if ((UInt32)'=' == c)
            {
                parseName = false;
            }
            else
            {
                name.AppendChar(c);
            }
        }
        else
        {
            value.AppendChar(c);
        }
        c = data.GetNextChar(&pstr);
    }

    // clean up stragglers
    if (name.GetLength() > 0)
    {
        pchar->SetMember(penv, penv->CreateString(name.ToCStr(), name.GetSize()), 
                               penv->CreateString(value.ToCStr(),value.GetSize()) );
    }    
    return true;
}

//////////////////
GASLoadVarsLoaderCtorFunction::GASLoadVarsLoaderCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
}

void GASLoadVarsLoaderCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_LoadVars && 
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        LoadVarsObject* ab = static_cast<LoadVarsObject*>(fn.ThisPtr);
        fn.Result->SetAsObject(ab);
    }
    else
        fn.Result->SetNull();
}

Object* GASLoadVarsLoaderCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) LoadVarsObject(penv);
}

FunctionRef GASLoadVarsLoaderCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) GASLoadVarsLoaderCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) LoadVarsProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_LoadVars, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_LoadVars), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS
