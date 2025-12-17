//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Timer.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_Timer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_IntervalTimer.h"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_currentCountGet, SInt32> TFunc_Instances_Timer_currentCountGet;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_delayGet, Value::Number> TFunc_Instances_Timer_delayGet;
typedef ThunkFunc1<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_delaySet, const Value, Value::Number> TFunc_Instances_Timer_delaySet;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_repeatCountGet, SInt32> TFunc_Instances_Timer_repeatCountGet;
typedef ThunkFunc1<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_repeatCountSet, const Value, SInt32> TFunc_Instances_Timer_repeatCountSet;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_runningGet, bool> TFunc_Instances_Timer_runningGet;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_reset, const Value> TFunc_Instances_Timer_reset;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_start, const Value> TFunc_Instances_Timer_start;
typedef ThunkFunc0<Instances::fl_utils::Timer, Instances::fl_utils::Timer::mid_stop, const Value> TFunc_Instances_Timer_stop;

template <> const TFunc_Instances_Timer_currentCountGet::TMethod TFunc_Instances_Timer_currentCountGet::Method = &Instances::fl_utils::Timer::currentCountGet;
template <> const TFunc_Instances_Timer_delayGet::TMethod TFunc_Instances_Timer_delayGet::Method = &Instances::fl_utils::Timer::delayGet;
template <> const TFunc_Instances_Timer_delaySet::TMethod TFunc_Instances_Timer_delaySet::Method = &Instances::fl_utils::Timer::delaySet;
template <> const TFunc_Instances_Timer_repeatCountGet::TMethod TFunc_Instances_Timer_repeatCountGet::Method = &Instances::fl_utils::Timer::repeatCountGet;
template <> const TFunc_Instances_Timer_repeatCountSet::TMethod TFunc_Instances_Timer_repeatCountSet::Method = &Instances::fl_utils::Timer::repeatCountSet;
template <> const TFunc_Instances_Timer_runningGet::TMethod TFunc_Instances_Timer_runningGet::Method = &Instances::fl_utils::Timer::runningGet;
template <> const TFunc_Instances_Timer_reset::TMethod TFunc_Instances_Timer_reset::Method = &Instances::fl_utils::Timer::reset;
template <> const TFunc_Instances_Timer_start::TMethod TFunc_Instances_Timer_start::Method = &Instances::fl_utils::Timer::start;
template <> const TFunc_Instances_Timer_stop::TMethod TFunc_Instances_Timer_stop::Method = &Instances::fl_utils::Timer::stop;

