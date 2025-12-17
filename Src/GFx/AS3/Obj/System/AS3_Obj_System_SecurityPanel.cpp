//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_SecurityPanel.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_SecurityPanel.h"
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
    SecurityPanel::SecurityPanel(ClassTraits::Traits& t)
    : Class(t)
    , CAMERA("camera")
    , DEFAULT("default")
    , DISPLAY("display")
    , LOCAL_STORAGE("localStorage")
    , MICROPHONE("microphone")
    , PRIVACY("privacy")
    , SETTINGS_MANAGER("settingsManager")
    {
//##protect##"class_::SecurityPanel::SecurityPanel()"
//##protect##"class_::SecurityPanel::SecurityPanel()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_system
{
    const MemberInfo SecurityPanel::mi[SecurityPanel::MemberInfoNum] = {
        {"CAMERA", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, CAMERA), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DEFAULT", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, DEFAULT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DISPLAY", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, DISPLAY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LOCAL_STORAGE", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, LOCAL_STORAGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MICROPHONE", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, MICROPHONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"PRIVACY", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, PRIVACY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SETTINGS_MANAGER", NULL, OFFSETOF(Classes::fl_system::SecurityPanel, SETTINGS_MANAGER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SecurityPanel::SecurityPanel(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SecurityPanel::SecurityPanel()"
//##protect##"ClassTraits::SecurityPanel::SecurityPanel()"

    }

    Pickable<Traits> SecurityPanel::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SecurityPanel(vm, AS3::fl_system::SecurityPanelCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::SecurityPanelCI));
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
    const TypeInfo SecurityPanelTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_system::SecurityPanel::InstanceType),
        0,
        ClassTraits::fl_system::SecurityPanel::MemberInfoNum,
        0,
        0,
        "SecurityPanel", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SecurityPanelCI = {
        &SecurityPanelTI,
        ClassTraits::fl_system::SecurityPanel::MakeClassTraits,
        NULL,
        ClassTraits::fl_system::SecurityPanel::mi,
        NULL,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

