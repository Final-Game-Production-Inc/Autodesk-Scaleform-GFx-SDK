//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_BitmapData.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_BitmapData.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Display_IBitmapDrawable.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#include "../Vec/AS3_Obj_Vec_Vector_uint.h"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
#include "../Geom/AS3_Obj_Geom_Point.h"
#include "../Geom/AS3_Obj_Geom_ColorTransform.h"
#include "../Geom/AS3_Obj_Geom_Matrix.h"
#include "../Filters/AS3_Obj_Filters_BitmapFilter.h"
#include "../Display/AS3_Obj_Display_BlendMode.h"
#include "../Display/AS3_Obj_Display_DisplayObject.h"
#include "../Display/AS3_Obj_Display_Bitmap.h"
#include "../Geom/AS3_Obj_Geom_ColorTransform.h"
#include "../Vec/AS3_Obj_Vec_Vector_double.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

// Provides an interface for DrawableImages to access pixels from a ByteArray object. The values read
// may need endian swapping, thus the pointer from the byte array cannot be provided directly.
class AS3ByteArray_DIPixelProvider : public DIPixelProvider
{
public:
    AS3ByteArray_DIPixelProvider(Instances::fl_utils::ByteArray& byteArray) : PixelArray(byteArray) { }
    virtual UInt32 ReadNextPixel() 
    {
        return PixelArray.readUnsignedInt();
    }
    virtual void WriteNextPixel(UInt32 pixel)
    {
        PixelArray.writeUnsignedInt(pixel);
    }

    virtual UPInt GetLength() const
    {
        return PixelArray.GetLength();
    }
private:
    void operator=(const AS3ByteArray_DIPixelProvider&) { };
    Instances::fl_utils::ByteArray& PixelArray;
};

// Provides an interface for DrawableImages to access pixels from a Vector<uint> object.
class AS3Vectoruint_DIPixelProvider : public DIPixelProvider
{
public:
    AS3Vectoruint_DIPixelProvider(Instances::fl_vec::Vector_uint& vec) : Location(0), PixelVector(vec) { }
    virtual UInt32 ReadNextPixel()
    {
        Value v;
        PixelVector.Get(Location++, v);
        return v.AsUInt();
    }
    virtual void WriteNextPixel(UInt32 pixel)
    {
        PixelVector.Set(Location++, Value(pixel), PixelVector.GetEnclosedClassTraits()).DoNotCheck();
    }
    virtual UPInt GetLength() const
    {
        return PixelVector.lengthGet();
    }
private:
    void operator=(const AS3Vectoruint_DIPixelProvider&) { };
    UInt32 Location;
    Instances::fl_vec::Vector_uint& PixelVector;
};