namespace Instances { namespace fl_utils
{
    Timer::Timer(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::Timer::Timer()$data"
    , RepeatCount(0), CurrentCount(0), Delay(0), pCoreTimer(NULL)
//##protect##"instance::Timer::Timer()$data"
    {
//##protect##"instance::Timer::Timer()$code"
//##protect##"instance::Timer::Timer()$code"
    }

    void Timer::currentCountGet(SInt32& result)
    {
//##protect##"instance::Timer::currentCountGet()"
        SF_UNUSED1(result);
        if (pCoreTimer)
            result = pCoreTimer->GetCurrentCount();
        else
            result = CurrentCount;
//##protect##"instance::Timer::currentCountGet()"
    }
    void Timer::delayGet(Value::Number& result)
    {
//##protect##"instance::Timer::delayGet()"
        SF_UNUSED1(result);
        result = Delay;
//##protect##"instance::Timer::delayGet()"
    }
    void Timer::delaySet(const Value& result, Value::Number value)
    {
//##protect##"instance::Timer::delaySet()"
        SF_UNUSED2(result, value);
        Delay = value;
//##protect##"instance::Timer::delaySet()"
    }
    void Timer::repeatCountGet(SInt32& result)
    {
//##protect##"instance::Timer::repeatCountGet()"
        SF_UNUSED1(result);
        result = RepeatCount;
//##protect##"instance::Timer::repeatCountGet()"
    }
    void Timer::repeatCountSet(const Value& result, SInt32 value)
    {
//##protect##"instance::Timer::repeatCountSet()"
        SF_UNUSED2(result, value);
        RepeatCount = value;
//##protect##"instance::Timer::repeatCountSet()"
    }
    void Timer::runningGet(bool& result)
    {
//##protect##"instance::Timer::runningGet()"
        SF_UNUSED1(result);
        if (pCoreTimer && pCoreTimer->IsActive())
            result = true;
        else
            result = false;
//##protect##"instance::Timer::runningGet()"
    }
    void Timer::reset(const Value& result)
    {
//##protect##"instance::Timer::reset()"
        stop(result);
        CurrentCount = 0;
//##protect##"instance::Timer::reset()"
    }
    void Timer::start(const Value& result)
    {
//##protect##"instance::Timer::start()"
        SF_UNUSED1(result);

        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        if (pCoreTimer)
        {
            if (pCoreTimer->IsActive())
                return; // already active? do nothing!
            CurrentCount = (SInt32)pCoreTimer->GetCurrentCount();
            pCoreTimer = NULL;
        }
        pCoreTimer = *SF_HEAP_NEW(GetVM().GetMemoryHeap()) 
            IntervalTimer(this, (unsigned)Delay, CurrentCount, RepeatCount);

        proot->AddIntervalTimer(pCoreTimer);
        pCoreTimer->Start(proot);

//##protect##"instance::Timer::start()"
    }
    void Timer::stop(const Value& result)
    {
//##protect##"instance::Timer::stop()"
        SF_UNUSED1(result);
        if (pCoreTimer && pCoreTimer->IsActive())
        {
            MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
            proot->ClearIntervalTimer(int(pCoreTimer->GetId()));
        }
        pCoreTimer = NULL;
//##protect##"instance::Timer::stop()"
    }

//##protect##"instance$methods"
    void Timer::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc < 1)
        {
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("Timer::AS3Constructor") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));
        }
        argv[0].Convert2Number(Delay).DoNotCheck();
        if (argc >= 2)
        {
            argv[1].Convert2Int32(RepeatCount).DoNotCheck();
        }
    }

    void Timer::ExecuteEvent()
    {
        SF_ASSERT(pCoreTimer);

        CurrentCount = (SInt32)pCoreTimer->GetCurrentCount();

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<fl_events::TimerEvent> efe = CreateTimerEventObject(asvm.GetBuiltin(AS3Builtin_timer));
        efe->Target  = this;
        DispatchSingleEvent(efe, false);

        //!AB consider efe->IsDefaultPrevented() ?

        if (CurrentCount >= RepeatCount)
        {
            // dispatch TIMER_COMPLETE
            SPtr<fl_events::TimerEvent> tefe = CreateTimerEventObject(asvm.GetBuiltin(AS3Builtin_timerComplete));
            tefe->Target  = this;
            DispatchSingleEvent(tefe, false);
        }
    }

    SPtr<Instances::fl_events::TimerEvent> Timer::CreateTimerEventObject(const ASString& type)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<Instances::fl_events::TimerEvent> evt;
        Value params[] = { Value(type), Value(false), Value(false) };
        asvm.ConstructInstance(evt, asvm.TimerEventClass, 3, params);
        SF_ASSERT(evt);
        return evt;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    // const UInt16 Timer::tito[Timer::ThunkInfoNum] = {
    //    0, 1, 2, 4, 5, 7, 8, 9, 10, 
    // };
    const TypeInfo* Timer::tit[11] = {
        &AS3::fl::int_TI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, 
        NULL, 
        NULL, 
    };
    const ThunkInfo Timer::ti[Timer::ThunkInfoNum] = {
        {TFunc_Instances_Timer_currentCountGet::Func, &Timer::tit[0], "currentCount", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_delayGet::Func, &Timer::tit[1], "delay", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_delaySet::Func, &Timer::tit[2], "delay", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Timer_repeatCountGet::Func, &Timer::tit[4], "repeatCount", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_repeatCountSet::Func, &Timer::tit[5], "repeatCount", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Timer_runningGet::Func, &Timer::tit[7], "running", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_reset::Func, &Timer::tit[8], "reset", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_start::Func, &Timer::tit[9], "start", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Timer_stop::Func, &Timer::tit[10], "stop", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Timer::Timer(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::Timer::Timer()"
//##protect##"InstanceTraits::Timer::Timer()"

    }

    void Timer::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Timer&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    Timer::Timer(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Timer::Timer()"
//##protect##"ClassTraits::Timer::Timer()"

    }

    Pickable<Traits> Timer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Timer(vm, AS3::fl_utils::TimerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::TimerCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_utils
{
    const TypeInfo TimerTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_utils::Timer::InstanceType),
        0,
        0,
        InstanceTraits::fl_utils::Timer::ThunkInfoNum,
        0,
        "Timer", "flash.utils", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo TimerCI = {
        &TimerTI,
        ClassTraits::fl_utils::Timer::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_utils::Timer::ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

