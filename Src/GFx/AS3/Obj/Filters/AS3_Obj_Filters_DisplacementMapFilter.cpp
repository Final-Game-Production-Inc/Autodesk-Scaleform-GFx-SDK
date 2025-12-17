//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_DisplacementMapFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_DisplacementMapFilter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Filters_DisplacementMapFilterMode.h"
#include "../Display/AS3_Obj_Display_BitmapData.h"
#include "../Geom/AS3_Obj_Geom_Point.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_alphaGet, Value::Number> TFunc_Instances_DisplacementMapFilter_alphaGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_alphaSet, const Value, Value::Number> TFunc_Instances_DisplacementMapFilter_alphaSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_colorGet, UInt32> TFunc_Instances_DisplacementMapFilter_colorGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_colorSet, const Value, UInt32> TFunc_Instances_DisplacementMapFilter_colorSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_componentXGet, UInt32> TFunc_Instances_DisplacementMapFilter_componentXGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_componentXSet, const Value, UInt32> TFunc_Instances_DisplacementMapFilter_componentXSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_componentYGet, UInt32> TFunc_Instances_DisplacementMapFilter_componentYGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_componentYSet, const Value, UInt32> TFunc_Instances_DisplacementMapFilter_componentYSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_mapBitmapGet, SPtr<Instances::fl_display::BitmapData> > TFunc_Instances_DisplacementMapFilter_mapBitmapGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_mapBitmapSet, const Value, Instances::fl_display::BitmapData*> TFunc_Instances_DisplacementMapFilter_mapBitmapSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_mapPointGet, SPtr<Instances::fl_geom::Point> > TFunc_Instances_DisplacementMapFilter_mapPointGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_mapPointSet, const Value, Instances::fl_geom::Point*> TFunc_Instances_DisplacementMapFilter_mapPointSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_modeGet, ASString> TFunc_Instances_DisplacementMapFilter_modeGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_modeSet, const Value, const ASString&> TFunc_Instances_DisplacementMapFilter_modeSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_scaleXGet, Value::Number> TFunc_Instances_DisplacementMapFilter_scaleXGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_scaleXSet, const Value, Value::Number> TFunc_Instances_DisplacementMapFilter_scaleXSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_scaleYGet, Value::Number> TFunc_Instances_DisplacementMapFilter_scaleYGet;
typedef ThunkFunc1<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_scaleYSet, const Value, Value::Number> TFunc_Instances_DisplacementMapFilter_scaleYSet;
typedef ThunkFunc0<Instances::fl_filters::DisplacementMapFilter, Instances::fl_filters::DisplacementMapFilter::mid_clone, SPtr<Instances::fl_filters::BitmapFilter> > TFunc_Instances_DisplacementMapFilter_clone;

