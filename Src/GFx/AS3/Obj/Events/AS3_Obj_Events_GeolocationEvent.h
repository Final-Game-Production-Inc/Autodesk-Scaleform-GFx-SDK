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

#ifndef INC_AS3_Obj_Events_GeolocationEvent_H
#define INC_AS3_Obj_Events_GeolocationEvent_H

#include "AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_events
{
    extern const TypeInfo GeolocationEventTI;
    extern const ClassInfo GeolocationEventCI;
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
    class GeolocationEvent;
}}

namespace InstanceTraits { namespace fl_events
{
    class GeolocationEvent;
}}

namespace Classes { namespace fl_events
{
    class GeolocationEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_events
{
    class GeolocationEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_events::GeolocationEvent;
        
    public:
        typedef GeolocationEvent SelfType;
        typedef Classes::fl_events::GeolocationEvent ClassType;
        typedef InstanceTraits::fl_events::GeolocationEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::GeolocationEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_events::GeolocationEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GeolocationEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_altitudeGet, 
            mid_altitudeSet, 
            mid_headingGet, 
            mid_headingSet, 
            mid_horizontalAccuracyGet, 
            mid_horizontalAccuracySet, 
            mid_latitudeGet, 
            mid_latitudeSet, 
            mid_longitudeGet, 
            mid_longitudeSet, 
            mid_speedGet, 
            mid_speedSet, 
            mid_timestampGet, 
            mid_timestampSet, 
            mid_verticalAccuracyGet, 
            mid_verticalAccuracySet, 
            mid_clone, 
            mid_toString, 
        };
        void altitudeGet(Value::Number& result);
        void altitudeSet(const Value& result, Value::Number value);
        void headingGet(Value::Number& result);
        void headingSet(const Value& result, Value::Number value);
        void horizontalAccuracyGet(Value::Number& result);
        void horizontalAccuracySet(const Value& result, Value::Number value);
        void latitudeGet(Value::Number& result);
        void latitudeSet(const Value& result, Value::Number value);
        void longitudeGet(Value::Number& result);
        void longitudeSet(const Value& result, Value::Number value);
        void speedGet(Value::Number& result);
        void speedSet(const Value& result, Value::Number value);
        void timestampGet(Value::Number& result);
        void timestampSet(const Value& result, Value::Number value);
        void verticalAccuracyGet(Value::Number& result);
        void verticalAccuracySet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number altitudeGet()
        {
            Value::Number result;
            altitudeGet(result);
            return result;
        }
        void altitudeSet(Value::Number value)
        {
            altitudeSet(Value::GetUndefined(), value);
        }
        Value::Number headingGet()
        {
            Value::Number result;
            headingGet(result);
            return result;
        }
        void headingSet(Value::Number value)
        {
            headingSet(Value::GetUndefined(), value);
        }
        Value::Number horizontalAccuracyGet()
        {
            Value::Number result;
            horizontalAccuracyGet(result);
            return result;
        }
        void horizontalAccuracySet(Value::Number value)
        {
            horizontalAccuracySet(Value::GetUndefined(), value);
        }
        Value::Number latitudeGet()
        {
            Value::Number result;
            latitudeGet(result);
            return result;
        }
        void latitudeSet(Value::Number value)
        {
            latitudeSet(Value::GetUndefined(), value);
        }
        Value::Number longitudeGet()
        {
            Value::Number result;
            longitudeGet(result);
            return result;
        }
        void longitudeSet(Value::Number value)
        {
            longitudeSet(Value::GetUndefined(), value);
        }
        Value::Number speedGet()
        {
            Value::Number result;
            speedGet(result);
            return result;
        }
        void speedSet(Value::Number value)
        {
            speedSet(Value::GetUndefined(), value);
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
        Value::Number verticalAccuracyGet()
        {
            Value::Number result;
            verticalAccuracyGet(result);
            return result;
        }
        void verticalAccuracySet(Value::Number value)
        {
            verticalAccuracySet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
		Value::Number latitude, longitude, altitude;
		Value::Number hAccuracy, vAccuracy, speed, heading;
		Value::Number timestamp;

//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    class GeolocationEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GeolocationEvent"; }
#endif
    public:
        typedef Instances::fl_events::GeolocationEvent InstanceType;

    public:
        GeolocationEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_events::GeolocationEvent> MakeInstance(GeolocationEvent& t)
        {
            return Pickable<Instances::fl_events::GeolocationEvent>(new(t.Alloc()) Instances::fl_events::GeolocationEvent(t));
        }
        SPtr<Instances::fl_events::GeolocationEvent> MakeInstanceS(GeolocationEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 18 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[26];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_events
{
    class GeolocationEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GeolocationEvent"; }
#endif
    public:
        typedef Classes::fl_events::GeolocationEvent ClassType;
        typedef InstanceTraits::fl_events::GeolocationEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GeolocationEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_events
{
    class GeolocationEvent : public Class
    {
        friend class ClassTraits::fl_events::GeolocationEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_events::GeolocationEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GeolocationEvent"; }
#endif
    public:
        typedef GeolocationEvent SelfType;
        typedef GeolocationEvent ClassType;
        
    private:
        GeolocationEvent(ClassTraits::Traits& t);
       
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

