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

#include "AS3_Obj_Events_GeolocationEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_altitudeGet, Value::Number> TFunc_Instances_GeolocationEvent_altitudeGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_altitudeSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_altitudeSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_headingGet, Value::Number> TFunc_Instances_GeolocationEvent_headingGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_headingSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_headingSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_horizontalAccuracyGet, Value::Number> TFunc_Instances_GeolocationEvent_horizontalAccuracyGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_horizontalAccuracySet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_horizontalAccuracySet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_latitudeGet, Value::Number> TFunc_Instances_GeolocationEvent_latitudeGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_latitudeSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_latitudeSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_longitudeGet, Value::Number> TFunc_Instances_GeolocationEvent_longitudeGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_longitudeSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_longitudeSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_speedGet, Value::Number> TFunc_Instances_GeolocationEvent_speedGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_speedSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_speedSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_timestampGet, Value::Number> TFunc_Instances_GeolocationEvent_timestampGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_timestampSet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_timestampSet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_verticalAccuracyGet, Value::Number> TFunc_Instances_GeolocationEvent_verticalAccuracyGet;
typedef ThunkFunc1<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_verticalAccuracySet, const Value, Value::Number> TFunc_Instances_GeolocationEvent_verticalAccuracySet;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_GeolocationEvent_clone;
typedef ThunkFunc0<Instances::fl_events::GeolocationEvent, Instances::fl_events::GeolocationEvent::mid_toString, ASString> TFunc_Instances_GeolocationEvent_toString;

template <> const TFunc_Instances_GeolocationEvent_altitudeGet::TMethod TFunc_Instances_GeolocationEvent_altitudeGet::Method = &Instances::fl_events::GeolocationEvent::altitudeGet;
template <> const TFunc_Instances_GeolocationEvent_altitudeSet::TMethod TFunc_Instances_GeolocationEvent_altitudeSet::Method = &Instances::fl_events::GeolocationEvent::altitudeSet;
template <> const TFunc_Instances_GeolocationEvent_headingGet::TMethod TFunc_Instances_GeolocationEvent_headingGet::Method = &Instances::fl_events::GeolocationEvent::headingGet;
template <> const TFunc_Instances_GeolocationEvent_headingSet::TMethod TFunc_Instances_GeolocationEvent_headingSet::Method = &Instances::fl_events::GeolocationEvent::headingSet;
template <> const TFunc_Instances_GeolocationEvent_horizontalAccuracyGet::TMethod TFunc_Instances_GeolocationEvent_horizontalAccuracyGet::Method = &Instances::fl_events::GeolocationEvent::horizontalAccuracyGet;
template <> const TFunc_Instances_GeolocationEvent_horizontalAccuracySet::TMethod TFunc_Instances_GeolocationEvent_horizontalAccuracySet::Method = &Instances::fl_events::GeolocationEvent::horizontalAccuracySet;
template <> const TFunc_Instances_GeolocationEvent_latitudeGet::TMethod TFunc_Instances_GeolocationEvent_latitudeGet::Method = &Instances::fl_events::GeolocationEvent::latitudeGet;
template <> const TFunc_Instances_GeolocationEvent_latitudeSet::TMethod TFunc_Instances_GeolocationEvent_latitudeSet::Method = &Instances::fl_events::GeolocationEvent::latitudeSet;
template <> const TFunc_Instances_GeolocationEvent_longitudeGet::TMethod TFunc_Instances_GeolocationEvent_longitudeGet::Method = &Instances::fl_events::GeolocationEvent::longitudeGet;
template <> const TFunc_Instances_GeolocationEvent_longitudeSet::TMethod TFunc_Instances_GeolocationEvent_longitudeSet::Method = &Instances::fl_events::GeolocationEvent::longitudeSet;
template <> const TFunc_Instances_GeolocationEvent_speedGet::TMethod TFunc_Instances_GeolocationEvent_speedGet::Method = &Instances::fl_events::GeolocationEvent::speedGet;
template <> const TFunc_Instances_GeolocationEvent_speedSet::TMethod TFunc_Instances_GeolocationEvent_speedSet::Method = &Instances::fl_events::GeolocationEvent::speedSet;
template <> const TFunc_Instances_GeolocationEvent_timestampGet::TMethod TFunc_Instances_GeolocationEvent_timestampGet::Method = &Instances::fl_events::GeolocationEvent::timestampGet;
template <> const TFunc_Instances_GeolocationEvent_timestampSet::TMethod TFunc_Instances_GeolocationEvent_timestampSet::Method = &Instances::fl_events::GeolocationEvent::timestampSet;
template <> const TFunc_Instances_GeolocationEvent_verticalAccuracyGet::TMethod TFunc_Instances_GeolocationEvent_verticalAccuracyGet::Method = &Instances::fl_events::GeolocationEvent::verticalAccuracyGet;
template <> const TFunc_Instances_GeolocationEvent_verticalAccuracySet::TMethod TFunc_Instances_GeolocationEvent_verticalAccuracySet::Method = &Instances::fl_events::GeolocationEvent::verticalAccuracySet;
template <> const TFunc_Instances_GeolocationEvent_clone::TMethod TFunc_Instances_GeolocationEvent_clone::Method = &Instances::fl_events::GeolocationEvent::clone;
template <> const TFunc_Instances_GeolocationEvent_toString::TMethod TFunc_Instances_GeolocationEvent_toString::Method = &Instances::fl_events::GeolocationEvent::toString;

