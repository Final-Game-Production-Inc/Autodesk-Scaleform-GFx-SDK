//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextColorType.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextColorType.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_text
{
    TextColorType::TextColorType(ClassTraits::Traits& t)
    : Class(t)
    , DARK_COLOR("dark")
    , LIGHT_COLOR("light")
    {
//##protect##"class_::TextColorType::TextColorType()"
//##protect##"class_::TextColorType::TextColorType()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_text
{
    const MemberInfo TextColorType::mi[TextColorType::MemberInfoNum] = {
        {"DARK_COLOR", NULL, OFFSETOF(Classes::fl_text::TextColorType, DARK_COLOR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LIGHT_COLOR", NULL, OFFSETOF(Classes::fl_text::TextColorType, LIGHT_COLOR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    TextColorType::TextColorType(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextColorType::TextColorType()"
//##protect##"ClassTraits::TextColorType::TextColorType()"

    }

    Pickable<Traits> TextColorType::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextColorType(vm, AS3::fl_text::TextColorTypeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextColorTypeCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_text
{
    const TypeInfo TextColorTypeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_text::TextColorType::InstanceType),
        0,
        ClassTraits::fl_text::TextColorType::MemberInfoNum,
        0,
        0,
        "TextColorType", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextColorTypeCI = {
        &TextColorTypeTI,
        ClassTraits::fl_text::TextColorType::MakeClassTraits,
        NULL,
        ClassTraits::fl_text::TextColorType::mi,
        NULL,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

