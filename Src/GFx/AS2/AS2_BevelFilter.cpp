/**********************************************************************

Filename    :   AS2_BevelFilter.cpp
Content     :   BevelFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BevelFilter.h"
#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// BevelFilterObject::BevelFilterObject()
// : BitmapFilterObject(GFxFilterDesc::Filter_Bevel)
// {
// }

BevelFilterObject::BevelFilterObject(Environment* penv)
: BitmapFilterObject(penv, Filter_Bevel)
{
}

BevelFilterObject::~BevelFilterObject()
{
}

bool BevelFilterObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags /* = PropFlags */)
{
    if (name == "angle")
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
    else if (name == "distance")
    {
        SetDistance((SInt16)val.ToInt32(penv));
        return true;
    }
    else if (name == "highlightAlpha")
    {
        Number alpha = val.ToNumber(penv);
        SetAlpha((float)alpha);
        return true;
    }
    else if (name == "highlightColor")
    {
        SetColor(val.ToUInt32(penv));
        return true;
    }
    else if (name == "shadowAlpha")
    {
        Number alpha = val.ToNumber(penv);
        SetAlpha2((float)alpha);
        return true;
    }
    else if (name == "shadowColor")
    {
        SetColor2(val.ToUInt32(penv));
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
    else if (name == "type")
    {
        ASString type = val.ToString(penv);
        if (type == "inner")
            SetInnerShadow(true);
        else
            SetInnerShadow(false);
        return true;
    }
    else if (name == "strength")
    {
        SetStrength((float)val.ToNumber(penv));
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

bool BevelFilterObject::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (name == "angle")
    {
        val->SetInt((int)GetAngle());
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
    else if (name == "distance")
    {
        val->SetInt((int)GetDistance());
        return true;
    }
    else if (name == "highlightAlpha")
    {
        val->SetNumber(GetAlpha());
        return true;
    }
    else if (name == "highlightColor")
    {
        val->SetUInt(GetColor());
        return true;
    }
    else if (name == "shadowAlpha")
    {
        val->SetNumber(GetAlpha2());
        return true;
    }
    else if (name == "shadowColor")
    {
        val->SetUInt(GetColor2());
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
    else if (name == "type")
    {
        if (IsInnerShadow())
            val->SetString(penv->CreateString("inner"));
        else
            val->SetString(penv->CreateString("outer"));
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

const NameFunction BevelFilterProto::FunctionTable[] = 
{
    { "clone",         &BevelFilterProto::Clone },
    { 0, 0 }
};

BevelFilterProto::BevelFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<BevelFilterObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, FunctionTable, 0);
}

void BevelFilterProto::Clone(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, BevelFilter);
    BevelFilterObject* pthis = (BevelFilterObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<Object> pfilter = *fn.Env->OperatorNew(fn.Env->GetGC()->FlashFiltersPackage, 
        fn.Env->GetBuiltin(ASBuiltin_BevelFilter), 0);
    BevelFilterObject* pnew = static_cast<BevelFilterObject*>(pfilter.GetPtr());
    pnew->SetFilter(pthis->GetFilter());
    fn.Result->SetAsObject(pfilter);
}


// --------------------------------------------------------------------


BevelFilterCtorFunction::BevelFilterCtorFunction(ASStringContext* psc) :
CFunctionObject(psc, BevelFilterCtorFunction::GlobalCtor)
{
    SF_UNUSED(psc);
}

void BevelFilterCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<BevelFilterObject> pnode;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_BevelFilter)
        pnode = static_cast<BevelFilterObject*>(fn.ThisPtr);
    else
        pnode = *SF_HEAP_NEW(fn.Env->GetHeap()) BevelFilterObject(fn.Env);
    fn.Result->SetAsObject(pnode.GetPtr());

    pnode->SetPasses(1);
    pnode->SetDistance(4);
    pnode->SetAngle(45);
    pnode->SetColor(Color::Black);
    pnode->SetAlpha(1.0f);
    pnode->SetColor2(Color::White);
    pnode->SetAlpha2(1.0f);
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
                // process highlight color
                pnode->SetColor(fn.Arg(2).ToUInt32(fn.Env));

                if (fn.NArgs > 3)
                {
                    // process highlight alpha
                    pnode->SetAlpha((float)fn.Arg(3).ToNumber(fn.Env));

                    if (fn.NArgs > 4)
                    {
                        // process color
                        pnode->SetColor2(fn.Arg(4).ToUInt32(fn.Env));

                        if (fn.NArgs > 5)
                        {
                            // process alpha
                            pnode->SetAlpha2((float)fn.Arg(5).ToNumber(fn.Env));

                            if (fn.NArgs > 6)
                            {
                                // process blurX
                                pnode->SetBlurX((float)fn.Arg(6).ToNumber(fn.Env));

                                if (fn.NArgs > 7)
                                {
                                    // process blurY
                                    pnode->SetBlurY((float)fn.Arg(7).ToNumber(fn.Env));

                                    if (fn.NArgs > 8)
                                    {
                                        // process strength 
                                        pnode->SetStrength((float)fn.Arg(8).ToNumber(fn.Env));

                                        if (fn.NArgs > 9)
                                        {
                                            // process quality 
                                            pnode->SetPasses((UInt32)fn.Arg(9).ToNumber(fn.Env));

                                            if (fn.NArgs > 10)
                                            {
                                                // process type
                                                ASString type = fn.Arg(10).ToString(fn.Env);
                                                if (type == "inner")
                                                    pnode->SetInnerShadow(true);
                                                else
                                                    pnode->SetInnerShadow(false);

                                                if (fn.NArgs > 11)
                                                {
                                                    // process knockout
                                                    pnode->SetKnockOut(fn.Arg(11).ToBool(fn.Env));
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
    }

    ASStringContext* psc = fn.Env->GetSC();
    pnode->SetMemberRaw(psc, psc->CreateConstString("shadowColor"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("shadowAlpha"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("highlightColor"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("highlightAlpha"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurX"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("blurY"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("strength"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("knockout"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("inner"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("type"), Value::UNSET);
    pnode->SetMemberRaw(psc, psc->CreateConstString("quality"), Value::UNSET);
}

Object* BevelFilterCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) BevelFilterObject(penv);
}

FunctionRef BevelFilterCtorFunction::Register(GlobalContext* pgc)
{
    // Register BitmapFilter here also
    if (!pgc->GetBuiltinClassRegistrar(pgc->GetBuiltin(ASBuiltin_BitmapFilter)))
        BitmapFilterCtorFunction::Register(pgc);

    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) BevelFilterCtorFunction(&sc));
    Ptr<BevelFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) BevelFilterProto(&sc, pgc->GetPrototype(ASBuiltin_BitmapFilter), ctor);
    pgc->SetPrototype(ASBuiltin_BevelFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_BevelFilter), Value(ctor));
    return ctor;
}

}}}; //SF::GFx::AS2

#endif  // GFX_AS2_ENABLE_FILTERS
