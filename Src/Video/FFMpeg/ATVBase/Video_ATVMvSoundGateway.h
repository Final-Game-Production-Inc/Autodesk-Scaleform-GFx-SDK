/**************************************************************************

Filename    :   Video_ATVMvSoundGateway.h
Content     :   Native audio decoder / output gateway for the ATV (Analog TV)
                FFmpeg backend
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

TIPS        :   The ATV backend no longer routes video PCM through the legacy
                VideoSoundSystem implementations (Video_VideoSoundSystemXXX).
                With CRI Mana 2019 / Sofdec2 the CRI side owns its audio sink
                (CriAtomEx), and the FFmpeg side owns its own here:
                the demuxed compressed packets are decoded by the codec
                libraries that ship with the FFmpeg build (fdk-aac for AAC,
                libopus for Opus, libavcodec for MP3) and played through
                SDL2 audio. No VideoSound / VideoSoundSystem object is created.
                Everything libavcodec can decode but which has no dedicated
                decoder here - AC-3 (Dolby Digital), E-AC-3, DTS, TrueHD, FLAC,
                Vorbis, PCM, ... - is handled by the generic libavcodec +
                libswresample path, so containers such as Blu-ray remuxes stay
                audible.

**************************************************************************/

#ifndef INC_GFX_VIDEO_ATVMVSOUNDGATEWAY_H
#define INC_GFX_VIDEO_ATVMVSOUNDGATEWAY_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Types.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Video/Common/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// ATVMvSoundConfig
//
// Central sound configuration for the ATV (Analog TV) FFmpeg backend.
// The player samples these defaults when it opens an audio stream.

struct ATVMvSoundConfig
{
    float   AudioBufferTime;    // seconds of audio buffered ahead of the device
    UInt32  MaxChannels;        // maximum channels accepted from the container
    UInt32  MaxQueuedPackets;   // compressed packets queued before back-pressure

    ATVMvSoundConfig() :
        AudioBufferTime(0.5f),
        MaxChannels(VideoSound::MaxChannels),
        MaxQueuedPackets(96)
    {}
};

//////////////////////////////////////////////////////////////////////////
// ATVMvAudioCodec
//
// Codecs decoded natively by the gateway. The mapping from FFmpeg's
// AVCodecID is done by the player.

enum ATVMvAudioCodec
{
    ATVMvCodec_Unknown = 0,
    ATVMvCodec_AAC,
    ATVMvCodec_Opus,
    ATVMvCodec_MP3,
    // Any other codec libavcodec can decode (AC-3, E-AC-3, DTS, TrueHD, FLAC,
    // Vorbis, PCM, ALAC, ...). Open() needs the container's codec id for this
    // one, which is passed as the codecTag argument.
    ATVMvCodec_Libav
};

//////////////////////////////////////////////////////////////////////////
// ATVMvSoundGateway
//
// One instance per playing video. The player pushes demuxed compressed
// packets (AVPacket payloads) through PushPacket(); the gateway decodes
// them on its own worker thread and streams the resulting float PCM to
// SDL2. The gateway also owns the decoder state needed for seeking
// (Flush) and for pause/stop.
//
// Lifetime: Open() -> (Start() -> PushPacket()* -> Pause()/Flush()) -> Close().
// PushPacket() copies the payload, so the caller may release the AVPacket
// immediately.

class ATVMvSoundGateway
{
public:
    ATVMvSoundGateway(MemoryHeap* pheap);
    ~ATVMvSoundGateway();

    // Selects and initializes the decoder. extradata may be NULL;
    // fallbackChannels/fallbackSampleRate are used when the codec headers do
    // not carry the information (ADTS AAC, raw MP3, ...).
    // codecTag is the container's AVCodecID and is only used by
    // ATVMvCodec_Libav, which opens the matching libavcodec decoder.
    bool            Open(ATVMvAudioCodec codec, UInt32 codecTag,
                         const UInt8* pextradata, int extradataSize,
                         UInt32 fallbackChannels, UInt32 fallbackSampleRate);
    void            Close();
    bool            IsOpen() const;

    // Queues one compressed access unit. Returns false when the gateway is
    // closed or the packet is empty. The queue is bounded; the oldest packet
    // is dropped when the player runs too far ahead of the device.
    bool            PushPacket(const UInt8* pdata, int size);

    // Starts device playback (packets may be queued before this call).
    void            Start();
    // Stops playback and discards queued packets; the decoder stays open.
    void            Stop();
    // Pauses/resumes the device without losing the decoder position.
    void            Pause(bool paused);
    // Drops queued packets and resets decoder/output state (seek, loop).
    void            Flush();

    UInt32          GetChannels() const;
    UInt32          GetSampleRate() const;
    UInt32          GetQueuedPackets() const;
    double          GetPosition() const;    // seconds of audio handed to the device
    bool            HasOutput() const;

private:
    class Impl;
    Impl*           pImpl;

    ATVMvSoundGateway(const ATVMvSoundGateway&);
    ATVMvSoundGateway& operator=(const ATVMvSoundGateway&);
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_ATVMVSOUNDGATEWAY_H
