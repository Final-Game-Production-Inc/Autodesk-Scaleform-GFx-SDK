//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_CapsStyle.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_CapsStyle_H
#define INC_AS3_Obj_Display_CapsStyle_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo CapsStyleTI;
    extern const ClassInfo CapsStyleCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class CapsStyle;
}}

namespace InstanceTraits { namespace fl_display
{
    class CapsStyle;
}}

namespace Classes { namespace fl_display
{
    class CapsStyle;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class CapsStyle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::CapsStyle"; }
#endif
    public:
        typedef Classes::fl_display::CapsStyle ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        CapsStyle(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class CapsStyle : public Class
    {
        friend class ClassTraits::fl_display::CapsStyle;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::CapsStyleTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::CapsStyle"; }
#endif
    public:
        typedef CapsStyle SelfType;
        typedef CapsStyle ClassType;
        
    private:
        CapsStyle(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* NONE;
        const char* ROUND;
        const char* SQUARE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

