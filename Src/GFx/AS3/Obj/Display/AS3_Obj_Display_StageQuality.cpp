//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_StageQuality.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_StageQuality.h"
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
    StageQuality::StageQuality(ClassTraits::Traits& t)
    : Class(t)
    , BEST("best")
    , HIGH("high")
    , LOW("low")
    , MEDIUM("medium")
    {
//##protect##"class_::StageQuality::StageQuality()"
//##protect##"class_::StageQuality::StageQuality()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo StageQuality::mi[StageQuality::MemberInfoNum] = {
        {"BEST", NULL, OFFSETOF(Classes::fl_display::StageQuality, BEST), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HIGH", NULL, OFFSETOF(Classes::fl_display::StageQuality, HIGH), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LOW", NULL, OFFSETOF(Classes::fl_display::StageQuality, LOW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MEDIUM", NULL, OFFSETOF(Classes::fl_display::StageQuality, MEDIUM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StageQuality::StageQuality(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::StageQuality::StageQuality()"
//##protect##"ClassTraits::StageQuality::StageQuality()"

    }

    Pickable<Traits> StageQuality::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StageQuality(vm, AS3::fl_display::StageQualityCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::StageQualityCI));
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
    const TypeInfo StageQualityTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::StageQuality::InstanceType),
        0,
        ClassTraits::fl_display::StageQuality::MemberInfoNum,
        0,
        0,
        "StageQuality", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo StageQualityCI = {
        &StageQualityTI,
        ClassTraits::fl_display::StageQuality::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::StageQuality::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

