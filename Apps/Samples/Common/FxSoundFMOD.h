/**************************************************************************

Filename    :   FxSoundFMOD.h
Content     :   Sound by FMOD
Created     :   Feb, 2009
Authors     :   Maxim Didenko, Andrew Reisse, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef _INC_FXSOUND_FMOD_H
#define _INC_FXSOUND_FMOD_H

#include "GFxConfig.h"
#if defined(GFX_ENABLE_SOUND) && defined(GFX_SOUND_FMOD)

#include <fmod.hpp>
#include "Sound/Sound_SoundRendererFMOD.h"

#include "GFx/GFx_Loader.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"

#ifdef SF_OS_PS3
#include <cell/audio.h>
#include <cell/spurs.h>
#include <sysutil/sysutil_sysparam.h>
#include <fmodps3.h>
#endif

#ifdef SF_OS_3DS
#include <nn/os.h>
#include <nn/fnd.h>
#include <fmod3ds.h>
#endif

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

class FxSoundFMOD : public NewOverrideBase<Stat_Default_Mem>
{
    FMOD::System *pFMOD;
    Ptr<Sound::SoundRendererFMOD> pSoundRenderer;
    bool Initialized;

public:
    FxSoundFMOD() : pFMOD(NULL), Initialized(false) {}    

#if defined(SF_OS_PS3)
    bool Initialize(const CellSpurs* pspurs);
#elif defined(SF_OS_3DS)
    bool Initialize(const nn::fnd::ExpHeap* pheap);
#else
    bool Initialize();
#endif    

    void Finalize();

    bool IsInitialized() const                  { return Initialized; }
    Sound::SoundRenderer* GetSoundRenderer()    { return pSoundRenderer; }
    FMOD::System *GetFMOD()                     { return pFMOD; }

    static MemoryHeap *pFMODHeap;
    static FileOpenerBase *pFileOpener;
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND && GFX_SOUND_FMOD

#endif
