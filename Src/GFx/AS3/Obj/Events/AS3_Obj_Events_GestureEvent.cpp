//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_GestureEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_GestureEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_altKeyGet, bool> TFunc_Instances_GestureEvent_altKeyGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_altKeySet, const Value, bool> TFunc_Instances_GestureEvent_altKeySet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_commandKeyGet, bool> TFunc_Instances_GestureEvent_commandKeyGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_commandKeySet, const Value, bool> TFunc_Instances_GestureEvent_commandKeySet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_controlKeyGet, bool> TFunc_Instances_GestureEvent_controlKeyGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_controlKeySet, const Value, bool> TFunc_Instances_GestureEvent_controlKeySet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_ctrlKeyGet, bool> TFunc_Instances_GestureEvent_ctrlKeyGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_ctrlKeySet, const Value, bool> TFunc_Instances_GestureEvent_ctrlKeySet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_localXGet, Value::Number> TFunc_Instances_GestureEvent_localXGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_localXSet, const Value, Value::Number> TFunc_Instances_GestureEvent_localXSet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_localYGet, Value::Number> TFunc_Instances_GestureEvent_localYGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_localYSet, const Value, Value::Number> TFunc_Instances_GestureEvent_localYSet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_phaseGet, ASString> TFunc_Instances_GestureEvent_phaseGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_phaseSet, const Value, const ASString&> TFunc_Instances_GestureEvent_phaseSet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_shiftKeyGet, bool> TFunc_Instances_GestureEvent_shiftKeyGet;
typedef ThunkFunc1<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_shiftKeySet, const Value, bool> TFunc_Instances_GestureEvent_shiftKeySet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_stageXGet, Value::Number> TFunc_Instances_GestureEvent_stageXGet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_stageYGet, Value::Number> TFunc_Instances_GestureEvent_stageYGet;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_GestureEvent_clone;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_toString, ASString> TFunc_Instances_GestureEvent_toString;
typedef ThunkFunc0<Instances::fl_events::GestureEvent, Instances::fl_events::GestureEvent::mid_updateAfterEvent, const Value> TFunc_Instances_GestureEvent_updateAfterEvent;

