//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_SyncEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_SyncEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 SyncEvent_tito[4] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* SyncEvent_tit[5] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo SyncEvent_ti[4] = {
        {ThunkInfo::EmptyFunc, &SyncEvent_tit[0], "changeList", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SyncEvent_tit[1], "changeList", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SyncEvent_tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SyncEvent_tit[4], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    SyncEvent::SyncEvent(ClassTraits::Traits& t)
    : Class(t)
    , SYNC("sync")
    {
//##protect##"class_::SyncEvent::SyncEvent()"
//##protect##"class_::SyncEvent::SyncEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo SyncEvent::mi[SyncEvent::MemberInfoNum] = {
        {"SYNC", NULL, OFFSETOF(Classes::fl_events::SyncEvent, SYNC), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SyncEvent::SyncEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::SyncEvent::SyncEvent()"
//##protect##"ClassTraits::SyncEvent::SyncEvent()"

    }

    Pickable<Traits> SyncEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SyncEvent(vm, AS3::fl_events::SyncEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::SyncEventCI));
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
    const TypeInfo SyncEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::SyncEvent::InstanceType),
        0,
        ClassTraits::fl_events::SyncEvent::MemberInfoNum,
        4,
        0,
        "SyncEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo SyncEventCI = {
        &SyncEventTI,
        ClassTraits::fl_events::SyncEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::SyncEvent::mi,
        InstanceTraits::fl_events::SyncEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

