/**************************************************************************

Filename    :   Video_AVHeap.cpp
Content     :   GFx video: FFmpeg memory heap and subsystem glue
Created     :   October 2026
Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/FFMpeg/ATVBase/Video_ATVHeap.h"

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_String.h"

#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
// FFmpeg public headers are plain C headers without extern "C" guards.
// They are third party code compiled at /W4: libavutil's inline clippers
// (av_clip_uint8_c & co. in libavutil/common.h) return an int expression from
// functions declared to return uint8_t/int8_t/uint16_t/int16_t, which makes MSVC
// report C4244 at the header line. The diagnostic is silenced around the include
// block instead of being patched in the vendored SDK.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244) // int -> uint8_t/int8_t/uint16_t/int16_t in libavutil
#endif
extern "C"
{
#include <libavutil/log.h>
#include <libavformat/avformat.h>
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include <stdio.h>

namespace Scaleform { namespace GFx { namespace Video {

// Heap used for all FFmpeg-side staging buffers created by the FFMpeg video
// player (YUV staging frame, ARGB fallback scratch, audio FIFO).
static MemoryHeap* s_AVHeap = NULL;

//////////////////////////////////////////////////////////////////////////
//
// FFmpeg log callback. Routes libav diagnostics to stderr in debug builds;
// in release builds messages are discarded (av_log_set_level still limits
// the volume of errors produced internally).

static void avLogCallback(void* avcl, int level, const char* fmt, va_list vl)
{
    SF_UNUSED(avcl);
    if (level > av_log_get_level())
        return;

    char buf[512];
    vsnprintf(buf, sizeof(buf) - 1, fmt, vl);
    buf[sizeof(buf) - 1] = 0;

    size_t len = SFstrlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = 0;

#ifdef SF_BUILD_DEBUG
    fprintf(stderr, "[FFmpeg] %s\n", buf);
#else
    SF_UNUSED(len);
#endif
}

//////////////////////////////////////////////////////////////////////////
//

void avHeap_Register(MemoryHeap* pheap)
{
    if (!s_AVHeap)
    {
        s_AVHeap = pheap;
        av_log_set_level(AV_LOG_ERROR);
        av_log_set_callback(avLogCallback);
        // Initializes the network protocols / file IO used by libavformat.
        avformat_network_init();
    }
}

void avHeap_Unregister()
{
    if (s_AVHeap)
    {
        avformat_network_deinit();
        av_log_set_callback(av_log_default_callback);
        s_AVHeap = NULL;
    }
}

MemoryHeap* avHeap_GetHeap()
{
    return s_AVHeap;
}

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
