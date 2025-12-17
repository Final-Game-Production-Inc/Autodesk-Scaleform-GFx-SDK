/**************************************************************************

Filename    :   GFx_Sound.cpp
Content     :   GFx audio support
Created     :   September 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_Sound.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

// Utility function to uncompress ADPCM.
static void GFx_AdpcmExpand(
    void* DataOut,
    Stream* in,
    int N_bits,         // 2 to 5 bits
    int sampleCount,    // in stereo, this is number of *pairs* of samples
    bool stereo);

static inline void s_ReadSoundData(Stream* in, int datasize, UByte* pdata, unsigned sample16bit, unsigned format)
{
    if (sample16bit)
    {
        for (int i = 0; i < datasize >> 1; i++)
            *((UInt16*) &pdata[i*2]) = Alg::ByteUtil::LEToSystem(in->ReadU16());
        if (datasize & 1)
            pdata[datasize-1] = in->ReadU8();
    }
    else
    {
        for (int i = 0; i < datasize; i++)
        {
            if (format == 0 || format == 3)
                pdata[i] = in->ReadU8() ^ 128;
            else
                pdata[i] = in->ReadU8();
        }
    }
}

Sound::SoundData* GFx_ReadSoundData(LoadProcess* p, UInt16  characterId)
{
    Stream* in = p->GetStream();

    const int SampleRateTable[] = { 5512, 11025, 22050, 44100 };

    unsigned format = in->ReadUInt(4);
    unsigned SampleRate = SampleRateTable[in->ReadUInt(2)];   // multiples of 5512.5
    unsigned Sample16bit = in->ReadUInt1();
    unsigned stereo = in->ReadUInt1();
    unsigned sampleCount = in->ReadU32();
    unsigned sf = (Sample16bit ? Sound::SoundData::Sample_16 : Sound::SoundData::Sample_8) |
                  (stereo      ? Sound::SoundData::Sample_Stereo : 0);
    unsigned InitialSeek = 0;
    unsigned adpcmCodeSize = 2;

    switch (format)
    {
    case 0:
    case 3:
        sf |= Sound::SoundData::Sample_PCM;
        break;
    case 1:
        sf |= Sound::SoundData::Sample_PCM;
//      sf |= Sound::SoundData::Sample_ADPCM;
        adpcmCodeSize = in->ReadUInt(2);
        break;
    case 2:
        sf |= Sound::SoundData::Sample_MP3;
        InitialSeek = in->ReadU16();
        break;
    }

    p->LogParse("define sound: ch=%d, format=%d, rate=%d, 16=%d, stereo=%d, ct=%d\n",
                 characterId, format, SampleRate, int(Sample16bit), int(stereo), sampleCount);

    int datasize = in->GetTagEndPosition() - in->Tell();
    Sound::SoundData* sound = NULL;
    if (format == 1)
    {
        sound = SF_NEW Sound::SoundData(sf, SampleRate, sampleCount, (stereo? sampleCount*4 : sampleCount*2));
        UByte* pdata = sound->GetData();
        GFx_AdpcmExpand(pdata, in, adpcmCodeSize+2, sampleCount, stereo != 0);
    }
    else
    {
        sound = SF_NEW Sound::SoundData(sf, SampleRate, sampleCount, datasize);
        sound->SetSeekSample(InitialSeek);

        UByte* pdata = sound->GetData();
        s_ReadSoundData(in, datasize, pdata, Sample16bit, format);
    }

    return sound;
}

//////////////////////////////////////////////////////////////////////////
//

SoundInfo::SoundInfo() {}

void SoundInfo::Play(InteractiveObject *pchar)
{
    if (!pchar)
        return;
    Sound::SoundRenderer *pplayer = pchar->GetMovieImpl()->GetSoundRenderer();
    if (!pplayer)
        return;

    SoundResource* psound = NULL;
    Resource* pres = pSample.GetResource(&pchar->GetResourceMovieDef()->GetResourceBinding());
    if (pres && pres->GetResourceType() == Resource::RT_SoundSample)
        psound = (SoundResource*) pres;

    if (!psound)
        return;

    Sprite* psprite = NULL;
    if (pchar->IsSprite())
        psprite = pchar->CharToSprite();
    else
    {
        if (pchar->GetParent())
        {
            SF_ASSERT(pchar->GetParent()->IsSprite());
            psprite = pchar->GetParent()->CharToSprite();
        }
    }
    if (!psprite) return;

    Ptr<Sound::SoundSample> psample = psound->GetSoundInfo()->GetSoundSample(pplayer);
    if (Style.StopPlayback)
    {
        pchar->GetMovieImpl()->GetRootMovie(NULL)->StopActiveSounds(psound);
        //psprite->StopActiveSounds(psound);
    }
    else
    {
        if (Style.NoMultiple && psound->IsPlaying())
        {
            return;
        }
        Ptr<Sound::SoundChannel> pchan = *pplayer->PlaySample(psample, true);
        if (!pchan)
            return;
        pchan->SetVolume(psprite->GetRealSoundVolume());
        if (Style.Envelopes.GetSize() > 0)
        {
            Array<Sound::SoundChannel::Transform> transforms;
            for(size_t i = 0; i < Style.Envelopes.GetSize(); ++i)
            {
                const SoundEnvelope& envelope = Style.Envelopes[i];
                Sound::SoundChannel::Transform transform;
                unsigned pos = envelope.Mark44/44;
                transform.Position = pos/1000.0f;
                transform.LeftVolume = (envelope.Level0 * 100/32768)/100.0f;
                transform.RightVolume = (envelope.Level1 * 100/32768)/100.0f;
                transforms.PushBack(transform);
            }
            pchan->SetTransforms(transforms);
        }

        float inpoint  = Style.HasInPoint? Style.InPoint * 1.0f / psound->GetSoundInfo()->GetRate() : 0.0f;
        float outpoint = Style.HasOutPoint? Style.OutPoint * 1.0f / psound->GetSoundInfo()->GetRate() : 0.0f;
        if (Style.LoopCount > 0)
            pchan->Loop(Style.LoopCount, inpoint, outpoint);
        else if (Style.HasInPoint || Style.HasOutPoint)
            pchan->Loop(1, inpoint, outpoint);

        pchan->Pause(false);
        psprite->AddActiveSound(pchan, NULL, psound);
    }
}

void ButtonSoundDefImpl::Play(InteractiveObject *pchar, int buttonIdx)
{
    if (buttonIdx >= 0)
    {
        SoundInfo& bs = ButtonSounds[buttonIdx];
        bs.Play(pchar);
    }
}

//////////////////////////////////////////////////////////////////////////
//

SoundStreamDefImpl::SoundStreamDefImpl() : StartFrame(0), LastFrame(0)
{
}

SoundStreamDefImpl::~SoundStreamDefImpl()
{
}

void SoundStreamDefImpl::ReadHeadTag(Stream* in)
{
    unsigned reserved = in->ReadUInt(4);
    SF_UNUSED(reserved);
    const int  SampleRateTable[] = { 5512, 11025, 22050, 44100 };

    PlaybackSoundRate = SampleRateTable[in->ReadUInt(2)];
    PlaybackSound16bit = in->ReadUInt1();
    PlaybackStereo = in->ReadUInt1();
    SoundCompression = in->ReadUInt(4);
    StreamSoundRate = SampleRateTable[in->ReadUInt(2)];
    StreamSound16bit = in->ReadUInt1();
    StreamStereo = in->ReadUInt1();
    StreamSampleCount = in->ReadU16();
    LatencySeek = 0;
    if (SoundCompression == 2)
        LatencySeek = in->ReadU16();

    in->LogParse("  PlaybackSoundRate: %d\n  PlaybackSound16bit: %d\n  PlaybackStereo: %d\n  SoundCompression: %d\n",
        PlaybackSoundRate, int(PlaybackSound16bit), int(PlaybackStereo), int(SoundCompression));
    in->LogParse("  StreamSoundRate: %d\n  StreamSound16bit: %d\n  StreamStereo: %d\n  StreamSampleCount: %d\n  LatencySeek: %d\n",
        StreamSoundRate, int(StreamSound16bit), int(StreamStereo), int(StreamSampleCount), int(LatencySeek));
}

bool SoundStreamDefImpl::ProcessSwfFrame(MovieImpl* proot, unsigned frame, Sprite* pchar)
{
    SF_ASSERT(proot && pchar);
    AudioBase* paudiostate = proot->pAudio;
    Sound::SoundRenderer* prenderer = proot->pSoundRenderer;
    float frameTime = proot->FrameTime;
    unsigned* forceFrameCatchUp = &proot->ForceFrameCatchUp;
    SF_ASSERT(paudiostate && prenderer && forceFrameCatchUp);
    Ptr<Sound::SoundChannel> pchan = pchar->GetStreamingSound();

    if (pSoundInfo && paudiostate && prenderer)
    {
        if (frame >= StartFrame && frame < LastFrame)
        {
            if (!pchan || !pchan->IsPlaying())
            {
                pchan = *prenderer->PlaySample(pSoundInfo->GetSoundSample(prenderer),true);
                if (!pchan)
                {
                    // cannot create sound channel
                    return false;
                }
                else
                {
                    float frame_time = (frame - StartFrame) * frameTime;
                    pchan->SetPosition(frame_time);
                    pchar->SetStreamingSound(pchan);
                    pchan->Pause(false);
                }
            }
            else
            {
                if (paudiostate->GetSyncType() != Audio::NoSync)
                {
                    if (frame % paudiostate->GetCheckFrameInterval() == 0)
                    {
                        float sound_time = pchan->GetPosition();
                        float frame_time = (frame - StartFrame) * frameTime;
                        if (sound_time - frame_time > paudiostate->GetMaxTimeDifference())
                        {
                            if (paudiostate->GetSyncType() == Audio::VideoMaster)
                            {
                                pchan->SetPosition(frame_time);
                            }
                            else
                            {
                                unsigned frameCatchUp = (unsigned)((sound_time-frame_time)/frameTime);
                                if (!*forceFrameCatchUp && frameCatchUp > 0)
                                    *forceFrameCatchUp = frameCatchUp+1;
                            }
                        } 
                        else if (frame_time - sound_time > paudiostate->GetMaxTimeDifference())
                        {
                            pchan->SetPosition(frame_time);
                        }
                    }
                }
            }
        }
        else if (pchan && pchan->IsPlaying())
        {
            pchar->SetStreamingSound(NULL);
        }
    }
    return true;
}

void SoundStreamDefImpl::ReadBlockTag(LoadProcess* p)
{
    unsigned datasize = 0;
    UInt16 sampleCount = 0;
    SInt16 seekSamples = 0;

    Stream* in = p->GetStream();

    datasize = in->GetTagEndPosition() - in->Tell();
    p->LogParse("SoundStreamBlock: size=%d", datasize);

    if (datasize == 0)
    {
        return;
    }

    if (!pSoundInfo)
    {
        unsigned sf = (StreamSound16bit ? Sound::SoundData::Sample_16 : Sound::SoundData::Sample_8) |
                      (StreamStereo     ? Sound::SoundData::Sample_Stereo : 0);

        switch (SoundCompression)
        {
        case 0:
        case 3:
        case 1:
            sf |= Sound::SoundData::Sample_PCM;
//          sf |= Sound::SoundData::Sample_ADPCM;
            break;
        case 2:
            sf |= Sound::SoundData::Sample_MP3;
            break;
        }
        StartFrame = p->GetLoadingFrameForStreamSound();
        Ptr<Sound::AppendableSoundData> sound = *SF_NEW Sound::AppendableSoundData(sf, StreamSoundRate);
        sound->SetSeekSample(LatencySeek);
        pSoundInfo = *SF_NEW Sound::SoundInfo(sound);
    }

    LastFrame = p->GetLoadingFrameForStreamSound();
    unsigned adpcmCodeSize = 2;
    if (SoundCompression == 2) 
    {
        sampleCount = in->ReadU16();
        seekSamples = in->ReadS16();
        datasize -= 4;
    }
    else if (SoundCompression == 1)
    {
        adpcmCodeSize = in->ReadUInt(2);
        sampleCount = (UInt16)StreamSampleCount;
    } 
    else 
        sampleCount = (UInt16)StreamSampleCount;

    if (datasize > 0)
    {
        SF_ASSERT(pSoundInfo->GetSound()->IsStreamSample());
        Sound::AppendableSoundData* sd = (Sound::AppendableSoundData*)pSoundInfo->GetSound();
        UByte* pdata = NULL;
        unsigned dsize = datasize;
        if (SoundCompression != 1 )
        {
            pdata = sd->LockDataForAppend(sampleCount, dsize);
            SF_ASSERT(pdata);
            s_ReadSoundData(in, dsize, pdata, StreamSound16bit, SoundCompression);           
            sd->UnlockData();
        }
        else
        {
            dsize = StreamStereo? sampleCount * 4 : sampleCount * 2;
            pdata = sd->LockDataForAppend(sampleCount, dsize);
            SF_ASSERT(pdata);
            GFx_AdpcmExpand(pdata, in, adpcmCodeSize+2, sampleCount, StreamStereo > 0);
            sd->UnlockData();
        }
    }
    else
    {
        SF_ASSERT(/*seekSamples == 0 &&*/ sampleCount == 0);
    }
}   

