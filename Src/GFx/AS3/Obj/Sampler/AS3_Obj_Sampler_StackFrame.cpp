//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_StackFrame.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Sampler_StackFrame.h"
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
    StackFrame::StackFrame(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , file(AS3::DefaultValue<char*>(GetStringManager()))
    , line()
    , name(AS3::DefaultValue<char*>(GetStringManager()))
//##protect##"instance::StackFrame::StackFrame()$data"
//##protect##"instance::StackFrame::StackFrame()$data"
    {
//##protect##"instance::StackFrame::StackFrame()$code"
//##protect##"instance::StackFrame::StackFrame()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    // const UInt16 StackFrame::tito[StackFrame::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* StackFrame::tit[1] = {
        &AS3::fl::StringTI, 
    };
    const ThunkInfo StackFrame::ti[StackFrame::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &StackFrame::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo StackFrame::mi[StackFrame::MemberInfoNum] = {
        {"file", NULL, OFFSETOF(Instances::fl_sampler::StackFrame, file), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"line", NULL, OFFSETOF(Instances::fl_sampler::StackFrame, line), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"name", NULL, OFFSETOF(Instances::fl_sampler::StackFrame, name), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    StackFrame::StackFrame(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::StackFrame::StackFrame()"
//##protect##"InstanceTraits::StackFrame::StackFrame()"

    }

    void StackFrame::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<StackFrame&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_sampler
{

    StackFrame::StackFrame(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::StackFrame::StackFrame()"
//##protect##"ClassTraits::StackFrame::StackFrame()"

    }

    Pickable<Traits> StackFrame::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StackFrame(vm, AS3::fl_sampler::StackFrameCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_sampler::StackFrameCI));
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
    const TypeInfo StackFrameTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_sampler::StackFrame::InstanceType),
        0,
        0,
        InstanceTraits::fl_sampler::StackFrame::ThunkInfoNum,
        InstanceTraits::fl_sampler::StackFrame::MemberInfoNum,
        "StackFrame", "flash.sampler", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo StackFrameCI = {
        &StackFrameTI,
        ClassTraits::fl_sampler::StackFrame::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_sampler::StackFrame::ti,
        InstanceTraits::fl_sampler::StackFrame::mi,
    };
}; // namespace fl_sampler


}}} // namespace Scaleform { namespace GFx { namespace AS3

