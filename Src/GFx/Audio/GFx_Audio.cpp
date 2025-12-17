/**************************************************************************

Filename    :   GFx_Audio.cpp
Content     :   GFx audio support
Created     :   September 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/Audio/AS2_SoundObject.h"
#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_SoundTagsReader.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

Audio::Audio(
    Sound::SoundRenderer* pplayer, float maxTimeDiffernce, unsigned checkFrameInterval, SyncTypeFlags syncType) :
    AudioBase(maxTimeDiffernce, checkFrameInterval, syncType), pPlayer(pplayer), pTagsReader(NULL)
{
    pTagsReader = SF_NEW SoundTagsReader;
}

Audio::~Audio()
{
    if (pTagsReader)
        pTagsReader->Release();
}

Sound::SoundRenderer* Audio::GetRenderer() const 
{ 
    return pPlayer.GetPtr(); 
}

SoundTagsReader* Audio::GetSoundTagsReader() const
{
    return pTagsReader;
}

void Audio::RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc)
{
#ifdef GFX_AS2_SUPPORT
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_Sound, AS2::SoundCtorFunction>(sc, gc.pGlobal);
#else
    SF_UNUSED2(gc, sc);
#endif
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND
