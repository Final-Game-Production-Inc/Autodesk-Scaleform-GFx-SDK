//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_AccessibilityImplementation.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Accessibility_AccessibilityImplementation.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_accessibility
{
    AccessibilityImplementation::AccessibilityImplementation(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , errno()
    , stub()
//##protect##"instance::AccessibilityImplementation::AccessibilityImplementation()$data"
//##protect##"instance::AccessibilityImplementation::AccessibilityImplementation()$data"
    {
//##protect##"instance::AccessibilityImplementation::AccessibilityImplementation()$code"
//##protect##"instance::AccessibilityImplementation::AccessibilityImplementation()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_accessibility
{
    // const UInt16 AccessibilityImplementation::tito[AccessibilityImplementation::ThunkInfoNum] = {
    //    0, 2, 4, 7, 9, 10, 12, 14, 15, 17, 19, 20, 
    // };
    const TypeInfo* AccessibilityImplementation::tit[22] = {
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::ArrayTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
    };
    const ThunkInfo AccessibilityImplementation::ti[AccessibilityImplementation::ThunkInfoNum] = {
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[0], "accDoDefaultAction", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[2], "accLocation", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[4], "accSelect", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[7], "get_accDefaultAction", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[9], "get_accFocus", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[10], "get_accName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[12], "get_accRole", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[14], "get_accSelection", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[15], "get_accState", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[17], "get_accValue", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[19], "getChildIDArray", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &AccessibilityImplementation::tit[20], "isLabeledBy", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };
    const MemberInfo AccessibilityImplementation::mi[AccessibilityImplementation::MemberInfoNum] = {
        {"errno", NULL, OFFSETOF(Instances::fl_accessibility::AccessibilityImplementation, errno), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"stub", NULL, OFFSETOF(Instances::fl_accessibility::AccessibilityImplementation, stub), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    AccessibilityImplementation::AccessibilityImplementation(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::AccessibilityImplementation::AccessibilityImplementation()"
//##protect##"InstanceTraits::AccessibilityImplementation::AccessibilityImplementation()"

    }

    void AccessibilityImplementation::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<AccessibilityImplementation&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_accessibility
{

    AccessibilityImplementation::AccessibilityImplementation(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::AccessibilityImplementation::AccessibilityImplementation()"
//##protect##"ClassTraits::AccessibilityImplementation::AccessibilityImplementation()"

    }

    Pickable<Traits> AccessibilityImplementation::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) AccessibilityImplementation(vm, AS3::fl_accessibility::AccessibilityImplementationCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_accessibility::AccessibilityImplementationCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_accessibility
{
    const TypeInfo AccessibilityImplementationTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_accessibility::AccessibilityImplementation::InstanceType),
        0,
        0,
        InstanceTraits::fl_accessibility::AccessibilityImplementation::ThunkInfoNum,
        InstanceTraits::fl_accessibility::AccessibilityImplementation::MemberInfoNum,
        "AccessibilityImplementation", "flash.accessibility", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo AccessibilityImplementationCI = {
        &AccessibilityImplementationTI,
        ClassTraits::fl_accessibility::AccessibilityImplementation::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_accessibility::AccessibilityImplementation::ti,
        InstanceTraits::fl_accessibility::AccessibilityImplementation::mi,
    };
}; // namespace fl_accessibility


}}} // namespace Scaleform { namespace GFx { namespace AS3