void SoundStyle::Read(Stream* in)
{
    InPoint = OutPoint = LoopCount = 0;
    in->ReadUInt(2);    // skip reserved bits.
    StopPlayback = in->ReadUInt(1) ? true : false;
    NoMultiple   = in->ReadUInt(1) ? true : false;
    HasEnvelope  = in->ReadUInt(1) ? true : false;
    HasLoops     = in->ReadUInt(1) ? true : false;
    HasOutPoint  = in->ReadUInt(1) ? true : false;
    HasInPoint   = in->ReadUInt(1) ? true : false;
    if (HasInPoint) InPoint = in->ReadU32();
    if (HasOutPoint) OutPoint = in->ReadU32();
    if (HasLoops) LoopCount = in->ReadU16();
    if (HasEnvelope)
    {
        int nPoints = in->ReadU8();
        Envelopes.Resize(nPoints);
        for (int i=0; i < nPoints; i++)
        {
            Envelopes[i].Mark44 = in->ReadU32();
            Envelopes[i].Level0 = in->ReadU16();
            Envelopes[i].Level1 = in->ReadU16();
        }
    }
    else
    {
        Envelopes.Resize(0);
    }

    // Log output
    in->LogParse("  HasEnvelope = %d\n", (int)HasEnvelope);
    in->LogParse("  HasLoops = %d\n", (int)HasLoops);
    in->LogParse("  HasOutPoint = %d\n", (int)HasOutPoint);
    in->LogParse("  HasInPoint = %d\n", (int)HasInPoint);
    in->LogParse("  InPoint = %d\n", (int)InPoint);
    in->LogParse("  OutPoint = %d\n", (int)OutPoint);
    in->LogParse("  LoopCount = %d\n", (int)LoopCount);
    in->LogParse("  envelope size = %d\n", (int)Envelopes.GetSize());       

}

