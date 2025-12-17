//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_MouseEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_MouseEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_altKeyGet, bool> TFunc_Instances_MouseEvent_altKeyGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_altKeySet, const Value, bool> TFunc_Instances_MouseEvent_altKeySet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_buttonDownGet, bool> TFunc_Instances_MouseEvent_buttonDownGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_buttonDownSet, const Value, bool> TFunc_Instances_MouseEvent_buttonDownSet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_clickCountGet, SInt32> TFunc_Instances_MouseEvent_clickCountGet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_commandKeyGet, bool> TFunc_Instances_MouseEvent_commandKeyGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_commandKeySet, const Value, bool> TFunc_Instances_MouseEvent_commandKeySet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_controlKeyGet, bool> TFunc_Instances_MouseEvent_controlKeyGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_controlKeySet, const Value, bool> TFunc_Instances_MouseEvent_controlKeySet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_ctrlKeyGet, bool> TFunc_Instances_MouseEvent_ctrlKeyGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_ctrlKeySet, const Value, bool> TFunc_Instances_MouseEvent_ctrlKeySet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_deltaGet, SInt32> TFunc_Instances_MouseEvent_deltaGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_deltaSet, const Value, SInt32> TFunc_Instances_MouseEvent_deltaSet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_localXGet, Value::Number> TFunc_Instances_MouseEvent_localXGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_localXSet, const Value, Value::Number> TFunc_Instances_MouseEvent_localXSet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_localYGet, Value::Number> TFunc_Instances_MouseEvent_localYGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_localYSet, const Value, Value::Number> TFunc_Instances_MouseEvent_localYSet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_relatedObjectGet, SPtr<Instances::fl_display::InteractiveObject> > TFunc_Instances_MouseEvent_relatedObjectGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_relatedObjectSet, const Value, Instances::fl_display::InteractiveObject*> TFunc_Instances_MouseEvent_relatedObjectSet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_shiftKeyGet, bool> TFunc_Instances_MouseEvent_shiftKeyGet;
typedef ThunkFunc1<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_shiftKeySet, const Value, bool> TFunc_Instances_MouseEvent_shiftKeySet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_stageXGet, Value::Number> TFunc_Instances_MouseEvent_stageXGet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_stageYGet, Value::Number> TFunc_Instances_MouseEvent_stageYGet;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_MouseEvent_clone;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_toString, ASString> TFunc_Instances_MouseEvent_toString;
typedef ThunkFunc0<Instances::fl_events::MouseEvent, Instances::fl_events::MouseEvent::mid_updateAfterEvent, const Value> TFunc_Instances_MouseEvent_updateAfterEvent;

