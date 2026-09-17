/**************************************************************************

Filename    :   Sound_SoundRendererFMOD.cpp
Content     :   SoundRenderer FMOD Ex implementation
Created     :   November, 2008
Authors     :   Andrew Reisse, Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Sound_SoundRendererFMOD.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Atomic.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_Allocator.h"
#include "Kernel/SF_Timer.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_HeapNew.h"

#include "Kernel/SF_Debug.h"

#include <fmod.h>
#include <fmod.hpp>

#include <fmod_common.h>
#include <fmod_errors.h>

#include <fmod_studio.hpp>
#include <fmod_studio_common.h>

#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
#include <fmod_event.hpp>
#include <fmod_event_net.hpp>
#endif

#include <iostream>

#define THREAD_NAME "Scaleform FMOD Update"
#define THREAD_STACK_SIZE 32768

// 错误检查宏（FMOD必备）
#define CHECK_FMOD_RESULT(result) \
    if (result != FMOD_OK) { \
        std::cerr << "FMOD Error: " << FMOD_ErrorString(result) << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
        return false; \
    }

#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
#include <fmodwiiu.h>
#endif

float CachedPan = 0.0f;

namespace Scaleform { namespace Sound {

class SoundRendererFMODImpl;
class SoundChannelFMODImpl;

//////////////////////////////////////////////////////////////////////////
//

// A simple doubly-linked list node, used to track textures and other resources
//
class DListNode
{
public:
	union   {
		DListNode *pPrev;       
		DListNode *pLast;
	};
	union   {
		DListNode *pNext;
		DListNode *pFirst;      
	};

	// Default constructor, creates root node (empty list)
	DListNode() { pNext = pPrev = this; }
	// Inserting constructor, inserts this node at the head of the list
	DListNode(DListNode *proot)
	{
		pNext = proot->pFirst;
		pPrev = proot;
		proot->pFirst->pPrev = this;
		proot->pFirst = this;
	}

	virtual ~DListNode() {}

	void RemoveNode()
	{
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
		pNext = pPrev = 0;
	}

	virtual void ReleaseResource() {}
};

class SoundSampleImplNode : public SoundSample, public DListNode
{
public:
	SoundSampleImplNode(DListNode* plistRoot) : DListNode(plistRoot) {}
	~SoundSampleImplNode() { if (pFirst) RemoveNode(); }
};

class SoundChannelImplNode : public SoundChannel, public DListNode
{
public:
	SoundChannelImplNode(DListNode* plistRoot) : DListNode(plistRoot) {}
	~SoundChannelImplNode() { if (pFirst) RemoveNode(); }
};


//////////////////////////////////////////////////////////////////////////
//

class SoundSampleFMODImpl : public SoundSampleImplNode
{
public:
    SoundRendererFMODImpl*   pPlayer;
    FMOD::Sound*             pSound;
    Ptr<SoundDataBase>       pSoundData;

    SoundSampleFMODImpl(SoundRendererFMODImpl* pp);
    ~SoundSampleFMODImpl();

    virtual SoundRenderer*   GetSoundRenderer() const;
    virtual bool             IsDataValid() const { return pSound != 0; }
    virtual float            GetDuration() const;

    virtual int              GetBytesTotal() const;
    virtual int              GetBytesLoaded() const;

    virtual void     ReleaseResource();
    void             ReleaseFMODObjects();

    bool             IsPlaying(void*);

    virtual SoundChannelFMODImpl* Start(bool paused);

    FMOD_RESULT CreateSubSound(SoundData* psd, FMOD::Sound** psound);
    FMOD_RESULT CreateSubSound(SoundFile* psd, FMOD::Sound** psound);
    FMOD_RESULT CreateSubSound(AppendableSoundData* psd, FMOD::Sound** psound);
};


class SoundChannelFMODImpl : public SoundChannelImplNode
{
public:
    SoundRendererFMODImpl*   pPlayer;
    SoundSampleFMODImpl*     pSample;
    FMOD::Channel*           pChan;

    Hash<FMOD_SYNCPOINT*, Transform> Tranforms;

    SoundChannelFMODImpl(SoundRendererFMODImpl* pp, SoundSampleFMODImpl* ps, FMOD::Channel* pc);
    ~SoundChannelFMODImpl();

    SoundRenderer*   GetSoundRenderer() const;
    SoundSample*     GetSample() const { return pSample; }

    virtual void     ReleaseResource();
    void             ReleaseFMODObjects();

    virtual void     Stop();
    virtual void     Pause(bool pause);
    virtual bool     IsPlaying() const;
    virtual void     SetPosition(float seconds);
    virtual float    GetPosition();
    virtual void     Loop(int count, float start = 0, float end = 0);
    virtual float    GetVolume();
    virtual void     SetVolume(float volume);
    virtual float    GetPan();
    virtual void     SetPan(float volume);
    virtual void     SetTransforms(const Array<Transform>& transforms);
    virtual void     SetSpatialInfo(const Array<Vector> []) {}

    static FMOD_RESULT F_CALL CallBackFunc(
        FMOD_CHANNELCONTROL* chancontrol,
        FMOD_CHANNELCONTROL_TYPE controltype,
        FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
        void* cmddata1,
        void* cmddata2);
};


//////////////////////////////////////////////////////////////////////////
//

class SoundChannelFMODImplAux;
class SoundSampleFMODImplAux;
typedef Hash<SoundRenderer::AuxStreamer*, Array<SoundChannelFMODImplAux*> > AuxStreamersType;

class SoundRendererFMODImpl : public SoundRendererFMOD
{
public:
    FMOD::System*                   pDevice;
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    FMOD::EventSystem*              pEventSys;
#endif
#ifdef SF_ENABLE_THREADS
    Ptr<Thread>         pUpdateThread;
    Event               EventObj;
    volatile bool       StopThread;
#endif
    Lock                AuxStreamsLock;
    AuxStreamersType	AuxStreamers;

    DListNode           SampleList;
    Lock                SampleListLock;
    bool                CallFMODUpdate;
    bool                ThreadedUpdate;
#ifdef SF_OS_XBOX360
    int                 ProcNumber;
#endif
    float               SystemBitRate;

    SoundRendererFMODImpl();
    virtual ~SoundRendererFMODImpl() { xFinalize(); }

    virtual bool                    GetRenderCaps(UInt32* pcaps) override
    {
        if (!pcaps)
            return false;

        *pcaps = 0;

        *pcaps |= SoundRenderer::Cap_NoMP3;
        *pcaps |= SoundRenderer::Cap_NoStreaming;
        *pcaps |= SoundRenderer::Cap_NoVideoSound;

        return true;
    };

    virtual FMOD::System*           GetFMODSystem()      { return pDevice; }
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    virtual FMOD::EventSystem*      GetFMODEventSystem() { return pEventSys; }
#endif

    virtual bool        Initialize(FMOD::System* pd, bool call_fmod_update, bool threaded_update
#ifdef SF_OS_XBOX360
                                 , int processor_core = 5
#endif
                        );
    virtual void        Finalize()  { xFinalize(); }
    void                xFinalize(); // Avoid call to a virtual method from the destructor

    virtual SoundSampleFMODImpl*    CreateSampleFromData(SoundDataBase* psd);
    virtual SoundSampleFMODImpl*    CreateSampleFromFile(const char* fname, bool streaming);
    virtual SoundSampleFMODImpl*    CreateSampleFromAuxStreamer(AuxStreamer* pStreamder,
                                                                UInt32 channels,
                                                                UInt32 samplerate,
                                                                AuxStreamer::PCMFormat fmt);

    virtual SoundChannelFMODImpl*   PlaySample(SoundSample* ps, bool paused);

    virtual float       GetMasterVolume();
    virtual void        SetMasterVolume(float volume);

    virtual float       Update();
    virtual void        Mute(bool mute);

    void                AttachAuxStreamer(SoundChannelFMODImplAux*);
    void                DetachAuxStreamer(SoundChannelFMODImplAux*);
    float               UpdateAuxStreams();

    void                LogError(FMOD_RESULT result);
    void                PrintStatistics();

#ifdef SF_ENABLE_THREADS
    static int          UpdateFunc(Thread*, void* h);
    void                PulseEvent() { EventObj.PulseEvent(); }
#endif
};


//////////////////////////////////////////////////////////////////////////
//

// FMOD_CALLBACK IN MODERN IS F_CALL
FMOD_RESULT F_CALL DecodeOpen(const char* name, unsigned int* filesize, void** handle, void* userdata)
{
    (void)name;
    AppendableSoundData* psd = (AppendableSoundData*)userdata;

    psd->SeekPos(0);

    *handle = psd;              // ✅ handle 存你自己的对象
    *filesize = 0x0FFFFFFF;     // 或者真实长度

    return FMOD_OK;
}

FMOD_RESULT F_CALL DecodeClose(void* handle, void* userdata)
{
	(void)userdata;
    AppendableSoundData* psd = (AppendableSoundData*)handle;
    psd->SeekPos(0);
    return FMOD_OK;
}

FMOD_RESULT F_CALL DecodeRead(void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata)
{
	(void)userdata;
    AppendableSoundData* psd = (AppendableSoundData*)handle;

    *bytesread = psd->GetData((UByte*)buffer, sizebytes);

    return FMOD_OK;
}

FMOD_RESULT F_CALL DecodeSeek(void* handle, unsigned int pos, void* userdata)
{
	(void)userdata;
    AppendableSoundData* psd = (AppendableSoundData*)handle;

    if (!psd->SeekPos(pos))
        return FMOD_ERR_FILE_COULDNOTSEEK;

    return FMOD_OK;
}

FMOD_RESULT F_CALL CreateSubSound(SoundRendererFMODImpl* prenderer, AppendableSoundData* psd, FMOD::Sound** psound)
{
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = 0x0FFFFFFF;

    FMOD_MODE flags = FMOD_DEFAULT | FMOD_2D;
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	flags |= FMOD_DEFAULT;
#else
	flags |= FMOD_DEFAULT;
#endif
    switch (psd->GetFormat() & SoundData::Sample_Format)
    {
    case SoundData::Sample_PCM:
        exinfo.format = (psd->GetFormat() & 0x7) == 2 ? FMOD_SOUND_FORMAT_PCM16 : FMOD_SOUND_FORMAT_PCM8;
        exinfo.defaultfrequency = psd->GetRate();
        exinfo.numchannels = (psd->GetFormat() & SoundData::Sample_Stereo) ? 2 : 1;
        flags |= FMOD_OPENRAW;
        break;

    // In Modern FMOD, FMOD_SOUND_FORMAT_MPEG Is Wasted.
    // So There is no need to set exinfo.format for MP3, FMOD will detect it by itself.
    case SoundData::Sample_MP3:
        flags |= FMOD_CREATESTREAM;
        flags |= FMOD_IGNORETAGS;
        break;
    default:
        std::cerr << "emmmmmmmm..........." << std::endl;
        return FMOD_ERR_FORMAT;
    }

    exinfo.fileuseropen = &DecodeOpen;
    exinfo.fileuserclose = &DecodeClose;
    exinfo.fileuserread = &DecodeRead;
    exinfo.fileuserseek = &DecodeSeek;

    exinfo.userdata = psd;           // 必须
    exinfo.decodebuffersize = 1024 * 8;

    FMOD_RESULT result = prenderer->pDevice->createStream((const char*)psd,flags, &exinfo, psound);
    return result;
}


//////////////////////////////////////////////////////////////////////////
//

class SwfSoundStreamer : public SoundRenderer::AuxStreamer, public DListNode
{
public:
    SwfSoundStreamer(SoundRendererFMODImpl* prenderer, AppendableSoundData* psd);
    ~SwfSoundStreamer();

    virtual unsigned    GetPCMData(UByte* pdata, unsigned datasize);
    virtual bool        SetPosition(float seconds);

    virtual void        ReleaseResource();
    void                ReleaseFMODObjects();

    bool CreateReader();
    bool GetSoundFormat(SoundRenderer::AuxStreamer::PCMFormat* fmt, UInt32 *channels, UInt32 *samplerate);

    Ptr<AppendableSoundData>    pSoundData;
    FMOD::Sound*                pSound;
    SoundRendererFMODImpl*      pRenderer;
};

SwfSoundStreamer::SwfSoundStreamer(SoundRendererFMODImpl* prenderer, AppendableSoundData* psd) :
    DListNode(&prenderer->SampleList), pSoundData(psd), pSound(NULL), pRenderer(prenderer)
{
}

void SwfSoundStreamer::ReleaseResource()
{
    pRenderer = NULL;
    if (GetRefCount() > 0)
        ReleaseFMODObjects();
    if (pNext)
        RemoveNode();
}

void SwfSoundStreamer::ReleaseFMODObjects()
{
    if (pSound)
        pSound->release();
    pSound = NULL;
}

SwfSoundStreamer::~SwfSoundStreamer()
{
    ReleaseFMODObjects();
    if(pFirst)
        RemoveNode();
}

unsigned SwfSoundStreamer::GetPCMData(UByte* pdata, unsigned datasize)
{
    unsigned got_bytes = 0;
    if (pSound)
        pSound->readData(pdata,datasize,&got_bytes);
    return got_bytes;
}

bool SwfSoundStreamer::SetPosition(float seconds)
{
    if (pSound && pSoundData)
    {
        unsigned latency = pSoundData ? pSoundData->GetSeekSample() : 0;
        float sample_rate;
        pSound->getDefaults(&sample_rate,NULL);
        unsigned sample = unsigned(seconds * sample_rate);
        FMOD_RESULT res = pSound->seekData(sample + latency);
        return res == FMOD_OK;
    }
    return false;
}

bool SwfSoundStreamer::CreateReader()
{
    if (pRenderer)
        return CreateSubSound(pRenderer, pSoundData, &pSound) == FMOD_OK;
    return false;
}

bool SwfSoundStreamer::GetSoundFormat(SoundRenderer::AuxStreamer::PCMFormat* fmt, UInt32 *channels, UInt32 *samplerate)
{
    if (!pSound)
        return false;
    FMOD_SOUND_FORMAT format = FMOD_SOUND_FORMAT_NONE;
    int chans = 0;
    FMOD_RESULT ret;
    ret = pSound->getFormat(NULL, &format, &chans, NULL);
    if (ret != FMOD_OK)
        return false;
    *channels = chans;
    float frequency = 0.0;
    ret = pSound->getDefaults(&frequency, NULL);
    if (ret != FMOD_OK)
        return false;
    *samplerate = (UInt32)frequency;
    if (format == FMOD_SOUND_FORMAT_PCM16)
        *fmt = PCM_SInt16;
    else if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
        *fmt = PCM_Float;
    else
        return false;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#define AUX_SOUND_READBUFLEN_MS 300
#define AUX_SOUND_LEN_MS (AUX_SOUND_READBUFLEN_MS * 5)

class SoundChannelFMODImplAux : public SoundChannelFMODImpl
{
public:
    SoundChannelFMODImplAux(SoundRendererFMODImpl* pp, SoundSampleFMODImplAux* ps, FMOD::Channel* pc);
    ~SoundChannelFMODImplAux();

    virtual float   GetPosition();
    virtual void    SetPosition(float seconds);
    virtual void    Stop();
    virtual void    Pause(bool pause);

    float           Update();

    UInt64          StartTick;
    UInt64          StopTick;
    UInt64          TotalTicks;
    UInt64          NextFillTick;
    bool            Paused;
    bool            Starving;
    Lock            ChannelLock;
    float           StartSecond;
};

class SoundSampleFMODImplAux : public SoundSampleFMODImpl
{
public:
    typedef SoundRenderer::AuxStreamer AuxStreamer;

    SoundSampleFMODImplAux(SoundRendererFMODImpl* pp, AuxStreamer* pstreamer, AuxStreamer::PCMFormat fmt,
                           UInt32 channels, UInt32 samplerate);
    ~SoundSampleFMODImplAux();

    virtual SoundChannelFMODImplAux* Start(bool paused);

    unsigned    ReadAndFillSound();
    void        ClearSoundBuffer();

    bool        SeekData(float seconds);

    unsigned    GetTotalBytesRead()        const { return TotatBytesRead; }
    UInt64      GetTotalBytesReadInTicks() const { return BytesToTicks(TotatBytesRead); }
    unsigned    GetFillPosition()          const { return FillPosition; }
    unsigned    TicksToSoundPos(UInt64 ticks) const
    {
        return unsigned((((ticks / 1000) % AUX_SOUND_LEN_MS) * (SampleRate / 1000)) * Channels * (Bits / 8));
    }
    UInt64      BytesToTicks(unsigned bytes) const
    { 
        return UInt64(bytes) * 8/ Bits / Channels * 1000000 / SampleRate;
    }
    inline unsigned DistToFillBuffPos(unsigned pos) const;

    Ptr<AuxStreamer>    pStreamer;
    UInt32              Channels;
    UInt32              SampleRate;
    unsigned            Bits;
    UByte*              pBlockBuffer;
    unsigned            BlockSize;
    unsigned            SoundLength;
    FMOD_SOUND_FORMAT   Format;
    unsigned            FillPosition;
    unsigned            TotatBytesRead;
};

SoundSampleFMODImplAux::SoundSampleFMODImplAux(SoundRendererFMODImpl* pp,
                                               AuxStreamer* pstreamer, AuxStreamer::PCMFormat fmt,
                                               UInt32 channels, UInt32 samplerate) :
    SoundSampleFMODImpl(pp), Channels(channels), SampleRate(samplerate), pBlockBuffer(0), BlockSize(0),
    FillPosition(0), TotatBytesRead(0)
{
    pStreamer = pstreamer;
    Format = fmt == AuxStreamer::PCM_SInt16 ? FMOD_SOUND_FORMAT_PCM16 : FMOD_SOUND_FORMAT_PCMFLOAT;
    Bits = fmt == SoundRenderer::AuxStreamer::PCM_SInt16 ? 16 : 32;
    SoundLength = (AUX_SOUND_LEN_MS * (SampleRate /1000)) * Channels * (Bits / 8);

    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO); // Required
    exinfo.length            = SoundLength;                    // Length of PCM data in bytes of whole song (for Sound::getLength)
    exinfo.numchannels       = Channels;                       // Number of channels in the sound
    exinfo.defaultfrequency  = SampleRate;                     // Default playback rate of sound
    exinfo.format            = Format;

    FMOD_MODE flags = FMOD_OPENUSER | FMOD_LOOP_NORMAL;
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	flags |= FMOD_DEFAULT;
#else
	flags |= FMOD_DEFAULT;
#endif
    FMOD_RESULT result = pPlayer->pDevice->createSound(0, flags, &exinfo, &pSound);
    if (result != FMOD_OK)
    {
        pSound = NULL;
        pPlayer->LogError(result);
        return;
    }
    unsigned int sl = 0;
    result = pSound->getLength(&sl, FMOD_TIMEUNIT_PCMBYTES);
    pPlayer->LogError(result);
}

SoundSampleFMODImplAux::~SoundSampleFMODImplAux()
{
    if (pBlockBuffer)
        SF_FREE(pBlockBuffer);
}

SoundChannelFMODImplAux* SoundSampleFMODImplAux::Start(bool paused)
{
    if (!pSound)
        return NULL;
    FMOD::Channel* pchannel;

    FMOD_RESULT result;
    result = pPlayer->pDevice->playSound(pSound, nullptr, paused, &pchannel);
    if (result != FMOD_OK)
    {
        pPlayer->LogError(result);
        return NULL;
    }
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
    FMOD_WiiU_SetControllerSpeaker((FMOD_CHANNEL *)pchannel, FMOD_WIIU_CONTROLLER_DRC);
#endif
    
    BlockSize = (AUX_SOUND_READBUFLEN_MS * SampleRate/1000) * Channels * (Bits / 8);
    if (pBlockBuffer)
        SF_FREE(pBlockBuffer);
    pBlockBuffer = (UByte*)SF_ALLOC(BlockSize, Stat_Sound_Mem);
    SoundChannelFMODImplAux* pChannel = SF_NEW SoundChannelFMODImplAux(pPlayer, this, pchannel);
    pPlayer->AttachAuxStreamer(pChannel);
    return pChannel;
}

void SoundSampleFMODImplAux::ClearSoundBuffer()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned int len1, len2;
    ret = pSound->lock(0, SoundLength, &ptr1, &ptr2, &len1, &len2);
    if (ret == FMOD_OK)
    {
        Alg::MemUtil::Set(ptr1, 0, len1);
        ret = pSound->unlock(ptr1, ptr2, len1, len2);
        if (ret != FMOD_OK)
        {
            pPlayer->LogError(ret);
            return;
        }
    }
}

unsigned SoundSampleFMODImplAux::ReadAndFillSound()
{
    FMOD_RESULT ret;
    void* ptr1, *ptr2;
    unsigned len1, len2;
    unsigned got_bytes = pStreamer->GetPCMData(pBlockBuffer, BlockSize);

    if (got_bytes < BlockSize)
        Alg::MemUtil::Set(&pBlockBuffer[got_bytes], 0, BlockSize-got_bytes );
    
    if (Channels == 6)
    {
        // CRI:  L,R,Ls,Rs,C,LFE
        // FMOD: L,R,C,LFE,Ls,Rs
        if (Format == FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            float tmp;
            float* psample = (float*)pBlockBuffer;
            for(UPInt i = 0; i < got_bytes/4; i += 6)
            {
                tmp = psample[i+2]; psample[i+2] = psample[i+4]; psample[i+4] = tmp;
                tmp = psample[i+3]; psample[i+3] = psample[i+5]; psample[i+5] = tmp;
            }
        }
        else
        {
            UInt16 tmp;
            UInt16* psample = (UInt16*)pBlockBuffer;
            for(UPInt i = 0; i < got_bytes/2; i += 6)
            {
                tmp = psample[i+2]; psample[i+2] = psample[i+4]; psample[i+4] = tmp;
                tmp = psample[i+3]; psample[i+3] = psample[i+5]; psample[i+5] = tmp;
            }
        }
    }

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
        if (ret != FMOD_OK)
        {
            pPlayer->LogError(ret);
            return 0;
        }
    }

    TotatBytesRead += got_bytes;
    FillPosition += got_bytes;
    if (FillPosition >= SoundLength)
        FillPosition -= SoundLength;
    return got_bytes;
}

inline unsigned SoundSampleFMODImplAux::DistToFillBuffPos(unsigned pos) const
{
    unsigned bytes_diff;
    if (pos > FillPosition)
        bytes_diff = (FillPosition + SoundLength) - pos;
    else
        bytes_diff = FillPosition - pos;
    return bytes_diff;
}

bool SoundSampleFMODImplAux::SeekData(float seconds)
{
    if (pStreamer && pStreamer->SetPosition(seconds))
    {
        TotatBytesRead = 0;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
//

SoundChannelFMODImplAux::SoundChannelFMODImplAux(SoundRendererFMODImpl* pp,
                                                 SoundSampleFMODImplAux* ps, FMOD::Channel* pc) :
    SoundChannelFMODImpl(pp, ps, pc), StartTick(Timer::GetProfileTicks()), Starving(false), StartSecond(0.0)
{
    ps->AddRef();
    TotalTicks = 0;
    NextFillTick = 0;
    pChan->getPaused(&Paused);
    if (Paused)
        StopTick = StartTick;
    else
    {
        ps->ReadAndFillSound();
        StartTick = Timer::GetProfileTicks();
        NextFillTick = ps->GetTotalBytesReadInTicks();
    }
}

SoundChannelFMODImplAux::~SoundChannelFMODImplAux()
{
    pSample->Release();
}

float SoundChannelFMODImplAux::GetPosition()
{
    Lock::Locker lock(&ChannelLock);
    if (!Paused && !Starving)
    {
        SoundSampleFMODImplAux* psample = (SoundSampleFMODImplAux*)pSample;
        UInt64 totalTicksRead = psample->GetTotalBytesReadInTicks();
        UInt64 curtick = Timer::GetProfileTicks();
        UInt64 pos = TotalTicks + (curtick - StartTick);
        if (pos > totalTicksRead)
        {
            Starving = true;
            StopTick = curtick;
            TotalTicks = totalTicksRead;
            return StartSecond + TotalTicks / 1000000.f;
        }
        return StartSecond + pos / 1000000.f;
    }
    return StartSecond + TotalTicks / 1000000.f;
}

void SoundChannelFMODImplAux::SetPosition(float seconds)
{
    Lock::Locker lock(&ChannelLock);
    SoundSampleFMODImplAux* psample = (SoundSampleFMODImplAux*)pSample;
    if (psample && psample->SeekData(seconds))
    {
        StartSecond = seconds;
        TotalTicks = 0;
        NextFillTick = 0;
        StartTick = Timer::GetProfileTicks();
        unsigned newPlayPos = psample->GetFillPosition();
        psample->ReadAndFillSound();
        pChan->setPosition(newPlayPos, FMOD_TIMEUNIT_PCMBYTES);
        Starving = false;
        NextFillTick = psample->GetTotalBytesReadInTicks();
    }
}

void SoundChannelFMODImplAux::Stop()
{
    if (pPlayer)
        pPlayer->DetachAuxStreamer(this);
    StopTick = Timer::GetProfileTicks();
}

void SoundChannelFMODImplAux::Pause(bool pause)
{
    Lock::Locker lock(&ChannelLock);
    if (Paused == pause)
        return;
    SoundChannelFMODImpl::Pause(pause);
    Paused = pause;
    if (Paused)
    {
        StopTick = Timer::GetProfileTicks();
        TotalTicks += StopTick - StartTick;
    }
    else
    {
        UInt64 curtick = Timer::GetProfileTicks();
        StartTick = curtick;
#ifdef SF_ENABLE_THREADS
        pPlayer->PulseEvent();
#endif
    }
}

float SoundChannelFMODImplAux::Update()
{
    Lock::Locker lock(&ChannelLock);
    
    if (!IsPlaying())
        return 0.5f;
    if (Paused)
        return 0.1f;

    SF_ASSERT(pSample);
    SoundSampleFMODImplAux* psample = (SoundSampleFMODImplAux*)pSample;
    UInt64 totalTicksRead = psample->GetTotalBytesReadInTicks();
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
        FMOD_RESULT ret = pChan->getPosition(&fmodpos, FMOD_TIMEUNIT_PCMBYTES);
        if (ret != FMOD_OK)
        {
            pPlayer->LogError(ret);
            return 0.0f;
        }
        dist = psample->DistToFillBuffPos(fmodpos);
    }

    if (dist < psample->BlockSize / 3 )
    {
        unsigned got_bytes = 0;
        if (Starving)
        {
            psample->FillPosition = 0;
            got_bytes = psample->ReadAndFillSound();
            if (got_bytes > 0)
            {
                pChan->setPosition(0, FMOD_TIMEUNIT_PCMBYTES);
                Starving = false;
                StartTick = Timer::GetProfileTicks();
                pos = TotalTicks;
            }
            else
            {
                psample->ClearSoundBuffer();
                return 0.02f;
            }
        }
        else
            got_bytes = psample->ReadAndFillSound();

        if (got_bytes > 0)
        {
            NextFillTick = psample->GetTotalBytesReadInTicks();
            float t =  (NextFillTick - pos) / 1000000.f *2/3;
            return t < 0.02f ? 0.02f : t;
        }
        return 0.02f;
    }
    else
    {
        float t =  (NextFillTick - pos) / 1000000.f / 2;
        return t < 0.02f ? 0.02f : t;
    }
}

//////////////////////////////////////////////////////////////////////////
//

void SoundRendererFMODImpl::AttachAuxStreamer(SoundChannelFMODImplAux* pchan)
{
    if (!pchan || !pchan->pSample)
        return;
    SoundSampleFMODImplAux* psample = (SoundSampleFMODImplAux*)pchan->pSample;
    if (!psample->pStreamer)
        return;
    AuxStreamsLock.DoLock();
    Array<SoundChannelFMODImplAux*>* pcont = AuxStreamers.Get(psample->pStreamer);
    if (!pcont)
    {
        AuxStreamers.Set(psample->pStreamer, Array<SoundChannelFMODImplAux*>());
        pcont = AuxStreamers.Get(psample->pStreamer);
    }
    pcont->PushBack(pchan);
    pchan->AddRef();
    AuxStreamsLock.Unlock();
#ifdef SF_ENABLE_THREADS
    if(ThreadedUpdate && !pUpdateThread)
    {
        StopThread = false;
        Thread::CreateParams params(UpdateFunc, this, THREAD_STACK_SIZE);
        params.priority = Thread::HighestPriority;
#ifdef SF_OS_XBOX360
        params.processor = ProcNumber;
#endif
        pUpdateThread = *SF_NEW Thread(params);
        if (pUpdateThread->Start())
        {
            pUpdateThread->SetThreadName(THREAD_NAME);
        }
        else {
            SF_DEBUG_ERROR1(1, "Failed to create %s thread!\n", THREAD_NAME);
            StopThread = true;
            pUpdateThread = NULL;
            // Fall back to non-threaded update
            ThreadedUpdate = false;
        }
    }
    EventObj.PulseEvent();
#endif
}

void SoundRendererFMODImpl::DetachAuxStreamer(SoundChannelFMODImplAux* pchan)
{
    if (!pchan || !pchan->pSample)
        return;
    SoundSampleFMODImplAux* psample = (SoundSampleFMODImplAux*)pchan->pSample;
    if (!psample->pStreamer)
        return;

    AuxStreamsLock.DoLock();
    Array<SoundChannelFMODImplAux*>* pcont = AuxStreamers.Get(psample->pStreamer);
    bool found = false;
    if (pcont)
    {
        UPInt nulls = 0;
        for(UPInt i = 0; i < pcont->GetSize(); ++i)
        {
            if ((*pcont)[i] == pchan)
            {
                (*pcont)[i] = NULL;
                found = true;
            }
            if ((*pcont)[i] == NULL)
                nulls++;
        }
        if (nulls == pcont->GetSize())
            AuxStreamers.Remove(psample->pStreamer);
#ifdef SF_ENABLE_THREADS
        if (AuxStreamers.GetSize() == 0 && ThreadedUpdate && pUpdateThread)
        {
            StopThread = true;
            pUpdateThread = NULL;
            EventObj.PulseEvent();
        }
#endif
    }
    if (!found)
        pchan = NULL;

    AuxStreamsLock.Unlock();
    if (pchan)
    {
        pchan->pChan->stop();
        pchan->Release();
    }
}

SoundSampleFMODImpl* SoundRendererFMODImpl::CreateSampleFromData(SoundDataBase* psd)
{
    if (!psd)
        return NULL;

    Lock::Locker lock(&SampleListLock);

    if (psd->IsStreamSample() && !psd->IsFileSample())
    {
        AppendableSoundData* pasd = (AppendableSoundData*)psd;
        Ptr<SwfSoundStreamer> pstreamer = *SF_NEW SwfSoundStreamer(this, pasd);
        if (!pstreamer->CreateReader())
            return NULL;
        AuxStreamer::PCMFormat fmt;
        UInt32 channels;
        UInt32 samplerate;
        if (!pstreamer->GetSoundFormat(&fmt, &channels,&samplerate))
            return NULL;
        return CreateSampleFromAuxStreamer(pstreamer, channels, samplerate, fmt);
    }

    Ptr<SoundSampleFMODImpl> psample = *SF_NEW SoundSampleFMODImpl(this);
    bool created = false;
    psample->pSoundData = psd;
    if (psd->IsFileSample())
    {
        SoundFile* ps = (SoundFile*) psd;
        created = (psample->CreateSubSound(ps, &psample->pSound) == FMOD_OK);
    }
    else
    {
        SoundData* ps = (SoundData*) psd;
        created = (psample->CreateSubSound(ps, &psample->pSound) == FMOD_OK);
    }
    if (created)
    {
        psample->AddRef();
        return psample;
    }
    return NULL;
}

SoundSampleFMODImpl* SoundRendererFMODImpl::CreateSampleFromAuxStreamer(
    AuxStreamer* pstreamder, UInt32 channels, UInt32 samplerate, AuxStreamer::PCMFormat fmt)
{
    Lock::Locker lock(&SampleListLock);
    SoundSampleFMODImplAux* psample = SF_NEW SoundSampleFMODImplAux(this, pstreamder, fmt, channels, samplerate);
    return psample;
}

SoundSampleFMODImpl* SoundRendererFMODImpl::CreateSampleFromFile(const char* fname, bool streaming)
{
    SoundSampleFMODImpl* psample = NULL;
    {
    Lock::Locker lock(&SampleListLock);
    psample = SF_NEW SoundSampleFMODImpl(this);
    }

    FMOD_MODE flags = FMOD_LOOP_OFF | FMOD_2D;
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	flags |= FMOD_DEFAULT;
#else
	flags |= FMOD_DEFAULT;
#endif
#if defined(SF_OS_WINMETRO)
    flags |= FMOD_NONBLOCKING;
#endif
    FMOD_RESULT result;
    if (streaming)
        result = pDevice->createStream(fname, flags, NULL, &(psample->pSound));
    else
        result = pDevice->createSound(fname, flags, NULL, &(psample->pSound));
    if (result != FMOD_OK)
    {
        LogError(result);
        psample->pSound = NULL;
        psample->Release();
        return NULL;
    }
    return psample;
}

static FMOD_RESULT CheckFMODResult(FMOD_RESULT result, const char* api_call)
{
    if (result != FMOD_OK)
    {
        // 官方模板风格：打印错误信息 + API 调用名
        printf("FMOD Error: %s (API: %s)\n", FMOD_ErrorString(result), api_call);
        // 也可根据项目需求改为日志/断言
        // assert(result == FMOD_OK);
    }
    return result;
}

bool SoundRendererFMODImpl::Initialize(FMOD::System* pd, bool call_fmod_update, bool threaded_update
#ifdef SF_OS_XBOX360
    , int processor_core
#endif
)
{
    SF_UNUSED2(call_fmod_update, threaded_update);

    if (pd)
    {
        pDevice = pd;   // 外部接管
    }

    else
    {
        FMOD_RESULT r = FMOD::System_Create(&pDevice);
        if (r != FMOD_OK)
            return false;

        r = pDevice->init(32, FMOD_INIT_NORMAL, nullptr);
        if (r != FMOD_OK)
        {
            pDevice->release();
            pDevice = nullptr;
            return false;
        }
    }

    // 重置状态
    CallFMODUpdate = true;
    ThreadedUpdate = false;

#ifdef SF_OS_XBOX360
    ProcNumber = processor_core;
#endif
    SystemBitRate = 0.0f;

    // ---------------- 官方模板核心：若外部未传入 System，则自行创建（贴合 play_sound 逻辑） ----------------
    if (!pDevice)
    {
        FMOD_RESULT result = CheckFMODResult(FMOD::System_Create(&pDevice), "System_Create");
        if (result != FMOD_OK)
        {
            return false;
        }

        // 官方模板：初始化 System（参数对齐 play_sound 的 32 声道、默认初始化标记）
        int max_channels = 32; // 官方 play_sound 常用 32 声道，可根据项目调整
        result = CheckFMODResult(
            pDevice->init(max_channels, FMOD_INIT_NORMAL, nullptr),
            "System::init"
        );
        if (result != FMOD_OK)
        {
            pDevice->release();
            pDevice = nullptr;
            return false;
        }
    }

    // ---------------- 原有逻辑：获取采样率（补充错误检查） ----------------
    if (!pDevice)
    {
        std::cerr << "FMOD System is null" << std::endl;
        return false;
    }

    int rate = 0;
    if (pDevice->getSoftwareFormat(&rate, nullptr, nullptr) == FMOD_OK)
        SystemBitRate = (float)rate;

    return true;

    // ---------------- 原有 FMOD Designer 逻辑（补充错误检查，对齐官方风格） ----------------
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    result = CheckFMODResult(FMOD::EventSystem_Create(&pEventSys), "EventSystem_Create");
    if (result == FMOD_OK)
    {
        FMOD::NetEventSystem_Init(pEventSys);
        result = CheckFMODResult(
            pEventSys->init(64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL),
            "EventSystem::init"
        );
        if (result != FMOD_OK)
        {
            CheckFMODResult(pEventSys->release(), "EventSystem::release");
            FMOD::NetEventSystem_Shutdown();
            pEventSys = nullptr;
        }
    }
#endif

    // ---------------- 线程初始化（若需，补充官方模板的线程安全逻辑） ----------------
#ifdef SF_ENABLE_THREADS
    if (ThreadedUpdate && !pUpdateThread)
    {
        StopThread = false;
        // 若项目有线程创建逻辑，此处补充（官方模板若有线程则会显式初始化）
        // pUpdateThread = CreateThread(...); 
    }
#endif

    return (pDevice != nullptr);
}

// ===================== 改造 xFinalize 函数（对齐官方销毁流程） =====================
void SoundRendererFMODImpl::xFinalize()
{
    // ---------------- 1. 销毁 EventSystem（原有逻辑 + 官方错误检查） ----------------
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    if (pEventSys)
    {
        CheckFMODResult(pEventSys->release(), "EventSystem::release");
        FMOD::NetEventSystem_Shutdown();
        pEventSys = nullptr;
    }
#endif

    // ---------------- 2. 销毁更新线程（官方模板：先终止线程，再释放资源） ----------------
#ifdef SF_ENABLE_THREADS
    if (pUpdateThread)
    {
        StopThread = true;
        EventObj.PulseEvent();
        pUpdateThread->Wait();
        // 补充：释放线程对象（官方模板会显式释放线程资源）
        delete pUpdateThread;
        pUpdateThread = nullptr;
    }
#endif

    // ---------------- 3. 释放 SampleList 资源（原有逻辑保留，补充注释） ----------------
    {
        Lock::Locker guard(&SampleListLock);
        // 官方模板风格：循环释放链表资源，确保无内存泄漏
        while (SampleList.pFirst != &SampleList)
        {
            SampleList.pFirst->ReleaseResource();
        }
    }

    // ---------------- 4. 核心：FMOD System 销毁（官方模板必有的 close + release） ----------------
    if (pDevice)
    {
        // 官方流程：先 close 再 release
        CheckFMODResult(pDevice->close(), "System::close");
        CheckFMODResult(pDevice->release(), "System::release");
        pDevice = nullptr;
        SystemBitRate = 0.0f;
    }
}

#ifdef SF_ENABLE_THREADS
int SoundRendererFMODImpl::UpdateFunc(Thread*, void* h)
{
    SoundRendererFMODImpl* pRenderer = (SoundRendererFMODImpl*)h;
    unsigned wait_time = 2000; // SF_WAIT_INFINITE;
    while(1)
    {
        pRenderer->EventObj.Wait(wait_time);
        if (pRenderer->StopThread)
            break;
        Lock::Locker lock(&pRenderer->AuxStreamsLock);
        wait_time = (unsigned)(pRenderer->UpdateAuxStreams() * 1000);
    }
    return 0;
}
#endif

float SoundRendererFMODImpl::UpdateAuxStreams()
{
    float nextcall = 0.5f;
    for (AuxStreamersType::Iterator it = AuxStreamers.Begin(); it != AuxStreamers.End(); ++it)
    {
        Array<SoundChannelFMODImplAux*>& cont = it->Second;
        for(UPInt i = 0; i < cont.GetSize(); ++i)
        {
            SoundChannelFMODImplAux* pchan = cont[i];
            if (pchan)
            {
                float t = pchan->Update();
                if (t < nextcall) nextcall = t;
            }
        }
    }
    return nextcall;
}

float SoundRendererFMODImpl::Update()
{
    float nextcall = 0.5f;
#ifdef SF_ENABLE_THREADS
    if (!pUpdateThread)
        nextcall = UpdateAuxStreams();
#else
    nextcall = UpdateAuxStreams();
#endif
    if (CallFMODUpdate)
    {
        pDevice->update();

#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
        if (pEventSys) {
            pEventSys->update();
            FMOD::NetEventSystem_Update();
        }
#endif
    }
    return nextcall;
}

void SoundRendererFMODImpl::Mute(bool mute)
{
    FMOD::ChannelGroup *pcg;
    FMOD_RESULT result;
    result = pDevice->getMasterChannelGroup(&pcg);
    if (result == FMOD_OK)
    {
        pcg->setMute(mute);
    }
}

void SoundRendererFMODImpl::LogError(FMOD_RESULT result)
{
    if (result != FMOD_OK && result != FMOD_ERR_INVALID_HANDLE && result != FMOD_ERR_CHANNEL_STOLEN)
    {
        const char* result_cstr = FMOD_ErrorString(result);
        SF_UNUSED(result_cstr);
        SF_DEBUG_ERROR2(1, "FMOD error! (%d) %s\n", result, result_cstr);
    }
}

SoundChannelFMODImpl* SoundRendererFMODImpl::PlaySample(SoundSample* ps, bool paused)
{
    SoundSampleFMODImpl* psample = (SoundSampleFMODImpl *) ps;
    if (!psample)
        return NULL;
    return psample->Start(paused);
}

SoundRendererFMODImpl::SoundRendererFMODImpl() :
    pDevice(NULL), 
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    pEventSys(NULL),
#endif
    CallFMODUpdate(false), SystemBitRate(0.0f)
{
#ifdef SF_ENABLE_THREADS
    StopThread = false;
#endif
}

float SoundRendererFMODImpl::GetMasterVolume()
{
    if (!pDevice)
    {
        std::cerr << ("FMOD system is null") << std::endl;
        return 1.0f;
    }

    float v = 1.0f;
    FMOD::ChannelGroup* master = nullptr;

    FMOD_RESULT result = pDevice->getMasterChannelGroup(&master);
    if (result != FMOD_OK || !master)
    {
        LogError(result);
        return v;
    }

    result = master->getVolume(&v);
    if (result != FMOD_OK)
    {
        LogError(result);
        return 1.0f;
    }

    return v;
}

void SoundRendererFMODImpl::SetMasterVolume(float volume)
{
    if (!pDevice)
    {
        std::cerr << ("FMOD system is null") << std::endl;
        return;
    }

    FMOD::ChannelGroup* master = nullptr;

    FMOD_RESULT result = pDevice->getMasterChannelGroup(&master);
    if (result != FMOD_OK || !master)
    {
        LogError(result);
        return;
    }

    result = master->setVolume(volume);
    if (result != FMOD_OK)
    {
        LogError(result);
    }
}

//////////////////////////////////////////////////////////////////////////
//

SoundSampleFMODImpl::SoundSampleFMODImpl(SoundRendererFMODImpl* prenderer) :
    SoundSampleImplNode(&prenderer->SampleList)
{
    pPlayer = prenderer;
    pSound    = NULL;
}

SoundSampleFMODImpl::~SoundSampleFMODImpl()
{
    ReleaseFMODObjects();
    if (!pPlayer)
        return;
    Lock::Locker guard(&pPlayer->SampleListLock);
    if (pFirst)
        RemoveNode();
}

void SoundSampleFMODImpl::ReleaseFMODObjects()
{
    if (pSound)
        pSound->release();
    pSound = NULL;
}

void SoundSampleFMODImpl::ReleaseResource()
{
    pPlayer = 0;
    if (AddRef_NotZero())
    {
        ReleaseFMODObjects();
        if (pNext)
            RemoveNode();
        Release();
    }
    else  {
        if (pNext)
            RemoveNode();
    }
}

float SoundSampleFMODImpl::GetDuration() const
{
    if (pSound)
    {
        unsigned slen = 0;
        FMOD_RESULT result = pSound->getLength(&slen, FMOD_TIMEUNIT_MS);
        if (result == FMOD_OK)
            return float(slen/1000.f);
        pPlayer->LogError(result);
    }
    return 0.0f;
}

int SoundSampleFMODImpl::GetBytesTotal() const
{
    if (pSound)
    {
        unsigned slen = 0;
        FMOD_RESULT result = pSound->getLength(&slen, FMOD_TIMEUNIT_RAWBYTES);
        if (result == FMOD_OK)
            return slen;
        pPlayer->LogError(result);
    }
    return 0;
}
int SoundSampleFMODImpl::GetBytesLoaded() const
{
    if (pSound)
    {
        unsigned slen = 0;
        FMOD_RESULT result = pSound->getLength(&slen, FMOD_TIMEUNIT_RAWBYTES);
        if (result == FMOD_OK)
            return slen;
        pPlayer->LogError(result);
    }
    return 0;
}

inline SoundRenderer* SoundSampleFMODImpl::GetSoundRenderer()  const
{
    return pPlayer;
}

//////////////////////////////////////////////////////////////////////////
//

FMOD_RESULT SoundSampleFMODImpl::CreateSubSound(SoundData* psd, FMOD::Sound** psound)
{
    FMOD_CREATESOUNDEXINFO exinfo;
    Alg::MemUtil::Set(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

    FMOD_MODE flags = FMOD_LOWMEM | FMOD_IGNORETAGS;
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	flags |= FMOD_DEFAULT;
#else
	flags |= FMOD_DEFAULT;
#endif
#if defined(SF_OS_IPHONE) || defined(SF_OS_ANDROID)
    flags |= FMOD_OPENMEMORY;
#else
    flags |= FMOD_OPENMEMORY_POINT;
#endif
    switch (psd->GetFormat() & SoundData::Sample_Format)
    {
    case SoundData::Sample_PCM:
        exinfo.format = (psd->GetFormat() & 0x7) == 2 ? FMOD_SOUND_FORMAT_PCM16 : FMOD_SOUND_FORMAT_PCM8;
        exinfo.defaultfrequency = psd->GetRate();
        exinfo.numchannels = (psd->GetFormat() & SoundData::Sample_Stereo) ? 2 : 1;
        flags |= FMOD_OPENRAW;
        break;

    // In Modern FMOD, FMOD_SOUND_FORMAT_MPEG Is Wasted.
    // So There is no need to set exinfo.format for MP3, FMOD will detect it by itself.
    case SoundData::Sample_MP3:
        flags |= FMOD_CREATECOMPRESSEDSAMPLE;
        break;
    default:
        std::cerr << "emmmmmmmm..........." << std::endl;
        return FMOD_ERR_FORMAT;
    }
    exinfo.length = psd->GetDataSize();    
    FMOD_RESULT result = pPlayer->pDevice->createSound((const char*)psd->GetData(), flags, &exinfo, psound);
    return result;
}

FMOD_RESULT SoundSampleFMODImpl::CreateSubSound(AppendableSoundData* psd, FMOD::Sound** psound)
{
    FMOD_CREATESOUNDEXINFO exinfo;
    Alg::MemUtil::Set(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.fileuseropen = &DecodeOpen;
    exinfo.fileuserclose= &DecodeClose;
    exinfo.fileuserread = &DecodeRead;
    exinfo.fileuserseek = &DecodeSeek;
    exinfo.decodebuffersize = 1024 * 4;

    FMOD_MODE flags = FMOD_LOWMEM | FMOD_CREATESTREAM | FMOD_IGNORETAGS;
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	flags |= FMOD_DEFAULT;
#else
	flags |= FMOD_DEFAULT;
#endif
    switch (psd->GetFormat() & SoundData::Sample_Format)
    {
    case SoundData::Sample_PCM:
        exinfo.format = (psd->GetFormat() & 0x7) == 2 ? FMOD_SOUND_FORMAT_PCM16 : FMOD_SOUND_FORMAT_PCM8;
        exinfo.defaultfrequency = psd->GetRate();
        exinfo.numchannels = (psd->GetFormat() & SoundData::Sample_Stereo) ? 2 : 1;
        flags |= FMOD_OPENRAW;
        break;

    // In Modern FMOD, FMOD_SOUND_FORMAT_MPEG Is Wasted.
    // So There is no need to set exinfo.format for MP3, FMOD will detect it by itself.
    case SoundData::Sample_MP3:
        exinfo.defaultfrequency = psd->GetRate();
        break;
    default:
        std::cerr << "emmmmmmmm..........." << std::endl;
        return FMOD_ERR_FORMAT;
    }
    exinfo.initialseekposition = psd->GetSeekSample();
    exinfo.initialseekpostype = FMOD_TIMEUNIT_PCM;
    FMOD_RESULT result = pPlayer->pDevice->createSound((const char*)psd, flags, &exinfo, psound);
    return result;
}

FMOD_RESULT SoundSampleFMODImpl::CreateSubSound(SoundFile* psd, FMOD::Sound** psound)
{
#if defined(GFX_SOUND_OUTPUT_DRC) && defined(SF_OS_WIIU)
	FMOD_MODE flags = FMOD_DEFAULT;
#else
	FMOD_MODE flags = FMOD_DEFAULT;
#endif
    if (psd->IsStreamSample())
        flags |= FMOD_CREATESTREAM;
    else
        flags |= FMOD_ACCURATETIME;

    FMOD_RESULT result = pPlayer->pDevice->createSound(psd->GetFileName(), flags, NULL, psound);
    if (result != FMOD_OK)
    {
        pPlayer->LogError(result);
        *psound = NULL;
        return result;
    }
    return result;
}

SoundChannelFMODImpl* SoundSampleFMODImpl::Start(bool paused)
{
    if (pSound)
    {
        unsigned scount = pSoundData ? pSoundData->GetSampleCount() : 0;
        if (scount == 0)
            pSound->getLength(&scount, FMOD_TIMEUNIT_PCM);

        float sample_rate;
        pSound->getDefaults(&sample_rate,NULL);

        FMOD::Channel* pchan = nullptr;
        FMOD_RESULT    r;
        r = pPlayer->pDevice->playSound(pSound, nullptr, paused, &pchan);

        if (r != FMOD_OK)
        {
            pPlayer->LogError(r);
            std::cerr << "emmmmmmmm........... looks like has problem for it........." << std::endl;
            return NULL;
        }

        SoundChannelFMODImpl* pchannel = SF_NEW SoundChannelFMODImpl(pPlayer, this, pchan);
        return pchannel;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//

inline SoundRenderer* SoundChannelFMODImpl::GetSoundRenderer() const
{
    return pPlayer;
}

SoundChannelFMODImpl::SoundChannelFMODImpl(SoundRendererFMODImpl* pp, SoundSampleFMODImpl* ps, FMOD::Channel* pc) :
    SoundChannelImplNode(&pp->SampleList)
{
    pPlayer = pp;
    pSample = ps;
    pChan = pc;

    pChan->setUserData(this);
    pChan->setCallback(CallBackFunc);
}

SoundChannelFMODImpl::~SoundChannelFMODImpl()
{
    ReleaseFMODObjects();
}

void SoundChannelFMODImpl::ReleaseResource()
{
    if (GetRefCount() > 0)
        ReleaseFMODObjects();
    pPlayer = NULL;
    if (pNext)
        RemoveNode();
}
void SoundChannelFMODImpl::ReleaseFMODObjects()
{
    if (pChan)
    {
        Stop();
        pChan->setCallback(NULL);
        pChan->setUserData(NULL);
        pChan = NULL;
    }
}

void SoundChannelFMODImpl::Stop()
{
    if (pChan)
    {
        FMOD_RESULT r = pChan->stop();
        pPlayer->LogError(r);
        pChan = NULL;
    }
}

void SoundChannelFMODImpl::Pause(bool pause)
{
    if (pChan)
    {
        FMOD_RESULT r = pChan->setPaused(pause);
        pPlayer->LogError(r);
    }
}

bool SoundChannelFMODImpl::IsPlaying() const
{
    bool res = false;
    if (pChan)
    {
        FMOD_RESULT r = pChan->isPlaying(&res);
        pPlayer->LogError(r);
    }
    return res;
}

void SoundChannelFMODImpl::SetPosition(float seconds)
{
    unsigned ms = unsigned(seconds * 1000.0f);
    if (pChan)
    {
        unsigned latency = pSample && pSample->pSoundData ?
            pSample->pSoundData->GetSeekSample() * 1000 / pSample->pSoundData->GetRate() : 0;
        FMOD_RESULT r = pChan->setPosition(ms+latency, FMOD_TIMEUNIT_MS);
        pPlayer->LogError(r);
    }
}

float SoundChannelFMODImpl::GetPosition()
{
    unsigned pos = 0;
    unsigned latency = 0;
    if (pChan)
    {
        latency = pSample && pSample->pSoundData ?
               pSample->pSoundData->GetSeekSample() * 1000 / pSample->pSoundData->GetRate() : 0;
        FMOD_RESULT r;
        r = pChan->getPosition(&pos, FMOD_TIMEUNIT_MS);
        pPlayer->LogError(r);
        unsigned pcmbytes = 0;
        r = pChan->getPosition(&pcmbytes, FMOD_TIMEUNIT_PCMBYTES);
        pPlayer->LogError(r);

    }
    return float((pos - latency)/1000.0f);
}

void SoundChannelFMODImpl::Loop(int count, float start, float end)
{
    SF_ASSERT(pSample);
    if (!pChan || count == 0)
        return;

    FMOD_RESULT result;
    if (count > 1)
    {
        pChan->setMode(FMOD_LOOP_NORMAL);
        pChan->setLoopCount(count);
    }
    unsigned seekpos = pSample->pSoundData ? pSample->pSoundData->GetSeekSample() : 0;
    unsigned scount  = pSample->pSoundData ? pSample->pSoundData->GetSampleCount() : 0;
    unsigned slen = 0;
    pSample->pSound->getLength(&slen, FMOD_TIMEUNIT_PCM);
    float sample_rate;
    pSample->pSound->getDefaults(&sample_rate,NULL);

    unsigned start_pcm = start > 0.0f ? unsigned(start * sample_rate) : seekpos;
    unsigned end_pcm = unsigned(end * sample_rate);
    if (end_pcm == 0 || end_pcm > slen - 1)
    {
        if (scount > 0)
            end_pcm = start_pcm + scount;
        else 
            end_pcm = slen - 1;
    }
    result = pChan->setPosition(start_pcm, FMOD_TIMEUNIT_PCM);
    result = pChan->setLoopPoints(start_pcm, FMOD_TIMEUNIT_PCM, end_pcm, FMOD_TIMEUNIT_PCM);

    // result = pChan->setPaused(end);
    SF_UNUSED(result);
}

void SoundChannelFMODImpl::SetVolume(float volume)
{
    if (pChan)
    {
        FMOD_RESULT r = pChan->setVolume(volume);
        pPlayer->LogError(r);
    }
}

float SoundChannelFMODImpl::GetVolume()
{
    float v = 1.0f;
    if (pChan)
    {
        FMOD_RESULT r = pChan->getVolume(&v);
        pPlayer->LogError(r);
    }
    return v;
}

void SoundChannelFMODImpl::SetPan(float pan)
{
    CachedPan = pan;

    if (pChan)
    {
        pChan->setPan(pan); // 如果API支持
    }
}

float SoundChannelFMODImpl::GetPan()
{
    return CachedPan;
}

#define deltaT 0.1f
#define deltaV 0.1f

static inline void s_SetChannelPan(FMOD::Channel* pchan, const SoundChannel::Transform& transform)
{
    FMOD_RESULT r;
    r = pchan->setVolume((transform.LeftVolume + transform.RightVolume)/2);
    // float levels[2];
    // levels[0] = transform.LeftVolume;
    // levels[1] = transform.RightVolume;
	float pan = Alg::Abs(transform.LeftVolume - transform.RightVolume);
    if (transform.LeftVolume > transform.RightVolume)
        pan *= -1;
    r = pchan->setPan(pan);
    SF_UNUSED(r);
}

void SoundChannelFMODImpl::SetTransforms(const Array<Transform>& transforms)
{
    UPInt size = transforms.GetSize();
    if (size == 0)
        return;
    if (!pSample || !pSample->pSound)
        return;

    pChan->setCallback(CallBackFunc);
    for (unsigned i = 0; i < size; ++i)
    {
        if (i == 0)
        {
            s_SetChannelPan(pChan, transforms[0]);
            continue;
        }
        float ar = (transforms[i-1].RightVolume - transforms[i].RightVolume) / 
            (transforms[i-1].Position - transforms[i].Position);
        float al = (transforms[i-1].LeftVolume  - transforms[i].LeftVolume) /
            (transforms[i-1].Position - transforms[i].Position);
        float br = transforms[i-1].RightVolume - ar * transforms[i-1].Position;
        float bl = transforms[i-1].LeftVolume  - al * transforms[i-1].Position;

        bool done = false;
        for(unsigned j = 1; !done ; j++)
        {
            Transform tr;
            if (transforms[i].Position < transforms[i-1].Position + deltaT * j)
            {
                tr = transforms[i];
                done = true;
            }
            else
            {
                tr.Position = transforms[i-1].Position + deltaT * j;
                tr.RightVolume = ar * tr.Position + br;
                tr.LeftVolume  = al * tr.Position + bl;
            }
            FMOD_SYNCPOINT* psync;
            FMOD_RESULT r = pSample->pSound->addSyncPoint(
                unsigned(tr.Position * 1000.0f), FMOD_TIMEUNIT_MS, "", &psync);
            if (r == FMOD_OK)
            {
                Tranforms.Add(psync, tr);
            }
            else
                pPlayer->LogError(r);
        }       
    }
}

FMOD_RESULT F_CALL Scaleform::Sound::SoundChannelFMODImpl::CallBackFunc(
    FMOD_CHANNELCONTROL* chancontrol,
    FMOD_CHANNELCONTROL_TYPE controltype,
    FMOD_CHANNELCONTROL_CALLBACK_TYPE callbacktype,
    void* cmddata1,
    void* cmddata2)
{
    (void)controltype;

    SF_UNUSED(cmddata2);

    if (controltype != FMOD_CHANNELCONTROL_CHANNEL)
        return FMOD_OK;

    FMOD::Channel* pchan = (FMOD::Channel*)chancontrol;

    void* vp = nullptr;

    pchan->getUserData(&vp);

    if (!vp)
        return FMOD_OK;

    SoundChannelFMODImpl* pChan = (SoundChannelFMODImpl*)vp;
    
    FMOD_RESULT r;

    if (callbacktype == FMOD_CHANNELCONTROL_CALLBACK_SYNCPOINT)
    {
        if (pChan && pChan->pSample && pChan->pSample->pSound)
        {
            FMOD_SYNCPOINT* psync;
            r = pChan->pSample->pSound->getSyncPoint((int)(SPInt)cmddata1, &psync);
            if (r == FMOD_OK)
            {
                SoundChannel::Transform* tr = pChan->Tranforms.Get(psync);
                if (tr)
                    s_SetChannelPan(pChan->pChan, *tr);
            }
            else
                pChan->pPlayer->LogError(r);
        }
    }
    else if (callbacktype == FMOD_CHANNELCONTROL_CALLBACK_END)
    {
    }
    return FMOD_OK;
}

//////////////////////////////////////////////////////////////////////////
//

SoundRendererFMOD* SF_CDECL SoundRendererFMOD::CreateSoundRenderer()
{
    return SF_NEW SoundRendererFMODImpl;
}

}} // Scaleform::Sound

#endif  // GFX_ENABLE_SOUND
