//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeMenuItem.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_NativeMenuItem_H
#define INC_AS3_Obj_Display_NativeMenuItem_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo NativeMenuItemTI;
    extern const ClassInfo NativeMenuItemCI;
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class NativeMenuItem;
}}

namespace InstanceTraits { namespace fl_display
{
    class NativeMenuItem;
}}

namespace Classes { namespace fl_display
{
    class NativeMenuItem;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class NativeMenu;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class NativeMenuItem : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeMenuItem"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeMenuItem(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

