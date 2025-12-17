//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_DisplayObject.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_DisplayObject.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "AS3_Obj_Display_Loader.h"
#include "AS3_Obj_Display_DisplayObjectContainer.h"
#include "AS3_Obj_Display_Stage.h"
#include "AS3_Obj_Display_LoaderInfo.h"
#include "AS3_Obj_Display_BlendMode.h"
#include "../Geom/AS3_Obj_Geom_Point.h"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
#include "../Geom/AS3_Obj_Geom_Transform.h"
#include "../Geom/AS3_Obj_Geom_ColorTransform.h"
#include "../Geom/AS3_Obj_Geom_Vector3D.h"
#include "../Accessibility/AS3_Obj_Accessibility_AccessibilityProperties.h"
#include "GFx/AS3/AS3_AvmDisplayObjContainer.h"
#include "GFx/AS3/AS3_AvmStage.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "../Filters/AS3_Obj_Filters_BevelFilter.h"
#include "../Filters/AS3_Obj_Filters_BlurFilter.h"
#include "../Filters/AS3_Obj_Filters_ColorMatrixFilter.h"
#include "../Filters/AS3_Obj_Filters_DropShadowFilter.h"
#include "../Filters/AS3_Obj_Filters_GlowFilter.h"
#include "../Filters/AS3_Obj_Filters_GradientGlowFilter.h"
#include "../Filters/AS3_Obj_Filters_GradientBevelFilter.h"
#include "../Filters/AS3_Obj_Filters_DisplacementMapFilter.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_accessibilityPropertiesGet, SPtr<Instances::fl_accessibility::AccessibilityProperties> > TFunc_Instances_DisplayObject_accessibilityPropertiesGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_accessibilityPropertiesSet, const Value, Instances::fl_accessibility::AccessibilityProperties*> TFunc_Instances_DisplayObject_accessibilityPropertiesSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_alphaGet, Value::Number> TFunc_Instances_DisplayObject_alphaGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_alphaSet, const Value, Value::Number> TFunc_Instances_DisplayObject_alphaSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_blendModeGet, ASString> TFunc_Instances_DisplayObject_blendModeGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_blendModeSet, const Value, const ASString&> TFunc_Instances_DisplayObject_blendModeSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_cacheAsBitmapGet, bool> TFunc_Instances_DisplayObject_cacheAsBitmapGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_cacheAsBitmapSet, const Value, bool> TFunc_Instances_DisplayObject_cacheAsBitmapSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_filtersGet, SPtr<Instances::fl::Array> > TFunc_Instances_DisplayObject_filtersGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_filtersSet, const Value, Instances::fl::Array*> TFunc_Instances_DisplayObject_filtersSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_heightGet, Value::Number> TFunc_Instances_DisplayObject_heightGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_heightSet, const Value, Value::Number> TFunc_Instances_DisplayObject_heightSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_loaderInfoGet, SPtr<Instances::fl_display::LoaderInfo> > TFunc_Instances_DisplayObject_loaderInfoGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_maskGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_DisplayObject_maskGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_maskSet, const Value, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObject_maskSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_mouseXGet, Value::Number> TFunc_Instances_DisplayObject_mouseXGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_mouseYGet, Value::Number> TFunc_Instances_DisplayObject_mouseYGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_nameGet, ASString> TFunc_Instances_DisplayObject_nameGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_nameSet, const Value, const ASString&> TFunc_Instances_DisplayObject_nameSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_opaqueBackgroundGet, SPtr<Instances::fl::Object> > TFunc_Instances_DisplayObject_opaqueBackgroundGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_opaqueBackgroundSet, const Value, const Value&> TFunc_Instances_DisplayObject_opaqueBackgroundSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_parentGet, SPtr<Instances::fl_display::DisplayObjectContainer> > TFunc_Instances_DisplayObject_parentGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rootGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_DisplayObject_rootGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationGet, Value::Number> TFunc_Instances_DisplayObject_rotationGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationSet, const Value, Value::Number> TFunc_Instances_DisplayObject_rotationSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationXGet, Value::Number> TFunc_Instances_DisplayObject_rotationXGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationXSet, const Value, Value::Number> TFunc_Instances_DisplayObject_rotationXSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationYGet, Value::Number> TFunc_Instances_DisplayObject_rotationYGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationYSet, const Value, Value::Number> TFunc_Instances_DisplayObject_rotationYSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationZGet, Value::Number> TFunc_Instances_DisplayObject_rotationZGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_rotationZSet, const Value, Value::Number> TFunc_Instances_DisplayObject_rotationZSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scale9GridGet, SPtr<Instances::fl_geom::Rectangle> > TFunc_Instances_DisplayObject_scale9GridGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scale9GridSet, const Value, Instances::fl_geom::Rectangle*> TFunc_Instances_DisplayObject_scale9GridSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleXGet, Value::Number> TFunc_Instances_DisplayObject_scaleXGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleXSet, const Value, Value::Number> TFunc_Instances_DisplayObject_scaleXSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleYGet, Value::Number> TFunc_Instances_DisplayObject_scaleYGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleYSet, const Value, Value::Number> TFunc_Instances_DisplayObject_scaleYSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleZGet, Value::Number> TFunc_Instances_DisplayObject_scaleZGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scaleZSet, const Value, Value::Number> TFunc_Instances_DisplayObject_scaleZSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scrollRectGet, SPtr<Instances::fl_geom::Rectangle> > TFunc_Instances_DisplayObject_scrollRectGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_scrollRectSet, const Value, Instances::fl_geom::Rectangle*> TFunc_Instances_DisplayObject_scrollRectSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_stageGet, SPtr<Instances::fl_display::Stage> > TFunc_Instances_DisplayObject_stageGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_transformGet, SPtr<Instances::fl_geom::Transform> > TFunc_Instances_DisplayObject_transformGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_transformSet, const Value, Instances::fl_geom::Transform*> TFunc_Instances_DisplayObject_transformSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_visibleGet, bool> TFunc_Instances_DisplayObject_visibleGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_visibleSet, const Value, bool> TFunc_Instances_DisplayObject_visibleSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_widthGet, Value::Number> TFunc_Instances_DisplayObject_widthGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_widthSet, const Value, Value::Number> TFunc_Instances_DisplayObject_widthSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_xGet, Value::Number> TFunc_Instances_DisplayObject_xGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_xSet, const Value, Value::Number> TFunc_Instances_DisplayObject_xSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_yGet, Value::Number> TFunc_Instances_DisplayObject_yGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_ySet, const Value, Value::Number> TFunc_Instances_DisplayObject_ySet;
typedef ThunkFunc0<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_zGet, Value::Number> TFunc_Instances_DisplayObject_zGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_zSet, const Value, Value::Number> TFunc_Instances_DisplayObject_zSet;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_getBounds, SPtr<Instances::fl_geom::Rectangle>, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObject_getBounds;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_getRect, SPtr<Instances::fl_geom::Rectangle>, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObject_getRect;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_globalToLocal, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_DisplayObject_globalToLocal;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_globalToLocal3D, SPtr<Instances::fl_geom::Vector3D>, Instances::fl_geom::Point*> TFunc_Instances_DisplayObject_globalToLocal3D;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_hitTestObject, bool, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObject_hitTestObject;
typedef ThunkFunc3<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_hitTestPoint, bool, Value::Number, Value::Number, bool> TFunc_Instances_DisplayObject_hitTestPoint;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_localToGlobal, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_DisplayObject_localToGlobal;
typedef ThunkFunc1<Instances::fl_display::DisplayObject, Instances::fl_display::DisplayObject::mid_local3DToGlobal, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Vector3D*> TFunc_Instances_DisplayObject_local3DToGlobal;

