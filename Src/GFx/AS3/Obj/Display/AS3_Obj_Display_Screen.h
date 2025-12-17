//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Screen.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Screen_H
#define INC_AS3_Obj_Display_Screen_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo ScreenTI;
    extern const ClassInfo ScreenCI;
} // namespace fl_display
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class Screen;
}}

namespace InstanceTraits { namespace fl_display
{
    class Screen;
}}

namespace Classes { namespace fl_display
{
    class Screen;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Rectangle;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class Screen : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Screen"; }
#endif
    public:
        typedef Classes::fl_display::Screen ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Screen(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class Screen : public Class
    {
        friend class ClassTraits::fl_display::Screen;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::ScreenTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Screen"; }
#endif
    public:
        typedef Screen SelfType;
        typedef Screen ClassType;
        
    private:
        Screen(ClassTraits::Traits& t);
       
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

