/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Audio.h
Content     :   GFx audio support
Created     :   September 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_AUDIO_H
#define INC_SF_GFX_AUDIO_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/GFx_Loader.h"
#include "Sound/Sound_Sound.h"
#include "Sound/Sound_SoundRenderer.h"

namespace Scaleform { namespace GFx {

namespace AS2 {
    class GlobalContext;
    class ASStringContext;
}
class SoundTagsReader;

//
// ***** AudioBase
//
// The purpose of the state it to provide the instance of SoundRenderer to the 
// rest of GFx library and to set synchronization parameters for SWF streaming sound.
//
// AudioBase is an abstract class separate from Audio to avoid audio linking
// for projects that don't use built-in audio.
//
class AudioBase : public State
{
public:
    enum SyncTypeFlags
    {       
        NoSync      = 0x00, // audio and video frames do not need to be synchronized.
        VideoMaster = 0x01, // audio frames should be synchronized to video frames.
        AudioMaster = 0x02  // video frames should be synchronized to audio frames (SWF behavior).
    };

    AudioBase(
        float maxTimeDiffernce = 100/1000.f, unsigned checkFrameInterval = 15, SyncTypeFlags syncType = AudioMaster) :
        State(State_Audio), MaxTimeDifference(maxTimeDiffernce), CheckFrameInterval(checkFrameInterval), SyncType(syncType) {}
    virtual ~AudioBase() {}

    inline void     SetMaxTimeDifference(float maxTimeDifference)  { MaxTimeDifference = maxTimeDifference; }
    inline float    GetMaxTimeDifference() const                   { return MaxTimeDifference; }
    inline void     SetCheckFrameInterval(unsigned checkFrameInterval) { CheckFrameInterval = checkFrameInterval; }
    inline unsigned GetCheckFrameInterval() const                  { return CheckFrameInterval; }

    inline void             SetSyncType(SyncTypeFlags syncType)    { SyncType = syncType; }
    inline SyncTypeFlags    GetSyncType() const                    { return SyncType; }

    virtual Sound::SoundRenderer* GetRenderer() const = 0;

    // Internally used APIs.
    virtual SoundTagsReader* GetSoundTagsReader() const  = 0;
    virtual void RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc) = 0;

protected:
    float           MaxTimeDifference;  // max allowed difference between video frame time and audio frame time
    unsigned        CheckFrameInterval; // how often the sync check should be performed
    SyncTypeFlags   SyncType;
};

//
// ***** Audio
//
// The implementation of AudioBase interface
//
// A separate implementation is provided so that Sound classes can be compiled
// out completely from GFx core and the resultant application executable
//
//
class Audio : public AudioBase
{ 
public:
    Audio(Sound::SoundRenderer* pplayer, float maxTimeDiffernce = 100/1000.f, 
          unsigned checkFrameInterval = 15, SyncTypeFlags syncType = AudioMaster);
    ~Audio();

    virtual Sound::SoundRenderer* GetRenderer() const;
    virtual SoundTagsReader* GetSoundTagsReader() const;

    virtual void RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc);

protected:
    Ptr<Sound::SoundRenderer> pPlayer;
    SoundTagsReader* pTagsReader;
};

inline void StateBag::SetAudio(AudioBase* ptr) 
{ 
    SetState(State::State_Audio, ptr); 
}

inline Ptr<AudioBase> StateBag::GetAudio() const 
{ 
    return *(AudioBase*) GetStateAddRef(State::State_Audio); 
}

inline Sound::SoundRenderer* StateBag::GetSoundRenderer() const
{
    Ptr<AudioBase> p = GetAudio();
    return p ? p->GetRenderer() : 0;
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND

#endif // INC_SF_GFX_AUDIO_H
