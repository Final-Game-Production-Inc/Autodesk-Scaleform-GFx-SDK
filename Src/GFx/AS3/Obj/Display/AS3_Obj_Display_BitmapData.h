//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_BitmapData.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_BitmapData_H
#define INC_AS3_Obj_Display_BitmapData_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//#include "Render/Render_ImageInfo.h"
#include "GFx/GFx_ImageResource.h"
#include "Render/Render_DrawableImage.h"
#include "../Vec/AS3_Obj_Vec_Vector_object.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo BitmapDataTI;
    extern const ClassInfo BitmapDataCI;
    extern const TypeInfo IBitmapDrawableTI;
    extern const ClassInfo IBitmapDrawableCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
    extern const TypeInfo ColorTransformTI;
    extern const ClassInfo ColorTransformCI;
    extern const TypeInfo MatrixTI;
    extern const ClassInfo MatrixCI;
} // namespace fl_geom
namespace fl_filters
{
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils
namespace fl_vec
{
    extern const TypeInfo Vector_uintTI;
    extern const ClassInfo Vector_uintCI;
} // namespace fl_vec

namespace ClassTraits { namespace fl_display
{
    class BitmapData;
}}

namespace InstanceTraits { namespace fl_display
{
    class BitmapData;
}}

namespace Classes { namespace fl_display
{
    class BitmapData;
}}

//##protect##"forward_declaration"
namespace Instances { 
    
    namespace fl_geom
    {
        class Rectangle;
        class Matrix;
        class Point;
        class ColorTransform;
    }
    namespace fl_utils
    {
        class Vector_uint;
        class ByteArray;
    }
    namespace fl_filters
    {
        class BitmapFilter;
    }
    namespace fl_display
    {
        class IBitmapDrawable;
    }
    namespace fl_vec
    {
        class Vector_uint;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class BitmapData : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::BitmapData;
        
    public:
        typedef BitmapData SelfType;
        typedef Classes::fl_display::BitmapData ClassType;
        typedef InstanceTraits::fl_display::BitmapData TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::BitmapDataTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::BitmapData"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        BitmapData(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual ~BitmapData();
        virtual void InitInstance(bool extCall);
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Render::Rect<SInt32> RectangleToRect(const Instances::fl_geom::Rectangle& rectangle);
        Render::Point<SInt32> PointToPoint(const Instances::fl_geom::Point& point);
    public:
        Render::DrawableImage* getDrawableImageFromBitmapData( Instances::fl_display::BitmapData* sourceBitmapData );
        ImageResource*  GetImageResource();
        bool            CreateLibraryObject(ImageResource* imgRes, MovieDefImpl* defImpl);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_heightGet, 
            mid_rectGet, 
            mid_transparentGet, 
            mid_widthGet, 
            mid_applyFilter, 
            mid_clone, 
            mid_colorTransform, 
            mid_compare, 
            mid_copyChannel, 
            mid_copyPixels, 
            mid_dispose, 
            mid_draw, 
            mid_fillRect, 
            mid_floodFill, 
            mid_generateFilterRect, 
            mid_getColorBoundsRect, 
            mid_getPixel, 
            mid_getPixel32, 
            mid_getPixels, 
            mid_getVector, 
            mid_hitTest, 
            mid_histogram, 
            mid_lock, 
            mid_merge, 
            mid_noise, 
            mid_paletteMap, 
            mid_perlinNoise, 
            mid_pixelDissolve, 
            mid_scroll, 
            mid_setPixel, 
            mid_setPixel32, 
            mid_setPixels, 
            mid_setVector, 
            mid_threshold, 
            mid_unlock, 
        };
        void heightGet(SInt32& result);
        void rectGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void transparentGet(bool& result);
        void widthGet(SInt32& result);
        void applyFilter(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_filters::BitmapFilter* filter);
        void clone(SPtr<Instances::fl_display::BitmapData>& result);
        void colorTransform(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_geom::ColorTransform* cTransform);
        void compare(Value& result, Instances::fl_display::BitmapData* otherBitmapData);
        void copyChannel(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, UInt32 sourceChannel, UInt32 destChannel);
        void copyPixels(const Value& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_display::BitmapData* alphaBitmapData = NULL, Instances::fl_geom::Point* alphaPoint = NULL, bool mergeAlpha = false);
        void dispose(const Value& result);
        void draw(const Value& result, Instances::fl::Object* source, Instances::fl_geom::Matrix* matrix, Instances::fl_geom::ColorTransform* colorTransform, const ASString& blendMode, Instances::fl_geom::Rectangle* clipRect, bool smoothing);
        void fillRect(const Value& result, Instances::fl_geom::Rectangle* rect, UInt32 color);
        void floodFill(const Value& result, SInt32 x, SInt32 y, UInt32 color);
        void generateFilterRect(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_filters::BitmapFilter* filter);
        void getColorBoundsRect(SPtr<Instances::fl_geom::Rectangle>& result, UInt32 mask, UInt32 color, bool findColor = true);
        void getPixel(UInt32& result, SInt32 x, SInt32 y);
        void getPixel32(UInt32& result, SInt32 x, SInt32 y);
        void getPixels(SPtr<Instances::fl_utils::ByteArray>& result, Instances::fl_geom::Rectangle* rect);
        void getVector(SPtr<Instances::fl_vec::Vector_uint>& result, Instances::fl_geom::Rectangle* rect);
        void hitTest(bool& result, Instances::fl_geom::Point* firstPoint, UInt32 firstAlphaThreshold, const Value& secondObject, Instances::fl_geom::Point* secondBitmapDataPoint = NULL, UInt32 secondAlphaThreshold = 1);
        void histogram(SPtr<Instances::fl::Object>& result, Instances::fl_geom::Rectangle* hRect = NULL);
        void lock(const Value& result);
        void merge(Value& result, unsigned argc, const Value* const argv);
        void noise(const Value& result, SInt32 randomSeed, UInt32 low = 0, UInt32 high = 255, UInt32 channelOptions = 7, bool grayScale = false);
        void paletteMap(Value& result, unsigned argc, const Value* const argv);
        void perlinNoise(Value& result, unsigned argc, const Value* const argv);
        void pixelDissolve(SInt32& result, Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, SInt32 randomSeed = 0, SInt32 numPixels = 0, UInt32 fillColor = 0);
        void scroll(const Value& result, SInt32 x, SInt32 y);
        void setPixel(const Value& result, SInt32 x, SInt32 y, UInt32 color);
        void setPixel32(const Value& result, SInt32 x, SInt32 y, UInt32 color);
        void setPixels(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_utils::ByteArray* inputByteArray);
        void setVector(const Value& result, Instances::fl_geom::Rectangle* rect, Instances::fl_vec::Vector_uint* inputVector);
        void threshold(Value& result, unsigned argc, const Value* const argv);
        void unlock(const Value& result, Instances::fl_geom::Rectangle* changeRect = NULL);