template <> const TFunc_Instances_DisplacementMapFilter_alphaGet::TMethod TFunc_Instances_DisplacementMapFilter_alphaGet::Method = &Instances::fl_filters::DisplacementMapFilter::alphaGet;
template <> const TFunc_Instances_DisplacementMapFilter_alphaSet::TMethod TFunc_Instances_DisplacementMapFilter_alphaSet::Method = &Instances::fl_filters::DisplacementMapFilter::alphaSet;
template <> const TFunc_Instances_DisplacementMapFilter_colorGet::TMethod TFunc_Instances_DisplacementMapFilter_colorGet::Method = &Instances::fl_filters::DisplacementMapFilter::colorGet;
template <> const TFunc_Instances_DisplacementMapFilter_colorSet::TMethod TFunc_Instances_DisplacementMapFilter_colorSet::Method = &Instances::fl_filters::DisplacementMapFilter::colorSet;
template <> const TFunc_Instances_DisplacementMapFilter_componentXGet::TMethod TFunc_Instances_DisplacementMapFilter_componentXGet::Method = &Instances::fl_filters::DisplacementMapFilter::componentXGet;
template <> const TFunc_Instances_DisplacementMapFilter_componentXSet::TMethod TFunc_Instances_DisplacementMapFilter_componentXSet::Method = &Instances::fl_filters::DisplacementMapFilter::componentXSet;
template <> const TFunc_Instances_DisplacementMapFilter_componentYGet::TMethod TFunc_Instances_DisplacementMapFilter_componentYGet::Method = &Instances::fl_filters::DisplacementMapFilter::componentYGet;
template <> const TFunc_Instances_DisplacementMapFilter_componentYSet::TMethod TFunc_Instances_DisplacementMapFilter_componentYSet::Method = &Instances::fl_filters::DisplacementMapFilter::componentYSet;
template <> const TFunc_Instances_DisplacementMapFilter_mapBitmapGet::TMethod TFunc_Instances_DisplacementMapFilter_mapBitmapGet::Method = &Instances::fl_filters::DisplacementMapFilter::mapBitmapGet;
template <> const TFunc_Instances_DisplacementMapFilter_mapBitmapSet::TMethod TFunc_Instances_DisplacementMapFilter_mapBitmapSet::Method = &Instances::fl_filters::DisplacementMapFilter::mapBitmapSet;
template <> const TFunc_Instances_DisplacementMapFilter_mapPointGet::TMethod TFunc_Instances_DisplacementMapFilter_mapPointGet::Method = &Instances::fl_filters::DisplacementMapFilter::mapPointGet;
template <> const TFunc_Instances_DisplacementMapFilter_mapPointSet::TMethod TFunc_Instances_DisplacementMapFilter_mapPointSet::Method = &Instances::fl_filters::DisplacementMapFilter::mapPointSet;
template <> const TFunc_Instances_DisplacementMapFilter_modeGet::TMethod TFunc_Instances_DisplacementMapFilter_modeGet::Method = &Instances::fl_filters::DisplacementMapFilter::modeGet;
template <> const TFunc_Instances_DisplacementMapFilter_modeSet::TMethod TFunc_Instances_DisplacementMapFilter_modeSet::Method = &Instances::fl_filters::DisplacementMapFilter::modeSet;
template <> const TFunc_Instances_DisplacementMapFilter_scaleXGet::TMethod TFunc_Instances_DisplacementMapFilter_scaleXGet::Method = &Instances::fl_filters::DisplacementMapFilter::scaleXGet;
template <> const TFunc_Instances_DisplacementMapFilter_scaleXSet::TMethod TFunc_Instances_DisplacementMapFilter_scaleXSet::Method = &Instances::fl_filters::DisplacementMapFilter::scaleXSet;
template <> const TFunc_Instances_DisplacementMapFilter_scaleYGet::TMethod TFunc_Instances_DisplacementMapFilter_scaleYGet::Method = &Instances::fl_filters::DisplacementMapFilter::scaleYGet;
template <> const TFunc_Instances_DisplacementMapFilter_scaleYSet::TMethod TFunc_Instances_DisplacementMapFilter_scaleYSet::Method = &Instances::fl_filters::DisplacementMapFilter::scaleYSet;
template <> const TFunc_Instances_DisplacementMapFilter_clone::TMethod TFunc_Instances_DisplacementMapFilter_clone::Method = &Instances::fl_filters::DisplacementMapFilter::clone;

namespace Instances { namespace fl_filters
{
    DisplacementMapFilter::DisplacementMapFilter(InstanceTraits::Traits& t)
    : Instances::fl_filters::BitmapFilter(t)
//##protect##"instance::DisplacementMapFilter::DisplacementMapFilter()$data"
//##protect##"instance::DisplacementMapFilter::DisplacementMapFilter()$data"
    {
//##protect##"instance::DisplacementMapFilter::DisplacementMapFilter()$code"
        FilterData = *SF_NEW Render::DisplacementMapFilter();
//##protect##"instance::DisplacementMapFilter::DisplacementMapFilter()$code"
    }

