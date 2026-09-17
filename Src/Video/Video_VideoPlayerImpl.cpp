/**************************************************************************

Filename    :   Video_VideoPlayerImpl.cpp
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

#include "Video/Video_VideoPlayerImpl.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Alg.h"

#include "GFx/GFx_Log.h"
#include "Kernel/SF_SysFile.h"

#define THREAD_STACK_SIZE   65536

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

#if defined( SF_OS_ORBIS )

static void DefaultInitVideoDecoderThrd( ScePthread pid, int requestedPriority )
{
	const SceKernelSchedParam schedParam = { requestedPriority };
	{
		ScePthreadAttr attr;
		scePthreadAttrInit( &attr );
		scePthreadAttrGet( pid, &attr );
		scePthreadAttrSetinheritsched( &attr, SCE_PTHREAD_EXPLICIT_SCHED );
		scePthreadSetschedparam( pid, SCE_KERNEL_SCHED_FIFO, &schedParam );
		scePthreadAttrDestroy( &attr );
	}
	scePthreadSetaffinity( pid, (1U<<0)|(1U<<1) );
}

typedef void (*FUNC_INIT_VIDEO_DECODER_THRD)(ScePthread,int);
FUNC_INIT_VIDEO_DECODER_THRD InitVideoDecoderThrd = &DefaultInitVideoDecoderThrd;

#endif

VideoDecoderThrd::VideoDecoderThrd(MemoryHeap* pheap, Thread::ThreadPriority prio) :
    pHeap(pheap), Priority(prio)
#ifdef SF_OS_XBOX360
,   ProcNumber(-1)
#endif
{
}

VideoDecoderThrd::~VideoDecoderThrd()
{
}

void VideoDecoderThrd::StartDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    {
    Lock::Locker lock(&DecodeLock);
    if (FindPlayer(DecodingQueue, pplayer) != -1)
        return;
    DecodingQueue.PushBack(pplayer);
    }
    if (!pDecodeThread)
    {
        Thread::CreateParams params(DecodeFunc, this, THREAD_STACK_SIZE);
        params.priority = Priority;
#ifdef SF_OS_XBOX360
        params.processor = ProcNumber;
#endif
        pDecodeThread = *SF_HEAP_NEW(pHeap) Thread(params);
        if ( pDecodeThread->Start() && pDecodeThread )
		{
            pDecodeThread->SetThreadName("Scaleform Video Decoder");
#if defined( SF_OS_ORBIS )
			InitVideoDecoderThrd( pDecodeThread->GetOSHandle(), Thread::GetOSPriority(Priority) );
#endif
		}
    }
}

void VideoDecoderThrd::StopDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    Lock::Locker lock(&DecodeLock);
    if (FindPlayer(DecodingQueue, pplayer) != -1)
        criManaPlayer_Stop(pplayer->GetManaPlayer());
}

bool VideoDecoderThrd::IsDecodingStopped(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    Lock::Locker lock(&DecodeLock);
    return FindPlayer(DecodingQueue, pplayer) == -1;
}

static bool IsManaPlayerActive(CriManaPlayerHn player)
{
    CriManaPlayerStatus status = criManaPlayer_GetStatus(player);
    return status != CRIMANAPLAYER_STATUS_STOP &&
           status != CRIMANAPLAYER_STATUS_PLAYEND &&
           status != CRIMANAPLAYER_STATUS_ERROR;
}

int VideoDecoderThrd::DecodeFunc(Thread* pthread, void* obj)
{
    SF_UNUSED(pthread);
    VideoDecoderThrd* pdecoder = (VideoDecoderThrd*)obj;
    while (1)
    {
        pdecoder->DecodeLock.DoLock();
        if (pdecoder->DecodingQueue.GetSize() == 1)
        {
            VideoPlayerImpl* pplayer = pdecoder->DecodingQueue[0];
            pdecoder->DecodeLock.Unlock();

            criManaPlayer_ExecuteMain(pplayer->GetManaPlayer());
            if (!IsManaPlayerActive(pplayer->GetManaPlayer()))
            {
                Lock::Locker lock(&pdecoder->DecodeLock);
                pdecoder->DecodingQueue.RemoveAt(0);
                if (pdecoder->DecodingQueue.GetSize() == 0)
                {
                    pdecoder->pDecodeThread = 0;
                    break;
                }
            }
        }
        else
        {
            Array<VideoPlayerImpl*> copy_array(pdecoder->DecodingQueue);
            pdecoder->DecodeLock.Unlock();
            Array<UPInt> remove_indexes;
            for (UPInt i = 0; i < copy_array.GetSize(); ++i)
            {
                criManaPlayer_ExecuteMain(copy_array[i]->GetManaPlayer());
                if (!IsManaPlayerActive(copy_array[i]->GetManaPlayer()))
                    remove_indexes.PushBack(i);
            }
            if (remove_indexes.GetSize() > 0)
            {
                Lock::Locker lock(&pdecoder->DecodeLock);
                for (UPInt j = 0; j < remove_indexes.GetSize(); ++j)
                    pdecoder->DecodingQueue.RemoveAt(remove_indexes[j]-j);
                if (pdecoder->DecodingQueue.GetSize() == 0)
                {
                    pdecoder->pDecodeThread = 0;
                    break;
                }
            }
        }
        Thread::MSleep(1);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//

VideoDecoderSmp::VideoDecoderSmp(): DecodingQueuePaused(false)
{
}

VideoDecoderSmp::~VideoDecoderSmp()
{
    SF_ASSERT(DecodingQueue.GetSize() == 0);
}

int VideoDecoderSmp::FindPlayer(Array<VideoPlayerImpl*>& players, VideoPlayerImpl* pplayer)
{
    for (UPInt i = 0; i < players.GetSize(); ++i)
    {
        if (players[i] == pplayer)
            return (int)i;
    }
    return -1;
}

void VideoDecoderSmp::StartDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    DecodingQueue.PushBack(pplayer);
}

void VideoDecoderSmp::StopDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    if (FindPlayer(DecodingQueue, pplayer) != -1)
    {
        criManaPlayer_Stop(pplayer->GetManaPlayer());
        ExecuteDecode(pp);
    }
}

bool VideoDecoderSmp::IsDecodingStopped(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    return !ExecuteDecode(pp);
}

bool VideoDecoderSmp::ExecuteDecode(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    int idx = FindPlayer(DecodingQueue, pplayer);
    if (idx == -1)
        return false;

    criManaPlayer_ExecuteMain(pplayer->GetManaPlayer());
    if (!IsManaPlayerActive(pplayer->GetManaPlayer()))
    {
        DecodingQueue.RemoveAt(idx);
        return false;
    }
    return true;
}

void VideoDecoderSmp::GetReadBufferInfo(Array<VideoPlayer::ReadBufferInfo>& info)
{
    info.Clear();
    for (UPInt i = 0; i < DecodingQueue.GetSize(); ++i)
    {
        VideoPlayer::ReadBufferInfo buf_info;
        DecodingQueue[i]->GetReadBufferInfo(&buf_info);
        info.PushBack(buf_info);
    }
}

bool VideoDecoderSmp::IsIORequired() const
{
    for (UPInt i = 0; i < DecodingQueue.GetSize(); ++i)
    {
        VideoPlayer::ReadBufferInfo buf_info;
        DecodingQueue[i]->GetReadBufferInfo(&buf_info);
        if (buf_info.DataSize <= buf_info.ReloadThreshold)
            return true;
    }
    return false;
}

void VideoDecoderSmp::PauseDecoding(bool pause)
{
    if(DecodingQueuePaused && pause)
        return;

    for (UPInt i = 0; i < DecodingQueue.GetSize(); ++i)
    {
        VideoPlayerImpl* pplayer = DecodingQueue[i];
        if (pause && pplayer->IsPaused())
        {
            NotActive.PushBack(pplayer);
            continue;
        }
        if (!pause)
        {
            int idx = FindPlayer(NotActive, pplayer);
            if (idx != -1)
            {
                NotActive.RemoveAt(idx);
                continue;
            }
        }
        pplayer->Pause(pause);
    }
    DecodingQueuePaused = pause;
}


//////////////////////////////////////////////////////////////////////////
//

static VideoPlayer::CuePoint convertCuepoint(const CriManaEventPoint& eventPoint)
{
    VideoPlayer::CuePoint cp;
    cp.Name = eventPoint.cue_name;
    cp.Type = eventPoint.type == 0 ? VideoPlayer::NavigationCuePoint : VideoPlayer::EventCuePoint;
    cp.Time = (UInt32)(eventPoint.time / (eventPoint.tunit > 0 ? eventPoint.tunit : 1));
    if (eventPoint.param_string)
    {
        char* p1 = eventPoint.param_string;
        VideoPlayer::CuePointParam cpp;
        while(p1)
        {
            char* p2 = SFstrchr(p1,'=');
            if (!p2)
            {
                cpp.Name = p1;
                cp.Params.PushBack(cpp);
                break;
            }
            cpp.Name.AssignString(p1, p2 - p1);
            p1 = p2 + 1;
            p2 = SFstrchr(p1, ',');
            if (!p2)
            {
                cpp.Value = p1;
                cp.Params.PushBack(cpp);
                break;
            }
            else
            {
                cpp.Value.AssignString(p1, p2 - p1);
                cp.Params.PushBack(cpp);
                cpp.Name.Clear();
                cpp.Value.Clear();
            }
            p1 = p2 + 1;
        }
    }
    return cp;
}

static void CRIAPI callbackCuePoint(void *usrObj, CriManaPlayerHn player, CriManaEventPoint *eventInfo)
{
    SF_UNUSED(player);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)usrObj;
    if (pplayer)
    {
        pplayer->AddCurrentCuePoint(convertCuepoint(*eventInfo));
    }
}


//////////////////////////////////////////////////////////////////////////
//

VideoPlayerImpl::VideoPlayerImpl(MemoryHeap* pheap) :
    pManaPlayer(NULL), Stat(NotInitialized), VideoWidth((unsigned)-1), VideoHeight((unsigned)-1),
    VideoAlpha(false), LastFrame(0), StartPos(0), StatusAfterStop(Finished), Paused(CRI_FALSE), LoopFlag(CRI_FALSE),
    UpdateTextureCalled(false), FrameOnTime(false), FrameNotInitialized(true), PausedStartup(false),
    CurrSubtitleChannel(CRIMANA_SUBTITLE_CHANNEL_OFF), DecodeHeaderTimeout(0),
    pGFxHeap(pheap)
{
    Alg::MemUtil::Set(&MovieInfo, 0, sizeof(MovieInfo));
    Alg::MemUtil::Set(&FrameInfo, 0, sizeof(FrameInfo));
}

VideoPlayerImpl::~VideoPlayerImpl()
{
    WaitForFinish();

    if (pManaPlayer)
        criManaPlayer_Destroy(pManaPlayer);
    pManaPlayer = NULL;
}

#ifdef GFX_VIDEO_DIAGS
static const char* ManaStatusToString(CriManaPlayerStatus stat)
{
    switch(stat)
    {
    case CRIMANAPLAYER_STATUS_STOP:      return "STOP";
    case CRIMANAPLAYER_STATUS_DECHDR:    return "DECHDR";
    case CRIMANAPLAYER_STATUS_WAIT_PREP: return "WAIT_PREP";
    case CRIMANAPLAYER_STATUS_PREP:      return "PREP";
    case CRIMANAPLAYER_STATUS_READY:     return "READY";
    case CRIMANAPLAYER_STATUS_PLAYING:   return "PLAYING";
    case CRIMANAPLAYER_STATUS_PLAYEND:   return "PLAYEND";
    case CRIMANAPLAYER_STATUS_ERROR:     return "ERROR";
    default:
        return "Unknown";
    }
}
#endif

bool VideoPlayerImpl::Init(Video* pvideo, TaskManager* ptaskManager,
                           FileOpenerBase* pfileOpener, Log* plog)
{
    SF_UNUSED2(ptaskManager, pfileOpener);
    pLog = plog;

    pDecoder = pvideo && pvideo->GetDecoder() ? pvideo->GetDecoder() : NULL;
    if (!pDecoder)
        pDecoder = *SF_HEAP_NEW(pGFxHeap) VideoDecoderSmp;
    SF_ASSERT(pDecoder);

    pManaPlayer = criManaPlayer_Create(NULL, 0);
    SF_ASSERT(pManaPlayer);

    // Default to audio timer sync (falls back to system timer if no audio)
    criManaPlayer_SetMasterTimerType(pManaPlayer, CRIMANAPLAYER_TIMER_AUDIO);

    Stat = Stopped;
    return true;
}

void VideoPlayerImpl::Open(const char* url)
{
    SF_ASSERT(pManaPlayer);

    WaitForFinish();

    criManaPlayer_SetCuePointCallback(pManaPlayer, NULL, NULL);
    ClearCurrentCuePoints();

    LastFrame = StartPos = 0;
    Stat = Opening;
    StatusAfterStop = Finished;
    Paused = CRI_FALSE;
    UpdateTextureCalled = false;
    CurrSubtitleChannel = CRIMANA_SUBTITLE_CHANNEL_OFF;

    Alg::MemUtil::Set(&MovieInfo, 0, sizeof(MovieInfo));
    Alg::MemUtil::Set(&FrameInfo, 0, sizeof(FrameInfo));

    // Validate the file exists before passing to CRI Mana.
    // CRI's internal decode thread can crash if the file is missing.
    {
        SysFile file(url);
        if (!file.IsValid())
        {
            if (pLog)
                pLog->LogError("Video file not found: %s\n", url);
            Stat = FileNotFound;
            return;
        }
    }

    criManaPlayer_SetFile(pManaPlayer, NULL, url);
    criManaPlayer_DecodeHeader(pManaPlayer);
    pDecoder->StartDecoding(this);

#ifdef GFX_VIDEO_DIAGS
    CriManaPlayerStatus playerStatus = criManaPlayer_GetStatus(pManaPlayer);
    pLog->LogMessage("[Video] Open: %s %s\n", url, ManaStatusToString(playerStatus));
#endif

    StartOpenTicks = Timer::GetTicks();
}

void VideoPlayerImpl::WaitForFinish()
{
    if (Stat != Opening && Stat != Ready && Stat != Starting && Stat != Playing &&
        Stat != Seeking && Stat != Stopping)
        return;

    pDecoder->StopDecoding(this);
    while(!pDecoder->IsDecodingStopped(this))
    {
        Decode();
        Thread::MSleep(1);
    }
}

void VideoPlayerImpl::GetVideoInfo(VideoInfo* pinfo)
{
    SF_ASSERT(pinfo);

    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        pinfo->Width         = MovieInfo.video_prm[0].width;
        pinfo->Height        = MovieInfo.video_prm[0].height;
        pinfo->FrameRate     = MovieInfo.video_prm[0].framerate;
        pinfo->AudioDataRate = MovieInfo.num_audio_streams > 0 ? MovieInfo.audio_prm[0].sampling_rate : 0;
        pinfo->TotalFrames   = MovieInfo.video_prm[0].total_frames;
        pinfo->SubtitleChannelsNumber = MovieInfo.num_subtitle_channels;
        pinfo->MaxSubtitleLen         = MovieInfo.max_subtitle_size;

        if (pinfo->AudioTracks.GetSize() == 0)
        {
            for(int i = 0; i < CRIMANA_MAX_AUDIO_NUM; ++i)
            {
                if (MovieInfo.audio_prm[i].num_channels != 0)
                {
                    AudioTrackInfo info;
                    info.Index = i;
                    info.ChannelsNumber = MovieInfo.audio_prm[i].num_channels;
                    info.SampleRate     = MovieInfo.audio_prm[i].sampling_rate;
                    info.TotalSamples   = MovieInfo.audio_prm[i].total_samples;
                    pinfo->AudioTracks.PushBack(info);
                }
            }
        }

        if (pinfo->CuePoints.GetSize() == 0)
        {
            CriManaCuePointInfo *pcuepoint_info = criManaPlayer_GetCuePointInfo(pManaPlayer);
            if (pcuepoint_info != NULL)
            {
                for (CriUint32 i = 0; i < pcuepoint_info->num_eventpoint; i++)
                    pinfo->CuePoints.PushBack(convertCuepoint(pcuepoint_info->eventtable[i]));

                criManaPlayer_SetCuePointCallback(pManaPlayer, callbackCuePoint, this);
            }
        }
    }
}

void VideoPlayerImpl::Play()
{
    if (Stat == Ready)
    {
        criManaPlayer_SetSeekPosition(pManaPlayer, StartPos);
        criManaPlayer_SetSubtitleChannel(pManaPlayer, CurrSubtitleChannel);
        criManaPlayer_Loop(pManaPlayer, LoopFlag);
        criManaPlayer_Start(pManaPlayer);
        Stat = Starting;
    }
}

void VideoPlayerImpl::Stop()
{
    if (Stat == Opening || Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        Stat = Stopping;
        StatusAfterStop = Stopped;
        criManaPlayer_Stop(pManaPlayer);
    }
}

void VideoPlayerImpl::Pause(bool on_off)
{
    if (FrameNotInitialized)
    {
        PausedStartup = on_off;
        return;
    }

    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        Paused = on_off ? CRI_TRUE : CRI_FALSE;
        criManaPlayer_Pause(pManaPlayer, Paused);
    }
}

void VideoPlayerImpl::Seek(UInt32 pos)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        StartPos = pos % MovieInfo.video_prm[0].total_frames;
        Stat = Seeking;
        criManaPlayer_Stop(pManaPlayer);
    }
    else if (Stat == Finished)
    {
        StartPos = pos % MovieInfo.video_prm[0].total_frames;
        pDecoder->StartDecoding(this);
        Stat = Ready;
        Play();
    }
}

UInt32 VideoPlayerImpl::GetPosition()
{
    if (Stat == Finished || Stat == Stopped)
        return 0;
    else
        return MovieInfo.num_video_streams > 0 ?
            LastFrame % MovieInfo.video_prm[0].total_frames : 0;
}

void VideoPlayerImpl::SetSubtitleChannel(int channel)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        if ((int)(MovieInfo.num_subtitle_channels) > channel && CurrSubtitleChannel != channel)
        {
            if (channel < 0)
            {
                if (CurrSubtitleChannel == CRIMANA_SUBTITLE_CHANNEL_OFF)
                    return;
                else
                    channel = CRIMANA_SUBTITLE_CHANNEL_OFF;
            }
            CurrSubtitleChannel = channel;
            criManaPlayer_SetSubtitleChannel(pManaPlayer, CurrSubtitleChannel);
        }
    }
}

int VideoPlayerImpl::GetSubtitleChannel()
{
    return CurrSubtitleChannel == CRIMANA_SUBTITLE_CHANNEL_OFF ? -1 : CurrSubtitleChannel;
}


//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::CheckHeaderDecoding()
{
    SF_ASSERT(Stat == Opening);

    criMana_ExecuteMain();
    criManaPlayer_ExecuteMain(pManaPlayer);
    pDecoder->ExecuteDecode(this);

    if (criManaPlayer_GetMovieInfo(pManaPlayer, &MovieInfo))
    {
        if (MovieInfo.is_playable && MovieInfo.num_video_streams > 0)
        {
            VideoWidth  = MovieInfo.video_prm[0].width;
            VideoHeight = MovieInfo.video_prm[0].height;
            VideoAlpha = (MovieInfo.num_alpha_streams > 0);
            Stat = Ready;
        }
        else
        {
            Stat = Stopping;
            StatusAfterStop = FileNotFound;
            pDecoder->StopDecoding(this);
        }
    }
    else
    {
        if(DecodeHeaderTimeout > 0)
        {
            UInt64 diff = Timer::GetTicks() - StartOpenTicks;
            if (diff > DecodeHeaderTimeout)
            {
                Stat = Stopping;
                StatusAfterStop = InternalError;
                pDecoder->StopDecoding(this);
                if (pLog)
                    pLog->LogError("Decode USM header timeout. Video file could be broken.\n");
                return;
            }
        }

        if (criManaPlayer_GetStatus(pManaPlayer) == CRIMANAPLAYER_STATUS_ERROR)
        {
            Stat = Stopping;
            StatusAfterStop = FileNotFound;
            pDecoder->StopDecoding(this);
        }
    }
}

void VideoPlayerImpl::SetDecodeHeaderTimeout(float timeout)
{
    DecodeHeaderTimeout = timeout > 0 ? (UInt64)(timeout * 1000 * 1000) : 0;
}

void VideoPlayerImpl::Decode()
{
    if (Stat != Opening && Stat != Ready && Stat != Starting && Stat != Playing &&
        Stat != Seeking && Stat != Stopping)
        return;

    if (Stat == Opening)
    {
        CheckHeaderDecoding();
        return;
    }

    // Global Mana server tick — drives CRI File System I/O and internal state.
    // Must be called periodically from the main thread (not thread-safe).
    criMana_ExecuteMain();
    criManaPlayer_SyncMasterTimer(pManaPlayer);
    criManaPlayer_ExecuteMain(pManaPlayer);

    CriManaPlayerStatus movieStatus = criManaPlayer_GetStatus(pManaPlayer);
    if (movieStatus == CRIMANAPLAYER_STATUS_PLAYEND ||
        movieStatus == CRIMANAPLAYER_STATUS_STOP)
    {
        if (pDecoder->IsDecodingStopped(this))
        {
            if (Stat == Stopping)
                Stat = StatusAfterStop;
            else if (Stat == Seeking && StartPos < MovieInfo.video_prm[0].total_frames)
            {
                criManaPlayer_Pause(pManaPlayer, CRI_FALSE);
                criManaPlayer_SetSeekPosition(pManaPlayer, StartPos);
                criManaPlayer_Loop(pManaPlayer, LoopFlag);
                criManaPlayer_SetSubtitleChannel(pManaPlayer, CurrSubtitleChannel);
                criManaPlayer_Start(pManaPlayer);
                pDecoder->StartDecoding(this);
                Stat = Starting;
            }
            else
                Stat = Finished;
        }
        return;
    }
    if (movieStatus == CRIMANAPLAYER_STATUS_ERROR)
    {
        if (StatusAfterStop != FileNotFound)
            StatusAfterStop = InternalError;
        Stat = Stopping;
        pDecoder->StopDecoding(this);
        if (pDecoder->IsDecodingStopped(this))
        {
            Stat = StatusAfterStop;
            return;
        }
    }

    if (FrameOnTime)
    {
        if (Stat == Starting)
        {
            LastFrame = StartPos;
            criManaPlayer_Pause(pManaPlayer, Paused);
            Stat = Playing;
            if (pSyncObj)
                pSyncObj->SetStartFrame(StartPos);
        }
        if (FrameInfo.frame_no >= 0)
            LastFrame = FrameInfo.frame_no;
    }

#ifdef GFX_VIDEO_DIAGS
    pLog->LogMessage("[Video] Decode: %d %s\n", FrameInfo.frame_no, ManaStatusToString(movieStatus));
#endif
}


//////////////////////////////////////////////////////////////////////////
//

VideoImage* VideoPlayerImpl::CreateTexture(Render::TextureManager* ptexman)
{
    if ((Stat != Ready && Stat != Starting && Stat != Playing && Stat != Seeking))
        return NULL;
    if (!ptexman)
        return NULL;

    FrameNotInitialized = true;

    ImageFormat format = IsAlphaVideo() ? Image_Y8_U2_V2_A8 : Image_Y8_U2_V2;
    ImageSize imageSize(VideoWidth, VideoHeight);
    VideoImage* pimage = SF_HEAP_NEW(pGFxHeap) VideoImage(format, imageSize, ptexman, this);
    SF_ASSERT(pimage);

    return pimage;
}

void VideoPlayerImpl::UpdateTexture(VideoImage* pimage, char* subtitle, int subtitleLength)
{
    if (Stat != Ready && Stat != Starting && Stat != Playing && Stat != Seeking && Stat != Stopping)
        return;
    if (!pimage)
        return;

    FrameOnTime = false;

	if ( !UpdateTextureCalled )
	{
		UpdateTextureCalled = true;
		pimage->Update();
	}

    if (subtitle && subtitleLength > 0 && MovieInfo.num_subtitle_channels > 0 &&
        CurrSubtitleChannel != CRIMANA_SUBTITLE_CHANNEL_OFF)
    {
        if ((Stat == Playing && !IsPaused()) || Stat == Seeking)
            criManaPlayer_GetSubtitleOnTime(pManaPlayer, (CriUint8*)subtitle, subtitleLength);
    }
}

void VideoPlayerImpl::SetSyncObject(VideoPlayer::SyncObject* psync)
{
    pSyncObj = psync;

    if (psync)
        criManaPlayer_SetMasterTimerType(pManaPlayer, CRIMANAPLAYER_TIMER_SYSTEM);
    else
        criManaPlayer_SetMasterTimerType(pManaPlayer, CRIMANAPLAYER_TIMER_AUDIO);
}

//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::GetCurrentCuePoints(Array<CuePoint>* cue_points)
{
    if (CurrentCuePoints.GetSize() > 0)
    {
        *cue_points = CurrentCuePoints;
        CurrentCuePoints.Clear();
    }
}

void VideoPlayerImpl::AddCurrentCuePoint(const CuePoint& cp)
{
    CurrentCuePoints.PushBack(cp);
}

void VideoPlayerImpl::ClearCurrentCuePoints()
{
    CurrentCuePoints.Clear();
}


//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::SetAudioTrack(int track_index)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        criManaPlayer_SetAudioTrack(pManaPlayer, track_index);
        StartPos = LastFrame % MovieInfo.video_prm[0].total_frames;
        if (Stat != Ready)
        {
            Stat = Seeking;
            criManaPlayer_Stop(pManaPlayer);
        }
    }
}

void VideoPlayerImpl::SetSubAudioTrack(int track_index)
{
    SF_UNUSED(track_index);
    // CRI Mana 2019: sub-audio track is set via criManaPlayer_SetSubAudioTrack
    // which requires extra work config. Not currently supported in this integration.
}

void VideoPlayerImpl::ReplaceCenterVoice(int track_index)
{
    SF_UNUSED(track_index);
    // CRI Mana 2019 does not have a direct equivalent for center voice replacement.
}


//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::SetLoopFlag(bool flag)
{
    LoopFlag = flag ? CRI_TRUE : CRI_FALSE;
    if (Stat == Ready || Stat == Starting || Stat == Playing)
        criManaPlayer_Loop(pManaPlayer, LoopFlag);
}

void VideoPlayerImpl::SetBufferTime(float time)
{
    if (pManaPlayer)
        criManaPlayer_SetBufferingTime(pManaPlayer, time);
}

void VideoPlayerImpl::SetNumberOfFramePools(unsigned pools)
{
    if (pManaPlayer)
        criManaPlayer_SetNumberOfFramePools(pManaPlayer, pools);
}

void VideoPlayerImpl::SetReloadThresholdTime(float time)
{
    SF_UNUSED(time);
    // CRI Mana 2019 does not have a direct reload threshold API.
    // Buffering is controlled via criManaPlayer_SetBufferingTime.
}

void VideoPlayerImpl::GetReadBufferInfo(ReadBufferInfo* info)
{
    SF_ASSERT(info);
    if (pManaPlayer)
    {
        CriSint32 bufSize = criManaPlayer_GetInputBufferSize(pManaPlayer);
        CriSint32 remainSize = criManaPlayer_GetInputBufferRemainSize(pManaPlayer);
        info->BufferSize      = (UInt32)(bufSize > 0 ? bufSize : 0);
        info->DataSize        = (UInt32)(remainSize > 0 ? remainSize : 0);
        info->ReloadThreshold = info->BufferSize / 4;
    }
}


//////////////////////////////////////////////////////////////////////////
//

bool VideoPlayer::LoadVideoInfo(const char* pfilename, VideoPlayer::VideoInfo* pinfo,
                                FileOpenerBase* popener)
{
    SF_UNUSED(popener);

    bool ret = false;

    CriManaPlayerHn player = criManaPlayer_Create(NULL, 0);
    if (!player)
        return false;

    criManaPlayer_SetFile(player, NULL, pfilename);
    criManaPlayer_DecodeHeader(player);

    // Poll until header is decoded
    for (int i = 0; i < 1000; ++i)
    {
        criMana_ExecuteMain();
        CriManaPlayerStatus status = criManaPlayer_GetStatus(player);
        if (status == CRIMANAPLAYER_STATUS_WAIT_PREP)
        {
            CriManaMovieInfo movieInfo;
            if (criManaPlayer_GetMovieInfo(player, &movieInfo) && movieInfo.is_playable)
            {
                pinfo->Width         = movieInfo.video_prm[0].width;
                pinfo->Height        = movieInfo.video_prm[0].height;
                pinfo->FrameRate     = movieInfo.video_prm[0].framerate;
                pinfo->TotalFrames   = movieInfo.video_prm[0].total_frames;
                pinfo->AudioDataRate = movieInfo.num_audio_streams > 0 ? movieInfo.audio_prm[0].sampling_rate : 0;
                pinfo->SubtitleChannelsNumber = movieInfo.num_subtitle_channels;
                pinfo->MaxSubtitleLen         = movieInfo.max_subtitle_size;

                for(int a = 0; a < CRIMANA_MAX_AUDIO_NUM; ++a)
                {
                    if (movieInfo.audio_prm[a].num_channels != 0)
                    {
                        AudioTrackInfo ainfo;
                        ainfo.Index = a;
                        ainfo.ChannelsNumber = movieInfo.audio_prm[a].num_channels;
                        ainfo.SampleRate     = movieInfo.audio_prm[a].sampling_rate;
                        ainfo.TotalSamples   = movieInfo.audio_prm[a].total_samples;
                        pinfo->AudioTracks.PushBack(ainfo);
                    }
                }
                ret = true;
            }
            break;
        }
        else if (status == CRIMANAPLAYER_STATUS_ERROR ||
                 status == CRIMANAPLAYER_STATUS_STOP)
        {
            break;
        }
        Thread::MSleep(1);
    }

    criManaPlayer_StopAndWaitCompletion(player);
    criManaPlayer_Destroy(player);

    return ret;
}


//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_ENABLE_SOUND

Sound::SoundChannel* VideoPlayerImpl::GetSoundChannel(SoundTrack track)
{
    SF_UNUSED(track);
    // CRI Mana 2019 manages audio internally via CriAtomEx.
    // Scaleform sound channel bridging is not supported with the new API.
    return NULL;
}

void VideoPlayerImpl::SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[])
{
    SF_UNUSED(spatinfo);
}

#endif // GFX_ENABLE_SOUND

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
