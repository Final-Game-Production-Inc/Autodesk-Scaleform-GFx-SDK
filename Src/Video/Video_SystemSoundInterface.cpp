/**************************************************************************

Filename    :   Video_SystemSoundInterface.cpp
Content     :   Video system sound interface
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_SystemSoundInterface.h"

namespace Scaleform {
    
namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoSoundStream::VideoSoundStream()
{
#ifdef GFX_ENABLE_SOUND
    pChannel = *new Sound::SoundChannelSystem(this);
#endif
}

VideoSoundStream::~VideoSoundStream()
{
#ifdef GFX_ENABLE_SOUND
    pChannel->DetachVideoSoundStream();
#endif
}

#ifdef GFX_ENABLE_SOUND
Sound::SoundChannel* VideoSoundStream::GetSoundChannel()
{
    return pChannel.GetPtr();
}
#endif

}} // GFx::Video

namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

#ifdef GFX_ENABLE_SOUND
void SoundChannelSystem::Stop()
{
    if (pInterface)
        pInterface->Stop();
}

bool SoundChannelSystem::IsPlaying() const
{
    return pInterface != NULL;
}

void SoundChannelSystem::SetVolume(float volume)
{
    Volume = volume;
    if (pInterface)
        pInterface->SetVolume(volume);
}

void SoundChannelSystem::SetSpatialInfo(const Array<Vector> spatinfo[])
{
    if(pInterface)
        pInterface->SetSpatialInfo(spatinfo);
}

#endif // GFX_ENABLE_SOUND

} // Sound

} // Scaleform

#endif // GFX_ENABLE_VIDEO
