//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_LoaderContext.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_LoaderContext.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_system
{
    LoaderContext::LoaderContext(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , allowLoadBytesCodeExecution(false)
    , applicationDomain()
    , checkPolicyFile(false)
    , securityDomain()
//##protect##"instance::LoaderContext::LoaderContext()$data"
//##protect##"instance::LoaderContext::LoaderContext()$data"
    {
//##protect##"instance::LoaderContext::LoaderContext()$code"
//##protect##"instance::LoaderContext::LoaderContext()$code"
    }


//##protect##"instance$methods"
    void LoaderContext::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 0 && !argv[0].IsNullOrUndefined())
        {
            checkPolicyFile = argv[0].AsBool();
        }
        if (argc > 1 && !argv[1].IsNullOrUndefined())
        {
            ApplicationDomain& appDomain = static_cast<ApplicationDomain&>(*argv[1].GetObject());
            InstanceTraits::fl_system::ApplicationDomain& appDomainTraits = static_cast<InstanceTraits::fl_system::ApplicationDomain&>(appDomain.GetInstanceTraits());
            applicationDomain = appDomainTraits.MakeInstanceS(appDomainTraits);
            applicationDomain->SetAppDomain(appDomain.GetAppDomain());
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    const MemberInfo LoaderContext::mi[LoaderContext::MemberInfoNum] = {
        {"allowLoadBytesCodeExecution", NULL, OFFSETOF(Instances::fl_system::LoaderContext, allowLoadBytesCodeExecution), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"applicationDomain", NULL, OFFSETOF(Instances::fl_system::LoaderContext, applicationDomain), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
        {"checkPolicyFile", NULL, OFFSETOF(Instances::fl_system::LoaderContext, checkPolicyFile), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"securityDomain", NULL, OFFSETOF(Instances::fl_system::LoaderContext, securityDomain), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 0},
    };


    LoaderContext::LoaderContext(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::LoaderContext::LoaderContext()"
//##protect##"InstanceTraits::LoaderContext::LoaderContext()"

    }

    void LoaderContext::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<LoaderContext&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_system
{

    LoaderContext::LoaderContext(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::LoaderContext::LoaderContext()"
//##protect##"ClassTraits::LoaderContext::LoaderContext()"

    }

    Pickable<Traits> LoaderContext::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) LoaderContext(vm, AS3::fl_system::LoaderContextCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::LoaderContextCI));
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
    const TypeInfo LoaderContextTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_system::LoaderContext::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_system::LoaderContext::MemberInfoNum,
        "LoaderContext", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo LoaderContextCI = {
        &LoaderContextTI,
        ClassTraits::fl_system::LoaderContext::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_system::LoaderContext::mi,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

