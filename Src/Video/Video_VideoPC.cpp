/**************************************************************************

Filename    :   Video_VideoPC.cpp
Content     :   GFx video for Windows based PC (CRI Mana 2019)
Created     :   Sep 2009
Changed    : March 10, 2026(MonstroFil, Claude),
                    April 11, 2026(FINALZ-27980) 
                    
Authors     :   Maxim Didenko, Vladislav Merker (2008.6.4) [is cri movie 3.50]
                    MonstroFil, Claude(2026.3) [Changed To Cri Mana 2019 From Criware SDK 2.19.03, From Github Author]
                    FINALZ-27980 [To Integration, Add H.264 Support For Criware SDK 2019]

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

TIPS          :   THE TYPICAL REASON OF THIS PROBLEM IS:
					When Open A New Sample Like GFL2 Exilium .usm File, It Runs Extremely Slowly(Threads Problem)
					Cuz CRIWARE 2019 SDK Embbed And Support H.264 Encoded (Reference To sf_video_4.6.33_windows_msvc11_lib_4K-ready),
                    USE Sofdec2 Viewer Open It Fluently, But Scaleform Video Player Is Extremely Slow To Open It,
                    And When Open More H.264 Files, The GFxPlayer Is Crashed.

                    Should Be Fix It Is Requirments.

                    This Criware SDK Will Be Stop Update For Scaleform 5.0 (After 2030) Stable Version, And More Support To FFMpeg,
                    FINAL VIDEO(FINAL ENGINE), A SelectaVision HDRVIDEOS Persensation(.264-.268 File Format), And More Support Video As Everyone Seen.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Video/Video_VideoPC.h"

#if defined(GFX_ENABLE_VIDEO) && defined(SF_OS_WIN32)

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_mana.h>
#include <cri_mana_pc.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#include "Kernel/SF_MemoryHeap.h"
#include "Video/Video_VideoPlayerImpl.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

VideoPC::VideoPC(const VideoVMSupport& vmSupport, Thread::ThreadPriority decodingThreadsPriority,
                 int decodingThreadsNumber, UInt32* affinityMask) : Video(vmSupport, decodingThreadsPriority)
{
    DecodingThreadNumber = decodingThreadsNumber > MAX_VIDEO_DECODING_THREADS
        ? MAX_VIDEO_DECODING_THREADS : decodingThreadsNumber;
    for(int i = 0 ; i < MAX_VIDEO_DECODING_THREADS; ++i)
    {
        AffinityMask[i] = affinityMask && i < DecodingThreadNumber
            ? affinityMask[i] : DEFAULT_VIDEO_DECODING_AFFINITY_MASK;
    }

    // Support Like Example Video File With H.264 Codec, Such GFL2 Exilium .usm File,
	// Alouthgh It's Open, But Framerate Is Very Low, Open More H.264 Files Will Crash...
    // Try To Fix It In Future Version, And Add More Support For H.264 Codec.
    CriManaMediaFoundationH264DecoderConfig_PC h264Config{};
    criMana_SetDefaultMediaFoundationH264DecoderConfig_PC(&h264Config);
    criMana_SetupMediaFoundationH264Decoder_PC(&h264Config, nullptr, 1);
}

void VideoPC::ApplySystemSettings(VideoPlayer* pvideoPlayer)
{
    SF_UNUSED(pvideoPlayer);

    // CRI Mana 2019: processor/thread config is set at library init time
    // via CriManaLibConfig_PC in Video::Initialize(). Per-player settings
    // are no longer available. Thread priority can be changed via:
    criMana_SetDecodeThreadPriority_PC(Thread::GetOSPriority(DecodeThreadPriority));
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO && SF_OS_WIN32