//////////////////////////////////////////////////////////////////////////
//

//
// ADPCM
//

// Data from Alexis' SWF reference
static int GFx_IndexUpdateTable2bits[2]  = { -1,  2 };
static int GFx_IndexUpdateTable3bits[4]  = { -1, -1,  2,  4 };
static int GFx_IndexUpdateTable4bits[8]  = { -1, -1, -1, -1,  2,  4,  6,  8 };
static int GFx_IndexUpdateTable5bits[16] = { -1, -1, -1, -1, -1, -1, -1, -1, 1,  2,  4,  6,  8, 10, 13, 16 };

static int* GFx_IndexUpdateTables[4] = {
    GFx_IndexUpdateTable2bits,
    GFx_IndexUpdateTable3bits,
    GFx_IndexUpdateTable4bits,
    GFx_IndexUpdateTable5bits,
};

//
// Data from Jansen. http://homepages.Cwi.Nl/~jack/
// Check out his Dutch retro punk songs, heh heh :)
const int GFX_STEPSIZE_CT = 89;
static int GFx_SoundStepSize[GFX_STEPSIZE_CT] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

//
// Algo from http://www.Circuitcellar.Com/pastissues/articles/richey110/text.Htm and also Jansen.
// Here's another reference: http://www.Geocities.Com/SiliconValley/8682/aud3.Txt
// Original IMA spec doesn't seem to be on the web :(