    void DisplacementMapFilter::alphaGet(Value::Number& result)
    {
//##protect##"instance::DisplacementMapFilter::alphaGet()"
        float alpha;
        GetDisplacementMapFilterData()->ColorValue.GetAlphaFloat(&alpha);
        result = alpha;
//##protect##"instance::DisplacementMapFilter::alphaGet()"
    }
    void DisplacementMapFilter::alphaSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplacementMapFilter::alphaSet()"
        SF_UNUSED1(result);
        GetDisplacementMapFilterData()->ColorValue.SetAlphaFloat((float)value);
//##protect##"instance::DisplacementMapFilter::alphaSet()"
    }
    void DisplacementMapFilter::colorGet(UInt32& result)
    {
//##protect##"instance::DisplacementMapFilter::colorGet()"
        Color rawColor = GetDisplacementMapFilterData()->ColorValue;
        rawColor.SetAlpha(0); // alpha is always zero.
        result = rawColor.ToColor32();
//##protect##"instance::DisplacementMapFilter::colorGet()"
    }
    void DisplacementMapFilter::colorSet(const Value& result, UInt32 value)
    {
//##protect##"instance::DisplacementMapFilter::colorSet()"
        SF_UNUSED1(result);
        Color newColor(value);
        GetDisplacementMapFilterData()->ColorValue.SetRGB(newColor.GetRed(), newColor.GetGreen(), newColor.GetBlue());
//##protect##"instance::DisplacementMapFilter::colorSet()"
    }
    void DisplacementMapFilter::componentXGet(UInt32& result)
    {
//##protect##"instance::DisplacementMapFilter::componentXGet()"
        result = GetDisplacementMapFilterData()->ComponentX;
//##protect##"instance::DisplacementMapFilter::componentXGet()"
    }
    void DisplacementMapFilter::componentXSet(const Value& result, UInt32 value)
    {
//##protect##"instance::DisplacementMapFilter::componentXSet()"
        SF_UNUSED(result);
        GetDisplacementMapFilterData()->ComponentX = (Render::DrawableImage::ChannelBits)value;
//##protect##"instance::DisplacementMapFilter::componentXSet()"
    }
    void DisplacementMapFilter::componentYGet(UInt32& result)
    {
//##protect##"instance::DisplacementMapFilter::componentYGet()"
        result = GetDisplacementMapFilterData()->ComponentY;
//##protect##"instance::DisplacementMapFilter::componentYGet()"
    }
    void DisplacementMapFilter::componentYSet(const Value& result, UInt32 value)
    {
//##protect##"instance::DisplacementMapFilter::componentYSet()"
        SF_UNUSED(result);
        GetDisplacementMapFilterData()->ComponentY = (Render::DrawableImage::ChannelBits)value;
//##protect##"instance::DisplacementMapFilter::componentYSet()"
    }
    void DisplacementMapFilter::mapBitmapGet(SPtr<Instances::fl_display::BitmapData>& result)
    {
//##protect##"instance::DisplacementMapFilter::mapBitmapGet()"
        result = MapBitmap;
//##protect##"instance::DisplacementMapFilter::mapBitmapGet()"
    }
    void DisplacementMapFilter::mapBitmapSet(const Value& result, Instances::fl_display::BitmapData* value)
    {
//##protect##"instance::DisplacementMapFilter::mapBitmapSet()"
        SF_UNUSED(result);
        MapBitmap = value;

        // Also need to change the texture within the Render::Filter's data.
        Render::DrawableImage* pimage = 0;
        if (MapBitmap)
        {
            pimage = value->getDrawableImageFromBitmapData(value);
        }
        GetDisplacementMapFilterData()->DisplacementMap = pimage;
//##protect##"instance::DisplacementMapFilter::mapBitmapSet()"
    }
    void DisplacementMapFilter::mapPointGet(SPtr<Instances::fl_geom::Point>& result)
    {
//##protect##"instance::DisplacementMapFilter::mapPointGet()"
        result->SetX((Value::Number)GetDisplacementMapFilterData()->MapPoint.x);
        result->SetY((Value::Number)GetDisplacementMapFilterData()->MapPoint.y);        
//##protect##"instance::DisplacementMapFilter::mapPointGet()"
    }
    void DisplacementMapFilter::mapPointSet(const Value& result, Instances::fl_geom::Point* value)
    {
//##protect##"instance::DisplacementMapFilter::mapPointSet()"
        SF_UNUSED(result);
        if (value)
            GetDisplacementMapFilterData()->MapPoint = PointF((float)value->GetX(), (float)value->GetY());
        else // Set to origin if NULL.            
            GetDisplacementMapFilterData()->MapPoint = PointF(0,0);
//##protect##"instance::DisplacementMapFilter::mapPointSet()"
    }
    void DisplacementMapFilter::modeGet(ASString& result)
    {
//##protect##"instance::DisplacementMapFilter::modeGet()"
        SF_UNUSED1(result);
        switch(GetDisplacementMapFilterData()->Mode)
        {
            default: 
                SF_DEBUG_WARNING1(1, "Unknown DisplacementMapFilter mode = %d. Defaulting to Wrap.", GetDisplacementMapFilterData()->Mode);
            case Render::DisplacementMapFilter::DisplacementMode_Wrap:      result = "wrap"; break;
            case Render::DisplacementMapFilter::DisplacementMode_Clamp:     result = "clamp"; break;
            case Render::DisplacementMapFilter::DisplacementMode_Ignore:    result = "ignore"; break;
            case Render::DisplacementMapFilter::DisplacementMode_Color:     result = "color"; break;
        }
//##protect##"instance::DisplacementMapFilter::modeGet()"
    }
    void DisplacementMapFilter::modeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::DisplacementMapFilter::modeSet()"
        SF_UNUSED(result);
        if (value == "wrap")
            GetDisplacementMapFilterData()->Mode = Render::DisplacementMapFilter::DisplacementMode_Wrap;
        else if (value == "clamp")
            GetDisplacementMapFilterData()->Mode = Render::DisplacementMapFilter::DisplacementMode_Wrap;
        else if (value == "ignore")
            GetDisplacementMapFilterData()->Mode = Render::DisplacementMapFilter::DisplacementMode_Wrap;
        else if (value == "color")
            GetDisplacementMapFilterData()->Mode = Render::DisplacementMapFilter::DisplacementMode_Wrap;
        else
        {
            SF_DEBUG_WARNING1(1, "Unrecognized DisplacementMapFilter mode = %s. Default to Wrap.", value.ToCStr());
            GetDisplacementMapFilterData()->Mode = Render::DisplacementMapFilter::DisplacementMode_Wrap;
        }
//##protect##"instance::DisplacementMapFilter::modeSet()"
    }
    void DisplacementMapFilter::scaleXGet(Value::Number& result)
    {
//##protect##"instance::DisplacementMapFilter::scaleXGet()"
        result = GetDisplacementMapFilterData()->ScaleX;
//##protect##"instance::DisplacementMapFilter::scaleXGet()"
    }
    void DisplacementMapFilter::scaleXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplacementMapFilter::scaleXSet()"
        SF_UNUSED(result);
        GetDisplacementMapFilterData()->ScaleX = (float)value;
