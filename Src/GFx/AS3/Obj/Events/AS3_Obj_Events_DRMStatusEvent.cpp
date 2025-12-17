//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_DRMStatusEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_DRMStatusEvent.h"
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
    // const UInt16 DRMStatusEvent_tito[8] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 
    // };
    const TypeInfo* DRMStatusEvent_tit[8] = {
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::ObjectTI, 
        &AS3::fl::DateTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo DRMStatusEvent_ti[8] = {
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[0], "detail", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[1], "isAnonymous", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[2], "isAvailableOffline", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[3], "offlineLeasePeriod", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[4], "policies", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[5], "voucherEndDate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[6], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DRMStatusEvent_tit[7], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    DRMStatusEvent::DRMStatusEvent(ClassTraits::Traits& t)
    : Class(t)
    , DRM_STATUS("drmStatus")
    {
//##protect##"class_::DRMStatusEvent::DRMStatusEvent()"
//##protect##"class_::DRMStatusEvent::DRMStatusEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo DRMStatusEvent::mi[DRMStatusEvent::MemberInfoNum] = {
        {"DRM_STATUS", NULL, OFFSETOF(Classes::fl_events::DRMStatusEvent, DRM_STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    DRMStatusEvent::DRMStatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::DRMStatusEvent::DRMStatusEvent()"
//##protect##"ClassTraits::DRMStatusEvent::DRMStatusEvent()"

    }

    Pickable<Traits> DRMStatusEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DRMStatusEvent(vm, AS3::fl_events::DRMStatusEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::DRMStatusEventCI));
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
    const TypeInfo DRMStatusEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::DRMStatusEvent::InstanceType),
        0,
        ClassTraits::fl_events::DRMStatusEvent::MemberInfoNum,
        8,
        0,
        "DRMStatusEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo DRMStatusEventCI = {
        &DRMStatusEventTI,
        ClassTraits::fl_events::DRMStatusEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::DRMStatusEvent::mi,
        InstanceTraits::fl_events::DRMStatusEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

