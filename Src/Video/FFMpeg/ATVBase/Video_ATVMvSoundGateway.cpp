/**************************************************************************

Filename    :   Video_ATVMvSoundGateway.cpp
Content     :   Native audio decoder / output gateway for the ATV (Analog TV)
                FFmpeg backend
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

TIPS        :   Decoding is done with the exact codec libraries that the
                bundled FFmpeg build links against:
                    AAC  -> fdk-aac (libAACdec)
                    Opus -> libopus (multistream decoder)
                    MP3  -> libavcodec's native MP3 decoder
                           (the bundled libmp3lame has no mpglib decoder)
                    AC-3, E-AC-3, DTS, TrueHD, FLAC, Vorbis, PCM, ... ->
                           libavcodec + libswresample
                Output is SDL2 audio (SDL2-static.lib, shipped with the FFmpeg
                SDK and already linked by the FFMpegVideo property sheet), so
                the gateway does not depend on the deprecated
                VideoSoundSystem implementations at all.

**************************************************************************/

#include "Video/FFMpeg/ATVBase/Video_ATVMvSoundGateway.h"

#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Array.h"

#include <string.h>
#include <stdio.h>

// Pull the CRT math header in before any third party header. libavutil's
// mathematics.h defines M_E/M_LN2/M_PI_4/M_1_PI/M_2_PI/M_SQRT1_2 only when
// <math.h> did not provide them, but SDL_stdinc.h (pulled in later through
// <SDL2/SDL.h>) defines _USE_MATH_DEFINES right before including <math.h>, which
// makes the CRT redefine all of them and produces C4005. Defining the switch here
// makes libavutil's definitions no-ops and keeps math.h a single-shot include.
#if !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#if defined(SF_BUILD_DEBUG)
#define ATV_AUDIO_TRACE(...) \
    do { fprintf(stderr, "[ATV-AUDIO] " __VA_ARGS__); fflush(stderr); } while (0)
#else
#define ATV_AUDIO_TRACE(...) do {} while (0)
#endif

// Third party audio/codec headers. They are compiled at /W4 and are not ours to
// fix, so the diagnostics they provoke are silenced around the include block:
//   C4244 - libavutil's inline clippers (av_clip_uint8_c & co. in
//           libavutil/common.h) return an int expression from functions declared
//           to return uint8_t/int8_t/uint16_t/int16_t.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244) // int -> uint8_t/int8_t/uint16_t/int16_t in libavutil
#endif

// Native codec libraries shipped with the FFmpeg SDK.
#include <fdk-aac/aacdecoder_lib.h>
#include <opus/opus.h>
#include <opus/opus_multistream.h>

