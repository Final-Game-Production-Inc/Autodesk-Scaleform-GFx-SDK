//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_SWFVersion.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_SWFVersion.h"
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
    SWFVersion::SWFVersion(ClassTraits::Traits& t)
    : Class(t)
    , FLASH1(1)
    , FLASH2(2)
    , FLASH3(3)
    , FLASH4(4)
    , FLASH5(5)
    , FLASH6(6)
    , FLASH7(7)
    , FLASH8(8)
    , FLASH9(9)
    {
//##protect##"class_::SWFVersion::SWFVersion()"
//##protect##"class_::SWFVersion::SWFVersion()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_display
{
    const MemberInfo SWFVersion::mi[SWFVersion::MemberInfoNum] = {
        {"FLASH1", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH1), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH2", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH2), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH3", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH3), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH4", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH4), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH5", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH5), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH6", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH6), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH7", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH7), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH8", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH8), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH9", NULL, OFFSETOF(Classes::fl_display::SWFVersion, FLASH9), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };


    SWFVersion::SWFVersion(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SWFVersion::SWFVersion()"
//##protect##"ClassTraits::SWFVersion::SWFVersion()"

    }

    Pickable<Traits> SWFVersion::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SWFVersion(vm, AS3::fl_display::SWFVersionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::SWFVersionCI));
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
    const TypeInfo SWFVersionTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::SWFVersion::InstanceType),
        0,
        ClassTraits::fl_display::SWFVersion::MemberInfoNum,
        0,
        0,
        "SWFVersion", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SWFVersionCI = {
        &SWFVersionTI,
        ClassTraits::fl_display::SWFVersion::MakeClassTraits,
        NULL,
        ClassTraits::fl_display::SWFVersion::mi,
        NULL,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

