/**************************************************************************

Filename    :   Video_CriHeap.cpp
Content     :   Video memory allocator for CRI Mana
Created     :   June 4, 2008
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

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Types.h"
#include "Kernel/SF_HeapNew.h"
#include "Video/Video_CriHeap.h"

#include "Kernel/SF_Memory.h"
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#undef new
#endif
#include <cri_mana.h>
#include <cri_atom_ex.h>
#if defined(SF_BUILD_DEFINE_NEW) && defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

static MemoryHeap* s_pManaHeap = NULL;

static void* CRIAPI manaAllocFunc(void* obj, CriUint32 size)
{
    SF_UNUSED(obj);
    SF_ASSERT(s_pManaHeap);
    void* ptr = SF_HEAP_MEMALIGN(s_pManaHeap, size, 16, Stat_Video_Mem);
    return ptr;
}

static void CRIAPI manaFreeFunc(void* obj, void* mem)
{
    SF_UNUSED(obj);
    if (mem)
        SF_FREE_ALIGN(mem);
}

void criManaHeap_Register(MemoryHeap* pHeap)
{
    s_pManaHeap = pHeap;
    criAtomEx_SetUserAllocator(manaAllocFunc, manaFreeFunc, NULL);
    criMana_SetUserAllocator(manaAllocFunc, manaFreeFunc, NULL);
}

void criManaHeap_Unregister()
{
    criMana_SetUserAllocator(NULL, NULL, NULL);
    criAtomEx_SetUserAllocator(NULL, NULL, NULL);
    s_pManaHeap = NULL;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
