//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_SignatureStatus.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Security_SignatureStatus.h"
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
    SignatureStatus::SignatureStatus(ClassTraits::Traits& t)
    : Class(t)
    , INVALID("invalid")
    , UNKNOWN("")
    , VALID("valid")
    {
//##protect##"class_::SignatureStatus::SignatureStatus()"
//##protect##"class_::SignatureStatus::SignatureStatus()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_security
{
    const MemberInfo SignatureStatus::mi[SignatureStatus::MemberInfoNum] = {
        {"INVALID", NULL, OFFSETOF(Classes::fl_security::SignatureStatus, INVALID), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UNKNOWN", NULL, OFFSETOF(Classes::fl_security::SignatureStatus, UNKNOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"VALID", NULL, OFFSETOF(Classes::fl_security::SignatureStatus, VALID), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SignatureStatus::SignatureStatus(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SignatureStatus::SignatureStatus()"
//##protect##"ClassTraits::SignatureStatus::SignatureStatus()"

    }

    Pickable<Traits> SignatureStatus::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SignatureStatus(vm, AS3::fl_security::SignatureStatusCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_security::SignatureStatusCI));
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
    const TypeInfo SignatureStatusTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_security::SignatureStatus::InstanceType),
        0,
        ClassTraits::fl_security::SignatureStatus::MemberInfoNum,
        0,
        0,
        "SignatureStatus", "flash.security", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SignatureStatusCI = {
        &SignatureStatusTI,
        ClassTraits::fl_security::SignatureStatus::MakeClassTraits,
        NULL,
        ClassTraits::fl_security::SignatureStatus::mi,
        NULL,
        NULL,
    };
}; // namespace fl_security


}}} // namespace Scaleform { namespace GFx { namespace AS3

