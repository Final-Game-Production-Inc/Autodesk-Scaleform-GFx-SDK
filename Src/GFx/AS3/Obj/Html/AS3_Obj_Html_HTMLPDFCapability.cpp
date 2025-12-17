//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLPDFCapability.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Html_HTMLPDFCapability.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_html
{
    HTMLPDFCapability::HTMLPDFCapability(ClassTraits::Traits& t)
    : Class(t)
    , ERROR_CANNOT_LOAD_READER(3204)
    , ERROR_INSTALLED_READER_NOT_FOUND(3201)
    , ERROR_INSTALLED_READER_TOO_OLD(3202)
    , ERROR_PREFERRED_READER_TOO_OLD(3203)
    , STATUS_OK(0)
    {
//##protect##"class_::HTMLPDFCapability::HTMLPDFCapability()"
//##protect##"class_::HTMLPDFCapability::HTMLPDFCapability()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_html
{
    const MemberInfo HTMLPDFCapability::mi[HTMLPDFCapability::MemberInfoNum] = {
        {"ERROR_CANNOT_LOAD_READER", NULL, OFFSETOF(Classes::fl_html::HTMLPDFCapability, ERROR_CANNOT_LOAD_READER), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"ERROR_INSTALLED_READER_NOT_FOUND", NULL, OFFSETOF(Classes::fl_html::HTMLPDFCapability, ERROR_INSTALLED_READER_NOT_FOUND), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"ERROR_INSTALLED_READER_TOO_OLD", NULL, OFFSETOF(Classes::fl_html::HTMLPDFCapability, ERROR_INSTALLED_READER_TOO_OLD), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"ERROR_PREFERRED_READER_TOO_OLD", NULL, OFFSETOF(Classes::fl_html::HTMLPDFCapability, ERROR_PREFERRED_READER_TOO_OLD), Abc::NS_Public, SlotInfo::BT_Int, 1},
        {"STATUS_OK", NULL, OFFSETOF(Classes::fl_html::HTMLPDFCapability, STATUS_OK), Abc::NS_Public, SlotInfo::BT_Int, 1},
    };


    HTMLPDFCapability::HTMLPDFCapability(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::HTMLPDFCapability::HTMLPDFCapability()"
//##protect##"ClassTraits::HTMLPDFCapability::HTMLPDFCapability()"

    }

    Pickable<Traits> HTMLPDFCapability::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLPDFCapability(vm, AS3::fl_html::HTMLPDFCapabilityCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_html::HTMLPDFCapabilityCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_html
{
    const TypeInfo HTMLPDFCapabilityTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_html::HTMLPDFCapability::InstanceType),
        0,
        ClassTraits::fl_html::HTMLPDFCapability::MemberInfoNum,
        0,
        0,
        "HTMLPDFCapability", "flash.html", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo HTMLPDFCapabilityCI = {
        &HTMLPDFCapabilityTI,
        ClassTraits::fl_html::HTMLPDFCapability::MakeClassTraits,
        NULL,
        ClassTraits::fl_html::HTMLPDFCapability::mi,
        NULL,
        NULL,
    };
}; // namespace fl_html


}}} // namespace Scaleform { namespace GFx { namespace AS3

