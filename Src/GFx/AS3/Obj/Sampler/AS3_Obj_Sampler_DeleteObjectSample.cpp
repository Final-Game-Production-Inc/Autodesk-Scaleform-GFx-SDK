//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_DeleteObjectSample.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Sampler_DeleteObjectSample.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_sampler
{
    DeleteObjectSample::DeleteObjectSample(InstanceTraits::Traits& t)
    : Instances::fl_sampler::Sample(t)
    , id()
    , size()
//##protect##"instance::DeleteObjectSample::DeleteObjectSample()$data"
//##protect##"instance::DeleteObjectSample::DeleteObjectSample()$data"
    {
//##protect##"instance::DeleteObjectSample::DeleteObjectSample()$code"
//##protect##"instance::DeleteObjectSample::DeleteObjectSample()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    const MemberInfo DeleteObjectSample::mi[DeleteObjectSample::MemberInfoNum] = {
        {"id", NULL, OFFSETOF(Instances::fl_sampler::DeleteObjectSample, id), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"size", NULL, OFFSETOF(Instances::fl_sampler::DeleteObjectSample, size), Abc::NS_Public, SlotInfo::BT_Number, 1},
    };


    DeleteObjectSample::DeleteObjectSample(VM& vm, const ClassInfo& ci)
    : fl_sampler::Sample(vm, ci)
    {
//##protect##"InstanceTraits::DeleteObjectSample::DeleteObjectSample()"
//##protect##"InstanceTraits::DeleteObjectSample::DeleteObjectSample()"

    }

    void DeleteObjectSample::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<DeleteObjectSample&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_sampler
{

    DeleteObjectSample::DeleteObjectSample(VM& vm, const ClassInfo& ci)
    : fl_sampler::Sample(vm, ci)
    {
//##protect##"ClassTraits::DeleteObjectSample::DeleteObjectSample()"
//##protect##"ClassTraits::DeleteObjectSample::DeleteObjectSample()"

    }

    Pickable<Traits> DeleteObjectSample::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DeleteObjectSample(vm, AS3::fl_sampler::DeleteObjectSampleCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_sampler::DeleteObjectSampleCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_sampler
{
    const TypeInfo DeleteObjectSampleTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_sampler::DeleteObjectSample::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_sampler::DeleteObjectSample::MemberInfoNum,
        "DeleteObjectSample", "flash.sampler", &fl_sampler::SampleTI,
        TypeInfo::None
    };

    const ClassInfo DeleteObjectSampleCI = {
        &DeleteObjectSampleTI,
        ClassTraits::fl_sampler::DeleteObjectSample::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_sampler::DeleteObjectSample::mi,
    };
}; // namespace fl_sampler


}}} // namespace Scaleform { namespace GFx { namespace AS3