//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    ASString GetMethodDefArg<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_draw, 3, const ASString&>(AS3::StringManager& sm)
    {
        return sm.CreateConstString("null");
    }

    template <>
    SF_INLINE
    bool GetMethodDefArg<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getColorBoundsRect, 2, bool>(AS3::StringManager&)
    {
        return true;
    }

    template <>
    SF_INLINE
    UInt32 GetMethodDefArg<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_hitTest, 4, UInt32>(AS3::StringManager&)
    {
        return 1;
    }

    template <>
    SF_INLINE
    UInt32 GetMethodDefArg<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_noise, 2, UInt32>(AS3::StringManager&)
    {
        return 255;
    }

    template <>
    SF_INLINE
    UInt32 GetMethodDefArg<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_noise, 3, UInt32>(AS3::StringManager&)
    {
        return 7;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_heightGet, SInt32> TFunc_Instances_BitmapData_heightGet;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_rectGet, SPtr<Instances::fl_geom::Rectangle> > TFunc_Instances_BitmapData_rectGet;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_transparentGet, bool> TFunc_Instances_BitmapData_transparentGet;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_widthGet, SInt32> TFunc_Instances_BitmapData_widthGet;
typedef ThunkFunc4<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_applyFilter, const Value, Instances::fl_display::BitmapData*, Instances::fl_geom::Rectangle*, Instances::fl_geom::Point*, Instances::fl_filters::BitmapFilter*> TFunc_Instances_BitmapData_applyFilter;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_clone, SPtr<Instances::fl_display::BitmapData> > TFunc_Instances_BitmapData_clone;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_colorTransform, const Value, Instances::fl_geom::Rectangle*, Instances::fl_geom::ColorTransform*> TFunc_Instances_BitmapData_colorTransform;
typedef ThunkFunc1<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_compare, Value, Instances::fl_display::BitmapData*> TFunc_Instances_BitmapData_compare;
typedef ThunkFunc5<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_copyChannel, const Value, Instances::fl_display::BitmapData*, Instances::fl_geom::Rectangle*, Instances::fl_geom::Point*, UInt32, UInt32> TFunc_Instances_BitmapData_copyChannel;
typedef ThunkFunc6<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_copyPixels, const Value, Instances::fl_display::BitmapData*, Instances::fl_geom::Rectangle*, Instances::fl_geom::Point*, Instances::fl_display::BitmapData*, Instances::fl_geom::Point*, bool> TFunc_Instances_BitmapData_copyPixels;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_dispose, const Value> TFunc_Instances_BitmapData_dispose;
typedef ThunkFunc6<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_draw, const Value, Instances::fl::Object*, Instances::fl_geom::Matrix*, Instances::fl_geom::ColorTransform*, const ASString&, Instances::fl_geom::Rectangle*, bool> TFunc_Instances_BitmapData_draw;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_fillRect, const Value, Instances::fl_geom::Rectangle*, UInt32> TFunc_Instances_BitmapData_fillRect;
typedef ThunkFunc3<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_floodFill, const Value, SInt32, SInt32, UInt32> TFunc_Instances_BitmapData_floodFill;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_generateFilterRect, SPtr<Instances::fl_geom::Rectangle>, Instances::fl_geom::Rectangle*, Instances::fl_filters::BitmapFilter*> TFunc_Instances_BitmapData_generateFilterRect;
typedef ThunkFunc3<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getColorBoundsRect, SPtr<Instances::fl_geom::Rectangle>, UInt32, UInt32, bool> TFunc_Instances_BitmapData_getColorBoundsRect;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getPixel, UInt32, SInt32, SInt32> TFunc_Instances_BitmapData_getPixel;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getPixel32, UInt32, SInt32, SInt32> TFunc_Instances_BitmapData_getPixel32;
typedef ThunkFunc1<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getPixels, SPtr<Instances::fl_utils::ByteArray>, Instances::fl_geom::Rectangle*> TFunc_Instances_BitmapData_getPixels;
typedef ThunkFunc1<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_getVector, SPtr<Instances::fl_vec::Vector_uint>, Instances::fl_geom::Rectangle*> TFunc_Instances_BitmapData_getVector;
typedef ThunkFunc5<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_hitTest, bool, Instances::fl_geom::Point*, UInt32, const Value&, Instances::fl_geom::Point*, UInt32> TFunc_Instances_BitmapData_hitTest;
typedef ThunkFunc1<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_histogram, SPtr<Instances::fl::Object>, Instances::fl_geom::Rectangle*> TFunc_Instances_BitmapData_histogram;
typedef ThunkFunc0<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_lock, const Value> TFunc_Instances_BitmapData_lock;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_merge, Value, unsigned, const Value*> TFunc_Instances_BitmapData_merge;
typedef ThunkFunc5<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_noise, const Value, SInt32, UInt32, UInt32, UInt32, bool> TFunc_Instances_BitmapData_noise;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_paletteMap, Value, unsigned, const Value*> TFunc_Instances_BitmapData_paletteMap;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_perlinNoise, Value, unsigned, const Value*> TFunc_Instances_BitmapData_perlinNoise;
typedef ThunkFunc6<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_pixelDissolve, SInt32, Instances::fl_display::BitmapData*, Instances::fl_geom::Rectangle*, Instances::fl_geom::Point*, SInt32, SInt32, UInt32> TFunc_Instances_BitmapData_pixelDissolve;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_scroll, const Value, SInt32, SInt32> TFunc_Instances_BitmapData_scroll;
typedef ThunkFunc3<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_setPixel, const Value, SInt32, SInt32, UInt32> TFunc_Instances_BitmapData_setPixel;
typedef ThunkFunc3<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_setPixel32, const Value, SInt32, SInt32, UInt32> TFunc_Instances_BitmapData_setPixel32;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_setPixels, const Value, Instances::fl_geom::Rectangle*, Instances::fl_utils::ByteArray*> TFunc_Instances_BitmapData_setPixels;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_setVector, const Value, Instances::fl_geom::Rectangle*, Instances::fl_vec::Vector_uint*> TFunc_Instances_BitmapData_setVector;
typedef ThunkFunc2<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_threshold, Value, unsigned, const Value*> TFunc_Instances_BitmapData_threshold;
typedef ThunkFunc1<Instances::fl_display::BitmapData, Instances::fl_display::BitmapData::mid_unlock, const Value, Instances::fl_geom::Rectangle*> TFunc_Instances_BitmapData_unlock;

template <> const TFunc_Instances_BitmapData_heightGet::TMethod TFunc_Instances_BitmapData_heightGet::Method = &Instances::fl_display::BitmapData::heightGet;
template <> const TFunc_Instances_BitmapData_rectGet::TMethod TFunc_Instances_BitmapData_rectGet::Method = &Instances::fl_display::BitmapData::rectGet;
template <> const TFunc_Instances_BitmapData_transparentGet::TMethod TFunc_Instances_BitmapData_transparentGet::Method = &Instances::fl_display::BitmapData::transparentGet;
template <> const TFunc_Instances_BitmapData_widthGet::TMethod TFunc_Instances_BitmapData_widthGet::Method = &Instances::fl_display::BitmapData::widthGet;
template <> const TFunc_Instances_BitmapData_applyFilter::TMethod TFunc_Instances_BitmapData_applyFilter::Method = &Instances::fl_display::BitmapData::applyFilter;
template <> const TFunc_Instances_BitmapData_clone::TMethod TFunc_Instances_BitmapData_clone::Method = &Instances::fl_display::BitmapData::clone;
template <> const TFunc_Instances_BitmapData_colorTransform::TMethod TFunc_Instances_BitmapData_colorTransform::Method = &Instances::fl_display::BitmapData::colorTransform;
template <> const TFunc_Instances_BitmapData_compare::TMethod TFunc_Instances_BitmapData_compare::Method = &Instances::fl_display::BitmapData::compare;
template <> const TFunc_Instances_BitmapData_copyChannel::TMethod TFunc_Instances_BitmapData_copyChannel::Method = &Instances::fl_display::BitmapData::copyChannel;
template <> const TFunc_Instances_BitmapData_copyPixels::TMethod TFunc_Instances_BitmapData_copyPixels::Method = &Instances::fl_display::BitmapData::copyPixels;
template <> const TFunc_Instances_BitmapData_dispose::TMethod TFunc_Instances_BitmapData_dispose::Method = &Instances::fl_display::BitmapData::dispose;
template <> const TFunc_Instances_BitmapData_draw::TMethod TFunc_Instances_BitmapData_draw::Method = &Instances::fl_display::BitmapData::draw;
template <> const TFunc_Instances_BitmapData_fillRect::TMethod TFunc_Instances_BitmapData_fillRect::Method = &Instances::fl_display::BitmapData::fillRect;
template <> const TFunc_Instances_BitmapData_floodFill::TMethod TFunc_Instances_BitmapData_floodFill::Method = &Instances::fl_display::BitmapData::floodFill;
template <> const TFunc_Instances_BitmapData_generateFilterRect::TMethod TFunc_Instances_BitmapData_generateFilterRect::Method = &Instances::fl_display::BitmapData::generateFilterRect;
template <> const TFunc_Instances_BitmapData_getColorBoundsRect::TMethod TFunc_Instances_BitmapData_getColorBoundsRect::Method = &Instances::fl_display::BitmapData::getColorBoundsRect;
template <> const TFunc_Instances_BitmapData_getPixel::TMethod TFunc_Instances_BitmapData_getPixel::Method = &Instances::fl_display::BitmapData::getPixel;
template <> const TFunc_Instances_BitmapData_getPixel32::TMethod TFunc_Instances_BitmapData_getPixel32::Method = &Instances::fl_display::BitmapData::getPixel32;
template <> const TFunc_Instances_BitmapData_getPixels::TMethod TFunc_Instances_BitmapData_getPixels::Method = &Instances::fl_display::BitmapData::getPixels;
template <> const TFunc_Instances_BitmapData_getVector::TMethod TFunc_Instances_BitmapData_getVector::Method = &Instances::fl_display::BitmapData::getVector;
template <> const TFunc_Instances_BitmapData_hitTest::TMethod TFunc_Instances_BitmapData_hitTest::Method = &Instances::fl_display::BitmapData::hitTest;
template <> const TFunc_Instances_BitmapData_histogram::TMethod TFunc_Instances_BitmapData_histogram::Method = &Instances::fl_display::BitmapData::histogram;
template <> const TFunc_Instances_BitmapData_lock::TMethod TFunc_Instances_BitmapData_lock::Method = &Instances::fl_display::BitmapData::lock;
template <> const TFunc_Instances_BitmapData_merge::TMethod TFunc_Instances_BitmapData_merge::Method = &Instances::fl_display::BitmapData::merge;
template <> const TFunc_Instances_BitmapData_noise::TMethod TFunc_Instances_BitmapData_noise::Method = &Instances::fl_display::BitmapData::noise;
template <> const TFunc_Instances_BitmapData_paletteMap::TMethod TFunc_Instances_BitmapData_paletteMap::Method = &Instances::fl_display::BitmapData::paletteMap;
template <> const TFunc_Instances_BitmapData_perlinNoise::TMethod TFunc_Instances_BitmapData_perlinNoise::Method = &Instances::fl_display::BitmapData::perlinNoise;
template <> const TFunc_Instances_BitmapData_pixelDissolve::TMethod TFunc_Instances_BitmapData_pixelDissolve::Method = &Instances::fl_display::BitmapData::pixelDissolve;
template <> const TFunc_Instances_BitmapData_scroll::TMethod TFunc_Instances_BitmapData_scroll::Method = &Instances::fl_display::BitmapData::scroll;
template <> const TFunc_Instances_BitmapData_setPixel::TMethod TFunc_Instances_BitmapData_setPixel::Method = &Instances::fl_display::BitmapData::setPixel;
template <> const TFunc_Instances_BitmapData_setPixel32::TMethod TFunc_Instances_BitmapData_setPixel32::Method = &Instances::fl_display::BitmapData::setPixel32;
template <> const TFunc_Instances_BitmapData_setPixels::TMethod TFunc_Instances_BitmapData_setPixels::Method = &Instances::fl_display::BitmapData::setPixels;
template <> const TFunc_Instances_BitmapData_setVector::TMethod TFunc_Instances_BitmapData_setVector::Method = &Instances::fl_display::BitmapData::setVector;
template <> const TFunc_Instances_BitmapData_threshold::TMethod TFunc_Instances_BitmapData_threshold::Method = &Instances::fl_display::BitmapData::threshold;
template <> const TFunc_Instances_BitmapData_unlock::TMethod TFunc_Instances_BitmapData_unlock::Method = &Instances::fl_display::BitmapData::unlock;

namespace Instances { namespace fl_display
{
    BitmapData::BitmapData(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::BitmapData::BitmapData()$data"
    , Width(0), Height(0), Transparent(true), ClearColor(0)
//##protect##"instance::BitmapData::BitmapData()$data"
    {
//##protect##"instance::BitmapData::BitmapData()$code"
//##protect##"instance::BitmapData::BitmapData()$code"
    }

    void BitmapData::heightGet(SInt32& result)
    {
//##protect##"instance::BitmapData::heightGet()"
        SF_UNUSED1(result);
        if (pImage)
            result = pImage->GetRect().Height();
        else
            result = (SInt32)Height;
//##protect##"instance::BitmapData::heightGet()"
    }
    void BitmapData::rectGet(SPtr<Instances::fl_geom::Rectangle>& result)
    {
//##protect##"instance::BitmapData::rectGet()"
        Value argv[4];
        argv[0].SetNumber(Alg::IRound(0));
        argv[1].SetNumber(Alg::IRound(0));
        argv[2].SetNumber(Alg::IRound(Width));
        argv[3].SetNumber(Alg::IRound(Height));
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(result, asvm.RectangleClass, 4, argv);
//##protect##"instance::BitmapData::rectGet()"
    }
    void BitmapData::transparentGet(bool& result)
    {
//##protect##"instance::BitmapData::transparentGet()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));
        result = image->IsTransparent();
//##protect##"instance::BitmapData::transparentGet()"
    }
    void BitmapData::widthGet(SInt32& result)
    {
//##protect##"instance::BitmapData::widthGet()"
        SF_UNUSED1(result);
        if (pImage)
            result = pImage->GetRect().Width();
        else
            result = (SInt32)Width;
//##protect##"instance::BitmapData::widthGet()"
    }
    void BitmapData::applyFilter(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_filters::BitmapFilter* filter)
    {
//##protect##"instance::BitmapData::applyFilter()"
        SF_UNUSED(result);
        if ( !sourceBitmapData)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        if ( !filter)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("filter")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        DrawableImage* srcImage = getDrawableImageFromBitmapData(sourceBitmapData);
        Rect<SInt32>   rect = RectangleToRect(*sourceRect);
        Render::Point<SInt32> pt = PointToPoint(*destPoint);
        Render::Filter * filterImpl = filter->GetFilterData();

        if (!filterImpl)
        {
            WARN_NOT_IMPLEMENTED("BitmapData::applyFilter (unsupported filter type)");
            return;
        }

        // Check transparency restrictions
        bool transparent;
        transparentGet(transparent);
        switch( filterImpl->GetFilterType() )
        {
            case Render::Filter_Bevel:
            case Render::Filter_Shadow:
            case Render::Filter_Glow:
            // + Chrome?
            {
                if ( !transparent )
                    // If the destination is opaque, performing these filter operations is invalid.
                    return GetVM().ThrowArgumentError(VM::Error(VM::eIllegalOperationError, GetVM() SF_DEBUG_ARG("This filter operation cannot be performed with the specified input parameters.")));               

                break;
            }
            case Render::Filter_DisplacementMap:
            {
                bool sourceTransparent;
                sourceBitmapData->transparentGet(sourceTransparent);
                if ( sourceTransparent != transparent )
                    // With displacement map filter, source and destination formats must be the same (ie. transparency must match).
                    return GetVM().ThrowArgumentError(VM::Error(VM::eIllegalOperationError, GetVM() SF_DEBUG_ARG("This filter operation cannot be performed with the specified input parameters.")));               

                break;
            }
			case Render::Filter_Blur:
			case Render::Filter_ColorMatrix:
			{
				// With blur / color matrix filters, source and destination formats can be different.
				break;
			}
            default:
            {
                WARN_NOT_IMPLEMENTED("BitmapData::applyFilter (unsupported filter type)");
                return;
            }
        }

        image->ApplyFilter(srcImage, rect, pt, filterImpl);
//##protect##"instance::BitmapData::applyFilter()"
    }
    void BitmapData::clone(SPtr<Instances::fl_display::BitmapData>& result)
    {
//##protect##"instance::BitmapData::clone()"
        InstanceTraits::fl_display::BitmapData& itr = static_cast<InstanceTraits::fl_display::BitmapData&>(GetInstanceTraits());
        result = itr.MakeInstance(itr);

        result->Width = Width;
        result->Height = Height;
        result->Transparent = Transparent;
        result->ClearColor = ClearColor;

//##protect##"instance::BitmapData::clone()"
    }
    void BitmapData::colorTransform(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_geom::ColorTransform* cTransform)
    {
//##protect##"instance::BitmapData::colorTransform()"
        SF_UNUSED(result);
        if ( !rect )
            // If the destination is opaque, performing these filter operations is invalid.
            return GetVM().ThrowArgumentError(VM::Error(VM::eIllegalOperationError, GetVM() SF_DEBUG_ARG("rect")));               

        if ( !cTransform )
            // If the destination is opaque, performing these filter operations is invalid.
            return GetVM().ThrowArgumentError(VM::Error(VM::eIllegalOperationError, GetVM() SF_DEBUG_ARG("colorTransform")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        Cxform cxform = ClassTraits::fl_geom::ColorTransform::GetCxformFromColorTransform(cTransform);
        image->ColorTransform(RectangleToRect(*rect), cxform);

//##protect##"instance::BitmapData::colorTransform()"
    }
    void BitmapData::compare(Value& result, Instances::fl_display::BitmapData* otherBitmapData)
    {
//##protect##"instance::BitmapData::compare()"
        if ( !otherBitmapData )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("otherBitmapData")));
        
        // Verify dimensions first.
        SInt32 w,h, thisw, thish;
        otherBitmapData->widthGet(w);
        otherBitmapData->heightGet(h);
        widthGet(thisw);
        heightGet(thish);

        if ( thisw != w )
        {            
            result.SetSInt32(-3);
            return;
        }
        if ( thish != h )
        {
            result.SetSInt32(-4);
            return;
        }

        // TDB: return 0 if images are equivalent. Would require GPU feedback. Currently, if images are equivalent,
        // a completely black texture is returned.
        DrawableImage* otherImage = getDrawableImageFromBitmapData(otherBitmapData);
        SPtr<Instances::fl_display::BitmapData> bitmapResult = clone();
		DrawableImage* destImage = getDrawableImageFromBitmapData(bitmapResult);
		DrawableImage* image = getDrawableImageFromBitmapData(this);

        if (!otherImage || !destImage || !image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        destImage->Compare(image, otherImage);
        result = bitmapResult;

//##protect##"instance::BitmapData::compare()"
    }
    void BitmapData::copyChannel(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, UInt32 sourceChannel, UInt32 destChannel)
    {
//##protect##"instance::BitmapData::copyChannel()"
        SF_UNUSED(result);

        if ( !sourceBitmapData )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage* srcImage = getDrawableImageFromBitmapData(sourceBitmapData);

        if (!image || !srcImage)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->CopyChannel(srcImage, RectangleToRect(*sourceRect), PointToPoint(*destPoint), 
            (DrawableImage::ChannelBits)sourceChannel, (DrawableImage::ChannelBits)destChannel);
//##protect##"instance::BitmapData::copyChannel()"
    }
    void BitmapData::copyPixels(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_display::BitmapData* alphaBitmapData, Instances::fl_geom::Point* alphaPoint, bool mergeAlpha)
    {
//##protect##"instance::BitmapData::copyPixels()"
        SF_UNUSED(result);
        
        if ( !sourceBitmapData )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage* alphasrcImage = getDrawableImageFromBitmapData(alphaBitmapData);
        DrawableImage* srcImage = getDrawableImageFromBitmapData(sourceBitmapData);
        Render::Point<SInt32> alphaPt;

        // NULL alphasrcImage is okay.
        if ( !srcImage || !image )
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        // TODO: what if a an alpha source is provided, but no point?
        if ( alphaPoint )
            alphaPt = PointToPoint(*alphaPoint);
        image->CopyPixels(srcImage, RectangleToRect(*sourceRect), PointToPoint(*destPoint), alphasrcImage, &alphaPt, mergeAlpha );

//##protect##"instance::BitmapData::copyPixels()"
    }
    void BitmapData::dispose(const Value& result)
    {
//##protect##"instance::BitmapData::dispose()"
        SF_UNUSED(result);
        pImage = 0;
        Width = 0;
        Height = 0;
//##protect##"instance::BitmapData::dispose()"
    }
    void BitmapData::draw(const Value& result, Instances::fl::Object* source, Instances::fl_geom::Matrix* matrix, Instances::fl_geom::ColorTransform* colorTransform, const ASString& blendMode, Instances::fl_geom::Rectangle* clipRect, bool smoothing)
    {
//##protect##"instance::BitmapData::draw()"
        SF_UNUSED(result);
        if ( !source )
            return GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("source")));

        // Convert given parameters.
        Matrix2F mtx;
        if ( matrix )
            mtx.Append(matrix->GetMatrixF());

        Cxform cx;
        if ( colorTransform )
            cx = ClassTraits::fl_geom::ColorTransform::GetCxformFromColorTransform(colorTransform);

        Render::BlendMode blend = Classes::fl_display::BlendMode::GetBlendMode(blendMode);

        Rect<SInt32> clip;
        if ( clipRect )
            clip = RectangleToRect(*clipRect);

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));


        if ( GetVM().IsOfType(Value(reinterpret_cast<Instances::fl::Object*>(source)), GetClass().GetClassTraits()))
        {
            Instances::fl_display::BitmapData* sourceBitmapData = reinterpret_cast<Instances::fl_display::BitmapData*>(source);
            DrawableImage* sourceImage = getDrawableImageFromBitmapData(sourceBitmapData);
            image->Draw(sourceImage, mtx, cx, blend, clipRect ? &clip : 0, smoothing);
        }
        else if ( GetVM().IsOfType(Value(reinterpret_cast<Instances::fl::Object*>(source)), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
        {
            // In the case we are drawing a DisplayObject, make sure to convert Twips -> Pixels.
            mtx.PrependScaling(TwipsToPixels(1.0f));

            Instances::fl_display::DisplayObject* displayObject = reinterpret_cast<Instances::fl_display::DisplayObject*>(source);
            if (!displayObject->HasDisplayObject())
                return;
            Render::TreeNode* node = displayObject->pDispObj->GetRenderNode();
			
			// Force any active Shape drawing to be pushed into a TreeShape tree
			ASVM& asvm = static_cast<ASVM&>(GetVM());
			asvm.GetMovieImpl()->UpdateAllRenderNodes();
            
			image->Draw(node, mtx, cx, blend, clipRect ? &clip : 0);
        }
        else
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("source")));
        }
//##protect##"instance::BitmapData::draw()"
    }
    void BitmapData::fillRect(const Value& result, Instances::fl_geom::Rectangle* rect, UInt32 color)
    {
//##protect##"instance::BitmapData::fillRect()"
        if ( !rect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("rect")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->FillRect(RectangleToRect(*rect), color);
        SF_UNUSED(result);
//##protect##"instance::BitmapData::fillRect()"
    }
    void BitmapData::floodFill(const Value& result, SInt32 x, SInt32 y, UInt32 color)
    {
//##protect##"instance::BitmapData::floodFill()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->FloodFill(Render::Point<SInt32>(x,y), color);
        SF_UNUSED(result);
//##protect##"instance::BitmapData::floodFill()"
    }
    void BitmapData::generateFilterRect(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_filters::BitmapFilter* filter)
    {
//##protect##"instance::BitmapData::generateFilterRect()"
        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !filter )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("filter")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));
        RectF bounds;
        RectF inputBounds = RectangleToRect(*sourceRect);
        image->CalcFilterRect(&bounds, PixelsToTwips(inputBounds), filter->GetFilterData());
        
        Value argv[4];
        argv[0].SetNumber(Alg::IRound(TwipsToPixels(bounds.x1)));
        argv[1].SetNumber(Alg::IRound(TwipsToPixels(bounds.y1)));
        argv[2].SetNumber(Alg::IRound(TwipsToPixels(bounds.Width())));
        argv[3].SetNumber(Alg::IRound(TwipsToPixels(bounds.Height())));

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ConstructInstance(result, asvm.RectangleClass, 4, argv);
        SF_ASSERT(result);