template <> const TFunc_Instances_DisplayObject_accessibilityPropertiesGet::TMethod TFunc_Instances_DisplayObject_accessibilityPropertiesGet::Method = &Instances::fl_display::DisplayObject::accessibilityPropertiesGet;
template <> const TFunc_Instances_DisplayObject_accessibilityPropertiesSet::TMethod TFunc_Instances_DisplayObject_accessibilityPropertiesSet::Method = &Instances::fl_display::DisplayObject::accessibilityPropertiesSet;
template <> const TFunc_Instances_DisplayObject_alphaGet::TMethod TFunc_Instances_DisplayObject_alphaGet::Method = &Instances::fl_display::DisplayObject::alphaGet;
template <> const TFunc_Instances_DisplayObject_alphaSet::TMethod TFunc_Instances_DisplayObject_alphaSet::Method = &Instances::fl_display::DisplayObject::alphaSet;
template <> const TFunc_Instances_DisplayObject_blendModeGet::TMethod TFunc_Instances_DisplayObject_blendModeGet::Method = &Instances::fl_display::DisplayObject::blendModeGet;
template <> const TFunc_Instances_DisplayObject_blendModeSet::TMethod TFunc_Instances_DisplayObject_blendModeSet::Method = &Instances::fl_display::DisplayObject::blendModeSet;
template <> const TFunc_Instances_DisplayObject_cacheAsBitmapGet::TMethod TFunc_Instances_DisplayObject_cacheAsBitmapGet::Method = &Instances::fl_display::DisplayObject::cacheAsBitmapGet;
template <> const TFunc_Instances_DisplayObject_cacheAsBitmapSet::TMethod TFunc_Instances_DisplayObject_cacheAsBitmapSet::Method = &Instances::fl_display::DisplayObject::cacheAsBitmapSet;
template <> const TFunc_Instances_DisplayObject_filtersGet::TMethod TFunc_Instances_DisplayObject_filtersGet::Method = &Instances::fl_display::DisplayObject::filtersGet;
template <> const TFunc_Instances_DisplayObject_filtersSet::TMethod TFunc_Instances_DisplayObject_filtersSet::Method = &Instances::fl_display::DisplayObject::filtersSet;
template <> const TFunc_Instances_DisplayObject_heightGet::TMethod TFunc_Instances_DisplayObject_heightGet::Method = &Instances::fl_display::DisplayObject::heightGet;
template <> const TFunc_Instances_DisplayObject_heightSet::TMethod TFunc_Instances_DisplayObject_heightSet::Method = &Instances::fl_display::DisplayObject::heightSet;
template <> const TFunc_Instances_DisplayObject_loaderInfoGet::TMethod TFunc_Instances_DisplayObject_loaderInfoGet::Method = &Instances::fl_display::DisplayObject::loaderInfoGet;
template <> const TFunc_Instances_DisplayObject_maskGet::TMethod TFunc_Instances_DisplayObject_maskGet::Method = &Instances::fl_display::DisplayObject::maskGet;
template <> const TFunc_Instances_DisplayObject_maskSet::TMethod TFunc_Instances_DisplayObject_maskSet::Method = &Instances::fl_display::DisplayObject::maskSet;
template <> const TFunc_Instances_DisplayObject_mouseXGet::TMethod TFunc_Instances_DisplayObject_mouseXGet::Method = &Instances::fl_display::DisplayObject::mouseXGet;
template <> const TFunc_Instances_DisplayObject_mouseYGet::TMethod TFunc_Instances_DisplayObject_mouseYGet::Method = &Instances::fl_display::DisplayObject::mouseYGet;
template <> const TFunc_Instances_DisplayObject_nameGet::TMethod TFunc_Instances_DisplayObject_nameGet::Method = &Instances::fl_display::DisplayObject::nameGet;
template <> const TFunc_Instances_DisplayObject_nameSet::TMethod TFunc_Instances_DisplayObject_nameSet::Method = &Instances::fl_display::DisplayObject::nameSet;
template <> const TFunc_Instances_DisplayObject_opaqueBackgroundGet::TMethod TFunc_Instances_DisplayObject_opaqueBackgroundGet::Method = &Instances::fl_display::DisplayObject::opaqueBackgroundGet;
template <> const TFunc_Instances_DisplayObject_opaqueBackgroundSet::TMethod TFunc_Instances_DisplayObject_opaqueBackgroundSet::Method = &Instances::fl_display::DisplayObject::opaqueBackgroundSet;
template <> const TFunc_Instances_DisplayObject_parentGet::TMethod TFunc_Instances_DisplayObject_parentGet::Method = &Instances::fl_display::DisplayObject::parentGet;
template <> const TFunc_Instances_DisplayObject_rootGet::TMethod TFunc_Instances_DisplayObject_rootGet::Method = &Instances::fl_display::DisplayObject::rootGet;
template <> const TFunc_Instances_DisplayObject_rotationGet::TMethod TFunc_Instances_DisplayObject_rotationGet::Method = &Instances::fl_display::DisplayObject::rotationGet;
template <> const TFunc_Instances_DisplayObject_rotationSet::TMethod TFunc_Instances_DisplayObject_rotationSet::Method = &Instances::fl_display::DisplayObject::rotationSet;
template <> const TFunc_Instances_DisplayObject_rotationXGet::TMethod TFunc_Instances_DisplayObject_rotationXGet::Method = &Instances::fl_display::DisplayObject::rotationXGet;
template <> const TFunc_Instances_DisplayObject_rotationXSet::TMethod TFunc_Instances_DisplayObject_rotationXSet::Method = &Instances::fl_display::DisplayObject::rotationXSet;
template <> const TFunc_Instances_DisplayObject_rotationYGet::TMethod TFunc_Instances_DisplayObject_rotationYGet::Method = &Instances::fl_display::DisplayObject::rotationYGet;
template <> const TFunc_Instances_DisplayObject_rotationYSet::TMethod TFunc_Instances_DisplayObject_rotationYSet::Method = &Instances::fl_display::DisplayObject::rotationYSet;
template <> const TFunc_Instances_DisplayObject_rotationZGet::TMethod TFunc_Instances_DisplayObject_rotationZGet::Method = &Instances::fl_display::DisplayObject::rotationZGet;
template <> const TFunc_Instances_DisplayObject_rotationZSet::TMethod TFunc_Instances_DisplayObject_rotationZSet::Method = &Instances::fl_display::DisplayObject::rotationZSet;
template <> const TFunc_Instances_DisplayObject_scale9GridGet::TMethod TFunc_Instances_DisplayObject_scale9GridGet::Method = &Instances::fl_display::DisplayObject::scale9GridGet;
template <> const TFunc_Instances_DisplayObject_scale9GridSet::TMethod TFunc_Instances_DisplayObject_scale9GridSet::Method = &Instances::fl_display::DisplayObject::scale9GridSet;
template <> const TFunc_Instances_DisplayObject_scaleXGet::TMethod TFunc_Instances_DisplayObject_scaleXGet::Method = &Instances::fl_display::DisplayObject::scaleXGet;
template <> const TFunc_Instances_DisplayObject_scaleXSet::TMethod TFunc_Instances_DisplayObject_scaleXSet::Method = &Instances::fl_display::DisplayObject::scaleXSet;
template <> const TFunc_Instances_DisplayObject_scaleYGet::TMethod TFunc_Instances_DisplayObject_scaleYGet::Method = &Instances::fl_display::DisplayObject::scaleYGet;
template <> const TFunc_Instances_DisplayObject_scaleYSet::TMethod TFunc_Instances_DisplayObject_scaleYSet::Method = &Instances::fl_display::DisplayObject::scaleYSet;
template <> const TFunc_Instances_DisplayObject_scaleZGet::TMethod TFunc_Instances_DisplayObject_scaleZGet::Method = &Instances::fl_display::DisplayObject::scaleZGet;
template <> const TFunc_Instances_DisplayObject_scaleZSet::TMethod TFunc_Instances_DisplayObject_scaleZSet::Method = &Instances::fl_display::DisplayObject::scaleZSet;
template <> const TFunc_Instances_DisplayObject_scrollRectGet::TMethod TFunc_Instances_DisplayObject_scrollRectGet::Method = &Instances::fl_display::DisplayObject::scrollRectGet;
template <> const TFunc_Instances_DisplayObject_scrollRectSet::TMethod TFunc_Instances_DisplayObject_scrollRectSet::Method = &Instances::fl_display::DisplayObject::scrollRectSet;
template <> const TFunc_Instances_DisplayObject_stageGet::TMethod TFunc_Instances_DisplayObject_stageGet::Method = &Instances::fl_display::DisplayObject::stageGet;
template <> const TFunc_Instances_DisplayObject_transformGet::TMethod TFunc_Instances_DisplayObject_transformGet::Method = &Instances::fl_display::DisplayObject::transformGet;
template <> const TFunc_Instances_DisplayObject_transformSet::TMethod TFunc_Instances_DisplayObject_transformSet::Method = &Instances::fl_display::DisplayObject::transformSet;
template <> const TFunc_Instances_DisplayObject_visibleGet::TMethod TFunc_Instances_DisplayObject_visibleGet::Method = &Instances::fl_display::DisplayObject::visibleGet;
template <> const TFunc_Instances_DisplayObject_visibleSet::TMethod TFunc_Instances_DisplayObject_visibleSet::Method = &Instances::fl_display::DisplayObject::visibleSet;
template <> const TFunc_Instances_DisplayObject_widthGet::TMethod TFunc_Instances_DisplayObject_widthGet::Method = &Instances::fl_display::DisplayObject::widthGet;
template <> const TFunc_Instances_DisplayObject_widthSet::TMethod TFunc_Instances_DisplayObject_widthSet::Method = &Instances::fl_display::DisplayObject::widthSet;
template <> const TFunc_Instances_DisplayObject_xGet::TMethod TFunc_Instances_DisplayObject_xGet::Method = &Instances::fl_display::DisplayObject::xGet;
template <> const TFunc_Instances_DisplayObject_xSet::TMethod TFunc_Instances_DisplayObject_xSet::Method = &Instances::fl_display::DisplayObject::xSet;
template <> const TFunc_Instances_DisplayObject_yGet::TMethod TFunc_Instances_DisplayObject_yGet::Method = &Instances::fl_display::DisplayObject::yGet;
template <> const TFunc_Instances_DisplayObject_ySet::TMethod TFunc_Instances_DisplayObject_ySet::Method = &Instances::fl_display::DisplayObject::ySet;
template <> const TFunc_Instances_DisplayObject_zGet::TMethod TFunc_Instances_DisplayObject_zGet::Method = &Instances::fl_display::DisplayObject::zGet;
template <> const TFunc_Instances_DisplayObject_zSet::TMethod TFunc_Instances_DisplayObject_zSet::Method = &Instances::fl_display::DisplayObject::zSet;
template <> const TFunc_Instances_DisplayObject_getBounds::TMethod TFunc_Instances_DisplayObject_getBounds::Method = &Instances::fl_display::DisplayObject::getBounds;
template <> const TFunc_Instances_DisplayObject_getRect::TMethod TFunc_Instances_DisplayObject_getRect::Method = &Instances::fl_display::DisplayObject::getRect;
template <> const TFunc_Instances_DisplayObject_globalToLocal::TMethod TFunc_Instances_DisplayObject_globalToLocal::Method = &Instances::fl_display::DisplayObject::globalToLocal;
template <> const TFunc_Instances_DisplayObject_globalToLocal3D::TMethod TFunc_Instances_DisplayObject_globalToLocal3D::Method = &Instances::fl_display::DisplayObject::globalToLocal3D;
template <> const TFunc_Instances_DisplayObject_hitTestObject::TMethod TFunc_Instances_DisplayObject_hitTestObject::Method = &Instances::fl_display::DisplayObject::hitTestObject;
template <> const TFunc_Instances_DisplayObject_hitTestPoint::TMethod TFunc_Instances_DisplayObject_hitTestPoint::Method = &Instances::fl_display::DisplayObject::hitTestPoint;
template <> const TFunc_Instances_DisplayObject_localToGlobal::TMethod TFunc_Instances_DisplayObject_localToGlobal::Method = &Instances::fl_display::DisplayObject::localToGlobal;
template <> const TFunc_Instances_DisplayObject_local3DToGlobal::TMethod TFunc_Instances_DisplayObject_local3DToGlobal::Method = &Instances::fl_display::DisplayObject::local3DToGlobal;

