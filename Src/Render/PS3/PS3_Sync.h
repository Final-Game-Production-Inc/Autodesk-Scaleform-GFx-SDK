/**********************************************************************

PublicHeader:   Render
Filename    :   PS3_Sync.h
Content     :   PS3 specific fencing implementation.
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS3_Sync_H
#define INC_SF_PS3_Sync_H

#include "Render/Render_Sync.h"
#include "Kernel/SF_Debug.h"
#include <cell/gcm.h>

namespace Scaleform { namespace Render { namespace PS3 { 

class RenderSync : public Render::RenderSync
{
public:
    RenderSync() : GcmCtx(0), Label(250), CurFence(0), pLabel(0) { };    

    void SetContext(CellGcmContextData* gcm, UInt32 label)
    {
        GcmCtx = gcm;
        Label = label;
        pLabel = (volatile UInt32 *) cellGcmGetLabelAddress(Label);
        CurFence = 0;
    }
    CellGcmContextData* GetContext() const  { return GcmCtx; }

    virtual void    KickOffFences(FenceType waitType)     
    {  
        SF_UNUSED(waitType);
        cellGcmFlush(GcmCtx);
    }

protected:

    virtual UInt64   SetFence()
    { 
        ++CurFence;
        cellGcmSetWriteTextureLabel(GcmCtx, Label, CurFence); 
        return CurFence;

    }
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, parent);
        return handle > *pLabel;
    }

    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        // Wait for the label to be written.
        KickOffFences(waitType);

        unsigned totalWaitTime = 0;
        const unsigned MaxWaitTime = 100 * 1000; // 100ms maximum wait.
        while (IsPending(waitType, handle, parent))
        {
            sys_timer_usleep(10);        
            totalWaitTime += 10;
            if ( totalWaitTime > MaxWaitTime)
            {
                SF_DEBUG_WARNING(1, "Failing waiting on cellGcmSetWriteTextureLabel value. Continuing.\n");
                break;
            }
        }
    }

    virtual bool CheckWraparound(UInt64 handle)
    {
        if ( handle > 0xEFFFFFFF)
        {
            CurFence = 0;
            return true;
        }
        return false;
    }

private:
    CellGcmContextData*  GcmCtx;
    UPInt                Label, CurFence;
    volatile UInt32*     pLabel;
};

}}}; // Scaleform::Render::PS3

#endif // INC_SF_PS3_Sync_H
