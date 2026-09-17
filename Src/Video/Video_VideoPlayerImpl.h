/**************************************************************************

Filename    :   Video_VideoPlayerImpl.h
Content     :   GFx video player implementation (CRI Mana 2019)
Created     :   October 2008
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

#ifndef INC_INC_GFX_VIDEO_PLAYERIMPL_H
#define INC_INC_GFX_VIDEO_PLAYERIMPL_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_mana.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Hash.h"

#include "GFx/GFx_TaskManager.h"
#include "Video/Video_Video.h"
#include "Video/Video_VideoImage.h"

namespace Scaleform { namespace GFx { namespace Video {

class VideoPlayerImpl;

//////////////////////////////////////////////////////////////////////////
//

class VideoDecoderSmp : public VideoDecoder
{
public:
    VideoDecoderSmp();
    ~VideoDecoderSmp();

    virtual void StartDecoding(VideoPlayer*);
    virtual void StopDecoding(VideoPlayer*);
    virtual bool IsDecodingStopped(VideoPlayer*);

    virtual bool ExecuteDecode(VideoPlayer*);

    virtual void GetReadBufferInfo(Array<VideoPlayer::ReadBufferInfo>&);
    virtual bool IsIORequired() const;

    virtual void PauseDecoding(bool);

protected:
    bool                    DecodingQueuePaused;
    Array<VideoPlayerImpl*> DecodingQueue;
    Array<VideoPlayerImpl*> NotActive;

    int          FindPlayer(Array<VideoPlayerImpl*>&, VideoPlayerImpl*);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoDecoderThrd : public VideoDecoderSmp
{
public:
    VideoDecoderThrd(MemoryHeap* pheap, Thread::ThreadPriority prio);
    ~VideoDecoderThrd();

    virtual void StartDecoding(VideoPlayer*);
    virtual void StopDecoding(VideoPlayer*);
    virtual bool IsDecodingStopped(VideoPlayer*);

    virtual bool ExecuteDecode(VideoPlayer*) { return false; }

#ifdef SF_OS_XBOX360
    virtual void SetUsableProcessors(int proc) { ProcNumber = proc; }
#endif

private:
    MemoryHeap*             pHeap;
    Ptr<Thread>             pDecodeThread;
    Thread::ThreadPriority  Priority;
    Lock                    DecodeLock;
#ifdef SF_OS_XBOX360
    int                     ProcNumber;
#endif

    static int   DecodeFunc(Thread* pthread, void* obj);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoPlayerImpl : public VideoPlayer
{
    friend class VideoImage;

protected:
    virtual ~VideoPlayerImpl();

public:
    VideoPlayerImpl(MemoryHeap* pheap);

    bool              Init(Video* pvideo, TaskManager* ptaskManager,
                           FileOpenerBase* pfileOpener, Log* plog);

    virtual void      Open(const char* url);
    virtual void      GetVideoInfo(VideoInfo* info);
    virtual Status    GetStatus() { return Stat; }
    virtual void      Play();
    virtual void      Stop();
    virtual void      Pause(bool on_off);
    virtual void      Seek(UInt32 pos);
    virtual UInt32    GetPosition();

    virtual void      SetSubtitleChannel(int channel);
    virtual int       GetSubtitleChannel();

    virtual VideoImage* CreateTexture(Render::TextureManager* ptexman);
    virtual void      UpdateTexture(VideoImage* pimage, char* subtitle, int subtitleLength);
    virtual bool      IsTextureReady() { return UpdateTextureCalled && FrameOnTime; }
    virtual void      Decode();

    virtual void      SetSyncObject(SyncObject*);

    virtual void      GetCurrentCuePoints(Array<CuePoint>* cuePoints);

    virtual void      SetAudioTrack(int trackIndex);
    virtual void      SetSubAudioTrack(int trackIndex);
    virtual void      ReplaceCenterVoice(int trackIndex);

    virtual void      SetLoopFlag(bool flag);

    virtual void      SetBufferTime(float time);
    virtual void      SetNumberOfFramePools(unsigned pools);
    virtual void      SetReloadThresholdTime(float time);
    virtual void      GetReadBufferInfo(ReadBufferInfo* info);

    virtual void      SetDecodeHeaderTimeout(float timeout);

    CriManaPlayerHn   GetManaPlayer() { return pManaPlayer; }

    void              AddCurrentCuePoint(const CuePoint& cp);
    void              ClearCurrentCuePoints();

    bool              IsAlphaVideo() { return VideoAlpha; }
    bool              IsPaused()     { return Paused != 0; }

#ifdef GFX_ENABLE_SOUND
    virtual Sound::SoundChannel*
                      GetSoundChannel(SoundTrack track = MainTrack);
    virtual void      SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[]);
#endif

private:
    CriManaPlayerHn           pManaPlayer;
    volatile Status           Stat;

    CriManaMovieInfo          MovieInfo;
    CriManaFrameInfo          FrameInfo;

    Array<CuePoint>           CurrentCuePoints;

    unsigned       VideoWidth;
    unsigned       VideoHeight;
    bool           VideoAlpha;

    UInt32         LastFrame;
    UInt32         StartPos;
    Status         StatusAfterStop;
    CriBool        Paused;
    CriBool        LoopFlag;
    bool           UpdateTextureCalled;
    bool           FrameOnTime;
    bool           FrameNotInitialized;
    bool           PausedStartup;

    int            CurrSubtitleChannel;
    UInt64         StartOpenTicks;
    UInt64         DecodeHeaderTimeout;

    Ptr<SyncObject> pSyncObj;

    void WaitForFinish();
    void CheckHeaderDecoding();

    Ptr<VideoDecoder>         pDecoder;
    MemoryHeap*               pGFxHeap;
    Ptr<Log>                  pLog;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif //INC_INC_GFX_VIDEO_PLAYERIMPL_H
