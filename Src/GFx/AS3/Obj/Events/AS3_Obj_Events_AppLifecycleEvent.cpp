//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_AppLifecycleEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_AppLifecycleEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::AppLifecycleEvent, Instances::fl_events::AppLifecycleEvent::mid_statusGet, SPtr<Instances::fl::Object> > TFunc_Instances_AppLifecycleEvent_statusGet;
typedef ThunkFunc0<Instances::fl_events::AppLifecycleEvent, Instances::fl_events::AppLifecycleEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_AppLifecycleEvent_clone;
typedef ThunkFunc0<Instances::fl_events::AppLifecycleEvent, Instances::fl_events::AppLifecycleEvent::mid_toString, ASString> TFunc_Instances_AppLifecycleEvent_toString;

template <> const TFunc_Instances_AppLifecycleEvent_statusGet::TMethod TFunc_Instances_AppLifecycleEvent_statusGet::Method = &Instances::fl_events::AppLifecycleEvent::statusGet;
template <> const TFunc_Instances_AppLifecycleEvent_clone::TMethod TFunc_Instances_AppLifecycleEvent_clone::Method = &Instances::fl_events::AppLifecycleEvent::clone;
template <> const TFunc_Instances_AppLifecycleEvent_toString::TMethod TFunc_Instances_AppLifecycleEvent_toString::Method = &Instances::fl_events::AppLifecycleEvent::toString;

namespace Instances { namespace fl_events
{
    AppLifecycleEvent::AppLifecycleEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::AppLifecycleEvent::AppLifecycleEvent()$data"
//##protect##"instance::AppLifecycleEvent::AppLifecycleEvent()$data"
    {
//##protect##"instance::AppLifecycleEvent::AppLifecycleEvent()$code"
        Status.SetNull();
//##protect##"instance::AppLifecycleEvent::AppLifecycleEvent()$code"
    }

    void AppLifecycleEvent::statusGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::AppLifecycleEvent::statusGet()"
        if (Status.IsObject())
            result = static_cast<Instances::fl::Object*>(Status.GetObject());
//##protect##"instance::AppLifecycleEvent::statusGet()"
    }
    void AppLifecycleEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::AppLifecycleEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::AppLifecycleEvent::clone()"
    }
    void AppLifecycleEvent::toString(ASString& result)
    {
//##protect##"instance::AppLifecycleEvent::toString()"
        SF_UNUSED1(result);
        //example: [AppLifecycleEvent type="sss" bubbles=false cancelable=false status=null]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("AppLifecycleEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("status")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::AppLifecycleEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> AppLifecycleEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void AppLifecycleEvent::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_events::Event::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC(prcc, Status, op SF_DEBUG_ARG(*this));
    }
    // public function AppLifecycleEvent(type:String, bubbles:Boolean = false, cancelable:Boolean = false, 
    //                                   status:String = null)
    void AppLifecycleEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        ASVM& vm = static_cast<ASVM&>(GetVM());
        if (argc >= 4)
        {
            Status = argv[3];
            if (!argv[3].IsNull())
            {
                Status.ToStringValue(vm.GetStringManager()).DoNotCheck();
            }
        }
    }
    AS3::Object* AppLifecycleEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).AppLifecycleEventClass; 
    }

    SPtr<Instances::fl_events::Event> AppLifecycleEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        AppLifecycleEvent* pe = static_cast<AppLifecycleEvent*>(p.GetPtr());
        pe->Status  = Status;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 AppLifecycleEvent::tito[AppLifecycleEvent::ThunkInfoNum] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* AppLifecycleEvent::tit[3] = {
        &AS3::fl::ObjectTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo AppLifecycleEvent::ti[AppLifecycleEvent::ThunkInfoNum] = {
        {TFunc_Instances_AppLifecycleEvent_statusGet::Func, &AppLifecycleEvent::tit[0], "status", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AppLifecycleEvent_clone::Func, &AppLifecycleEvent::tit[1], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AppLifecycleEvent_toString::Func, &AppLifecycleEvent::tit[2], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    AppLifecycleEvent::AppLifecycleEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::AppLifecycleEvent::AppLifecycleEvent()"
//##protect##"InstanceTraits::AppLifecycleEvent::AppLifecycleEvent()"

    }

    void AppLifecycleEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<AppLifecycleEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    AppLifecycleEvent::AppLifecycleEvent(ClassTraits::Traits& t)
    : Class(t)
    , ONPAUSE("onPause")
    , ONRESUME("onResume")
    {
//##protect##"class_::AppLifecycleEvent::AppLifecycleEvent()"
//##protect##"class_::AppLifecycleEvent::AppLifecycleEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo AppLifecycleEvent::mi[AppLifecycleEvent::MemberInfoNum] = {
        {"ONPAUSE", NULL, OFFSETOF(Classes::fl_events::AppLifecycleEvent, ONPAUSE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ONRESUME", NULL, OFFSETOF(Classes::fl_events::AppLifecycleEvent, ONRESUME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    AppLifecycleEvent::AppLifecycleEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::AppLifecycleEvent::AppLifecycleEvent()"
//##protect##"ClassTraits::AppLifecycleEvent::AppLifecycleEvent()"

    }

    Pickable<Traits> AppLifecycleEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) AppLifecycleEvent(vm, AS3::fl_events::AppLifecycleEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::AppLifecycleEventCI));
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
    const TypeInfo AppLifecycleEventTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_events::AppLifecycleEvent::InstanceType),
        0,
        ClassTraits::fl_events::AppLifecycleEvent::MemberInfoNum,
        InstanceTraits::fl_events::AppLifecycleEvent::ThunkInfoNum,
        0,
        "AppLifecycleEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo AppLifecycleEventCI = {
        &AppLifecycleEventTI,
        ClassTraits::fl_events::AppLifecycleEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::AppLifecycleEvent::mi,
        InstanceTraits::fl_events::AppLifecycleEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

