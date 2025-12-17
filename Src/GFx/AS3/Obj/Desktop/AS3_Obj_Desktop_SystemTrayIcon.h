//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_SystemTrayIcon.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Desktop_SystemTrayIcon_H
#define INC_AS3_Obj_Desktop_SystemTrayIcon_H

#include "AS3_Obj_Desktop_InteractiveIcon.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_desktop
{
    extern const TypeInfo SystemTrayIconTI;
    extern const ClassInfo SystemTrayIconCI;
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
    class SystemTrayIcon;
}}

namespace InstanceTraits { namespace fl_desktop
{
    class SystemTrayIcon;
}}

namespace Classes { namespace fl_desktop
{
    class SystemTrayIcon;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class NativeMenu;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_desktop
{
    class SystemTrayIcon : public fl_desktop::InteractiveIcon
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SystemTrayIcon"; }
#endif
    public:
        typedef Classes::fl_desktop::SystemTrayIcon ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SystemTrayIcon(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_desktop
{
    class SystemTrayIcon : public Class
    {
        friend class ClassTraits::fl_desktop::SystemTrayIcon;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_desktop::SystemTrayIconTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SystemTrayIcon"; }
#endif
    public:
        typedef SystemTrayIcon SelfType;
        typedef SystemTrayIcon ClassType;
        
    private:
        SystemTrayIcon(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const Value::Number MAX_TIP_LENGTH;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

