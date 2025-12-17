//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_GradientBevelFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_GradientBevelFilter.h"
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
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_alphasGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientBevelFilter_alphasGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_alphasSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientBevelFilter_alphasSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_angleGet, Value::Number> TFunc_Instances_GradientBevelFilter_angleGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_angleSet, const Value, Value::Number> TFunc_Instances_GradientBevelFilter_angleSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_blurXGet, Value::Number> TFunc_Instances_GradientBevelFilter_blurXGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_blurXSet, const Value, Value::Number> TFunc_Instances_GradientBevelFilter_blurXSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_blurYGet, Value::Number> TFunc_Instances_GradientBevelFilter_blurYGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_blurYSet, const Value, Value::Number> TFunc_Instances_GradientBevelFilter_blurYSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_colorsGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientBevelFilter_colorsGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_colorsSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientBevelFilter_colorsSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_distanceGet, Value::Number> TFunc_Instances_GradientBevelFilter_distanceGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_distanceSet, const Value, Value::Number> TFunc_Instances_GradientBevelFilter_distanceSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_knockoutGet, bool> TFunc_Instances_GradientBevelFilter_knockoutGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_knockoutSet, const Value, bool> TFunc_Instances_GradientBevelFilter_knockoutSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_qualityGet, SInt32> TFunc_Instances_GradientBevelFilter_qualityGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_qualitySet, const Value, SInt32> TFunc_Instances_GradientBevelFilter_qualitySet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_ratiosGet, SPtr<Instances::fl::Array> > TFunc_Instances_GradientBevelFilter_ratiosGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_ratiosSet, const Value, Instances::fl::Array*> TFunc_Instances_GradientBevelFilter_ratiosSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_strengthGet, Value::Number> TFunc_Instances_GradientBevelFilter_strengthGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_strengthSet, const Value, Value::Number> TFunc_Instances_GradientBevelFilter_strengthSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_typeGet, ASString> TFunc_Instances_GradientBevelFilter_typeGet;
typedef ThunkFunc1<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_typeSet, const Value, const ASString&> TFunc_Instances_GradientBevelFilter_typeSet;
typedef ThunkFunc0<Instances::fl_filters::GradientBevelFilter, Instances::fl_filters::GradientBevelFilter::mid_clone, SPtr<Instances::fl_filters::BitmapFilter> > TFunc_Instances_GradientBevelFilter_clone;