// @@ lots of macros here! It seems that VC6 can't correctly
// handle integer template args, although it's happy to compile them?!

//  void GFX_DO_SAMPLE(int N_bits, int& sample, int& GFx_SoundStepSizeIndex, int RawCode)
#define GFX_DO_SAMPLE(N_bits, sample, GFx_SoundStepSizeIndex, RawCode)              \
{                                                                                   \
    SF_ASSERT(RawCode >= 0 && RawCode < (1 << N_bits));                             \
                                                                                    \
    static const int HI_BIT = (1 << (N_bits - 1));                                  \
    int* GFx_IndexUpdateTable = GFx_IndexUpdateTables[N_bits - 2];                  \
                                                                                    \
    /* Core of ADPCM. */                                                            \
                                                                                    \
    int CodeMag = RawCode & (HI_BIT - 1);                                           \
    bool CodeSignBit = (RawCode & HI_BIT) ? 1 : 0;                                  \
    int mag = (CodeMag << 1) + 1;   /* shift in LSB (they do this so that pos & neg zero are different)*/ \
                                                                                    \
    int stepsize = GFx_SoundStepSize[GFx_SoundStepSizeIndex];                       \
                                                                                    \
    /* Compute the new sample.  It's the predicted value             */             \
    /* (i.E. the previous value), plus a delta.  The delta           */             \
    /* comes from the code times the stepsize.  going for            */             \
    /* something like: delta = stepsize * (code * 2 + 1) >> CodeBits */             \
    int delta = (stepsize * mag) >> (N_bits - 1);                                   \
    if (CodeSignBit) delta = -delta;                                                \
                                                                                    \
    sample += delta;                                                                \
    sample = Alg::Clamp<int>(sample, -32768, 32767);                                \
                                                                                    \
    /* Update our stepsize index.  Use a lookup table. */                           \
    GFx_SoundStepSizeIndex += GFx_IndexUpdateTable[CodeMag];                        \
    GFx_SoundStepSizeIndex = Alg::Clamp<int>(GFx_SoundStepSizeIndex, 0, GFX_STEPSIZE_CT - 1); \
}

