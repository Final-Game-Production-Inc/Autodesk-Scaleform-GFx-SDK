//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_FocusDirection.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_FocusDirection.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_display
{
    FocusDirection::FocusDirection(ClassTraits::Traits& t)
    : Class(t)
    , BOTTOM("bottom")
    , NONE("none")
    , TOP("top")
    {
//##protect##"class_::FocusDirection::FocusDirection()"
//##protect##"class_::FocusDirection::FocusDirection()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo FocusDirection::mi[FocusDirection::MemberInfoNum] = {
        {"BOTTOM", NULL, OFFSETOF(Classes::fl_display::FocusDirection, BOTTOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NONE", NULL, OFFSETOF(Classes::fl_display::FocusDirection, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP", NULL, OFFSETOF(Classes::fl_display::FocusDirection, TOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    FocusDirection::FocusDirection(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::FocusDirection::FocusDirection()"
//##protect##"ClassTraits::FocusDirection::FocusDirection()"

    }

    Pickable<Traits> FocusDirection::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FocusDirection(vm, AS3::fl_display::FocusDirectionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::FocusDirectionCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo FocusDirectionTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::FocusDirection::InstanceType),
        0,
        ClassTraits::fl_display::FocusDirection::MemberInfoNum,
        0,
        0,
        "FocusDirection", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo FocusDirectionCI = {
        &FocusDirectionTI,
        ClassTraits::fl_display::FocusDirection::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::FocusDirection::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

