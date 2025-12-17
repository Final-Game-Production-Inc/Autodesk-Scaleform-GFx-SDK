//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_IBitmapDrawable.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_IBitmapDrawable.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"


namespace ClassTraits { namespace fl_display
{

    IBitmapDrawable::IBitmapDrawable(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IBitmapDrawable::IBitmapDrawable()"
//##protect##"ClassTraits::IBitmapDrawable::IBitmapDrawable()"

    }

    Pickable<Traits> IBitmapDrawable::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IBitmapDrawable(vm, AS3::fl_display::IBitmapDrawableCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::IBitmapDrawableCI));
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
    const TypeInfo IBitmapDrawableTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_display::IBitmapDrawable::InstanceType),
        0,
        0,
        0,
        0,
        "IBitmapDrawable", "flash.display", NULL,
        TypeInfo::None
    };

    const ClassInfo IBitmapDrawableCI = {
        &IBitmapDrawableTI,
        ClassTraits::fl_display::IBitmapDrawable::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

