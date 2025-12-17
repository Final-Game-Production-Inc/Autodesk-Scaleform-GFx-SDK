//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_StaticText.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_StaticText.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//#include "GFx/AS3/AS3_MovieRoot.h"
#include "Kernel/SF_HeapNew.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_text::StaticText, Instances::fl_text::StaticText::mid_textGet, ASString> TFunc_Instances_StaticText_textGet;

template <> const TFunc_Instances_StaticText_textGet::TMethod TFunc_Instances_StaticText_textGet::Method = &Instances::fl_text::StaticText::textGet;

namespace Instances { namespace fl_text
{
    StaticText::StaticText(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::StaticText::StaticText()$data"
//##protect##"instance::StaticText::StaticText()$data"
    {
//##protect##"instance::StaticText::StaticText()$code"
//##protect##"instance::StaticText::StaticText()$code"
    }

    void StaticText::textGet(ASString& result)
    {
//##protect##"instance::StaticText::textGet()"
        SF_UNUSED1(result);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        StaticTextCharacter* pch = GetStaticTextChar();
        GFx::StaticTextSnapshotData* snapShot = SF_HEAP_NEW(GetVM().GetMemoryHeap()) GFx::StaticTextSnapshotData();
        snapShot->Add(pch);
        UPInt n = snapShot->GetCharCount();
        result = GetVM().GetStringManager().CreateString(snapShot->GetSubString(0, n, true));
        delete snapShot;
#endif // #ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
//##protect##"instance::StaticText::textGet()"
    }

//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 StaticText::tito[StaticText::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* StaticText::tit[1] = {
        &AS3::fl::StringTI, 
    };
    const ThunkInfo StaticText::ti[StaticText::ThunkInfoNum] = {
        {TFunc_Instances_StaticText_textGet::Func, &StaticText::tit[0], "text", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    StaticText::StaticText(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::StaticText::StaticText()"
//##protect##"InstanceTraits::StaticText::StaticText()"

    }

    void StaticText::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<StaticText&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    StaticText::StaticText(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::StaticText::StaticText()"
//##protect##"ClassTraits::StaticText::StaticText()"

    }

    Pickable<Traits> StaticText::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) StaticText(vm, AS3::fl_text::StaticTextCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::StaticTextCI));
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
    const TypeInfo StaticTextTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_text::StaticText::InstanceType),
        0,
        0,
        InstanceTraits::fl_text::StaticText::ThunkInfoNum,
        0,
        "StaticText", "flash.text", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo StaticTextCI = {
        &StaticTextTI,
        ClassTraits::fl_text::StaticText::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_text::StaticText::ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

