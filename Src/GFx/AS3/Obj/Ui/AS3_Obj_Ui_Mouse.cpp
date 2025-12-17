//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Mouse.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_Mouse.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_ui
{
    Mouse::Mouse(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Mouse::Mouse()"
//##protect##"class_::Mouse::Mouse()"
    }
    void Mouse::cursorSet(const Value& result, const ASString& value)
    {
//##protect##"class_::Mouse::cursorSet()"
        SF_UNUSED2(result, value);
        // cursor property is supported for mouseIndex = 0 only
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.GetMovieRoot()->SetMouseCursorType(value, 0);
//##protect##"class_::Mouse::cursorSet()"
    }
    void Mouse::cursorGet(ASString& result)
    {
//##protect##"class_::Mouse::cursorGet()"
        SF_UNUSED1(result);
        // cursor property is supported for mouseIndex = 0 only
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.GetMovieRoot()->GetMouseCursorType(result, 0);
//##protect##"class_::Mouse::cursorGet()"
    }
    void Mouse::hide(const Value& result)
    {
//##protect##"class_::Mouse::hide()"
        SF_UNUSED1(result);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        if (proot->pUserEventHandler)
        {
            unsigned mouseIndex = 0;
            //if (fn.NArgs >= 1)
            //    mouseIndex = fn.Arg(0).ToUInt32(fn.Env);
            proot->pUserEventHandler->HandleEvent(proot,
                GFx::MouseCursorEvent(GFx::Event::DoHideMouse, mouseIndex));
        }
        else if (proot->GetLogState())
            proot->GetLogState()->LogScriptWarning(
                 "No user event handler interface is installed; Mouse.hide failed.");
//##protect##"class_::Mouse::hide()"
    }
    void Mouse::show(const Value& result)
    {
//##protect##"class_::Mouse::show()"
        SF_UNUSED1(result);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        if (proot->pUserEventHandler)
        {
            unsigned mouseIndex = 0;
            //if (fn.NArgs >= 1)
            //    mouseIndex = fn.Arg(0).ToUInt32(fn.Env);
            proot->pUserEventHandler->HandleEvent(proot,
                GFx::MouseCursorEvent(GFx::Event::DoShowMouse, mouseIndex));
        }
        else if (proot->GetLogState())
            proot->GetLogState()->LogScriptWarning(
                "No user event handler interface is installed; Mouse.hide failed.");
//##protect##"class_::Mouse::show()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_ui::Mouse, Classes::fl_ui::Mouse::mid_cursorSet, const Value, const ASString&> TFunc_Classes_Mouse_cursorSet;
typedef ThunkFunc0<Classes::fl_ui::Mouse, Classes::fl_ui::Mouse::mid_cursorGet, ASString> TFunc_Classes_Mouse_cursorGet;
typedef ThunkFunc0<Classes::fl_ui::Mouse, Classes::fl_ui::Mouse::mid_hide, const Value> TFunc_Classes_Mouse_hide;
typedef ThunkFunc0<Classes::fl_ui::Mouse, Classes::fl_ui::Mouse::mid_show, const Value> TFunc_Classes_Mouse_show;

template <> const TFunc_Classes_Mouse_cursorSet::TMethod TFunc_Classes_Mouse_cursorSet::Method = &Classes::fl_ui::Mouse::cursorSet;
template <> const TFunc_Classes_Mouse_cursorGet::TMethod TFunc_Classes_Mouse_cursorGet::Method = &Classes::fl_ui::Mouse::cursorGet;
template <> const TFunc_Classes_Mouse_hide::TMethod TFunc_Classes_Mouse_hide::Method = &Classes::fl_ui::Mouse::hide;
template <> const TFunc_Classes_Mouse_show::TMethod TFunc_Classes_Mouse_show::Method = &Classes::fl_ui::Mouse::show;

namespace ClassTraits { namespace fl_ui
{
    // const UInt16 Mouse::tito[Mouse::ThunkInfoNum] = {
    //    0, 2, 3, 4, 
    // };
    const TypeInfo* Mouse::tit[5] = {
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, 
        NULL, 
    };
    const ThunkInfo Mouse::ti[Mouse::ThunkInfoNum] = {
        {TFunc_Classes_Mouse_cursorSet::Func, &Mouse::tit[0], "cursor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Mouse_cursorGet::Func, &Mouse::tit[2], "cursor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Mouse_hide::Func, &Mouse::tit[3], "hide", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Mouse_show::Func, &Mouse::tit[4], "show", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Mouse::Mouse(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Mouse::Mouse()"
//##protect##"ClassTraits::Mouse::Mouse()"

    }

    Pickable<Traits> Mouse::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Mouse(vm, AS3::fl_ui::MouseCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::MouseCI));
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
    const TypeInfo MouseTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::Mouse::InstanceType),
        ClassTraits::fl_ui::Mouse::ThunkInfoNum,
        0,
        0,
        0,
        "Mouse", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo MouseCI = {
        &MouseTI,
        ClassTraits::fl_ui::Mouse::MakeClassTraits,
        ClassTraits::fl_ui::Mouse::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

