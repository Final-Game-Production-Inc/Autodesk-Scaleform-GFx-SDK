//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowResize.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_NativeWindowResize_H
#define INC_AS3_Obj_Display_NativeWindowResize_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo NativeWindowResizeTI;
    extern const ClassInfo NativeWindowResizeCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class NativeWindowResize;
}}

namespace InstanceTraits { namespace fl_display
{
    class NativeWindowResize;
}}

namespace Classes { namespace fl_display
{
    class NativeWindowResize;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class NativeWindowResize : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeWindowResize"; }
#endif
    public:
        typedef Classes::fl_display::NativeWindowResize ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeWindowResize(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 9 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class NativeWindowResize : public Class
    {
        friend class ClassTraits::fl_display::NativeWindowResize;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::NativeWindowResizeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NativeWindowResize"; }
#endif
    public:
        typedef NativeWindowResize SelfType;
        typedef NativeWindowResize ClassType;
        
    private:
        NativeWindowResize(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* BOTTOM;
        const char* BOTTOM_LEFT;
        const char* BOTTOM_RIGHT;
        const char* LEFT;
        const char* NONE;
        const char* RIGHT;
        const char* TOP;
        const char* TOP_LEFT;
        const char* TOP_RIGHT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

