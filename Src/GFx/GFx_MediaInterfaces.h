/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_MediaInterfaces.h
Content     :   GFx media interfaces
Created     :   August, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_MEDIAINTERFACES_H
#define INC_SF_GFX_MEDIAINTERFACES_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform { namespace GFx { namespace Video {

class VideoCharacter;
class VideoImage;

//////////////////////////////////////////////////////////////////////////
//

class VideoProvider : public RefCountBaseWeakSupport<VideoProvider, StatMV_Other_Mem>
{
public:
    virtual ~VideoProvider() {};

    virtual VideoImage* GetTexture(int* width, int* height) = 0;

    virtual void    Advance()         = 0;
    virtual void    Pause(bool pause) = 0;
    virtual void    Close()           = 0;

    virtual bool    IsActive()        = 0;
    virtual float   GetFrameTime()    = 0;

    virtual void            RegisterVideoCharacter(VideoCharacter* pvideo) = 0;
    virtual VideoCharacter* RemoveFirstVideoCharacter() = 0;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_SF_GFX_MEDIAINTERFACES_H
