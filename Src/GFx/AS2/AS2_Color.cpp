/**************************************************************************

Filename    :   AS2_Color.cpp
Content     :   Color object functinality
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Color.h"
#ifdef GFX_AS2_ENABLE_COLOR
#include "AS2_FunctionRef.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

// NOTE: SetTransform allows for negative color transforms (inverse mul/color subtract?).
// TODO: Such transforms probably won't work in renderer -> needs testing!

void ColorProto::SetTransform(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Color);
    ColorObject* pthis = (ColorObject*) fn.ThisPtr;       
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<InteractiveObject>    pc = pthis->pCharacter;
    if ((fn.NArgs < 1) || !pc)
        return;
    ObjectInterface* pobj = fn.Arg(0).ToObjectInterface(fn.Env);
    if (!pobj)
        return;

    // Get color transform, so that we can assign only
    // the fields available in the object. Other fields must
    // remain untouched.
    Cxform   c = pc->GetCxform();
    Value            val;
    ASStringContext*   psc = fn.Env->GetSC();
    
    // Multiply.
    if (pobj->GetConstMemberRaw(psc, "ba", &val))
        c.M[0][2] = (float) val.ToNumber(fn.Env) / 100.0f;
    if (pobj->GetConstMemberRaw(psc, "ga", &val))
        c.M[0][1] = (float) val.ToNumber(fn.Env) / 100.0f;
    if (pobj->GetConstMemberRaw(psc, "ra", &val))
        c.M[0][0] = (float) val.ToNumber(fn.Env) / 100.0f;
    if (pobj->GetConstMemberRaw(psc, "aa", &val))
        c.M[0][3] = (float) val.ToNumber(fn.Env) / 100.0f;
    // Add.
    if (pobj->GetConstMemberRaw(psc, "bb", &val))
        c.M[1][2] = (float) val.ToNumber(fn.Env) / 255.f;
    if (pobj->GetConstMemberRaw(psc, "gb", &val))
        c.M[1][1] = (float) val.ToNumber(fn.Env) / 255.f;
    if (pobj->GetConstMemberRaw(psc, "rb", &val))
        c.M[1][0] = (float) val.ToNumber(fn.Env) / 255.f;
    if (pobj->GetConstMemberRaw(psc, "ab", &val))
        c.M[1][3] = (float) val.ToNumber(fn.Env) / 255.f;

    pc->SetCxform(c);
    pc->SetAcceptAnimMoves(false);
}

void ColorProto::GetTransform(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Color);
    ColorObject* pthis = (ColorObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<InteractiveObject>    pc = pthis->pCharacter;
    if (!pc)
        return;
    Cxform c = pc->GetCxform();

    Ptr<Object>     pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    ASStringContext*   psc = fn.Env->GetSC();

    // Percentage: should be -100 to 100.
    pobj->SetConstMemberRaw(psc, "ba", c.M[0][2] * 100.0f);
    pobj->SetConstMemberRaw(psc, "ga", c.M[0][1] * 100.0f);
    pobj->SetConstMemberRaw(psc, "ra", c.M[0][0] * 100.0f);
    pobj->SetConstMemberRaw(psc, "aa", c.M[0][3] * 100.0f);
    // Add. Should be -255 to 255.
    pobj->SetConstMemberRaw(psc, "bb", c.M[1][2] * 255.0f);
    pobj->SetConstMemberRaw(psc, "gb", c.M[1][1] * 255.0f);
    pobj->SetConstMemberRaw(psc, "rb", c.M[1][0] * 255.0f);
    pobj->SetConstMemberRaw(psc, "ab", c.M[1][3] * 255.0f);

    fn.Result->SetAsObject(pobj.GetPtr());
}

void ColorProto::SetRGB(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Color);
    ColorObject* pthis = (ColorObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<InteractiveObject>    pc = pthis->pCharacter;
    if ((fn.NArgs < 1) || !pc)
        return;         
    UInt32  colorVal = (UInt32)fn.Arg(0).ToNumber(fn.Env);

    // Get the transform so that we can keep the Alpha unmodified.
    Cxform c = pc->GetCxform();
    // No multiply.
    c.M[0][0] = c.M[0][1] = c.M[0][2] = 0;
    // Add.
    c.M[1][0] = (float) ((colorVal >> 16) & 0xFF); // R
    c.M[1][1] = (float) ((colorVal >> 8) & 0xFF);  // G
    c.M[1][2] = (float) (colorVal & 0xFF);         // B
    c.Normalize();
    pc->SetCxform(c);
    pc->SetAcceptAnimMoves(false);
}

void ColorProto::GetRGB(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Color);
    ColorObject* pthis = (ColorObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    Ptr<InteractiveObject>    pc = pthis->pCharacter;
    if (!pc)
        return;         

    Cxform c = pc->GetCxform();
    UInt32  colorVal = 
        (((unsigned)c.M[1][2]*255) & 0xFF)         |
        ((((unsigned)c.M[1][1]*255) & 0xFF) << 8)  |
        ((((unsigned)c.M[1][0]*255) & 0xFF) << 16);

    fn.Result->SetUInt((unsigned)colorVal);
}

static const NameFunction GAS_ColorFunctionTable[] = 
{
    { "setTransform",  &ColorProto::SetTransform  },
    { "getTransform",  &ColorProto::GetTransform  },
    { "setRGB",        &ColorProto::SetRGB  },
    { "getRGB",        &ColorProto::GetRGB  },
    { 0, 0 }
};

ColorProto::ColorProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<ColorObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GAS_ColorFunctionTable);
}

//////////////////////////////////////////////////////////////////////////
// ColorCtorFunction
void ColorCtorFunction::GlobalCtor(const FnCall& fn)
{
    InteractiveObject* ptarget = 0;
    if (fn.NArgs >= 1)
        ptarget = fn.Env->FindTargetByValue(fn.Arg(0));

    Ptr<ColorObject> pcolor;
    if (!fn.ThisPtr || fn.ThisPtr->GetObjectType() != Object_Color)
        pcolor = *SF_HEAP_NEW(fn.Env->GetHeap()) ColorObject(fn.Env, ptarget);
    else
    {
        pcolor = static_cast<ColorObject*>(fn.ThisPtr);
        pcolor->SetTarget(ptarget);
    }
    fn.Result->SetAsObject(pcolor.GetPtr());
}

Object* ColorCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) ColorObject(penv, NULL);
}

FunctionRef ColorCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) ColorCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ColorProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Color, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Color), Value(ctor));
    return ctor;
}
#endif //#ifdef GFX_AS2_ENABLE_COLOR

}}} //SF::GFx::AS2
