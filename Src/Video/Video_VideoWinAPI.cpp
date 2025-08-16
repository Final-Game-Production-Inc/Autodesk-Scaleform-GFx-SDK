/**************************************************************************

Filename    :   Video_VideoWinAPI.cpp
Content     :    GFx video for Windows based PC
Created     :   Sep 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoWinAPI.h"

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif

#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIN32)

#include "Kernel/SF_MemoryHeap.h"
#include "Video/Video_VideoPlayerImpl.h"

namespace Scaleform { namespace GFx { namespace Video {

VideoWin32::VideoWin32(const VideoVMSupport& vmSupport, unsigned decodingProcs,
                           Thread::ThreadPriority decodeThreadsPriority, Win32Proc readerProc) :
    Video(vmSupport, decodeThreadsPriority),
    DecoderProcs(decodingProcs), ReaderProc(readerProc)
{
}

/////Unfinished


}}} // Scaleform::GFx::Video