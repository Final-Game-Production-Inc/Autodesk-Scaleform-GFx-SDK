//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowDisplayState.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeWindowDisplayState.h"
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
    NativeWindowDisplayState::NativeWindowDisplayState(ClassTraits::Traits& t)
    : Class(t)
    , MAXIMIZED("maximized")
    , MINIMIZED("minimized")
    , NORMAL("normal")
    {
//##protect##"class_::NativeWindowDisplayState::NativeWindowDisplayState()"
//##protect##"class_::NativeWindowDisplayState::NativeWindowDisplayState()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo NativeWindowDisplayState::mi[NativeWindowDisplayState::MemberInfoNum] = {
        {"MAXIMIZED", NULL, OFFSETOF(Classes::fl_display::NativeWindowDisplayState, MAXIMIZED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MINIMIZED", NULL, OFFSETOF(Classes::fl_display::NativeWindowDisplayState, MINIMIZED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NORMAL", NULL, OFFSETOF(Classes::fl_display::NativeWindowDisplayState, NORMAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeWindowDisplayState::NativeWindowDisplayState(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowDisplayState::NativeWindowDisplayState()"
//##protect##"ClassTraits::NativeWindowDisplayState::NativeWindowDisplayState()"

    }

    Pickable<Traits> NativeWindowDisplayState::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowDisplayState(vm, AS3::fl_display::NativeWindowDisplayStateCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeWindowDisplayStateCI));
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
    const TypeInfo NativeWindowDisplayStateTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::NativeWindowDisplayState::InstanceType),
        0,
        ClassTraits::fl_display::NativeWindowDisplayState::MemberInfoNum,
        0,
        0,
        "NativeWindowDisplayState", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowDisplayStateCI = {
        &NativeWindowDisplayStateTI,
        ClassTraits::fl_display::NativeWindowDisplayState::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::NativeWindowDisplayState::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

