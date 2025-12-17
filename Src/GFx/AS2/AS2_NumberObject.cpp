/**************************************************************************

Filename    :   AS2_NumberObject.cpp
Content     :   Number object functinality
Created     :   March 10, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS2_NumberObject.h"
#include "AS2_FunctionRef.h"
#include "GFx/GFx_ASUtils.h"
#include "Kernel/SF_MsgFormat.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

////////////
NumberObject::NumberObject(Environment* penv) 
: Object(penv), mValue(0)
{
    CommonInit (penv);
}

NumberObject::NumberObject(Environment* penv, Number val) 
: Object(penv), mValue(val)
{
    CommonInit(penv);
}

void NumberObject::CommonInit(Environment* penv)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Number));
}

const char* NumberObject::GetTextValue(Environment* penv) const
{
    SF_UNUSED(penv);
    char buf[NumberUtil::TOSTRING_BUF_SIZE];
    StringValue = NumberUtil::ToString(mValue, buf, sizeof(buf), 10);
    return StringValue.ToCStr();
}

void        NumberObject::SetValue(Environment* penv, const Value& v)
{
    mValue = v.ToNumber(penv);
}

Value    NumberObject::GetValue() const
{
    return Value(mValue);
}

const char* NumberObject::ToString(int radix) const
{
    char buf[NumberUtil::TOSTRING_BUF_SIZE];
    StringValue = NumberUtil::ToString(mValue, buf, sizeof(buf), radix);
    return StringValue.ToCStr();
}

bool NumberObject::InvokePrimitiveMethod(const FnCall& fnCall, const ASString& methodName)
{
    NumberObject* pthis = (NumberObject*) fnCall.ThisPtr;
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

////////////////////////
static void GAS_NumberToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Number);
    NumberObject* pthis = (NumberObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    int radix = 10;

    if (fn.NArgs > 0) {
        // special case if optional parameter (radix) is specified.
        radix = (int) fn.Arg(0).ToNumber(fn.Env);
    }

    fn.Result->SetString(fn.Env->CreateString(pthis->ToString(radix)));
}

static void GAS_NumberValueOf(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Number);
    NumberObject* pthis = (NumberObject*) fn.ThisPtr;
    SF_ASSERT(pthis);

    fn.Result->SetNumber (pthis->GetValue ().ToNumber(fn.Env));
}


static const NameFunction NumberFunctionTable[] = 
{
    { "toString",       &GAS_NumberToString  },
    { "valueOf",        &GAS_NumberValueOf  },
    { 0, 0 }
};

NumberProto::NumberProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<NumberObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, NumberFunctionTable);
}

////////////////////////////////////////
//
struct GASNameNumberFunc
{
    const char* Name;
    Double (SF_CDECL *Function)();
};

static const GASNameNumberFunc GASNumberConstTable[] = 
{
    { "MAX_VALUE",         NumberUtil::MAX_VALUE          },
    { "MIN_VALUE",         NumberUtil::MIN_VALUE          },
    { "NaN",               NumberUtil::NaN                },
    { "NEGATIVE_INFINITY", NumberUtil::NEGATIVE_INFINITY  },
    { "POSITIVE_INFINITY", NumberUtil::POSITIVE_INFINITY  },
    { 0, 0 }
};

NumberCtorFunction::NumberCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, GlobalCtor)
{
    for(int i = 0; GASNumberConstTable[i].Name; i++)
    {
        SetConstMemberRaw(psc, GASNumberConstTable[i].Name, Value(GASNumberConstTable[i].Function()),  
            PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    }
}

void NumberCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Number && 
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        NumberObject* nobj = static_cast<NumberObject*>(fn.ThisPtr);
        Value retVal = (fn.NArgs > 0)? fn.Arg(0) : Value();
        nobj->SetValue(fn.Env, retVal);
        *fn.Result = retVal;
    }
    else
        fn.Result->SetNumber((fn.NArgs == 0) ? Double(0.0) : fn.Arg(0).ToNumber(fn.Env));
}

Object* NumberCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) NumberObject(penv);
}


FunctionRef NumberCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) NumberCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) NumberProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Number, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Number), Value(ctor));
    return ctor;
}

}}} // SF::GFx::AS2
