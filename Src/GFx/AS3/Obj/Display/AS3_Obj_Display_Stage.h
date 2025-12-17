//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Stage.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Stage_H
#define INC_AS3_Obj_Display_Stage_H

#include "AS3_Obj_Display_DisplayObjectContainer.h"
//##protect##"includes"
#include "GFx/AS3/AS3_AvmStage.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo StageTI;
    extern const ClassInfo StageCI;
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
    extern const TypeInfo NativeWindowTI;
    extern const ClassInfo NativeWindowCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
} // namespace fl
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom
namespace fl_text
{
    extern const TypeInfo TextSnapshotTI;
    extern const ClassInfo TextSnapshotCI;
} // namespace fl_text
namespace fl_events
{
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_display
{
    class Stage;
}}

namespace InstanceTraits { namespace fl_display
{
    class Stage;
}}

namespace Classes { namespace fl_display
{
    class Stage;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class NativeWindow;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Stage : public Instances::fl_display::DisplayObjectContainer
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Stage;
        
    public:
        typedef Stage SelfType;
        typedef Classes::fl_display::Stage ClassType;
        typedef InstanceTraits::fl_display::Stage TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::StageTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Stage"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Stage(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        ~Stage();
    public:
        AS3::Stage* GetStageObj() 
        {
            return static_cast<AS3::Stage*>(pDispObj.GetPtr());
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_alignGet, 
            mid_alignSet, 
            mid_autoOrientsGet, 
            mid_deviceOrientationGet, 
            mid_displayStateGet, 
            mid_displayStateSet, 
            mid_focusGet, 
            mid_focusSet, 
            mid_frameRateGet, 
            mid_frameRateSet, 
            mid_fullScreenHeightGet, 
            mid_fullScreenSourceRectGet, 
            mid_fullScreenSourceRectSet, 
            mid_fullScreenWidthGet, 
            mid_heightGet, 
            mid_heightSet, 
            mid_mouseChildrenGet, 
            mid_mouseChildrenSet, 
            mid_nativeWindowGet, 
            mid_numChildrenGet, 
            mid_orientationGet, 
            mid_qualityGet, 
            mid_qualitySet, 
            mid_scaleModeGet, 
            mid_scaleModeSet, 
            mid_showDefaultContextMenuGet, 
            mid_showDefaultContextMenuSet, 
            mid_stageFocusRectGet, 
            mid_stageFocusRectSet, 
            mid_stageHeightGet, 
            mid_stageHeightSet, 
            mid_stageWidthGet, 
            mid_stageWidthSet, 
            mid_tabChildrenGet, 
            mid_tabChildrenSet, 
            mid_textSnapshotGet, 
            mid_widthGet, 
            mid_widthSet, 
            mid_addChild, 
            mid_addChildAt, 
            mid_addEventListener, 
            mid_assignFocus, 
            mid_dispatchEvent, 
            mid_hasEventListener, 
            mid_invalidate, 
            mid_isFocusInaccessible, 
            mid_removeChildAt, 
            mid_removeEventListener, 
            mid_setChildIndex, 
            mid_setOrientation, 
            mid_swapChildrenAt, 
            mid_willTrigger, 
        };
        void alignGet(ASString& result);
        void alignSet(const Value& result, const ASString& value);
        void autoOrientsGet(bool& result);
        void deviceOrientationGet(ASString& result);
        void displayStateGet(ASString& result);
        void displayStateSet(const Value& result, const ASString& value);
        void focusGet(SPtr<Instances::fl_display::InteractiveObject>& result);
        void focusSet(const Value& result, Instances::fl_display::InteractiveObject* value);
        void frameRateGet(Value::Number& result);
        void frameRateSet(const Value& result, Value::Number value);
        void fullScreenHeightGet(UInt32& result);
        void fullScreenSourceRectGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void fullScreenSourceRectSet(const Value& result, Instances::fl_geom::Rectangle* value);
        void fullScreenWidthGet(UInt32& result);
        void heightGet(Value::Number& result);
        void heightSet(const Value& result, Value::Number value);
        void mouseChildrenGet(bool& result);
        void mouseChildrenSet(const Value& result, bool value);
        void nativeWindowGet(SPtr<Instances::fl_display::NativeWindow>& result);
        void numChildrenGet(SInt32& result);
        void orientationGet(ASString& result);
        void qualityGet(ASString& result);
        void qualitySet(const Value& result, const ASString& value);
        void scaleModeGet(ASString& result);
        void scaleModeSet(const Value& result, const ASString& value);
        void showDefaultContextMenuGet(bool& result);
        void showDefaultContextMenuSet(const Value& result, bool value);
        void stageFocusRectGet(bool& result);
        void stageFocusRectSet(const Value& result, bool value);
        void stageHeightGet(SInt32& result);
        void stageHeightSet(const Value& result, SInt32 value);
        void stageWidthGet(SInt32& result);
        void stageWidthSet(const Value& result, SInt32 value);
        void tabChildrenGet(bool& result);
        void tabChildrenSet(const Value& result, bool value);
        void textSnapshotGet(SPtr<Instances::fl_text::TextSnapshot>& result);
        void widthGet(Value::Number& result);
        void widthSet(const Value& result, Value::Number value);
        void addChild(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child);
        void addChildAt(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child, SInt32 index);
        void addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference);
        void assignFocus(const Value& result, Instances::fl_display::InteractiveObject* objectToFocus, const ASString& direction);
        void dispatchEvent(bool& result, Instances::fl_events::Event* event);
        void hasEventListener(bool& result, const ASString& type);
        void invalidate(const Value& result);
        void isFocusInaccessible(bool& result);
        void removeChildAt(SPtr<Instances::fl_display::DisplayObject>& result, SInt32 index);
        void removeEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture);
        void setChildIndex(const Value& result, Instances::fl_display::DisplayObject* child, SInt32 index);
        void setOrientation(const Value& result, const ASString& newOrientation);
        void swapChildrenAt(const Value& result, SInt32 index1, SInt32 index2);
        void willTrigger(bool& result, const ASString& type);

        // C++ friendly wrappers for AS3 methods.
        ASString alignGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            alignGet(result);
            return result;
        }
        void alignSet(const ASString& value)
        {
            alignSet(Value::GetUndefined(), value);
        }
        bool autoOrientsGet()
        {
            bool result;
            autoOrientsGet(result);
            return result;
        }
        ASString deviceOrientationGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            deviceOrientationGet(result);
            return result;
        }
        ASString displayStateGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            displayStateGet(result);
            return result;
        }
        void displayStateSet(const ASString& value)
        {
            displayStateSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::InteractiveObject> focusGet();
        void focusSet(Instances::fl_display::InteractiveObject* value)
        {
            focusSet(Value::GetUndefined(), value);
        }
        Value::Number frameRateGet()
        {
            Value::Number result;
            frameRateGet(result);
            return result;
        }
        void frameRateSet(Value::Number value)
        {
            frameRateSet(Value::GetUndefined(), value);
        }
        UInt32 fullScreenHeightGet()
        {
            UInt32 result;
            fullScreenHeightGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Rectangle> fullScreenSourceRectGet();
        void fullScreenSourceRectSet(Instances::fl_geom::Rectangle* value)
        {
            fullScreenSourceRectSet(Value::GetUndefined(), value);
        }
        UInt32 fullScreenWidthGet()
        {
            UInt32 result;
            fullScreenWidthGet(result);
            return result;
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
        bool mouseChildrenGet()
        {
            bool result;
            mouseChildrenGet(result);
            return result;
        }
        void mouseChildrenSet(bool value)
        {
            mouseChildrenSet(Value::GetUndefined(), value);
        }
        SInt32 numChildrenGet()
        {
            SInt32 result;
            numChildrenGet(result);
            return result;
        }
        ASString orientationGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            orientationGet(result);
            return result;
        }
        ASString qualityGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            qualityGet(result);
            return result;
        }
        void qualitySet(const ASString& value)
        {
            qualitySet(Value::GetUndefined(), value);
        }
        ASString scaleModeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            scaleModeGet(result);
            return result;
        }
        void scaleModeSet(const ASString& value)
        {
            scaleModeSet(Value::GetUndefined(), value);
        }
        bool showDefaultContextMenuGet()
        {
            bool result;
            showDefaultContextMenuGet(result);
            return result;
        }
        void showDefaultContextMenuSet(bool value)
        {
            showDefaultContextMenuSet(Value::GetUndefined(), value);
        }
        bool stageFocusRectGet()
        {
            bool result;
            stageFocusRectGet(result);
            return result;
        }
        void stageFocusRectSet(bool value)
        {
            stageFocusRectSet(Value::GetUndefined(), value);
        }
        SInt32 stageHeightGet()
        {
            SInt32 result;
            stageHeightGet(result);
            return result;
        }
        void stageHeightSet(SInt32 value)
        {
            stageHeightSet(Value::GetUndefined(), value);
        }
        SInt32 stageWidthGet()
        {
            SInt32 result;
            stageWidthGet(result);
            return result;
        }
        void stageWidthSet(SInt32 value)
        {
            stageWidthSet(Value::GetUndefined(), value);
        }
        bool tabChildrenGet()
        {
            bool result;
            tabChildrenGet(result);
            return result;
        }
        void tabChildrenSet(bool value)
        {
            tabChildrenSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_text::TextSnapshot> textSnapshotGet();
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
        SPtr<Instances::fl_display::DisplayObject> addChild(Instances::fl_display::DisplayObject* child);
        SPtr<Instances::fl_display::DisplayObject> addChildAt(Instances::fl_display::DisplayObject* child, SInt32 index);
        void addEventListener(const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
        {
            addEventListener(Value::GetUndefined(), type, listener, useCapture, priority, useWeakReference);
        }
        void assignFocus(Instances::fl_display::InteractiveObject* objectToFocus, const ASString& direction)
        {
            assignFocus(Value::GetUndefined(), objectToFocus, direction);
        }
        bool dispatchEvent(Instances::fl_events::Event* event)
        {
            bool result;
            dispatchEvent(result, event);
            return result;
        }
        bool hasEventListener(const ASString& type)
        {
            bool result;
            hasEventListener(result, type);
            return result;
        }
        void invalidate()
        {
            invalidate(Value::GetUndefined());
        }
        bool isFocusInaccessible()
        {
            bool result;
            isFocusInaccessible(result);
            return result;
        }
        SPtr<Instances::fl_display::DisplayObject> removeChildAt(SInt32 index);
        void removeEventListener(const ASString& type, const Value& listener, bool useCapture)
        {
            removeEventListener(Value::GetUndefined(), type, listener, useCapture);
        }
        void setChildIndex(Instances::fl_display::DisplayObject* child, SInt32 index)
        {
            setChildIndex(Value::GetUndefined(), child, index);
        }
        void setOrientation(const ASString& newOrientation)
        {
            setOrientation(Value::GetUndefined(), newOrientation);
        }
        void swapChildrenAt(SInt32 index1, SInt32 index2)
        {
            swapChildrenAt(Value::GetUndefined(), index1, index2);
        }
        bool willTrigger(const ASString& type)
        {
            bool result;
            willTrigger(result, type);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Stage : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Stage"; }
#endif
    public:
        typedef Instances::fl_display::Stage InstanceType;

    public:
        Stage(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Stage> MakeInstance(Stage& t)
        {
            return Pickable<Instances::fl_display::Stage>(new(t.Alloc()) Instances::fl_display::Stage(t));
        }
        SPtr<Instances::fl_display::Stage> MakeInstanceS(Stage& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 51 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[88];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Stage : public fl_display::DisplayObjectContainer
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Stage"; }
#endif
    public:
        typedef Classes::fl_display::Stage ClassType;
        typedef InstanceTraits::fl_display::Stage InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Stage(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class Stage : public Class
    {
        friend class ClassTraits::fl_display::Stage;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::StageTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Stage"; }
#endif
    public:
        typedef Stage SelfType;
        typedef Stage ClassType;
        
    private:
        Stage(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_supportsOrientationChangeGet, 
        };
        void supportsOrientationChangeGet(bool& result);

        // C++ friendly wrappers for AS3 methods.
        bool supportsOrientationChangeGet()
        {
            bool result;
            supportsOrientationChangeGet(result);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

