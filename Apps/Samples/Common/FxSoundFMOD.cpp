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

#ifdef SF_BUILD_DEBUG
#include <fmod_errors.h>
#endif

#ifdef SF_OS_IPHONE
#include <fmodiphone.h>
#endif

#include "..\..\include\iob_func_fix.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

// Global static FMOD heap and GFx file opener
MemoryHeap *FxSoundFMOD::pFMODHeap = 0;
FileOpenerBase *FxSoundFMOD::pFileOpener = 0;

// Custom heap callbacks
void* F_CALLBACK FMOD_AllocCallback(unsigned int size, FMOD_MEMORY_TYPE, const char*)
{
    return FxSoundFMOD::pFMODHeap->Alloc(size);
}
void* F_CALLBACK FMOD_ReallocCallback(void* ptr, unsigned int size, FMOD_MEMORY_TYPE, const char*)
{
    return FxSoundFMOD::pFMODHeap->Realloc(ptr, size);
}
void F_CALLBACK FMOD_FreeCallback(void* ptr, FMOD_MEMORY_TYPE, const char*)
{
    FxSoundFMOD::pFMODHeap->Free(ptr);
}

// Custom file system callbacks
FMOD_RESULT F_CALLBACK FileOpenCallback(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
    SF_UNUSED2(unicode, userdata);
    if (name)
    {
        Scaleform::File* pfile = FxSoundFMOD::pFileOpener->OpenFile(name);
        if (!pfile)
            return FMOD_ERR_FILE_NOTFOUND;

        *filesize = pfile->GetLength();
        *handle   = pfile;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FileCloseCallback(void *handle, void *userdata)
{
    SF_UNUSED(userdata);
    if (!handle)
        return FMOD_ERR_INVALID_PARAM;

    ((Scaleform::File *)handle)->Close();
    ((Scaleform::File *)handle)->Release();
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FileReadCallback(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    SF_UNUSED(userdata);
    if (!handle)
        return FMOD_ERR_INVALID_PARAM;

    if (bytesread)
    {
        *bytesread = (int)((Scaleform::File *)handle)->Read((UByte *)buffer, sizebytes);
        if (*bytesread < sizebytes)
            return FMOD_ERR_FILE_EOF;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FileSeekCallback(void *handle, unsigned int pos, void *userdata)
{
    SF_UNUSED(userdata);
    if (!handle)
        return FMOD_ERR_INVALID_PARAM;

    ((Scaleform::File *)handle)->Seek(pos, SEEK_SET);
    return FMOD_OK;
}


// FMOD error check macro
#define FMOD_ERRCHK(err)    \
    if (err != FMOD_OK)     \
    {   \
        SF_DEBUG_ERROR2(1, "FMOD error: (%d) %s. No sound will be playing.\n",  \
            result, FMOD_ErrorString(result));  \
        Finalize();     \
        return false;   \
    }


//////////////////////////////////////////////////////////////////////////
//

// Windows excluding Metro, Durango, and WPhone8; Mac OS X; Linux excluding Android
#if (defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)) || defined(SF_OS_MAC) || (defined(SF_OS_LINUX) && !defined(SF_OS_ANDROID))

bool FxSoundFMOD::Initialize()
{
    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    FMOD_ERRCHK(result);

    unsigned int version;
    result = pFMOD->getVersion(&version);
    FMOD_ERRCHK(result);
    if (version < FMOD_VERSION)
    {
        SF_DEBUG_ERROR2(1, "You are using an old version of FMOD %08x. This program requires %08x\n",
            version, FMOD_VERSION);
        Finalize();
        return false;
    }

    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS caps;
    result = pFMOD->getDriverCaps(0, &caps, 0, &speakermode);
    FMOD_ERRCHK(result);

    result = pFMOD->setSpeakerMode(speakermode);        // Set the user selected speaker mode.
    FMOD_ERRCHK(result);

    if (caps & FMOD_CAPS_HARDWARE_EMULATED)             // The user has the 'Acceleration' slider set to off! This is really
    {                                                   // bad for latency!. You might want to warn the user about this.
        result = pFMOD->setDSPBufferSize(1024, 10);     // At 48khz, the latency between issuing an fmod command and hearing
        FMOD_ERRCHK(result);                            // it will now be about 213ms.
    }

    result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);     // Replace with whatever channel count and flags you use!
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         // Ok, the speaker mode selected isn't supported by this soundcard.
    {                                                   // Switch it back to stereo...
        result = pFMOD->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        FMOD_ERRCHK(result);
        result = pFMOD->init(100, FMOD_INIT_NORMAL, 0); // Replace with whatever channel count and flags you use!
        FMOD_ERRCHK(result);
    }
    else {
        FMOD_ERRCHK(result);
    }

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
        return false;
    }

    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

// Windows 8, Durango and Windows Phone 8
#elif defined(SF_OS_WINMETRO)

bool FxSoundFMOD::Initialize()
{
	pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    FMOD_ERRCHK(result);

    unsigned version;
    result = pFMOD->getVersion(&version);
    FMOD_ERRCHK(result);

    if (version < FMOD_VERSION)
    {
        SF_DEBUG_ERROR2(1, "You are using an old version of FMOD %08x. This program requires %08x\n",
            version, FMOD_VERSION);
        Finalize();
        return false;
    }

    int numdrivers;
    result = pFMOD->getNumDrivers(&numdrivers);
    FMOD_ERRCHK(result);

    if (numdrivers == 0)
    {
	    result = pFMOD->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
	    FMOD_ERRCHK(result);
    }
    else
    {
        FMOD_CAPS caps;
        FMOD_SPEAKERMODE speakermode;

        result = pFMOD->getDriverCaps(0, &caps, 0, &speakermode);
        FMOD_ERRCHK(result);

        result = pFMOD->setSpeakerMode(speakermode);
        FMOD_ERRCHK(result);
    }

    result = pFMOD->init(32, FMOD_INIT_NORMAL, 0);
    FMOD_ERRCHK(result);

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, true))
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
        return false;
    }

    Initialized = true;
    return true;
}

//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_XBOX360) || defined(SF_OS_WII) || defined(SF_OS_WIIU) || \
    defined(SF_OS_PSVITA) || defined(SF_OS_IPHONE) || defined(SF_OS_ANDROID)

bool FxSoundFMOD::Initialize()
{
    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    FMOD_ERRCHK(result);

#ifdef SF_OS_XBOX360
    result = pFMOD->init(100, FMOD_INIT_NORMAL, 0);
#elif defined(SF_OS_IPHONE)
    FMOD_IPHONE_EXTRADRIVERDATA extradriverdata;
    memset(&extradriverdata, 0, sizeof(FMOD_IPHONE_EXTRADRIVERDATA));
    extradriverdata.forceMixWithOthers = true; // Force mixing behavior allowing iPod audio to play with FMOD
    extradriverdata.sessionCategory = FMOD_IPHONE_SESSIONCATEGORY_MEDIAPLAYBACK;
    result = pFMOD->init(32, FMOD_INIT_NORMAL, &extradriverdata);
#else
#ifdef SF_OS_ANDROID
    unsigned bufferlength;
    int numbuffers;
    pFMOD->getDSPBufferSize(&bufferlength, &numbuffers);
    pFMOD->setDSPBufferSize(1024, numbuffers);
    pFMOD->setOutput(FMOD_OUTPUTTYPE_AUDIOTRACK);
#endif
    result = pFMOD->init(64,  FMOD_INIT_NORMAL, 0);
#endif
    FMOD_ERRCHK(result);

#ifdef SF_OS_ANDROID
    if (pFileOpener)
    {
        result = pFMOD->setFileSystem(FileOpenCallback, FileCloseCallback, FileReadCallback, FileSeekCallback, 0, 0, 2048);
        if (result != FMOD_OK)
            SF_DEBUG_ERROR(1, "Can not set filesystem callbacks. There will be no audio from assets.\n");
    }
#endif

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
#ifdef SF_OS_XBOX360
    if (!pSoundRenderer->Initialize(pFMOD, true, false, 5))
#else
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
#endif
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
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
    FMOD_ERRCHK(result);

    SF_ASSERT(pheap);
    FMOD_3DS_EXTRADRIVERDATA extraDriverData;
    extraDriverData.deviceMemoryHeap = const_cast<nn::fnd::ExpHeap*>(pheap);

    result = pFMOD->init(16, FMOD_INIT_NORMAL, (void *)&extraDriverData);
    FMOD_ERRCHK(result);

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
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
    CellAudioOutConfiguration audioOutConfig;
    memset(&audioOutConfig, 0, sizeof(CellAudioOutConfiguration)); 

    // First, check for 8 ch or 6ch HDMI
    int channelsAvailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM,
	                                                         CELL_AUDIO_OUT_FS_48KHZ, 0);
    if (channelsAvailable != 8 && channelsAvailable != 6)
    {
        // If there's no 8 ch or 6ch HDMI, check for DTS
        channelsAvailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS,
		                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
        if (channelsAvailable)
        {
            // DTS
            audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_DTS;
        }
        else
        {
            // DTS not supported, check for DD support
            channelsAvailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3,
			                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
            if (channelsAvailable)
            {
                // Dolby digital supported
                audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_AC3;
            }
            else
            {
                channelsAvailable = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM,
				                                                     CELL_AUDIO_OUT_FS_48KHZ, 0);
                audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
            }
        }
    }
    if (channelsAvailable != 8)
    {
        // Need to downmix
        if (channelsAvailable == 6)
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
        SF_DEBUG_ERROR1(1, "cellAudioOutConfigure: %x. No sound will be playing.\n", res);
        return false;
    }

    res = cellAudioInit();
    if (res != CELL_OK && res != CELL_AUDIO_ERROR_ALREADY_INIT)
    {
        SF_DEBUG_ERROR1(1, "cellAudioInit: %x. No sound will be playing.\n", res);
        return false;
    }

    CellAudioPortParam audioParam;
    UInt32 audioPort;

    // Audio port open
    audioParam.nChannel = CELL_AUDIO_PORT_8CH;
    audioParam.nBlock   = CELL_AUDIO_BLOCK_8;
    audioParam.attr     = 0;
    res = cellAudioPortOpen(&audioParam, &audioPort);
    if (res != CELL_OK)
    {
        SF_DEBUG_ERROR1(1, "cellAudioPortOpen: %x. No sound will be playing.\n", res);
        return false;
    }

	pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    FMOD_ERRCHK(result);

    SF_ASSERT(pspurs);
    uint8_t sprus_priorities[] = {1, 1, 0, 0, 0, 0, 0, 0};
    FMOD_PS3_EXTRADRIVERDATA extradriverdata;
    memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));
    extradriverdata.spurs = const_cast<CellSpurs*>(pspurs);
    extradriverdata.cell_audio_initialized = 1;
    extradriverdata.cell_audio_port = audioPort;
    extradriverdata.spurs_taskset_priorities = sprus_priorities;

    result = pFMOD->init(64, FMOD_INIT_NORMAL, (void *)&extradriverdata);
    FMOD_ERRCHK(result);

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
        return false;
    }

    Initialized = true;
    return true;
}


