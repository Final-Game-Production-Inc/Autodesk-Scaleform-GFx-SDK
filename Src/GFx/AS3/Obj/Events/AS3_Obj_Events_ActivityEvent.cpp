//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ActivityEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_ActivityEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 ActivityEvent_tito[4] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* ActivityEvent_tit[5] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ActivityEvent_ti[4] = {
        {ThunkInfo::EmptyFunc, &ActivityEvent_tit[0], "activating", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ActivityEvent_tit[1], "activating", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ActivityEvent_tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ActivityEvent_tit[4], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    ActivityEvent::ActivityEvent(ClassTraits::Traits& t)
    : Class(t)
    , ACTIVITY("activity")
    {
//##protect##"class_::ActivityEvent::ActivityEvent()"
//##protect##"class_::ActivityEvent::ActivityEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo ActivityEvent::mi[ActivityEvent::MemberInfoNum] = {
        {"ACTIVITY", NULL, OFFSETOF(Classes::fl_events::ActivityEvent, ACTIVITY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    ActivityEvent::ActivityEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::ActivityEvent::ActivityEvent()"
//##protect##"ClassTraits::ActivityEvent::ActivityEvent()"

    }

    Pickable<Traits> ActivityEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ActivityEvent(vm, AS3::fl_events::ActivityEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::ActivityEventCI));
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
    const TypeInfo ActivityEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::ActivityEvent::InstanceType),
        0,
        ClassTraits::fl_events::ActivityEvent::MemberInfoNum,
        4,
        0,
        "ActivityEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo ActivityEventCI = {
        &ActivityEventTI,
        ClassTraits::fl_events::ActivityEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::ActivityEvent::mi,
        InstanceTraits::fl_events::ActivityEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

