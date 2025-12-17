//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_NativeWindowType.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_NativeWindowType.h"
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
    NativeWindowType::NativeWindowType(ClassTraits::Traits& t)
    : Class(t)
    , LIGHTWEIGHT("lightweight")
    , NORMAL("normal")
    , UTILITY("utility")
    {
//##protect##"class_::NativeWindowType::NativeWindowType()"
//##protect##"class_::NativeWindowType::NativeWindowType()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo NativeWindowType::mi[NativeWindowType::MemberInfoNum] = {
        {"LIGHTWEIGHT", NULL, OFFSETOF(Classes::fl_display::NativeWindowType, LIGHTWEIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NORMAL", NULL, OFFSETOF(Classes::fl_display::NativeWindowType, NORMAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UTILITY", NULL, OFFSETOF(Classes::fl_display::NativeWindowType, UTILITY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    NativeWindowType::NativeWindowType(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeWindowType::NativeWindowType()"
//##protect##"ClassTraits::NativeWindowType::NativeWindowType()"

    }

    Pickable<Traits> NativeWindowType::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeWindowType(vm, AS3::fl_display::NativeWindowTypeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::NativeWindowTypeCI));
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
    const TypeInfo NativeWindowTypeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::NativeWindowType::InstanceType),
        0,
        ClassTraits::fl_display::NativeWindowType::MemberInfoNum,
        0,
        0,
        "NativeWindowType", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeWindowTypeCI = {
        &NativeWindowTypeTI,
        ClassTraits::fl_display::NativeWindowType::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::NativeWindowType::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

