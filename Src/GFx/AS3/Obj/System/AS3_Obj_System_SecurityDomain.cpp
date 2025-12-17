//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_SecurityDomain.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_SecurityDomain.h"
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
    SecurityDomain::SecurityDomain(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::SecurityDomain::SecurityDomain()$data"
//##protect##"instance::SecurityDomain::SecurityDomain()$data"
    {
//##protect##"instance::SecurityDomain::SecurityDomain()$code"
//##protect##"instance::SecurityDomain::SecurityDomain()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{

    SecurityDomain::SecurityDomain(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::SecurityDomain::SecurityDomain()"
//##protect##"InstanceTraits::SecurityDomain::SecurityDomain()"

    }

    void SecurityDomain::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<SecurityDomain&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_system
{
    SecurityDomain::SecurityDomain(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::SecurityDomain::SecurityDomain()"
//##protect##"class_::SecurityDomain::SecurityDomain()"
    }
    void SecurityDomain::currentDomainGet(SPtr<Instances::fl_system::SecurityDomain>& result)
    {
//##protect##"class_::SecurityDomain::currentDomainGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::SecurityDomain::currentDomainGet()");
//##protect##"class_::SecurityDomain::currentDomainGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_system::SecurityDomain, Classes::fl_system::SecurityDomain::mid_currentDomainGet, SPtr<Instances::fl_system::SecurityDomain> > TFunc_Classes_SecurityDomain_currentDomainGet;

template <> const TFunc_Classes_SecurityDomain_currentDomainGet::TMethod TFunc_Classes_SecurityDomain_currentDomainGet::Method = &Classes::fl_system::SecurityDomain::currentDomainGet;

namespace ClassTraits { namespace fl_system
{
    // const UInt16 SecurityDomain::tito[SecurityDomain::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* SecurityDomain::tit[1] = {
        &AS3::fl_system::SecurityDomainTI, 
    };
    const ThunkInfo SecurityDomain::ti[SecurityDomain::ThunkInfoNum] = {
        {TFunc_Classes_SecurityDomain_currentDomainGet::Func, &SecurityDomain::tit[0], "currentDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    SecurityDomain::SecurityDomain(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::SecurityDomain::SecurityDomain()"
//##protect##"ClassTraits::SecurityDomain::SecurityDomain()"

    }

    Pickable<Traits> SecurityDomain::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SecurityDomain(vm, AS3::fl_system::SecurityDomainCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_system::SecurityDomainCI));
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
    const TypeInfo SecurityDomainTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_system::SecurityDomain::InstanceType),
        ClassTraits::fl_system::SecurityDomain::ThunkInfoNum,
        0,
        0,
        0,
        "SecurityDomain", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SecurityDomainCI = {
        &SecurityDomainTI,
        ClassTraits::fl_system::SecurityDomain::MakeClassTraits,
        ClassTraits::fl_system::SecurityDomain::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

