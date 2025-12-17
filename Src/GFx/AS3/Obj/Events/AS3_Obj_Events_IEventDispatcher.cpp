//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_IEventDispatcher.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_IEventDispatcher.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Events_Event.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 IEventDispatcher_tito[5] = {
    //    0, 6, 8, 10, 14, 
    // };
    const TypeInfo* IEventDispatcher_tit[16] = {
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_events::EventTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo IEventDispatcher_ti[5] = {
        {ThunkInfo::EmptyFunc, &IEventDispatcher_tit[0], "addEventListener", "flash.events:IEventDispatcher", Abc::NS_Public, CT_Method, 2, 5, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IEventDispatcher_tit[6], "dispatchEvent", "flash.events:IEventDispatcher", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IEventDispatcher_tit[8], "hasEventListener", "flash.events:IEventDispatcher", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IEventDispatcher_tit[10], "removeEventListener", "flash.events:IEventDispatcher", Abc::NS_Public, CT_Method, 2, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IEventDispatcher_tit[14], "willTrigger", "flash.events:IEventDispatcher", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_events
{

    IEventDispatcher::IEventDispatcher(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IEventDispatcher::IEventDispatcher()"
//##protect##"ClassTraits::IEventDispatcher::IEventDispatcher()"

    }

    Pickable<Traits> IEventDispatcher::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IEventDispatcher(vm, AS3::fl_events::IEventDispatcherCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::IEventDispatcherCI));
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
    const TypeInfo IEventDispatcherTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_events::IEventDispatcher::InstanceType),
        0,
        0,
        5,
        0,
        "IEventDispatcher", "flash.events", NULL,
        TypeInfo::None
    };

    const ClassInfo IEventDispatcherCI = {
        &IEventDispatcherTI,
        ClassTraits::fl_events::IEventDispatcher::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_events::IEventDispatcher_ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

