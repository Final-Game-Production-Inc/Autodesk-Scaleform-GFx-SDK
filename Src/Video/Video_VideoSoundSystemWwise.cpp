/**************************************************************************

Filename    :   Video_VideoSoundSystemWwise.cpp
Content     :   Video sound system implementation based on AK Wwise sound system
Created     :   August 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoSoundSystemWwise.h"

#ifdef GFX_ENABLE_VIDEO
// Wwise support for Windows, Mac, Xbox360, PS3 and Wii only
#if defined(SF_OS_WIN32)   || defined(SF_OS_MAC) || \
    defined(SF_OS_XBOX360) || defined(SF_OS_PS3) || defined(SF_OS_WII) || defined(SF_OS_ORBIS)

#include "Video/Video_SystemSoundInterface.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Alg.h"

#if defined(SF_OS_XBOX360) && !defined(XBOX360)
#define XBOX360
#endif

#include <AK/AkWwiseSDKVersion.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>          // Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>             // Default memory and stream managers
#include <AK/SoundEngine/Common/AkSoundEngine.h>        // Sound engine
#include <AK/SoundEngine/Common/IAkStreamMgr.h>         // Streaming Manager
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>            // Thread defines
#include <AK/Plugin/AkAudioInputSourceFactory.h>        // AudioInputSource plug-in
#include <AK/SoundEngine/Common/AkQueryParameters.h>	// Queries

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundWwise : public VideoSoundStream
{
public:
    VideoSoundWwise(MemoryHeap* pheap);
    virtual ~VideoSoundWwise();

    virtual bool        CreateOutput(UInt32 channel, UInt32 smplrate);
    virtual void        DestroyOutput();

#if defined(SF_OS_WII)
    virtual PCMFormat   GetPCMFormat() const       { return PCM_SInt16; }
#else
    virtual PCMFormat   GetPCMFormat() const       { return PCM_Float; }
#endif

    virtual void        Start(PCMStream* pstream);
    virtual void        Stop();
    virtual void        Pause(bool sw);

    virtual Status      GetStatus() const          { return SoundStatus; }
    virtual void        GetTime(UInt64* count, UInt64* unit) const;

    virtual void        Update() {}

    virtual void        SetVolume(float volume);
    virtual float       GetVolume()                { return Volume; }
    UInt32              GetChannelNumber() const   { return ChannelCount; }
    UInt32              GetSampleRate() const      { return SampleRate; }

    virtual void        SetSpatialInfo(const Array<Sound::SoundChannel::Vector> spatinfo[]);

    void                UpdateAudioBuffer(AkAudioBuffer* io_pBufferOut);

private:
    friend class VideoSoundSystemWwiseImpl;

    MemoryHeap*         pHeap;
    PCMStream*          pDataStream;

    UInt32              ChannelCount;
    Status              SoundStatus;    
    UInt64              TotalSamples;
    UInt32              SampleRate;
    float               Volume;

#if defined(SF_OS_WII)
    UInt32              GetSoundData(UInt32 nch, SInt16 *sample[], UInt32 nsmpl);
#else
    UInt32              GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl);
#endif

    AkPlayingID         PlayingID;
    static Hash<AkPlayingID, VideoSoundWwise*> SoundMap;

    AKRESULT res;
};

//////////////////////////////////////////////////////////////////////////
//

Hash<AkPlayingID, VideoSoundWwise*> VideoSoundWwise::SoundMap;

VideoSoundWwise::VideoSoundWwise(MemoryHeap* pheap) :
    pHeap(pheap), pDataStream(0),
    ChannelCount(0), SoundStatus(Sound_Stopped),
    TotalSamples(0), SampleRate(0), Volume(1.0f)
{
    PlayingID = AK_INVALID_PLAYING_ID;
}

VideoSoundWwise::~VideoSoundWwise()
{
}

bool VideoSoundWwise::CreateOutput(UInt32 channel, UInt32 smplrate)
{
    if (channel == 0)
        return false;

    ChannelCount = channel;
    SampleRate = smplrate;

    res = AK::SoundEngine::RegisterGameObj((AkGameObjectID)this);
    SF_ASSERT(res == AK_Success);

    return true;
}

void VideoSoundWwise::DestroyOutput()
{
    res = AK::SoundEngine::UnregisterGameObj((AkGameObjectID)this);
    SF_ASSERT(res == AK_Success);
}

void VideoSoundWwise::SetSpatialInfo(const Array<Sound::SoundChannel::Vector> spatinfo[])
{
    Array<AkSoundPosition> sndpos;
    for (UInt32 i = 0; i < spatinfo[0].GetSize(); ++i)
    {
        AkSoundPosition p;
        p.Position.X = spatinfo[0][i].X;
        p.Position.Y = spatinfo[0][i].Y;
        p.Position.Z = spatinfo[0][i].Z;
        if (spatinfo[1].GetSize() > i)
        {
            p.Orientation.X = spatinfo[1][i].X;
            p.Orientation.Y = spatinfo[1][i].Y;
            p.Orientation.Z = spatinfo[1][i].Z;
        }
        else {
            p.Orientation.X = p.Orientation.Y = p.Orientation.Z = 0.0f;
        }
        sndpos.PushBack(p);
    }

    if (sndpos.GetSize())
    {
        res = AK::SoundEngine::SetMultiplePositions((AkGameObjectID)this, 
            &sndpos[0], (AkUInt16)sndpos.GetSize(),
            AK::SoundEngine::MultiPositionType_MultiSources);
        SF_ASSERT(res == AK_Success);
    }
}

void VideoSoundWwise::Start(PCMStream* pstream)
{
    pDataStream  = pstream;
    SoundStatus = Sound_Playing;

    res = PlayAudioInput(PlayingID, (AkGameObjectID)this);
    SF_ASSERT(res == AK_Success);

	SoundMap.Set(PlayingID, this);
    TotalSamples = 0;
}

void VideoSoundWwise::Stop()
{
    SoundStatus = Sound_Stopped;
    pDataStream  = 0;
    if (PlayingID != AK_INVALID_PLAYING_ID)
    {
        res = StopAudioInput(PlayingID);
        SF_ASSERT(res == AK_Success);

        SoundMap.Remove(PlayingID);
        PlayingID = AK_INVALID_PLAYING_ID;
    }
    TotalSamples = 0;
}

void VideoSoundWwise::Pause(bool sw)
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

void VideoSoundWwise::GetTime(UInt64* count, UInt64* unit) const
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

void VideoSoundWwise::SetVolume(float volume)
{
    Volume = volume;
}

void VideoSoundWwise::UpdateAudioBuffer(AkAudioBuffer* io_pBufferOut)
{

	static UInt32 ch_map[8]={ 0, 1, 3, 4, 2, 5, 6, 7 };
//	UInt32* map = &ch_map[ 0 ];
    AkSampleType* pBufOuts[6];
    for (UInt32 i = 0; i < io_pBufferOut->NumChannels(); ++i)
        pBufOuts[i] = io_pBufferOut->GetChannel(ch_map[ i ]);

    if (SoundStatus == Sound_Paused)
    {
        for (UInt32 i = 0; i < io_pBufferOut->NumChannels(); ++i)
            Alg::MemUtil::Set(pBufOuts[i], 0, io_pBufferOut->MaxFrames() * sizeof(AkSampleType));
        io_pBufferOut->uValidFrames = io_pBufferOut->MaxFrames();
        io_pBufferOut->eState = AK_DataReady;
    }
	else
	{
        io_pBufferOut->uValidFrames = (AkUInt16)GetSoundData(io_pBufferOut->NumChannels(), pBufOuts, io_pBufferOut->MaxFrames());
        io_pBufferOut->eState = io_pBufferOut->uValidFrames ? AK_DataReady : AK_NoDataReady;
    }
}

// Get PCM data
#if defined(SF_OS_WII)
UInt32 VideoSoundWwise::GetSoundData(UInt32 nch, SInt16 *sample[], UInt32 nsmpl)
#else
UInt32 VideoSoundWwise::GetSoundData(UInt32 nch, float *sample[], UInt32 nsmpl)
#endif
{
    UInt32 recv_nsmpl = 0;

    if (pDataStream)
#if defined(SF_OS_WII)
        recv_nsmpl = pDataStream->GetDataSInt16(nch, sample, nsmpl);
#else
        recv_nsmpl = pDataStream->GetDataFloat(nch, sample, nsmpl);
#endif

    TotalSamples += recv_nsmpl;
    return recv_nsmpl;
}

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundSystemWwiseImpl : public NewOverrideBase<StatMD_Other_Mem>
{
public:
    VideoSoundSystemWwiseImpl();
    ~VideoSoundSystemWwiseImpl();

    bool IsInitialized() const { return Initialized; }

    void Update();

protected:
    static void     GetFormatCallback(AkPlayingID in_PlayingID, AkAudioFormat& io_AudioFormat);
    static void     ExecuteCallback  (AkPlayingID in_PlayingID, AkAudioBuffer* io_pBufferOut);
    static AkReal32 GetGainCallback  (AkPlayingID in_PlayingID);

	bool Initialized;
};

VideoSoundSystemWwiseImpl::VideoSoundSystemWwiseImpl(): Initialized(false)
{
    // The sound engine should be already initialized
    SF_ASSERT(AK::SoundEngine::IsInitialized());

    // Register AudioInputSource plug-in
    SetAudioInputCallbacks(ExecuteCallback, GetFormatCallback, GetGainCallback);
    AKRESULT res = AK::SoundEngine::RegisterPlugin(
        AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_AUDIOINPUT,
        CreateAudioInputSource, CreateAudioInputSourceParams);
    SF_ASSERT(res == AK_Success);
    SF_UNUSED(res);

    Initialized = true;
}

VideoSoundSystemWwiseImpl::~VideoSoundSystemWwiseImpl()
{
    if (AK::SoundEngine::IsInitialized())
        Update();
    VideoSoundWwise::SoundMap.Clear();
}

void VideoSoundSystemWwiseImpl::Update()
{
    // Update the sound engine
    AK::SoundEngine::RenderAudio();

    // Call update for each video sound object
    Hash<AkPlayingID, VideoSoundWwise*>::ConstIterator it;
    for (it = VideoSoundWwise::SoundMap.Begin(); it != VideoSoundWwise::SoundMap.End(); ++it)
        it->Second->Update();
}

void VideoSoundSystemWwiseImpl::GetFormatCallback(
    AkPlayingID    in_PlayingID,    // Playing ID
    AkAudioFormat& io_AudioFormat   // Already filled format, modify it if required
    )
{
    VideoSoundWwise* pVideoSound = NULL;
    VideoSoundWwise::SoundMap.Get(in_PlayingID, &pVideoSound);
    if (pVideoSound)
    {
        UInt32 channels = pVideoSound->GetChannelNumber();
#if defined(SF_OS_WII)
        UInt32 bits_per_sample = 16;
        UInt32 sample_format = AK_INT;
#else
        UInt32 bits_per_sample = 32;
        UInt32 sample_format = AK_FLOAT;
#endif
        UInt32 block_align = channels * (bits_per_sample/8);

        io_AudioFormat.SetAll(
            pVideoSound->GetSampleRate(),
            channels == 6 ? AK_SPEAKER_SETUP_5POINT1 :
            channels == 2 ? AK_SPEAKER_SETUP_STEREO  : AK_SPEAKER_SETUP_MONO,
            bits_per_sample, block_align, sample_format,
            sample_format == AK_FLOAT ? AK_NONINTERLEAVED : AK_INTERLEAVED
        );
    }
}

void VideoSoundSystemWwiseImpl::ExecuteCallback(
    AkPlayingID    in_PlayingID,    // Playing ID
    AkAudioBuffer* io_pBufferOut    // Buffer to fill
    )
{
    VideoSoundWwise* pVideoSound = NULL;
    VideoSoundWwise::SoundMap.Get(in_PlayingID, &pVideoSound);
    if (pVideoSound)
        pVideoSound->UpdateAudioBuffer(io_pBufferOut);
    else {
        io_pBufferOut->uValidFrames = 0;
        io_pBufferOut->eState = AK_Fail;
    }
}

AkReal32 VideoSoundSystemWwiseImpl::GetGainCallback(
    AkPlayingID    in_PlayingID     // Playing ID
    )
{
    VideoSoundWwise* pVideoSound = NULL;
    VideoSoundWwise::SoundMap.Get(in_PlayingID, &pVideoSound);

	AK::SoundEngine::Query::RTPCValue_type type = AK::SoundEngine::Query::RTPCValue_GameObject;
	AkRtpcValue value = 0;	
	AkPlayingID result = AK::SoundEngine::Query::GetRTPCValue( "menu_volume_movies", AK_INVALID_GAME_OBJECT , value, type );
	if(result == AK_Success)
	{
		// the value is consistent with other menu_volume_xxx which are 0-100. But we need the gain to be 0-1.
		return value / AkReal32(100.0f); 
	}

    if (pVideoSound)
        return pVideoSound->GetVolume();

    return 1.0f;
}

//////////////////////////////////////////////////////////////////////////
//

VideoSoundSystemWwise::VideoSoundSystemWwise(MemoryHeap* pheap) :
    VideoSoundSystem(pheap)
{
    pImpl = SF_HEAP_NEW(pheap) VideoSoundSystemWwiseImpl;
}

VideoSoundSystemWwise::~VideoSoundSystemWwise()
{
    delete pImpl;
}

void VideoSoundSystemWwise::Update()
{
    if (pImpl)
        pImpl->Update();
}

VideoSound* VideoSoundSystemWwise::Create()
{
    if (pImpl->IsInitialized())
        return SF_HEAP_NEW(GetHeap()) VideoSoundWwise(GetHeap());
    return NULL;
}

}}} // Scaleform::GFx::Video

#endif
#endif // GFX_ENABLE_VIDEO
