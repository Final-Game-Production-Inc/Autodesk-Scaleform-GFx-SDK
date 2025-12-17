//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_MouseEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_MouseEvent_H
#define INC_AS3_Obj_Events_MouseEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
#include "../Display/AS3_Obj_Display_InteractiveObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo MouseEventTI;
    extern const ClassInfo MouseEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
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
    class MouseEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class MouseEvent;
}}

namespace Classes { namespace fl_events
{
    class MouseEvent;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class InteractiveObject;
    }
}
#ifdef DOUBLE_CLICK
#undef DOUBLE_CLICK
#endif
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class MouseEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::MouseEvent;
        
    public:
        typedef MouseEvent SelfType;
        typedef Classes::fl_events::MouseEvent ClassType;
        typedef InstanceTraits::fl_events::MouseEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::MouseEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::MouseEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        MouseEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
    public:
        void SetStageCoords(const PointF& p);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_altKeyGet, 
            mid_altKeySet, 
            mid_buttonDownGet, 
            mid_buttonDownSet, 
            mid_clickCountGet, 
            mid_commandKeyGet, 
            mid_commandKeySet, 
            mid_controlKeyGet, 
            mid_controlKeySet, 
            mid_ctrlKeyGet, 
            mid_ctrlKeySet, 
            mid_deltaGet, 
            mid_deltaSet, 
            mid_localXGet, 
            mid_localXSet, 
            mid_localYGet, 
            mid_localYSet, 
            mid_relatedObjectGet, 
            mid_relatedObjectSet, 
            mid_shiftKeyGet, 
            mid_shiftKeySet, 
            mid_stageXGet, 
            mid_stageYGet, 
            mid_clone, 
            mid_toString, 
            mid_updateAfterEvent, 
        };
        void altKeyGet(bool& result);
        void altKeySet(const Value& result, bool value);
        void buttonDownGet(bool& result);
        void buttonDownSet(const Value& result, bool value);
        void clickCountGet(SInt32& result);
        void commandKeyGet(bool& result);
        void commandKeySet(const Value& result, bool value);
        void controlKeyGet(bool& result);
        void controlKeySet(const Value& result, bool value);
        void ctrlKeyGet(bool& result);
        void ctrlKeySet(const Value& result, bool value);
        void deltaGet(SInt32& result);
        void deltaSet(const Value& result, SInt32 value);
        void localXGet(Value::Number& result);
        void localXSet(const Value& result, Value::Number value);
        void localYGet(Value::Number& result);
        void localYSet(const Value& result, Value::Number value);
        void relatedObjectGet(SPtr<Instances::fl_display::InteractiveObject>& result);
        void relatedObjectSet(const Value& result, Instances::fl_display::InteractiveObject* value);
        void shiftKeyGet(bool& result);
        void shiftKeySet(const Value& result, bool value);
        void stageXGet(Value::Number& result);
        void stageYGet(Value::Number& result);
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
        bool buttonDownGet()
        {
            bool result;
            buttonDownGet(result);
            return result;
        }
        void buttonDownSet(bool value)
        {
            buttonDownSet(Value::GetUndefined(), value);
        }
        SInt32 clickCountGet()
        {
            SInt32 result;
            clickCountGet(result);
            return result;
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
        SInt32 deltaGet()
        {
            SInt32 result;
            deltaGet(result);
            return result;
        }
        void deltaSet(SInt32 value)
        {
            deltaSet(Value::GetUndefined(), value);
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
        int             Delta;
        SPtr<Instances::fl_display::InteractiveObject> RelatedObj;
        Value::Number   LocalX, LocalY;
        enum
        {
            Button_Left     = 0x1,
            Button_Right    = 0x2,
            Button_Middle   = 0x4
        };
        unsigned        ButtonsMask; // bit 0 - left, bit 1 - right, bit 2 - middle, etc
        bool            AltKey, CtrlKey, ShiftKey;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class MouseEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::MouseEvent"; }
#endif
    public:
        typedef Instances::fl_events::MouseEvent InstanceType;

    public:
        MouseEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::MouseEvent> MakeInstance(MouseEvent& t)
        {
            return Pickable<Instances::fl_events::MouseEvent>(new(t.Alloc()) Instances::fl_events::MouseEvent(t));
        }
        SPtr<Instances::fl_events::MouseEvent> MakeInstanceS(MouseEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 26 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[36];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class MouseEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MouseEvent"; }
#endif
    public:
        typedef Classes::fl_events::MouseEvent ClassType;
        typedef InstanceTraits::fl_events::MouseEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MouseEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 17 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class MouseEvent : public Class
    {
        friend class ClassTraits::fl_events::MouseEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::MouseEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::MouseEvent"; }
#endif
    public:
        typedef MouseEvent SelfType;
        typedef MouseEvent ClassType;
        
    private:
        MouseEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CLICK;
        const char* CONTEXT_MENU;
        const char* DOUBLE_CLICK;
        const char* MIDDLE_CLICK;
        const char* MIDDLE_MOUSE_DOWN;
        const char* MIDDLE_MOUSE_UP;
        const char* MOUSE_DOWN;
        const char* MOUSE_MOVE;
        const char* MOUSE_OUT;
        const char* MOUSE_OVER;
        const char* MOUSE_UP;
        const char* MOUSE_WHEEL;
        const char* RIGHT_CLICK;
        const char* RIGHT_MOUSE_DOWN;
        const char* RIGHT_MOUSE_UP;
        const char* ROLL_OUT;
        const char* ROLL_OVER;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

