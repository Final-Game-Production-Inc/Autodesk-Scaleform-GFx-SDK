//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_DisplayObject.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_DisplayObject_H
#define INC_AS3_Obj_Display_DisplayObject_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
    extern const TypeInfo LoaderInfoTI;
    extern const ClassInfo LoaderInfoCI;
    extern const TypeInfo DisplayObjectContainerTI;
    extern const ClassInfo DisplayObjectContainerCI;
    extern const TypeInfo StageTI;
    extern const ClassInfo StageCI;
    extern const TypeInfo IBitmapDrawableTI;
    extern const ClassInfo IBitmapDrawableCI;
} // namespace fl_display
namespace fl_accessibility
{
    extern const TypeInfo AccessibilityPropertiesTI;
    extern const ClassInfo AccessibilityPropertiesCI;
} // namespace fl_accessibility
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
} // namespace fl
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
    extern const TypeInfo TransformTI;
    extern const ClassInfo TransformCI;
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
    extern const TypeInfo Vector3DTI;
    extern const ClassInfo Vector3DCI;
} // namespace fl_geom

namespace ClassTraits { namespace fl_display
{
    class DisplayObject;
}}

namespace InstanceTraits { namespace fl_display
{
    class DisplayObject;
}}

namespace Classes { namespace fl_display
{
    class DisplayObject;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class DisplayObjectContainer;
        class Stage;
        class LoaderInfo;
        class Loader;
    }

    namespace fl_geom
    {
        class Rectangle;
        class Transform;
        class Point;
        class Vector3D;  
    }

    namespace fl_accessibility
    {
        class AccessibilityProperties;
    }
}

typedef PtrReleaseProxy<Mem_Stat> ReleaseProxy;
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class DisplayObject : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::DisplayObject;
        
    public:
        typedef DisplayObject SelfType;
        typedef Classes::fl_display::DisplayObject ClassType;
        typedef InstanceTraits::fl_display::DisplayObject TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::DisplayObjectTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::DisplayObject"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        DisplayObject(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        ~DisplayObject();

        virtual GFx::DisplayObject* CreateStageObject();
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        bool FindLibarySymbol(CharacterCreateInfo* pccinfo, MovieDefImpl* pdefImpl) const;
        bool HasDisplayObject() const { return pDispObj != NULL; }

        // sets pLoaderInfo = Loader.contentLoaderInfo.
        void SetLoaderInfo(const Loader&);
        void SetLoaderInfo(LoaderInfo*);

        // returns true if pLoaderInfo is set for the instance
        bool HasLoaderInfo() const;

#ifdef GFX_AS3_VERBOSE
        virtual Scaleform::String GetAS3ObjectName() const 
        { 
            if (pDispObj) return Scaleform::String(pDispObj->GetName().ToCStr());
            return Instances::fl_events::EventDispatcher::GetAS3ObjectName();
        }