template <> const TFunc_Instances_MouseEvent_altKeyGet::TMethod TFunc_Instances_MouseEvent_altKeyGet::Method = &Instances::fl_events::MouseEvent::altKeyGet;
template <> const TFunc_Instances_MouseEvent_altKeySet::TMethod TFunc_Instances_MouseEvent_altKeySet::Method = &Instances::fl_events::MouseEvent::altKeySet;
template <> const TFunc_Instances_MouseEvent_buttonDownGet::TMethod TFunc_Instances_MouseEvent_buttonDownGet::Method = &Instances::fl_events::MouseEvent::buttonDownGet;
template <> const TFunc_Instances_MouseEvent_buttonDownSet::TMethod TFunc_Instances_MouseEvent_buttonDownSet::Method = &Instances::fl_events::MouseEvent::buttonDownSet;
template <> const TFunc_Instances_MouseEvent_clickCountGet::TMethod TFunc_Instances_MouseEvent_clickCountGet::Method = &Instances::fl_events::MouseEvent::clickCountGet;
template <> const TFunc_Instances_MouseEvent_commandKeyGet::TMethod TFunc_Instances_MouseEvent_commandKeyGet::Method = &Instances::fl_events::MouseEvent::commandKeyGet;
template <> const TFunc_Instances_MouseEvent_commandKeySet::TMethod TFunc_Instances_MouseEvent_commandKeySet::Method = &Instances::fl_events::MouseEvent::commandKeySet;
template <> const TFunc_Instances_MouseEvent_controlKeyGet::TMethod TFunc_Instances_MouseEvent_controlKeyGet::Method = &Instances::fl_events::MouseEvent::controlKeyGet;
template <> const TFunc_Instances_MouseEvent_controlKeySet::TMethod TFunc_Instances_MouseEvent_controlKeySet::Method = &Instances::fl_events::MouseEvent::controlKeySet;
template <> const TFunc_Instances_MouseEvent_ctrlKeyGet::TMethod TFunc_Instances_MouseEvent_ctrlKeyGet::Method = &Instances::fl_events::MouseEvent::ctrlKeyGet;
template <> const TFunc_Instances_MouseEvent_ctrlKeySet::TMethod TFunc_Instances_MouseEvent_ctrlKeySet::Method = &Instances::fl_events::MouseEvent::ctrlKeySet;
template <> const TFunc_Instances_MouseEvent_deltaGet::TMethod TFunc_Instances_MouseEvent_deltaGet::Method = &Instances::fl_events::MouseEvent::deltaGet;
template <> const TFunc_Instances_MouseEvent_deltaSet::TMethod TFunc_Instances_MouseEvent_deltaSet::Method = &Instances::fl_events::MouseEvent::deltaSet;
template <> const TFunc_Instances_MouseEvent_localXGet::TMethod TFunc_Instances_MouseEvent_localXGet::Method = &Instances::fl_events::MouseEvent::localXGet;
template <> const TFunc_Instances_MouseEvent_localXSet::TMethod TFunc_Instances_MouseEvent_localXSet::Method = &Instances::fl_events::MouseEvent::localXSet;
template <> const TFunc_Instances_MouseEvent_localYGet::TMethod TFunc_Instances_MouseEvent_localYGet::Method = &Instances::fl_events::MouseEvent::localYGet;
template <> const TFunc_Instances_MouseEvent_localYSet::TMethod TFunc_Instances_MouseEvent_localYSet::Method = &Instances::fl_events::MouseEvent::localYSet;
template <> const TFunc_Instances_MouseEvent_relatedObjectGet::TMethod TFunc_Instances_MouseEvent_relatedObjectGet::Method = &Instances::fl_events::MouseEvent::relatedObjectGet;
template <> const TFunc_Instances_MouseEvent_relatedObjectSet::TMethod TFunc_Instances_MouseEvent_relatedObjectSet::Method = &Instances::fl_events::MouseEvent::relatedObjectSet;
template <> const TFunc_Instances_MouseEvent_shiftKeyGet::TMethod TFunc_Instances_MouseEvent_shiftKeyGet::Method = &Instances::fl_events::MouseEvent::shiftKeyGet;
template <> const TFunc_Instances_MouseEvent_shiftKeySet::TMethod TFunc_Instances_MouseEvent_shiftKeySet::Method = &Instances::fl_events::MouseEvent::shiftKeySet;
template <> const TFunc_Instances_MouseEvent_stageXGet::TMethod TFunc_Instances_MouseEvent_stageXGet::Method = &Instances::fl_events::MouseEvent::stageXGet;
template <> const TFunc_Instances_MouseEvent_stageYGet::TMethod TFunc_Instances_MouseEvent_stageYGet::Method = &Instances::fl_events::MouseEvent::stageYGet;
template <> const TFunc_Instances_MouseEvent_clone::TMethod TFunc_Instances_MouseEvent_clone::Method = &Instances::fl_events::MouseEvent::clone;
template <> const TFunc_Instances_MouseEvent_toString::TMethod TFunc_Instances_MouseEvent_toString::Method = &Instances::fl_events::MouseEvent::toString;
template <> const TFunc_Instances_MouseEvent_updateAfterEvent::TMethod TFunc_Instances_MouseEvent_updateAfterEvent::Method = &Instances::fl_events::MouseEvent::updateAfterEvent;

namespace Instances { namespace fl_events
{
    MouseEvent::MouseEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::MouseEvent::MouseEvent()$data"
//##protect##"instance::MouseEvent::MouseEvent()$data"
    {
//##protect##"instance::MouseEvent::MouseEvent()$code"
        AltKey = CtrlKey    = ShiftKey = false;
        Delta               = 0;
        LocalX = LocalY     = 0;
        ButtonsMask         = 0;
//##protect##"instance::MouseEvent::MouseEvent()$code"
    }

