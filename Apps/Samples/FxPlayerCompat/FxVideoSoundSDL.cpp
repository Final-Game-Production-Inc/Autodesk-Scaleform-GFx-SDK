/**************************************************************************

Filename    :   FxVideoSoundSDL.cpp
Content     :   Sample system based allocator class that can be
                installed in FxPlayer to show overriding allocators.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GAtomic.h"
#include "FxVideoSoundSDL.h"


#include "SDL.h"
#include "SDL_audio.h"

/*
#include "GSoundPlayer.h"
class GSoundChannelSDL : public GSoundChannel
{
public:
    virtual GSoundPlayer*  GetSoundPlayer() const                              = 0;
    virtual GSoundSample*  GetSample() const                                   = 0;

    virtual void           Stop()                                              = 0;
    virtual void           Pause(bool pause)                                   = 0;
    virtual bool           IsPlaying() const                                   = 0;
    virtual void           SetPosition(UInt samples)                           = 0;
    virtual void           SetPositionF(Float seconds)                         = 0;
    virtual UInt           GetPosition()                                       = 0;
    virtual Float          GetPositionF()                                      = 0;
    virtual void           Loop(SInt count, UInt start = 0, UInt end = 0)      = 0;
    virtual void           LoopF(SInt count, Float start = 0, Float end = 0)   = 0;
    virtual SInt           GetVolume()                                         = 0;
    virtual void           SetVolume(SInt volume)                              = 0;
};

*/


//////////////////////////////////////////////////////////////////////////
// GFxVideoSoundInterfaceSDL

#define MAX_CHENNALS 6
#define SAMPL_BUFF_SIZE 16384

class GFxVideoSoundCreatorSDLImpl;
class GFxVideoSoundInterfaceSDL : public GFxVideoSoundInterface
{
public:
    GFxVideoSoundInterfaceSDL(GFxVideoSoundCreatorSDLImpl* parent);
    virtual ~GFxVideoSoundInterfaceSDL();

    virtual bool CreateOutput(UInt32 channel, UInt32 samplerate);
    virtual void DestroyOutput();

    virtual void SetCallbackPCMDataFunc(PCMDataFunc func, Handle obj);
    virtual void Start();
    virtual void Stop();
    virtual Status GetStatus();

    virtual void Pause(bool sw);
    virtual void GetTime(UInt64* count, UInt64* unit);

    virtual GSoundChannel* GetSoundChannel();


    void    GetSamples(UByte* data, UInt datalen);

    GFxVideoSoundCreatorSDLImpl* pParent;
    UInt32				OutChannels;
    PCMDataFunc         UsrFunc;
    Handle				UsrObj;
    Status	            Stat;
    UInt64				TotalGotSamples;
    UInt32				SampleRate;
    GPtr<GSoundChannel> pChannel;
    SInt16              SampleBuffers[MAX_CHENNALS][SAMPL_BUFF_SIZE];
    bool                Paused;

};

class GFxVideoSoundCreatorSDLImpl : public GFxVideoSoundCreator
{
public:
    GFxVideoSoundCreatorSDLImpl();
    ~GFxVideoSoundCreatorSDLImpl();

    virtual GFxVideoSoundInterface* Create();

    static void mixaudio(void *obj, Uint8* stream, int len);

    void AttachStream(GFxVideoSoundInterfaceSDL*);
    void DetachStream(GFxVideoSoundInterfaceSDL*);
    void DetachStream_NoLock(GFxVideoSoundInterfaceSDL*);

    GLock Lock;
    GTL::garray<GPtr<GFxVideoSoundInterfaceSDL> > pInterfaces;

    SDL_AudioSpec fmt;
};

GFxVideoSoundCreatorSDLImpl::GFxVideoSoundCreatorSDLImpl()
{
    /* Set 16-bit stereo audio at 22Khz */
    fmt.freq = 44100;
    fmt.format = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples = 512; /* A good value for games */
    fmt.callback = GFxVideoSoundCreatorSDLImpl::mixaudio;
    fmt.userdata = this;

    SDL_OpenAudio(&fmt, NULL);
}

GFxVideoSoundCreatorSDLImpl::~GFxVideoSoundCreatorSDLImpl()
{
    GLock::Locker lock(&Lock);
    for(size_t i = 0; i < pInterfaces.size(); ++i)
    {
        DetachStream_NoLock(pInterfaces[i]);
    }
    SDL_CloseAudio();//closes the audio and setting the fmt to null
}

GFxVideoSoundInterface* GFxVideoSoundCreatorSDLImpl::Create()
{
    return new GFxVideoSoundInterfaceSDL(this);
}


void GFxVideoSoundCreatorSDLImpl::AttachStream(GFxVideoSoundInterfaceSDL* pstream)
{
    GLock::Locker lock(&Lock);
    pInterfaces.push_back(pstream);
    if (pInterfaces.size() == 1)
        SDL_PauseAudio(0);
}
void GFxVideoSoundCreatorSDLImpl::DetachStream(GFxVideoSoundInterfaceSDL* pstream)
{
    GLock::Locker lock(&Lock);
    DetachStream_NoLock(pstream);
}
void GFxVideoSoundCreatorSDLImpl::DetachStream_NoLock(GFxVideoSoundInterfaceSDL* pstream)
{
    GLock::Locker lock(&Lock);
    if (pInterfaces.size() == 1)
        SDL_PauseAudio(1);
    for(size_t i = 0; i < pInterfaces.size(); i++)
    {
        if (pInterfaces[i].GetPtr() == pstream)
        {
            pInterfaces.remove(i);
            pstream->pParent = NULL;
            pstream->Stat = GFxVideoSoundInterface::SoundStop;
        }
    }
}