namespace Instances { namespace fl_display
{
    DisplayObject::DisplayObject(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::DisplayObject::DisplayObject()$data"
    , pDispObj(NULL)
//##protect##"instance::DisplayObject::DisplayObject()$data"
    {
//##protect##"instance::DisplayObject::DisplayObject()$code"
        pReleaseProxy = *SF_HEAP_AUTO_NEW(this) ReleaseProxy();
//##protect##"instance::DisplayObject::DisplayObject()$code"
    }

    void DisplayObject::accessibilityPropertiesGet(SPtr<Instances::fl_accessibility::AccessibilityProperties>& result)
    {
//##protect##"instance::DisplayObject::accessibilityPropertiesGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::DisplayObject::accessibilityPropertiesGet()");
//##protect##"instance::DisplayObject::accessibilityPropertiesGet()"
    }
    void DisplayObject::accessibilityPropertiesSet(const Value& result, Instances::fl_accessibility::AccessibilityProperties* value)
    {
//##protect##"instance::DisplayObject::accessibilityPropertiesSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::DisplayObject::accessibilityPropertiesSet()");
//##protect##"instance::DisplayObject::accessibilityPropertiesSet()"
    }
    void DisplayObject::alphaGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::alphaGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetAlpha()/100;
//##protect##"instance::DisplayObject::alphaGet()"
    }
    void DisplayObject::alphaSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::alphaSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        pDispObj->SetAlpha(value * 100);        
//##protect##"instance::DisplayObject::alphaSet()"
    }
    void DisplayObject::blendModeGet(ASString& result)
    {
//##protect##"instance::DisplayObject::blendModeGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        GFx::DisplayObjectBase::BlendType bm = pDispObj->GetBlendMode();
        switch (bm)
        {
        case Render::Blend_None:
        case Render::Blend_Normal:
            result = GetVM().GetStringManager().CreateConstString("normal");
            break;
        case Render::Blend_Layer:         
            result = GetVM().GetStringManager().CreateConstString("layer");
            break;
        case Render::Blend_Multiply:
            result = GetVM().GetStringManager().CreateConstString("multiply");
            break;
        case Render::Blend_Screen:        
            result = GetVM().GetStringManager().CreateConstString("screen");
            break;
        case Render::Blend_Lighten:       
            result = GetVM().GetStringManager().CreateConstString("lighten");
            break;
        case Render::Blend_Darken:        
            result = GetVM().GetStringManager().CreateConstString("darken");
            break;
        case Render::Blend_Difference:    
            result = GetVM().GetStringManager().CreateConstString("difference");
            break;
        case Render::Blend_Add:           
            result = GetVM().GetStringManager().CreateConstString("add");
            break;
        case Render::Blend_Subtract:      
            result = GetVM().GetStringManager().CreateConstString("subtract");
            break;
        case Render::Blend_Invert:        
            result = GetVM().GetStringManager().CreateConstString("invert");
            break;
        case Render::Blend_Alpha:         
            result = GetVM().GetStringManager().CreateConstString("alpha");
            break;
        case Render::Blend_Erase:         
            result = GetVM().GetStringManager().CreateConstString("erase");
            break;
        case Render::Blend_Overlay:       
            result = GetVM().GetStringManager().CreateConstString("overlay");
            break;
        case Render::Blend_HardLight:
            result = GetVM().GetStringManager().CreateConstString("hardlight");
            break;
        default: SF_ASSERT(0); // unknown
        }
//##protect##"instance::DisplayObject::blendModeGet()"
    }
    void DisplayObject::blendModeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::DisplayObject::blendModeSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);

        Render::BlendMode blend = Classes::fl_display::BlendMode::GetBlendMode(value);
        pDispObj->SetBlendMode(blend);