//##protect##"instance::BitmapData::generateFilterRect()"
    }
    void BitmapData::getColorBoundsRect(SPtr<Instances::fl_geom::Rectangle>& result, UInt32 mask, UInt32 color, bool findColor)
    {
//##protect##"instance::BitmapData::getColorBoundsRect()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        Rect<SInt32> rect = image->GetColorBoundsRect(mask, color, findColor);

        Value args[] = {Value(rect.x1), Value(rect.y1), Value(rect.Width()), Value(rect.Height())};
        if (!GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args))
            return;
//##protect##"instance::BitmapData::getColorBoundsRect()"
    }
    void BitmapData::getPixel(UInt32& result, SInt32 x, SInt32 y)
    {
//##protect##"instance::BitmapData::getPixel()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        result = image->GetPixel(x, y).ToColor32();
//##protect##"instance::BitmapData::getPixel()"
    }
    void BitmapData::getPixel32(UInt32& result, SInt32 x, SInt32 y)
    {
//##protect##"instance::BitmapData::getPixel32()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        result = image->GetPixel32(x, y).ToColor32();
//##protect##"instance::BitmapData::getPixel32()"
    }
    void BitmapData::getPixels(SPtr<Instances::fl_utils::ByteArray>& result, Instances::fl_geom::Rectangle* rect)
    {
//##protect##"instance::BitmapData::getPixels()"

        VM& vm = GetVM();

        if ( !rect )
            return vm.ThrowArgumentError(VM::Error(VM::eNullPointerError, vm SF_DEBUG_ARG("rect")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return vm.ThrowArgumentError(VM::Error(VM::eArgumentError, vm SF_DEBUG_ARG("Invalid BitmapData")));

        Render::Rect<SInt32> r = RectangleToRect(*rect);
        if ( r.Width() == 0 || r.Height() == 0 )
            return;

        // Create ByteArray instance.
        result = vm.MakeByteArray();

        result->lengthSet(r.Width() * r.Height() *sizeof(UInt32));
        AS3ByteArray_DIPixelProvider provider(*result);
        image->GetPixels(provider, r );

//##protect##"instance::BitmapData::getPixels()"
    }
    void BitmapData::getVector(SPtr<Instances::fl_vec::Vector_uint>& result, Instances::fl_geom::Rectangle* rect)
    {
//##protect##"instance::BitmapData::getVector()"
        if ( !rect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("rect")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        Render::Rect<SInt32> r = RectangleToRect(*rect);
        if ( r.Width() == 0 || r.Height() == 0 )
            return;

        // Create ByteArray instance.
        if ( !GetVM().ConstructBuiltinObject(result, "Vector.<uint>"))
        {
            result = 0;
            return;
        }
        result->lengthSet(r.Width() * r.Height() *sizeof(UInt32));
        AS3Vectoruint_DIPixelProvider provider(*result);
        image->GetPixels(provider, r );
//##protect##"instance::BitmapData::getVector()"
    }
    void BitmapData::hitTest(bool& result, Instances::fl_geom::Point* firstPoint, UInt32 firstAlphaThreshold, const Value& secondObject, Instances::fl_geom::Point* secondBitmapDataPoint, UInt32 secondAlphaThreshold)
    {
//##protect##"instance::BitmapData::hitTest()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        Render::Point<SInt32> fp = PointToPoint(*firstPoint);

        if ( GetVM().IsOfType(secondObject, "flash.geom.Rectangle", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_geom::Rectangle* r = static_cast<Instances::fl_geom::Rectangle*>(secondObject.GetObject());
            Rect<SInt32> rect = RectangleToRect(*r);
            result = image->HitTest(fp, rect, firstAlphaThreshold);
            return;
        }
        else if ( GetVM().IsOfType(secondObject, "flash.geom.Point", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_geom::Point* pt = static_cast<Instances::fl_geom::Point*>(secondObject.GetObject());
            Rect<SInt32> rect = Rect<SInt32>(PointToPoint(*pt), Size<SInt32>(1,1));
            // NOTE: flash does not respect alpha threshold == 0 when doing Point comparisons only,
            // so make it a minimum of 1.
            result = image->HitTest(fp, rect, Alg::Max<unsigned>(1, firstAlphaThreshold));
            return;
        }

        Render::ImageBase* secondImage = 0;
        Render::Point<SInt32> sp(0,0);

        if ( GetVM().IsOfType(secondObject, "flash.display.Bitmap", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_display::Bitmap* bitmap = static_cast<Instances::fl_display::Bitmap*>(secondObject.GetObject());
            ImageResource* imgres = bitmap->GetImageResource();
            if ( !imgres )
                return;
            secondImage = imgres->GetImage();
            if ( !secondImage )
                return;
        }
        else if ( GetVM().IsOfType(secondObject, "flash.display.BitmapData", GetVM().GetCurrentAppDomain()))
        {
            if ( secondBitmapDataPoint )
                sp = PointToPoint(*secondBitmapDataPoint);
            BitmapData* secondBitmapData = static_cast<Instances::fl_display::BitmapData*>(secondObject.GetObject());
            secondImage = getDrawableImageFromBitmapData(secondBitmapData);
        }
        else
        {
            // Must be one of the following above types.
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("secondObject")));
            return;
        }

        // secondpoint only applies for Bitmapdata, not Bitmap.
        result = image->HitTest(secondImage, fp, sp, firstAlphaThreshold, secondAlphaThreshold );
//##protect##"instance::BitmapData::hitTest()"
    }
    void BitmapData::histogram(SPtr<Instances::fl::Object>& result, Instances::fl_geom::Rectangle* hRect)
    {
//##protect##"instance::BitmapData::histogram()"
        Render::Rect<SInt32> hr;
        Render::Rect<SInt32>* rect = 0;
        if ( hRect )
        {
            hr = RectangleToRect(*hRect);
            rect = &hr;
        }
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        unsigned colors[4][256];
        image->Histogram(rect, colors);

        // Now make the return value.
        Value topargs[] = {Value((UInt32)4)};
        Value resVal;
        if (!GetVM().ConstructBuiltinValue(resVal, "Vector.<Vector.<Number>>", 1, topargs))
            return;
        result = (Instances::fl::Object*)resVal.GetObject();

        SPtr<Instances::fl_vec::Vector_double> vectors[4];
        for ( unsigned i = 0; i < 4; ++i )
        {
            Value args[] = {Value((UInt32)256)};
            if ( !GetVM().ConstructBuiltinObject(vectors[i], "Vector.<Number>", 1, args ) )
                return;
            for ( unsigned j = 0; j < 256; ++j )
            {
                Multiname mn(GetVM().GetPublicNamespace(), Value((UInt32)j));
                if (!vectors[i]->SetProperty(mn, Value((UInt32)colors[i][j])))
                    return;
            }
            Multiname topmn(GetVM().GetPublicNamespace(), Value((UInt32)i));
            if (!result->SetProperty(topmn, Value(vectors[i])))
                return;
        }
//##protect##"instance::BitmapData::histogram()"
    }
    void BitmapData::lock(const Value& result)
    {
//##protect##"instance::BitmapData::lock()"
        SF_UNUSED1(result);
//##protect##"instance::BitmapData::lock()"
    }
    void BitmapData::merge(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::BitmapData::merge()"
        SF_UNUSED(result);

        if ( argc != 7 )
            return;
        
        BitmapData*         srcBitmapData = static_cast<Instances::fl_display::BitmapData*>(argv[0].GetObject());
        fl_geom::Rectangle*          rect = static_cast<Instances::fl_geom::Rectangle*>(argv[1].GetObject());
        Instances::fl_geom::Point*   pt   = static_cast<Instances::fl_geom::Point*>(argv[2].GetObject());
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage*  srcImage = getDrawableImageFromBitmapData(srcBitmapData);

        if ( !srcImage || !image )
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        UInt32 multipliers[4];
        for ( unsigned i = 0; i < 4; ++i )
        {
            argv[i+3].Convert2UInt32(multipliers[i]).DoNotCheck();
        }

        image->Merge(srcImage, RectangleToRect(*rect), PointToPoint(*pt), multipliers[0], multipliers[1], multipliers[2], multipliers[3] );
//##protect##"instance::BitmapData::merge()"
    }
    void BitmapData::noise(const Value& result, SInt32 randomSeed, UInt32 low, UInt32 high, UInt32 channelOptions, bool grayScale)
    {
//##protect##"instance::BitmapData::noise()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->Noise(randomSeed, low, high, channelOptions, grayScale);
        SF_UNUSED(result);
//##protect##"instance::BitmapData::noise()"
    }
    void BitmapData::paletteMap(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::BitmapData::paletteMap()"
        Instances::fl::Array * colorArray[4];
        if ( argc < 3 )
            return;
        Instances::fl_display::BitmapData* sourceBitmapData = static_cast<Instances::fl_display::BitmapData*>(argv[0].GetObject());
        Instances::fl_geom::Rectangle*  sourceRect = static_cast<Instances::fl_geom::Rectangle*>(argv[1].GetObject());
        Instances::fl_geom::Point*      destPoint = static_cast<Instances::fl_geom::Point*>(argv[2].GetObject());

        SF_UNUSED(result);
        if ( !sourceBitmapData)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        // Convert the arrays to arrays of UInt32
        UInt32 channelStorage[4][256];
        UInt32* channels[4];

        for ( unsigned arr = 0; arr < 4; ++arr)
        {
            // Parameter wasn't supplied.
            if ( arr >= argc-3 )
            {
                channels[arr] = 0;
                continue;
            }

            colorArray[arr] = static_cast<Instances::fl::Array*>(argv[arr+3].GetObject());

            // Parameter was supplied as null.
            if ( !colorArray[arr] )
            {
                channels[arr] = 0;
                continue;
            }

            // Parameter has real value.
            channels[arr] = channelStorage[arr];
            for ( unsigned i = 0; i < 256; ++i )
                colorArray[arr]->At(i).Convert2UInt32(channels[arr][i]).DoNotCheck();
        }
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage* sourceImage = getDrawableImageFromBitmapData(sourceBitmapData);
        if (!image || !sourceImage)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->PaletteMap(sourceImage, RectangleToRect(*sourceRect), PointToPoint(*destPoint), channels);
        

//##protect##"instance::BitmapData::paletteMap()"
    }
    void BitmapData::perlinNoise(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::BitmapData::perlinNoise()"
        SF_UNUSED(result);
        if (argc < 6 )
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("BitmapData::perlinNoise")));

        Value::Number frequencyX(1), frequencyY(1);
        UInt32 octaves =1, seed =0;

        if (!argv[0].Convert2Number(frequencyX) ||
            !argv[1].Convert2Number(frequencyY) ||
            !argv[2].Convert2UInt32(octaves) ||
            !argv[3].Convert2UInt32(seed))
        {
            return;
        }

        bool stitch = argv[4].Convert2Boolean();
        bool fractal = argv[5].Convert2Boolean();
        UInt32 channels = 7;
        if ( argc >= 7 && !argv[6].Convert2UInt32(channels))
        {
            return;
        }
        bool grayScale = argc >= 8 ? argv[7].Convert2Boolean() : false;
        float* offsets = 0;
        unsigned offsetCount = 0;
        if ( argc >= 9 && argv[8].GetObject())
        {
            float offsetStorage[128];
            memset(offsetStorage, 0, sizeof(offsetStorage));

            offsets = offsetStorage;
            if (!GetVM().IsOfType(Value(argv[8]), "Array", GetVM().GetCurrentAppDomain()))
            {
                // throw type error
                return;
            }
            Instances::fl::Array* offsetArray = static_cast<Instances::fl::Array*>(argv[8].GetObject());
            offsetCount = Alg::Min<unsigned>(128, offsetArray->lengthGet());

            for ( unsigned offset = 0; offset < offsetCount; ++offset)
            {
				offsetStorage[offset*2+0] = 0.0f;
				offsetStorage[offset*2+1] = 0.0f;
                Instances::fl::Object* pointObj = static_cast<Instances::fl::Object*>(offsetArray->At(offset).GetObject());
                if ( GetVM().IsOfType(Value(pointObj), "flash.geom.Point", GetVM().GetCurrentAppDomain()))
                {
                    Instances::fl_geom::Point* offsetPoint = static_cast<Instances::fl_geom::Point*>(pointObj);
                    offsetStorage[offset*2+0] = (float)offsetPoint->x;
                    offsetStorage[offset*2+1] = (float)offsetPoint->y;
                }
            }
        }

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));
        image->PerlinNoise((float)frequencyX, (float)frequencyY, octaves, seed, stitch, 
            fractal, channels, grayScale, offsets, offsetCount);
//##protect##"instance::BitmapData::perlinNoise()"
    }
    void BitmapData::pixelDissolve(SInt32& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, SInt32 randomSeed, SInt32 numPixels, UInt32 fillColor)
    {
//##protect##"instance::BitmapData::pixelDissolve()"
        if ( !sourceBitmapData)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        if ( numPixels < 0)
            return GetVM().ThrowArgumentError(VM::Error(VM::eMustBeNonNegative, GetVM() SF_DEBUG_ARG("numPixels")));

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage* sourceImage = getDrawableImageFromBitmapData(sourceBitmapData);
        if (!image || !sourceImage)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        result = image->PixelDissolve(sourceImage, RectangleToRect(*sourceRect), PointToPoint(*destPoint), 
                                      (unsigned)randomSeed, (unsigned)numPixels, fillColor);
        
//##protect##"instance::BitmapData::pixelDissolve()"
    }
    void BitmapData::scroll(const Value& result, SInt32 x, SInt32 y)
    {
//##protect##"instance::BitmapData::scroll()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->Scroll(x, y);
        SF_UNUSED(result);
//##protect##"instance::BitmapData::scroll()"
    }
    void BitmapData::setPixel(const Value& result, SInt32 x, SInt32 y, UInt32 color)
    {
//##protect##"instance::BitmapData::setPixel()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));
        image->SetPixel(x, y, color);
        SF_UNUSED(result);        
//##protect##"instance::BitmapData::setPixel()"
    }
    void BitmapData::setPixel32(const Value& result, SInt32 x, SInt32 y, UInt32 color)
    {
//##protect##"instance::BitmapData::setPixel32()"
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));
        image->SetPixel32(x, y, color);
        SF_UNUSED(result);        
