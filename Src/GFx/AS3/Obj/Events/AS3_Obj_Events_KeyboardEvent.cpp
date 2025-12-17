//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_KeyboardEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_KeyboardEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_altKeyGet, bool> TFunc_Instances_KeyboardEvent_altKeyGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_altKeySet, const Value, bool> TFunc_Instances_KeyboardEvent_altKeySet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_charCodeGet, UInt32> TFunc_Instances_KeyboardEvent_charCodeGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_charCodeSet, const Value, UInt32> TFunc_Instances_KeyboardEvent_charCodeSet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_commandKeyGet, bool> TFunc_Instances_KeyboardEvent_commandKeyGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_commandKeySet, const Value, bool> TFunc_Instances_KeyboardEvent_commandKeySet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_controlKeyGet, bool> TFunc_Instances_KeyboardEvent_controlKeyGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_controlKeySet, const Value, bool> TFunc_Instances_KeyboardEvent_controlKeySet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_ctrlKeyGet, bool> TFunc_Instances_KeyboardEvent_ctrlKeyGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_ctrlKeySet, const Value, bool> TFunc_Instances_KeyboardEvent_ctrlKeySet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_keyCodeGet, UInt32> TFunc_Instances_KeyboardEvent_keyCodeGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_keyCodeSet, const Value, UInt32> TFunc_Instances_KeyboardEvent_keyCodeSet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_keyLocationGet, UInt32> TFunc_Instances_KeyboardEvent_keyLocationGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_keyLocationSet, const Value, UInt32> TFunc_Instances_KeyboardEvent_keyLocationSet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_shiftKeyGet, bool> TFunc_Instances_KeyboardEvent_shiftKeyGet;
typedef ThunkFunc1<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_shiftKeySet, const Value, bool> TFunc_Instances_KeyboardEvent_shiftKeySet;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_KeyboardEvent_clone;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_toString, ASString> TFunc_Instances_KeyboardEvent_toString;
typedef ThunkFunc0<Instances::fl_events::KeyboardEvent, Instances::fl_events::KeyboardEvent::mid_updateAfterEvent, const Value> TFunc_Instances_KeyboardEvent_updateAfterEvent;

