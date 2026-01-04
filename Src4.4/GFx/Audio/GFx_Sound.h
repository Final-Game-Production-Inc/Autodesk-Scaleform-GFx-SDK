/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Sound.h
Content     :   GFx audio support
Created     :   September 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SOUND_H
#define INC_SF_GFX_SOUND_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/GFx_Audio.h"
#include "GFx/GFx_Resource.h"
#include "GFx/GFx_ResourceHandle.h"
#include "GFx/Audio/GFx_SoundResource.h"

namespace Scaleform { namespace GFx {

//class Stream;
class LoadProcess;
class MovieImpl;
class Sprite;
class SoundResource;
class Audio;

//////////////////////////////////////////////////////////////////////////
//

class SoundEnvelope
{
public:
    UInt32 Mark44;
    UInt16 Level0;
    UInt16 Level1;
};

class SoundStyle
{
public:
    bool   NoMultiple;
    bool   StopPlayback;
    bool   HasEnvelope;
    bool   HasLoops;
    bool   HasOutPoint;
    bool   HasInPoint;
    UInt32 InPoint;
    UInt32 OutPoint;
    UInt16 LoopCount;

    Array<SoundEnvelope> Envelopes;

    void Read(Stream* in);
};

Sound::SoundData* GFx_ReadSoundData(LoadProcess* p, UInt16 characterId);

class SoundInfo
{
public:
    SoundInfo();
    void Play(InteractiveObject *pchar);

    ResourcePtr<SoundResource> pSample;
    SoundStyle Style;
    ResourceId SoundId;
};

//////////////////////////////////////////////////////////////////////////
//

class ButtonSoundDef : public NewOverrideBase<StatMD_CharDefs_Mem>
{
public:
    virtual ~ButtonSoundDef() {};
    virtual void Play(InteractiveObject *pchar, int buttonIdx) = 0;
};

class SoundStreamDef : public RefCountBaseNTS<SoundStreamDef,StatMD_Sounds_Mem>
{
public:
    virtual ~SoundStreamDef() {}
    virtual bool ProcessSwfFrame(MovieImpl* proot, unsigned frame, Sprite* pchar) = 0;
};

class ButtonSoundDefImpl : public ButtonSoundDef
{
public:
    virtual ~ButtonSoundDefImpl() {};
    virtual void Play(InteractiveObject *pchar, int buttonIdx);

    SoundInfo ButtonSounds[4];
};

class SoundStreamDefImpl : public SoundStreamDef
{
public:
    unsigned PlaybackSoundRate;
    unsigned PlaybackSound16bit;
    unsigned PlaybackStereo;
    unsigned SoundCompression;
    unsigned StreamSoundRate;
    unsigned StreamSound16bit;
    unsigned StreamStereo;
    unsigned StreamSampleCount;
    unsigned LatencySeek;

    unsigned StartFrame;
    unsigned LastFrame;

	Ptr<Sound::SoundInfo> pSoundInfo;

    SoundStreamDefImpl();
    ~SoundStreamDefImpl();

    void ReadHeadTag(Stream* in);
    void ReadBlockTag(LoadProcess* p);
    
    virtual bool ProcessSwfFrame(MovieImpl* proot, unsigned frame, Sprite* pchar);
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND

#endif // INC_SF_GFX_SOUND_H
