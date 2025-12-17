//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_GradientGlowFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_GradientGlowFilter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "../Display/AS3_Obj_Display_Graphics.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_alphasGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientGlowFilter_alphasGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_alphasSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientGlowFilter_alphasSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_angleGet, Value::Number> TFunc_Instances_GradientGlowFilter_angleGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_angleSet, const Value, Value::Number> TFunc_Instances_GradientGlowFilter_angleSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_blurXGet, Value::Number> TFunc_Instances_GradientGlowFilter_blurXGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_blurXSet, const Value, Value::Number> TFunc_Instances_GradientGlowFilter_blurXSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_blurYGet, Value::Number> TFunc_Instances_GradientGlowFilter_blurYGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_blurYSet, const Value, Value::Number> TFunc_Instances_GradientGlowFilter_blurYSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_colorsGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientGlowFilter_colorsGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_colorsSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientGlowFilter_colorsSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_distanceGet, Value::Number> TFunc_Instances_GradientGlowFilter_distanceGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_distanceSet, const Value, Value::Number> TFunc_Instances_GradientGlowFilter_distanceSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_knockoutGet, bool> TFunc_Instances_GradientGlowFilter_knockoutGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_knockoutSet, const Value, bool> TFunc_Instances_GradientGlowFilter_knockoutSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_qualityGet, SInt32> TFunc_Instances_GradientGlowFilter_qualityGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_qualitySet, const Value, SInt32> TFunc_Instances_GradientGlowFilter_qualitySet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_ratiosGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientGlowFilter_ratiosGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_ratiosSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientGlowFilter_ratiosSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_strengthGet, Value::Number> TFunc_Instances_GradientGlowFilter_strengthGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_strengthSet, const Value, Value::Number> TFunc_Instances_GradientGlowFilter_strengthSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_typeGet, ASString> TFunc_Instances_GradientGlowFilter_typeGet;
typedef ThunkFunc1<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_typeSet, const Value, const ASString&> TFunc_Instances_GradientGlowFilter_typeSet;
typedef ThunkFunc0<Instances::fl_filters::GradientGlowFilter, Instances::fl_filters::GradientGlowFilter::mid_clone, SPtr<Instances::fl_filters::BitmapFilter> > TFunc_Instances_GradientGlowFilter_clone;

