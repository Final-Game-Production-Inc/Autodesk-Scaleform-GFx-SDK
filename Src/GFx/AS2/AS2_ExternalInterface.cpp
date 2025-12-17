/**************************************************************************

Filename    :   AS2_ExternalInterface.cpp
Content     :   ExternalInterface AS class implementation
Created     :   
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_ExternalInterface.h"
#include "AS2_FunctionRef.h"
#include "AS2_Value.h"
#include "AS2_MovieRoot.h"

#include <stdio.h>
#include <stdlib.h>

#define GFX_MAX_NUM_OF_VALUES_ON_STACK 10

namespace Scaleform { namespace GFx { namespace AS2 {

ExternalInterface::ExternalInterface(Environment* penv)
: Object(penv)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_ExternalInterface));
}


static const NameFunction GFx_EIFunctionTable[] = 
{
    { 0, 0 }
};

ExternalInterfaceProto::ExternalInterfaceProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<ExternalInterface>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GFx_EIFunctionTable);
}

const NameFunction ExternalInterfaceCtorFunction::StaticFunctionTable[] = 
{
    { "addCallback", &ExternalInterfaceCtorFunction::AddCallback },
    { "call",        &ExternalInterfaceCtorFunction::Call   },
    { 0, 0 }
};

ExternalInterfaceCtorFunction::ExternalInterfaceCtorFunction(ASStringContext *psc) : 
    CFunctionObject(psc, GlobalCtor)
{ 
    SetConstMemberRaw(psc, "available", Value(Value::UNSET));

    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

bool    ExternalInterfaceCtorFunction::GetMember(Environment *penv, const ASString& name, Value* val)
{
    if (penv->GetSC()->CompareConstString_CaseCheck(name, "available"))
    {
        // "available" returns "true" if external interface handler is set.
        MovieImpl* proot = penv->GetMovieImpl();
        SF_ASSERT(proot);

        val->SetBool((proot->pExtIntfHandler)?true:false);
        return true;
    }
    return FunctionObject::GetMember(penv, name, val);
}

void    ExternalInterfaceCtorFunction::AddCallback(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs < 3)
        return;

    MovieImpl* proot = fn.Env->GetMovieImpl();
    SF_ASSERT(proot);

    ASString methodName     = fn.Arg(0).ToString(fn.Env);
    FunctionRef  function   = fn.Arg(2).ToFunction(fn.Env);
    Ptr<CharacterHandle>    thisChar;
    Ptr<Object>             thisObject;

    if (fn.Arg(1).IsCharacter())
    {
        Ptr<InteractiveObject> pchar = fn.Arg(1).ToCharacter(fn.Env);
        if (pchar)
            thisChar = pchar->GetCharacterHandle();
    }
    else
        thisObject = fn.Arg(1).ToObject(fn.Env);

    ToAS2Root(proot)->AddInvokeAlias(methodName, thisChar, thisObject, function);

    fn.Result->SetBool(true);
}

void    ExternalInterfaceCtorFunction::Call(const FnCall& fn)
{
    MovieImpl* proot = fn.Env->GetMovieImpl();
    SF_ASSERT(proot);

    if (proot->pExtIntfHandler)
    {
        unsigned nArgs = 0;
        ASString methodName(fn.Env->GetBuiltin(ASBuiltin_empty_));
        if (fn.NArgs >= 1)
        {
            methodName = fn.Arg(0).ToString(fn.Env);
            nArgs = fn.NArgs - 1;
        }

        // convert arguments into Value, starting from index 1
        void* argArrayOnStack[GFX_MAX_NUM_OF_VALUES_ON_STACK*((sizeof(GFx::Value)+sizeof(void*)-1)/sizeof(void*))];
        GFx::Value* pargArray;
        if (nArgs > GFX_MAX_NUM_OF_VALUES_ON_STACK)
            pargArray = (GFx::Value*)
                SF_HEAP_ALLOC(fn.Env->GetHeap(), sizeof(GFx::Value) * nArgs, StatMV_ActionScript_Mem);
        else
            pargArray = (GFx::Value*)argArrayOnStack;

        // convert params to Value array
        for (unsigned i = 0, n = nArgs; i < n; ++i)
        {
            const Value& val = fn.Arg(i + 1);
            GFx::Value* pdestVal = Construct<GFx::Value>(&pargArray[i]);
            ToAS2Root(proot)->ASValue2Value(fn.Env, val, pdestVal);
        }
        
        ToAS2Root(proot)->ExternalIntfRetVal.SetUndefined();

        proot->pExtIntfHandler->Callback
            (proot, (methodName.IsEmpty()) ? NULL : methodName.ToCStr(), pargArray, nArgs);

        *fn.Result = ToAS2Root(proot)->ExternalIntfRetVal;

        // destruct elements in Value array
        for (unsigned i = 0, n = nArgs; i < n; ++i)
        {
            pargArray[i].~Value();
        }
        if (nArgs > sizeof(argArrayOnStack)/sizeof(argArrayOnStack[0]))
        {
            SF_HEAP_FREE(fn.Env->GetHeap(), pargArray);
        }
    }
    else
    {
        fn.LogScriptWarning("ExternalInterface.call - handler is not installed.");
        fn.Result->SetUndefined();
    }
}

void ExternalInterfaceCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

Object* ExternalInterfaceCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) ExternalInterface(penv);
}


FunctionRef ExternalInterfaceCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) ExternalInterfaceCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ExternalInterfaceProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_ExternalInterface, proto);
    pgc->FlashExternalPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_ExternalInterface), Value(ctor));
    return ctor;
}
}}} // SF::GFx::AS2
