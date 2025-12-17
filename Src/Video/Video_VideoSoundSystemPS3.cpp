/**************************************************************************

Filename    :   Video_VideoSoundSystemPS3.cpp
Content     :   Video sound system implementation for PS3 based on MultiStream
Created     :   March 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemPS3.h"

#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_SystemSoundInterface.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

#include <string.h>
#include <cell/mstream.h>
#include <sys/timer.h>
#include <cell/sysmodule.h>
#include <sys/ppu_thread.h>

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundPS3 : public VideoSoundStream
{
public:
    VideoSoundPS3(MemoryHeap* pheap);
    virtual ~VideoSoundPS3();

    virtual bool        CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void        DestroyOutput();

    virtual PCMFormat   GetPCMFormat() const { return PCM_Float; };
    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const { return SoundStatus; }

    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    virtual void        SetVolume(float volume);
    virtual float       GetVolume();

private:
    static const UInt32 BUFFER_SIZE_PER_CH = 1024 * 8;

    MemoryHeap*         pHeap;
    PCMStream*          pPCMStream;

    UInt32              ChannelCount;
    Status              SoundStatus;    
    UInt64              TotalSamples;
    UInt64              SampleRate;
    float               Volume;

    int                 stream_id;
    float*              pcm_buffer_array[8];
    SInt32              pcm_buffer_size;

    void*               FirstBuffer;
    void*               SecondBuffer;
    volatile bool       close_callback_flag;
    volatile bool       finish_callback_flag;

    static void         StreamCallback(int streamNumber, void* userData, int cType, void* pWriteBuffer, int nBufferSize);
    UInt32              GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl);

    void                AdjustVolume();
};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundPS3::VideoSoundPS3(MemoryHeap* pheap) : pHeap(pheap), pPCMStream(0),
    ChannelCount(0), SoundStatus(Sound_Stopped), TotalSamples(0), SampleRate(0), Volume(1.0f)
{
    stream_id    = -1;
    FirstBuffer  = NULL;
    SecondBuffer = NULL;
    close_callback_flag  = false;
    finish_callback_flag = false;
}

VideoSoundPS3::~VideoSoundPS3()
{
}

bool VideoSoundPS3::CreateOutput(UInt32 channel, UInt32 samplerate)
{
    CellMSInfo MS_Info;

    if (channel == 0 || channel > 8)
        return false;

    SampleRate   = samplerate;
    ChannelCount = channel;
    close_callback_flag  = false;
    finish_callback_flag = false;

    stream_id = cellMSStreamOpen();

    // Which sub bus we are routing through
    MS_Info.SubBusGroup      = CELL_MS_MASTER_BUS;

    MS_Info.FirstBufferSize  = BUFFER_SIZE_PER_CH * ChannelCount;
    FirstBuffer              = SF_HEAP_MEMALIGN(pHeap, MS_Info.FirstBufferSize, 64, StatMV_Other_Mem);
    MS_Info.FirstBuffer      = this->FirstBuffer;
    memset(MS_Info.FirstBuffer, 0, MS_Info.FirstBufferSize);

    MS_Info.SecondBufferSize = BUFFER_SIZE_PER_CH * ChannelCount;
    SecondBuffer             = SF_HEAP_MEMALIGN(pHeap, MS_Info.SecondBufferSize, 64, StatMV_Other_Mem);
    MS_Info.SecondBuffer     = this->SecondBuffer;
    memset(MS_Info.SecondBuffer, 0, MS_Info.SecondBufferSize);

    // Set pitch and number of channels
    MS_Info.Pitch            = samplerate;
    MS_Info.numChannels      = channel;
    MS_Info.flags            = CELL_MS_STREAM_AUTOCLOSE;

    // Initial delay (in samples) before playback starts. Allows for sample accurate playback
    MS_Info.initialOffset    = 0;

    // Input data type (WAV = Float 32Bit)
    MS_Info.inputType        = CELL_MS_32BIT_FLOAT;

    cellMSStreamSetInfo(stream_id, &MS_Info);

    // Setup our callback
    // This is only needed if you're using cellMSSystemGenerateCallbacks();
    cellMSStreamSetCallbackData(stream_id, (void*)this);
    cellMSStreamSetCallbackFunc(stream_id, (CELL_MS_CALLBACK_FUNC)VideoSoundPS3::StreamCallback);

    AdjustVolume();

    UInt32 chno;
    for (chno = 0; chno < ChannelCount; chno++)
    {
        // Temporary PCM buffer for each channel
        pcm_buffer_array[chno] = (float*)SF_HEAP_MEMALIGN(pHeap, BUFFER_SIZE_PER_CH, 64, StatMV_Other_Mem);
        pcm_buffer_size = BUFFER_SIZE_PER_CH;
    }

    return true;
}

void VideoSoundPS3::AdjustVolume()
{
    if (ChannelCount == 6)
    {
        // 5.1ch
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FL , CELL_MS_CHANNEL_0, Volume);
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FR , CELL_MS_CHANNEL_1, Volume);
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FC , CELL_MS_CHANNEL_2, Volume); // C
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_LFE, CELL_MS_CHANNEL_3, Volume); // LFE
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_RL , CELL_MS_CHANNEL_4, Volume); // LS
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_RR , CELL_MS_CHANNEL_5, Volume); // RS
    }
    else if (ChannelCount == 2)
    {
        // Stereo
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FL, CELL_MS_CHANNEL_0, Volume);
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FR, CELL_MS_CHANNEL_1, Volume);
    } 
    else {
        // Monaural
        // Play audio from both front left and front right speakers
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FL, CELL_MS_CHANNEL_0, Volume * 0.7f);
        cellMSCoreSetVolume1(stream_id, CELL_MS_DRY, CELL_MS_SPEAKER_FR, CELL_MS_CHANNEL_0, Volume * 0.7f);
    }
}

void VideoSoundPS3::StreamCallback(int streamNumber, void *userData, int cType, void *pWriteBuffer, int nBufferSize)
{
    VideoSoundPS3 *mvsound = (VideoSoundPS3 *)userData;
    float* pbuf = (float *)pWriteBuffer;

    if (streamNumber != mvsound->stream_id)
        return;

    switch(cType)
    {
        case CELL_MS_CALLBACK_MOREDATA:
            UInt32 wsmpl1, chno, i;
            UInt32 req_smpl;
            memset(pWriteBuffer, 0, nBufferSize);
            if (mvsound->SoundStatus == Sound_Playing)
            {
                // Calculate the number of samples
                req_smpl = nBufferSize / mvsound->ChannelCount / sizeof(float);
                // Get PCM data to temporary buffer from CRI
                wsmpl1 = mvsound->GetSoundData(mvsound->ChannelCount, mvsound->pcm_buffer_array, req_smpl);
                // Copy and Interleave to the sound buffer
                for (i = 0; i < wsmpl1; i++)
                {
                    for (chno = 0; chno < mvsound->ChannelCount; chno++)
                        pbuf[mvsound->ChannelCount*i+chno] = mvsound->pcm_buffer_array[chno][i];
                }
            }
            break;

        case CELL_MS_CALLBACK_CLOSESTREAM:
            mvsound->close_callback_flag = true;
            break;

        case CELL_MS_CALLBACK_FINISHSTREAM:
            mvsound->finish_callback_flag = true;
            break;

        default:
            break;
    }

    return;
}

UInt32 VideoSoundPS3::GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl)
{
    UInt32 recv_nsmpl = 0;

    // Mono, Stereo or 5.1ch
    if (pPCMStream)
        recv_nsmpl = pPCMStream->GetDataFloat(nch, sample, nsmpl);

    TotalSamples += recv_nsmpl;
    return recv_nsmpl;
}

void VideoSoundPS3::DestroyOutput()
{
    if (SoundStatus != Sound_Stopped)
        Stop();

    UInt32 chno;
    for (chno = 0; chno < ChannelCount; chno++)
    {
        if (pcm_buffer_array[chno])
            SF_HEAP_FREE(pHeap, pcm_buffer_array[chno]);

        pcm_buffer_array[chno] = NULL;
        pcm_buffer_size = 0;
    }

    if (FirstBuffer)
    {
        SF_HEAP_FREE(pHeap, FirstBuffer);
        FirstBuffer = NULL;
    }
    if (SecondBuffer)
    {
        SF_HEAP_FREE(pHeap, SecondBuffer);
        SecondBuffer = NULL;
    }
}

void VideoSoundPS3::Start(PCMStream* pstream)
{
    if (stream_id < 0)
        return;

    pPCMStream   = pstream;
    SoundStatus  = Sound_Playing;
    TotalSamples = 0;

    // Play sound 
    cellMSStreamPlay(stream_id);
}

void VideoSoundPS3::Stop()
{
    if (stream_id < 0)
        return;

    // Stop sound
    cellMSStreamClose(stream_id);

    // Remove callback function
    cellMSStreamSetCallbackFunc(stream_id, NULL);
    cellMSStreamSetCallbackData(stream_id, NULL);

    int stream_stat;
    for (;;)
    {
        stream_stat = cellMSStreamGetStatus(stream_id);
        if (stream_stat && CELL_MS_STREAM_CLOSED == CELL_MS_STREAM_CLOSED)
            break;
    }

    // Reset stream ID
    stream_id   = -1;
    SoundStatus = Sound_Stopped;
}

void VideoSoundPS3::Pause(bool sw)
{
    if (sw)
    {
        if (SoundStatus == Sound_Playing)
            SoundStatus = Sound_Paused;
    }
    else
    {
        if (SoundStatus == Sound_Paused)
            SoundStatus = Sound_Playing;
    }    
}

void VideoSoundPS3::GetTime(UInt64* count, UInt64* unit) const
{
    if (TotalSamples == 0)
    {
        *count = 0;
        *unit  = 1000;
    }
    else
    {
        *count = TotalSamples * 1000 / SampleRate;
        *unit  = 1000;
    }    
}

void VideoSoundPS3::SetVolume(float volume)
{
    Volume = volume;
    if (SoundStatus == Sound_Playing)
        AdjustVolume();
}

float VideoSoundPS3::GetVolume()
{
    return Volume;
}

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundPS3Init : public NewOverrideBase<Stat_Video_Mem>
{
    unsigned int        AudioPort;
    void*               pMSMem;
    AtomicInt<int>      DoFinish;
    sys_ppu_thread_t    UpdateThread;

    static void MSUpdateThread(uint64_t psound);

public:
    VideoSoundPS3Init(CellSpurs* spurs);
};

void VideoSoundPS3Init::MSUpdateThread(uint64_t psound)
{
    VideoSoundPS3Init* pSound = (VideoSoundPS3Init*)psound;

    cellAudioPortStart(pSound->AudioPort);
    while(pSound->DoFinish == 0)
    {
        sys_timer_usleep(1000000 / 1920);
        cellMSSystemSignalSPU();
        cellMSSystemGenerateCallbacks();
    }
    cellAudioPortStop(pSound->AudioPort);
}

VideoSoundPS3Init::VideoSoundPS3Init(CellSpurs* spurs)
{
    cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
    cellMSSystemConfigureSysUtilEx(CELL_MS_AUDIOMODESELECT_SUPPORTSLPCM);

    int result = cellAudioInit();
    if (result != CELL_OK && result != CELL_AUDIO_ERROR_ALREADY_INIT)
    {
        SF_DEBUG_ERROR1(1, "VideoSoundPS3: cellAudioInit: %x\n", result);
        return;
    }

    CellAudioPortParam  PortParam;
    CellAudioPortConfig PortConfig;
    memset(&PortParam, 0, sizeof(CellAudioPortParam));
    memset(&PortConfig, 0, sizeof(CellAudioPortConfig));
    PortParam.nChannel = CELL_AUDIO_PORT_8CH;
    PortParam.nBlock   = CELL_AUDIO_BLOCK_8;

    result = cellAudioPortOpen(&PortParam, &AudioPort);
    if (result != CELL_OK)
    {
        SF_DEBUG_ERROR1(1, "VideoSoundPS3: cellAudioPortOpen: %x\n", result);
        cellAudioQuit();
        return;
    }

    result = cellAudioGetPortConfig(AudioPort, &PortConfig);
    if (result != CELL_OK)
    {
        SF_DEBUG_ERROR1(1, "VideoSoundPS3: cellAudioGetPortConfig: %x\n", result);
        cellAudioQuit();
        return;
    }

    cellMSSystemConfigureLibAudio(&PortParam, &PortConfig);

    CellMSSystemConfig cfg;
    memset(&cfg, 0, sizeof(CellMSSystemConfig));
    cfg.channelCount = 128;
    cfg.subCount     = 16;
    cfg.dspPageCount = 2;
    cfg.flags        = CELL_MS_ROUTABLE_STREAMS_FLAG | CELL_MS_DISABLE_SPU_PRINTF_SERVER;

    int memsize = cellMSSystemGetNeededMemorySize(&cfg);
    pMSMem = SF_MEMALIGN(memsize, 128, Stat_Default_Mem);

#ifdef CELLMS_SPU_THREADS
    result = cellMSSystemInitSPUThread(pMSMem, &cfg, 200);
#else
    uint8_t prios[8] = {1, 1, 0, 0, 0, 0, 0, 0};
    result = cellMSSystemInitSPURS(pMSMem, &cfg, spurs, prios);
#endif
    if (result != CELL_OK)
    {
        SF_DEBUG_ERROR1(1, "VideoSoundPS3: cellMSSystemInit: %x\n", cellMSSystemGetLastError());
        return;
    }

    float Volumes[64] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    cellMSCoreSetVolume64(CELL_MS_SUBBUS_1, CELL_MS_DRY, Volumes);
    cellMSCoreSetVolume64(CELL_MS_MASTER_BUS, CELL_MS_DRY, Volumes);
    DoFinish = 0;

    result = sys_ppu_thread_create(&UpdateThread, MSUpdateThread, (UPInt)this, 0, 0x4000,
        SYS_PPU_THREAD_CREATE_JOINABLE, "MS Update Thread");
}

//////////////////////////////////////////////////////////////////////////
//
static VideoSoundPS3Init* SoundInit = 0;

VideoSoundSystemPS3::VideoSoundSystemPS3(bool initialize, CellSpurs* pspurs, MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    if (initialize && SoundInit == 0)
        SoundInit = SF_NEW VideoSoundPS3Init(pspurs);
}

VideoSoundSystemPS3::~VideoSoundSystemPS3()
{
}

VideoSound* VideoSoundSystemPS3::Create()
{
    return new VideoSoundPS3(GetHeap());
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

