//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextLineMetrics.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextLineMetrics.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_text
{
    TextLineMetrics::TextLineMetrics(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , ascent()
    , descent()
    , height()
    , leading()
    , width()
    , x()
//##protect##"instance::TextLineMetrics::TextLineMetrics()$data"
//##protect##"instance::TextLineMetrics::TextLineMetrics()$data"
    {
//##protect##"instance::TextLineMetrics::TextLineMetrics()$code"
//##protect##"instance::TextLineMetrics::TextLineMetrics()$code"
    }


//##protect##"instance$methods"
    void TextLineMetrics::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc > 0)
        {
            if (argc >= 1) argv[0].Convert2Number(ascent).DoNotCheck();
            if (argc >= 2) argv[1].Convert2Number(descent).DoNotCheck();
            if (argc >= 3) argv[2].Convert2Number(height).DoNotCheck();
            if (argc >= 4) argv[3].Convert2Number(leading).DoNotCheck();
            if (argc >= 5) argv[4].Convert2Number(width).DoNotCheck();
            if (argc >= 6) argv[5].Convert2Number(x).DoNotCheck();
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    const MemberInfo TextLineMetrics::mi[TextLineMetrics::MemberInfoNum] = {
        {"ascent", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, ascent), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"descent", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, descent), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"height", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, height), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"leading", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, leading), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"width", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, width), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"x", NULL, OFFSETOF(Instances::fl_text::TextLineMetrics, x), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    TextLineMetrics::TextLineMetrics(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::TextLineMetrics::TextLineMetrics()"
//##protect##"InstanceTraits::TextLineMetrics::TextLineMetrics()"

    }

    void TextLineMetrics::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<TextLineMetrics&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_text
{

    TextLineMetrics::TextLineMetrics(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextLineMetrics::TextLineMetrics()"
//##protect##"ClassTraits::TextLineMetrics::TextLineMetrics()"

    }

    Pickable<Traits> TextLineMetrics::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextLineMetrics(vm, AS3::fl_text::TextLineMetricsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextLineMetricsCI));
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
    const TypeInfo TextLineMetricsTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_text::TextLineMetrics::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_text::TextLineMetrics::MemberInfoNum,
        "TextLineMetrics", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextLineMetricsCI = {
        &TextLineMetricsTI,
        ClassTraits::fl_text::TextLineMetrics::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_text::TextLineMetrics::mi,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

