/**************************************************************************

Filename    :   Platform_RenderThread.h
Content     :   Default RenderThread implementation used by applications.
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_RenderThread_H
#define INC_SF_Platform_RenderThread_H

#include "Kernel/SF_Types.h"
#ifdef SF_OS_ANDROID
#include "Platform_SystemRenderHALThread.h"
#else
#include "Platform_RenderHALThread.h"
#endif
#include "Render/Render_Primitive.h"
#include "Render/Render_GlyphCacheConfig.h"

namespace Scaleform { namespace Platform {

class DisplayCallBack
{
public:
    virtual ~DisplayCallBack() { }
    virtual void OnDisplay(Render::Renderer2D* ren) = 0;
    virtual void OnShutdown(Render::Renderer2D* ren) = 0;
};


//------------------------------------------------------------------------
// ***** RenderThread

// RenderThread adds Renderer2D support, allowing rendering of a display handle.

class RenderThread : public RenderHALThread
{
public:    
    typedef     Render::TreeRootDisplayHandle       DisplayHandleType;

    enum    DisplayHandleCategory
    {
        DHCAT_Normal,
        DHCAT_Overlay,
    };

    struct  DisplayHandleDesc
    {
        // TreeRoot handle describing item to draw.
        DisplayHandleCategory   Category;
        DisplayHandleType       hRoot;
        DisplayCallBack*        pOnDisplay;
        Device::Window*         pWindow;

        DisplayHandleDesc() : Category(DHCAT_Normal), hRoot(0), pOnDisplay(0) {}
        DisplayHandleDesc(DisplayHandleCategory cat,
                          const DisplayHandleType& hroot, 
                          DisplayCallBack* pondisp = 0, Device::Window* pwin = 0)
        : Category(cat), hRoot(hroot), pOnDisplay(pondisp), pWindow(pwin) {}
    };
    typedef     Array<DisplayHandleDesc>     DHContainerType;

    struct DisplayWindow : public NewOverrideBase<Stat_Default_Mem>
    {
        Device::Window*         pWindow;
        ViewConfig              VConfig;
        Size<unsigned>          ViewSize;

        // The handles will be rendered in-order (ascending)
        DHContainerType         NormalHandles;
        DHContainerType         OverlayHandles;

        DisplayWindow(Device::Window* pwin)
            : pWindow(pwin)
        {
            pWindow->GetViewConfig(&VConfig);
            ViewSize = VConfig.ViewSize;
        }
        virtual ~DisplayWindow() {}

        DHContainerType&    getDHContainer(DisplayHandleCategory cat);
    };

    RenderThread(RTCommandQueue::ThreadingType threadingType = SF_PLATFORM_THREADING_TYPE);

    void    SetViewportFlags(UInt32 vpflags)
    {
        PushCall(&RenderThread::setViewportFlags, vpflags);
    }

    void    AddDisplayHandle(const DisplayHandleType& handle, 
                             DisplayHandleCategory cat = DHCAT_Normal, 
                             bool clearBeforeAdd = true,
                             DisplayCallBack* dcb = 0,
                             Device::Window* pwindow = 0)
    {
        PushCall(&RenderThread::addDisplayHandle, handle, cat, clearBeforeAdd, dcb, pwindow);
    }

    // If handle is not specified, the remove operation is equivalent to a pop-back operation
    void    RemoveDisplayHandle(DisplayHandleCategory cat = DHCAT_Normal)
    {
        PushCall(&RenderThread::popDisplayHandle, cat);
    }
    void    RemoveDisplayHandle(const DisplayHandleType& handle, 
                                DisplayHandleCategory cat = DHCAT_Normal)
    {
        PushCall(&RenderThread::removeDisplayHandle, handle, cat);
    }

    void    ClearDisplayHandles(DisplayHandleCategory cat = DHCAT_Normal)
    {
        PushCall(&RenderThread::clearDisplayHandles, cat);
    }

    void    SetGlyphCacheParams(const Render::GlyphCacheParams& params)
    {
        PushCall(&RenderThread::setGlyphCacheParams, params);
    }

    void    SetToleranceParams(const Render::ToleranceParams& params)
    {
        PushCall(&RenderThread::setToleranceParams, params);
    }
	void    GetToleranceParams(Render::ToleranceParams* params)
	{
		PushCallAndWait(&RenderThread::getToleranceParams, params);
	}

    unsigned GetGlyphRasterizationCount() const;
    void     ResetRasterizationCount();

    virtual int Run();

    Render::Renderer2D* GetRenderer2D() {return pRenderer;}

protected:
    virtual bool initGraphics(const Platform::ViewConfig& config, Device::Window* window,
                              ThreadId renderThread);
    virtual void destroyGraphics();

    
    void    setGlyphCacheParams(const Render::GlyphCacheParams& params);
    void    setToleranceParams(const Render::ToleranceParams& params);
	void    getToleranceParams(Render::ToleranceParams* params);


    virtual void updateConfiguration()
    {
        for (unsigned j = 0; j < Windows.GetSize(); j++)
        {
            DisplayWindow* win = Windows[j];
            win->pWindow->GetViewConfig(&win->VConfig);
            win->ViewSize = win->VConfig.ViewSize;
        }
        ViewSize = Windows[0]->ViewSize;
        VConfig = Windows[0]->VConfig;
        if (!VConfig.HasFlag(View_Stereo))
            getHAL()->SetStereoDisplay(Render::StereoCenter, 1);
    }

    void         addDisplayHandle(const DisplayHandleType& root, DisplayHandleCategory cat, bool clearBeforeAdd,
                                  DisplayCallBack* dcb, Device::Window* pwindow);
    void         removeDisplayHandle(const DisplayHandleType& root, DisplayHandleCategory cat);
    void         popDisplayHandle(DisplayHandleCategory cat);
    void         clearDisplayHandles(DisplayHandleCategory cat);
    void         clearDisplayHandles(DisplayWindow* pDispWin, DisplayHandleCategory cat);

    DisplayWindow* createWindowData(Device::Window* win) const { return SF_NEW DisplayWindow(win); }

    virtual void finishFrame();
    virtual void drawFrame();
    virtual void drawFrame(DisplayWindow* pdispwin);
    virtual void drawFrame1(DisplayWindow* pdispwin, bool capture);
    virtual void drawDisplayHandle(DisplayHandleDesc& desc, const Render::Viewport& vp, bool capture);
    virtual void createCursorPrimitives(Render::HAL* pHal);

    virtual Render::ProfilerModes   getProfileMode() const { return Render::Profile_None; }
    virtual unsigned                getProfileFlags() const { return 0; }
    virtual int                     getProfileBatchHighlight() const { return -1; }

    void         notifyShutdown(DHContainerType& arr) const;

    void         setViewportFlags(UInt32 vpflags)
    {
        ViewportFlags = vpflags;
    }

    virtual void GetRenderInterfaces(Render::Interfaces* p);

    Ptr<Render::Renderer2D>     pRenderer;
    Render::GlyphCacheParams    GCParams;  // Parameters copy stored before Renderer2D creation.
    Render::ToleranceParams     TolParams; // Parameters copy stored before Renderer2D creation.
    UInt32                      ViewportFlags;
    unsigned                    PresentMode;

    Array<DisplayWindow*>       Windows;
};

//--------------------------------------------------------------------

// FrameStatsTracker is a helper class that helps track and report App
// title-bar statistics.

class FrameStatsTracker
{
    double      PrevTimer;
    unsigned    Frames;
    bool        FirstFrame;

public:
    FrameStatsTracker()
        : PrevTimer(0), Frames(0), FirstFrame(true)
    { }

    void    Initialize(double timer)
    {
        PrevTimer = timer;
        Frames = 0;
        FirstFrame = true;
    }

    bool    NeedsUpdate(double timer) const
    {
        return ((timer-PrevTimer) >= 1.0) || FirstFrame;
    }

    void    UpdateStats(Platform::RenderHALThread* rt, String* str, double timer)
    {
        char buffer[512];
        Render::HAL::Stats rstats;
        rt->GetRenderStats(&rstats);

        if (rt->GetThreadingType() == Platform::RenderHALThread::SingleThreaded)
        {
            Scaleform::SFsprintf(buffer, 512, "FPS:%d  (M=%d, P=%d, T=%d)",
                Frames, rstats.Meshes, rstats.Primitives,rstats.Triangles);
        }
        else
        {
            Scaleform::SFsprintf(buffer, 512, "A_FPS:%d, R_FPS:%d  (M=%d, P=%d, T=%d)",
                Frames, rt->GetFrames(),
                rstats.Meshes, rstats.Primitives,rstats.Triangles);
        }
        *str = buffer;
        PrevTimer = timer;
        Frames = 0;
        FirstFrame = false;
    }

    void    NextFrame()
    {
        Frames++;
    }

};

}} // Scaleform::Platform

#endif //INC_SF_Platform_RenderThread_H
