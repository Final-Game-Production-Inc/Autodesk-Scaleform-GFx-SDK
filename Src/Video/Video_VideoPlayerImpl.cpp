/**************************************************************************

Filename    :   Video_VideoPlayerImpl.cpp
Content     :   GFx video player implementation
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoPlayerImpl.h"
#ifdef GFX_ENABLE_VIDEO

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_xpt.h>
#include <cri_error.h>
#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Alg.h"

#include "GFx/GFx_Log.h"

#define THREAD_STACK_SIZE   65536

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

#if defined( SF_OS_ORBIS )

static void DefaultInitVideoDecoderThrd( ScePthread pid, int requestedPriority )
{
	// Hack, SF thread fails to use SCE_PTHREAD_EXPLICIT_SCHED, so it fails to properly set anything
	// it just gets the calling thread scheduling and priority

	const SceKernelSchedParam schedParam = { requestedPriority };
	{
		ScePthreadAttr attr;
		scePthreadAttrInit( &attr );
		scePthreadAttrGet( pid, &attr );
		scePthreadAttrSetinheritsched( &attr, SCE_PTHREAD_EXPLICIT_SCHED );
		scePthreadSetschedparam( pid, SCE_KERNEL_SCHED_FIFO, &schedParam ); // CriDelegate uses FIFO scheduling (seen in target manager dump), so we use the same here.
		scePthreadAttrDestroy( &attr );
	}

	// Use CRI worker thread affinity mask
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
        pplayer->GetCriPlayer()->Stop();
}

bool VideoDecoderThrd::IsDecodingStopped(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pp;
    Lock::Locker lock(&DecodeLock);
    return FindPlayer(DecodingQueue, pplayer) == -1;
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
            if (!pplayer->GetCriPlayer()->ExecuteDecode())
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
                if (!copy_array[i]->GetCriPlayer()->ExecuteDecode())
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
        pplayer->GetCriPlayer()->Stop();
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
    if (!pplayer->GetCriPlayer()->ExecuteDecode())
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

static VideoPlayer::CuePoint convertCuepoint(const CriMvEventPoint& eventPoint)
{
    VideoPlayer::CuePoint cp;
    cp.Name = eventPoint.cue_name;
    cp.Type = eventPoint.type == 0 ? VideoPlayer::NavigationCuePoint : VideoPlayer::EventCuePoint;
    cp.Time = (UInt32)eventPoint.time;
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

static void callbackCuePoint(CriMvEasyPlayer *mveasy, CriMvEventPoint *eventInfo, void *usrObj)
{
    SF_UNUSED(mveasy);
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)usrObj;
    if (pplayer)
    {
        pplayer->AddCurrentCuePoint(convertCuepoint(*eventInfo));
    }
}


//////////////////////////////////////////////////////////////////////////
//

VideoPlayerImpl::VideoPlayerImpl(MemoryHeap* pheap) :
    pCriPlayer(NULL), Stat(NotInitialized), VideoWidth((unsigned)-1), VideoHeight((unsigned)-1),
    VideoAlpha(false), LastFrame(0), StartPos(0), StatusAfterStop(Finished), Paused(FALSE), LoopFlag(FALSE),
    UpdateTextureCalled(false), FrameOnTime(false), FrameNotInitialized(true), PausedStartup(false),
    CurrSubtitleChannel(CRIMV_SUBTITLE_CHANNEL_OFF), DecodeHeaderTimeout(0), UseAudioTimer(true),
    pGFxHeap(pheap)
{
    Heap = criSmpCustomHeap_Create(pheap);
}

VideoPlayerImpl::~VideoPlayerImpl()
{
    WaitForFinish();

    if (pSubSound)
        pCriPlayer->DetachSubAudioInterface();

    if (pCriPlayer)
        pCriPlayer->Destroy();
    pCriPlayer = NULL;

    pTimer = NULL;
    pSound = NULL;
    pSubSound = NULL;
    pReader = NULL;

    criSmpCustomHeap_Destroy(Heap);
}

#ifdef GFX_VIDEO_DIAGS
static const char* StatusToString(CriMvEasyPlayer::Status stat)
{
    switch(stat)
    {
    case CriMvEasyPlayer::MVEASY_STATUS_STOP:      return "STOP";
    case CriMvEasyPlayer::MVEASY_STATUS_DECHDR:    return "DECHDR";
    case CriMvEasyPlayer::MVEASY_STATUS_WAIT_PREP: return "WAIT_PREP";
    case CriMvEasyPlayer::MVEASY_STATUS_PREP:      return "PREP";
    case CriMvEasyPlayer::MVEASY_STATUS_READY:     return "READY";
    case CriMvEasyPlayer::MVEASY_STATUS_PLAYING:   return "PLAYING";
    case CriMvEasyPlayer::MVEASY_STATUS_PLAYEND:   return "PLAYEND";
    case CriMvEasyPlayer::MVEASY_STATUS_ERROR:     return "ERROR";
    default:
        return "Unknown";
    }
}

static const char* FrameResultToString(CriMvLastFrameResult result)
{
    switch(result)
    {
    case CRIMV_LASTFRAME_OK:           return "OK";
    case CRIMV_LASTFRAME_TIME_EARLY:   return "EARLY";
    case CRIMV_LASTFRAME_DECODE_DELAY: return "DECODE_DELAY";
    case CRIMV_LASTFRAME_DISCARDED:    return "DISCARDED";
    default:
        return "Unknown";
    }
}
#endif

bool VideoPlayerImpl::Init(Video* pvideo, TaskManager* ptaskManager, 
                           FileOpenerBase* pfileOpener, Log* plog)
{
    SF_UNUSED(ptaskManager);
    pLog = plog;
    VideoSoundSystem* psndsystem = pvideo? pvideo->GetSoundSystem() : NULL;
    if (psndsystem)
    {
        Ptr<VideoSound> pinterface    = *psndsystem->Create();
        Ptr<VideoSound> psubinterface = *psndsystem->Create();
        pSound    = *SF_HEAP_NEW(pGFxHeap) CriMvSound(pGFxHeap, pinterface);
        pSubSound = *SF_HEAP_NEW(pGFxHeap) CriMvSound(pGFxHeap, psubinterface);
    }
    pDecoder = pvideo && pvideo->GetDecoder() ? pvideo->GetDecoder() : NULL;
    if (!pDecoder) 
        pDecoder = *SF_HEAP_NEW(pGFxHeap) VideoDecoderSmp;
    SF_ASSERT(pDecoder);

    if (pvideo)
        pReader = *pvideo->GetReaderConfig()->CreateReader();
    else
    {   // LoadInfo() special case
        SF_ASSERT(pfileOpener);
        Ptr<VideoReaderConfig> preaderConfig = *SF_HEAP_NEW(pGFxHeap) VideoReaderConfigSmp(pGFxHeap, pfileOpener);
        pReader = *SF_HEAP_NEW(pGFxHeap) VideoReader(preaderConfig);
    }

    Ptr<SyncObject> psyncObj = *SF_HEAP_NEW(pGFxHeap) SystemTimerSyncObject;
    pTimer = *SF_HEAP_NEW(pGFxHeap) CriMvSystemTimer(psyncObj);

    pCriPlayer = CriMvEasyPlayer::Create(Heap, 
        (CriMvFileReaderInterface*)pReader, (CriMvSystemTimerInterface*)pTimer, (CriMvSoundInterface*)pSound);
    SF_ASSERT(pCriPlayer);

    if (pSound && UseAudioTimer)
        pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_AUDIO);
    else
        pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_SYSTEM);

    Stat = Stopped;
    return true;
}

void VideoPlayerImpl::Open(const char* url)
{
    SF_ASSERT(pCriPlayer);

    WaitForFinish();

    pCriPlayer->SetCuePointCallback(NULL, NULL);
    ClearCurrentCuePoints();

    LastFrame = StartPos = 0;
    Stat = Opening;
    StatusAfterStop = Finished;
    Paused = FALSE;
    UpdateTextureCalled = false;
    CurrSubtitleChannel = CRIMV_SUBTITLE_CHANNEL_OFF;

    Alg::MemUtil::Set(&StreamParams, 0, sizeof(StreamParams));
    Alg::MemUtil::Set(&FrameInfo, 0, sizeof(FrameInfo));

    pCriPlayer->SetFile((Char8*)url);
    pCriPlayer->DecodeHeader();
    pDecoder->StartDecoding(this);

    if (pSubSound)
        pCriPlayer->AttachSubAudioInterface(pSubSound);

#ifdef GFX_VIDEO_DIAGS
    CriMvEasyPlayer::Status playerStatus = pCriPlayer->GetStatus();
    pLog->LogMessage("[Video] Open: %s %s\n", (Char8*)url, StatusToString(playerStatus));

    LastDecodeTicks = 0;
    LastUpdateTextureTicks = 0;
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

#ifdef GFX_VIDEO_DIAGS
    CriMvEasyPlayer::Status playerStatus = pCriPlayer->GetStatus();
    pLog->LogMessage("[Video] WaitForFinish: %s\n", StatusToString(playerStatus));
#endif
}

void VideoPlayerImpl::GetVideoInfo(VideoInfo* pinfo)
{
    SF_ASSERT(pinfo);

    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
#ifdef GFX_VIDEO_DIAGS
        if (StreamParams.num_audio > 0 && pLog)
        {
            float videoTime = StreamParams.video_prm[0].total_frames /
                (StreamParams.video_prm[0].framerate/1000.0f);
            for (unsigned int i = 0; i < StreamParams.num_audio; ++i)
            {
                if (StreamParams.audio_prm[i].num_channel != 0)
                {
                    SF_ASSERT(StreamParams.audio_prm[i].sampling_rate > 0); // Division by zero
                    float audioTime =
                        StreamParams.audio_prm[i].total_samples * 1.0f /
                        StreamParams.audio_prm[i].sampling_rate;
                    if (Alg::Abs(videoTime - audioTime) > 0.01)
                    {
                        pLog->LogMessage("[Video] GetVideoInfo: "
                            "Duration of the video (%.2f sec) and audio track %d (%.2f sec) is different. "
                            "This may cause a video playback problem.\n", videoTime, i, audioTime);
                    }
                }
            }
        }
#endif
        pinfo->Width         = StreamParams.video_prm[0].max_width;
        pinfo->Height        = StreamParams.video_prm[0].max_height;
        pinfo->FrameRate     = StreamParams.video_prm[0].framerate;
        pinfo->AudioDataRate = StreamParams.num_audio > 0 ? StreamParams.audio_prm[0].sampling_rate : 0;
        pinfo->TotalFrames   = StreamParams.video_prm[0].total_frames;
        pinfo->SubtitleChannelsNumber = StreamParams.num_subtitle;
        pinfo->MaxSubtitleLen         = StreamParams.max_subtitle_size;

        if (pinfo->AudioTracks.GetSize() == 0)
        {
            for(int i = 0; i < CRIMV_MAX_AUDIO_NUM; ++i)
            {
                if (StreamParams.audio_prm[i].num_channel != 0)
                {
                    AudioTrackInfo info;
                    info.Index = i;
                    info.ChannelsNumber = StreamParams.audio_prm[i].num_channel;
                    info.SampleRate     = StreamParams.audio_prm[i].sampling_rate;
                    info.TotalSamples   = StreamParams.audio_prm[i].total_samples;
                    pinfo->AudioTracks.PushBack(info);
                }
            }
        }

        if (pinfo->CuePoints.GetSize() == 0)
        {
            CriMvCuePointInfo *pcuepoint_info = pCriPlayer->GetCuePointInfo();
            if (pcuepoint_info != NULL) 
            {
                for (Uint32 i = 0; i < pcuepoint_info->num_eventpoint; i++) 
                    pinfo->CuePoints.PushBack(convertCuepoint(pcuepoint_info->eventtable[i]));

                pCriPlayer->SetCuePointCallback(callbackCuePoint, this);
            }
        }
    }
}

void VideoPlayerImpl::Play()
{
    if (Stat == Ready)
    {
        pCriPlayer->SetSeekPosition(StartPos);
        pCriPlayer->SetSubtitleChannel(CurrSubtitleChannel);
        pCriPlayer->SetLoopFlag(LoopFlag);
        pCriPlayer->Start();
        Stat = Starting;
    }
}

void VideoPlayerImpl::Stop()
{
    if (Stat == Opening || Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        Stat = Stopping;
        StatusAfterStop = Stopped;
        pCriPlayer->Stop();
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
        Paused = (CriBool)on_off;
        pCriPlayer->Pause(Paused);
    }
}

void VideoPlayerImpl::Seek(UInt32 pos)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        StartPos = pos % StreamParams.video_prm[0].total_frames;
        Stat = Seeking;
        pCriPlayer->Stop();
    } 
    else if (Stat == Finished)
    {
        StartPos = pos % StreamParams.video_prm[0].total_frames;
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
        return StreamParams.num_video > 0 ? 
            LastFrame % StreamParams.video_prm[0].total_frames : 0;
}

void VideoPlayerImpl::SetSubtitleChannel(int channel)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        if ((int)(StreamParams.num_subtitle) > channel && CurrSubtitleChannel != channel)
        {
            if (channel < 0)
            {
                if (CurrSubtitleChannel == CRIMV_SUBTITLE_CHANNEL_OFF)
                    return;
                else
                    channel = CRIMV_SUBTITLE_CHANNEL_OFF;
            }
            CurrSubtitleChannel = channel;
            pCriPlayer->SetSubtitleChannel(CurrSubtitleChannel);
        }
    }
}

int VideoPlayerImpl::GetSubtitleChannel()
{
    return CurrSubtitleChannel == CRIMV_SUBTITLE_CHANNEL_OFF ? -1 : CurrSubtitleChannel;
}


//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::CheckHeaderDecoding()
{
    SF_ASSERT(Stat == Opening);

    pCriPlayer->Update();
    pDecoder->ExecuteDecode(this);

    if (pCriPlayer->GetMovieInfo(StreamParams))
    {
        if (StreamParams.is_playable && StreamParams.num_video > 0)
        {
            VideoWidth  = StreamParams.video_prm[0].max_width;
            VideoHeight = StreamParams.video_prm[0].max_height;
            VideoAlpha = (StreamParams.num_alpha > 0);
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
        
        if (pCriPlayer->GetStatus() == CriMvEasyPlayer::MVEASY_STATUS_ERROR)
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

    pCriPlayer->SyncMasterTimer();
    pCriPlayer->Update();

    CriMvEasyPlayer::Status movieStatus = pCriPlayer->GetStatus();
    if (movieStatus == CriMvEasyPlayer::MVEASY_STATUS_PLAYEND ||
        movieStatus == CriMvEasyPlayer::MVEASY_STATUS_STOP)
    {
        if (pDecoder->IsDecodingStopped(this))
        {
            if (Stat == Stopping)
                Stat = StatusAfterStop;
            else if (Stat == Seeking && StartPos < StreamParams.video_prm[0].total_frames)
            {
                pCriPlayer->Pause(FALSE);
                pCriPlayer->SetSeekPosition(StartPos);
                pCriPlayer->SetLoopFlag(LoopFlag);
                pCriPlayer->SetSubtitleChannel(CurrSubtitleChannel);
                pCriPlayer->Start();
                pDecoder->StartDecoding(this);
                Stat = Starting;
            }
            else
                Stat = Finished;
        }
        return;
    }
    if (movieStatus == CriMvEasyPlayer::MVEASY_STATUS_ERROR) 
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
            pCriPlayer->Pause(Paused);
            Stat = Playing;
            SyncObject* psyncObj = pTimer->GetSyncObject();
            if (psyncObj)
                psyncObj->SetStartFrame(StartPos);
        }
        if (FrameInfo.frame_id >= 0)
            LastFrame = FrameInfo.frame_id;
    }

// NOTE: ExecuteDecode only does anything here if using the VideoDecoderSmp class.
//     if (UpdateTextureCalled)
//     {
//         pDecoder->ExecuteDecode(this);
//         UpdateTextureCalled = false;
//     }

#ifdef GFX_VIDEO_DIAGS
    pLog->LogMessage("[Video] Decode: %u %s %.2f\n", FrameInfo.frame_id, StatusToString(movieStatus),
        (LastDecodeTicks ? Float32(Timer::GetProfileTicks() - LastDecodeTicks) / 1000 : 0));
    LastDecodeTicks = Timer::GetProfileTicks();
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

#if defined(SF_OS_WII)
    // Special case: Wii uses RGBA image format
    ImageFormat format = Image_Wii_R8G8B8A8;
#else
    // Using YUV textures by default. Switching to Image_B8G8R8A8 requires YUV to RGBA
    // conversion to be enabled. See Video::Initialize() for details.    
    ImageFormat format = IsAlphaVideo() ? Image_Y8_U2_V2_A8 : Image_Y8_U2_V2;
#endif
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

	// Don't queue up a bunch of update requests to be handled in a single render tick... we'll die from drying to map the discard-texture
	if ( !UpdateTextureCalled )
	{
		UpdateTextureCalled = true;
		pimage->Update();
	}

    if (subtitle && subtitleLength > 0 && StreamParams.num_subtitle > 0 &&
        CurrSubtitleChannel != CRIMV_SUBTITLE_CHANNEL_OFF)
    {
        if ((Stat == Playing && !IsPaused()) || Stat == Seeking)
            pCriPlayer->GetSubtitleOnTime((Uint8*)subtitle, subtitleLength);
    }

#ifdef GFX_VIDEO_DIAGS
    // Result of the last video frame retrieval
    LastFrameResult = pCriPlayer->GetLastFrameResult();
    pLog->LogMessage("[Video] UpdateTexture: %u %s %.2f\n", FrameInfo.frame_id, FrameResultToString(LastFrameResult),
        (LastUpdateTextureTicks ? Float32(Timer::GetProfileTicks() - LastUpdateTextureTicks) / 1000 : 0));

    // Get playback information
    pCriPlayer->GetPlaybackInfo(PlaybackInfo);
    pLog->LogMessage(
        "[Video] countAppLoop=     %lld\n"
        "[Video] countTimeEarly=   %lld\n"
        "[Video] countDecodeDelay= %lld\n"
        "[Video] timeMaxDelay=     %.2f\n\n",
        PlaybackInfo.cnt_app_loop,     // Loop count of application. Number of calls of Update()
        PlaybackInfo.cnt_time_early,   // How many times IsNextFrameOnTime() returns FALSE due to CRIMV_LASTFRAME_TIME_EARLY
        PlaybackInfo.cnt_decode_delay, // How many times IsNextFrameOnTime() returns FALSE due to CRIMV_LASTFRAME_DECODE_DELAY
        PlaybackInfo.time_max_delay    // Maximum delay time [msec]
    );

    LastUpdateTextureTicks = Timer::GetProfileTicks();
#endif
}

void VideoPlayerImpl::SetSyncObject(VideoPlayer::SyncObject* psyncObj)
{
    if (Stat != Opening && Stat != Ready && Stat != Starting && Stat != Playing &&
        Stat != Seeking && Stat != Stopping)
    {
        if (!psyncObj && !pTimer->pSyncObj)
        {
            Ptr<SyncObject> ps = *SF_HEAP_NEW(pGFxHeap) SystemTimerSyncObject;
            pTimer->SetSyncObject(ps);
            if (pSound && UseAudioTimer)
                pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_AUDIO);
            else
                pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_SYSTEM);
        }
        else if (psyncObj && pTimer->pSyncObj != psyncObj)
        {
            pTimer->SetSyncObject(psyncObj);
            pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_SYSTEM);
        }
    }
    else
    {
        WaitForFinish();

        pCriPlayer->SetCuePointCallback(NULL, NULL);
        ClearCurrentCuePoints();

        LastFrame = StartPos = 0;
        Stat = Opening;
        StatusAfterStop = Finished;
        Paused = FALSE;
        UpdateTextureCalled = false;
        CurrSubtitleChannel = CRIMV_SUBTITLE_CHANNEL_OFF;

        Alg::MemUtil::Set(&StreamParams, 0, sizeof(StreamParams));
        Alg::MemUtil::Set(&FrameInfo, 0, sizeof(FrameInfo));

        if (!psyncObj)
        {
            Ptr<SyncObject> ps = *SF_HEAP_NEW(pGFxHeap) SystemTimerSyncObject;
            pTimer->SetSyncObject(ps);
            if (pSound && UseAudioTimer)
                pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_AUDIO);
            else
                pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_SYSTEM);
        }
        else
        {
            pTimer->SetSyncObject(psyncObj);
            pCriPlayer->SetMasterTimer(CriMvEasyPlayer::MVEASY_TIMER_SYSTEM);
        }

        pCriPlayer->DecodeHeader();
        pDecoder->StartDecoding(this);

        StartOpenTicks = Timer::GetTicks();
    }
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
        pCriPlayer->SetAudioTrack(track_index);
        StartPos = LastFrame % StreamParams.video_prm[0].total_frames;
        if (Stat != Ready)
        {
            Stat = Seeking;
            pCriPlayer->Stop();
        }
    }
}

void VideoPlayerImpl::SetSubAudioTrack(int track_index)
{
    if (Stat == Ready && pSubSound)
        pCriPlayer->SetSubAudioTrack(track_index);
}

void VideoPlayerImpl::ReplaceCenterVoice(int track_index)
{
    if (Stat == Ready)
        pCriPlayer->ReplaceCenterVoice(track_index);
}


//////////////////////////////////////////////////////////////////////////
//

void VideoPlayerImpl::SetLoopFlag(bool flag)
{
    LoopFlag = flag;
    if (Stat == Ready || Stat == Starting || Stat == Playing)
        pCriPlayer->SetLoopFlag(LoopFlag);
}

void VideoPlayerImpl::SetBufferTime(float time)
{
    if (pCriPlayer)
        pCriPlayer->SetBufferingTime(time);
}

void VideoPlayerImpl::SetNumberOfFramePools(unsigned pools)
{
    if (pCriPlayer)
        pCriPlayer->SetNumberOfFramePools(pools);
}

void VideoPlayerImpl::SetReloadThresholdTime(float time)
{
    if (pCriPlayer)
        pCriPlayer->SetReloadThresholdTime(time);
}

void VideoPlayerImpl::GetReadBufferInfo(ReadBufferInfo* info)
{
    SF_ASSERT(info);
    if (pCriPlayer)
    {
        CriMvInputBufferInfo cri_buf_info;
        pCriPlayer->GetInputBufferInfo(cri_buf_info);
        info->BufferSize      = cri_buf_info.buffer_size;
        info->DataSize        = cri_buf_info.data_size;
        info->ReloadThreshold = cri_buf_info.reload_threshold;
    }
}


//////////////////////////////////////////////////////////////////////////
//

bool VideoPlayer::LoadVideoInfo(const char* pfilename, VideoPlayer::VideoInfo* pinfo,
                                FileOpenerBase* popener)
{
    bool ret = false;
    MemoryHeap* pheap = Memory::GetGlobalHeap()->CreateHeap(
        "_VideoInfo_Heap", 0, 16, 16*1024, 16*1024, ~UPInt(0), 0, HeapId_Video);

    VideoPlayerImpl* pvideo = SF_HEAP_NEW(pheap) VideoPlayerImpl(pheap);
    pvideo->Init(NULL, NULL, popener, NULL);
    pvideo->Open(pfilename);
    while (pvideo->GetStatus() == VideoPlayer::Opening)
    {
        pvideo->Decode();
    }

    if (pvideo->GetStatus() == VideoPlayer::Ready)
    {
        ret = true;
        pvideo->GetVideoInfo(pinfo);
    }

    pvideo->Destroy();
    pheap->Release();

    return ret;
}


//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_ENABLE_SOUND

Sound::SoundChannel* VideoPlayerImpl::GetSoundChannel(SoundTrack track)
{
    if (track == VideoPlayer::MainTrack && pSound)
        return pSound->GetSoundChannel();
    if (track == VideoPlayer::SubAudio && pSubSound)
        return pSubSound->GetSoundChannel();
    return NULL;
}

void VideoPlayerImpl::SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[])
{
    if (pSound) {
        Sound::SoundChannel* pmainchan = pSound->GetSoundChannel();
        if (pmainchan)
            pmainchan->SetSpatialInfo(spatinfo);
    }
    if (pSubSound) {
        Sound::SoundChannel* psubchan = pSubSound->GetSoundChannel();
        if (psubchan)
            psubchan->SetSpatialInfo(spatinfo);
    }
}

#endif // GFX_ENABLE_SOUND

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
