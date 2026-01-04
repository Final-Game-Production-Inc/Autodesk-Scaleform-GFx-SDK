/**************************************************************************

Filename    :   Sound_SoundEvent.h
Content     :   SoundEvent interface
Created     :   March 2010
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GSOUNDEVENT_H
#define INC_GSOUNDEVENT_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_String.h"

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEvent : public RefCountBase<SoundEvent,Stat_Sound_Mem>
{
public:
    SoundEvent() : pUserData(NULL) {}
    virtual ~SoundEvent() {}

    virtual void PostEvent(String event, String eventId = "") = 0;
    virtual void SetParam (String param, float paramValue, String eventId = "") = 0;

    void* pUserData;
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif // INC_GSOUNDEVENT_H
