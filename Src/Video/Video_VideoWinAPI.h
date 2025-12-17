/**************************************************************************

Filename    :   Video_VideoPC.h
Content     :   GFx video for Windows based PC
Created     :   Sep 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOPC_H
#define INC_GFX_VIDEOPC_H

#include "Video/Video_Video.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIN32)

#define MAX_VIDEO_DECODING_THREADS 3
#define DEFAULT_VIDEO_DECODING_AFFINITY_MASK 0xFFFFFFFF

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

class VideoPC : public Video
{
public:
    // The first parameter specifies ActionScript VM support: AS2, AS3 or both.
    // The second parameter specifies priority of the video decoding threads.
    // The third, specifies the number of threads which will be used for video
    // decoding (maximum is 3). And forth parameter is an array of the affinity
    // masks for each decoding thread.
    VideoPC(const VideoVMSupport& vmSupport,
            Thread::ThreadPriority decodingThreadsPriority = Thread::NormalPriority, 
            int decodingThreadsNumber = MAX_VIDEO_DECODING_THREADS,
            UInt32* affinityMask = NULL);

    // PC specific settings such as number and priority of decoding threads
    // and affinity mask for the video player.
    virtual void ApplySystemSettings(VideoPlayer* pvideoPlayer);
    virtual void ApplySystemSettings(VideoDecoder*) {}
    virtual void ApplySystemSettings(VideoReaderConfig*) {}

private:
    int     DecodingThreadNumber;
    UInt32  AffinityMask[MAX_VIDEO_DECODING_THREADS];
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WIN32

#endif // INC_GFX_VIDEOPC_H