//static 
void GFxVideoSoundCreatorSDLImpl::mixaudio(void *obj, Uint8* stream, int len)
{
    GFxVideoSoundCreatorSDLImpl* pthis = (GFxVideoSoundCreatorSDLImpl*)obj;
    GLock::Locker lock(&pthis->Lock);
    memset(stream, 0, len);
    Uint8* mix_buf = (Uint8*)GALLOC(len);
    for(size_t i = 0; i < pthis->pInterfaces.size(); i++)
    {
        GFxVideoSoundInterfaceSDL* piface = pthis->pInterfaces[i];
        if (piface->Stat == GFxVideoSoundInterface::SoundExec && !piface->Paused)
        {
            piface->GetSamples(mix_buf,len);
            SDL_MixAudio(stream, mix_buf, len, SDL_MIX_MAXVOLUME);
        }
    }
    GFREE(mix_buf);
}

//////////////////////////////////////////////////////////////////////////

GFxVideoSoundInterfaceSDL::GFxVideoSoundInterfaceSDL(GFxVideoSoundCreatorSDLImpl* pparent)
    : pParent(pparent), Paused(false)
{
}
GFxVideoSoundInterfaceSDL::~GFxVideoSoundInterfaceSDL()
{
}


bool GFxVideoSoundInterfaceSDL::CreateOutput(UInt32 channels, UInt32 samplerate)
{
    if (channels == 0)
        return false;

    OutChannels = channels;
    SampleRate = samplerate;
    Stat = SoundStop;
    UsrFunc = NULL;
    UsrObj = NULL;
    return true;
}


void GFxVideoSoundInterfaceSDL::DestroyOutput()
{
    OutChannels = 0;
}


void GFxVideoSoundInterfaceSDL::SetCallbackPCMDataFunc(PCMDataFunc func, Handle obj)
{
    UsrFunc = func;
    UsrObj = obj;
}

void GFxVideoSoundInterfaceSDL::Start(void)
{
    if (pParent)
    {
        pParent->AttachStream(this);
        Stat = SoundExec;
        Paused = false;
    }
    TotalGotSamples = 0;
}


void GFxVideoSoundInterfaceSDL::Stop(void)
{

    //printf("GFxCriMvSoundLoc::Stop(): total samples: %d\n", this->total_got_nsmpls);
    if (pParent)
        pParent->DetachStream(this);
    Stat = SoundStop;
    TotalGotSamples = 0;
}

void GFxVideoSoundInterfaceSDL::Pause(bool sw)
{
    Paused = sw;
}

GFxVideoSoundInterface::Status GFxVideoSoundInterfaceSDL::GetStatus(void)
{
    return Stat;
}

void GFxVideoSoundInterfaceSDL::GetTime(UInt64* count, UInt64* unit)
{	
    if ( TotalGotSamples == 0 ) {
        *count = 0;
        *unit  = 1000;
    } else {
        *count = TotalGotSamples * 1000 / SampleRate;
        *unit = 1000;
    }
}

GSoundChannel* GFxVideoSoundInterfaceSDL::GetSoundChannel()
{
    return pChannel;
}
/*
inline Sint16 PcmfToPcm16(Float32 pcmf) {
if ( pcmf > -1.0f && pcmf <=1.0f ) {
return (Sint16)(pcmf*32767.0f);
} else if ( pcmf > 1.0f ) {
return  32767;
} else {
return -32768;
}
}
*/
// static
void GFxVideoSoundInterfaceSDL::GetSamples(UByte* data, UInt datalen)
{
    UInt32 got_nsmpl = 0;
    UInt got_bytes = 0;
    UInt32 nsmpl = datalen * 8 /16 / OutChannels;
    int nch = MAX_CHENNALS;
    SInt16 *samples[MAX_CHENNALS];

    signed short *stereo16bitbuffer = (signed short *)data;

    if (OutChannels == 1) 
    {
        // MONO 
        if (UsrFunc != NULL) 
            got_nsmpl = UsrFunc(UsrObj, 1, &stereo16bitbuffer, nsmpl);
    } 
    else 
    {
        // STEREO or 5.1ch 
        if (UsrFunc != NULL) 
        {
            for(int i = 0; i < nch; ++i)
                samples[i] = SampleBuffers[i];
            got_nsmpl = UsrFunc(UsrObj, nch, samples, nsmpl);
            for (unsigned int count=0; count < got_nsmpl; count++)
            {
                for( unsigned int i = 0; i < OutChannels; ++i)
                    *stereo16bitbuffer++ = samples[i][count]; 
            }
        }
    }
    memset(stereo16bitbuffer,0,nsmpl-got_nsmpl);

    TotalGotSamples += got_nsmpl;
/*
    got_bytes = got_nsmpl * 16 * OutChannels / 8;

    return got_bytes;
*/
}


//////////////////////////////////////////////////////////////////////////

GFxVideoSoundCreatorSDL::GFxVideoSoundCreatorSDL()
{
    pImpl = new GFxVideoSoundCreatorSDLImpl;
}
GFxVideoSoundCreatorSDL::~GFxVideoSoundCreatorSDL()
{
    pImpl->Release();
}

GFxVideoSoundInterface* GFxVideoSoundCreatorSDL::Create()
{
    return pImpl->Create();
}
