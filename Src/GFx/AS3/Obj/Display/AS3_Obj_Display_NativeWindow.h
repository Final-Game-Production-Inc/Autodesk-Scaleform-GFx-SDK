//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindow.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_NativeWindow_H
#define INC_AS3_Obj_Display_NativeWindow_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo NativeWindowTI;
    extern const ClassInfo NativeWindowCI;
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
    extern const TypeInfo StageTI;
    extern const ClassInfo StageCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_display
{
    class NativeWindow;
}}

namespace InstanceTraits { namespace fl_display
{
    class NativeWindow;
}}

namespace Classes { namespace fl_display
{
    class NativeWindow;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Point;
    class Rectangle;
    class NativeMenu;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class NativeWindow : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeWindow"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeWindow(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

