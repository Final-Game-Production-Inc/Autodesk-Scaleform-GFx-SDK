//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_Extensions.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_Extensions.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    Extensions::Extensions(ClassTraits::Traits& t)
    : Class(t)
    , EDGEAA_INHERIT(0)
    , EDGEAA_ON(1)
    , EDGEAA_OFF(2)
    , EDGEAA_DISABLE(3)
    {
//##protect##"class_::Extensions::Extensions()"
//##protect##"class_::Extensions::Extensions()"
    }
    void Extensions::enabledSet(const Value& result, bool value)
    {
//##protect##"class_::Extensions::enabledSet()"
        
        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.ExtensionsEnabled = value;

//##protect##"class_::Extensions::enabledSet()"
    }
    void Extensions::enabledGet(bool& result)
    {
//##protect##"class_::Extensions::enabledGet()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        result = asvm.ExtensionsEnabled;

//##protect##"class_::Extensions::enabledGet()"
    }
    void Extensions::noInvisibleAdvanceSet(const Value& result, bool value)
    {
//##protect##"class_::Extensions::noInvisibleAdvanceSet()"
        
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            proot->SetNoInvisibleAdvanceFlag(value);
        }

//##protect##"class_::Extensions::noInvisibleAdvanceSet()"
    }
    void Extensions::noInvisibleAdvanceGet(bool& result)
    {
//##protect##"class_::Extensions::noInvisibleAdvanceGet()"
        
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            result = proot->IsNoInvisibleAdvanceFlagSet();
        }

