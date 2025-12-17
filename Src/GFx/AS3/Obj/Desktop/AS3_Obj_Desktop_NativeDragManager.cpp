//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NativeDragManager.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_NativeDragManager.h"
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
    // const UInt16 NativeDragManager_tito[1] = {
    //    0, 
    // };
    const TypeInfo* NativeDragManager_tit[2] = {
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeDragManager_ti[1] = {
        {ThunkInfo::EmptyFunc, &NativeDragManager_tit[0], "dropAction", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_desktop
{
    NativeDragManager::NativeDragManager(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::NativeDragManager::NativeDragManager()"
//##protect##"class_::NativeDragManager::NativeDragManager()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_desktop
{
    // const UInt16 NativeDragManager_tito[2] = {
    //    0, 2, 
    // };
    const TypeInfo* NativeDragManager_tit[8] = {
        NULL, &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, &AS3::fl_desktop::ClipboardTI, &AS3::fl_display::BitmapDataTI, &AS3::fl_geom::PointTI, &AS3::fl_desktop::NativeDragOptionsTI, 
    };
    const ThunkInfo NativeDragManager_ti[2] = {
        {ThunkInfo::EmptyFunc, &NativeDragManager_tit[0], "acceptDragDrop", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &NativeDragManager_tit[2], "doDrag", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 0, NULL},
    };

    NativeDragManager::NativeDragManager(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeDragManager::NativeDragManager()"
//##protect##"ClassTraits::NativeDragManager::NativeDragManager()"

    }

    Pickable<Traits> NativeDragManager::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeDragManager(vm, AS3::fl_desktop::NativeDragManagerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::NativeDragManagerCI));
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
    const TypeInfo NativeDragManagerTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::NativeDragManager::InstanceType),
        2,
        0,
        1,
        0,
        "NativeDragManager", "flash.desktop", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeDragManagerCI = {
        &NativeDragManagerTI,
        ClassTraits::fl_desktop::NativeDragManager::MakeClassTraits,
        ClassTraits::fl_desktop::NativeDragManager_ti,
        NULL,
        InstanceTraits::fl_desktop::NativeDragManager_ti,
        NULL,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

