//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Apr, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_GraphicsPathCommand.h"
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
    GraphicsPathCommand::GraphicsPathCommand(ClassTraits::Traits& t)
    : Class(t)
    , CUBIC_CURVE_TO(6)
    , CURVE_TO(3)
    , LINE_TO(2)
    , MOVE_TO(1)
    , NO_OP(0)
    , WIDE_LINE_TO(5)
    , WIDE_MOVE_TO(4)
    {
//##protect##"class_::GraphicsPathCommand::GraphicsPathCommand()"
//##protect##"class_::GraphicsPathCommand::GraphicsPathCommand()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo GraphicsPathCommand::mi[GraphicsPathCommand::MemberInfoNum] = {
        {"CUBIC_CURVE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, CUBIC_CURVE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"CURVE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, CURVE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"LINE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, LINE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"MOVE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, MOVE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"NO_OP", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, NO_OP), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"WIDE_LINE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, WIDE_LINE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"WIDE_MOVE_TO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathCommand, WIDE_MOVE_TO), Abc::NS_Public, SlotInfo::BT_Int, 1},
    };


    GraphicsPathCommand::GraphicsPathCommand(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsPathCommand::GraphicsPathCommand()"
//##protect##"ClassTraits::GraphicsPathCommand::GraphicsPathCommand()"

    }

    Pickable<Traits> GraphicsPathCommand::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsPathCommand(vm, AS3::fl_display::GraphicsPathCommandCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsPathCommandCI));
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
    const TypeInfo GraphicsPathCommandTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsPathCommand::InstanceType),
        0,
        ClassTraits::fl_display::GraphicsPathCommand::MemberInfoNum,
        0,
        0,
        "GraphicsPathCommand", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo GraphicsPathCommandCI = {
        &GraphicsPathCommandTI,
        ClassTraits::fl_display::GraphicsPathCommand::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::GraphicsPathCommand::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

