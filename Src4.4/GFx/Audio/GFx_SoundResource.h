/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_SoundResource.h
Content     :   GFx audio support
Created     :   September, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SOUNDRESOURCE_H
#define INC_SF_GFX_SOUNDRESOURCE_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_Sound.h"
#include "GFx/GFx_Resource.h"
#include "GFx/GFx_ResourceHandle.h"

namespace Scaleform { namespace GFx {

class FileOpener;

//////////////////////////////////////////////////////////////////////////
//

struct SoundFileInfo : public ResourceFileInfo
{
    Resource::ResourceUse Use;
    String      ExportName;
    unsigned    Bits;
    unsigned    Channels;
    unsigned    SampleRate;
    unsigned    SampleCount;
    unsigned    SeekSample;

    SoundFileInfo()
    {
        Use = Resource::Use_SoundSample;
    }
    SoundFileInfo(const SoundFileInfo& other) : ResourceFileInfo(other)
    {
        Use = other.Use;
        ExportName = other.ExportName;
        Bits = other.Bits;
        Channels = other.Channels;
        SampleRate = other.SampleRate;
        SampleCount = other.SampleCount;
        SeekSample = other.SeekSample;
    }
};

//////////////////////////////////////////////////////////////////////////
//

class SoundResource : public Resource
{
protected:
    Ptr<Sound::SoundInfoBase> pSoundInfo;
    // Key used to look up / resolve this object (contains file data).
    ResourceKey Key;
    // What the resource is used for
    ResourceUse UseType;
    int         PlayingCount;

public:
    SoundResource(Sound::SoundInfoBase* psound, ResourceUse use = Use_SoundSample)
    {
        pSoundInfo   = psound;
        UseType      = use;
        PlayingCount = 0;
    }
    SoundResource(Sound::SoundInfoBase* psound, const ResourceKey& key, ResourceUse use = Use_SoundSample)
    {
        pSoundInfo   = psound;
        Key          = key;
        UseType      = use;
        PlayingCount = 0;
    }

    bool                IsPlaying() const  { return PlayingCount > 0; }
    void                IncPlayingCount()  { PlayingCount++; }
    void                DecPlayingCount()  { PlayingCount--; SF_ASSERT(PlayingCount>=0); }

    // Referenced image accessors.
    inline Sound::SoundInfoBase* GetSoundInfo() const
    {
        return pSoundInfo.GetPtr();
    }
    inline void SetSoundInfo(Sound::SoundInfoBase* psoundInfo)
    {
        pSoundInfo = psoundInfo;
    }

    // GImplement Resource interface.
    virtual ResourceKey GetKey()                    { return Key; }
    virtual unsigned    GetResourceTypeCode() const { return MakeTypeCode(RT_SoundSample, UseType); }

    // Create a key for an image file.
    static  ResourceKey CreateSoundFileKey(SoundFileInfo* pfileInfo, FileOpener* pfileOpener);
};

//////////////////////////////////////////////////////////////////////////
//

class SoundResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool CreateResource(DataHandle hdata, ResourceBindData *pbindData,
        LoadStates *plp, MemoryHeap* pbindHeap) const;

public:
	static ResourceData CreateSoundResourceData(Sound::SoundData* pimage);
};

class SoundFileResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool CreateResource(DataHandle hdata, ResourceBindData *pbindData,
        LoadStates *plp, MemoryHeap* pbindHeap) const;

public:
    static  ResourceData CreateSoundFileResourceData(SoundFileInfo * prfi);
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND

#endif // INC_SF_GFX_SOUNDRESOURCE_H