    void MouseEvent::altKeyGet(bool& result)
    {
//##protect##"instance::MouseEvent::altKeyGet()"
        SF_UNUSED1(result);
        result = AltKey;
//##protect##"instance::MouseEvent::altKeyGet()"
    }
    void MouseEvent::altKeySet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::altKeySet()"
        SF_UNUSED2(result, value);
        AltKey = value;
//##protect##"instance::MouseEvent::altKeySet()"
    }
    void MouseEvent::buttonDownGet(bool& result)
    {
//##protect##"instance::MouseEvent::buttonDownGet()"
        SF_UNUSED1(result);
        result = (ButtonsMask & Button_Left) != 0;
//##protect##"instance::MouseEvent::buttonDownGet()"
    }
    void MouseEvent::buttonDownSet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::buttonDownSet()"
        SF_UNUSED2(result, value);
        if (value)
            ButtonsMask |= Button_Left;
        else
            ButtonsMask &= ~Button_Left;
//##protect##"instance::MouseEvent::buttonDownSet()"
    }
    void MouseEvent::clickCountGet(SInt32& result)
    {
//##protect##"instance::MouseEvent::clickCountGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("MouseEvent::clickCountGet()");
//##protect##"instance::MouseEvent::clickCountGet()"
    }
    void MouseEvent::commandKeyGet(bool& result)
    {
//##protect##"instance::MouseEvent::commandKeyGet()"
        SF_UNUSED1(result);
        result = CtrlKey;
//##protect##"instance::MouseEvent::commandKeyGet()"
    }
    void MouseEvent::commandKeySet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::commandKeySet()"
        SF_UNUSED2(result, value);
        CtrlKey = value;
//##protect##"instance::MouseEvent::commandKeySet()"
    }
    void MouseEvent::controlKeyGet(bool& result)
    {
//##protect##"instance::MouseEvent::controlKeyGet()"
        SF_UNUSED1(result);
        result = CtrlKey;
//##protect##"instance::MouseEvent::controlKeyGet()"
    }
    void MouseEvent::controlKeySet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::controlKeySet()"
        SF_UNUSED2(result, value);
        CtrlKey = value;
//##protect##"instance::MouseEvent::controlKeySet()"
    }
    void MouseEvent::ctrlKeyGet(bool& result)
    {
//##protect##"instance::MouseEvent::ctrlKeyGet()"
        SF_UNUSED1(result);
        result = CtrlKey;
//##protect##"instance::MouseEvent::ctrlKeyGet()"
    }
    void MouseEvent::ctrlKeySet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::ctrlKeySet()"
        SF_UNUSED2(result, value);
        CtrlKey = value;
//##protect##"instance::MouseEvent::ctrlKeySet()"
    }
    void MouseEvent::deltaGet(SInt32& result)
    {
//##protect##"instance::MouseEvent::deltaGet()"
        SF_UNUSED1(result);
        result = Delta;
//##protect##"instance::MouseEvent::deltaGet()"
    }
    void MouseEvent::deltaSet(const Value& result, SInt32 value)
    {
//##protect##"instance::MouseEvent::deltaSet()"
        SF_UNUSED2(result, value);
        Delta = value;
//##protect##"instance::MouseEvent::deltaSet()"
    }
    void MouseEvent::localXGet(Value::Number& result)
    {
//##protect##"instance::MouseEvent::localXGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(LocalX);
//##protect##"instance::MouseEvent::localXGet()"
    }
    void MouseEvent::localXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::MouseEvent::localXSet()"
        SF_UNUSED2(result, value);
        LocalX = PixelsToTwips(value);
//##protect##"instance::MouseEvent::localXSet()"
    }
    void MouseEvent::localYGet(Value::Number& result)
    {
//##protect##"instance::MouseEvent::localYGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(LocalY);
//##protect##"instance::MouseEvent::localYGet()"
    }
    void MouseEvent::localYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::MouseEvent::localYSet()"
        SF_UNUSED2(result, value);
        LocalY = PixelsToTwips(value);
