//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_IMECandidateListStyle.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_IMECandidateListStyle.h"
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
    IMECandidateListStyle::IMECandidateListStyle(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , textColor(-1)
    , selectedTextColor(-1)
    , fontSize(-1)
    , backgroundColor(-1)
    , selectedBackgroundColor(-1)
    , indexBackgroundColor(-1)
    , selectedIndexBackgroundColor(-1)
    , readingWindowTextColor(-1)
    , readingWindowBackgroundColor(-1)
    , readingWindowFontSize(-1)
//##protect##"instance::IMECandidateListStyle::IMECandidateListStyle()$data"
//##protect##"instance::IMECandidateListStyle::IMECandidateListStyle()$data"
    {
//##protect##"instance::IMECandidateListStyle::IMECandidateListStyle()$code"
//##protect##"instance::IMECandidateListStyle::IMECandidateListStyle()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    const MemberInfo IMECandidateListStyle::mi[IMECandidateListStyle::MemberInfoNum] = {
        {"textColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, textColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"selectedTextColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, selectedTextColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"fontSize", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, fontSize), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"backgroundColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, backgroundColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"selectedBackgroundColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, selectedBackgroundColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"indexBackgroundColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, indexBackgroundColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"selectedIndexBackgroundColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, selectedIndexBackgroundColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"readingWindowTextColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, readingWindowTextColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"readingWindowBackgroundColor", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, readingWindowBackgroundColor), Abc::NS_Public, SlotInfo::BT_Int, 0},
        {"readingWindowFontSize", NULL, OFFSETOF(Instances::fl_gfx::IMECandidateListStyle, readingWindowFontSize), Abc::NS_Public, SlotInfo::BT_Int, 0},
    };


    IMECandidateListStyle::IMECandidateListStyle(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::IMECandidateListStyle::IMECandidateListStyle()"
//##protect##"InstanceTraits::IMECandidateListStyle::IMECandidateListStyle()"

    }

    void IMECandidateListStyle::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<IMECandidateListStyle&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_gfx
{

    IMECandidateListStyle::IMECandidateListStyle(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IMECandidateListStyle::IMECandidateListStyle()"
//##protect##"ClassTraits::IMECandidateListStyle::IMECandidateListStyle()"

    }

    Pickable<Traits> IMECandidateListStyle::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IMECandidateListStyle(vm, AS3::fl_gfx::IMECandidateListStyleCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::IMECandidateListStyleCI));
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
    const TypeInfo IMECandidateListStyleTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::IMECandidateListStyle::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_gfx::IMECandidateListStyle::MemberInfoNum,
        "IMECandidateListStyle", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo IMECandidateListStyleCI = {
        &IMECandidateListStyleTI,
        ClassTraits::fl_gfx::IMECandidateListStyle::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_gfx::IMECandidateListStyle::mi,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

