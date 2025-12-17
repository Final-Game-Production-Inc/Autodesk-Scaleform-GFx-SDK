//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowDisplayState.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_NativeWindowDisplayState_H
#define INC_AS3_Obj_Display_NativeWindowDisplayState_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo NativeWindowDisplayStateTI;
    extern const ClassInfo NativeWindowDisplayStateCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class NativeWindowDisplayState;
}}

namespace InstanceTraits { namespace fl_display
{
    class NativeWindowDisplayState;
}}

namespace Classes { namespace fl_display
{
    class NativeWindowDisplayState;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class NativeWindowDisplayState : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeWindowDisplayState"; }
#endif
    public:
        typedef Classes::fl_display::NativeWindowDisplayState ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeWindowDisplayState(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class NativeWindowDisplayState : public Class
    {
        friend class ClassTraits::fl_display::NativeWindowDisplayState;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::NativeWindowDisplayStateTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::NativeWindowDisplayState"; }
#endif
    public:
        typedef NativeWindowDisplayState SelfType;
        typedef NativeWindowDisplayState ClassType;
        
    private:
        NativeWindowDisplayState(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* MAXIMIZED;
        const char* MINIMIZED;
        const char* NORMAL;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

