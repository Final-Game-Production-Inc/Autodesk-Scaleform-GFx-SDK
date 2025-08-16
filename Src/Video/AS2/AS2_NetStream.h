/**************************************************************************

Filename    :   AS2_NetStream.h
Content     :   GFx video: AS2 NetStream and VideoProviderNetStream classes
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_AS2NETSTREAM_H
#define INC_GFX_VIDEO_AS2NETSTREAM_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/GFx_MediaInterfaces.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "Video/Video_Video.h"
#include "Video/Video_VideoImage.h"
#include "Video/AS2/AS2_VideoObject.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class NetStream;
class NetStreamProto;
class NetStreamCtorFunction;

class VideoProviderNetStream;

//////////////////////////////////////////////////////////////////////////
//

class NetStream : public Object
{
    friend class VideoProviderNetStream;

protected:    
    NetStream(ASStringContext *psc, Object* pprototype);
    void commonInit(Environment* penv);

public:
    NetStream(Environment* penv);
    ~NetStream();

    ObjectType GetObjectType() const { return Object_NetStream; }

    void OpenUrl(const char* url, Environment* penv);
    void Stop();
    void Pause();
    void Pause(bool);
    void Seek(Number pos, Environment* env);

    void SetBufferTime(Number time);
    void SetNumberOfFramePools(Number pools);
    void SetReloadThresholdTime(Number time);
    void SetOpenURLTimeout(Number timeout);
    void SetSubtitleChannel(Number channel);

    void SetAudioTarget(Sprite*);

    virtual bool GetMember(Environment *penv, const ASString& name, Value* val);
    virtual bool SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags);

#ifdef GFX_ENABLE_SOUND
    Sound::SoundChannel* GetSoundChannel();

    void SetSoundPosition(Array<Sound::SoundChannel::Vector>& varr);
    void SetSoundOrientation(Array<Sound::SoundChannel::Vector>& varr);
    void SetSoundVelocity(Array<Sound::SoundChannel::Vector>& varr);
    void ApplySoundSpatial();
#endif

    void NotifyStopPlaying();

    VideoProviderNetStream* GetVideoProvider();
    Video::VideoPlayer*     GetVideoPlayer();

#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;

    template <class Functor>
    void ForEachChild_GC(Collector* prcc) const
    {
        Object::template ForEachChild_GC<Functor>(prcc);
        OnSubtitleCallback.template ForEachChild_GC<Functor>(prcc);
        OnMetaDataCallback.template ForEachChild_GC<Functor>(prcc);
        OnStatusCallback.template ForEachChild_GC<Functor>(prcc);
        OnCuePointCallback.template ForEachChild_GC<Functor>(prcc);
    }
    virtual void ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
    {
        ASRefCountBaseType::CallForEachChild<NetStream>(prcc, operation);
    }
    virtual void Finalize_GC();
#endif // GFX_AS_ENABLE_GC

private:
    void Finalize();

    Ptr<VideoProviderNetStream> pVideoProvider;
    bool        Paused;
    FunctionRef OnSubtitleCallback;
    FunctionRef OnMetaDataCallback;
    FunctionRef OnStatusCallback;
    FunctionRef OnCuePointCallback;
};

//////////////////////////////////////////////////////////////////////////
//

class NetStreamProto : public Prototype<NetStream>
{
public:
    NetStreamProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void Close(const FnCall& fn);
    static void Pause(const FnCall& fn);
    static void Play(const FnCall& fn);
    static void Seek(const FnCall& fn);
    static void SetBufferTime(const FnCall& fn);
    static void SetNumberOfFramePools(const FnCall& fn);
    static void SetReloadThresholdTime(const FnCall& fn);
    static void SetOpenURLTimeout(const FnCall& fn);
#ifdef GFX_ENABLE_SOUND
    static void SetSoundSpatial(const FnCall& fn);
#endif

    void AddActiveNetstream()    { ActiveNetstreamCounter++; }
    void RemoveActiveNetstream() { SF_ASSERT(ActiveNetstreamCounter > 0);
                                   ActiveNetstreamCounter--; }
private:
    int  ActiveNetstreamCounter;  // This counter is needed for minimizing of number
                                  // off calls to the GC release method
};

//////////////////////////////////////////////////////////////////////////
//

class NetStreamCtorFunction : public CFunctionObject
{
public:
    NetStreamCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment *penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

//////////////////////////////////////////////////////////////////////////
//

class VideoProviderNetStream : public Video::VideoProvider
{
public:
    VideoProviderNetStream(NetStream* pnetstream);
    ~VideoProviderNetStream();

    virtual Video::VideoImage* GetTexture(int* width, int* height);

    virtual void  Advance();
    virtual void  Pause(bool pause);
    virtual void  Close();

    virtual bool  IsActive();
    virtual float GetFrameTime() { return FrameTime; }

    virtual void                   RegisterVideoCharacter(Video::VideoCharacter* pvideo);
    virtual Video::VideoCharacter* RemoveFirstVideoCharacter();

    void   OpenUrl(const char* url, Environment *penv);
    void   Stop();

    UInt32 GetPosition();
    void   Seek(float pos, MovieImpl* proot);
    void   SetLoopFlag(bool flag);
    void   SetSwfSync(bool flag, Sprite* ptarget);

    void   SetAudioTrack(int track);
    void   SetSubAudioTrack(int track);
    void   SetExtraAudioTrack(int track);
    void   ReplaceCenterVoice(int track);

    void   SetSubtitleChannel(int channel);
    int    GetSubtitleChannel();

    void   SetBufferTime(float time);
    void   SetNumberOfFramePools(unsigned pools);
    void   SetReloadThresholdTime(float time);
    void   SetOpenURLTimeout(float timeout);

#ifdef GFX_ENABLE_SOUND
    void   SetAudioTarget(Sprite*);

    void   SetSoundSpatial(unsigned idx, Array<Sound::SoundChannel::Vector>& varr);
    void   ApplySoundSpatial();

    void   SetSoundPosition(Array<Sound::SoundChannel::Vector>& varr)    { SetSoundSpatial(0, varr); }
    void   SetSoundOrientation(Array<Sound::SoundChannel::Vector>& varr) { SetSoundSpatial(1, varr); }
    void   SetSoundVelocity(Array<Sound::SoundChannel::Vector>& varr)    { SetSoundSpatial(2, varr); }
#endif

    void   NotifyNetStreamRemoved() { pNetStream = NULL; }
    void   SendNotification(const char* note, bool error);
    void   SendMetaInformation();
    void   SendSubtitleInformation();
    void   SendCuePoints(const Array<Video::VideoPlayer::CuePoint>& cue_points);

    Video::VideoPlayer* GetPlayer() { return pVideoPlayer; }
    const Video::VideoPlayer::VideoInfo& GetVideoInfo() const { return VInfo; }

private:
    void   Init();
#ifdef GFX_ENABLE_SOUND
    void   InitSoundChannel();
#endif
    void   NotifyVideoCharacters();

    NetStream*                             pNetStream;
    Ptr<CharacterHandle>                   pTarget;
    Ptr<CharacterHandle>                   pAudioTarget;

    Array<WeakPtr<Video::VideoCharacter> > VideoCharacters;
    Video::VideoPlayer*                    pVideoPlayer;
    Video::VideoPlayer::VideoInfo          VInfo;
    Ptr<Video::VideoPlayer::SyncObject>    pSyncObj;
    float                                  FrameTime;

    Render::TextureManager*                pTextureManager;
    Ptr<Video::VideoImage>                 pTextureImage;

    bool     LoopFlag;
    unsigned PoolsNumber;
    float    BufferTime;
    float    ReloadThreshHoldTime;
    float    OpenURLTimeout;
    float    SeekPos;
    bool     FrameIsReady;

    bool     StartNotificationSent;
    bool     StopNotificationSent;
    bool     SeekNotificationSent;
    bool     NotFoundNotificationSent;

    char*    pSubtitleMsg;
    char*    pSubtitleMsgTmp;

#ifdef GFX_ENABLE_SOUND
    Array<Sound::SoundChannel::Vector>     SpatialInfo[3];
#endif
};

inline VideoProviderNetStream* NetStream::GetVideoProvider() 
{ 
    return pVideoProvider;
}

inline Video::VideoPlayer* NetStream::GetVideoPlayer()
{
    return pVideoProvider ? pVideoProvider->GetPlayer() : NULL;
}

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_AS2NETSTREAM_H
