//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_GradientBevelFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_GradientBevelFilter_H
#define INC_AS3_Obj_Filters_GradientBevelFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo GradientBevelFilterTI;
    extern const ClassInfo GradientBevelFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class GradientBevelFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class GradientBevelFilter;
}}

namespace Classes { namespace fl_filters
{
    class GradientBevelFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class GradientBevelFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::GradientBevelFilter;
        
    public:
        typedef GradientBevelFilter SelfType;
        typedef Classes::fl_filters::GradientBevelFilter ClassType;
        typedef InstanceTraits::fl_filters::GradientBevelFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::GradientBevelFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::GradientBevelFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GradientBevelFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Render::GradientFilter* GetGradientBevelFilterData() const { return (Render::GradientFilter*)GetFilterData(); }
        void recomputeGradient();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alphasGet, 
            mid_alphasSet, 
            mid_angleGet, 
            mid_angleSet, 
            mid_blurXGet, 
            mid_blurXSet, 
            mid_blurYGet, 
            mid_blurYSet, 
            mid_colorsGet, 
            mid_colorsSet, 
            mid_distanceGet, 
            mid_distanceSet, 
            mid_knockoutGet, 
            mid_knockoutSet, 
            mid_qualityGet, 
            mid_qualitySet, 
            mid_ratiosGet, 
            mid_ratiosSet, 
            mid_strengthGet, 
            mid_strengthSet, 
            mid_typeGet, 
            mid_typeSet, 
            mid_clone, 
        };
        void alphasGet(SPtr<Instances::fl::Array>& result);
        void alphasSet(const Value& result, Instances::fl::Array* value);
        void angleGet(Value::Number& result);
        void angleSet(const Value& result, Value::Number value);
        void blurXGet(Value::Number& result);
        void blurXSet(const Value& result, Value::Number value);
        void blurYGet(Value::Number& result);
        void blurYSet(const Value& result, Value::Number value);
        void colorsGet(SPtr<Instances::fl::Array>& result);
        void colorsSet(const Value& result, Instances::fl::Array* value);
        void distanceGet(Value::Number& result);
        void distanceSet(const Value& result, Value::Number value);
        void knockoutGet(bool& result);
        void knockoutSet(const Value& result, bool value);
        void qualityGet(SInt32& result);
        void qualitySet(const Value& result, SInt32 value);
        void ratiosGet(SPtr<Instances::fl::Array>& result);
        void ratiosSet(const Value& result, Instances::fl::Array* value);
        void strengthGet(Value::Number& result);
        void strengthSet(const Value& result, Value::Number value);
        void typeGet(ASString& result);
        void typeSet(const Value& result, const ASString& value);
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> alphasGet();
        void alphasSet(Instances::fl::Array* value)
        {
            alphasSet(Value::GetUndefined(), value);
        }
        Value::Number angleGet()
        {
            Value::Number result;
            angleGet(result);
            return result;
        }
        void angleSet(Value::Number value)
        {
            angleSet(Value::GetUndefined(), value);
        }
        Value::Number blurXGet()
        {
            Value::Number result;
            blurXGet(result);
            return result;
        }
        void blurXSet(Value::Number value)
        {
            blurXSet(Value::GetUndefined(), value);
        }
        Value::Number blurYGet()
        {
            Value::Number result;
            blurYGet(result);
            return result;
        }
        void blurYSet(Value::Number value)
        {
            blurYSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> colorsGet();
        void colorsSet(Instances::fl::Array* value)
        {
            colorsSet(Value::GetUndefined(), value);
        }
        Value::Number distanceGet()
        {
            Value::Number result;
            distanceGet(result);
            return result;
        }
        void distanceSet(Value::Number value)
        {
            distanceSet(Value::GetUndefined(), value);
        }
        bool knockoutGet()
        {
            bool result;
            knockoutGet(result);
            return result;
        }
        void knockoutSet(bool value)
        {
            knockoutSet(Value::GetUndefined(), value);
        }
        SInt32 qualityGet()
        {
            SInt32 result;
            qualityGet(result);
            return result;
        }
        void qualitySet(SInt32 value)
        {
            qualitySet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> ratiosGet();
        void ratiosSet(Instances::fl::Array* value)
        {
            ratiosSet(Value::GetUndefined(), value);
        }
        Value::Number strengthGet()
        {
            Value::Number result;
            strengthGet(result);
            return result;
        }
        void strengthSet(Value::Number value)
        {
            strengthSet(Value::GetUndefined(), value);
        }
        ASString typeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            typeGet(result);
            return result;
        }
        void typeSet(const ASString& value)
        {
            typeSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_filters::BitmapFilter> clone();

//##protect##"instance$data"
        // NOTE: These arrays are stored directly as passed in, because when using them within the renderer
        // they are quantized into a gradient texture, and thus are not retrievable.
        SPtr<Instances::fl::Array>  Alphas;
        SPtr<Instances::fl::Array>  Ratios;
        SPtr<Instances::fl::Array>  Colors;
        ASString                    Type;
        Ptr<Render::GradientData>   pGradientData;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class GradientBevelFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GradientBevelFilter"; }
#endif
    public:
        typedef Instances::fl_filters::GradientBevelFilter InstanceType;

    public:
        GradientBevelFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::GradientBevelFilter> MakeInstance(GradientBevelFilter& t)
        {
            return Pickable<Instances::fl_filters::GradientBevelFilter>(new(t.Alloc()) Instances::fl_filters::GradientBevelFilter(t));
        }
        SPtr<Instances::fl_filters::GradientBevelFilter> MakeInstanceS(GradientBevelFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 23 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[34];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class GradientBevelFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GradientBevelFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::GradientBevelFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GradientBevelFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

