//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_InteractiveIcon.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_InteractiveIcon.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 InteractiveIcon_tito[4] = {
    //    0, 1, 3, 4, 
    // };
    const TypeInfo* InteractiveIcon_tit[5] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
    };
    const ThunkInfo InteractiveIcon_ti[4] = {
        {ThunkInfo::EmptyFunc, &InteractiveIcon_tit[0], "bitmaps", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &InteractiveIcon_tit[1], "bitmaps", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &InteractiveIcon_tit[3], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &InteractiveIcon_tit[4], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    InteractiveIcon::InteractiveIcon(VM& vm, const ClassInfo& ci)
    : fl_desktop::Icon(vm, ci)
    {
//##protect##"ClassTraits::InteractiveIcon::InteractiveIcon()"
//##protect##"ClassTraits::InteractiveIcon::InteractiveIcon()"

    }

    Pickable<Traits> InteractiveIcon::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InteractiveIcon(vm, AS3::fl_desktop::InteractiveIconCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::InteractiveIconCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_desktop
{
    const TypeInfo InteractiveIconTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::InteractiveIcon::InstanceType),
        0,
        0,
        4,
        0,
        "InteractiveIcon", "flash.desktop", &fl_desktop::IconTI,
        TypeInfo::None
    };

    const ClassInfo InteractiveIconCI = {
        &InteractiveIconTI,
        ClassTraits::fl_desktop::InteractiveIcon::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::InteractiveIcon_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

