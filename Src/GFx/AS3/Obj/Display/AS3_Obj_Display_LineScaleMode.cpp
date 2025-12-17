//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_LineScaleMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_LineScaleMode.h"
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
    LineScaleMode::LineScaleMode(ClassTraits::Traits& t)
    : Class(t)
    , HORIZONTAL("horizontal")
    , NONE("none")
    , NORMAL("normal")
    , VERTICAL("vertical")
    {
//##protect##"class_::LineScaleMode::LineScaleMode()"
//##protect##"class_::LineScaleMode::LineScaleMode()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo LineScaleMode::mi[LineScaleMode::MemberInfoNum] = {
        {"HORIZONTAL", NULL, OFFSETOF(Classes::fl_display::LineScaleMode, HORIZONTAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NONE", NULL, OFFSETOF(Classes::fl_display::LineScaleMode, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NORMAL", NULL, OFFSETOF(Classes::fl_display::LineScaleMode, NORMAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VERTICAL", NULL, OFFSETOF(Classes::fl_display::LineScaleMode, VERTICAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    LineScaleMode::LineScaleMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::LineScaleMode::LineScaleMode()"
//##protect##"ClassTraits::LineScaleMode::LineScaleMode()"

    }

    Pickable<Traits> LineScaleMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) LineScaleMode(vm, AS3::fl_display::LineScaleModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::LineScaleModeCI));
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
    const TypeInfo LineScaleModeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::LineScaleMode::InstanceType),
        0,
        ClassTraits::fl_display::LineScaleMode::MemberInfoNum,
        0,
        0,
        "LineScaleMode", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo LineScaleModeCI = {
        &LineScaleModeTI,
        ClassTraits::fl_display::LineScaleMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::LineScaleMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

