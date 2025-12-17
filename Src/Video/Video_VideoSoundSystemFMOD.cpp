/**************************************************************************

Filename    :   Video_VideoSoundSystemFMOD.cpp
Content     :   Video sound system implementation based on FMOD sound library
Created     :   July 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemFMOD.h"

#ifdef GFX_ENABLE_VIDEO

#include <fmod.hpp>
#include <fmod_errors.h>

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Alg.h"

#define THREAD_STACK_SIZE   16384

namespace Scaleform { namespace GFx { namespace Video {

class VideoSoundFMOD;

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundSystemFMODImpl : public NewOverrideBase<StatMD_Other_Mem>
{
public:
#ifdef SF_OS_PS3
    VideoSoundSystemFMODImpl(FMOD::System* pfmod, const CellSpurs* pSpurs);
#else
    VideoSoundSystemFMODImpl(FMOD::System* pfmod);
#endif
    ~VideoSoundSystemFMODImpl();

    void                        AttachSound(VideoSoundFMOD*);
    void                        DetachSound(VideoSoundFMOD*);
    void                        Finalize();

    void                        LogError(FMOD_RESULT);
    void                        PulseEvent()  { Event.PulseEvent(); }

    FMOD::System*               GetFMOD() const { return pFMOD; }

protected:
    static int                  UpdateFunc(Thread*, void* h);
    float                       UpdateAuxStreams();

    Array<VideoSoundFMOD*>      Sounds;
    Lock                        SoundsLock;

    FMOD::System*               pFMOD;
    Ptr<Thread>                 pUpdateThread;
    Scaleform::Event            Event;
    volatile bool               StopThread;

    bool                        InitializedByUs;

public:
    static MemoryHeap           *pFMODHeap;
};

//////////////////////////////////////////////////////////////////////////
//
#define AUX_SOUND_READBUFLEN_MS 300
#define AUX_SOUND_LEN_MS (AUX_SOUND_READBUFLEN_MS * 5)

class VideoSoundFMOD : public VideoSoundStream
{
public:
    VideoSoundFMOD(MemoryHeap* pheap, VideoSoundSystemFMODImpl*);
    virtual ~VideoSoundFMOD();

    virtual bool        CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void        DestroyOutput();

    virtual PCMFormat   GetPCMFormat() const       { return PCM_SInt16; }
    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const          { return SoundStatus; }

    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    virtual void        SetVolume(float volume);
    virtual float       GetVolume() { return Volume; }

    float               Update();

private:
    MemoryHeap*         pHeap;
    PCMStream*          pPCMStream;

    UInt32              Channels;
    Status              SoundStatus;    
    UInt32              SampleRate;
    float               Volume;

    VideoSoundSystemFMODImpl* pSystem;
    FMOD::Sound*        pSound;
    FMOD::Channel*      pChannel;

    UByte*              pBlockBuffer;
    unsigned            BlockSize;
    unsigned            SoundLength;

    unsigned            FillPosition;
    unsigned            TotatBytesRead;

    UInt64              StartTick;
    UInt64              StopTick;
    UInt64              TotalTicks;
    UInt64              NextFillTick;
    bool                Starving;
    Lock                ChannelLock;

    unsigned ReadAndFillSound();
    void ClearSoundBuffer();

    UInt64 GetTotalBytesReadInTicks()   const { return BytesToTicks(TotatBytesRead); }
    UInt64 BytesToTicks(unsigned bytes) const { return UInt64(bytes)*8/16/Channels * 1000000/SampleRate; }
    inline unsigned DistToFillBuffPos(unsigned pos) const;

    // Temporary buffer to receive PCM data
    SInt16              *TmpBuffers[6];

    unsigned            GetSoundData(UByte* pdata, unsigned datasize);
};

//////////////////////////////////////////////////////////////////////////
//

VideoSoundFMOD::VideoSoundFMOD(MemoryHeap* pheap, VideoSoundSystemFMODImpl* psystem) :
    pHeap(pheap), pPCMStream(0),
    Channels(0), SoundStatus(Sound_Stopped),
    SampleRate(0), Volume(1.0f)
{
    SF_ASSERT(psystem);
    pSystem = psystem;
    pSound = NULL;
    pChannel = NULL;
    pBlockBuffer = NULL;
    FillPosition = 0;
    TotatBytesRead = 0;
    for(unsigned i = 0; i < 6; ++i)
        TmpBuffers[i] = NULL;
}

VideoSoundFMOD::~VideoSoundFMOD()
{
}

bool VideoSoundFMOD::CreateOutput(UInt32 channel, UInt32 samplerate)
{
    if (channel == 0) {
        return false;
    }

    Lock::Locker lock(&ChannelLock);
    Channels   = channel;
    SampleRate = samplerate;
    SoundLength = (AUX_SOUND_LEN_MS * (SampleRate /1000)) * Channels * (16 /8);

    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length            = SoundLength; // Length of PCM data in bytes of whole song (for Sound::getLength)
    exinfo.numchannels       = Channels;    // Number of channels in the sound.
    exinfo.defaultfrequency  = SampleRate;  // Default playback rate of sound.
    exinfo.format            = FMOD_SOUND_FORMAT_PCM16;

    FMOD_MODE flags =  FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_HARDWARE;
    FMOD_RESULT result = pSystem->GetFMOD()->createSound(0, flags, &exinfo, &pSound);
    if (result != FMOD_OK)
    {
        pSound = NULL;
        pSystem->LogError(result);
        return false;
    }
    unsigned int sl = 0;
    pSound->getLength(&sl, FMOD_TIMEUNIT_PCMBYTES);
    SF_ASSERT(sl == SoundLength);

    return true;
}

void VideoSoundFMOD::DestroyOutput()
{
    // Release of the thread
    Lock::Locker lock(&ChannelLock);
    SF_ASSERT(!pChannel);
    if (pSound)
        pSound->release();
    pSound = NULL;
}

void VideoSoundFMOD::Start(PCMStream* pstream)
{
    Lock::Locker lock(&ChannelLock);
    SF_ASSERT(pSound);
    FMOD_RESULT result = pSystem->GetFMOD()->playSound(FMOD_CHANNEL_REUSE, pSound, false, &pChannel);
    if (result != FMOD_OK)
    {
        pSystem->LogError(result);
        pSound->release();
        pSound = NULL;
        pChannel = NULL;
        return;
    }
    BlockSize = (AUX_SOUND_READBUFLEN_MS * SampleRate/1000) * Channels * 2;
    if (pBlockBuffer)
        SF_FREE(pBlockBuffer);
    pBlockBuffer = (UByte*)SF_ALLOC(BlockSize, Stat_Sound_Mem);

    UInt32 nsmpl = BlockSize * 8 / 16 / Channels;
    for(unsigned i = 0; i < Channels; ++i)
    {
        TmpBuffers[i] = (SInt16*)SF_HEAP_ALLOC(pHeap, sizeof(SInt16) * nsmpl, Stat_Video_Mem);
        if (!TmpBuffers[i])
        {
            while (i > 0)
            {
                i--;
                SF_HEAP_FREE(pHeap, TmpBuffers[i]);
                TmpBuffers[i] = NULL;
            }
            return;
        }
    }

    pPCMStream  = pstream;
    SoundStatus = Sound_Playing;
    TotatBytesRead = 0;
    TotalTicks = 0;
    FillPosition = 0;
    Starving = false;
    ReadAndFillSound();
    StartTick = Timer::GetProfileTicks();
    NextFillTick = GetTotalBytesReadInTicks();

    SetVolume(Volume);
    pSystem->AttachSound(this);
}

void VideoSoundFMOD::Stop()
{
    pSystem->DetachSound(this);

    Lock::Locker lock(&ChannelLock);
    if (pChannel)
        pChannel->stop();

    SoundStatus = Sound_Stopped;
    pPCMStream  = 0;
    pChannel = NULL;

    if (pBlockBuffer)
        SF_FREE(pBlockBuffer);
    pBlockBuffer = NULL;

    for(unsigned i = 0; i < Channels; ++i)
    {
        SF_HEAP_FREE(pHeap, TmpBuffers[i]);
        TmpBuffers[i] = NULL;
    }
}

void VideoSoundFMOD::Pause(bool sw)
{
    Lock::Locker lock(&ChannelLock);
    bool pulse_event = false;
    if (sw)
    {
        if (SoundStatus == Sound_Playing)
        {
            SoundStatus = Sound_Paused;
	        StopTick = Timer::GetProfileTicks();
    	    TotalTicks += StopTick - StartTick;
        }
    }
    else
    {
        if (SoundStatus == Sound_Paused)
        {
            SoundStatus = Sound_Playing;
        	StartTick = Timer::GetProfileTicks();
        	pulse_event = true;
        }
    }    
    if (pChannel)
    {
        pChannel->setPaused(sw);
		if (pulse_event)    	
			pSystem->PulseEvent();
    }
}

void VideoSoundFMOD::GetTime(UInt64* count, UInt64* unit) const
{
    VideoSoundFMOD* pthis = const_cast<VideoSoundFMOD*>(this);
    Lock::Locker lock(&pthis->ChannelLock);

    *unit = 1000000;
    if (SoundStatus == Sound_Playing && !Starving)
    {
        UInt64 totalTicksRead = GetTotalBytesReadInTicks();
        UInt64 curtick = Timer::GetProfileTicks();
        UInt64 pos = TotalTicks + (curtick - StartTick);
        if (pos > totalTicksRead)
        {
            pthis->Starving = true;
            pthis->StopTick = curtick;
            pthis->TotalTicks = totalTicksRead;

            *count = TotalTicks;
            return;
        }
        *count = pos;
        return;
    }
    *count = TotalTicks;
    return;
}

void VideoSoundFMOD::SetVolume(float volume)
{
    Lock::Locker lock(&ChannelLock);
    Volume = volume;
    if (pChannel)
        pChannel->setVolume(volume);
}

unsigned VideoSoundFMOD::GetSoundData(UByte *pdata, unsigned datasize)
{
    UInt32 recv_nsmpl = 0;
    SInt16 *buffer = (signed short *)pdata;
    UInt32 nsmpl   = datasize * 8 / 16 / Channels;
    
    if (Channels == 1)
    {   // Mono
        recv_nsmpl = pPCMStream->GetDataSInt16(1, &buffer, nsmpl);
    }
    else
    {   // Stereo or 5.1ch
        recv_nsmpl = pPCMStream->GetDataSInt16(Channels, TmpBuffers, nsmpl);
        if (Channels == 6)
        {
            // CRI:  L,R,Ls,Rs,C,LFE
            // FMOD: L,R,C,LFE,Ls,Rs
            static UInt32 ch_map[6]={ 0, 1, 4, 5, 2, 3 };
            for (unsigned count=0; count < recv_nsmpl; count++)
            {
                for(unsigned i = 0; i < Channels; ++i)
                    buffer[count*Channels + i] = TmpBuffers[ch_map[i]][count]; 
            }
        }
        else
        {
            for (unsigned count = 0; count < recv_nsmpl; count++)
            {
                for(unsigned i = 0; i < Channels; ++i)
                    *buffer++ = TmpBuffers[i][count]; 
            }
        }
    }

    unsigned recv_bytes = recv_nsmpl * 16 * Channels / 8;
    return recv_bytes;
}

inline unsigned VideoSoundFMOD::DistToFillBuffPos(unsigned pos) const
{
    unsigned bytes_diff;
    if (pos > FillPosition)
        bytes_diff = (FillPosition + SoundLength) - pos;
    else
        bytes_diff = FillPosition - pos;
    return bytes_diff;
}

#ifdef SF_OS_WII
#define MAX_BUFFER_SIZE 16384

void VideoSoundFMOD::ClearSoundBuffer()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned int len1, len2;
    unsigned size = SoundLength;
    unsigned fill_pos = 0;
    while (size > 0)
    {
        unsigned block_size = size > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : size;
        ret = pSound->lock(fill_pos, block_size, &ptr1, &ptr2, &len1, &len2);
        if (ret != FMOD_OK)
            break;
        Alg::MemUtil::Set(ptr1, 0, len1);
        ret = pSound->unlock(ptr1, ptr2, len1, len2);
        if (ret != FMOD_OK)
            break;
        size -= block_size;
        fill_pos += block_size;
    }
}

unsigned VideoSoundFMOD::ReadAndFillSound()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned int len1, len2;
    unsigned recv_bytes = GetSoundData(pBlockBuffer, BlockSize);

	if (recv_bytes < BlockSize)
		Alg::MemUtil::Set(&pBlockBuffer[recv_bytes], 0, BlockSize - recv_bytes);

    unsigned size = BlockSize;
    UByte* pdata = pBlockBuffer;
    unsigned fill_pos = FillPosition;
    while (size > 0)
    {
        unsigned block_size = size > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : size;
        if (block_size > SoundLength - fill_pos)
            block_size = SoundLength - fill_pos;

        ret = pSound->lock(fill_pos, block_size, &ptr1, &ptr2, &len1, &len2);
        if (ret != FMOD_OK)
            break;
        SF_ASSERT(!ptr2);
        SF_ASSERT(block_size == len1);
        Alg::MemUtil::Copy(ptr1, pdata, len1);
        ret = pSound->unlock(ptr1, ptr2, len1, len2);
        if (ret != FMOD_OK)
            break;
        size -= block_size;
        pdata += block_size;
        fill_pos += block_size;
        if (fill_pos == SoundLength)
            fill_pos = 0;
    }

    TotatBytesRead += recv_bytes;
    FillPosition += recv_bytes;
    if (FillPosition >= SoundLength)
        FillPosition -= SoundLength;
    return recv_bytes;
}

#else
void VideoSoundFMOD::ClearSoundBuffer()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned int len1, len2;
    ret = pSound->lock(0, SoundLength, &ptr1, &ptr2, &len1, &len2);
    if (ret == FMOD_OK)
    {
        Alg::MemUtil::Set(ptr1, 0, len1);
        ret = pSound->unlock(ptr1, ptr2, len1, len2);
        SF_ASSERT(ret == FMOD_OK);
        SF_UNUSED(ret);
    }
}

unsigned VideoSoundFMOD::ReadAndFillSound()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned int len1, len2;
    unsigned recv_bytes = GetSoundData(pBlockBuffer, BlockSize);

	if (recv_bytes < BlockSize)
        Alg::MemUtil::Set(&pBlockBuffer[recv_bytes], 0, BlockSize - recv_bytes );
	
    ret = pSound->lock(FillPosition, BlockSize, &ptr1, &ptr2, &len1, &len2);
    if (ret == FMOD_OK)
    {
        if (BlockSize > len1)
        {
            Alg::MemUtil::Copy(ptr1, pBlockBuffer, len1);
            if (BlockSize-len1 > len2)
            {
                Alg::MemUtil::Copy(ptr2, pBlockBuffer+len1, len2);
            }
            else
            {
                Alg::MemUtil::Copy(ptr2, pBlockBuffer+len1, BlockSize-len1);
                Alg::MemUtil::Set(((UByte*)ptr2)+(BlockSize-len1), 0, len2 - (BlockSize-len1));
            }
        }
        else
        {
            Alg::MemUtil::Copy(ptr1, pBlockBuffer, BlockSize);
            Alg::MemUtil::Set(((UByte*)ptr1)+BlockSize, 0, len1-BlockSize);
            Alg::MemUtil::Set(ptr2, 0, len2);
        }
        ret = pSound->unlock(ptr1, ptr2, len1, len2);
        SF_ASSERT(ret == FMOD_OK);
        SF_UNUSED(ret);
    }

    TotatBytesRead += recv_bytes;
    FillPosition += recv_bytes;
    if (FillPosition >= SoundLength)
        FillPosition -= SoundLength;

    return recv_bytes;
}
#endif

float VideoSoundFMOD::Update()
{
    SF_ASSERT(pChannel);

    Lock::Locker lock(&ChannelLock);
    if (SoundStatus != Sound_Playing)
          return 0.015f;

    UInt64 totalTicksRead = GetTotalBytesReadInTicks();
    UInt64 curtick = Timer::GetProfileTicks();
    UInt64 pos = TotalTicks + (curtick - StartTick);
    if (!Starving && pos > totalTicksRead)
    {
        Starving = true;
        StopTick = curtick;
        TotalTicks = totalTicksRead;
    }
    unsigned dist = 0;
    if (!Starving)
    {
        unsigned fmodpos = 0;
        FMOD_RESULT ret = pChannel->getPosition(&fmodpos, FMOD_TIMEUNIT_PCMBYTES);
        SF_ASSERT(ret == FMOD_OK);
        SF_UNUSED(ret);
#if defined(SF_OS_WII) && ((FMOD_VERSION > 0x00042400 && FMOD_VERSION < 0x00042417) || \
                           (FMOD_VERSION > 0x00042600 && FMOD_VERSION < 0x00042604))
        // on Wii FMOD before version 4.24.17 (for 4.26 branch before 4.26.04) had a bug which caused 
        // Channel::getPosition method to return incorrect PCMBYTES position for stereo sounds created 
        // with FMOD_HARDWARE flag. We need to correct it here
        fmodpos *= Channels;
#endif
        SF_ASSERT(fmodpos <= SoundLength);
        dist = DistToFillBuffPos(fmodpos);
    }
    if (dist < BlockSize / 3 )
    {
        unsigned recv_bytes = 0;
        if (Starving)
        {
            FillPosition = 0;
            recv_bytes = ReadAndFillSound();
            if (recv_bytes > 0)
            {
                pChannel->setPosition(0, FMOD_TIMEUNIT_PCMBYTES);
                Starving = false;
                StartTick = Timer::GetProfileTicks();
                pos = TotalTicks;
            }
            else
            {
                ClearSoundBuffer();
                return 0.015f;
            }
        }
        else
            recv_bytes = ReadAndFillSound();

        if (recv_bytes > 0)
        {
            NextFillTick = GetTotalBytesReadInTicks();
            float t =  (NextFillTick - pos) / 1000000.f *2/3;
            return t < 0.015f ? 0.015f : t;
        }
        return 0.015f;
    }
    else
    {
        float t =  (NextFillTick - pos) / 1000000.f / 2;
        return t < 0.015f ? 0.015f : t;
    }

}
//////////////////////////////////////////////////////////////////////////
//

// Global static FMOD heap
MemoryHeap *VideoSoundSystemFMODImpl::pFMODHeap = 0;

#define ERR_NOSOUND     "FMOD error! (%d) %s. No sound will be playing.\n"
#define ERR_OLDVERSION  "Error! You are using an old version of FMOD %08x. This program requires %08x\n"

#if defined(SF_OS_WIN32) || defined(SF_OS_MAC) || defined(SF_OS_LINUX) || defined(SF_OS_PSP)

static void * F_CALLBACK FMOD_AllocCallback(unsigned int size, FMOD_MEMORY_TYPE)
{
    return VideoSoundSystemFMODImpl::pFMODHeap->Alloc(size);
}
static void * F_CALLBACK FMOD_ReallocCallback(void*  ptr, unsigned int  size, FMOD_MEMORY_TYPE)
{
    return VideoSoundSystemFMODImpl::pFMODHeap->Realloc(ptr, size);
}
static void F_CALLBACK FMOD_FreeCallback(void *ptr, FMOD_MEMORY_TYPE)
{
    Memory::pGlobalHeap->Free(ptr);
}

VideoSoundSystemFMODImpl::VideoSoundSystemFMODImpl(FMOD::System* pfmod) : pFMOD(NULL)
{
    InitializedByUs = false;
    if (pfmod)
    {
        pFMOD = pfmod;
        return;
    }

    FMOD_RESULT      result;
    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS        caps;
    unsigned int     version;

    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(0,0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
        return;
    }

    result = pFMOD->getVersion(&version);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD->release();
        pFMOD = NULL;
        return;
    }
    if (version < FMOD_VERSION)
    {
        fprintf(stderr, ERR_OLDVERSION, version, FMOD_VERSION);
        pFMOD->release();
        pFMOD = NULL;
        return;
    }

    result = pFMOD->getDriverCaps(0, &caps, 0, 0, &speakermode);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD->release();
        pFMOD = NULL;
        return;
    }

    result = pFMOD->setSpeakerMode(speakermode);        // Set the user selected speaker mode.
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD->release();
        pFMOD = NULL;
        return;
    }

    if (caps & FMOD_CAPS_HARDWARE_EMULATED)             // The user has the 'Acceleration' slider set to off! This is really
    {                                                   // bad for latency!. You might want to warn the user about this.
        result = pFMOD->setDSPBufferSize(1024, 10);     // At 48khz, the latency between issuing an fmod command and hearing
        if (result != FMOD_OK)                          // it will now be about 213ms.
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
            return;
        }
    }

    result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);     // Replace with whatever channel count and flags you use!
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         // Ok, the speaker mode selected isn't supported by this soundcard.
    {                                                   // Switch it back to stereo...
        result = pFMOD->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
            return;
        }

        result = pFMOD->init(100, FMOD_INIT_NORMAL, 0); // Replace with whatever channel count and flags you use!
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
            return;
        }
    }
    else if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD->release();
        pFMOD = NULL;
        return;
    }

    InitializedByUs = true;
}

#elif defined(SF_OS_XBOX360) || defined(SF_OS_WII)

VideoSoundSystemFMODImpl::VideoSoundSystemFMODImpl(FMOD::System* pfmod) : pFMOD(NULL)
{
    InitializedByUs = false;
    if (pfmod)
    {
        pFMOD = pfmod;
        return;
    }

    FMOD_RESULT result;

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
    }
    if (pFMOD)
    {
#if defined(SF_OS_WII)
        result = pFMOD->init(64,  FMOD_INIT_NORMAL, 0);
#else
        result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);
#endif
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
        }
    }
    if (pFMOD)
        InitializedByUs = true;
}

#elif defined(SF_OS_PS3)
#include <cell/audio.h>
#include <sysutil/sysutil_sysparam.h>
#include <fmodps3.h>

VideoSoundSystemFMODImpl::VideoSoundSystemFMODImpl(FMOD::System* pfmod, const CellSpurs* pSpurs) : pFMOD(NULL)
{
    InitializedByUs = false;
    if (pfmod)
    {
        pFMOD = pfmod;
        return;
    }

    FMOD_RESULT result;

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
    }
    if (pFMOD)
    {
        uint8_t sprus_priorities[] = {1, 1, 0, 0, 0, 0, 0, 0};
        FMOD_PS3_EXTRADRIVERDATA extradriverdata;
        memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));
        extradriverdata.spurs = const_cast<CellSpurs*>(pSpurs);
        extradriverdata.spurs_taskset_priorities = sprus_priorities;

        result = pFMOD->init(64, FMOD_INIT_NORMAL, (void *)&extradriverdata);
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
        }
    }
    if (pFMOD)
        InitializedByUs = true;
}

#else

VideoSoundSystemFMODImpl::VideoSoundSystemFMODImpl(FMOD::System*) : pFMOD(NULL)
{
    InitializedByUs = false;
}

#endif

VideoSoundSystemFMODImpl::~VideoSoundSystemFMODImpl()
{
    if (InitializedByUs && pFMOD) 
    {
        if (pUpdateThread)
        {
            StopThread = true;
            Event.PulseEvent();
            pUpdateThread->Wait();
            pUpdateThread = NULL;
        }
        pFMOD->release();
        pFMOD = NULL;
        if (VideoSoundSystemFMODImpl::pFMODHeap)
            pFMODHeap->Release();
        VideoSoundSystemFMODImpl::pFMODHeap = NULL;
    }
}

void VideoSoundSystemFMODImpl::AttachSound(VideoSoundFMOD* psound)
{
    SF_ASSERT(psound);
    SoundsLock.DoLock();
    Sounds.PushBack(psound);
    SoundsLock.Unlock();
    if (!pUpdateThread)
    {
        StopThread = false;
        Thread::CreateParams params(UpdateFunc, this, THREAD_STACK_SIZE);
#ifdef SF_OS_WII
        params.priority = Thread::AboveNormalPriority;
#else
        params.priority = Thread::HighestPriority;
#endif
#ifdef SF_OS_XBOX360
        params.processor = 4;
#endif
        pUpdateThread = *SF_NEW Thread(params);
        pUpdateThread->Start();
    }
    Event.PulseEvent();
}

void VideoSoundSystemFMODImpl::DetachSound(VideoSoundFMOD* psound)
{
    SF_ASSERT(psound);
    Lock::Locker lck(&SoundsLock);
    for(size_t i = 0; i < Sounds.GetSize(); ++i)
    {
        if (Sounds[i] == psound)
        {
            Sounds.RemoveAt(i);
            break;
        }
    }
    if (Sounds.GetSize() == 0 && pUpdateThread)
    {
        StopThread = true;
        pUpdateThread = NULL;
        Event.PulseEvent();
    }
}

void VideoSoundSystemFMODImpl::LogError(FMOD_RESULT result)
{
    if (result != FMOD_OK && result != FMOD_ERR_INVALID_HANDLE && result != FMOD_ERR_CHANNEL_STOLEN)
        fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
}

int VideoSoundSystemFMODImpl::UpdateFunc(Thread*, void* h)
{
    VideoSoundSystemFMODImpl* psystem = (VideoSoundSystemFMODImpl*)h;
    unsigned wait_time = 2000; // SF_WAIT_INFINITE;
    while (1)
    {
        psystem->Event.Wait(wait_time);
        if (psystem->StopThread)
            break;
        Lock::Locker lock(&psystem->SoundsLock);
        wait_time = (unsigned)(psystem->UpdateAuxStreams() * 1000);
    }
    return 0;
}

float VideoSoundSystemFMODImpl::UpdateAuxStreams()
{
    float nextcall = 0.5f;
    for (size_t i = 0; i < Sounds.GetSize(); ++i)
    {
        float t = Sounds[i]->Update();
        if (t < nextcall) nextcall = t;
    }
    return nextcall;
}

//////////////////////////////////////////////////////////////////////////
//

#ifdef SF_OS_PS3
VideoSoundSystemFMOD::VideoSoundSystemFMOD(FMOD::System* pfmod, const CellSpurs* pSpurs, 
                                           MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    pImpl = SF_HEAP_NEW(pheap) VideoSoundSystemFMODImpl(pfmod, pSpurs);
}
#else
VideoSoundSystemFMOD::VideoSoundSystemFMOD(FMOD::System* pfmod, MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    pImpl = SF_HEAP_NEW(pheap) VideoSoundSystemFMODImpl(pfmod);
}
#endif

VideoSoundSystemFMOD::~VideoSoundSystemFMOD()
{
    delete pImpl;
}

VideoSound* VideoSoundSystemFMOD::Create()
{
    if (pImpl && pImpl->GetFMOD())
        return SF_HEAP_NEW(GetHeap()) VideoSoundFMOD(GetHeap(), pImpl);
    return NULL;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