template <> const TFunc_Instances_GradientGlowFilter_alphasGet::TMethod TFunc_Instances_GradientGlowFilter_alphasGet::Method = &Instances::fl_filters::GradientGlowFilter::alphasGet;
template <> const TFunc_Instances_GradientGlowFilter_alphasSet::TMethod TFunc_Instances_GradientGlowFilter_alphasSet::Method = &Instances::fl_filters::GradientGlowFilter::alphasSet;
template <> const TFunc_Instances_GradientGlowFilter_angleGet::TMethod TFunc_Instances_GradientGlowFilter_angleGet::Method = &Instances::fl_filters::GradientGlowFilter::angleGet;
template <> const TFunc_Instances_GradientGlowFilter_angleSet::TMethod TFunc_Instances_GradientGlowFilter_angleSet::Method = &Instances::fl_filters::GradientGlowFilter::angleSet;
template <> const TFunc_Instances_GradientGlowFilter_blurXGet::TMethod TFunc_Instances_GradientGlowFilter_blurXGet::Method = &Instances::fl_filters::GradientGlowFilter::blurXGet;
template <> const TFunc_Instances_GradientGlowFilter_blurXSet::TMethod TFunc_Instances_GradientGlowFilter_blurXSet::Method = &Instances::fl_filters::GradientGlowFilter::blurXSet;
template <> const TFunc_Instances_GradientGlowFilter_blurYGet::TMethod TFunc_Instances_GradientGlowFilter_blurYGet::Method = &Instances::fl_filters::GradientGlowFilter::blurYGet;
template <> const TFunc_Instances_GradientGlowFilter_blurYSet::TMethod TFunc_Instances_GradientGlowFilter_blurYSet::Method = &Instances::fl_filters::GradientGlowFilter::blurYSet;
template <> const TFunc_Instances_GradientGlowFilter_colorsGet::TMethod TFunc_Instances_GradientGlowFilter_colorsGet::Method = &Instances::fl_filters::GradientGlowFilter::colorsGet;
template <> const TFunc_Instances_GradientGlowFilter_colorsSet::TMethod TFunc_Instances_GradientGlowFilter_colorsSet::Method = &Instances::fl_filters::GradientGlowFilter::colorsSet;
template <> const TFunc_Instances_GradientGlowFilter_distanceGet::TMethod TFunc_Instances_GradientGlowFilter_distanceGet::Method = &Instances::fl_filters::GradientGlowFilter::distanceGet;
template <> const TFunc_Instances_GradientGlowFilter_distanceSet::TMethod TFunc_Instances_GradientGlowFilter_distanceSet::Method = &Instances::fl_filters::GradientGlowFilter::distanceSet;
template <> const TFunc_Instances_GradientGlowFilter_knockoutGet::TMethod TFunc_Instances_GradientGlowFilter_knockoutGet::Method = &Instances::fl_filters::GradientGlowFilter::knockoutGet;
template <> const TFunc_Instances_GradientGlowFilter_knockoutSet::TMethod TFunc_Instances_GradientGlowFilter_knockoutSet::Method = &Instances::fl_filters::GradientGlowFilter::knockoutSet;
template <> const TFunc_Instances_GradientGlowFilter_qualityGet::TMethod TFunc_Instances_GradientGlowFilter_qualityGet::Method = &Instances::fl_filters::GradientGlowFilter::qualityGet;
template <> const TFunc_Instances_GradientGlowFilter_qualitySet::TMethod TFunc_Instances_GradientGlowFilter_qualitySet::Method = &Instances::fl_filters::GradientGlowFilter::qualitySet;
template <> const TFunc_Instances_GradientGlowFilter_ratiosGet::TMethod TFunc_Instances_GradientGlowFilter_ratiosGet::Method = &Instances::fl_filters::GradientGlowFilter::ratiosGet;
template <> const TFunc_Instances_GradientGlowFilter_ratiosSet::TMethod TFunc_Instances_GradientGlowFilter_ratiosSet::Method = &Instances::fl_filters::GradientGlowFilter::ratiosSet;
template <> const TFunc_Instances_GradientGlowFilter_strengthGet::TMethod TFunc_Instances_GradientGlowFilter_strengthGet::Method = &Instances::fl_filters::GradientGlowFilter::strengthGet;
template <> const TFunc_Instances_GradientGlowFilter_strengthSet::TMethod TFunc_Instances_GradientGlowFilter_strengthSet::Method = &Instances::fl_filters::GradientGlowFilter::strengthSet;
template <> const TFunc_Instances_GradientGlowFilter_typeGet::TMethod TFunc_Instances_GradientGlowFilter_typeGet::Method = &Instances::fl_filters::GradientGlowFilter::typeGet;
template <> const TFunc_Instances_GradientGlowFilter_typeSet::TMethod TFunc_Instances_GradientGlowFilter_typeSet::Method = &Instances::fl_filters::GradientGlowFilter::typeSet;
template <> const TFunc_Instances_GradientGlowFilter_clone::TMethod TFunc_Instances_GradientGlowFilter_clone::Method = &Instances::fl_filters::GradientGlowFilter::clone;

namespace Instances { namespace fl_filters
{
    GradientGlowFilter::GradientGlowFilter(InstanceTraits::Traits& t)
    : Instances::fl_filters::BitmapFilter(t)
//##protect##"instance::GradientGlowFilter::GradientGlowFilter()$data"
      , Type(GetStringManager().CreateEmptyString())
//##protect##"instance::GradientGlowFilter::GradientGlowFilter()$data"
    {
//##protect##"instance::GradientGlowFilter::GradientGlowFilter()$code"
        FilterData = *SF_NEW Render::GradientFilter(Render::Filter_GradientGlow);
//##protect##"instance::GradientGlowFilter::GradientGlowFilter()$code"
    }

