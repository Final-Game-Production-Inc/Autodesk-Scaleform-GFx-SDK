//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenuItem.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_ContextMenuItem.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_ui
{
    // const UInt16 ContextMenuItem_tito[7] = {
    //    0, 1, 3, 4, 6, 7, 9, 
    // };
    const TypeInfo* ContextMenuItem_tit[10] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_ui::ContextMenuItemTI, 
    };
    const ThunkInfo ContextMenuItem_ti[7] = {
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[0], "caption", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[1], "caption", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[3], "separatorBefore", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[4], "separatorBefore", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[6], "visible", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[7], "visible", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ContextMenuItem_tit[9], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_ui
{

    ContextMenuItem::ContextMenuItem(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::ContextMenuItem::ContextMenuItem()"
//##protect##"ClassTraits::ContextMenuItem::ContextMenuItem()"

    }

    Pickable<Traits> ContextMenuItem::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ContextMenuItem(vm, AS3::fl_ui::ContextMenuItemCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::ContextMenuItemCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_ui
{
    const TypeInfo ContextMenuItemTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_ui::ContextMenuItem::InstanceType),
        0,
        0,
        7,
        0,
        "ContextMenuItem", "flash.ui", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo ContextMenuItemCI = {
        &ContextMenuItemTI,
        ClassTraits::fl_ui::ContextMenuItem::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_ui::ContextMenuItem_ti,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

