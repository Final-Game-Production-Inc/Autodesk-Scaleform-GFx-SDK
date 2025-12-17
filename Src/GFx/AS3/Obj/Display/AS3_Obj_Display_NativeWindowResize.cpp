//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowResize.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeWindowResize.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_display
{
    NativeWindowResize::NativeWindowResize(ClassTraits::Traits& t)
    : Class(t)
    , BOTTOM("B")
    , BOTTOM_LEFT("BL")
    , BOTTOM_RIGHT("BR")
    , LEFT("L")
    , NONE("")
    , RIGHT("R")
    , TOP("T")
    , TOP_LEFT("TL")
    , TOP_RIGHT("TR")
    {
//##protect##"class_::NativeWindowResize::NativeWindowResize()"
//##protect##"class_::NativeWindowResize::NativeWindowResize()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo NativeWindowResize::mi[NativeWindowResize::MemberInfoNum] = {
        {"BOTTOM", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, BOTTOM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"BOTTOM_LEFT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, BOTTOM_LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"BOTTOM_RIGHT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, BOTTOM_RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LEFT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NONE", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, TOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP_LEFT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, TOP_LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TOP_RIGHT", NULL, OFFSETOF(Classes::fl_display::NativeWindowResize, TOP_RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeWindowResize::NativeWindowResize(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowResize::NativeWindowResize()"
//##protect##"ClassTraits::NativeWindowResize::NativeWindowResize()"

    }

    Pickable<Traits> NativeWindowResize::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowResize(vm, AS3::fl_display::NativeWindowResizeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeWindowResizeCI));
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
    const TypeInfo NativeWindowResizeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::NativeWindowResize::InstanceType),
        0,
        ClassTraits::fl_display::NativeWindowResize::MemberInfoNum,
        0,
        0,
        "NativeWindowResize", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowResizeCI = {
        &NativeWindowResizeTI,
        ClassTraits::fl_display::NativeWindowResize::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::NativeWindowResize::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

