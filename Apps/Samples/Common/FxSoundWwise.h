/**************************************************************************

Filename    :   FxSoundWwise.h
Content     :   Sound by AK Wwise
Created     :   Oct, 2009
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef _INC_FXSOUND_WWISE_H
#define _INC_FXSOUND_WWISE_H

#include "GFxConfig.h"
#if defined(GFX_ENABLE_SOUND) && defined(GFX_SOUND_WWISE)

#include "Kernel/SF_Types.h"
#if defined(SF_OS_PS3)
#include <cell/spurs.h>
#endif

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

class FxSoundWwise
{
public:
    FxSoundWwise(): Initialized(false) {}

    bool Initialize(
#if defined(SF_OS_PS3)
        const CellSpurs* pSpurs
#endif
    );
    void Finalize();
    bool IsInitialized() const { return Initialized; }

protected:
    bool Initialized;
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND && GFX_SOUND_WWISE

#endif
