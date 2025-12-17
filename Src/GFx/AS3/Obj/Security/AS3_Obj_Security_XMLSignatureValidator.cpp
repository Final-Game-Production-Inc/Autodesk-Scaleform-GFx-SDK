//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_XMLSignatureValidator.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Security_XMLSignatureValidator.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#ifdef GFX_ENABLE_XML
#include "../AS3_Obj_XML.h"
#endif
#include "AS3_Obj_Security_IURIDereferencer.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_security
{
    // const UInt16 XMLSignatureValidator_tito[15] = {
    //    0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 15, 16, 19, 
    // };
    const TypeInfo* XMLSignatureValidator_tit[21] = {
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl_security::IURIDereferencerTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::XMLTI, 
    };
    const ThunkInfo XMLSignatureValidator_ti[15] = {
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[0], "digestStatus", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[1], "identityStatus", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[2], "referencesStatus", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[3], "revocationCheckSetting", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[4], "revocationCheckSetting", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[6], "signerCN", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[7], "signerDN", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[8], "signerExtendedKeyUsages", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[9], "signerTrustSettings", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[10], "uriDereferencer", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[12], "useSystemTrustStore", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[13], "useSystemTrustStore", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[15], "validityStatus", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[16], "addCertificate", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &XMLSignatureValidator_tit[19], "verify", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_security
{

    XMLSignatureValidator::XMLSignatureValidator(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::XMLSignatureValidator::XMLSignatureValidator()"
//##protect##"ClassTraits::XMLSignatureValidator::XMLSignatureValidator()"

    }

    Pickable<Traits> XMLSignatureValidator::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) XMLSignatureValidator(vm, AS3::fl_security::XMLSignatureValidatorCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_security::XMLSignatureValidatorCI));
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
    const TypeInfo XMLSignatureValidatorTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_security::XMLSignatureValidator::InstanceType),
        0,
        0,
        15,
        0,
        "XMLSignatureValidator", "flash.security", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo XMLSignatureValidatorCI = {
        &XMLSignatureValidatorTI,
        ClassTraits::fl_security::XMLSignatureValidator::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_security::XMLSignatureValidator_ti,
        NULL,
    };
}; // namespace fl_security


}}} // namespace Scaleform { namespace GFx { namespace AS3

