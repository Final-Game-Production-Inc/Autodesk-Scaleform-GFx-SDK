/**************************************************************************

Filename    :   Video_VideoPS4.cpp
Content     :   GFx video for PS4
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoPS4.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_ORBIS)

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif

#include <cri_movie.h>
#include <cri_movie_ps4.h>

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoPS4::VideoPS4(const VideoVMSupport& vmSupport, int numberDecodingThreads, const SceKernelCpumask* affinityMasks, const int* priorities ) 
	: Video(vmSupport, Thread::NormalPriority /*not used: we override the decoder thread pri ourselves*/, false)
{
    NumberThreads = numberDecodingThreads > PS4MaxVideoDecodingThreads
                  ? PS4MaxVideoDecodingThreads
                  : numberDecodingThreads;

    for(int i = 0 ; i < PS4MaxVideoDecodingThreads; ++i)
    {
        AffinityMasks[i] = affinityMasks && i < NumberThreads ? affinityMasks[i] : PS4AffinityMask::AllCPUs;
        Priorities[i] = priorities[i];
    }

    CriMv::SetUsableProcessors_PS4(NumberThreads, AffinityMasks, Priorities);
    Initialize();
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_ORBIS
