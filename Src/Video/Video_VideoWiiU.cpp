/**************************************************************************

Filename    :   Video_VideoWiiU.cpp
Content     :   GFx video for WiiU
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoWiiU.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIIU)

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif

#include <cri_movie.h>
#include <cri_movie_wiiu.h>

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoWiiU::VideoWiiU(const VideoVMSupport& vmSupport,
                     Thread::ThreadPriority decodingThreadsPriority,
                     int numberDecodingThreads,
                     u16* affinityMasks) : Video(vmSupport, decodingThreadsPriority, false)
{
    NumberThreads = numberDecodingThreads > WiiUMaxVideoDecodingThreads
                  ? WiiUMaxVideoDecodingThreads
                  : numberDecodingThreads;

    for(int i = 0 ; i < WiiUMaxVideoDecodingThreads; ++i)
    {
        AffinityMasks[i] = affinityMasks && i < NumberThreads ? affinityMasks[i] : OS_THREAD_ATTR_AFFINITY_NONE;
        Priorities[i] = Thread::GetOSPriority(DecodeThreadPriority);
    }

    CriMv::SetUsableProcessors_WIIU(NumberThreads, AffinityMasks, Priorities);
    Initialize();
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WiiU
