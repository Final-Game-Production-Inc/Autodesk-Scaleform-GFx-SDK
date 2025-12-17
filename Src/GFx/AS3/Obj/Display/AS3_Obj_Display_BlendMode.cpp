//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_BlendMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_BlendMode.h"
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
    BlendMode::BlendMode(ClassTraits::Traits& t)
    : Class(t)
    , ADD("add")
    , ALPHA("alpha")
    , DARKEN("darken")
    , DIFFERENCE("difference")
    , ERASE("erase")
    , HARDLIGHT("hardlight")
    , INVERT("invert")
    , LAYER("layer")
    , LIGHTEN("lighten")
    , MULTIPLY("multiply")
    , NORMAL("normal")
    , OVERLAY("overlay")
    , SCREEN("screen")
    , SUBTRACT("subtract")
    {
//##protect##"class_::BlendMode::BlendMode()"
//##protect##"class_::BlendMode::BlendMode()"
    }
//##protect##"class_$methods"
Render::BlendMode BlendMode::GetBlendMode(const ASString& value)
{
    Render::BlendMode blend = Render::Blend_None;
    if (value == "normal")
        blend = Render::Blend_Normal;
    else if (value == "add")
        blend = Render::Blend_Add;
    else if (value == "alpha")
        blend = Render::Blend_Alpha;
    else if (value == "multiply")
        blend = Render::Blend_Multiply;
    else if (value == "subtract")
        blend = Render::Blend_Subtract;
    else if (value == "layer")
        blend = Render::Blend_Layer;
    else if (value == "screen")
        blend = Render::Blend_Screen;
    else if (value == "lighten")
        blend = Render::Blend_Lighten;
    else if (value == "darken")
        blend = Render::Blend_Darken;
    else if (value == "difference")
        blend = Render::Blend_Difference;
    else if (value == "invert")
        blend = Render::Blend_Invert;
    else if (value == "erase")
        blend = Render::Blend_Erase;
    else if (value == "overlay")
        blend = Render::Blend_Overlay;
    else if (value == "hardlight")
        blend = Render::Blend_HardLight;    
    return blend;
}

//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo BlendMode::mi[BlendMode::MemberInfoNum] = {
        {"ADD", NULL, OFFSETOF(Classes::fl_display::BlendMode, ADD), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ALPHA", NULL, OFFSETOF(Classes::fl_display::BlendMode, ALPHA), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DARKEN", NULL, OFFSETOF(Classes::fl_display::BlendMode, DARKEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DIFFERENCE", NULL, OFFSETOF(Classes::fl_display::BlendMode, DIFFERENCE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ERASE", NULL, OFFSETOF(Classes::fl_display::BlendMode, ERASE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HARDLIGHT", NULL, OFFSETOF(Classes::fl_display::BlendMode, HARDLIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"INVERT", NULL, OFFSETOF(Classes::fl_display::BlendMode, INVERT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LAYER", NULL, OFFSETOF(Classes::fl_display::BlendMode, LAYER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LIGHTEN", NULL, OFFSETOF(Classes::fl_display::BlendMode, LIGHTEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MULTIPLY", NULL, OFFSETOF(Classes::fl_display::BlendMode, MULTIPLY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NORMAL", NULL, OFFSETOF(Classes::fl_display::BlendMode, NORMAL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"OVERLAY", NULL, OFFSETOF(Classes::fl_display::BlendMode, OVERLAY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SCREEN", NULL, OFFSETOF(Classes::fl_display::BlendMode, SCREEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SUBTRACT", NULL, OFFSETOF(Classes::fl_display::BlendMode, SUBTRACT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    BlendMode::BlendMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::BlendMode::BlendMode()"
//##protect##"ClassTraits::BlendMode::BlendMode()"

    }

    Pickable<Traits> BlendMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) BlendMode(vm, AS3::fl_display::BlendModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::BlendModeCI));
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
    const TypeInfo BlendModeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::BlendMode::InstanceType),
        0,
        ClassTraits::fl_display::BlendMode::MemberInfoNum,
        0,
        0,
        "BlendMode", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo BlendModeCI = {
        &BlendModeTI,
        ClassTraits::fl_display::BlendMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::BlendMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

