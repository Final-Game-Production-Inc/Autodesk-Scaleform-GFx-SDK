/**************************************************************************

Filename    :   Video_CriMvSystemTimer.h
Content     :   Video system timer
Created     :   June 4, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H
#define INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_xpt.h>
#include <cri_error.h>
#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_RefCount.h"
#include "Video/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class CriTimer
{
public:
    CriTimer();

    void Start(void);
    void Stop(void);
    void GetTime(UInt64 &count, UInt64 &unit);

    UInt64 StartTicks;
    UInt64 StopTicks;
    bool   IsRunning;
};

//////////////////////////////////////////////////////////////////////////
//

class SystemTimerSyncObject : public VideoPlayer::SyncObject
{
public:
    SystemTimerSyncObject();
    ~SystemTimerSyncObject();

    virtual void SetStartFrame(unsigned) {}
    virtual void Start(void);
    virtual void Stop(void);
    virtual void Pause(bool sw);
    virtual void GetTime(UInt64 *count, UInt64 *unit);

private:
    CriTimer    smptim;
    bool        pause_flag;

    UInt64      time_count;
    UInt64      time_unit;
    UInt64      total_count;
};

//////////////////////////////////////////////////////////////////////////
//

class CriMvSystemTimer : public RefCountBaseNTS<CriMvSystemTimer,Stat_Video_Mem>,
                         public CriMvSystemTimerInterface
{
public:
    CriMvSystemTimer(VideoPlayer::SyncObject* psyncObj) { pSyncObj = psyncObj; }
    ~CriMvSystemTimer() {};

    void SetSyncObject(VideoPlayer::SyncObject* psyncObj) { pSyncObj = psyncObj; }
    VideoPlayer::SyncObject* GetSyncObject() const { return pSyncObj; }

    virtual void Start() { if (pSyncObj) pSyncObj->Start(); }
    virtual void Stop()  { if (pSyncObj) pSyncObj->Stop(); }
    virtual void Pause(Bool sw) { if (pSyncObj) pSyncObj->Pause(sw == CRI_TRUE); }
    virtual void GetTime(Uint64 &count, Uint64 &unit) 
    { 
        if (pSyncObj) pSyncObj->GetTime((UInt64*)&count, (UInt64*)&unit);
        else { count = 0; unit = 1000; }
    }

    Ptr<VideoPlayer::SyncObject> pSyncObj;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_CRIMVSYSTEMTIMER_H
