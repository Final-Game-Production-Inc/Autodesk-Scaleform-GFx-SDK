//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_TextEventEx.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_TextEventEx.h"
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
    TextEventEx::TextEventEx(InstanceTraits::Traits& t)
    : Instances::fl_events::TextEvent(t)
    , controllerIdx()
    , buttonIdx()
//##protect##"instance::TextEventEx::TextEventEx()$data"
//##protect##"instance::TextEventEx::TextEventEx()$data"
    {
//##protect##"instance::TextEventEx::TextEventEx()$code"
//##protect##"instance::TextEventEx::TextEventEx()$code"
    }


//##protect##"instance$methods"
    AS3::Object* TextEventEx::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).TextEventExClass; 
    }

    SPtr<Instances::fl_events::Event> TextEventEx::Clone() const
    {
        SPtr<Instances::fl_events::Event> p = Event::Clone();
        TextEventEx* pe = static_cast<TextEventEx*>(p.GetPtr());
        pe->Text = Text;
        pe->controllerIdx = controllerIdx;
        pe->buttonIdx = buttonIdx;
        return p;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    const MemberInfo TextEventEx::mi[TextEventEx::MemberInfoNum] = {
        {"controllerIdx", NULL, OFFSETOF(Instances::fl_gfx::TextEventEx, controllerIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"buttonIdx", NULL, OFFSETOF(Instances::fl_gfx::TextEventEx, buttonIdx), Abc::NS_Public, SlotInfo::BT_UInt, 0},
    };


    TextEventEx::TextEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::TextEvent(vm, ci)
    {
//##protect##"InstanceTraits::TextEventEx::TextEventEx()"
//##protect##"InstanceTraits::TextEventEx::TextEventEx()"

    }

    void TextEventEx::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<TextEventEx&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_gfx
{
    TextEventEx::TextEventEx(ClassTraits::Traits& t)
    : Class(t)
    , LINK_MOUSE_OVER("linkMouseOver")
    , LINK_MOUSE_OUT("linkMouseOut")
    {
//##protect##"class_::TextEventEx::TextEventEx()"
//##protect##"class_::TextEventEx::TextEventEx()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo TextEventEx::mi[TextEventEx::MemberInfoNum] = {
        {"LINK_MOUSE_OVER", NULL, OFFSETOF(Classes::fl_gfx::TextEventEx, LINK_MOUSE_OVER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LINK_MOUSE_OUT", NULL, OFFSETOF(Classes::fl_gfx::TextEventEx, LINK_MOUSE_OUT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    TextEventEx::TextEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::TextEvent(vm, ci)
    {
//##protect##"ClassTraits::TextEventEx::TextEventEx()"
//##protect##"ClassTraits::TextEventEx::TextEventEx()"

    }

    Pickable<Traits> TextEventEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextEventEx(vm, AS3::fl_gfx::TextEventExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::TextEventExCI));
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
    const TypeInfo TextEventExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::TextEventEx::InstanceType),
        0,
        ClassTraits::fl_gfx::TextEventEx::MemberInfoNum,
        0,
        InstanceTraits::fl_gfx::TextEventEx::MemberInfoNum,
        "TextEventEx", "scaleform.gfx", &fl_events::TextEventTI,
        TypeInfo::None
    };

    const ClassInfo TextEventExCI = {
        &TextEventExTI,
        ClassTraits::fl_gfx::TextEventEx::MakeClassTraits,
        NULL,
        ClassTraits::fl_gfx::TextEventEx::mi,
        NULL,
        InstanceTraits::fl_gfx::TextEventEx::mi,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