    void GradientGlowFilter::alphasGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientGlowFilter::alphasGet()"
        result = Alphas;
//##protect##"instance::GradientGlowFilter::alphasGet()"
    }
    void GradientGlowFilter::alphasSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientGlowFilter::alphasSet()"
        SF_UNUSED(result);
        Alphas = value;
        recomputeGradient();
//##protect##"instance::GradientGlowFilter::alphasSet()"
    }
    void GradientGlowFilter::angleGet(Value::Number& result)
    {
//##protect##"instance::GradientGlowFilter::angleGet()"
        result = GetGradientGlowFilterData()->GetParams().Colors[0].GetAlpha() / 255.0;
//##protect##"instance::GradientGlowFilter::angleGet()"
    }
    void GradientGlowFilter::angleSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientGlowFilter::angleSet()"
        SF_UNUSED(result);
        GetGradientGlowFilterData()->GetParams().Colors[0].SetAlpha((UByte)(value * 255));
//##protect##"instance::GradientGlowFilter::angleSet()"
    }
    void GradientGlowFilter::blurXGet(Value::Number& result)
    {
//##protect##"instance::GradientGlowFilter::blurXGet()"
        result = TwipsToPixels(GetGradientGlowFilterData()->GetParams().BlurX);
//##protect##"instance::GradientGlowFilter::blurXGet()"
    }
    void GradientGlowFilter::blurXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientGlowFilter::blurXSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetGradientGlowFilterData()->GetParams().BlurX = PixelsToTwips((float)value);
//##protect##"instance::GradientGlowFilter::blurXSet()"
    }
    void GradientGlowFilter::blurYGet(Value::Number& result)
    {
//##protect##"instance::GradientGlowFilter::blurYGet()"
        result = TwipsToPixels(GetGradientGlowFilterData()->GetParams().BlurY);
//##protect##"instance::GradientGlowFilter::blurYGet()"
    }
    void GradientGlowFilter::blurYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientGlowFilter::blurYSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetGradientGlowFilterData()->GetParams().BlurY = PixelsToTwips((float)value);
//##protect##"instance::GradientGlowFilter::blurYSet()"
    }
    void GradientGlowFilter::colorsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientGlowFilter::colorsGet()"
        result = Colors;
//##protect##"instance::GradientGlowFilter::colorsGet()"
    }
    void GradientGlowFilter::colorsSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientGlowFilter::colorsSet()"
        SF_UNUSED(result);
        Colors = value;
        recomputeGradient();
//##protect##"instance::GradientGlowFilter::colorsSet()"
    }
    void GradientGlowFilter::distanceGet(Value::Number& result)
    {
//##protect##"instance::GradientGlowFilter::distanceGet()"
        result = TwipsToPixels(GetGradientGlowFilterData()->GetDistance());
//##protect##"instance::GradientGlowFilter::distanceGet()"
    }
    void GradientGlowFilter::distanceSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientGlowFilter::distanceSet()"
        SF_UNUSED(result);
        GetGradientGlowFilterData()->SetAngleDistance( GetGradientGlowFilterData()->GetAngle(), PixelsToTwips((float)value));
//##protect##"instance::GradientGlowFilter::distanceSet()"
    }
    void GradientGlowFilter::knockoutGet(bool& result)
    {
//##protect##"instance::GradientGlowFilter::knockoutGet()"
        result = (GetGradientGlowFilterData()->GetParams().Mode & Render::BlurFilterParams::Mode_Knockout) != 0;
//##protect##"instance::GradientGlowFilter::knockoutGet()"
    }
    void GradientGlowFilter::knockoutSet(const Value& result, bool value)
    {
//##protect##"instance::GradientGlowFilter::knockoutSet()"
        SF_UNUSED(result);
        unsigned& mode = GetGradientGlowFilterData()->GetParams().Mode;
        mode &= ~Render::BlurFilterParams::Mode_Knockout;
        mode |= (value ? Render::BlurFilterParams::Mode_Knockout : 0);
//##protect##"instance::GradientGlowFilter::knockoutSet()"
    }
    void GradientGlowFilter::qualityGet(SInt32& result)
    {
//##protect##"instance::GradientGlowFilter::qualityGet()"
        result = (GetGradientGlowFilterData()->GetParams().Passes);
//##protect##"instance::GradientGlowFilter::qualityGet()"
    }
    void GradientGlowFilter::qualitySet(const Value& result, SInt32 value)
    {
//##protect##"instance::GradientGlowFilter::qualitySet()"
        SF_UNUSED(result);
        GetGradientGlowFilterData()->GetParams().Passes = Alg::Clamp<unsigned>(value, 0, 15);
//##protect##"instance::GradientGlowFilter::qualitySet()"
    }
    void GradientGlowFilter::ratiosGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientGlowFilter::ratiosGet()"
        result = Ratios;
