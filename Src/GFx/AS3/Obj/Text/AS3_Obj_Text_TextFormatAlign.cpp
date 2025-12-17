//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextFormatAlign.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextFormatAlign.h"
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
    TextFormatAlign::TextFormatAlign(ClassTraits::Traits& t)
    : Class(t)
    , CENTER("center")
    , JUSTIFY("justify")
    , LEFT("left")
    , RIGHT("right")
    {
//##protect##"class_::TextFormatAlign::TextFormatAlign()"
//##protect##"class_::TextFormatAlign::TextFormatAlign()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_text
{
    const MemberInfo TextFormatAlign::mi[TextFormatAlign::MemberInfoNum] = {
        {"CENTER", NULL, OFFSETOF(Classes::fl_text::TextFormatAlign, CENTER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"JUSTIFY", NULL, OFFSETOF(Classes::fl_text::TextFormatAlign, JUSTIFY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LEFT", NULL, OFFSETOF(Classes::fl_text::TextFormatAlign, LEFT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RIGHT", NULL, OFFSETOF(Classes::fl_text::TextFormatAlign, RIGHT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    TextFormatAlign::TextFormatAlign(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextFormatAlign::TextFormatAlign()"
//##protect##"ClassTraits::TextFormatAlign::TextFormatAlign()"

    }

    Pickable<Traits> TextFormatAlign::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextFormatAlign(vm, AS3::fl_text::TextFormatAlignCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextFormatAlignCI));
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
    const TypeInfo TextFormatAlignTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_text::TextFormatAlign::InstanceType),
        0,
        ClassTraits::fl_text::TextFormatAlign::MemberInfoNum,
        0,
        0,
        "TextFormatAlign", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextFormatAlignCI = {
        &TextFormatAlignTI,
        ClassTraits::fl_text::TextFormatAlign::MakeClassTraits,
        NULL,
        ClassTraits::fl_text::TextFormatAlign::mi,
        NULL,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

