//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_InteractiveObject.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_InteractiveObject.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_AvmInteractiveObj.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_contextMenuGet, SPtr<Instances::fl::Object> > TFunc_Instances_InteractiveObject_contextMenuGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_contextMenuSet, const Value, const Value&> TFunc_Instances_InteractiveObject_contextMenuSet;
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_doubleClickEnabledGet, bool> TFunc_Instances_InteractiveObject_doubleClickEnabledGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_doubleClickEnabledSet, const Value, bool> TFunc_Instances_InteractiveObject_doubleClickEnabledSet;
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_focusRectGet, Value> TFunc_Instances_InteractiveObject_focusRectGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_focusRectSet, const Value, const Value&> TFunc_Instances_InteractiveObject_focusRectSet;
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_mouseEnabledGet, bool> TFunc_Instances_InteractiveObject_mouseEnabledGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_mouseEnabledSet, const Value, bool> TFunc_Instances_InteractiveObject_mouseEnabledSet;
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_tabEnabledGet, bool> TFunc_Instances_InteractiveObject_tabEnabledGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_tabEnabledSet, const Value, bool> TFunc_Instances_InteractiveObject_tabEnabledSet;
typedef ThunkFunc0<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_tabIndexGet, SInt32> TFunc_Instances_InteractiveObject_tabIndexGet;
typedef ThunkFunc1<Instances::fl_display::InteractiveObject, Instances::fl_display::InteractiveObject::mid_tabIndexSet, const Value, SInt32> TFunc_Instances_InteractiveObject_tabIndexSet;

template <> const TFunc_Instances_InteractiveObject_contextMenuGet::TMethod TFunc_Instances_InteractiveObject_contextMenuGet::Method = &Instances::fl_display::InteractiveObject::contextMenuGet;
template <> const TFunc_Instances_InteractiveObject_contextMenuSet::TMethod TFunc_Instances_InteractiveObject_contextMenuSet::Method = &Instances::fl_display::InteractiveObject::contextMenuSet;
template <> const TFunc_Instances_InteractiveObject_doubleClickEnabledGet::TMethod TFunc_Instances_InteractiveObject_doubleClickEnabledGet::Method = &Instances::fl_display::InteractiveObject::doubleClickEnabledGet;
template <> const TFunc_Instances_InteractiveObject_doubleClickEnabledSet::TMethod TFunc_Instances_InteractiveObject_doubleClickEnabledSet::Method = &Instances::fl_display::InteractiveObject::doubleClickEnabledSet;
template <> const TFunc_Instances_InteractiveObject_focusRectGet::TMethod TFunc_Instances_InteractiveObject_focusRectGet::Method = &Instances::fl_display::InteractiveObject::focusRectGet;
template <> const TFunc_Instances_InteractiveObject_focusRectSet::TMethod TFunc_Instances_InteractiveObject_focusRectSet::Method = &Instances::fl_display::InteractiveObject::focusRectSet;
template <> const TFunc_Instances_InteractiveObject_mouseEnabledGet::TMethod TFunc_Instances_InteractiveObject_mouseEnabledGet::Method = &Instances::fl_display::InteractiveObject::mouseEnabledGet;
template <> const TFunc_Instances_InteractiveObject_mouseEnabledSet::TMethod TFunc_Instances_InteractiveObject_mouseEnabledSet::Method = &Instances::fl_display::InteractiveObject::mouseEnabledSet;
template <> const TFunc_Instances_InteractiveObject_tabEnabledGet::TMethod TFunc_Instances_InteractiveObject_tabEnabledGet::Method = &Instances::fl_display::InteractiveObject::tabEnabledGet;
template <> const TFunc_Instances_InteractiveObject_tabEnabledSet::TMethod TFunc_Instances_InteractiveObject_tabEnabledSet::Method = &Instances::fl_display::InteractiveObject::tabEnabledSet;
template <> const TFunc_Instances_InteractiveObject_tabIndexGet::TMethod TFunc_Instances_InteractiveObject_tabIndexGet::Method = &Instances::fl_display::InteractiveObject::tabIndexGet;
template <> const TFunc_Instances_InteractiveObject_tabIndexSet::TMethod TFunc_Instances_InteractiveObject_tabIndexSet::Method = &Instances::fl_display::InteractiveObject::tabIndexSet;