        // C++ friendly wrappers for AS3 methods.
        SInt32 heightGet()
        {
            SInt32 result;
            heightGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Rectangle> rectGet();
        bool transparentGet()
        {
            bool result;
            transparentGet(result);
            return result;
        }
        SInt32 widthGet()
        {
            SInt32 result;
            widthGet(result);
            return result;
        }
        void applyFilter(Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_filters::BitmapFilter* filter)
        {
            applyFilter(Value::GetUndefined(), sourceBitmapData, sourceRect, destPoint, filter);
        }
        SPtr<Instances::fl_display::BitmapData> clone()
        {
            SPtr<Instances::fl_display::BitmapData> result;
            clone(result);
            return result;
        }
        void colorTransform(Instances::fl_geom::Rectangle* rect, Instances::fl_geom::ColorTransform* cTransform)
        {
            colorTransform(Value::GetUndefined(), rect, cTransform);
        }
        Value compare(Instances::fl_display::BitmapData* otherBitmapData)
        {
            Value result;
            compare(result, otherBitmapData);
            return result;
        }
        void copyChannel(Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, UInt32 sourceChannel, UInt32 destChannel)
        {
            copyChannel(Value::GetUndefined(), sourceBitmapData, sourceRect, destPoint, sourceChannel, destChannel);
        }
        void copyPixels(Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, Instances::fl_display::BitmapData* alphaBitmapData = NULL, Instances::fl_geom::Point* alphaPoint = NULL, bool mergeAlpha = false)
        {
            copyPixels(Value::GetUndefined(), sourceBitmapData, sourceRect, destPoint, alphaBitmapData, alphaPoint, mergeAlpha);
        }
        void dispose()
        {
            dispose(Value::GetUndefined());
        }
        void draw(Instances::fl::Object* source, Instances::fl_geom::Matrix* matrix, Instances::fl_geom::ColorTransform* colorTransform, const ASString& blendMode, Instances::fl_geom::Rectangle* clipRect, bool smoothing)
        {
            draw(Value::GetUndefined(), source, matrix, colorTransform, blendMode, clipRect, smoothing);
        }
        void fillRect(Instances::fl_geom::Rectangle* rect, UInt32 color)
        {
            fillRect(Value::GetUndefined(), rect, color);
        }
        void floodFill(SInt32 x, SInt32 y, UInt32 color)
        {
            floodFill(Value::GetUndefined(), x, y, color);
        }
        SPtr<Instances::fl_geom::Rectangle> generateFilterRect(Instances::fl_geom::Rectangle* sourceRect, Instances::fl_filters::BitmapFilter* filter);
        SPtr<Instances::fl_geom::Rectangle> getColorBoundsRect(UInt32 mask, UInt32 color, bool findColor = true);
        UInt32 getPixel(SInt32 x, SInt32 y)
        {
            UInt32 result;
            getPixel(result, x, y);
            return result;
        }
        UInt32 getPixel32(SInt32 x, SInt32 y)
        {
            UInt32 result;
            getPixel32(result, x, y);
            return result;
        }
        SPtr<Instances::fl_utils::ByteArray> getPixels(Instances::fl_geom::Rectangle* rect);
        SPtr<Instances::fl_vec::Vector_uint> getVector(Instances::fl_geom::Rectangle* rect);
        bool hitTest(Instances::fl_geom::Point* firstPoint, UInt32 firstAlphaThreshold, const Value& secondObject, Instances::fl_geom::Point* secondBitmapDataPoint = NULL, UInt32 secondAlphaThreshold = 1)
        {
            bool result;
            hitTest(result, firstPoint, firstAlphaThreshold, secondObject, secondBitmapDataPoint, secondAlphaThreshold);
            return result;
        }
        SPtr<Instances::fl::Object> histogram(Instances::fl_geom::Rectangle* hRect = NULL)
        {
            SPtr<Instances::fl::Object> result;
            histogram(result, hRect);
            return result;
        }
        void lock()
        {
            lock(Value::GetUndefined());
        }
        void noise(SInt32 randomSeed, UInt32 low = 0, UInt32 high = 255, UInt32 channelOptions = 7, bool grayScale = false)
        {
            noise(Value::GetUndefined(), randomSeed, low, high, channelOptions, grayScale);
        }
        SInt32 pixelDissolve(Instances::fl_display::BitmapData* sourceBitmapData, Instances::fl_geom::Rectangle* sourceRect, Instances::fl_geom::Point* destPoint, SInt32 randomSeed = 0, SInt32 numPixels = 0, UInt32 fillColor = 0)
        {
            SInt32 result;
            pixelDissolve(result, sourceBitmapData, sourceRect, destPoint, randomSeed, numPixels, fillColor);
            return result;
        }
        void scroll(SInt32 x, SInt32 y)
        {
            scroll(Value::GetUndefined(), x, y);
        }
        void setPixel(SInt32 x, SInt32 y, UInt32 color)
        {
            setPixel(Value::GetUndefined(), x, y, color);
        }
        void setPixel32(SInt32 x, SInt32 y, UInt32 color)
        {
            setPixel32(Value::GetUndefined(), x, y, color);
        }
        void setPixels(Instances::fl_geom::Rectangle* rect, Instances::fl_utils::ByteArray* inputByteArray)
        {
            setPixels(Value::GetUndefined(), rect, inputByteArray);
        }
        void setVector(Instances::fl_geom::Rectangle* rect, Instances::fl_vec::Vector_uint* inputVector)
        {
            setVector(Value::GetUndefined(), rect, inputVector);
        }
        void unlock(Instances::fl_geom::Rectangle* changeRect = NULL)
        {
            unlock(Value::GetUndefined(), changeRect);
        }

//##protect##"instance$data"
        Ptr<ImageResource>      pImageResource;
        Ptr<Render::ImageBase>  pImage;
        Ptr<MovieDefImpl>       pDefImpl;
        unsigned                Width, Height;
        bool                    Transparent;
        Color                   ClearColor;
        static const int        MaximumArea = 16777215;
        static const int        MaximumDimension = 8191;

//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class BitmapData : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::BitmapData"; }
#endif
    public:
        typedef Instances::fl_display::BitmapData InstanceType;

    public:
        BitmapData(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::BitmapData> MakeInstance(BitmapData& t)
        {
            return Pickable<Instances::fl_display::BitmapData>(new(t.Alloc()) Instances::fl_display::BitmapData(t));
        }
        SPtr<Instances::fl_display::BitmapData> MakeInstanceS(BitmapData& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 35 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[136];
        static const Abc::ConstValue dva[15];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class BitmapData : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BitmapData"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::BitmapData InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BitmapData(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

