/**********************************************************************

PublicHeader:   Render
Filename    :   PS4_Sync.h
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_Sync_H
#define INC_SF_PS4_Sync_H

#include "Render/Render_Sync.h"
#include "Render/Render_MemoryManager.h"
#include "Kernel/SF_Debug.h"
#include "Render/PS4/PS4_Config.h"

#include <unistd.h>
#include <sdk_version.h>

namespace Scaleform { namespace Render { namespace PS4 { 

class HAL;

class RenderSync : public Render::RenderSync
{
public:
    RenderSync(PS4::HAL* hal);

    void SetContext(sceGnmxContextType* context, Render::MemoryManager& memoryManager);
    sceGnmxContextType* GetContext() const  { return GnmxCtx; }
    virtual void    KickOffFences(FenceType waitType);

protected:

    virtual UInt64  SetFence();    
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent);
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent);

private:
    PS4::HAL*              pHAL;
    sceGnmxContextType*    GnmxCtx;
    UInt64                 CurFence;
    UInt64*                pLabelAddress;
};

}}}; // Scaleform::Render::PS4

#endif // INC_SF_PS4_Sync_H
