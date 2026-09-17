/**************************************************************************

Filename    :   Video_VideoPS4.h
Content     :   GFx video for PS4
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOPS4_H
#define INC_GFX_VIDEOPS4_H

#include "Video/Video_Video.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_ORBIS)

namespace Scaleform { namespace GFx { namespace Video {

// The maximum number of worker threads is 5 on the PS4 environment
const int PS4MaxVideoDecodingThreads = 5;

enum PS4AffinityMask
{
    CPU0 = 0x1, CPU1 = 0x2,  CPU2 = 0x4,
    CPU3 = 0x8, CPU4 = 0x10, CPU5 = 0x20,
    AllCPUs = SCE_KERNEL_CPUMASK_USER_ALL
};

//////////////////////////////////////////////////////////////////////////
//

class VideoPS4 : public Video
{
public:
    // The first parameter specifies ActionScript VM support: AS2, AS3 or both.
    // The second parameter specifies priority of the video decoding threads.
    // The third, specifies the number of threads which will be used for video
    // decoding (maximum is 5). And forth parameter is an array of the affinity
    // masks for each decoding thread.
    VideoPS4( const VideoVMSupport& vmSupport, int numberDecodingThreads, const SceKernelCpumask* affinityMasks, const int* priorities );

private:
    int                 NumberThreads;
    SceKernelCpumask    AffinityMasks[PS4MaxVideoDecodingThreads];
    int                 Priorities[PS4MaxVideoDecodingThreads];
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_ORBIS

#endif // INC_GFX_VIDEOPS4_H
