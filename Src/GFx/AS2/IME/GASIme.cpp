/**********************************************************************

Filename    :   GFxSelection.cpp
Content     :   Implementation of IME class
Created     :   February, 2007
Authors     :   Ankur Mohan

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_String.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GASIme.h"
#include "GFx/GFx_TextField.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/IME/AS2_IMEManager.h"

#ifndef SF_NO_IME_SUPPORT

namespace Scaleform { namespace GFx { namespace AS2 {

GASIme::GASIme(Environment* penv) : Object(penv)
{
    commonInit(penv);
}

void GASIme::commonInit (Environment* penv)
{
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_Ime));
}

//////////////////////////////////////////
//
static void GFx_FuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GAS_ImeFunctionTable[] = 
{
    { "addListener",            &GFx_FuncStub },
    { "doConversion",           &GFx_FuncStub },
    { "getConversionMode",      &GFx_FuncStub },
    { "getEnabled",             &GFx_FuncStub },
    { "setCompositionString",   &GFx_FuncStub },
	{ "getCompositionString",   &GFx_FuncStub },
    { "setConversionMode",      &GFx_FuncStub },
    { "setEnabled",             &GFx_FuncStub },
    { "removeListener",         &GFx_FuncStub },
    { 0, 0 }
};

GASImeProto::GASImeProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
Prototype<GASIme>(psc, pprototype, constructor)
{
    InitFunctionMembers (psc, GAS_ImeFunctionTable);
}

void GASImeProto::GlobalCtor(const FnCall& fn)
{
    Ptr<GASIme> ab = *new GASIme(fn.Env);
    fn.Result->SetAsObject(ab.GetPtr());
}

const NameNumber GASImeCtorFunction::GASNumberConstTable[] = 
{
    { "ALPHANUMERIC_FULL",          0x00000000},
    { "ALPHANUMERIC_HALF",          0x00000001},
    { "CHINESE",                    0x00000002},
    { "JAPANESE_HIRAGANA",          0x00000004},
    { "JAPANESE_KATAKANA_FULL",     0x00000008},
    { "JAPANESE_KATAKANA_HALF",     0x00000016},
    { "KOREAN",                     0x00000032},
    { "UNKNOWN",                    0x00000064},
    { 0, 0 }
};

//////////////////
const NameFunction GASImeCtorFunction::StaticFunctionTable[] = 
{
    { "doConversion",           &GASImeCtorFunction::DoConversion },
    { "getConversionMode",      &GASImeCtorFunction::GetConversionMode },
    { "getEnabled",             &GASImeCtorFunction::GetEnabled },
    { "setCompositionString",   &GASImeCtorFunction::SetCompositionString },
	{ "getCompositionString",   &GASImeCtorFunction::GetCompositionString },
    { "setConversionMode",      &GASImeCtorFunction::SetConversionMode },
    { "setEnabled",             &GASImeCtorFunction::SetEnabled },
	{ "SendLangBarMessage",     &GASImeCtorFunction::SendLangBarMessage },
    { 0, 0 }
};

GASImeCtorFunction::GASImeCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, ObjectProto::GlobalCtor)
{
    AsBroadcaster::Initialize(psc, this);

    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);

    for(int i = 0; GASNumberConstTable[i].Name; i++)
    {
        SetConstMemberRaw(psc, GASNumberConstTable[i].Name, Value(GASNumberConstTable[i].Number), 
            PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    }
}

void GASImeCtorFunction::DoConversion(const FnCall& fn)
{
    SF_UNUSED(fn);
}

void GASImeCtorFunction::GetConversionMode(const FnCall& fn)
{
    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    ASString resultStr = fn.Env->GetGC()->CreateConstString("UNKNOWN");

    if(pimeManager)
    {
        resultStr = pimeManager->GetConversionMode();
    }

    fn.Result->SetString(resultStr);

}

void GASImeCtorFunction::SetCompositionString(const FnCall& fn)
{
    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    bool result = false; 
    if(pimeManager)
        result = pimeManager->SetCompositionString(fn.Arg(0).ToString(fn.Env).ToCStr());

    fn.Result->SetBool(result);
}

void GASImeCtorFunction::GetCompositionString(const FnCall& fn)
{
	if (!fn.Env)
		return;

	// Try to get IME Manager.
	Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
	const wchar_t* result = NULL; 
	if(pimeManager)
		result = pimeManager->GetCompositionString();

	fn.Result->SetString(fn.Env->GetMovieImpl()->GetStringManager()->CreateString(result));
}

void GASImeCtorFunction::SetConversionMode(const FnCall& fn)
{
    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    bool result = false;
    if(pimeManager)
        result = pimeManager->SetConversionMode((unsigned)(fn.Arg(0).ToNumber(fn.Env)));

    fn.Result->SetBool(result);
}

void GASImeCtorFunction::SetEnabled(const FnCall& fn)
{

    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    bool result = false;
    if(pimeManager)
        result = pimeManager->SetEnabled(fn.Arg(0).ToBool(fn.Env));

    fn.Result->SetBool(result);
}

void GASImeCtorFunction::GetEnabled(const FnCall& fn)
{

    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    if(pimeManager)
        fn.Result->SetBool(pimeManager->GetEnabled());
}

void GASImeCtorFunction::SendLangBarMessage(const FnCall& fn)
{

	if (!fn.Env)
		return;

	// Try to get IME Manager.
	Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
	if(pimeManager)
	{
		if (fn.NArgs == 3)
		{
			Value::type tp = fn.Arg(0).GetType();
			if (tp == AS2::Value::CHARACTER)
			{
				InteractiveObject* pobj = fn.Arg(0).ToCharacter(fn.Env);
				ASString command = fn.Arg(1).ToString(fn.Env);
				ASString message = fn.Arg(2).ToString(fn.Env);
				pimeManager->SendLangBarMessage(pobj, command, message);
			}
		}		
	}
	fn.Result->SetUndefined();
}

FunctionRef GASImeCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) GASImeCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) GASImeProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Ime, proto);
    pgc->SystemPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Ime), Value(ctor));
    return ctor;
}

void GASIme::BroadcastOnIMEComposition(Environment* penv, const ASString& compString)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!compString.IsEmpty())
                    penv->Push(Value(compString));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onIMEComposition"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSwitchLanguage(Environment* penv, const ASString& compString)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!compString.IsEmpty())
                    penv->Push(Value(compString));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSwitchLanguage"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSetSupportedLanguages(Environment* penv, const ASString& supportedLangs)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!supportedLangs.IsEmpty())
                    penv->Push(Value(supportedLangs));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSetSupportedLanguages"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSetSupportedIMEs(Environment* penv, const ASString& supportedImes)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!supportedImes.IsEmpty())
                    penv->Push(Value(supportedImes));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSetSupportedIMEs"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSetCurrentInputLang(Environment* penv, const ASString& currentInputLang)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!currentInputLang.IsEmpty())
                    penv->Push(Value(currentInputLang));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSetCurrentInputLanguage"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSetIMEName(Environment* penv, const ASString& imeName)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!imeName.IsEmpty())
                    penv->Push(Value(imeName));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSetIMEName"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnSetConversionMode(Environment* penv, const ASString& convStatus)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                if (!convStatus.IsEmpty())
                    penv->Push(Value(convStatus));
                else
                    penv->Push(Value::NULLTYPE);

                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onSetConversionStatus"), 1, penv->GetTopIndex());
                penv->Drop(1);
            }
        }
    }  
}

void GASIme::BroadcastOnRemoveStatusWindow(Environment* penv)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onRemoveStatusWindow"), 0, penv->GetTopIndex());
            }
        }
    }  
}

void GASIme::BroadcastOnDisplayStatusWindow(Environment* penv)
{
    Value imeCtorVal;
    Value systemPackage;
    ASString systemPackageStr = penv->GetGC()->CreateConstString("System");

    if(penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), systemPackageStr, &systemPackage))
    {
        Object* psystemPackage = systemPackage.ToObject(penv);

        if (psystemPackage->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Ime), &imeCtorVal))
        {
            ObjectInterface* pimeObj = imeCtorVal.ToObject(penv);
            if (pimeObj)
            {
                AsBroadcaster::BroadcastMessage(penv, pimeObj, penv->CreateConstString("onDisplayStatusWindow"), 0, penv->GetTopIndex());
            }
        }
    }  
}
}}}
#endif // SF_NO_IME_SUPPORT
