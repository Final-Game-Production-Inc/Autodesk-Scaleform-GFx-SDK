/**************************************************************************

Filename    :   Video_VideoPC.cpp
Content     :   GFx video for Windows based PC
Created     :   Sep 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoPC.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIN32)

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_movie.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_MemoryHeap.h"
#include "Video/Video_VideoPlayerImpl.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoPC::VideoPC(const VideoVMSupport& vmSupport, Thread::ThreadPriority decodingThreadsPriority,
                 int decodingThreadsNumber, UInt32* affinityMask) : Video(vmSupport, decodingThreadsPriority)
{
    DecodingThreadNumber = decodingThreadsNumber > MAX_VIDEO_DECODING_THREADS
        ? MAX_VIDEO_DECODING_THREADS : decodingThreadsNumber;
    for(int i = 0 ; i < MAX_VIDEO_DECODING_THREADS; ++i)
    {
        AffinityMask[i] = affinityMask && i < DecodingThreadNumber
            ? affinityMask[i] : CRIMV_DEFAULT_AFFNITY_MASK_PC;
    }
}

void VideoPC::ApplySystemSettings(VideoPlayer* pvideoPlayer)
{
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pvideoPlayer;
    CriMvEasyPlayer* pcriPlayer = pplayer->GetCriPlayer();
    if (pcriPlayer)
    {
        pcriPlayer->SetUsableProcessors_PC(
            DecodingThreadNumber, AffinityMask, Thread::GetOSPriority(DecodeThreadPriority));
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WIN32
