//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BlurFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_BlurFilter_H
#define INC_AS3_Obj_Filters_BlurFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo BlurFilterTI;
    extern const ClassInfo BlurFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class BlurFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class BlurFilter;
}}

namespace Classes { namespace fl_filters
{
    class BlurFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class BlurFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::BlurFilter;
        
    public:
        typedef BlurFilter SelfType;
        typedef Classes::fl_filters::BlurFilter ClassType;
        typedef InstanceTraits::fl_filters::BlurFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::BlurFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::BlurFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        BlurFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        Render::BlurFilter* GetBlurFilterData() const { return (Render::BlurFilter*)GetFilterData(); }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_blurXGet, 
            mid_blurXSet, 
            mid_blurYGet, 
            mid_blurYSet, 
            mid_qualityGet, 
            mid_qualitySet, 
            mid_clone, 
        };
        void blurXGet(Value::Number& result);
        void blurXSet(const Value& result, Value::Number value);
        void blurYGet(Value::Number& result);
        void blurYSet(const Value& result, Value::Number value);
        void qualityGet(SInt32& result);
        void qualitySet(const Value& result, SInt32 value);
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
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
        SPtr<Instances::fl_filters::BitmapFilter> clone();

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class BlurFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::BlurFilter"; }
#endif
    public:
        typedef Instances::fl_filters::BlurFilter InstanceType;

    public:
        BlurFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::BlurFilter> MakeInstance(BlurFilter& t)
        {
            return Pickable<Instances::fl_filters::BlurFilter>(new(t.Alloc()) Instances::fl_filters::BlurFilter(t));
        }
        SPtr<Instances::fl_filters::BlurFilter> MakeInstanceS(BlurFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class BlurFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BlurFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::BlurFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BlurFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

