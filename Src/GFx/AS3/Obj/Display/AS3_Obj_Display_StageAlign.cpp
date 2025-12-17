//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_StageAlign.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_StageAlign.h"
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
    StageAlign::StageAlign(ClassTraits::Traits& t)
    : Class(t)
    , BOTTOM("B")
    , BOTTOM_LEFT("BL")
    , BOTTOM_RIGHT("BR")
    , LEFT("L")
    , RIGHT("R")
    , TOP("T")
    , TOP_LEFT("TL")
    , TOP_RIGHT("TR")
    {
//##protect##"class_::StageAlign::StageAlign()"
//##protect##"class_::StageAlign::StageAlign()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo StageAlign::mi[StageAlign::MemberInfoNum] = {
        {"BOTTOM", NULL, OFFSETOF(Classes::fl_display::StageAlign, BOTTOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"BOTTOM_LEFT", NULL, OFFSETOF(Classes::fl_display::StageAlign, BOTTOM_LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"BOTTOM_RIGHT", NULL, OFFSETOF(Classes::fl_display::StageAlign, BOTTOM_RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LEFT", NULL, OFFSETOF(Classes::fl_display::StageAlign, LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT", NULL, OFFSETOF(Classes::fl_display::StageAlign, RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP", NULL, OFFSETOF(Classes::fl_display::StageAlign, TOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP_LEFT", NULL, OFFSETOF(Classes::fl_display::StageAlign, TOP_LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP_RIGHT", NULL, OFFSETOF(Classes::fl_display::StageAlign, TOP_RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StageAlign::StageAlign(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::StageAlign::StageAlign()"
//##protect##"ClassTraits::StageAlign::StageAlign()"

    }

    Pickable<Traits> StageAlign::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StageAlign(vm, AS3::fl_display::StageAlignCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::StageAlignCI));
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
    const TypeInfo StageAlignTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::StageAlign::InstanceType),
        0,
        ClassTraits::fl_display::StageAlign::MemberInfoNum,
        0,
        0,
        "StageAlign", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo StageAlignCI = {
        &StageAlignTI,
        ClassTraits::fl_display::StageAlign::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::StageAlign::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

