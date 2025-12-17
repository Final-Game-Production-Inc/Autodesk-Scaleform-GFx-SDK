/**************************************************************************

Filename    :   AS2_AmpMarker.cpp
Content     :   Implementation of AMP marker class
Created     :   May, 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_AmpMarker.h"
#ifdef SF_AMP_SERVER
#include "AS2_Value.h"
#include "GFx/AMP/Amp_ViewStats.h"

namespace Scaleform { namespace GFx { namespace AS2 {


AmpMarker::AmpMarker(Environment* env) : Object(env)
{
    commonInit(env);
}

void AmpMarker::commonInit(Environment* env)
{
    Set__proto__(env->GetSC(), env->GetPrototype(ASBuiltin_Amp));
}

AmpMarkerProto::AmpMarkerProto(ASStringContext* sc, Object* prototype, const FunctionRef& constructor) : 
    Prototype<AmpMarker>(sc, prototype, constructor)
{
}

//////////////////
const NameFunction AmpMarkerCtorFunction::StaticFunctionTable[] = 
{
    { "addMarker",   &AmpMarkerCtorFunction::AddMarker },
    { 0, 0 }
};

AmpMarkerCtorFunction::AmpMarkerCtorFunction(ASStringContext *sc) : CFunctionObject(sc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, sc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void AmpMarkerCtorFunction::AddMarker(const FnCall& fn)
{
    fn.Result->SetNull();
    if (!fn.Env || fn.NArgs == 0)
    {
        return;
    }

    fn.Env->GetMovieImpl()->GetAdvanceStats().AddMarker(fn.Arg(0).ToString(fn.Env).ToCStr());
}

void AmpMarkerCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}

bool AmpMarkerCtorFunction::GetMember(Environment* env, const ASString& name, Value* val)
{
    if (name == "addMarker") 
    {
        *val = Value(env->GetSC(), AmpMarkerCtorFunction::AddMarker);
        return true;
    }
    return FunctionObject::GetMember(env, name, val);
}

bool AmpMarkerCtorFunction::SetMember(Environment* env, const ASString& name, const Value& val, const PropFlags& flags)
{
    MovieImpl* proot = env->GetMovieImpl();
    if (name == "addMarker")
    {
        proot->GetAdvanceStats().AddMarker(val.ToString(env).ToCStr());
        return true;
    }
    return FunctionObject::SetMember(env, name, val, flags);
}


FunctionRef AmpMarkerCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) AmpMarkerCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) AmpMarkerProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Amp, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Amp), Value(ctor));
    return ctor;
}

}}} // SF::GFx::AS2

#endif
