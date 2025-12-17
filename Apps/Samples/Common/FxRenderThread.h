/**************************************************************************

Filename    :   FxRenderThread.h
Content     :   RenderThread class used by GFxPlayer. Includes extra
                HUD support and timing logic.
Created     :   Dec, 2010
Authors     :   Art Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_FxRenderThread_H
#define SF_FxRenderThread_H

#include "Kernel/SF_Types.h"
#include "Platform/Platform_RenderThread.h"

using namespace Scaleform;

// Render thread class.
class FxRenderThread : public Platform::RenderThread
{
public:
    FxRenderThread(Scaleform::RTCommandQueue::ThreadingType threadingType);
    virtual ~FxRenderThread() {}

    void SetProfileMode(Render::ProfilerModes mode)         { ProfileMode = mode; }
    void SetProfileFlag(unsigned flag, bool state)          { ProfileFlags = state ? ProfileFlags |= flag : ProfileFlags &= ~flag; }
    unsigned GetProfileFlag( unsigned flag )                { return ProfileFlags & flag; }
    void SetBatchHighlight(int highlight)                   { BatchHighlight = highlight; }


    // A separate display handle is maintained explicitly for HUD.
    void    SetHUDDisplayHandle(const Render::TreeRootDisplayHandle& handle)
    {
        if (handle == 0) RemoveDisplayHandle(DHCAT_Overlay);
        else             AddDisplayHandle(handle, DHCAT_Overlay);
    }

    // User matrix for main movie.
    void    SetUserMatrix(const Render::Matrix2F& m)
    {
        PushCall(&FxRenderThread::setUserMatrix, m);
    }
    void    SetHUDUserMatrix(const Render::Matrix2F& m)
    {
        PushCall(&FxRenderThread::setHUDUserMatrix, m);
    }

    // Queues up Present call without draw, only used for windowed resize.
    void    PresentOnResize()
    {
        PushCall(&FxRenderThread::presentOnResize);
    }

    // LastDisplayTicks is the time directly associated with displaying
    // the tree, which is the cost of {NextCapture + Display}.
    // LastDrawFrameTicks is the timing for the entire DrawFrame call,
    // which includes the HUD, Clear and Present logic.
    UInt64      GetLastDisplayTicks() const   { return LastDisplayTicks; }
    UInt64      GetLastDrawFrameTicks() const { return LastDrawFrameTicks; }
    
protected:    

    void setUserMatrix(const Render::Matrix2F& m)
    {
        UserMatrix = m;
    }
    void setHUDUserMatrix(const Render::Matrix2F& m)
    {
        HUDUserMatrix = m;
    }

    // Override drawFrame to provide custom implementation.
    virtual void drawFrame();
    virtual void drawDisplayHandle(Platform::RenderThread::DisplayHandleDesc& desc,
                                   const Render::Viewport& vp, bool capture);
    virtual void finishFrame();

    virtual Render::ProfilerModes   getProfileMode() const           { return ProfileMode; }
    virtual unsigned                getProfileFlags() const          { return ProfileFlags; }
    virtual int                     getProfileBatchHighlight() const { return BatchHighlight; }

    void         presentOnResize();

private:    
    Render::Matrix2F                UserMatrix;
    Render::Matrix2F				HUDUserMatrix;

    UInt32                          LastDisplayTicks;
    UInt32                          LastDrawFrameTicks;

    UInt32                          LastDisplayTicksAccum;
    UInt32                          LastDrawFrameTicksAccum;

    Render::ProfilerModes           ProfileMode;
    unsigned                        ProfileFlags;
    int                             BatchHighlight;

  //  FxRenderThread& operator=(const FxRenderThread&) { return *this; }
};


#endif // SF_FxRenderThread_H