template <> const TFunc_Instances_KeyboardEvent_altKeyGet::TMethod TFunc_Instances_KeyboardEvent_altKeyGet::Method = &Instances::fl_events::KeyboardEvent::altKeyGet;
template <> const TFunc_Instances_KeyboardEvent_altKeySet::TMethod TFunc_Instances_KeyboardEvent_altKeySet::Method = &Instances::fl_events::KeyboardEvent::altKeySet;
template <> const TFunc_Instances_KeyboardEvent_charCodeGet::TMethod TFunc_Instances_KeyboardEvent_charCodeGet::Method = &Instances::fl_events::KeyboardEvent::charCodeGet;
template <> const TFunc_Instances_KeyboardEvent_charCodeSet::TMethod TFunc_Instances_KeyboardEvent_charCodeSet::Method = &Instances::fl_events::KeyboardEvent::charCodeSet;
template <> const TFunc_Instances_KeyboardEvent_commandKeyGet::TMethod TFunc_Instances_KeyboardEvent_commandKeyGet::Method = &Instances::fl_events::KeyboardEvent::commandKeyGet;
template <> const TFunc_Instances_KeyboardEvent_commandKeySet::TMethod TFunc_Instances_KeyboardEvent_commandKeySet::Method = &Instances::fl_events::KeyboardEvent::commandKeySet;
template <> const TFunc_Instances_KeyboardEvent_controlKeyGet::TMethod TFunc_Instances_KeyboardEvent_controlKeyGet::Method = &Instances::fl_events::KeyboardEvent::controlKeyGet;
template <> const TFunc_Instances_KeyboardEvent_controlKeySet::TMethod TFunc_Instances_KeyboardEvent_controlKeySet::Method = &Instances::fl_events::KeyboardEvent::controlKeySet;
template <> const TFunc_Instances_KeyboardEvent_ctrlKeyGet::TMethod TFunc_Instances_KeyboardEvent_ctrlKeyGet::Method = &Instances::fl_events::KeyboardEvent::ctrlKeyGet;
template <> const TFunc_Instances_KeyboardEvent_ctrlKeySet::TMethod TFunc_Instances_KeyboardEvent_ctrlKeySet::Method = &Instances::fl_events::KeyboardEvent::ctrlKeySet;
template <> const TFunc_Instances_KeyboardEvent_keyCodeGet::TMethod TFunc_Instances_KeyboardEvent_keyCodeGet::Method = &Instances::fl_events::KeyboardEvent::keyCodeGet;
template <> const TFunc_Instances_KeyboardEvent_keyCodeSet::TMethod TFunc_Instances_KeyboardEvent_keyCodeSet::Method = &Instances::fl_events::KeyboardEvent::keyCodeSet;
template <> const TFunc_Instances_KeyboardEvent_keyLocationGet::TMethod TFunc_Instances_KeyboardEvent_keyLocationGet::Method = &Instances::fl_events::KeyboardEvent::keyLocationGet;
template <> const TFunc_Instances_KeyboardEvent_keyLocationSet::TMethod TFunc_Instances_KeyboardEvent_keyLocationSet::Method = &Instances::fl_events::KeyboardEvent::keyLocationSet;
template <> const TFunc_Instances_KeyboardEvent_shiftKeyGet::TMethod TFunc_Instances_KeyboardEvent_shiftKeyGet::Method = &Instances::fl_events::KeyboardEvent::shiftKeyGet;
template <> const TFunc_Instances_KeyboardEvent_shiftKeySet::TMethod TFunc_Instances_KeyboardEvent_shiftKeySet::Method = &Instances::fl_events::KeyboardEvent::shiftKeySet;
template <> const TFunc_Instances_KeyboardEvent_clone::TMethod TFunc_Instances_KeyboardEvent_clone::Method = &Instances::fl_events::KeyboardEvent::clone;
template <> const TFunc_Instances_KeyboardEvent_toString::TMethod TFunc_Instances_KeyboardEvent_toString::Method = &Instances::fl_events::KeyboardEvent::toString;
template <> const TFunc_Instances_KeyboardEvent_updateAfterEvent::TMethod TFunc_Instances_KeyboardEvent_updateAfterEvent::Method = &Instances::fl_events::KeyboardEvent::updateAfterEvent;

namespace Instances { namespace fl_events
{
    KeyboardEvent::KeyboardEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::KeyboardEvent::KeyboardEvent()$data"
    , KeyLocation(-1)
//##protect##"instance::KeyboardEvent::KeyboardEvent()$data"
    {
//##protect##"instance::KeyboardEvent::KeyboardEvent()$code"
//##protect##"instance::KeyboardEvent::KeyboardEvent()$code"
    }

