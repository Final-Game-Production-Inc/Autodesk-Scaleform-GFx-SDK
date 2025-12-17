//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_DisplacementMapFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_DisplacementMapFilter_H
#define INC_AS3_Obj_Filters_DisplacementMapFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_BitmapData.h"
#include "../Geom/AS3_Obj_Geom_Point.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo DisplacementMapFilterTI;
    extern const ClassInfo DisplacementMapFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo BitmapDataTI;
    extern const ClassInfo BitmapDataCI;
} // namespace fl_display
namespace fl_geom
{
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_filters
{
    class DisplacementMapFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class DisplacementMapFilter;
}}

namespace Classes { namespace fl_filters
{
    class DisplacementMapFilter;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Point;
    class BitmapData;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class DisplacementMapFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::DisplacementMapFilter;
        
    public:
        typedef DisplacementMapFilter SelfType;
        typedef Classes::fl_filters::DisplacementMapFilter ClassType;
        typedef InstanceTraits::fl_filters::DisplacementMapFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::DisplacementMapFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::DisplacementMapFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        DisplacementMapFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Render::DisplacementMapFilter* GetDisplacementMapFilterData() const { return (Render::DisplacementMapFilter*)GetFilterData(); }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alphaGet, 
            mid_alphaSet, 
            mid_colorGet, 
            mid_colorSet, 
            mid_componentXGet, 
            mid_componentXSet, 
            mid_componentYGet, 
            mid_componentYSet, 
            mid_mapBitmapGet, 
            mid_mapBitmapSet, 
            mid_mapPointGet, 
            mid_mapPointSet, 
            mid_modeGet, 
            mid_modeSet, 
            mid_scaleXGet, 
            mid_scaleXSet, 
            mid_scaleYGet, 
            mid_scaleYSet, 
            mid_clone, 
        };
        void alphaGet(Value::Number& result);
        void alphaSet(const Value& result, Value::Number value);
        void colorGet(UInt32& result);
        void colorSet(const Value& result, UInt32 value);
        void componentXGet(UInt32& result);
        void componentXSet(const Value& result, UInt32 value);
        void componentYGet(UInt32& result);
        void componentYSet(const Value& result, UInt32 value);
        void mapBitmapGet(SPtr<Instances::fl_display::BitmapData>& result);
        void mapBitmapSet(const Value& result, Instances::fl_display::BitmapData* value);
        void mapPointGet(SPtr<Instances::fl_geom::Point>& result);
        void mapPointSet(const Value& result, Instances::fl_geom::Point* value);
        void modeGet(ASString& result);
        void modeSet(const Value& result, const ASString& value);
        void scaleXGet(Value::Number& result);
        void scaleXSet(const Value& result, Value::Number value);
        void scaleYGet(Value::Number& result);
        void scaleYSet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number alphaGet()
        {
            Value::Number result;
            alphaGet(result);
            return result;
        }
        void alphaSet(Value::Number value)
        {
            alphaSet(Value::GetUndefined(), value);
        }
        UInt32 colorGet()
        {
            UInt32 result;
            colorGet(result);
            return result;
        }
        void colorSet(UInt32 value)
        {
            colorSet(Value::GetUndefined(), value);
        }
        UInt32 componentXGet()
        {
            UInt32 result;
            componentXGet(result);
            return result;
        }
        void componentXSet(UInt32 value)
        {
            componentXSet(Value::GetUndefined(), value);
        }
        UInt32 componentYGet()
        {
            UInt32 result;
            componentYGet(result);
            return result;
        }
        void componentYSet(UInt32 value)
        {
            componentYSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::BitmapData> mapBitmapGet();
        void mapBitmapSet(Instances::fl_display::BitmapData* value)
        {
            mapBitmapSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Point> mapPointGet();
        void mapPointSet(Instances::fl_geom::Point* value)
        {
            mapPointSet(Value::GetUndefined(), value);
        }
        ASString modeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            modeGet(result);
            return result;
        }
        void modeSet(const ASString& value)
        {
            modeSet(Value::GetUndefined(), value);
        }
        Value::Number scaleXGet()
        {
            Value::Number result;
            scaleXGet(result);
            return result;
        }
        void scaleXSet(Value::Number value)
        {
            scaleXSet(Value::GetUndefined(), value);
        }
        Value::Number scaleYGet()
        {
            Value::Number result;
            scaleYGet(result);
            return result;
        }
        void scaleYSet(Value::Number value)
        {
            scaleYSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_filters::BitmapFilter> clone();

//##protect##"instance$data"
        SPtr<Instances::fl_display::BitmapData> MapBitmap;  // The BitmapData source for the displacement image.
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class DisplacementMapFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::DisplacementMapFilter"; }
#endif
    public:
        typedef Instances::fl_filters::DisplacementMapFilter InstanceType;

    public:
        DisplacementMapFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::DisplacementMapFilter> MakeInstance(DisplacementMapFilter& t)
        {
            return Pickable<Instances::fl_filters::DisplacementMapFilter>(new(t.Alloc()) Instances::fl_filters::DisplacementMapFilter(t));
        }
        SPtr<Instances::fl_filters::DisplacementMapFilter> MakeInstanceS(DisplacementMapFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 19 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[28];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class DisplacementMapFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DisplacementMapFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::DisplacementMapFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DisplacementMapFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

