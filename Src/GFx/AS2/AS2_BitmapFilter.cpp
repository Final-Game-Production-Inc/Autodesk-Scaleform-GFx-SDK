/**************************************************************************

Filename    :   AS2_BitmapFilter.cpp
Content     :   BitmapFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "GFx/AS2/AS2_BitmapFilter.h"

#include "GFx/AS2/AS2_Action.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//BitmapFilterObject::BitmapFilterObject(FilterDesc::FilterType ft)
//{
//    Filter.Flags = (UInt8)ft;
//}

BitmapFilterObject::BitmapFilterObject(Environment* penv, FilterType ft)
: Object(penv)
{
    SF_UNUSED(penv);
    pFilter = 0;
    switch(ft)
    {
    case Render::Filter_Blur:           pFilter = *SF_HEAP_AUTO_NEW(this) Render::BlurFilter();    break;
    case Render::Filter_Shadow:         pFilter = *SF_HEAP_AUTO_NEW(this) Render::ShadowFilter();  break;
    case Render::Filter_Glow:           pFilter = *SF_HEAP_AUTO_NEW(this) Render::GlowFilter();    break;
    case Render::Filter_Bevel:          pFilter = *SF_HEAP_AUTO_NEW(this) Render::BevelFilter();   break;

    // Non-blur filter implementations.
    case Render::Filter_ColorMatrix:    pFilter = *SF_HEAP_AUTO_NEW(this) Render::ColorMatrixFilter();   break; 

    case Render::Filter_GradientGlow:   // Not implemented.
    case Render::Filter_GradientBevel:  // Not implemented.
    case Render::Filter_Convolution:    // Not implemented.
    case Render::Filter_DisplacementMap:// Not implemented.
        break;

    case Render::Filter_CacheAsBitmap:
        SF_DEBUG_ASSERT(1,"Should never construct a CacheAsBitmap filter via this method"); 
        break;
    }
}

BitmapFilterObject::~BitmapFilterObject()
{

}

void BitmapFilterObject::Finalize_GC()
{
    Object::Finalize_GC();
    pFilter = NULL;
}

// --------------------------------------------------------------------


BitmapFilterProto::BitmapFilterProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<BitmapFilterObject>(psc, prototype, constructor)
{
}


// --------------------------------------------------------------------


BitmapFilterCtorFunction::BitmapFilterCtorFunction(ASStringContext* psc)
:   CFunctionObject(psc, NULL)
{
    SF_UNUSED(psc);
}


FunctionRef BitmapFilterCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) BitmapFilterCtorFunction(&sc));
    Ptr<BitmapFilterProto> proto = 
        *SF_HEAP_NEW(pgc->GetHeap()) BitmapFilterProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_BitmapFilter, proto);
    pgc->FlashFiltersPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_BitmapFilter), Value(ctor));
    return ctor;
}

BitmapFilterObject* BitmapFilterObject::CreateFromDesc(Environment* penv, const Filter& filter)
{
    BitmapFilterObject* pobj;
    ASBuiltinType type;
    switch(filter.GetFilterType()) 
    {
    case Filter_Blur:        type = ASBuiltin_BlurFilter; break;
    case Filter_Shadow:      type = ASBuiltin_DropShadowFilter; break;
    case Filter_Glow:        type = ASBuiltin_GlowFilter; break;
    case Filter_Bevel:       type = ASBuiltin_BevelFilter; break;
    case Filter_ColorMatrix: type = ASBuiltin_ColorMatrixFilter; break;
    default:
        return 0;
    }
    pobj = static_cast<BitmapFilterObject*>(penv->OperatorNew(penv->GetGC()->FlashFiltersPackage,
        penv->GetBuiltin(type), 0));
    if (pobj)
        pobj->pFilter = *filter.Clone();
    return pobj;
}

}}} //SF::GFx::AS2

#endif  // GFX_AS2_ENABLE_FILTERS