namespace Instances { namespace fl_display
{
    InteractiveObject::InteractiveObject(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::InteractiveObject::InteractiveObject()$data"
//##protect##"instance::InteractiveObject::InteractiveObject()$data"
    {
//##protect##"instance::InteractiveObject::InteractiveObject()$code"
//##protect##"instance::InteractiveObject::InteractiveObject()$code"
    }

    void InteractiveObject::contextMenuGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::InteractiveObject::contextMenuGet()"
        SF_UNUSED1(result);
        result = pContextMenu;
//##protect##"instance::InteractiveObject::contextMenuGet()"
    }
    void InteractiveObject::contextMenuSet(const Value& result, const Value& value)
    {
//##protect##"instance::InteractiveObject::contextMenuSet()"
        SF_UNUSED2(result, value);
        if (value.IsObject() && GetVM().GetValueTraits(value).IsInstanceTraits())
            pContextMenu = static_cast<Instances::fl::Object*>(value.GetObject());
        else
        {
            return GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                SF_DEBUG_ARG(GetVM().GetValueTraits(value).GetName().ToCStr())
                SF_DEBUG_ARG("flash.ui.ContextMenu")
                ));
        }
//##protect##"instance::InteractiveObject::contextMenuSet()"
    }
    void InteractiveObject::doubleClickEnabledGet(bool& result)
    {
//##protect##"instance::InteractiveObject::doubleClickEnabledGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        result = ToAvmInteractiveObj(pDispObj->CharToInteractiveObject())->IsDoubleClickEnabled();
//##protect##"instance::InteractiveObject::doubleClickEnabledGet()"
    }
    void InteractiveObject::doubleClickEnabledSet(const Value& result, bool value)
    {
//##protect##"instance::InteractiveObject::doubleClickEnabledSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        ToAvmInteractiveObj(pDispObj->CharToInteractiveObject())->SetDoubleClickEnabled(value);
//##protect##"instance::InteractiveObject::doubleClickEnabledSet()"
    }
    void InteractiveObject::focusRectGet(Value& result)
    {
//##protect##"instance::InteractiveObject::focusRectGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        if (GetIntObj()->IsTabEnabledFlagDefined())
            result.SetBool(GetIntObj()->IsTabEnabledFlagTrue());
        else
            result.SetNull();
//##protect##"instance::InteractiveObject::focusRectGet()"
    }
    void InteractiveObject::focusRectSet(const Value& result, const Value& value)
    {
//##protect##"instance::InteractiveObject::focusRectSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());

        if (!value.IsUndefined())
        {
            GetIntObj()->SetFocusRectFlag(value.Convert2Boolean());
        }
        else
            GetIntObj()->UndefineTabEnabledFlag();
//##protect##"instance::InteractiveObject::focusRectSet()"
    }
    void InteractiveObject::mouseEnabledGet(bool& result)
    {
//##protect##"instance::InteractiveObject::mouseEnabledGet()"
        result = !pDispObj->CharToInteractiveObject()->IsMouseDisabledFlagSet();
//##protect##"instance::InteractiveObject::mouseEnabledGet()"
    }
    void InteractiveObject::mouseEnabledSet(const Value& result, bool value)
    {
//##protect##"instance::InteractiveObject::mouseEnabledSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);

        pDispObj->CharToInteractiveObject()->SetMouseDisabledFlag(!value);
//##protect##"instance::InteractiveObject::mouseEnabledSet()"
    }
    void InteractiveObject::tabEnabledGet(bool& result)
    {
//##protect##"instance::InteractiveObject::tabEnabledGet()"
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        SF_UNUSED1(result);
        result = GetIntObj()->IsTabEnabledFlagTrue();
//##protect##"instance::InteractiveObject::tabEnabledGet()"
    }
    void InteractiveObject::tabEnabledSet(const Value& result, bool value)
    {
//##protect##"instance::InteractiveObject::tabEnabledSet()"
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        SF_UNUSED2(result, value);
        GetIntObj()->SetTabEnabledFlag(value);

        // fire tabChildrenChange event
        SPtr<fl_events::Event> e = 
            CreateEventObject(GetVM().GetStringManager().CreateConstString("tabEnabledChange"), true, false);
        Dispatch(e, pDispObj);

//##protect##"instance::InteractiveObject::tabEnabledSet()"
    }
    void InteractiveObject::tabIndexGet(SInt32& result)
    {
//##protect##"instance::InteractiveObject::tabIndexGet()"
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        SF_UNUSED1(result);
        result = GetIntObj()->GetTabIndex();
//##protect##"instance::InteractiveObject::tabIndexGet()"
    }
    void InteractiveObject::tabIndexSet(const Value& result, SInt32 value)
    {
//##protect##"instance::InteractiveObject::tabIndexSet()"
        SF_UNUSED2(result, value);
        GetIntObj()->SetTabIndex(value);

        // fire tabChildrenChange event
        SPtr<fl_events::Event> e = 
            CreateEventObject(GetVM().GetStringManager().CreateConstString("tabIndexChange"), true, false);
        Dispatch(e, pDispObj);

//##protect##"instance::InteractiveObject::tabIndexSet()"
    }