//##protect##"instance::BitmapData::setPixel32()"
    }
    void BitmapData::setPixels(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_utils::ByteArray* inputByteArray)
    {
//##protect##"instance::BitmapData::setPixels()"
        if ( !rect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("rect")));

        if ( !inputByteArray )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("inputByteArray")));

        AS3ByteArray_DIPixelProvider pixelProvider(*inputByteArray);
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        if (!image->SetPixels(RectangleToRect(*rect), pixelProvider) &&
            inputByteArray->GetLength() < pixelProvider.GetLength())
        {
            return GetVM().ThrowError(VM::Error(VM::eEOFError, GetVM() SF_DEBUG_ARG("EOF")));
        }
        SF_UNUSED(result);        
//##protect##"instance::BitmapData::setPixels()"
    }
    void BitmapData::setVector(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_vec::Vector_uint* inputVector)
    {
//##protect##"instance::BitmapData::setVector()"
        if ( !rect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("rect")));

        if ( !inputVector )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("inputVector")));

        AS3Vectoruint_DIPixelProvider pixelProvider(*inputVector);
        DrawableImage* image = getDrawableImageFromBitmapData(this);
        if (!image)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        if (!image->SetPixels(RectangleToRect(*rect), pixelProvider) &&
            inputVector->lengthGet() < pixelProvider.GetLength())
        {
            GetVM().ThrowError(VM::Error(VM::eInvalidRangeError, GetVM() SF_DEBUG_ARG("inputVector not large enough to read all the pixel data.")));
            return;
        }
        SF_UNUSED(result);        