//  void GFX_DO_MONO_BLOCK(SInt16** OutData, int N_bits, int sampleCount, Stream* in, int sample, int GFx_SoundStepSizeIndex)
#define GFX_DO_MONO_BLOCK(OutData, N_bits, sampleCount, in, sample, GFx_SoundStepSizeIndex) \
{                                                                                   \
    /* First sample doesn't need to be decompressed. */                             \
    sampleCount--;                                                                  \
    *(*OutData)++ = Alg::ByteUtil::SystemToLE((SInt16) sample);                     \
                                                                                    \
    while (sampleCount--)                                                           \
    {                                                                               \
        int RawCode = in->ReadUInt(N_bits);                                         \
        GFX_DO_SAMPLE(N_bits, sample, GFx_SoundStepSizeIndex, RawCode);             \
        *(*OutData)++ = Alg::ByteUtil::SystemToLE((UInt16)sample);                  \
    }                                                                               \
}

// Uncompress 4096 stereo sample pairs of ADPCM.
// void DoStereoBlock(SInt16** OutData, int N_bits, int sampleCount, Stream* in, int LeftSample, int LeftGFx_SoundStepSizeIndex, int RightSample, int RightGFx_SoundStepSizeIndex)
#define GFX_DO_STEREO_BLOCK(OutData, N_bits, sampleCount, in, LeftSample, LeftGFx_SoundStepSizeIndex, RightSample, RightGFx_SoundStepSizeIndex) \
{                                                                                   \
    sampleCount--;                                                                  \
    *(*OutData)++ = Alg::ByteUtil::SystemToLE((SInt16) LeftSample);                 \
    *(*OutData)++ = Alg::ByteUtil::SystemToLE((SInt16) RightSample);                \
                                                                                    \
    while (sampleCount--)                                                           \
    {                                                                               \
        int LeftRawCode = in->ReadUInt(N_bits);                                     \
        GFX_DO_SAMPLE(N_bits, LeftSample, LeftGFx_SoundStepSizeIndex, LeftRawCode); \
        *(*OutData)++ = Alg::ByteUtil::SystemToLE((SInt16)LeftSample);              \
                                                                                    \
        int RightRawCode = in->ReadUInt(N_bits);                                    \
        GFX_DO_SAMPLE(N_bits, RightSample, RightGFx_SoundStepSizeIndex, RightRawCode); \
        *(*OutData)++ = Alg::ByteUtil::SystemToLE((SInt16)RightSample);             \
    }                                                                               \
}

