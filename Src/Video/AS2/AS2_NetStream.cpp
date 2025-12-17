/**************************************************************************

Filename    :   AS2_NetStream.cpp
Content     :	GFx video: AS2 NetStream and VideoProviderNetStream classes
Created     :	May, 2008
Authors     :	Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/AS2/AS2_NetStream.h"

#ifdef GFX_ENABLE_VIDEO

#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx {
    
#ifdef GFX_ENABLE_SOUND
using namespace Sound;    
#endif
using namespace Video;
    
namespace AS2 {

//////////////////////////////////////////////////////////////////////////
//

NetStream::NetStream(ASStringContext *psc, Object* pprototype) : Object(psc), Paused(false)
{
    Set__proto__(psc, pprototype);
}

NetStream::NetStream(Environment* penv) : Object(penv), Paused(false)
{
    commonInit(penv);
}

void NetStream::commonInit(Environment* penv)
{
    NetStreamProto* pproto = (NetStreamProto*)penv->GetPrototype(ASBuiltin_NetStream);
    Set__proto__(penv->GetSC(), pproto);
    pVideoProvider = *SF_HEAP_NEW(penv->GetHeap()) VideoProviderNetStream(this);
    SF_ASSERT(pVideoProvider);
}

NetStream::~NetStream()
{
    Finalize();
}

void NetStream::Finalize()
{
    if(pVideoProvider)
    {
        pVideoProvider->NotifyNetStreamRemoved();
        Stop();
    }
}

#ifdef GFX_AS_ENABLE_GC
void NetStream::Finalize_GC()
{
    Finalize();
    OnSubtitleCallback.Finalize_GC();
    OnMetaDataCallback.Finalize_GC();
    OnStatusCallback.Finalize_GC();
    OnCuePointCallback.Finalize_GC();
    pVideoProvider.~Ptr();
    Object::Finalize_GC();
}
#endif // GFX_AS_ENABLE_GC

void NetStream::OpenUrl(const char* url, Environment* env)
{
    Paused = false;
    GetVideoProvider()->OpenUrl(url, env);
}

void NetStream::Stop()
{
    GetVideoProvider()->Stop();
}
void NetStream::Pause()
{
    Paused = !Paused;
    GetVideoProvider()->Pause(Paused);
}
void NetStream::Pause(bool p)
{
    Paused = p;
    GetVideoProvider()->Pause(Paused);
}

void NetStream::Seek(Number fpos, Environment* env)
{
    MovieImpl* proot = env->GetMovieImpl();
    GetVideoProvider()->Seek((float)fpos, proot);
}

void NetStream::SetAudioTarget(Sprite* ptarget)
{
#ifdef GFX_ENABLE_SOUND
    GetVideoProvider()->SetAudioTarget(ptarget);
#else
    SF_UNUSED(ptarget);
#endif
}

bool NetStream::GetMember(Environment *penv, const ASString& name, Value *val)
{
    if (penv->CheckExtensions())
    {
        if (name == "setReloadThresholdTime")
        {
            *val = Value(penv->GetSC(), NetStreamProto::SetReloadThresholdTime);
            return true;
        }
        else if (name == "setNumberOfFramePools")
        {
            *val = Value(penv->GetSC(), NetStreamProto::SetNumberOfFramePools);
            return true;
        }
        else if (name == "setOpenTimeout")
        {
            *val = Value(penv->GetSC(), NetStreamProto::SetOpenURLTimeout);
            return true;
        }
#ifdef GFX_ENABLE_SOUND
        else if (name == "setSoundSpatial")
        {
            *val = Value(penv->GetSC(), NetStreamProto::SetSoundSpatial);
            return true;
        }
#endif
        else if (name == "onSubtitle")
        {
            *val = OnSubtitleCallback;
            return true;
        }
        else if (name == "subtitleTrack")
        {
            int c = 0;
            c = GetVideoProvider()->GetSubtitleChannel();
            if (c < 0)
                c = -1;
            c++;
            *val = Value(c);
            return true;
        }
        else if (name == "currentFrame")
        {
            UInt32 curr_frame = GetVideoProvider()->GetPosition();
            *val = Value(1.0f * (curr_frame + 1));
            return true;
        }
    }
    if (name == "time")
    {
        UInt32 curr_frame = GetVideoProvider()->GetPosition();
        const VideoPlayer::VideoInfo& info = GetVideoProvider()->GetVideoInfo();
        if (info.FrameRate > 0)
            *val = Value((1.0f * curr_frame ) / (info.FrameRate / 1000.0f));
        else
            *val = Value(0.0f);
        return true;
    }
    else if(name == "onMetaData")
    {
        *val = OnMetaDataCallback;
        return true;
    }
    else if(name == "onStatus")
    {
        *val = OnStatusCallback;
        return true;
    }
    else if(name == "onCuePoint")
    {
        *val = OnCuePointCallback;
        return true;
    }
    else if(name == "currentFps")
    {
        const VideoPlayer::VideoInfo& info = GetVideoProvider()->GetVideoInfo();
        *val = Value(1.0f * info.FrameRate / 1000.f);
        return true;
    }
    return Object::GetMember(penv, name, val);
}

bool NetStream::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    if(name == "onMetaData")
    {
        FunctionRef mfunc = val.ToFunction(penv);
        OnMetaDataCallback = mfunc;
        return true;
    }
    else if(name == "onStatus")
    {
        FunctionRef mfunc = val.ToFunction(penv);
        OnStatusCallback = mfunc;
        return true;
    }
    else if(name == "onCuePoint")
    {
        FunctionRef mfunc = val.ToFunction(penv);
        OnCuePointCallback = mfunc;
        return true;
    }

    if (penv->CheckExtensions() )
    {
        if(name == "onSubtitle")
        {
            FunctionRef mfunc = val.ToFunction(penv);
            OnSubtitleCallback = mfunc;
            return true;
        }
        else if (name == "subtitleTrack")
        {
            if (!OnSubtitleCallback.IsNull())
            {
                int c = (int)val.ToNumber(penv);
                c--;
                GetVideoProvider()->SetSubtitleChannel(c);
            }
            return true;
        }
        else if (name == "audioTrack")
        {
            GetVideoProvider()->SetAudioTrack((int)val.ToNumber(penv));
            return true;
        }
        else if (name == "subAudioTrack")
        {
            GetVideoProvider()->SetSubAudioTrack((int)val.ToNumber(penv));
            return true;
        }
        else if (name == "voiceTrack")
        {
            GetVideoProvider()->ReplaceCenterVoice((int)val.ToNumber(penv));
            return true;
        }
        else if (name == "loop")
        {
            GetVideoProvider()->SetLoopFlag(val.ToBool(penv));
            return true;
        }
        else if (name == "swfSync")
        {
            AvmCharacter* target = penv->GetAvmTarget();
            if (target->IsSprite())
                GetVideoProvider()->SetSwfSync(val.ToBool(penv), target->ToSprite());
        }
    }
    return Object::SetMember(penv, name, val, flags);
}

void NetStream::SetBufferTime(Number time)
{
    GetVideoProvider()->SetBufferTime((float)time);
}
void NetStream::SetNumberOfFramePools(Number pools)
{
    GetVideoProvider()->SetNumberOfFramePools((unsigned)pools);
}
void NetStream::SetReloadThresholdTime(Number time)
{
    GetVideoProvider()->SetReloadThresholdTime((float)time);
}
void NetStream::SetOpenURLTimeout(Number timeout)
{
    GetVideoProvider()->SetOpenURLTimeout((float)timeout);
}
void NetStream::SetSubtitleChannel(Number channel)
{
    GetVideoProvider()->SetSubtitleChannel((int)channel);
}

#ifdef GFX_ENABLE_SOUND

SoundChannel* NetStream::GetSoundChannel() 
{
    VideoPlayer* pplayer = GetVideoPlayer();
    if (pplayer)
        return pplayer->GetSoundChannel();
    return NULL;
}

void NetStream::SetSoundPosition(Array<SoundChannel::Vector>& varr)
{
    GetVideoProvider()->SetSoundPosition(varr);
}
void NetStream::SetSoundOrientation(Array<SoundChannel::Vector>& varr)
{
    GetVideoProvider()->SetSoundOrientation(varr);
}
void NetStream::SetSoundVelocity(Array<SoundChannel::Vector>& varr)
{
    GetVideoProvider()->SetSoundVelocity(varr);
}
void NetStream::ApplySoundSpatial()
{
    GetVideoProvider()->ApplySoundSpatial();
}

#endif // GFX_ENABLE_SOUND

//////////////////////////////////////////////////////////////////////////
//

void NetStreamProto::Close(const FnCall& fn)
{
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (pnetstream)
    {
        pnetstream->Stop();
        // Release resources immediately after stop
        if (fn.NArgs > 0 && fn.Arg(0).ToBool(fn.Env))
            pnetstream->GetVideoProvider()->Close();
    }
}

void NetStreamProto::Pause(const FnCall& fn)
{
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream)
        return;
    if (fn.NArgs > 0)
    {
        bool p = fn.Arg(0).ToBool(fn.Env);
        pnetstream->Pause(p);
    }
    else
        pnetstream->Pause();
}

void NetStreamProto::Play(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.Play requires one argument (file name)\n");
        return;
    }
    ASString url(fn.Arg(0).ToString(fn.Env));
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (pnetstream)
        pnetstream->OpenUrl(url.ToCStr(), fn.Env);
}

void NetStreamProto::SetBufferTime(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.setBufferTime requires one argument (time)\n");
        return;
    }
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream || !fn.Arg(0).IsNumber())
        return;
    pnetstream->SetBufferTime(fn.Arg(0).ToNumber(fn.Env));
}

void NetStreamProto::SetNumberOfFramePools(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.setNumberOfFramePools requires one argument (pools)\n");
        return;
    }
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream || !fn.Arg(0).IsNumber())
        return;
    pnetstream->SetNumberOfFramePools(fn.Arg(0).ToNumber(fn.Env));
}

void NetStreamProto::SetReloadThresholdTime(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.setReloadThresholdTime requires one argument (time)\n");
        return;
    }
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream || !fn.Arg(0).IsNumber())
        return;
    pnetstream->SetReloadThresholdTime(fn.Arg(0).ToNumber(fn.Env));
}

void NetStreamProto::SetOpenURLTimeout(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.setOpenTimeout requires one argument (timeout)\n");
        return;
    }
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream || !fn.Arg(0).IsNumber())
        return;
    pnetstream->SetOpenURLTimeout(fn.Arg(0).ToNumber(fn.Env));
}

#ifdef GFX_ENABLE_SOUND

inline void parseArrayArg(Environment *penv, const Value& val, Array<SoundChannel::Vector>& vecarr)
{
    if (val.IsObject() && val.ToObject(penv)->GetObjectType() == Object::Object_Array)
    {
        ArrayObject* parr = static_cast<ArrayObject*>(val.ToObject(penv));
        for (int i = 0, n = parr->GetSize(); i < n; i += 3)
        {
            SoundChannel::Vector vec;
            vec.X = (float)parr->GetElementPtr(i  )->ToNumber(penv);
            vec.Y = (float)parr->GetElementPtr(i+1)->ToNumber(penv);
            vec.Z = (float)parr->GetElementPtr(i+2)->ToNumber(penv);
            vecarr.PushBack(vec);
        }
    }
}

void NetStreamProto::SetSoundSpatial(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.SetSoundSpatial requires at least one argument (position)\n");
        return;
    }
    Environment* penv = fn.Env;
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream)
        return;

    // Sound spatial position
    Array<SoundChannel::Vector> sndpos;
    parseArrayArg(penv, fn.Arg(0), sndpos);
    pnetstream->SetSoundPosition(sndpos);

    // Sound spatial orientation and velocity (optional)
    if (fn.NArgs > 1)
    {
        Array<SoundChannel::Vector> sndorient;
        parseArrayArg(penv, fn.Arg(1), sndorient);
        pnetstream->SetSoundOrientation(sndorient);
    }
    if (fn.NArgs > 2)
    {
        Array<SoundChannel::Vector> sndvelo;
        parseArrayArg(penv, fn.Arg(2), sndvelo);
        pnetstream->SetSoundVelocity(sndvelo);
    }
    pnetstream->ApplySoundSpatial();
}

#endif // GFX_ENABLE_SOUND

void NetStreamProto::Seek(const FnCall& fn)
{
    if (fn.NArgs < 1)
    {
        fn.Env->LogScriptError("Error: NetStream.seek requires one argument (offset)\n");
        return;
    }
    NetStream* pnetstream = static_cast<NetStream*>(fn.ThisPtr);
    if (!pnetstream || !fn.Arg(0).IsNumber())
        return;
    pnetstream->Seek(fn.Arg(0).ToNumber(fn.Env), fn.Env);
}

//////////////////////////////////////////////////////////////////////////
//

static const NameFunction AS2_NetStreamFunctionTable[] =
{
    { "close",         &NetStreamProto::Close },
    { "pause",         &NetStreamProto::Pause },
    { "play",          &NetStreamProto::Play },
    { "setBufferTime", &NetStreamProto::SetBufferTime },
    { "seek",          &NetStreamProto::Seek },
    { 0, 0 }
};

NetStreamProto::NetStreamProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) :
    Prototype<NetStream>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, AS2_NetStreamFunctionTable);
}

//////////////////////////////////////////////////////////////////////////
//

NetStreamCtorFunction::NetStreamCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor)
{
    SF_UNUSED(psc);
}

Object* NetStreamCtorFunction::CreateNewObject(Environment *penv) const
{
    Object* obj = SF_HEAP_NEW(penv->GetHeap()) NetStream(penv);
    return obj;
}

void NetStreamCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<NetStream> pobj;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_NetStream
                   && !fn.ThisPtr->IsBuiltinPrototype())
        pobj = static_cast<NetStream*>(fn.ThisPtr);
    else
        pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) NetStream(fn.Env);
    fn.Result->SetAsObject(pobj.GetPtr());
}

FunctionRef NetStreamCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef ctor(*SF_HEAP_NEW(pgc->GetHeap()) NetStreamCtorFunction(&sc));
    Ptr<Object> pproto = *SF_HEAP_NEW(pgc->GetHeap()) NetStreamProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_NetStream, pproto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_NetStream), Value(ctor));
    return ctor;
}

//////////////////////////////////////////////////////////////////////////
//

VideoProviderNetStream::VideoProviderNetStream(NetStream* pnetstream) : pNetStream(pnetstream)
{
    Init();
}

VideoProviderNetStream::~VideoProviderNetStream()
{
    Close();
}

void VideoProviderNetStream::Init()
{
    pVideoPlayer = NULL;
    Alg::MemUtil::Set(&VInfo, 0, sizeof(VInfo));
    FrameTime = 1.f/12.0f;
    pTextureManager = NULL;
    LoopFlag = false;
    PoolsNumber = 2;
    BufferTime = 0.0f;
    ReloadThreshHoldTime = 0.8f;
    OpenURLTimeout = 5.0f;
    SeekPos = 0.0f;
    FrameIsReady = false;

    StartNotificationSent = false;
    StopNotificationSent = false;
    SeekNotificationSent = true;
    NotFoundNotificationSent = false;
    pSubtitleMsg = pSubtitleMsgTmp = NULL;
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::OpenUrl(const char* url, Environment* env)
{
    MovieImpl* proot = env->GetMovieImpl();
    Ptr<LoadStates> pls = *new LoadStates(proot->GetLoaderImpl(), proot->pStateBag);

    String level0Path;
    MovieRoot* pas2root = env->GetAS2Root();
    pas2root->GetLevel0Path(&level0Path);
    URLBuilder::LocationInfo loc(URLBuilder::File_Regular, String(url), level0Path);
    String fileName;
    pls->BuildURL(&fileName, loc);

    FrameTime = 1.f/12.0f;
    SeekPos = 0.0f;
    VInfo.CuePoints.Clear();
    VInfo.AudioTracks.Clear();
    pTextureImage = NULL;
    FrameIsReady = false;
    if (!pVideoPlayer)
    {
        Ptr<VideoBase> pvideo = proot->GetVideo();
        if (pvideo)
        {
            pTextureManager = pvideo->GetTextureManager();
            if (!pTextureManager)
                pTextureManager = proot->GetImageCreator()->GetTextureManager();
            SF_DEBUG_ASSERT(pTextureManager, "Texture manager is not set for Video subsystem");

            pVideoPlayer = pvideo->CreateVideoPlayer(
                proot->GetHeap(), proot->GetTaskManager(), proot->GetFileOpener(), proot->GetLog());
        }
        if (pVideoPlayer)
        {
            pVideoPlayer->SetSyncObject(pSyncObj);
            pVideoPlayer->SetBufferTime(BufferTime);
            pVideoPlayer->SetNumberOfFramePools(PoolsNumber);
            pVideoPlayer->SetReloadThresholdTime(ReloadThreshHoldTime);
            pVideoPlayer->SetDecodeHeaderTimeout(OpenURLTimeout);
#ifdef GFX_ENABLE_SOUND
            pVideoPlayer->SetSoundSpatialInfo(SpatialInfo);
#endif
            pTarget = env->GetTarget()->GetCharacterHandle();
            proot->AddVideoProvider(this);
        }
    }
    if (pVideoPlayer)
    {
        pVideoPlayer->Open(fileName.ToCStr());
        pVideoPlayer->SetLoopFlag(LoopFlag);
        StartNotificationSent = false;
        NotFoundNotificationSent = false;
        SeekNotificationSent = true;
        if (pSubtitleMsgTmp)
        {
            SF_FREE(pSubtitleMsgTmp);
            pSubtitleMsgTmp = NULL;
        }
        if (pSubtitleMsg)
        {
            SF_FREE(pSubtitleMsg);
            pSubtitleMsg = NULL;
        }
#ifdef GFX_ENABLE_SOUND
        if (!pAudioTarget)
            pAudioTarget = proot->GetRootMovie()->GetCharacterHandle();
#endif
    }
}

void VideoProviderNetStream::Close()
{
    if (pTextureImage)
        pTextureImage->NotifyVideoPlayerRemoved();
    pTextureImage = NULL;

    if (pVideoPlayer)
        pVideoPlayer->Destroy();
    pVideoPlayer = NULL;

    if (pSubtitleMsgTmp)
    {
        SF_FREE(pSubtitleMsgTmp);
        pSubtitleMsgTmp = NULL;
    }
    if (pSubtitleMsg)
    {
        SF_FREE(pSubtitleMsg);
        pSubtitleMsg = NULL;
    }
}

bool VideoProviderNetStream::IsActive()
{
    if (!pVideoPlayer)
        return false;
    VideoPlayer::Status st = pVideoPlayer->GetStatus();
    return (st == VideoPlayer::Opening || st == VideoPlayer::Ready   || st == VideoPlayer::Starting ||
            st == VideoPlayer::Playing || st == VideoPlayer::Seeking || st == VideoPlayer::Stopping);
}

//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_ENABLE_SOUND
inline Sprite* getSpriteByHandle(CharacterHandle* ptarget)
{
    SF_ASSERT(ptarget);

    Sprite* psprite = NULL;
    DisplayObject* pchar = ptarget->GetCharacter();
    if (pchar)
    {
        if(pchar->IsSprite())
            psprite = pchar->CharToSprite();
        else
        {
            if(pchar->GetParent())
            {
                SF_ASSERT(pchar->GetParent()->IsSprite());
                psprite = pchar->GetParent()->CharToSprite();
            }
        }
    }
    return psprite;
}

void VideoProviderNetStream::SetAudioTarget(Sprite* ptarget)
{
    SF_ASSERT(ptarget);

    Sprite* poldSprite = NULL;
    if (pAudioTarget)
        poldSprite = getSpriteByHandle(pAudioTarget);

    if (poldSprite == ptarget)
        return;

    if (poldSprite)
    {
        if (pVideoPlayer)
        {
            SoundChannel* pchan = pVideoPlayer->GetSoundChannel();
            if (pchan)
            {
                Ptr<Sprite::ActiveSoundItem> psi = *poldSprite->ReleaseActiveSound(pchan);
                if (psi)
                {
                    psi->pChannel->SetVolume(ptarget->GetRealSoundVolume());
                    ptarget->AttachActiveSound(psi);
                }
                else
                {
                    pchan->SetVolume(ptarget->GetRealSoundVolume());
                    ptarget->AddActiveSound(pchan, NULL, NULL);
                }
            }
        }
    }
    pAudioTarget = ptarget->GetCharacterHandle();
}

void VideoProviderNetStream::InitSoundChannel()
{
    if (!pAudioTarget)
        return;

    Sprite* psprite = getSpriteByHandle(pAudioTarget);

    SoundChannel* pmainchan = pVideoPlayer->GetSoundChannel(VideoPlayer::MainTrack);
    if (psprite && pmainchan)
    {
        pmainchan->SetVolume(psprite->GetRealSoundVolume());
        psprite->AddActiveSound(pmainchan, NULL, NULL);
    }
    SoundChannel* psubchan = pVideoPlayer->GetSoundChannel(VideoPlayer::SubAudio);
    if (psprite && psubchan)
    {
        psubchan->SetVolume(psprite->GetRealSubSoundVolume());
        psprite->AddActiveSound(psubchan, NULL, NULL);
    }
}
#endif // GFX_ENABLE_SOUND

void VideoProviderNetStream::Stop()
{
    if (pVideoPlayer)
        pVideoPlayer->Stop();
}

void VideoProviderNetStream::Pause(bool p)
{
    if (pVideoPlayer)
        pVideoPlayer->Pause(p);
}


UInt32 VideoProviderNetStream::GetPosition()
{
    if (!pVideoPlayer)
        return 0;
    return pVideoPlayer->GetPosition();
}

void VideoProviderNetStream::Seek(float fpos, MovieImpl* proot)
{
    SeekPos = fpos;
    if (!pVideoPlayer)
        return;

    SeekNotificationSent = false;
    VideoPlayer::Status st = pVideoPlayer->GetStatus();
    if (st == VideoPlayer::Ready   || st == VideoPlayer::Stopping ||
        st == VideoPlayer::Playing || st == VideoPlayer::Finished)
    {
        UInt32 pos = UInt32(SeekPos * (VInfo.FrameRate / 1000.0f));
        pVideoPlayer->Seek(pos);
    }
    if (st == VideoPlayer::Finished)
    {
        if (proot)
            proot->AddVideoProvider(this);
    }
#ifdef GFX_ENABLE_SOUND
    InitSoundChannel();
#endif
}

void VideoProviderNetStream::SetLoopFlag(bool flag)
{
    LoopFlag = flag;
    if (pVideoPlayer)
        pVideoPlayer->SetLoopFlag(flag);
}

void VideoProviderNetStream::SetAudioTrack(int track)
{
    if (!pVideoPlayer)
        return;
    pVideoPlayer->SetAudioTrack(track);
}
void VideoProviderNetStream::SetSubAudioTrack(int track)
{
    if (!pVideoPlayer)
        return;
    pVideoPlayer->SetSubAudioTrack(track);
}
void VideoProviderNetStream::ReplaceCenterVoice(int track)
{
    if (!pVideoPlayer)
        return;
    pVideoPlayer->ReplaceCenterVoice(track);
}

void VideoProviderNetStream::SetSubtitleChannel(int channel)
{
    if (!pVideoPlayer)
        return;
    pVideoPlayer->SetSubtitleChannel(channel);
}
int VideoProviderNetStream::GetSubtitleChannel()
{
    if (!pVideoPlayer)
        return -1;
    return pVideoPlayer->GetSubtitleChannel();
}

//////////////////////////////////////////////////////////////////////////
//

class SwfVideoSyncObject : public VideoPlayer::SyncObject
{
public:
    SwfVideoSyncObject(Sprite* ptarget) : pTarget(ptarget), 
        StartFrame(ptarget->GetCurrentFrame()),
        FrameTime(ptarget->GetMovieImpl()->GetFrameTime()) {}
    ~SwfVideoSyncObject() {}

    virtual void SetStartFrame(unsigned frame) { StartFrame = frame; }
    
    virtual void Start() {}
    virtual void Stop() {}
    virtual void Pause(bool) {}

    virtual void GetTime(UInt64* count, UInt64* unit)
    {
        Ptr<Sprite> ptarget = pTarget;
        if (ptarget)
        {
            unsigned curFrame = ptarget->GetCurrentFrame();
            if (curFrame > StartFrame)
            {
                *unit = 1000000;
                *count = UInt64((curFrame - StartFrame) * FrameTime * (*unit));
                return;
            }
        }
        *count = 0;
        *unit = 1000;
    }

    WeakPtr<Sprite> pTarget;
    unsigned StartFrame;
    float    FrameTime;
};

void VideoProviderNetStream::SetSwfSync(bool flag, Sprite* ptarget)
{
    if (flag)
        pSyncObj = *SF_NEW SwfVideoSyncObject(ptarget);
    else
        pSyncObj = NULL;
    if (pVideoPlayer)
        pVideoPlayer->SetSyncObject(pSyncObj);
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::SendNotification(const char* note, bool error)
{
    if (pTarget && pNetStream && !pNetStream->OnStatusCallback.IsNull())
    {
        InteractiveObject* ptarget = pTarget->GetCharacter()->CharToInteractiveObject();
        if (ptarget)
        {
            Environment* env = ToAvmCharacter(ptarget)->GetASEnvironment();
            Ptr<Object> status = *SF_HEAP_NEW(env->GetHeap()) Object(env);
            if (error)
                status->SetConstMemberRaw(env->GetSC(), "level", Value(env->CreateConstString("error")));
            else
                status->SetConstMemberRaw(env->GetSC(), "level", Value(env->CreateConstString("status")));
            status->SetConstMemberRaw(env->GetSC(), "code", Value(env->CreateConstString(note)));
            env->Push(status);
            Value result;
            pNetStream->OnStatusCallback.Invoke(FnCall(&result, pNetStream, env, 1, env->GetTopIndex()));
            env->Drop1();
        }
    }
}

static Object* convertCuepoint(const VideoPlayer::CuePoint& cppcp, Environment* env)
{
    Object* cp = SF_HEAP_NEW(env->GetHeap()) Object(env);
    cp->SetConstMemberRaw(env->GetSC(), "name", Value(env->CreateString(cppcp.Name.ToCStr())));
    cp->SetConstMemberRaw(env->GetSC(), "time", Value(cppcp.Time * 1.f / 1000.0f));
    cp->SetConstMemberRaw(env->GetSC(), "type", Value(cppcp.Type == VideoPlayer::NavigationCuePoint ?
        env->CreateConstString("navigation") : env->CreateConstString("event") ));
    if (cppcp.Params.GetSize() > 0)
    {
        Ptr<Object> params = *SF_HEAP_NEW(env->GetHeap()) Object(env);
        for(UPInt p = 0; p < cppcp.Params.GetSize(); ++p)
        {
            Ptr<Object> param = *SF_HEAP_NEW(env->GetHeap()) Object(env);
            params->SetMemberRaw(env->GetSC(), env->CreateString(cppcp.Params[p].Name.ToCStr()),
                Value(env->CreateString(cppcp.Params[p].Value.ToCStr())));
        }
        cp->SetConstMemberRaw(env->GetSC(), "parameters", Value(params));
    }
    return cp;
}

void VideoProviderNetStream::SendMetaInformation()
{
    if (pNetStream && pTarget && !pNetStream->OnMetaDataCallback.IsNull() )
    {
        InteractiveObject* ptarget = pTarget->GetCharacter()->CharToInteractiveObject();
        if (ptarget)
        {
            Environment* env = ToAvmCharacter(ptarget)->GetASEnvironment();
            Ptr<Object> info = *SF_HEAP_NEW(env->GetHeap()) Object(env);
            info->SetConstMemberRaw(env->GetSC(), "canSeekToEnd", Value(true));
            info->SetConstMemberRaw(env->GetSC(), "framerate", Value(VInfo.FrameRate/1000.f));
            info->SetConstMemberRaw(env->GetSC(), "width",     Value(VInfo.Width));
            info->SetConstMemberRaw(env->GetSC(), "height",    Value(VInfo.Height));
            info->SetConstMemberRaw(env->GetSC(), "duration",  Value(VInfo.TotalFrames * 1000.f/VInfo.FrameRate));
            if (env->CheckExtensions())
            {
                info->SetConstMemberRaw(env->GetSC(), "subtitleTracksNumber", Value(VInfo.SubtitleChannelsNumber));
                if (VInfo.AudioTracks.GetSize() > 0)
                {
                    Ptr<ArrayObject> audio_tracks =
                        *static_cast<ArrayObject*>(env->OperatorNew(env->GetBuiltin(ASBuiltin_Array)));
                    for(UPInt i = 0; i < VInfo.AudioTracks.GetSize(); ++i)
                    {
                        Ptr<Object> atinfo = *SF_HEAP_NEW(env->GetHeap()) Object(env);
                        atinfo->SetConstMemberRaw(env->GetSC(), "trackIndex",     Value(VInfo.AudioTracks[i].Index));
                        atinfo->SetConstMemberRaw(env->GetSC(), "channelsNumber", Value((long)VInfo.AudioTracks[i].ChannelsNumber));
                        atinfo->SetConstMemberRaw(env->GetSC(), "sampleRate",     Value((long)VInfo.AudioTracks[i].SampleRate));
                        atinfo->SetConstMemberRaw(env->GetSC(), "totalSamples",   Value((long)VInfo.AudioTracks[i].TotalSamples));
                        audio_tracks->PushBack(Value(atinfo));
                    }
                    info->SetConstMemberRaw(env->GetSC(), "audioTracks", Value(audio_tracks));
                }
            }
            if (VInfo.CuePoints.GetSize() > 0)
            {
                Ptr<ArrayObject> cuepoints =
                    *static_cast<ArrayObject*>(env->OperatorNew(env->GetBuiltin(ASBuiltin_Array)));
                for(UPInt i = 0; i < VInfo.CuePoints.GetSize(); ++i)
                {
                    Ptr<Object> cp = *convertCuepoint(VInfo.CuePoints[i], env);
                    cuepoints->PushBack(Value(cp));
                }
                info->SetConstMemberRaw(env->GetSC(), "cuePoints", Value(cuepoints));
            }
            env->Push(info);
            Value result;
            pNetStream->OnMetaDataCallback.Invoke(FnCall(&result, pNetStream, env, 1, env->GetTopIndex()));
            env->Drop1();
        }
    }
}

void VideoProviderNetStream::SendCuePoints(const Array<VideoPlayer::CuePoint>& cuePoints)
{
    if (pNetStream && pTarget && !pNetStream->OnCuePointCallback.IsNull() && cuePoints.GetSize() > 0)
    {
        InteractiveObject* ptarget = pTarget->GetCharacter()->CharToInteractiveObject();
        if (ptarget)
        {
            Environment* env = ToAvmCharacter(ptarget)->GetASEnvironment();
            for(UPInt i = 0; i < cuePoints.GetSize(); ++i)
            {
                Ptr<Object> cp = *convertCuepoint(cuePoints[i], env);
                env->Push(cp);
                Value result;
                pNetStream->OnCuePointCallback.Invoke(FnCall(&result, pNetStream, env, 1, env->GetTopIndex()));
                env->Drop1();
            }
        }
    }
}

void VideoProviderNetStream::SendSubtitleInformation()
{
    SF_ASSERT(pNetStream && !pNetStream->OnSubtitleCallback.IsNull());
    if (pTarget)
    {
        InteractiveObject* ptarget = pTarget->GetCharacter()->CharToInteractiveObject();
        if (ptarget)
        {
            Value method;
            Environment* env = ToAvmCharacter(ptarget)->GetASEnvironment();
            env->Push(env->CreateString(pSubtitleMsg));
            Value result;
            pNetStream->OnSubtitleCallback.Invoke(FnCall(&result, pNetStream, env, 1, env->GetTopIndex()));
            env->Drop1();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::Advance()
{
    if (!pVideoPlayer)
        return;

    FrameIsReady = pVideoPlayer->IsTextureReady();
    pVideoPlayer->Decode();

    VideoPlayer::Status st = pVideoPlayer->GetStatus();
    switch (st)
    {
        case VideoPlayer::NotInitialized:
        case VideoPlayer::Opening:
        case VideoPlayer::Starting:
        case VideoPlayer::Seeking:
        case VideoPlayer::Stopping:
            break;

        case VideoPlayer::Playing:
            if (!SeekNotificationSent && FrameIsReady)
            {
                SeekNotificationSent = true;
                SendNotification("NetStream.Seek.Notify", false);
            }
            break;

        case VideoPlayer::Ready:
            {
                if (!pTextureImage)
                {
                    pTextureImage = *pVideoPlayer->CreateTexture(pTextureManager);
                    if (pTextureImage)
                    {
                        pVideoPlayer->GetVideoInfo(&VInfo);
                        FrameTime = 1000.0f / VInfo.FrameRate;
                        FrameTime -= FrameTime * 0.1f;  // -10%
                    }
                }
                if (SeekPos > 0)
                {
                    UInt32 pos = UInt32(SeekPos * (VInfo.FrameRate/1000.0f));
                    pVideoPlayer->Seek(pos);
                }
#ifdef GFX_ENABLE_SOUND
                InitSoundChannel();
#endif
                NotifyVideoCharacters();
                SendNotification("NetStream.Play.Start", false);
                SendMetaInformation();
                pVideoPlayer->Play();
            }
            break;

        case VideoPlayer::Stopped:
        case VideoPlayer::Finished:
            if (!SeekNotificationSent)
            {
                SeekNotificationSent = true;
                SendNotification("NetStream.Seek.InvalidTime", true);
            }
            SendNotification("NetStream.Play.Stop", false);
            break;

        case VideoPlayer::FileNotFound:
            if (!NotFoundNotificationSent)
            {
                NotFoundNotificationSent = true;
                SendNotification("NetStream.Play.StreamNotFound", true);
            }
            break;

        case VideoPlayer::InternalError:
            SendNotification("NetStream.Play.InternalError", true);

        default:
            break;
    }

    if (st == VideoPlayer::Ready || st == VideoPlayer::Playing)
    {
        Array<VideoPlayer::CuePoint> cue_points;
        pVideoPlayer->GetCurrentCuePoints(&cue_points);
        SendCuePoints(cue_points);
    }

    if (pNetStream && !pNetStream->OnSubtitleCallback.IsNull() && VInfo.MaxSubtitleLen > 0)
    {
        if (!pSubtitleMsg)
        {
            pSubtitleMsg = (char*) SF_ALLOC(VInfo.MaxSubtitleLen, Stat_Default_Mem);
            Alg::MemUtil::Set(pSubtitleMsg, 0, VInfo.MaxSubtitleLen);
        }
        if (!pSubtitleMsgTmp)
        {
            pSubtitleMsgTmp = (char*) SF_ALLOC(VInfo.MaxSubtitleLen, Stat_Default_Mem);
            Alg::MemUtil::Set(pSubtitleMsgTmp, 0, VInfo.MaxSubtitleLen);
        }
        if (pVideoPlayer)
            pVideoPlayer->UpdateTexture(pTextureImage, pSubtitleMsgTmp, VInfo.MaxSubtitleLen);

        if (pSubtitleMsg && pSubtitleMsgTmp)
        {
            if (SFstrncmp(pSubtitleMsg, pSubtitleMsgTmp, VInfo.MaxSubtitleLen) != 0)
            {
                SFstrncpy(pSubtitleMsg, VInfo.MaxSubtitleLen, pSubtitleMsgTmp, VInfo.MaxSubtitleLen);
                SendSubtitleInformation();
            }
        }
    }
    else {
        if (pVideoPlayer)
            pVideoPlayer->UpdateTexture(pTextureImage, NULL, 0);
    }
}

VideoImage* VideoProviderNetStream::GetTexture(int* width, int* height)
{
    if (!pVideoPlayer)
        return NULL;
    if (!pTextureImage)
        return NULL;

    *width  = VInfo.Width;
    *height = VInfo.Height;
    return pTextureImage;
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::NotifyVideoCharacters()
{
    for(size_t i = 0; i < VideoCharacters.GetSize(); ++i)
    {
        Ptr<VideoCharacter> pvideo = VideoCharacters[i];
        if (pvideo)
            pvideo->CreateTexture();
    }
}

void VideoProviderNetStream::RegisterVideoCharacter(VideoCharacter* pvideo)
{
    if (pvideo)
    {
        for(size_t i = 0; i < VideoCharacters.GetSize(); ++i)
        {
            Ptr<VideoCharacter> pv = VideoCharacters[i];
            if (pvideo == pv.GetPtr())
                return;
        }
        VideoCharacters.PushBack(pvideo);
    }
}

VideoCharacter* VideoProviderNetStream::RemoveFirstVideoCharacter()
{
    if (VideoCharacters.GetSize() > 0)
    {
        Ptr<VideoCharacter> pvideo = VideoCharacters[0];
        VideoCharacters.RemoveAt(0);
        pTextureImage = NULL;
        return pvideo;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::SetBufferTime(float time)
{
    BufferTime = time;
    if (pVideoPlayer)
        pVideoPlayer->SetBufferTime(time);
}

void VideoProviderNetStream::SetNumberOfFramePools(unsigned pools)
{
    PoolsNumber = pools;
    if (pVideoPlayer)
        pVideoPlayer->SetNumberOfFramePools(pools);
}

void VideoProviderNetStream::SetReloadThresholdTime(float time)
{
    ReloadThreshHoldTime = time;
    if (pVideoPlayer)
        pVideoPlayer->SetReloadThresholdTime(time);
}

void VideoProviderNetStream::SetOpenURLTimeout(float timeout)
{
    OpenURLTimeout = timeout;
    if (pVideoPlayer)
        pVideoPlayer->SetDecodeHeaderTimeout(timeout);
}

//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_ENABLE_SOUND
void VideoProviderNetStream::SetSoundSpatial(unsigned idx, Array<SoundChannel::Vector>& varr)
{
    SpatialInfo[idx] = varr;
}

void VideoProviderNetStream::ApplySoundSpatial()
{
    if (pVideoPlayer)
        pVideoPlayer->SetSoundSpatialInfo(SpatialInfo);
}
#endif // GFX_ENABLE_SOUND

}}} // Scaleform::GFx::AS2

#endif // GFX_ENABLE_VIDEO
