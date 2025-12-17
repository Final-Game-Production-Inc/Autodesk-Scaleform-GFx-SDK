//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NativeApplication.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_NativeApplication_H
#define INC_AS3_Obj_Desktop_NativeApplication_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo NativeApplicationTI;
    extern const ClassInfo NativeApplicationCI;
    extern const TypeInfo InteractiveIconTI;
    extern const ClassInfo InteractiveIconCI;
} // namespace fl_desktop
namespace fl_display
{
    extern const TypeInfo NativeWindowTI;
    extern const ClassInfo NativeWindowCI;
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo XMLTI;
    extern const ClassInfo XMLCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
} // namespace fl
namespace fl_events
{
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_desktop
{
    class NativeApplication;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class NativeApplication;
}}

namespace Classes { namespace fl_desktop
{
    class NativeApplication;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class NativeMenu;
    class NativeWindow;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_desktop
{
    class NativeApplication : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NativeApplication"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NativeApplication(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