//##protect##"instance::DisplayObject::blendModeSet()"
    }
    void DisplayObject::cacheAsBitmapGet(bool& result)
    {
//##protect##"instance::DisplayObject::cacheAsBitmapGet()"
        // NOTE: If a display object has a filter pointer, it always returns that CaB is enabled,
        // regardless of whether the property is set or not (pDispObj->GetCacheAsBitmap will return
        // whether the flag is set).
        result = pDispObj->GetFilters() != 0;
//##protect##"instance::DisplayObject::cacheAsBitmapGet()"
    }
    void DisplayObject::cacheAsBitmapSet(const Value& result, bool value)
    {
//##protect##"instance::DisplayObject::cacheAsBitmapSet()"
        SF_UNUSED1(result);
        pDispObj->SetCacheAsBitmap(value);
//##protect##"instance::DisplayObject::cacheAsBitmapSet()"
    }
    void DisplayObject::filtersGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::DisplayObject::filtersGet()"
        Pickable<Instances::fl::Array> array = GetVM().MakeArray();
        const FilterSet* filters = pDispObj->GetFilters();
        if ( !filters || filters->GetFilterCount() == 0 )
        {
            result = array;
            return;
        }
        for (unsigned i = 0; i < filters->GetFilterCount(); ++i )
        {
            const Filter* filter = filters->GetFilter(i);
            SPtr<fl_filters::BitmapFilter> as3filter;
            const char* className = 0;
            switch( filter->GetFilterType() )
            {
                case Render::Filter_Blur:            className = "flash.filters.BlurFilter"; break;
                case Render::Filter_Shadow:          className = "flash.filters.DropShadowFilter"; break;
                case Render::Filter_Glow:            className = "flash.filters.GlowFilter"; break;
                case Render::Filter_Bevel:           className = "flash.filters.BevelFilter"; break;
                case Render::Filter_GradientGlow:    className = "flash.filters.GradientGlowFilter"; break;
                case Render::Filter_GradientBevel:   className = "flash.filters.GradientBevelFilter"; break;
                case Render::Filter_ColorMatrix:     className = "flash.filters.ColorMatrixFilter"; break;
                case Render::Filter_DisplacementMap: className = "flash.filters.DisplacementMapFilter"; break;
                default:
                    array->PushBack(0);
                    continue;
            }
            if (GetVM().ConstructBuiltinObject(as3filter, className ))
            {
                Ptr<Render::Filter> filterClone = *filter->Clone();
                as3filter->SetFilterData(filterClone);
                array->PushBack(Value(as3filter));
            }
        }
        result = array;
//##protect##"instance::DisplayObject::filtersGet()"
    }
    void DisplayObject::filtersSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::DisplayObject::filtersSet()"
        SF_UNUSED2(result, value);
        Ptr<FilterSet> filters = *SF_NEW FilterSet();

        if (value)
        {
            for ( UPInt i =0; i < value->GetSize(); i++ )
            {
                const Value& fv = value->At(i);
                ASString className = fv.GetObject()->GetTraits().GetName();
                if ( className == "GlowFilter")
                {
                    Instances::fl_filters::GlowFilter* filter = (Instances::fl_filters::GlowFilter*)fv.GetObject();
                    Ptr<Render::GlowFilter> data = *(Render::GlowFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "BevelFilter")
                {
                    Instances::fl_filters::BevelFilter* filter = (Instances::fl_filters::BevelFilter*)fv.GetObject();
                    Ptr<Render::BevelFilter> data = *(Render::BevelFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "DropShadowFilter")
                {
                    Instances::fl_filters::DropShadowFilter* filter = (Instances::fl_filters::DropShadowFilter*)fv.GetObject();
                    Ptr<Render::ShadowFilter> data = *(Render::ShadowFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "BlurFilter")
                {
                    Instances::fl_filters::BlurFilter* filter = (Instances::fl_filters::BlurFilter*)fv.GetObject();
                    Ptr<Render::BlurFilter> data = *(Render::BlurFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "GradientGlowFilter")
                {
                    Instances::fl_filters::GradientGlowFilter* filter = (Instances::fl_filters::GradientGlowFilter*)fv.GetObject();
                    Ptr<Render::GradientFilter> data = *(Render::GradientFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "GradientBevelFilter")
                {
                    Instances::fl_filters::GradientBevelFilter* filter = (Instances::fl_filters::GradientBevelFilter*)fv.GetObject();
                    Ptr<Render::GradientFilter> data = *(Render::GradientFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "ColorMatrixFilter")
                {
                    Instances::fl_filters::ColorMatrixFilter* filter = (Instances::fl_filters::ColorMatrixFilter*)fv.GetObject();
                    Ptr<Render::ColorMatrixFilter> data = *(Render::ColorMatrixFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
                else if ( className == "DisplacementMapFilter")
                {
                    Instances::fl_filters::DisplacementMapFilter* filter = (Instances::fl_filters::DisplacementMapFilter*)fv.GetObject();
                    Ptr<Render::DisplacementMapFilter> data = *(Render::DisplacementMapFilter*)filter->GetFilterData()->Clone(GetVM().GetMemoryHeap());
                    filters->AddFilter(data);
                }
            }
        }

        // Keep current state of cacheAsBitmap (true value of the flag).
        filters->SetCacheAsBitmap(pDispObj->GetCacheAsBitmap());

        pDispObj->SetFilters(filters);
        pDispObj->SetAcceptAnimMoves(false);
//##protect##"instance::DisplayObject::filtersSet()"
    }
    void DisplayObject::heightGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::heightGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetHeight();
//##protect##"instance::DisplayObject::heightGet()"
    }
    void DisplayObject::heightSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::heightSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        pDispObj->SetHeight(value);
//##protect##"instance::DisplayObject::heightSet()"
    }
    void DisplayObject::loaderInfoGet(SPtr<Instances::fl_display::LoaderInfo>& result)
    {
//##protect##"instance::DisplayObject::loaderInfoGet()"
        result = NULL;
        if (!pLoaderInfo)
        {
            AvmDisplayObj* prootContainer = ToAvmDisplayObj(ToAvmDisplayObj(pDispObj)->GetRoot());
            if (prootContainer)
            {
                Instances::fl_display::DisplayObject* as3obj = prootContainer->GetAS3Obj();
                SF_ASSERT(as3obj != NULL);
                if (as3obj != NULL)
                {
                    result = as3obj->pLoaderInfo;
                }
            }
        }
        else
        {
            result = pLoaderInfo;
        }
//##protect##"instance::DisplayObject::loaderInfoGet()"
    }
    void DisplayObject::maskGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::DisplayObject::maskGet()"
        SF_UNUSED1(result);
        result = NULL;
        if (pDispObj)
        {
            if (pDispObj->GetMask())
            {
                result = ToAvmDisplayObj(pDispObj->GetMask())->GetAS3Obj();
                SF_ASSERT(result != NULL); // AS3 obj is not created? Should be!
            }
        }
//##protect##"instance::DisplayObject::maskGet()"
    }
    void DisplayObject::maskSet(const Value& result, Instances::fl_display::DisplayObject* value)
    {
//##protect##"instance::DisplayObject::maskSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        if (!value)
        {
            // remove mask
            pDispObj->SetMask(NULL);
        }
        else
        {
            if (!value->pDispObj) value->CreateStageObject();
            SF_ASSERT(value->pDispObj);
            pDispObj->SetMask(value->pDispObj);
        }
//##protect##"instance::DisplayObject::maskSet()"
    }
    void DisplayObject::mouseXGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::mouseXGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetMouseX();
//##protect##"instance::DisplayObject::mouseXGet()"
    }
    void DisplayObject::mouseYGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::mouseYGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetMouseY();
//##protect##"instance::DisplayObject::mouseYGet()"
    }
    void DisplayObject::nameGet(ASString& result)
    {
//##protect##"instance::DisplayObject::nameGet()"
        SF_ASSERT(pDispObj);
        const ASString& n = pDispObj->GetName();
        if (n.IsEmpty() && pDispObj->IsInstanceBasedNameFlagSet())
            result.SetNull();
        else
            result = n;
//##protect##"instance::DisplayObject::nameGet()"
    }
    void DisplayObject::nameSet(const Value& result, const ASString& value)
    {
//##protect##"instance::DisplayObject::nameSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        if (pDispObj->IsTimelineObjectFlagSet())
            GetVM().ThrowTypeError(VM::Error(VM::eConstWriteError, GetVM()
                SF_DEBUG_ARG("name")
                SF_DEBUG_ARG("Timeline")
                ));
        else
        {
            pDispObj->SetName(value);
            pDispObj->ClearInstanceBasedNameFlag();
        }
//##protect##"instance::DisplayObject::nameSet()"
    }
    void DisplayObject::opaqueBackgroundGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::DisplayObject::opaqueBackgroundGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("DisplayObject::opaqueBackgroundGet()");
//##protect##"instance::DisplayObject::opaqueBackgroundGet()"
    }
    void DisplayObject::opaqueBackgroundSet(const Value& result, const Value& value)
    {
//##protect##"instance::DisplayObject::opaqueBackgroundSet()"
        SF_UNUSED2(result, value);
        if (!value.IsNull() && !value.IsUndefined())
            WARN_NOT_IMPLEMENTED("DisplayObject::opaqueBackgroundSet()");
//##protect##"instance::DisplayObject::opaqueBackgroundSet()"
    }
    void DisplayObject::parentGet(SPtr<Instances::fl_display::DisplayObjectContainer>& result)
    {
//##protect##"instance::DisplayObject::parentGet()"
        SF_ASSERT(pDispObj);
        GFx::InteractiveObject* pparent = pDispObj->GetParent();
        if (pparent)
        {
            AvmInteractiveObj* pcontObj = ToAvmInteractiveObj(pparent);

            // We can't create AS instance here since this may break the order of ctors' 
            // execution. Is 'null' result acceptable? (AB)
            //pcontObj->CreateASInstance(true);
            //SF_ASSERT(pcontObj->GetAS3Obj());

            Instances::fl_display::DisplayObject* ido = pcontObj->GetAS3Obj();
            if (ido && IsDisplayObjectContainer(ido->GetTraitsType()))
            {
                result = static_cast<Instances::fl_display::DisplayObjectContainer*>(ido);
                return;
            }
        }

        result = NULL;;
//##protect##"instance::DisplayObject::parentGet()"
    }
    void DisplayObject::rootGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::DisplayObject::rootGet()"
        SF_ASSERT(pDispObj);
        AvmDisplayObj* pcontObj = ToAvmDisplayObj(ToAvmDisplayObj(pDispObj)->GetRoot());
        if (pcontObj)
        {
            pcontObj->CreateASInstance(true);
            result = pcontObj->GetAS3Obj();
        }
        else
            result = NULL;
//##protect##"instance::DisplayObject::rootGet()"
    }
    void DisplayObject::rotationGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::rotationGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetRotation();
