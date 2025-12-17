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

#include "AS3_Obj_Display_GraphicsEndFill.h"
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
    GraphicsEndFill::GraphicsEndFill(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::GraphicsEndFill::GraphicsEndFill()$data"
//##protect##"instance::GraphicsEndFill::GraphicsEndFill()$data"
    {
//##protect##"instance::GraphicsEndFill::GraphicsEndFill()$code"
//##protect##"instance::GraphicsEndFill::GraphicsEndFill()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{

    GraphicsEndFill::GraphicsEndFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsEndFill::GraphicsEndFill()"
//##protect##"InstanceTraits::GraphicsEndFill::GraphicsEndFill()"

    }

    void GraphicsEndFill::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsEndFill&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsEndFill::GraphicsEndFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsEndFill::GraphicsEndFill()"
//##protect##"ClassTraits::GraphicsEndFill::GraphicsEndFill()"

    }

    Pickable<Traits> GraphicsEndFill::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsEndFill(vm, AS3::fl_display::GraphicsEndFillCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsEndFillCI));
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
    const TypeInfo* GraphicsEndFillImplements[] = {
        &fl_display::IGraphicsFillTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsEndFillTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsEndFill::InstanceType),
        0,
        0,
        0,
        0,
        "GraphicsEndFill", "flash.display", &fl::ObjectTI,
        GraphicsEndFillImplements
    };

    const ClassInfo GraphicsEndFillCI = {
        &GraphicsEndFillTI,
        ClassTraits::fl_display::GraphicsEndFill::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

