//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_IOErrorEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_IOErrorEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::IOErrorEvent, Instances::fl_events::IOErrorEvent::mid_textGet, ASString> TFunc_Instances_IOErrorEvent_textGet;
typedef ThunkFunc1<Instances::fl_events::IOErrorEvent, Instances::fl_events::IOErrorEvent::mid_textSet, const Value, const ASString&> TFunc_Instances_IOErrorEvent_textSet;
typedef ThunkFunc0<Instances::fl_events::IOErrorEvent, Instances::fl_events::IOErrorEvent::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_IOErrorEvent_clone;
typedef ThunkFunc0<Instances::fl_events::IOErrorEvent, Instances::fl_events::IOErrorEvent::mid_toString, ASString> TFunc_Instances_IOErrorEvent_toString;

template <> const TFunc_Instances_IOErrorEvent_textGet::TMethod TFunc_Instances_IOErrorEvent_textGet::Method = &Instances::fl_events::IOErrorEvent::textGet;
template <> const TFunc_Instances_IOErrorEvent_textSet::TMethod TFunc_Instances_IOErrorEvent_textSet::Method = &Instances::fl_events::IOErrorEvent::textSet;
template <> const TFunc_Instances_IOErrorEvent_clone::TMethod TFunc_Instances_IOErrorEvent_clone::Method = &Instances::fl_events::IOErrorEvent::clone;
template <> const TFunc_Instances_IOErrorEvent_toString::TMethod TFunc_Instances_IOErrorEvent_toString::Method = &Instances::fl_events::IOErrorEvent::toString;

namespace Instances { namespace fl_events
{
    IOErrorEvent::IOErrorEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::ErrorEvent(t)
//##protect##"instance::IOErrorEvent::IOErrorEvent()$data"
    , Text(GetStringManager().GetBuiltin(AS3Builtin_empty_))
//##protect##"instance::IOErrorEvent::IOErrorEvent()$data"
    {
//##protect##"instance::IOErrorEvent::IOErrorEvent()$code"
//##protect##"instance::IOErrorEvent::IOErrorEvent()$code"
    }

    void IOErrorEvent::textGet(ASString& result)
    {
//##protect##"instance::IOErrorEvent::textGet()"
        SF_UNUSED1(result);
        result = Text;
//##protect##"instance::IOErrorEvent::textGet()"
    }
    void IOErrorEvent::textSet(const Value& result, const ASString& value)
    {
//##protect##"instance::IOErrorEvent::textSet()"
        SF_UNUSED2(result, value);
        Text = value;
//##protect##"instance::IOErrorEvent::textSet()"
    }
    void IOErrorEvent::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::IOErrorEvent::clone()"
        SF_UNUSED1(result);
        result = Clone().GetPtr();
//##protect##"instance::IOErrorEvent::clone()"
    }
    void IOErrorEvent::toString(ASString& result)
    {
//##protect##"instance::IOErrorEvent::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString("IOErrorEvent"),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("text")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::IOErrorEvent::toString()"
    }

    SPtr<Instances::fl_events::Event> IOErrorEvent::clone()
    {
        SPtr<Instances::fl_events::Event> result;
        clone(result);
        return result;
    }
//##protect##"instance$methods"
    AS3::Object* IOErrorEvent::GetEventClass() const 
    { 
        Class* result = GetVM().GetClass("flash.events.IOErrorEvent", GetVM().GetCurrentAppDomain());

        SF_ASSERT(result);
        return result;
    }

    SPtr<Instances::fl_events::Event> IOErrorEvent::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = fl_events::Event::Clone();
        IOErrorEvent* pe = static_cast<IOErrorEvent*>(p.GetPtr());
        pe->Text = Text;
        return p;
    }
    void IOErrorEvent::AS3Constructor(unsigned argc, const Value* argv)
    {
        Event::AS3Constructor(argc, argv);
        if (argc >= 4)
        {
            argv[3].Convert2String(Text).DoNotCheck();
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 IOErrorEvent::tito[IOErrorEvent::ThunkInfoNum] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* IOErrorEvent::tit[5] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo IOErrorEvent::ti[IOErrorEvent::ThunkInfoNum] = {
        {TFunc_Instances_IOErrorEvent_textGet::Func, &IOErrorEvent::tit[0], "text", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_IOErrorEvent_textSet::Func, &IOErrorEvent::tit[1], "text", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_IOErrorEvent_clone::Func, &IOErrorEvent::tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_IOErrorEvent_toString::Func, &IOErrorEvent::tit[4], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    IOErrorEvent::IOErrorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::ErrorEvent(vm, ci)
    {
//##protect##"InstanceTraits::IOErrorEvent::IOErrorEvent()"
//##protect##"InstanceTraits::IOErrorEvent::IOErrorEvent()"

    }

    void IOErrorEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<IOErrorEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    IOErrorEvent::IOErrorEvent(ClassTraits::Traits& t)
    : Class(t)
    , IO_ERROR("ioError")
    {
//##protect##"class_::IOErrorEvent::IOErrorEvent()"
//##protect##"class_::IOErrorEvent::IOErrorEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo IOErrorEvent::mi[IOErrorEvent::MemberInfoNum] = {
        {"IO_ERROR", NULL, OFFSETOF(Classes::fl_events::IOErrorEvent, IO_ERROR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    IOErrorEvent::IOErrorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::ErrorEvent(vm, ci)
    {
//##protect##"ClassTraits::IOErrorEvent::IOErrorEvent()"
//##protect##"ClassTraits::IOErrorEvent::IOErrorEvent()"

    }

    Pickable<Traits> IOErrorEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IOErrorEvent(vm, AS3::fl_events::IOErrorEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::IOErrorEventCI));
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
    const TypeInfo IOErrorEventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::IOErrorEvent::InstanceType),
        0,
        ClassTraits::fl_events::IOErrorEvent::MemberInfoNum,
        InstanceTraits::fl_events::IOErrorEvent::ThunkInfoNum,
        0,
        "IOErrorEvent", "flash.events", &fl_events::ErrorEventTI,
        TypeInfo::None
    };

    const ClassInfo IOErrorEventCI = {
        &IOErrorEventTI,
        ClassTraits::fl_events::IOErrorEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::IOErrorEvent::mi,
        InstanceTraits::fl_events::IOErrorEvent::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

