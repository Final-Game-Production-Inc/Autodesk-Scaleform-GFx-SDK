//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_TransformGestureEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_TransformGestureEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_offsetXGet, Value::Number> TFunc_Instances_TransformGestureEvent_offsetXGet;
typedef ThunkFunc1<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_offsetXSet, const Value, Value::Number> TFunc_Instances_TransformGestureEvent_offsetXSet;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_offsetYGet, Value::Number> TFunc_Instances_TransformGestureEvent_offsetYGet;
typedef ThunkFunc1<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_offsetYSet, const Value, Value::Number> TFunc_Instances_TransformGestureEvent_offsetYSet;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_rotationGet, Value::Number> TFunc_Instances_TransformGestureEvent_rotationGet;
typedef ThunkFunc1<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_rotationSet, const Value, Value::Number> TFunc_Instances_TransformGestureEvent_rotationSet;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_scaleXGet, Value::Number> TFunc_Instances_TransformGestureEvent_scaleXGet;
typedef ThunkFunc1<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_scaleXSet, const Value, Value::Number> TFunc_Instances_TransformGestureEvent_scaleXSet;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_scaleYGet, Value::Number> TFunc_Instances_TransformGestureEvent_scaleYGet;
typedef ThunkFunc1<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_scaleYSet, const Value, Value::Number> TFunc_Instances_TransformGestureEvent_scaleYSet;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_TransformGestureEvent_clone;
typedef ThunkFunc0<Instances::fl_events::TransformGestureEvent, Instances::fl_events::TransformGestureEvent::mid_toString, ASString> TFunc_Instances_TransformGestureEvent_toString;

template <> const TFunc_Instances_TransformGestureEvent_offsetXGet::TMethod TFunc_Instances_TransformGestureEvent_offsetXGet::Method = &Instances::fl_events::TransformGestureEvent::offsetXGet;
template <> const TFunc_Instances_TransformGestureEvent_offsetXSet::TMethod TFunc_Instances_TransformGestureEvent_offsetXSet::Method = &Instances::fl_events::TransformGestureEvent::offsetXSet;
template <> const TFunc_Instances_TransformGestureEvent_offsetYGet::TMethod TFunc_Instances_TransformGestureEvent_offsetYGet::Method = &Instances::fl_events::TransformGestureEvent::offsetYGet;
template <> const TFunc_Instances_TransformGestureEvent_offsetYSet::TMethod TFunc_Instances_TransformGestureEvent_offsetYSet::Method = &Instances::fl_events::TransformGestureEvent::offsetYSet;
template <> const TFunc_Instances_TransformGestureEvent_rotationGet::TMethod TFunc_Instances_TransformGestureEvent_rotationGet::Method = &Instances::fl_events::TransformGestureEvent::rotationGet;
template <> const TFunc_Instances_TransformGestureEvent_rotationSet::TMethod TFunc_Instances_TransformGestureEvent_rotationSet::Method = &Instances::fl_events::TransformGestureEvent::rotationSet;
template <> const TFunc_Instances_TransformGestureEvent_scaleXGet::TMethod TFunc_Instances_TransformGestureEvent_scaleXGet::Method = &Instances::fl_events::TransformGestureEvent::scaleXGet;
template <> const TFunc_Instances_TransformGestureEvent_scaleXSet::TMethod TFunc_Instances_TransformGestureEvent_scaleXSet::Method = &Instances::fl_events::TransformGestureEvent::scaleXSet;
template <> const TFunc_Instances_TransformGestureEvent_scaleYGet::TMethod TFunc_Instances_TransformGestureEvent_scaleYGet::Method = &Instances::fl_events::TransformGestureEvent::scaleYGet;
template <> const TFunc_Instances_TransformGestureEvent_scaleYSet::TMethod TFunc_Instances_TransformGestureEvent_scaleYSet::Method = &Instances::fl_events::TransformGestureEvent::scaleYSet;
template <> const TFunc_Instances_TransformGestureEvent_clone::TMethod TFunc_Instances_TransformGestureEvent_clone::Method = &Instances::fl_events::TransformGestureEvent::clone;
template <> const TFunc_Instances_TransformGestureEvent_toString::TMethod TFunc_Instances_TransformGestureEvent_toString::Method = &Instances::fl_events::TransformGestureEvent::toString;

