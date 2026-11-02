/**************************************************************************

Filename    :   Video_ATVMvSystemTimer.cpp
Content     :   Video system timer (ATV / Analog TV FFmpeg backend)
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Timer.h"
#include "Video/FFMpeg/ATVBase/Video_ATVMvSystemTimer.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// SystemTimerSyncObject — used for external synchronization with Mana's
// MANUAL timer mode when a SyncObject is provided by the application.

void SystemTimerSyncObject::Start(void)
{
    time_count = 0;
    time_unit = Timer::GetRawFrequency();
    total_count = 0;
    pause_flag = false;
    IsRunning = true;
    StartTicks = Timer::GetRawTicks();
}

void SystemTimerSyncObject::Stop(void)
{
    StopTicks = Timer::GetRawTicks();
    IsRunning = false;
    total_count = 0;
}

void SystemTimerSyncObject::Pause(bool sw)
{
    if (pause_flag == sw)
        return;

    if (sw)
    {
        StopTicks = Timer::GetRawTicks();
        IsRunning = false;
        time_count = StopTicks - StartTicks;
        total_count += time_count;
    }
    else
    {
        IsRunning = true;
        StartTicks = Timer::GetRawTicks();
    }
    pause_flag = sw;
}

void SystemTimerSyncObject::GetTime(UInt64 *count, UInt64 *unit)
{
    *unit = Timer::GetRawFrequency();

    if (pause_flag)
    {
        *count = total_count;
    }
    else
    {
        if (IsRunning)
        {
            UInt64 ccnt = Timer::GetRawTicks();
            *count = (ccnt - StartTicks) + total_count;
        }
        else
        {
            *count = total_count;
        }
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