//##protect##"instance::BitmapData::setVector()"
    }
    void BitmapData::threshold(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::BitmapData::threshold()"
        if ( argc < 5 )
            return;
        Instances::fl_display::BitmapData* sourceBitmapData = static_cast<Instances::fl_display::BitmapData*>(argv[0].GetObject());
        Instances::fl_geom::Rectangle*  sourceRect = static_cast<Instances::fl_geom::Rectangle*>(argv[1].GetObject());
        Instances::fl_geom::Point*      destPoint = static_cast<Instances::fl_geom::Point*>(argv[2].GetObject());
        SF_UNUSED(result);
        if ( !sourceBitmapData)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceBitmapData")));

        if ( !sourceRect )
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("sourceRect")));

        if ( !destPoint)
            return GetVM().ThrowArgumentError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("destPoint")));

        // Figure out the operator
        ASString operation = argv[3].AsString();
        DrawableImage::OperationType op;
        if      ( operation == "<=" )  op = DrawableImage::Operator_LE;
        else if ( operation == "<"  )  op = DrawableImage::Operator_LT;
        else if ( operation == ">"  )  op = DrawableImage::Operator_GT;
        else if ( operation == ">=" )  op = DrawableImage::Operator_GE;
        else if ( operation == "!=" )  op = DrawableImage::Operator_NE;
        else if ( operation == "==" )  op = DrawableImage::Operator_EQ;
        else
            return GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("The operation string is not a valid operation.")));

        UInt32 threshold, color = 0, mask = 0xFFFFFFFF;
        bool copySource = true;

        if (!argv[4].Convert2UInt32(threshold))
            return;

        // Optional parameters.
        if (argc >= 6 && !argv[5].Convert2UInt32(color))
            return;
        if (argc >= 7 && !argv[6].Convert2UInt32(mask))
            return;
        if (argc >= 8 )
            copySource = argv[7].Convert2Boolean();

        DrawableImage* image = getDrawableImageFromBitmapData(this);
        DrawableImage* srcImage = getDrawableImageFromBitmapData(sourceBitmapData);
        if (!image || !srcImage)
            return GetVM().ThrowArgumentError(VM::Error(VM::eArgumentError, GetVM() SF_DEBUG_ARG("Invalid BitmapData")));

        image->Threshold(srcImage, RectangleToRect(*sourceRect), PointToPoint(*destPoint), op, threshold, color, mask, copySource);
