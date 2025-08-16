/**************************************************************************

Filename    :   Video_VideoSoundSystemFMOD.h
Content     :   Video sound system implementation based on FMOD sound library
Created     :   July 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_FMOD_H
#define INC_GFX_VIDEO_SYSTEMSOUND_FMOD_H

#include "GFxConfig.h"
#if defined(GFX_ENABLE_VIDEO) && defined(GFX_VIDEO_USE_FMOD)

#include "Video/Video_Video.h"
#include "Kernel/SF_MemoryHeap.h"

#ifdef SF_OS_PS3
#include <cell/spurs/types.h>
#endif

namespace FMOD
{
    class System;
}

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystemFMOD is a sample video sound system implementation based on FMOD sound
// library and can be used on all supported platform. An instance of this class is usually
// created and passed to the Video::SetSoundSystem method as a part of GFx video initialization.

// Typically the FMOD::System object would already be created by the game; when that is
// the case, proper FMOD::System pointer need to be passed in the constructor.
// Alternatively, developers can pass null pointers for this argument to have
// the VideoSoundSystemFMOD create a new FMOD::System instance. On PS3 for creating
// the new FMOD::System instance additional parameter is needed: initialized CellSpurs instance.

class VideoSoundSystemFMOD : public VideoSoundSystem
{
public:
    // Initialize the VideoSound system based on an existing FMOD::System object.
    // If the passed 'pfmod' argument is null, a new FMOD::System instance will be created;
#ifdef SF_OS_PS3
    VideoSoundSystemFMOD(FMOD::System* pfmod, const CellSpurs* pSpurs, 
                         MemoryHeap* pheap = Memory::GetGlobalHeap());
#else
    VideoSoundSystemFMOD(FMOD::System* pfmod, 
                         MemoryHeap* pheap = Memory::GetGlobalHeap());
#endif
    ~VideoSoundSystemFMOD();

    virtual VideoSound* Create();

private:
    class VideoSoundSystemFMODImpl* pImpl;
}; 

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_FMOD_H
