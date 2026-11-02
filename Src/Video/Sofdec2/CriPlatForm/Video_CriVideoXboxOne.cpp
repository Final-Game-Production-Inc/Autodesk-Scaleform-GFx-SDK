/**************************************************************************

Filename    :   Video_VideoXboxOne.cpp
Content     :   GFx video for XboxOne
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoXboxOne.h"

#if defined(GFX_ENABLE_VIDEO) && defined(_DURANGO)

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif

#include <cri_movie.h>
#include <cri_movie_xboxone.h>

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoXboxOne::VideoXboxOne(const VideoVMSupport& vmSupport,
                           Thread::ThreadPriority decodingThreadsPriority,
                           int numberDecodingThreads,
                           DWORD_PTR* affinityMasks) : Video(vmSupport, decodingThreadsPriority, false)
{
    NumberThreads = numberDecodingThreads > XboxOneMaxVideoDecodingThreads
                  ? XboxOneMaxVideoDecodingThreads
                  : numberDecodingThreads;

    for(int i = 0 ; i < XboxOneMaxVideoDecodingThreads; ++i)
    {
        AffinityMasks[i] = affinityMasks && i < NumberThreads ? affinityMasks[i] : XboxOneAffinityMask::AllCPUs;
        Priorities[i] = Thread::GetOSPriority(DecodeThreadPriority);
    }

    CriMv::SetUsableProcessors_XBOXONE(NumberThreads, AffinityMasks, Priorities);
    Initialize();
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && _DURANGO
