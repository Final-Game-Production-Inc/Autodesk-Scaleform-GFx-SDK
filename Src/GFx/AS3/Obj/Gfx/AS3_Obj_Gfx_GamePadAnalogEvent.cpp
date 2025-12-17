//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_GamePadAnalogEvent.cpp
Content     :   
Created     :   Jun, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_GamePadAnalogEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_codeGet, UInt32> TFunc_Instances_GamePadAnalogEvent_codeGet;
typedef ThunkFunc1<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_codeSet, const Value, UInt32> TFunc_Instances_GamePadAnalogEvent_codeSet;
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_controllerIdxGet, UInt32> TFunc_Instances_GamePadAnalogEvent_controllerIdxGet;
typedef ThunkFunc1<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_controllerIdxSet, const Value, UInt32> TFunc_Instances_GamePadAnalogEvent_controllerIdxSet;
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_xvalueGet, Value::Number> TFunc_Instances_GamePadAnalogEvent_xvalueGet;
typedef ThunkFunc1<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_xvalueSet, const Value, Value::Number> TFunc_Instances_GamePadAnalogEvent_xvalueSet;
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_yvalueGet, Value::Number> TFunc_Instances_GamePadAnalogEvent_yvalueGet;
typedef ThunkFunc1<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_yvalueSet, const Value, Value::Number> TFunc_Instances_GamePadAnalogEvent_yvalueSet;
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_GamePadAnalogEvent_clone;
typedef ThunkFunc0<Instances::fl_gfx::GamePadAnalogEvent, Instances::fl_gfx::GamePadAnalogEvent::mid_toString, ASString> TFunc_Instances_GamePadAnalogEvent_toString;

template <> const TFunc_Instances_GamePadAnalogEvent_codeGet::TMethod TFunc_Instances_GamePadAnalogEvent_codeGet::Method = &Instances::fl_gfx::GamePadAnalogEvent::codeGet;
template <> const TFunc_Instances_GamePadAnalogEvent_codeSet::TMethod TFunc_Instances_GamePadAnalogEvent_codeSet::Method = &Instances::fl_gfx::GamePadAnalogEvent::codeSet;
template <> const TFunc_Instances_GamePadAnalogEvent_controllerIdxGet::TMethod TFunc_Instances_GamePadAnalogEvent_controllerIdxGet::Method = &Instances::fl_gfx::GamePadAnalogEvent::controllerIdxGet;
template <> const TFunc_Instances_GamePadAnalogEvent_controllerIdxSet::TMethod TFunc_Instances_GamePadAnalogEvent_controllerIdxSet::Method = &Instances::fl_gfx::GamePadAnalogEvent::controllerIdxSet;
template <> const TFunc_Instances_GamePadAnalogEvent_xvalueGet::TMethod TFunc_Instances_GamePadAnalogEvent_xvalueGet::Method = &Instances::fl_gfx::GamePadAnalogEvent::xvalueGet;
template <> const TFunc_Instances_GamePadAnalogEvent_xvalueSet::TMethod TFunc_Instances_GamePadAnalogEvent_xvalueSet::Method = &Instances::fl_gfx::GamePadAnalogEvent::xvalueSet;
template <> const TFunc_Instances_GamePadAnalogEvent_yvalueGet::TMethod TFunc_Instances_GamePadAnalogEvent_yvalueGet::Method = &Instances::fl_gfx::GamePadAnalogEvent::yvalueGet;
template <> const TFunc_Instances_GamePadAnalogEvent_yvalueSet::TMethod TFunc_Instances_GamePadAnalogEvent_yvalueSet::Method = &Instances::fl_gfx::GamePadAnalogEvent::yvalueSet;
template <> const TFunc_Instances_GamePadAnalogEvent_clone::TMethod TFunc_Instances_GamePadAnalogEvent_clone::Method = &Instances::fl_gfx::GamePadAnalogEvent::clone;
template <> const TFunc_Instances_GamePadAnalogEvent_toString::TMethod TFunc_Instances_GamePadAnalogEvent_toString::Method = &Instances::fl_gfx::GamePadAnalogEvent::toString;