template <> const TFunc_Instances_GestureEvent_altKeyGet::TMethod TFunc_Instances_GestureEvent_altKeyGet::Method = &Instances::fl_events::GestureEvent::altKeyGet;
template <> const TFunc_Instances_GestureEvent_altKeySet::TMethod TFunc_Instances_GestureEvent_altKeySet::Method = &Instances::fl_events::GestureEvent::altKeySet;
template <> const TFunc_Instances_GestureEvent_commandKeyGet::TMethod TFunc_Instances_GestureEvent_commandKeyGet::Method = &Instances::fl_events::GestureEvent::commandKeyGet;
template <> const TFunc_Instances_GestureEvent_commandKeySet::TMethod TFunc_Instances_GestureEvent_commandKeySet::Method = &Instances::fl_events::GestureEvent::commandKeySet;
template <> const TFunc_Instances_GestureEvent_controlKeyGet::TMethod TFunc_Instances_GestureEvent_controlKeyGet::Method = &Instances::fl_events::GestureEvent::controlKeyGet;
template <> const TFunc_Instances_GestureEvent_controlKeySet::TMethod TFunc_Instances_GestureEvent_controlKeySet::Method = &Instances::fl_events::GestureEvent::controlKeySet;
template <> const TFunc_Instances_GestureEvent_ctrlKeyGet::TMethod TFunc_Instances_GestureEvent_ctrlKeyGet::Method = &Instances::fl_events::GestureEvent::ctrlKeyGet;
template <> const TFunc_Instances_GestureEvent_ctrlKeySet::TMethod TFunc_Instances_GestureEvent_ctrlKeySet::Method = &Instances::fl_events::GestureEvent::ctrlKeySet;
template <> const TFunc_Instances_GestureEvent_localXGet::TMethod TFunc_Instances_GestureEvent_localXGet::Method = &Instances::fl_events::GestureEvent::localXGet;
template <> const TFunc_Instances_GestureEvent_localXSet::TMethod TFunc_Instances_GestureEvent_localXSet::Method = &Instances::fl_events::GestureEvent::localXSet;
template <> const TFunc_Instances_GestureEvent_localYGet::TMethod TFunc_Instances_GestureEvent_localYGet::Method = &Instances::fl_events::GestureEvent::localYGet;
template <> const TFunc_Instances_GestureEvent_localYSet::TMethod TFunc_Instances_GestureEvent_localYSet::Method = &Instances::fl_events::GestureEvent::localYSet;
template <> const TFunc_Instances_GestureEvent_phaseGet::TMethod TFunc_Instances_GestureEvent_phaseGet::Method = &Instances::fl_events::GestureEvent::phaseGet;
template <> const TFunc_Instances_GestureEvent_phaseSet::TMethod TFunc_Instances_GestureEvent_phaseSet::Method = &Instances::fl_events::GestureEvent::phaseSet;
template <> const TFunc_Instances_GestureEvent_shiftKeyGet::TMethod TFunc_Instances_GestureEvent_shiftKeyGet::Method = &Instances::fl_events::GestureEvent::shiftKeyGet;
template <> const TFunc_Instances_GestureEvent_shiftKeySet::TMethod TFunc_Instances_GestureEvent_shiftKeySet::Method = &Instances::fl_events::GestureEvent::shiftKeySet;
template <> const TFunc_Instances_GestureEvent_stageXGet::TMethod TFunc_Instances_GestureEvent_stageXGet::Method = &Instances::fl_events::GestureEvent::stageXGet;
template <> const TFunc_Instances_GestureEvent_stageYGet::TMethod TFunc_Instances_GestureEvent_stageYGet::Method = &Instances::fl_events::GestureEvent::stageYGet;
template <> const TFunc_Instances_GestureEvent_clone::TMethod TFunc_Instances_GestureEvent_clone::Method = &Instances::fl_events::GestureEvent::clone;
template <> const TFunc_Instances_GestureEvent_toString::TMethod TFunc_Instances_GestureEvent_toString::Method = &Instances::fl_events::GestureEvent::toString;
template <> const TFunc_Instances_GestureEvent_updateAfterEvent::TMethod TFunc_Instances_GestureEvent_updateAfterEvent::Method = &Instances::fl_events::GestureEvent::updateAfterEvent;

namespace Instances { namespace fl_events
{
    GestureEvent::GestureEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::GestureEvent::GestureEvent()$data"
//##protect##"instance::GestureEvent::GestureEvent()$data"
    {
//##protect##"instance::GestureEvent::GestureEvent()$code"
        AltKey = CtrlKey    = ShiftKey = CommandKey = ControlKey = false;
        LocalX = LocalY     = 0;
        StageX = StageY     = 0;
        Phase               = Phase_All;
        LocalInitialized    = false;
//##protect##"instance::GestureEvent::GestureEvent()$code"
    }

