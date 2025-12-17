//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_ColorMatrixFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_ColorMatrixFilter_H
#define INC_AS3_Obj_Filters_ColorMatrixFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo ColorMatrixFilterTI;
    extern const ClassInfo ColorMatrixFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class ColorMatrixFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class ColorMatrixFilter;
}}

namespace Classes { namespace fl_filters
{
    class ColorMatrixFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class ColorMatrixFilter : public Instances::fl_filters::BitmapFilter
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::ColorMatrixFilter;
        
    public:
        typedef ColorMatrixFilter SelfType;
        typedef Classes::fl_filters::ColorMatrixFilter ClassType;
        typedef InstanceTraits::fl_filters::ColorMatrixFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::ColorMatrixFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::ColorMatrixFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ColorMatrixFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        bool IsValidArrayObject(const Value& v) const;
        Render::ColorMatrixFilter* GetColorMatrixFilterData() const { return (Render::ColorMatrixFilter*)GetFilterData(); };
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_matrixGet, 
            mid_matrixSet, 
            mid_clone, 
        };
        void matrixGet(SPtr<Instances::fl::Array>& result);
        void matrixSet(const Value& result, Instances::fl::Array* value);
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Array> matrixGet();
        void matrixSet(Instances::fl::Array* value)
        {
            matrixSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_filters::BitmapFilter> clone();

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class ColorMatrixFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ColorMatrixFilter"; }
#endif
    public:
        typedef Instances::fl_filters::ColorMatrixFilter InstanceType;

    public:
        ColorMatrixFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::ColorMatrixFilter> MakeInstance(ColorMatrixFilter& t)
        {
            return Pickable<Instances::fl_filters::ColorMatrixFilter>(new(t.Alloc()) Instances::fl_filters::ColorMatrixFilter(t));
        }
        SPtr<Instances::fl_filters::ColorMatrixFilter> MakeInstanceS(ColorMatrixFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[4];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class ColorMatrixFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ColorMatrixFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::ColorMatrixFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ColorMatrixFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

