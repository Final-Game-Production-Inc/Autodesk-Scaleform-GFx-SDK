//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Jul, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_MouseCursorEvent.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_gfx
{
    MouseCursorEvent::MouseCursorEvent(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
    , cursor(AS3::DefaultValue<ASString>(GetStringManager()))
    , mouseIdx(0)
//##protect##"instance::MouseCursorEvent::MouseCursorEvent()$data"
//##protect##"instance::MouseCursorEvent::MouseCursorEvent()$data"
    {
//##protect##"instance::MouseCursorEvent::MouseCursorEvent()$code"
        Bubbles    = false;
        Cancelable = true;
//##protect##"instance::MouseCursorEvent::MouseCursorEvent()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    const MemberInfo MouseCursorEvent::mi[MouseCursorEvent::MemberInfoNum] = {
        {"cursor", NULL, OFFSETOF(Instances::fl_gfx::MouseCursorEvent, cursor), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"mouseIdx", NULL, OFFSETOF(Instances::fl_gfx::MouseCursorEvent, mouseIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
    };


    MouseCursorEvent::MouseCursorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::MouseCursorEvent::MouseCursorEvent()"
//##protect##"InstanceTraits::MouseCursorEvent::MouseCursorEvent()"

    }

    void MouseCursorEvent::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<MouseCursorEvent&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_gfx
{
    MouseCursorEvent::MouseCursorEvent(ClassTraits::Traits& t)
    : Class(t)
    , CURSOR_CHANGE("mouseCursorChange")
    {
//##protect##"class_::MouseCursorEvent::MouseCursorEvent()"
//##protect##"class_::MouseCursorEvent::MouseCursorEvent()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo MouseCursorEvent::mi[MouseCursorEvent::MemberInfoNum] = {
        {"CURSOR_CHANGE", NULL, OFFSETOF(Classes::fl_gfx::MouseCursorEvent, CURSOR_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    MouseCursorEvent::MouseCursorEvent(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::MouseCursorEvent::MouseCursorEvent()"
//##protect##"ClassTraits::MouseCursorEvent::MouseCursorEvent()"

    }

    Pickable<Traits> MouseCursorEvent::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MouseCursorEvent(vm, AS3::fl_gfx::MouseCursorEventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::MouseCursorEventCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo MouseCursorEventTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::MouseCursorEvent::InstanceType),
        0,
        ClassTraits::fl_gfx::MouseCursorEvent::MemberInfoNum,
        0,
        InstanceTraits::fl_gfx::MouseCursorEvent::MemberInfoNum,
        "MouseCursorEvent", "scaleform.gfx", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo MouseCursorEventCI = {
        &MouseCursorEventTI,
        ClassTraits::fl_gfx::MouseCursorEvent::MakeClassTraits,
        NULL,
        ClassTraits::fl_gfx::MouseCursorEvent::mi,
        NULL,
        InstanceTraits::fl_gfx::MouseCursorEvent::mi,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

