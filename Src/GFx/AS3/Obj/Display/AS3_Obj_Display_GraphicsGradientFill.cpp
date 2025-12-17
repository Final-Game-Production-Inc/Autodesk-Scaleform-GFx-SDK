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

#include "AS3_Obj_Display_GraphicsGradientFill.h"
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
    GraphicsGradientFill::GraphicsGradientFill(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , alphas()
    , colors()
    , focalPointRatio()
    , interpolationMethod(AS3::DefaultValue<ASString>(GetStringManager()))
    , matrix()
    , ratios()
    , spreadMethod(AS3::DefaultValue<ASString>(GetStringManager()))
    , type(AS3::DefaultValue<ASString>(GetStringManager()))
//##protect##"instance::GraphicsGradientFill::GraphicsGradientFill()$data"
//##protect##"instance::GraphicsGradientFill::GraphicsGradientFill()$data"
    {
//##protect##"instance::GraphicsGradientFill::GraphicsGradientFill()$code"
//##protect##"instance::GraphicsGradientFill::GraphicsGradientFill()$code"
    }


//##protect##"instance$methods"
void GraphicsGradientFill::AS3Constructor(unsigned argc, const Value* argv)
{
    // Assign defaults
    type = "linear";
    colors = 0;
    alphas = 0;
    ratios = 0;
    matrix = 0;
    spreadMethod = "pad";
    interpolationMethod = "rgb";
    focalPointRatio = 0.0;

    if (argc >= 1)
    {
        if (!argv[0].Convert2String(type))
            return;

        // Check whether this is a valid option.
        if (type != "linear" &&
            type != "radial")
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("type")));
            return;
        }
    }

    if (argc >= 2)
    {
        Instances::fl::Object* pobj = (Instances::fl::Object*)argv[1].GetObject();
        if (pobj == 0 ||
            (pobj->GetTraitsType() == Traits_Array && 
            pobj->GetTraits().IsInstanceTraits()))
        {
            colors = reinterpret_cast<Instances::fl::Array*>(pobj);
        }
        else
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eCheckTypeFailedError, GetVM() 
                SF_DEBUG_ARG(GetVM().GetClassTraits(argv[1]).GetName().ToCStr())
                SF_DEBUG_ARG("Array")));
            return;
        }
    }

    if (argc >= 3)
    {
        Instances::fl::Object* pobj = (Instances::fl::Object*)argv[2].GetObject();
        if (pobj == 0 ||
            (pobj->GetTraitsType() == Traits_Array && 
            pobj->GetTraits().IsInstanceTraits()))
        {
            alphas = reinterpret_cast<Instances::fl::Array*>(argv[2].GetObject());
        }
        else
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eCheckTypeFailedError, GetVM() 
                SF_DEBUG_ARG(GetVM().GetClassTraits(argv[2]).GetName().ToCStr())
                SF_DEBUG_ARG("Array")));
            return;
        }
    }

    if (argc >= 4)
    {
        Instances::fl::Object* pobj = (Instances::fl::Object*)argv[3].GetObject();
        if (pobj == 0 ||
            (pobj->GetTraitsType() == Traits_Array && 
            pobj->GetTraits().IsInstanceTraits()))
        {
            ratios = reinterpret_cast<Instances::fl::Array*>(argv[3].GetObject());
        }
        else
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eCheckTypeFailedError, GetVM() 
                SF_DEBUG_ARG(GetVM().GetClassTraits(argv[3]).GetName().ToCStr())
                SF_DEBUG_ARG("Array")));
            return;
        }
    }

    if (argc >= 5)
    {
        matrix = reinterpret_cast<Instances::fl_geom::Matrix*>(argv[4].GetObject());
    }

    if (argc >= 6)
    {
        if (!argv[5].Convert2String(spreadMethod))
            return;

        if (spreadMethod != "pad" && spreadMethod != "reflect" && spreadMethod != "repeat")
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("spreadMethod")));
            return;
        }
    }

    if (argc >= 7)
    {
        if (!argv[6].Convert2String(interpolationMethod))
            return;

        if (interpolationMethod != "rgb" && interpolationMethod != "linearRGB")
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidEnumError, GetVM() SF_DEBUG_ARG("interpolationMethod")));
            return;
        }
    }

    if (argc >= 8 && !argv[7].Convert2Number(focalPointRatio)) return;
}
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    const MemberInfo GraphicsGradientFill::mi[GraphicsGradientFill::MemberInfoNum] = {
        {"alphas", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, alphas), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"colors", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, colors), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"focalPointRatio", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, focalPointRatio), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"interpolationMethod", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, interpolationMethod), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"matrix", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, matrix), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"ratios", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, ratios), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"spreadMethod", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, spreadMethod), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"type", NULL, OFFSETOF(Instances::fl_display::GraphicsGradientFill, type), Abc::NS_Public, SlotInfo::BT_String, 0},
    };


    GraphicsGradientFill::GraphicsGradientFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::GraphicsGradientFill::GraphicsGradientFill()"
//##protect##"InstanceTraits::GraphicsGradientFill::GraphicsGradientFill()"

    }

    void GraphicsGradientFill::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<GraphicsGradientFill&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    GraphicsGradientFill::GraphicsGradientFill(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GraphicsGradientFill::GraphicsGradientFill()"
//##protect##"ClassTraits::GraphicsGradientFill::GraphicsGradientFill()"

    }

    Pickable<Traits> GraphicsGradientFill::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GraphicsGradientFill(vm, AS3::fl_display::GraphicsGradientFillCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::GraphicsGradientFillCI));
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
    const TypeInfo* GraphicsGradientFillImplements[] = {
        &fl_display::IGraphicsFillTI,
        &fl_display::IGraphicsDataTI,
        NULL
    };

    const TypeInfo GraphicsGradientFillTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::GraphicsGradientFill::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_display::GraphicsGradientFill::MemberInfoNum,
        "GraphicsGradientFill", "flash.display", &fl::ObjectTI,
        GraphicsGradientFillImplements
    };

    const ClassInfo GraphicsGradientFillCI = {
        &GraphicsGradientFillTI,
        ClassTraits::fl_display::GraphicsGradientFill::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_display::GraphicsGradientFill::mi,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