namespace Instances { namespace fl_events
{
    TransformGestureEvent::TransformGestureEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::GestureEvent(t)
//##protect##"instance::TransformGestureEvent::TransformGestureEvent()$data"
//##protect##"instance::TransformGestureEvent::TransformGestureEvent()$data"
    {
//##protect##"instance::TransformGestureEvent::TransformGestureEvent()$code"
        OffsetX = OffsetY = LocalOffsetX = LocalOffsetY = 0;
        ScaleX = ScaleY   = 1.0;
        Rotation          = 0;
		LocalInitialized  = false;
//##protect##"instance::TransformGestureEvent::TransformGestureEvent()$code"
    }

    void TransformGestureEvent::offsetXGet(Value::Number& result)
    {
//##protect##"instance::TransformGestureEvent::offsetXGet()"
        SF_UNUSED1(result);
		InitLocalCoords();
        result = TwipsToPixels(LocalOffsetX);
//##protect##"instance::TransformGestureEvent::offsetXGet()"
    }
    void TransformGestureEvent::offsetXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TransformGestureEvent::offsetXSet()"
        SF_UNUSED2(result, value);
		InitLocalCoords();
        LocalOffsetX = PixelsToTwips(value);
//##protect##"instance::TransformGestureEvent::offsetXSet()"
    }
    void TransformGestureEvent::offsetYGet(Value::Number& result)
    {
//##protect##"instance::TransformGestureEvent::offsetYGet()"
        SF_UNUSED1(result);
		InitLocalCoords();
        result = TwipsToPixels(LocalOffsetY);
//##protect##"instance::TransformGestureEvent::offsetYGet()"
    }
    void TransformGestureEvent::offsetYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TransformGestureEvent::offsetYSet()"
        SF_UNUSED2(result, value);
		InitLocalCoords();
        LocalOffsetY = PixelsToTwips(value);
//##protect##"instance::TransformGestureEvent::offsetYSet()"
    }
    void TransformGestureEvent::rotationGet(Value::Number& result)
    {
//##protect##"instance::TransformGestureEvent::rotationGet()"
        SF_UNUSED1(result);
        result = Rotation;
//##protect##"instance::TransformGestureEvent::rotationGet()"
    }
    void TransformGestureEvent::rotationSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TransformGestureEvent::rotationSet()"
        SF_UNUSED2(result, value);
        Rotation = value;
//##protect##"instance::TransformGestureEvent::rotationSet()"
    }
    void TransformGestureEvent::scaleXGet(Value::Number& result)
    {
//##protect##"instance::TransformGestureEvent::scaleXGet()"
        SF_UNUSED1(result);
        result = ScaleX;
//##protect##"instance::TransformGestureEvent::scaleXGet()"
    }
    void TransformGestureEvent::scaleXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TransformGestureEvent::scaleXSet()"
        SF_UNUSED2(result, value);
        ScaleX = value;
//##protect##"instance::TransformGestureEvent::scaleXSet()"
    }
    void TransformGestureEvent::scaleYGet(Value::Number& result)
    {
//##protect##"instance::TransformGestureEvent::scaleYGet()"
        SF_UNUSED1(result);
        result = ScaleY;
//##protect##"instance::TransformGestureEvent::scaleYGet()"
    }
    void TransformGestureEvent::scaleYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::TransformGestureEvent::scaleYSet()"
        SF_UNUSED2(result, value);
        ScaleY = value;