//##protect##"instance::DisplayObject::rotationGet()"
    }
    void DisplayObject::rotationSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::rotationSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetRotation(value);
//##protect##"instance::DisplayObject::rotationSet()"
    }
    void DisplayObject::rotationXGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::rotationXGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetXRotation();
//##protect##"instance::DisplayObject::rotationXGet()"
    }
    void DisplayObject::rotationXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::rotationXSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetXRotation(value);
//##protect##"instance::DisplayObject::rotationXSet()"
    }
    void DisplayObject::rotationYGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::rotationYGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetYRotation();
//##protect##"instance::DisplayObject::rotationYGet()"
    }
    void DisplayObject::rotationYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::rotationYSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetYRotation(value);
//##protect##"instance::DisplayObject::rotationYSet()"
    }
    void DisplayObject::rotationZGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::rotationZGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetRotation();
//##protect##"instance::DisplayObject::rotationZGet()"
    }
    void DisplayObject::rotationZSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::rotationZSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetRotation(value);
//##protect##"instance::DisplayObject::rotationZSet()"
    }
    void DisplayObject::scale9GridGet(SPtr<Instances::fl_geom::Rectangle>& result)
    {
//##protect##"instance::DisplayObject::scale9GridGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        
        if (pDispObj->HasScale9Grid())
        {
            RectF s9g = pDispObj->GetScale9Grid();
            // create Rectangle object
            Value params[] = { Value(TwipsToPixels(s9g.x1)), 
                               Value(TwipsToPixels(s9g.y1)), 
                               Value(TwipsToPixels(s9g.Width())), 
                               Value(TwipsToPixels(s9g.Height())) 
            };
            Value r;
            static_cast<ASVM&>(GetVM()).RectangleClass->Construct(r, 4, params, true);
            result = static_cast<Instances::fl_geom::Rectangle*>(r.GetObject());
        }
        else
            result = NULL;
//##protect##"instance::DisplayObject::scale9GridGet()"
    }
    void DisplayObject::scale9GridSet(const Value& result, Instances::fl_geom::Rectangle* value)
    {
//##protect##"instance::DisplayObject::scale9GridSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);

        if (value)
        {
            RectF sg;
            sg.x1 = PixelsToTwips(float(value->GetX()));
            sg.y1 = PixelsToTwips(float(value->GetY()));
            sg.SetWidth(PixelsToTwips(float(value->GetWidth())));
            sg.SetHeight(PixelsToTwips(float(value->GetHeight())));
            pDispObj->SetScale9Grid(sg);
        }
        else
            pDispObj->SetScale9Grid(RectF(0));

//##protect##"instance::DisplayObject::scale9GridSet()"
    }
    void DisplayObject::scaleXGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::scaleXGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetXScale()/100;
//##protect##"instance::DisplayObject::scaleXGet()"
    }
    void DisplayObject::scaleXSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::scaleXSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetXScale(value*100);
//##protect##"instance::DisplayObject::scaleXSet()"
    }
    void DisplayObject::scaleYGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::scaleYGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetYScale()/100;
//##protect##"instance::DisplayObject::scaleYGet()"
    }
    void DisplayObject::scaleYSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::scaleYSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetYScale(value*100);
//##protect##"instance::DisplayObject::scaleYSet()"
    }
    void DisplayObject::scaleZGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::scaleZGet()"
        SF_ASSERT(pDispObj);
        result = (Value::Number)pDispObj->GetZScale()/100;
//##protect##"instance::DisplayObject::scaleZGet()"
    }
    void DisplayObject::scaleZSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::scaleZSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetZScale(value*100);
//##protect##"instance::DisplayObject::scaleZSet()"
    }
    void DisplayObject::scrollRectGet(SPtr<Instances::fl_geom::Rectangle>& result)
    {
//##protect##"instance::DisplayObject::scrollRectGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj);
        const Render::RectD* r = pDispObj->GetScrollRect();
        if (r)
        {
            Value argv[4];
            argv[0].SetNumber(Alg::IRound(TwipsToPixels(r->x1)));
            argv[1].SetNumber(Alg::IRound(TwipsToPixels(r->y1)));
            argv[2].SetNumber(Alg::IRound(TwipsToPixels(r->Width())));
            argv[3].SetNumber(Alg::IRound(TwipsToPixels(r->Height())));

            ASVM& asvm = static_cast<ASVM&>(GetVM());
            asvm.ConstructInstance(result, asvm.RectangleClass, 4, argv);
            SF_ASSERT(result);
        }
        else
            result = NULL;
//##protect##"instance::DisplayObject::scrollRectGet()"
    }
    void DisplayObject::scrollRectSet(const Value& result, Instances::fl_geom::Rectangle* value)
    {
//##protect##"instance::DisplayObject::scrollRectSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj);
        if (value)
        {
            Render::RectD r(PixelsToTwips(value->GetX()),
                            PixelsToTwips(value->GetY()),
                            Render::SizeD(PixelsToTwips(value->GetWidth()),
                                          PixelsToTwips(value->GetHeight())));
            pDispObj->SetScrollRect(&r);
        }
        else
            pDispObj->SetScrollRect(NULL);
//##protect##"instance::DisplayObject::scrollRectSet()"
    }
    void DisplayObject::stageGet(SPtr<Instances::fl_display::Stage>& result)
    {
//##protect##"instance::DisplayObject::stageGet()"
        SF_ASSERT(pDispObj);
        if (ToAvmDisplayObj(pDispObj)->IsStageAccessible())
        {
            AvmDisplayObjContainer* pcontObj = ToAvmDisplayObjContainer(ToAvmDisplayObj(pDispObj)->GetStage());

            pcontObj->CreateASInstance(true);
            result = static_cast<Stage*>(pcontObj->GetAS3Obj());
            SF_ASSERT(GetVM().IsOfType(Value(result), "flash.display.Stage", GetVM().GetCurrentAppDomain()));
        }
        else
            result = NULL;
//##protect##"instance::DisplayObject::stageGet()"
    }
    void DisplayObject::transformGet(SPtr<Instances::fl_geom::Transform>& result)
    {
//##protect##"instance::DisplayObject::transformGet()"
		ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<fl_geom::Transform> ptransform;
		if (asvm.ConstructInstance(ptransform, asvm.TransformClass))
		{
			ptransform->pDispObj = pDispObj;
		}
		result = ptransform;
//##protect##"instance::DisplayObject::transformGet()"
    }
    void DisplayObject::transformSet(const Value& result, Instances::fl_geom::Transform* value)
    {
//##protect##"instance::DisplayObject::transformSet()"
		// Obtain the values of the members of the transform class and set it on 
		// the underlying display object.
		pDispObj->SetCxform(value->pDispObj->GetCxform());
        if (pDispObj->Is3D())
            pDispObj->SetMatrix3D(value->pDispObj->GetMatrix3D());
        else
            pDispObj->SetMatrix(value->pDispObj->GetMatrix());
        
        Matrix3F view;
        if (value->pDispObj->GetViewMatrix3D(&view))
            pDispObj->SetViewMatrix3D(view);

        Matrix4F proj;
        if (value->pDispObj->GetProjectionMatrix3D(&proj))
            pDispObj->SetProjectionMatrix3D(proj);

        SF_UNUSED1(result);
//##protect##"instance::DisplayObject::transformSet()"
    }
    void DisplayObject::visibleGet(bool& result)
    {
//##protect##"instance::DisplayObject::visibleGet()"
        SF_ASSERT(pDispObj);

        result = pDispObj->GetVisible();
//##protect##"instance::DisplayObject::visibleGet()"
    }
    void DisplayObject::visibleSet(const Value& result, bool value)
    {
//##protect##"instance::DisplayObject::visibleSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetVisible(value);
//##protect##"instance::DisplayObject::visibleSet()"
    }
    void DisplayObject::widthGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::widthGet()"
        SF_ASSERT(pDispObj);

        result = pDispObj->GetWidth();
