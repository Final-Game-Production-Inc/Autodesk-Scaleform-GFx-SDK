//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_CSMSettings.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_CSMSettings.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_text
{
    CSMSettings::CSMSettings(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , fontSize()
    , insideCutoff()
    , outsideCutoff()
//##protect##"instance::CSMSettings::CSMSettings()$data"
//##protect##"instance::CSMSettings::CSMSettings()$data"
    {
//##protect##"instance::CSMSettings::CSMSettings()$code"
//##protect##"instance::CSMSettings::CSMSettings()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    const MemberInfo CSMSettings::mi[CSMSettings::MemberInfoNum] = {
        {"fontSize", NULL, OFFSETOF(Instances::fl_text::CSMSettings, fontSize), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"insideCutoff", NULL, OFFSETOF(Instances::fl_text::CSMSettings, insideCutoff), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"outsideCutoff", NULL, OFFSETOF(Instances::fl_text::CSMSettings, outsideCutoff), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    CSMSettings::CSMSettings(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::CSMSettings::CSMSettings()"
//##protect##"InstanceTraits::CSMSettings::CSMSettings()"

    }

    void CSMSettings::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<CSMSettings&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    CSMSettings::CSMSettings(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::CSMSettings::CSMSettings()"
//##protect##"ClassTraits::CSMSettings::CSMSettings()"

    }

    Pickable<Traits> CSMSettings::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) CSMSettings(vm, AS3::fl_text::CSMSettingsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::CSMSettingsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_text
{
    const TypeInfo CSMSettingsTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_text::CSMSettings::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_text::CSMSettings::MemberInfoNum,
        "CSMSettings", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo CSMSettingsCI = {
        &CSMSettingsTI,
        ClassTraits::fl_text::CSMSettings::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_text::CSMSettings::mi,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

