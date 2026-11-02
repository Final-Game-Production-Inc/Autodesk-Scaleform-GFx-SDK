/**************************************************************************

Filename    :   Video_ATVVideoPlayerImpl.h
Content     :   GFx video player implementation (FFmpeg / libav)
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

TIPS        :   THE FFMpeg BACKEND IS THE COMPANION IMPLEMENTATION OF THE
                Sofdec2 (CRI Mana 2019) BACKEND. IT PROVIDES THE SAME
                VideoPlayer INTERFACE, DECODING THE FILE WITH THE FFMPEG 8
                (libavcodec/libavformat/libswscale/libswresample) LIBRARIES
                INSTALLED IN 3rdParty/FFMpeg. DECODING IS SINGLE-THREADED
                AND DRIVEN BY THE ENGINE'S Decode() CALLS (ATVVideoDecoderSmp),
                WHICH KEEPS THE INTEGRATION DETERMINISTIC AND FREE OF
                CROSS-THREAD FRAME HANDOFF.

**************************************************************************/

#ifndef INC_GFX_ATVVIDEO_PLAYERIMPL_H
#define INC_GFX_ATVVIDEO_PLAYERIMPL_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_File.h"

#include "GFx/GFx_TaskManager.h"
#include "Video/Common/Video_Video.h"
#include "Video/FFMpeg/ATVBase/Video_ATVideoImage.h"
#include "Video/FFMpeg/ATVBase/Video_ATVMvFileReader.h"

// Forward declarations of the FFmpeg opaque types; the FFmpeg headers are
// only pulled into the implementation file.
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;
struct AVIOContext;

namespace Scaleform { namespace GFx { namespace Video {

class ATVVideoPlayerImpl;
class ATVMvSoundGateway;

//////////////////////////////////////////////////////////////////////////
//

// Single-threaded decoding queue. The engine drives ATVVideoPlayerImpl::Decode()
// from the GFx task thread; ExecuteDecode runs one decode step per player.
class ATVVideoDecoderSmp : public VideoDecoder
{
public:
    ATVVideoDecoderSmp();
    ~ATVVideoDecoderSmp();

    virtual void StartDecoding(VideoPlayer*);
    virtual void StopDecoding(VideoPlayer*);
    virtual bool IsDecodingStopped(VideoPlayer*);

    virtual bool ExecuteDecode(VideoPlayer*);

    virtual void GetReadBufferInfo(Array<VideoPlayer::ReadBufferInfo>&);
    virtual bool IsIORequired() const;

    virtual void PauseDecoding(bool);

protected:
    bool                    DecodingQueuePaused;
    Array<ATVVideoPlayerImpl*> DecodingQueue;
    Array<ATVVideoPlayerImpl*> NotActive;

    int          FindPlayer(Array<ATVVideoPlayerImpl*>&, ATVVideoPlayerImpl*);
};

//////////////////////////////////////////////////////////////////////////
//

class ATVVideoPlayerImpl : public VideoPlayer
{
    friend class ATVVideoImage;

protected:
    virtual ~ATVVideoPlayerImpl();

public:
    ATVVideoPlayerImpl(MemoryHeap* pheap);

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

    virtual ATVVideoImage* CreateTexture(Render::TextureManager* ptexman);
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

    // ATVVideoImage / decoder access
    bool              IsAlphaVideo() { return VideoAlpha; }
    bool              IsPaused()     { return Paused; }
    bool              CopyFrameData(ImageData* pdest);
    bool              ExecuteDecode();
    void              StopInternal() { StopRequested = true; }

    // State flags shared with ATVVideoImage (mirrors the Sofdec2 integration)
    bool              UpdateTextureCalled;
    bool              FrameOnTime;
    bool              FrameNotInitialized;
    UInt32            LastFrame;

#ifdef GFX_ENABLE_SOUND
    virtual Sound::SoundChannel*
                      GetSoundChannel(SoundTrack track = MainTrack);
    virtual void      SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[]);
#endif

private:
    Video*                    pVideo;
    volatile Status           Stat;
    Status                    StatusAfterStop;

