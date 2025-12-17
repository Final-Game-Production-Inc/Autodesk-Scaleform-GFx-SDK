//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_SecurityErrorEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_SecurityErrorEvent.h"
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
    // const UInt16 SecurityErrorEvent_tito[2] = {
    //    0, 1, 
    // };
    const TypeInfo* SecurityErrorEvent_tit[2] = {
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo SecurityErrorEvent_ti[2] = {
        {ThunkInfo::EmptyFunc, &SecurityErrorEvent_tit[0], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SecurityErrorEvent_tit[1], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    SecurityErrorEvent::SecurityErrorEvent(ClassTraits::Traits& t)
    : Class(t)
    , SECURITY_ERROR("securityError")
    {
//##protect##"class_::SecurityErrorEvent::SecurityErrorEvent()"
//##protect##"class_::SecurityErrorEvent::SecurityErrorEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo SecurityErrorEvent::mi[SecurityErrorEvent::MemberInfoNum] = {
        {"SECURITY_ERROR", NULL, OFFSETOF(Classes::fl_events::SecurityErrorEvent, SECURITY_ERROR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SecurityErrorEvent::SecurityErrorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::ErrorEvent(vm, ci)
    {
//##protect##"ClassTraits::SecurityErrorEvent::SecurityErrorEvent()"
//##protect##"ClassTraits::SecurityErrorEvent::SecurityErrorEvent()"

    }

    Pickable<Traits> SecurityErrorEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SecurityErrorEvent(vm, AS3::fl_events::SecurityErrorEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::SecurityErrorEventCI));
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
    const TypeInfo SecurityErrorEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::SecurityErrorEvent::InstanceType),
        0,
        ClassTraits::fl_events::SecurityErrorEvent::MemberInfoNum,
        2,
        0,
        "SecurityErrorEvent", "flash.events", &fl_events::ErrorEventTI,
        TypeInfo::None
    };

    const ClassInfo SecurityErrorEventCI = {
        &SecurityErrorEventTI,
        ClassTraits::fl_events::SecurityErrorEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::SecurityErrorEvent::mi,
        InstanceTraits::fl_events::SecurityErrorEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

