//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_TimerEvent.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_TimerEvent_H
#define INC_AS3_Obj_Events_TimerEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo TimerEventTI;
    extern const ClassInfo TimerEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class TimerEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class TimerEvent;
}}

namespace Classes { namespace fl_events
{
    class TimerEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class TimerEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::TimerEvent;
        
    public:
        typedef TimerEvent SelfType;
        typedef Classes::fl_events::TimerEvent ClassType;
        typedef InstanceTraits::fl_events::TimerEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TimerEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::TimerEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TimerEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual const char*     GetEventName() const { return "TimerEvent"; }
        virtual AS3::Object*    GetEventClass() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clone, 
            mid_toString, 
            mid_updateAfterEvent, 
        };
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);
        void updateAfterEvent(const Value& result);

        // C++ friendly wrappers for AS3 methods.
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
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class TimerEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TimerEvent"; }
#endif
    public:
        typedef Instances::fl_events::TimerEvent InstanceType;

    public:
        TimerEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::TimerEvent> MakeInstance(TimerEvent& t)
        {
            return Pickable<Instances::fl_events::TimerEvent>(new(t.Alloc()) Instances::fl_events::TimerEvent(t));
        }
        SPtr<Instances::fl_events::TimerEvent> MakeInstanceS(TimerEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class TimerEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TimerEvent"; }
#endif
    public:
        typedef Classes::fl_events::TimerEvent ClassType;
        typedef InstanceTraits::fl_events::TimerEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TimerEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class TimerEvent : public Class
    {
        friend class ClassTraits::fl_events::TimerEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::TimerEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TimerEvent"; }
#endif
    public:
        typedef TimerEvent SelfType;
        typedef TimerEvent ClassType;
        
    private:
        TimerEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* TIMER;
        const char* TIMER_COMPLETE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