//##protect##"instance::TransformGestureEvent::scaleYSet()"
    }
    void TransformGestureEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::TransformGestureEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::TransformGestureEvent::clone()"
    }
    void TransformGestureEvent::toString(ASString& result)
    {
//##protect##"instance::TransformGestureEvent::toString()"
        SF_UNUSED1(result);
        //example: [TransformGestureEvent type="gestureZoom" bubbles=true cancelable=false phase="end" localX=-16.2 localY=35.1 
        // stageX=153.75 stageY=143.6 scaleX=1 scaleY=1 rotation=0 offsetX=0 offsetY=0 ctrlKey=false 
        // altKey=false shiftKey=false commandKey=false controlKey=false]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("TransformGestureEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("phase"),
            vm.GetStringManager().CreateConstString("localX"),
            vm.GetStringManager().CreateConstString("localY"),
            vm.GetStringManager().CreateConstString("stageX"),
            vm.GetStringManager().CreateConstString("stageY"),
			vm.GetStringManager().CreateConstString("scaleX"),
			vm.GetStringManager().CreateConstString("scaleY"),
            vm.GetStringManager().CreateConstString("rotation"),
            vm.GetStringManager().CreateConstString("offsetX"),
            vm.GetStringManager().CreateConstString("offsetY"),
            vm.GetStringManager().CreateConstString("ctrlKey"),
            vm.GetStringManager().CreateConstString("altKey"),
            vm.GetStringManager().CreateConstString("shiftKey"),
            vm.GetStringManager().CreateConstString("commandKey"),
            vm.GetStringManager().CreateConstString("controlKey")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::TransformGestureEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> TransformGestureEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void TransformGestureEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        GestureEvent::AS3Constructor(argc, argv);
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
            Value::Number v;
            argv[6].Convert2Number(v).DoNotCheck();
            ScaleX = v;
        }
        if (argc >= 8)
        {
            Value::Number v;
            argv[7].Convert2Number(v).DoNotCheck();
            ScaleY = v;
        }
        if (argc >= 9)
        {
            Value::Number v;
            argv[8].Convert2Number(v).DoNotCheck();
            Rotation = v;
        }
        if (argc >= 10)
        {
            Value::Number v;
            argv[9].Convert2Number(v).DoNotCheck();
            OffsetX = PixelsToTwips(v);
        }
        if (argc >= 11)
        {
            Value::Number v;
            argv[10].Convert2Number(v).DoNotCheck();
            OffsetY = PixelsToTwips(v);
        }
        if (argc >= 12)
        {
            CtrlKey = argv[11].Convert2Boolean();
        }
        if (argc >= 13)
        {
            AltKey = argv[12].Convert2Boolean();
        }
        if (argc >= 14)
        {
            ShiftKey = argv[13].Convert2Boolean();
        }
        if (argc >= 15)
        {
            CommandKey = argv[14].Convert2Boolean();
        }
        if (argc >= 16)
        {
            ControlKey = argv[15].Convert2Boolean();
        }
    }
    AS3::Object* TransformGestureEvent::GetEventClass() const 
    { 
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
		return static_cast<ASVM&>(GetVM()).TransformGestureEventClass; 
#else
		return NULL;
#endif
    }

    SPtr<Instances::fl_events::Event> TransformGestureEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = GestureEvent::Clone();
        TransformGestureEvent* pe = static_cast<TransformGestureEvent*>(p.GetPtr());
        pe->OffsetX         = OffsetX;
        pe->OffsetY         = OffsetY;
        pe->ScaleX          = ScaleX;
        pe->ScaleY          = ScaleY;
        pe->Rotation        = Rotation;
        return p;
    }

	void TransformGestureEvent::InitLocalCoords()
	{
		if (!LocalInitialized)
		{
			if (Target && GetVM().IsOfType(Value(Target), "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
			{
				Instances::fl_display::DisplayObject* dobj = static_cast<Instances::fl_display::DisplayObject*>(Target.GetPtr());
				SF_ASSERT(dobj->pDispObj);
				Matrix2F m;
				if (dobj->pDispObj->GetParent())
					m = dobj->pDispObj->GetParent()->GetWorldMatrix();

				PointF p ((float)StageX, (float)StageY);
				p  = m.TransformByInverse(p);
				PointF p1((float)(StageX + OffsetX), (float)(StageY + OffsetY));
				p1 = m.TransformByInverse(p1);

				LocalOffsetX = p1.x - p.x;
				LocalOffsetY = p1.y - p.y;
			}
			else
			{
				LocalOffsetX = LocalOffsetY = 0;
			}
			LocalInitialized = true;
		}
	}
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 TransformGestureEvent::tito[TransformGestureEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 
    // };
    const TypeInfo* TransformGestureEvent::tit[17] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo TransformGestureEvent::ti[TransformGestureEvent::ThunkInfoNum] = {
        {TFunc_Instances_TransformGestureEvent_offsetXGet::Func, &TransformGestureEvent::tit[0], "offsetX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_offsetXSet::Func, &TransformGestureEvent::tit[1], "offsetX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_offsetYGet::Func, &TransformGestureEvent::tit[3], "offsetY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_offsetYSet::Func, &TransformGestureEvent::tit[4], "offsetY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_rotationGet::Func, &TransformGestureEvent::tit[6], "rotation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_rotationSet::Func, &TransformGestureEvent::tit[7], "rotation", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_scaleXGet::Func, &TransformGestureEvent::tit[9], "scaleX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_scaleXSet::Func, &TransformGestureEvent::tit[10], "scaleX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_scaleYGet::Func, &TransformGestureEvent::tit[12], "scaleY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_scaleYSet::Func, &TransformGestureEvent::tit[13], "scaleY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_clone::Func, &TransformGestureEvent::tit[15], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_TransformGestureEvent_toString::Func, &TransformGestureEvent::tit[16], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    TransformGestureEvent::TransformGestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::GestureEvent(vm, ci)
    {
//##protect##"InstanceTraits::TransformGestureEvent::TransformGestureEvent()"
//##protect##"InstanceTraits::TransformGestureEvent::TransformGestureEvent()"

    }

    void TransformGestureEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<TransformGestureEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    TransformGestureEvent::TransformGestureEvent(ClassTraits::Traits& t)
    : Class(t)
    , GESTURE_PAN("gesturePan")
    , GESTURE_ROTATE("gestureRotate")
    , GESTURE_SWIPE("gestureSwipe")
    , GESTURE_ZOOM("gestureZoom")
    {
//##protect##"class_::TransformGestureEvent::TransformGestureEvent()"
//##protect##"class_::TransformGestureEvent::TransformGestureEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo TransformGestureEvent::mi[TransformGestureEvent::MemberInfoNum] = {
        {"GESTURE_PAN", NULL, OFFSETOF(Classes::fl_events::TransformGestureEvent, GESTURE_PAN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"GESTURE_ROTATE", NULL, OFFSETOF(Classes::fl_events::TransformGestureEvent, GESTURE_ROTATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"GESTURE_SWIPE", NULL, OFFSETOF(Classes::fl_events::TransformGestureEvent, GESTURE_SWIPE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"GESTURE_ZOOM", NULL, OFFSETOF(Classes::fl_events::TransformGestureEvent, GESTURE_ZOOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    TransformGestureEvent::TransformGestureEvent(VM& vm, const ClassInfo& ci)
    : fl_events::GestureEvent(vm, ci)
    {
//##protect##"ClassTraits::TransformGestureEvent::TransformGestureEvent()"
//##protect##"ClassTraits::TransformGestureEvent::TransformGestureEvent()"

    }

    Pickable<Traits> TransformGestureEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TransformGestureEvent(vm, AS3::fl_events::TransformGestureEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::TransformGestureEventCI));
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
    const TypeInfo TransformGestureEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::TransformGestureEvent::InstanceType),
        0,
        ClassTraits::fl_events::TransformGestureEvent::MemberInfoNum,
        InstanceTraits::fl_events::TransformGestureEvent::ThunkInfoNum,
        0,
        "TransformGestureEvent", "flash.events", &fl_events::GestureEventTI,
        TypeInfo::None
    };

    const ClassInfo TransformGestureEventCI = {
        &TransformGestureEventTI,
        ClassTraits::fl_events::TransformGestureEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::TransformGestureEvent::mi,
        InstanceTraits::fl_events::TransformGestureEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

