//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_PressAndTapGestureEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_PressAndTapGestureEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapLocalXGet, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapLocalXGet;
typedef ThunkFunc1<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapLocalXSet, const Value, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapLocalXSet;
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapLocalYGet, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapLocalYGet;
typedef ThunkFunc1<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapLocalYSet, const Value, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapLocalYSet;
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapStageXGet, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapStageXGet;
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_tapStageYGet, Value::Number> TFunc_Instances_PressAndTapGestureEvent_tapStageYGet;
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_PressAndTapGestureEvent_clone;
typedef ThunkFunc0<Instances::fl_events::PressAndTapGestureEvent, Instances::fl_events::PressAndTapGestureEvent::mid_toString, ASString> TFunc_Instances_PressAndTapGestureEvent_toString;

template <> const TFunc_Instances_PressAndTapGestureEvent_tapLocalXGet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapLocalXGet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapLocalXGet;
template <> const TFunc_Instances_PressAndTapGestureEvent_tapLocalXSet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapLocalXSet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapLocalXSet;
template <> const TFunc_Instances_PressAndTapGestureEvent_tapLocalYGet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapLocalYGet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapLocalYGet;
template <> const TFunc_Instances_PressAndTapGestureEvent_tapLocalYSet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapLocalYSet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapLocalYSet;
template <> const TFunc_Instances_PressAndTapGestureEvent_tapStageXGet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapStageXGet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapStageXGet;
template <> const TFunc_Instances_PressAndTapGestureEvent_tapStageYGet::TMethod TFunc_Instances_PressAndTapGestureEvent_tapStageYGet::Method = &Instances::fl_events::PressAndTapGestureEvent::tapStageYGet;
template <> const TFunc_Instances_PressAndTapGestureEvent_clone::TMethod TFunc_Instances_PressAndTapGestureEvent_clone::Method = &Instances::fl_events::PressAndTapGestureEvent::clone;
template <> const TFunc_Instances_PressAndTapGestureEvent_toString::TMethod TFunc_Instances_PressAndTapGestureEvent_toString::Method = &Instances::fl_events::PressAndTapGestureEvent::toString;

namespace Instances { namespace fl_events
{
    PressAndTapGestureEvent::PressAndTapGestureEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::GestureEvent(t)
//##protect##"instance::PressAndTapGestureEvent::PressAndTapGestureEvent()$data"
//##protect##"instance::PressAndTapGestureEvent::PressAndTapGestureEvent()$data"
    {
//##protect##"instance::PressAndTapGestureEvent::PressAndTapGestureEvent()$code"
        TapLocalX = TapLocalY = 0;
        TapStageX = TapStageY = 0;
        LocalInitialized = false;
		TapStageSet = false;
//##protect##"instance::PressAndTapGestureEvent::PressAndTapGestureEvent()$code"
    }