    void KeyboardEvent::altKeyGet(bool& result)
    {
//##protect##"instance::KeyboardEvent::altKeyGet()"
        result = EvtId.KeysState.IsAltPressed();
//##protect##"instance::KeyboardEvent::altKeyGet()"
    }
    void KeyboardEvent::altKeySet(const Value& result, bool value)
    {
//##protect##"instance::KeyboardEvent::altKeySet()"
        SF_UNUSED(result);
        EvtId.KeysState.SetAltPressed(value);
//##protect##"instance::KeyboardEvent::altKeySet()"
    }
    void KeyboardEvent::charCodeGet(UInt32& result)
    {
//##protect##"instance::KeyboardEvent::charCodeGet()"
        result = (UInt32)EvtId.AsciiCode;
//##protect##"instance::KeyboardEvent::charCodeGet()"
    }
    void KeyboardEvent::charCodeSet(const Value& result, UInt32 value)
    {
//##protect##"instance::KeyboardEvent::charCodeSet()"
        SF_UNUSED(result);
        EvtId.AsciiCode = (UByte)value;
//##protect##"instance::KeyboardEvent::charCodeSet()"
    }
    void KeyboardEvent::commandKeyGet(bool& result)
    {
//##protect##"instance::KeyboardEvent::commandKeyGet()"
        result = EvtId.KeysState.IsCtrlPressed();
//##protect##"instance::KeyboardEvent::commandKeyGet()"
    }
    void KeyboardEvent::commandKeySet(const Value& result, bool value)
    {
//##protect##"instance::KeyboardEvent::commandKeySet()"
        SF_UNUSED1(result);
        EvtId.KeysState.SetCtrlPressed(value);
//##protect##"instance::KeyboardEvent::commandKeySet()"
    }
    void KeyboardEvent::controlKeyGet(bool& result)
    {
//##protect##"instance::KeyboardEvent::controlKeyGet()"
        result = EvtId.KeysState.IsCtrlPressed();
//##protect##"instance::KeyboardEvent::controlKeyGet()"
    }
    void KeyboardEvent::controlKeySet(const Value& result, bool value)
    {
//##protect##"instance::KeyboardEvent::controlKeySet()"
        SF_UNUSED1(result);
        EvtId.KeysState.SetCtrlPressed(value);
//##protect##"instance::KeyboardEvent::controlKeySet()"
    }
    void KeyboardEvent::ctrlKeyGet(bool& result)
    {
//##protect##"instance::KeyboardEvent::ctrlKeyGet()"
        result = EvtId.KeysState.IsCtrlPressed();
//##protect##"instance::KeyboardEvent::ctrlKeyGet()"
    }
    void KeyboardEvent::ctrlKeySet(const Value& result, bool value)
    {
//##protect##"instance::KeyboardEvent::ctrlKeySet()"
        SF_UNUSED1(result);
        EvtId.KeysState.SetCtrlPressed(value);
//##protect##"instance::KeyboardEvent::ctrlKeySet()"
    }
    void KeyboardEvent::keyCodeGet(UInt32& result)
    {
//##protect##"instance::KeyboardEvent::keyCodeGet()"
        result = (UInt32)EvtId.KeyCode;
//##protect##"instance::KeyboardEvent::keyCodeGet()"
    }
    void KeyboardEvent::keyCodeSet(const Value& result, UInt32 value)
    {
//##protect##"instance::KeyboardEvent::keyCodeSet()"
        SF_UNUSED(result);
        EvtId.KeyCode = value;
//##protect##"instance::KeyboardEvent::keyCodeSet()"
    }
    void KeyboardEvent::keyLocationGet(UInt32& result)
    {
//##protect##"instance::KeyboardEvent::keyLocationGet()"
        SF_UNUSED1(result);
        if (KeyLocation < 0) // not initialized yet
        {
            if (EvtId.KeysState.IsSpecialKeyPressed())
                KeyLocation = (EvtId.KeysState.IsExtendedKey() ? 2 : 1);
            else
                KeyLocation = 0;
        }
        result = (UInt32)KeyLocation;
//##protect##"instance::KeyboardEvent::keyLocationGet()"
    }
    void KeyboardEvent::keyLocationSet(const Value& result, UInt32 value)
    {
//##protect##"instance::KeyboardEvent::keyLocationSet()"
        SF_UNUSED2(result, value);
        KeyLocation = value;
//##protect##"instance::KeyboardEvent::keyLocationSet()"
    }
    void KeyboardEvent::shiftKeyGet(bool& result)
    {
//##protect##"instance::KeyboardEvent::shiftKeyGet()"
        result = EvtId.KeysState.IsShiftPressed();
//##protect##"instance::KeyboardEvent::shiftKeyGet()"
    }
    void KeyboardEvent::shiftKeySet(const Value& result, bool value)
    {
//##protect##"instance::KeyboardEvent::shiftKeySet()"
        SF_UNUSED1(result);
        EvtId.KeysState.SetShiftPressed(value);
//##protect##"instance::KeyboardEvent::shiftKeySet()"
    }
    void KeyboardEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::KeyboardEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::KeyboardEvent::clone()"
    }
    void KeyboardEvent::toString(ASString& result)
    {
//##protect##"instance::KeyboardEvent::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("KeyboardEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("charCode"),
            vm.GetStringManager().CreateConstString("keyCode"),
            vm.GetStringManager().CreateConstString("keyLocation"),
            vm.GetStringManager().CreateConstString("ctrlKey"),
            vm.GetStringManager().CreateConstString("altKey"),
            vm.GetStringManager().CreateConstString("shiftKey")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
        
//##protect##"instance::KeyboardEvent::toString()"
    }
    void KeyboardEvent::updateAfterEvent(const Value& result)
    {
//##protect##"instance::KeyboardEvent::updateAfterEvent()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("KeyboardEvent::updateAfterEvent()");
//##protect##"instance::KeyboardEvent::updateAfterEvent()"
    }

