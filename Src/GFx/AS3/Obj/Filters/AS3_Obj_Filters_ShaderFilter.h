//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Aug, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filters_ShaderFilter_H
#define INC_AS3_Obj_Filters_ShaderFilter_H

#include "AS3_Obj_Filters_BitmapFilter.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filters
{
    extern const TypeInfo ShaderFilterTI;
    extern const ClassInfo ShaderFilterCI;
} // namespace fl_filters
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo ShaderTI;
    extern const ClassInfo ShaderCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_filters
{
    class ShaderFilter;
}}

namespace InstanceTraits { namespace fl_filters
{
    class ShaderFilter;
}}

namespace Classes { namespace fl_filters
{
    class ShaderFilter;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filters
{
    class ShaderFilter : public fl_filters::BitmapFilter
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ShaderFilter"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_filters::BitmapFilter InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ShaderFilter(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

