//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_NativeDragEvent.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_NativeDragEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Desktop/AS3_Obj_Desktop_NativeDragOptions.h"
#include "../Desktop/AS3_Obj_Desktop_Clipboard.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_events
{
    NativeDragEvent::NativeDragEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::MouseEvent(t)
    , allowedActions()
    , clipboard()
    , dropAction(AS3::DefaultValue<ASString>(GetStringManager()))
//##protect##"instance::NativeDragEvent::NativeDragEvent()$data"
//##protect##"instance::NativeDragEvent::NativeDragEvent()$data"
    {
//##protect##"instance::NativeDragEvent::NativeDragEvent()$code"
//##protect##"instance::NativeDragEvent::NativeDragEvent()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 NativeDragEvent::tito[NativeDragEvent::ThunkInfoNum] = {
    //    0, 1, 
    // };
    const TypeInfo* NativeDragEvent::tit[2] = {
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeDragEvent::ti[NativeDragEvent::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &NativeDragEvent::tit[0], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeDragEvent::tit[1], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo NativeDragEvent::mi[NativeDragEvent::MemberInfoNum] = {
        {"allowedActions", NULL, OFFSETOF(Instances::fl_events::NativeDragEvent, allowedActions), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"clipboard", NULL, OFFSETOF(Instances::fl_events::NativeDragEvent, clipboard), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"dropAction", NULL, OFFSETOF(Instances::fl_events::NativeDragEvent, dropAction), Abc::NS_Public, SlotInfo::BT_String, 0},
    };


    NativeDragEvent::NativeDragEvent(VM& vm, const ClassInfo& ci)
    : fl_events::MouseEvent(vm, ci)
    {
//##protect##"InstanceTraits::NativeDragEvent::NativeDragEvent()"
//##protect##"InstanceTraits::NativeDragEvent::NativeDragEvent()"

    }

    void NativeDragEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<NativeDragEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    NativeDragEvent::NativeDragEvent(ClassTraits::Traits& t)
    : Class(t)
    , NATIVE_DRAG_COMPLETE("nativeDragComplete")
    , NATIVE_DRAG_DROP("nativeDragDrop")
    , NATIVE_DRAG_ENTER("nativeDragEnter")
    , NATIVE_DRAG_EXIT("nativeDragExit")
    , NATIVE_DRAG_OVER("nativeDragOver")
    , NATIVE_DRAG_START("nativeDragStart")
    , NATIVE_DRAG_UPDATE("nativeDragUpdate")
    {
//##protect##"class_::NativeDragEvent::NativeDragEvent()"
//##protect##"class_::NativeDragEvent::NativeDragEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo NativeDragEvent::mi[NativeDragEvent::MemberInfoNum] = {
        {"NATIVE_DRAG_COMPLETE", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_COMPLETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_DROP", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_DROP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_ENTER", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_ENTER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_EXIT", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_EXIT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_OVER", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_OVER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_START", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_START), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NATIVE_DRAG_UPDATE", NULL, OFFSETOF(Classes::fl_events::NativeDragEvent, NATIVE_DRAG_UPDATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeDragEvent::NativeDragEvent(VM& vm, const ClassInfo& ci)
    : fl_events::MouseEvent(vm, ci)
    {
//##protect##"ClassTraits::NativeDragEvent::NativeDragEvent()"
//##protect##"ClassTraits::NativeDragEvent::NativeDragEvent()"

    }

    Pickable<Traits> NativeDragEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeDragEvent(vm, AS3::fl_events::NativeDragEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::NativeDragEventCI));
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
    const TypeInfo NativeDragEventTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_events::NativeDragEvent::InstanceType),
        0,
        ClassTraits::fl_events::NativeDragEvent::MemberInfoNum,
        InstanceTraits::fl_events::NativeDragEvent::ThunkInfoNum,
        InstanceTraits::fl_events::NativeDragEvent::MemberInfoNum,
        "NativeDragEvent", "flash.events", &fl_events::MouseEventTI,
        TypeInfo::None
    };

    const ClassInfo NativeDragEventCI = {
        &NativeDragEventTI,
        ClassTraits::fl_events::NativeDragEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::NativeDragEvent::mi,
        InstanceTraits::fl_events::NativeDragEvent::ti,
        InstanceTraits::fl_events::NativeDragEvent::mi,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