//##protect##"class_::Extensions::noInvisibleAdvanceGet()"
    }
    void Extensions::getTopMostEntity(SPtr<Instances::fl_display::DisplayObject>& result, Value::Number x, Value::Number y, bool testAll)
    {
//##protect##"class_::Extensions::getTopMostEntity()"
        SF_UNUSED4(result, x, y, testAll);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        MovieImpl* proot = asvm.GetMovieImpl();

        // three parameters version: _x and _y in _root coordinates, optional boolean - testAll
        x = PixelsToTwips(x);
        y = PixelsToTwips(y);
        PointF mousePos;

        if (proot->GetMainContainer())
        {
            // x and y vars are in _root coords. Need to transform them
            // into world coords
            Matrix2F m = proot->GetMainContainer()->GetWorldMatrix();
            PointF a;
            a.SetPoint(float(x), float(y));
            m.Transform(&mousePos, a);
        }
        else
        {
            result = NULL;
            return;
        }
        GFx::InteractiveObject* ptopCh = proot->GetTopMostEntity(mousePos, 0, testAll);
        if (ptopCh)
        {
            AvmInteractiveObj* avobj = ToAvmInteractiveObj(ptopCh);
            avobj->CreateASInstance(true);
            result = avobj->GetAS3Obj();
        }
        else
            result = NULL;
//##protect##"class_::Extensions::getTopMostEntity()"
    }
    void Extensions::getMouseTopMostEntity(SPtr<Instances::fl_display::DisplayObject>& result, bool testAll, UInt32 mouseIndex)
    {
//##protect##"class_::Extensions::getMouseTopMostEntity()"
        SF_UNUSED3(result, testAll, mouseIndex);

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        MovieImpl* proot = asvm.GetMovieImpl();

        if (mouseIndex >= proot->GetMouseCursorCount())
            return; // invalid index of mouse
        SF_ASSERT(proot->GetMouseState(mouseIndex)); // only first mouse is supported for now
        PointF mousePos;
        mousePos = proot->GetMouseState(mouseIndex)->GetLastPosition();
        GFx::InteractiveObject* ptopCh = proot->GetTopMostEntity(mousePos, mouseIndex, testAll);
        if (ptopCh)
        {
            AvmInteractiveObj* avobj = ToAvmInteractiveObj(ptopCh);
            avobj->CreateASInstance(true);
            result = avobj->GetAS3Obj();
        }
        else
            result = NULL;
//##protect##"class_::Extensions::getMouseTopMostEntity()"
    }
    void Extensions::setMouseCursorType(const Value& result, const ASString& cursor, UInt32 mouseIndex)
    {
//##protect##"class_::Extensions::setMouseCursorType()"
        SF_UNUSED3(result, cursor, mouseIndex);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.GetMovieRoot()->SetMouseCursorType(cursor, mouseIndex);
//##protect##"class_::Extensions::setMouseCursorType()"
    }
    void Extensions::getMouseCursorType(ASString& result, UInt32 mouseIndex)
    {
//##protect##"class_::Extensions::getMouseCursorType()"
        SF_UNUSED2(result, mouseIndex);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        asvm.GetMovieRoot()->GetMouseCursorType(result, mouseIndex);
//##protect##"class_::Extensions::getMouseCursorType()"
    }
    void Extensions::getEdgeAAMode(UInt32& result, Instances::fl_display::DisplayObject* dispObj)
    {
//##protect##"class_::Extensions::getEdgeAAMode()"

        GFx::DisplayObject* pd = dispObj->pDispObj;
        SF_ASSERT(pd);
        switch (pd->GetRenderNode()->GetEdgeAAMode())
        {
        case EdgeAA_On:         result = EDGEAA_ON;         break;
        case EdgeAA_Off:        result = EDGEAA_OFF;        break;
        case EdgeAA_Disable:    result = EDGEAA_DISABLE;    break;
        default:                result = EDGEAA_INHERIT;    break;
        }

//##protect##"class_::Extensions::getEdgeAAMode()"
    }
    void Extensions::setEdgeAAMode(const Value& result, Instances::fl_display::DisplayObject* dispObj, UInt32 mode)
    {
//##protect##"class_::Extensions::setEdgeAAMode()"

        SF_UNUSED(result);
        GFx::DisplayObject* pd = dispObj->pDispObj;
        SF_ASSERT(pd);
        EdgeAAMode edgeAA = EdgeAA_Inherit;
        if (mode == EDGEAA_DISABLE) 
            edgeAA = EdgeAA_Disable;
        else
        {
            if (mode == EDGEAA_ON)      edgeAA = EdgeAA_On;
            else if (mode == EDGEAA_OFF)  edgeAA = EdgeAA_Off;
        }
        pd->GetRenderNode()->SetEdgeAAMode(edgeAA);

//##protect##"class_::Extensions::setEdgeAAMode()"
    }
    void Extensions::numControllersGet(UInt32& result)
    {
//##protect##"class_::Extensions::numControllersGet()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            result = proot->GetControllerCount();
        }

