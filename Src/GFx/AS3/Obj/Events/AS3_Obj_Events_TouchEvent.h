//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_TouchEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_TouchEvent_H
#define INC_AS3_Obj_Events_TouchEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo TouchEventTI;
    extern const ClassInfo TouchEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_display
{
    extern const TypeInfo InteractiveObjectTI;
    extern const ClassInfo InteractiveObjectCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_events
{
    class TouchEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class TouchEvent;
}}

namespace Classes { namespace fl_events
{
    class TouchEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class TouchEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::TouchEvent;
        
    public:
        typedef TouchEvent SelfType;
        typedef Classes::fl_events::TouchEvent ClassType;
        typedef InstanceTraits::fl_events::TouchEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TouchEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::TouchEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TouchEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
        void InitLocalCoords();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_altKeyGet, 
            mid_altKeySet, 
            mid_commandKeyGet, 
            mid_commandKeySet, 
            mid_controlKeyGet, 
            mid_controlKeySet, 
            mid_ctrlKeyGet, 
            mid_ctrlKeySet, 
            mid_isPrimaryTouchPointGet, 
            mid_isPrimaryTouchPointSet, 
            mid_isRelatedObjectInaccessibleGet, 
            mid_isRelatedObjectInaccessibleSet, 
            mid_localXGet, 
            mid_localXSet, 
            mid_localYGet, 
            mid_localYSet, 
            mid_pressureGet, 
            mid_pressureSet, 
            mid_relatedObjectGet, 
            mid_relatedObjectSet, 
            mid_shiftKeyGet, 
            mid_shiftKeySet, 
            mid_sizeXGet, 
            mid_sizeXSet, 
            mid_sizeYGet, 
            mid_sizeYSet, 
            mid_stageXGet, 
            mid_stageYGet, 
            mid_touchPointIDGet, 
            mid_touchPointIDSet, 
            mid_clone, 
            mid_toString, 
            mid_updateAfterEvent, 
        };
        void altKeyGet(bool& result);
        void altKeySet(const Value& result, bool value);
        void commandKeyGet(bool& result);
        void commandKeySet(const Value& result, bool value);
        void controlKeyGet(bool& result);
        void controlKeySet(const Value& result, bool value);
        void ctrlKeyGet(bool& result);
        void ctrlKeySet(const Value& result, bool value);
        void isPrimaryTouchPointGet(bool& result);
        void isPrimaryTouchPointSet(const Value& result, bool value);
        void isRelatedObjectInaccessibleGet(bool& result);
        void isRelatedObjectInaccessibleSet(const Value& result, bool value);
        void localXGet(Value::Number& result);
        void localXSet(const Value& result, Value::Number value);
        void localYGet(Value::Number& result);
        void localYSet(const Value& result, Value::Number value);
        void pressureGet(Value::Number& result);
        void pressureSet(const Value& result, Value::Number value);
        void relatedObjectGet(SPtr<Instances::fl_display::InteractiveObject>& result);
        void relatedObjectSet(const Value& result, Instances::fl_display::InteractiveObject* value);
        void shiftKeyGet(bool& result);
        void shiftKeySet(const Value& result, bool value);
        void sizeXGet(Value::Number& result);
        void sizeXSet(const Value& result, Value::Number value);
        void sizeYGet(Value::Number& result);
        void sizeYSet(const Value& result, Value::Number value);
        void stageXGet(Value::Number& result);
        void stageYGet(Value::Number& result);
        void touchPointIDGet(SInt32& result);
        void touchPointIDSet(const Value& result, SInt32 value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);
        void updateAfterEvent(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        bool altKeyGet()
        {
            bool result;
            altKeyGet(result);
            return result;
        }
        void altKeySet(bool value)
        {
            altKeySet(Value::GetUndefined(), value);
        }
        bool commandKeyGet()
        {
            bool result;
            commandKeyGet(result);
            return result;
        }
        void commandKeySet(bool value)
        {
            commandKeySet(Value::GetUndefined(), value);
        }
        bool controlKeyGet()
        {
            bool result;
            controlKeyGet(result);
            return result;
        }
        void controlKeySet(bool value)
        {
            controlKeySet(Value::GetUndefined(), value);
        }
        bool ctrlKeyGet()
        {
            bool result;
            ctrlKeyGet(result);
            return result;
        }
        void ctrlKeySet(bool value)
        {
            ctrlKeySet(Value::GetUndefined(), value);
        }
        bool isPrimaryTouchPointGet()
        {
            bool result;
            isPrimaryTouchPointGet(result);
            return result;
        }
        void isPrimaryTouchPointSet(bool value)
        {
            isPrimaryTouchPointSet(Value::GetUndefined(), value);
        }
        bool isRelatedObjectInaccessibleGet()
        {
            bool result;
            isRelatedObjectInaccessibleGet(result);
            return result;
        }
        void isRelatedObjectInaccessibleSet(bool value)
        {
            isRelatedObjectInaccessibleSet(Value::GetUndefined(), value);
        }
        Value::Number localXGet()
        {
            Value::Number result;
            localXGet(result);
            return result;
        }
        void localXSet(Value::Number value)
        {
            localXSet(Value::GetUndefined(), value);
        }
        Value::Number localYGet()
        {
            Value::Number result;
            localYGet(result);
            return result;
        }
        void localYSet(Value::Number value)
        {
            localYSet(Value::GetUndefined(), value);
        }
        Value::Number pressureGet()
        {
            Value::Number result;
            pressureGet(result);
            return result;
        }
        void pressureSet(Value::Number value)
        {
            pressureSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::InteractiveObject> relatedObjectGet();
        void relatedObjectSet(Instances::fl_display::InteractiveObject* value)
        {
            relatedObjectSet(Value::GetUndefined(), value);
        }
        bool shiftKeyGet()
        {
            bool result;
            shiftKeyGet(result);
            return result;
        }
        void shiftKeySet(bool value)
        {
            shiftKeySet(Value::GetUndefined(), value);
        }
        Value::Number sizeXGet()
        {
            Value::Number result;
            sizeXGet(result);
            return result;
        }
        void sizeXSet(Value::Number value)
        {
            sizeXSet(Value::GetUndefined(), value);
        }
        Value::Number sizeYGet()
        {
            Value::Number result;
            sizeYGet(result);
            return result;
        }
        void sizeYSet(Value::Number value)
        {
            sizeYSet(Value::GetUndefined(), value);
        }
        Value::Number stageXGet()
        {
            Value::Number result;
            stageXGet(result);
            return result;
        }
        Value::Number stageYGet()
        {
            Value::Number result;
            stageYGet(result);
            return result;
        }
        SInt32 touchPointIDGet()
        {
            SInt32 result;
            touchPointIDGet(result);
            return result;
        }
        void touchPointIDSet(SInt32 value)
        {
            touchPointIDSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }
        void updateAfterEvent()
        {
            updateAfterEvent(Value::GetUndefined());
        }

//##protect##"instance$data"
    public:
        bool            AltKey, CtrlKey, ShiftKey, CommandKey, ControlKey;
        SPtr<Instances::fl_display::InteractiveObject> RelatedObj;
        Value::Number   LocalX, LocalY;
        Value::Number   SizeX, SizeY;
        Value::Number   StageX, StageY;
        Value::Number   Pressure;
        SInt32          TouchPointID;
        bool            PrimaryPoint;
        bool            LocalInitialized;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class TouchEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TouchEvent"; }
#endif
    public:
        typedef Instances::fl_events::TouchEvent InstanceType;

    public:
        TouchEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::TouchEvent> MakeInstance(TouchEvent& t)
        {
            return Pickable<Instances::fl_events::TouchEvent>(new(t.Alloc()) Instances::fl_events::TouchEvent(t));
        }
        SPtr<Instances::fl_events::TouchEvent> MakeInstanceS(TouchEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 33 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[47];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class TouchEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TouchEvent"; }
#endif
    public:
        typedef Classes::fl_events::TouchEvent ClassType;
        typedef InstanceTraits::fl_events::TouchEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TouchEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class TouchEvent : public Class
    {
        friend class ClassTraits::fl_events::TouchEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TouchEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TouchEvent"; }
#endif
    public:
        typedef TouchEvent SelfType;
        typedef TouchEvent ClassType;
        
    private:
        TouchEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* TOUCH_BEGIN;
        const char* TOUCH_END;
        const char* TOUCH_MOVE;
        const char* TOUCH_OUT;
        const char* TOUCH_OVER;
        const char* TOUCH_ROLL_OUT;
        const char* TOUCH_ROLL_OVER;
        const char* TOUCH_TAP;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