//////////////////////////////////////////////////////////////////////////
//

#define DO_MONO(n)   GFX_DO_MONO_BLOCK(&OutData, n, SamplesThisBlock, in, sample, GFx_SoundStepSizeIndex)
#define DO_STEREO(n) GFX_DO_STEREO_BLOCK(&OutData, n, SamplesThisBlock, in, sample, GFx_SoundStepSizeIndex, RightSample, RightGFx_SoundStepSizeIndex)

// Utility function: uncompress ADPCM data from in Stream to OutData[].
// The output buffer must Have (sampleCount*2) bytes for mono, Or (sampleCount*4) bytes for stereo.
void GFx_AdpcmExpand(
    void* OutDataVoid,  // ALWAYS IN LITTLE ENDIAN byte order
    Stream* in,
    int N_bits,         // 2 to 5 bits
    int sampleCount,    // in stereo, this is number of *pairs* of samples
    bool stereo)
{
    SInt16* OutData = (SInt16*) OutDataVoid;

    // Read header.
    //int N_bits = in->ReadUInt(2) + 2;   // 2 to 5 bits

    while (sampleCount)
    {
        // Read initial sample & index values.
        int sample = in->ReadSInt(16);

        int GFx_SoundStepSizeIndex = in->ReadUInt(6);
        SF_ASSERT(GFX_STEPSIZE_CT >= (1 << 6));   // ensure we don't need to clamp.

        int SamplesThisBlock = Alg::Min<int>(sampleCount, 4096);
        sampleCount -= SamplesThisBlock;

        if (stereo == false)
        {
            switch (N_bits)
            {
                default: SF_ASSERT(0); break;
                case 2: DO_MONO(2); break;
                case 3: DO_MONO(3); break;
                case 4: DO_MONO(4); break;
                case 5: DO_MONO(5); break;
            }
        }
        else {
            // Got values for left channel; now get initial sample
            // & index for right channel.
            int RightSample = in->ReadSInt(16);

            int RightGFx_SoundStepSizeIndex = in->ReadUInt(6);
            SF_ASSERT(GFX_STEPSIZE_CT >= (1 << 6));   // ensure we don't need to clamp.

            switch (N_bits)
            {
                default: SF_ASSERT(0); break;
                case 2: DO_STEREO(2); break;
                case 3: DO_STEREO(3); break;
                case 4: DO_STEREO(4); break;
                case 5: DO_STEREO(5); break;
            }
        }
    }
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND
