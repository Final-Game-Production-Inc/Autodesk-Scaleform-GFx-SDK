//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_FocusEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_FocusEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_directionGet, ASString> TFunc_Instances_FocusEvent_directionGet;
typedef ThunkFunc1<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_directionSet, const Value, const ASString&> TFunc_Instances_FocusEvent_directionSet;
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_keyCodeGet, UInt32> TFunc_Instances_FocusEvent_keyCodeGet;
typedef ThunkFunc1<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_keyCodeSet, const Value, UInt32> TFunc_Instances_FocusEvent_keyCodeSet;
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_relatedObjectGet, SPtr<Instances::fl_display::InteractiveObject> > TFunc_Instances_FocusEvent_relatedObjectGet;
typedef ThunkFunc1<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_relatedObjectSet, const Value, Instances::fl_display::InteractiveObject*> TFunc_Instances_FocusEvent_relatedObjectSet;
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_shiftKeyGet, bool> TFunc_Instances_FocusEvent_shiftKeyGet;
typedef ThunkFunc1<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_shiftKeySet, const Value, bool> TFunc_Instances_FocusEvent_shiftKeySet;
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_FocusEvent_clone;
typedef ThunkFunc0<Instances::fl_events::FocusEvent, Instances::fl_events::FocusEvent::mid_toString, ASString> TFunc_Instances_FocusEvent_toString;

template <> const TFunc_Instances_FocusEvent_directionGet::TMethod TFunc_Instances_FocusEvent_directionGet::Method = &Instances::fl_events::FocusEvent::directionGet;
template <> const TFunc_Instances_FocusEvent_directionSet::TMethod TFunc_Instances_FocusEvent_directionSet::Method = &Instances::fl_events::FocusEvent::directionSet;
template <> const TFunc_Instances_FocusEvent_keyCodeGet::TMethod TFunc_Instances_FocusEvent_keyCodeGet::Method = &Instances::fl_events::FocusEvent::keyCodeGet;
template <> const TFunc_Instances_FocusEvent_keyCodeSet::TMethod TFunc_Instances_FocusEvent_keyCodeSet::Method = &Instances::fl_events::FocusEvent::keyCodeSet;
template <> const TFunc_Instances_FocusEvent_relatedObjectGet::TMethod TFunc_Instances_FocusEvent_relatedObjectGet::Method = &Instances::fl_events::FocusEvent::relatedObjectGet;
template <> const TFunc_Instances_FocusEvent_relatedObjectSet::TMethod TFunc_Instances_FocusEvent_relatedObjectSet::Method = &Instances::fl_events::FocusEvent::relatedObjectSet;
template <> const TFunc_Instances_FocusEvent_shiftKeyGet::TMethod TFunc_Instances_FocusEvent_shiftKeyGet::Method = &Instances::fl_events::FocusEvent::shiftKeyGet;
template <> const TFunc_Instances_FocusEvent_shiftKeySet::TMethod TFunc_Instances_FocusEvent_shiftKeySet::Method = &Instances::fl_events::FocusEvent::shiftKeySet;
template <> const TFunc_Instances_FocusEvent_clone::TMethod TFunc_Instances_FocusEvent_clone::Method = &Instances::fl_events::FocusEvent::clone;
template <> const TFunc_Instances_FocusEvent_toString::TMethod TFunc_Instances_FocusEvent_toString::Method = &Instances::fl_events::FocusEvent::toString;

namespace Instances { namespace fl_events
{
    FocusEvent::FocusEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::FocusEvent::FocusEvent()$data"
    , ShiftKey(false)
    , KeyCode(0)
//##protect##"instance::FocusEvent::FocusEvent()$data"
    {
//##protect##"instance::FocusEvent::FocusEvent()$code"
//##protect##"instance::FocusEvent::FocusEvent()$code"
    }

    void FocusEvent::directionGet(ASString& result)
    {
//##protect##"instance::FocusEvent::directionGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("FocusEvent::directionGet()");
//##protect##"instance::FocusEvent::directionGet()"
    }
    void FocusEvent::directionSet(const Value& result, const ASString& value)
    {
//##protect##"instance::FocusEvent::directionSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("FocusEvent::directionSet()");
//##protect##"instance::FocusEvent::directionSet()"
    }
    void FocusEvent::keyCodeGet(UInt32& result)
    {
//##protect##"instance::FocusEvent::keyCodeGet()"
        SF_UNUSED1(result);
        result = KeyCode;
//##protect##"instance::FocusEvent::keyCodeGet()"
    }
    void FocusEvent::keyCodeSet(const Value& result, UInt32 value)
    {
//##protect##"instance::FocusEvent::keyCodeSet()"
        SF_UNUSED2(result, value);
        KeyCode = value;
//##protect##"instance::FocusEvent::keyCodeSet()"
    }
    void FocusEvent::relatedObjectGet(SPtr<Instances::fl_display::InteractiveObject>& result)
    {
//##protect##"instance::FocusEvent::relatedObjectGet()"
        SF_UNUSED1(result);
        result = RelatedObj;
//##protect##"instance::FocusEvent::relatedObjectGet()"
    }
    void FocusEvent::relatedObjectSet(const Value& result, Instances::fl_display::InteractiveObject* value)
    {
//##protect##"instance::FocusEvent::relatedObjectSet()"
        SF_UNUSED2(result, value);
        RelatedObj = value;
//##protect##"instance::FocusEvent::relatedObjectSet()"
    }
    void FocusEvent::shiftKeyGet(bool& result)
    {
//##protect##"instance::FocusEvent::shiftKeyGet()"
        SF_UNUSED1(result);
        result = ShiftKey;
//##protect##"instance::FocusEvent::shiftKeyGet()"
    }
    void FocusEvent::shiftKeySet(const Value& result, bool value)
    {
//##protect##"instance::FocusEvent::shiftKeySet()"
        SF_UNUSED2(result, value);
        ShiftKey = value;
//##protect##"instance::FocusEvent::shiftKeySet()"
    }
    void FocusEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::FocusEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::FocusEvent::clone()"
    }
    void FocusEvent::toString(ASString& result)
    {
//##protect##"instance::FocusEvent::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("FocusEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("relatedObject"),
            vm.GetStringManager().CreateConstString("shiftKey"),
            vm.GetStringManager().CreateConstString("keyCode")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::FocusEvent::toString()"
    }

