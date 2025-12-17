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

#include "AS3_Obj_Display_GraphicsBitmapFill.h"
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
    GraphicsBitmapFill::GraphicsBitmapFill(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , bitmapData()
    , matrix()
    , repeat()
    , smooth()
//##protect##"instance::GraphicsBitmapFill::GraphicsBitmapFill()$data"
//##protect##"instance::GraphicsBitmapFill::GraphicsBitmapFill()$data"
    {
//##protect##"instance::GraphicsBitmapFill::GraphicsBitmapFill()$code"
        repeat = true;
        smooth = false;
//##protect##"instance::GraphicsBitmapFill::GraphicsBitmapFill()$code"
    }


//##protect##"instance$methods"
void GraphicsBitmapFill::AS3Constructor(unsigned argc, const Value* argv)
{
    if (argc > 4)
    {
        // TODO: throw correct error.
        return;
    }

    if (argc >= 1)
    {
        // TODO: type check and throw correct error.
        bitmapData = reinterpret_cast<Instances::fl_display::BitmapData*>(argv[0].GetObject());
    }
    if (argc >= 2)
    {
        // TODO: type check and throw correct error.
        matrix = reinterpret_cast<Instances::fl_geom::Matrix*>(argv[1].GetObject());
    }

    if (argc >= 3)
        repeat = argv[2].Convert2Boolean();
    if (argc == 4)
        smooth = argv[3].Convert2Boolean();
}
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    const MemberInfo GraphicsBitmapFill::mi[GraphicsBitmapFill::MemberInfoNum] = {
        {"bitmapData", NULL, OFFSETOF(Instances::fl_display::GraphicsBitmapFill, bitmapData), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"matrix", NULL, OFFSETOF(Instances::fl_display::GraphicsBitmapFill, matrix), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"repeat", NULL, OFFSETOF(Instances::fl_display::GraphicsBitmapFill, repeat), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"smooth", NULL, OFFSETOF(Instances::fl_display::GraphicsBitmapFill, smooth), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    GraphicsBitmapFill::GraphicsBitmapFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsBitmapFill::GraphicsBitmapFill()"
//##protect##"InstanceTraits::GraphicsBitmapFill::GraphicsBitmapFill()"

    }

    void GraphicsBitmapFill::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsBitmapFill&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsBitmapFill::GraphicsBitmapFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsBitmapFill::GraphicsBitmapFill()"
//##protect##"ClassTraits::GraphicsBitmapFill::GraphicsBitmapFill()"

    }

    Pickable<Traits> GraphicsBitmapFill::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsBitmapFill(vm, AS3::fl_display::GraphicsBitmapFillCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsBitmapFillCI));
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
    const TypeInfo* GraphicsBitmapFillImplements[] = {
        &fl_display::IGraphicsPathTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsBitmapFillTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsBitmapFill::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_display::GraphicsBitmapFill::MemberInfoNum,
        "GraphicsBitmapFill", "flash.display", &fl::ObjectTI,
        GraphicsBitmapFillImplements
    };

    const ClassInfo GraphicsBitmapFillCI = {
        &GraphicsBitmapFillTI,
        ClassTraits::fl_display::GraphicsBitmapFill::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_display::GraphicsBitmapFill::mi,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