template <> const TFunc_Instances_GradientBevelFilter_alphasGet::TMethod TFunc_Instances_GradientBevelFilter_alphasGet::Method = &Instances::fl_filters::GradientBevelFilter::alphasGet;
template <> const TFunc_Instances_GradientBevelFilter_alphasSet::TMethod TFunc_Instances_GradientBevelFilter_alphasSet::Method = &Instances::fl_filters::GradientBevelFilter::alphasSet;
template <> const TFunc_Instances_GradientBevelFilter_angleGet::TMethod TFunc_Instances_GradientBevelFilter_angleGet::Method = &Instances::fl_filters::GradientBevelFilter::angleGet;
template <> const TFunc_Instances_GradientBevelFilter_angleSet::TMethod TFunc_Instances_GradientBevelFilter_angleSet::Method = &Instances::fl_filters::GradientBevelFilter::angleSet;
template <> const TFunc_Instances_GradientBevelFilter_blurXGet::TMethod TFunc_Instances_GradientBevelFilter_blurXGet::Method = &Instances::fl_filters::GradientBevelFilter::blurXGet;
template <> const TFunc_Instances_GradientBevelFilter_blurXSet::TMethod TFunc_Instances_GradientBevelFilter_blurXSet::Method = &Instances::fl_filters::GradientBevelFilter::blurXSet;
template <> const TFunc_Instances_GradientBevelFilter_blurYGet::TMethod TFunc_Instances_GradientBevelFilter_blurYGet::Method = &Instances::fl_filters::GradientBevelFilter::blurYGet;
template <> const TFunc_Instances_GradientBevelFilter_blurYSet::TMethod TFunc_Instances_GradientBevelFilter_blurYSet::Method = &Instances::fl_filters::GradientBevelFilter::blurYSet;
template <> const TFunc_Instances_GradientBevelFilter_colorsGet::TMethod TFunc_Instances_GradientBevelFilter_colorsGet::Method = &Instances::fl_filters::GradientBevelFilter::colorsGet;
template <> const TFunc_Instances_GradientBevelFilter_colorsSet::TMethod TFunc_Instances_GradientBevelFilter_colorsSet::Method = &Instances::fl_filters::GradientBevelFilter::colorsSet;
template <> const TFunc_Instances_GradientBevelFilter_distanceGet::TMethod TFunc_Instances_GradientBevelFilter_distanceGet::Method = &Instances::fl_filters::GradientBevelFilter::distanceGet;
template <> const TFunc_Instances_GradientBevelFilter_distanceSet::TMethod TFunc_Instances_GradientBevelFilter_distanceSet::Method = &Instances::fl_filters::GradientBevelFilter::distanceSet;
template <> const TFunc_Instances_GradientBevelFilter_knockoutGet::TMethod TFunc_Instances_GradientBevelFilter_knockoutGet::Method = &Instances::fl_filters::GradientBevelFilter::knockoutGet;
template <> const TFunc_Instances_GradientBevelFilter_knockoutSet::TMethod TFunc_Instances_GradientBevelFilter_knockoutSet::Method = &Instances::fl_filters::GradientBevelFilter::knockoutSet;
template <> const TFunc_Instances_GradientBevelFilter_qualityGet::TMethod TFunc_Instances_GradientBevelFilter_qualityGet::Method = &Instances::fl_filters::GradientBevelFilter::qualityGet;
template <> const TFunc_Instances_GradientBevelFilter_qualitySet::TMethod TFunc_Instances_GradientBevelFilter_qualitySet::Method = &Instances::fl_filters::GradientBevelFilter::qualitySet;
template <> const TFunc_Instances_GradientBevelFilter_ratiosGet::TMethod TFunc_Instances_GradientBevelFilter_ratiosGet::Method = &Instances::fl_filters::GradientBevelFilter::ratiosGet;
template <> const TFunc_Instances_GradientBevelFilter_ratiosSet::TMethod TFunc_Instances_GradientBevelFilter_ratiosSet::Method = &Instances::fl_filters::GradientBevelFilter::ratiosSet;
template <> const TFunc_Instances_GradientBevelFilter_strengthGet::TMethod TFunc_Instances_GradientBevelFilter_strengthGet::Method = &Instances::fl_filters::GradientBevelFilter::strengthGet;
template <> const TFunc_Instances_GradientBevelFilter_strengthSet::TMethod TFunc_Instances_GradientBevelFilter_strengthSet::Method = &Instances::fl_filters::GradientBevelFilter::strengthSet;
template <> const TFunc_Instances_GradientBevelFilter_typeGet::TMethod TFunc_Instances_GradientBevelFilter_typeGet::Method = &Instances::fl_filters::GradientBevelFilter::typeGet;
template <> const TFunc_Instances_GradientBevelFilter_typeSet::TMethod TFunc_Instances_GradientBevelFilter_typeSet::Method = &Instances::fl_filters::GradientBevelFilter::typeSet;
template <> const TFunc_Instances_GradientBevelFilter_clone::TMethod TFunc_Instances_GradientBevelFilter_clone::Method = &Instances::fl_filters::GradientBevelFilter::clone;

