/**************************************************************************

Filename    :   Video_Video.cpp
Content     :   Main GFx video classes implementation
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Video/Video_VideoPlayerImpl.h"
#include "Video/Video_VideoCharacter.h"

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Array.h"

#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_Log.h"

#include "Sound/Sound_SoundRenderer.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// Internal GFx Evaluation License Reader
#ifdef SF_BUILD_EVAL
#define SF_EVAL(x) GFxVideo_##x
#define SF_LIB_NAME_S "GFxVideo"
#define SF_PRODUCT_ID SCALEFORM_VIDEO
#define SF_LICENSE_FILE "sf_video_license.txt"
#include "GFCValidateEval.cpp"
#else
void GFxVideo_ValidateEvaluation() {}
void GFxVideo_SetEvalKey(const char* key) {SF_UNUSED(key);}
#endif

#ifdef SF_BUILD_DEBUG
void criErrorFunc(const Char8 *errid, Uint32 p1, Uint32 p2, Uint32 *parray)
{
    SF_UNUSED4(parray, p1, p2, parray);
    fprintf(stderr, "CRI-MW ERROR: %s\n", errid);
}
#endif

//////////////////////////////////////////////////////////////////////////
//

Video::Video(const VideoVMSupport& vmSupport,
             Thread::ThreadPriority priority, bool autoInit) : DecodeThreadPriority(priority)
{
    GFxVideo_ValidateEvaluation();

    pAS2VSupport = vmSupport.pAS2VSupport;
    pAS3VSupport = vmSupport.pAS3VSupport;
    pHeap = *Memory::GetGlobalHeap()->CreateHeap(
        "_Video_Heap", 0, 64, 128*1024, 0, ~UPInt(0), 0, HeapId_Video);
    if (autoInit)
        Initialize();
}

Video::~Video()
{
    pSoundSystem = NULL;
    pReaderConfig = NULL;
    pDecoder = NULL;

	pTextureManager = NULL;

    if (pAS2VSupport)
        SF_FREE(pAS2VSupport);
    if (pAS3VSupport)
        SF_FREE(pAS3VSupport);

    Finalize();
    pHeap = NULL;
}

void Video::Initialize(bool argbInit)
{
    if (!Initialized++)
    {
#ifdef SF_BUILD_DEBUG
        criErr_SetCallback(criErrorFunc);
        criErr_SetErrorNotificationLevel(CRIERR_NOTIFY_ALL);
#endif
        criHeap_Initialize();

        HandleWorkSize = CriMv::CalcMovieHandleWork(VIDEO_HANDLE_MAX);
        pHandleWorkBuf = SF_ALLOC(HandleWorkSize, Stat_Video_Mem);
        CriMv::SetupMovieHandleWork(VIDEO_HANDLE_MAX, pHandleWorkBuf, HandleWorkSize);

#ifdef SF_OS_WII
        // Special case: Wii uses R8G8B8A8 image format
        argbInit = true;
#endif
        CriMv::Initialize();
        if (argbInit)
            CriMv::InitializeFrame32bitARGB();
    }
}

void Video::Finalize()
{
    if (1 == Initialized--)
    {
        CriMv::Finalize();
        if (pHandleWorkBuf)
            SF_FREE(pHandleWorkBuf);
        criHeap_Finalize();
    }
}

void Video::ReadDefineVideoStreamTag(LoadProcess* p, const TagInfo& tagInfo)
{
    int characterId = p->ReadU16();
    p->LogParse("  video = %d\n", characterId);
    Ptr<VideoCharacterDef> ch = *new VideoCharacterDef;
    ch->Read(p, tagInfo.TagType);
    p->AddResource(ResourceId(characterId), ch);
}

unsigned Video::Initialized;
void*    Video::pHandleWorkBuf;
UInt32   Video::HandleWorkSize;

}} // GFx::Video


#ifdef GFX_ENABLE_SOUND

namespace Sound {

//////////////////////////////////////////////////////////////////////////
// 

#define SAMPL_BUFF_SIZE 4096

class SoundChannelDelegate : public SoundChannel
{
public:
	SoundChannelDelegate() : Volume(1.0f), Pan(0.0f), JustCreated(true) {}

    void            SetDelegate(SoundChannel* pdel) 
                                                 { if (pDelegate) pDelegate->Stop(); pDelegate = pdel;
                                                   JustCreated = false; if (pDelegate) pDelegate->SetVolume(Volume); }

    virtual SoundRenderer* GetSoundRenderer() const { return pDelegate? pDelegate->GetSoundRenderer() : NULL; }
    virtual SoundSample*   GetSample() const        { return pDelegate? pDelegate->GetSample() : NULL; }

    virtual void    Stop()                       { if (pDelegate) pDelegate->Stop(); }
    virtual void    Pause(bool pause)            { if (pDelegate) pDelegate->Pause(pause);}
	virtual bool    IsPlaying() const            { return pDelegate? pDelegate->IsPlaying() : JustCreated; }
    virtual void    SetPosition(float seconds)   { if (pDelegate) pDelegate->SetPosition(seconds); }
    virtual float   GetPosition()                { return pDelegate? pDelegate->GetPosition() : 0.0f; }
    virtual void    Loop(int count, float start = 0, float end = 0)
                                                 { if (pDelegate) pDelegate->Loop(count,start,end); }
    virtual float   GetVolume()                  { return pDelegate? pDelegate->GetVolume() : Volume; }
    virtual void    SetVolume(float volume)      { Volume = volume; if (pDelegate) pDelegate->SetVolume(volume); }
    virtual float   GetPan()                     { return pDelegate? pDelegate->GetPan() : Pan; }
    virtual void    SetPan(float pan)            { Pan = pan; if (pDelegate) pDelegate->SetPan(pan); }
    virtual void    SetTransforms(const Array<Transform>& transforms)
                                                 { if (pDelegate) pDelegate->SetTransforms(transforms); }
    virtual void    SetSpatialInfo(const Array<Vector> spatinfo[])
                                                 { if (pDelegate) pDelegate->SetSpatialInfo(spatinfo); }
private:
    Ptr<SoundChannel> pDelegate;
    float             Volume;
    float             Pan;
    bool              JustCreated;
};

} // Sound

namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
// 

#if defined(SF_OS_WII) || defined(SF_OS_PS3)
#define USE_SINI16_STREAM_SOUND
#endif

class VideoSoundRendererStream : public VideoSound
{
public:
    VideoSoundRendererStream(MemoryHeap* pheap, Sound::SoundRenderer* psoundPlayer);
    virtual ~VideoSoundRendererStream() {}

    virtual bool        CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void        DestroyOutput();

#ifdef USE_SINI16_STREAM_SOUND
    virtual PCMFormat   GetPCMFormat() const { return PCM_SInt16; }
#else
    virtual PCMFormat   GetPCMFormat() const { return PCM_Float; }
#endif
    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual Status      GetStatus() const;

    virtual void        Pause(bool sw);
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    virtual Sound::SoundChannel* GetSoundChannel();
    virtual void SetSpatialInfo(const Array<Sound::SoundChannel::Vector> []) {}

public:
    friend class Streamer;

    // Renderer Streamer implementation, that will request data directly from
    // the PCMStream passed to Start().
    class Streamer : public Sound::SoundRenderer::AuxStreamer
    {
    public:
        Streamer(VideoSoundRendererStream* pparent) : pParent(pparent) {}
        virtual ~Streamer() {}

        virtual unsigned GetPCMData(UByte* pdata, unsigned datasize);

#ifdef USE_SINI16_STREAM_SOUND
        // Helper to query 16-bit data
        unsigned GetData(UInt32 nch, SInt16 *pcmbuf[], UInt32 req_nsmpl)
        {
            SF_ASSERT(pParent->pPCMStream);
            return pParent->pPCMStream->GetDataSInt16(nch, pcmbuf, req_nsmpl);
        }
#else
        // Helper to query float 32-bit data
        unsigned GetData(UInt32 nch, float *pcmbuf[], UInt32 req_nsmpl)
        {
            SF_ASSERT(pParent->pPCMStream);
            return pParent->pPCMStream->GetDataFloat(nch, pcmbuf, req_nsmpl);
        }
#endif
        VideoSoundRendererStream* pParent;
    };
private:
    MemoryHeap*        pHeap;

    Ptr<Streamer>      pStreamer;
    Ptr<Sound::SoundRenderer>        pSoundPlayer;
    Ptr<Sound::SoundChannelDelegate> pChannel;
    
    PCMStream*          pPCMStream;
    unsigned            ChannelCount;    
    UInt32              SampleRate;
    Status              Stat;
    UInt64              TotalGotSamples;
    
#ifdef USE_SINI16_STREAM_SOUND
    SInt16              *SampleBuffers[MaxChannels];
#else
    float               *SampleBuffers[MaxChannels];
#endif
};

VideoSoundRendererStream::VideoSoundRendererStream(MemoryHeap* pheap,
                                                   Sound::SoundRenderer* psoundPlayer) :
    pHeap(pheap), pSoundPlayer(psoundPlayer),
    pPCMStream(0), ChannelCount(0), SampleRate(0), Stat(Sound_Stopped), TotalGotSamples(0)
{
    for (unsigned i = 0; i < MaxChannels; ++i)
        SampleBuffers[i] = NULL;
}

bool VideoSoundRendererStream::CreateOutput(UInt32 channels, UInt32 samplerate)
{
    if (channels == 0)
        return false;
    
    SF_ASSERT(ChannelCount <= MaxChannels);

    ChannelCount = channels;
    SampleRate   = samplerate;
    Stat         = Sound_Stopped;
    
    for (unsigned i = 0; i < ChannelCount; ++i)
    {
#ifdef USE_SINI16_STREAM_SOUND
        SampleBuffers[i] = (SInt16*)SF_HEAP_ALLOC(pHeap, sizeof(SInt16)*SAMPL_BUFF_SIZE, Stat_Video_Mem);
#else
        SampleBuffers[i] = (float*)SF_HEAP_ALLOC(pHeap, sizeof(float)*SAMPL_BUFF_SIZE, Stat_Video_Mem);
#endif
        if (!SampleBuffers[i])
        {
            while (i > 0)
            {
                i--;
                SF_HEAP_FREE(pHeap, SampleBuffers[i]);
                SampleBuffers[i] = NULL;
            }
            return false;
        }
    }
    return true;
}

void VideoSoundRendererStream::DestroyOutput()
{
    for (unsigned i = 0; i < ChannelCount; ++i)
    {
        SF_HEAP_FREE(pHeap,SampleBuffers[i]);
        SampleBuffers[i] = NULL;
    }
    ChannelCount = 0;
}

void VideoSoundRendererStream::Start(PCMStream* pstream)
{
    SF_ASSERT(pstream != 0);

    Stat            = Sound_Playing;
    TotalGotSamples = 0;
    pPCMStream      = pstream;

    if (!pStreamer)
        pStreamer = *SF_NEW Streamer(this);
    if (!pChannel)
        pChannel = *SF_HEAP_NEW(pHeap) Sound::SoundChannelDelegate;

    Ptr<Sound::SoundSample> psample;
#ifdef USE_SINI16_STREAM_SOUND
    psample = *pSoundPlayer->CreateSampleFromAuxStreamer(pStreamer, ChannelCount, SampleRate,
                                                         Streamer::PCM_SInt16);
#else
    psample = *pSoundPlayer->CreateSampleFromAuxStreamer(pStreamer, ChannelCount, SampleRate,
                                                         Streamer::PCM_Float);
#endif
    Ptr<Sound::SoundChannel> pchan = *pSoundPlayer->PlaySample(psample, false);
    pChannel->SetDelegate(pchan);
}

void VideoSoundRendererStream::Stop(void)
{
    if (pChannel)
        pChannel->SetDelegate(NULL);
    pChannel        = NULL;
    Stat            = Sound_Stopped;
    TotalGotSamples = 0;
    pPCMStream      = 0;
}

void VideoSoundRendererStream::Pause(bool sw)
{
    if (sw == true)
    {
        if (Stat == Sound_Playing)
        {
            Stat = Sound_Paused;
            if (pChannel)
                pChannel->Pause(true);
        }
    }
    else
    {
        if (Stat == Sound_Paused)
        {
            Stat = Sound_Playing;
            if (pChannel)
                pChannel->Pause(false);
        }
    }
}

VideoSound::Status VideoSoundRendererStream::GetStatus() const
{
    return Stat;
}

void VideoSoundRendererStream::GetTime(UInt64* count, UInt64* unit) const
{   
    if (TotalGotSamples == 0)
    {
        *count = 0;
        *unit  = 1000;
    }
    else
    {       
        if (pChannel)
        {
            float pos = pChannel->GetPosition();
            *count = (UInt64)(pos * 1000);
            *unit = 1000;
        }
        else
        {
            *count = TotalGotSamples * 1000 / SampleRate;
            *unit = 1000;
        }
    }
}

Sound::SoundChannel* VideoSoundRendererStream::GetSoundChannel()
{
    if (!pChannel)
        pChannel = *SF_HEAP_NEW(pHeap) Sound::SoundChannelDelegate;
    return pChannel;
}

unsigned VideoSoundRendererStream::Streamer::GetPCMData(UByte* pdata, unsigned datasize)
{
    UInt32   recv_nsmpl = 0;
    unsigned recv_bytes = 0;
#ifdef USE_SINI16_STREAM_SOUND
    unsigned ssize  = 16;
    UInt32   nsmpl  = datasize * 8 / ssize / pParent->ChannelCount;
    SInt16  *buffer = (signed short *)pdata;
#else
    unsigned ssize  = 32;
    UInt32   nsmpl  = datasize * 8 / ssize / pParent->ChannelCount;
    float   *buffer = (float*)pdata;
#endif

    if (pParent->pPCMStream) 
    {
        if (pParent->ChannelCount == 1) 
        {
            recv_nsmpl = GetData(1, &buffer, nsmpl);
        }
        else 
        {
            while (nsmpl > 0)
            {
                UInt32 gsmpl = GetData(pParent->ChannelCount, pParent->SampleBuffers,
                    nsmpl > SAMPL_BUFF_SIZE ? SAMPL_BUFF_SIZE: nsmpl);
                if (gsmpl == 0)
                    break;

                for (unsigned count=0; count < gsmpl; count++)
                {
                    for (unsigned i = 0; i < pParent->ChannelCount; ++i)
                        *buffer++ = pParent->SampleBuffers[i][count]; 
                }
                nsmpl -= gsmpl;
                recv_nsmpl += gsmpl;
            }
        }
    }

    pParent->TotalGotSamples += recv_nsmpl;
    recv_bytes = recv_nsmpl * ssize * pParent->ChannelCount / 8;
    SF_ASSERT(recv_bytes <= datasize);
    return recv_bytes;
}

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundSystem_SoundRenderer : public VideoSoundSystem
{
public:
    VideoSoundSystem_SoundRenderer(Sound::SoundRenderer* psoundPlayer, MemoryHeap* pheap) :
        VideoSoundSystem(pheap), pSoundPlayer(psoundPlayer) {}
    virtual ~VideoSoundSystem_SoundRenderer() {}

    virtual VideoSound* Create()
    {
        return SF_HEAP_NEW(GetHeap()) VideoSoundRendererStream(GetHeap(), pSoundPlayer);
    }

protected:
    Ptr<Sound::SoundRenderer> pSoundPlayer;
};

void Video::SetSoundSystem(Sound::SoundRenderer* psoundRenderer)
{
    SF_ASSERT(pHeap);

    if (!psoundRenderer)
        return;
    UInt32 cap_bits = 0;
    psoundRenderer->GetRenderCaps(&cap_bits);
    if (cap_bits & Sound::SoundRenderer::Cap_NoVideoSound)
        return;

    Ptr<VideoSoundSystem> psc = *SF_NEW VideoSoundSystem_SoundRenderer(psoundRenderer, pHeap);
    SetSoundSystem(psc);
}

}} // GFx::Video

#endif // GFX_ENABLE_SOUND

namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

void VideoReaderConfig::DetachReader(VideoReader* preader)
{
	for(size_t i = 0; i < Readers.GetSize(); ++i)
	{
		if (Readers[i] == preader)
		{
			Readers.RemoveAt(i);
			break;
		}
	}
}

void VideoReaderConfig::AttachReader(VideoReader* preader)
{
	Readers.PushBack(preader);
}

//////////////////////////////////////////////////////////////////////////
//

VideoPlayer* Video::CreateVideoPlayer(MemoryHeap* pheap, TaskManager* ptaskManager, 
                                      FileOpenerBase* pfileOpener, Log* plog)
{
    SF_UNUSED(pheap);

    SF_ASSERT(Initialized);
    SF_ASSERT(pHeap);

    if (!pDecoder)
    {
        pDecoder = *SF_HEAP_NEW(pHeap) VideoDecoderThrd(pHeap, DecodeThreadPriority);
//      pDecoder = *SF_HEAP_NEW(pHeap) VideoDecoderSmp();
        ApplySystemSettings(pDecoder);
    }
    if (!pReaderConfig)
    {
        pReaderConfig = *SF_HEAP_NEW(pHeap) VideoReaderConfigThrd(pHeap, pfileOpener);
//      pReaderConfig = *SF_HEAP_NEW(pHeap) VideoReaderConfigSmp(pHeap, pfileOpener);
        ApplySystemSettings(pReaderConfig);
        pReaderConfig->SetReadCallback(pReadCallback);
    }

    VideoPlayerImpl* pplayer = SF_HEAP_NEW(pHeap) VideoPlayerImpl(pHeap);
    if (pplayer->Init(this, ptaskManager, pfileOpener, plog))
        ApplySystemSettings(pplayer);
    else {
        pplayer->Destroy();
        pplayer= NULL;
    }

    return pplayer;
}

void Video::SetReadCallback(ReadCallback* pcallback)
{
    pReadCallback = pcallback;
    if (pReaderConfig)
        pReaderConfig->SetReadCallback(pReadCallback);
}

bool Video::IsIORequired() const
{
    if (!pDecoder)
        return false;
    return pDecoder->IsIORequired();
}

void Video::EnableIO(bool enable)
{
    if (pReaderConfig)
        pReaderConfig->EnableIO(enable);
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