#endif
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_accessibilityPropertiesGet, 
            mid_accessibilityPropertiesSet, 
            mid_alphaGet, 
            mid_alphaSet, 
            mid_blendModeGet, 
            mid_blendModeSet, 
            mid_cacheAsBitmapGet, 
            mid_cacheAsBitmapSet, 
            mid_filtersGet, 
            mid_filtersSet, 
            mid_heightGet, 
            mid_heightSet, 
            mid_loaderInfoGet, 
            mid_maskGet, 
            mid_maskSet, 
            mid_mouseXGet, 
            mid_mouseYGet, 
            mid_nameGet, 
            mid_nameSet, 
            mid_opaqueBackgroundGet, 
            mid_opaqueBackgroundSet, 
            mid_parentGet, 
            mid_rootGet, 
            mid_rotationGet, 
            mid_rotationSet, 
            mid_rotationXGet, 
            mid_rotationXSet, 
            mid_rotationYGet, 
            mid_rotationYSet, 
            mid_rotationZGet, 
            mid_rotationZSet, 
            mid_scale9GridGet, 
            mid_scale9GridSet, 
            mid_scaleXGet, 
            mid_scaleXSet, 
            mid_scaleYGet, 
            mid_scaleYSet, 
            mid_scaleZGet, 
            mid_scaleZSet, 
            mid_scrollRectGet, 
            mid_scrollRectSet, 
            mid_stageGet, 
            mid_transformGet, 
            mid_transformSet, 
            mid_visibleGet, 
            mid_visibleSet, 
            mid_widthGet, 
            mid_widthSet, 
            mid_xGet, 
            mid_xSet, 
            mid_yGet, 
            mid_ySet, 
            mid_zGet, 
            mid_zSet, 
            mid_getBounds, 
            mid_getRect, 
            mid_globalToLocal, 
            mid_globalToLocal3D, 
            mid_hitTestObject, 
            mid_hitTestPoint, 
            mid_localToGlobal, 
            mid_local3DToGlobal, 
        };
        void accessibilityPropertiesGet(SPtr<Instances::fl_accessibility::AccessibilityProperties>& result);
        void accessibilityPropertiesSet(const Value& result, Instances::fl_accessibility::AccessibilityProperties* value);
        void alphaGet(Value::Number& result);
        void alphaSet(const Value& result, Value::Number value);
        void blendModeGet(ASString& result);
        void blendModeSet(const Value& result, const ASString& value);
        void cacheAsBitmapGet(bool& result);
        void cacheAsBitmapSet(const Value& result, bool value);
        void filtersGet(SPtr<Instances::fl::Array>& result);
        void filtersSet(const Value& result, Instances::fl::Array* value);
        void heightGet(Value::Number& result);
        void heightSet(const Value& result, Value::Number value);
        void loaderInfoGet(SPtr<Instances::fl_display::LoaderInfo>& result);
        void maskGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void maskSet(const Value& result, Instances::fl_display::DisplayObject* value);
        void mouseXGet(Value::Number& result);
        void mouseYGet(Value::Number& result);
        void nameGet(ASString& result);
        void nameSet(const Value& result, const ASString& value);
        void opaqueBackgroundGet(SPtr<Instances::fl::Object>& result);
        void opaqueBackgroundSet(const Value& result, const Value& value);
        void parentGet(SPtr<Instances::fl_display::DisplayObjectContainer>& result);
        void rootGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void rotationGet(Value::Number& result);
        void rotationSet(const Value& result, Value::Number value);
        void rotationXGet(Value::Number& result);
        void rotationXSet(const Value& result, Value::Number value);
        void rotationYGet(Value::Number& result);
        void rotationYSet(const Value& result, Value::Number value);
        void rotationZGet(Value::Number& result);
        void rotationZSet(const Value& result, Value::Number value);
        void scale9GridGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void scale9GridSet(const Value& result, Instances::fl_geom::Rectangle* value);
        void scaleXGet(Value::Number& result);
        void scaleXSet(const Value& result, Value::Number value);
        void scaleYGet(Value::Number& result);
        void scaleYSet(const Value& result, Value::Number value);
        void scaleZGet(Value::Number& result);
        void scaleZSet(const Value& result, Value::Number value);
        void scrollRectGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void scrollRectSet(const Value& result, Instances::fl_geom::Rectangle* value);
        void stageGet(SPtr<Instances::fl_display::Stage>& result);
        void transformGet(SPtr<Instances::fl_geom::Transform>& result);
        void transformSet(const Value& result, Instances::fl_geom::Transform* value);
        void visibleGet(bool& result);
        void visibleSet(const Value& result, bool value);
        void widthGet(Value::Number& result);
        void widthSet(const Value& result, Value::Number value);
        void xGet(Value::Number& result);
        void xSet(const Value& result, Value::Number value);
        void yGet(Value::Number& result);
        void ySet(const Value& result, Value::Number value);
        void zGet(Value::Number& result);
        void zSet(const Value& result, Value::Number value);
        void getBounds(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_display::DisplayObject* targetCoordinateSpace);
        void getRect(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_display::DisplayObject* targetCoordinateSpace);
        void globalToLocal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point);
        void globalToLocal3D(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Point* point);
        void hitTestObject(bool& result, Instances::fl_display::DisplayObject* obj);
        void hitTestPoint(bool& result, Value::Number x, Value::Number y, bool shapeFlag = false);
        void localToGlobal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point);
        void local3DToGlobal(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Vector3D* point);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_accessibility::AccessibilityProperties> accessibilityPropertiesGet();
        void accessibilityPropertiesSet(Instances::fl_accessibility::AccessibilityProperties* value)
        {
            accessibilityPropertiesSet(Value::GetUndefined(), value);
        }
        Value::Number alphaGet()
        {
            Value::Number result;
            alphaGet(result);
            return result;
        }
        void alphaSet(Value::Number value)
        {
            alphaSet(Value::GetUndefined(), value);
        }
        ASString blendModeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            blendModeGet(result);
            return result;
        }
        void blendModeSet(const ASString& value)
        {
            blendModeSet(Value::GetUndefined(), value);
        }
        bool cacheAsBitmapGet()
        {
            bool result;
            cacheAsBitmapGet(result);
            return result;
        }
        void cacheAsBitmapSet(bool value)
        {
            cacheAsBitmapSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> filtersGet();
        void filtersSet(Instances::fl::Array* value)
        {
            filtersSet(Value::GetUndefined(), value);
        }
        Value::Number heightGet()
        {
            Value::Number result;
            heightGet(result);
            return result;
        }
        void heightSet(Value::Number value)
        {
            heightSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::LoaderInfo> loaderInfoGet();
        SPtr<Instances::fl_display::DisplayObject> maskGet()
        {
            SPtr<Instances::fl_display::DisplayObject> result;
            maskGet(result);
            return result;
        }
        void maskSet(Instances::fl_display::DisplayObject* value)
        {
            maskSet(Value::GetUndefined(), value);
        }
        Value::Number mouseXGet()
        {
            Value::Number result;
            mouseXGet(result);
            return result;
        }
        Value::Number mouseYGet()
        {
            Value::Number result;
            mouseYGet(result);
            return result;
        }
        ASString nameGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            nameGet(result);
            return result;
        }
        void nameSet(const ASString& value)
        {
            nameSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Object> opaqueBackgroundGet()
        {
            SPtr<Instances::fl::Object> result;
            opaqueBackgroundGet(result);
            return result;
        }
        void opaqueBackgroundSet(const Value& value)
        {
            opaqueBackgroundSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObjectContainer> parentGet();
        SPtr<Instances::fl_display::DisplayObject> rootGet()
        {
            SPtr<Instances::fl_display::DisplayObject> result;
            rootGet(result);
            return result;
        }
        Value::Number rotationGet()
        {
            Value::Number result;
            rotationGet(result);
            return result;
        }
        void rotationSet(Value::Number value)
        {
            rotationSet(Value::GetUndefined(), value);
        }
        Value::Number rotationXGet()
        {
            Value::Number result;
            rotationXGet(result);
            return result;
        }
        void rotationXSet(Value::Number value)
        {
            rotationXSet(Value::GetUndefined(), value);
        }
        Value::Number rotationYGet()
        {
            Value::Number result;
            rotationYGet(result);
            return result;
        }
        void rotationYSet(Value::Number value)
        {
            rotationYSet(Value::GetUndefined(), value);
        }
        Value::Number rotationZGet()
        {
            Value::Number result;
            rotationZGet(result);
            return result;
        }
        void rotationZSet(Value::Number value)
        {
            rotationZSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Rectangle> scale9GridGet();
        void scale9GridSet(Instances::fl_geom::Rectangle* value)
        {
            scale9GridSet(Value::GetUndefined(), value);
        }
        Value::Number scaleXGet()
        {
            Value::Number result;
            scaleXGet(result);
            return result;
        }
        void scaleXSet(Value::Number value)
        {
            scaleXSet(Value::GetUndefined(), value);
        }
        Value::Number scaleYGet()
        {
            Value::Number result;
            scaleYGet(result);
            return result;
        }
        void scaleYSet(Value::Number value)
        {
            scaleYSet(Value::GetUndefined(), value);
        }
        Value::Number scaleZGet()
        {
            Value::Number result;
            scaleZGet(result);
            return result;
        }
        void scaleZSet(Value::Number value)
        {
            scaleZSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Rectangle> scrollRectGet();
        void scrollRectSet(Instances::fl_geom::Rectangle* value)
        {
            scrollRectSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::Stage> stageGet();
        SPtr<Instances::fl_geom::Transform> transformGet();
        void transformSet(Instances::fl_geom::Transform* value)
        {
            transformSet(Value::GetUndefined(), value);
        }
        bool visibleGet()
        {
            bool result;
            visibleGet(result);
            return result;
        }
        void visibleSet(bool value)
        {
            visibleSet(Value::GetUndefined(), value);
        }
        Value::Number widthGet()
        {
            Value::Number result;
            widthGet(result);
            return result;
        }
        void widthSet(Value::Number value)
        {
            widthSet(Value::GetUndefined(), value);
        }
        Value::Number xGet()
        {
            Value::Number result;
            xGet(result);
            return result;
        }
        void xSet(Value::Number value)
        {
            xSet(Value::GetUndefined(), value);
        }
        Value::Number yGet()
        {
            Value::Number result;
            yGet(result);
            return result;
        }
        void ySet(Value::Number value)
        {
            ySet(Value::GetUndefined(), value);
        }
        Value::Number zGet()
        {
            Value::Number result;
            zGet(result);
            return result;
        }
        void zSet(Value::Number value)
        {
            zSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Rectangle> getBounds(Instances::fl_display::DisplayObject* targetCoordinateSpace);
        SPtr<Instances::fl_geom::Rectangle> getRect(Instances::fl_display::DisplayObject* targetCoordinateSpace);
        SPtr<Instances::fl_geom::Point> globalToLocal(Instances::fl_geom::Point* point);
        SPtr<Instances::fl_geom::Vector3D> globalToLocal3D(Instances::fl_geom::Point* point);
        bool hitTestObject(Instances::fl_display::DisplayObject* obj)
        {
            bool result;
            hitTestObject(result, obj);
            return result;
        }
        bool hitTestPoint(Value::Number x, Value::Number y, bool shapeFlag = false)
        {
            bool result;
            hitTestPoint(result, x, y, shapeFlag);
            return result;
        }
        SPtr<Instances::fl_geom::Point> localToGlobal(Instances::fl_geom::Point* point);
        SPtr<Instances::fl_geom::Point> local3DToGlobal(Instances::fl_geom::Vector3D* point);

//##protect##"instance$data"
    public:
        SPtr<LoaderInfo>        pLoaderInfo;
        Ptr<GFx::DisplayObject> pDispObj;
        Ptr<ReleaseProxy>       pReleaseProxy;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class DisplayObject : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::DisplayObject"; }
#endif
    public:
        typedef Instances::fl_display::DisplayObject InstanceType;

    public:
        DisplayObject(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::DisplayObject> MakeInstance(DisplayObject& t)
        {
            return Pickable<Instances::fl_display::DisplayObject>(new(t.Alloc()) Instances::fl_display::DisplayObject(t));
        }
        SPtr<Instances::fl_display::DisplayObject> MakeInstanceS(DisplayObject& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 62 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[96];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class DisplayObject : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DisplayObject"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::DisplayObject InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DisplayObject(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"

// Helper method - quickly determines that BuiltinTraitsType belong to display object.
inline bool AreDisplayObjectTraits(AS3::Object* obj)
{
    const BuiltinTraitsType tt = obj->GetTraitsType();
    return ((unsigned)(tt-Traits_DisplayObject_Begin) <
            (unsigned)(Traits_DisplayObject_End-Traits_DisplayObject_Begin) &&
            obj->GetTraits().IsInstanceTraits());
}
inline bool AreDisplayObjectContainerTraits(AS3::Object* obj)
{
    BuiltinTraitsType tt = obj->GetTraitsType();
    return ((unsigned)(tt-Traits_DisplayObjectContainer) <
            (unsigned)(Traits_DisplayObject_End-Traits_DisplayObjectContainer) &&
            obj->GetTraits().IsInstanceTraits());
}


//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

