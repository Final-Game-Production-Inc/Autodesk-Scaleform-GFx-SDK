//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenuBuiltInItems.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_ContextMenuBuiltInItems.h"
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
    ContextMenuBuiltInItems::ContextMenuBuiltInItems(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , forwardAndBack(true)
    , loop(true)
    , play(true)
    , print(true)
    , quality(true)
    , rewind(true)
    , save(true)
    , zoom(true)
//##protect##"instance::ContextMenuBuiltInItems::ContextMenuBuiltInItems()$data"
//##protect##"instance::ContextMenuBuiltInItems::ContextMenuBuiltInItems()$data"
    {
//##protect##"instance::ContextMenuBuiltInItems::ContextMenuBuiltInItems()$code"
//##protect##"instance::ContextMenuBuiltInItems::ContextMenuBuiltInItems()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    const MemberInfo ContextMenuBuiltInItems::mi[ContextMenuBuiltInItems::MemberInfoNum] = {
        {"forwardAndBack", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, forwardAndBack), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"loop", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, loop), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"play", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, play), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"print", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, print), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"quality", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, quality), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"rewind", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, rewind), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"save", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, save), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
        {"zoom", NULL, OFFSETOF(Instances::fl_ui::ContextMenuBuiltInItems, zoom), Abc::NS_Public, SlotInfo::BT_Boolean, 0},
    };


    ContextMenuBuiltInItems::ContextMenuBuiltInItems(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::ContextMenuBuiltInItems::ContextMenuBuiltInItems()"
//##protect##"InstanceTraits::ContextMenuBuiltInItems::ContextMenuBuiltInItems()"

    }

    void ContextMenuBuiltInItems::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ContextMenuBuiltInItems&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_ui
{

    ContextMenuBuiltInItems::ContextMenuBuiltInItems(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ContextMenuBuiltInItems::ContextMenuBuiltInItems()"
//##protect##"ClassTraits::ContextMenuBuiltInItems::ContextMenuBuiltInItems()"

    }

    Pickable<Traits> ContextMenuBuiltInItems::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ContextMenuBuiltInItems(vm, AS3::fl_ui::ContextMenuBuiltInItemsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::ContextMenuBuiltInItemsCI));
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
    const TypeInfo ContextMenuBuiltInItemsTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::ContextMenuBuiltInItems::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_ui::ContextMenuBuiltInItems::MemberInfoNum,
        "ContextMenuBuiltInItems", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ContextMenuBuiltInItemsCI = {
        &ContextMenuBuiltInItemsTI,
        ClassTraits::fl_ui::ContextMenuBuiltInItems::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_ui::ContextMenuBuiltInItems::mi,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

