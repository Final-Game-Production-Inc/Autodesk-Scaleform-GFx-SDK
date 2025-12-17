//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_RevocationCheckSettings.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Security_RevocationCheckSettings.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_security
{
    RevocationCheckSettings::RevocationCheckSettings(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::RevocationCheckSettings::RevocationCheckSettings()"
//##protect##"class_::RevocationCheckSettings::RevocationCheckSettings()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_security
{
    const MemberInfo RevocationCheckSettings::mi[RevocationCheckSettings::MemberInfoNum] = {
        {"ALWAYS_REQUIRED", NULL, OFFSETOF(Classes::fl_security::RevocationCheckSettings, ALWAYS_REQUIRED), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"BEST_EFFORT", NULL, OFFSETOF(Classes::fl_security::RevocationCheckSettings, BEST_EFFORT), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"NEVER", NULL, OFFSETOF(Classes::fl_security::RevocationCheckSettings, NEVER), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"REQUIRED_IF_AVAILABLE", NULL, OFFSETOF(Classes::fl_security::RevocationCheckSettings, REQUIRED_IF_AVAILABLE), Abc::NS_Public, SlotInfo::BT_Value, 1},
    };


    RevocationCheckSettings::RevocationCheckSettings(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::RevocationCheckSettings::RevocationCheckSettings()"
//##protect##"ClassTraits::RevocationCheckSettings::RevocationCheckSettings()"

    }

    Pickable<Traits> RevocationCheckSettings::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) RevocationCheckSettings(vm, AS3::fl_security::RevocationCheckSettingsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_security::RevocationCheckSettingsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_security
{
    const TypeInfo RevocationCheckSettingsTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_security::RevocationCheckSettings::InstanceType),
        0,
        ClassTraits::fl_security::RevocationCheckSettings::MemberInfoNum,
        0,
        0,
        "RevocationCheckSettings", "flash.security", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo RevocationCheckSettingsCI = {
        &RevocationCheckSettingsTI,
        ClassTraits::fl_security::RevocationCheckSettings::MakeClassTraits,
        NULL,
        ClassTraits::fl_security::RevocationCheckSettings::mi,
        NULL,
        NULL,
    };
}; // namespace fl_security


}}} // namespace Scaleform { namespace GFx { namespace AS3

