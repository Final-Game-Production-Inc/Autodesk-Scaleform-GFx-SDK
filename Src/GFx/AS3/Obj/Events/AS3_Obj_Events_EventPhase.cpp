//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_EventPhase.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_EventPhase.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_events
{
    EventPhase::EventPhase(ClassTraits::Traits& t)
    : Class(t)
    , AT_TARGET(2)
    , BUBBLING_PHASE(3)
    , CAPTURING_PHASE(1)
    {
//##protect##"class_::EventPhase::EventPhase()"
//##protect##"class_::EventPhase::EventPhase()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo EventPhase::mi[EventPhase::MemberInfoNum] = {
        {"AT_TARGET", NULL, OFFSETOF(Classes::fl_events::EventPhase, AT_TARGET), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"BUBBLING_PHASE", NULL, OFFSETOF(Classes::fl_events::EventPhase, BUBBLING_PHASE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"CAPTURING_PHASE", NULL, OFFSETOF(Classes::fl_events::EventPhase, CAPTURING_PHASE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    EventPhase::EventPhase(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::EventPhase::EventPhase()"
//##protect##"ClassTraits::EventPhase::EventPhase()"

    }

    Pickable<Traits> EventPhase::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) EventPhase(vm, AS3::fl_events::EventPhaseCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::EventPhaseCI));
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
    const TypeInfo EventPhaseTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_events::EventPhase::InstanceType),
        0,
        ClassTraits::fl_events::EventPhase::MemberInfoNum,
        0,
        0,
        "EventPhase", "flash.events", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo EventPhaseCI = {
        &EventPhaseTI,
        ClassTraits::fl_events::EventPhase::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::EventPhase::mi,
        NULL,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

