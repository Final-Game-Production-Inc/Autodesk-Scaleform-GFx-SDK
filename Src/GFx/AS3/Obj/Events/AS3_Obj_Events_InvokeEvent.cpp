//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_InvokeEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_InvokeEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class File;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 InvokeEvent_tito[3] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* InvokeEvent_tit[3] = {
        &AS3::fl::ArrayTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_events::EventTI, 
    };
    const ThunkInfo InvokeEvent_ti[3] = {
        {ThunkInfo::EmptyFunc, &InvokeEvent_tit[0], "arguments", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &InvokeEvent_tit[1], "currentDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &InvokeEvent_tit[2], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    InvokeEvent::InvokeEvent(ClassTraits::Traits& t)
    : Class(t)
    , INVOKE("invoke")
    {
//##protect##"class_::InvokeEvent::InvokeEvent()"
//##protect##"class_::InvokeEvent::InvokeEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo InvokeEvent::mi[InvokeEvent::MemberInfoNum] = {
        {"INVOKE", NULL, OFFSETOF(Classes::fl_events::InvokeEvent, INVOKE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    InvokeEvent::InvokeEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::InvokeEvent::InvokeEvent()"
//##protect##"ClassTraits::InvokeEvent::InvokeEvent()"

    }

    Pickable<Traits> InvokeEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InvokeEvent(vm, AS3::fl_events::InvokeEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::InvokeEventCI));
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
    const TypeInfo InvokeEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::InvokeEvent::InstanceType),
        0,
        ClassTraits::fl_events::InvokeEvent::MemberInfoNum,
        3,
        0,
        "InvokeEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo InvokeEventCI = {
        &InvokeEventTI,
        ClassTraits::fl_events::InvokeEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::InvokeEvent::mi,
        InstanceTraits::fl_events::InvokeEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

