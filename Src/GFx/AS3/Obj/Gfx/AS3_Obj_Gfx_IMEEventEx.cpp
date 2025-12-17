//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_IMEEventEx.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_IMEEventEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_gfx
{
    IMEEventEx::IMEEventEx(InstanceTraits::Traits& t)
    : Instances::fl_events::Event(t)
    , message(AS3::DefaultValue<ASString>(GetStringManager()))
//##protect##"instance::IMEEventEx::IMEEventEx()$data"
//##protect##"instance::IMEEventEx::IMEEventEx()$data"
    {
//##protect##"instance::IMEEventEx::IMEEventEx()$code"
//##protect##"instance::IMEEventEx::IMEEventEx()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    const MemberInfo IMEEventEx::mi[IMEEventEx::MemberInfoNum] = {
        {"message", NULL, OFFSETOF(Instances::fl_gfx::IMEEventEx, message), Abc::NS_Public, SlotInfo::BT_String, 0},
    };


    IMEEventEx::IMEEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"InstanceTraits::IMEEventEx::IMEEventEx()"
//##protect##"InstanceTraits::IMEEventEx::IMEEventEx()"

    }

    void IMEEventEx::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<IMEEventEx&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_gfx
{
    IMEEventEx::IMEEventEx(ClassTraits::Traits& t)
    : Class(t)
    , SET_CURRENT_INPUT_LANGUAGE("SetCurrentInputLanguage")
    , SET_SUPPORTED_LANGUAGES("SetSupportedLanguages")
    , SET_SUPPORTED_IMENAMES("SetSupportedIMENames")
    , SET_IMENAME("SetIMEName")
    , SET_CURRENT_LANGUAGE("SetCurrentLanguage")
    , REMOVE_STATUS_WINDOW("RemoveStatusWindow")
    , DISPLAY_STATUS_WINDOW("DisplayStatusWindow")
    , SET_CONVERSION_STATUS("SetConversionStatus")
    {
//##protect##"class_::IMEEventEx::IMEEventEx()"
//##protect##"class_::IMEEventEx::IMEEventEx()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo IMEEventEx::mi[IMEEventEx::MemberInfoNum] = {
        {"SET_CURRENT_INPUT_LANGUAGE", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_CURRENT_INPUT_LANGUAGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SET_SUPPORTED_LANGUAGES", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_SUPPORTED_LANGUAGES), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SET_SUPPORTED_IMENAMES", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_SUPPORTED_IMENAMES), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SET_IMENAME", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_IMENAME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SET_CURRENT_LANGUAGE", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_CURRENT_LANGUAGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"REMOVE_STATUS_WINDOW", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, REMOVE_STATUS_WINDOW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DISPLAY_STATUS_WINDOW", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, DISPLAY_STATUS_WINDOW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SET_CONVERSION_STATUS", NULL, OFFSETOF(Classes::fl_gfx::IMEEventEx, SET_CONVERSION_STATUS), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    IMEEventEx::IMEEventEx(VM& vm, const ClassInfo& ci)
    : fl_events::Event(vm, ci)
    {
//##protect##"ClassTraits::IMEEventEx::IMEEventEx()"
//##protect##"ClassTraits::IMEEventEx::IMEEventEx()"

    }

    Pickable<Traits> IMEEventEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IMEEventEx(vm, AS3::fl_gfx::IMEEventExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::IMEEventExCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo IMEEventExTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::IMEEventEx::InstanceType),
        0,
        ClassTraits::fl_gfx::IMEEventEx::MemberInfoNum,
        0,
        InstanceTraits::fl_gfx::IMEEventEx::MemberInfoNum,
        "IMEEventEx", "scaleform.gfx", &fl_events::EventTI,
        TypeInfo::None
    };

    const ClassInfo IMEEventExCI = {
        &IMEEventExTI,
        ClassTraits::fl_gfx::IMEEventEx::MakeClassTraits,
        NULL,
        ClassTraits::fl_gfx::IMEEventEx::mi,
        NULL,
        InstanceTraits::fl_gfx::IMEEventEx::mi,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