//##protect##"instance::DisplayObject::widthGet()"
    }
    void DisplayObject::widthSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::widthSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetWidth(value);
//##protect##"instance::DisplayObject::widthSet()"
    }
    void DisplayObject::xGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::xGet()"
        SF_ASSERT(pDispObj);

        result = pDispObj->GetX();
//##protect##"instance::DisplayObject::xGet()"
    }
    void DisplayObject::xSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::xSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetX(value);
//##protect##"instance::DisplayObject::xSet()"
    }
    void DisplayObject::yGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::yGet()"
        SF_ASSERT(pDispObj);

        result = pDispObj->GetY();
//##protect##"instance::DisplayObject::yGet()"
    }
    void DisplayObject::ySet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::ySet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetY(value);
//##protect##"instance::DisplayObject::ySet()"
    }
    void DisplayObject::zGet(Value::Number& result)
    {
//##protect##"instance::DisplayObject::zGet()"
        SF_ASSERT(pDispObj);

        result = TwipsToPixels(pDispObj->GetZ());
//##protect##"instance::DisplayObject::zGet()"
    }
    void DisplayObject::zSet(const Value& result, Value::Number value)
    {
//##protect##"instance::DisplayObject::zSet()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj);

        pDispObj->SetZ(PixelsToTwips(value));
//##protect##"instance::DisplayObject::zSet()"
    }
    void DisplayObject::getBounds(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_display::DisplayObject* targetCoordinateSpace)
    {
//##protect##"instance::DisplayObject::getBounds()"
        SF_UNUSED2(result, targetCoordinateSpace);

        RectF      b(0);
        Matrix2F   matrix;

        // Transform first by sprite's matrix, then by inverse of target.
        if (targetCoordinateSpace && targetCoordinateSpace != this) // Optimize identity case.
        {
            matrix.SetInverse(targetCoordinateSpace->pDispObj->GetWorldMatrix());
            matrix *= pDispObj->GetWorldMatrix();
        }

        // A "perfect" implementation would be { b = psprite->GetBounds(matrix); },
        // however, Flash always gets the local bounding box before a transform,
        // as follows.
        matrix.EncloseTransform(&b, pDispObj->GetBounds(Matrix2F()));
        
        // to pixels
        b = TwipsToPixels(b);

        // create Rectangle object
        Value params[] = { Value(b.x1), Value(b.y1), Value(b.Width()), Value(b.Height()) };
        Value r;
        static_cast<ASVM&>(GetVM()).RectangleClass->Construct(r, 4, params, true);
        result = static_cast<Instances::fl_geom::Rectangle*>(r.GetObject());
//##protect##"instance::DisplayObject::getBounds()"
    }
    void DisplayObject::getRect(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_display::DisplayObject* targetCoordinateSpace)
    {
//##protect##"instance::DisplayObject::getRect()"
        SF_UNUSED2(result, targetCoordinateSpace);
        RectF      b(0);
        Matrix2F   matrix;

        // Transform first by sprite's matrix, then by inverse of target.
        if (targetCoordinateSpace && targetCoordinateSpace != this) // Optimize identity case.
        {
            matrix.SetInverse(targetCoordinateSpace->pDispObj->GetWorldMatrix());
            matrix *= pDispObj->GetWorldMatrix();
        }

        // A "perfect" implementation would be { b = psprite->GetBounds(matrix); },
        // however, Flash always gets the local bounding box before a transform,
        // as follows.
        matrix.EncloseTransform(&b, pDispObj->GetRectBounds(Matrix2F()));

        // to pixels
        b = TwipsToPixels(b);

        // create Rectangle object
        Value params[] = { Value(b.x1), Value(b.y1), Value(b.Width()), Value(b.Height()) };
        Value r;
        static_cast<ASVM&>(GetVM()).RectangleClass->Construct(r, 4, params, true);
        result = static_cast<Instances::fl_geom::Rectangle*>(r.GetObject());
//##protect##"instance::DisplayObject::getRect()"
    }
    void DisplayObject::globalToLocal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::DisplayObject::globalToLocal()"
        // Compute transformation and create a point.
        Render::PointF ptIn((float)PixelsToTwips(point->GetX()), (float)PixelsToTwips(point->GetY()));
		Render::PointF ptOut;
		ptOut = pDispObj->GlobalToLocal(ptIn);
        Value params[] = { Value(TwipsToPixels(ptOut.x)), Value(TwipsToPixels(ptOut.y)) };
        Value r;
        static_cast<ASVM&>(GetVM()).PointClass->Construct(r, 2, params, true);
        result = static_cast<Instances::fl_geom::Point*>(r.GetObject());
