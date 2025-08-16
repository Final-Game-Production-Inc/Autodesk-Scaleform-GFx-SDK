/**************************************************************************

Filename    :   Video_VideoXbox360.h
Content     :   GFx video for Xbox360
Created     :   Sep 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOXBOX360_H
#define INC_GFX_VIDEOXBOX360_H

#include "Video/Video_Video.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_XBOX360)

namespace Scaleform { namespace GFx { namespace Video {

enum Xbox360Proc
{
    Xbox360_Proc0 = 1 << 0, // Core 0 thread 0
    Xbox360_Proc1 = 1 << 1, // Core 0 thread 1
    Xbox360_Proc2 = 1 << 2, // Core 1 thread 0
    Xbox360_Proc3 = 1 << 3, // Core 1 thread 1
    Xbox360_Proc4 = 1 << 4, // Core 2 thread 0
    Xbox360_Proc5 = 1 << 5  // Core 2 thread 1
};

//////////////////////////////////////////////////////////////////////////
//

class VideoXbox360 : public Video
{
public:
    // This takes four parameters that specify ActionScript VM support and which
    // hardware threads will be used for the video decoding. The first parameter
    // specifies ActionScript VM support: AS2, AS3 or both. The second parameter
    // specifies the threads which will do the most work internally, so these threads
    // should not overlap if possible. The third parameter specifies priority for the
    // video decoding threads. The forth parameter specifies additional thread for
    // the reader.
    VideoXbox360(const VideoVMSupport& vmSupport,
                 unsigned decodingProcs = Xbox360_Proc1 | Xbox360_Proc3 | Xbox360_Proc5,
                 Thread::ThreadPriority decodingThreadsPriority = Thread::NormalPriority,
                 Xbox360Proc readerProc = Xbox360_Proc2);

    // Xbox360 specific settings such as usable processors and thread priorities
    // for the player, decoder and reader.
    virtual void ApplySystemSettings(VideoPlayer* pvideoPlayer);
    virtual void ApplySystemSettings(VideoDecoder* pvideoDecoder);
    virtual void ApplySystemSettings(VideoReaderConfig* pvideoReader);

private:
    unsigned    DecoderProcs;
    Xbox360Proc ReaderProc;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_XBOX360

#endif // INC_GFX_VIDEOXBOX360_H
