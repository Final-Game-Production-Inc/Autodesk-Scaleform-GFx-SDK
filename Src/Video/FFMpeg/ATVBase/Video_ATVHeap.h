/**************************************************************************

Filename    :   Video_ATVHeap.h
Content     :   GFx video: FFmpeg memory heap and subsystem glue
Created     :   October 24th, 2026
Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_ATVHEAP_H
#define INC_GFX_VIDEO_ATVHEAP_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_MemoryHeap.h"

namespace Scaleform { namespace GFx { namespace Video {

// The FFmpeg (libav) video backend. FFmpeg does not expose a public global
// allocator override hook (av_malloc uses the CRT), so this module registers
// the Scaleform memory heap used for all FFmpeg-side staging buffers owned by
// the FFMpeg video player, installs the FFmpeg log callback and initializes
// the libavformat network/file IO subsystem.
//
// avHeap_Register must be called once (from Video::Initialize) before the
// first video player is created, and avHeap_Unregister from Video::Finalize.

void avHeap_Register(MemoryHeap* pheap);
void avHeap_Unregister();
MemoryHeap* avHeap_GetHeap();

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_ATVHEAP_H
