//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_KeyboardEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_KeyboardEvent_H
#define INC_AS3_Obj_Events_KeyboardEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
#include "GFx/GFx_Input.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo KeyboardEventTI;
    extern const ClassInfo KeyboardEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class KeyboardEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class KeyboardEvent;
}}

namespace Classes { namespace fl_events
{
    class KeyboardEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class KeyboardEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::KeyboardEvent;
        
    public:
        typedef KeyboardEvent SelfType;
        typedef Classes::fl_events::KeyboardEvent ClassType;
        typedef InstanceTraits::fl_events::KeyboardEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::KeyboardEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::KeyboardEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        KeyboardEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
    public:
        void SetEventId(const EventId& eid);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_altKeyGet, 
            mid_altKeySet, 
            mid_charCodeGet, 
            mid_charCodeSet, 
            mid_commandKeyGet, 
            mid_commandKeySet, 
            mid_controlKeyGet, 
            mid_controlKeySet, 
            mid_ctrlKeyGet, 
            mid_ctrlKeySet, 
            mid_keyCodeGet, 
            mid_keyCodeSet, 
            mid_keyLocationGet, 
            mid_keyLocationSet, 
            mid_shiftKeyGet, 
            mid_shiftKeySet, 
            mid_clone, 
            mid_toString, 
            mid_updateAfterEvent, 
        };
        void altKeyGet(bool& result);
        void altKeySet(const Value& result, bool value);
        void charCodeGet(UInt32& result);
        void charCodeSet(const Value& result, UInt32 value);
        void commandKeyGet(bool& result);
        void commandKeySet(const Value& result, bool value);
        void controlKeyGet(bool& result);
        void controlKeySet(const Value& result, bool value);
        void ctrlKeyGet(bool& result);
        void ctrlKeySet(const Value& result, bool value);
        void keyCodeGet(UInt32& result);
        void keyCodeSet(const Value& result, UInt32 value);
        void keyLocationGet(UInt32& result);
        void keyLocationSet(const Value& result, UInt32 value);
        void shiftKeyGet(bool& result);
        void shiftKeySet(const Value& result, bool value);
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
        UInt32 charCodeGet()
        {
            UInt32 result;
            charCodeGet(result);
            return result;
        }
        void charCodeSet(UInt32 value)
        {
            charCodeSet(Value::GetUndefined(), value);
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
        UInt32 keyCodeGet()
        {
            UInt32 result;
            keyCodeGet(result);
            return result;
        }
        void keyCodeSet(UInt32 value)
        {
            keyCodeSet(Value::GetUndefined(), value);
        }
        UInt32 keyLocationGet()
        {
            UInt32 result;
            keyLocationGet(result);
            return result;
        }
        void keyLocationSet(UInt32 value)
        {
            keyLocationSet(Value::GetUndefined(), value);
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
        EventId     EvtId;
        int         KeyLocation;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class KeyboardEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::KeyboardEvent"; }
#endif
    public:
        typedef Instances::fl_events::KeyboardEvent InstanceType;

    public:
        KeyboardEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::KeyboardEvent> MakeInstance(KeyboardEvent& t)
        {
            return Pickable<Instances::fl_events::KeyboardEvent>(new(t.Alloc()) Instances::fl_events::KeyboardEvent(t));
        }
        SPtr<Instances::fl_events::KeyboardEvent> MakeInstanceS(KeyboardEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 19 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[27];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class KeyboardEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::KeyboardEvent"; }
#endif
    public:
        typedef Classes::fl_events::KeyboardEvent ClassType;
        typedef InstanceTraits::fl_events::KeyboardEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        KeyboardEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class KeyboardEvent : public Class
    {
        friend class ClassTraits::fl_events::KeyboardEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::KeyboardEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::KeyboardEvent"; }
#endif
    public:
        typedef KeyboardEvent SelfType;
        typedef KeyboardEvent ClassType;
        
    private:
        KeyboardEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* KEY_DOWN;
        const char* KEY_UP;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

