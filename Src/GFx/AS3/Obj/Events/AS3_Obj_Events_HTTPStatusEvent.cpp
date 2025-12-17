//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_HTTPStatusEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_HTTPStatusEvent.h"
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
    // const UInt16 HTTPStatusEvent_tito[7] = {
    //    0, 1, 3, 4, 6, 7, 8, 
    // };
    const TypeInfo* HTTPStatusEvent_tit[9] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo HTTPStatusEvent_ti[7] = {
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[0], "responseHeaders", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[1], "responseHeaders", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[3], "responseURL", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[4], "responseURL", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[6], "status", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[7], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTTPStatusEvent_tit[8], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    HTTPStatusEvent::HTTPStatusEvent(ClassTraits::Traits& t)
    : Class(t)
    , HTTP_RESPONSE_STATUS("httpResponseStatus")
    , HTTP_STATUS("httpStatus")
    {
//##protect##"class_::HTTPStatusEvent::HTTPStatusEvent()"
//##protect##"class_::HTTPStatusEvent::HTTPStatusEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo HTTPStatusEvent::mi[HTTPStatusEvent::MemberInfoNum] = {
        {"HTTP_RESPONSE_STATUS", NULL, OFFSETOF(Classes::fl_events::HTTPStatusEvent, HTTP_RESPONSE_STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HTTP_STATUS", NULL, OFFSETOF(Classes::fl_events::HTTPStatusEvent, HTTP_STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    HTTPStatusEvent::HTTPStatusEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::HTTPStatusEvent::HTTPStatusEvent()"
//##protect##"ClassTraits::HTTPStatusEvent::HTTPStatusEvent()"

    }

    Pickable<Traits> HTTPStatusEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTTPStatusEvent(vm, AS3::fl_events::HTTPStatusEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::HTTPStatusEventCI));
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
    const TypeInfo HTTPStatusEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::HTTPStatusEvent::InstanceType),
        0,
        ClassTraits::fl_events::HTTPStatusEvent::MemberInfoNum,
        7,
        0,
        "HTTPStatusEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo HTTPStatusEventCI = {
        &HTTPStatusEventTI,
        ClassTraits::fl_events::HTTPStatusEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::HTTPStatusEvent::mi,
        InstanceTraits::fl_events::HTTPStatusEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

