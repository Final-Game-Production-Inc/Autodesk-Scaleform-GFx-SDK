//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowSystemChrome.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeWindowSystemChrome.h"
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
    NativeWindowSystemChrome::NativeWindowSystemChrome(ClassTraits::Traits& t)
    : Class(t)
    , NONE("none")
    , STANDARD("standard")
    {
//##protect##"class_::NativeWindowSystemChrome::NativeWindowSystemChrome()"
//##protect##"class_::NativeWindowSystemChrome::NativeWindowSystemChrome()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo NativeWindowSystemChrome::mi[NativeWindowSystemChrome::MemberInfoNum] = {
        {"NONE", NULL, OFFSETOF(Classes::fl_display::NativeWindowSystemChrome, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STANDARD", NULL, OFFSETOF(Classes::fl_display::NativeWindowSystemChrome, STANDARD), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeWindowSystemChrome::NativeWindowSystemChrome(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowSystemChrome::NativeWindowSystemChrome()"
//##protect##"ClassTraits::NativeWindowSystemChrome::NativeWindowSystemChrome()"

    }

    Pickable<Traits> NativeWindowSystemChrome::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowSystemChrome(vm, AS3::fl_display::NativeWindowSystemChromeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeWindowSystemChromeCI));
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
    const TypeInfo NativeWindowSystemChromeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::NativeWindowSystemChrome::InstanceType),
        0,
        ClassTraits::fl_display::NativeWindowSystemChrome::MemberInfoNum,
        0,
        0,
        "NativeWindowSystemChrome", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowSystemChromeCI = {
        &NativeWindowSystemChromeTI,
        ClassTraits::fl_display::NativeWindowSystemChrome::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::NativeWindowSystemChrome::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

