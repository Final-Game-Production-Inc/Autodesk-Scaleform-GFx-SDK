/**************************************************************************

Filename    :   Video_VideoSoundSystemDX8.cpp
Content     :   Video sound system implementation based on DirectSound
Created     :   March 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemDX8.h"

#ifdef GFX_ENABLE_VIDEO

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma warning(disable : 4201)
#endif

#include <dsound.h>
#include <ks.h>
#include <ksmedia.h>
#include <windows.h>
#include <process.h>

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundDX8 : public VideoSoundStream
{
public:
    VideoSoundDX8(MemoryHeap* pheap, IDirectSound8* ds);
    virtual ~VideoSoundDX8();

    virtual bool        CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void        DestroyOutput();

    virtual PCMFormat   GetPCMFormat() const { return PCM_Float; }
    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const { return SoundStatus; }

    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    // Normalized value (0.0f - 1.0f)
    virtual void        SetVolume(float volume);
    virtual float       GetVolume() { return Volume; }

    // Variables borrowed from CRI CriSmpMvSoundPc
    static const UInt32 NSMPL_BLK = 128;
    static const UInt32 BUF_NSMPL = 3200;
    static const UInt32 PLAY_THRESHOLD = 800;
    static const UInt32 MAX_NUM_SPEAKERS = 6;
    static const UInt32 DEFAULT_SRATE = 48000;

private:
    MemoryHeap*         pHeap;
    PCMStream*          pPCMStream;

    UInt32				ChannelCount;
    Status	            SoundStatus;    
    UInt64				TotalSamples;
    UInt64				SampleRate;
    float				Volume;

    // PC specific members
    LPDIRECTSOUND8		pDS8;		    // DirectSound
    LPDIRECTSOUNDBUFFER pDSBuffer;		// DirectSound secondary sound buffer
    UInt32              pRead;          // Read pointer (SAMPLE)
    UInt32              pWrite;         // Write pointer (SAMPLE)
    UInt32              ndata;          // Number of samples in the buffer
    UInt32              nch;            // Number of channels
                                        // Sample buffer for receiving from App
    float               sample_buffer[MaxChannels][BUF_NSMPL];
    CRITICAL_SECTION    cs;
    bool                cs_initialized;
    HANDLE              hevent;
    HANDLE              hthread;
    unsigned int        thread_id;
    UINT                timer_id;
    volatile bool       thread_exit_flag;

private:
    static unsigned WINAPI  SoundOutputTimerProc(LPVOID param);
    void                    ExecuteAudioFrame(void);
    UInt32                  GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl);

protected:
    inline SInt16 PcmfToPcm16(float pcmf)
    {
        if ((pcmf > -1.0f) && (pcmf <= 1.0f))
            return (SInt16)(pcmf*32767.0f);
        else if (pcmf > 1.0f )
            return 32767;
        else
            return -32768;
    }

    inline SInt32 LinearToDecibel(float linear)
    {
        SInt32	decibel;

        if (linear >= 1.0f)
            decibel = DSBVOLUME_MAX;
        else if (linear > 0.00001f)
            decibel = ((SInt32)(2000.0f * log10(linear)));
        else
            decibel = DSBVOLUME_MIN;
        return decibel;
    }

};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundDX8::VideoSoundDX8(MemoryHeap* pheap, IDirectSound8* pds) :
    pHeap(pheap), pPCMStream(0),
    ChannelCount(0), SoundStatus(Sound_Stopped),
    TotalSamples(0), SampleRate(0), Volume(1.0f)
{        
    pDS8              = pds;
    pDSBuffer         = 0;
    hevent            = NULL;
    hthread           = NULL;
    ZeroMemory(&cs, sizeof(cs));
    cs_initialized    = FALSE;

    InitializeCriticalSection(&cs);
    cs_initialized = TRUE;
}

VideoSoundDX8::~VideoSoundDX8()
{    
    if (cs_initialized)
    {
        DeleteCriticalSection(&cs);
        cs_initialized = FALSE;
    }
}

bool VideoSoundDX8::CreateOutput(UInt32 channel, UInt32 samplerate)
{   
    SoundStatus   = Sound_Stopped;
    pDSBuffer     = NULL;
    pRead         = 0;
    pWrite        = 0;
    ndata         = 0;
    nch           = MAX_NUM_SPEAKERS;
    ZeroMemory(&sample_buffer, sizeof(sample_buffer));
    hevent         = NULL;
    hthread        = NULL;
    thread_id      = 0;
    timer_id       = 0;
    thread_exit_flag = FALSE;

    if (channel == 0)
        return false;

    WAVEFORMATEXTENSIBLE wfext;
    WAVEFORMATEX *       pwfex;
    ZeroMemory(&wfext, sizeof(wfext));
    pwfex = &wfext.Format;

    // Configure WAVEFORMATEX structure
    // NOTE: number of channels of secondary buffer is always MAX_NUM_SPEAKERS
    pwfex->wFormatTag       = WAVE_FORMAT_EXTENSIBLE;
    pwfex->nChannels        = (WORD)nch;
    // pwfex->nSamplesPerSec = DEFAULT_SRATE;
    pwfex->nSamplesPerSec   = samplerate;
    pwfex->wBitsPerSample   = 16;
    pwfex->nBlockAlign      = pwfex->nChannels * pwfex->wBitsPerSample / 8;
    pwfex->nAvgBytesPerSec  = pwfex->nBlockAlign * pwfex->nSamplesPerSec;
    pwfex->cbSize           = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

    // Configure WAVEFORMATEXTENSIBLE structure
    wfext.Samples.wValidBitsPerSample =  pwfex->wBitsPerSample;
    wfext.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
    wfext.SubFormat     = KSDATAFORMAT_SUBTYPE_PCM;

    // Create secondary sound buffer
    DSBUFFERDESC dsbdesc;
    ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize  = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS
                     | DSBCAPS_LOCDEFER | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
    dsbdesc.dwBufferBytes = BUF_NSMPL * pwfex->nBlockAlign;
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&wfext;
    HRESULT hr = pDS8->CreateSoundBuffer(&dsbdesc, &pDSBuffer, NULL);
    if (FAILED(hr)) {
        DestroyOutput();
        return false;
    }

    // Create an event object for periodical processing
    hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hevent == NULL) {
        DestroyOutput();
        return false;
    }
    timer_id = timeSetEvent(16, 0, (LPTIMECALLBACK)hevent,
        (DWORD)NULL, TIME_PERIODIC | TIME_CALLBACK_EVENT_PULSE);

    // Create thread for playback
    hthread = (HANDLE)_beginthreadex(NULL, 0, SoundOutputTimerProc,
                                     this, CREATE_SUSPENDED, &thread_id);
    if (hthread == NULL) {
        DestroyOutput();
        return false;
    }
    // Change the priority of the thread to THREAD_PRIORITY_HIGHEST
    SetThreadPriority(hthread, THREAD_PRIORITY_HIGHEST);
    ResumeThread(hthread);

    ChannelCount = channel;
    SampleRate   = samplerate;
    SInt32	decibel = LinearToDecibel(Volume);

    EnterCriticalSection(&cs);
    if (pDSBuffer != NULL) {
        pDSBuffer->SetVolume(decibel);
    }
    LeaveCriticalSection(&cs);
    
#ifdef GFX_ENABLE_SOUND
    SetVolume(pChannel->GetVolume());
#endif
    return true;
}

void VideoSoundDX8::DestroyOutput()
{
    if (hthread)
    {
        // Wait for thread end
        thread_exit_flag = TRUE;
        WaitForSingleObject(hthread, INFINITE);

        // Release the thread handle
        CloseHandle(hthread);
        hthread = NULL;
    }

    if (hevent)
    {
        // Kill timer event
        timeKillEvent(timer_id);

        // Release the event handle
        CloseHandle(hevent);
        hevent = NULL;
    }

    if (pDSBuffer != NULL) {
        // Release the secondary buffer
        pDSBuffer->Release();
        pDSBuffer = NULL;
    }

    ChannelCount     = 0;
    SoundStatus      = Sound_Stopped;
    TotalSamples = 0;
}


void VideoSoundDX8::Start(PCMStream* pstream)
{
    EnterCriticalSection(&cs);
    pPCMStream  = pstream;
    SoundStatus = Sound_Playing;
    SetVolume(Volume);
    LeaveCriticalSection(&cs);

    TotalSamples = 0;
}

void VideoSoundDX8::Stop()
{
    EnterCriticalSection(&cs);
    SoundStatus = Sound_Stopped;
    pPCMStream  = 0;
    LeaveCriticalSection(&cs);
    TotalSamples = 0;
}

void VideoSoundDX8::Pause(bool sw)
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

void VideoSoundDX8::GetTime(UInt64* count, UInt64* unit) const
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

void VideoSoundDX8::SetVolume(float volume)
{
    Volume = volume;
    if (SoundStatus == Sound_Playing)
    {
        SInt32	decibel = LinearToDecibel(volume);

        EnterCriticalSection(&cs);
        if (pDSBuffer)
            pDSBuffer->SetVolume(decibel);
        LeaveCriticalSection(&cs);
    }
}

unsigned WINAPI VideoSoundDX8::SoundOutputTimerProc(LPVOID param)
{
    VideoSoundDX8 *pthis = (VideoSoundDX8*)param;

    while (pthis->thread_exit_flag == FALSE)
    {
        // Sleep until audio frame
        WaitForSingleObject(pthis->hevent, INFINITE);

        // Execute sound output processing
        pthis->ExecuteAudioFrame();
    }

    return 0;
}

void VideoSoundDX8::ExecuteAudioFrame()
{
    DWORD dsb_stat;
    float *sample[MaxChannels];
    UInt32 nsmpl, wsmpl;
    HRESULT hr;

    EnterCriticalSection(&cs);

    if (SoundStatus != Sound_Playing)
    {
        pDSBuffer->Stop();
        LeaveCriticalSection(&cs);
        return;
    }

    pDSBuffer->GetStatus(&dsb_stat);

    // Update read pointer
    if (dsb_stat & DSBSTATUS_PLAYING)
    {
        UInt32 cpos, wpos;

        // Because this file is always compiled on Windows, int is equal to long ...
        pDSBuffer->GetCurrentPosition(reinterpret_cast<LPDWORD>(&cpos),
                                      reinterpret_cast<LPDWORD>(&wpos));
        cpos /= nch*sizeof(SInt16);
        if (cpos >= pRead) {
            ndata -= cpos - pRead;
        } else {
            ndata -= BUF_NSMPL - (pRead - cpos);
        }
        pRead = cpos;
    }

    nsmpl = BUF_NSMPL - ndata;
    if (nsmpl > BUF_NSMPL) {
        nsmpl = BUF_NSMPL;
    }
    nsmpl = nsmpl / NSMPL_BLK * NSMPL_BLK;

    //	Transfer PCM data to DirectSound buffer
    SInt16 *pcmbuf[2];
    UInt32 bsize[2], bsmpl[2], i, j, k, n;

    for (i=0; i<nch; i++)
        sample[i] = sample_buffer[i];

    wsmpl = GetSoundData(nch, sample, nsmpl);

    if (wsmpl > 0)
    {
        // Because this file is always compiled on Windows, int is equal to long ...
        hr = pDSBuffer->Lock(pWrite*nch*sizeof(SInt16), wsmpl*nch*sizeof(SInt16),
                             (LPVOID*)&pcmbuf[0], reinterpret_cast<LPDWORD>(&bsize[0]),
                             (LPVOID*)&pcmbuf[1], reinterpret_cast<LPDWORD>(&bsize[1]), 0);
        if(FAILED(hr)) {
            LeaveCriticalSection(&cs);
            return;
        }

        bsmpl[0] = bsize[0]/(nch*sizeof(SInt16));
        bsmpl[1] = bsize[1]/(nch*sizeof(SInt16));
        i = 0;
        // Interleaving
        for (k=0; k<2; k++)
        {
            for (j=0; j<bsmpl[k]; j++)
            {
                static UInt32 ch_map[8]={0, 1, 4, 5, 2, 3, 6, 7};
                if (i >= wsmpl)
                    break;
                for (n=0; n<nch; n++) {
                    pcmbuf[k][j*nch+n] = PcmfToPcm16(sample[ch_map[n]][i]);
                }
                i++;
            }
        }
        
        if (i <  bsmpl[0])
        {
            bsize[0] = i*(nch*sizeof(SInt16));
            bsize[1] = 0;
        } else if (i < bsmpl[0] + bsmpl[1]) {
            bsize[1] = (i - bsmpl[0]) * (nch*sizeof(SInt16));
        }
        pDSBuffer->Unlock((LPVOID)pcmbuf[0], bsize[0], (LPVOID)pcmbuf[1], bsize[1]);
    }

    // Update write pointer
    ndata += wsmpl;
    pWrite += wsmpl;
    if (pWrite >= BUF_NSMPL)
        pWrite = pWrite % BUF_NSMPL;

    // Playback control
    if ((dsb_stat & DSBSTATUS_PLAYING) == 0)
    { 
        if (ndata >= PLAY_THRESHOLD) {
            pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
        }
    }

    LeaveCriticalSection(&cs);
}

UInt32 VideoSoundDX8::GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl)
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
                    memset(sample[i], 0, nsmpl * sizeof(float));
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

// This is the main VideoSoundSystem implementation class, storing
// such things as:
//  - IDirectSound interface
//  - List of video sound instances.

const static char GFx_VideoSoundWindowClass[] = "GFxVideoSoundWindowClass";

class VideoSoundSystemDX8Impl : public NewOverrideBase<StatMD_Other_Mem>
{
public:
    VideoSoundSystemDX8Impl(IDirectSound8 *pds8)
    {        
        hWnd            = 0;
        InitializedByUs = false;

        // If user passed IDirectSound8 to us, just store it and we are done
        if (pds8)
        {
            pDS8 = pds8;
            return;
        }
       
        // Get the instance handle
        HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
        if (hinst == NULL) {
            Finalize();
            return;
        }

        // Register window class
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = DefWindowProc;
        wc.lpszClassName = GFx_VideoSoundWindowClass;
        ATOM atom = RegisterClass(&wc);
        if (atom == 0) {
            Finalize();
            return;
        }

        // Create dummy window
        hWnd = CreateWindow(GFx_VideoSoundWindowClass, "GFxVideoSoundOutput",
                            0, 0, 0, 0, 0, NULL, NULL, hinst, 0L);
        if (!hWnd)
        {
            Finalize();
            return;
        }

        // Create Direct Sound Object
        HRESULT hr;
        hr = DirectSoundCreate8(NULL, &pDS8.GetRawRef(), NULL);
        if (FAILED(hr)) {
            Finalize();
            return;
        }

        // Set cooperative level
        hr = pDS8->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
        if (FAILED(hr)) {
            Finalize();
            return;
        }

        // Create a primary buffer
        DSBUFFERDESC dsbdesc;
        ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
        dsbdesc.dwSize = sizeof(DSBUFFERDESC);
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        hr = pDS8->CreateSoundBuffer(&dsbdesc, &pDSBPrimary.GetRawRef(), NULL);
        if (FAILED(hr)) {
            Finalize();
            return;
        }

        // Set format of the primary buffer
        WAVEFORMATEX wfx;
        ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
        wfx.wFormatTag		= WAVE_FORMAT_PCM;
        wfx.nChannels		= VideoSoundDX8::MAX_NUM_SPEAKERS;
        wfx.nSamplesPerSec	= VideoSoundDX8::DEFAULT_SRATE;
        wfx.wBitsPerSample	= 16;
        wfx.nBlockAlign		= wfx.wBitsPerSample / 8 * wfx.nChannels;
        wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
        hr = pDSBPrimary->SetFormat(&wfx);
        if (FAILED(hr))
        {
            // If the function failed, retry it with minimum format
            wfx.nChannels       = 2;
            wfx.nSamplesPerSec  = 44100;
            wfx.wBitsPerSample	= 16;
            wfx.nBlockAlign		= wfx.wBitsPerSample / 8 * wfx.nChannels;
            wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
            hr = pDSBPrimary->SetFormat(&wfx);
            if (SUCCEEDED(hr))
            {
                InitializedByUs = true;
            }
            else
            {
                Finalize();
            }
        }
        else
        {
            InitializedByUs = true;
        }
    }

    ~VideoSoundSystemDX8Impl()
    {
        Finalize();
    }

    void Finalize(void)
    {
        // Release DirectSound interfaces
        pDSBPrimary = 0;
        pDS8        = 0;

        if (hWnd)
        {
            DestroyWindow(hWnd);
            hWnd = NULL;
            UnregisterClass(GFx_VideoSoundWindowClass,
                            (HINSTANCE)GetModuleHandle(NULL));
        }
    }

    IDirectSound8* GetDirectSound() const
    {
        return pDS8.GetPtr();
    }

protected:    
    Ptr<IDirectSound8>       pDS8;
    Ptr<IDirectSoundBuffer>  pDSBPrimary;
    HWND                     hWnd;

    // InitializedByUs flag is set to 'true' if DirectSound object has been
    // created locally instead of bing passed by the user
    bool InitializedByUs;
};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundSystemDX8::VideoSoundSystemDX8(IDirectSound8 *pds8, MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    pImpl = SF_HEAP_NEW(pheap) VideoSoundSystemDX8Impl(pds8);
}

VideoSoundSystemDX8::~VideoSoundSystemDX8()
{
    delete pImpl;
}

VideoSound* VideoSoundSystemDX8::Create()
{
    IDirectSound8* pds8 = pImpl ? pImpl->GetDirectSound() : 0;
    if (pds8)
        return SF_HEAP_NEW(GetHeap()) VideoSoundDX8(GetHeap(), pds8);
    return 0;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
