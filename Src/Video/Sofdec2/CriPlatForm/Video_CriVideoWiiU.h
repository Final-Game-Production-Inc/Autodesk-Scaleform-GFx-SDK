/**************************************************************************

Filename    :   Video_VideoWiiU.h
Content     :   GFx video for Wii U
Created     :   Dec 2013
Authors     :   Vladislav Merker

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOWIIU_H
#define INC_GFX_VIDEOWIIU_H

#include "Video/Video_Video.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIIU)

namespace Scaleform { namespace GFx { namespace Video {

// The maximum number of worker threads is 2 on the Wii U
const int WiiUMaxVideoDecodingThreads = 2;

//////////////////////////////////////////////////////////////////////////
//

class VideoWiiU : public Video
{
public:
    // The first parameter specifies ActionScript VM support: AS2, AS3 or both.
    // The second parameter specifies priority of the video decoding threads.
    // The third, specifies the number of threads which will be used for video
    // decoding (maximum is 2). And forth parameter is an array of the affinity
    // masks for each decoding thread.
    VideoWiiU(const VideoVMSupport& vmSupport,
             Thread::ThreadPriority decodingThreadsPriority = Thread::BelowNormalPriority,
             int numberDecodingThreads = WiiUMaxVideoDecodingThreads,
             u16* affinityMasks = NULL);

private:
    int NumberThreads;
    u16 AffinityMasks[WiiUMaxVideoDecodingThreads];
    int Priorities[WiiUMaxVideoDecodingThreads];
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WIIU

#endif // INC_GFX_VIDEOWIIU_H
