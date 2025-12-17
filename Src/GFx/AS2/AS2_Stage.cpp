/**************************************************************************

Filename    :   AS2_Stage.cpp
Content     :   Stage class implementation
Created     :   
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Stage.h"

#ifdef GFX_AS2_ENABLE_STAGE
#include "AS2_FunctionRef.h"
#include "AS2_Value.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_RectangleObject.h"

#include <stdio.h>
#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

StageObject::StageObject(Environment* penv)
: Object(penv)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Stage));
}


//////////////////////////////////////////
//
static void GFx_StageFuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GFx_StageFunctionTable[] = 
{
    { "addListener",        &GFx_StageFuncStub },
    { "removeListener",     &GFx_StageFuncStub },
    { "broadcastMessage",   &GFx_StageFuncStub },
    { 0, 0 }
};

StageProto::StageProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<StageObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GFx_StageFunctionTable);
}

StageCtorFunction::StageCtorFunction(ASStringContext *psc, MovieImpl* movieRoot) : 
    CFunctionObject(psc, GlobalCtor), pMovieRoot(movieRoot) 
{ 
    // Stage is a broadcaster
    AsBroadcaster::Initialize(psc, this);

    FunctionObject::SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_width), Value(Value::UNSET));
    FunctionObject::SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_height), Value(Value::UNSET));
    SetConstMemberRaw(psc, "scaleMode", Value(Value::UNSET));
    SetConstMemberRaw(psc, "align", Value(Value::UNSET));
    
    SetConstMemberRaw(psc, "showMenu", Value(true));
}

bool    StageCtorFunction::GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
{
    if (psc->GetBuiltin(ASBuiltin_width).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()))        
    {
        val->SetInt(int(TwipsToPixels(pMovieRoot->VisibleFrameRect.Width())));
        return true;
    }
    else if (psc->GetBuiltin(ASBuiltin_height).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()))
    {
        val->SetInt(int(TwipsToPixels(pMovieRoot->VisibleFrameRect.Height())));
        return true;
    }
    else if (psc->CompareConstString_CaseCheck(name, "scaleMode"))
    {
        const char* scaleMode; 
        switch(pMovieRoot->GetViewScaleMode())
        {
            case Movie::SM_NoScale:  scaleMode = "noScale"; break;
            case Movie::SM_ExactFit: scaleMode = "exactFit"; break;
            case Movie::SM_NoBorder: scaleMode = "noBorder"; break;
            default: scaleMode = "showAll";
        }
        val->SetString(psc->CreateConstString(scaleMode));
        return true;
    }
    else if (psc->CompareConstString_CaseCheck(name, "align"))
    {
        const char* align; 
        switch(pMovieRoot->GetViewAlignment())
        {
            case Movie::Align_TopCenter:     align = "T"; break;
            case Movie::Align_BottomCenter:  align = "B"; break;
            case Movie::Align_CenterLeft:    align = "L"; break;
            case Movie::Align_CenterRight:   align = "R"; break;
            case Movie::Align_TopLeft:       align = "LT"; break; // documented as TL, but returns LT
            case Movie::Align_TopRight:      align = "TR"; break;
            case Movie::Align_BottomLeft:    align = "LB"; break; // documented as BL, but returns LB
            case Movie::Align_BottomRight:   align = "RB"; break; // documented as BR, but returns RB
            default: align = "";
        }
        val->SetString(psc->CreateConstString(align));
        return true;
    }
    return FunctionObject::GetMemberRaw(psc, name, val);
}

Value StageCtorFunction::CreateRectangleObject(Environment *penv, const RectF& rect)
{
    Value v;
#ifdef GFX_AS2_ENABLE_RECTANGLE
    Ptr<RectangleObject> prect = *static_cast<RectangleObject*>
        (penv->OperatorNew(penv->GetGC()->FlashGeomPackage, penv->GetBuiltin(ASBuiltin_Rectangle)));
    ASRect gr( (Number)rect.x1,  (Number)rect.y1,
                (Number)rect.x2, (Number)rect.y2); 
    prect->SetProperties(penv, gr);
    v.SetAsObject(prect);
#else
    Ptr<Object> bounds = *penv->OperatorNew(penv->GetBuiltin(ASBuiltin_Object));
    ASStringContext *psc = penv->GetSC();
    bounds->SetConstMemberRaw(psc, "x", (Number)rect.x1);
    bounds->SetConstMemberRaw(psc, "y", (Number)rect.y1);
    bounds->SetConstMemberRaw(psc, "width", (Number)rect.Width());
    bounds->SetConstMemberRaw(psc, "height", (Number)rect.Height());
    v.SetAsObject(bounds);
#endif
    return v;
}

bool    StageCtorFunction::GetMember(Environment *penv, const ASString& name, Value* val)
{
    if (penv->CheckExtensions())
    {
        if (penv->GetSC()->CompareConstString_CaseCheck(name, "visibleRect"))
        {
            RectF frameRect = penv->GetMovieImpl()->GetVisibleFrameRect();
            *val = CreateRectangleObject(penv, frameRect);
            return true;
        }
        else if (penv->GetSC()->CompareConstString_CaseCheck(name, "safeRect"))
        {
            RectF safeRect = penv->GetMovieImpl()->GetSafeRect();
            if (safeRect.IsEmpty())
                safeRect = penv->GetMovieImpl()->GetVisibleFrameRect();
            *val = CreateRectangleObject(penv, safeRect);
            return true;
        }
        else if (penv->GetSC()->CompareConstString_CaseCheck(name, "originalRect"))
        {
            RectF origRect = penv->GetMovieImpl()->GetMovieDef()->GetFrameRect();
            *val = CreateRectangleObject(penv, origRect);
            return true;
        }
    }
    return FunctionObject::GetMember(penv, name, val);
}

bool    StageCtorFunction::SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    ASStringContext * const psc = penv->GetSC();

    if (psc->CompareConstString_CaseCheck(name, "scaleMode"))
    {   
        const ASString& scaleModeStr = val.ToString(penv);
        Movie::ScaleModeType scaleMode, prevScaleMode = pMovieRoot->GetViewScaleMode();

        if (psc->CompareConstString_CaseInsensitive(scaleModeStr, "noScale"))
            scaleMode = Movie::SM_NoScale;
        else if (psc->CompareConstString_CaseInsensitive(scaleModeStr, "exactFit"))
            scaleMode = Movie::SM_ExactFit;
        else if (psc->CompareConstString_CaseInsensitive(scaleModeStr, "noBorder"))
            scaleMode = Movie::SM_NoBorder;
        else
            scaleMode = Movie::SM_ShowAll;
        pMovieRoot->SetViewScaleMode(scaleMode);
        if (prevScaleMode != scaleMode && scaleMode == Movie::SM_NoScale)
        {
            // invoke onResize
            NotifyOnResize(penv);
        }
        return true;
    }
    else if (psc->CompareConstString_CaseCheck(name, "align"))
    {
        const ASString& alignStr = val.ToString(penv).ToUpper();
        Movie::AlignType align;
        unsigned char1 = 0, char2 = 0;
        unsigned len = alignStr.GetLength();
        if (len >= 1)
        {
            char1 = alignStr.GetCharAt(0);
            if (len >= 2)
                char2 = alignStr.GetCharAt(1);
        }
        if ((char1 == 'T' && char2 == 'L') || (char1 == 'L' && char2 == 'T')) // TL or LT
            align = Movie::Align_TopLeft;
        else if ((char1 == 'T' && char2 == 'R') || (char1 == 'R' && char2 == 'T')) // TR or RT
            align = Movie::Align_TopRight;
        else if ((char1 == 'B' && char2 == 'L') || (char1 == 'L' && char2 == 'B')) // BL or LB
            align = Movie::Align_BottomLeft;
        else if ((char1 == 'B' && char2 == 'R') || (char1 == 'R' && char2 == 'B')) // BR or RB
            align = Movie::Align_BottomRight;
        else if (char1 == 'T')
            align = Movie::Align_TopCenter;
        else if (char1 == 'B')
            align = Movie::Align_BottomCenter;
        else if (char1 == 'L')
            align = Movie::Align_CenterLeft;
        else if (char1 == 'R')
            align = Movie::Align_CenterRight;
        else 
            align = Movie::Align_Center;
        pMovieRoot->SetViewAlignment(align);
        return true;
    }
    return FunctionObject::SetMember(penv, name, val, flags);
}

bool    StageCtorFunction::SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags)
{
    if (psc->GetBuiltin(ASBuiltin_width).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()) ||
        psc->GetBuiltin(ASBuiltin_height).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()))
    {
        // read-only, do nothing
        return true;
    }
    return FunctionObject::SetMemberRaw(psc, name, val, flags);
}

void StageCtorFunction::NotifyOnResize(Environment* penv)
{
    if (penv->CheckExtensions())
    {
        // extension: onResize handler receives a parameter (flash.geom.Rectangle) with 
        // the currently visible rectangle.

        RectF frameRect = penv->GetMovieImpl()->GetVisibleFrameRect();
        Value v = CreateRectangleObject(penv, frameRect);
        penv->Push(v);
        AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onResize"),  1, penv->GetTopIndex());
        penv->Drop1();
    }
    else
    {
        AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onResize"), 0, 0);
    }
}

void StageCtorFunction::NotifyOnResize(const FnCall& fn)
{
    Value stageCtorVal;
    if (fn.Env->GetGC()->pGlobal->GetMemberRaw(fn.Env->GetSC(), fn.Env->GetBuiltin(ASBuiltin_Stage), &stageCtorVal))
    {
        //RectF r = fn.Env->GetMovieImpl()->GetVisibleFrameRect();
        //printf("Frame rect = %f, %f, {%f, %f}\n", r.y1, r.x1, r.Width(), r.Height());
        if (!stageCtorVal.IsFunctionName()) // is it resolved already? if not - skip
        {
            ObjectInterface* pstageObj = stageCtorVal.ToObject(fn.Env);
            if (pstageObj)
            {
                static_cast<StageCtorFunction*>(pstageObj)->NotifyOnResize(fn.Env);
            }
            else
                SF_ASSERT(0);
        }
    }
    else
        SF_ASSERT(0);
}

void StageCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}

FunctionRef StageCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) StageCtorFunction(&sc, pgc->GetMovieImpl()));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) StageProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Stage, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Stage), Value(ctor));
    return ctor;
}

}}} // SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_STAGE
