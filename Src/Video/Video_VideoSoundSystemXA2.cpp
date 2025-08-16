/**************************************************************************

Filename    :   Video_VideoSoundSystemXA2.cpp
Content     :   Video sound system implementation based on XAudio2
Created     :   March 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemXA2.h"

#ifdef GFX_ENABLE_VIDEO

#ifdef SF_OS_XBOX360
#include <xtl.h>
#else
#define _WIN32_DCOM
#define INITGUID
#include <windows.h>
#include <process.h>
#endif
#include <XAudio2.h>

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundXA2 : public VideoSoundStream
{
public:
    VideoSoundXA2(MemoryHeap* pheap, IXAudio2* pxa2, IXAudio2MasteringVoice* pmasteringVoice);
    virtual ~VideoSoundXA2();

    virtual bool        CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void        DestroyOutput();

    virtual PCMFormat   GetPCMFormat() const       { return PCM_Float; }
    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const          { return SoundStatus; }

    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    virtual void        SetVolume(float volume);
    virtual float       GetVolume() { return Volume; }

    static const UInt32 SAMPLING_RATE = 48000;
    static const UInt32 NUM_CHANNELS = 6;
    static const UInt32 SAMPLE_SIZE = sizeof(float);
#ifdef SF_OS_XBOX360
    static const UInt32 SAMPLES_PER_PACKET = 256;
    static const UInt32 AUDIO_FRAME_INTERVAL = 4;
#else
    static const UInt32 SAMPLES_PER_PACKET = 1024;
    static const UInt32 AUDIO_FRAME_INTERVAL = 10;
#endif
    static const UInt32 PACKET_SIZE = (NUM_CHANNELS * SAMPLE_SIZE * SAMPLES_PER_PACKET);
    static const UInt32 MAX_PACKETS = 3;

private:
    MemoryHeap*         pHeap;
    PCMStream*          pPCMStream;

    UInt32              ChannelCount;
    Status              SoundStatus;
    UInt64              TotalSamples;
    UInt64              SampleRate;
    float               Volume;

    IXAudio2*               pXA2;
    IXAudio2MasteringVoice* pMasteringVoice;
    // Source voice
    IXAudio2SourceVoice*    pSourceVoice;

    // Temporary buffer to receive PCM data
    float               temp_buffer[NUM_CHANNELS][SAMPLES_PER_PACKET];
    // Buffers to submit to source voice
    float               sound_buffer[MAX_PACKETS][PACKET_SIZE];
    // Write position
    UInt32              WritePosition;

    CRITICAL_SECTION    cs;
    bool                cs_initialized;

    HANDLE              hthread;
    volatile bool       thread_exit_flag;

#if defined(SF_OS_XBOX360)
    static DWORD WINAPI     AudioProc(void *arg);
#elif defined(_DURANGO)
    static DWORD WINAPI     AudioProc(LPVOID arg);
#else
    static unsigned WINAPI  AudioProc(void *arg);
#endif
    void                    ExecuteAudioFrame(void);
    UInt32                  GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl);
};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundXA2::VideoSoundXA2(MemoryHeap* pheap, IXAudio2* pxa2, IXAudio2MasteringVoice* pmasteringVoice) :
    pHeap(pheap), pPCMStream(0), ChannelCount(0), SoundStatus(Sound_Stopped),
    TotalSamples(0), SampleRate(0), Volume(1.0f)
{
    SF_ASSERT(pxa2 && pmasteringVoice);
    pXA2 = pxa2;
    pMasteringVoice = pmasteringVoice;
    pSourceVoice = NULL;
    hthread = NULL;
    WritePosition = 0;
    ZeroMemory(&cs, sizeof(cs));
    cs_initialized    = FALSE;

    InitializeCriticalSection(&cs);
    cs_initialized = TRUE;
}

VideoSoundXA2::~VideoSoundXA2()
{
    if (cs_initialized)
    {
        DeleteCriticalSection(&cs);
        cs_initialized = FALSE;
    }
}

bool VideoSoundXA2::CreateOutput(UInt32 channel, UInt32 samplerate)
{
    if (channel == 0)
        return false;

    ChannelCount = channel;
    SampleRate = samplerate;

    hthread = NULL;
    WritePosition = 0;

    // Configure WAVEFORMATEX structure
    HRESULT hr;
    WAVEFORMATEX wfex;
    ZeroMemory(&wfex, sizeof(wfex));
    wfex.cbSize = sizeof(wfex);
    wfex.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wfex.nChannels = NUM_CHANNELS;
    wfex.wBitsPerSample = SAMPLE_SIZE * 8;
    wfex.nSamplesPerSec = samplerate;
    wfex.nBlockAlign = (wfex.wBitsPerSample / 8) * wfex.nChannels;
    wfex.nAvgBytesPerSec = wfex.nBlockAlign * wfex.nSamplesPerSec;

    // Creation of source voice
    hr = pXA2->CreateSourceVoice(
        &pSourceVoice, &wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL);

    if (FAILED(hr)) 
    {
        DestroyOutput();
        return false;
    }

    thread_exit_flag = false;
    // Creation of thread
#if defined(SF_OS_XBOX360) || defined(_DURANGO)
    hthread = CreateThread(NULL, 0, AudioProc, this, CREATE_SUSPENDED, NULL);
#else
    hthread = (HANDLE)_beginthreadex(NULL, 0, AudioProc, this, CREATE_SUSPENDED, NULL);
#endif

    if (hthread == NULL)
    {
        DestroyOutput();
        return false;
    }

    // Change the priority of the thread to THREAD_PRIORITY_HIGHEST
    SetThreadPriority(hthread, THREAD_PRIORITY_HIGHEST);

#ifdef SF_OS_XBOX360
    // If needed, change the processor of the thread
    XSetThreadProcessor(hthread, 5);
#endif

    // Start thread processing
    ResumeThread(hthread);

    // Call ExecuteAudioFrame function to invoke callback
    ExecuteAudioFrame();

    // Start playback
    pSourceVoice->Start(0, 0);
    return true;
}

void VideoSoundXA2::DestroyOutput()
{
    // Release of the thread
    if (hthread) {
        // Wait until thread processing ends
        thread_exit_flag = true;
        WaitForSingleObject(hthread, INFINITE);

        // Release of the thread handle
        CloseHandle(hthread);
        hthread = NULL;
    }

    // Release of the source voice
    if (pSourceVoice != NULL) {
        pSourceVoice->Stop(0);
        pSourceVoice->FlushSourceBuffers();
        pSourceVoice->DestroyVoice();
        pSourceVoice = NULL;
    }
}

void VideoSoundXA2::Start(PCMStream* pstream)
{
    EnterCriticalSection(&cs);
    pPCMStream  = pstream;
    SoundStatus = Sound_Playing;
    LeaveCriticalSection(&cs);

    pSourceVoice->SetVolume(Volume);
    TotalSamples = 0;
}

void VideoSoundXA2::Stop()
{
    EnterCriticalSection(&cs);
    SoundStatus = Sound_Stopped;
    pPCMStream  = 0;
    LeaveCriticalSection(&cs);
    TotalSamples = 0;
}

void VideoSoundXA2::Pause(bool sw)
{
    EnterCriticalSection(&cs);
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
    LeaveCriticalSection(&cs);
}

void VideoSoundXA2::GetTime(UInt64* count, UInt64* unit) const
{
    if (TotalSamples == 0)
    {
        *count = 0;
        *unit  = 1000;
    }
    else
    {
        *count = TotalSamples * 1000 / SampleRate;
        *unit = 1000;
    }    
}

void VideoSoundXA2::SetVolume(float volume)
{
    Volume = volume;
    if (SoundStatus == Sound_Playing)
        pSourceVoice->SetVolume(volume);
}

void VideoSoundXA2::ExecuteAudioFrame(void)
{
    UInt32 i, j, n;
    float *sample[MaxChannels];
    UInt32 wsmpl;

    EnterCriticalSection(&cs);

    // Get PCM data
    for (i = 0; i < NUM_CHANNELS; i++) 
        sample[i] = temp_buffer[i];

    if (SoundStatus == Sound_Playing) 
    {
        wsmpl = GetSoundData(NUM_CHANNELS, sample, SAMPLES_PER_PACKET);
    } 
    else 
    {
        ZeroMemory(temp_buffer, sizeof(temp_buffer));
        wsmpl = SAMPLES_PER_PACKET;
    }

    // Interleaving
    float *pcmbuf = sound_buffer[WritePosition];
    for (j = 0; j < wsmpl; j++) {
        static UInt32 ch_map[8]={ 0, 1, 4, 5, 2, 3, 6, 7 };
        for (n = 0; n < NUM_CHANNELS; n++) {
            pcmbuf[j * NUM_CHANNELS + n] = sample[ch_map[n]][j];
        }
    }

    // Submit PCM buffer
    if (wsmpl > 0) {
        XAUDIO2_BUFFER buf = {0};
        buf.AudioBytes = wsmpl * NUM_CHANNELS * SAMPLE_SIZE;
        buf.pAudioData = (BYTE *)pcmbuf;
        pSourceVoice->SubmitSourceBuffer(&buf);
    }

    // Update write position
    WritePosition++;
    if(WritePosition >= MAX_PACKETS) {
        WritePosition = 0;
    }

    LeaveCriticalSection(&cs);

    return;
}

// Audio processing thread
// This thread is waked up from XAudio2 callback
#if defined(SF_OS_XBOX360)
DWORD WINAPI VideoSoundXA2::AudioProc(void *arg)
#elif defined(_DURANGO)
DWORD WINAPI VideoSoundXA2::AudioProc(LPVOID arg)
#else
unsigned WINAPI VideoSoundXA2::AudioProc(void *arg)
#endif
{
    VideoSoundXA2 *sndout = (VideoSoundXA2*)arg;

    while (sndout->thread_exit_flag == false) 
    {
        // Transfer audio data. Send all available packets
        XAUDIO2_VOICE_STATE state;
        sndout->pSourceVoice->GetState(&state);
        for (int i = state.BuffersQueued; i < MAX_PACKETS; i++) 
        {
            // Execute sound output processing
            sndout->ExecuteAudioFrame();
        }

        // Sleep until next audio frame
        Sleep(AUDIO_FRAME_INTERVAL);
    }

    return (0);
}

UInt32 VideoSoundXA2::GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl)
{
    UInt32 recv_nsmpl = 0;

    if (pPCMStream)
    {
        if (ChannelCount == 1)
        {
            // Mono
            UInt32 i, center_ch = 4;
            for (i = 0; i < nch; i++)
            {
                if (i != center_ch)
                    ZeroMemory(sample[i], nsmpl * sizeof(float));
            }
            recv_nsmpl = pPCMStream->GetDataFloat(1, &sample[4], nsmpl);
        }
        else
        {
            // Stereo or 5.1ch
            recv_nsmpl = pPCMStream->GetDataFloat(nch, sample, nsmpl);
        }
    }

    TotalSamples += recv_nsmpl;
    return recv_nsmpl;
}


//////////////////////////////////////////////////////////////////////////
//
class VideoSoundSystemXA2Impl : public NewOverrideBase<StatMD_Other_Mem>
{
public:
    VideoSoundSystemXA2Impl(IXAudio2* pxa2, IXAudio2MasteringVoice* pmasteringVoice)
    {
        InitializedByUs = false;

        HRESULT hr;
        UINT32 flags;

        if (pxa2)
        {
            SF_ASSERT(pmasteringVoice);
            pXAUDIO2     = pxa2;
            pMasterVoice = pmasteringVoice;
            return;
        }
#if !(defined(SF_OS_XBOX360) || defined(_DURANGO))
        hr = CoInitialize(NULL);
#endif

        // Selection of initialization method
        flags = 0;
#if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
        flags |= XAUDIO2_DEBUG_ENGINE;
#endif

        // Creation of XAudio2 object
        hr = XAudio2Create(&pXAUDIO2.GetRawRef(), 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(hr)) 
        {
            Finalize();
            return;
        }
        // Creation of mastering voice
        hr = pXAUDIO2->CreateMasteringVoice(&pMasterVoice,
            XAUDIO2_DEFAULT_CHANNELS, VideoSoundXA2::SAMPLING_RATE, 0, 0, NULL);
        if (FAILED(hr)) 
        {
            pMasterVoice = NULL;
            Finalize();
            return;
        }
        InitializedByUs = true;
    }

    ~VideoSoundSystemXA2Impl()
    {
        Finalize();
    }

    void Finalize(void)
    {
        // Release of the mastering voice
        if (pMasterVoice && InitializedByUs) 
        {
            pMasterVoice->DestroyVoice();
            pMasterVoice = NULL;
        }

        // Release of the XAudio2 object
        pXAUDIO2 = NULL;

#if !(defined(SF_OS_XBOX360) || defined(_DURANGO))
        if (InitializedByUs) 
        {
            CoUninitialize();
        }
#endif
    }

    IXAudio2*                GetXAudio2() const      { return pXAUDIO2.GetPtr(); }
    IXAudio2MasteringVoice*  GetMasterVoice() const  { return pMasterVoice; }

protected:
    Ptr<IXAudio2>            pXAUDIO2;
    IXAudio2MasteringVoice*  pMasterVoice;

    // InitializedByUs flag is set to 'true' if XAudio2 object has been
    // created locally instead of bing passed by the user
    bool InitializedByUs;
};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundSystemXA2::VideoSoundSystemXA2(IXAudio2* pxa2, IXAudio2MasteringVoice* pmasteringVoice, 
                                         MemoryHeap* pheap) : VideoSoundSystem(pheap)
{
    pImpl = SF_HEAP_NEW(pheap) VideoSoundSystemXA2Impl(pxa2, pmasteringVoice);
}

VideoSoundSystemXA2::~VideoSoundSystemXA2()
{
    delete pImpl;
}

VideoSound* VideoSoundSystemXA2::Create()
{
    IXAudio2* pxa2 = pImpl->GetXAudio2();
    IXAudio2MasteringVoice* pmasteringVoice = pImpl->GetMasterVoice();
    if (pxa2 && pmasteringVoice)
        return SF_HEAP_NEW(GetHeap()) VideoSoundXA2(GetHeap(), pxa2, pmasteringVoice);
    return NULL;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
