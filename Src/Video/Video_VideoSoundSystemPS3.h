/**************************************************************************

Filename    :   Video_VideoSoundSystemPS3.h
Content     :   Video sound system implementation for PS3 based on MultiStream
Created     :   March 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_SYSTEMSOUND_PS3_H
#define INC_GFX_VIDEO_SYSTEMSOUND_PS3_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_Video.h"
#include "Kernel/SF_MemoryHeap.h"

#include <cell/spurs.h>

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

// VideoSoundSystemPS3 is a sample video sound system implementation based on MultiStream
// used on the PlayStation3 platform. An instance of this class is usually created and passed
// to the Video::SetSoundSystem method as a part of GFx video initialization.

// Typically the cellAudio and MultiStream systems would already be initialized by the game; when
// that is the case, false should be passed for the 'initialize' argument in the constructor.
// Alternatively, developers can pass a value of 'true' and a CellSpurs pointer to have the 
// VideoSoundSystemPS3 initialize the PS3 sound system.

class VideoSoundSystemPS3 : public VideoSoundSystem
{
public:

    // Pass 'initialize' argument set to true to initialize cellAudio, MS,
    // and open 1 audio port for MS. pspurs argument needs to only be passed
    // if initialize is true; otherwise it can be null.
    VideoSoundSystemPS3(bool initialize, CellSpurs* pspurs,
                        MemoryHeap* pheap = Memory::GetGlobalHeap());
    ~VideoSoundSystemPS3();

    virtual VideoSound* Create();
}; 

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_SYSTEMSOUND_PS3_H
