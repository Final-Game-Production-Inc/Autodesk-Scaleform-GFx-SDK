//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_Accessibility.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Accessibility_Accessibility.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_accessibility
{
    Accessibility::Accessibility(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Accessibility::Accessibility()"
//##protect##"class_::Accessibility::Accessibility()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_accessibility
{
    // const UInt16 Accessibility_tito[2] = {
    //    0, 1, 
    // };
    const TypeInfo* Accessibility_tit[2] = {
        &AS3::fl::BooleanTI, 
        NULL, 
    };
    const ThunkInfo Accessibility_ti[2] = {
        {ThunkInfo::EmptyFunc, &Accessibility_tit[0], "active", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Accessibility_tit[1], "updateProperties", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Accessibility::Accessibility(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Accessibility::Accessibility()"
//##protect##"ClassTraits::Accessibility::Accessibility()"

    }

    Pickable<Traits> Accessibility::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Accessibility(vm, AS3::fl_accessibility::AccessibilityCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_accessibility::AccessibilityCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_accessibility
{
    const TypeInfo AccessibilityTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_accessibility::Accessibility::InstanceType),
        2,
        0,
        0,
        0,
        "Accessibility", "flash.accessibility", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo AccessibilityCI = {
        &AccessibilityTI,
        ClassTraits::fl_accessibility::Accessibility::MakeClassTraits,
        ClassTraits::fl_accessibility::Accessibility_ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_accessibility


}}} // namespace Scaleform { namespace GFx { namespace AS3

