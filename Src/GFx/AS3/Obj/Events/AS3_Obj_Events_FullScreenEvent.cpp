//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_FullScreenEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_FullScreenEvent.h"
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
    // const UInt16 FullScreenEvent_tito[3] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* FullScreenEvent_tit[3] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo FullScreenEvent_ti[3] = {
        {ThunkInfo::EmptyFunc, &FullScreenEvent_tit[0], "fullScreen", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FullScreenEvent_tit[1], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FullScreenEvent_tit[2], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    FullScreenEvent::FullScreenEvent(ClassTraits::Traits& t)
    : Class(t)
    , FULL_SCREEN("fullScreen")
    {
//##protect##"class_::FullScreenEvent::FullScreenEvent()"
//##protect##"class_::FullScreenEvent::FullScreenEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo FullScreenEvent::mi[FullScreenEvent::MemberInfoNum] = {
        {"FULL_SCREEN", NULL, OFFSETOF(Classes::fl_events::FullScreenEvent, FULL_SCREEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    FullScreenEvent::FullScreenEvent(VM& vm, const ClassInfo& ci)
    : fl_events::ActivityEvent(vm, ci)
    {
//##protect##"ClassTraits::FullScreenEvent::FullScreenEvent()"
//##protect##"ClassTraits::FullScreenEvent::FullScreenEvent()"

    }

    Pickable<Traits> FullScreenEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FullScreenEvent(vm, AS3::fl_events::FullScreenEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::FullScreenEventCI));
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
    const TypeInfo FullScreenEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::FullScreenEvent::InstanceType),
        0,
        ClassTraits::fl_events::FullScreenEvent::MemberInfoNum,
        3,
        0,
        "FullScreenEvent", "flash.events", &fl_events::ActivityEventTI,
        TypeInfo::None
    };

    const ClassInfo FullScreenEventCI = {
        &FullScreenEventTI,
        ClassTraits::fl_events::FullScreenEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::FullScreenEvent::mi,
        InstanceTraits::fl_events::FullScreenEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