//##protect##"instance::GradientGlowFilter::ratiosGet()"
    }
    void GradientGlowFilter::ratiosSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientGlowFilter::ratiosSet()"
        SF_UNUSED(result);
        Ratios = value;
        recomputeGradient();
//##protect##"instance::GradientGlowFilter::ratiosSet()"
    }
    void GradientGlowFilter::strengthGet(Value::Number& result)
    {
//##protect##"instance::GradientGlowFilter::strengthGet()"
        result = GetGradientGlowFilterData()->GetParams().Strength;
//##protect##"instance::GradientGlowFilter::strengthGet()"
    }
    void GradientGlowFilter::strengthSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientGlowFilter::strengthSet()"
        SF_UNUSED(result);
        GetGradientGlowFilterData()->GetParams().Strength = (float)value;
//##protect##"instance::GradientGlowFilter::strengthSet()"
    }
    void GradientGlowFilter::typeGet(ASString& result)
    {
//##protect##"instance::GradientGlowFilter::typeGet()"
        result = Type;
//##protect##"instance::GradientGlowFilter::typeGet()"
    }
    void GradientGlowFilter::typeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::GradientGlowFilter::typeSet()"
        SF_UNUSED(result);
        unsigned& mode = GetGradientGlowFilterData()->GetParams().Mode;
        mode &= ~(Render::BlurFilterParams::Mode_Inner|Render::BlurFilterParams::Mode_Highlight);
        if ( value == "inner" )
        {
            mode |= Render::BlurFilterParams::Mode_Inner;
        }
        else if ( value == "outer" )
        {
            // Nothing.
        }
        else // "full" is the default if the string is not recognized as a valid type. No expection is thrown.
        {
            mode |= Render::BlurFilterParams::Mode_Highlight;
        }
