//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_Accessibility.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Accessibility_Accessibility_H
#define INC_AS3_Obj_Accessibility_Accessibility_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_accessibility
{
    extern const TypeInfo AccessibilityTI;
    extern const ClassInfo AccessibilityCI;
} // namespace fl_accessibility
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_accessibility
{
    class Accessibility;
}}

namespace InstanceTraits { namespace fl_accessibility
{
    class Accessibility;
}}

namespace Classes { namespace fl_accessibility
{
    class Accessibility;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_accessibility
{
    class Accessibility : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Accessibility"; }
#endif
    public:
        typedef Classes::fl_accessibility::Accessibility ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Accessibility(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_accessibility
{
    class Accessibility : public Class
    {
        friend class ClassTraits::fl_accessibility::Accessibility;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_accessibility::AccessibilityTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Accessibility"; }
#endif
    public:
        typedef Accessibility SelfType;
        typedef Accessibility ClassType;
        
    private:
        Accessibility(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

