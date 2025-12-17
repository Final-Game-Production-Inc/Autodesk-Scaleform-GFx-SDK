//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_SignerTrustSettings.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Security_SignerTrustSettings.h"
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
    SignerTrustSettings::SignerTrustSettings(ClassTraits::Traits& t)
    : Class(t)
    , CODE_SIGNING("codeSigning")
    , PLAYLIST_SIGNING("playlistSigning")
    , SIGNING("signing")
    {
//##protect##"class_::SignerTrustSettings::SignerTrustSettings()"
//##protect##"class_::SignerTrustSettings::SignerTrustSettings()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_security
{
    const MemberInfo SignerTrustSettings::mi[SignerTrustSettings::MemberInfoNum] = {
        {"CODE_SIGNING", NULL, OFFSETOF(Classes::fl_security::SignerTrustSettings, CODE_SIGNING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"PLAYLIST_SIGNING", NULL, OFFSETOF(Classes::fl_security::SignerTrustSettings, PLAYLIST_SIGNING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SIGNING", NULL, OFFSETOF(Classes::fl_security::SignerTrustSettings, SIGNING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SignerTrustSettings::SignerTrustSettings(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SignerTrustSettings::SignerTrustSettings()"
//##protect##"ClassTraits::SignerTrustSettings::SignerTrustSettings()"

    }

    Pickable<Traits> SignerTrustSettings::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SignerTrustSettings(vm, AS3::fl_security::SignerTrustSettingsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_security::SignerTrustSettingsCI));
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
    const TypeInfo SignerTrustSettingsTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_security::SignerTrustSettings::InstanceType),
        0,
        ClassTraits::fl_security::SignerTrustSettings::MemberInfoNum,
        0,
        0,
        "SignerTrustSettings", "flash.security", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SignerTrustSettingsCI = {
        &SignerTrustSettingsTI,
        ClassTraits::fl_security::SignerTrustSettings::MakeClassTraits,
        NULL,
        ClassTraits::fl_security::SignerTrustSettings::mi,
        NULL,
        NULL,
    };
}; // namespace fl_security


}}} // namespace Scaleform { namespace GFx { namespace AS3