namespace Instances { namespace fl_filters
{
    GradientBevelFilter::GradientBevelFilter(InstanceTraits::Traits& t)
    : Instances::fl_filters::BitmapFilter(t)
//##protect##"instance::GradientBevelFilter::GradientBevelFilter()$data"
    , Type(GetStringManager().CreateEmptyString())
//##protect##"instance::GradientBevelFilter::GradientBevelFilter()$data"
    {
//##protect##"instance::GradientBevelFilter::GradientBevelFilter()$code"
        FilterData = *SF_NEW Render::GradientFilter(Render::Filter_GradientBevel);
//##protect##"instance::GradientBevelFilter::GradientBevelFilter()$code"
    }

    void GradientBevelFilter::alphasGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientBevelFilter::alphasGet()"
        result = Alphas;
//##protect##"instance::GradientBevelFilter::alphasGet()"
    }
    void GradientBevelFilter::alphasSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientBevelFilter::alphasSet()"
        SF_UNUSED(result);
        Alphas = value;
        recomputeGradient();
//##protect##"instance::GradientBevelFilter::alphasSet()"
    }
    void GradientBevelFilter::angleGet(Value::Number& result)
    {
//##protect##"instance::GradientBevelFilter::angleGet()"
        result = GetGradientBevelFilterData()->GetParams().Colors[0].GetAlpha() / 255.0;
//##protect##"instance::GradientBevelFilter::angleGet()"
    }
    void GradientBevelFilter::angleSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientBevelFilter::angleSet()"
        SF_UNUSED(result);
        GetGradientBevelFilterData()->GetParams().Colors[0].SetAlpha((UByte)(value * 255));
//##protect##"instance::GradientBevelFilter::angleSet()"
    }
    void GradientBevelFilter::blurXGet(Value::Number& result)
    {
//##protect##"instance::GradientBevelFilter::blurXGet()"
        result = TwipsToPixels(GetGradientBevelFilterData()->GetParams().BlurX);
//##protect##"instance::GradientBevelFilter::blurXGet()"
    }
    void GradientBevelFilter::blurXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientBevelFilter::blurXSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetGradientBevelFilterData()->GetParams().BlurX = PixelsToTwips((float)value);
//##protect##"instance::GradientBevelFilter::blurXSet()"
    }
    void GradientBevelFilter::blurYGet(Value::Number& result)
    {
//##protect##"instance::GradientBevelFilter::blurYGet()"
        result = TwipsToPixels(GetGradientBevelFilterData()->GetParams().BlurY);
//##protect##"instance::GradientBevelFilter::blurYGet()"
    }
    void GradientBevelFilter::blurYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientBevelFilter::blurYSet()"
        SF_UNUSED(result);
        value = Alg::Max(0.0, value);
        GetGradientBevelFilterData()->GetParams().BlurY = PixelsToTwips((float)value);
//##protect##"instance::GradientBevelFilter::blurYSet()"
    }
    void GradientBevelFilter::colorsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientBevelFilter::colorsGet()"
        result = Colors;
//##protect##"instance::GradientBevelFilter::colorsGet()"
    }
    void GradientBevelFilter::colorsSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientBevelFilter::colorsSet()"
        SF_UNUSED(result);
        Colors = value;
        recomputeGradient();
//##protect##"instance::GradientBevelFilter::colorsSet()"
    }
    void GradientBevelFilter::distanceGet(Value::Number& result)
    {
//##protect##"instance::GradientBevelFilter::distanceGet()"
        result = TwipsToPixels(GetGradientBevelFilterData()->GetDistance());
//##protect##"instance::GradientBevelFilter::distanceGet()"
    }
    void GradientBevelFilter::distanceSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientBevelFilter::distanceSet()"
        SF_UNUSED(result);
        GetGradientBevelFilterData()->SetAngleDistance( GetGradientBevelFilterData()->GetAngle(), PixelsToTwips((float)value));