//##protect##"instance$methods"
    void InteractiveObject::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        DisplayObject::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<Instances::fl::Object, Mem_Stat>(prcc, pContextMenu, op SF_DEBUG_ARG(*this));
    }

    InteractiveObject::~InteractiveObject()
    {
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 InteractiveObject::tito[InteractiveObject::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 
    // };
    const TypeInfo* InteractiveObject::tit[18] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, NULL, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
    };
    const ThunkInfo InteractiveObject::ti[InteractiveObject::ThunkInfoNum] = {
        {TFunc_Instances_InteractiveObject_contextMenuGet::Func, &InteractiveObject::tit[0], "contextMenu", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_contextMenuSet::Func, &InteractiveObject::tit[1], "contextMenu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_doubleClickEnabledGet::Func, &InteractiveObject::tit[3], "doubleClickEnabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_doubleClickEnabledSet::Func, &InteractiveObject::tit[4], "doubleClickEnabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_focusRectGet::Func, &InteractiveObject::tit[6], "focusRect", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_focusRectSet::Func, &InteractiveObject::tit[7], "focusRect", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_mouseEnabledGet::Func, &InteractiveObject::tit[9], "mouseEnabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_mouseEnabledSet::Func, &InteractiveObject::tit[10], "mouseEnabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_tabEnabledGet::Func, &InteractiveObject::tit[12], "tabEnabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_tabEnabledSet::Func, &InteractiveObject::tit[13], "tabEnabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_tabIndexGet::Func, &InteractiveObject::tit[15], "tabIndex", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_InteractiveObject_tabIndexSet::Func, &InteractiveObject::tit[16], "tabIndex", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

    InteractiveObject::InteractiveObject(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::InteractiveObject::InteractiveObject()"
        SetTraitsType(Traits_InteractiveObject);
//##protect##"InstanceTraits::InteractiveObject::InteractiveObject()"

    }

    void InteractiveObject::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<InteractiveObject&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    InteractiveObject::InteractiveObject(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::InteractiveObject::InteractiveObject()"
        SetTraitsType(Traits_InteractiveObject);
//##protect##"ClassTraits::InteractiveObject::InteractiveObject()"

    }

    Pickable<Traits> InteractiveObject::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InteractiveObject(vm, AS3::fl_display::InteractiveObjectCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::InteractiveObjectCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo InteractiveObjectTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::InteractiveObject::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::InteractiveObject::ThunkInfoNum,
        0,
        "InteractiveObject", "flash.display", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo InteractiveObjectCI = {
        &InteractiveObjectTI,
        ClassTraits::fl_display::InteractiveObject::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::InteractiveObject::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

