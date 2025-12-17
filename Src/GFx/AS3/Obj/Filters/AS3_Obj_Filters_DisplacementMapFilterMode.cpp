//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filters_DisplacementMapFilterMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filters_DisplacementMapFilterMode.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_filters
{
    DisplacementMapFilterMode::DisplacementMapFilterMode(ClassTraits::Traits& t)
    : Class(t)
    , CLAMP("clamp")
    , COLOR("color")
    , IGNORE("ignore")
    , WRAP("wrap")
    {
//##protect##"class_::DisplacementMapFilterMode::DisplacementMapFilterMode()"
//##protect##"class_::DisplacementMapFilterMode::DisplacementMapFilterMode()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_filters
{
    const MemberInfo DisplacementMapFilterMode::mi[DisplacementMapFilterMode::MemberInfoNum] = {
        {"CLAMP", NULL, OFFSETOF(Classes::fl_filters::DisplacementMapFilterMode, CLAMP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"COLOR", NULL, OFFSETOF(Classes::fl_filters::DisplacementMapFilterMode, COLOR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"IGNORE", NULL, OFFSETOF(Classes::fl_filters::DisplacementMapFilterMode, IGNORE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"WRAP", NULL, OFFSETOF(Classes::fl_filters::DisplacementMapFilterMode, WRAP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    DisplacementMapFilterMode::DisplacementMapFilterMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::DisplacementMapFilterMode::DisplacementMapFilterMode()"
//##protect##"ClassTraits::DisplacementMapFilterMode::DisplacementMapFilterMode()"

    }

    Pickable<Traits> DisplacementMapFilterMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DisplacementMapFilterMode(vm, AS3::fl_filters::DisplacementMapFilterModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filters::DisplacementMapFilterModeCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_filters
{
    const TypeInfo DisplacementMapFilterModeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_filters::DisplacementMapFilterMode::InstanceType),
        0,
        ClassTraits::fl_filters::DisplacementMapFilterMode::MemberInfoNum,
        0,
        0,
        "DisplacementMapFilterMode", "flash.filters", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DisplacementMapFilterModeCI = {
        &DisplacementMapFilterModeTI,
        ClassTraits::fl_filters::DisplacementMapFilterMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_filters::DisplacementMapFilterMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_filters


}}} // namespace Scaleform { namespace GFx { namespace AS3

