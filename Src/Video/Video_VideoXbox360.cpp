/**************************************************************************

Filename    :   Video_VideoXbox360.cpp
Content     :   GFx video for Xbox360
Created     :   Sep 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoXbox360.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_XBOX360)

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

VideoXbox360::VideoXbox360(const VideoVMSupport& vmSupport, unsigned decodingProcs,
                           Thread::ThreadPriority decodeThreadsPriority, Xbox360Proc readerProc) :
    Video(vmSupport, decodeThreadsPriority),
    DecoderProcs(decodingProcs), ReaderProc(readerProc)
{
}

void VideoXbox360::ApplySystemSettings(VideoPlayer* pvideoPlayer)
{
    VideoPlayerImpl* pplayer = (VideoPlayerImpl*)pvideoPlayer;
    CriMvEasyPlayer* pcriPlayer = pplayer->GetCriPlayer();
    if (pcriPlayer)
    {
        CriMvProcessorParameters_XBOX360 params;
        params.processor0_flag = DecoderProcs & Xbox360_Proc0;
        params.processor1_flag = DecoderProcs & Xbox360_Proc1;
        params.processor2_flag = DecoderProcs & Xbox360_Proc2;
        params.processor3_flag = DecoderProcs & Xbox360_Proc3;
        params.processor4_flag = DecoderProcs & Xbox360_Proc4;
        params.processor5_flag = DecoderProcs & Xbox360_Proc5;
        params.thread_priority = Thread::GetOSPriority(DecodeThreadPriority);
        pcriPlayer->SetUsableProcessors_XBOX360(&params);
    }
}

static inline int mapToXbox360MaskToProc(unsigned coreMask)
{
    for(int i = 0; i < 6; ++i)
    {
        if ((coreMask >> i) & 1)
            return i;
    }
    return 1;
}

static inline int mapToXbox360Proc(Xbox360Proc core)
{
    switch(core)
    {
        case Xbox360_Proc0: return 0;
        case Xbox360_Proc1: return 1;
        case Xbox360_Proc2: return 2;
        case Xbox360_Proc3: return 3;
        case Xbox360_Proc4: return 4;
        case Xbox360_Proc5: return 5;
    }
    return 1;
}

void VideoXbox360::ApplySystemSettings(VideoDecoder* pvideoDecoder)
{
    pvideoDecoder->SetUsableProcessors(mapToXbox360MaskToProc(DecoderProcs));
}

void VideoXbox360::ApplySystemSettings(VideoReaderConfig* pvideoReader)
{
    pvideoReader->SetUsableProcessors(mapToXbox360Proc(ReaderProc));
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_XBOX360

