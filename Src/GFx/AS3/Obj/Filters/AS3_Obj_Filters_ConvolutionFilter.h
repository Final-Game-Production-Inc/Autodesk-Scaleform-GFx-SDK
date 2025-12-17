//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_ConvolutionFilter.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_ConvolutionFilter_H
#define INC_AS3_Obj_Filters_ConvolutionFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo ConvolutionFilterTI;
    extern const ClassInfo ConvolutionFilterCI;
    extern const TypeInfo BitmapFilterTI;
    extern const ClassInfo BitmapFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_filters
{
    class ConvolutionFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class ConvolutionFilter;
}}

namespace Classes { namespace fl_filters
{
    class ConvolutionFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filters
{
    class ConvolutionFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ConvolutionFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::BitmapFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ConvolutionFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

