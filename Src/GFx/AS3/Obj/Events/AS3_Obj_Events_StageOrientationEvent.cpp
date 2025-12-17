//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_StageOrientationEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_StageOrientationEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::StageOrientationEvent, Instances::fl_events::StageOrientationEvent::mid_afterOrientationGet, ASString> TFunc_Instances_StageOrientationEvent_afterOrientationGet;
typedef ThunkFunc0<Instances::fl_events::StageOrientationEvent, Instances::fl_events::StageOrientationEvent::mid_beforeOrientationGet, ASString> TFunc_Instances_StageOrientationEvent_beforeOrientationGet;
typedef ThunkFunc0<Instances::fl_events::StageOrientationEvent, Instances::fl_events::StageOrientationEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_StageOrientationEvent_clone;
typedef ThunkFunc0<Instances::fl_events::StageOrientationEvent, Instances::fl_events::StageOrientationEvent::mid_toString, ASString> TFunc_Instances_StageOrientationEvent_toString;

template <> const TFunc_Instances_StageOrientationEvent_afterOrientationGet::TMethod TFunc_Instances_StageOrientationEvent_afterOrientationGet::Method = &Instances::fl_events::StageOrientationEvent::afterOrientationGet;
template <> const TFunc_Instances_StageOrientationEvent_beforeOrientationGet::TMethod TFunc_Instances_StageOrientationEvent_beforeOrientationGet::Method = &Instances::fl_events::StageOrientationEvent::beforeOrientationGet;
template <> const TFunc_Instances_StageOrientationEvent_clone::TMethod TFunc_Instances_StageOrientationEvent_clone::Method = &Instances::fl_events::StageOrientationEvent::clone;
template <> const TFunc_Instances_StageOrientationEvent_toString::TMethod TFunc_Instances_StageOrientationEvent_toString::Method = &Instances::fl_events::StageOrientationEvent::toString;

namespace Instances { namespace fl_events
{
    StageOrientationEvent::StageOrientationEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
//##protect##"instance::StageOrientationEvent::StageOrientationEvent()$data"
//##protect##"instance::StageOrientationEvent::StageOrientationEvent()$data"
    {
//##protect##"instance::StageOrientationEvent::StageOrientationEvent()$code"
        BeforeOrientation.SetNull();
        AfterOrientation.SetNull();
//##protect##"instance::StageOrientationEvent::StageOrientationEvent()$code"
    }

    void StageOrientationEvent::afterOrientationGet(ASString& result)
    {
//##protect##"instance::StageOrientationEvent::afterOrientationGet()"
        SF_UNUSED1(result);
        result = AfterOrientation;
//##protect##"instance::StageOrientationEvent::afterOrientationGet()"
    }
    void StageOrientationEvent::beforeOrientationGet(ASString& result)
    {
//##protect##"instance::StageOrientationEvent::beforeOrientationGet()"
        SF_UNUSED1(result);
        result = BeforeOrientation;
//##protect##"instance::StageOrientationEvent::beforeOrientationGet()"
    }
    void StageOrientationEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::StageOrientationEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::StageOrientationEvent::clone()"
    }
    void StageOrientationEvent::toString(ASString& result)
    {
//##protect##"instance::StageOrientationEvent::toString()"
        SF_UNUSED1(result);
        //example: [StageOrientationEvent type="sss" bubbles=false cancelable=false beforeOrientation=null afterOrientation=null]
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("StageOrientationEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("beforeOrientation"),
            vm.GetStringManager().CreateConstString("afterOrientation")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::StageOrientationEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> StageOrientationEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    void StageOrientationEvent::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_events::Event::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC(prcc, BeforeOrientation, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC(prcc, AfterOrientation, op SF_DEBUG_ARG(*this));
    }
    // public function StageOrientationEvent(type:String, bubbles:Boolean = false, cancelable:Boolean = false, 
    //                                       beforeOrientation:String = null, afterOrientation:String = null)
    void StageOrientationEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        ASVM& vm = static_cast<ASVM&>(GetVM());
        if (argc >= 4)
        {
            BeforeOrientation = argv[3];
            if (!argv[3].IsNull())
            {
                BeforeOrientation.ToStringValue(vm.GetStringManager()).DoNotCheck();
            }
        }
        if (argc >= 5)
        {
            AfterOrientation = argv[4];
            if (!argv[3].IsNull())
            {
                AfterOrientation.ToStringValue(vm.GetStringManager()).DoNotCheck();
            }
        }
    }
    AS3::Object* StageOrientationEvent::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).StageOrientationEventClass; 
    }

    SPtr<Instances::fl_events::Event> StageOrientationEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        StageOrientationEvent* pe = static_cast<StageOrientationEvent*>(p.GetPtr());
        pe->BeforeOrientation = BeforeOrientation;
        pe->AfterOrientation  = AfterOrientation;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 StageOrientationEvent::tito[StageOrientationEvent::ThunkInfoNum] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* StageOrientationEvent::tit[4] = {
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo StageOrientationEvent::ti[StageOrientationEvent::ThunkInfoNum] = {
        {TFunc_Instances_StageOrientationEvent_afterOrientationGet::Func, &StageOrientationEvent::tit[0], "afterOrientation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StageOrientationEvent_beforeOrientationGet::Func, &StageOrientationEvent::tit[1], "beforeOrientation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StageOrientationEvent_clone::Func, &StageOrientationEvent::tit[2], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_StageOrientationEvent_toString::Func, &StageOrientationEvent::tit[3], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    StageOrientationEvent::StageOrientationEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::StageOrientationEvent::StageOrientationEvent()"
//##protect##"InstanceTraits::StageOrientationEvent::StageOrientationEvent()"

    }

    void StageOrientationEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<StageOrientationEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    StageOrientationEvent::StageOrientationEvent(ClassTraits::Traits& t)
    : Class(t)
    , ORIENTATION_CHANGE("orientationChange")
    , ORIENTATION_CHANGING("orientationChanging")
    {
//##protect##"class_::StageOrientationEvent::StageOrientationEvent()"
//##protect##"class_::StageOrientationEvent::StageOrientationEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo StageOrientationEvent::mi[StageOrientationEvent::MemberInfoNum] = {
        {"ORIENTATION_CHANGE", NULL, OFFSETOF(Classes::fl_events::StageOrientationEvent, ORIENTATION_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ORIENTATION_CHANGING", NULL, OFFSETOF(Classes::fl_events::StageOrientationEvent, ORIENTATION_CHANGING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StageOrientationEvent::StageOrientationEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::StageOrientationEvent::StageOrientationEvent()"
//##protect##"ClassTraits::StageOrientationEvent::StageOrientationEvent()"

    }

    Pickable<Traits> StageOrientationEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StageOrientationEvent(vm, AS3::fl_events::StageOrientationEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::StageOrientationEventCI));
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
    const TypeInfo StageOrientationEventTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_events::StageOrientationEvent::InstanceType),
        0,
        ClassTraits::fl_events::StageOrientationEvent::MemberInfoNum,
        InstanceTraits::fl_events::StageOrientationEvent::ThunkInfoNum,
        0,
        "StageOrientationEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo StageOrientationEventCI = {
        &StageOrientationEventTI,
        ClassTraits::fl_events::StageOrientationEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::StageOrientationEvent::mi,
        InstanceTraits::fl_events::StageOrientationEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

