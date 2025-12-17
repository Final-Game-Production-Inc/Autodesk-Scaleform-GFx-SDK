//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_SharedObjectFlushStatus.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_SharedObjectFlushStatus.h"
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
    SharedObjectFlushStatus::SharedObjectFlushStatus(ClassTraits::Traits& t)
    : Class(t)
    , FLUSHED("flushed")
    , PENDING("pending")
    {
//##protect##"class_::SharedObjectFlushStatus::SharedObjectFlushStatus()"
//##protect##"class_::SharedObjectFlushStatus::SharedObjectFlushStatus()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_net
{
    const MemberInfo SharedObjectFlushStatus::mi[SharedObjectFlushStatus::MemberInfoNum] = {
        {"FLUSHED", NULL, OFFSETOF(Classes::fl_net::SharedObjectFlushStatus, FLUSHED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"PENDING", NULL, OFFSETOF(Classes::fl_net::SharedObjectFlushStatus, PENDING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    SharedObjectFlushStatus::SharedObjectFlushStatus(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SharedObjectFlushStatus::SharedObjectFlushStatus()"
//##protect##"ClassTraits::SharedObjectFlushStatus::SharedObjectFlushStatus()"

    }

    Pickable<Traits> SharedObjectFlushStatus::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SharedObjectFlushStatus(vm, AS3::fl_net::SharedObjectFlushStatusCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::SharedObjectFlushStatusCI));
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
    const TypeInfo SharedObjectFlushStatusTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_net::SharedObjectFlushStatus::InstanceType),
        0,
        ClassTraits::fl_net::SharedObjectFlushStatus::MemberInfoNum,
        0,
        0,
        "SharedObjectFlushStatus", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SharedObjectFlushStatusCI = {
        &SharedObjectFlushStatusTI,
        ClassTraits::fl_net::SharedObjectFlushStatus::MakeClassTraits,
        NULL,
        ClassTraits::fl_net::SharedObjectFlushStatus::mi,
        NULL,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

