/**************************************************************************

Filename    :   Video_ATVMvSystemTimer.h
Content     :   Video system timer for the ATV (Analog TV) FFmpeg backend
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_ATVMVSYSTEMTIMER_H
#define INC_GFX_VIDEO_ATVMVSYSTEMTIMER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_RefCount.h"
#include "Video/Common/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// SystemTimerSyncObject
//
// Wall-clock SyncObject used for external synchronization when the
// application supplies a SyncObject to the ATV player
// (VideoPlayer::SetSyncObject). It accumulates elapsed raw timer ticks
// and handles Start/Stop/Pause, matching the Sofdec2 backend timer so
// that both backends behave identically to the engine.

class SystemTimerSyncObject : public VideoPlayer::SyncObject
{
public:
    SystemTimerSyncObject() : pause_flag(false), time_count(0), time_unit(1000000), total_count(0) {}
    ~SystemTimerSyncObject() {}

    virtual void SetStartFrame(unsigned) {}
    virtual void Start(void);
    virtual void Stop(void);
    virtual void Pause(bool sw);
    virtual void GetTime(UInt64 *count, UInt64 *unit);

private:
    bool        pause_flag;
    UInt64      time_count;
    UInt64      time_unit;
    UInt64      total_count;
    UInt64      StartTicks;
    UInt64      StopTicks;
    bool        IsRunning;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_ATVMVSYSTEMTIMER_H
