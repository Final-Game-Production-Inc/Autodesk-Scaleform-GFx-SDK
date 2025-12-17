/**************************************************************************

Filename    :   GFx_SoundTagsReader.h
Content     :   GFx audio support
Created     :   February, 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SOUNDTAGSREADER_H
#define INC_SF_GFX_SOUNDTAGSREADER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_Sound.h"

namespace Scaleform { namespace GFx {

class LoadProcess;
class ButtonSoundDef;

//////////////////////////////////////////////////////////////////////////
//

class SoundTagsReader : public RefCountBaseNTS<SoundTagsReader,StatMD_Sounds_Mem>
{
public:
    virtual ButtonSoundDef* ReadButtonSoundDef(LoadProcess* p);
    virtual void ReadDefineSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadStartSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadButtonSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadDefineExternalSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadDefineExternalStreamSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadSoundStreamHeadTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadSoundStreamBlockTag(LoadProcess* p, const TagInfo& tagInfo);
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND

#endif // INC_SF_GFX_SOUNDTAGSREADER_H
