/**************************************************************************

Filename    :   Video_VideoSoundSystemPS4.cpp
Content     :   Video sound system implementation based on PS4 AudioOut library
Created     :   May 2014
Authors     :   Vladislav Merker

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemPS4.h"

#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_HeapNew.h"

#include <audioout.h>
#include <sceerror.h>
#include <pthread.h>

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundPS4 : public VideoSoundStream
{
public:
    VideoSoundPS4(MemoryHeap *pheap) :
        pHeap(pheap), pPCMStream(0),
        SoundStatus(Sound_Stopped), NumChannels(0), SampleRate(0), Volume(1.0f), TotalSamples(0),
        AudioPort(0), SoundThrExit(false) {}
    virtual ~VideoSoundPS4() {}

    virtual bool        CreateOutput(UInt32 channels, UInt32 sample_rate);
    virtual void        DestroyOutput();

    virtual PCMFormat   GetPCMFormat() const { return PCM_Float; }

    virtual void        Start(PCMStream *pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const { return SoundStatus; }
    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64 *count, UInt64 *unit) const;

    virtual void        SetVolume(float volume);
    virtual float       GetVolume() { return Volume; }

private:
    static const UInt32 NUM_CHANNELS = 8;     // SCE_AUDIO_OUT_CHANNEL_MAX
    static const UInt32 SAMPLES_PER_PACKET = 1024;
    static const UInt32 SAMPLE_SIZE = sizeof(float);
    static const UInt32 PACKET_SIZE = (NUM_CHANNELS * SAMPLES_PER_PACKET * SAMPLE_SIZE);
    static const UInt32 AUDIO_FRAME_INTERVAL = 10;

    MemoryHeap*         pHeap;
    PCMStream*          pPCMStream;

    Status              SoundStatus;
    UInt32              NumChannels;
    UInt32              SampleRate;
    float               Volume;
    UInt64              TotalSamples;

    // AudioOut port handle
    int32_t             AudioPort;
    // Buffers to receive and to submit PCM data
    float               TempBuffer[NUM_CHANNELS][SAMPLES_PER_PACKET];
    float               SampleBuffer[PACKET_SIZE];

    pthread_t           SoundThr;
    volatile bool       SoundThrExit;
    static void*        AudioProc(void* arg);
    void                ExecuteAudioFrame();
};


//////////////////////////////////////////////////////////////////////////
//

bool VideoSoundPS4::CreateOutput(UInt32 channels, UInt32 sample_rate)
{   
    NumChannels = channels < NUM_CHANNELS ? channels : NUM_CHANNELS;
    SampleRate = sample_rate;

    // Initialize audio system
    int32_t ret = sceAudioOutInit();
    SF_ASSERT(ret == SCE_OK || ret == SCE_AUDIO_OUT_ERROR_ALREADY_INIT);

    int32_t portType = SCE_AUDIO_OUT_PORT_TYPE_MAIN;
    int32_t dataFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_MONO; // Float 32-bit monaural by default
    if (NumChannels == 2)
    {   // Float 32-bit 2ch stereo
        dataFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_STEREO;
    }
    else if(NumChannels > 2)
    {   // Float 32-bit 7.1 multi-channel (L-R-C-LFE-Lsur-Rsur-Lext-Rext interleaved)
        NumChannels = 8;
        dataFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_8CH;
    }

    // Open an audio port. Note: only 48000Hz sampling frequency is supported by AudioOut library
    int32_t	handle = sceAudioOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, 
        portType, 0,  SAMPLES_PER_PACKET, 48000 /*SampleRate*/, dataFormat);
    SF_ASSERT(handle > 0);
    AudioPort = handle;

    // Create sound thread
    SoundThrExit = false;
    if(pthread_create(&SoundThr, NULL, VideoSoundPS4::AudioProc, this))
    {
        DestroyOutput();
        return false;
    }
    return true;
}

void VideoSoundPS4::DestroyOutput()
{
    // Shutdown sound thread
    if(SoundThr)
    {
        SoundThrExit = true;
        pthread_join(SoundThr, NULL);
        SoundThr = 0;
    }

    // Release an audio port
    if(AudioPort)
    {
        int32_t ret = sceAudioOutClose(AudioPort);
        SF_ASSERT(ret == SCE_OK);
    }
}

