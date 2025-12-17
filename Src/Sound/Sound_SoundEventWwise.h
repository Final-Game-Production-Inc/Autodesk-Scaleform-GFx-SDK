/**************************************************************************

PublicHeader:   Sound_WWise
Filename    :   Sound_SoundEventWwise.h
Content     :   AK Wwise/SoundFrame implementation of SoundEvent
Created     :   March 2010
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GSOUNDEVENTWWISE_H
#define INC_GSOUNDEVENTWWISE_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Sound_SoundEvent.h"

#if defined(GFX_SOUND_WWISE)

#include <AK/AkWwiseSDKVersion.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

#if defined(SF_OS_WIN32)

#include <AK/SoundFrame/SF.h>
#include "Kernel/SF_UTF8Util.h"

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEventWwiseSF : public SoundEvent, public AK::SoundFrame::IClient
{
public:
    SoundEventWwiseSF() : pSoundFrame(NULL)
    {
        SF_ASSERT(AK::SoundEngine::IsInitialized());

        AK::SoundFrame::Create(this, &pSoundFrame);
        SF_ASSERT(pSoundFrame);
    }
    virtual ~SoundEventWwiseSF()
    {
        if(pSoundFrame)
            pSoundFrame->Release();
    }

    virtual void PostEvent(String event, String eventId = "")
    {
        SF_UNUSED(eventId);

        if(pSoundFrame && pSoundFrame->IsConnected())
        {
            UTF8Util::DecodeString(pWcharBuf, event.ToCStr(), WcharBufSize);
            LPCWSTR eventNameW = pWcharBuf;
            pSoundFrame->PlayEvents(&eventNameW, 1);
        }
    }

    virtual void SetParam(String param, float paramValue, String eventId = "")
    {
        SF_UNUSED(eventId);

        if(pSoundFrame && pSoundFrame->IsConnected())
        {
            UTF8Util::DecodeString(pWcharBuf, param.ToCStr(), WcharBufSize);
            LPCWSTR paramNameW = pWcharBuf;
            pSoundFrame->SetRTPCValue(
                paramNameW, (AkRtpcValue)paramValue,
                AK::SoundFrame::IGameObject::s_InvalidGameObject);
        }
    }

    virtual void OnConnect            (bool) {}
    virtual void OnEventNotif         (Notif, AkUniqueID) {}
    virtual void OnSoundObjectNotif   (Notif, AkUniqueID) {}
    virtual void OnStatesNotif        (Notif, AkUniqueID) {}
    virtual void OnSwitchesNotif      (Notif, AkUniqueID) {}
    virtual void OnGameParametersNotif(Notif, AkUniqueID) {}
    virtual void OnTriggersNotif      (Notif, AkUniqueID) {}
    virtual void OnEnvironmentsNotif  (Notif, AkUniqueID) {}
    virtual void OnGameObjectsNotif   (Notif, AkGameObjectID) {}

#if (AK_WWISESDK_VERSION_BUILD >= 3887)
    virtual void OnDialogueEventNotif (Notif, AkUniqueID) {}
    virtual void OnArgumentsNotif     (Notif, AkUniqueID) {}
#endif

private:
    static const UInt8 WcharBufSize = 128;
    wchar_t pWcharBuf[WcharBufSize];

    AK::SoundFrame::ISoundFrame* pSoundFrame;
};

}} // Scaleform::Sound

#endif // SF_OS_WIN32

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEventWwise : public SoundEvent
{
public:
    SoundEventWwise() : ObjectID(AK_INVALID_GAME_OBJECT), PlayingID(AK_INVALID_PLAYING_ID)
    {
        SF_ASSERT(AK::SoundEngine::IsInitialized());

        ObjectID = (AkGameObjectID)this;
        AKRESULT res = AK::SoundEngine::RegisterGameObj(ObjectID);
        SF_ASSERT(res == AK_Success);
        SF_UNUSED(res);
	}
    SoundEventWwise(AkGameObjectID objectId) : ObjectID(objectId), PlayingID(AK_INVALID_PLAYING_ID)
    {
        SF_ASSERT(AK::SoundEngine::IsInitialized());

        AKRESULT res = AK::SoundEngine::RegisterGameObj(ObjectID);
        SF_ASSERT(res == AK_Success);
        SF_UNUSED(res);
    }
    virtual ~SoundEventWwise()
    {
        if(ObjectID != AK_INVALID_GAME_OBJECT) {
            AKRESULT res = AK::SoundEngine::UnregisterGameObj(ObjectID);
            SF_ASSERT(res == AK_Success);
            SF_UNUSED(res);
        }
    }

    virtual void PostEvent(String event, String eventId = "")
    {
        SF_UNUSED(eventId);
        PlayingID = AK::SoundEngine::PostEvent(event.ToCStr(), ObjectID);
        SF_ASSERT(PlayingID != AK_INVALID_PLAYING_ID);
    }

    virtual void SetParam(String param, float paramValue, String eventId = "")
    {
        SF_UNUSED(eventId);
        AK::SoundEngine::SetRTPCValue(param.ToCStr(), (AkRtpcValue)paramValue, ObjectID);
    }

private:
    AkGameObjectID ObjectID;
    AkPlayingID    PlayingID;
};

}} // Scaleform::Sound

#endif // GFC_SOUND_WWISE

#endif // GFX_ENABLE_SOUND

#endif // INC_GSOUNDEVENTWWISE_H
