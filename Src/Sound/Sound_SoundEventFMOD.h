/**************************************************************************

PublicHeader:   Sound_FMOD
Filename    :   Sound_SoundEventFMOD.h
Content     :   FMOD Ex/Designer (based on SoundRender) implementation of SoundEvent
Created     :   March 2010
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GSOUNDEVENTFMOD_H
#define INC_GSOUNDEVENTFMOD_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Sound_SoundEvent.h"
#include "Sound_SoundRenderer.h"

#if defined(GFX_SOUND_FMOD)

#include "Kernel/SF_Array.h"
#include "GFx/GFx_StringHash.h"

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEventFMOD : public SoundEvent
{
public:
    SoundEventFMOD(const char* path = "") : Streaming(true), MediaPath(path) {}
    virtual ~SoundEventFMOD() {}

    virtual void PostEvent(String event, String eventId)
    {
        ReleaseStoppedSounds();

        if(!eventId.IsEmpty())
        {
            Ptr<SoundSample> pSample;
            Ptr<SoundChannel> pChannel;
            ChannelMap.Get(eventId, &pChannel);

            if(event.ToLower() == "play" && !pChannel)
            {
                if(!MediaPath.IsEmpty())
                    eventId = MediaPath + eventId;

                SF_ASSERT(pUserData);
                SoundRenderer *pSoundRenderer = (SoundRenderer *)pUserData;

                pSample  = *pSoundRenderer->CreateSampleFromFile(eventId, Streaming);
                pChannel = *pSoundRenderer->PlaySample(pSample, false);
                if(pChannel) {
                    Samples.PushBack(pSample);
                    ChannelMap.Set(eventId, pChannel);
                    pChannel->Pause(false);
                }
                return;
            }

            if(pChannel && pChannel->IsPlaying())
            {
                if(event.ToLower() == "pause")  pChannel->Pause(true);
                if(event.ToLower() == "resume") pChannel->Pause(false);
                if(event.ToLower() == "stop")   pChannel->Stop();
            }
        }
    }

    virtual void SetParam(String param, float paramValue, String eventId)
    {
        //ReleaseStoppedSounds();

        if(!eventId.IsEmpty())
        {
            Ptr<SoundChannel> pChannel;
            ChannelMap.Get(eventId, &pChannel);
            if(pChannel && pChannel->IsPlaying())
            {
                if(param.ToLower() == "volume") pChannel->SetVolume(paramValue);
                if(param.ToLower() == "pan")    pChannel->SetPan(paramValue);
            }
        }
    }

private:
    int FindSample(Array<Ptr<SoundSample> >& samples, SoundSample* psample)
    {
        for(UPInt i = 0; i < samples.GetSize(); ++i)
        {
            if(samples[i] == psample)
                return (int)i;
        }
        return -1;
    }

    void ReleaseStoppedSounds()
    {
        for(StringHash<Ptr<SoundChannel> >::ConstIterator it = ChannelMap.Begin();
            it != ChannelMap.End(); ++it)
        {
            Ptr<SoundChannel> pChannel = it->Second;
            if(!pChannel->IsPlaying())
            {
                Ptr<SoundSample> pSample = pChannel->GetSample();
                int idx = FindSample(Samples, pSample);
                if(idx != -1) {
                    Samples.RemoveAt(idx);
                    pSample = NULL;
                }
                ChannelMap.Remove(it->First);
                pChannel = NULL;
            }
        }
    }

    Array<Ptr<SoundSample> > Samples;
    StringHash<Ptr<SoundChannel> > ChannelMap;
    bool Streaming;
    String MediaPath;
};

}} // Scaleform::Sound

#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))

#include <fmod_event.hpp>
#include <fmod_event_net.hpp>

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEventFMODDesigner : public SoundEvent
{
public:
    SoundEventFMODDesigner(const char *proj, const char *group, const char* path = "") :
        pEventGroup(NULL), ProjectLoaded(false), Project(proj), Group(group), MediaPath(path) {}
    virtual ~SoundEventFMODDesigner() { StopAllSounds(); }

    virtual void PostEvent(String event, String eventId)
    {
        if(!ProjectLoaded)
        {
            SF_ASSERT(pUserData);
            FMOD::EventSystem *pEventSys = (FMOD::EventSystem *)pUserData;

            if(!MediaPath.IsEmpty())
                pEventSys->setMediaPath(MediaPath.ToCStr());
            pEventSys->load(Project.ToCStr(), 0, 0);

            int result = pEventSys->getGroup(Group.ToCStr(), FMOD_EVENT_DEFAULT, &pEventGroup);
            SF_ASSERT(result == FMOD_OK && pEventGroup);
            ProjectLoaded = true;
        }

        StopNotActiveSounds();

        if(!eventId.IsEmpty())
        {
            FMOD::Event *pEvent = NULL;
            EventMap.Get(eventId, &pEvent);

            if(event.ToLower() == "play" && !pEvent)
            {
                if(pEventGroup)
                {
                    int result = pEventGroup->getEvent(eventId.ToCStr(), FMOD_EVENT_DEFAULT, &pEvent);
                    if(result == FMOD_OK && pEvent) {
                        EventMap.Set(eventId, pEvent);
                        pEvent->start();
                    }
                }
                return;
            }

            if(pEvent)
            {
                if(event.ToLower() == "stop") {
                    pEvent->stop();
                    EventMap.Remove(eventId);
                }
                FMOD_EVENT_STATE eventState;
                int result = pEvent->getState(&eventState);
                if(result == FMOD_OK &&
                    (eventState & FMOD_EVENT_STATE_CHANNELSACTIVE) == FMOD_EVENT_STATE_CHANNELSACTIVE)
                {
                    if(event.ToLower() == "pause")  pEvent->setPaused(true);
                    if(event.ToLower() == "resume") pEvent->setPaused(false);
                    if(event.ToLower() == "mute")   pEvent->setMute(true);
                    if(event.ToLower() == "unmute") pEvent->setMute(false);
                }
            }
        }
    }

    virtual void SetParam(String param, float paramValue, String eventId)
    {
        if(!eventId.IsEmpty())
        {
            FMOD::Event *pEvent = NULL;
            EventMap.Get(eventId, &pEvent);
            if(pEvent)
            {
                FMOD_EVENT_STATE eventState;
                int result = pEvent->getState(&eventState);
                if(result == FMOD_OK &&
                    (eventState & FMOD_EVENT_STATE_CHANNELSACTIVE) == FMOD_EVENT_STATE_CHANNELSACTIVE)
                {
                    if(param.ToLower() == "volume") pEvent->setVolume(paramValue);
                    if(param.ToLower() == "pitch")  pEvent->setPitch(paramValue, FMOD_EVENT_PITCHUNITS_RAW);
                }
            }
        }
    }

private:
    void StopNotActiveSounds()
    {
        for(StringHash<FMOD::Event*>::ConstIterator it = EventMap.Begin();
            it != EventMap.End(); ++it)
        {
            FMOD::Event *pEvent = it->Second;
            FMOD_EVENT_STATE eventState;
            int result = pEvent->getState(&eventState);
            if(result == FMOD_OK &&
                (eventState & FMOD_EVENT_STATE_CHANNELSACTIVE) != FMOD_EVENT_STATE_CHANNELSACTIVE)
            {
                pEvent->stop();
                EventMap.Remove(it->First);
            }
        }
    }
    void StopAllSounds()
    {
        for(StringHash<FMOD::Event*>::ConstIterator it = EventMap.Begin();
            it != EventMap.End(); ++it)
        {
            FMOD::Event *pEvent = it->Second;
            pEvent->stop();
            EventMap.Remove(it->First);
        }
    }

    FMOD::EventGroup *pEventGroup;
    StringHash<FMOD::Event*> EventMap;

    bool ProjectLoaded;
    String Project;
    String Group;
    String MediaPath;
};

}} // Scaleform::Sound

#endif // GFX_SOUND_FMOD_DESIGNER
#endif // GFX_SOUND_FMOD

#endif // GFX_ENABLE_SOUND

#endif // INC_GSOUNDEVENTFMOD_H
