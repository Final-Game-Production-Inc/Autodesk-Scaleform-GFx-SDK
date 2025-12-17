/**************************************************************************

Filename    :   AS2_GlowFilter.cpp
Content     :   GlowFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_GlowFilter.h"

#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// GlowFilterObject::GlowFilterObject()
// : BitmapFilterObject(FilterDesc::Filter_Glow)
// {
// }

GlowFilterObject::GlowFilterObject(Environment* penv)
: BitmapFilterObject(penv, Render::Filter_Glow)
{
}

GlowFilterObject::~GlowFilterObject()
{
}

bool GlowFilterObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags /* = PropFlags */)
{
    if (name == "alpha")
    {
        Number alpha = val.ToNumber(penv);
        SetAlpha((float)alpha);
        return true;
    }
    else if (name == "blurX")
    {
        SetBlurX((float)val.ToNumber(penv));
        return true;
    }
    else if (name == "blurY")
    {
        SetBlurY((float)val.ToNumber(penv));
        return true;
    }
    else if (name == "color")
    {
        SetColor(val.ToUInt32(penv));
        return true;
    }
    else if (name == "inner")
    {
        SetInnerShadow(val.ToBool(penv));
       return true;
    }
    else if (name == "knockout")
    {
        SetKnockOut(val.ToBool(penv));
        return true;
    }
    else if (name == "quality")
    {
        SetPasses((UInt32)val.ToNumber(penv));
        return true;
    }
    else if (name == "strength")
    {
        SetStrength((float)val.ToNumber(penv));
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

bool GlowFilterObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name == "alpha")
    {
        val->SetNumber(GetAlpha());
        return true;
    }
    else if (name == "blurX")
    {
        val->SetNumber(GetBlurX());
        return true;
    }
    else if (name == "blurY")
    {
        val->SetNumber(GetBlurY());
        return true;
    }
    else if (name == "color")
    {
        val->SetUInt(GetColor());
        return true;
    }
    else if (name == "inner")
    {
        val->SetBool(IsInnerShadow());
        return true;
    }
    else if (name == "knockout")
    {
        val->SetBool(IsKnockOut());
        return true;
    }
    else if (name == "quality")
    {
        val->SetInt(GetPasses());
        return true;
    }
    else if (name == "strength")
    {
        val->SetNumber(GetStrength());
        return true;
    }
    return Object::GetMember(penv, name, val);
}


// --------------------------------------------------------------------

const NameFunction GlowFilterProto::FunctionTable[] = 
{
    { "clone",         &GlowFilterProto::Clone },
    { 0, 0 }
};

GlowFilterProto::GlowFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<GlowFilterObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, 0);
}

void GlowFilterProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, GlowFilter);
    GlowFilterObject* pthis = (GlowFilterObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<Object> pfilter = *fn.Env->OperatorNew(fn.Env->GetGC()->FlashFiltersPackage, 
        fn.Env->GetBuiltin(ASBuiltin_GlowFilter), 0);
    GlowFilterObject* pnew = static_cast<GlowFilterObject*>(pfilter.GetPtr());
    pnew->SetFilter((BlurFilterImpl*)pthis->GetFilter()->Clone(Memory::GetHeapByAddress(pthis)));    
    fn.Result->SetAsObject(pfilter);
}


// --------------------------------------------------------------------


GlowFilterCtorFunction::GlowFilterCtorFunction(ASStringContext* psc) :
CFunctionObject(psc, GlowFilterCtorFunction::GlobalCtor)
{
    SF_UNUSED(psc);
}

void GlowFilterCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<GlowFilterObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_GlowFilter)
        pnode = static_cast<GlowFilterObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) GlowFilterObject(fn.Env);
    fn.Result->SetAsObject(pnode.GetPtr());

    pnode->SetDistance(0);
    pnode->SetAngle(0);
    pnode->SetColor(Color::Red);
    pnode->SetAlpha(1.0f);
    pnode->SetBlurX(6.0f);
    pnode->SetBlurY(6.0f);
    pnode->SetStrength(2.0f);
    pnode->SetKnockOut(false);
    pnode->SetHideObject(false);

    if (fn.NArgs > 0)
    {
        // process color
        pnode->SetColor(fn.Arg(0).ToUInt32(fn.Env));

        if (fn.NArgs > 1)
        {
            // process alpha
            pnode->SetAlpha((float)fn.Arg(1).ToNumber(fn.Env));

            if (fn.NArgs > 2)
            {
                // process blurX
                pnode->SetBlurX((float)fn.Arg(2).ToNumber(fn.Env));

                if (fn.NArgs > 3)
                {
                    // process blurY
                    pnode->SetBlurY((float)fn.Arg(3).ToNumber(fn.Env));

                    if (fn.NArgs > 4)
                    {
                        // process strength 
                        pnode->SetStrength((float)fn.Arg(4).ToNumber(fn.Env));

                        if (fn.NArgs > 5)
                        {
                            // process quality 
                            pnode->SetPasses((UInt32)fn.Arg(5).ToNumber(fn.Env));

                            if (fn.NArgs > 6)
                            {
                                // process inner
                                pnode->SetInnerShadow(fn.Arg(6).ToBool(fn.Env));

                                if (fn.NArgs > 7)
                                {
                                    // process knockout
                                    pnode->SetKnockOut(fn.Arg(7).ToBool(fn.Env));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ASStringContext* psc = fn.Env->GetSC();
    pnode->SetMemberRaw(psc, psc->CreateConstString("color"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("alpha"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurX"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurY"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("strength"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("knockout"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("inner"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("quality"), Value::UNSET);
}

Object* GlowFilterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) GlowFilterObject(penv);
}

FunctionRef GlowFilterCtorFunction::Register(GlobalContext* pgc)
{
    // Register BitmapFilter here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_BitmapFilter)))
        BitmapFilterCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) GlowFilterCtorFunction(&sc));
    Ptr<GlowFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) GlowFilterProto(&sc, pgc->GetPrototype(ASBuiltin_BitmapFilter), ctor);
    pgc->SetPrototype(ASBuiltin_GlowFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_GlowFilter), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2
#endif  // SF_NO_FXPLAYER_AS_FILTERS
