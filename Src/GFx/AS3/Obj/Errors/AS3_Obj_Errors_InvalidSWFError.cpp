//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Errors_InvalidSWFError.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Errors_InvalidSWFError.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"


namespace ClassTraits { namespace fl_errors
{

    InvalidSWFError::InvalidSWFError(VM& vm, const ClassInfo& ci)
    : fl::Error(vm, ci)
    {
//##protect##"ClassTraits::InvalidSWFError::InvalidSWFError()"
//##protect##"ClassTraits::InvalidSWFError::InvalidSWFError()"

    }

    Pickable<Traits> InvalidSWFError::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InvalidSWFError(vm, AS3::fl_errors::InvalidSWFErrorCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_errors::InvalidSWFErrorCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_errors
{
    const TypeInfo InvalidSWFErrorTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl_errors::InvalidSWFError::InstanceType),
        0,
        0,
        0,
        0,
        "InvalidSWFError", "flash.errors", &fl::ErrorTI,
        TypeInfo::None
    };

    const ClassInfo InvalidSWFErrorCI = {
        &InvalidSWFErrorTI,
        ClassTraits::fl_errors::InvalidSWFError::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_errors


}}} // namespace Scaleform { namespace GFx { namespace AS3