//##protect##"instance::BitmapData::threshold()"
    }
    void BitmapData::unlock(const Value& result, Instances::fl_geom::Rectangle* changeRect)
    {
//##protect##"instance::BitmapData::unlock()"
        SF_UNUSED2(result, changeRect);
//##protect##"instance::BitmapData::unlock()"
    }

    SPtr<Instances::fl_geom::Rectangle> BitmapData::rectGet()
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        rectGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> BitmapData::generateFilterRect(Instances::fl_geom::Rectangle* sourceRect, Instances::fl_filters::BitmapFilter* filter)
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        generateFilterRect(result, sourceRect, filter);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> BitmapData::getColorBoundsRect(UInt32 mask, UInt32 color, bool findColor)
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        getColorBoundsRect(result, mask, color, findColor);
        return result;
    }
    SPtr<Instances::fl_utils::ByteArray> BitmapData::getPixels(Instances::fl_geom::Rectangle* rect)
    {
        SPtr<Instances::fl_utils::ByteArray> result;
        getPixels(result, rect);
        return result;
    }
    SPtr<Instances::fl_vec::Vector_uint> BitmapData::getVector(Instances::fl_geom::Rectangle* rect)
    {
        SPtr<Instances::fl_vec::Vector_uint> result;
        getVector(result, rect);
        return result;
    }