//##protect##"instance::DisplacementMapFilter::scaleXSet()"
    }
    void DisplacementMapFilter::scaleYGet(Value::Number& result)
    {
//##protect##"instance::DisplacementMapFilter::scaleYGet()"
        result = GetDisplacementMapFilterData()->ScaleY;
//##protect##"instance::DisplacementMapFilter::scaleYGet()"
    }
    void DisplacementMapFilter::scaleYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplacementMapFilter::scaleYSet()"
        SF_UNUSED(result);
        GetDisplacementMapFilterData()->ScaleY = (float)value;
//##protect##"instance::DisplacementMapFilter::scaleYSet()"
    }
    void DisplacementMapFilter::clone(SPtr<Instances::fl_filters::BitmapFilter>& result)
    {
//##protect##"instance::DisplacementMapFilter::clone()"
        InstanceTraits::fl_filters::DisplacementMapFilter& itr = static_cast<InstanceTraits::fl_filters::DisplacementMapFilter&>(GetTraits());
        Pickable<DisplacementMapFilter> r = itr.MakeInstance(itr);

        Value::Number                           alpha;
        UInt32                                  color;
        UInt32                                  componentX;
        UInt32                                  componentY;
        SPtr<Instances::fl_display::BitmapData> mapBitmap;
        SPtr<Instances::fl_geom::Point>         mapPoint;
        ASString                                mode = GetVM().GetStringManager().CreateEmptyString();
        Value::Number                           scaleX;
        Value::Number                           scaleY;

        alphaGet(alpha);
        colorGet(color);
        componentXGet(componentX);
        componentYGet(componentY);
        mapBitmapGet(mapBitmap);
        mapPointGet(mapPoint);
        modeGet(mode);
        scaleXGet(scaleX);
        scaleYGet(scaleY);

        Value tempResult;
        r->alphaSet(tempResult, alpha);
        r->colorSet(tempResult, color);
        r->componentXSet(tempResult, componentX);
        r->componentYSet(tempResult, componentY);
        r->mapBitmapSet(tempResult, mapBitmap);
        r->mapPointSet(tempResult, mapPoint);
        r->modeSet(tempResult, mode);
        r->scaleXSet(tempResult, scaleX);
        r->scaleYSet(tempResult, scaleY);
        
        result = r;
//##protect##"instance::DisplacementMapFilter::clone()"
    }

    SPtr<Instances::fl_display::BitmapData> DisplacementMapFilter::mapBitmapGet()
    {
        SPtr<Instances::fl_display::BitmapData> result;
        mapBitmapGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Point> DisplacementMapFilter::mapPointGet()
    {
        SPtr<Instances::fl_geom::Point> result;
        mapPointGet(result);
        return result;
    }
    SPtr<Instances::fl_filters::BitmapFilter> DisplacementMapFilter::clone()
    {
        SPtr<Instances::fl_filters::BitmapFilter> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void DisplacementMapFilter::AS3Constructor(unsigned argc, const Value* argv)
    {
        Value tempResult;

        Instances::fl_display::BitmapData* map = 0;
        Instances::fl_geom::Point* point = 0;
        UInt32 componentX(0), componentY(0);
        Value::Number scaleX(0), scaleY(0);
        StringManager& sm = GetVM().GetStringManager();
        ASString mode = sm.CreateConstString("wrap");
        UInt32 color(0);
        Value::Number alpha(0);

        if (argc >= 1)
        {
            if (!GetVM().IsOfType(argv[0], "flash.display.BitmapData", GetVM().GetCurrentAppDomain()))
            {
                // TODO: This is not the correct error.
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("mapBitmap")));
                return;
            }
            map = static_cast<Instances::fl_display::BitmapData*>(argv[0].GetObject());
        }

        if (argc >= 2)
        {
            if (!GetVM().IsOfType(argv[1], "flash.geom.Point", GetVM().GetCurrentAppDomain()))
            {
                // This is not the correct error.
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("mapPoint")));
                return;
            }
            point = static_cast<Instances::fl_geom::Point*>(argv[1].GetObject());
        }

        if (argc >= 3 && !argv[2].Convert2UInt32(componentX)) return;
        if (argc >= 4 && !argv[3].Convert2UInt32(componentY)) return;
        if (argc >= 5 && !argv[4].Convert2Number(scaleX)) return;
        if (argc >= 6 && !argv[5].Convert2Number(scaleY)) return;
        if (argc >= 7 && !argv[6].Convert2String(mode)) return;
        if (argc >= 8 && !argv[7].Convert2UInt32(color)) return;
        if (argc >= 9 && !argv[8].Convert2Number(alpha)) return;

        mapBitmapSet(tempResult, map);
        mapPointSet(tempResult, point);
        componentXSet(tempResult, componentX);
        componentYSet(tempResult, componentY);
        scaleXSet(tempResult, scaleX);
        scaleYSet(tempResult, scaleY);
        modeSet(tempResult, mode);
        colorSet(tempResult, color);
        alphaSet(tempResult, alpha);
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    // const UInt16 DisplacementMapFilter::tito[DisplacementMapFilter::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 27, 
    // };
    const TypeInfo* DisplacementMapFilter::tit[28] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl_display::BitmapDataTI, 
        NULL, &AS3::fl_display::BitmapDataTI, 
        &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl_geom::PointTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_filters::BitmapFilterTI, 
    };
    const ThunkInfo DisplacementMapFilter::ti[DisplacementMapFilter::ThunkInfoNum] = {
        {TFunc_Instances_DisplacementMapFilter_alphaGet::Func, &DisplacementMapFilter::tit[0], "alpha", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_alphaSet::Func, &DisplacementMapFilter::tit[1], "alpha", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_colorGet::Func, &DisplacementMapFilter::tit[3], "color", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_colorSet::Func, &DisplacementMapFilter::tit[4], "color", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_componentXGet::Func, &DisplacementMapFilter::tit[6], "componentX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_componentXSet::Func, &DisplacementMapFilter::tit[7], "componentX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_componentYGet::Func, &DisplacementMapFilter::tit[9], "componentY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_componentYSet::Func, &DisplacementMapFilter::tit[10], "componentY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_mapBitmapGet::Func, &DisplacementMapFilter::tit[12], "mapBitmap", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_mapBitmapSet::Func, &DisplacementMapFilter::tit[13], "mapBitmap", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_mapPointGet::Func, &DisplacementMapFilter::tit[15], "mapPoint", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_mapPointSet::Func, &DisplacementMapFilter::tit[16], "mapPoint", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_modeGet::Func, &DisplacementMapFilter::tit[18], "mode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_modeSet::Func, &DisplacementMapFilter::tit[19], "mode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_scaleXGet::Func, &DisplacementMapFilter::tit[21], "scaleX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_scaleXSet::Func, &DisplacementMapFilter::tit[22], "scaleX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_scaleYGet::Func, &DisplacementMapFilter::tit[24], "scaleY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_scaleYSet::Func, &DisplacementMapFilter::tit[25], "scaleY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplacementMapFilter_clone::Func, &DisplacementMapFilter::tit[27], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    DisplacementMapFilter::DisplacementMapFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"InstanceTraits::DisplacementMapFilter::DisplacementMapFilter()"
//##protect##"InstanceTraits::DisplacementMapFilter::DisplacementMapFilter()"

    }

    void DisplacementMapFilter::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<DisplacementMapFilter&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filters
{

    DisplacementMapFilter::DisplacementMapFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"ClassTraits::DisplacementMapFilter::DisplacementMapFilter()"

//##protect##"ClassTraits::DisplacementMapFilter::DisplacementMapFilter()"

    }

    Pickable<Traits> DisplacementMapFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DisplacementMapFilter(vm, AS3::fl_filters::DisplacementMapFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::DisplacementMapFilterCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_filters
{
    const TypeInfo DisplacementMapFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::DisplacementMapFilter::InstanceType),
        0,
        0,
        InstanceTraits::fl_filters::DisplacementMapFilter::ThunkInfoNum,
        0,
        "DisplacementMapFilter", "flash.filters", &fl_filters::BitmapFilterTI,
        TypeInfo::None
    };

    const ClassInfo DisplacementMapFilterCI = {
        &DisplacementMapFilterTI,
        ClassTraits::fl_filters::DisplacementMapFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filters::DisplacementMapFilter::ti,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

