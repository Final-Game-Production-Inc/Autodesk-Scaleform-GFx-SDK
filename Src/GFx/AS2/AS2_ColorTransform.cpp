/**************************************************************************

Filename    :   AS2_ColorTransform.cpp
Content     :   flash.geom.Rectangle reference class for ActionScript 2.0
Created     :   3/19/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_ColorTransform.h"


namespace Scaleform { namespace GFx { namespace AS2 {

// SF_NO_FXPLAYER_AS_COLORTRANSFORM disables ColorTransform support
#ifdef GFX_AS2_ENABLE_COLORTRANSFORM


// ****************************************************************************
// Helper function to retrieve the point properties from an object
//
void GFxObject_GetColorTransformProperties(Environment *penv, Object *pobj, Value params[GFxColorTransform_NumProperties])
{
    ASStringContext* psc = penv->GetSC();
    pobj->GetConstMemberRaw(psc, "redMultiplier", &(params[GFxColorTransform_RedMultiplier]));
    pobj->GetConstMemberRaw(psc, "greenMultiplier", &(params[GFxColorTransform_GreenMultiplier]));
    pobj->GetConstMemberRaw(psc, "blueMultiplier", &(params[GFxColorTransform_BlueMultiplier]));
    pobj->GetConstMemberRaw(psc, "alphaMultiplier", &(params[GFxColorTransform_AlphaMultiplier]));
    pobj->GetConstMemberRaw(psc, "redOffset", &(params[GFxColorTransform_RedOffset]));
    pobj->GetConstMemberRaw(psc, "greenOffset", &(params[GFxColorTransform_GreenOffset]));
    pobj->GetConstMemberRaw(psc, "blueOffset", &(params[GFxColorTransform_BlueOffset]));
    pobj->GetConstMemberRaw(psc, "alphaOffset", &(params[GFxColorTransform_AlphaOffset]));
}

// ****************************************************************************
// ColorTransform  constructor
//
ColorTransformObject::ColorTransformObject(Environment *penv)
: Object(penv)
{
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_ColorTransform));
}

const Cxform ColorTransformObject::GetCxform() 
{ 
    Cxform cx(mColorTransform);
    cx.Normalize();
    return cx; 
}

void         ColorTransformObject::SetCxform(const Cxform &ct)
{
    mColorTransform = ct;
    mColorTransform.M[1][0] *= 255;
    mColorTransform.M[1][1] *= 255;
    mColorTransform.M[1][2] *= 255;
    mColorTransform.M[1][3] *= 255;
}


// ****************************************************************************
// Intercept the get command so that dependent properties can be recalculated
//
bool ColorTransformObject::GetMember(Environment *penv, const ASString &name, Value* val)
{
    if (name == "redMultiplier")
    {
        *val = mColorTransform.M[0][0];
        return true;
    }
    else if (name == "greenMultiplier")
    {
        *val = mColorTransform.M[0][1];
        return true;
    }
    else if (name == "blueMultiplier")
    {
        *val = mColorTransform.M[0][2];
        return true;
    }
    else if (name == "alphaMultiplier")
    {
        *val = mColorTransform.M[0][3];
        return true;
    }
    else if (name == "redOffset")
    {
        *val = mColorTransform.M[1][0];
        return true;
    }
    else if (name == "greenOffset")
    {
        *val = mColorTransform.M[1][1];
        return true;
    }
    else if (name == "blueOffset")
    {
        *val = mColorTransform.M[1][2];
        return true;
    }
    else if (name == "alphaOffset")
    {
        *val = mColorTransform.M[1][3];
        return true;
    }
    else if (name == "rgb")
    {
        Value ro, go, bo;
        UInt8 rc = (UInt8) (NumberUtil::IsNaN(mColorTransform.M[1][0]) ? 0 : mColorTransform.M[1][0]);
        UInt8 gc = (UInt8) (NumberUtil::IsNaN(mColorTransform.M[1][1]) ? 0 : mColorTransform.M[1][1]);
        UInt8 bc = (UInt8) (NumberUtil::IsNaN(mColorTransform.M[1][2]) ? 0 : mColorTransform.M[1][2]);
        UInt32 p = (rc << 16) | (gc << 8) | bc;
        *val = Double(p);
        return true;
    }
    return Object::GetMember(penv, name, val);
}

// ****************************************************************************
// Intercept the set command so that dependent properties can be recalculated
//
bool ColorTransformObject::SetMember(Environment *penv, const ASString &name, const Value &val, const PropFlags& flags)
{
    if (name == "redMultiplier")
    {
        mColorTransform.M[0][0] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "greenMultiplier")
    {
        mColorTransform.M[0][1] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "blueMultiplier")
    {
        mColorTransform.M[0][2] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "alphaMultiplier")
    {
        mColorTransform.M[0][3] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "redOffset")
    {
        mColorTransform.M[1][0] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "greenOffset")
    {
        mColorTransform.M[1][1] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "blueOffset")
    {
        mColorTransform.M[1][2] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "alphaOffset")
    {
        mColorTransform.M[1][3] = static_cast<float>(val.ToNumber(penv));
        return true;
    }
    else if (name == "rgb")
    {
        // When you set this property, it changes the three color offset values 
        // (redOffset, greenOffset, and blueOffset) accordingly, and it sets the 
        // three color multiplier values (redMultiplier, greenMultiplier, and 
        // blueMultiplier) to 0. The alpha transparency multiplier and offset 
        // values do not change.
        mColorTransform.M[0][0] = 0;
        mColorTransform.M[0][1] = 0;
        mColorTransform.M[0][2] = 0;
        Number n = val.ToNumber(penv);
        UInt32 p = 0, rc = 0, gc = 0, bc = 0;
        if (!NumberUtil::IsNaN(n))
        {
            p = (UInt32) val.ToNumber(penv);
            rc = (p >> 16) & 0x000000ff;
            gc = (p >> 8) & 0x000000ff;
            bc = p & 0x000000ff;
        }
        mColorTransform.M[1][0] = static_cast<float>(rc);
        mColorTransform.M[1][1] = static_cast<float>(gc);
        mColorTransform.M[1][2] = static_cast<float>(bc);
        return true;
    }
    return Object::SetMember(penv, name, val, flags);
}

// ****************************************************************************
// AS to GFx function mapping
//
const NameFunction ColorTransformProto::FunctionTable[] = 
{
    { "concat", &ColorTransformProto::Concat },
    { "toString", &ColorTransformProto::ToString },
    { 0, 0 }
};

// ****************************************************************************
// ColorTransform Prototype constructor
//
ColorTransformProto::ColorTransformProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) 
 : Prototype<ColorTransformObject>(psc, pprototype, constructor)
{
    // we make the functions enumerable
    InitFunctionMembers(psc, FunctionTable, PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete);

    // add the on-the-fly calculated properties
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("redMultiplier"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("greenMultiplier"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("blueMultiplier"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("alphaMultiplier"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("redOffset"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("greenOffset"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("blueOffset"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("alphaOffset"), 0, PropFlags::PropFlag_DontDelete);
    ColorTransformObject::SetMemberRaw(psc, psc->CreateConstString("rgb"), 0, PropFlags::PropFlag_DontDelete);
    //InitFunctionMembers(psc, FunctionTable, pprototype);
}

// ****************************************************************************
// ColorTransform.Concat method
//
void ColorTransformProto::Concat(const FnCall& fn)
{
    if (fn.NArgs > 0)
    {
        Object* p = fn.Arg(0).ToObject(fn.Env);
        if (NULL != p)
        {
            CHECK_THIS_PTR(fn, ColorTransform);
            ColorTransformObject *pthis = (ColorTransformObject*)fn.ThisPtr;
            SF_ASSERT(pthis);
            // begin concat logic
            if (p->GetObjectType() == Object_ColorTransform)
            {
                // the properties aren't stored if it is a ColorTransform obj
                // so, manually get the properties
                ColorTransformObject *pparam = (ColorTransformObject*)p;
                pthis->mColorTransform.Prepend(pparam->mColorTransform);
            }
            else
            {
                Value c[GFxColorTransform_NumProperties];
                // get properties for parameter
                GFxObject_GetColorTransformProperties(fn.Env, p, c); 
                Cxform ct;
                ct.M[0][0] = static_cast<float>(c[GFxColorTransform_RedMultiplier].ToNumber(fn.Env));
                ct.M[0][1] = static_cast<float>(c[GFxColorTransform_GreenMultiplier].ToNumber(fn.Env));
                ct.M[0][2] = static_cast<float>(c[GFxColorTransform_BlueMultiplier].ToNumber(fn.Env));
                ct.M[0][3] = static_cast<float>(c[GFxColorTransform_AlphaMultiplier].ToNumber(fn.Env));
                ct.M[1][0] = static_cast<float>(c[GFxColorTransform_RedOffset].ToNumber(fn.Env));
                ct.M[1][1] = static_cast<float>(c[GFxColorTransform_GreenOffset].ToNumber(fn.Env));
                ct.M[1][2] = static_cast<float>(c[GFxColorTransform_BlueOffset].ToNumber(fn.Env));
                ct.M[1][3] = static_cast<float>(c[GFxColorTransform_AlphaOffset].ToNumber(fn.Env));
                pthis->mColorTransform.Prepend(ct);
            }
            // end concat logic
        }
    }
}

// ****************************************************************************
// ColorTransform.toString method
//
void ColorTransformProto::ToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, ColorTransform);
    ColorTransformObject* pthis = (ColorTransformObject*) fn.ThisPtr;
    SF_ASSERT(pthis);
    const Cxform& ct = pthis->mColorTransform;

    ASString ps[8] = 
    {
        Value(ct.M[0][0]).ToString(fn.Env, 6),
        Value(ct.M[0][1]).ToString(fn.Env, 6),
        Value(ct.M[0][2]).ToString(fn.Env, 6),
        Value(ct.M[0][3]).ToString(fn.Env, 6),
        Value(ct.M[1][0]).ToString(fn.Env, 6),
        Value(ct.M[1][1]).ToString(fn.Env, 6),
        Value(ct.M[1][2]).ToString(fn.Env, 6),
        Value(ct.M[1][3]).ToString(fn.Env, 6)
    };

    StringBuffer str;
    str += "(redMultiplier=";
    str += ps[0].ToCStr();
    str += ", greenMultiplier=";
    str += ps[1].ToCStr();
    str += ", blueMultiplier=";
    str += ps[2].ToCStr();
    str += ", alphaMultiplier=";
    str += ps[3].ToCStr();
    str += ", redOffset=";
    str += ps[4].ToCStr();
    str += ", greenOffset=";
    str += ps[5].ToCStr();
    str += ", blueOffset=";
    str += ps[6].ToCStr();
    str += ", alphaOffset=";
    str += ps[7].ToCStr();
    str += ")";

    fn.Result->SetString(fn.Env->CreateString(str.ToCStr(),str.GetSize()));
}

// ****************************************************************************
// Called when the constructor is invoked for the ColorTransform class
//
void ColorTransformCtorFunction::GlobalCtor(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_ColorTransform)
    {
        Ptr<ColorTransformObject> pct = static_cast<ColorTransformObject*>(fn.ThisPtr);
        fn.Result->SetAsObject(pct.GetPtr());
        // handle the constructor parameters
        if (fn.NArgs > 7)
        {
            Cxform& ct = pct->mColorTransform;
            ct.M[0][0] = static_cast<float>(fn.Arg(0).ToNumber(fn.Env));
            ct.M[0][1] = static_cast<float>(fn.Arg(1).ToNumber(fn.Env));
            ct.M[0][2] = static_cast<float>(fn.Arg(2).ToNumber(fn.Env));
            ct.M[0][3] = static_cast<float>(fn.Arg(3).ToNumber(fn.Env));
            ct.M[1][0] = static_cast<float>(fn.Arg(4).ToNumber(fn.Env));
            ct.M[1][1] = static_cast<float>(fn.Arg(5).ToNumber(fn.Env));
            ct.M[1][2] = static_cast<float>(fn.Arg(6).ToNumber(fn.Env));
            ct.M[1][3] = static_cast<float>(fn.Arg(7).ToNumber(fn.Env));
        }
    }
}

Object* ColorTransformCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) ColorTransformObject(penv);
}

FunctionRef ColorTransformCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) ColorTransformCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ColorTransformProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_ColorTransform, proto);
    pgc->FlashGeomPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_ColorTransform), Value(ctor));
    return ctor;
}

#else

void GASColorTransform_DummyFunction() {}   // Exists to quelch compiler warning

#endif // SF_NO_FXPLAYER_AS_COLORTRANSFORM
}}} //SF::GFx::AS2