//////////////////////////////////////////////////////////////////////////
//

#elif defined(SF_OS_ORBIS)

bool FxSoundFMOD::Initialize()
{
    pFMODHeap = Memory::GetGlobalHeap()->CreateHeap("_FMOD_Heap", 0, 32);
    FMOD::Memory_Initialize(NULL, 0, FMOD_AllocCallback, FMOD_ReallocCallback, FMOD_FreeCallback);

    FMOD_RESULT result;
    result = FMOD::System_Create(&pFMOD);
    FMOD_ERRCHK(result);

    result = pFMOD->init(64, FMOD_INIT_NORMAL, NULL);
    FMOD_ERRCHK(result);

    pSoundRenderer = *Sound::SoundRendererFMOD::CreateSoundRenderer();
    if (!pSoundRenderer->Initialize(pFMOD, true, false))
    {
        SF_DEBUG_ERROR(1, "Can not initialize sound system. No sound will be playing.\n");
        Finalize();
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

void FxSoundFMOD::Finalize()
{
    Initialized = false;

    if (pSoundRenderer)
        pSoundRenderer->Finalize();
    pSoundRenderer = NULL;

    if (pFMOD) {
        pFMOD->release();
        pFMOD = NULL;
    }
    if (pFMODHeap) {
        pFMODHeap->Release();
        pFMODHeap = NULL;
    }
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND && GFX_SOUND_FMOD
