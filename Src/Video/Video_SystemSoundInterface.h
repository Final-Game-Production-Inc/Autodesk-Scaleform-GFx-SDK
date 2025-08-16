/**************************************************************************

Filename    :   Video_SystemSoundInterface.h
Content     :   Video system sound interface
Created     :   October 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_H
#define INC_GFX_VIDEO_SYSTEMSOUND_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Sound/Sound_SoundRenderer.h"

namespace Scaleform { 

#ifdef GFX_ENABLE_SOUND

namespace GFx { namespace Video {
class VideoSoundStream;
}}

namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundChannelSystem : public SoundChannel
{
public:
    SoundChannelSystem(GFx::Video::VideoSoundStream* pinter)
    {
        pInterface = pinter;
        Volume = 1.0f;
    }

    virtual SoundRenderer* GetSoundRenderer() const { return 0; }
    virtual SoundSample*   GetSample() const { return 0; }

    virtual void    Stop();
    virtual void    Pause(bool) {};
    virtual bool    IsPlaying() const ;
    virtual void    SetPosition(float) {}
    virtual float   GetPosition() { return 0.0f; }
    virtual void    Loop(int, float, float) {}
    virtual float   GetVolume() { return Volume; }
    virtual void    SetVolume(float volume);
    virtual float   GetPan() { return 0.0f; }
    virtual void    SetPan(float) {}
    virtual void    SetTransforms(const Array<Transform>&) {}
    virtual void    SetSpatialInfo(const Array<Vector> spatinfo[]);

    void DetachVideoSoundStream() { pInterface = NULL; }

private:
    GFx::Video::VideoSoundStream* pInterface;
    float                         Volume;
};

} // Sound

#endif

namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoSoundStream : public VideoSound
{
public:
    VideoSoundStream();
    virtual ~VideoSoundStream();

    virtual void  SetVolume(float volume) = 0;
    virtual float GetVolume() = 0;

#ifdef GFX_ENABLE_SOUND
    virtual Sound::SoundChannel* GetSoundChannel();
    virtual void SetSpatialInfo(const Array<Sound::SoundChannel::Vector> []) {}

protected:
    Ptr<Sound::SoundChannelSystem> pChannel;
#endif
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_H
