//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_MouseEventEx.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_MouseEventEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_gfx
{
    MouseEventEx::MouseEventEx(InstanceTraits::Traits& t)
    : Instances::fl_events::MouseEvent(t)
    , mouseIdx()
    , buttonIdx()
    , nestingIdx()
//##protect##"instance::MouseEventEx::MouseEventEx()$data"
//##protect##"instance::MouseEventEx::MouseEventEx()$data"
    {
//##protect##"instance::MouseEventEx::MouseEventEx()$code"
//##protect##"instance::MouseEventEx::MouseEventEx()$code"
    }


//##protect##"instance$methods"
    AS3::Object* MouseEventEx::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).MouseEventExClass;
    }

    SPtr<Instances::fl_events::Event> MouseEventEx::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        MouseEventEx* pe = static_cast<MouseEventEx*>(p.GetPtr());
        pe->AltKey          = AltKey;
        pe->CtrlKey         = CtrlKey;
        pe->ShiftKey        = ShiftKey;
        pe->Delta           = Delta;
        pe->RelatedObj      = RelatedObj;
        pe->LocalX          = LocalX;
        pe->LocalY          = LocalY;
        pe->ButtonsMask     = ButtonsMask;
        pe->mouseIdx        = mouseIdx;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    const MemberInfo MouseEventEx::mi[MouseEventEx::MemberInfoNum] = {
        {"mouseIdx", NULL, OFFSETOF(Instances::fl_gfx::MouseEventEx, mouseIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"buttonIdx", NULL, OFFSETOF(Instances::fl_gfx::MouseEventEx, buttonIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"nestingIdx", NULL, OFFSETOF(Instances::fl_gfx::MouseEventEx, nestingIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
    };


    MouseEventEx::MouseEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::MouseEvent(vm, ci)
    {
//##protect##"InstanceTraits::MouseEventEx::MouseEventEx()"
//##protect##"InstanceTraits::MouseEventEx::MouseEventEx()"

    }

    void MouseEventEx::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<MouseEventEx&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_gfx
{
    MouseEventEx::MouseEventEx(ClassTraits::Traits& t)
    : Class(t)
    , LEFT_BUTTON(0)
    , RIGHT_BUTTON(1)
    , MIDDLE_BUTTON(2)
    {
//##protect##"class_::MouseEventEx::MouseEventEx()"
//##protect##"class_::MouseEventEx::MouseEventEx()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo MouseEventEx::mi[MouseEventEx::MemberInfoNum] = {
        {"LEFT_BUTTON", NULL, OFFSETOF(Classes::fl_gfx::MouseEventEx, LEFT_BUTTON), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"RIGHT_BUTTON", NULL, OFFSETOF(Classes::fl_gfx::MouseEventEx, RIGHT_BUTTON), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"MIDDLE_BUTTON", NULL, OFFSETOF(Classes::fl_gfx::MouseEventEx, MIDDLE_BUTTON), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    MouseEventEx::MouseEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::MouseEvent(vm, ci)
    {
//##protect##"ClassTraits::MouseEventEx::MouseEventEx()"
//##protect##"ClassTraits::MouseEventEx::MouseEventEx()"

    }

    Pickable<Traits> MouseEventEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MouseEventEx(vm, AS3::fl_gfx::MouseEventExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::MouseEventExCI));
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
    const TypeInfo MouseEventExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::MouseEventEx::InstanceType),
        0,
        ClassTraits::fl_gfx::MouseEventEx::MemberInfoNum,
        0,
        InstanceTraits::fl_gfx::MouseEventEx::MemberInfoNum,
        "MouseEventEx", "scaleform.gfx", &fl_events::MouseEventTI,
        TypeInfo::None
    };

    const ClassInfo MouseEventExCI = {
        &MouseEventExTI,
        ClassTraits::fl_gfx::MouseEventEx::MakeClassTraits,
        NULL,
        ClassTraits::fl_gfx::MouseEventEx::mi,
        NULL,
        InstanceTraits::fl_gfx::MouseEventEx::mi,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

