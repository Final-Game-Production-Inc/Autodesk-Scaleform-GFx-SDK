//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLWindowCreateOptions.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Html_HTMLWindowCreateOptions.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_html
{
    HTMLWindowCreateOptions::HTMLWindowCreateOptions(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , fullscreen(false)
    , height(NumberUtil::NaN())
    , locationBarVisible(false)
    , menuBarVisible(false)
    , resizable(false)
    , scrollBarsVisible(true)
    , statusBarVisible(false)
    , toolBarVisible(false)
    , width(NumberUtil::NaN())
    , x(NumberUtil::NaN())
    , y(NumberUtil::NaN())
//##protect##"instance::HTMLWindowCreateOptions::HTMLWindowCreateOptions()$data"
//##protect##"instance::HTMLWindowCreateOptions::HTMLWindowCreateOptions()$data"
    {
//##protect##"instance::HTMLWindowCreateOptions::HTMLWindowCreateOptions()$code"
//##protect##"instance::HTMLWindowCreateOptions::HTMLWindowCreateOptions()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_html
{
    const MemberInfo HTMLWindowCreateOptions::mi[HTMLWindowCreateOptions::MemberInfoNum] = {
        {"fullscreen", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, fullscreen), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"height", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, height), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"locationBarVisible", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, locationBarVisible), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"menuBarVisible", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, menuBarVisible), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"resizable", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, resizable), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"scrollBarsVisible", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, scrollBarsVisible), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"statusBarVisible", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, statusBarVisible), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"toolBarVisible", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, toolBarVisible), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"width", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, width), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"x", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, x), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"y", NULL, OFFSETOF(Instances::fl_html::HTMLWindowCreateOptions, y), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    HTMLWindowCreateOptions::HTMLWindowCreateOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::HTMLWindowCreateOptions::HTMLWindowCreateOptions()"
//##protect##"InstanceTraits::HTMLWindowCreateOptions::HTMLWindowCreateOptions()"

    }

    void HTMLWindowCreateOptions::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<HTMLWindowCreateOptions&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_html
{

    HTMLWindowCreateOptions::HTMLWindowCreateOptions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::HTMLWindowCreateOptions::HTMLWindowCreateOptions()"
//##protect##"ClassTraits::HTMLWindowCreateOptions::HTMLWindowCreateOptions()"

    }

    Pickable<Traits> HTMLWindowCreateOptions::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLWindowCreateOptions(vm, AS3::fl_html::HTMLWindowCreateOptionsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_html::HTMLWindowCreateOptionsCI));
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
    const TypeInfo HTMLWindowCreateOptionsTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_html::HTMLWindowCreateOptions::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_html::HTMLWindowCreateOptions::MemberInfoNum,
        "HTMLWindowCreateOptions", "flash.html", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo HTMLWindowCreateOptionsCI = {
        &HTMLWindowCreateOptionsTI,
        ClassTraits::fl_html::HTMLWindowCreateOptions::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_html::HTMLWindowCreateOptions::mi,
    };
}; // namespace fl_html


}}} // namespace Scaleform { namespace GFx { namespace AS3

