//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_CapsStyle.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_CapsStyle.h"
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
    CapsStyle::CapsStyle(ClassTraits::Traits& t)
    : Class(t)
    , NONE("none")
    , ROUND("round")
    , SQUARE("square")
    {
//##protect##"class_::CapsStyle::CapsStyle()"
//##protect##"class_::CapsStyle::CapsStyle()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo CapsStyle::mi[CapsStyle::MemberInfoNum] = {
        {"NONE", NULL, OFFSETOF(Classes::fl_display::CapsStyle, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ROUND", NULL, OFFSETOF(Classes::fl_display::CapsStyle, ROUND), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SQUARE", NULL, OFFSETOF(Classes::fl_display::CapsStyle, SQUARE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    CapsStyle::CapsStyle(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::CapsStyle::CapsStyle()"
//##protect##"ClassTraits::CapsStyle::CapsStyle()"

    }

    Pickable<Traits> CapsStyle::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) CapsStyle(vm, AS3::fl_display::CapsStyleCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::CapsStyleCI));
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
    const TypeInfo CapsStyleTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::CapsStyle::InstanceType),
        0,
        ClassTraits::fl_display::CapsStyle::MemberInfoNum,
        0,
        0,
        "CapsStyle", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo CapsStyleCI = {
        &CapsStyleTI,
        ClassTraits::fl_display::CapsStyle::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::CapsStyle::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