//##protect##"instance::GradientGlowFilter::typeSet()"
    }
    void GradientGlowFilter::clone(SPtr<Instances::fl_filters::BitmapFilter>& result)
    {
//##protect##"instance::GradientGlowFilter::clone()"
        InstanceTraits::fl_filters::GradientGlowFilter& itr = static_cast<InstanceTraits::fl_filters::GradientGlowFilter&>(GetTraits());
        Pickable<GradientGlowFilter> r = itr.MakeInstance(itr);

        Value::Number dist, angl;
        SPtr<Instances::fl::Array> colors, alphas, ratios;
        Value::Number blurX, blurY, stren;
        ASString type(GetStringManager().CreateEmptyString());
        SInt32 qual;
        bool knock;

        distanceGet(dist);
        angleGet(angl);
        colorsGet(colors);
        alphasGet(alphas);
        ratiosGet(ratios);
        blurXGet(blurX);
        blurYGet(blurY);
        strengthGet(stren);
        qualityGet(qual);
        typeGet(type);
        knockoutGet(knock);

        Value tempResult;
        r->distanceSet(tempResult, dist);
        r->angleSet(tempResult, angl);
        r->colorsSet(tempResult, colors);
        r->alphasSet(tempResult, alphas);
        r->ratiosSet(tempResult, ratios);
        r->blurXSet(tempResult, blurX);
        r->blurYSet(tempResult, blurY);
        r->strengthSet(tempResult, stren);
        r->qualitySet(tempResult, qual);
        r->typeSet(tempResult, type);
        r->knockoutSet(tempResult, knock);
        result = r;
//##protect##"instance::GradientGlowFilter::clone()"
    }

    SPtr<Instances::fl::Array> GradientGlowFilter::alphasGet()
    {
        SPtr<Instances::fl::Array> result;
        alphasGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> GradientGlowFilter::colorsGet()
    {
        SPtr<Instances::fl::Array> result;
        colorsGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> GradientGlowFilter::ratiosGet()
    {
        SPtr<Instances::fl::Array> result;
        ratiosGet(result);
        return result;
    }
    SPtr<Instances::fl_filters::BitmapFilter> GradientGlowFilter::clone()
    {
        SPtr<Instances::fl_filters::BitmapFilter> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void GradientGlowFilter::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 11)
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("flash.filters::GradientGlowFilter()") SF_DEBUG_ARG(0) SF_DEBUG_ARG(11) SF_DEBUG_ARG(argc)));
            return;
        }

        Value::Number dist(4), angl(45);
        Value::Number blurX(4), blurY(4), stren(1);
        SInt32 qual(1);
        bool knock(false);
        StringManager& sm = GetVM().GetStringManager();
        ASString type = sm.CreateConstString("inner");

        if ( argc >= 1 && !argv[0].Convert2Number(dist)) return;
        if ( argc >= 2 && !argv[1].Convert2Number(angl)) return;

        // NOTE: It is valid for any of the colors, alphas and ratios arrays to be null in AS3. If colors and/or alphas are NULL,
        // the filter is just ignored. If the ratios is null (and colors and alphas are non-null), a ratios array is implied that
        // spreads the entries of colors and alphas over the 0-255 range evenly.
        if ( argc >= 3)
        {
            if (!argv[2].IsNull() && !GetVM().IsOfType(argv[2], "Array", GetVM().GetCurrentAppDomain()))
            {
                GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                    SF_DEBUG_ARG(GetVM().GetValueTraits(argv[2]).GetName().ToCStr())
                    SF_DEBUG_ARG("Array")));
                return;
            }
            Colors = reinterpret_cast<Instances::fl::Array*>(argv[2].GetObject());
        }
        if ( argc >= 4)
        {
            if (!argv[3].IsNull() && !GetVM().IsOfType(argv[3], "Array", GetVM().GetCurrentAppDomain()))
            {
                GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                    SF_DEBUG_ARG(GetVM().GetValueTraits(argv[3]).GetName().ToCStr())
                    SF_DEBUG_ARG("Array")));
                return;
            }
            Alphas = reinterpret_cast<Instances::fl::Array*>(argv[3].GetObject());
        }
        if ( argc >= 5)
        {
            if (!argv[4].IsNull() && !GetVM().IsOfType(argv[4], "Array", GetVM().GetCurrentAppDomain()))
            {
                GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                    SF_DEBUG_ARG(GetVM().GetValueTraits(argv[4]).GetName().ToCStr())
                    SF_DEBUG_ARG("Array")));
                return;
            }
            Ratios = reinterpret_cast<Instances::fl::Array*>(argv[4].GetObject());
        }
        if ( argc >= 6 && !argv[5].Convert2Number(blurX)) return;
        if ( argc >= 7 && !argv[6].Convert2Number(blurY)) return;
        if ( argc >= 8 && !argv[7].Convert2Number(stren)) return;
        if ( argc >= 9 && !argv[8].Convert2Int32(qual)) return;
        if ( argc >= 10 && !argv[9].Convert2String(type)) return;
        if ( argc >= 11 ) knock = argv[10].Convert2Boolean();

        Value result;
        distanceSet(result, dist);
        angleSet(result, angl);
        blurXSet(result, blurX);
        blurYSet(result, blurY);
        strengthSet(result, stren);
        qualitySet(result, qual);
        typeSet(result, type);
        knockoutSet(result, knock);

        recomputeGradient();
    }

    void GradientGlowFilter::recomputeGradient()
    {
        // Gradient data must live in a global heap since it is used as a key in ResourceLib.
        Render::GradientType gradType = Render::GradientLinear;
        if (Type == "radial")
        {
            gradType = Render::GradientRadial;
        }

        // If Colors or Alphas are NULL, then no gradient is generated.
        pGradientData = 0;
        if (Colors != 0 && Alphas != 0)
        {
            pGradientData = *SF_NEW Render::GradientData(gradType, (UInt16)Colors->GetSize(), false);
            Instances::fl_display::Graphics::FillGradientData(Colors, Alphas, Ratios, pGradientData);
        }
        GetGradientGlowFilterData()->GetParams().Gradient = pGradientData;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    // const UInt16 GradientGlowFilter::tito[GradientGlowFilter::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 27, 28, 30, 31, 33, 
    // };
    const TypeInfo* GradientGlowFilter::tit[34] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_filters::BitmapFilterTI, 
    };
    const ThunkInfo GradientGlowFilter::ti[GradientGlowFilter::ThunkInfoNum] = {
        {TFunc_Instances_GradientGlowFilter_alphasGet::Func, &GradientGlowFilter::tit[0], "alphas", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_alphasSet::Func, &GradientGlowFilter::tit[1], "alphas", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_angleGet::Func, &GradientGlowFilter::tit[3], "angle", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_angleSet::Func, &GradientGlowFilter::tit[4], "angle", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_blurXGet::Func, &GradientGlowFilter::tit[6], "blurX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_blurXSet::Func, &GradientGlowFilter::tit[7], "blurX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_blurYGet::Func, &GradientGlowFilter::tit[9], "blurY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_blurYSet::Func, &GradientGlowFilter::tit[10], "blurY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_colorsGet::Func, &GradientGlowFilter::tit[12], "colors", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_colorsSet::Func, &GradientGlowFilter::tit[13], "colors", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_distanceGet::Func, &GradientGlowFilter::tit[15], "distance", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_distanceSet::Func, &GradientGlowFilter::tit[16], "distance", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_knockoutGet::Func, &GradientGlowFilter::tit[18], "knockout", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_knockoutSet::Func, &GradientGlowFilter::tit[19], "knockout", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_qualityGet::Func, &GradientGlowFilter::tit[21], "quality", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_qualitySet::Func, &GradientGlowFilter::tit[22], "quality", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_ratiosGet::Func, &GradientGlowFilter::tit[24], "ratios", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_ratiosSet::Func, &GradientGlowFilter::tit[25], "ratios", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_strengthGet::Func, &GradientGlowFilter::tit[27], "strength", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_strengthSet::Func, &GradientGlowFilter::tit[28], "strength", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_typeGet::Func, &GradientGlowFilter::tit[30], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_typeSet::Func, &GradientGlowFilter::tit[31], "type", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientGlowFilter_clone::Func, &GradientGlowFilter::tit[33], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GradientGlowFilter::GradientGlowFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"InstanceTraits::GradientGlowFilter::GradientGlowFilter()"
//##protect##"InstanceTraits::GradientGlowFilter::GradientGlowFilter()"

    }

    void GradientGlowFilter::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GradientGlowFilter&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filters
{

    GradientGlowFilter::GradientGlowFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"ClassTraits::GradientGlowFilter::GradientGlowFilter()"
//##protect##"ClassTraits::GradientGlowFilter::GradientGlowFilter()"

    }

    Pickable<Traits> GradientGlowFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GradientGlowFilter(vm, AS3::fl_filters::GradientGlowFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::GradientGlowFilterCI));
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
    const TypeInfo GradientGlowFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::GradientGlowFilter::InstanceType),
        0,
        0,
        InstanceTraits::fl_filters::GradientGlowFilter::ThunkInfoNum,
        0,
        "GradientGlowFilter", "flash.filters", &fl_filters::BitmapFilterTI,
        TypeInfo::None
    };

    const ClassInfo GradientGlowFilterCI = {
        &GradientGlowFilterTI,
        ClassTraits::fl_filters::GradientGlowFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filters::GradientGlowFilter::ti,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

