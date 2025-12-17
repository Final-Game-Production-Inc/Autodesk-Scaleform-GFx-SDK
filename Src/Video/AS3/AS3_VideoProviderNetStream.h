/**************************************************************************

Filename    :   AS3_VideoProviderNetStream.h
Content     :   GFx video: AS3 VideoProviderNetStream class
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_AS3PROVIDERNETSTREAM_H
#define INC_GFX_VIDEO_AS3PROVIDERNETSTREAM_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/GFx_MediaInterfaces.h"
#include "Video/Video_Video.h"
#include "Video/Video_VideoImage.h"

#include "GFx/AS3/AS3_MovieRoot.h"
#include "Video/AS3/AS3_VideoCharacter.h"

namespace Scaleform { namespace GFx { namespace AS3 {

class VideoProviderNetStream;
class NetStreamInterface;

//////////////////////////////////////////////////////////////////////////
//

class NetStreamInterface : public RefCountBaseNTS<NetStreamInterface, StatMV_ActionScript_Mem>
{
public:
    // Supported notifications
    enum Notification
    {
        PlayStart,
        PlayStop,
        SeekNotify,
        PlayStreamNotFound,
        SeekInvalidTime,
        PlayInternalError,
        NoCode = -1
    };

    virtual void DispatchNetStatus(const String& code, const String& level) = 0;
    virtual void ExecuteOnMataData(Video::VideoPlayer::VideoInfo* pinfo) = 0;
    virtual void ExecuteOnCuePoint(Video::VideoPlayer::CuePoint* pcuepoint) = 0;
    virtual void ExecuteOnSubtitle(const String& subtitle) = 0;

    static const char* NotificationToString(Notification code);
};


//////////////////////////////////////////////////////////////////////////
//

class VideoProviderNetStream : public Video::VideoProvider
{
public:
    VideoProviderNetStream(NetStreamInterface* pnsintf);
    ~VideoProviderNetStream();

    virtual Video::VideoImage* GetTexture(int* width, int* height);

    virtual void  Advance();
    virtual void  Pause(bool pause);
    virtual void  Close();

    virtual bool  IsActive();
    virtual float GetFrameTime() { return FrameTime; }

    virtual void                   RegisterVideoCharacter(Video::VideoCharacter* pvideo);
    virtual Video::VideoCharacter* RemoveFirstVideoCharacter();

    void   OpenUrl(const char* url, ASVM& asvm);
    void   Stop();

    UInt32 GetPosition();
    void   Seek(float pos, MovieImpl* proot);
    void   SetLoopFlag(bool flag);
    void   SetSwfSync(bool flag, Sprite* ptarget);
    void   SetAudioTrack(int track);
    void   SetSubAudioTrack(int track);
    void   ReplaceCenterVoice(int track);
    void   SetSubtitleChannel(int channel);
    int    GetSubtitleChannel();

    void   SetBufferTime(float time);
    void   SetNumberOfFramePools(unsigned pools);
    void   SetReloadThresholdTime(float time);
    void   SetOpenURLTimeout(float timeout);

#ifdef GFX_ENABLE_SOUND
    void   SetAudioTarget(Sprite* psprite);
    void   SetSoundVolume(int volume, int subVolume = 100);
#endif

    void   SendNotification(NetStreamInterface::Notification n, bool error = false);
    void   SendMetaData();
    void   SendCuePoints();
    void   SendSubtitle();

    void   NotifyNetStreamRemoved() { pNetStreamInterface = NULL; }

    Video::VideoPlayer* GetPlayer() { return pVideoPlayer; }
    const Video::VideoPlayer::VideoInfo& GetVideoInfo() const { return VInfo; }

private:
    void   Init();
#ifdef GFX_ENABLE_SOUND
    void   InitSoundChannel();
#endif
    void   NotifyVideoCharacters();

    void   AllocSubtitleBuffers();
    void   ReleaseSubtitleBuffers();

    NetStreamInterface*                    pNetStreamInterface;
    Ptr<CharacterHandle>                   pTarget;
    Ptr<CharacterHandle>                   pAudioTarget;

    Array<WeakPtr<Video::VideoCharacter> > VideoCharacters;
    Video::VideoPlayer*                    pVideoPlayer;
    Video::VideoPlayer::VideoInfo          VInfo;
    Ptr<Video::VideoPlayer::SyncObject>    pSyncObj;
    float                                  FrameTime;
    Array<Video::VideoPlayer::CuePoint>    CuePoints;
    char*                                  pSubtitle;
    char*                                  pSubtitleTmp;

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
};


inline const char* NetStreamInterface::NotificationToString(Notification code)
{
    switch (code)
    {
    case PlayStart:          return "NetStream.Play.Start";
    case PlayStop:           return "NetStream.Play.Stop";
    case SeekNotify:         return "NetStream.Seek.Notify";
    case PlayStreamNotFound: return "NetStream.Play.StreamNotFound";
    case SeekInvalidTime:    return "NetStream.Seek.InvalidTime";
    case PlayInternalError:  return "NetStream.Play.InternalError";
    default:
        return "NetStream.Unknown";
    }
}

}}} // Scaleform::GFx::AS3

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_AS3PROVIDERNETSTREAM_H
