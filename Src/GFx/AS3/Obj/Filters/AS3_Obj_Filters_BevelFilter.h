//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BevelFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_BevelFilter_H
#define INC_AS3_Obj_Filters_BevelFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo BevelFilterTI;
    extern const ClassInfo BevelFilterCI;
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
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class BevelFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class BevelFilter;
}}

namespace Classes { namespace fl_filters
{
    class BevelFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class BevelFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::BevelFilter;
        
    public:
        typedef BevelFilter SelfType;
        typedef Classes::fl_filters::BevelFilter ClassType;
        typedef InstanceTraits::fl_filters::BevelFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::BevelFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::BevelFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        BevelFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual Render::BevelFilter* GetBevelFilterData() const { return (Render::BevelFilter*)GetFilterData(); };
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_angleGet, 
            mid_angleSet, 
            mid_blurXGet, 
            mid_blurXSet, 
            mid_blurYGet, 
            mid_blurYSet, 
            mid_distanceGet, 
            mid_distanceSet, 
            mid_highlightAlphaGet, 
            mid_highlightAlphaSet, 
            mid_highlightColorGet, 
            mid_highlightColorSet, 
            mid_knockoutGet, 
            mid_knockoutSet, 
            mid_qualityGet, 
            mid_qualitySet, 
            mid_shadowAlphaGet, 
            mid_shadowAlphaSet, 
            mid_shadowColorGet, 
            mid_shadowColorSet, 
            mid_strengthGet, 
            mid_strengthSet, 
            mid_typeGet, 
            mid_typeSet, 
            mid_clone, 
        };
        void angleGet(Value::Number& result);
        void angleSet(const Value& result, Value::Number value);
        void blurXGet(Value::Number& result);
        void blurXSet(const Value& result, Value::Number value);
        void blurYGet(Value::Number& result);
        void blurYSet(const Value& result, Value::Number value);
        void distanceGet(Value::Number& result);
        void distanceSet(const Value& result, Value::Number value);
        void highlightAlphaGet(Value::Number& result);
        void highlightAlphaSet(const Value& result, Value::Number value);
        void highlightColorGet(UInt32& result);
        void highlightColorSet(const Value& result, UInt32 value);
        void knockoutGet(bool& result);
        void knockoutSet(const Value& result, bool value);
        void qualityGet(SInt32& result);
        void qualitySet(const Value& result, SInt32 value);
        void shadowAlphaGet(Value::Number& result);
        void shadowAlphaSet(const Value& result, Value::Number value);
        void shadowColorGet(UInt32& result);
        void shadowColorSet(const Value& result, UInt32 value);
        void strengthGet(Value::Number& result);
        void strengthSet(const Value& result, Value::Number value);
        void typeGet(ASString& result);
        void typeSet(const Value& result, const ASString& value);
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
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
        Value::Number highlightAlphaGet()
        {
            Value::Number result;
            highlightAlphaGet(result);
            return result;
        }
        void highlightAlphaSet(Value::Number value)
        {
            highlightAlphaSet(Value::GetUndefined(), value);
        }
        UInt32 highlightColorGet()
        {
            UInt32 result;
            highlightColorGet(result);
            return result;
        }
        void highlightColorSet(UInt32 value)
        {
            highlightColorSet(Value::GetUndefined(), value);
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
        Value::Number shadowAlphaGet()
        {
            Value::Number result;
            shadowAlphaGet(result);
            return result;
        }
        void shadowAlphaSet(Value::Number value)
        {
            shadowAlphaSet(Value::GetUndefined(), value);
        }
        UInt32 shadowColorGet()
        {
            UInt32 result;
            shadowColorGet(result);
            return result;
        }
        void shadowColorSet(UInt32 value)
        {
            shadowColorSet(Value::GetUndefined(), value);
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
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class BevelFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::BevelFilter"; }
#endif
    public:
        typedef Instances::fl_filters::BevelFilter InstanceType;

    public:
        BevelFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::BevelFilter> MakeInstance(BevelFilter& t)
        {
            return Pickable<Instances::fl_filters::BevelFilter>(new(t.Alloc()) Instances::fl_filters::BevelFilter(t));
        }
        SPtr<Instances::fl_filters::BevelFilter> MakeInstanceS(BevelFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 25 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[37];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class BevelFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BevelFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::BevelFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BevelFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

