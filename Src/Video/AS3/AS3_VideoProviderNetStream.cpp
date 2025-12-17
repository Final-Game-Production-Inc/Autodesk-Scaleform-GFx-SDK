/**************************************************************************

Filename    :   AS3_VideoProviderNetStream.cpp
Content     :	GFx video: AS3 VideoProviderNetStream class
Created     :	May, 2008
Authors     :	Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/AS3/AS3_VideoProviderNetStream.h"

#ifdef GFX_ENABLE_VIDEO

namespace Scaleform { namespace GFx {
    
#ifdef GFX_ENABLE_SOUND
using namespace Sound;    
#endif
using namespace Video;

namespace AS3 {

//////////////////////////////////////////////////////////////////////////
//
VideoProviderNetStream::VideoProviderNetStream(NetStreamInterface* pnsintf) :
    pNetStreamInterface(pnsintf)
{
    SF_ASSERT(pNetStreamInterface);
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
    pSubtitle = pSubtitleTmp = NULL;
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
}

//////////////////////////////////////////////////////////////////////////
//

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
            SendNotification(NetStreamInterface::SeekNotify);
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
            SendNotification(NetStreamInterface::PlayStart);
            SendMetaData();
            pVideoPlayer->Play();
        }
        break;

    case VideoPlayer::Stopped:
    case VideoPlayer::Finished:
        if (!SeekNotificationSent)
        {
            SeekNotificationSent = true;
            SendNotification(NetStreamInterface::SeekInvalidTime, true);
        }
        SendNotification(NetStreamInterface::PlayStop);
        break;

    case VideoPlayer::FileNotFound:
        if (!NotFoundNotificationSent)
        {
            NotFoundNotificationSent = true;
            SendNotification(NetStreamInterface::PlayStreamNotFound, true);
        }
        break;

    case VideoPlayer::InternalError:
        SendNotification(NetStreamInterface::PlayInternalError, true);

    default:
        break;
    }

    if (st == VideoPlayer::Ready || st == VideoPlayer::Playing)
    {
        CuePoints.Clear();
        pVideoPlayer->GetCurrentCuePoints(&CuePoints);
        SendCuePoints();
    }

    if (pNetStreamInterface && VInfo.MaxSubtitleLen > 0)
    {
        AllocSubtitleBuffers();
        if (pVideoPlayer)
            pVideoPlayer->UpdateTexture(pTextureImage, pSubtitleTmp, VInfo.MaxSubtitleLen);
        if (pSubtitle && pSubtitleTmp)
            SendSubtitle();
    }
    else {
        if (pVideoPlayer)
            pVideoPlayer->UpdateTexture(pTextureImage, NULL, 0);
    }
}

void VideoProviderNetStream::Pause(bool p)
{
    if (pVideoPlayer)
        pVideoPlayer->Pause(p);
}

void VideoProviderNetStream::Close()
{
    if (pTextureImage)
        pTextureImage->NotifyVideoPlayerRemoved();
    pTextureImage = NULL;

    if (pVideoPlayer)
        pVideoPlayer->Destroy();
    pVideoPlayer = NULL;

    ReleaseSubtitleBuffers();
}

bool VideoProviderNetStream::IsActive()
{
    if (!pVideoPlayer)
        return false;

    VideoPlayer::Status st = pVideoPlayer->GetStatus();
    return (st == VideoPlayer::Opening || st == VideoPlayer::Ready   || st == VideoPlayer::Starting ||
            st == VideoPlayer::Playing || st == VideoPlayer::Seeking || st == VideoPlayer::Stopping);
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

void VideoProviderNetStream::NotifyVideoCharacters()
{
    for(size_t i = 0; i < VideoCharacters.GetSize(); ++i)
    {
        Ptr<VideoCharacter> pvideo = VideoCharacters[i];
        if (pvideo)
            pvideo->CreateTexture();
    }
}

void VideoProviderNetStream::AllocSubtitleBuffers()
{
    if (!pSubtitle)
    {
        pSubtitle = (char*) SF_ALLOC(VInfo.MaxSubtitleLen, Stat_Default_Mem);
        Alg::MemUtil::Set(pSubtitle, 0, VInfo.MaxSubtitleLen);
    }
    if (!pSubtitleTmp)
    {
        pSubtitleTmp = (char*) SF_ALLOC(VInfo.MaxSubtitleLen, Stat_Default_Mem);
        Alg::MemUtil::Set(pSubtitleTmp, 0, VInfo.MaxSubtitleLen);
    }
}
void VideoProviderNetStream::ReleaseSubtitleBuffers()
{
    if (pSubtitleTmp)
    {
        SF_FREE(pSubtitleTmp);
        pSubtitleTmp = NULL;
    }
    if (pSubtitle)
    {
        SF_FREE(pSubtitle);
        pSubtitle = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//

void VideoProviderNetStream::OpenUrl(const char* url, ASVM& asvm)
{
    MovieImpl* proot = asvm.GetMovieImpl();
    Ptr<LoadStates> pls = *SF_NEW LoadStates(proot->GetLoaderImpl(), proot->pStateBag);

    String level0Path;
    proot->GetMainMoviePath(&level0Path);
    URLBuilder::LocationInfo loc(URLBuilder::File_Regular, String(url), level0Path);
    String fileName;
    pls->BuildURL(&fileName, loc);

    DisplayObjContainer* pobj = asvm.GetMovieRoot()->GetMainTimeline();
    CharacterHandle* ptarget = NULL;
    if (pobj && pobj->IsSprite())
        ptarget = pobj->CharToSprite()->GetCharacterHandle();

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

            pTarget = ptarget;
            proot->AddVideoProvider(this);
        }
    }
    if (pVideoPlayer)
    {
        pVideoPlayer->Open(fileName.ToCStr());
        pVideoPlayer->SetLoopFlag(LoopFlag);
        ReleaseSubtitleBuffers();

        StartNotificationSent = false;
        NotFoundNotificationSent = false;
        SeekNotificationSent = true;

#ifdef GFX_ENABLE_SOUND
        if (!pAudioTarget)
            pAudioTarget = proot->GetRootMovie()->GetCharacterHandle();
#endif
    }
}

void VideoProviderNetStream::Stop()
{
    if (pVideoPlayer)
        pVideoPlayer->Stop();
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

    if (poldSprite && pVideoPlayer)
    {
        Sound::SoundChannel* pchan = pVideoPlayer->GetSoundChannel();
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
    pAudioTarget = ptarget->GetCharacterHandle();
}


void VideoProviderNetStream::InitSoundChannel()
{
    if (!pAudioTarget)
        return;

    Sprite* psprite = getSpriteByHandle(pAudioTarget);

    Sound::SoundChannel* pmainchan = pVideoPlayer->GetSoundChannel(VideoPlayer::MainTrack);
    if (psprite && pmainchan)
    {
        pmainchan->SetVolume(psprite->GetRealSoundVolume());
        psprite->AddActiveSound(pmainchan, NULL, NULL);
    }
    Sound::SoundChannel* psubchan = pVideoPlayer->GetSoundChannel(VideoPlayer::SubAudio);
    if (psprite && psubchan)
    {
        psubchan->SetVolume(psprite->GetRealSubSoundVolume());
        psprite->AddActiveSound(psubchan, NULL, NULL);
    }
}

void VideoProviderNetStream::SetSoundVolume(int volume, int subVolume)
{
    if (!pAudioTarget)
        return;

    Sprite* psprite = getSpriteByHandle(pAudioTarget);
    if (psprite && pVideoPlayer)
    {
        psprite->SetSoundVolume(volume, subVolume);

        Sound::SoundChannel* pmainchan = pVideoPlayer->GetSoundChannel(VideoPlayer::MainTrack);
        if (pmainchan)
            pmainchan->SetVolume(psprite->GetRealSoundVolume());

        Sound::SoundChannel* psubchan = pVideoPlayer->GetSoundChannel(VideoPlayer::SubAudio);
        if (psubchan)
            psubchan->SetVolume(psprite->GetRealSubSoundVolume());
    }
}
#endif // GFX_ENABLE_SOUND

//////////////////////////////////////////////////////////////////////////
//

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

//////////////////////////////////////////////////////////////////////////
//

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

void VideoProviderNetStream::SendNotification(NetStreamInterface::Notification n, bool error)
{
    const String code = NetStreamInterface::NotificationToString(n);
    const String level = error ? "error" : "status";
    if (pNetStreamInterface)
        pNetStreamInterface->DispatchNetStatus(code, level);
}

void VideoProviderNetStream::SendMetaData()
{
    if (pNetStreamInterface)
        pNetStreamInterface->ExecuteOnMataData(&VInfo);
}

void VideoProviderNetStream::SendCuePoints()
{
    for(UPInt i = 0; i < CuePoints.GetSize(); ++i)
    {
        VideoPlayer::CuePoint cp = CuePoints[i];
        if (pNetStreamInterface)
            pNetStreamInterface->ExecuteOnCuePoint(&cp);
    }
}

void VideoProviderNetStream::SendSubtitle()
{
    if (SFstrncmp(pSubtitle, pSubtitleTmp, VInfo.MaxSubtitleLen) != 0)
    {
        SFstrncpy(pSubtitle, VInfo.MaxSubtitleLen, pSubtitleTmp, VInfo.MaxSubtitleLen);
        if (pNetStreamInterface)
            pNetStreamInterface->ExecuteOnSubtitle(pSubtitle);
    }
}

}}} // Scaleform::GFx::AS3

#endif // GFX_ENABLE_VIDEO
