/**************************************************************************

Filename    :   AS2_Math.cpp
Content     :   AS2 Implementation of Math class
Created     :   January 6, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Math.h"
#include "GFx/AS2/AS2_Action.h"
#include "Kernel/SF_Types.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx { namespace AS2 {

Math::Math(Environment* penv)
: Object(penv)
{
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_Math));
}

UInt32 Math::GetNextRandom(MovieImpl* proot)
{
    UInt32 rn;
    TestStream* pts = proot->GetTestStream();
    if (pts)
    {
        if (pts->TestStatus == TestStream::Record)
        {
            rn =  Alg::Random::NextRandom();

            LongFormatter f(rn);
            f.Convert();
            pts->SetParameter("random", f.ToCStr());
        }
        else
        {
            String tstr;
            pts->GetParameter("random", &tstr);
            rn = SFstrtoul(tstr.ToCStr(), NULL, 10);
        }
    }
    else
        rn = Alg::Random::NextRandom();

    return rn;
}
    
//////////////////////////////////////////
//

MathProto::MathProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<Math>(psc, pprototype, constructor)
{
    // Needed? Check back to GASSelection code
    //InitFunctionMembers (psc, GAS_MathFunctionTable);
}

//////////////////

const NameFunction MathCtorFunction::StaticFunctionTable[] = 
{
    { "abs",    &MathCtorFunction::Abs },
    { "acos",    &MathCtorFunction::Acos },
    { "asin",    &MathCtorFunction::Asin },
    { "atan",   &MathCtorFunction::Atan },
    { "atan2",  &MathCtorFunction::Atan2 },
    { "ceil",    &MathCtorFunction::Ceil },
    { "cos",    &MathCtorFunction::Cos },
    { "exp",    &MathCtorFunction::Exp },
    { "floor",    &MathCtorFunction::Floor },
    { "log",    &MathCtorFunction::Log },
    { "max",    &MathCtorFunction::Max },
    { "min",    &MathCtorFunction::Min },
    { "pow",    &MathCtorFunction::Pow },
    { "random",    &MathCtorFunction::Random },
    { "round",    &MathCtorFunction::Round },
    { "sin",    &MathCtorFunction::Sin },
    { "sqrt",    &MathCtorFunction::Sqrt },
    { "tan",    &MathCtorFunction::Tan },
    { 0, 0 }
};

MathCtorFunction::MathCtorFunction(ASStringContext *psc)
:   CFunctionObject(psc, GlobalCtor)
{
    // constant
    this->SetConstMemberRaw(psc, "E",       (Double)2.7182818284590452354    );
    this->SetConstMemberRaw(psc, "LN2",     (Double)0.69314718055994530942   );
    this->SetConstMemberRaw(psc, "LOG2E",   (Double)1.4426950408889634074    );
    this->SetConstMemberRaw(psc, "LN10",    (Double)2.30258509299404568402   );
    this->SetConstMemberRaw(psc, "LOG10E",  (Double)0.43429448190325182765   );
    this->SetConstMemberRaw(psc, "PI",      (Double)3.14159265358979323846   );
    this->SetConstMemberRaw(psc, "SQRT1_2", (Double)0.7071067811865475244    );
    this->SetConstMemberRaw(psc, "SQRT2",   (Double)1.4142135623730950488    );

    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void MathCtorFunction::Random(const FnCall& fn)
{
    // Random number between 0 and 1.
    fn.Result->SetNumber(Math::GetNextRandom(fn.Env->GetMovieImpl())/ double(UInt32(0xFFFFFFFF)));
}

void MathCtorFunction::Round(const FnCall& fn)
{
    // round argument to nearest int.
    Number   arg0 = fn.Arg(0).ToNumber(fn.Env);
    fn.Result->SetNumber((Number)floor(arg0 + 0.5f));
}

void MathCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}

FunctionRef MathCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) MathCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) MathProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Math, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Math), Value(ctor));
    return ctor;
}
}}} //SF::GFx::AS2
