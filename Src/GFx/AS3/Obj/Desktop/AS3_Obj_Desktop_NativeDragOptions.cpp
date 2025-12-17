//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Desktop_NativeDragOptions.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Desktop_NativeDragOptions.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_desktop
{
    NativeDragOptions::NativeDragOptions(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , allowCopy(true)
    , allowLink(true)
    , allowMove(true)
//##protect##"instance::NativeDragOptions::NativeDragOptions()$data"
//##protect##"instance::NativeDragOptions::NativeDragOptions()$data"
    {
//##protect##"instance::NativeDragOptions::NativeDragOptions()$code"
//##protect##"instance::NativeDragOptions::NativeDragOptions()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_desktop
{
    // const UInt16 NativeDragOptions::tito[NativeDragOptions::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* NativeDragOptions::tit[1] = {
        &AS3::fl::StringTI, 
    };
    const ThunkInfo NativeDragOptions::ti[NativeDragOptions::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &NativeDragOptions::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo NativeDragOptions::mi[NativeDragOptions::MemberInfoNum] = {
        {"allowCopy", NULL, OFFSETOF(Instances::fl_desktop::NativeDragOptions, allowCopy), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"allowLink", NULL, OFFSETOF(Instances::fl_desktop::NativeDragOptions, allowLink), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"allowMove", NULL, OFFSETOF(Instances::fl_desktop::NativeDragOptions, allowMove), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    NativeDragOptions::NativeDragOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::NativeDragOptions::NativeDragOptions()"
//##protect##"InstanceTraits::NativeDragOptions::NativeDragOptions()"

    }

    void NativeDragOptions::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<NativeDragOptions&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_desktop
{

    NativeDragOptions::NativeDragOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::NativeDragOptions::NativeDragOptions()"
//##protect##"ClassTraits::NativeDragOptions::NativeDragOptions()"

    }

    Pickable<Traits> NativeDragOptions::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NativeDragOptions(vm, AS3::fl_desktop::NativeDragOptionsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_desktop::NativeDragOptionsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_desktop
{
    const TypeInfo NativeDragOptionsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_desktop::NativeDragOptions::InstanceType),
        0,
        0,
        InstanceTraits::fl_desktop::NativeDragOptions::ThunkInfoNum,
        InstanceTraits::fl_desktop::NativeDragOptions::MemberInfoNum,
        "NativeDragOptions", "flash.desktop", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo NativeDragOptionsCI = {
        &NativeDragOptionsTI,
        ClassTraits::fl_desktop::NativeDragOptions::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_desktop::NativeDragOptions::ti,
        InstanceTraits::fl_desktop::NativeDragOptions::mi,
    };
}; // namespace fl_desktop


}}} // namespace Scaleform { namespace GFx { namespace AS3

