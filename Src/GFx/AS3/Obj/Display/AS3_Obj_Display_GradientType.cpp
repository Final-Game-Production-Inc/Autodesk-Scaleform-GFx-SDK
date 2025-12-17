//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_GradientType.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_GradientType.h"
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
    GradientType::GradientType(ClassTraits::Traits& t)
    : Class(t)
    , LINEAR("linear")
    , RADIAL("radial")
    {
//##protect##"class_::GradientType::GradientType()"
//##protect##"class_::GradientType::GradientType()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo GradientType::mi[GradientType::MemberInfoNum] = {
        {"LINEAR", NULL, OFFSETOF(Classes::fl_display::GradientType, LINEAR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RADIAL", NULL, OFFSETOF(Classes::fl_display::GradientType, RADIAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    GradientType::GradientType(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GradientType::GradientType()"
//##protect##"ClassTraits::GradientType::GradientType()"

    }

    Pickable<Traits> GradientType::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GradientType(vm, AS3::fl_display::GradientTypeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GradientTypeCI));
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
    const TypeInfo GradientTypeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GradientType::InstanceType),
        0,
        ClassTraits::fl_display::GradientType::MemberInfoNum,
        0,
        0,
        "GradientType", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo GradientTypeCI = {
        &GradientTypeTI,
        ClassTraits::fl_display::GradientType::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::GradientType::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

