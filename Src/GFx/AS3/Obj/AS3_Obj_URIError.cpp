//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_URIError.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_URIError.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"


namespace ClassTraits { namespace fl
{

    URIError::URIError(VM& vm, const ClassInfo& ci)
    : fl::Error(vm, ci)
    {
//##protect##"ClassTraits::URIError::URIError()"
//##protect##"ClassTraits::URIError::URIError()"

    }

    Pickable<Traits> URIError::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URIError(vm, AS3::fl::URIErrorCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::URIErrorCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo URIErrorTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(ClassTraits::fl::URIError::InstanceType),
        0,
        0,
        0,
        0,
        "URIError", "", &fl::ErrorTI,
        TypeInfo::None
    };

    const ClassInfo URIErrorCI = {
        &URIErrorTI,
        ClassTraits::fl::URIError::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