    SPtr<Instances::fl_display::InteractiveObject> FocusEvent::relatedObjectGet()
    {
        SPtr<Instances::fl_display::InteractiveObject> result;
        relatedObjectGet(result);
        return result;
    }
    SPtr<Instances::fl_events::Event> FocusEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void FocusEvent::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_events::Event::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<fl_display::InteractiveObject, Mem_Stat>(prcc, RelatedObj, op SF_DEBUG_ARG(*this));
    }
    AS3::Object* FocusEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).FocusEventClass; 
    }

    SPtr<Instances::fl_events::Event> FocusEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        FocusEvent* pe = static_cast<FocusEvent*>(p.GetPtr());
        pe->RelatedObj = RelatedObj;
        pe->ShiftKey   = ShiftKey;
        pe->KeyCode    = KeyCode;
        return p;
    }
    void FocusEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            RelatedObj = NULL;
            AS3::Object* ptr = argv[3].GetObject();
            if (ptr)
            {
                if (GetVM().IsOfType(argv[3], "flash.display.InteractiveObject", GetVM().GetCurrentAppDomain()))
                {
                    RelatedObj = static_cast<fl_display::InteractiveObject*>(ptr);
                }
            }
        }
        if (argc >= 5)
        {
            ShiftKey = argv[4].Convert2Boolean();
        }
        if (argc >= 6)
        {
            UInt32 v;
            argv[5].Convert2UInt32(v).DoNotCheck();
            KeyCode = v;
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 FocusEvent::tito[FocusEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 
    // };
    const TypeInfo* FocusEvent::tit[14] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo FocusEvent::ti[FocusEvent::ThunkInfoNum] = {
        {TFunc_Instances_FocusEvent_directionGet::Func, &FocusEvent::tit[0], "direction", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_directionSet::Func, &FocusEvent::tit[1], "direction", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_keyCodeGet::Func, &FocusEvent::tit[3], "keyCode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_keyCodeSet::Func, &FocusEvent::tit[4], "keyCode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_relatedObjectGet::Func, &FocusEvent::tit[6], "relatedObject", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_relatedObjectSet::Func, &FocusEvent::tit[7], "relatedObject", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_shiftKeyGet::Func, &FocusEvent::tit[9], "shiftKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_shiftKeySet::Func, &FocusEvent::tit[10], "shiftKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_clone::Func, &FocusEvent::tit[12], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_FocusEvent_toString::Func, &FocusEvent::tit[13], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    FocusEvent::FocusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::FocusEvent::FocusEvent()"
//##protect##"InstanceTraits::FocusEvent::FocusEvent()"

    }

    void FocusEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<FocusEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    FocusEvent::FocusEvent(ClassTraits::Traits& t)
    : Class(t)
    , FOCUS_IN("focusIn")
    , FOCUS_OUT("focusOut")
    , KEY_FOCUS_CHANGE("keyFocusChange")
    , MOUSE_FOCUS_CHANGE("mouseFocusChange")
    {
//##protect##"class_::FocusEvent::FocusEvent()"
//##protect##"class_::FocusEvent::FocusEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo FocusEvent::mi[FocusEvent::MemberInfoNum] = {
        {"FOCUS_IN", NULL, OFFSETOF(Classes::fl_events::FocusEvent, FOCUS_IN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"FOCUS_OUT", NULL, OFFSETOF(Classes::fl_events::FocusEvent, FOCUS_OUT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEY_FOCUS_CHANGE", NULL, OFFSETOF(Classes::fl_events::FocusEvent, KEY_FOCUS_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_FOCUS_CHANGE", NULL, OFFSETOF(Classes::fl_events::FocusEvent, MOUSE_FOCUS_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    FocusEvent::FocusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::FocusEvent::FocusEvent()"
//##protect##"ClassTraits::FocusEvent::FocusEvent()"

    }

    Pickable<Traits> FocusEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FocusEvent(vm, AS3::fl_events::FocusEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::FocusEventCI));
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
    const TypeInfo FocusEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::FocusEvent::InstanceType),
        0,
        ClassTraits::fl_events::FocusEvent::MemberInfoNum,
        InstanceTraits::fl_events::FocusEvent::ThunkInfoNum,
        0,
        "FocusEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo FocusEventCI = {
        &FocusEventTI,
        ClassTraits::fl_events::FocusEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::FocusEvent::mi,
        InstanceTraits::fl_events::FocusEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

