//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_BitmapFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_BitmapFilter_H
#define INC_AS3_Obj_Filters_BitmapFilter_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "Render/Render_Filters.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters

namespace ClassTraits { namespace fl_filters
{
    class BitmapFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class BitmapFilter;
}}

namespace Classes { namespace fl_filters
{
    class BitmapFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_filters
{
    class BitmapFilter : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_filters::BitmapFilter;
        
    public:
        typedef BitmapFilter SelfType;
        typedef Classes::fl_filters::BitmapFilter ClassType;
        typedef InstanceTraits::fl_filters::BitmapFilter TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filters::BitmapFilterTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_filters::BitmapFilter"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        BitmapFilter(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        Render::Filter* GetFilterData() const { return FilterData; };
        void            SetFilterData(Render::Filter* filter) { FilterData = filter; } 
    protected:
        Ptr<Render::Filter>     FilterData;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clone, 
        };
        void clone(SPtr<Instances::fl_filters::BitmapFilter>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_filters::BitmapFilter> clone()
        {
            SPtr<Instances::fl_filters::BitmapFilter> result;
            clone(result);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_filters
{
    class BitmapFilter : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::BitmapFilter"; }
#endif
    public:
        typedef Instances::fl_filters::BitmapFilter InstanceType;

    public:
        BitmapFilter(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_filters::BitmapFilter> MakeInstance(BitmapFilter& t)
        {
            return Pickable<Instances::fl_filters::BitmapFilter>(new(t.Alloc()) Instances::fl_filters::BitmapFilter(t));
        }
        SPtr<Instances::fl_filters::BitmapFilter> MakeInstanceS(BitmapFilter& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_filters
{
    class BitmapFilter : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BitmapFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::BitmapFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BitmapFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

