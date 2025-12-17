//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_MorphShape.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_MorphShape.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_display
{
    MorphShape::MorphShape(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::MorphShape::MorphShape()$data"
//##protect##"instance::MorphShape::MorphShape()$data"
    {
//##protect##"instance::MorphShape::MorphShape()$code"
//##protect##"instance::MorphShape::MorphShape()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{

    MorphShape::MorphShape(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::MorphShape::MorphShape()"
        SetTraitsType(Traits_MorphShape);
//##protect##"InstanceTraits::MorphShape::MorphShape()"

    }

    void MorphShape::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<MorphShape&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    MorphShape::MorphShape(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::MorphShape::MorphShape()"
        SetTraitsType(Traits_MorphShape);
//##protect##"ClassTraits::MorphShape::MorphShape()"

    }

    Pickable<Traits> MorphShape::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) MorphShape(vm, AS3::fl_display::MorphShapeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::MorphShapeCI));
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
    const TypeInfo MorphShapeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::MorphShape::InstanceType),
        0,
        0,
        0,
        0,
        "MorphShape", "flash.display", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo MorphShapeCI = {
        &MorphShapeTI,
        ClassTraits::fl_display::MorphShape::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

