//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ContextMenuEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_ContextMenuEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 ContextMenuEvent_tito[6] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* ContextMenuEvent_tit[8] = {
        &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, 
        &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ContextMenuEvent_ti[6] = {
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[0], "contextMenuOwner", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[1], "contextMenuOwner", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[3], "mouseTarget", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[4], "mouseTarget", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[6], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuEvent_tit[7], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    ContextMenuEvent::ContextMenuEvent(ClassTraits::Traits& t)
    : Class(t)
    , MENU_ITEM_SELECT("menuItemSelect")
    , MENU_SELECT("menuSelect")
    {
//##protect##"class_::ContextMenuEvent::ContextMenuEvent()"
//##protect##"class_::ContextMenuEvent::ContextMenuEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo ContextMenuEvent::mi[ContextMenuEvent::MemberInfoNum] = {
        {"MENU_ITEM_SELECT", NULL, OFFSETOF(Classes::fl_events::ContextMenuEvent, MENU_ITEM_SELECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MENU_SELECT", NULL, OFFSETOF(Classes::fl_events::ContextMenuEvent, MENU_SELECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    ContextMenuEvent::ContextMenuEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::ContextMenuEvent::ContextMenuEvent()"
//##protect##"ClassTraits::ContextMenuEvent::ContextMenuEvent()"

    }

    Pickable<Traits> ContextMenuEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ContextMenuEvent(vm, AS3::fl_events::ContextMenuEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::ContextMenuEventCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_events
{
    const TypeInfo ContextMenuEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::ContextMenuEvent::InstanceType),
        0,
        ClassTraits::fl_events::ContextMenuEvent::MemberInfoNum,
        6,
        0,
        "ContextMenuEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo ContextMenuEventCI = {
        &ContextMenuEventTI,
        ClassTraits::fl_events::ContextMenuEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::ContextMenuEvent::mi,
        InstanceTraits::fl_events::ContextMenuEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

