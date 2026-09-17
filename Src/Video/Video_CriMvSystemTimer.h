/**************************************************************************

Filename    :   Video_CriMvSystemTimer.h
Content     :   Video system timer (LEGACY STUB - CRI Mana has built-in timer modes)
Created     :   June 4, 2008
Changed    : March 10, 2026(MonstroFil, Claude),
                    April 11, 2026(FINALZ-27980) 
                    
Authors     :   Maxim Didenko, Vladislav Merker (2008.6.4) [is cri movie 3.50]
                    MonstroFil, Claude(2026.3) [Changed To Cri Mana 2019 From Criware SDK 2.19.03, From Github Author]
                    FINALZ-27980 [To Integration, Add H.264 Support For Criware SDK 2019]

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

TIPS          :   THE TYPICAL REASON OF THIS PROBLEM IS:
					When Open A New Sample Like GFL2 Exilium .usm File, It Runs Extremely Slowly(Threads Problem)
					Cuz CRIWARE 2019 SDK Embbed And Support H.264 Encoded (Reference To sf_video_4.6.33_windows_msvc11_lib_4K-ready),
                    USE Sofdec2 Viewer Open It Fluently, But Scaleform Video Player Is Extremely Slow To Open It,
                    And When Open More H.264 Files, The GFxPlayer Is Crashed.

                    Should Be Fix It Is Requirments.

                    This Criware SDK Will Be Stop Update For Scaleform 5.0 (After 2030) Stable Version, And More Support To FFMpeg,
                    FINAL VIDEO(FINAL ENGINE), A SelectaVision HDRVIDEOS Persensation(.264-.268 File Format), And More Support Video As Everyone Seen.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H
#define INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

// CRI Mana 2019 has built-in timer modes (SYSTEM, AUDIO, MANUAL).
// The CriMvSystemTimerInterface wrapper is no longer needed.
// This header is kept for backward compatibility.

#include "Kernel/SF_RefCount.h"
#include "Video/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// SystemTimerSyncObject is still used by VideoPlayerImpl for external sync

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

#endif // INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H