//##protect##"class_::Extensions::numControllersGet()"
    }
    void Extensions::visibleRectGet(SPtr<Instances::fl_geom::Rectangle>& result)
    {
//##protect##"class_::Extensions::visibleRectGet()"
		ASVM& vm = static_cast<ASVM&>(GetVM());
		MovieImpl* proot = vm.GetMovieImpl();
		RectF visibleRect = proot->GetVisibleFrameRect();
        // Create Rectangle object
        Value params[] = { Value(visibleRect.X1()), 
                           Value(visibleRect.Y1()), 
                           Value(visibleRect.Width()), 
                           Value(visibleRect.Height()) 
        };
        Value r;
        static_cast<ASVM&>(GetVM()).RectangleClass->Construct(r, 4, params, true);
        result = static_cast<Instances::fl_geom::Rectangle*>(r.GetObject());
//##protect##"class_::Extensions::visibleRectGet()"
    }
    void Extensions::isScaleformGet(bool& result)
    {
//##protect##"class_::Extensions::isScaleformGet()"
        result = true;
//##protect##"class_::Extensions::isScaleformGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_enabledSet, const Value, bool> TFunc_Classes_Extensions_enabledSet;
typedef ThunkFunc0<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_enabledGet, bool> TFunc_Classes_Extensions_enabledGet;
typedef ThunkFunc1<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_noInvisibleAdvanceSet, const Value, bool> TFunc_Classes_Extensions_noInvisibleAdvanceSet;
typedef ThunkFunc0<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_noInvisibleAdvanceGet, bool> TFunc_Classes_Extensions_noInvisibleAdvanceGet;
typedef ThunkFunc3<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_getTopMostEntity, SPtr<Instances::fl_display::DisplayObject>, Value::Number, Value::Number, bool> TFunc_Classes_Extensions_getTopMostEntity;
typedef ThunkFunc2<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_getMouseTopMostEntity, SPtr<Instances::fl_display::DisplayObject>, bool, UInt32> TFunc_Classes_Extensions_getMouseTopMostEntity;
typedef ThunkFunc2<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_setMouseCursorType, const Value, const ASString&, UInt32> TFunc_Classes_Extensions_setMouseCursorType;
typedef ThunkFunc1<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_getMouseCursorType, ASString, UInt32> TFunc_Classes_Extensions_getMouseCursorType;
typedef ThunkFunc1<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_getEdgeAAMode, UInt32, Instances::fl_display::DisplayObject*> TFunc_Classes_Extensions_getEdgeAAMode;
typedef ThunkFunc2<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_setEdgeAAMode, const Value, Instances::fl_display::DisplayObject*, UInt32> TFunc_Classes_Extensions_setEdgeAAMode;
typedef ThunkFunc0<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_numControllersGet, UInt32> TFunc_Classes_Extensions_numControllersGet;
typedef ThunkFunc0<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_visibleRectGet, SPtr<Instances::fl_geom::Rectangle> > TFunc_Classes_Extensions_visibleRectGet;
typedef ThunkFunc0<Classes::fl_gfx::Extensions, Classes::fl_gfx::Extensions::mid_isScaleformGet, bool> TFunc_Classes_Extensions_isScaleformGet;

template <> const TFunc_Classes_Extensions_enabledSet::TMethod TFunc_Classes_Extensions_enabledSet::Method = &Classes::fl_gfx::Extensions::enabledSet;
template <> const TFunc_Classes_Extensions_enabledGet::TMethod TFunc_Classes_Extensions_enabledGet::Method = &Classes::fl_gfx::Extensions::enabledGet;
template <> const TFunc_Classes_Extensions_noInvisibleAdvanceSet::TMethod TFunc_Classes_Extensions_noInvisibleAdvanceSet::Method = &Classes::fl_gfx::Extensions::noInvisibleAdvanceSet;
template <> const TFunc_Classes_Extensions_noInvisibleAdvanceGet::TMethod TFunc_Classes_Extensions_noInvisibleAdvanceGet::Method = &Classes::fl_gfx::Extensions::noInvisibleAdvanceGet;
template <> const TFunc_Classes_Extensions_getTopMostEntity::TMethod TFunc_Classes_Extensions_getTopMostEntity::Method = &Classes::fl_gfx::Extensions::getTopMostEntity;
template <> const TFunc_Classes_Extensions_getMouseTopMostEntity::TMethod TFunc_Classes_Extensions_getMouseTopMostEntity::Method = &Classes::fl_gfx::Extensions::getMouseTopMostEntity;
template <> const TFunc_Classes_Extensions_setMouseCursorType::TMethod TFunc_Classes_Extensions_setMouseCursorType::Method = &Classes::fl_gfx::Extensions::setMouseCursorType;
template <> const TFunc_Classes_Extensions_getMouseCursorType::TMethod TFunc_Classes_Extensions_getMouseCursorType::Method = &Classes::fl_gfx::Extensions::getMouseCursorType;
template <> const TFunc_Classes_Extensions_getEdgeAAMode::TMethod TFunc_Classes_Extensions_getEdgeAAMode::Method = &Classes::fl_gfx::Extensions::getEdgeAAMode;
template <> const TFunc_Classes_Extensions_setEdgeAAMode::TMethod TFunc_Classes_Extensions_setEdgeAAMode::Method = &Classes::fl_gfx::Extensions::setEdgeAAMode;
template <> const TFunc_Classes_Extensions_numControllersGet::TMethod TFunc_Classes_Extensions_numControllersGet::Method = &Classes::fl_gfx::Extensions::numControllersGet;
template <> const TFunc_Classes_Extensions_visibleRectGet::TMethod TFunc_Classes_Extensions_visibleRectGet::Method = &Classes::fl_gfx::Extensions::visibleRectGet;
template <> const TFunc_Classes_Extensions_isScaleformGet::TMethod TFunc_Classes_Extensions_isScaleformGet::Method = &Classes::fl_gfx::Extensions::isScaleformGet;

namespace ClassTraits { namespace fl_gfx
{
    const MemberInfo Extensions::mi[Extensions::MemberInfoNum] = {
        {"EDGEAA_INHERIT", NULL, OFFSETOF(Classes::fl_gfx::Extensions, EDGEAA_INHERIT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"EDGEAA_ON", NULL, OFFSETOF(Classes::fl_gfx::Extensions, EDGEAA_ON), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"EDGEAA_OFF", NULL, OFFSETOF(Classes::fl_gfx::Extensions, EDGEAA_OFF), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"EDGEAA_DISABLE", NULL, OFFSETOF(Classes::fl_gfx::Extensions, EDGEAA_DISABLE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };

    // const UInt16 Extensions::tito[Extensions::ThunkInfoNum] = {
    //    0, 2, 3, 5, 6, 10, 13, 16, 18, 20, 23, 24, 25, 
    // };
    const TypeInfo* Extensions::tit[26] = {
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::BooleanTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::BooleanTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue Extensions::dva[3] = {
        {Abc::CONSTANT_True, 0}, 
        {Abc::CONSTANT_True, 0}, {Abc::CONSTANT_UInt, 0}, 
    };
    const ThunkInfo Extensions::ti[Extensions::ThunkInfoNum] = {
        {TFunc_Classes_Extensions_enabledSet::Func, &Extensions::tit[0], "enabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Extensions_enabledGet::Func, &Extensions::tit[2], "enabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Extensions_noInvisibleAdvanceSet::Func, &Extensions::tit[3], "noInvisibleAdvance", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Extensions_noInvisibleAdvanceGet::Func, &Extensions::tit[5], "noInvisibleAdvance", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Extensions_getTopMostEntity::Func, &Extensions::tit[6], "getTopMostEntity", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 1, &Extensions::dva[0]},
        {TFunc_Classes_Extensions_getMouseTopMostEntity::Func, &Extensions::tit[10], "getMouseTopMostEntity", NULL, Abc::NS_Public, CT_Method, 0, 2, 0, 2, &Extensions::dva[1]},
        {TFunc_Classes_Extensions_setMouseCursorType::Func, &Extensions::tit[13], "setMouseCursorType", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Classes_Extensions_getMouseCursorType::Func, &Extensions::tit[16], "getMouseCursorType", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_Extensions_getEdgeAAMode::Func, &Extensions::tit[18], "getEdgeAAMode", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_Extensions_setEdgeAAMode::Func, &Extensions::tit[20], "setEdgeAAMode", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_Extensions_numControllersGet::Func, &Extensions::tit[23], "numControllers", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Extensions_visibleRectGet::Func, &Extensions::tit[24], "visibleRect", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Extensions_isScaleformGet::Func, &Extensions::tit[25], "isScaleform", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Extensions::Extensions(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Extensions::Extensions()"
//##protect##"ClassTraits::Extensions::Extensions()"

    }

    Pickable<Traits> Extensions::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Extensions(vm, AS3::fl_gfx::ExtensionsCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::ExtensionsCI));
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
    const TypeInfo ExtensionsTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::Extensions::InstanceType),
        ClassTraits::fl_gfx::Extensions::ThunkInfoNum,
        ClassTraits::fl_gfx::Extensions::MemberInfoNum,
        0,
        0,
        "Extensions", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ExtensionsCI = {
        &ExtensionsTI,
        ClassTraits::fl_gfx::Extensions::MakeClassTraits,
        ClassTraits::fl_gfx::Extensions::ti,
        ClassTraits::fl_gfx::Extensions::mi,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