// LAME in the bundled SDK is built without mpglib (hip_decode_* return -1),
// so MP3 is decoded with libavcodec's native MP3 decoder instead.
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(SF_OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// SDL2-static's device enumeration pulls these Win32 setup APIs.
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")
#endif

// SDL2 is shipped with the FFmpeg SDK (3rdParty/FFMpeg/include/SDL2) and
// SDL2-static.lib is already part of the FFmpeg video link set.
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace Scaleform { namespace GFx { namespace Video {

namespace {

// -----------------------------------------------------------------------
// Native decoder abstraction. Each decoder owns a float PCM scratch buffer
// that stays valid until the next Decode() call.

class NativeAudioDecoder
{
public:
    NativeAudioDecoder(MemoryHeap* pheap)
        : pHeap(pheap), Pcm(NULL), PcmFloats(0), Channels(0), SampleRate(0)
    {
    }
    virtual ~NativeAudioDecoder()
    {
        if (Pcm)
            SF_FREE(Pcm);
    }

    virtual bool    Open(const UInt8* pextradata, int extradataSize,
                         UInt32 fallbackChannels, UInt32 fallbackSampleRate) = 0;
    virtual int     Decode(const UInt8* pdata, int size, const float** ppcm) = 0;
    virtual void    Flush() = 0;

    UInt32          GetChannels() const   { return Channels; }
    UInt32          GetSampleRate() const { return SampleRate; }

protected:
    bool AllocPcm(int floats)
    {
        Pcm = (float*)SF_HEAP_ALLOC(pHeap, (UPInt)floats * sizeof(float), Stat_Video_Mem);
        PcmFloats = Pcm ? floats : 0;
        return Pcm != NULL;
    }

    MemoryHeap* pHeap;
    float*      Pcm;
    int         PcmFloats;
    UInt32      Channels;
    UInt32      SampleRate;
};

// -----------------------------------------------------------------------
// AAC (fdk-aac)

class AacAudioDecoder : public NativeAudioDecoder
{
public:
    AacAudioDecoder(MemoryHeap* pheap)
        : NativeAudioDecoder(pheap), Handle(NULL), Temp(NULL)
    {
    }
    virtual ~AacAudioDecoder()
    {
        if (Handle)
            aacDecoder_Close(Handle);
        if (Temp)
            SF_FREE(Temp);
    }

    virtual bool Open(const UInt8* pextradata, int extradataSize,
                      UInt32 fallbackChannels, UInt32 fallbackSampleRate)
    {
        // Raw MPEG-4 access units carry an AudioSpecificConfig in extradata;
        // ADTS streams configure themselves from the in-band headers.
        Handle = aacDecoder_Open(extradataSize > 0 ? TT_MP4_RAW : TT_MP4_ADTS, 1);
        if (!Handle)
            return false;

        if (extradataSize > 0)
        {
            UCHAR* pconf = (UCHAR*)pextradata;
            UINT   conlen = (UINT)extradataSize;
            if (aacDecoder_ConfigRaw(Handle, &pconf, &conlen) != AAC_DEC_OK)
            {
                aacDecoder_Close(Handle);
                Handle = NULL;
                return false;
            }
        }

        // Ask for a stereo downmix; if this FDK build ignores the parameter the
        // gateway downmixes whatever channel count comes back.
        aacDecoder_SetParam(Handle, AAC_PCM_MAX_OUTPUT_CHANNELS, 2);

        Temp = (INT_PCM*)SF_HEAP_ALLOC(pHeap, (UPInt)(8 * 2048) * sizeof(INT_PCM),
                                       Stat_Video_Mem);
        if (!Temp || !AllocPcm(8 * 2048))
            return false;

        Channels   = fallbackChannels > 0 ? (fallbackChannels > 2 ? 2 : fallbackChannels) : 2;
        SampleRate = fallbackSampleRate > 0 ? fallbackSampleRate : 44100;
        return true;
    }

    virtual int Decode(const UInt8* pdata, int size, const float** ppcm)
    {
        if (!Handle || !Temp || size <= 0)
            return 0;

        UINT valid = (UINT)size;
        int  total = 0;

        // One packet normally carries one access unit, but ADTS packets can
        // contain several frames; drain the decoder after every Fill().
        for (int guard = 0; guard < 16 && valid > 0; ++guard)
        {
            UCHAR* pbuf = (UCHAR*)pdata + (size - (int)valid);
            UINT   buflen = valid;
            AAC_DECODER_ERROR err = aacDecoder_Fill(Handle, &pbuf, &buflen, &valid);
            if (err != AAC_DEC_OK)
                break;

            for (;;)
            {
                err = aacDecoder_DecodeFrame(Handle, Temp, 8 * 2048, 0);
                if (err == AAC_DEC_NOT_ENOUGH_BITS)
                    break;
                if (!IS_OUTPUT_VALID(err))
                    break;

                CStreamInfo* pinfo = aacDecoder_GetStreamInfo(Handle);
                if (!pinfo || pinfo->numChannels <= 0 || pinfo->frameSize <= 0)
                    break;

                int ch = pinfo->numChannels;
                int n  = pinfo->frameSize;
                if (ch > 8)
                    ch = 8;
                if ((total + n) * ch > PcmFloats)
                    break;

                for (int i = 0; i < n * ch; ++i)
                    Pcm[total * ch + i] = (float)Temp[i] * (1.0f / 32768.0f);
                total += n;
                Channels = (UInt32)ch;
                if (pinfo->sampleRate > 0)
                    SampleRate = (UInt32)pinfo->sampleRate;

                if (valid == 0)
                    break;
            }
        }

        if (total <= 0)
            return 0;
        *ppcm = Pcm;
        return total;
    }

    virtual void Flush()
    {
        if (Handle)
            aacDecoder_SetParam(Handle, AAC_TPDEC_CLEAR_BUFFER, 1);
    }

private:
    HANDLE_AACDECODER Handle;
    INT_PCM*          Temp;
};

// -----------------------------------------------------------------------
// Opus (libopus multistream)

class OpusAudioDecoder : public NativeAudioDecoder
{
public:
    OpusAudioDecoder(MemoryHeap* pheap)
        : NativeAudioDecoder(pheap), Dec(NULL), PreSkip(0), Skip(0)
    {
    }
    virtual ~OpusAudioDecoder()
    {
        if (Dec)
            opus_multistream_decoder_destroy(Dec);
    }

    virtual bool Open(const UInt8* pextradata, int extradataSize,
                      UInt32 fallbackChannels, UInt32 fallbackSampleRate)
    {
        SF_UNUSED(fallbackSampleRate);

        static const unsigned char kStereoMapping[2] = { 0, 1 };
        int channels = fallbackChannels > 0 ? (int)fallbackChannels : 2;
        int streams  = 1;
        int coupled  = channels > 1 ? 1 : 0;
        const unsigned char* pmapping = kStereoMapping;
        bool haveMapping = false;
        int gain = 0;

        if (pextradata && extradataSize >= 19)
        {
            channels = pextradata[9] ? pextradata[9] : channels;
            PreSkip  = pextradata[10] | (pextradata[11] << 8);
            gain     = (SInt16)(pextradata[16] | (pextradata[17] << 8));

            if (extradataSize >= 19 + 2 + channels)
            {
                streams = pextradata[19];
                coupled = pextradata[20];
                pmapping = pextradata + 21;
                haveMapping = true;
            }
        }

        // Without a full OpusHead mapping table only mono/stereo are safe;
        // the two-entry default table must not be indexed for 5.1 content.
        if (!haveMapping)
        {
            if (channels > 2)
                channels = 2;
            streams = 1;
            coupled = channels > 1 ? 1 : 0;
            pmapping = kStereoMapping;
        }

        if (channels < 1)
            channels = 1;
        if (channels > 8)
            channels = 8;
        if (streams < 1)
            streams = 1;
        if (coupled < 0)
            coupled = 0;

        int err = OPUS_OK;
        Dec = opus_multistream_decoder_create(48000, channels, streams, coupled,
                                              pmapping, &err);
        if (!Dec || err != OPUS_OK)
            return false;

        Skip = PreSkip;
        opus_multistream_decoder_ctl(Dec, OPUS_SET_GAIN(gain));

        Channels   = (UInt32)channels;
        SampleRate = 48000;
        return AllocPcm(8 * 5760);
    }

    virtual int Decode(const UInt8* pdata, int size, const float** ppcm)
    {
        if (!Dec || size <= 0)
            return 0;

        int n = opus_multistream_decode_float(Dec, pdata, size, Pcm, 5760, 0);
        if (n <= 0)
            return 0;

        // OpusHead pre-skip: discard the decoder priming samples.
        if (Skip > 0)
        {
            int drop = Skip < n ? Skip : n;
            memmove(Pcm, Pcm + (UPInt)drop * Channels,
                    (UPInt)(n - drop) * Channels * sizeof(float));
            n -= drop;
            Skip -= drop;
        }
        if (n <= 0)
            return 0;

        *ppcm = Pcm;
        return n;
    }

    virtual void Flush()
    {
        if (Dec)
            opus_multistream_decoder_ctl(Dec, OPUS_RESET_STATE);
        Skip = PreSkip;
    }

private:
    OpusMSDecoder* Dec;
    int            PreSkip;
    int            Skip;
};

// -----------------------------------------------------------------------
// MP3 (libavcodec native decoder)
//
// The bundled libmp3lame is built without mpglib, so its hip_decode_* entry
// points exist but always fail. libavcodec (already linked by this backend)
// provides the MP3 decoder used here.

class Mp3AudioDecoder : public NativeAudioDecoder
{
public:
    Mp3AudioDecoder(MemoryHeap* pheap)
        : NativeAudioDecoder(pheap), Ctx(NULL), Frame(NULL), Packet(NULL)
    {
    }
    virtual ~Mp3AudioDecoder()
    {
        if (Packet)
            av_packet_free(&Packet);
        if (Frame)
            av_frame_free(&Frame);
        if (Ctx)
            avcodec_free_context(&Ctx);
    }

    virtual bool Open(const UInt8* pextradata, int extradataSize,
                      UInt32 fallbackChannels, UInt32 fallbackSampleRate)
    {
        SF_UNUSED2(pextradata, extradataSize);
        SF_UNUSED(fallbackChannels);

        const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
        if (!codec)
            return false;

        Ctx = avcodec_alloc_context3(codec);
        if (!Ctx)
            return false;
        if (fallbackSampleRate > 0)
            Ctx->sample_rate = (int)fallbackSampleRate;
        if (avcodec_open2(Ctx, codec, NULL) < 0)
            return false;

        Frame  = av_frame_alloc();
        Packet = av_packet_alloc();
        if (!Frame || !Packet || !AllocPcm(2 * MaxSamples))
            return false;

        Channels   = 2;
        SampleRate = fallbackSampleRate > 0 ? fallbackSampleRate : 44100;
        return true;
    }

    virtual int Decode(const UInt8* pdata, int size, const float** ppcm)
    {
        if (!Ctx || !Frame || !Packet || size <= 0)
            return 0;

        av_packet_unref(Packet);
        if (av_new_packet(Packet, size) < 0)
            return 0;
        memcpy(Packet->data, pdata, (size_t)size);

        int ret = avcodec_send_packet(Ctx, Packet);
        av_packet_unref(Packet);
        if (ret < 0 && ret != AVERROR(EAGAIN))
            return 0;

        int total = 0;
        for (;;)
        {
            ret = avcodec_receive_frame(Ctx, Frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0)
                break;

            int ch = Frame->ch_layout.nb_channels;
            if (ch <= 0)
                ch = 2;
            if (ch > 2)
                ch = 2;

            int n = Frame->nb_samples;
            if (n <= 0 || (total + n) * 2 > PcmFloats)
            {
                av_frame_unref(Frame);
                break;
            }

            if (Frame->format == AV_SAMPLE_FMT_FLTP)
            {
                const float* pl = (const float*)Frame->extended_data[0];
                const float* pr = ch > 1 ? (const float*)Frame->extended_data[1] : pl;
                for (int i = 0; i < n; ++i)
                {
                    Pcm[(total + i) * 2 + 0] = pl[i];
                    Pcm[(total + i) * 2 + 1] = pr[i];
                }
            }
            else if (Frame->format == AV_SAMPLE_FMT_S16P)
            {
                const SInt16* pl = (const SInt16*)Frame->extended_data[0];
                const SInt16* pr = ch > 1 ? (const SInt16*)Frame->extended_data[1] : pl;
                for (int i = 0; i < n; ++i)
                {
                    Pcm[(total + i) * 2 + 0] = (float)pl[i] * (1.0f / 32768.0f);
                    Pcm[(total + i) * 2 + 1] = (float)pr[i] * (1.0f / 32768.0f);
                }
            }
            else if (Frame->format == AV_SAMPLE_FMT_S16)
            {
                const SInt16* pinter = (const SInt16*)Frame->extended_data[0];
                for (int i = 0; i < n; ++i)
                {
                    Pcm[(total + i) * 2 + 0] = (float)pinter[i * ch + 0] * (1.0f / 32768.0f);
                    Pcm[(total + i) * 2 + 1] = (float)(ch > 1 ? pinter[i * ch + 1]
                                                              : pinter[i * ch + 0]) *
                                               (1.0f / 32768.0f);
                }
            }
            else
            {
                av_frame_unref(Frame);
                break;
            }

            if (Frame->sample_rate > 0)
                SampleRate = (UInt32)Frame->sample_rate;
            total += n;
            av_frame_unref(Frame);
        }

        if (total <= 0)
            return 0;

        Channels = 2;
        *ppcm = Pcm;
        return total;
    }

    virtual void Flush()
    {
        if (Ctx)
            avcodec_flush_buffers(Ctx);
    }

private:
    enum { MaxSamples = 1 << 16 };

    AVCodecContext* Ctx;
    AVFrame*        Frame;
    AVPacket*       Packet;
};

// -----------------------------------------------------------------------
// Generic libavcodec decoder
//
// Covers every audio codec the bundled FFmpeg build can decode but which has
// no dedicated decoder above: AC-3 (Dolby Digital - the audio track of most
// MKV/Blu-ray remuxes and the reason videos used to play silently), E-AC-3,
// DTS, TrueHD, FLAC, Vorbis, PCM, ALAC, WMA, ...
//
// The decoder emits libavcodec's native planar formats at the stream's own
// rate, so libswresample converts each frame to the interleaved float PCM the
// gateway streams to SDL2 and performs the multichannel -> stereo downmix with
// FFmpeg's own rematrixing coefficients.

class LibavAudioDecoder : public NativeAudioDecoder
{
public:
    LibavAudioDecoder(MemoryHeap* pheap, enum AVCodecID codecId)
        : NativeAudioDecoder(pheap), CodecId(codecId), Ctx(NULL), Frame(NULL),
          Packet(NULL), Swr(NULL), SwrInFmt(AV_SAMPLE_FMT_NONE), SwrInRate(0),
          SwrInChannels(0), SwrOutChannels(0)
    {
    }
    virtual ~LibavAudioDecoder()
    {
        if (Swr)
            swr_free(&Swr);
        if (Packet)
            av_packet_free(&Packet);
        if (Frame)
            av_frame_free(&Frame);
        if (Ctx)
            avcodec_free_context(&Ctx);
    }

    virtual bool Open(const UInt8* pextradata, int extradataSize,
                      UInt32 fallbackChannels, UInt32 fallbackSampleRate)
    {
        const AVCodec* codec = avcodec_find_decoder(CodecId);
        if (!codec)
        {
            ATV_AUDIO_TRACE("libav: no decoder for codec id %d\n", (int)CodecId);
            return false;
        }

        Ctx = avcodec_alloc_context3(codec);
        if (!Ctx)
            return false;

        if (pextradata && extradataSize > 0)
        {
            Ctx->extradata = (UInt8*)av_mallocz((size_t)extradataSize +
                                                AV_INPUT_BUFFER_PADDING_SIZE);
            if (!Ctx->extradata)
                return false;
            memcpy(Ctx->extradata, pextradata, (size_t)extradataSize);
            Ctx->extradata_size = extradataSize;
        }

        if (fallbackSampleRate > 0)
            Ctx->sample_rate = (int)fallbackSampleRate;
        if (fallbackChannels > 0)
        {
            unsigned ch = fallbackChannels > (unsigned)VideoSound::MaxChannels
                        ? (unsigned)VideoSound::MaxChannels : fallbackChannels;
            av_channel_layout_default(&Ctx->ch_layout, (int)ch);
        }

        if (avcodec_open2(Ctx, codec, NULL) < 0)
        {
            ATV_AUDIO_TRACE("libav: avcodec_open2(%s) failed\n", codec->name);
            return false;
        }

        Frame  = av_frame_alloc();
        Packet = av_packet_alloc();
        if (!Frame || !Packet || !ReservePcm(16384))
            return false;

        Channels   = fallbackChannels > 0
                   ? (fallbackChannels > 2 ? 2 : fallbackChannels) : 2;
        SampleRate = fallbackSampleRate > 0 ? fallbackSampleRate : 48000;
        ATV_AUDIO_TRACE("libav: %s open ch=%u rate=%u\n",
                        codec->name, Channels, SampleRate);
        return true;
    }

    virtual int Decode(const UInt8* pdata, int size, const float** ppcm)
    {
        if (!Ctx || !Frame || !Packet || size <= 0)
            return 0;

        av_packet_unref(Packet);
        if (av_new_packet(Packet, size) < 0)
            return 0;
        memcpy(Packet->data, pdata, (size_t)size);

        int ret = avcodec_send_packet(Ctx, Packet);
        av_packet_unref(Packet);
        // The worker drains the decoder after every packet, so a full input
        // queue cannot happen here; anything else is a corrupt packet.
        if (ret < 0 && ret != AVERROR(EAGAIN))
            return 0;

        int total = 0;
        for (;;)
        {
            ret = avcodec_receive_frame(Ctx, Frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0)
                break;

            int inChannels = Frame->ch_layout.nb_channels;
            if (inChannels <= 0)
            {
                av_frame_unref(Frame);
                break;
            }
            int outChannels = inChannels > 2 ? 2 : inChannels;

            if (!EnsureResampler(Frame, outChannels))
            {
                av_frame_unref(Frame);
                break;
            }

            int inRate = Frame->sample_rate > 0 ? Frame->sample_rate : (int)SampleRate;
            int outMax = Frame->nb_samples + (int)swr_get_delay(Swr, inRate) + 32;
            if (outMax <= 0 || !ReservePcm((total + outMax) * outChannels))
            {
                av_frame_unref(Frame);
                break;
            }

            UInt8* pout = (UInt8*)(Pcm + (UPInt)total * outChannels);
            int got = swr_convert(Swr, &pout, outMax,
                                  (const UInt8* const*)Frame->extended_data,
                                  Frame->nb_samples);
            if (got <= 0)
            {
                av_frame_unref(Frame);
                break;
            }

            total += got;
            Channels = (UInt32)outChannels;
            if (Frame->sample_rate > 0)
                SampleRate = (UInt32)Frame->sample_rate;
            av_frame_unref(Frame);
        }

        if (total <= 0)
            return 0;
        *ppcm = Pcm;
        return total;
    }

    virtual void Flush()
    {
        if (Ctx)
            avcodec_flush_buffers(Ctx);
        // Same-rate conversion keeps no samples in swresample, only reset the
        // resampler state in case a seek lands on a different stream format.
        if (Swr)
        {
            swr_free(&Swr);
            SwrInFmt      = AV_SAMPLE_FMT_NONE;
            SwrInRate     = 0;
            SwrInChannels = 0;
            SwrOutChannels = 0;
        }
    }

private:
    bool EnsureResampler(const AVFrame* pframe, int outChannels)
    {
        const int inFmt  = pframe->format;
        const int inRate = pframe->sample_rate > 0 ? pframe->sample_rate : (int)SampleRate;
        const int inCh   = pframe->ch_layout.nb_channels;

        if (Swr && SwrInFmt == inFmt && SwrInRate == inRate &&
            SwrInChannels == inCh && SwrOutChannels == outChannels)
            return true;

        if (Swr)
        {
            swr_free(&Swr);
            SwrInFmt = AV_SAMPLE_FMT_NONE;
            SwrInRate = 0;
            SwrInChannels = 0;
            SwrOutChannels = 0;
        }

        AVChannelLayout outLayout;
        av_channel_layout_default(&outLayout, outChannels);

        // The input layout comes from the frame, so non-standard layouts keep
        // their own channel order and rematrixing coefficients.
        int err = swr_alloc_set_opts2(&Swr, &outLayout, AV_SAMPLE_FMT_FLT, inRate,
                                      &pframe->ch_layout,
                                      (enum AVSampleFormat)inFmt, inRate, 0, NULL);
        av_channel_layout_uninit(&outLayout);
        if (err < 0 || !Swr)
        {
            ATV_AUDIO_TRACE("libav: swr_alloc_set_opts2 failed (%d)\n", err);
            return false;
        }
        if (swr_init(Swr) < 0)
        {
            ATV_AUDIO_TRACE("libav: swr_init failed\n");
            swr_free(&Swr);
            return false;
        }

        SwrInFmt      = inFmt;
        SwrInRate     = inRate;
        SwrInChannels = inCh;
        SwrOutChannels = outChannels;
        return true;
    }

    // Grows the shared float PCM scratch buffer; the base class frees it.
    bool ReservePcm(int floats)
    {
        if (Pcm && floats <= PcmFloats)
            return true;

        int cap = PcmFloats > 0 ? PcmFloats : 4096;
        while (cap < floats)
            cap *= 2;

        float* pnew = (float*)SF_HEAP_ALLOC(pHeap, (UPInt)cap * sizeof(float),
                                            Stat_Video_Mem);
        if (!pnew)
            return false;
        if (Pcm)
        {
            if (PcmFloats > 0)
                memcpy(pnew, Pcm, (UPInt)PcmFloats * sizeof(float));
            SF_FREE(Pcm);
        }
        Pcm = pnew;
        PcmFloats = cap;
        return true;
    }

    enum AVCodecID  CodecId;
    AVCodecContext* Ctx;
    AVFrame*        Frame;
    AVPacket*       Packet;
    SwrContext*     Swr;
    int             SwrInFmt;
    int             SwrInRate;
    int             SwrInChannels;
    int             SwrOutChannels;
};

// -----------------------------------------------------------------------
// SDL audio subsystem. Reference counted so multiple simultaneous videos
// share one SDL audio subsystem instance; each gateway opens its own device.

class SdlAudioModule
{
public:
    SdlAudioModule() : Refs(0), Owned(false)
    {
        InitializeCriticalSection(&Cs);
    }
    ~SdlAudioModule()
    {
        if (Refs > 0 && Owned)
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        DeleteCriticalSection(&Cs);
    }

    bool Acquire()
    {
        EnterCriticalSection(&Cs);
        if (Refs == 0)
        {
            // Only shut the subsystem down on release if this module was the
            // one that initialized it; the host application may already use
            // SDL audio for its own sounds.
            if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
            {
                if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
                {
                    ATV_AUDIO_TRACE("SDL audio init failed: %s\n", SDL_GetError());
                    LeaveCriticalSection(&Cs);
                    return false;
                }
                Owned = true;
            }
            else
            {
                Owned = false;
            }
        }
        ++Refs;
        LeaveCriticalSection(&Cs);
        return true;
    }

    void Release()
    {
        EnterCriticalSection(&Cs);
        if (Refs > 0 && --Refs == 0 && Owned)
        {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            Owned = false;
        }
        LeaveCriticalSection(&Cs);
    }

private:
    CRITICAL_SECTION Cs;
    int              Refs;
    bool             Owned;
};

SdlAudioModule& SdlAudio()
{
    static SdlAudioModule module;
    return module;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////
//
// ATVMvSoundGateway::Impl

class ATVMvSoundGateway::Impl
{
public:
    Impl(MemoryHeap* pheap);
    virtual ~Impl();

    bool Open(ATVMvAudioCodec codec, UInt32 codecTag,
              const UInt8* pextradata, int extradataSize,
              UInt32 fallbackChannels, UInt32 fallbackSampleRate);
    void Close();
    bool IsOpen() const { return Opened; }

    bool PushPacket(const UInt8* pdata, int size);
    void Start();
    void Stop();
    void Pause(bool paused);
    void Flush();

    UInt32 GetChannels() const     { return Channels; }
    UInt32 GetSampleRate() const   { return SampleRate; }
    UInt32 GetQueuedPackets() const;
    double GetPosition() const;
    bool   HasOutput() const { return OutputReady; }

private:
    struct Packet
    {
        UInt8* Data;
        int    Size;
        Packet() : Data(NULL), Size(0) {}
    };

    static DWORD WINAPI ThreadProc(LPVOID pthis);
    DWORD Worker();

    void ClearPackets();
    bool PopPacket(Packet* ppacket);
    void FreePacket(Packet* ppacket);

    void DecodeAndSubmit(const UInt8* pdata, int size);
    void SubmitPcm(const float* pcm, int samples, UInt32 channels);
    static void DownmixToStereo(const float* src, int samples, UInt32 channels,
                                float* dst);

    bool EnsureOutput(UInt32 channels, UInt32 sampleRate);
    void DestroyOutput();

    MemoryHeap*       pHeap;
    NativeAudioDecoder* pDecoder;
    ATVMvAudioCodec   Codec;

    UInt32            Channels;
    UInt32            SampleRate;
    UInt32            MaxQueuedPackets;

    volatile bool     Opened;
    volatile bool     Playing;
    volatile bool     Paused;
    volatile bool     Running;
    volatile bool     StopRequested;

    Array<Packet>     Packets;
    CRITICAL_SECTION  QueueCs;
    CRITICAL_SECTION  DecodeCs;

    UInt64            SubmittedSamples;

    float*            StereoScratch;
    int               StereoScratchSamples;

    HANDLE            hThread;
    HANDLE            hWake;

    SDL_AudioDeviceID AudioDevice;
    bool              OutputReady;

    enum { SamplesPerBuffer = 512, MaxBuffers = 2 };
};

ATVMvSoundGateway::Impl::Impl(MemoryHeap* pheap)
    : pHeap(pheap), pDecoder(NULL), Codec(ATVMvCodec_Unknown),
      Channels(0), SampleRate(0), MaxQueuedPackets(96),
      Opened(false), Playing(false), Paused(false), Running(false),
      StopRequested(false), SubmittedSamples(0),
      StereoScratch(NULL), StereoScratchSamples(0),
      hThread(NULL), hWake(NULL), AudioDevice(0), OutputReady(false)
{
    InitializeCriticalSection(&QueueCs);
    InitializeCriticalSection(&DecodeCs);

    hWake = CreateEvent(NULL, FALSE, FALSE, NULL);
}

ATVMvSoundGateway::Impl::~Impl()
{
    Close();

    if (hWake)
        CloseHandle(hWake);

    DeleteCriticalSection(&QueueCs);
    DeleteCriticalSection(&DecodeCs);

    if (StereoScratch)
        SF_FREE(StereoScratch);
}

bool ATVMvSoundGateway::Impl::Open(ATVMvAudioCodec codec, UInt32 codecTag,
                                   const UInt8* pextradata, int extradataSize,
                                   UInt32 fallbackChannels, UInt32 fallbackSampleRate)
{
    Close();

    switch (codec)
    {
    case ATVMvCodec_AAC:
        pDecoder = new AacAudioDecoder(pHeap);
        break;
    case ATVMvCodec_Opus:
        pDecoder = new OpusAudioDecoder(pHeap);
        break;
    case ATVMvCodec_MP3:
        pDecoder = new Mp3AudioDecoder(pHeap);
        break;
    case ATVMvCodec_Libav:
        // AC-3, E-AC-3, DTS, FLAC, Vorbis, PCM, ... : libavcodec picks the
        // decoder from the container's codec id.
        pDecoder = new LibavAudioDecoder(pHeap, (enum AVCodecID)codecTag);
        break;
    default:
        return false;
    }

    if (!pDecoder->Open(pextradata, extradataSize, fallbackChannels, fallbackSampleRate))
    {
        delete pDecoder;
        pDecoder = NULL;
        return false;
    }

    Codec      = codec;
    Channels   = pDecoder->GetChannels();
    SampleRate = pDecoder->GetSampleRate();
    Opened     = true;
    ATV_AUDIO_TRACE("open codec=%d(%s) ch=%u rate=%u\n", (int)codec,
                    codec == ATVMvCodec_Libav
                        ? avcodec_get_name((enum AVCodecID)codecTag) : "native",
                    Channels, SampleRate);
    return true;
}

void ATVMvSoundGateway::Impl::Close()
{
    ATV_AUDIO_TRACE("close\n");
    Opened = false;
    Playing = false;
    Paused = false;
    Running = false;
    StopRequested = true;
    if (hWake)
        SetEvent(hWake);

    if (hThread)
    {
        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
        hThread = NULL;
    }

    DestroyOutput();
    ClearPackets();

    if (pDecoder)
    {
        delete pDecoder;
        pDecoder = NULL;
    }

    Channels = SampleRate = 0;
    SubmittedSamples = 0;
    Codec = ATVMvCodec_Unknown;
}

bool ATVMvSoundGateway::Impl::PushPacket(const UInt8* pdata, int size)
{
    if (!Opened || !pdata || size <= 0)
        return false;

    Packet packet;
    packet.Data = (UInt8*)SF_HEAP_ALLOC(pHeap, (UPInt)size, Stat_Video_Mem);
    if (!packet.Data)
        return false;
    memcpy(packet.Data, pdata, (size_t)size);
    packet.Size = size;

    bool dropped = false;
    EnterCriticalSection(&QueueCs);
    if (Packets.GetSize() >= MaxQueuedPackets)
    {
        FreePacket(&Packets[0]);
        Packets.RemoveAt(0);
        dropped = true;
    }
    Packets.PushBack(packet);
    LeaveCriticalSection(&QueueCs);

    SF_UNUSED(dropped);

    if (hWake)
        SetEvent(hWake);
    return true;
}

void ATVMvSoundGateway::Impl::Start()
{
    if (!Opened)
        return;

    ATV_AUDIO_TRACE("start\n");
    Playing = true;
    Paused = false;
    StopRequested = false;

    if (!hThread)
    {
        Running = true;
        hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
    }

    if (AudioDevice)
        SDL_PauseAudioDevice(AudioDevice, 0);
    if (hWake)
        SetEvent(hWake);
}

void ATVMvSoundGateway::Impl::Stop()
{
    ATV_AUDIO_TRACE("stop\n");
    Playing = false;
    Paused = false;
    StopRequested = true;

    ClearPackets();

    if (AudioDevice)
    {
        SDL_PauseAudioDevice(AudioDevice, 1);
        SDL_ClearQueuedAudio(AudioDevice);
    }

    SubmittedSamples = 0;
}

void ATVMvSoundGateway::Impl::Pause(bool paused)
{
    if (Paused == paused)
        return;

    ATV_AUDIO_TRACE("pause %d\n", paused ? 1 : 0);
    Paused = paused;

    if (AudioDevice)
        SDL_PauseAudioDevice(AudioDevice, paused ? 1 : 0);
    if (hWake)
        SetEvent(hWake);
}

void ATVMvSoundGateway::Impl::Flush()
{
    ATV_AUDIO_TRACE("flush\n");
    ClearPackets();

    EnterCriticalSection(&DecodeCs);
    if (pDecoder)
        pDecoder->Flush();
    LeaveCriticalSection(&DecodeCs);

    if (AudioDevice)
        SDL_ClearQueuedAudio(AudioDevice);

    SubmittedSamples = 0;

    if (AudioDevice && Playing && !Paused)
        SDL_PauseAudioDevice(AudioDevice, 0);
}

UInt32 ATVMvSoundGateway::Impl::GetQueuedPackets() const
{
    ATVMvSoundGateway::Impl* pthis = const_cast<ATVMvSoundGateway::Impl*>(this);
    EnterCriticalSection(&pthis->QueueCs);
    UInt32 count = (UInt32)pthis->Packets.GetSize();
    LeaveCriticalSection(&pthis->QueueCs);
    return count;
}

double ATVMvSoundGateway::Impl::GetPosition() const
{
    if (!SampleRate)
        return 0.0;

    // SubmittedSamples counts everything queued to SDL, which is ahead of
    // what the speakers have actually played by the device queue. Subtract
    // the queued part so the video clock follows the audible position.
    UInt64 submitted = SubmittedSamples;
    UInt64 queuedSamples = 0;
    if (AudioDevice)
    {
        UInt32 queuedBytes = SDL_GetQueuedAudioSize(AudioDevice);
        UInt32 frameBytes = (Channels > 0 ? Channels : 1) * (UInt32)sizeof(float);
        if (frameBytes)
            queuedSamples = queuedBytes / frameBytes;
    }
    if (queuedSamples > submitted)
        queuedSamples = submitted;
    return (double)(submitted - queuedSamples) / (double)SampleRate;
}

void ATVMvSoundGateway::Impl::ClearPackets()
{
    EnterCriticalSection(&QueueCs);
    for (UPInt i = 0; i < Packets.GetSize(); ++i)
        FreePacket(&Packets[i]);
    Packets.Clear();
    LeaveCriticalSection(&QueueCs);
}

bool ATVMvSoundGateway::Impl::PopPacket(Packet* ppacket)
{
    bool found = false;
    EnterCriticalSection(&QueueCs);
    if (Packets.GetSize() > 0)
    {
        *ppacket = Packets[0];
        Packets.RemoveAt(0);
        found = true;
    }
    LeaveCriticalSection(&QueueCs);
    return found;
}

void ATVMvSoundGateway::Impl::FreePacket(Packet* ppacket)
{
    if (ppacket && ppacket->Data)
    {
        SF_FREE(ppacket->Data);
        ppacket->Data = NULL;
        ppacket->Size = 0;
    }
}

DWORD WINAPI ATVMvSoundGateway::Impl::ThreadProc(LPVOID pthis)
{
    return ((ATVMvSoundGateway::Impl*)pthis)->Worker();
}

DWORD ATVMvSoundGateway::Impl::Worker()
{
    for (;;)
    {
        if (!Running)
            break;

        if (!Playing || Paused)
        {
            WaitForSingleObject(hWake, 30);
            continue;
        }

        Packet packet;
        if (!PopPacket(&packet))
        {
            WaitForSingleObject(hWake, 20);
            continue;
        }

        DecodeAndSubmit(packet.Data, packet.Size);
        FreePacket(&packet);
    }
    return 0;
}

void ATVMvSoundGateway::Impl::DecodeAndSubmit(const UInt8* pdata, int size)
{
    if (!pDecoder)
        return;

    const float* pcm = NULL;
    int samples = 0;
    EnterCriticalSection(&DecodeCs);
    samples = pDecoder->Decode(pdata, size, &pcm);
    LeaveCriticalSection(&DecodeCs);
    if (samples <= 0 || !pcm)
        return;

    UInt32 channels = pDecoder->GetChannels();
    UInt32 rate     = pDecoder->GetSampleRate();
    if (channels == 0)
        channels = 2;
    if (rate == 0)
        rate = 44100;

    UInt32 outChannels = channels > 2 ? 2 : channels;
    if (!EnsureOutput(outChannels, rate))
        return;

    {
        static int nTrace = 0;
        if (nTrace < 8)
        {
            ATV_AUDIO_TRACE("decoded samples=%d ch=%u rate=%u out=%u\n",
                            samples, channels, rate, outChannels);
            ++nTrace;
        }
    }

    SubmitPcm(pcm, samples, channels);
}

void ATVMvSoundGateway::Impl::SubmitPcm(const float* pcm, int samples, UInt32 channels)
{
    const float* psrc = pcm;
    if (channels > 2)
    {
        if (StereoScratchSamples < samples)
        {
            if (StereoScratch)
                SF_FREE(StereoScratch);
            StereoScratch = (float*)SF_HEAP_ALLOC(pHeap, (UPInt)samples * 2 * sizeof(float),
                                                  Stat_Video_Mem);
            StereoScratchSamples = StereoScratch ? samples : 0;
        }
        if (!StereoScratch)
            return;
        DownmixToStereo(pcm, samples, channels, StereoScratch);
        psrc = StereoScratch;
        channels = 2;
    }

    int offset = 0;
    const UInt32 maxQueuedBytes =
        (UInt32)(SamplesPerBuffer * channels * sizeof(float) * MaxBuffers);

    while (offset < samples)
    {
        if (!Running || StopRequested)
            return;
        if (!Playing || Paused)
        {
            WaitForSingleObject(hWake, 30);
            continue;
        }
        if (!AudioDevice)
            return;

        // Keep at most ~4 decoded chunks in the device queue; SDL_QueueAudio
        // copies the data, so the worker never blocks on the audio thread.
        if (SDL_GetQueuedAudioSize(AudioDevice) > maxQueuedBytes)
        {
            WaitForSingleObject(hWake, 10);
            continue;
        }

        int chunk = samples - offset;
        if (chunk > SamplesPerBuffer)
            chunk = SamplesPerBuffer;

        if (SDL_QueueAudio(AudioDevice, psrc + (UPInt)offset * channels,
                           (UInt32)((UPInt)chunk * channels * sizeof(float))) != 0)
        {
            ATV_AUDIO_TRACE("SDL_QueueAudio failed: %s\n", SDL_GetError());
            return;
        }

        SubmittedSamples += (UInt64)chunk;
        offset += chunk;

        {
            static UInt32 nChunks = 0;
            if (++nChunks % 100 == 0)
            {
                ATV_AUDIO_TRACE("pos=%.2fs queued=%u bytes\n",
                                GetPosition(),
                                (unsigned)SDL_GetQueuedAudioSize(AudioDevice));
            }
        }
    }
}

// Simple Vorbis-order -> stereo fold used only when a decoder returns more
// than two channels (5.1 Opus, multi-channel AAC when FDK ignored the
// downmix request). Keeps the centre/LFE content audible.
void ATVMvSoundGateway::Impl::DownmixToStereo(const float* src, int samples,
                                              UInt32 channels, float* dst)
{
    for (int i = 0; i < samples; ++i)
    {
        const float* f = src + (UPInt)i * channels;
        float l = f[0];
        float r = channels > 1 ? f[1] : f[0];

        switch (channels)
        {
        case 3:  // FL FR FC
            l += 0.7071f * f[2];
            r += 0.7071f * f[2];
            break;
        case 4:  // FL FR BL BR
            l += 0.7071f * f[2];
            r += 0.7071f * f[3];
            break;
        case 5:  // FL FR FC BL BR
            l += 0.7071f * f[2] + 0.7071f * f[3];
            r += 0.7071f * f[2] + 0.7071f * f[4];
            break;
        case 6:  // FL FR FC LFE BL BR
            l += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[4];
            r += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[5];
            break;
        case 7:  // FL FR FC LFE BC SL SR
            l += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[4] + 0.7071f * f[5];
            r += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[4] + 0.7071f * f[6];
            break;
        case 8:  // FL FR FC LFE BL BR SL SR
            l += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[4] + 0.7071f * f[6];
            r += 0.7071f * f[2] + 0.5f * f[3] + 0.7071f * f[5] + 0.7071f * f[7];
            break;
        default:
            break;
        }

        if (l >  1.0f) l =  1.0f;
        if (l < -1.0f) l = -1.0f;
        if (r >  1.0f) r =  1.0f;
        if (r < -1.0f) r = -1.0f;

        dst[i * 2 + 0] = l;
        dst[i * 2 + 1] = r;
    }
}

bool ATVMvSoundGateway::Impl::EnsureOutput(UInt32 channels, UInt32 sampleRate)
{
    if (OutputReady && AudioDevice &&
        Channels == channels && SampleRate == sampleRate)
        return true;

    DestroyOutput();

    if (!SdlAudio().Acquire())
        return false;

    SDL_AudioSpec want;
    SDL_AudioSpec have;
    SDL_zero(want);
    want.freq     = (int)sampleRate;
    want.format   = AUDIO_F32SYS;
    want.channels = (Uint8)channels;
    want.samples  = 512;
    want.callback = NULL;   // queue mode: SDL_QueueAudio()

    AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!AudioDevice)
    {
        ATV_AUDIO_TRACE("SDL_OpenAudioDevice failed ch=%u rate=%u: %s\n",
                        channels, sampleRate, SDL_GetError());
        SdlAudio().Release();
        return false;
    }

    if (have.format != AUDIO_F32SYS ||
        have.channels != channels ||
        have.freq != (int)sampleRate)
    {
        ATV_AUDIO_TRACE("SDL audio format mismatch: want f=%d ch=%u r=%u, "
                        "got f=%d ch=%u r=%d\n",
                        (int)AUDIO_F32SYS, channels, sampleRate,
                        (int)have.format, have.channels, have.freq);
        SDL_CloseAudioDevice(AudioDevice);
        AudioDevice = 0;
        SdlAudio().Release();
        return false;
    }

    OutputReady = true;
    Channels = channels;
    SampleRate = sampleRate;

    if (Playing && !Paused)
        SDL_PauseAudioDevice(AudioDevice, 0);
    ATV_AUDIO_TRACE("output open ch=%u rate=%u (SDL %d.%d.%d)\n",
                    channels, sampleRate,
                    SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    return true;
}

void ATVMvSoundGateway::Impl::DestroyOutput()
{
    if (AudioDevice)
    {
        SDL_PauseAudioDevice(AudioDevice, 1);
        SDL_ClearQueuedAudio(AudioDevice);
        SDL_CloseAudioDevice(AudioDevice);
        AudioDevice = 0;
        SdlAudio().Release();
    }
    OutputReady = false;
}

//////////////////////////////////////////////////////////////////////////
//
// ATVMvSoundGateway

ATVMvSoundGateway::ATVMvSoundGateway(MemoryHeap* pheap)
{
    pImpl = new Impl(pheap);
}

ATVMvSoundGateway::~ATVMvSoundGateway()
{
    delete pImpl;
}

bool ATVMvSoundGateway::Open(ATVMvAudioCodec codec, UInt32 codecTag,
                             const UInt8* pextradata, int extradataSize,
                             UInt32 fallbackChannels, UInt32 fallbackSampleRate)
{
    return pImpl->Open(codec, codecTag, pextradata, extradataSize,
                       fallbackChannels, fallbackSampleRate);
}

void ATVMvSoundGateway::Close()
{
    pImpl->Close();
}

bool ATVMvSoundGateway::IsOpen() const
{
    return pImpl->IsOpen();
}

bool ATVMvSoundGateway::PushPacket(const UInt8* pdata, int size)
{
    return pImpl->PushPacket(pdata, size);
}

void ATVMvSoundGateway::Start()
{
    pImpl->Start();
}

void ATVMvSoundGateway::Stop()
{
    pImpl->Stop();
}

void ATVMvSoundGateway::Pause(bool paused)
{
    pImpl->Pause(paused);
}

void ATVMvSoundGateway::Flush()
{
    pImpl->Flush();
}

UInt32 ATVMvSoundGateway::GetChannels() const
{
    return pImpl->GetChannels();
}

UInt32 ATVMvSoundGateway::GetSampleRate() const
{
    return pImpl->GetSampleRate();
}

UInt32 ATVMvSoundGateway::GetQueuedPackets() const
{
    return pImpl->GetQueuedPackets();
}

double ATVMvSoundGateway::GetPosition() const
{
    return pImpl->GetPosition();
}

bool ATVMvSoundGateway::HasOutput() const
{
    return pImpl->HasOutput();
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
