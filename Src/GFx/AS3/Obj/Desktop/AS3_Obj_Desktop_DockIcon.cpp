//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_DockIcon.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_DockIcon.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class NativeMenu;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 DockIcon_tito[6] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* DockIcon_tit[9] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::NativeMenuTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::StringTI, 
    };
    const Abc::ConstValue DockIcon_dva[1] = {
        {Abc::CONSTANT_Utf8, 3}, 
    };
    const ThunkInfo DockIcon_ti[6] = {
        {ThunkInfo::EmptyFunc, &DockIcon_tit[0], "bitmaps", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DockIcon_tit[1], "bitmaps", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DockIcon_tit[3], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DockIcon_tit[4], "menu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DockIcon_tit[6], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &DockIcon_tit[7], "bounce", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &DockIcon_dva[0]},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    DockIcon::DockIcon(VM& vm, const ClassInfo& ci)
    : fl_desktop::InteractiveIcon(vm, ci)
    {
//##protect##"ClassTraits::DockIcon::DockIcon()"
//##protect##"ClassTraits::DockIcon::DockIcon()"

    }

    Pickable<Traits> DockIcon::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DockIcon(vm, AS3::fl_desktop::DockIconCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::DockIconCI));
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
    const TypeInfo DockIconTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::DockIcon::InstanceType),
        0,
        0,
        6,
        0,
        "DockIcon", "flash.desktop", &fl_desktop::InteractiveIconTI,
        TypeInfo::None
    };

    const ClassInfo DockIconCI = {
        &DockIconTI,
        ClassTraits::fl_desktop::DockIcon::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::DockIcon_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

