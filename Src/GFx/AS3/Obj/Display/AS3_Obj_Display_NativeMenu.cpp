//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeMenu.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeMenu.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class NativeMenuItem;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 NativeMenu_tito[14] = {
    //    0, 1, 2, 4, 7, 10, 14, 16, 20, 22, 24, 26, 28, 30, 
    // };
    const TypeInfo* NativeMenu_tit[33] = {
        &AS3::fl::ArrayTI, 
        &AS3::fl::int_TI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl_display::NativeMenuItemTI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl_display::NativeMenuItemTI, &AS3::fl::int_TI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl_display::NativeMenuTI, &AS3::fl::StringTI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl_display::NativeMenuTI, &AS3::fl::int_TI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::NativeMenuItemTI, 
        NULL, &AS3::fl_display::StageTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl::int_TI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, &AS3::fl_display::NativeMenuItemTI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl_display::NativeMenuItemTI, 
        &AS3::fl_display::NativeMenuItemTI, &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::NativeMenuItemTI, &AS3::fl::int_TI, 
    };
    const ThunkInfo NativeMenu_ti[14] = {
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[0], "items", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[1], "numItems", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[2], "addItem", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[4], "addItemAt", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[7], "addSubmenu", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[10], "addSubmenuAt", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[14], "containsItem", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[16], "display", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[20], "getItemAt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[22], "getItemByName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[24], "getItemIndex", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[26], "removeItem", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[28], "removeItemAt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeMenu_tit[30], "setItemIndex", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    NativeMenu::NativeMenu(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::NativeMenu::NativeMenu()"
//##protect##"ClassTraits::NativeMenu::NativeMenu()"

    }

    Pickable<Traits> NativeMenu::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeMenu(vm, AS3::fl_display::NativeMenuCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeMenuCI));
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
    const TypeInfo NativeMenuTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_display::NativeMenu::InstanceType),
        0,
        0,
        14,
        0,
        "NativeMenu", "flash.display", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo NativeMenuCI = {
        &NativeMenuTI,
        ClassTraits::fl_display::NativeMenu::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::NativeMenu_ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