namespace Instances { namespace fl_gfx
{
    GamePadAnalogEvent::GamePadAnalogEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::GamePadAnalogEvent::GamePadAnalogEvent()$data"
//##protect##"instance::GamePadAnalogEvent::GamePadAnalogEvent()$data"
    {
//##protect##"instance::GamePadAnalogEvent::GamePadAnalogEvent()$code"
//##protect##"instance::GamePadAnalogEvent::GamePadAnalogEvent()$code"
    }

    void GamePadAnalogEvent::codeGet(UInt32& result)
    {
//##protect##"instance::GamePadAnalogEvent::codeGet()"

        result = Code;

//##protect##"instance::GamePadAnalogEvent::codeGet()"
    }
    void GamePadAnalogEvent::codeSet(const Value& result, UInt32 value)
    {
//##protect##"instance::GamePadAnalogEvent::codeSet()"

        SF_UNUSED1(result);
        Code = value;

//##protect##"instance::GamePadAnalogEvent::codeSet()"
    }
    void GamePadAnalogEvent::controllerIdxGet(UInt32& result)
    {
//##protect##"instance::GamePadAnalogEvent::controllerIdxGet()"
        
        result = ControllerIdx;

//##protect##"instance::GamePadAnalogEvent::controllerIdxGet()"
    }
    void GamePadAnalogEvent::controllerIdxSet(const Value& result, UInt32 value)
    {
//##protect##"instance::GamePadAnalogEvent::controllerIdxSet()"
        
        SF_UNUSED1(result);
        ControllerIdx = value;

//##protect##"instance::GamePadAnalogEvent::controllerIdxSet()"
    }
    void GamePadAnalogEvent::xvalueGet(Value::Number& result)
    {
//##protect##"instance::GamePadAnalogEvent::xvalueGet()"
        
        result = XValue;

//##protect##"instance::GamePadAnalogEvent::xvalueGet()"
    }
    void GamePadAnalogEvent::xvalueSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GamePadAnalogEvent::xvalueSet()"
        
        SF_UNUSED1(result);
        XValue = value;

//##protect##"instance::GamePadAnalogEvent::xvalueSet()"
    }
    void GamePadAnalogEvent::yvalueGet(Value::Number& result)
    {
//##protect##"instance::GamePadAnalogEvent::yvalueGet()"
        
        result = YValue;

//##protect##"instance::GamePadAnalogEvent::yvalueGet()"
    }
    void GamePadAnalogEvent::yvalueSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GamePadAnalogEvent::yvalueSet()"
        
        SF_UNUSED1(result);
        YValue = value;

//##protect##"instance::GamePadAnalogEvent::yvalueSet()"
    }
    void GamePadAnalogEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::GamePadAnalogEvent::clone()"

        SF_UNUSED1(result);
        result = Clone().GetPtr();

