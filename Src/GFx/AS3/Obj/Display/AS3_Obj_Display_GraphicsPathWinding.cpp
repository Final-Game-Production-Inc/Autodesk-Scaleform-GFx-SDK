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

#include "AS3_Obj_Display_GraphicsPathWinding.h"
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
    GraphicsPathWinding::GraphicsPathWinding(ClassTraits::Traits& t)
    : Class(t)
    , EVEN_ODD("evenOdd")
    , NON_ZERO("nonZero")
    {
//##protect##"class_::GraphicsPathWinding::GraphicsPathWinding()"
//##protect##"class_::GraphicsPathWinding::GraphicsPathWinding()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo GraphicsPathWinding::mi[GraphicsPathWinding::MemberInfoNum] = {
        {"EVEN_ODD", NULL, OFFSETOF(Classes::fl_display::GraphicsPathWinding, EVEN_ODD), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NON_ZERO", NULL, OFFSETOF(Classes::fl_display::GraphicsPathWinding, NON_ZERO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    GraphicsPathWinding::GraphicsPathWinding(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsPathWinding::GraphicsPathWinding()"
//##protect##"ClassTraits::GraphicsPathWinding::GraphicsPathWinding()"

    }

    Pickable<Traits> GraphicsPathWinding::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsPathWinding(vm, AS3::fl_display::GraphicsPathWindingCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsPathWindingCI));
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
    const TypeInfo GraphicsPathWindingTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsPathWinding::InstanceType),
        0,
        ClassTraits::fl_display::GraphicsPathWinding::MemberInfoNum,
        0,
        0,
        "GraphicsPathWinding", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo GraphicsPathWindingCI = {
        &GraphicsPathWindingTI,
        ClassTraits::fl_display::GraphicsPathWinding::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::GraphicsPathWinding::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

