/**************************************************************************

Filename    :   Video_VideoSoundSystemXA2.h
Content     :   Video sound system implementation based on XAudio2
Created     :   March 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_XA2_H
#define INC_GFX_VIDEO_SYSTEMSOUND_XA2_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Kernel/SF_MemoryHeap.h"
#include <XAudio2.h>

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystemXA2 is a sample video sound system implementation based on XAudio2
// used on Windows and XBox360 platforms. An instance of this class is usually created and
// passed to the Video::SetSoundSystem method as a part of GFx video initialization.

// Typically the XAudio 2 object would already be created by the game; when that is
// the case, proper IXAudio2 and IXAudio2MasteringVoice pointers need to be passed in the
// constructor. Alternatively, developers can pass null pointers for both of those 
// arguments to have the VideoSoundSystemXA2 create a new XAudio2 instance.

class VideoSoundSystemXA2 : public VideoSoundSystem
{
public:
    // Initialize the VideoSound system based on an existing XAudio2 interface.
    // If the passed 'pxa2' argument is null, a new XAudio2 instance will be created;
    // in that case pmasteringVoice should also be null.
    VideoSoundSystemXA2(IXAudio2* pxa2, IXAudio2MasteringVoice* pmasteringVoice, 
                        MemoryHeap* pheap = Memory::GetGlobalHeap());
    ~VideoSoundSystemXA2();

    virtual VideoSound* Create();

private:
    class VideoSoundSystemXA2Impl* pImpl;
}; 

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_XA2_H
