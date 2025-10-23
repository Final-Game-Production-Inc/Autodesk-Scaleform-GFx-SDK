/**************************************************************************

Filename    :   FxSoundFMOD.cpp
Content     :   FMOD Ex support
Created     :   Feb, 2009
Authors     :   Maxim Didenko, Andrew Reisse, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxSoundFMOD.h"
#if defined(GFX_ENABLE_SOUND) && defined(GFX_SOUND_FMOD)

#include <fmod_errors.h>
#include <stdio.h>

#include "..\Include\iob_func_fix.h"

#define ERR_NOSOUND     "FMOD error! (%d) %s. No sound will be playing.\n"
#define ERR_OLDVERSION  "Error! You are using an old version of FMOD %08x. This program requires %08x\n"
#define ERR_INITFAIL    "Error! Can not initialize sound system. No sound will be playing.\n"

namespace Scaleform { namespace GFx {

// Global static FMOD heap
MemoryHeap *FxSoundFMOD::pFMODHeap = 0;

// Custom heap callbacks
void* F_CALLBACK FMOD_AllocCallback(unsigned int size, FMOD_MEMORY_TYPE
#if (FMOD_VERSION >= 0x00043404)
    , const char*
#endif
) { return FxSoundFMOD::pFMODHeap->Alloc(size); }

void* F_CALLBACK FMOD_ReallocCallback(void* ptr, unsigned int size, FMOD_MEMORY_TYPE
#if (FMOD_VERSION >= 0x00043404)
    , const char*
#endif
) { return FxSoundFMOD::pFMODHeap->Realloc(ptr, size); }

void F_CALLBACK FMOD_FreeCallback(void* ptr, FMOD_MEMORY_TYPE
#if (FMOD_VERSION >= 0x00043404)
    , const char*
#endif
) { FxSoundFMOD::pFMODHeap->Free(ptr); }


//////////////////////////////////////////////////////////////////////////
//

#if (defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)) || defined(SF_OS_MAC) || (defined(SF_OS_LINUX) && !defined(SF_OS_ANDROID))

bool FxSoundFMOD::Initialize()
{
    FMOD_RESULT      result;
    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS        caps;
    unsigned int     version;

    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        Finalize();
        return false;
    }

    result = pFMOD->getVersion(&version);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        Finalize();
        return false;
    }
    if (version < FMOD_VERSION)
    {
        fprintf(stderr, ERR_OLDVERSION, version, FMOD_VERSION);
        Finalize();
        return false;
    }

#if (FMOD_VERSION >= 0x00043604)
    result = pFMOD->getDriverCaps(0, &caps, 0, &speakermode);
#else
    result = pFMOD->getDriverCaps(0, &caps, 0, 0, &speakermode);
#endif
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        Finalize();
        return false;
    }

    result = pFMOD->setSpeakerMode(speakermode);        // Set the user selected speaker mode.
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        Finalize();
        return false;
    }

    if (caps & FMOD_CAPS_HARDWARE_EMULATED)             // The user has the 'Acceleration' slider set to off! This is really
    {                                                   // bad for latency!. You might want to warn the user about this.
        result = pFMOD->setDSPBufferSize(1024, 10);     // At 48khz, the latency between issuing an fmod command and hearing
        if (result != FMOD_OK)                          // it will now be about 213ms.
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            Finalize();
            return false;
        }
    }

    result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);     // Replace with whatever channel count and flags you use!
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         // Ok, the speaker mode selected isn't supported by this soundcard.
    {                                                   // Switch it back to stereo...
        result = pFMOD->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            Finalize();
            return false;
        }

        result = pFMOD->init(100, FMOD_INIT_NORMAL, 0); // Replace with whatever channel count and flags you use!
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            Finalize();
            return false;
        }
    }
    else if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        Finalize();
        return false;
    }

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        fprintf(stderr, ERR_INITFAIL);
        Finalize();
        return false;
    }

    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_WINMETRO)

bool FxSoundFMOD::Initialize()
{
	pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    SF_ASSERT(result == FMOD_OK);

    unsigned version;
    result = pFMOD->getVersion(&version);
    SF_ASSERT(result == FMOD_OK);

    if (version < FMOD_VERSION)
    {
        fprintf(stderr, ERR_OLDVERSION, version, FMOD_VERSION);
        Finalize();
        return false;
    }

    int numdrivers;
    result = pFMOD->getNumDrivers(&numdrivers);
    SF_ASSERT(result == FMOD_OK);

    if (numdrivers == 0)
    {
	    result = pFMOD->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
	    SF_ASSERT(result == FMOD_OK);
    }

    result = pFMOD->init(32, FMOD_INIT_NORMAL, 0);
    SF_ASSERT(result == FMOD_OK);

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, true))
    {
        fprintf(stderr, ERR_INITFAIL);
        Finalize();
        return false;
    }

    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_XBOX360) || defined(SF_OS_WII) || defined(SF_OS_WIIU) || defined(SF_OS_PSVITA) \
    || defined(SF_OS_IPHONE) || defined(SF_OS_ANDROID)

bool FxSoundFMOD::Initialize()
{
    FMOD_RESULT result;

    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
    }
    if (pFMOD)
    {
#ifdef SF_OS_XBOX360
        result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);
#else
        result = pFMOD->init(64,  FMOD_INIT_NORMAL, 0);
#endif
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
        }
    }
    if (!pFMOD)
        return false;

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
#ifdef SF_OS_XBOX360
    if (!pSoundRenderer->Initialize(pFMOD, true, false, 5))
#else
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
#endif
    {
        fprintf(stderr, ERR_INITFAIL);
        pFMOD->release();
        pFMOD = NULL;
        pSoundRenderer = NULL;
        return false;
    }
    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_3DS)

bool FxSoundFMOD::Initialize(const nn::fnd::ExpHeap* pheap)
{
    FMOD_RESULT result;

    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
    }
    if (pFMOD)
    {
        SF_ASSERT(pheap);
        FMOD_3DS_EXTRADRIVERDATA extraDriverData;
        extraDriverData.deviceMemoryHeap = const_cast<nn::fnd::ExpHeap*>(pheap);

        result = pFMOD->init(16, FMOD_INIT_NORMAL, (void *)&extraDriverData);
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
        }
    }
    if (!pFMOD)
        return false;

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        fprintf(stderr, ERR_INITFAIL);
        pFMOD->release();
        pFMOD = NULL;
        pSoundRenderer = NULL;
        return false;
    }
    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_PS3)

bool FxSoundFMOD::Initialize(const CellSpurs* pspurs)
{
    int channelsavailable;
    CellAudioOutConfiguration audioOutConfig;
    CellAudioPortParam audioParam;
    UInt32 audioPort;

    memset(&audioOutConfig, 0, sizeof(CellAudioOutConfiguration)); 

    // First, check for 8 ch or 6ch HDMI
    channelsavailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM,
	                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
    if (channelsavailable != 8 && channelsavailable != 6)
    {
        // If there's no 8 ch or 6ch HDMI, check for DTS
        channelsavailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS,
		                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
        if (channelsavailable)
        {
            // DTS
            audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_DTS;
        }
        else
        {
            // DTS not supported, check for DD support
            channelsavailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3,
			                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
            if (channelsavailable)
            {
                // Dolby digital supported
                audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_AC3;
            }
            else
            {
                channelsavailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM,
				                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
                audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
            }
        }
    }
    if (channelsavailable != 8)
    {
        // Need to downmix
        if (channelsavailable == 6)
        {
            audioOutConfig.channel   = 6;
            audioOutConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B; // 8ch => 6ch downmix.
        }
        else
        {
            audioOutConfig.channel   = 2;
            audioOutConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_A; // 8ch => 2ch downmix.
        }
    }
    else
    {
        // No downmix required
        audioOutConfig.channel  = 8;
        audioOutConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_NONE;
    }

    int res = cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &audioOutConfig, NULL, 0);
    if (res != CELL_OK)
    {
        fprintf(stderr, "cellAudioOutConfigure: %x. No sound will be playing.\n", res);
        return false;
    }

    res = cellAudioInit();
    if (res != CELL_OK && res != CELL_AUDIO_ERROR_ALREADY_INIT)
    {
        fprintf(stderr, "cellAudioInit: %x. No sound will be playing.\n", res);
        return false;
    }

    // Audio port open
    audioParam.nChannel = CELL_AUDIO_PORT_8CH;
    audioParam.nBlock   = CELL_AUDIO_BLOCK_8;
    audioParam.attr     = 0;
    res = cellAudioPortOpen(&audioParam, &audioPort);
    if (res != CELL_OK)
    {
        fprintf(stderr, "cellAudioPortOpen: %x. No sound will be playing.\n", res);
        return false;
    }

    FMOD_RESULT result;

	pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);
	
    result = FMOD::System_Create(&pFMOD);
    if (result != FMOD_OK)
    {
        fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
        pFMOD = NULL;
    }
    if (pFMOD)
    {
        SF_ASSERT(pspurs);
        uint8_t sprus_priorities[] = {1, 1, 0, 0, 0, 0, 0, 0};
        FMOD_PS3_EXTRADRIVERDATA extradriverdata;
        memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));
        extradriverdata.spurs = const_cast<CellSpurs*>(pspurs);
        extradriverdata.cell_audio_initialized = 1;
        extradriverdata.cell_audio_port = audioPort;
        extradriverdata.spurs_taskset_priorities = sprus_priorities;

        result = pFMOD->init(64, FMOD_INIT_NORMAL, (void *)&extradriverdata);
        if (result != FMOD_OK)
        {
            fprintf(stderr, ERR_NOSOUND, result, FMOD_ErrorString(result));
            pFMOD->release();
            pFMOD = NULL;
        }
    }
    if (!pFMOD)
        return false;

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        fprintf(stderr, ERR_INITFAIL);
        pFMOD->release();
        pFMOD = NULL;
        pSoundRenderer = NULL;
        return false;
    }

    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#else

bool FxSoundFMOD::Initialize()
{
	// Unsupported or unknown platform
    return false;
}

#endif

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND && GFX_SOUND_FMOD

