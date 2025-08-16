/**************************************************************************

Filename    :   Video_VideoSoundSystemWwise.h
Content     :   Video sound system implementation based on AK Wwise sound system
Created     :   August 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_WWISE_H
#define INC_GFX_VIDEO_SYSTEMSOUND_WWISE_H

#include "GFxConfig.h"
#if defined(GFX_ENABLE_VIDEO) && defined(GFX_VIDEO_USE_WWISE)

#include "Video/Video_Video.h"
#include "Kernel/SF_MemoryHeap.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystemWwise is a sample video sound system implementation based on
// Audiokinetic Wwise SDK used on Windows and consoles. An instance of this class is
// usually created and passed to the Video::SetSoundSystem method as a part of
// GFx video initialization.

class VideoSoundSystemWwise : public VideoSoundSystem
{
public:
    VideoSoundSystemWwise(MemoryHeap* pheap = Memory::GetGlobalHeap());
    ~VideoSoundSystemWwise();

    virtual VideoSound* Create();
    virtual void Update();

private:
    class VideoSoundSystemWwiseImpl* pImpl;
}; 

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_WWISE_H
