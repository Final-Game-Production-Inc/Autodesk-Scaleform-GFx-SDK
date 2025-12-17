//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_Icon.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_Icon.h"
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
    // const UInt16 Icon_tito[2] = {
    //    0, 1, 
    // };
    const TypeInfo* Icon_tit[3] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
    };
    const ThunkInfo Icon_ti[2] = {
        {ThunkInfo::EmptyFunc, &Icon_tit[0], "bitmaps", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Icon_tit[1], "bitmaps", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    Icon::Icon(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Icon::Icon()"
//##protect##"ClassTraits::Icon::Icon()"

    }

    Pickable<Traits> Icon::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Icon(vm, AS3::fl_desktop::IconCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::IconCI));
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
    const TypeInfo IconTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::Icon::InstanceType),
        0,
        0,
        2,
        0,
        "Icon", "flash.desktop", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo IconCI = {
        &IconTI,
        ClassTraits::fl_desktop::Icon::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::Icon_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

