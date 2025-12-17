//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextFieldAutoSize.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextFieldAutoSize.h"
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
    TextFieldAutoSize::TextFieldAutoSize(ClassTraits::Traits& t)
    : Class(t)
    , CENTER("center")
    , LEFT("left")
    , NONE("none")
    , RIGHT("right")
    {
//##protect##"class_::TextFieldAutoSize::TextFieldAutoSize()"
//##protect##"class_::TextFieldAutoSize::TextFieldAutoSize()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_text
{
    const MemberInfo TextFieldAutoSize::mi[TextFieldAutoSize::MemberInfoNum] = {
        {"CENTER", NULL, OFFSETOF(Classes::fl_text::TextFieldAutoSize, CENTER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LEFT", NULL, OFFSETOF(Classes::fl_text::TextFieldAutoSize, LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NONE", NULL, OFFSETOF(Classes::fl_text::TextFieldAutoSize, NONE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT", NULL, OFFSETOF(Classes::fl_text::TextFieldAutoSize, RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    TextFieldAutoSize::TextFieldAutoSize(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextFieldAutoSize::TextFieldAutoSize()"
//##protect##"ClassTraits::TextFieldAutoSize::TextFieldAutoSize()"

    }

    Pickable<Traits> TextFieldAutoSize::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextFieldAutoSize(vm, AS3::fl_text::TextFieldAutoSizeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextFieldAutoSizeCI));
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
    const TypeInfo TextFieldAutoSizeTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_text::TextFieldAutoSize::InstanceType),
        0,
        ClassTraits::fl_text::TextFieldAutoSize::MemberInfoNum,
        0,
        0,
        "TextFieldAutoSize", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextFieldAutoSizeCI = {
        &TextFieldAutoSizeTI,
        ClassTraits::fl_text::TextFieldAutoSize::MakeClassTraits,
        NULL,
        ClassTraits::fl_text::TextFieldAutoSize::mi,
        NULL,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

