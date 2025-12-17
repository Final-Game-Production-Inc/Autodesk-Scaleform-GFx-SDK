//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_StatusEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_StatusEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_codeGet, ASString> TFunc_Instances_StatusEvent_codeGet;
typedef ThunkFunc1<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_codeSet, const Value, const ASString&> TFunc_Instances_StatusEvent_codeSet;
typedef ThunkFunc0<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_levelGet, ASString> TFunc_Instances_StatusEvent_levelGet;
typedef ThunkFunc1<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_levelSet, const Value, const ASString&> TFunc_Instances_StatusEvent_levelSet;
typedef ThunkFunc0<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_StatusEvent_clone;
typedef ThunkFunc0<Instances::fl_events::StatusEvent, Instances::fl_events::StatusEvent::mid_toString, ASString> TFunc_Instances_StatusEvent_toString;

template <> const TFunc_Instances_StatusEvent_codeGet::TMethod TFunc_Instances_StatusEvent_codeGet::Method = &Instances::fl_events::StatusEvent::codeGet;
template <> const TFunc_Instances_StatusEvent_codeSet::TMethod TFunc_Instances_StatusEvent_codeSet::Method = &Instances::fl_events::StatusEvent::codeSet;
template <> const TFunc_Instances_StatusEvent_levelGet::TMethod TFunc_Instances_StatusEvent_levelGet::Method = &Instances::fl_events::StatusEvent::levelGet;
template <> const TFunc_Instances_StatusEvent_levelSet::TMethod TFunc_Instances_StatusEvent_levelSet::Method = &Instances::fl_events::StatusEvent::levelSet;
template <> const TFunc_Instances_StatusEvent_clone::TMethod TFunc_Instances_StatusEvent_clone::Method = &Instances::fl_events::StatusEvent::clone;
template <> const TFunc_Instances_StatusEvent_toString::TMethod TFunc_Instances_StatusEvent_toString::Method = &Instances::fl_events::StatusEvent::toString;

namespace Instances { namespace fl_events
{
    StatusEvent::StatusEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::StatusEvent::StatusEvent()$data"
, Code(GetVM().GetStringManager().CreateEmptyString())
, Level(GetVM().GetStringManager().CreateEmptyString())
//##protect##"instance::StatusEvent::StatusEvent()$data"
    {
//##protect##"instance::StatusEvent::StatusEvent()$code"
//##protect##"instance::StatusEvent::StatusEvent()$code"
    }

    void StatusEvent::codeGet(ASString& result)
    {
//##protect##"instance::StatusEvent::codeGet()"
        SF_UNUSED1(result);
        result = Code;
//##protect##"instance::StatusEvent::codeGet()"
    }
    void StatusEvent::codeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::StatusEvent::codeSet()"
        SF_UNUSED2(result, value);
        Code = value;
//##protect##"instance::StatusEvent::codeSet()"
    }
    void StatusEvent::levelGet(ASString& result)
    {
//##protect##"instance::StatusEvent::levelGet()"
        SF_UNUSED1(result);
        result = Level;
//##protect##"instance::StatusEvent::levelGet()"
    }
    void StatusEvent::levelSet(const Value& result, const ASString& value)
    {
//##protect##"instance::StatusEvent::levelSet()"
        SF_UNUSED2(result, value);
        Level = value;
//##protect##"instance::StatusEvent::levelSet()"
    }
    void StatusEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::StatusEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::StatusEvent::clone()"
    }
    void StatusEvent::toString(ASString& result)
    {
//##protect##"instance::StatusEvent::toString()"
        SF_UNUSED1(result);
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("StatusEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("code"),
			vm.GetStringManager().CreateConstString("level")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::StatusEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> StatusEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"

	AS3::Object* StatusEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).TextEventClass; 
    }

    SPtr<Instances::fl_events::Event> StatusEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        StatusEvent* pe = static_cast<StatusEvent*>(p.GetPtr());
        pe->Code = Code;
		pe->Level = Level;
        return p;
    }

    void StatusEvent::SetCode(wchar_t ch)
    {
        Code = GetVM().GetStringManager().CreateString(&ch, 1);
    }
    void StatusEvent::SetCode(const wchar_t* pwstr)
    {
        Code = GetVM().GetStringManager().CreateString(pwstr);
    }

	void StatusEvent::SetLevel(wchar_t ch)
    {
        Level = GetVM().GetStringManager().CreateString(&ch, 1);
    }
    void StatusEvent::SetLevel(const wchar_t* pwstr)
    {
        Level = GetVM().GetStringManager().CreateString(pwstr);
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 StatusEvent::tito[StatusEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* StatusEvent::tit[8] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo StatusEvent::ti[StatusEvent::ThunkInfoNum] = {
        {TFunc_Instances_StatusEvent_codeGet::Func, &StatusEvent::tit[0], "code", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StatusEvent_codeSet::Func, &StatusEvent::tit[1], "code", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_StatusEvent_levelGet::Func, &StatusEvent::tit[3], "level", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StatusEvent_levelSet::Func, &StatusEvent::tit[4], "level", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_StatusEvent_clone::Func, &StatusEvent::tit[6], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StatusEvent_toString::Func, &StatusEvent::tit[7], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    StatusEvent::StatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::StatusEvent::StatusEvent()"
//##protect##"InstanceTraits::StatusEvent::StatusEvent()"

    }

    void StatusEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<StatusEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    StatusEvent::StatusEvent(ClassTraits::Traits& t)
    : Class(t)
    , STATUS("status")
    {
//##protect##"class_::StatusEvent::StatusEvent()"
//##protect##"class_::StatusEvent::StatusEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo StatusEvent::mi[StatusEvent::MemberInfoNum] = {
        {"STATUS", NULL, OFFSETOF(Classes::fl_events::StatusEvent, STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StatusEvent::StatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::StatusEvent::StatusEvent()"
//##protect##"ClassTraits::StatusEvent::StatusEvent()"

    }

    Pickable<Traits> StatusEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StatusEvent(vm, AS3::fl_events::StatusEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::StatusEventCI));
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
    const TypeInfo StatusEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::StatusEvent::InstanceType),
        0,
        ClassTraits::fl_events::StatusEvent::MemberInfoNum,
        InstanceTraits::fl_events::StatusEvent::ThunkInfoNum,
        0,
        "StatusEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo StatusEventCI = {
        &StatusEventTI,
        ClassTraits::fl_events::StatusEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::StatusEvent::mi,
        InstanceTraits::fl_events::StatusEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