    SPtr<Instances::fl_events::Event> KeyboardEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void KeyboardEvent::SetEventId(const EventId& eid)
    {
        EvtId = eid;
        if (EvtId.AsciiCode == 0)
        {
            EvtId.AsciiCode = EvtId.ConvertKeyCodeToAscii();
        }
    }

    AS3::Object* KeyboardEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).KeyboardEventClass; 
    }

    SPtr<Instances::fl_events::Event> KeyboardEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        KeyboardEvent* pe   = static_cast<KeyboardEvent*>(p.GetPtr());
        pe->EvtId           = EvtId;
        pe->KeyLocation     = KeyLocation;
        return p;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 KeyboardEvent::tito[KeyboardEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 26, 
    // };
    const TypeInfo* KeyboardEvent::tit[27] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
        NULL, 
    };
    const ThunkInfo KeyboardEvent::ti[KeyboardEvent::ThunkInfoNum] = {
        {TFunc_Instances_KeyboardEvent_altKeyGet::Func, &KeyboardEvent::tit[0], "altKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_altKeySet::Func, &KeyboardEvent::tit[1], "altKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_charCodeGet::Func, &KeyboardEvent::tit[3], "charCode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_charCodeSet::Func, &KeyboardEvent::tit[4], "charCode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_commandKeyGet::Func, &KeyboardEvent::tit[6], "commandKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_commandKeySet::Func, &KeyboardEvent::tit[7], "commandKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_controlKeyGet::Func, &KeyboardEvent::tit[9], "controlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_controlKeySet::Func, &KeyboardEvent::tit[10], "controlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_ctrlKeyGet::Func, &KeyboardEvent::tit[12], "ctrlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_ctrlKeySet::Func, &KeyboardEvent::tit[13], "ctrlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_keyCodeGet::Func, &KeyboardEvent::tit[15], "keyCode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_keyCodeSet::Func, &KeyboardEvent::tit[16], "keyCode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_keyLocationGet::Func, &KeyboardEvent::tit[18], "keyLocation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_keyLocationSet::Func, &KeyboardEvent::tit[19], "keyLocation", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_shiftKeyGet::Func, &KeyboardEvent::tit[21], "shiftKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_shiftKeySet::Func, &KeyboardEvent::tit[22], "shiftKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_clone::Func, &KeyboardEvent::tit[24], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_toString::Func, &KeyboardEvent::tit[25], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_KeyboardEvent_updateAfterEvent::Func, &KeyboardEvent::tit[26], "updateAfterEvent", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    KeyboardEvent::KeyboardEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::KeyboardEvent::KeyboardEvent()"
//##protect##"InstanceTraits::KeyboardEvent::KeyboardEvent()"

    }

    void KeyboardEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<KeyboardEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    KeyboardEvent::KeyboardEvent(ClassTraits::Traits& t)
    : Class(t)
    , KEY_DOWN("keyDown")
    , KEY_UP("keyUp")
    {
//##protect##"class_::KeyboardEvent::KeyboardEvent()"
//##protect##"class_::KeyboardEvent::KeyboardEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo KeyboardEvent::mi[KeyboardEvent::MemberInfoNum] = {
        {"KEY_DOWN", NULL, OFFSETOF(Classes::fl_events::KeyboardEvent, KEY_DOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEY_UP", NULL, OFFSETOF(Classes::fl_events::KeyboardEvent, KEY_UP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    KeyboardEvent::KeyboardEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::KeyboardEvent::KeyboardEvent()"
//##protect##"ClassTraits::KeyboardEvent::KeyboardEvent()"

    }

    Pickable<Traits> KeyboardEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) KeyboardEvent(vm, AS3::fl_events::KeyboardEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::KeyboardEventCI));
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
    const TypeInfo KeyboardEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::KeyboardEvent::InstanceType),
        0,
        ClassTraits::fl_events::KeyboardEvent::MemberInfoNum,
        InstanceTraits::fl_events::KeyboardEvent::ThunkInfoNum,
        0,
        "KeyboardEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo KeyboardEventCI = {
        &KeyboardEventTI,
        ClassTraits::fl_events::KeyboardEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::KeyboardEvent::mi,
        InstanceTraits::fl_events::KeyboardEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