//##protect##"instance::DisplayObject::globalToLocal()"
    }
    void DisplayObject::globalToLocal3D(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::DisplayObject::globalToLocal3D()"
        // Compute transformation and create a point.
		Render::PointF ptIn((float)PixelsToTwips(point->GetX()), (float)PixelsToTwips(point->GetY()));
		Render::Point3F ptOut;
		ptOut = pDispObj->GlobalToLocal3D(ptIn);
		Value params[] = { Value(TwipsToPixels(ptOut.x)), Value(TwipsToPixels(ptOut.y)), Value(TwipsToPixels(ptOut.z)) };
        Value r;
		static_cast<ASVM&>(GetVM()).Vector3DClass->Construct(r, 3, params, true);
        result = static_cast<Instances::fl_geom::Vector3D*>(r.GetObject());
//##protect##"instance::DisplayObject::globalToLocal3D()"
    }
    void DisplayObject::hitTestObject(bool& result, Instances::fl_display::DisplayObject* obj)
    {
//##protect##"instance::DisplayObject::hitTestObject()"
        SF_ASSERT(pDispObj);

        result = false;
        RectF spriteLocalBounds = pDispObj->GetBounds(Matrix2F());
        if (spriteLocalBounds.IsNull())
            return;
        if (!obj)
            return;
        SF_ASSERT(obj->pDispObj);

        RectF targetLocalRect  = obj->pDispObj->GetBounds(Matrix2F());
        if (targetLocalRect.IsNull())
            return;
        // Rect rectangles in same coordinate space & check intersection.
		// TODO - need to handle if object are 3D
        RectF spriteWorldRect  = pDispObj->GetWorldMatrix().EncloseTransform(spriteLocalBounds);
        RectF targetWorldRect  = obj->pDispObj->GetWorldMatrix().EncloseTransform(targetLocalRect);
        result = spriteWorldRect.Intersects(targetWorldRect);
//##protect##"instance::DisplayObject::hitTestObject()"
    }
    void DisplayObject::hitTestPoint(bool& result, Value::Number x, Value::Number y, bool shapeFlag)
    {
//##protect##"instance::DisplayObject::hitTestPoint()"
        SF_ASSERT(pDispObj);

        result = false;
        RectF spriteLocalBounds = pDispObj->GetBounds(Matrix2F());
        if (spriteLocalBounds.IsNull())
            return;

        UInt8 hitTestMask = 0;
        // x, y, shapeFlag version of hitTest.
        Render::PointF pt(
            (float)SF_PIXELS_TO_TWIPS(x),
            (float)SF_PIXELS_TO_TWIPS(y));
        if (shapeFlag) // optional parameter shapeFlag
            hitTestMask |= GFx::DisplayObject::HitTest_TestShape;

        MovieImpl * proot = pDispObj->GetMovieImpl();
        if ( proot && pDispObj->Is3D(true) )
        {   // 3D case
            proot->SetNormalizedScreenCoords(pt);

            Matrix3F view;
            Matrix4F proj; 

            Render::ScreenToWorld & stw = proot->ScreenToWorld;
            if (pDispObj->GetProjectionMatrix3D(&proj))
                stw.SetProjection(proj);

            if (pDispObj->GetViewMatrix3D(&view))
                stw.SetView(view);

            stw.SetWorld(pDispObj->GetWorldMatrix3D());
            Render::PointF pp;
            stw.GetWorldPoint(&pp);

            result = pDispObj->PointTestLocal (pp, hitTestMask);
        }
        else
        {   // 2D case            
            Render::PointF ptSpr = pDispObj->GetLevelMatrix().TransformByInverse(pt);
            // pt is already in root's coordinates!

            if (pDispObj->DoesScale9GridExist())
            {
                result = pDispObj->PointTestLocal (ptSpr, hitTestMask);
            }
            else
            {
                if (spriteLocalBounds.Contains(ptSpr))
                {
                    if (!(hitTestMask & GFx::DisplayObject::HitTest_TestShape))
                        result = true;
                    else
                        result = pDispObj->PointTestLocal (ptSpr, hitTestMask);
                }
            }
        }
//##protect##"instance::DisplayObject::hitTestPoint()"
    }
    void DisplayObject::localToGlobal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::DisplayObject::localToGlobal()"
        SF_UNUSED(result);
        // Compute transformation and create a point.
        Render::PointF pt((float)PixelsToTwips(point->GetX()), (float)PixelsToTwips(point->GetY()));
        pt = pDispObj->LocalToGlobal(pt);
        Value params[] = { Value(TwipsToPixels(pt.x)), Value(TwipsToPixels(pt.y)) };
        Value r;
        static_cast<ASVM&>(GetVM()).PointClass->Construct(r, 2, params, true);
        result = static_cast<Instances::fl_geom::Point*>(r.GetObject());
