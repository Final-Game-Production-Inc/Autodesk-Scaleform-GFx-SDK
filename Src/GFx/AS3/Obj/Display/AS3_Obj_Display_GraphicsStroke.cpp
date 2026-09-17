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

#include "AS3_Obj_Display_GraphicsStroke.h"
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
    GraphicsStroke::GraphicsStroke(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , caps(AS3::DefaultValue<ASString>(GetStringManager()))
    , fill()
    , joints(AS3::DefaultValue<ASString>(GetStringManager()))
    , miterLimit()
    , pixelHinting()
    , scaleMode(AS3::DefaultValue<ASString>(GetStringManager()))
    , thickness()
//##protect##"instance::GraphicsStroke::GraphicsStroke()$data"
//##protect##"instance::GraphicsStroke::GraphicsStroke()$data"
    {
//##protect##"instance::GraphicsStroke::GraphicsStroke()$code"
//##protect##"instance::GraphicsStroke::GraphicsStroke()$code"
    }


//##protect##"instance$methods"
    void GraphicsStroke::AS3Constructor(unsigned argc, const Value* argv)
    {
        // defaults
        scaleMode = "normal";
        caps = "none";
        joints = "round";
        miterLimit = 3.0;
        thickness = NumberUtil::NaN();

        if (argc > 0)
        {
            if (!argv[0].Convert2Number(thickness))
                return;
        }
        if (argc > 1)
        {
            pixelHinting = argv[1].Convert2Boolean();
        }
        if (argc > 2)
        {
            if (!argv[2].Convert2String(scaleMode))
                return;

            if (scaleMode != "normal" && scaleMode != "none" && scaleMode != "vertical" && scaleMode != "horizontal")
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("scaleMode")));
                return;
            }
        }

        if (argc > 3)
        {
            if (!argv[3].Convert2String(caps))
                return;

            if (caps != "none" && caps != "round" && caps != "square")
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("caps")));
                return;
            }
        }

        if (argc > 4)
        {
            if (!argv[4].Convert2String(joints))
                return;

            if (joints != "bevel" && joints != "miter" && joints != "round")
            {
                GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("caps")));
                return;
            }
        }

        if (argc > 5 && !argv[5].Convert2Number(miterLimit)) 
            return;
        if (argc > 6)
        {
            fill = reinterpret_cast<Instances::fl::Object*>(argv[6].GetObject());
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    const MemberInfo GraphicsStroke::mi[GraphicsStroke::MemberInfoNum] = {
        {"caps", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, caps), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"fill", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, fill), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"joints", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, joints), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"miterLimit", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, miterLimit), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"pixelHinting", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, pixelHinting), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"scaleMode", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, scaleMode), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"thickness", NULL, OFFSETOF(Instances::fl_display::GraphicsStroke, thickness), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    GraphicsStroke::GraphicsStroke(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsStroke::GraphicsStroke()"
//##protect##"InstanceTraits::GraphicsStroke::GraphicsStroke()"

    }

    void GraphicsStroke::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsStroke&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsStroke::GraphicsStroke(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsStroke::GraphicsStroke()"
//##protect##"ClassTraits::GraphicsStroke::GraphicsStroke()"

    }

    Pickable<Traits> GraphicsStroke::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsStroke(vm, AS3::fl_display::GraphicsStrokeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsStrokeCI));
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
    const TypeInfo* GraphicsStrokeImplements[] = {
        &fl_display::IGraphicsPathTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsStrokeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsStroke::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_display::GraphicsStroke::MemberInfoNum,
        "GraphicsStroke", "flash.display", &fl::ObjectTI,
        GraphicsStrokeImplements
    };

    const ClassInfo GraphicsStrokeCI = {
        &GraphicsStrokeTI,
        ClassTraits::fl_display::GraphicsStroke::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_display::GraphicsStroke::mi,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

