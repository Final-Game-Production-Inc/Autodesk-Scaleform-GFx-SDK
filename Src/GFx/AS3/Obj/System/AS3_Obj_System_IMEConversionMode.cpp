//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_IMEConversionMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_IMEConversionMode.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_system
{
    IMEConversionMode::IMEConversionMode(ClassTraits::Traits& t)
    : Class(t)
    , ALPHANUMERIC_FULL("ALPHANUMERIC_FULL")
    , ALPHANUMERIC_HALF("ALPHANUMERIC_HALF")
    , CHINESE("CHINESE")
    , JAPANESE_HIRAGANA("JAPANESE_HIRAGANA")
    , JAPANESE_KATAKANA_FULL("JAPANESE_KATAKANA_FULL")
    , JAPANESE_KATAKANA_HALF("JAPANESE_KATAKANA_HALF")
    , KOREAN("KOREAN")
    , UNKNOWN("UNKNOWN")
    {
//##protect##"class_::IMEConversionMode::IMEConversionMode()"
//##protect##"class_::IMEConversionMode::IMEConversionMode()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_system
{
    const MemberInfo IMEConversionMode::mi[IMEConversionMode::MemberInfoNum] = {
        {"ALPHANUMERIC_FULL", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, ALPHANUMERIC_FULL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ALPHANUMERIC_HALF", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, ALPHANUMERIC_HALF), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CHINESE", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, CHINESE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"JAPANESE_HIRAGANA", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, JAPANESE_HIRAGANA), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"JAPANESE_KATAKANA_FULL", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, JAPANESE_KATAKANA_FULL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"JAPANESE_KATAKANA_HALF", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, JAPANESE_KATAKANA_HALF), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KOREAN", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, KOREAN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UNKNOWN", NULL, OFFSETOF(Classes::fl_system::IMEConversionMode, UNKNOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    IMEConversionMode::IMEConversionMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IMEConversionMode::IMEConversionMode()"
//##protect##"ClassTraits::IMEConversionMode::IMEConversionMode()"

    }

    Pickable<Traits> IMEConversionMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IMEConversionMode(vm, AS3::fl_system::IMEConversionModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::IMEConversionModeCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_system
{
    const TypeInfo IMEConversionModeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_system::IMEConversionMode::InstanceType),
        0,
        ClassTraits::fl_system::IMEConversionMode::MemberInfoNum,
        0,
        0,
        "IMEConversionMode", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo IMEConversionModeCI = {
        &IMEConversionModeTI,
        ClassTraits::fl_system::IMEConversionMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_system::IMEConversionMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

