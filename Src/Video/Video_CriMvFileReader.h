/**************************************************************************

Filename    :   Video_CriMvFileReader.h
Content     :   Video file reader (LEGACY STUB - CRI Mana handles file I/O internally)
Created     :   July 2008
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

#ifndef INC_GFX_VIDEO_CRIMVFILEREADER_H
#define INC_GFX_VIDEO_CRIMVFILEREADER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

// CRI Mana 2019 handles file I/O internally via its own file system (CRI FS).
// The CriMvFileReaderInterface wrapper is no longer needed.
// This header is kept for backward compatibility with any code that includes it.

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_MemoryHeap.h"
#include "Kernel/SF_Threads.h"

#include "GFx/GFx_Loader.h"
#include "Video/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

class VideoReader;

//////////////////////////////////////////////////////////////////////////
// Simplified VideoReaderConfig (no CRI interfaces)

class VideoReaderConfigSmp : public VideoReaderConfig
{
public:
    VideoReaderConfigSmp(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        VideoReaderConfig(pheap, pfileopener) {}
    ~VideoReaderConfigSmp() {}

    virtual VideoReader* CreateReader() { return NULL; }
};

//////////////////////////////////////////////////////////////////////////
//

class VideoReaderConfigThrd : public VideoReaderConfig
{
public:
    VideoReaderConfigThrd(MemoryHeap* pheap, FileOpenerBase* pfileopener) :
        VideoReaderConfig(pheap, pfileopener) {}
    ~VideoReaderConfigThrd() {}

    virtual VideoReader* CreateReader() { return NULL; }
    virtual void SetReadCallback(ReadCallback*) {}
    virtual void EnableIO(bool) {}
    virtual void DetachReader(VideoReader*) {}
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_CRIMVFILEREADER_H
