/**********************************************************************

PublicHeader:   Render
Filename    :   Wii_Sync.h
Content     :   Wii specific fencing implementation.
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Wii_Sync_H
#define INC_SF_Wii_Sync_H

#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render { namespace Wii { 

class RenderSync_DrawSync : public Render::RenderSync
{
    UInt32                  Fence;
    static UInt32           LastV, LastW;
    static OSThreadQueue    WaitQ;
    static bool             Init;
    static unsigned         Count;
    static UInt32           WaitOn;

    static void DrawSyncHandler (UInt16 value)
    {
        if (value < LastV)
            LastW++;
        LastV = value;
        if (WaitOn && (((LastV | (LastW<<16)) >= WaitOn) || (LastW == 0 && (WaitOn & 0xffff0000))))
            OSWakeupThread(&WaitQ);
    }

public:

    RenderSync_DrawSync()
    {
        Fence = 0;
        LastW = 0;
        if (!Init)
        {
            OSInitThreadQueue(&WaitQ);
            Init = 1;
            GXSetDrawSyncCallback(DrawSyncHandler);
        }
        SF_DEBUG_WARNING(Count, "multiple use of RenderSync_DrawSync");
        Count++;
    }
    virtual ~RenderSync_DrawSync() 
    { 
        Count--; 
    }

    virtual void     BeginFrame()
    {
        Render::RenderSync::BeginFrame();
        if (LastW)
            Fence = 0;
        LastW = 0;
    }

    virtual void    KickOffFences(FenceType waitType)     
    {  
        SF_UNUSED(waitType);
        GXFlush();
    }

protected:

    virtual UInt64   SetFence()
    { 
        GXSetDrawSync(Fence & 0xffff);
        Fence++;
        return Fence;
    }

    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, parent);
        return !(((LastV | (LastW<<16)) >= handle) || (LastW == 0 && (handle & 0xffff0000)));
    }

    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        while ((LastV | (LastW<<16)) < handle || (LastW == 0 && (handle & 0xffff0000)))
        {
            GXFlush();
            bool v = OSDisableInterrupts();
            if ((LastV | (LastW<<16)) < handle || (LastW == 0 && (handle & 0xffff0000)))
            {
                WaitOn = handle;
                OSSleepThread(&WaitQ);
                WaitOn = 0;
            }
            OSRestoreInterrupts(v);
        }
    }
};

// This implementation simply always waits for drawing to be completed. All fences always report being pending.
class RenderSync_DrawDone : public Render::RenderSync
{
public:
	RenderSync_DrawDone() : CurrentFence(1) { }

	virtual void    BeginFrame()
	{
		Render::RenderSync::BeginFrame();
		
		// DrawDone will not clear frames, if it never calls KickOffFences. However, it is
		// undesirable to call this to clear the frames, because it causes the CPU to stall.
		// Instead, assume that any frames older than 3 frames ago have completely passed.
		while ( OutstandingFrames > 3 )
		{
		    FenceFrame* passFrame = FenceFrames.GetFirst();
	        FenceFrame * next = passFrame->pNext;
	        passFrame->RemoveNode();
	        FenceFrameAlloc.Free(passFrame);
	        passFrame = next;
	        OutstandingFrames--;
		}
	}
	
    virtual void    KickOffFences(FenceType waitType)     
    {  
        GXDrawDone();
        CurrentFence++;
    }
    
protected:
    virtual UInt64   SetFence()
    { 
        return CurrentFence;
    }
    
    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, handle);
		return handle >= CurrentFence;
    }
    
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(handle, parent);
        KickOffFences(waitType);
    }
    
    UInt64 CurrentFence;
};

}}}; // Scaleform::Render::Wii

#endif // INC_SF_Wii_Sync_H
