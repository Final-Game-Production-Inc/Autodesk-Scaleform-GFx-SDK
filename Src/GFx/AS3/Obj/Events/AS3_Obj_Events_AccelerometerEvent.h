//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Jan, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Events_AccelerometerEvent_H
#define INC_AS3_Obj_Events_AccelerometerEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo AccelerometerEventTI;
    extern const ClassInfo AccelerometerEventCI;
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_events
{
    class AccelerometerEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class AccelerometerEvent;
}}

namespace Classes { namespace fl_events
{
    class AccelerometerEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class AccelerometerEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::AccelerometerEvent;
        
    public:
        typedef AccelerometerEvent SelfType;
        typedef Classes::fl_events::AccelerometerEvent ClassType;
        typedef InstanceTraits::fl_events::AccelerometerEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AccelerometerEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::AccelerometerEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        AccelerometerEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_accelerationXGet, 
            mid_accelerationXSet, 
            mid_accelerationYGet, 
            mid_accelerationYSet, 
            mid_accelerationZGet, 
            mid_accelerationZSet, 
            mid_timestampGet, 
            mid_timestampSet, 
            mid_clone, 
            mid_toString, 
        };
        void accelerationXGet(Value::Number& result);
        void accelerationXSet(const Value& result, Value::Number value);
        void accelerationYGet(Value::Number& result);
        void accelerationYSet(const Value& result, Value::Number value);
        void accelerationZGet(Value::Number& result);
        void accelerationZSet(const Value& result, Value::Number value);
        void timestampGet(Value::Number& result);
        void timestampSet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number accelerationXGet()
        {
            Value::Number result;
            accelerationXGet(result);
            return result;
        }
        void accelerationXSet(Value::Number value)
        {
            accelerationXSet(Value::GetUndefined(), value);
        }
        Value::Number accelerationYGet()
        {
            Value::Number result;
            accelerationYGet(result);
            return result;
        }
        void accelerationYSet(Value::Number value)
        {
            accelerationYSet(Value::GetUndefined(), value);
        }
        Value::Number accelerationZGet()
        {
            Value::Number result;
            accelerationZGet(result);
            return result;
        }
        void accelerationZSet(Value::Number value)
        {
            accelerationZSet(Value::GetUndefined(), value);
        }
        Value::Number timestampGet()
        {
            Value::Number result;
            timestampGet(result);
            return result;
        }
        void timestampSet(Value::Number value)
        {
            timestampSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
		Value::Number   accelerationX, accelerationY, accelerationZ;
        Value::Number   timestamp;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class AccelerometerEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::AccelerometerEvent"; }
#endif
    public:
        typedef Instances::fl_events::AccelerometerEvent InstanceType;

    public:
        AccelerometerEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::AccelerometerEvent> MakeInstance(AccelerometerEvent& t)
        {
            return Pickable<Instances::fl_events::AccelerometerEvent>(new(t.Alloc()) Instances::fl_events::AccelerometerEvent(t));
        }
        SPtr<Instances::fl_events::AccelerometerEvent> MakeInstanceS(AccelerometerEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 10 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class AccelerometerEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AccelerometerEvent"; }
#endif
    public:
        typedef Classes::fl_events::AccelerometerEvent ClassType;
        typedef InstanceTraits::fl_events::AccelerometerEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AccelerometerEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class AccelerometerEvent : public Class
    {
        friend class ClassTraits::fl_events::AccelerometerEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::AccelerometerEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::AccelerometerEvent"; }
#endif
    public:
        typedef AccelerometerEvent SelfType;
        typedef AccelerometerEvent ClassType;
        
    private:
        AccelerometerEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* UPDATE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

