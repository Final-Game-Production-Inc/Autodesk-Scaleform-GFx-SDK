/**************************************************************************

Filename    :   Video_VideoPlayerImpl.h
Content     :   GFx video player implementation
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_INC_GFX_VIDEO_PLAYERIMPL_H
#define INC_INC_GFX_VIDEO_PLAYERIMPL_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_CriHeap.h"
#include "Video/Video_CriMvSystemTimer.h"
#include "Video/Video_CriMvFileReader.h"
#include "Video/Video_CriMvSoundGateway.h"

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Hash.h"

#include "GFx/GFx_TaskManager.h"
#include "Video/Video_Video.h"
#include "Video/Video_VideoImage.h"

#ifdef SF_OS_ORBIS
#include <cri_movie_ps4.h>
#include "Video/Video_VideoPS4.h"
#endif

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
    virtual void      SetExtraAudioTrack(int trackIndex);
    virtual void      ReplaceCenterVoice(int trackIndex);

    virtual void      SetLoopFlag(bool flag);

    virtual void      SetBufferTime(float time);
    virtual void      SetNumberOfFramePools(unsigned pools);
    virtual void      SetReloadThresholdTime(float time);
    virtual void      GetReadBufferInfo(ReadBufferInfo* info);

    virtual void      SetDecodeHeaderTimeout(float timeout);

    CriMvEasyPlayer*  GetCriPlayer() { return pCriPlayer; }

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
    CriMvEasyPlayer*          pCriPlayer;
    volatile Status           Stat;
    Ptr<CriMvSystemTimer>     pTimer;
    Ptr<VideoReader>          pReader;
    Ptr<CriMvSound>           pSound;
    Ptr<CriMvSound>           pSubSound;
    Ptr<CriMvSound>           pExtraSound;

    CriMvStreamingParameters  StreamParams;
    CriMvFrameInfo            FrameInfo;

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
    bool           UseAudioTimer;

#ifdef GFX_VIDEO_DIAGS
    CriMvLastFrameResult      LastFrameResult;
    CriMvPlaybackInfo         PlaybackInfo;
    UInt64         LastDecodeTicks;
    UInt64         LastUpdateTextureTicks;
#endif

    void WaitForFinish();
    void CheckHeaderDecoding();

    Ptr<VideoDecoder>         pDecoder;
    MemoryHeap*               pGFxHeap;
    CriHeap                   Heap;
    Ptr<Log>                  pLog;

#ifdef SF_OS_ORBIS
    // Work buffer for H.264 decoder 
    VideoPS4::OnionMemoryInfo   H264MemoryInfo;
#endif
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif //INC_INC_GFX_VIDEO_PLAYERIMPL_H
