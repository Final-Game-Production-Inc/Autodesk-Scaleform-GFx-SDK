/**************************************************************************

Filename    :   AS3_IntervalTimer.cpp
Content     :   
Created     :   Feb, 2010
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_IntervalTimer.h"

namespace Scaleform { namespace GFx { namespace AS3 {

IntervalTimer::IntervalTimer(const Value& function, unsigned delay, bool timeOut) :
    Function(function), CurrentCount(0), RepeatCount(0), Interval(UInt64(delay) * 1000), InvokeTime(0), Id(0), 
    Active(true), Timeout(timeOut)
{
}

IntervalTimer::IntervalTimer
(Instances::fl_utils::Timer* timerObj, unsigned delay, unsigned curCount, unsigned repeatCount) 
:
    TimerObj(timerObj), CurrentCount(curCount), RepeatCount(repeatCount), 
    Interval(UInt64(delay) * 1000), InvokeTime(0), Id(0), Active(true), Timeout(false)
{
}

void IntervalTimer::SetArguments(unsigned num, const Value* argArr)
{
    for (unsigned i = 0; i < num; ++i)
    {
        Params.PushBack(argArr[i]);
    }
}

void IntervalTimer::Start(MovieImpl* proot)
{
    SF_ASSERT(proot);

    UInt64 startTime = proot->GetTimeElapsedMks();
    InvokeTime = startTime + Interval;
}

bool IntervalTimer::Invoke(MovieImpl* proot, float frameTime)
{
    SF_AMP_SCOPE_TIMER(proot->AdvanceStats, "IntervalTimer::Invoke", Amp_Profile_Level_Medium);

    SF_ASSERT(proot);
    if (!Active) return false;
    MovieRoot* asroot = ToAS3Root(proot);

    UInt64 currentTime  = proot->GetTimeElapsedMks();
    bool retval         = false;
    if (currentTime >= InvokeTime)
    {
        if (TimerObj)
        {
            // invoke Timer's listeners
            if (RepeatCount == 0 || CurrentCount < RepeatCount)
            {
                ++CurrentCount;
                TimerObj->ExecuteEvent();
            }
        }
        else
        {
            // invoke a function
            Value _this;
            Value result;
            asroot->GetAVM()->Execute_IgnoreExceptionUnsafe(
                Function, _this, result, static_cast<unsigned>(Params.GetSize()), Params.GetDataPtr());            
        }

        if (Timeout || (RepeatCount != 0 && CurrentCount >= RepeatCount))
        {
            // clear the timeout timer
            Active = false;
        }
        else
        {
            // set new invoke time
            UInt32 interval = GetNextInterval(currentTime, UInt64(frameTime * 1000000.f));
            if (interval > 0)
                InvokeTime += interval;
            else
                InvokeTime = currentTime; //?
        }
        retval = true;
    }
    return retval;
}

UInt32 IntervalTimer::GetNextInterval(UInt64 currentTime, UInt64 frameTime) const
{
    if (RepeatCount != 0 && CurrentCount >= RepeatCount)
        return 0;

    UInt32 interval; 
    if (10 * Interval < frameTime) // make sure to have not more than 10 calls a frame
        interval = UInt32(frameTime)/10;
    else
        interval = Interval;
    if (interval == 0)
        return 0;
    return ((UInt32(currentTime - InvokeTime) + interval)/interval)*interval;
}

void IntervalTimer::Clear()
{
    Active   = false;
    TimerObj = NULL; // remove circular reference
}

bool IntervalTimer::ClearFor(MovieImpl* proot, MovieDefImpl* defimpl)
{
    if (!Function.IsNull())
    {
        VMAbcFile* abc = ToAS3Root(proot)->GetAVM()->GetValueTraits(Function).GetFilePtr();
        if (abc)
        {
            const ASVM::AbcFileWithMovieDef& abcF = static_cast<const ASVM::AbcFileWithMovieDef&>(abc->GetAbcFile());
            if (abcF.pDefImpl == defimpl)
            {
                Clear();
                return true;
            }
        }
    }
    if (TimerObj)
    {
        VMAbcFile* abc = TimerObj->GetInstanceTraits().GetFilePtr();
        if (abc)
        {
            const ASVM::AbcFileWithMovieDef& abcF = static_cast<const ASVM::AbcFileWithMovieDef&>(abc->GetAbcFile());
            if (abcF.pDefImpl == defimpl)
            {
                Clear();
                return true;
            }
        }
    }
    return false;
}

}}} //SF::GFx::AS3