    // FFmpeg handles
    AVFormatContext*          pFormatCtx;
    AVCodecContext*           pVideoCodecCtx;
    AVCodecContext*           pSubtitleCodecCtx;
    int                       VideoStreamIndex;
    int                       AudioStreamIndex;
    int                       SubtitleStreamIndex;
    Array<int>                AudioStreams;
    Array<unsigned>           AudioStreamChannels;
    Array<unsigned>           AudioStreamRates;
    unsigned                  SelectedAudioTrack;

    // FileOpener-backed custom IO (avio). Falls back to plain file I/O
    // through libavformat when the FileOpener cannot open the resource.
    Ptr<FileOpenerBase>       pFileOpener;
    Ptr<File>                 pFile;
    AVIOContext*              pAVIO;
    UByte*                    pAVIOBuffer;

    SwsContext*               pSwsCtx;
    SwsContext*               pArgbSws;

    AVFrame*                  pFrame;
    AVFrame*                  pYuvFrame;
    AVPacket*                 pPacket;
    UByte*                    pYuvBuffer;
    UByte*                    pArgbScratch;

    // Media info
    int                       Width;
    int                       Height;
    unsigned                  FrameRateNum;
    unsigned                  FrameRateDen;
    unsigned                  VideoTimeBaseNum;
    unsigned                  VideoTimeBaseDen;
    bool                      VideoAlpha;
    unsigned                  AudioChannels;
    unsigned                  AudioSampleRate;
    UInt32                    TotalFrames;
    int                       MaxSubtitleLen;
    UInt64                    DecodeHeaderTimeout;

    // Playback state
    UInt32                    StartPos;
    UInt32                    SeekToFrame;
    bool                      SeekPending;
    bool                      Paused;
    bool                      LoopFlag;
    bool                      StopRequested;
    bool                      DecodingEOF;
    bool                      FrameReady;
    UInt32                    StagingFrameIndex;
    UInt64                    FrameIntervalUs;
    int                       CurrSubtitleChannel;
    UInt64                    StartOpenTicks;
    UInt64                    PlayStartTicks;
    UInt64                    PauseStartTicks;
    UInt64                    LastFrameTicks;

    // Compressed audio packets demuxed ahead of the native sound gateway.
    // Bounds the read-ahead so seeking/looping stays responsive.
    enum { MaxPendingVideoPackets = 48 };
    Array<AVPacket*>          PendingVideoPackets;

    String                    SubtitleText;

    Lock                      FrameLock;

    Ptr<SyncObject>           pSyncObj;
    Ptr<VideoDecoder>         pDecoder;
    ATVMvSoundGateway*        pSound;
    MemoryHeap*               pGFxHeap;
    Ptr<Log>                  pLog;

    // Helpers
    bool    OpenFFmpeg(const char* url);
    void    CloseFFmpeg();
    void    CloseAVIO();
    bool    SetupAudioStream(int streamIndex);
    void    SetupSubtitleStream(int streamIndex);
    void    BuildAudioStreamList();
    void    FlushDecoders();
    bool    FeedVideoPacket();
    bool    PumpPackets();
    void    ClearPendingVideoPackets();
    void    OnVideoFrameDecoded(AVFrame* pframe);
    void    DecodeSubtitlePacket(AVPacket* pkt);
    void    DoSeek(UInt32 pos);
    void    RestartFromBeginning();
    void    WaitForFinish();
    void    CheckHeaderDecoding();
    void    GetPlayTime(UInt64* count, UInt64* unit);
    bool    IsFrameOnTime(UInt32 frameIndex);
    UInt32  FrameIndexFromPts(int64_t pts) const;
    int64_t RescaleFrameToPts(UInt32 pos) const;
    void    CopyPlaneData(const ImagePlane& plane, const UByte* src, int srcPitch) const;
    void    CopyPlaneDataARGB(const ImagePlane& argb);

    // AVIO callbacks (see Video_ATVMvFileReader.h)
    static int     AVRead(void* opaque, UByte* buf, int size);
    static int64_t AVSeek(void* opaque, int64_t offset, int whence);
};

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_FFMPEG_PLAYERIMPL_H