//##protect##"instance::GamePadAnalogEvent::clone()"
    }
    void GamePadAnalogEvent::toString(ASString& result)
    {
//##protect##"instance::GamePadAnalogEvent::toString()"

        SF_UNUSED1(result);
        //example: [GamePadAnalogEvent type="gamePadAnalogChange" bubbles=true cancelable=false eventPhase=2 
        // code=27 controllerIdx=0 xvalue=0.3 yvalue=-0.2]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("GamePadAnalogEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("code"),
            vm.GetStringManager().CreateConstString("controllerIdx"),
            vm.GetStringManager().CreateConstString("xvalue"),
            vm.GetStringManager().CreateConstString("yvalue")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();

//##protect##"instance::GamePadAnalogEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> GamePadAnalogEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void GamePadAnalogEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            UInt32 v;
            argv[3].Convert2UInt32(v).DoNotCheck();
            Code = v;
        }
        if (argc >= 5)
        {
            UInt32 v;
            argv[4].Convert2UInt32(v).DoNotCheck();
            ControllerIdx = v;
        }
        if (argc >= 6)
        {
            Value::Number v;
            argv[5].Convert2Number(v).DoNotCheck();
            XValue = v;
        }
        if (argc >= 7)
        {
            Value::Number v;
            argv[6].Convert2Number(v).DoNotCheck();
            YValue = v;
        }
    }
    AS3::Object* GamePadAnalogEvent::GetEventClass() const
    { 
#if defined(GFX_ENABLE_ANALOG_GAMEPAD)
        return static_cast<ASVM&>(GetVM()).GamePadAnalogEventClass; 
#else
        return NULL;
#endif
    }

    SPtr<Instances::fl_events::Event> GamePadAnalogEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        GamePadAnalogEvent* pe = static_cast<GamePadAnalogEvent*>(p.GetPtr());
        pe->ControllerIdx   = ControllerIdx;
        pe->XValue          = XValue;
        pe->YValue          = YValue;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    // const UInt16 GamePadAnalogEvent::tito[GamePadAnalogEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 
    // };
    const TypeInfo* GamePadAnalogEvent::tit[14] = {
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo GamePadAnalogEvent::ti[GamePadAnalogEvent::ThunkInfoNum] = {
        {TFunc_Instances_GamePadAnalogEvent_codeGet::Func, &GamePadAnalogEvent::tit[0], "code", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_codeSet::Func, &GamePadAnalogEvent::tit[1], "code", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_controllerIdxGet::Func, &GamePadAnalogEvent::tit[3], "controllerIdx", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_controllerIdxSet::Func, &GamePadAnalogEvent::tit[4], "controllerIdx", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_xvalueGet::Func, &GamePadAnalogEvent::tit[6], "xvalue", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_xvalueSet::Func, &GamePadAnalogEvent::tit[7], "xvalue", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_yvalueGet::Func, &GamePadAnalogEvent::tit[9], "yvalue", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_yvalueSet::Func, &GamePadAnalogEvent::tit[10], "yvalue", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_clone::Func, &GamePadAnalogEvent::tit[12], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GamePadAnalogEvent_toString::Func, &GamePadAnalogEvent::tit[13], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GamePadAnalogEvent::GamePadAnalogEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::GamePadAnalogEvent::GamePadAnalogEvent()"
//##protect##"InstanceTraits::GamePadAnalogEvent::GamePadAnalogEvent()"

    }

    void GamePadAnalogEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GamePadAnalogEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_gfx
{
    GamePadAnalogEvent::GamePadAnalogEvent(ClassTraits::Traits& t)
    : Class(t)
    , CHANGE("gamePadAnalogChange")
    {
//##protect##"class_::GamePadAnalogEvent::GamePadAnalogEvent()"
//##protect##"class_::GamePadAnalogEvent::GamePadAnalogEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo GamePadAnalogEvent::mi[GamePadAnalogEvent::MemberInfoNum] = {
        {"CHANGE", NULL, OFFSETOF(Classes::fl_gfx::GamePadAnalogEvent, CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    GamePadAnalogEvent::GamePadAnalogEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::GamePadAnalogEvent::GamePadAnalogEvent()"
//##protect##"ClassTraits::GamePadAnalogEvent::GamePadAnalogEvent()"

    }

    Pickable<Traits> GamePadAnalogEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GamePadAnalogEvent(vm, AS3::fl_gfx::GamePadAnalogEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::GamePadAnalogEventCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo GamePadAnalogEventTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::GamePadAnalogEvent::InstanceType),
        0,
        ClassTraits::fl_gfx::GamePadAnalogEvent::MemberInfoNum,
        InstanceTraits::fl_gfx::GamePadAnalogEvent::ThunkInfoNum,
        0,
        "GamePadAnalogEvent", "scaleform.gfx", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo GamePadAnalogEventCI = {
        &GamePadAnalogEventTI,
        ClassTraits::fl_gfx::GamePadAnalogEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_gfx::GamePadAnalogEvent::mi,
        InstanceTraits::fl_gfx::GamePadAnalogEvent::ti,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

