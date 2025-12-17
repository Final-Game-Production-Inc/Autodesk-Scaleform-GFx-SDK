//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Jul, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_GamePad.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    GamePad::GamePad(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::GamePad::GamePad()"
        PAD_NONE.SetUInt32(Pad_None);
        PAD_BACK.SetUInt32(Pad_Back);
        PAD_A.SetUInt32(Pad_A);
        PAD_B.SetUInt32(Pad_B);
        PAD_X.SetUInt32(Pad_X);
        PAD_Y.SetUInt32(Pad_Y);
        PAD_R1.SetUInt32(Pad_R1);
        PAD_L1.SetUInt32(Pad_L1);
        PAD_R2.SetUInt32(Pad_R2);
        PAD_L2.SetUInt32(Pad_L2);
        PAD_UP.SetUInt32(Pad_Up);
        PAD_DOWN.SetUInt32(Pad_Down);
        PAD_RIGHT.SetUInt32(Pad_Right);
        PAD_LEFT.SetUInt32(Pad_Left);
        PAD_PLUS.SetUInt32(Pad_Plus);
        PAD_MINUS.SetUInt32(Pad_Minus);
        PAD_1.SetUInt32(Pad_1);
        PAD_2.SetUInt32(Pad_2);
        PAD_H.SetUInt32(Pad_H);
        PAD_C.SetUInt32(Pad_C);
        PAD_Z.SetUInt32(Pad_Z);
        PAD_O.SetUInt32(Pad_O);
        PAD_T.SetUInt32(Pad_T);
        PAD_S.SetUInt32(Pad_S);
        PAD_SELECT.SetUInt32(Pad_Select);
        PAD_HOME.SetUInt32(Pad_Home);
        PAD_RT.SetUInt32(Pad_RT);
        PAD_LT.SetUInt32(Pad_LT);
//##protect##"class_::GamePad::GamePad()"
    }
    void GamePad::supportsAnalogEvents(bool& result)
    {
//##protect##"class_::GamePad::supportsAnalogEvents()"
        
#ifdef GFX_ENABLE_ANALOG_GAMEPAD
        result = true;
#else
        result = false;
#endif

//##protect##"class_::GamePad::supportsAnalogEvents()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_gfx::GamePad, Classes::fl_gfx::GamePad::mid_supportsAnalogEvents, bool> TFunc_Classes_GamePad_supportsAnalogEvents;

template <> const TFunc_Classes_GamePad_supportsAnalogEvents::TMethod TFunc_Classes_GamePad_supportsAnalogEvents::Method = &Classes::fl_gfx::GamePad::supportsAnalogEvents;

namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo GamePad::mi[GamePad::MemberInfoNum] = {
        {"PAD_NONE", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_NONE), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_BACK", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_BACK), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_START", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_START), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_A", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_A), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_B", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_B), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_X", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_X), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_Y", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_Y), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_R1", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_R1), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_L1", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_L1), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_R2", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_R2), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_L2", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_L2), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_UP", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_UP), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_DOWN", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_DOWN), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_RIGHT", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_RIGHT), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_LEFT", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_LEFT), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_PLUS", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_PLUS), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_MINUS", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_MINUS), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_1", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_1), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_2", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_2), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_H", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_H), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_C", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_C), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_Z", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_Z), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_O", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_O), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_T", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_T), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_S", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_S), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_SELECT", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_SELECT), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_HOME", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_HOME), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_RT", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_RT), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"PAD_LT", NULL, OFFSETOF(Classes::fl_gfx::GamePad, PAD_LT), Abc::NS_Public, SlotInfo::BT_Value, 1},
    };

    // const UInt16 GamePad::tito[GamePad::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* GamePad::tit[1] = {
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo GamePad::ti[GamePad::ThunkInfoNum] = {
        {TFunc_Classes_GamePad_supportsAnalogEvents::Func, &GamePad::tit[0], "supportsAnalogEvents", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    GamePad::GamePad(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::GamePad::GamePad()"
//##protect##"ClassTraits::GamePad::GamePad()"

    }

    Pickable<Traits> GamePad::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) GamePad(vm, AS3::fl_gfx::GamePadCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::GamePadCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo GamePadTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::GamePad::InstanceType),
        ClassTraits::fl_gfx::GamePad::ThunkInfoNum,
        ClassTraits::fl_gfx::GamePad::MemberInfoNum,
        0,
        0,
        "GamePad", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo GamePadCI = {
        &GamePadTI,
        ClassTraits::fl_gfx::GamePad::MakeClassTraits,
        ClassTraits::fl_gfx::GamePad::ti,
        ClassTraits::fl_gfx::GamePad::mi,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

