//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_ScreenMouseEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_ScreenMouseEvent.h"
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
    // const UInt16 ScreenMouseEvent_tito[4] = {
    //    0, 1, 2, 3, 
    // };
    const TypeInfo* ScreenMouseEvent_tit[4] = {
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo ScreenMouseEvent_ti[4] = {
        {ThunkInfo::EmptyFunc, &ScreenMouseEvent_tit[0], "screenX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ScreenMouseEvent_tit[1], "screenY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ScreenMouseEvent_tit[2], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ScreenMouseEvent_tit[3], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_events
{

    ScreenMouseEvent::ScreenMouseEvent(VM& vm, const ClassInfo& ci)
    : fl_events::MouseEvent(vm, ci)
    {
//##protect##"ClassTraits::ScreenMouseEvent::ScreenMouseEvent()"
//##protect##"ClassTraits::ScreenMouseEvent::ScreenMouseEvent()"

    }

    Pickable<Traits> ScreenMouseEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ScreenMouseEvent(vm, AS3::fl_events::ScreenMouseEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::ScreenMouseEventCI));
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
    const TypeInfo ScreenMouseEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::ScreenMouseEvent::InstanceType),
        0,
        0,
        4,
        0,
        "ScreenMouseEvent", "flash.events", &fl_events::MouseEventTI,
        TypeInfo::None
    };

    const ClassInfo ScreenMouseEventCI = {
        &ScreenMouseEventTI,
        ClassTraits::fl_events::ScreenMouseEvent::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_events::ScreenMouseEvent_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

