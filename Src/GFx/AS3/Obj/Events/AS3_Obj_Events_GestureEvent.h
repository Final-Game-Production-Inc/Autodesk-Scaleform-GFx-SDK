//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_GestureEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_GestureEvent_H
#define INC_AS3_Obj_Events_GestureEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo GestureEventTI;
    extern const ClassInfo GestureEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class GestureEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class GestureEvent;
}}

namespace Classes { namespace fl_events
{
    class GestureEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class GestureEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::GestureEvent;
        
    public:
        typedef GestureEvent SelfType;
        typedef Classes::fl_events::GestureEvent ClassType;
        typedef InstanceTraits::fl_events::GestureEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::GestureEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::GestureEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GestureEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        enum PhaseType
        {
            Phase_Null,
            Phase_All,
            Phase_Begin,
            Phase_End,
            Phase_Update,

            Phase_Count
        };

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
            mid_localXGet, 
            mid_localXSet, 
            mid_localYGet, 
            mid_localYSet, 
            mid_phaseGet, 
            mid_phaseSet, 
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
        void commandKeyGet(bool& result);
        void commandKeySet(const Value& result, bool value);
        void controlKeyGet(bool& result);
        void controlKeySet(const Value& result, bool value);
        void ctrlKeyGet(bool& result);
        void ctrlKeySet(const Value& result, bool value);
        void localXGet(Value::Number& result);
        void localXSet(const Value& result, Value::Number value);
        void localYGet(Value::Number& result);
        void localYSet(const Value& result, Value::Number value);
        void phaseGet(ASString& result);
        void phaseSet(const Value& result, const ASString& value);
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
        ASString phaseGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            phaseGet(result);
            return result;
        }
        void phaseSet(const ASString& value)
        {
            phaseSet(Value::GetUndefined(), value);
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
        bool            AltKey, CtrlKey, ShiftKey, CommandKey, ControlKey;
        Value::Number   LocalX, LocalY;
        Value::Number   StageX, StageY;
        PhaseType       Phase;           
        bool            LocalInitialized;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class GestureEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GestureEvent"; }
#endif
    public:
        typedef Instances::fl_events::GestureEvent InstanceType;

    public:
        GestureEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::GestureEvent> MakeInstance(GestureEvent& t)
        {
            return Pickable<Instances::fl_events::GestureEvent>(new(t.Alloc()) Instances::fl_events::GestureEvent(t));
        }
        SPtr<Instances::fl_events::GestureEvent> MakeInstanceS(GestureEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 21 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[29];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class GestureEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GestureEvent"; }
#endif
    public:
        typedef Classes::fl_events::GestureEvent ClassType;
        typedef InstanceTraits::fl_events::GestureEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GestureEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class GestureEvent : public Class
    {
        friend class ClassTraits::fl_events::GestureEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::GestureEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GestureEvent"; }
#endif
    public:
        typedef GestureEvent SelfType;
        typedef GestureEvent ClassType;
        
    private:
        GestureEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* GESTURE_TWO_FINGER_TAP;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

