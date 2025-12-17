//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequestMethod.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLRequestMethod.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_net
{
    URLRequestMethod::URLRequestMethod(ClassTraits::Traits& t)
    : Class(t)
    , DELETE("DELETE")
    , GET("GET")
    , HEAD("HEAD")
    , OPTIONS("OPTIONS")
    , POST("POST")
    , PUT("PUT")
    {
//##protect##"class_::URLRequestMethod::URLRequestMethod()"
//##protect##"class_::URLRequestMethod::URLRequestMethod()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_net
{
    const MemberInfo URLRequestMethod::mi[URLRequestMethod::MemberInfoNum] = {
        {"DELETE", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, DELETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"GET", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, GET), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HEAD", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, HEAD), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"OPTIONS", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, OPTIONS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"POST", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, POST), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"PUT", NULL, OFFSETOF(Classes::fl_net::URLRequestMethod, PUT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    URLRequestMethod::URLRequestMethod(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::URLRequestMethod::URLRequestMethod()"
//##protect##"ClassTraits::URLRequestMethod::URLRequestMethod()"

    }

    Pickable<Traits> URLRequestMethod::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLRequestMethod(vm, AS3::fl_net::URLRequestMethodCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLRequestMethodCI));
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
    const TypeInfo URLRequestMethodTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_net::URLRequestMethod::InstanceType),
        0,
        ClassTraits::fl_net::URLRequestMethod::MemberInfoNum,
        0,
        0,
        "URLRequestMethod", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo URLRequestMethodCI = {
        &URLRequestMethodTI,
        ClassTraits::fl_net::URLRequestMethod::MakeClassTraits,
        NULL,
        ClassTraits::fl_net::URLRequestMethod::mi,
        NULL,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

