//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_ContextMenu.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_ContextMenu.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
// #include "../Display/AS3_Obj_Display_NativeMenu.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    namespace fl_display
    {
        class NativeMenu;
    }
}
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_builtInItemsGet, SPtr<Instances::fl_ui::ContextMenuBuiltInItems> > TFunc_Instances_ContextMenu_builtInItemsGet;
typedef ThunkFunc1<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_builtInItemsSet, const Value, Instances::fl_ui::ContextMenuBuiltInItems*> TFunc_Instances_ContextMenu_builtInItemsSet;
typedef ThunkFunc0<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_customItemsGet, SPtr<Instances::fl::Array> > TFunc_Instances_ContextMenu_customItemsGet;
typedef ThunkFunc1<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_customItemsSet, const Value, Instances::fl::Array*> TFunc_Instances_ContextMenu_customItemsSet;
typedef ThunkFunc0<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_clone, SPtr<Instances::fl_display::NativeMenu> > TFunc_Instances_ContextMenu_clone;
typedef ThunkFunc0<Instances::fl_ui::ContextMenu, Instances::fl_ui::ContextMenu::mid_hideBuiltInItems, const Value> TFunc_Instances_ContextMenu_hideBuiltInItems;

template <> const TFunc_Instances_ContextMenu_builtInItemsGet::TMethod TFunc_Instances_ContextMenu_builtInItemsGet::Method = &Instances::fl_ui::ContextMenu::builtInItemsGet;
template <> const TFunc_Instances_ContextMenu_builtInItemsSet::TMethod TFunc_Instances_ContextMenu_builtInItemsSet::Method = &Instances::fl_ui::ContextMenu::builtInItemsSet;
template <> const TFunc_Instances_ContextMenu_customItemsGet::TMethod TFunc_Instances_ContextMenu_customItemsGet::Method = &Instances::fl_ui::ContextMenu::customItemsGet;
template <> const TFunc_Instances_ContextMenu_customItemsSet::TMethod TFunc_Instances_ContextMenu_customItemsSet::Method = &Instances::fl_ui::ContextMenu::customItemsSet;
template <> const TFunc_Instances_ContextMenu_clone::TMethod TFunc_Instances_ContextMenu_clone::Method = &Instances::fl_ui::ContextMenu::clone;
template <> const TFunc_Instances_ContextMenu_hideBuiltInItems::TMethod TFunc_Instances_ContextMenu_hideBuiltInItems::Method = &Instances::fl_ui::ContextMenu::hideBuiltInItems;

namespace Instances { namespace fl_ui
{
    ContextMenu::ContextMenu(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::ContextMenu::ContextMenu()$data"
//##protect##"instance::ContextMenu::ContextMenu()$data"
    {
//##protect##"instance::ContextMenu::ContextMenu()$code"
//##protect##"instance::ContextMenu::ContextMenu()$code"
    }

    void ContextMenu::builtInItemsGet(SPtr<Instances::fl_ui::ContextMenuBuiltInItems>& result)
    {
//##protect##"instance::ContextMenu::builtInItemsGet()"
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::builtInItemsGet()");

        // fl_ui::ContextMenuBuiltInItems is a final class.
        GetVM().ConstructBuiltinObject(result, "flash.ui.ContextMenuBuiltInItems").DoNotCheck();
//##protect##"instance::ContextMenu::builtInItemsGet()"
    }
    void ContextMenu::builtInItemsSet(const Value& result, Instances::fl_ui::ContextMenuBuiltInItems* value)
    {
//##protect##"instance::ContextMenu::builtInItemsSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::builtInItemsSet()");
//##protect##"instance::ContextMenu::builtInItemsSet()"
    }
    void ContextMenu::customItemsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::ContextMenu::customItemsGet()"
        //@TODO
        result = GetVM().MakeArray();
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::customItemsGet()");
//##protect##"instance::ContextMenu::customItemsGet()"
    }
    void ContextMenu::customItemsSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::ContextMenu::customItemsSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::customItemsSet()");
//##protect##"instance::ContextMenu::customItemsSet()"
    }
    void ContextMenu::clone(SPtr<Instances::fl_display::NativeMenu>& result)
    {
//##protect##"instance::ContextMenu::clone()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::clone()");
//##protect##"instance::ContextMenu::clone()"
    }
    void ContextMenu::hideBuiltInItems(const Value& result)
    {
//##protect##"instance::ContextMenu::hideBuiltInItems()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::ContextMenu::hideBuiltInItems()");
//##protect##"instance::ContextMenu::hideBuiltInItems()"
    }

    SPtr<Instances::fl_ui::ContextMenuBuiltInItems> ContextMenu::builtInItemsGet()
    {
        SPtr<Instances::fl_ui::ContextMenuBuiltInItems> result;
        builtInItemsGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> ContextMenu::customItemsGet()
    {
        SPtr<Instances::fl::Array> result;
        customItemsGet(result);
        return result;
    }
//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_ui
{
    // const UInt16 ContextMenu::tito[ContextMenu::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 
    // };
    const TypeInfo* ContextMenu::tit[7] = {
        &AS3::fl_ui::ContextMenuBuiltInItemsTI, 
        NULL, &AS3::fl_ui::ContextMenuBuiltInItemsTI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        NULL, 
    };
    const ThunkInfo ContextMenu::ti[ContextMenu::ThunkInfoNum] = {
        {TFunc_Instances_ContextMenu_builtInItemsGet::Func, &ContextMenu::tit[0], "builtInItems", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ContextMenu_builtInItemsSet::Func, &ContextMenu::tit[1], "builtInItems", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ContextMenu_customItemsGet::Func, &ContextMenu::tit[3], "customItems", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ContextMenu_customItemsSet::Func, &ContextMenu::tit[4], "customItems", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ContextMenu_hideBuiltInItems::Func, &ContextMenu::tit[6], "hideBuiltInItems", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    ContextMenu::ContextMenu(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::ContextMenu::ContextMenu()"
//##protect##"InstanceTraits::ContextMenu::ContextMenu()"

    }

    void ContextMenu::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ContextMenu&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_ui
{

    ContextMenu::ContextMenu(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::ContextMenu::ContextMenu()"
//##protect##"ClassTraits::ContextMenu::ContextMenu()"

    }

    Pickable<Traits> ContextMenu::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ContextMenu(vm, AS3::fl_ui::ContextMenuCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::ContextMenuCI));
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
    const TypeInfo ContextMenuTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::ContextMenu::InstanceType),
        0,
        0,
        InstanceTraits::fl_ui::ContextMenu::ThunkInfoNum,
        0,
        "ContextMenu", "flash.ui", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo ContextMenuCI = {
        &ContextMenuTI,
        ClassTraits::fl_ui::ContextMenu::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_ui::ContextMenu::ti,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

