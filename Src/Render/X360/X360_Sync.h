/**********************************************************************

PublicHeader:   Render
Filename    :   X360_Sync.h
Content     :   X360 specific fencing implementation.
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_X360_Sync_H
#define INC_SF_X360_Sync_H

#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render { namespace X360 { 

class RenderSync : public Render::RenderSync
{
public:
    RenderSync() : pDevice(0) { };    

    void              SetDevice(IDirect3DDevice9* pdevice) { pDevice = pdevice; }
    IDirect3DDevice9* GetDevice() const                    { return pDevice; }

    virtual void    KickOffFences(FenceType waitType)     
    {  
        SF_UNUSED(waitType);
        pDevice->InsertFence(); // Causes a kickoff.
    }

protected:

    virtual UInt64   SetFence()
    { 
        return pDevice->GetCurrentFence();
    }
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, parent);
        return pDevice->IsFencePending((DWORD)handle) != 0;
    }
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        pDevice->BlockOnFence((DWORD)handle);
    }

    // No need to implement wraparound; X360 should handle the wraparound internally.

private:
    Ptr<IDirect3DDevice9> pDevice;
};

}}}; // Scaleform::Render::X360

#endif // INC_SF_X360_Sync_H