namespace Instances { namespace fl_events
{
    GeolocationEvent::GeolocationEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::GeolocationEvent::GeolocationEvent()$data"
//##protect##"instance::GeolocationEvent::GeolocationEvent()$data"
    {
//##protect##"instance::GeolocationEvent::GeolocationEvent()$code"
//##protect##"instance::GeolocationEvent::GeolocationEvent()$code"
    }

    void GeolocationEvent::altitudeGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::altitudeGet()"
        SF_UNUSED1(result);
        result = altitude;
//##protect##"instance::GeolocationEvent::altitudeGet()"
    }
    void GeolocationEvent::altitudeSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::altitudeSet()"
        SF_UNUSED2(result, value);
        altitude = result;
//##protect##"instance::GeolocationEvent::altitudeSet()"
    }
    void GeolocationEvent::headingGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::headingGet()"
        SF_UNUSED1(result);
		result = heading;
//##protect##"instance::GeolocationEvent::headingGet()"
    }
    void GeolocationEvent::headingSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::headingSet()"
        SF_UNUSED2(result, value);
        heading = result;
//##protect##"instance::GeolocationEvent::headingSet()"
    }
    void GeolocationEvent::horizontalAccuracyGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::horizontalAccuracyGet()"
        SF_UNUSED1(result);
		result =  hAccuracy;
//##protect##"instance::GeolocationEvent::horizontalAccuracyGet()"
    }
    void GeolocationEvent::horizontalAccuracySet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::horizontalAccuracySet()"
        SF_UNUSED2(result, value);
        hAccuracy = result;
//##protect##"instance::GeolocationEvent::horizontalAccuracySet()"
    }
    void GeolocationEvent::latitudeGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::latitudeGet()"
        SF_UNUSED1(result);
        result = latitude;
//##protect##"instance::GeolocationEvent::latitudeGet()"
    }
    void GeolocationEvent::latitudeSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::latitudeSet()"
        SF_UNUSED2(result, value);
        latitude = result;
//##protect##"instance::GeolocationEvent::latitudeSet()"
    }
    void GeolocationEvent::longitudeGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::longitudeGet()"
        SF_UNUSED1(result);
        result = longitude;
//##protect##"instance::GeolocationEvent::longitudeGet()"
    }
    void GeolocationEvent::longitudeSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::longitudeSet()"
        SF_UNUSED2(result, value);
        longitude = result;
//##protect##"instance::GeolocationEvent::longitudeSet()"
    }
    void GeolocationEvent::speedGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::speedGet()"
        SF_UNUSED1(result);
        result = speed;
//##protect##"instance::GeolocationEvent::speedGet()"
    }
    void GeolocationEvent::speedSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::speedSet()"
        SF_UNUSED2(result, value);
        speed = result;
//##protect##"instance::GeolocationEvent::speedSet()"
    }
    void GeolocationEvent::timestampGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::timestampGet()"
        SF_UNUSED1(result);
        result = timestamp;
//##protect##"instance::GeolocationEvent::timestampGet()"
    }
    void GeolocationEvent::timestampSet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::timestampSet()"
        SF_UNUSED2(result, value);
        timestamp = result;
//##protect##"instance::GeolocationEvent::timestampSet()"
    }
    void GeolocationEvent::verticalAccuracyGet(Value::Number& result)
    {
//##protect##"instance::GeolocationEvent::verticalAccuracyGet()"
        SF_UNUSED1(result);
        result = vAccuracy;
//##protect##"instance::GeolocationEvent::verticalAccuracyGet()"
    }
    void GeolocationEvent::verticalAccuracySet(const Value& result, Value::Number value)
    {
//##protect##"instance::GeolocationEvent::verticalAccuracySet()"
        SF_UNUSED2(result, value);
        vAccuracy = result;
//##protect##"instance::GeolocationEvent::verticalAccuracySet()"
    }
    void GeolocationEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::GeolocationEvent::clone()"
        SF_UNUSED1(result);
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        GeolocationEvent* pe = static_cast<GeolocationEvent*>(p.GetPtr());
		pe->latitude = latitude;
		pe->longitude = longitude;
		pe->altitude = altitude;
		pe->hAccuracy = hAccuracy;
		pe->vAccuracy = vAccuracy;
		pe->speed = speed;
		pe->heading = heading;
		pe->timestamp = timestamp;
        result = p.GetPtr();
