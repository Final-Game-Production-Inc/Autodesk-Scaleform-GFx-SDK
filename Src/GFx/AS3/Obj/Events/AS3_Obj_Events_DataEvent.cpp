//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_DataEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_DataEvent.h"
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
    // const UInt16 DataEvent_tito[4] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* DataEvent_tit[5] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo DataEvent_ti[4] = {
        {ThunkInfo::EmptyFunc, &DataEvent_tit[0], "data", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DataEvent_tit[1], "data", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DataEvent_tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DataEvent_tit[4], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    DataEvent::DataEvent(ClassTraits::Traits& t)
    : Class(t)
    , DATA("data")
    , UPLOAD_COMPLETE_DATA("uploadCompleteData")
    {
//##protect##"class_::DataEvent::DataEvent()"
//##protect##"class_::DataEvent::DataEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo DataEvent::mi[DataEvent::MemberInfoNum] = {
        {"DATA", NULL, OFFSETOF(Classes::fl_events::DataEvent, DATA), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UPLOAD_COMPLETE_DATA", NULL, OFFSETOF(Classes::fl_events::DataEvent, UPLOAD_COMPLETE_DATA), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    DataEvent::DataEvent(VM& vm, const ClassInfo& ci)
    : fl_events::TextEvent(vm, ci)
    {
//##protect##"ClassTraits::DataEvent::DataEvent()"
//##protect##"ClassTraits::DataEvent::DataEvent()"

    }

    Pickable<Traits> DataEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DataEvent(vm, AS3::fl_events::DataEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::DataEventCI));
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
    const TypeInfo DataEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::DataEvent::InstanceType),
        0,
        ClassTraits::fl_events::DataEvent::MemberInfoNum,
        4,
        0,
        "DataEvent", "flash.events", &fl_events::TextEventTI,
        TypeInfo::None
    };

    const ClassInfo DataEventCI = {
        &DataEventTI,
        ClassTraits::fl_events::DataEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::DataEvent::mi,
        InstanceTraits::fl_events::DataEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

