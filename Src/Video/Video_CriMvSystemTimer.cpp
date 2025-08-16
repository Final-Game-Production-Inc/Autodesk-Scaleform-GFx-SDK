/**************************************************************************

Filename    :   Video_CriMvSystemTimer.cpp
Content     :   Video system timer
Created     :   June 4, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#ifdef SF_OS_ORBIS
# include <kernel.h>
#endif

#include "Kernel/SF_Timer.h"
#include "Kernel/SF_HeapNew.h"

#include "Video/Video_CriMvSystemTimer.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//


// Avoiding modfication to the header/class layout until done for other platforms
#ifdef SF_OS_ORBIS
static Uint64 GetRawFrequency()
{
	static Uint64 Freq = ::sceKernelGetTscFrequency();
	return Freq;
}

static Uint64 GetRawTicks()
{
	return ::sceKernelReadTsc();
}
#endif // SF_OS_ORBIS

CriTimer::CriTimer(void)
{
    Start();
}

void CriTimer::Start(void) 
{
#ifdef SF_OS_ORBIS
	StartTicks = GetRawTicks();
#else
    StartTicks = Timer::GetRawTicks();
#endif
    IsRunning  = true;
}

void CriTimer::Stop(void) 
{
#ifdef SF_OS_ORBIS
	StopTicks = GetRawTicks();
#else
    StopTicks = Timer::GetRawTicks();
#endif
    IsRunning = false;
}

void CriTimer::GetTime(Uint64 &count, Uint64 &unit)
{
#ifdef SF_OS_ORBIS
	static uint64_t freq = GetRawFrequency();
	unit  = freq;
#else
    unit = Timer::GetRawFrequency();;
#endif

    if (IsRunning) 
    {
#ifdef SF_OS_ORBIS
		UInt64 ccnt = GetRawTicks();
#else
		UInt64 ccnt = Timer::GetRawTicks();
#endif
        count = (Uint64)(ccnt - StartTicks);
    } 
    else 
        count = (Uint64)(StopTicks - StartTicks);

    return;
}

//////////////////////////////////////////////////////////////////////////
//

SystemTimerSyncObject::SystemTimerSyncObject() :
    pause_flag(false), time_count(0), time_unit(1000000), total_count(0)
{
}

SystemTimerSyncObject::~SystemTimerSyncObject()
{
}

void SystemTimerSyncObject::Start(void)
{
    time_count = 0;

#ifdef SF_OS_ORBIS
	static uint64_t freq = GetRawFrequency();
	time_unit = freq;
#else
    time_unit = Timer::GetRawFrequency();
#endif

    total_count = 0;
    pause_flag = false;
    smptim.Start();
}

void SystemTimerSyncObject::Stop(void)
{
    smptim.Stop();
    total_count = 0;
}

void SystemTimerSyncObject::Pause(bool sw)
{
    if (pause_flag == sw)
        return;

    if (sw) 
    {
        smptim.Stop();
        smptim.GetTime(time_count, time_unit);
        total_count += time_count;
    }
    else {
        smptim.Start();
    }
    pause_flag = sw;
}


void SystemTimerSyncObject::GetTime(UInt64 *count, UInt64 *unit)
{
    if (pause_flag) 
    {
        *count = total_count;
        *unit = time_unit;
    } 
    else 
    {
        smptim.GetTime(time_count, time_unit);
        *count = time_count + total_count;
        *unit = time_unit;
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