    void GestureEvent::altKeyGet(bool& result)
    {
//##protect##"instance::GestureEvent::altKeyGet()"
        SF_UNUSED1(result);
        result = AltKey;
//##protect##"instance::GestureEvent::altKeyGet()"
    }
    void GestureEvent::altKeySet(const Value& result, bool value)
    {
//##protect##"instance::GestureEvent::altKeySet()"
        SF_UNUSED2(result, value);
        AltKey = value;
//##protect##"instance::GestureEvent::altKeySet()"
    }
    void GestureEvent::commandKeyGet(bool& result)
    {
//##protect##"instance::GestureEvent::commandKeyGet()"
        SF_UNUSED1(result);
        result = CommandKey;
//##protect##"instance::GestureEvent::commandKeyGet()"
    }
    void GestureEvent::commandKeySet(const Value& result, bool value)
    {
//##protect##"instance::GestureEvent::commandKeySet()"
        SF_UNUSED2(result, value);
        CommandKey = value;
//##protect##"instance::GestureEvent::commandKeySet()"
    }
    void GestureEvent::controlKeyGet(bool& result)
    {
//##protect##"instance::GestureEvent::controlKeyGet()"
        SF_UNUSED1(result);
        result = ControlKey;
//##protect##"instance::GestureEvent::controlKeyGet()"
    }
    void GestureEvent::controlKeySet(const Value& result, bool value)
    {
//##protect##"instance::GestureEvent::controlKeySet()"
        SF_UNUSED2(result, value);
        ControlKey = value;
//##protect##"instance::GestureEvent::controlKeySet()"
    }
    void GestureEvent::ctrlKeyGet(bool& result)
    {
//##protect##"instance::GestureEvent::ctrlKeyGet()"
        SF_UNUSED1(result);
        result = CtrlKey;
//##protect##"instance::GestureEvent::ctrlKeyGet()"
    }
    void GestureEvent::ctrlKeySet(const Value& result, bool value)
    {
//##protect##"instance::GestureEvent::ctrlKeySet()"
        SF_UNUSED2(result, value);
        CtrlKey = value;
//##protect##"instance::GestureEvent::ctrlKeySet()"
    }
    void GestureEvent::localXGet(Value::Number& result)
    {
//##protect##"instance::GestureEvent::localXGet()"
        SF_UNUSED1(result);
        InitLocalCoords();
        result = TwipsToPixels(LocalX);
//##protect##"instance::GestureEvent::localXGet()"
    }
    void GestureEvent::localXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GestureEvent::localXSet()"
        SF_UNUSED2(result, value);
        InitLocalCoords();
        LocalX = PixelsToTwips(value);
//##protect##"instance::GestureEvent::localXSet()"
    }
    void GestureEvent::localYGet(Value::Number& result)
    {
//##protect##"instance::GestureEvent::localYGet()"
        SF_UNUSED1(result);
        InitLocalCoords();
        result = TwipsToPixels(LocalY);
//##protect##"instance::GestureEvent::localYGet()"
    }
    void GestureEvent::localYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GestureEvent::localYSet()"
        SF_UNUSED2(result, value);
        InitLocalCoords();
        LocalY = PixelsToTwips(value);
//##protect##"instance::GestureEvent::localYSet()"
    }
    void GestureEvent::phaseGet(ASString& result)
    {
//##protect##"instance::GestureEvent::phaseGet()"
        SF_UNUSED1(result);
        static const char* phases[] = { 0, "all", "begin", "end", "update" };
        SF_ASSERT(sizeof(phases)/sizeof(phases[0]) == Phase_Count);
        ASVM& vm = static_cast<ASVM&>(GetVM());
        if (phases[Phase])
            result = vm.GetStringManager().CreateConstString(phases[Phase]);
        else
            result.SetNull();
//##protect##"instance::GestureEvent::phaseGet()"
    }
    void GestureEvent::phaseSet(const Value& result, const ASString& value)
    {
//##protect##"instance::GestureEvent::phaseSet()"
        SF_UNUSED1(result);
        if (value.IsNull())
            Phase = Phase_Null;
        else if (value == "all")
            Phase = Phase_All;
        else if (value == "begin")
            Phase = Phase_Begin;
        else if (value == "end")
            Phase = Phase_End;
        else if (value == "update")
            Phase = Phase_Update;
        else
            Phase = Phase_Null;
//##protect##"instance::GestureEvent::phaseSet()"
    }
    void GestureEvent::shiftKeyGet(bool& result)
    {
//##protect##"instance::GestureEvent::shiftKeyGet()"
        SF_UNUSED1(result);
        result = ShiftKey;
//##protect##"instance::GestureEvent::shiftKeyGet()"
    }
    void GestureEvent::shiftKeySet(const Value& result, bool value)
    {
//##protect##"instance::GestureEvent::shiftKeySet()"
        SF_UNUSED2(result, value);
        ShiftKey = value;
//##protect##"instance::GestureEvent::shiftKeySet()"
    }
    void GestureEvent::stageXGet(Value::Number& result)
    {
//##protect##"instance::GestureEvent::stageXGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(StageX);
//##protect##"instance::GestureEvent::stageXGet()"
    }
    void GestureEvent::stageYGet(Value::Number& result)
    {
//##protect##"instance::GestureEvent::stageYGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(StageY);
//##protect##"instance::GestureEvent::stageYGet()"
    }
    void GestureEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::GestureEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::GestureEvent::clone()"
    }
    void GestureEvent::toString(ASString& result)
    {
//##protect##"instance::GestureEvent::toString()"
        SF_UNUSED1(result);
        //example: [GestureEvent type="gestureZoom" bubbles=true cancelable=false phase=null localX=0 localY=0 
        // stageX=0 stageY=0 ctrlKey=false altKey=false shiftKey=false commandKey=false controlKey=false]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("GestureEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("phase"),
            vm.GetStringManager().CreateConstString("localX"),
            vm.GetStringManager().CreateConstString("localY"),
            vm.GetStringManager().CreateConstString("stageX"),
            vm.GetStringManager().CreateConstString("stageY"),
            vm.GetStringManager().CreateConstString("ctrlKey"),
            vm.GetStringManager().CreateConstString("altKey"),
            vm.GetStringManager().CreateConstString("shiftKey"),
            vm.GetStringManager().CreateConstString("commandKey"),
            vm.GetStringManager().CreateConstString("controlKey")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::GestureEvent::toString()"
    }
    void GestureEvent::updateAfterEvent(const Value& result)
    {
//##protect##"instance::GestureEvent::updateAfterEvent()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::GestureEvent::updateAfterEvent()");
//##protect##"instance::GestureEvent::updateAfterEvent()"
    }

    SPtr<Instances::fl_events::Event> GestureEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void GestureEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            Value r;
            phaseSet(r, argv[3]);
        }
        if (argc >= 5)
        {
            Value::Number v;
            argv[4].Convert2Number(v).DoNotCheck();
            LocalX = PixelsToTwips(v);
            LocalInitialized = true;
        }
        if (argc >= 6)
        {
            Value::Number v;
            argv[5].Convert2Number(v).DoNotCheck();
            LocalY = PixelsToTwips(v);
            LocalInitialized = true;
        }
        if (argc >= 7)
        {
            CtrlKey = argv[6].Convert2Boolean();
        }
        if (argc >= 8)
        {
            AltKey = argv[7].Convert2Boolean();
        }
        if (argc >= 9)
        {
            ShiftKey = argv[8].Convert2Boolean();
        }
        if (argc >= 10)
        {
            CommandKey = argv[9].Convert2Boolean();
        }
        if (argc >= 11)
        {
            ControlKey = argv[10].Convert2Boolean();
        }
    }
    AS3::Object* GestureEvent::GetEventClass() const 
    { 
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
		return static_cast<ASVM&>(GetVM()).GestureEventClass; 
#else
		return NULL;
#endif
    }

    SPtr<Instances::fl_events::Event> GestureEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        GestureEvent* pe = static_cast<GestureEvent*>(p.GetPtr());
        pe->AltKey          = AltKey;
        pe->CtrlKey         = CtrlKey;
        pe->ShiftKey        = ShiftKey;
        pe->CommandKey      = CommandKey;
        pe->ControlKey      = ControlKey;
        pe->LocalX          = LocalX;
        pe->LocalY          = LocalY;
        pe->StageX          = StageX;
        pe->StageY          = StageY;
        pe->Phase           = Phase;
        pe->LocalInitialized= LocalInitialized;
        return p;
    }

    void GestureEvent::InitLocalCoords()
    {
        if (!LocalInitialized)
        {
            if (Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                Instances::fl_display::DisplayObject* dobj = static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
                SF_ASSERT(dobj->pDispObj);
                Matrix2F m = dobj->pDispObj->GetWorldMatrix();

                PointF p((float)StageX, (float)StageY);
                p = m.TransformByInverse(p);
                LocalX = p.x;
                LocalY = p.y;
            }
            else
            {
                LocalX = LocalY = 0;
            }
            LocalInitialized = true;
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 GestureEvent::tito[GestureEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 26, 27, 28, 
    // };
    const TypeInfo* GestureEvent::tit[29] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
        NULL, 
    };
    const ThunkInfo GestureEvent::ti[GestureEvent::ThunkInfoNum] = {
        {TFunc_Instances_GestureEvent_altKeyGet::Func, &GestureEvent::tit[0], "altKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_altKeySet::Func, &GestureEvent::tit[1], "altKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_commandKeyGet::Func, &GestureEvent::tit[3], "commandKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_commandKeySet::Func, &GestureEvent::tit[4], "commandKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_controlKeyGet::Func, &GestureEvent::tit[6], "controlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_controlKeySet::Func, &GestureEvent::tit[7], "controlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_ctrlKeyGet::Func, &GestureEvent::tit[9], "ctrlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_ctrlKeySet::Func, &GestureEvent::tit[10], "ctrlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_localXGet::Func, &GestureEvent::tit[12], "localX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_localXSet::Func, &GestureEvent::tit[13], "localX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_localYGet::Func, &GestureEvent::tit[15], "localY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_localYSet::Func, &GestureEvent::tit[16], "localY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_phaseGet::Func, &GestureEvent::tit[18], "phase", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_phaseSet::Func, &GestureEvent::tit[19], "phase", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_shiftKeyGet::Func, &GestureEvent::tit[21], "shiftKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_shiftKeySet::Func, &GestureEvent::tit[22], "shiftKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_stageXGet::Func, &GestureEvent::tit[24], "stageX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_stageYGet::Func, &GestureEvent::tit[25], "stageY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_clone::Func, &GestureEvent::tit[26], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_toString::Func, &GestureEvent::tit[27], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GestureEvent_updateAfterEvent::Func, &GestureEvent::tit[28], "updateAfterEvent", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GestureEvent::GestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::GestureEvent::GestureEvent()"
//##protect##"InstanceTraits::GestureEvent::GestureEvent()"

    }

    void GestureEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GestureEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    GestureEvent::GestureEvent(ClassTraits::Traits& t)
    : Class(t)
    , GESTURE_TWO_FINGER_TAP("gestureTwoFingerTap")
    {
//##protect##"class_::GestureEvent::GestureEvent()"
//##protect##"class_::GestureEvent::GestureEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo GestureEvent::mi[GestureEvent::MemberInfoNum] = {
        {"GESTURE_TWO_FINGER_TAP", NULL, OFFSETOF(Classes::fl_events::GestureEvent, GESTURE_TWO_FINGER_TAP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    GestureEvent::GestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::GestureEvent::GestureEvent()"
//##protect##"ClassTraits::GestureEvent::GestureEvent()"

    }

    Pickable<Traits> GestureEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GestureEvent(vm, AS3::fl_events::GestureEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::GestureEventCI));
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
    const TypeInfo GestureEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::GestureEvent::InstanceType),
        0,
        ClassTraits::fl_events::GestureEvent::MemberInfoNum,
        InstanceTraits::fl_events::GestureEvent::ThunkInfoNum,
        0,
        "GestureEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo GestureEventCI = {
        &GestureEventTI,
        ClassTraits::fl_events::GestureEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::GestureEvent::mi,
        InstanceTraits::fl_events::GestureEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

