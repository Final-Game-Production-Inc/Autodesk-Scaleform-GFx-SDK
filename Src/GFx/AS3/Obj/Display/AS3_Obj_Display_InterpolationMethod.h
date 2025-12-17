//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_InterpolationMethod.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_InterpolationMethod_H
#define INC_AS3_Obj_Display_InterpolationMethod_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo InterpolationMethodTI;
    extern const ClassInfo InterpolationMethodCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class InterpolationMethod;
}}

namespace InstanceTraits { namespace fl_display
{
    class InterpolationMethod;
}}

namespace Classes { namespace fl_display
{
    class InterpolationMethod;
}}

//##protect##"forward_declaration"
#ifdef RGB
#undef RGB
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class InterpolationMethod : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::InterpolationMethod"; }
#endif
    public:
        typedef Classes::fl_display::InterpolationMethod ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        InterpolationMethod(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class InterpolationMethod : public Class
    {
        friend class ClassTraits::fl_display::InterpolationMethod;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::InterpolationMethodTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::InterpolationMethod"; }
#endif
    public:
        typedef InterpolationMethod SelfType;
        typedef InterpolationMethod ClassType;
        
    private:
        InterpolationMethod(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* LINEAR_RGB;
        const char* RGB;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

