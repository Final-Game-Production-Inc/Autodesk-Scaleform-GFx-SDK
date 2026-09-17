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

#ifndef INC_AS3_Obj_Sensors_Accelerometer_H
#define INC_AS3_Obj_Sensors_Accelerometer_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_sensors
{
    extern const TypeInfo AccelerometerTI;
    extern const ClassInfo AccelerometerCI;
} // namespace fl_sensors
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_sensors
{
    class Accelerometer;
}}

namespace InstanceTraits { namespace fl_sensors
{
    class Accelerometer;
}}

namespace Classes { namespace fl_sensors
{
    class Accelerometer;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_sensors
{
    class Accelerometer : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_sensors::Accelerometer;
        
    public:
        typedef Accelerometer SelfType;
        typedef Classes::fl_sensors::Accelerometer ClassType;
        typedef InstanceTraits::fl_sensors::Accelerometer TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sensors::AccelerometerTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_sensors::Accelerometer"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Accelerometer(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		~Accelerometer();
		void addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_mutedGet, 
            mid_setRequestedUpdateInterval, 
        };
        void mutedGet(bool& result);
        void setRequestedUpdateInterval(const Value& result, Value::Number interval);

        // C++ friendly wrappers for AS3 methods.
        bool mutedGet()
        {
            bool result;
            mutedGet(result);
            return result;
        }
        void setRequestedUpdateInterval(Value::Number interval)
        {
            setRequestedUpdateInterval(Value::GetUndefined(), interval);
        }

//##protect##"instance$data"
		static int AccelerometerIdCount;
		int AccelerometerId;

//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_sensors
{
    class Accelerometer : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Accelerometer"; }
#endif
    public:
        typedef Instances::fl_sensors::Accelerometer InstanceType;

    public:
        Accelerometer(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_sensors::Accelerometer> MakeInstance(Accelerometer& t)
        {
            return Pickable<Instances::fl_sensors::Accelerometer>(new(t.Alloc()) Instances::fl_sensors::Accelerometer(t));
        }
        SPtr<Instances::fl_sensors::Accelerometer> MakeInstanceS(Accelerometer& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_sensors
{
    class Accelerometer : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Accelerometer"; }
#endif
    public:
        typedef Classes::fl_sensors::Accelerometer ClassType;
        typedef InstanceTraits::fl_sensors::Accelerometer InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Accelerometer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_sensors
{
    class Accelerometer : public Class
    {
        friend class ClassTraits::fl_sensors::Accelerometer;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sensors::AccelerometerTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Accelerometer"; }
#endif
    public:
        typedef Accelerometer SelfType;
        typedef Accelerometer ClassType;
        
    private:
        Accelerometer(ClassTraits::Traits& t);
       
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
            mid_isSupportedGet, 
        };
        void isSupportedGet(bool& result);

        // C++ friendly wrappers for AS3 methods.
        bool isSupportedGet()
        {
            bool result;
            isSupportedGet(result);
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

