//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_OutputProgressEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_OutputProgressEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_bytesPendingGet, Value::Number> TFunc_Instances_OutputProgressEvent_bytesPendingGet;
typedef ThunkFunc1<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_bytesPendingSet, const Value, Value::Number> TFunc_Instances_OutputProgressEvent_bytesPendingSet;
typedef ThunkFunc0<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_bytesTotalGet, Value::Number> TFunc_Instances_OutputProgressEvent_bytesTotalGet;
typedef ThunkFunc1<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_bytesTotalSet, const Value, Value::Number> TFunc_Instances_OutputProgressEvent_bytesTotalSet;
typedef ThunkFunc0<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_OutputProgressEvent_clone;
typedef ThunkFunc0<Instances::fl_events::OutputProgressEvent, Instances::fl_events::OutputProgressEvent::mid_toString, ASString> TFunc_Instances_OutputProgressEvent_toString;

template <> const TFunc_Instances_OutputProgressEvent_bytesPendingGet::TMethod TFunc_Instances_OutputProgressEvent_bytesPendingGet::Method = &Instances::fl_events::OutputProgressEvent::bytesPendingGet;
template <> const TFunc_Instances_OutputProgressEvent_bytesPendingSet::TMethod TFunc_Instances_OutputProgressEvent_bytesPendingSet::Method = &Instances::fl_events::OutputProgressEvent::bytesPendingSet;
template <> const TFunc_Instances_OutputProgressEvent_bytesTotalGet::TMethod TFunc_Instances_OutputProgressEvent_bytesTotalGet::Method = &Instances::fl_events::OutputProgressEvent::bytesTotalGet;
template <> const TFunc_Instances_OutputProgressEvent_bytesTotalSet::TMethod TFunc_Instances_OutputProgressEvent_bytesTotalSet::Method = &Instances::fl_events::OutputProgressEvent::bytesTotalSet;
template <> const TFunc_Instances_OutputProgressEvent_clone::TMethod TFunc_Instances_OutputProgressEvent_clone::Method = &Instances::fl_events::OutputProgressEvent::clone;
template <> const TFunc_Instances_OutputProgressEvent_toString::TMethod TFunc_Instances_OutputProgressEvent_toString::Method = &Instances::fl_events::OutputProgressEvent::toString;

namespace Instances { namespace fl_events
{
    OutputProgressEvent::OutputProgressEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::OutputProgressEvent::OutputProgressEvent()$data"
    , BytesPending(0)
    , BytesTotal(0)
//##protect##"instance::OutputProgressEvent::OutputProgressEvent()$data"
    {
//##protect##"instance::OutputProgressEvent::OutputProgressEvent()$code"
//##protect##"instance::OutputProgressEvent::OutputProgressEvent()$code"
    }

    void OutputProgressEvent::bytesPendingGet(Value::Number& result)
    {
//##protect##"instance::OutputProgressEvent::bytesPendingGet()"
        result = (Value::Number)BytesPending;
//##protect##"instance::OutputProgressEvent::bytesPendingGet()"
    }
    void OutputProgressEvent::bytesPendingSet(const Value& result, Value::Number value)
    {
//##protect##"instance::OutputProgressEvent::bytesPendingSet()"
        SF_UNUSED1(result);
        BytesPending = (UPInt)value;
//##protect##"instance::OutputProgressEvent::bytesPendingSet()"
    }
    void OutputProgressEvent::bytesTotalGet(Value::Number& result)
    {
//##protect##"instance::OutputProgressEvent::bytesTotalGet()"
        result = (Value::Number)BytesTotal;
//##protect##"instance::OutputProgressEvent::bytesTotalGet()"
    }
    void OutputProgressEvent::bytesTotalSet(const Value& result, Value::Number value)
    {
//##protect##"instance::OutputProgressEvent::bytesTotalSet()"
        SF_UNUSED1(result);
        BytesTotal = (UPInt)value;
//##protect##"instance::OutputProgressEvent::bytesTotalSet()"
    }
    void OutputProgressEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::OutputProgressEvent::clone()"
        result = Clone().GetPtr();
//##protect##"instance::OutputProgressEvent::clone()"
    }
    void OutputProgressEvent::toString(ASString& result)
    {
//##protect##"instance::OutputProgressEvent::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("OutputProgressEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("bytesLoaded"),
            vm.GetStringManager().CreateConstString("bytesTotal")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::OutputProgressEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> OutputProgressEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    AS3::Object* OutputProgressEvent::GetEventClass() const 
    { 
#ifdef SF_AS3_VERSION_AIR
        return static_cast<ASVM&>(GetVM()).OutputProgressEventClass; 
#else
        return NULL;
#endif
    }

    SPtr<Instances::fl_events::Event> OutputProgressEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        OutputProgressEvent* pe = static_cast<OutputProgressEvent*>(p.GetPtr());
        pe->BytesPending = BytesPending;
        pe->BytesTotal  = BytesTotal;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 OutputProgressEvent::tito[OutputProgressEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* OutputProgressEvent::tit[8] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo OutputProgressEvent::ti[OutputProgressEvent::ThunkInfoNum] = {
        {TFunc_Instances_OutputProgressEvent_bytesPendingGet::Func, &OutputProgressEvent::tit[0], "bytesPending", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_OutputProgressEvent_bytesPendingSet::Func, &OutputProgressEvent::tit[1], "bytesPending", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_OutputProgressEvent_bytesTotalGet::Func, &OutputProgressEvent::tit[3], "bytesTotal", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_OutputProgressEvent_bytesTotalSet::Func, &OutputProgressEvent::tit[4], "bytesTotal", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_OutputProgressEvent_clone::Func, &OutputProgressEvent::tit[6], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_OutputProgressEvent_toString::Func, &OutputProgressEvent::tit[7], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    OutputProgressEvent::OutputProgressEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::OutputProgressEvent::OutputProgressEvent()"
//##protect##"InstanceTraits::OutputProgressEvent::OutputProgressEvent()"

    }

    void OutputProgressEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<OutputProgressEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    OutputProgressEvent::OutputProgressEvent(ClassTraits::Traits& t)
    : Class(t)
    , OUTPUT_PROGRESS("outputProgress")
    {
//##protect##"class_::OutputProgressEvent::OutputProgressEvent()"
//##protect##"class_::OutputProgressEvent::OutputProgressEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo OutputProgressEvent::mi[OutputProgressEvent::MemberInfoNum] = {
        {"OUTPUT_PROGRESS", NULL, OFFSETOF(Classes::fl_events::OutputProgressEvent, OUTPUT_PROGRESS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    OutputProgressEvent::OutputProgressEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::OutputProgressEvent::OutputProgressEvent()"
//##protect##"ClassTraits::OutputProgressEvent::OutputProgressEvent()"

    }

    Pickable<Traits> OutputProgressEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) OutputProgressEvent(vm, AS3::fl_events::OutputProgressEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::OutputProgressEventCI));
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
    const TypeInfo OutputProgressEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::OutputProgressEvent::InstanceType),
        0,
        ClassTraits::fl_events::OutputProgressEvent::MemberInfoNum,
        InstanceTraits::fl_events::OutputProgressEvent::ThunkInfoNum,
        0,
        "OutputProgressEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo OutputProgressEventCI = {
        &OutputProgressEventTI,
        ClassTraits::fl_events::OutputProgressEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::OutputProgressEvent::mi,
        InstanceTraits::fl_events::OutputProgressEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

