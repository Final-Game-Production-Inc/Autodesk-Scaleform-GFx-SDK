//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_InterpolationMethod.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_InterpolationMethod.h"
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
    InterpolationMethod::InterpolationMethod(ClassTraits::Traits& t)
    : Class(t)
    , LINEAR_RGB("linearRGB")
    , RGB("rgb")
    {
//##protect##"class_::InterpolationMethod::InterpolationMethod()"
//##protect##"class_::InterpolationMethod::InterpolationMethod()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo InterpolationMethod::mi[InterpolationMethod::MemberInfoNum] = {
        {"LINEAR_RGB", NULL, OFFSETOF(Classes::fl_display::InterpolationMethod, LINEAR_RGB), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RGB", NULL, OFFSETOF(Classes::fl_display::InterpolationMethod, RGB), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    InterpolationMethod::InterpolationMethod(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::InterpolationMethod::InterpolationMethod()"
//##protect##"ClassTraits::InterpolationMethod::InterpolationMethod()"

    }

    Pickable<Traits> InterpolationMethod::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InterpolationMethod(vm, AS3::fl_display::InterpolationMethodCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::InterpolationMethodCI));
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
    const TypeInfo InterpolationMethodTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::InterpolationMethod::InstanceType),
        0,
        ClassTraits::fl_display::InterpolationMethod::MemberInfoNum,
        0,
        0,
        "InterpolationMethod", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo InterpolationMethodCI = {
        &InterpolationMethodTI,
        ClassTraits::fl_display::InterpolationMethod::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::InterpolationMethod::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

