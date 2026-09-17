/**********************************************************************

PublicHeader:   Render
Filename    :   GL_Sync.h
Content     :   
Created     :   2012/12/05
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_GL_Sync_H
#define INC_SF_GL_Sync_H

#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render { namespace GL {

class HAL;

class RenderSync : public Render::RenderSync
{
public:
    RenderSync(HAL* phal) : pHal(phal) { }

    virtual void    KickOffFences(FenceType waitType);

protected:

    virtual UInt64  SetFence();
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent);
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent);
    virtual void    ReleaseFence(UInt64 handle);

    // No need to implement wraparound; Timestap sizes are 64bits, should take decades to wraparound.

    // Need this to support SF_GL_RUNTIMELINK platforms.
    HAL*    GetHAL() const { return pHal; }
    HAL*    pHal;
};

}}}; // Scaleform::Render::GL

#endif // INC_SF_GL_Sync_H
