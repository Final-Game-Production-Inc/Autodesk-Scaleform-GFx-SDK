//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Jan, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_AccelerometerEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationXGet, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationXGet;
typedef ThunkFunc1<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationXSet, const Value, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationXSet;
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationYGet, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationYGet;
typedef ThunkFunc1<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationYSet, const Value, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationYSet;
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationZGet, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationZGet;
typedef ThunkFunc1<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_accelerationZSet, const Value, Value::Number> TFunc_Instances_AccelerometerEvent_accelerationZSet;
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_timestampGet, Value::Number> TFunc_Instances_AccelerometerEvent_timestampGet;
typedef ThunkFunc1<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_timestampSet, const Value, Value::Number> TFunc_Instances_AccelerometerEvent_timestampSet;
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_AccelerometerEvent_clone;
typedef ThunkFunc0<Instances::fl_events::AccelerometerEvent, Instances::fl_events::AccelerometerEvent::mid_toString, ASString> TFunc_Instances_AccelerometerEvent_toString;

template <> const TFunc_Instances_AccelerometerEvent_accelerationXGet::TMethod TFunc_Instances_AccelerometerEvent_accelerationXGet::Method = &Instances::fl_events::AccelerometerEvent::accelerationXGet;
template <> const TFunc_Instances_AccelerometerEvent_accelerationXSet::TMethod TFunc_Instances_AccelerometerEvent_accelerationXSet::Method = &Instances::fl_events::AccelerometerEvent::accelerationXSet;
template <> const TFunc_Instances_AccelerometerEvent_accelerationYGet::TMethod TFunc_Instances_AccelerometerEvent_accelerationYGet::Method = &Instances::fl_events::AccelerometerEvent::accelerationYGet;
template <> const TFunc_Instances_AccelerometerEvent_accelerationYSet::TMethod TFunc_Instances_AccelerometerEvent_accelerationYSet::Method = &Instances::fl_events::AccelerometerEvent::accelerationYSet;
template <> const TFunc_Instances_AccelerometerEvent_accelerationZGet::TMethod TFunc_Instances_AccelerometerEvent_accelerationZGet::Method = &Instances::fl_events::AccelerometerEvent::accelerationZGet;
template <> const TFunc_Instances_AccelerometerEvent_accelerationZSet::TMethod TFunc_Instances_AccelerometerEvent_accelerationZSet::Method = &Instances::fl_events::AccelerometerEvent::accelerationZSet;
template <> const TFunc_Instances_AccelerometerEvent_timestampGet::TMethod TFunc_Instances_AccelerometerEvent_timestampGet::Method = &Instances::fl_events::AccelerometerEvent::timestampGet;
template <> const TFunc_Instances_AccelerometerEvent_timestampSet::TMethod TFunc_Instances_AccelerometerEvent_timestampSet::Method = &Instances::fl_events::AccelerometerEvent::timestampSet;
template <> const TFunc_Instances_AccelerometerEvent_clone::TMethod TFunc_Instances_AccelerometerEvent_clone::Method = &Instances::fl_events::AccelerometerEvent::clone;
template <> const TFunc_Instances_AccelerometerEvent_toString::TMethod TFunc_Instances_AccelerometerEvent_toString::Method = &Instances::fl_events::AccelerometerEvent::toString;

namespace Instances { namespace fl_events
{
    AccelerometerEvent::AccelerometerEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::AccelerometerEvent::AccelerometerEvent()$data"
//##protect##"instance::AccelerometerEvent::AccelerometerEvent()$data"
    {
//##protect##"instance::AccelerometerEvent::AccelerometerEvent()$code"
//##protect##"instance::AccelerometerEvent::AccelerometerEvent()$code"
    }

