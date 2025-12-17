//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Bitmap.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Bitmap_H
#define INC_AS3_Obj_Display_Bitmap_H

#include "AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
#include "GFx/AS3/AS3_AvmBitmap.h"
#include "AS3_Obj_Display_BitmapData.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo BitmapTI;
    extern const ClassInfo BitmapCI;
    extern const TypeInfo BitmapDataTI;
    extern const ClassInfo BitmapDataCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class Bitmap;
}}

namespace InstanceTraits { namespace fl_display
{
    class Bitmap;
}}

namespace Classes { namespace fl_display
{
    class Bitmap;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class BitmapData;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Bitmap : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Bitmap;
        
    public:
        typedef Bitmap SelfType;
        typedef Classes::fl_display::Bitmap ClassType;
        typedef InstanceTraits::fl_display::Bitmap TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::BitmapTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Bitmap"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Bitmap(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        enum PixelSnappingType
        {
            Never,
            Always,
            Auto
        };

        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual void InitInstance(bool extCall);
        virtual GFx::DisplayObject* CreateStageObject();

        PixelSnappingType String2PixelSnapping(const char* str);
    public:
        BitmapData*         GetBitmapData() const { return pBitmapData; }
        void                SetBitmapData(BitmapData* b)
        {
            Value r;
            bitmapDataSet(r, b);
        }
        void                NotifyBitmapDataChanged();
        GFx::ImageResource* GetImageResource() const;
        bool                IsSmoothed() const { return Smoothing; }
        AvmBitmap*          GetAvmBitmap() const { return static_cast<AvmBitmap*>(pDispObj.GetPtr()); }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bitmapDataGet, 
            mid_bitmapDataSet, 
            mid_pixelSnappingGet, 
            mid_pixelSnappingSet, 
            mid_smoothingGet, 
            mid_smoothingSet, 
        };
        void bitmapDataGet(SPtr<Instances::fl_display::BitmapData>& result);
        void bitmapDataSet(const Value& result, Instances::fl_display::BitmapData* value);
        void pixelSnappingGet(ASString& result);
        void pixelSnappingSet(const Value& result, const ASString& value);
        void smoothingGet(bool& result);
        void smoothingSet(const Value& result, bool value);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_display::BitmapData> bitmapDataGet();
        void bitmapDataSet(Instances::fl_display::BitmapData* value)
        {
            bitmapDataSet(Value::GetUndefined(), value);
        }
        ASString pixelSnappingGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            pixelSnappingGet(result);
            return result;
        }
        void pixelSnappingSet(const ASString& value)
        {
            pixelSnappingSet(Value::GetUndefined(), value);
        }
        bool smoothingGet()
        {
            bool result;
            smoothingGet(result);
            return result;
        }
        void smoothingSet(bool value)
        {
            smoothingSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
        SPtr<BitmapData>    pBitmapData;
        PixelSnappingType   PixelSnapping;
        bool                Smoothing;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Bitmap : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Bitmap"; }
#endif
    public:
        typedef Instances::fl_display::Bitmap InstanceType;

    public:
        Bitmap(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Bitmap> MakeInstance(Bitmap& t)
        {
            return Pickable<Instances::fl_display::Bitmap>(new(t.Alloc()) Instances::fl_display::Bitmap(t));
        }
        SPtr<Instances::fl_display::Bitmap> MakeInstanceS(Bitmap& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Bitmap : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Bitmap"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Bitmap InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Bitmap(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

