//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Nov, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_GraphicsSolidFill.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_display
{
    GraphicsSolidFill::GraphicsSolidFill(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , alpha()
    , color()
//##protect##"instance::GraphicsSolidFill::GraphicsSolidFill()$data"
//##protect##"instance::GraphicsSolidFill::GraphicsSolidFill()$data"
    {
//##protect##"instance::GraphicsSolidFill::GraphicsSolidFill()$code"
//##protect##"instance::GraphicsSolidFill::GraphicsSolidFill()$code"
    }


//##protect##"instance$methods"
void GraphicsSolidFill::AS3Constructor(unsigned argc, const Value* argv)
{
    color = 0;
    alpha = 1.0;

    // TODO: argument count exception
    if (argc>=1)
    {
        if (!argv[0].Convert2UInt32(color))
        {
            // TODO: typecheck
        }
    }

    if (argc>=2)
    {
        if (!argv[1].Convert2Number(alpha))
        {
            // TODO: typecheck
        }
    }
}
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    const MemberInfo GraphicsSolidFill::mi[GraphicsSolidFill::MemberInfoNum] = {
        {"alpha", NULL, OFFSETOF(Instances::fl_display::GraphicsSolidFill, alpha), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"color", NULL, OFFSETOF(Instances::fl_display::GraphicsSolidFill, color), Abc::NS_Public, SlotInfo::BT_UInt, 0},
    };


    GraphicsSolidFill::GraphicsSolidFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsSolidFill::GraphicsSolidFill()"
//##protect##"InstanceTraits::GraphicsSolidFill::GraphicsSolidFill()"

    }

    void GraphicsSolidFill::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsSolidFill&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsSolidFill::GraphicsSolidFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsSolidFill::GraphicsSolidFill()"
//##protect##"ClassTraits::GraphicsSolidFill::GraphicsSolidFill()"

    }

    Pickable<Traits> GraphicsSolidFill::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsSolidFill(vm, AS3::fl_display::GraphicsSolidFillCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsSolidFillCI));
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
    const TypeInfo* GraphicsSolidFillImplements[] = {
        &fl_display::IGraphicsFillTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsSolidFillTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsSolidFill::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_display::GraphicsSolidFill::MemberInfoNum,
        "GraphicsSolidFill", "flash.display", &fl::ObjectTI,
        GraphicsSolidFillImplements
    };

    const ClassInfo GraphicsSolidFillCI = {
        &GraphicsSolidFillTI,
        ClassTraits::fl_display::GraphicsSolidFill::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_display::GraphicsSolidFill::mi,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

