//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_NetStatusEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_NetStatusEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::NetStatusEvent, Instances::fl_events::NetStatusEvent::mid_infoGet, SPtr<Instances::fl::Object> > TFunc_Instances_NetStatusEvent_infoGet;
typedef ThunkFunc1<Instances::fl_events::NetStatusEvent, Instances::fl_events::NetStatusEvent::mid_infoSet, const Value, const Value&> TFunc_Instances_NetStatusEvent_infoSet;
typedef ThunkFunc0<Instances::fl_events::NetStatusEvent, Instances::fl_events::NetStatusEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_NetStatusEvent_clone;
typedef ThunkFunc0<Instances::fl_events::NetStatusEvent, Instances::fl_events::NetStatusEvent::mid_toString, ASString> TFunc_Instances_NetStatusEvent_toString;

template <> const TFunc_Instances_NetStatusEvent_infoGet::TMethod TFunc_Instances_NetStatusEvent_infoGet::Method = &Instances::fl_events::NetStatusEvent::infoGet;
template <> const TFunc_Instances_NetStatusEvent_infoSet::TMethod TFunc_Instances_NetStatusEvent_infoSet::Method = &Instances::fl_events::NetStatusEvent::infoSet;
template <> const TFunc_Instances_NetStatusEvent_clone::TMethod TFunc_Instances_NetStatusEvent_clone::Method = &Instances::fl_events::NetStatusEvent::clone;
template <> const TFunc_Instances_NetStatusEvent_toString::TMethod TFunc_Instances_NetStatusEvent_toString::Method = &Instances::fl_events::NetStatusEvent::toString;

namespace Instances { namespace fl_events
{
    NetStatusEvent::NetStatusEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::NetStatusEvent::NetStatusEvent()$data"
    , Code( GetVM().GetStringManager().CreateEmptyString())
    , Level(GetVM().GetStringManager().CreateEmptyString())
//##protect##"instance::NetStatusEvent::NetStatusEvent()$data"
    {
//##protect##"instance::NetStatusEvent::NetStatusEvent()$code"
//##protect##"instance::NetStatusEvent::NetStatusEvent()$code"
    }

    void NetStatusEvent::infoGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::NetStatusEvent::infoGet()"
        SPtr<Instances::fl::Object> pobj = GetVM().MakeObject();
        StringManager& sm = GetVM().GetStringManager();
        pobj->AddDynamicSlotValuePair(sm.CreateString("code"),  Code);
        pobj->AddDynamicSlotValuePair(sm.CreateString("level"), Level);
        result = pobj;
//##protect##"instance::NetStatusEvent::infoGet()"
    }
    void NetStatusEvent::infoSet(const Value& result, const Value& value)
    {
//##protect##"instance::NetStatusEvent::infoSet()"
        SF_UNUSED(result);
        Instances::fl::Object* pobj = static_cast<Instances::fl::Object*>(value.GetObject());
        StringManager& sm = GetVM().GetStringManager();

        Multiname mnCode( GetVM().GetPublicNamespace(), sm.CreateString("code"));
        Multiname mnLevel(GetVM().GetPublicNamespace(), sm.CreateString("level"));
        Value prop;
        if (pobj->GetProperty(mnCode, prop))
            prop.Convert2String(Code);
        if (pobj->GetProperty(mnLevel, prop))
            prop.Convert2String(Level);
//##protect##"instance::NetStatusEvent::infoSet()"
    }
    void NetStatusEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::NetStatusEvent::clone()"
        SF_UNUSED(result);
        result = Clone().GetPtr();
//##protect##"instance::NetStatusEvent::clone()"
    }
    void NetStatusEvent::toString(ASString& result)
    {
//##protect##"instance::NetStatusEvent::toString()"
        StringManager& sm = GetVM().GetStringManager();
        Value params[] = {
            sm.CreateConstString(GetEventName()),
            sm.CreateConstString("type"),
            sm.CreateConstString("bubbles"),
            sm.CreateConstString("cancelable"),
            sm.CreateConstString("info"),
            sm.CreateConstString("target")
        };
        Value v;
        formatToString(v, sizeof(params)/sizeof(params[0]), params);
        v.Convert2String(result).DoNotCheck();
//##protect##"instance::NetStatusEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> NetStatusEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void NetStatusEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
            argv[3].Convert2String(Code).DoNotCheck();
        if (argc >= 5)
            argv[4].Convert2String(Level).DoNotCheck();
    }

    SPtr<Instances::fl_events::Event> NetStatusEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        NetStatusEvent* pe = static_cast<NetStatusEvent*>(p.GetPtr());
        pe->Code  = Code;
        pe->Level = Level;
        return p;
    }
    const char* NetStatusEvent::GetEventName() const
    {
        return "NetStatusEvent";
    }
    AS3::Object* NetStatusEvent::GetEventClass() const
    { 
        Class* result = GetVM().GetClass("flash.events.NetStatusEvent", GetVM().GetCurrentAppDomain());
        SF_ASSERT(result);
        return result;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 NetStatusEvent::tito[NetStatusEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* NetStatusEvent::tit[5] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo NetStatusEvent::ti[NetStatusEvent::ThunkInfoNum] = {
        {TFunc_Instances_NetStatusEvent_infoGet::Func, &NetStatusEvent::tit[0], "info", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetStatusEvent_infoSet::Func, &NetStatusEvent::tit[1], "info", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_NetStatusEvent_clone::Func, &NetStatusEvent::tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetStatusEvent_toString::Func, &NetStatusEvent::tit[4], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    NetStatusEvent::NetStatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::NetStatusEvent::NetStatusEvent()"
//##protect##"InstanceTraits::NetStatusEvent::NetStatusEvent()"

    }

    void NetStatusEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<NetStatusEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    NetStatusEvent::NetStatusEvent(ClassTraits::Traits& t)
    : Class(t)
    , NET_STATUS("netStatus")
    {
//##protect##"class_::NetStatusEvent::NetStatusEvent()"
//##protect##"class_::NetStatusEvent::NetStatusEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo NetStatusEvent::mi[NetStatusEvent::MemberInfoNum] = {
        {"NET_STATUS", NULL, OFFSETOF(Classes::fl_events::NetStatusEvent, NET_STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NetStatusEvent::NetStatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::NetStatusEvent::NetStatusEvent()"
//##protect##"ClassTraits::NetStatusEvent::NetStatusEvent()"

    }

    Pickable<Traits> NetStatusEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NetStatusEvent(vm, AS3::fl_events::NetStatusEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::NetStatusEventCI));
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
    const TypeInfo NetStatusEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::NetStatusEvent::InstanceType),
        0,
        ClassTraits::fl_events::NetStatusEvent::MemberInfoNum,
        InstanceTraits::fl_events::NetStatusEvent::ThunkInfoNum,
        0,
        "NetStatusEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo NetStatusEventCI = {
        &NetStatusEventTI,
        ClassTraits::fl_events::NetStatusEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::NetStatusEvent::mi,
        InstanceTraits::fl_events::NetStatusEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

