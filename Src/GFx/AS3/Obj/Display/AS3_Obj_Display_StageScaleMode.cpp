//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_StageScaleMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_StageScaleMode.h"
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
    StageScaleMode::StageScaleMode(ClassTraits::Traits& t)
    : Class(t)
    , EXACT_FIT("exactFit")
    , NO_BORDER("noBorder")
    , NO_SCALE("noScale")
    , SHOW_ALL("showAll")
    {
//##protect##"class_::StageScaleMode::StageScaleMode()"
//##protect##"class_::StageScaleMode::StageScaleMode()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo StageScaleMode::mi[StageScaleMode::MemberInfoNum] = {
        {"EXACT_FIT", NULL, OFFSETOF(Classes::fl_display::StageScaleMode, EXACT_FIT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NO_BORDER", NULL, OFFSETOF(Classes::fl_display::StageScaleMode, NO_BORDER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NO_SCALE", NULL, OFFSETOF(Classes::fl_display::StageScaleMode, NO_SCALE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SHOW_ALL", NULL, OFFSETOF(Classes::fl_display::StageScaleMode, SHOW_ALL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StageScaleMode::StageScaleMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::StageScaleMode::StageScaleMode()"
//##protect##"ClassTraits::StageScaleMode::StageScaleMode()"

    }

    Pickable<Traits> StageScaleMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StageScaleMode(vm, AS3::fl_display::StageScaleModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::StageScaleModeCI));
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
    const TypeInfo StageScaleModeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::StageScaleMode::InstanceType),
        0,
        ClassTraits::fl_display::StageScaleMode::MemberInfoNum,
        0,
        0,
        "StageScaleMode", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo StageScaleModeCI = {
        &StageScaleModeTI,
        ClassTraits::fl_display::StageScaleMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::StageScaleMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

