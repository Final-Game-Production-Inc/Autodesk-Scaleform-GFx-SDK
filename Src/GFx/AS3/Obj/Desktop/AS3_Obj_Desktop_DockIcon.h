//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_DockIcon.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_DockIcon_H
#define INC_AS3_Obj_Desktop_DockIcon_H

#include "AS3_Obj_Desktop_InteractiveIcon.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo DockIconTI;
    extern const ClassInfo DockIconCI;
} // namespace fl_desktop
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo NativeMenuTI;
    extern const ClassInfo NativeMenuCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_desktop
{
    class DockIcon;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class DockIcon;
}}

namespace Classes { namespace fl_desktop
{
    class DockIcon;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class NativeMenu;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_desktop
{
    class DockIcon : public fl_desktop::InteractiveIcon
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DockIcon"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DockIcon(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

