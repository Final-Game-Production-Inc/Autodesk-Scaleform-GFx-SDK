//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_SystemTrayIcon.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_SystemTrayIcon.h"
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
    // const UInt16 SystemTrayIcon_tito[7] = {
    //    0, 1, 3, 4, 6, 7, 9, 
    // };
    const TypeInfo* SystemTrayIcon_tit[10] = {
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::NativeMenuTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
    };
    const ThunkInfo SystemTrayIcon_ti[7] = {
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[0], "bitmaps", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[1], "bitmaps", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[3], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[4], "menu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[6], "tooltip", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[7], "tooltip", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &SystemTrayIcon_tit[9], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_desktop
{
    SystemTrayIcon::SystemTrayIcon(ClassTraits::Traits& t)
    : Class(t)
    , MAX_TIP_LENGTH(63)
    {
//##protect##"class_::SystemTrayIcon::SystemTrayIcon()"
//##protect##"class_::SystemTrayIcon::SystemTrayIcon()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_desktop
{
    const MemberInfo SystemTrayIcon::mi[SystemTrayIcon::MemberInfoNum] = {
        {"MAX_TIP_LENGTH", NULL, OFFSETOF(Classes::fl_desktop::SystemTrayIcon, MAX_TIP_LENGTH), Abc::NS_Public, SlotInfo::BT_Number, 1},
    };


    SystemTrayIcon::SystemTrayIcon(VM& vm, const ClassInfo& ci)
    : fl_desktop::InteractiveIcon(vm, ci)
    {
//##protect##"ClassTraits::SystemTrayIcon::SystemTrayIcon()"
//##protect##"ClassTraits::SystemTrayIcon::SystemTrayIcon()"

    }

    Pickable<Traits> SystemTrayIcon::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SystemTrayIcon(vm, AS3::fl_desktop::SystemTrayIconCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::SystemTrayIconCI));
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
    const TypeInfo SystemTrayIconTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::SystemTrayIcon::InstanceType),
        0,
        ClassTraits::fl_desktop::SystemTrayIcon::MemberInfoNum,
        7,
        0,
        "SystemTrayIcon", "flash.desktop", &fl_desktop::InteractiveIconTI,
        TypeInfo::None
    };

    const ClassInfo SystemTrayIconCI = {
        &SystemTrayIconTI,
        ClassTraits::fl_desktop::SystemTrayIcon::MakeClassTraits,
        NULL,
        ClassTraits::fl_desktop::SystemTrayIcon::mi,
        InstanceTraits::fl_desktop::SystemTrayIcon_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

