/**************************************************************************

Filename    :   AS2_DropShadowFilter.cpp
Content     :   DropShadowFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_DropShadowFilter.h"
#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// DropShadowFilterObject::DropShadowFilterObject()
// : BitmapFilterObject(FilterDesc::Filter_DropShadow)
// {
// }

DropShadowFilterObject::DropShadowFilterObject(Environment* penv)
: BitmapFilterObject(penv, Render::Filter_Shadow)
{
}

DropShadowFilterObject::~DropShadowFilterObject()
{
}

bool DropShadowFilterObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags /* = PropFlags */)
{
    if (name == "alpha")
    {
        Number alpha = val.ToNumber(penv);
        SetAlpha((float)alpha);
        return true;
    }
    else if (name == "angle")
    {
        SetAngle((SInt16)val.ToInt32(penv));
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
    else if (name == "distance")
    {
        SetDistance((SInt16)val.ToInt32(penv));
        return true;
    }
    else if (name == "hideObject")
    {
        SetHideObject(val.ToBool(penv));
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

bool DropShadowFilterObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name == "alpha")
    {
        val->SetNumber(GetAlpha());
        return true;
    }
    else if (name == "angle")
    {
        val->SetNumber(GetAngle());
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
    else if (name == "distance")
    {
        val->SetNumber(GetDistance());
        return true;
    }
    else if (name == "hideObject")
    {
        val->SetBool(IsHideObject());
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

const NameFunction DropShadowFilterProto::FunctionTable[] = 
{
    { "clone",         &DropShadowFilterProto::Clone },
    { 0, 0 }
};

DropShadowFilterProto::DropShadowFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<DropShadowFilterObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, 0);
}

void DropShadowFilterProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, DropShadowFilter);
    DropShadowFilterObject* pthis = (DropShadowFilterObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<Object> pfilter = *fn.Env->OperatorNew(fn.Env->GetGC()->FlashFiltersPackage, 
        fn.Env->GetBuiltin(ASBuiltin_DropShadowFilter), 0);
    DropShadowFilterObject* pnew = static_cast<DropShadowFilterObject*>(pfilter.GetPtr());
    pnew->SetFilter((BlurFilterImpl*)pthis->GetFilter()->Clone(Memory::GetHeapByAddress(pthis)));
    fn.Result->SetAsObject(pfilter);
}


// --------------------------------------------------------------------


DropShadowFilterCtorFunction::DropShadowFilterCtorFunction(ASStringContext* psc) :
CFunctionObject(psc, DropShadowFilterCtorFunction::GlobalCtor)
{
    SF_UNUSED(psc);
}

void DropShadowFilterCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<DropShadowFilterObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_DropShadowFilter)
        pnode = static_cast<DropShadowFilterObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) DropShadowFilterObject(fn.Env);
    fn.Result->SetAsObject(pnode.GetPtr());

    pnode->SetPasses(1);
    pnode->SetDistance(4);
    pnode->SetAngle(45);
    pnode->SetColor(Color::Black);
    pnode->SetAlpha(1.0f);
    pnode->SetBlurX(4.0f);
    pnode->SetBlurY(4.0f);
    pnode->SetStrength(1.0f);
    pnode->SetKnockOut(false);
    pnode->SetHideObject(false);

    if (fn.NArgs > 0)
    {
        // process distance 
        pnode->SetDistance((SInt16)fn.Arg(0).ToInt32(fn.Env));
        if (fn.NArgs > 1)
        {
            // process angle 
            pnode->SetAngle((SInt16)fn.Arg(1).ToInt32(fn.Env));

            if (fn.NArgs > 2)
            {
                // process color
                pnode->SetColor(fn.Arg(2).ToUInt32(fn.Env));

                if (fn.NArgs > 3)
                {
                    // process alpha
                    pnode->SetAlpha((float)fn.Arg(3).ToNumber(fn.Env));

                    if (fn.NArgs > 4)
                    {
                        // process blurX
                        pnode->SetBlurX((float)fn.Arg(4).ToNumber(fn.Env));

                        if (fn.NArgs > 5)
                        {
                            // process blurY
                            pnode->SetBlurY((float)fn.Arg(5).ToNumber(fn.Env));

                            if (fn.NArgs > 6)
                            {
                                // process strength 
                                pnode->SetStrength((float)fn.Arg(6).ToNumber(fn.Env));

                                if (fn.NArgs > 7)
                                {
                                    // process quality 
                                    pnode->SetPasses((UInt32)fn.Arg(7).ToNumber(fn.Env));

                                    if (fn.NArgs > 8)
                                    {
                                        // process inner
                                        pnode->SetInnerShadow(fn.Arg(8).ToBool(fn.Env));

                                        if (fn.NArgs > 9)
                                        {
                                            // process knockout
                                            pnode->SetKnockOut(fn.Arg(9).ToBool(fn.Env));

                                            if (fn.NArgs > 10)
                                            {
                                                // process hideObject
                                                pnode->SetHideObject(fn.Arg(10).ToBool(fn.Env));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ASStringContext* psc = fn.Env->GetSC();
    pnode->SetMemberRaw(psc, psc->CreateConstString("distance"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("angle"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("color"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("alpha"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurX"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurY"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("strength"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("knockout"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("hideObject"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("inner"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("quality"), Value::UNSET);
}

Object* DropShadowFilterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) DropShadowFilterObject(penv);
}

FunctionRef DropShadowFilterCtorFunction::Register(GlobalContext* pgc)
{
    // Register BitmapFilter here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_BitmapFilter)))
        BitmapFilterCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) DropShadowFilterCtorFunction(&sc));
    Ptr<DropShadowFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) DropShadowFilterProto(&sc, pgc->GetPrototype(ASBuiltin_BitmapFilter), ctor);
    pgc->SetPrototype(ASBuiltin_DropShadowFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_DropShadowFilter), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2
#endif  // SF_NO_FXPLAYER_AS_FILTERS
