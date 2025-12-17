//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ContextMenuEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_ContextMenuEvent_H
#define INC_AS3_Obj_Events_ContextMenuEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo ContextMenuEventTI;
    extern const ClassInfo ContextMenuEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class ContextMenuEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class ContextMenuEvent;
}}

namespace Classes { namespace fl_events
{
    class ContextMenuEvent;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class InteractiveObject;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_events
{
    class ContextMenuEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ContextMenuEvent"; }
#endif
    public:
        typedef Classes::fl_events::ContextMenuEvent ClassType;
        typedef InstanceTraits::fl_events::Event InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ContextMenuEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class ContextMenuEvent : public Class
    {
        friend class ClassTraits::fl_events::ContextMenuEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::ContextMenuEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ContextMenuEvent"; }
#endif
    public:
        typedef ContextMenuEvent SelfType;
        typedef ContextMenuEvent ClassType;
        
    private:
        ContextMenuEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* MENU_ITEM_SELECT;
        const char* MENU_SELECT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