    void PressAndTapGestureEvent::tapLocalXGet(Value::Number& result)
    {
//##protect##"instance::PressAndTapGestureEvent::tapLocalXGet()"
        SF_UNUSED1(result);
        InitLocalCoords();
        result = TwipsToPixels(TapLocalX);
//##protect##"instance::PressAndTapGestureEvent::tapLocalXGet()"
    }
    void PressAndTapGestureEvent::tapLocalXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::PressAndTapGestureEvent::tapLocalXSet()"
        SF_UNUSED2(result, value);
        InitLocalCoords();
        TapLocalX = PixelsToTwips(value);
//##protect##"instance::PressAndTapGestureEvent::tapLocalXSet()"
    }
    void PressAndTapGestureEvent::tapLocalYGet(Value::Number& result)
    {
//##protect##"instance::PressAndTapGestureEvent::tapLocalYGet()"
        SF_UNUSED1(result);
        InitLocalCoords();
        result = TwipsToPixels(TapLocalY);
//##protect##"instance::PressAndTapGestureEvent::tapLocalYGet()"
    }
    void PressAndTapGestureEvent::tapLocalYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::PressAndTapGestureEvent::tapLocalYSet()"
        SF_UNUSED2(result, value);
        InitLocalCoords();
        TapLocalY = PixelsToTwips(value);
//##protect##"instance::PressAndTapGestureEvent::tapLocalYSet()"
    }
    void PressAndTapGestureEvent::tapStageXGet(Value::Number& result)
    {
//##protect##"instance::PressAndTapGestureEvent::tapStageXGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(TapStageX);
//##protect##"instance::PressAndTapGestureEvent::tapStageXGet()"
    }
    void PressAndTapGestureEvent::tapStageYGet(Value::Number& result)
    {
//##protect##"instance::PressAndTapGestureEvent::tapStageYGet()"
        SF_UNUSED1(result);
        result = TwipsToPixels(TapStageY);
//##protect##"instance::PressAndTapGestureEvent::tapStageYGet()"
    }
    void PressAndTapGestureEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::PressAndTapGestureEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::PressAndTapGestureEvent::clone()"
    }
    void PressAndTapGestureEvent::toString(ASString& result)
    {
//##protect##"instance::PressAndTapGestureEvent::toString()"
        SF_UNUSED1(result);
        //example: [PressAndTapGestureEvent type="gesturePressAndTap" bubbles=true cancelable=false phase=null 
        // localX=0 localY=0 tapLocalX=0 tapLocalY=0 ctrlKey=false altKey=false shiftKey=false 
        // commandKey=false controlKey=false]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("PressAndTapGestureEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("phase"),
            vm.GetStringManager().CreateConstString("localX"),
            vm.GetStringManager().CreateConstString("localY"),
            vm.GetStringManager().CreateConstString("tapLocalX"),
            vm.GetStringManager().CreateConstString("tapLocalY"),
            vm.GetStringManager().CreateConstString("ctrlKey"),
            vm.GetStringManager().CreateConstString("altKey"),
            vm.GetStringManager().CreateConstString("shiftKey"),
            vm.GetStringManager().CreateConstString("commandKey"),
            vm.GetStringManager().CreateConstString("controlKey")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::PressAndTapGestureEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> PressAndTapGestureEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void PressAndTapGestureEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            Value r;
            phaseSet(r, argv[3]);
        }
        if (argc >= 5)
        {
            Value r;
            localXSet(r, Value::Number(argv[4]));
        }
        if (argc >= 6)
        {
            Value r;
            localYSet(r, Value::Number(argv[5]));
        }
        if (argc >= 7)
        {
            Value r;
            tapLocalXSet(r, Value::Number(argv[6]));
        }
        if (argc >= 8)
        {
            Value r;
            tapLocalYSet(r, Value::Number(argv[7]));
        }
        if (argc >= 9)
        {
            CtrlKey = argv[8].Convert2Boolean();
        }
        if (argc >= 10)
        {
            AltKey = argv[9].Convert2Boolean();
        }
        if (argc >= 11)
        {
            ShiftKey = argv[10].Convert2Boolean();
        }
        if (argc >= 12)
        {
            CommandKey = argv[11].Convert2Boolean();
        }
        if (argc >= 13)
        {
            ControlKey = argv[12].Convert2Boolean();
        }
    }
    AS3::Object* PressAndTapGestureEvent::GetEventClass() const 
    { 
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
        return static_cast<ASVM&>(GetVM()).PressAndTapGestureEventClass; 
#else
        return NULL;
#endif
    }

    SPtr<Instances::fl_events::Event> PressAndTapGestureEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = GestureEvent::Clone();
        PressAndTapGestureEvent* pe = static_cast<PressAndTapGestureEvent*>(p.GetPtr());
        pe->TapLocalX       = TapLocalX;
        pe->TapLocalY       = TapLocalY;
        pe->TapStageX       = TapStageX;
        pe->TapStageY       = TapStageY;
        pe->LocalInitialized= LocalInitialized;
		pe->TapStageSet     = TapStageSet;
        return p;
    }

    void PressAndTapGestureEvent::InitLocalCoords()
    {
        if (!LocalInitialized)
        {
            if (TapStageSet && Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                Instances::fl_display::DisplayObject* dobj = static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
                SF_ASSERT(dobj->pDispObj);
                Matrix2F m = dobj->pDispObj->GetWorldMatrix();

                PointF p((float)TapStageX, (float)TapStageY);
                p = m.TransformByInverse(p);
                TapLocalX = p.x;
                TapLocalY = p.y;
            }
            else
            {
                TapLocalX = TapLocalY = 0;
            }
            LocalInitialized = true;
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 PressAndTapGestureEvent::tito[PressAndTapGestureEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 8, 9, 
    // };
    const TypeInfo* PressAndTapGestureEvent::tit[10] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo PressAndTapGestureEvent::ti[PressAndTapGestureEvent::ThunkInfoNum] = {
        {TFunc_Instances_PressAndTapGestureEvent_tapLocalXGet::Func, &PressAndTapGestureEvent::tit[0], "tapLocalX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_tapLocalXSet::Func, &PressAndTapGestureEvent::tit[1], "tapLocalX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_tapLocalYGet::Func, &PressAndTapGestureEvent::tit[3], "tapLocalY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_tapLocalYSet::Func, &PressAndTapGestureEvent::tit[4], "tapLocalY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_tapStageXGet::Func, &PressAndTapGestureEvent::tit[6], "tapStageX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_tapStageYGet::Func, &PressAndTapGestureEvent::tit[7], "tapStageY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_clone::Func, &PressAndTapGestureEvent::tit[8], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_PressAndTapGestureEvent_toString::Func, &PressAndTapGestureEvent::tit[9], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    PressAndTapGestureEvent::PressAndTapGestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::GestureEvent(vm, ci)
    {
//##protect##"InstanceTraits::PressAndTapGestureEvent::PressAndTapGestureEvent()"
//##protect##"InstanceTraits::PressAndTapGestureEvent::PressAndTapGestureEvent()"

    }

    void PressAndTapGestureEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<PressAndTapGestureEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    PressAndTapGestureEvent::PressAndTapGestureEvent(ClassTraits::Traits& t)
    : Class(t)
    , GESTURE_PRESS_AND_TAP("gesturePressAndTap")
    {
//##protect##"class_::PressAndTapGestureEvent::PressAndTapGestureEvent()"
//##protect##"class_::PressAndTapGestureEvent::PressAndTapGestureEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo PressAndTapGestureEvent::mi[PressAndTapGestureEvent::MemberInfoNum] = {
        {"GESTURE_PRESS_AND_TAP", NULL, OFFSETOF(Classes::fl_events::PressAndTapGestureEvent, GESTURE_PRESS_AND_TAP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    PressAndTapGestureEvent::PressAndTapGestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::GestureEvent(vm, ci)
    {
//##protect##"ClassTraits::PressAndTapGestureEvent::PressAndTapGestureEvent()"
//##protect##"ClassTraits::PressAndTapGestureEvent::PressAndTapGestureEvent()"

    }

    Pickable<Traits> PressAndTapGestureEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) PressAndTapGestureEvent(vm, AS3::fl_events::PressAndTapGestureEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::PressAndTapGestureEventCI));
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
    const TypeInfo PressAndTapGestureEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::PressAndTapGestureEvent::InstanceType),
        0,
        ClassTraits::fl_events::PressAndTapGestureEvent::MemberInfoNum,
        InstanceTraits::fl_events::PressAndTapGestureEvent::ThunkInfoNum,
        0,
        "PressAndTapGestureEvent", "flash.events", &fl_events::GestureEventTI,
        TypeInfo::None
    };

    const ClassInfo PressAndTapGestureEventCI = {
        &PressAndTapGestureEventTI,
        ClassTraits::fl_events::PressAndTapGestureEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::PressAndTapGestureEvent::mi,
        InstanceTraits::fl_events::PressAndTapGestureEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