void VideoSoundPS4::Start(PCMStream *pstream)
{
    pPCMStream = pstream;
    TotalSamples = 0;
    SoundStatus = Sound_Playing;

#ifndef GFC_NO_SOUND
    SetVolume(pChannel->GetVolume());
#endif
}

void VideoSoundPS4::Stop()
{
    SoundStatus = Sound_Stopped;
    pPCMStream = 0;
    TotalSamples = 0;
}

void VideoSoundPS4::Pause(bool sw)
{
    if(sw) {
        if (SoundStatus == Sound_Playing)
            SoundStatus = Sound_Paused;
    }
    else {
        if (SoundStatus == Sound_Paused)
            SoundStatus = Sound_Playing;
    }
}

void VideoSoundPS4::GetTime(UInt64* count, UInt64* unit) const
{
    if(TotalSamples == 0) {
        *count = 0;
        *unit  = 1000;
    }
    else {
        *count = TotalSamples * 1000 / SampleRate;
        *unit = 1000;
    }    
}

void VideoSoundPS4::SetVolume(float volume)
{
    Volume = volume;

    if(SoundStatus == Sound_Playing)
    {
        int32_t vol[NUM_CHANNELS];
        for(UInt32 i = 0; i < NUM_CHANNELS; ++i)
        {
            vol[i] = SCE_AUDIO_VOLUME_0dB * Volume;
        }

        // Set volume for all 8 channels
        int32_t ret = sceAudioOutSetVolume(AudioPort,
            (SCE_AUDIO_VOLUME_FLAG_FL_CH  | SCE_AUDIO_VOLUME_FLAG_FR_CH | SCE_AUDIO_VOLUME_FLAG_CNT_CH | SCE_AUDIO_VOLUME_FLAG_LFE_CH 
            | SCE_AUDIO_VOLUME_FLAG_RL_CH | SCE_AUDIO_VOLUME_FLAG_RR_CH | SCE_AUDIO_VOLUME_FLAG_BL_CH  | SCE_AUDIO_VOLUME_FLAG_BR_CH), vol);
        SF_ASSERT(ret == SCE_OK);
    }
}


//////////////////////////////////////////////////////////////////////////
//

void *VideoSoundPS4::AudioProc(void* arg)
{
    VideoSoundPS4* psnd = (VideoSoundPS4 *)arg;
    while(!psnd->SoundThrExit)
    {
        psnd->ExecuteAudioFrame();
        sceKernelUsleep(AUDIO_FRAME_INTERVAL);
    }
    return NULL;
}

void VideoSoundPS4::ExecuteAudioFrame()
{
    if(SoundStatus == Sound_Stopped)
        return;

    // Get PCM data
    float *sample[NUM_CHANNELS];
    for(UInt32 i = 0; i < NumChannels; ++i)
    {
        sample[i] = TempBuffer[i];
    }
    UInt32 wsmpl = 0;
    if (SoundStatus == Sound_Playing  && pPCMStream) 
    {
        wsmpl = pPCMStream->GetDataFloat(NumChannels, sample, SAMPLES_PER_PACKET);
    } 
    else 
    {
        memset(TempBuffer, 0, sizeof(TempBuffer));
        wsmpl = SAMPLES_PER_PACKET;
    }

    // Interleaving
    float *pcmbuf = SampleBuffer;
    for(UInt32 n = 0; n < wsmpl; ++n)
    {
        static UInt32 ch_map[NUM_CHANNELS] = {0, 1, 4, 5, 2, 3, 6, 7};
        for(UInt32 i = 0; i < NumChannels; ++i)
        {
            pcmbuf[n * NumChannels + i] = sample[ch_map[i]][n];
        }
    }

    // Submit PCM buffer
    int32_t ret = sceAudioOutOutput(AudioPort, pcmbuf);
    SF_ASSERT(ret > 0);

    // Wait for last data output to complete
    ret = sceAudioOutOutput(AudioPort, NULL);
    SF_ASSERT(ret > 0);

    TotalSamples += wsmpl;
}


//////////////////////////////////////////////////////////////////////////
//

VideoSound* VideoSoundSystemPS4::Create()
{
    return SF_HEAP_NEW(pHeap) VideoSoundPS4(pHeap);
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

