//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Timer.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_Timer_H
#define INC_AS3_Obj_Utils_Timer_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "../Events/AS3_Obj_Events_TimerEvent.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo TimerTI;
    extern const ClassInfo TimerCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_utils
{
    class Timer;
}}

namespace InstanceTraits { namespace fl_utils
{
    class Timer;
}}

namespace Classes { namespace fl_utils
{
    class Timer;
}}

//##protect##"forward_declaration"
    class IntervalTimer;
//##protect##"forward_declaration"

namespace Instances { namespace fl_utils
{
    class Timer : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_utils::Timer;
        
    public:
        typedef Timer SelfType;
        typedef Classes::fl_utils::Timer ClassType;
        typedef InstanceTraits::fl_utils::Timer TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_utils::TimerTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_utils::Timer"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Timer(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        void AS3Constructor(unsigned argc, const Value* argv);
        SPtr<Instances::fl_events::TimerEvent> CreateTimerEventObject(const ASString& type);
    public:
        void ExecuteEvent();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_currentCountGet, 
            mid_delayGet, 
            mid_delaySet, 
            mid_repeatCountGet, 
            mid_repeatCountSet, 
            mid_runningGet, 
            mid_reset, 
            mid_start, 
            mid_stop, 
        };
        void currentCountGet(SInt32& result);
        void delayGet(Value::Number& result);
        void delaySet(const Value& result, Value::Number value);
        void repeatCountGet(SInt32& result);
        void repeatCountSet(const Value& result, SInt32 value);
        void runningGet(bool& result);
        void reset(const Value& result);
        void start(const Value& result);
        void stop(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        SInt32 currentCountGet()
        {
            SInt32 result;
            currentCountGet(result);
            return result;
        }
        Value::Number delayGet()
        {
            Value::Number result;
            delayGet(result);
            return result;
        }
        void delaySet(Value::Number value)
        {
            delaySet(Value::GetUndefined(), value);
        }
        SInt32 repeatCountGet()
        {
            SInt32 result;
            repeatCountGet(result);
            return result;
        }
        void repeatCountSet(SInt32 value)
        {
            repeatCountSet(Value::GetUndefined(), value);
        }
        bool runningGet()
        {
            bool result;
            runningGet(result);
            return result;
        }
        void reset()
        {
            reset(Value::GetUndefined());
        }
        void start()
        {
            start(Value::GetUndefined());
        }
        void stop()
        {
            stop(Value::GetUndefined());
        }

//##protect##"instance$data"
        SInt32              RepeatCount;
        SInt32              CurrentCount;
        Value::Number       Delay;
        Ptr<IntervalTimer>  pCoreTimer;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    class Timer : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Timer"; }
#endif
    public:
        typedef Instances::fl_utils::Timer InstanceType;

    public:
        Timer(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_utils::Timer> MakeInstance(Timer& t)
        {
            return Pickable<Instances::fl_utils::Timer>(new(t.Alloc()) Instances::fl_utils::Timer(t));
        }
        SPtr<Instances::fl_utils::Timer> MakeInstanceS(Timer& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 9 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[11];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_utils
{
    class Timer : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Timer"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_utils::Timer InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Timer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

