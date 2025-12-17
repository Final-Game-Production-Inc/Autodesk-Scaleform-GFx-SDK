/**************************************************************************

Filename    :   AS2_BooleanObject.cpp
Content     :   Boolean object functinality
Created     :   April 10, 2006
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_BooleanObject.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

BooleanObject::BooleanObject(Environment* penv) 
: Object(penv), bValue (0)
{
    CommonInit(penv);
}

BooleanObject::BooleanObject(Environment* penv, bool v) 
: Object(penv), bValue (v)
{
    CommonInit(penv);
}

void BooleanObject::CommonInit(Environment* penv)
{ 
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Boolean));
}

const char* BooleanObject::GetTextValue(Environment*) const
{
    return (bValue) ? "true" : "false";
}

bool BooleanObject::InvokePrimitiveMethod(const FnCall& fnCall, const ASString& methodName)
{
    BooleanObject* pthis = (BooleanObject*) fnCall.ThisPtr;
    SF_ASSERT(pthis);

    // if (methodName == "toString" || methodName == "valueOf")
    if (fnCall.Env->GetBuiltin(ASBuiltin_toString).CompareBuiltIn_CaseCheck(methodName, fnCall.Env->IsCaseSensitive()) ||
        fnCall.Env->GetBuiltin(ASBuiltin_valueOf).CompareBuiltIn_CaseCheck(methodName, fnCall.Env->IsCaseSensitive()) )
    {
        Value method;
        if (pthis->GetMemberRaw(fnCall.Env->GetSC(), methodName, &method)) 
        {
            FunctionRef  func = method.ToFunction(fnCall.Env);
            if (!func.IsNull())
            {
                // It's a C function.  Call it.
                func.Invoke(fnCall);
                return true;
            }
        }
    }
    fnCall.Result->SetUndefined();
    return false;
}

void        BooleanObject::SetValue(Environment* penv, const Value& v)
{
    bValue = v.ToBool(penv);
}

Value    BooleanObject::GetValue() const
{
    return Value(bValue);
}

////////////////////////
static void GAS_BooleanToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Boolean);
    BooleanObject* pthis = (BooleanObject*) fn.ThisPtr;
    SF_ASSERT(pthis);

    fn.Result->SetString(pthis->GetValue().ToString(fn.Env));
}

static void GAS_BooleanValueOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Boolean);
    BooleanObject* pthis = (BooleanObject*) fn.ThisPtr;
    SF_ASSERT(pthis);

    fn.Result->SetBool(pthis->GetValue().ToBool(fn.Env));
}


static const NameFunction GAS_BooleanFunctionTable[] = 
{
    { "toString",       &GAS_BooleanToString  },
    { "valueOf",        &GAS_BooleanValueOf  },
    { 0, 0 }
};

BooleanProto::BooleanProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<BooleanObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_BooleanFunctionTable);
}

BooleanCtorFunction::BooleanCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

Object* BooleanCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) BooleanObject(penv);
}

void BooleanCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Boolean &&
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        BooleanObject* nobj = static_cast<BooleanObject*>(fn.ThisPtr);
        Value retVal = (fn.NArgs > 0)? fn.Arg(0) : Value();
        nobj->SetValue(fn.Env, retVal);
        *fn.Result = retVal;
    }
    else
    {
        if (fn.NArgs == 0)
            fn.Result->SetBool(0);
        else
            fn.Result->SetBool(fn.Arg(0).ToBool(fn.Env));
    }
}

FunctionRef BooleanCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) BooleanCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) BooleanProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Boolean, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Boolean), Value(ctor));
    return ctor;
}
}}} // SF::GFx::AS2
