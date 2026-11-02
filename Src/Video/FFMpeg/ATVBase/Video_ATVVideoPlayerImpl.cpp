/**************************************************************************

Filename    :   Video_ATVVideoPlayerImpl.cpp
Content     :   GFx video player implementation (FFmpeg / libav)
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/FFMpeg/ATVBase/Video_ATVVideoPlayerImpl.h"
#include "Video/FFMpeg/ATVBase/Video_ATVMvSoundGateway.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_Timer.h"

#include "GFx/GFx_Log.h"

#include <stdio.h>
#if defined(SF_BUILD_DEBUG)
#define ATV_TRACE(...) do { fprintf(stderr, "[ATV] " __VA_ARGS__); fflush(stderr); } while (0)
#else
#define ATV_TRACE(...) do {} while (0)
#endif

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
// FFmpeg public headers are plain C headers and carry no extern "C" guards,
// so they must be wrapped when included from C++ sources.
// They are third party code compiled at /W4: libavutil's inline clippers
// (av_clip_uint8_c & co. in libavutil/common.h) return an int expression from
// functions declared to return uint8_t/int8_t/uint16_t/int16_t, which makes MSVC
// report C4244 at the header line. The diagnostic is silenced around the include
// block instead of being patched in the vendored SDK.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244) // int -> uint8_t/int8_t/uint16_t/int16_t in libavutil
#endif
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/cpu.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>
#include <libavutil/log.h>
#include <libavutil/mathematics.h>
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

static bool IsPixFmtAlpha(enum AVPixelFormat fmt)
{
    switch (fmt)
    {
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
    case AV_PIX_FMT_YUVA420P10:
    case AV_PIX_FMT_YUVA422P10:
    case AV_PIX_FMT_YUVA444P10:
    case AV_PIX_FMT_YUVA420P16:
    case AV_PIX_FMT_YUVA422P16:
    case AV_PIX_FMT_YUVA444P16:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_ABGR:
    case AV_PIX_FMT_RGBA64:
    case AV_PIX_FMT_BGRA64:
    case AV_PIX_FMT_GBRAP:
    case AV_PIX_FMT_GBRAP10:
    case AV_PIX_FMT_GBRAP12:
    case AV_PIX_FMT_GBRAP16:
    case AV_PIX_FMT_YA8:
    case AV_PIX_FMT_YA16:
    case AV_PIX_FMT_NV20:
    case AV_PIX_FMT_VUYA:
    case AV_PIX_FMT_AYUV64:
        return true;
    default:
        return false;
    }
}

static enum AVPixelFormat GetYuvTargetFormat(bool alpha)
{
    return alpha ? AV_PIX_FMT_YUVA420P : AV_PIX_FMT_YUV420P;
}

// Maps the container codec to the codec decoded by the sound gateway.
// AAC/Opus/MP3 have dedicated native decoders; everything else the bundled
// FFmpeg build can decode (AC-3, E-AC-3, DTS, TrueHD, FLAC, Vorbis, PCM, ...)
// goes through the gateway's generic libavcodec + libswresample path.
static ATVMvAudioCodec GetNativeAudioCodec(enum AVCodecID codecId)
{
    switch (codecId)
    {
    case AV_CODEC_ID_AAC:
        return ATVMvCodec_AAC;
    case AV_CODEC_ID_OPUS:
        return ATVMvCodec_Opus;
    case AV_CODEC_ID_MP3:
        return ATVMvCodec_MP3;
    default:
        break;
    }

    // Video-only containers must stay silent; anything libavcodec has a
    // decoder for is worth playing.
    return avcodec_find_decoder(codecId) ? ATVMvCodec_Libav
                                         : ATVMvCodec_Unknown;
}

//////////////////////////////////////////////////////////////////////////
//
// ATVVideoDecoderSmp — single-threaded decoding queue. The engine drives the
// player through VideoPlayer::Decode(); the queue tracks active players and
// reports read-buffer state to the video system.

ATVVideoDecoderSmp::ATVVideoDecoderSmp() : DecodingQueuePaused(false)
{
}

ATVVideoDecoderSmp::~ATVVideoDecoderSmp()
{
    SF_ASSERT(DecodingQueue.GetSize() == 0);
}

int ATVVideoDecoderSmp::FindPlayer(Array<ATVVideoPlayerImpl*>& players, ATVVideoPlayerImpl* pplayer)
{
    for (UPInt i = 0; i < players.GetSize(); ++i)
    {
        if (players[i] == pplayer)
            return (int)i;
    }
    return -1;
}

void ATVVideoDecoderSmp::StartDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    ATVVideoPlayerImpl* pplayer = (ATVVideoPlayerImpl*)pp;
    if (FindPlayer(DecodingQueue, pplayer) == -1)
        DecodingQueue.PushBack(pplayer);
}

void ATVVideoDecoderSmp::StopDecoding(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    ATVVideoPlayerImpl* pplayer = (ATVVideoPlayerImpl*)pp;
    if (FindPlayer(DecodingQueue, pplayer) != -1)
    {
        pplayer->StopInternal();
        ExecuteDecode(pp);
    }
}

bool ATVVideoDecoderSmp::IsDecodingStopped(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    return !ExecuteDecode(pp);
}

bool ATVVideoDecoderSmp::ExecuteDecode(VideoPlayer* pp)
{
    SF_ASSERT(pp);
    ATVVideoPlayerImpl* pplayer = (ATVVideoPlayerImpl*)pp;
    int idx = FindPlayer(DecodingQueue, pplayer);
    if (idx == -1)
        return false;

    if (!pplayer->ExecuteDecode())
    {
        DecodingQueue.RemoveAt(idx);
        return false;
    }
    return true;
}

void ATVVideoDecoderSmp::GetReadBufferInfo(Array<VideoPlayer::ReadBufferInfo>& info)
{
    info.Clear();
    for (UPInt i = 0; i < DecodingQueue.GetSize(); ++i)
    {
        VideoPlayer::ReadBufferInfo buf_info;
        DecodingQueue[i]->GetReadBufferInfo(&buf_info);
        info.PushBack(buf_info);
    }
}

bool ATVVideoDecoderSmp::IsIORequired() const
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

void ATVVideoDecoderSmp::PauseDecoding(bool pause)
{
    if (DecodingQueuePaused && pause)
        return;

    for (UPInt i = 0; i < DecodingQueue.GetSize(); ++i)
    {
        ATVVideoPlayerImpl* pplayer = DecodingQueue[i];
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

ATVVideoPlayerImpl::ATVVideoPlayerImpl(MemoryHeap* pheap) :
    pVideo(NULL), Stat(NotInitialized), StatusAfterStop(Finished),
    pFormatCtx(NULL), pVideoCodecCtx(NULL), pSubtitleCodecCtx(NULL),
    VideoStreamIndex(-1), AudioStreamIndex(-1), SubtitleStreamIndex(-1), SelectedAudioTrack(0),
    pFileOpener(NULL), pAVIO(NULL), pAVIOBuffer(NULL),
    pSwsCtx(NULL), pArgbSws(NULL),
    pFrame(NULL), pYuvFrame(NULL), pPacket(NULL),
    pYuvBuffer(NULL), pArgbScratch(NULL),
    Width(0), Height(0), FrameRateNum(0), FrameRateDen(1),
    VideoTimeBaseNum(1), VideoTimeBaseDen(1000),
    VideoAlpha(false), AudioChannels(0), AudioSampleRate(0), TotalFrames(0),
    MaxSubtitleLen(0), DecodeHeaderTimeout(0),
    StartPos(0), SeekToFrame(0), SeekPending(false), Paused(false), LoopFlag(false),
    StopRequested(false), DecodingEOF(false), FrameReady(false), StagingFrameIndex(0),
    FrameIntervalUs(0), CurrSubtitleChannel(-1),
    StartOpenTicks(0), PlayStartTicks(0), PauseStartTicks(0), LastFrameTicks(0),
    pSound(NULL),
    UpdateTextureCalled(false), FrameOnTime(false), FrameNotInitialized(true), LastFrame(0),
    pGFxHeap(pheap)
{
}

ATVVideoPlayerImpl::~ATVVideoPlayerImpl()
{
    WaitForFinish();
    CloseFFmpeg();

    if (pFrame)
        av_frame_free(&pFrame);
    if (pYuvFrame)
        av_frame_free(&pYuvFrame);
    if (pPacket)
        av_packet_free(&pPacket);
}

bool ATVVideoPlayerImpl::Init(Video* pvideo, TaskManager* ptaskManager,
                           FileOpenerBase* pfileOpener, Log* plog)
{
    SF_UNUSED1(ptaskManager);
    pVideo = pvideo;
    pLog = plog;
    pFileOpener = pfileOpener;

    pDecoder = pvideo && pvideo->GetDecoder() ? pvideo->GetDecoder() : NULL;
    if (!pDecoder)
        pDecoder = *SF_HEAP_NEW(pGFxHeap) ATVVideoDecoderSmp;
    SF_ASSERT(pDecoder);

    pFrame      = av_frame_alloc();
    pYuvFrame   = av_frame_alloc();
    pPacket     = av_packet_alloc();

    Stat = Stopped;
    return true;
}


//////////////////////////////////////////////////////////////////////////
//

void ATVVideoPlayerImpl::Open(const char* url)
{
    WaitForFinish();

    LastFrame = StartPos = 0;
    Stat = Opening;
    StatusAfterStop = Finished;
    Paused = false;
    LoopFlag = false;
    StopRequested = false;
    DecodingEOF = false;
    SeekPending = false;
    FrameReady = false;
    StagingFrameIndex = 0;
    UpdateTextureCalled = false;
    FrameNotInitialized = true;
    FrameOnTime = false;
    CurrSubtitleChannel = -1;

    // Validate the file exists before handing it to FFmpeg.
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

    if (!OpenFFmpeg(url))
    {
        if (pLog)
            pLog->LogError("Failed to open video file with FFmpeg: %s\n", url);
        Stat = InternalError;
        return;
    }

    StartOpenTicks = Timer::GetTicks();
    Stat = Ready;
}

void ATVVideoPlayerImpl::WaitForFinish()
{
    if (Stat != Opening && Stat != Ready && Stat != Starting && Stat != Playing &&
        Stat != Seeking && Stat != Stopping)
        return;

    pDecoder->StopDecoding(this);
    while (!pDecoder->IsDecodingStopped(this))
    {
        Decode();
        Thread::MSleep(1);
    }

    CloseFFmpeg();
}

//////////////////////////////////////////////////////////////////////////
//
// FFmpeg open / close

//////////////////////////////////////////////////////////////////////////
//
// AVIO callbacks - bridge libavformat to the FileOpener-backed File.

int ATVVideoPlayerImpl::AVRead(void* opaque, UByte* buf, int size)
{
    ATVVideoPlayerImpl* p = (ATVVideoPlayerImpl*)opaque;
    if (!p || !p->pFile)
        return -1;
    return p->pFile->Read(buf, size);
}

int64_t ATVVideoPlayerImpl::AVSeek(void* opaque, int64_t offset, int whence)
{
    ATVVideoPlayerImpl* p = (ATVVideoPlayerImpl*)opaque;
    if (!p || !p->pFile)
        return -1;

    switch (whence)
    {
    case AVSEEK_SIZE:
        return p->pFile->LGetLength();
    case SEEK_SET:
        return p->pFile->LSeek(offset, File::Seek_Set);
    case SEEK_CUR:
        return p->pFile->LSeek(offset, File::Seek_Cur);
    case SEEK_END:
        return p->pFile->LSeek(offset, File::Seek_End);
    default:
        return -1;
    }
}

//////////////////////////////////////////////////////////////////////////
//

bool ATVVideoPlayerImpl::OpenFFmpeg(const char* url)
{
    SF_ASSERT(!pFormatCtx);

    // Bound the header scan so that opening large files is fast.
    AVDictionary* opts = NULL;
    av_dict_set(&opts, "probesize", "5000000", 0);
    av_dict_set(&opts, "analyzeduration", "5000000", 0);

    int ret = -1;

    // Prefer a FileOpener-backed reader so that virtual file systems
    // (ZIP archives etc.) are supported through the Scaleform FileOpener.
    // Falls back to plain file I/O through libavformat.
    if (pFileOpener)
    {
        pFile = *pFileOpener->OpenFile(url);
        if (pFile && pFile->IsValid())
        {
            const int avioBufSize = 64 * 1024;
            pAVIOBuffer = (UByte*)av_malloc(avioBufSize);
            if (pAVIOBuffer)
            {
                pAVIO = avio_alloc_context(pAVIOBuffer, avioBufSize, 0, this,
                                           ATVVideoPlayerImpl::AVRead, NULL,
                                           ATVVideoPlayerImpl::AVSeek);
                if (pAVIO)
                {
                    pFormatCtx = avformat_alloc_context();
                    if (pFormatCtx)
                    {
                        pFormatCtx->pb = pAVIO;
                        pFormatCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
                        ret = avformat_open_input(&pFormatCtx, NULL, NULL, &opts);
                    }
                }
            }
        }
    }

    if (ret < 0 || !pFormatCtx)
    {
        // Custom IO unavailable; fall back to standard file I/O.
        if (pFormatCtx)
        {
            avformat_free_context(pFormatCtx);
            pFormatCtx = NULL;
        }
        CloseAVIO();
        ret = avformat_open_input(&pFormatCtx, url, NULL, &opts);
    }
    av_dict_free(&opts);
    if (ret < 0)
        return false;

    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return false;
    }

    // ---- Video stream --------------------------------------------------
    VideoStreamIndex = -1;
    AVStream* vstream = NULL;
    for (unsigned i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        AVStream* st = pFormatCtx->streams[i];
        if (st->codecpar && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            VideoStreamIndex = (int)i;
            vstream = st;
            break;
        }
    }
    if (VideoStreamIndex < 0 || !vstream || !vstream->codecpar)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return false;
    }

    const AVCodec* vcodec = avcodec_find_decoder(vstream->codecpar->codec_id);
    if (!vcodec)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return false;
    }

    pVideoCodecCtx = avcodec_alloc_context3(vcodec);
    if (!pVideoCodecCtx ||
        avcodec_parameters_to_context(pVideoCodecCtx, vstream->codecpar) < 0)
    {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return false;
    }
    pVideoCodecCtx->pkt_timebase = vstream->time_base;
    // H.264 USM files from CRI Mana 2019 are commonly 1080p and need more than
    // the old two decoder threads to stay real-time when decoded by libavcodec.
    {
        int threads = av_cpu_count();
        if (threads < 2)
            threads = 2;
        if (threads > 8)
            threads = 8;
        pVideoCodecCtx->thread_count = threads;
        pVideoCodecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
    }
    if (avcodec_open2(pVideoCodecCtx, vcodec, NULL) < 0)
    {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
        return false;
    }

    Width  = pVideoCodecCtx->width  > 0 ? pVideoCodecCtx->width  : vstream->codecpar->width;
    Height = pVideoCodecCtx->height > 0 ? pVideoCodecCtx->height : vstream->codecpar->height;
    if (Width <= 0 || Height <= 0)
    {
        CloseFFmpeg();
        return false;
    }

    if (vstream->time_base.num > 0 && vstream->time_base.den > 0)
    {
        VideoTimeBaseNum = (unsigned)vstream->time_base.num;
        VideoTimeBaseDen = (unsigned)vstream->time_base.den;
    }
    else
    {
        VideoTimeBaseNum = 1;
        VideoTimeBaseDen = 1000;
    }

    AVRational fps = vstream->avg_frame_rate;
    if (fps.num <= 0 || fps.den <= 0)
        fps = vstream->r_frame_rate;
    if (fps.num <= 0 || fps.den <= 0)
        fps = av_guess_frame_rate(pFormatCtx, vstream, NULL);
    if (fps.num <= 0 || fps.den <= 0)
    {
        fps.num = 1;
        fps.den = (int)VideoTimeBaseDen;
    }
    FrameRateNum = (unsigned)fps.num;
    FrameRateDen = (unsigned)fps.den;
    FrameIntervalUs = FrameRateDen > 0 ?
        (UInt64)(1000000ULL * (UInt64)FrameRateDen / (UInt64)FrameRateNum) : 0;

    VideoAlpha = IsPixFmtAlpha(pVideoCodecCtx->pix_fmt);

    if (vstream->nb_frames > 0)
        TotalFrames = (UInt32)vstream->nb_frames;
    else
    {
        double durSec = 0.0;
        if (vstream->duration > 0)
            durSec = (double)vstream->duration * (double)VideoTimeBaseNum / (double)VideoTimeBaseDen;
        else if (pFormatCtx->duration > 0)
            durSec = (double)pFormatCtx->duration / (double)AV_TIME_BASE;
        TotalFrames = durSec > 0.0 ?
            (UInt32)(durSec * (double)FrameRateNum / (double)FrameRateDen + 0.5) : 0;
    }

    // ---- Pixel conversion ----------------------------------------------
    enum AVPixelFormat dstFmt = GetYuvTargetFormat(VideoAlpha);
    pSwsCtx = sws_getContext(Width, Height, pVideoCodecCtx->pix_fmt,
                             Width, Height, dstFmt, SWS_BILINEAR, NULL, NULL, NULL);
    if (!pSwsCtx)
    {
        CloseFFmpeg();
        return false;
    }

    int yuvSize = av_image_get_buffer_size(dstFmt, Width, Height, 32);
    if (yuvSize <= 0)
    {
        CloseFFmpeg();
        return false;
    }
    pYuvBuffer = (UByte*)SF_HEAP_MEMALIGN(pGFxHeap, (UPInt)yuvSize, 32, Stat_Video_Mem);
    if (!pYuvBuffer ||
        av_image_fill_arrays(pYuvFrame->data, pYuvFrame->linesize, pYuvBuffer,
                             dstFmt, Width, Height, 32) < 0)
    {
        CloseFFmpeg();
        return false;
    }

    // ---- Audio stream --------------------------------------------------
    BuildAudioStreamList();
    AudioStreamIndex = -1;
    ATV_TRACE("audio: %u audio stream(s)\n", (unsigned)AudioStreams.GetSize());
    if (AudioStreams.GetSize() > 0)
    {
        SelectedAudioTrack = 0;
        AudioStreamIndex = AudioStreams[0];
        if (SetupAudioStream(AudioStreamIndex))
        {
            ATV_TRACE("audio: stream=%d ch=%u rate=%u native=%p\n",
                      AudioStreamIndex, AudioChannels, AudioSampleRate,
                      (void*)pSound);
        }
        else
            AudioStreamIndex = -1;
    }

    // ---- Subtitle stream ----------------------------------------------
    SubtitleStreamIndex = -1;
    for (unsigned i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        AVStream* st = pFormatCtx->streams[i];
        if (st->codecpar && st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
        {
            SubtitleStreamIndex = (int)i;
            break;
        }
    }
    if (SubtitleStreamIndex >= 0)
        SetupSubtitleStream(SubtitleStreamIndex);

    return true;
}

void ATVVideoPlayerImpl::CloseAVIO()
{
    // avio_alloc_context() keeps the caller's buffer, but libavformat may
    // replace it via ffio_ensure_seekback() (which av_free()s the old one).
    // The AVIOContext's current buffer is therefore the only pointer that is
    // safe to release; pAVIOBuffer may already be dangling.
    if (pAVIO)
    {
        UByte* pbuf = pAVIO->buffer;
        avio_context_free(&pAVIO);
        pAVIO = NULL;
        if (pbuf)
            av_free(pbuf);
    }
    else if (pAVIOBuffer)
    {
        av_free(pAVIOBuffer);
    }
    pAVIOBuffer = NULL;
}

void ATVVideoPlayerImpl::CloseFFmpeg()
{
    ClearPendingVideoPackets();

    if (pSound)
    {
        delete pSound;
        pSound = NULL;
    }

    if (pSubtitleCodecCtx)
    {
        avcodec_free_context(&pSubtitleCodecCtx);
        pSubtitleCodecCtx = NULL;
    }
    if (pVideoCodecCtx)
    {
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
    }
    if (pSwsCtx)
    {
        sws_freeContext(pSwsCtx);
        pSwsCtx = NULL;
    }
    if (pArgbSws)
    {
        sws_freeContext(pArgbSws);
        pArgbSws = NULL;
    }
    if (pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
    if (pAVIO || pAVIOBuffer)
        CloseAVIO();
    if (pFile)
    {
        pFile->Close();
        pFile = NULL;
    }

    if (pYuvBuffer)
    {
        SF_FREE(pYuvBuffer);
        pYuvBuffer = NULL;
    }
    if (pArgbScratch)
    {
        SF_FREE(pArgbScratch);
        pArgbScratch = NULL;
    }

    if (pYuvFrame)
    {
        Alg::MemUtil::Set(pYuvFrame->data, 0, sizeof(pYuvFrame->data));
        Alg::MemUtil::Set(pYuvFrame->linesize, 0, sizeof(pYuvFrame->linesize));
    }

    VideoStreamIndex = AudioStreamIndex = SubtitleStreamIndex = -1;
    AudioStreams.Clear();
    AudioStreamChannels.Clear();
    AudioStreamRates.Clear();
    SelectedAudioTrack = 0;
    Width = Height = 0;
    FrameRateNum = 0;
    FrameRateDen = 1;
    VideoTimeBaseNum = 1;
    VideoTimeBaseDen = 1000;
    VideoAlpha = false;
    AudioChannels = AudioSampleRate = 0;
    TotalFrames = 0;
    MaxSubtitleLen = 0;
    SubtitleText.Clear();
}

bool ATVVideoPlayerImpl::SetupAudioStream(int streamIndex)
{
    if (!pFormatCtx || streamIndex < 0)
        return false;
    AVStream* astream = pFormatCtx->streams[streamIndex];
    if (!astream || !astream->codecpar)
        return false;

    ATVMvAudioCodec codec = GetNativeAudioCodec(astream->codecpar->codec_id);
    if (codec == ATVMvCodec_Unknown)
    {
        ATV_TRACE("audio: stream=%d codec=%s (%d) has no decoder\n",
                  streamIndex, avcodec_get_name(astream->codecpar->codec_id),
                  (int)astream->codecpar->codec_id);
        return false;
    }

    AudioChannels = (unsigned)astream->codecpar->ch_layout.nb_channels;
    if (AudioChannels == 0)
        AudioChannels = 2;
    if (AudioChannels > (unsigned)VideoSound::MaxChannels)
        AudioChannels = (unsigned)VideoSound::MaxChannels;
    AudioSampleRate = astream->codecpar->sample_rate > 0
        ? (unsigned)astream->codecpar->sample_rate : 44100;

    if (pSound)
    {
        delete pSound;
        pSound = NULL;
    }

    const UInt32 codecTag = (UInt32)astream->codecpar->codec_id;

    pSound = new ATVMvSoundGateway(pGFxHeap);
    bool opened = pSound->Open(codec, codecTag,
                               astream->codecpar->extradata,
                               astream->codecpar->extradata_size,
                               AudioChannels, AudioSampleRate);
    if (!opened && codec != ATVMvCodec_Libav)
    {
        // The dedicated decoder refused the stream (an AAC profile fdk cannot
        // handle, an OpusHead without a mapping table, ...). libavcodec decodes
        // those variants too.
        opened = pSound->Open(ATVMvCodec_Libav, codecTag,
                              astream->codecpar->extradata,
                              astream->codecpar->extradata_size,
                              AudioChannels, AudioSampleRate);
    }
    if (!opened)
    {
        ATV_TRACE("audio: stream=%d codec=%s has no usable decoder\n",
                  streamIndex, avcodec_get_name(astream->codecpar->codec_id));
        delete pSound;
        pSound = NULL;
        return false;
    }

    AudioChannels   = pSound->GetChannels();
    AudioSampleRate = pSound->GetSampleRate();
    return true;
}

void ATVVideoPlayerImpl::SetupSubtitleStream(int streamIndex)
{
    if (!pFormatCtx || streamIndex < 0)
        return;
    AVStream* sstream = pFormatCtx->streams[streamIndex];
    if (!sstream || !sstream->codecpar)
    {
        SubtitleStreamIndex = -1;
        return;
    }

    const AVCodec* codec = avcodec_find_decoder(sstream->codecpar->codec_id);
    if (!codec)
    {
        SubtitleStreamIndex = -1;
        return;
    }

    pSubtitleCodecCtx = avcodec_alloc_context3(codec);
    if (!pSubtitleCodecCtx ||
        avcodec_parameters_to_context(pSubtitleCodecCtx, sstream->codecpar) < 0 ||
        avcodec_open2(pSubtitleCodecCtx, codec, NULL) < 0)
    {
        avcodec_free_context(&pSubtitleCodecCtx);
        pSubtitleCodecCtx = NULL;
        SubtitleStreamIndex = -1;
        return;
    }
    MaxSubtitleLen = 4096;
}

void ATVVideoPlayerImpl::BuildAudioStreamList()
{
    AudioStreams.Clear();
    AudioStreamChannels.Clear();
    AudioStreamRates.Clear();
    if (!pFormatCtx)
        return;

    for (unsigned i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        AVStream* st = pFormatCtx->streams[i];
        if (st->codecpar && st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            AudioStreams.PushBack((int)i);
            unsigned nch = (unsigned)st->codecpar->ch_layout.nb_channels;
            unsigned rate = (unsigned)(st->codecpar->sample_rate > 0 ? st->codecpar->sample_rate : 0);
            AudioStreamChannels.PushBack(nch > (unsigned)VideoSound::MaxChannels ?
                                         (unsigned)VideoSound::MaxChannels : nch);
            AudioStreamRates.PushBack(rate);
        }
    }
}

void ATVVideoPlayerImpl::FlushDecoders()
{
    if (pVideoCodecCtx)
        avcodec_flush_buffers(pVideoCodecCtx);
    if (pSubtitleCodecCtx)
        avcodec_flush_buffers(pSubtitleCodecCtx);
}


//////////////////////////////////////////////////////////////////////////
//
// Decode pipeline

bool ATVVideoPlayerImpl::ExecuteDecode()
{
    if (StopRequested || !pFormatCtx || !pVideoCodecCtx)
        return false;
    if (Paused)
        return true;

    // Pace from the start of the decode step, not from its end: measuring the
    // interval after avcodec_receive_frame() adds the decode time to every
    // frame period and caps 60 fps content at about 54 fps.
    //
    // Advance the schedule by exactly one frame interval instead of re-anchoring it at
    // "now": the player loop only polls every few milliseconds, so "now - last >= 16.7ms"
    // fires one poll late and the error compounds - a 16.7 ms period stretches to 20-33 ms
    // and 60 fps content plays at ~43 fps. Re-anchor only when more than two frames behind.
    {
        const UInt64 now = Timer::GetTicks();
        if (FrameIntervalUs > 0 && LastFrameTicks != 0 &&
            now > LastFrameTicks && (now - LastFrameTicks) < FrameIntervalUs * 2)
            LastFrameTicks += FrameIntervalUs;
        else
            LastFrameTicks = now;
    }

    for (;;)
    {
        int ret = avcodec_receive_frame(pVideoCodecCtx, pFrame);
        if (ret == 0)
        {
            OnVideoFrameDecoded(pFrame);
            av_frame_unref(pFrame);
            return true;
        }
        if (ret == AVERROR(EAGAIN))
        {
            if (!FeedVideoPacket())
            {
                // EOF reached: drain whatever the decoder still has buffered.
                ret = avcodec_receive_frame(pVideoCodecCtx, pFrame);
                if (ret == 0)
                {
                    OnVideoFrameDecoded(pFrame);
                    av_frame_unref(pFrame);
                    return true;
                }
                break;
            }
            continue;
        }
        break;   // AVERROR_EOF or a decoder error
    }

    // Decoder drained (end of stream).
    if (LoopFlag && TotalFrames > 0)
    {
        RestartFromBeginning();
        return true;
    }
    return false;
}

bool ATVVideoPlayerImpl::FeedVideoPacket()
{
    if (!pFormatCtx || !pPacket)
        return false;

    // No queued packet left: pump the demuxer once and retry.
    PumpPackets();

    if (PendingVideoPackets.GetSize() == 0)
        return false;

    AVPacket* pqueued = PendingVideoPackets[0];
    PendingVideoPackets.RemoveAt(0);

    int sret = (pqueued && pVideoCodecCtx) ? avcodec_send_packet(pVideoCodecCtx, pqueued)
                                           : AVERROR_INVALIDDATA;
    if (pqueued)
        av_packet_free(&pqueued);
    return sret >= 0;
}

bool ATVVideoPlayerImpl::PumpPackets()
{
    if (!pFormatCtx || !pPacket || DecodingEOF)
        return false;

    // Audio is paced by the native sound gateway, not by the video frame
    // slot: demux until the gateway has enough compressed packets queued,
    // keeping the video packets that come along in a bounded queue for
    // FeedVideoPacket.
    const UInt32 audioTarget   = 24;
    const UPInt  videoQueueLow = 8;
    bool any = false;

    for (;;)
    {
        if (PendingVideoPackets.GetSize() >= (UPInt)MaxPendingVideoPackets)
            break;
        if (any && pSound && pSound->GetQueuedPackets() >= audioTarget &&
            PendingVideoPackets.GetSize() >= videoQueueLow)
            break;

        int ret = av_read_frame(pFormatCtx, pPacket);
        if (ret == AVERROR_EOF || ret < 0)
        {
            DecodingEOF = true;
            break;
        }

        if (pPacket->stream_index == VideoStreamIndex)
        {
            AVPacket* pclone = av_packet_alloc();
            if (pclone && av_packet_ref(pclone, pPacket) == 0)
                PendingVideoPackets.PushBack(pclone);
            else if (pclone)
                av_packet_free(&pclone);
        }
        else if (pSound && pPacket->stream_index == AudioStreamIndex)
        {
            pSound->PushPacket(pPacket->data, pPacket->size);
        }
        else if (pSubtitleCodecCtx && pPacket->stream_index == SubtitleStreamIndex)
        {
            DecodeSubtitlePacket(pPacket);
        }

        av_packet_unref(pPacket);
        any = true;
    }

    return any;
}

void ATVVideoPlayerImpl::ClearPendingVideoPackets()
{
    for (UPInt i = 0; i < PendingVideoPackets.GetSize(); ++i)
    {
        AVPacket* pkt = PendingVideoPackets[i];
        if (pkt)
            av_packet_free(&pkt);
    }
    PendingVideoPackets.Clear();
}

void ATVVideoPlayerImpl::OnVideoFrameDecoded(AVFrame* pframe)
{
    if (!pframe || !pframe->data[0])
        return;

    if (SeekPending)
    {
        UInt32 idx = FrameIndexFromPts(pframe->pts);
        if (idx < SeekToFrame)
            return;   // skip frames before the seek target (fast seek)
        SeekPending = false;
    }

    Lock::Locker lock(&FrameLock);
    if (!pSwsCtx || !pYuvFrame)
        return;

    sws_scale(pSwsCtx, pframe->data, pframe->linesize, 0, Height,
              pYuvFrame->data, pYuvFrame->linesize);
    StagingFrameIndex = FrameIndexFromPts(pframe->pts);
    FrameReady = true;

#if defined(SF_BUILD_DEBUG)
    {
        static UInt64 fpsStart = 0;
        static int    fpsFrames = 0;
        static int    syncFrames = 0;
        UInt64 now = Timer::GetTicks();
        if (fpsStart == 0)
            fpsStart = now;
        ++fpsFrames;
        if (now - fpsStart >= 1000000)
        {
            ATV_TRACE("video fps=%.1f\n",
                      (double)fpsFrames * 1000000.0 / (double)(now - fpsStart));
            fpsFrames = 0;
            fpsStart = now;
        }
        if (++syncFrames % 30 == 0 && FrameRateNum)
        {
            UInt64 c = 0, u = 1;
            GetPlayTime(&c, &u);
            double clock = u ? (double)c / (double)u : 0.0;
            double sys = (double)(now - PlayStartTicks) / 1000000.0;
            double pts = (double)StagingFrameIndex * (double)FrameRateDen /
                         (double)FrameRateNum;
            ATV_TRACE("sync frame=%u pts=%.3f clock=%.3f sys=%.3f\n",
                      StagingFrameIndex, pts, clock, sys);
        }
    }
#endif
}

void ATVVideoPlayerImpl::DecodeSubtitlePacket(AVPacket* pkt)
{
    if (!pSubtitleCodecCtx || !pkt)
        return;

    AVSubtitle sub;
    Alg::MemUtil::Set(&sub, 0, sizeof(sub));
    int gotSubtitle = 0;
    if (avcodec_decode_subtitle2(pSubtitleCodecCtx, &sub, &gotSubtitle, pkt) >= 0 &&
        gotSubtitle != 0)
    {
        if (sub.num_rects > 0 && sub.rects[0])
        {
            const char* text = sub.rects[0]->text;
            if (!text)
                text = sub.rects[0]->ass;
            if (text)
                SubtitleText = text;
            else
                SubtitleText.Clear();
        }
        avsubtitle_free(&sub);
    }
}

//////////////////////////////////////////////////////////////////////////
//
// VideoPlayer interface

void ATVVideoPlayerImpl::GetVideoInfo(VideoInfo* pinfo)
{
    SF_ASSERT(pinfo);

    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        pinfo->Width       = Width;
        pinfo->Height      = Height;
        pinfo->FrameRate   = FrameRateDen > 0 ?
            (FrameRateNum + FrameRateDen / 2) / FrameRateDen : 0;
        pinfo->AudioDataRate = AudioSampleRate;
        pinfo->TotalFrames = TotalFrames;
        pinfo->SubtitleChannelsNumber = SubtitleStreamIndex >= 0 ? 1 : 0;
        pinfo->MaxSubtitleLen = MaxSubtitleLen;

        if (pinfo->AudioTracks.GetSize() == 0)
        {
            for (UPInt i = 0; i < AudioStreams.GetSize(); ++i)
            {
                AudioTrackInfo ati;
                ati.Index = (int)i;
                ati.ChannelsNumber = AudioStreamChannels[i];
                ati.SampleRate     = AudioStreamRates[i];
                ati.TotalSamples   = 0;
                pinfo->AudioTracks.PushBack(ati);
            }
        }
    }
}

void ATVVideoPlayerImpl::Play()
{
    if (Stat == Ready)
    {
        DecodingEOF = false;
        StopRequested = false;
        FrameReady = false;
        FrameOnTime = false;
        SeekPending = false;
        if (pSound)
            pSound->Flush();
        FlushDecoders();

        if (StartPos > 0)
            DoSeek(StartPos);

        pDecoder->StartDecoding(this);
        if (pSound)
            pSound->Start();
        Stat = Starting;
    }
}

void ATVVideoPlayerImpl::Stop()
{
    if (Stat == Opening || Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        Stat = Stopping;
        StatusAfterStop = Stopped;
        pDecoder->StopDecoding(this);
        StopRequested = true;
        if (pSound)
            pSound->Stop();
        if (pSyncObj)
            pSyncObj->Stop();
    }
}

void ATVVideoPlayerImpl::Pause(bool on_off)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        if (Paused == on_off)
            return;
        Paused = on_off;
        if (pSound)
            pSound->Pause(on_off);
        if (pSyncObj)
            pSyncObj->Pause(on_off);
        if (on_off)
        {
            PauseStartTicks = Timer::GetTicks();
        }
        else
        {
            // Compensate the paused interval in the play clock.
            PlayStartTicks += Timer::GetTicks() - PauseStartTicks;
            LastFrameTicks = Timer::GetTicks();
        }
    }
}

void ATVVideoPlayerImpl::Seek(UInt32 pos)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        StartPos = TotalFrames > 0 ? pos % TotalFrames : pos;
        Stat = Seeking;
        DoSeek(StartPos);
    }
    else if (Stat == Finished)
    {
        StartPos = TotalFrames > 0 ? pos % TotalFrames : pos;
        Stat = Ready;
        Play();
    }
}

void ATVVideoPlayerImpl::DoSeek(UInt32 pos)
{
    if (!pFormatCtx || VideoStreamIndex < 0)
        return;

    FlushDecoders();
    ClearPendingVideoPackets();
    int64_t target = RescaleFrameToPts(pos);
    av_seek_frame(pFormatCtx, VideoStreamIndex, target, AVSEEK_FLAG_BACKWARD);

    DecodingEOF = false;
    SeekPending = true;
    SeekToFrame = pos;
    FrameReady = false;
    FrameOnTime = false;
    if (pSound)
        pSound->Flush();
    LastFrame = pos;
    if (pSyncObj)
        pSyncObj->SetStartFrame(pos);
}

void ATVVideoPlayerImpl::RestartFromBeginning()
{
    if (!pFormatCtx || VideoStreamIndex < 0)
        return;

    FlushDecoders();
    ClearPendingVideoPackets();
    av_seek_frame(pFormatCtx, VideoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);

    DecodingEOF = false;
    SeekPending = false;
    FrameReady = false;
    FrameOnTime = false;
    if (pSound)
        pSound->Flush();
    LastFrame = 0;
    if (pSyncObj)
        pSyncObj->SetStartFrame(0);
}

UInt32 ATVVideoPlayerImpl::GetPosition()
{
    if (Stat == Finished || Stat == Stopped)
        return 0;
    return TotalFrames > 0 ? LastFrame % TotalFrames : LastFrame;
}

void ATVVideoPlayerImpl::SetSubtitleChannel(int channel)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        if (channel < 0)
        {
            CurrSubtitleChannel = -1;
        }
        else if (SubtitleStreamIndex >= 0 && channel == 0)
        {
            CurrSubtitleChannel = 0;
        }
        // Out-of-range channel numbers are ignored.
    }
}

int ATVVideoPlayerImpl::GetSubtitleChannel()
{
    return CurrSubtitleChannel;
}

void ATVVideoPlayerImpl::Decode()
{
    if (Stat != Opening && Stat != Ready && Stat != Starting && Stat != Playing &&
        Stat != Seeking && Stat != Stopping)
        return;

    if (Stat == Opening)
    {
        CheckHeaderDecoding();
        return;
    }

    if (Stat == Ready || Stat == Starting || Stat == Playing || Stat == Seeking)
    {
        if (Paused)
            return;

        if (Stat == Starting)
        {
            PlayStartTicks = Timer::GetTicks();
            LastFrameTicks = PlayStartTicks;
            if (pSyncObj)
            {
                pSyncObj->SetStartFrame(StartPos);
                pSyncObj->Start();
            }
            Stat = Playing;
        }

        if (Stat == Playing)
        {
            // Keep the audio FIFO topped up with demuxed (and decoded) audio
            // even while the video decoder is waiting for its next frame slot.
            if (!SeekPending)
                PumpPackets();

            // Mark the staged frame on-time when its presentation time is due.
            if (FrameReady && !FrameOnTime && IsFrameOnTime(StagingFrameIndex))
            {
                FrameOnTime = true;
            }

            // Pace decoding: pull the next frame only when the previous one
            // was consumed by the texture and the frame interval elapsed.
            bool canDecode = !FrameReady;
            if (canDecode && !SeekPending && FrameIntervalUs > 0)
            {
                UInt64 now = Timer::GetTicks();
                canDecode = (now - LastFrameTicks) >= FrameIntervalUs;
            }

            if (canDecode)
            {
                if (ExecuteDecode())
                {
                    if (FrameReady && !FrameOnTime && IsFrameOnTime(StagingFrameIndex))
                    {
                        FrameOnTime = true;
                    }

                    // If the render/task loop is slower than the video frame
                    // rate, decoding one frame per tick makes the picture run
                    // in slow motion (PTS falls behind the audio clock). Drop
                    // already-late frames until the staged frame is current.
                    if (FrameReady)
                    {
                        UInt64 c = 0, u = 1;
                        GetPlayTime(&c, &u);
                        double playSec = u ? (double)c / (double)u : 0.0;
                        double frameDur = FrameRateNum ?
                            (double)FrameRateDen / (double)FrameRateNum : 0.0;
                        int guard = 0;
                        while (FrameReady && frameDur > 0.0 && guard++ < 8 &&
                               (double)StagingFrameIndex * frameDur + frameDur * 1.5 < playSec)
                        {
                            if (!ExecuteDecode())
                                break;
                        }
                    }
                }
                else
                {
                    if (Stat == Stopping)
                        Stat = StatusAfterStop;
                    else
                        Stat = Finished;
                }
            }
        }
        else if (Stat == Seeking)
        {
            // Fast-forward decoding until the seek target frame is staged.
            bool more = true;
            int guard = 0;
            while (more && SeekPending && guard++ < 64)
                more = ExecuteDecode();

            if (!SeekPending)
            {
                Stat = Playing;
                LastFrameTicks = Timer::GetTicks();
                if (FrameReady && IsFrameOnTime(StagingFrameIndex))
                    FrameOnTime = true;
                if (pSyncObj)
                    pSyncObj->Start();
            }
            else if (!more)
            {
                if (LoopFlag && TotalFrames > 0)
                {
                    RestartFromBeginning();
                    Stat = Playing;
                }
                else
                    Stat = Finished;
            }
        }
    }
    else if (Stat == Stopping)
    {
        if (pDecoder->IsDecodingStopped(this))
            Stat = StatusAfterStop;
    }
}

void ATVVideoPlayerImpl::CheckHeaderDecoding()
{
    // FFmpeg header decoding is synchronous and completed inside Open();
    // this method is kept for interface parity.
    if (Stat == Opening)
        Stat = (pFormatCtx != NULL) ? Ready : InternalError;
}

void ATVVideoPlayerImpl::SetDecodeHeaderTimeout(float timeout)
{
    DecodeHeaderTimeout = timeout > 0 ? (UInt64)(timeout * 1000 * 1000) : 0;
}

void ATVVideoPlayerImpl::SetSyncObject(VideoPlayer::SyncObject* psync)
{
    pSyncObj = psync;
}

void ATVVideoPlayerImpl::GetCurrentCuePoints(Array<CuePoint>* cuePoints)
{
    SF_ASSERT(cuePoints);
    // FFmpeg containers carry no cue points; keep the list empty.
    cuePoints->Clear();
}

void ATVVideoPlayerImpl::SetAudioTrack(int trackIndex)
{
    if (Stat == Ready || Stat == Starting || Stat == Playing)
    {
        if (trackIndex < 0 || (UPInt)trackIndex >= AudioStreams.GetSize())
            return;
        if (SelectedAudioTrack == (unsigned)trackIndex)
            return;

        SelectedAudioTrack = (unsigned)trackIndex;
        AudioStreamIndex = AudioStreams[trackIndex];

        if (pSound)
        {
            pSound->Stop();
            delete pSound;
            pSound = NULL;
        }

        if (!SetupAudioStream(AudioStreamIndex))
        {
            AudioStreamIndex = -1;
            return;
        }
        if (pSound && (Stat == Starting || Stat == Playing))
            pSound->Start();

        StartPos = TotalFrames > 0 ? LastFrame % TotalFrames : LastFrame;
        if (Stat != Ready)
        {
            Stat = Seeking;
            DoSeek(StartPos);
        }
    }
}

void ATVVideoPlayerImpl::SetSubAudioTrack(int trackIndex)
{
    SF_UNUSED(trackIndex);
    // Sub-audio requires a second decode chain; not supported in this
    // integration (mirrors the Sofdec2 behavior).
}

void ATVVideoPlayerImpl::ReplaceCenterVoice(int trackIndex)
{
    SF_UNUSED(trackIndex);
    // No direct FFmpeg equivalent for 5.1 center-channel replacement.
}

void ATVVideoPlayerImpl::SetLoopFlag(bool flag)
{
    LoopFlag = flag;
}

void ATVVideoPlayerImpl::SetBufferTime(float time)
{
    SF_UNUSED(time);
    // Buffering is owned by the native sound gateway (compressed-packet
    // queue + SDL2 audio device); there is no PCM FIFO to resize.
}

void ATVVideoPlayerImpl::SetNumberOfFramePools(unsigned pools)
{
    SF_UNUSED(pools);
    // The FFMpeg backend stages a single frame; frame pooling is not used.
}

void ATVVideoPlayerImpl::SetReloadThresholdTime(float time)
{
    SF_UNUSED(time);
    // Buffering is controlled through SetBufferTime.
}

void ATVVideoPlayerImpl::GetReadBufferInfo(ReadBufferInfo* info)
{
    SF_ASSERT(info);
    if (pSound)
    {
        // Report the compressed-packet queue. BufferSize is an approximation
        // in bytes (the gateway stores raw packets, not a PCM FIFO).
        UInt32 queued = pSound->GetQueuedPackets();
        info->BufferSize      = 96 * 4096;
        info->DataSize        = queued * 4096;
        info->ReloadThreshold = info->BufferSize / 4;
    }
    else
    {
        info->BufferSize      = 0;
        info->DataSize        = 0;
        info->ReloadThreshold = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//

ATVVideoImage* ATVVideoPlayerImpl::CreateTexture(Render::TextureManager* ptexman)
{
    if (Stat != Ready && Stat != Starting && Stat != Playing && Stat != Seeking)
        return NULL;
    if (!ptexman)
        return NULL;

    FrameNotInitialized = true;

    ImageFormat format = IsAlphaVideo() ? Image_Y8_U2_V2_A8 : Image_Y8_U2_V2;
    ImageSize imageSize(Width, Height);
    ATVVideoImage* pimage = SF_HEAP_NEW(pGFxHeap) ATVVideoImage(format, imageSize, ptexman, this);
    SF_ASSERT(pimage);

    return pimage;
}

void ATVVideoPlayerImpl::UpdateTexture(VideoImage* pimage, char* subtitle, int subtitleLength)
{
    if (Stat != Ready && Stat != Starting && Stat != Playing && Stat != Seeking && Stat != Stopping)
        return;
    if (!pimage)
        return;

    FrameOnTime = false;

    if (!UpdateTextureCalled)
    {
        UpdateTextureCalled = true;
        pimage->Update();
    }

    if (subtitle && subtitleLength > 0 && CurrSubtitleChannel >= 0)
    {
        if ((Stat == Playing && !IsPaused()) || Stat == Seeking)
        {
            SFstrncpy(subtitle, (UPInt)subtitleLength, SubtitleText.ToCStr(),
                      (UPInt)subtitleLength - 1);
            subtitle[subtitleLength - 1] = 0;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Texture data handoff (called by ATVVideoImage::Decode on the render thread)

bool ATVVideoPlayerImpl::CopyFrameData(ImageData* pdest)
{
    Lock::Locker lock(&FrameLock);
    if (!pYuvFrame || !FrameReady)
        return false;

    if (pdest->GetPlaneCount() >= 3)
    {
        ImagePlane y, u, v, a;
        pdest->GetPlane(0, &y);
        pdest->GetPlane(1, &u);
        pdest->GetPlane(2, &v);
        CopyPlaneData(y, pYuvFrame->data[0], pYuvFrame->linesize[0]);
        CopyPlaneData(u, pYuvFrame->data[1], pYuvFrame->linesize[1]);
        CopyPlaneData(v, pYuvFrame->data[2], pYuvFrame->linesize[2]);
        if (pdest->GetPlaneCount() >= 4 && VideoAlpha && pYuvFrame->data[3])
        {
            pdest->GetPlane(3, &a);
            CopyPlaneData(a, pYuvFrame->data[3], pYuvFrame->linesize[3]);
        }
    }
    else
    {
        ImagePlane argb;
        pdest->GetPlane(0, &argb);
        CopyPlaneDataARGB(argb);
    }

    LastFrame = StagingFrameIndex;
    FrameReady = false;
    FrameOnTime = true;
    return true;
}

void ATVVideoPlayerImpl::CopyPlaneData(const ImagePlane& plane, const UByte* src, int srcPitch) const
{
    if (!plane.pData || !src)
        return;

    for (UInt32 y = 0; y < plane.Height; ++y)
        Alg::MemUtil::Copy(plane.pData + (UPInt)y * plane.Pitch,
                           src + (UPInt)y * srcPitch, plane.Width);
}

void ATVVideoPlayerImpl::CopyPlaneDataARGB(const ImagePlane& argb)
{
    if (!pYuvFrame)
        return;

    enum AVPixelFormat srcFmt = GetYuvTargetFormat(VideoAlpha);
    pArgbSws = sws_getCachedContext(pArgbSws, Width, Height, srcFmt,
                                    Width, Height, AV_PIX_FMT_BGRA,
                                    SWS_BILINEAR, NULL, NULL, NULL);
    if (!pArgbSws)
        return;

    if (!pArgbScratch)
        pArgbScratch = (UByte*)SF_HEAP_ALLOC(pGFxHeap,
            (UPInt)Width * (UPInt)Height * 4, Stat_Video_Mem);
    if (!pArgbScratch)
        return;

    int linesize = Width * 4;
    UByte* dst[4] = { pArgbScratch, NULL, NULL, NULL };
    int dstStride[4] = { linesize, 0, 0, 0 };

    sws_scale(pArgbSws, pYuvFrame->data, pYuvFrame->linesize, 0, Height, dst, dstStride);

    UInt32 copyH = (UInt32)Height;
    if (copyH > argb.Height)
        copyH = argb.Height;
    for (UInt32 y = 0; y < copyH; ++y)
        Alg::MemUtil::Copy(argb.pData + (UPInt)y * argb.Pitch,
                           pArgbScratch + (UPInt)y * linesize, (UPInt)linesize);
}


//////////////////////////////////////////////////////////////////////////
//
// Clock helpers

void ATVVideoPlayerImpl::GetPlayTime(UInt64* count, UInt64* unit)
{
    // The FFmpeg backend drives video from the audio presentation clock. The
    // SWF sync object is frame based: when the SWF render loop slows down or
    // the internal video SWF changes its frame rate, that clock drifts away
    // from the audio device and the picture slowly desynchronizes.
    if (pSound && pSound->HasOutput())
    {
        double pos = pSound->GetPosition();
        *count = (UInt64)(pos * 1000000.0);
        *unit  = 1000000;
        return;
    }

    *count = Timer::GetTicks() - PlayStartTicks;
    *unit  = 1000000;   // Timer ticks are microseconds
}

bool ATVVideoPlayerImpl::IsFrameOnTime(UInt32 frameIndex)
{
    if (FrameRateNum == 0 || FrameRateDen == 0)
        return true;

    UInt64 count = 0, unit = 1;
    GetPlayTime(&count, &unit);
    if (unit == 0)
        unit = 1;

    double frameSec = (double)frameIndex * (double)FrameRateDen / (double)FrameRateNum;
    double playSec  = (double)count / (double)unit;
    double tolerance = (double)FrameRateDen / (2.0 * (double)FrameRateNum);
    return frameSec <= playSec + tolerance;
}

UInt32 ATVVideoPlayerImpl::FrameIndexFromPts(int64_t pts) const
{
    if (pts == AV_NOPTS_VALUE || pts < 0)
        return LastFrame;
    if (VideoTimeBaseDen == 0)
        return 0;

    double sec = (double)pts * (double)VideoTimeBaseNum / (double)VideoTimeBaseDen;
    if (FrameRateNum == 0 || FrameRateDen == 0)
        return sec > 0 ? (UInt32)sec : 0;

    double idx = sec * (double)FrameRateNum / (double)FrameRateDen;
    return idx > 0 ? (UInt32)(idx + 0.5) : 0;
}

int64_t ATVVideoPlayerImpl::RescaleFrameToPts(UInt32 pos) const
{
    if (FrameRateNum == 0 || FrameRateDen == 0 || VideoTimeBaseDen == 0)
        return (int64_t)pos;

    double sec = (double)pos * (double)FrameRateDen / (double)FrameRateNum;
    double pts = sec * (double)VideoTimeBaseDen / (double)VideoTimeBaseNum;
    return (int64_t)pts;
}


//////////////////////////////////////////////////////////////////////////
//
// Sound bridging

#ifdef GFX_ENABLE_SOUND

Sound::SoundChannel* ATVVideoPlayerImpl::GetSoundChannel(SoundTrack track)
{
    SF_UNUSED(track);
    // The native gateway talks to SDL2 audio directly and does not expose a
    // GFx SoundChannel; SWF-side spatial audio is not applicable anymore.
    return NULL;
}

void ATVVideoPlayerImpl::SetSoundSpatialInfo(Array<Sound::SoundChannel::Vector> spatinfo[])
{
    SF_UNUSED(spatinfo);
}

#endif // GFX_ENABLE_SOUND


//////////////////////////////////////////////////////////////////////////
//
// Static header probe: reads a video file header without playing it.

bool VideoPlayer::LoadVideoInfo(const char* pfilename, VideoPlayer::VideoInfo* pinfo,
                                FileOpenerBase* popener)
{
    SF_UNUSED(popener);
    if (!pinfo)
        return false;

    bool ret = false;

    AVDictionary* opts = NULL;
    av_dict_set(&opts, "probesize", "5000000", 0);
    av_dict_set(&opts, "analyzeduration", "5000000", 0);

    AVFormatContext* pfmt = NULL;
    if (avformat_open_input(&pfmt, pfilename, NULL, &opts) == 0)
    {
        av_dict_free(&opts);
        if (avformat_find_stream_info(pfmt, NULL) >= 0)
        {
            AVStream* vstream = NULL;
            for (unsigned i = 0; i < pfmt->nb_streams; ++i)
            {
                if (pfmt->streams[i]->codecpar &&
                    pfmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    vstream = pfmt->streams[i];
                    break;
                }
            }

            if (vstream && vstream->codecpar)
            {
                pinfo->Width  = vstream->codecpar->width;
                pinfo->Height = vstream->codecpar->height;

                AVRational fps = vstream->avg_frame_rate;
                if (fps.num <= 0 || fps.den <= 0)
                    fps = vstream->r_frame_rate;
                if (fps.num <= 0 || fps.den <= 0)
                    fps = av_guess_frame_rate(pfmt, vstream, NULL);
                if (fps.num <= 0 || fps.den <= 0)
                {
                    fps.num = 1;
                    fps.den = vstream->time_base.den > 0 ? vstream->time_base.den : 1;
                }
                pinfo->FrameRate = (UInt32)((fps.num + fps.den / 2) / fps.den);

                if (vstream->nb_frames > 0)
                    pinfo->TotalFrames = (UInt32)vstream->nb_frames;
                else
                {
                    double durSec = 0.0;
                    if (vstream->duration > 0 && vstream->time_base.den > 0)
                        durSec = (double)vstream->duration * (double)vstream->time_base.num /
                                 (double)vstream->time_base.den;
                    else if (pfmt->duration > 0)
                        durSec = (double)pfmt->duration / (double)AV_TIME_BASE;
                    pinfo->TotalFrames = durSec > 0.0 ?
                        (UInt32)(durSec * (double)fps.num / (double)fps.den + 0.5) : 0;
                }

                pinfo->AudioDataRate = 0;
                pinfo->SubtitleChannelsNumber = 0;
                pinfo->MaxSubtitleLen = 0;

                for (unsigned i = 0; i < pfmt->nb_streams; ++i)
                {
                    AVStream* st = pfmt->streams[i];
                    if (!st->codecpar)
                        continue;
                    if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                    {
                        AudioTrackInfo ati;
                        ati.Index = (int)i;
                        ati.ChannelsNumber = (UInt32)st->codecpar->ch_layout.nb_channels;
                        ati.SampleRate     = (UInt32)(st->codecpar->sample_rate > 0 ? st->codecpar->sample_rate : 0);
                        ati.TotalSamples   = 0;
                        if (pinfo->AudioTracks.GetSize() == 0)
                            pinfo->AudioDataRate = ati.SampleRate;
                        pinfo->AudioTracks.PushBack(ati);
                    }
                    else if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
                    {
                        pinfo->SubtitleChannelsNumber++;
                        pinfo->MaxSubtitleLen = 4096;
                    }
                }

                ret = true;
            }
        }
        avformat_close_input(&pfmt);
    }
    else
        av_dict_free(&opts);

    return ret;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
