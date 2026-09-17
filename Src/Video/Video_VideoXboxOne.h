/**************************************************************************

Filename    :   Video_VideoXboxOne.h
Content     :   GFx video for XboxOne
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOXBOXONE_H
#define INC_GFX_VIDEOXBOXONE_H

#include "Video/Video_Video.h"

#if defined(GFX_ENABLE_VIDEO) && defined(_DURANGO)

namespace Scaleform { namespace GFx { namespace Video {

// The maximum number of worker threads is 5 on the XboxOne environment
const int XboxOneMaxVideoDecodingThreads = 5;

enum XboxOneAffinityMask
{
    CPU0 = 0x1, CPU1 = 0x2,  CPU2 = 0x4,
    CPU3 = 0x8, CPU4 = 0x10, CPU5 = 0x20,
    AllCPUs = 0x3F
};

//////////////////////////////////////////////////////////////////////////
//

class VideoXboxOne : public Video
{
public:
    // The first parameter specifies ActionScript VM support: AS2, AS3 or both.
    // The second parameter specifies priority of the video decoding threads.
    // The third, specifies the number of threads which will be used for video
    // decoding (maximum is 5). And forth parameter is an array of the affinity
    // masks for each decoding thread.
    VideoXboxOne(const VideoVMSupport& vmSupport,
                 Thread::ThreadPriority decodingThreadsPriority = Thread::BelowNormalPriority,
                 int numberDecodingThreads = XboxOneMaxVideoDecodingThreads,
                 DWORD_PTR* affinityMasks = NULL);

private:
    int         NumberThreads;
    DWORD_PTR   AffinityMasks[XboxOneMaxVideoDecodingThreads];
    int         Priorities[XboxOneMaxVideoDecodingThreads];
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && _DURANGO

#endif // INC_GFX_VIDEOXBOXONE_H
