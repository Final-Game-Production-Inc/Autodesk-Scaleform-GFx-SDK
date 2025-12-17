//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_MouseCursor.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_MouseCursor.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_ui
{
    MouseCursor::MouseCursor(ClassTraits::Traits& t)
    : Class(t)
    , ARROW("arrow")
    , AUTO("auto")
    , BUTTON("button")
    , HAND("hand")
    , IBEAM("ibeam")
    {
//##protect##"class_::MouseCursor::MouseCursor()"
//##protect##"class_::MouseCursor::MouseCursor()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_ui
{
    const MemberInfo MouseCursor::mi[MouseCursor::MemberInfoNum] = {
        {"ARROW", NULL, OFFSETOF(Classes::fl_ui::MouseCursor, ARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"AUTO", NULL, OFFSETOF(Classes::fl_ui::MouseCursor, AUTO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"BUTTON", NULL, OFFSETOF(Classes::fl_ui::MouseCursor, BUTTON), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HAND", NULL, OFFSETOF(Classes::fl_ui::MouseCursor, HAND), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"IBEAM", NULL, OFFSETOF(Classes::fl_ui::MouseCursor, IBEAM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    MouseCursor::MouseCursor(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::MouseCursor::MouseCursor()"
//##protect##"ClassTraits::MouseCursor::MouseCursor()"

    }

    Pickable<Traits> MouseCursor::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MouseCursor(vm, AS3::fl_ui::MouseCursorCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::MouseCursorCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_ui
{
    const TypeInfo MouseCursorTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::MouseCursor::InstanceType),
        0,
        ClassTraits::fl_ui::MouseCursor::MemberInfoNum,
        0,
        0,
        "MouseCursor", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo MouseCursorCI = {
        &MouseCursorTI,
        ClassTraits::fl_ui::MouseCursor::MakeClassTraits,
        NULL,
        ClassTraits::fl_ui::MouseCursor::mi,
        NULL,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

