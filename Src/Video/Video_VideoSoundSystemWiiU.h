/**************************************************************************

Filename    :   Video_VideoSoundSystemWiiU.h
Content     :   Video sound system implementation for WiiU
Created     :   February 2012
Authors     :   Vladislav Merker

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_WIIU_H
#define INC_GFX_VIDEO_SYSTEMSOUND_WIIU_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Kernel/SF_MemoryHeap.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystemWiiU is a video sound system implementation for WiiU.
// An instance of this class is usually created and passed to Video::SetSoundSystem
// method as a part of GFx video initialization.

// Typically the WiiU AI/AX/MIX sound APIs will already be initialized by the game;
// when that is the case, 'false' should be passed for the initialize constructor parameter.
// An argument value of 'true' can be passed to have the VideoSoundSystem initalize
// Wii sound APIs.

class VideoSoundSystemWiiU : public VideoSoundSystem
{
public:
    VideoSoundSystemWiiU(bool init, MemoryHeap* pheap = Memory::GetGlobalHeap());

    virtual VideoSound* Create();
}; 

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_WIIU_H
