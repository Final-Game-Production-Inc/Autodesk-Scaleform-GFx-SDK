//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_BrowserInvokeEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_BrowserInvokeEvent.h"
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
    // const UInt16 BrowserInvokeEvent_tito[6] = {
    //    0, 1, 2, 3, 4, 5, 
    // };
    const TypeInfo* BrowserInvokeEvent_tit[6] = {
        &AS3::fl::ArrayTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
    };
    const ThunkInfo BrowserInvokeEvent_ti[6] = {
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[0], "arguments", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[1], "isHTTPS", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[2], "isUserEvent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[3], "sandboxType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[4], "securityDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &BrowserInvokeEvent_tit[5], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    BrowserInvokeEvent::BrowserInvokeEvent(ClassTraits::Traits& t)
    : Class(t)
    , BROWSER_INVOKE("browserInvoke")
    {
//##protect##"class_::BrowserInvokeEvent::BrowserInvokeEvent()"
//##protect##"class_::BrowserInvokeEvent::BrowserInvokeEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo BrowserInvokeEvent::mi[BrowserInvokeEvent::MemberInfoNum] = {
        {"BROWSER_INVOKE", NULL, OFFSETOF(Classes::fl_events::BrowserInvokeEvent, BROWSER_INVOKE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    BrowserInvokeEvent::BrowserInvokeEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::BrowserInvokeEvent::BrowserInvokeEvent()"
//##protect##"ClassTraits::BrowserInvokeEvent::BrowserInvokeEvent()"

    }

    Pickable<Traits> BrowserInvokeEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) BrowserInvokeEvent(vm, AS3::fl_events::BrowserInvokeEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::BrowserInvokeEventCI));
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
    const TypeInfo BrowserInvokeEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::BrowserInvokeEvent::InstanceType),
        0,
        ClassTraits::fl_events::BrowserInvokeEvent::MemberInfoNum,
        6,
        0,
        "BrowserInvokeEvent", "flash.events", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo BrowserInvokeEventCI = {
        &BrowserInvokeEventTI,
        ClassTraits::fl_events::BrowserInvokeEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::BrowserInvokeEvent::mi,
        InstanceTraits::fl_events::BrowserInvokeEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

