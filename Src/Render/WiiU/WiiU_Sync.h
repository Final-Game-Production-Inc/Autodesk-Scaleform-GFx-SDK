/**********************************************************************

PublicHeader:   Render
Filename    :   WiiU_Sync.h
Content     :   WiiU specific fencing implementation.
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_WiiU_Sync_H
#define INC_SF_WiiU_Sync_H

#include "Render/Render_Sync.h"
#include <cafe/gx2.h>

namespace Scaleform { namespace Render { namespace WiiU { 

class RenderSync : public Render::RenderSync
{
public:

    virtual void    KickOffFences(FenceType waitType)     
    {  
        SF_UNUSED(waitType);
        GX2Flush();
    }

protected:

    virtual UInt64   SetFence()
    { 
        GX2Flush(); 
        return GX2GetLastSubmittedTimeStamp();
    }
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, parent);
        u64 current = GX2GetRetiredTimeStamp();
        return current < handle;
    }
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        GX2WaitTimeStamp(handle);
    }

    // No need to implement wraparound; Timestap sizes are 64bits, should take decades to wraparound.
};

}}}; // Scaleform::Render::WiiU

#endif // INC_SF_WiiU_Sync_H
