/**************************************************************************

Filename    :   FxSoundWwise.cpp
Content     :   Sound by AK Wwise
Created     :   Oct, 2009
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxSoundWwise.h"
#if defined(GFX_ENABLE_SOUND) && defined(GFX_SOUND_WWISE)

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "GFx/GFx_Stats.h"

// Wwise supports only for Windows, Xbox360, PS3 and Wii only
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360) || defined(SF_OS_PS3) || defined(SF_OS_WII)

#if defined(SF_OS_XBOX360) && !defined(XBOX360)
#define XBOX360
#endif

#include <AK/SoundEngine/Common/AkMemoryMgr.h>          // Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>             // Default memory and stream managers
#include <AK/SoundEngine/Common/AkSoundEngine.h>        // Sound engine
#include <AK/SoundEngine/Common/IAkStreamMgr.h>         // Streaming Manager
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

//////////////////////////////////////////////////////////////////////////
//

namespace AK
{
    void *AllocHook(size_t in_size) { return SF_ALLOC(in_size, Scaleform::Stat_Sound_Mem); }
    void  FreeHook(void *in_ptr)    { SF_FREE(in_ptr); }

#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
    void *VirtualAllocHook(void *in_pMemAddress, size_t in_size, DWORD in_dwAllocationType, DWORD in_dwProtect)
    {
        SF_UNUSED(in_pMemAddress);
        SF_UNUSED(in_dwAllocationType);
        SF_UNUSED(in_dwProtect);

        return SF_ALLOC(in_size, Scaleform::Stat_Sound_Mem);
    }
    void VirtualFreeHook(void *in_pMemAddress, size_t in_size, DWORD in_dwFreeType)
    {
        SF_UNUSED(in_size);
        SF_UNUSED(in_dwFreeType);

        SF_FREE(in_pMemAddress);
    }
#endif

#if defined(SF_OS_XBOX360)
    void *PhysicalAllocHook(size_t in_size, ULONG_PTR in_ulPhysicalAddress, ULONG_PTR in_ulAlignment, DWORD in_dwProtect)
    {
        SF_UNUSED(in_ulPhysicalAddress);
        SF_UNUSED(in_ulAlignment);
        SF_UNUSED(in_dwProtect);

        return SF_ALLOC(in_size, Scaleform::Stat_Sound_Mem);
    }
    void PhysicalFreeHook(void *in_pMemAddress)
    {
        SF_FREE(in_pMemAddress);
    }
#endif

#if defined(SF_OS_WII)
    void *AllocMEM2Hook(size_t in_size) { return SF_ALLOC(in_size, Scaleform::Stat_Sound_Mem); }
    void  FreeMEM2Hook(void *in_ptr)    { SF_FREE(in_ptr); }
#endif
}

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

bool FxSoundWwise::Initialize(
#if defined(SF_OS_PS3)
    const CellSpurs* pSpurs
#endif
)
{
    if(Initialized)
        return true;

    // Initialize memory manager
    AkMemSettings memSettings;
    memSettings.uMaxNumPools = 20;
    AKRESULT res = AK::MemoryMgr::Init(&memSettings);
    if(res != AK_Success) return false;

    // Create streaming manager
    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);
    AK::IAkStreamMgr *sm = AK::StreamMgr::Create(stmSettings);
    SF_ASSERT(sm != NULL);
    SF_UNUSED(sm);

    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

    AkInitSettings initSettings;
    AkPlatformInitSettings platformInitSettings;
    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

#if defined(SF_OS_PS3)
    SF_ASSERT(pSpurs != NULL);
    platformInitSettings.pSpurs = const_cast<CellSpurs*>(pSpurs);
#endif

    // Initialize the sound engine
    res = AK::SoundEngine::Init(&initSettings, &platformInitSettings);
    if(res != AK_Success) return false;

    Initialized = true;

    return true;
}

void FxSoundWwise::Finalize()
{
    if (AK::SoundEngine::IsInitialized())
        AK::SoundEngine::Term();

    if (AK::IAkStreamMgr::Get())
        AK::IAkStreamMgr::Get()->Destroy();

    AK::MemoryMgr::Term();

    Initialized = false;
}

}} // namespace Scaleform::GFx

#endif
#endif // GFX_ENABLE_SOUND && GFX_SOUND_WWISE