//##protect##"instance::GeolocationEvent::clone()"
    }
    void GeolocationEvent::toString(ASString& result)
    {
//##protect##"instance::GeolocationEvent::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("GeolocationEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("latitude"),
            vm.GetStringManager().CreateConstString("longitude"),
            vm.GetStringManager().CreateConstString("altitude"),
			vm.GetStringManager().CreateConstString("hAccuracy"),
            vm.GetStringManager().CreateConstString("vAccuracy"),
            vm.GetStringManager().CreateConstString("speed"),
            vm.GetStringManager().CreateConstString("heading"),
			vm.GetStringManager().CreateConstString("timestamp")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::GeolocationEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> GeolocationEvent::clone()
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
    // const UInt16 GeolocationEvent::tito[GeolocationEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 
    // };
    const TypeInfo* GeolocationEvent::tit[26] = {
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
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo GeolocationEvent::ti[GeolocationEvent::ThunkInfoNum] = {
        {TFunc_Instances_GeolocationEvent_altitudeGet::Func, &GeolocationEvent::tit[0], "altitude", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_altitudeSet::Func, &GeolocationEvent::tit[1], "altitude", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_headingGet::Func, &GeolocationEvent::tit[3], "heading", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_headingSet::Func, &GeolocationEvent::tit[4], "heading", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_horizontalAccuracyGet::Func, &GeolocationEvent::tit[6], "horizontalAccuracy", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_horizontalAccuracySet::Func, &GeolocationEvent::tit[7], "horizontalAccuracy", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_latitudeGet::Func, &GeolocationEvent::tit[9], "latitude", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_latitudeSet::Func, &GeolocationEvent::tit[10], "latitude", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_longitudeGet::Func, &GeolocationEvent::tit[12], "longitude", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_longitudeSet::Func, &GeolocationEvent::tit[13], "longitude", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_speedGet::Func, &GeolocationEvent::tit[15], "speed", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_speedSet::Func, &GeolocationEvent::tit[16], "speed", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_timestampGet::Func, &GeolocationEvent::tit[18], "timestamp", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_timestampSet::Func, &GeolocationEvent::tit[19], "timestamp", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_verticalAccuracyGet::Func, &GeolocationEvent::tit[21], "verticalAccuracy", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_verticalAccuracySet::Func, &GeolocationEvent::tit[22], "verticalAccuracy", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_clone::Func, &GeolocationEvent::tit[24], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_GeolocationEvent_toString::Func, &GeolocationEvent::tit[25], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GeolocationEvent::GeolocationEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::GeolocationEvent::GeolocationEvent()"
//##protect##"InstanceTraits::GeolocationEvent::GeolocationEvent()"

    }

    void GeolocationEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GeolocationEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    GeolocationEvent::GeolocationEvent(ClassTraits::Traits& t)
    : Class(t)
    , UPDATE("update")
    {
//##protect##"class_::GeolocationEvent::GeolocationEvent()"
//##protect##"class_::GeolocationEvent::GeolocationEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo GeolocationEvent::mi[GeolocationEvent::MemberInfoNum] = {
        {"UPDATE", NULL, OFFSETOF(Classes::fl_events::GeolocationEvent, UPDATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    GeolocationEvent::GeolocationEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::GeolocationEvent::GeolocationEvent()"
//##protect##"ClassTraits::GeolocationEvent::GeolocationEvent()"

    }

    Pickable<Traits> GeolocationEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GeolocationEvent(vm, AS3::fl_events::GeolocationEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::GeolocationEventCI));
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
    const TypeInfo GeolocationEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::GeolocationEvent::InstanceType),
        0,
        ClassTraits::fl_events::GeolocationEvent::MemberInfoNum,
        InstanceTraits::fl_events::GeolocationEvent::ThunkInfoNum,
        0,
        "GeolocationEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo GeolocationEventCI = {
        &GeolocationEventTI,
        ClassTraits::fl_events::GeolocationEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::GeolocationEvent::mi,
        InstanceTraits::fl_events::GeolocationEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

