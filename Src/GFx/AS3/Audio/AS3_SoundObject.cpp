/**************************************************************************

Filename    :   AS3_SoundObject.cpp
Content     :   GFx audio: AS3 sound
Created     :   March 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_Sound.h"

#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Audio/AS3_SoundObject.h"
#include "GFx/AS3/Obj/Media/AS3_Obj_Media_Sound.h"
#include "GFx/AS3/Obj/Media/AS3_Obj_Media_SoundChannel.h"

namespace Scaleform { namespace GFx { namespace AS3 {

//////////////////////////////////////////////////////////////////////////
//

SoundObject::SoundObject(ASVM& asvm, Instances::fl_media::Sound* psound) : Volume(100), Pan(0)
{
    pMovieRoot = asvm.GetMovieImpl();
    DisplayObjContainer* pobj = asvm.GetMovieRoot()->GetMainTimeline();
    if (pobj && pobj->IsSprite())
        AttachToTarget(pobj->CharToSprite());
    pSound = psound;
}
SoundObject::~SoundObject()
{
    DetachFromTarget();
}

void SoundObject::AttachToTarget(Sprite *psprite)
{
    SF_ASSERT(psprite);
    pTargetHandle = psprite->GetCharacterHandle();
    psprite->AttachSoundObject(this);
}

void SoundObject::DetachFromTarget()
{
    if(!pTargetHandle) return;
    Sprite* psprite = GetSprite();
    if (psprite)
        psprite->DetachSoundObject(this);
}

Sprite* SoundObject::GetSprite()
{
    SF_ASSERT(pTargetHandle);
    SF_ASSERT(pMovieRoot);

    DisplayObject* ptarget = pTargetHandle->ResolveCharacter(pMovieRoot);
    if (!ptarget) return NULL;

    SF_ASSERT(ptarget->IsSprite());
    return ptarget->CharToSprite();
}

void SoundObject::ReleaseTarget()
{
    pTargetHandle = NULL;
    pSound = NULL;
}

void SoundObject::ExecuteOnSoundComplete()
{
    if (pSound)
        pSound->DispatchEventSoundComplete();
}

void* SoundObject::GetOwner()
{
    return (pSound ? pSound->pMovieDef : NULL);
}

//////////////////////////////////////////////////////////////////////////
//

void SoundObject::AttachResource(SoundResource* presource)
{
    Sound::SoundRenderer* pplayer = pMovieRoot->GetSoundRenderer();
    if (!pplayer) return;

    if (presource && presource->GetResourceType() == Resource::RT_SoundSample)
    {
        pResource = presource;
        pSample = pResource->GetSoundInfo()->GetSoundSample(pplayer);
    }
}

void SoundObject::LoadFile(const String& url, bool streaming)
{ 
    Sprite* psprite = GetSprite();
    if (!psprite) return;
    Sound::SoundRenderer* pplayer = pMovieRoot->GetSoundRenderer();
    if (!pplayer) return;

    if (pSound)
        pSound->DispatchEventOpen();

    Ptr<LoadStates> ploadStates = *SF_NEW LoadStates(
        pMovieRoot->pMainMovieDef->pLoaderImpl,
        pMovieRoot->pStateBag);

    String level0Path;
    pMovieRoot->GetMainMoviePath(&level0Path);
    URLBuilder::LocationInfo locInfo(URLBuilder::File_Regular, url, level0Path);
    String fileName;
    ploadStates->BuildURL(&fileName, locInfo);

    Ptr<Sound::SoundSample> psample = *pplayer->CreateSampleFromFile(fileName.ToCStr(), streaming);
    if (!psample)
    {
        if (pSound)
            pSound->DispatchEventIOError();
        return;
    }

    pSample = psample;
    pResource = NULL;

    if (pSound)
        pSound->DispatchEventComplete();
}

void SoundObject::Play(int startTime, int loops)
{
    Sprite* psprite = GetSprite();
    if (!psprite) return;
    Sound::SoundRenderer* pplayer = pMovieRoot->GetSoundRenderer();
    if (!pplayer) return;

    if (loops == 0)
        loops = 1;

    if (pSample)
    {
        Ptr<Sound::SoundChannel> pchan = *pplayer->PlaySample(pSample, true);
        if (pchan)
        {
            if (startTime > 0 || loops > 0)
                pchan->Loop(loops, startTime/1000.f);

            Volume = psprite->GetSoundVolume();
            pchan->SetVolume(Volume/100.f);
            Pan = psprite->GetSoundPan();
            pchan->SetPan(Pan/100.f);

            pchan->Pause(false);
            psprite->AddActiveSound(pchan, this, pResource);
        }
    }
}

void SoundObject::Stop()
{
    Sprite* psprite = GetSprite();
    if (psprite)
    {
        if (pResource)
            psprite->StopActiveSounds(pResource);
        else
            psprite->StopActiveSounds(this);
    }
}

void SoundObject::Close()
{
    pSound = NULL;
}

void SoundObject::SetVolume(int volume)
{ 
    Volume = volume;
    Sprite* psprite = GetSprite();
    if (psprite)
        psprite->UpdateActiveSoundVolume();
}

int SoundObject::GetVolume()
{
    return Volume;
}

void SoundObject::SetPan(int pan)
{ 
    Pan = pan;
    Sprite* psprite = GetSprite();
    if (psprite)
        psprite->UpdateActiveSoundPan();
}

int SoundObject::GetPan()
{
    return Pan; 
}

//////////////////////////////////////////////////////////////////////////
//

float SoundObject::GetPosition()
{
    float position = 0.0f;
    Sprite* psprite = GetSprite();
    if (psprite)
        position = psprite->GetActiveSoundPosition(this) * 1000;
    return position;
}

float SoundObject::GetDuration()
{
    float duration = 0.0f;
    if (pSample)
        duration = pSample->GetDuration() * 1000;
    return duration;
}

unsigned SoundObject::GetBytesLoaded()
{
    int bytesLoaded = 0;
    if (pSample)
        bytesLoaded = pSample->GetBytesLoaded();
    return bytesLoaded;
}

unsigned SoundObject::GetBytesTotal()
{ 
    int bytesTotal = 0;
    if (pSample)
        bytesTotal = pSample->GetBytesTotal();
    return bytesTotal;
}

}}} // namespace Scaleform::GFx::AS3

#endif // GFX_ENABLE_SOUND
