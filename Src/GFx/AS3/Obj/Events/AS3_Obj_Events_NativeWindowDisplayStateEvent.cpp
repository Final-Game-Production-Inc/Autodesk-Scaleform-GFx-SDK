//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_NativeWindowDisplayStateEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_NativeWindowDisplayStateEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 NativeWindowDisplayStateEvent_tito[4] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* NativeWindowDisplayStateEvent_tit[4] = {
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeWindowDisplayStateEvent_ti[4] = {
        {ThunkInfo::EmptyFunc, &NativeWindowDisplayStateEvent_tit[0], "afterDisplayState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowDisplayStateEvent_tit[1], "beforeDisplayState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowDisplayStateEvent_tit[2], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeWindowDisplayStateEvent_tit[3], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent(ClassTraits::Traits& t)
    : Class(t)
    , DISPLAY_STATE_CHANGE("displayStateChange")
    , DISPLAY_STATE_CHANGING("displayStateChanging")
    {
//##protect##"class_::NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent()"
//##protect##"class_::NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo NativeWindowDisplayStateEvent::mi[NativeWindowDisplayStateEvent::MemberInfoNum] = {
        {"DISPLAY_STATE_CHANGE", NULL, OFFSETOF(Classes::fl_events::NativeWindowDisplayStateEvent, DISPLAY_STATE_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DISPLAY_STATE_CHANGING", NULL, OFFSETOF(Classes::fl_events::NativeWindowDisplayStateEvent, DISPLAY_STATE_CHANGING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent()"
//##protect##"ClassTraits::NativeWindowDisplayStateEvent::NativeWindowDisplayStateEvent()"

    }

    Pickable<Traits> NativeWindowDisplayStateEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowDisplayStateEvent(vm, AS3::fl_events::NativeWindowDisplayStateEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::NativeWindowDisplayStateEventCI));
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
    const TypeInfo NativeWindowDisplayStateEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::NativeWindowDisplayStateEvent::InstanceType),
        0,
        ClassTraits::fl_events::NativeWindowDisplayStateEvent::MemberInfoNum,
        4,
        0,
        "NativeWindowDisplayStateEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowDisplayStateEventCI = {
        &NativeWindowDisplayStateEventTI,
        ClassTraits::fl_events::NativeWindowDisplayStateEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::NativeWindowDisplayStateEvent::mi,
        InstanceTraits::fl_events::NativeWindowDisplayStateEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

