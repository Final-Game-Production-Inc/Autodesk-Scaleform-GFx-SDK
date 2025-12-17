//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenuClipboardItems.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_ContextMenuClipboardItems.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_ui
{
    ContextMenuClipboardItems::ContextMenuClipboardItems(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , clear(false)
    , copy(false)
    , cut(false)
    , paste(false)
    , selectAll(true)
//##protect##"instance::ContextMenuClipboardItems::ContextMenuClipboardItems()$data"
//##protect##"instance::ContextMenuClipboardItems::ContextMenuClipboardItems()$data"
    {
//##protect##"instance::ContextMenuClipboardItems::ContextMenuClipboardItems()$code"
//##protect##"instance::ContextMenuClipboardItems::ContextMenuClipboardItems()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    const MemberInfo ContextMenuClipboardItems::mi[ContextMenuClipboardItems::MemberInfoNum] = {
        {"clear", NULL, OFFSETOF(Instances::fl_ui::ContextMenuClipboardItems, clear), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"copy", NULL, OFFSETOF(Instances::fl_ui::ContextMenuClipboardItems, copy), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"cut", NULL, OFFSETOF(Instances::fl_ui::ContextMenuClipboardItems, cut), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"paste", NULL, OFFSETOF(Instances::fl_ui::ContextMenuClipboardItems, paste), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"selectAll", NULL, OFFSETOF(Instances::fl_ui::ContextMenuClipboardItems, selectAll), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    ContextMenuClipboardItems::ContextMenuClipboardItems(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::ContextMenuClipboardItems::ContextMenuClipboardItems()"
//##protect##"InstanceTraits::ContextMenuClipboardItems::ContextMenuClipboardItems()"

    }

    void ContextMenuClipboardItems::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ContextMenuClipboardItems&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_ui
{

    ContextMenuClipboardItems::ContextMenuClipboardItems(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ContextMenuClipboardItems::ContextMenuClipboardItems()"
//##protect##"ClassTraits::ContextMenuClipboardItems::ContextMenuClipboardItems()"

    }

    Pickable<Traits> ContextMenuClipboardItems::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ContextMenuClipboardItems(vm, AS3::fl_ui::ContextMenuClipboardItemsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::ContextMenuClipboardItemsCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_ui
{
    const TypeInfo ContextMenuClipboardItemsTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::ContextMenuClipboardItems::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_ui::ContextMenuClipboardItems::MemberInfoNum,
        "ContextMenuClipboardItems", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ContextMenuClipboardItemsCI = {
        &ContextMenuClipboardItemsTI,
        ClassTraits::fl_ui::ContextMenuClipboardItems::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_ui::ContextMenuClipboardItems::mi,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

