//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_HTMLUncaughtScriptExceptionEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_events
{
    HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
    , exceptionValue()
    , stackTrace()
//##protect##"instance::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()$data"
//##protect##"instance::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()$data"
    {
//##protect##"instance::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()$code"
//##protect##"instance::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 HTMLUncaughtScriptExceptionEvent::tito[HTMLUncaughtScriptExceptionEvent::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* HTMLUncaughtScriptExceptionEvent::tit[1] = {
        &AS3::fl_events::EventTI, 
    };
    const ThunkInfo HTMLUncaughtScriptExceptionEvent::ti[HTMLUncaughtScriptExceptionEvent::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &HTMLUncaughtScriptExceptionEvent::tit[0], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo HTMLUncaughtScriptExceptionEvent::mi[HTMLUncaughtScriptExceptionEvent::MemberInfoNum] = {
        {"exceptionValue", NULL, OFFSETOF(Instances::fl_events::HTMLUncaughtScriptExceptionEvent, exceptionValue), Abc::NS_Public, SlotInfo::BT_Value, 0},
        {"stackTrace", NULL, OFFSETOF(Instances::fl_events::HTMLUncaughtScriptExceptionEvent, stackTrace), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
    };


    HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"
//##protect##"InstanceTraits::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"

    }

    void HTMLUncaughtScriptExceptionEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<HTMLUncaughtScriptExceptionEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"
//##protect##"class_::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo HTMLUncaughtScriptExceptionEvent::mi[HTMLUncaughtScriptExceptionEvent::MemberInfoNum] = {
        {"UNCAUGHT_SCRIPT_EXCEPTION", NULL, OFFSETOF(Classes::fl_events::HTMLUncaughtScriptExceptionEvent, UNCAUGHT_SCRIPT_EXCEPTION), Abc::NS_Public, SlotInfo::BT_Value, 1},
    };


    HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"
//##protect##"ClassTraits::HTMLUncaughtScriptExceptionEvent::HTMLUncaughtScriptExceptionEvent()"

    }

    Pickable<Traits> HTMLUncaughtScriptExceptionEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLUncaughtScriptExceptionEvent(vm, AS3::fl_events::HTMLUncaughtScriptExceptionEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::HTMLUncaughtScriptExceptionEventCI));
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
    const TypeInfo HTMLUncaughtScriptExceptionEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::HTMLUncaughtScriptExceptionEvent::InstanceType),
        0,
        ClassTraits::fl_events::HTMLUncaughtScriptExceptionEvent::MemberInfoNum,
        InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent::ThunkInfoNum,
        InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent::MemberInfoNum,
        "HTMLUncaughtScriptExceptionEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo HTMLUncaughtScriptExceptionEventCI = {
        &HTMLUncaughtScriptExceptionEventTI,
        ClassTraits::fl_events::HTMLUncaughtScriptExceptionEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::HTMLUncaughtScriptExceptionEvent::mi,
        InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent::ti,
        InstanceTraits::fl_events::HTMLUncaughtScriptExceptionEvent::mi,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