//##protect##"instance::GradientBevelFilter::distanceSet()"
    }
    void GradientBevelFilter::knockoutGet(bool& result)
    {
//##protect##"instance::GradientBevelFilter::knockoutGet()"
        result = (GetGradientBevelFilterData()->GetParams().Mode & Render::BlurFilterParams::Mode_Knockout) != 0;
//##protect##"instance::GradientBevelFilter::knockoutGet()"
    }
    void GradientBevelFilter::knockoutSet(const Value& result, bool value)
    {
//##protect##"instance::GradientBevelFilter::knockoutSet()"
        SF_UNUSED(result);
        unsigned& mode = GetGradientBevelFilterData()->GetParams().Mode;
        mode &= ~Render::BlurFilterParams::Mode_Knockout;
        mode |= (value ? Render::BlurFilterParams::Mode_Knockout : 0);
//##protect##"instance::GradientBevelFilter::knockoutSet()"
    }
    void GradientBevelFilter::qualityGet(SInt32& result)
    {
//##protect##"instance::GradientBevelFilter::qualityGet()"
        result = (GetGradientBevelFilterData()->GetParams().Passes);
//##protect##"instance::GradientBevelFilter::qualityGet()"
    }
    void GradientBevelFilter::qualitySet(const Value& result, SInt32 value)
    {
//##protect##"instance::GradientBevelFilter::qualitySet()"
        SF_UNUSED(result);
        GetGradientBevelFilterData()->GetParams().Passes = Alg::Clamp<unsigned>(value, 0, 15);
//##protect##"instance::GradientBevelFilter::qualitySet()"
    }
    void GradientBevelFilter::ratiosGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::GradientBevelFilter::ratiosGet()"
        result = Ratios;
//##protect##"instance::GradientBevelFilter::ratiosGet()"
    }
    void GradientBevelFilter::ratiosSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::GradientBevelFilter::ratiosSet()"
        SF_UNUSED(result);
        Ratios = value;
        recomputeGradient();
//##protect##"instance::GradientBevelFilter::ratiosSet()"
    }
    void GradientBevelFilter::strengthGet(Value::Number& result)
    {
//##protect##"instance::GradientBevelFilter::strengthGet()"
        result = GetGradientBevelFilterData()->GetParams().Strength;
//##protect##"instance::GradientBevelFilter::strengthGet()"
    }
    void GradientBevelFilter::strengthSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GradientBevelFilter::strengthSet()"
        SF_UNUSED(result);
        GetGradientBevelFilterData()->GetParams().Strength = (float)value;
