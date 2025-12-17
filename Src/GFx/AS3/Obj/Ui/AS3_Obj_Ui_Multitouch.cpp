//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Multitouch.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_Multitouch.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "../Vec/AS3_Obj_Vec_Vector_String.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_ui
{
    Multitouch::Multitouch(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Multitouch::Multitouch()"
//##protect##"class_::Multitouch::Multitouch()"
    }
    void Multitouch::inputModeGet(ASString& result)
    {
//##protect##"class_::Multitouch::inputModeGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        switch(asvm.GetMovieImpl()->GetMultitouchInputMode())
        {
        case MovieImpl::MTI_TouchPoint:
            result = asvm.GetStringManager().CreateConstString("touchPoint");
            break;
        case MovieImpl::MTI_Gesture:
            result = asvm.GetStringManager().CreateConstString("gesture");
            break;
        case MovieImpl::MTI_Mixed:
            result = asvm.GetStringManager().CreateConstString("mixed");
            break;
        default:
            result = asvm.GetStringManager().CreateConstString("none");
            break;
        }
//##protect##"class_::Multitouch::inputModeGet()"
    }
    void Multitouch::inputModeSet(const Value& result, const ASString& value)
    {
//##protect##"class_::Multitouch::inputModeSet()"
        SF_UNUSED2(result, value);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        MovieImpl::MultitouchInputMode m = MovieImpl::MTI_None;
        if (value == "touchPoint")
            m = MovieImpl::MTI_TouchPoint;
        else if (value == "gesture")
            m = MovieImpl::MTI_Gesture;
        else if (value == "mixed")
            m = MovieImpl::MTI_Mixed;
        asvm.GetMovieImpl()->SetMultitouchInputMode(m);
//##protect##"class_::Multitouch::inputModeSet()"
    }
    void Multitouch::maxTouchPointsGet(SInt32& result)
    {
//##protect##"class_::Multitouch::maxTouchPointsGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = asvm.GetMovieImpl()->GetMaxTouchPoints();
//##protect##"class_::Multitouch::maxTouchPointsGet()"
    }
    void Multitouch::supportedGesturesGet(SPtr<Instances::fl_vec::Vector_String>& result)
    {
//##protect##"class_::Multitouch::supportedGesturesGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        UInt32 m = asvm.GetMovieImpl()->GetSupportedGesturesMask();
        InstanceTraits::fl_vec::Vector_String& itr = static_cast<InstanceTraits::fl_vec::Vector_String&>(asvm.GetITraitsVectorString());

        // Create Vector.<String>
        result = itr.MakeInstance(itr);
        if (m & MovieImpl::MTG_Pan)
            result->PushBack(asvm.GetStringManager().CreateConstString("pan").GetNode());
        if (m & MovieImpl::MTG_Rotate)
            result->PushBack(asvm.GetStringManager().CreateConstString("rotate").GetNode());
        if (m & MovieImpl::MTG_Swipe)
            result->PushBack(asvm.GetStringManager().CreateConstString("swipe").GetNode());
        if (m & MovieImpl::MTG_Zoom)
            result->PushBack(asvm.GetStringManager().CreateConstString("zoom").GetNode());
//##protect##"class_::Multitouch::supportedGesturesGet()"
    }
    void Multitouch::supportsGestureEventsGet(bool& result)
    {
//##protect##"class_::Multitouch::supportsGestureEventsGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        UInt32 m = asvm.GetMovieImpl()->GetSupportedGesturesMask();
        result = (m != 0);
//##protect##"class_::Multitouch::supportsGestureEventsGet()"
    }
    void Multitouch::supportsTouchEventsGet(bool& result)
    {
//##protect##"class_::Multitouch::supportsTouchEventsGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        UInt32 m = asvm.GetMovieImpl()->GetMaxTouchPoints();
        //result = (m != 0 && (asvm.GetMovieImpl()->GetMultitouchInputMode() & MovieImpl::MTI_TouchPoint) != 0);
        result = (m != 0);
//##protect##"class_::Multitouch::supportsTouchEventsGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_inputModeGet, ASString> TFunc_Classes_Multitouch_inputModeGet;
typedef ThunkFunc1<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_inputModeSet, const Value, const ASString&> TFunc_Classes_Multitouch_inputModeSet;
typedef ThunkFunc0<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_maxTouchPointsGet, SInt32> TFunc_Classes_Multitouch_maxTouchPointsGet;
typedef ThunkFunc0<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_supportedGesturesGet, SPtr<Instances::fl_vec::Vector_String> > TFunc_Classes_Multitouch_supportedGesturesGet;
typedef ThunkFunc0<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_supportsGestureEventsGet, bool> TFunc_Classes_Multitouch_supportsGestureEventsGet;
typedef ThunkFunc0<Classes::fl_ui::Multitouch, Classes::fl_ui::Multitouch::mid_supportsTouchEventsGet, bool> TFunc_Classes_Multitouch_supportsTouchEventsGet;

template <> const TFunc_Classes_Multitouch_inputModeGet::TMethod TFunc_Classes_Multitouch_inputModeGet::Method = &Classes::fl_ui::Multitouch::inputModeGet;
template <> const TFunc_Classes_Multitouch_inputModeSet::TMethod TFunc_Classes_Multitouch_inputModeSet::Method = &Classes::fl_ui::Multitouch::inputModeSet;
template <> const TFunc_Classes_Multitouch_maxTouchPointsGet::TMethod TFunc_Classes_Multitouch_maxTouchPointsGet::Method = &Classes::fl_ui::Multitouch::maxTouchPointsGet;
template <> const TFunc_Classes_Multitouch_supportedGesturesGet::TMethod TFunc_Classes_Multitouch_supportedGesturesGet::Method = &Classes::fl_ui::Multitouch::supportedGesturesGet;
template <> const TFunc_Classes_Multitouch_supportsGestureEventsGet::TMethod TFunc_Classes_Multitouch_supportsGestureEventsGet::Method = &Classes::fl_ui::Multitouch::supportsGestureEventsGet;
template <> const TFunc_Classes_Multitouch_supportsTouchEventsGet::TMethod TFunc_Classes_Multitouch_supportsTouchEventsGet::Method = &Classes::fl_ui::Multitouch::supportsTouchEventsGet;

namespace ClassTraits { namespace fl_ui
{
    // const UInt16 Multitouch::tito[Multitouch::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 6, 
    // };
    const TypeInfo* Multitouch::tit[7] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        NULL, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Multitouch::ti[Multitouch::ThunkInfoNum] = {
        {TFunc_Classes_Multitouch_inputModeGet::Func, &Multitouch::tit[0], "inputMode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Multitouch_inputModeSet::Func, &Multitouch::tit[1], "inputMode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Multitouch_maxTouchPointsGet::Func, &Multitouch::tit[3], "maxTouchPoints", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Multitouch_supportedGesturesGet::Func, &Multitouch::tit[4], "supportedGestures", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Multitouch_supportsGestureEventsGet::Func, &Multitouch::tit[5], "supportsGestureEvents", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Multitouch_supportsTouchEventsGet::Func, &Multitouch::tit[6], "supportsTouchEvents", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Multitouch::Multitouch(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Multitouch::Multitouch()"
//##protect##"ClassTraits::Multitouch::Multitouch()"

    }

    Pickable<Traits> Multitouch::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Multitouch(vm, AS3::fl_ui::MultitouchCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::MultitouchCI));
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
    const TypeInfo MultitouchTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::Multitouch::InstanceType),
        ClassTraits::fl_ui::Multitouch::ThunkInfoNum,
        0,
        0,
        0,
        "Multitouch", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo MultitouchCI = {
        &MultitouchTI,
        ClassTraits::fl_ui::Multitouch::MakeClassTraits,
        ClassTraits::fl_ui::Multitouch::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

