//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequestDefaults.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLRequestDefaults.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 URLRequestDefaults_tito[6] = {
    //    0, 2, 4, 6, 8, 10, 
    // };
    const TypeInfo* URLRequestDefaults_tit[12] = {
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo URLRequestDefaults_ti[6] = {
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[0], "authenticate", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[2], "cacheResponse", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[4], "followRedirects", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[6], "manageCookies", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[8], "useCache", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[10], "userAgent", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_net
{
    URLRequestDefaults::URLRequestDefaults(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::URLRequestDefaults::URLRequestDefaults()"
//##protect##"class_::URLRequestDefaults::URLRequestDefaults()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_net
{
    // const UInt16 URLRequestDefaults_tito[1] = {
    //    0, 
    // };
    const TypeInfo* URLRequestDefaults_tit[4] = {
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo URLRequestDefaults_ti[1] = {
        {ThunkInfo::EmptyFunc, &URLRequestDefaults_tit[0], "setLoginCredentialsForHost", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
    };

    URLRequestDefaults::URLRequestDefaults(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::URLRequestDefaults::URLRequestDefaults()"
//##protect##"ClassTraits::URLRequestDefaults::URLRequestDefaults()"

    }

    Pickable<Traits> URLRequestDefaults::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLRequestDefaults(vm, AS3::fl_net::URLRequestDefaultsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLRequestDefaultsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo URLRequestDefaultsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::URLRequestDefaults::InstanceType),
        1,
        0,
        6,
        0,
        "URLRequestDefaults", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo URLRequestDefaultsCI = {
        &URLRequestDefaultsTI,
        ClassTraits::fl_net::URLRequestDefaults::MakeClassTraits,
        ClassTraits::fl_net::URLRequestDefaults_ti,
        NULL,
        InstanceTraits::fl_net::URLRequestDefaults_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