//##protect##"instance::DisplayObject::localToGlobal()"
    }
    void DisplayObject::local3DToGlobal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Vector3D* point)
    {
//##protect##"instance::DisplayObject::local3DToGlobal()"
        SF_UNUSED(result);
        // Compute transformation and create a point.
        Render::Point3F ptIn((float)PixelsToTwips(point->GetX()), (float)PixelsToTwips(point->GetY()), (float)PixelsToTwips(point->GetZ()));
        Render::PointF ptOut = pDispObj->Local3DToGlobal(ptIn);
        Value params[] = { Value(TwipsToPixels(ptOut.x)), Value(TwipsToPixels(ptOut.y)) };
        Value r;
        static_cast<ASVM&>(GetVM()).PointClass->Construct(r, 2, params, true);
        result = static_cast<Instances::fl_geom::Point*>(r.GetObject());
//##protect##"instance::DisplayObject::local3DToGlobal()"
    }

    SPtr<Instances::fl_accessibility::AccessibilityProperties> DisplayObject::accessibilityPropertiesGet()
    {
        SPtr<Instances::fl_accessibility::AccessibilityProperties> result;
        accessibilityPropertiesGet(result);
        return result;
    }
    SPtr<Instances::fl::Array> DisplayObject::filtersGet()
    {
        SPtr<Instances::fl::Array> result;
        filtersGet(result);
        return result;
    }
    SPtr<Instances::fl_display::LoaderInfo> DisplayObject::loaderInfoGet()
    {
        SPtr<Instances::fl_display::LoaderInfo> result;
        loaderInfoGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObjectContainer> DisplayObject::parentGet()
    {
        SPtr<Instances::fl_display::DisplayObjectContainer> result;
        parentGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> DisplayObject::scale9GridGet()
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        scale9GridGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> DisplayObject::scrollRectGet()
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        scrollRectGet(result);
        return result;
    }
    SPtr<Instances::fl_display::Stage> DisplayObject::stageGet()
    {
        SPtr<Instances::fl_display::Stage> result;
        stageGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Transform> DisplayObject::transformGet()
    {
        SPtr<Instances::fl_geom::Transform> result;
        transformGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> DisplayObject::getBounds(Instances::fl_display::DisplayObject* targetCoordinateSpace)
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        getBounds(result, targetCoordinateSpace);
        return result;
    }
    SPtr<Instances::fl_geom::Rectangle> DisplayObject::getRect(Instances::fl_display::DisplayObject* targetCoordinateSpace)
    {
        SPtr<Instances::fl_geom::Rectangle> result;
        getRect(result, targetCoordinateSpace);
        return result;
    }
    SPtr<Instances::fl_geom::Point> DisplayObject::globalToLocal(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl_geom::Point> result;
        globalToLocal(result, point);
        return result;
    }
    SPtr<Instances::fl_geom::Vector3D> DisplayObject::globalToLocal3D(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl_geom::Vector3D> result;
        globalToLocal3D(result, point);
        return result;
    }
    SPtr<Instances::fl_geom::Point> DisplayObject::localToGlobal(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl_geom::Point> result;
        localToGlobal(result, point);
        return result;
    }
    SPtr<Instances::fl_geom::Point> DisplayObject::local3DToGlobal(Instances::fl_geom::Vector3D* point)
    {
        SPtr<Instances::fl_geom::Point> result;
        local3DToGlobal(result, point);
        return result;
    }
//##protect##"instance$methods"
    DisplayObject::~DisplayObject()
    {
        if (pDispObj)
        {
            ToAvmDisplayObj(pDispObj)->ReleaseAS3Obj();
            if (GetVMSafe().GetGC().IsInCollect())
            {
                pReleaseProxy->Data = pDispObj;
                GetVMSafe().GetGC().AddDelayedReleaseProxy(pReleaseProxy);
            }
        }
    }

    GFx::DisplayObject* DisplayObject::CreateStageObject()
    {
        SF_ASSERT(0); // should be overridden!
        return pDispObj;
    }

    // sets pLoaderInfo = Loader.contentLoaderInfo.
    void DisplayObject::SetLoaderInfo(const Loader& loader)
    {
        pLoaderInfo = loader.GetContentLoaderInfo();
    }
    void DisplayObject::SetLoaderInfo(LoaderInfo* loaderInfo)
    {
        // need to hold this since assignment to pLoaderInfo may cause
        // 'this' release...
        SPtr<DisplayObject> t = this; 
        pLoaderInfo = loaderInfo;
    }

    // returns true if pLoaderInfo is set for the instance
    bool DisplayObject::HasLoaderInfo() const
    {
        return pLoaderInfo.GetPtr() != NULL;
    }

    void DisplayObject::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        EventDispatcher::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<LoaderInfo, Mem_Stat>(prcc, pLoaderInfo, op SF_DEBUG_ARG(*this));
    }

    bool DisplayObject::FindLibarySymbol(CharacterCreateInfo* pccinfo, MovieDefImpl* pdefImpl) const
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        pccinfo->pCharDef     = NULL;
        pccinfo->pResource    = NULL;
        pccinfo->pBindDefImpl = pdefImpl;
        const Traits* curTraits = &GetTraits();

        // We need to interate through all base classes in order to find a library symbol.
        // The library symbol may be linked to one of the base class and it still should be
        // picked up (see mc_linkage_to_base_class.swf)
        while (curTraits && curTraits->IsUserDefined() && 
               !pccinfo->pCharDef && !pccinfo->pResource)
        {
            // here we should get fully qualified name of class, with package, 
            // for example: example_fla.Symbol1. 
            //                    ASString className = GetInstanceTraits().GetQualifiedName(Traits::qnfWithDot);
            ASString className = curTraits->GetQualifiedName(Traits::qnfWithDot);

            ResourceBindData resBindData;
            if (asvm.GetMovieImpl()->FindExportedResource(pdefImpl, &resBindData, className.ToCStr()))
            {
                if (resBindData.pResource->GetResourceType() & Resource::RT_CharacterDef_Bit)
                    pccinfo->pCharDef = static_cast<CharacterDef*>(resBindData.pResource.GetPtr());
                else
                    pccinfo->pResource = resBindData.pResource.GetPtr();
                pccinfo->pBindDefImpl = resBindData.pBinding->GetOwnerDefImpl();
            }
            else
                curTraits = curTraits->GetParent();
        }
        return (pccinfo->pCharDef != NULL || pccinfo->pResource != NULL);
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 DisplayObject::tito[DisplayObject::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 20, 22, 23, 24, 25, 27, 28, 30, 31, 32, 33, 35, 36, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51, 53, 54, 56, 57, 59, 60, 61, 63, 64, 66, 67, 69, 70, 72, 73, 75, 76, 78, 80, 82, 84, 86, 88, 92, 94, 
    // };
    const TypeInfo* DisplayObject::tit[96] = {
        &AS3::fl_accessibility::AccessibilityPropertiesTI, 
        NULL, &AS3::fl_accessibility::AccessibilityPropertiesTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_display::LoaderInfoTI, 
        &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl_display::DisplayObjectContainerTI, 
        &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl_geom::RectangleTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl_geom::RectangleTI, 
        &AS3::fl_display::StageTI, 
        &AS3::fl_geom::TransformTI, 
        NULL, &AS3::fl_geom::TransformTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::Vector3DTI, &AS3::fl_geom::PointTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::BooleanTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::BooleanTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::Vector3DTI, 
    };
    const ThunkInfo DisplayObject::ti[DisplayObject::ThunkInfoNum] = {
        {TFunc_Instances_DisplayObject_accessibilityPropertiesGet::Func, &DisplayObject::tit[0], "accessibilityProperties", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_accessibilityPropertiesSet::Func, &DisplayObject::tit[1], "accessibilityProperties", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_alphaGet::Func, &DisplayObject::tit[3], "alpha", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_alphaSet::Func, &DisplayObject::tit[4], "alpha", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_blendModeGet::Func, &DisplayObject::tit[6], "blendMode", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_blendModeSet::Func, &DisplayObject::tit[7], "blendMode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_cacheAsBitmapGet::Func, &DisplayObject::tit[9], "cacheAsBitmap", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_cacheAsBitmapSet::Func, &DisplayObject::tit[10], "cacheAsBitmap", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_filtersGet::Func, &DisplayObject::tit[12], "filters", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_filtersSet::Func, &DisplayObject::tit[13], "filters", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_heightGet::Func, &DisplayObject::tit[15], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_heightSet::Func, &DisplayObject::tit[16], "height", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_loaderInfoGet::Func, &DisplayObject::tit[18], "loaderInfo", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_maskGet::Func, &DisplayObject::tit[19], "mask", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_maskSet::Func, &DisplayObject::tit[20], "mask", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_mouseXGet::Func, &DisplayObject::tit[22], "mouseX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_mouseYGet::Func, &DisplayObject::tit[23], "mouseY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_nameGet::Func, &DisplayObject::tit[24], "name", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_nameSet::Func, &DisplayObject::tit[25], "name", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_opaqueBackgroundGet::Func, &DisplayObject::tit[27], "opaqueBackground", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_opaqueBackgroundSet::Func, &DisplayObject::tit[28], "opaqueBackground", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_parentGet::Func, &DisplayObject::tit[30], "parent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rootGet::Func, &DisplayObject::tit[31], "root", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationGet::Func, &DisplayObject::tit[32], "rotation", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationSet::Func, &DisplayObject::tit[33], "rotation", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationXGet::Func, &DisplayObject::tit[35], "rotationX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationXSet::Func, &DisplayObject::tit[36], "rotationX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationYGet::Func, &DisplayObject::tit[38], "rotationY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationYSet::Func, &DisplayObject::tit[39], "rotationY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationZGet::Func, &DisplayObject::tit[41], "rotationZ", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_rotationZSet::Func, &DisplayObject::tit[42], "rotationZ", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scale9GridGet::Func, &DisplayObject::tit[44], "scale9Grid", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scale9GridSet::Func, &DisplayObject::tit[45], "scale9Grid", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleXGet::Func, &DisplayObject::tit[47], "scaleX", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleXSet::Func, &DisplayObject::tit[48], "scaleX", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleYGet::Func, &DisplayObject::tit[50], "scaleY", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleYSet::Func, &DisplayObject::tit[51], "scaleY", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleZGet::Func, &DisplayObject::tit[53], "scaleZ", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scaleZSet::Func, &DisplayObject::tit[54], "scaleZ", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scrollRectGet::Func, &DisplayObject::tit[56], "scrollRect", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_scrollRectSet::Func, &DisplayObject::tit[57], "scrollRect", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_stageGet::Func, &DisplayObject::tit[59], "stage", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_transformGet::Func, &DisplayObject::tit[60], "transform", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_transformSet::Func, &DisplayObject::tit[61], "transform", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_visibleGet::Func, &DisplayObject::tit[63], "visible", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_visibleSet::Func, &DisplayObject::tit[64], "visible", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_widthGet::Func, &DisplayObject::tit[66], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_widthSet::Func, &DisplayObject::tit[67], "width", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_xGet::Func, &DisplayObject::tit[69], "x", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_xSet::Func, &DisplayObject::tit[70], "x", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_yGet::Func, &DisplayObject::tit[72], "y", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_ySet::Func, &DisplayObject::tit[73], "y", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_zGet::Func, &DisplayObject::tit[75], "z", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_zSet::Func, &DisplayObject::tit[76], "z", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_getBounds::Func, &DisplayObject::tit[78], "getBounds", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_getRect::Func, &DisplayObject::tit[80], "getRect", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_globalToLocal::Func, &DisplayObject::tit[82], "globalToLocal", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_globalToLocal3D::Func, &DisplayObject::tit[84], "globalToLocal3D", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_hitTestObject::Func, &DisplayObject::tit[86], "hitTestObject", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_hitTestPoint::Func, &DisplayObject::tit[88], "hitTestPoint", NULL, Abc::NS_Public, CT_Method, 2, 3, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_localToGlobal::Func, &DisplayObject::tit[92], "localToGlobal", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObject_local3DToGlobal::Func, &DisplayObject::tit[94], "local3DToGlobal", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    DisplayObject::DisplayObject(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::DisplayObject::DisplayObject()"
        SetTraitsType(Traits_DisplayObject);
//##protect##"InstanceTraits::DisplayObject::DisplayObject()"

    }

    void DisplayObject::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<DisplayObject&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    DisplayObject::DisplayObject(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::DisplayObject::DisplayObject()"
        SetTraitsType(Traits_DisplayObject);
//##protect##"ClassTraits::DisplayObject::DisplayObject()"

    }

    Pickable<Traits> DisplayObject::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DisplayObject(vm, AS3::fl_display::DisplayObjectCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::DisplayObjectCI));
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
    const TypeInfo* DisplayObjectImplements[] = {
        &fl_display::IBitmapDrawableTI,
        NULL
    };

    const TypeInfo DisplayObjectTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::DisplayObject::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::DisplayObject::ThunkInfoNum,
        0,
        "DisplayObject", "flash.display", &fl_events::EventDispatcherTI,
        DisplayObjectImplements
    };

    const ClassInfo DisplayObjectCI = {
        &DisplayObjectTI,
        ClassTraits::fl_display::DisplayObject::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::DisplayObject::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

