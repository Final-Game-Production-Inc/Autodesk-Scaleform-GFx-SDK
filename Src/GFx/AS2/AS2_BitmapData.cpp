/**************************************************************************

Filename    :   AS2_BitmapData.cpp
Content     :   Implementation of BitmapData class
Created     :   March, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_BitmapData.h"
#ifdef GFX_AS2_ENABLE_BITMAPDATA
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "Kernel/SF_UTF8Util.h"

namespace Scaleform { namespace GFx { namespace AS2 {

BitmapData::BitmapData(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

void BitmapData::commonInit (Environment* penv)
{    
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_BitmapData));
    SetMemberRaw(penv->GetSC(), penv->CreateConstString("width"), 
        Value(Value::UNSET), PropFlags::PropFlag_ReadOnly);
    SetMemberRaw(penv->GetSC(), penv->CreateConstString("height"), Value(Value::UNSET), 
        PropFlags::PropFlag_ReadOnly);
}

template <typename StringType>
BitmapData* GFx_LoadBitmap(Environment* penv, const StringType& linkageId)
{
    Ptr<ImageResource> ir = penv->GetMovieImpl()->
        GetImageResourceByLinkageId(penv->GetTarget()->GetResourceMovieDef(), linkageId.ToCStr());
    if (!ir)
    {
        penv->LogScriptWarning(
            "BitmapData::LoadBitmap: LoadMovieImageCallback failed to load image \"%s\"", 
            linkageId.ToCStr());
        return NULL;
    }
    Ptr<MovieDefImpl> md = penv->GetTarget()->GetResourceMovieDef();
    if (md) // can it be NULL?
    {
        BitmapData* pobj = SF_HEAP_NEW(penv->GetHeap()) BitmapData(penv);
        pobj->SetImage(penv, ir, md);
        return pobj;
    }
    return NULL;
}

BitmapData* BitmapData::LoadBitmap(Environment* penv, const String& linkageId)
{
    return GFx_LoadBitmap<>(penv, linkageId);
}

void BitmapData::SetImage(Environment *penv, ImageResource* pimg, MovieDef* pmovieDef) 
{ 
    pImageRes = pimg; 
    pMovieDef = pmovieDef;

    Ptr<Render::Image> img;
    Render::ImageRect dims = pimg->GetImage()->GetRect();
    Value params[4];
    params[0] = Value(0);
    params[1] = Value(0);
    params[2] = Value((float)dims.Width());
    params[3] = Value((float)dims.Height());

#ifdef GFX_AS2_ENABLE_RECTANGLE
    Ptr<RectangleObject> bounds = //*SF_HEAP_NEW(penv->GetHeap()) RectangleObject(penv);
        *static_cast<RectangleObject*>
        (penv->OperatorNew(penv->GetGC()->FlashGeomPackage, penv->GetBuiltin(ASBuiltin_Rectangle)));
    bounds->SetProperties(penv->GetSC(), params);
#else
    Ptr<Object> bounds = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
    ASStringContext *psc = penv->GetSC();
    bounds->SetConstMemberRaw(psc, "x", params[0]);
    bounds->SetConstMemberRaw(psc, "y", params[1]);
    bounds->SetConstMemberRaw(psc, "width", params[2]);
    bounds->SetConstMemberRaw(psc, "height", params[3]);
#endif 
    SetMemberRaw(penv->GetSC(), penv->CreateConstString("rectangle"), Value(bounds), PropFlags::PropFlag_ReadOnly);
}

bool BitmapData::SetMember(Environment* penv, const ASString& name, const Value& val, 
                              const PropFlags& flags)
{
    if (name == "width" || name == "height")
        return true;
    return Object::SetMember(penv, name, val, flags);
}

bool BitmapData::GetMember(Environment* penv, const ASString& name, Value* val)
{
    if (pImageRes)
    {
        if (name == "width")
        {
            val->SetNumber((Number)pImageRes->GetWidth());
            return true;
        }
        else if (name == "height")
        {
            val->SetNumber((Number)pImageRes->GetHeight());
            return true;
        }
    }
    return Object::GetMember(penv, name, val);
}

//////////////////////////////////////////
//
static void GFx_BitmapDataFuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GFx_BitmapDataFunctionTable[] = 
{
    { "loadBitmap",        &GFx_BitmapDataFuncStub },
    { 0, 0 }
};

BitmapDataProto::BitmapDataProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<BitmapData>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GFx_BitmapDataFunctionTable);
}

//////////////////
const NameFunction BitmapDataCtorFunction::StaticFunctionTable[] = 
{
    { "loadBitmap",         &BitmapDataCtorFunction::LoadBitmap    },
    { 0, 0 }
};

BitmapDataCtorFunction::BitmapDataCtorFunction(ASStringContext* psc) :
    CFunctionObject(psc, BitmapDataCtorFunction::GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void BitmapDataCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<BitmapData> ab = *SF_HEAP_NEW(fn.Env->GetHeap()) BitmapData(fn.Env);
    fn.Result->SetAsObject(ab.GetPtr());
}

void BitmapDataCtorFunction::LoadBitmap(const FnCall& fn)
{
    fn.Result->SetNull();
    if (fn.NArgs < 1)
        return;

    SF_ASSERT(fn.ThisPtr);

    ASString linkageId(fn.Arg(0).ToString(fn.Env));
    Ptr<BitmapData> pobj = *GFx_LoadBitmap<>(fn.Env, linkageId);
    if (pobj)
        fn.Result->SetAsObject(pobj.GetPtr());
}

FunctionRef BitmapDataCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) BitmapDataCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) BitmapDataProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_BitmapData, proto);
    pgc->FlashDisplayPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_BitmapData), Value(ctor));
    return ctor;
}
}}} //SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_BITMAPDATA
