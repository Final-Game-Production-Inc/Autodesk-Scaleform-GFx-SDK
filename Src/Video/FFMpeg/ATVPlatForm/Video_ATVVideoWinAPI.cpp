/**************************************************************************

Filename    :   Video_ATVVideoWinAPI.cpp
Content     :   GFx FFMpeg (ATV / Analog TV) video for Windows based PC
Created     :   October 24th, 2026

Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/FFMpeg/ATVPlatForm/Video_ATVVideoWinAPI.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIN32)

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Video/FFMpeg/ATVBase/Video_ATVVideoPlayerImpl.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoPC::VideoPC(const VideoVMSupport& vmSupport, Thread::ThreadPriority decodingThreadsPriority,
                 int decodingThreadsNumber, UInt32* affinityMask) : Video(vmSupport, decodingThreadsPriority)
{
    // The ATV (Analog TV) FFmpeg backend decodes single-threaded and is
    // driven by the engine's Decode() calls, so the legacy PC decoder
    // thread pool settings are accepted for interface compatibility and
    // stored only for reporting; they do not spin up any decode threads.
    DecodingThreadNumber = decodingThreadsNumber > MAX_VIDEO_DECODING_THREADS
        ? MAX_VIDEO_DECODING_THREADS : decodingThreadsNumber;
    for(int i = 0 ; i < MAX_VIDEO_DECODING_THREADS; ++i)
    {
        AffinityMask[i] = affinityMask && i < DecodingThreadNumber
            ? affinityMask[i] : DEFAULT_VIDEO_DECODING_AFFINITY_MASK;
    }
}

void VideoPC::ApplySystemSettings(VideoPlayer* pvideoPlayer)
{
    SF_UNUSED(pvideoPlayer);

    // Nothing to configure: the FFmpeg backend performs no thread-pool or
    // affinity setup. Decoding is single-threaded (VideoDecoderSmp) and the
    // FFmpeg library configuration (log level, network init) is handled by
    // avHeap_Register/avHeap_Unregister in Video::Initialize/Finalize.
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WIN32
