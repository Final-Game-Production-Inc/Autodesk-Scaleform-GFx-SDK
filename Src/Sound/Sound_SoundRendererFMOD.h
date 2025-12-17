/**************************************************************************

PublicHeader:   Sound_FMOD
Filename    :   Sound_SoundRendererFMOD.h
Content     :   SoundRenderer FMOD Ex implementation
Created     :   November, 2008
Authors     :   Andrew Reisse, Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GSOUNDRENDERERMOD_H
#define INC_GSOUNDRENDERERMOD_H

#include "Sound_SoundRenderer.h"
#ifdef GFX_ENABLE_SOUND

#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO) && !defined(_DURANGO)
#if defined(_WIN64)         // Windows PC x64/x86
    #pragma comment(lib, "fmodex64_vc.lib")
#else
    #pragma comment(lib, "fmodex_vc.lib")
#endif
#elif defined(SF_OS_WINMETRO) && !defined(_DURANGO)
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    #if defined(_M_ARM_FP)  // Windows Phone8 ARM/x86
    #pragma comment(lib, "fmodex_arm.lib")
    #else
    #pragma comment(lib, "fmodex_x86.lib")
    #endif
#else
#if (SF_CC_MSVC == 1700) //Windows 8
	#if defined(_WIN64)     // Windows Store x64/ARM/x86
	#pragma comment(lib, "fmodexWSA8064_vc.lib")
	#elif defined(_M_ARM_FP)
	#pragma comment(lib, "fmodexWSA80arm_vc.lib")
	#else
	#pragma comment(lib, "fmodexWSA80_vc.lib")
	#endif
#endif
#if (SF_CC_MSVC == 1800) //Windows store 8.1
#if defined(_WIN64)     // Windows Store x64/ARM/x86
	#pragma comment(lib, "fmodexWSA8164_vc.lib")
	#elif defined(_M_ARM_FP)
	#pragma comment(lib, "fmodexWSA81arm_vc.lib")
	#else
	#pragma comment(lib, "fmodexWSA81_vc.lib")
	 #endif
#endif
#endif                      // Xbox360, PS3, XboxOne, PS4
#elif defined(SF_OS_XBOX360)
    #pragma comment(lib, "fmodxbox360.lib")
#elif defined(SF_OS_PS3)
    #pragma comment(lib, "fmodex_SPURS.a")
#elif defined(_DURANGO)
    #pragma comment(lib, "fmodex.lib")
#elif defined(SF_OS_ORBIS)
    #pragma comment(lib, "libfmodex.a")
#endif

namespace FMOD
{
    class System;
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    class EventSystem;
#endif
}

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundRendererFMOD : public SoundRenderer
{
public:
    static SoundRendererFMOD* SF_CDECL CreateSoundRenderer();

    virtual FMOD::System* GetFMODSystem() = 0;
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    virtual FMOD::EventSystem* GetFMODEventSystem() = 0;
#endif

    // Initialize FMOD sound renderer
    // if call_fmod_update is true them the SoundRenderer will call FMOD::System::update() method
    // from its Update() method
    // if threaded_update is true then the SoundRenderer will create a separate thread for retrieving 
    // audio data from video files. If this parameter is false audio data from video file will be retrieved
    // on the main thread from Update() method (which is called from GFxMovieRoot::Advance method)
    // (Xbox360 only) processor_core parameter allows specifying a processor core which will be used for 
    // SoundRenderer thread if it is created.
    virtual bool Initialize(FMOD::System* pd, bool call_fmod_update = true, bool threaded_update = true
#ifdef SF_OS_XBOX360
                          , int processor_core = 4
#endif
                           ) = 0;

    // Finalize can be called to stop SoundRenderer thread and release all internal objects if this is 
    // needs to be done before the SoundRenderer is destructed.
    virtual void Finalize() = 0;
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif
