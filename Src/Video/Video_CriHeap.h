/**************************************************************************

Filename    :   Video_CriHeap.h
Content     :   Video memory allocator for CRI Mana
Created     :   June 4, 2008
Changed    :   March 10, 2026
Authors     :   Maxim Didenko, Vladislav Merker (2008.6.4) [is cri movie 3.50]
                    MonstroFil, Claude(2026.3) [Changed To Cri Mana 2019 From Criware SDK 2.19.03, From Github Author]
                    FINALZ-27980 [To Integration]

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

#ifndef INC_GFX_VIDEO_CRIHEAP_H
#define INC_GFX_VIDEO_CRIHEAP_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_MemoryHeap.h"

namespace Scaleform { namespace GFx { namespace Video {

// Register Scaleform memory heap as the allocator for CRI Mana library.
// Must be called before criMana_Initialize.
void criManaHeap_Register(MemoryHeap* pHeap);

// Unregister the Scaleform memory allocator from CRI Mana.
// Call after criMana_Finalize.
void criManaHeap_Unregister();

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_CRIHEAP_H
