/**************************************************************************

Filename    :   Video_CriMvSoundGateway.cpp
Content     :   Video sound data gateway
Created     :   July 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Video/Video_CriMvSoundGateway.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

Bool CriMvSound::CreateOutput(CriHeap heap, Uint32 channel, Uint32 samplerate)
{
    SF_UNUSED(heap);
    if (pVSInterface)
        return (Bool)pVSInterface->CreateOutput(channel, samplerate);
    return FALSE;
}

void CriMvSound::DestroyOutput(void)
{
    if (pVSInterface)
        return pVSInterface->DestroyOutput();
}

CriMvSoundInterface::PcmFormat CriMvSound::GetPcmFormat(void)
{
    if (!pVSInterface || pVSInterface->GetPCMFormat() == VideoSound::PCM_Float)
        return CriMvSoundInterface::MVEASY_PCM_FLOAT32;
    return CriMvSoundInterface::MVEASY_PCM_SINT16;
}

void CriMvSound::Start(void)
{
    if (pVSInterface)
        pVSInterface->Start(&StreamImpl);
}

void CriMvSound::Stop(void)
{
    if (pVSInterface)
        pVSInterface->Stop();
}

void CriMvSound::Pause(Bool sw)
{
    if (pVSInterface)
        pVSInterface->Pause(sw == TRUE);
}

CriMvSoundInterface::Status CriMvSound::GetStatus(void)
{
    if (pVSInterface)
    {
        switch(pVSInterface->GetStatus())
        {
        case VideoSound::Sound_Playing:
            return CriMvSoundInterface::MVEASY_SOUND_STATUS_EXEC;
        case VideoSound::Sound_Stopped:
        case VideoSound::Sound_Paused:
            return CriMvSoundInterface::MVEASY_SOUND_STATUS_STOP;
        case VideoSound::Sound_Error:
        default:
            return CriMvSoundInterface::MVEASY_SOUND_STATUS_ERROR;
        }
    }
    return CriMvSoundInterface::MVEASY_SOUND_STATUS_STOP;
}

void CriMvSound::GetTime(Uint64 &count, Uint64 &unit)
{
    if (pVSInterface)
        pVSInterface->GetTime(&count, &unit);
}

#ifdef GFX_ENABLE_SOUND
Sound::SoundChannel* CriMvSound::GetSoundChannel()
{
    if(pVSInterface)
        return pVSInterface->GetSoundChannel();
    return NULL;
}
#endif

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
