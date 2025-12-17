//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_InteractiveObjectEx.cpp
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_InteractiveObjectEx.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    InteractiveObjectEx::InteractiveObjectEx(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::InteractiveObjectEx::InteractiveObjectEx()"
//##protect##"class_::InteractiveObjectEx::InteractiveObjectEx()"
    }
    void InteractiveObjectEx::setHitTestDisable(const Value& result, Instances::fl_display::InteractiveObject* o, bool f)
    {
//##protect##"class_::InteractiveObjectEx::setHitTestDisable()"
        SF_UNUSED3(result, o, f);
        if (o && o->pDispObj && o->pDispObj->IsInteractiveObject())
        {
            o->pDispObj->CharToInteractiveObject_Unsafe()->SetHitTestDisableFlag(f);
        }
//##protect##"class_::InteractiveObjectEx::setHitTestDisable()"
    }
    void InteractiveObjectEx::getHitTestDisable(bool& result, Instances::fl_display::InteractiveObject* o)
    {
//##protect##"class_::InteractiveObjectEx::getHitTestDisable()"
        SF_UNUSED2(result, o);
        if (o && o->pDispObj && o->pDispObj->IsInteractiveObject())
        {
            result = o->pDispObj->CharToInteractiveObject_Unsafe()->IsHitTestDisableFlagSet();
        }
        else
            result = false;
//##protect##"class_::InteractiveObjectEx::getHitTestDisable()"
    }
    void InteractiveObjectEx::setTopmostLevel(const Value& result, Instances::fl_display::InteractiveObject* o, bool f)
    {
//##protect##"class_::InteractiveObjectEx::setTopmostLevel()"
        SF_UNUSED3(result, o, f);
        if (o && o->pDispObj && o->pDispObj->IsInteractiveObject())
        {
            GFx::InteractiveObject* iobj = o->pDispObj->CharToInteractiveObject_Unsafe();
            iobj->SetTopmostLevelFlag(f);
            if (iobj->IsTopmostLevelFlagSet())
                static_cast<ASVM&>(GetVM()).GetMovieImpl()->AddTopmostLevelCharacter(iobj);
            else
                static_cast<ASVM&>(GetVM()).GetMovieImpl()->RemoveTopmostLevelCharacter(iobj);
        }
//##protect##"class_::InteractiveObjectEx::setTopmostLevel()"
    }
    void InteractiveObjectEx::getTopmostLevel(bool& result, Instances::fl_display::InteractiveObject* o)
    {
//##protect##"class_::InteractiveObjectEx::getTopmostLevel()"
        SF_UNUSED2(result, o);
        if (o && o->pDispObj && o->pDispObj->IsInteractiveObject())
        {
            result = o->pDispObj->CharToInteractiveObject_Unsafe()->IsTopmostLevelFlagSet();
        }
        else
            result = false;
//##protect##"class_::InteractiveObjectEx::getTopmostLevel()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc2<Classes::fl_gfx::InteractiveObjectEx, Classes::fl_gfx::InteractiveObjectEx::mid_setHitTestDisable, const Value, Instances::fl_display::InteractiveObject*, bool> TFunc_Classes_InteractiveObjectEx_setHitTestDisable;
typedef ThunkFunc1<Classes::fl_gfx::InteractiveObjectEx, Classes::fl_gfx::InteractiveObjectEx::mid_getHitTestDisable, bool, Instances::fl_display::InteractiveObject*> TFunc_Classes_InteractiveObjectEx_getHitTestDisable;
typedef ThunkFunc2<Classes::fl_gfx::InteractiveObjectEx, Classes::fl_gfx::InteractiveObjectEx::mid_setTopmostLevel, const Value, Instances::fl_display::InteractiveObject*, bool> TFunc_Classes_InteractiveObjectEx_setTopmostLevel;
typedef ThunkFunc1<Classes::fl_gfx::InteractiveObjectEx, Classes::fl_gfx::InteractiveObjectEx::mid_getTopmostLevel, bool, Instances::fl_display::InteractiveObject*> TFunc_Classes_InteractiveObjectEx_getTopmostLevel;

template <> const TFunc_Classes_InteractiveObjectEx_setHitTestDisable::TMethod TFunc_Classes_InteractiveObjectEx_setHitTestDisable::Method = &Classes::fl_gfx::InteractiveObjectEx::setHitTestDisable;
template <> const TFunc_Classes_InteractiveObjectEx_getHitTestDisable::TMethod TFunc_Classes_InteractiveObjectEx_getHitTestDisable::Method = &Classes::fl_gfx::InteractiveObjectEx::getHitTestDisable;
template <> const TFunc_Classes_InteractiveObjectEx_setTopmostLevel::TMethod TFunc_Classes_InteractiveObjectEx_setTopmostLevel::Method = &Classes::fl_gfx::InteractiveObjectEx::setTopmostLevel;
template <> const TFunc_Classes_InteractiveObjectEx_getTopmostLevel::TMethod TFunc_Classes_InteractiveObjectEx_getTopmostLevel::Method = &Classes::fl_gfx::InteractiveObjectEx::getTopmostLevel;

namespace ClassTraits { namespace fl_gfx
{
    // const UInt16 InteractiveObjectEx::tito[InteractiveObjectEx::ThunkInfoNum] = {
    //    0, 3, 5, 8, 
    // };
    const TypeInfo* InteractiveObjectEx::tit[10] = {
        NULL, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::InteractiveObjectTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::InteractiveObjectTI, 
    };
    const ThunkInfo InteractiveObjectEx::ti[InteractiveObjectEx::ThunkInfoNum] = {
        {TFunc_Classes_InteractiveObjectEx_setHitTestDisable::Func, &InteractiveObjectEx::tit[0], "setHitTestDisable", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_InteractiveObjectEx_getHitTestDisable::Func, &InteractiveObjectEx::tit[3], "getHitTestDisable", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_InteractiveObjectEx_setTopmostLevel::Func, &InteractiveObjectEx::tit[5], "setTopmostLevel", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_InteractiveObjectEx_getTopmostLevel::Func, &InteractiveObjectEx::tit[8], "getTopmostLevel", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    InteractiveObjectEx::InteractiveObjectEx(VM& vm, const ClassInfo& ci)
    : fl_gfx::DisplayObjectEx(vm, ci)
    {
//##protect##"ClassTraits::InteractiveObjectEx::InteractiveObjectEx()"
//##protect##"ClassTraits::InteractiveObjectEx::InteractiveObjectEx()"

    }

    Pickable<Traits> InteractiveObjectEx::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) InteractiveObjectEx(vm, AS3::fl_gfx::InteractiveObjectExCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::InteractiveObjectExCI));
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
    const TypeInfo InteractiveObjectExTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_gfx::InteractiveObjectEx::InstanceType),
        ClassTraits::fl_gfx::InteractiveObjectEx::ThunkInfoNum,
        0,
        0,
        0,
        "InteractiveObjectEx", "scaleform.gfx", &fl_gfx::DisplayObjectExTI,
        TypeInfo::None
    };

    const ClassInfo InteractiveObjectExCI = {
        &InteractiveObjectExTI,
        ClassTraits::fl_gfx::InteractiveObjectEx::MakeClassTraits,
        ClassTraits::fl_gfx::InteractiveObjectEx::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