//##protect##"instance$methods"
    BitmapData::~BitmapData()
    {
        // Specify the destruction order explicitly. The MovieDefImpl's heap may contain the Image[Resource], so if it was destroyed first,
        // then it would cause these to become dangling pointers, causing problems when their refcounts cause them to be destroyed.
        pImageResource.Clear();
        pImage.Clear();
        pDefImpl.Clear();
    }

    void BitmapData::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            BitmapData::CreateLibraryObject(NULL, NULL);
        }
    }

    bool BitmapData::CreateLibraryObject(ImageResource* imgRes, MovieDefImpl* defImpl)
    {
        // pImage might be created by AS3Constructor and we need to re-create it 
        // if this method is called. (!AB)
        //if (!pImage)
        {
            if (imgRes)
            {
                pImageResource = imgRes;
                pImage = pImageResource->GetImage();
                pDefImpl = defImpl;
            }
            else
            {
                ASVM& asvm = static_cast<ASVM&>(GetVM());
                MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);

                if (pdefImpl && GetTraits().IsUserDefined())
                {
                    // here we should get fully qualified name of class, with package, 
                    // for example: example_fla.Symbol1. 
                    ASString className = GetInstanceTraits().GetQualifiedName(Traits::qnfWithDot);

                    ResourceBindData    resBindData;
                    if (!asvm.GetMovieImpl()->FindExportedResource(pdefImpl, &resBindData, className.ToCStr()))
                    {
                        MovieRoot* proot = asvm.GetMovieRoot();
                        if (proot->GetLog())
                            proot->GetLog()->LogWarning("Attaching a bitmap with class '%s' failed",
                            className.ToCStr());
                        return false;
                    }
                    
                    if (resBindData.pResource)
                    {
                        if (resBindData.pResource->GetResourceType() == Resource::RT_Image)
                        {
                            pImageResource = static_cast<ImageResource*>(resBindData.pResource.GetPtr());
                            pImage = pImageResource->GetImage();
                            if (resBindData.pBinding)
                                pDefImpl = resBindData.pBinding->GetOwnerDefImpl();
                            else
                                pDefImpl = pdefImpl;
                        }
                    }
                }
            }
        }
        return false;
    }

    ImageResource* BitmapData::GetImageResource()
    {
        if (pImageResource)
            return pImageResource;

        // If the ImageResource is being queried, create the Image now. This means that the image likely will actually
        // be displayed somehow, and therefore the image needs to exist.
        if (!pImage)
        {
            DrawableImage* image = getDrawableImageFromBitmapData(this);
            SF_DEBUG_ASSERT(image == pImage, "Expected getDrawableImageFromBitmapData(this) == pImage. Internal error.");
            SF_UNUSED(image);
        }

        if (pImage)
        {
            pImageResource = *SF_HEAP_AUTO_NEW(pImage) ImageResource(pImage, Resource::Use_Bitmap);
            return pImageResource;
        }
        return 0;
    }

    void BitmapData::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc < 2)
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("BitmapData::AS3Constructor") SF_DEBUG_ARG(2) SF_DEBUG_ARG(2) SF_DEBUG_ARG(argc)));

        // Extract parameters from the constructor.
        SInt32 v;
        if (!argv[0].Convert2Int32(v))
            return;
        Width = v;

        if (!argv[1].Convert2Int32(v))
            return;
        Height = v;
        
        UInt32 clearColor   = 0xFFFFFFFF;   // default to white.
        if (argc >= 3)
            Transparent = argv[2].Convert2Boolean();
        if (argc >= 4 )
            argv[3].Convert2UInt32(clearColor).DoNotCheck();

        ClearColor = Color(clearColor);
    }

    Render::Rect<SInt32> BitmapData::RectangleToRect(const Instances::fl_geom::Rectangle& rect)
    {
        return Rect<SInt32>((SInt32)rect.GetX(), (SInt32)rect.GetY(), 
            (SInt32)(rect.GetX() + rect.GetWidth()), 
            (SInt32)(rect.GetY() + rect.GetHeight()));
    }

    Render::Point<SInt32> BitmapData::PointToPoint(const Instances::fl_geom::Point& point)
    {
        return Render::Point<SInt32>((SInt32)point.GetX(), (SInt32)point.GetY());
    }

    DrawableImage* BitmapData::getDrawableImageFromBitmapData( Instances::fl_display::BitmapData* sourceBitmapData )
    {
        if ( !sourceBitmapData )
            return 0;
        
        // Lazy initialization of the DrawableImage.
        if (!sourceBitmapData->pImage || sourceBitmapData->pImage->GetImageType() != ImageBase::Type_DrawableImage)
        {
            // Create the DrawableImage instance.
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();
            SF_DEBUG_ASSERT(proot, "Unexpected NULL MovieRoot.");
            MovieImpl* pimpl = proot->GetMovieImpl();
            SF_DEBUG_ASSERT(pimpl, "Unexpected NULL MovieImpl.");
            DrawableImageContext* dicontext = pimpl->GetDrawableImageContext();
            ThreadCommandQueue* queue = dicontext->GetQueue();

            SF_DEBUG_ASSERT(dicontext, "Unexpected NULL DrawableImageContext.");
            SF_DEBUG_WARNING(queue == 0, "To use BitmapData objects, you must provide a ThreadCommandQueue to MovieDef::CreateInstance.");
            if (dicontext && queue)
            {
                Render::Interfaces interfaces;
                queue->GetRenderInterfaces(&interfaces);
                TextureManager* ptm = interfaces.pTextureManager;
                SF_DEBUG_ASSERT(ptm, "To use BitmapData objects, you must first create a Render::TextureManager, and return it in TheadCommandQueue::GetRenderInterfaces.");

                if (!sourceBitmapData->pImageResource)
                {
                    // If there was no ImageResource, it means that this BitmapData was created directly in AS3.
                    // Use the DrawableImage constructor that will allocate new texture storage.

                    // Ensure that the BitmapData dimensions don't violate the size restrictions. In general, sizes above these
                    // will cause the HAL backends to explode anyhow (eg. D3D9 cannot handle textures with a dimension greater than 8192).
                    ImageSize size(sourceBitmapData->Width, sourceBitmapData->Height);
                    if (size.Area() > MaximumArea ||
                        size.Width > MaximumDimension ||
                        size.Height > MaximumDimension)
                    {
                        return 0;
                    }

                    sourceBitmapData->pImage = *SF_NEW DrawableImage(ptm->GetDrawableImageFormat(), size, 
                        sourceBitmapData->Transparent, sourceBitmapData->ClearColor, dicontext);
                }
                else
                {
                    // ImageResource was present; this BitmapData is being created from constructing a Bitmap's class.
                    // In this case, reference the original Bitmap's image as a delegate, instead of actually creating new storage.
                    // TODOBM: determine value of transparent flag.
                    sourceBitmapData->pImage = *SF_NEW DrawableImage(true, sourceBitmapData->pImageResource->GetImage(), dicontext);
                }
            }
        }

        SF_DEBUG_ASSERT(sourceBitmapData->pImage->GetImageType() == ImageBase::Type_DrawableImage, "Expected image type to be DrawableImage.");
        return (DrawableImage*)sourceBitmapData->pImage.GetPtr();
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 BitmapData::tito[BitmapData::ThunkInfoNum] = {
    //    0, 1, 2, 3, 4, 9, 10, 13, 15, 21, 28, 29, 36, 39, 43, 46, 50, 53, 56, 58, 60, 66, 68, 69, 77, 83, 91, 101, 108, 111, 115, 119, 122, 125, 134, 
    // };
    const TypeInfo* BitmapData::tit[136] = {
        &AS3::fl::int_TI, 
        &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl_filters::BitmapFilterTI, 
        &AS3::fl_display::BitmapDataTI, 
        NULL, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::ColorTransformTI, 
        NULL, &AS3::fl_display::BitmapDataTI, 
        NULL, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::PointTI, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, &AS3::fl_display::IBitmapDrawableTI, &AS3::fl_geom::MatrixTI, &AS3::fl_geom::ColorTransformTI, &AS3::fl::StringTI, &AS3::fl_geom::RectangleTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_geom::RectangleTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::uintTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl_geom::RectangleTI, &AS3::fl_filters::BitmapFilterTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl::uintTI, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        &AS3::fl_utils::ByteArrayTI, &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::PointTI, &AS3::fl::uintTI, &AS3::fl::ObjectTI, &AS3::fl_geom::PointTI, &AS3::fl::uintTI, 
        &AS3::fl::ObjectTI, &AS3::fl_geom::RectangleTI, 
        NULL, 
        NULL, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl::ArrayTI, &AS3::fl::ArrayTI, &AS3::fl::ArrayTI, &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::uintTI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, &AS3::fl::BooleanTI, &AS3::fl::uintTI, &AS3::fl::BooleanTI, &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, &AS3::fl::uintTI, 
        NULL, &AS3::fl_geom::RectangleTI, &AS3::fl_utils::ByteArrayTI, 
        NULL, &AS3::fl_geom::RectangleTI, NULL, 
        &AS3::fl::uintTI, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::RectangleTI, &AS3::fl_geom::PointTI, &AS3::fl::StringTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::uintTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_geom::RectangleTI, 
    };
    const Abc::ConstValue BitmapData::dva[15] = {
        {Abc::CONSTANT_Null, 0}, {Abc::CONSTANT_Null, 0}, {Abc::CONSTANT_False, 0}, 
        {Abc::CONSTANT_True, 0}, 
        {Abc::CONSTANT_UInt, 2}, 
        {Abc::CONSTANT_UInt, 3}, {Abc::CONSTANT_UInt, 4}, {Abc::CONSTANT_False, 0}, 
        {Abc::CONSTANT_UInt, 4}, {Abc::CONSTANT_False, 0}, {Abc::CONSTANT_Null, 0}, {}, 
        {Abc::CONSTANT_UInt, 5}, {Abc::CONSTANT_False, 0}, {}, 
    };
    const ThunkInfo BitmapData::ti[BitmapData::ThunkInfoNum] = {
        {TFunc_Instances_BitmapData_heightGet::Func, &BitmapData::tit[0], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_rectGet::Func, &BitmapData::tit[1], "rect", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_transparentGet::Func, &BitmapData::tit[2], "transparent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_widthGet::Func, &BitmapData::tit[3], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_applyFilter::Func, &BitmapData::tit[4], "applyFilter", NULL, Abc::NS_Public, CT_Method, 4, 4, 0, 0, NULL},
        {TFunc_Instances_BitmapData_clone::Func, &BitmapData::tit[9], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_colorTransform::Func, &BitmapData::tit[10], "colorTransform", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_compare::Func, &BitmapData::tit[13], "compare", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BitmapData_copyChannel::Func, &BitmapData::tit[15], "copyChannel", NULL, Abc::NS_Public, CT_Method, 5, 5, 0, 0, NULL},
        {TFunc_Instances_BitmapData_copyPixels::Func, &BitmapData::tit[21], "copyPixels", NULL, Abc::NS_Public, CT_Method, 3, 6, 0, 0, NULL},
        {TFunc_Instances_BitmapData_dispose::Func, &BitmapData::tit[28], "dispose", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_draw::Func, &BitmapData::tit[29], "draw", NULL, Abc::NS_Public, CT_Method, 1, 6, 0, 3, &BitmapData::dva[0]},
        {TFunc_Instances_BitmapData_fillRect::Func, &BitmapData::tit[36], "fillRect", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_floodFill::Func, &BitmapData::tit[39], "floodFill", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Instances_BitmapData_generateFilterRect::Func, &BitmapData::tit[43], "generateFilterRect", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_getColorBoundsRect::Func, &BitmapData::tit[46], "getColorBoundsRect", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 1, &BitmapData::dva[3]},
        {TFunc_Instances_BitmapData_getPixel::Func, &BitmapData::tit[50], "getPixel", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_getPixel32::Func, &BitmapData::tit[53], "getPixel32", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_getPixels::Func, &BitmapData::tit[56], "getPixels", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BitmapData_getVector::Func, &BitmapData::tit[58], "getVector", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_BitmapData_hitTest::Func, &BitmapData::tit[60], "hitTest", NULL, Abc::NS_Public, CT_Method, 3, 5, 0, 1, &BitmapData::dva[4]},
        {TFunc_Instances_BitmapData_histogram::Func, &BitmapData::tit[66], "histogram", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Instances_BitmapData_lock::Func, &BitmapData::tit[68], "lock", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_BitmapData_merge::Func, &BitmapData::tit[69], "merge", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_BitmapData_noise::Func, &BitmapData::tit[77], "noise", NULL, Abc::NS_Public, CT_Method, 1, 5, 0, 3, &BitmapData::dva[5]},
        {TFunc_Instances_BitmapData_paletteMap::Func, &BitmapData::tit[83], "paletteMap", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_BitmapData_perlinNoise::Func, &BitmapData::tit[91], "perlinNoise", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 4, &BitmapData::dva[8]},
        {TFunc_Instances_BitmapData_pixelDissolve::Func, &BitmapData::tit[101], "pixelDissolve", NULL, Abc::NS_Public, CT_Method, 3, 6, 0, 0, NULL},
        {TFunc_Instances_BitmapData_scroll::Func, &BitmapData::tit[108], "scroll", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_setPixel::Func, &BitmapData::tit[111], "setPixel", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Instances_BitmapData_setPixel32::Func, &BitmapData::tit[115], "setPixel32", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Instances_BitmapData_setPixels::Func, &BitmapData::tit[119], "setPixels", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_setVector::Func, &BitmapData::tit[122], "setVector", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_BitmapData_threshold::Func, &BitmapData::tit[125], "threshold", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 3, &BitmapData::dva[12]},
        {TFunc_Instances_BitmapData_unlock::Func, &BitmapData::tit[134], "unlock", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
    };

    BitmapData::BitmapData(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::BitmapData::BitmapData()"
//##protect##"InstanceTraits::BitmapData::BitmapData()"

    }

    void BitmapData::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<BitmapData&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    BitmapData::BitmapData(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::BitmapData::BitmapData()"
//##protect##"ClassTraits::BitmapData::BitmapData()"

    }

    Pickable<Traits> BitmapData::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) BitmapData(vm, AS3::fl_display::BitmapDataCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::BitmapDataCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo* BitmapDataImplements[] = {
        &fl_display::IBitmapDrawableTI,
        NULL
    };

    const TypeInfo BitmapDataTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::BitmapData::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::BitmapData::ThunkInfoNum,
        0,
        "BitmapData", "flash.display", &fl::ObjectTI,
        BitmapDataImplements
    };

    const ClassInfo BitmapDataCI = {
        &BitmapDataTI,
        ClassTraits::fl_display::BitmapData::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::BitmapData::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