//##protect##"instance::GradientBevelFilter::strengthSet()"
    }
    void GradientBevelFilter::typeGet(ASString& result)
    {
//##protect##"instance::GradientBevelFilter::typeGet()"
        result = Type;
//##protect##"instance::GradientBevelFilter::typeGet()"
    }
    void GradientBevelFilter::typeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::GradientBevelFilter::typeSet()"
        SF_UNUSED(result);
        unsigned& mode = GetGradientBevelFilterData()->GetParams().Mode;
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
//##protect##"instance::GradientBevelFilter::typeSet()"
    }
    void GradientBevelFilter::clone(SPtr<Instances::fl_filters::BitmapFilter>& result)
    {
//##protect##"instance::GradientBevelFilter::clone()"
        InstanceTraits::fl_filters::GradientBevelFilter& itr = static_cast<InstanceTraits::fl_filters::GradientBevelFilter&>(GetTraits());
        Pickable<GradientBevelFilter> r = itr.MakeInstance(itr);

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
//##protect##"instance::GradientBevelFilter::clone()"
    }

    SPtr<Instances::fl::Array> GradientBevelFilter::alphasGet()
    {
        SPtr<Instances::fl::Array> result;
        alphasGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> GradientBevelFilter::colorsGet()
    {
        SPtr<Instances::fl::Array> result;
        colorsGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> GradientBevelFilter::ratiosGet()
    {
        SPtr<Instances::fl::Array> result;
        ratiosGet(result);
        return result;
    }
    SPtr<Instances::fl_filters::BitmapFilter> GradientBevelFilter::clone()
    {
        SPtr<Instances::fl_filters::BitmapFilter> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void GradientBevelFilter::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 11)
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("flash.filters::GradientBevelFilter()") SF_DEBUG_ARG(0) SF_DEBUG_ARG(11) SF_DEBUG_ARG(argc)));
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

    void GradientBevelFilter::recomputeGradient()
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
        GetGradientBevelFilterData()->GetParams().Gradient = pGradientData;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    // const UInt16 GradientBevelFilter::tito[GradientBevelFilter::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 27, 28, 30, 31, 33, 
    // };
    const TypeInfo* GradientBevelFilter::tit[34] = {
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
    const ThunkInfo GradientBevelFilter::ti[GradientBevelFilter::ThunkInfoNum] = {
        {TFunc_Instances_GradientBevelFilter_alphasGet::Func, &GradientBevelFilter::tit[0], "alphas", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_alphasSet::Func, &GradientBevelFilter::tit[1], "alphas", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_angleGet::Func, &GradientBevelFilter::tit[3], "angle", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_angleSet::Func, &GradientBevelFilter::tit[4], "angle", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_blurXGet::Func, &GradientBevelFilter::tit[6], "blurX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_blurXSet::Func, &GradientBevelFilter::tit[7], "blurX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_blurYGet::Func, &GradientBevelFilter::tit[9], "blurY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_blurYSet::Func, &GradientBevelFilter::tit[10], "blurY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_colorsGet::Func, &GradientBevelFilter::tit[12], "colors", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_colorsSet::Func, &GradientBevelFilter::tit[13], "colors", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_distanceGet::Func, &GradientBevelFilter::tit[15], "distance", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_distanceSet::Func, &GradientBevelFilter::tit[16], "distance", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_knockoutGet::Func, &GradientBevelFilter::tit[18], "knockout", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_knockoutSet::Func, &GradientBevelFilter::tit[19], "knockout", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_qualityGet::Func, &GradientBevelFilter::tit[21], "quality", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_qualitySet::Func, &GradientBevelFilter::tit[22], "quality", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_ratiosGet::Func, &GradientBevelFilter::tit[24], "ratios", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_ratiosSet::Func, &GradientBevelFilter::tit[25], "ratios", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_strengthGet::Func, &GradientBevelFilter::tit[27], "strength", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_strengthSet::Func, &GradientBevelFilter::tit[28], "strength", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_typeGet::Func, &GradientBevelFilter::tit[30], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_typeSet::Func, &GradientBevelFilter::tit[31], "type", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GradientBevelFilter_clone::Func, &GradientBevelFilter::tit[33], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GradientBevelFilter::GradientBevelFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"InstanceTraits::GradientBevelFilter::GradientBevelFilter()"
//##protect##"InstanceTraits::GradientBevelFilter::GradientBevelFilter()"

    }

    void GradientBevelFilter::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GradientBevelFilter&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filters
{

    GradientBevelFilter::GradientBevelFilter(VM& vm, const ClassInfo& ci)
    : fl_filters::BitmapFilter(vm, ci)
    {
//##protect##"ClassTraits::GradientBevelFilter::GradientBevelFilter()"
//##protect##"ClassTraits::GradientBevelFilter::GradientBevelFilter()"

    }

    Pickable<Traits> GradientBevelFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GradientBevelFilter(vm, AS3::fl_filters::GradientBevelFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::GradientBevelFilterCI));
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
    const TypeInfo GradientBevelFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::GradientBevelFilter::InstanceType),
        0,
        0,
        InstanceTraits::fl_filters::GradientBevelFilter::ThunkInfoNum,
        0,
        "GradientBevelFilter", "flash.filters", &fl_filters::BitmapFilterTI,
        TypeInfo::None
    };

    const ClassInfo GradientBevelFilterCI = {
        &GradientBevelFilterTI,
        ClassTraits::fl_filters::GradientBevelFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filters::GradientBevelFilter::ti,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