    void AccelerometerEvent::accelerationXGet(Value::Number& result)
    {
//##protect##"instance::AccelerometerEvent::accelerationXGet()"
        SF_UNUSED1(result);
		result = accelerationX;
//##protect##"instance::AccelerometerEvent::accelerationXGet()"
    }
    void AccelerometerEvent::accelerationXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::AccelerometerEvent::accelerationXSet()"
        SF_UNUSED2(result, value);
        accelerationX = value;
//##protect##"instance::AccelerometerEvent::accelerationXSet()"
    }
    void AccelerometerEvent::accelerationYGet(Value::Number& result)
    {
//##protect##"instance::AccelerometerEvent::accelerationYGet()"
        SF_UNUSED1(result);
        result = accelerationY;
//##protect##"instance::AccelerometerEvent::accelerationYGet()"
    }
    void AccelerometerEvent::accelerationYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::AccelerometerEvent::accelerationYSet()"
        SF_UNUSED2(result, value);
        accelerationY = value;
//##protect##"instance::AccelerometerEvent::accelerationYSet()"
    }
    void AccelerometerEvent::accelerationZGet(Value::Number& result)
    {
//##protect##"instance::AccelerometerEvent::accelerationZGet()"
        SF_UNUSED1(result);
        result = accelerationZ;
//##protect##"instance::AccelerometerEvent::accelerationZGet()"
    }
    void AccelerometerEvent::accelerationZSet(const Value& result, Value::Number value)
    {
//##protect##"instance::AccelerometerEvent::accelerationZSet()"
        SF_UNUSED2(result, value);
        accelerationZ = value;
//##protect##"instance::AccelerometerEvent::accelerationZSet()"
    }
    void AccelerometerEvent::timestampGet(Value::Number& result)
    {
//##protect##"instance::AccelerometerEvent::timestampGet()"
        SF_UNUSED1(result);
        result = timestamp;
//##protect##"instance::AccelerometerEvent::timestampGet()"
    }
    void AccelerometerEvent::timestampSet(const Value& result, Value::Number value)
    {
//##protect##"instance::AccelerometerEvent::timestampSet()"
        SF_UNUSED2(result, value);
        timestamp = value;
//##protect##"instance::AccelerometerEvent::timestampSet()"
    }
    void AccelerometerEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::AccelerometerEvent::clone()"
        SF_UNUSED1(result);
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        AccelerometerEvent* pe = static_cast<AccelerometerEvent*>(p.GetPtr());
        pe->timestamp       = timestamp;
        pe->accelerationX   = accelerationX;
		pe->accelerationY   = accelerationY;
		pe->accelerationZ   = accelerationZ;
        result = p.GetPtr();
//##protect##"instance::AccelerometerEvent::clone()"
    }
    void AccelerometerEvent::toString(ASString& result)
    {
//##protect##"instance::AccelerometerEvent::toString()"
		//[AccelerometerEvent type=value bubbles=value cancelable=value timestamp=value accelerationX=value accelerationY=value accelerationZ=value ] 
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("AccelerometerEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("timestamp"),
            vm.GetStringManager().CreateConstString("accelerationX"),
            vm.GetStringManager().CreateConstString("accelerationY"),
            vm.GetStringManager().CreateConstString("accelerationZ")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::AccelerometerEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> AccelerometerEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 AccelerometerEvent::tito[AccelerometerEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 
    // };
    const TypeInfo* AccelerometerEvent::tit[14] = {
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
    const ThunkInfo AccelerometerEvent::ti[AccelerometerEvent::ThunkInfoNum] = {
        {TFunc_Instances_AccelerometerEvent_accelerationXGet::Func, &AccelerometerEvent::tit[0], "accelerationX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_accelerationXSet::Func, &AccelerometerEvent::tit[1], "accelerationX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_accelerationYGet::Func, &AccelerometerEvent::tit[3], "accelerationY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_accelerationYSet::Func, &AccelerometerEvent::tit[4], "accelerationY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_accelerationZGet::Func, &AccelerometerEvent::tit[6], "accelerationZ", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_accelerationZSet::Func, &AccelerometerEvent::tit[7], "accelerationZ", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_timestampGet::Func, &AccelerometerEvent::tit[9], "timestamp", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_timestampSet::Func, &AccelerometerEvent::tit[10], "timestamp", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_clone::Func, &AccelerometerEvent::tit[12], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_AccelerometerEvent_toString::Func, &AccelerometerEvent::tit[13], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    AccelerometerEvent::AccelerometerEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::AccelerometerEvent::AccelerometerEvent()"
//##protect##"InstanceTraits::AccelerometerEvent::AccelerometerEvent()"

    }

    void AccelerometerEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<AccelerometerEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    AccelerometerEvent::AccelerometerEvent(ClassTraits::Traits& t)
    : Class(t)
    , UPDATE("update")
    {
//##protect##"class_::AccelerometerEvent::AccelerometerEvent()"
//##protect##"class_::AccelerometerEvent::AccelerometerEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo AccelerometerEvent::mi[AccelerometerEvent::MemberInfoNum] = {
        {"UPDATE", NULL, OFFSETOF(Classes::fl_events::AccelerometerEvent, UPDATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    AccelerometerEvent::AccelerometerEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::AccelerometerEvent::AccelerometerEvent()"
//##protect##"ClassTraits::AccelerometerEvent::AccelerometerEvent()"

    }

    Pickable<Traits> AccelerometerEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) AccelerometerEvent(vm, AS3::fl_events::AccelerometerEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::AccelerometerEventCI));
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
    const TypeInfo AccelerometerEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::AccelerometerEvent::InstanceType),
        0,
        ClassTraits::fl_events::AccelerometerEvent::MemberInfoNum,
        InstanceTraits::fl_events::AccelerometerEvent::ThunkInfoNum,
        0,
        "AccelerometerEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo AccelerometerEventCI = {
        &AccelerometerEventTI,
        ClassTraits::fl_events::AccelerometerEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::AccelerometerEvent::mi,
        InstanceTraits::fl_events::AccelerometerEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