//##protect##"instance::MouseEvent::localYSet()"
    }
    void MouseEvent::relatedObjectGet(SPtr<Instances::fl_display::InteractiveObject>& result)
    {
//##protect##"instance::MouseEvent::relatedObjectGet()"
        result = RelatedObj;
//##protect##"instance::MouseEvent::relatedObjectGet()"
    }
    void MouseEvent::relatedObjectSet(const Value& result, Instances::fl_display::InteractiveObject* value)
    {
//##protect##"instance::MouseEvent::relatedObjectSet()"
        SF_UNUSED1(result);
        RelatedObj = value;
//##protect##"instance::MouseEvent::relatedObjectSet()"
    }
    void MouseEvent::shiftKeyGet(bool& result)
    {
//##protect##"instance::MouseEvent::shiftKeyGet()"
        SF_UNUSED1(result);
        result = ShiftKey;
//##protect##"instance::MouseEvent::shiftKeyGet()"
    }
    void MouseEvent::shiftKeySet(const Value& result, bool value)
    {
//##protect##"instance::MouseEvent::shiftKeySet()"
        SF_UNUSED2(result, value);
        ShiftKey = value;
//##protect##"instance::MouseEvent::shiftKeySet()"
    }
    void MouseEvent::stageXGet(Value::Number& result)
    {
//##protect##"instance::MouseEvent::stageXGet()"
        SF_UNUSED1(result);
        if (Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_display::DisplayObject* dobj = 
                static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
            SF_ASSERT(dobj->pDispObj);
            Matrix2F m = dobj->pDispObj->GetWorldMatrix();

            PointF p((float)LocalX, (float)LocalY);
            p = m.Transform(p);

            result = TwipsToPixels(p.x);
        }
        else
        {
            result = 0;
        }
//##protect##"instance::MouseEvent::stageXGet()"
    }
    void MouseEvent::stageYGet(Value::Number& result)
    {
//##protect##"instance::MouseEvent::stageYGet()"
        SF_UNUSED1(result);
        if (Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_display::DisplayObject* dobj = 
                static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
            SF_ASSERT(dobj->pDispObj);
            Matrix2F m = dobj->pDispObj->GetWorldMatrix();

            PointF p((float)LocalX, (float)LocalY);
            p = m.Transform(p);

            result = TwipsToPixels(p.y);
        }
        else
        {
            result = 0;
        }
//##protect##"instance::MouseEvent::stageYGet()"
    }
    void MouseEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::MouseEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::MouseEvent::clone()"
    }
    void MouseEvent::toString(ASString& result)
    {
//##protect##"instance::MouseEvent::toString()"
        SF_UNUSED1(result);
        //example: [MouseEvent type="rollOver" bubbles=false cancelable=false eventPhase=2 localX=248 localY=47 
        // stageX=369 stageY=152 relatedObject=null ctrlKey=false altKey=false shiftKey=false delta=0]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("MouseEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("localX"),
            vm.GetStringManager().CreateConstString("localY"),
            vm.GetStringManager().CreateConstString("stageX"),
            vm.GetStringManager().CreateConstString("stageY"),
            vm.GetStringManager().CreateConstString("relatedObject"),
            vm.GetStringManager().CreateConstString("ctrlKey"),
            vm.GetStringManager().CreateConstString("altKey"),
            vm.GetStringManager().CreateConstString("shiftKey"),
            vm.GetStringManager().CreateConstString("delta")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::MouseEvent::toString()"
    }
    void MouseEvent::updateAfterEvent(const Value& result)
    {
//##protect##"instance::MouseEvent::updateAfterEvent()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("MouseEvent::updateAfterEvent()");
//##protect##"instance::MouseEvent::updateAfterEvent()"
    }

    SPtr<Instances::fl_display::InteractiveObject> MouseEvent::relatedObjectGet()
    {
        SPtr<Instances::fl_display::InteractiveObject> result;
        relatedObjectGet(result);
        return result;
    }
    SPtr<Instances::fl_events::Event> MouseEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    // AS3 ctor of Event class
    void MouseEvent::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_events::Event::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<fl_display::InteractiveObject, Mem_Stat>(prcc, RelatedObj, op SF_DEBUG_ARG(*this));
    }
    void MouseEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);

        if (argc < 2)
        {
            // default Bubbles property is true for MouseEvent, unlike for Event
            Bubbles = true;
        }
        if (argc >= 4)
        {
            Value::Number v;
            argv[3].Convert2Number(v).DoNotCheck();
            LocalX = PixelsToTwips(v);
        }
        if (argc >= 5)
        {
            Value::Number v;
            argv[4].Convert2Number(v).DoNotCheck();
            LocalY = PixelsToTwips(v);
        }
        if (argc >= 6)
        {
            RelatedObj = NULL;
            AS3::Object* ptr = argv[5].GetObject();
            if (ptr)
            {
                if (GetVM().IsOfType(argv[5], "flash.display.InteractiveObject", GetVM().GetCurrentAppDomain()))
                {
                    RelatedObj = static_cast<fl_display::InteractiveObject*>(ptr);
                }
            }
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
            if (argv[9].Convert2Boolean())
                ButtonsMask |= Button_Left;
            else
                ButtonsMask &= ~Button_Left;
        }
        if (argc >= 11)
        {
            SInt32 v;
            argv[10].Convert2Int32(v).DoNotCheck();
            Delta = v;
        }
    }
    AS3::Object* MouseEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).MouseEventClass; 
    }

    SPtr<Instances::fl_events::Event> MouseEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        MouseEvent* pe = static_cast<MouseEvent*>(p.GetPtr());
        pe->AltKey          = AltKey;
        pe->CtrlKey         = CtrlKey;
        pe->ShiftKey        = ShiftKey;
        pe->Delta           = Delta;
        pe->RelatedObj      = RelatedObj;
        pe->LocalX          = LocalX;
        pe->LocalY          = LocalY;
        pe->ButtonsMask     = ButtonsMask;
        return p;
    }

    void MouseEvent::SetStageCoords(const PointF& p)
    {
        if (Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
        {
            Instances::fl_display::DisplayObject* dobj = 
                static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
            SF_ASSERT(dobj->pDispObj);
            Matrix2F m = dobj->pDispObj->GetWorldMatrix();
     
            PointF loc = m.TransformByInverse(p);
            LocalX = loc.x;
            LocalY = loc.y;
        }
        else
        {
            LocalX = LocalY = 0;
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 MouseEvent::tito[MouseEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23, 25, 26, 28, 29, 31, 32, 33, 34, 35, 
    // };
    const TypeInfo* MouseEvent::tit[36] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
        NULL, 
    };
    const ThunkInfo MouseEvent::ti[MouseEvent::ThunkInfoNum] = {
        {TFunc_Instances_MouseEvent_altKeyGet::Func, &MouseEvent::tit[0], "altKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_altKeySet::Func, &MouseEvent::tit[1], "altKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_buttonDownGet::Func, &MouseEvent::tit[3], "buttonDown", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_buttonDownSet::Func, &MouseEvent::tit[4], "buttonDown", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_clickCountGet::Func, &MouseEvent::tit[6], "clickCount", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_commandKeyGet::Func, &MouseEvent::tit[7], "commandKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_commandKeySet::Func, &MouseEvent::tit[8], "commandKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_controlKeyGet::Func, &MouseEvent::tit[10], "controlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_controlKeySet::Func, &MouseEvent::tit[11], "controlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_ctrlKeyGet::Func, &MouseEvent::tit[13], "ctrlKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_ctrlKeySet::Func, &MouseEvent::tit[14], "ctrlKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_deltaGet::Func, &MouseEvent::tit[16], "delta", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_deltaSet::Func, &MouseEvent::tit[17], "delta", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_localXGet::Func, &MouseEvent::tit[19], "localX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_localXSet::Func, &MouseEvent::tit[20], "localX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_localYGet::Func, &MouseEvent::tit[22], "localY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_localYSet::Func, &MouseEvent::tit[23], "localY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_relatedObjectGet::Func, &MouseEvent::tit[25], "relatedObject", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_relatedObjectSet::Func, &MouseEvent::tit[26], "relatedObject", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_shiftKeyGet::Func, &MouseEvent::tit[28], "shiftKey", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_shiftKeySet::Func, &MouseEvent::tit[29], "shiftKey", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_stageXGet::Func, &MouseEvent::tit[31], "stageX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_stageYGet::Func, &MouseEvent::tit[32], "stageY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_clone::Func, &MouseEvent::tit[33], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_toString::Func, &MouseEvent::tit[34], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_MouseEvent_updateAfterEvent::Func, &MouseEvent::tit[35], "updateAfterEvent", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    MouseEvent::MouseEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::MouseEvent::MouseEvent()"
//##protect##"InstanceTraits::MouseEvent::MouseEvent()"

    }

    void MouseEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<MouseEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    MouseEvent::MouseEvent(ClassTraits::Traits& t)
    : Class(t)
    , CLICK("click")
    , CONTEXT_MENU("contextMenu")
    , DOUBLE_CLICK("doubleClick")
    , MIDDLE_CLICK("middleClick")
    , MIDDLE_MOUSE_DOWN("middleMouseDown")
    , MIDDLE_MOUSE_UP("middleMouseUp")
    , MOUSE_DOWN("mouseDown")
    , MOUSE_MOVE("mouseMove")
    , MOUSE_OUT("mouseOut")
    , MOUSE_OVER("mouseOver")
    , MOUSE_UP("mouseUp")
    , MOUSE_WHEEL("mouseWheel")
    , RIGHT_CLICK("rightClick")
    , RIGHT_MOUSE_DOWN("rightMouseDown")
    , RIGHT_MOUSE_UP("rightMouseUp")
    , ROLL_OUT("rollOut")
    , ROLL_OVER("rollOver")
    {
//##protect##"class_::MouseEvent::MouseEvent()"
//##protect##"class_::MouseEvent::MouseEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo MouseEvent::mi[MouseEvent::MemberInfoNum] = {
        {"CLICK", NULL, OFFSETOF(Classes::fl_events::MouseEvent, CLICK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CONTEXT_MENU", NULL, OFFSETOF(Classes::fl_events::MouseEvent, CONTEXT_MENU), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DOUBLE_CLICK", NULL, OFFSETOF(Classes::fl_events::MouseEvent, DOUBLE_CLICK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MIDDLE_CLICK", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MIDDLE_CLICK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MIDDLE_MOUSE_DOWN", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MIDDLE_MOUSE_DOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MIDDLE_MOUSE_UP", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MIDDLE_MOUSE_UP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_DOWN", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_DOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_MOVE", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_MOVE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_OUT", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_OUT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_OVER", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_OVER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_UP", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_UP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_WHEEL", NULL, OFFSETOF(Classes::fl_events::MouseEvent, MOUSE_WHEEL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT_CLICK", NULL, OFFSETOF(Classes::fl_events::MouseEvent, RIGHT_CLICK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT_MOUSE_DOWN", NULL, OFFSETOF(Classes::fl_events::MouseEvent, RIGHT_MOUSE_DOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT_MOUSE_UP", NULL, OFFSETOF(Classes::fl_events::MouseEvent, RIGHT_MOUSE_UP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ROLL_OUT", NULL, OFFSETOF(Classes::fl_events::MouseEvent, ROLL_OUT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ROLL_OVER", NULL, OFFSETOF(Classes::fl_events::MouseEvent, ROLL_OVER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    MouseEvent::MouseEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::MouseEvent::MouseEvent()"
//##protect##"ClassTraits::MouseEvent::MouseEvent()"

    }

    Pickable<Traits> MouseEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MouseEvent(vm, AS3::fl_events::MouseEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::MouseEventCI));
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
    const TypeInfo MouseEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::MouseEvent::InstanceType),
        0,
        ClassTraits::fl_events::MouseEvent::MemberInfoNum,
        InstanceTraits::fl_events::MouseEvent::ThunkInfoNum,
        0,
        "MouseEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo MouseEventCI = {
        &MouseEventTI,
        ClassTraits::fl_events::MouseEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::MouseEvent::mi,
        InstanceTraits::fl_events::MouseEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

