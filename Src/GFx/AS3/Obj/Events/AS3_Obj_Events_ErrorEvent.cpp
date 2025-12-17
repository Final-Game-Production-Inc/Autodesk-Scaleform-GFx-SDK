//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ErrorEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_ErrorEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::ErrorEvent, Instances::fl_events::ErrorEvent::mid_errorIDGet, SInt32> TFunc_Instances_ErrorEvent_errorIDGet;
typedef ThunkFunc0<Instances::fl_events::ErrorEvent, Instances::fl_events::ErrorEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_ErrorEvent_clone;
typedef ThunkFunc0<Instances::fl_events::ErrorEvent, Instances::fl_events::ErrorEvent::mid_toString, ASString> TFunc_Instances_ErrorEvent_toString;

template <> const TFunc_Instances_ErrorEvent_errorIDGet::TMethod TFunc_Instances_ErrorEvent_errorIDGet::Method = &Instances::fl_events::ErrorEvent::errorIDGet;
template <> const TFunc_Instances_ErrorEvent_clone::TMethod TFunc_Instances_ErrorEvent_clone::Method = &Instances::fl_events::ErrorEvent::clone;
template <> const TFunc_Instances_ErrorEvent_toString::TMethod TFunc_Instances_ErrorEvent_toString::Method = &Instances::fl_events::ErrorEvent::toString;

namespace Instances { namespace fl_events
{
    ErrorEvent::ErrorEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::TextEvent(t)
//##protect##"instance::ErrorEvent::ErrorEvent()$data"
//##protect##"instance::ErrorEvent::ErrorEvent()$data"
    {
//##protect##"instance::ErrorEvent::ErrorEvent()$code"
//##protect##"instance::ErrorEvent::ErrorEvent()$code"
    }

    void ErrorEvent::errorIDGet(SInt32& result)
    {
//##protect##"instance::ErrorEvent::errorIDGet()"
        result = errorID;
//##protect##"instance::ErrorEvent::errorIDGet()"
    }
    void ErrorEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::ErrorEvent::clone()"
        result = Clone().GetPtr();
//##protect##"instance::ErrorEvent::clone()"
    }
    void ErrorEvent::toString(ASString& result)
    {
//##protect##"instance::ErrorEvent::toString()"
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("ErrorEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase"),
            vm.GetStringManager().CreateConstString("text"),
            vm.GetStringManager().CreateConstString("errorID")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::ErrorEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> ErrorEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void ErrorEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        TextEvent::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            if (!argv[3].IsNull() && argv[3].IsInt())
            {
                errorID = argv[3].AsInt();
            }
        }
    }

    SPtr<Instances::fl_events::Event> ErrorEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = TextEvent::Clone();
        ErrorEvent* pe = static_cast<ErrorEvent*>(p.GetPtr());
        pe->errorID = errorID;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 ErrorEvent::tito[ErrorEvent::ThunkInfoNum] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* ErrorEvent::tit[3] = {
        &AS3::fl::int_TI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ErrorEvent::ti[ErrorEvent::ThunkInfoNum] = {
        {TFunc_Instances_ErrorEvent_errorIDGet::Func, &ErrorEvent::tit[0], "errorID", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ErrorEvent_clone::Func, &ErrorEvent::tit[1], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ErrorEvent_toString::Func, &ErrorEvent::tit[2], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    ErrorEvent::ErrorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::TextEvent(vm, ci)
    {
//##protect##"InstanceTraits::ErrorEvent::ErrorEvent()"
//##protect##"InstanceTraits::ErrorEvent::ErrorEvent()"

    }

    void ErrorEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ErrorEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    ErrorEvent::ErrorEvent(ClassTraits::Traits& t)
    : Class(t)
    , ERROR("error")
    {
//##protect##"class_::ErrorEvent::ErrorEvent()"
//##protect##"class_::ErrorEvent::ErrorEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo ErrorEvent::mi[ErrorEvent::MemberInfoNum] = {
        {"ERROR", NULL, OFFSETOF(Classes::fl_events::ErrorEvent, ERROR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    ErrorEvent::ErrorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::TextEvent(vm, ci)
    {
//##protect##"ClassTraits::ErrorEvent::ErrorEvent()"
//##protect##"ClassTraits::ErrorEvent::ErrorEvent()"

    }

    Pickable<Traits> ErrorEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ErrorEvent(vm, AS3::fl_events::ErrorEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::ErrorEventCI));
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
    const TypeInfo ErrorEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::ErrorEvent::InstanceType),
        0,
        ClassTraits::fl_events::ErrorEvent::MemberInfoNum,
        InstanceTraits::fl_events::ErrorEvent::ThunkInfoNum,
        0,
        "ErrorEvent", "flash.events", &fl_events::TextEventTI,
        TypeInfo::None
    };

    const ClassInfo ErrorEventCI = {
        &ErrorEventTI,
        ClassTraits::fl_events::ErrorEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::ErrorEvent::mi,
        InstanceTraits::fl_events::ErrorEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

