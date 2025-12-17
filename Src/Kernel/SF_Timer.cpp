/**************************************************************************

Filename    :   SF_Timer.cpp
Content     :   Provides static functions for precise timing
Created     :   June 28, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Timer.h"

#if defined (SF_OS_WIN32)
#include <windows.h>

#elif defined(SF_OS_XBOX360)
#include <xtl.h>

#elif defined(SF_OS_PS3)
#include <sys/sys_time.h>

#elif defined(SF_OS_WII)
#include <revolution/os.h>

#elif defined(SF_OS_PSVITA)
#include <time.h>
#include <rtc.h>

#elif defined(SF_OS_3DS)
#include <nn/fnd.h>
#include <nn/os.h>

#elif defined(SF_OS_WIIU)

#else
#include <sys/time.h>
#endif

namespace Scaleform {

//------------------------------------------------------------------------
static Timer::TimerOverride* TimerOverrideInstance = 0;

//------------------------------------------------------------------------
// ***** Timer Class

UInt64 Timer::GetProfileTicks()
{
    return (GetRawTicks() * MksPerSecond) / GetRawFrequency();
}
Double Timer::GetProfileSeconds()
{
    static UInt64 StartTime = GetProfileTicks();
    return TicksToSeconds(GetProfileTicks()-StartTime);
}


//------------------------------------------------------------------------
// *** Win32, Xbox, Xbox360 Specific Timer

#if defined (SF_OS_WINMETRO)

UInt64 Timer::GetTicks()
{
	return GetTickCount64()*1000;
}

UInt32 Timer::GetTicksMs()
{
    return (UInt32)GetTickCount64();
}

void Timer::initializeTimerSystem()
{
}
void Timer::shutdownTimerSystem()
{
}

#elif (defined (SF_OS_WIN32) || defined (SF_OS_XBOX) || defined (SF_OS_XBOX360) || defined(SF_OS_WINCE))

CRITICAL_SECTION WinAPI_GetTimeCS;
volatile UInt32  WinAPI_OldTime = 0;
volatile UInt32  WinAPI_WrapCounter = 0;

UInt32 Timer::GetTicksMs()
{

    UInt32 ticks;
#if defined (SF_OS_XBOX360) 
    ticks = GetTickCount();
#else
    ticks = timeGetTime();
#endif

    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetTicksMs(ticks);
    return ticks;
}

UInt64 Timer::GetTicks()
{
#if defined (SF_OS_XBOX360) 
    return GetProfileTicks();
#else
    
    UInt64 result;

    // On Win32 QueryPerformanceFrequency is unreliable due to SMP and
    // performance levels, so use this logic to detect wrapping and track
    // high bits.
    ::EnterCriticalSection(&WinAPI_GetTimeCS);

    // The ticks must be measured in the critical section otherwise 
    // if a second thread reads it right after the first one and the second one accesses the section before the first one
    // hence the second thread will overwrite WinAPI_OldTime with a value greater than the measure of the first thread
    // therefore the first thread will consider WinAPI_OldTime being greater than its measured ticks, and will increase WinAPI_WrapCounter
    // creating a difference of 49days instead of -1ms.
    DWORD  ticks = GetTicksMs();

    if (WinAPI_OldTime > ticks)
        WinAPI_WrapCounter++;
    WinAPI_OldTime = ticks;

    result = (UInt64(WinAPI_WrapCounter) << 32) | ticks;
    ::LeaveCriticalSection(&WinAPI_GetTimeCS);

    return result * MksPerMs;
#endif    
}


void Timer::initializeTimerSystem()
{
#if defined (SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
    timeBeginPeriod(1);
#endif
#if defined(SF_OS_WINMETRO)
    InitializeCriticalSectionEx(&WinAPI_GetTimeCS);
#else
    InitializeCriticalSection(&WinAPI_GetTimeCS);
#endif
}
void Timer::shutdownTimerSystem()
{
    DeleteCriticalSection(&WinAPI_GetTimeCS);
#if defined (SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
    timeEndPeriod(1);
#endif
}

#endif

#if (defined (SF_OS_WIN32) || defined (SF_OS_XBOX) || defined (SF_OS_XBOX360) || defined(SF_OS_WINCE))

UInt64 Timer::GetRawTicks()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(li.QuadPart);
    return li.QuadPart;
}

UInt64 Timer::GetRawFrequency()
{
    static UInt64 perfFreq = 0;
    if (perfFreq == 0)
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        perfFreq = freq.QuadPart;
    }

    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(perfFreq);
    return perfFreq;
}

#else   // !SF_OS_WIN32


//------------------------------------------------------------------------
// *** Other OS Specific Timer     

UInt32 Timer::GetTicksMs()
{
    return (UInt32)(GetProfileTicks() / 1000);
}
// The profile ticks implementation is just fine for a normal timer.
UInt64 Timer::GetTicks()
{
    return GetProfileTicks();
}

void Timer::initializeTimerSystem()
{
}
void Timer::shutdownTimerSystem()
{
}


// System specific GetProfileTicks

#if defined(SF_OS_PS3)

UInt64 Timer::GetRawTicks()
{
    // read time
    UInt64 ticks;
#ifdef SF_CC_SNC
    ticks = __mftb();
#else
    asm volatile ("mftb %0" : "=r"(ticks));
#endif
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(ticks);
    return ticks;
}

static UInt64 iPerfFreq = 0;

UInt64 Timer::GetRawFrequency()
{

    if (iPerfFreq == 0)
    {
        iPerfFreq = sys_time_get_timebase_frequency();
    }
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(iPerfFreq);
    return iPerfFreq;
}

#elif defined(SF_OS_WII)

UInt64 Timer::GetRawTicks()
{
    UInt64 ret = OSTicksToMicroseconds(OSGetTime());
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(ret);
    return ret;
}

UInt64 Timer::GetRawFrequency()
{
    UInt64 ret = MksPerSecond;
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(ret);
    return ret;
}

#elif defined(SF_OS_WIIU)

UInt64 Timer::GetRawTicks()
{
    UInt64 ret = OSGetTime();
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(ret);
    return ret;
}

static UInt64 iPerfFreq = 0;

UInt64 Timer::GetRawFrequency()
{
    if (iPerfFreq == 0)
    {
        iPerfFreq = OS_TIMER_CLOCK;
    }
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(iPerfFreq);
    return iPerfFreq;
}

#elif defined(SF_OS_PSVITA)

UInt64 Timer::GetRawTicks()
{
    SceRtcTick ticks;
    sceRtcGetCurrentTick(&ticks);

    UInt64 ret = static_cast<UInt64>(ticks.tick);
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(ret);
    return ret;
}

UInt64 Timer::GetRawFrequency()
{
    UInt64 ret = MksPerSecond;
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(ret);
    return ret;
}

#elif defined(SF_OS_3DS)

UInt64 Timer::GetRawTicks()
{
    UInt64 ret = nn::fnd::TimeSpan(nn::os::Tick::GetSystemCurrent()).GetMicroSeconds();
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(ret);
    return ret;
}

UInt64 Timer::GetRawFrequency()
{
    UInt64 ret = MksPerSecond;
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(ret);
    return ret;
}

// Standard or Other OS 
#else

UInt64  Timer::GetRawTicks()
{
    // TODO: prefer rdtsc when available?

    // Return microseconds.
    struct timeval tv;
    UInt64 result;

    gettimeofday(&tv, 0);

    result = (UInt64)tv.tv_sec * 1000000;
    result += tv.tv_usec;

    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawTicks(result);
    return result;
}

UInt64 Timer::GetRawFrequency()
{
    UInt64 ret = MksPerSecond;
    if (TimerOverrideInstance)
        return TimerOverrideInstance->GetRawFrequency(ret);
    return ret;
}

#endif // GetProfileTicks


#endif  // !SF_OS_WIN32

void Timer::SetTimerOverride(Timer::TimerOverride* instance)
{
    TimerOverrideInstance = instance;
}

Timer::TimerOverride* Timer::GetTimerOverride()
{
    return TimerOverrideInstance;
}

} // Scaleform

