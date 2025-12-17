//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_GlowFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_GlowFilter_H
#define INC_AS3_Obj_Filters_GlowFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo GlowFilterTI;
    extern const ClassInfo GlowFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class GlowFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class GlowFilter;
}}

namespace Classes { namespace fl_filters
{
    class GlowFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class GlowFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::GlowFilter;
        
    public:
        typedef GlowFilter SelfType;
        typedef Classes::fl_filters::GlowFilter ClassType;
        typedef InstanceTraits::fl_filters::GlowFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::GlowFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::GlowFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GlowFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Render::GlowFilter* GetGlowFilterData() const { return (Render::GlowFilter*)GetFilterData(); }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alphaGet, 
            mid_alphaSet, 
            mid_blurXGet, 
            mid_blurXSet, 
            mid_blurYGet, 
            mid_blurYSet, 
            mid_colorGet, 
            mid_colorSet, 
            mid_innerGet, 
            mid_innerSet, 
            mid_knockoutGet, 
            mid_knockoutSet, 
            mid_qualityGet, 
            mid_qualitySet, 
            mid_strengthGet, 
            mid_strengthSet, 
            mid_clone, 
        };
        void alphaGet(Value::Number& result);
        void alphaSet(const Value& result, Value::Number value);
        void blurXGet(Value::Number& result);
        void blurXSet(const Value& result, Value::Number value);
        void blurYGet(Value::Number& result);
        void blurYSet(const Value& result, Value::Number value);
        void colorGet(UInt32& result);
        void colorSet(const Value& result, UInt32 value);
        void innerGet(bool& result);
        void innerSet(const Value& result, bool value);
        void knockoutGet(bool& result);
        void knockoutSet(const Value& result, bool value);
        void qualityGet(SInt32& result);
        void qualitySet(const Value& result, SInt32 value);
        void strengthGet(Value::Number& result);
        void strengthSet(const Value& result, Value::Number value);
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
        bool innerGet()
        {
            bool result;
            innerGet(result);
            return result;
        }
        void innerSet(bool value)
        {
            innerSet(Value::GetUndefined(), value);
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
        SPtr<Instances::fl_filters::BitmapFilter> clone();

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class GlowFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GlowFilter"; }
#endif
    public:
        typedef Instances::fl_filters::GlowFilter InstanceType;

    public:
        GlowFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::GlowFilter> MakeInstance(GlowFilter& t)
        {
            return Pickable<Instances::fl_filters::GlowFilter>(new(t.Alloc()) Instances::fl_filters::GlowFilter(t));
        }
        SPtr<Instances::fl_filters::GlowFilter> MakeInstanceS(GlowFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 17 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[25];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class GlowFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GlowFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::GlowFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GlowFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

