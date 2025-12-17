/**************************************************************************

Filename    :   Platform_RenderHALThread.h
Content     :   Default RenderThread implementation used by applications
                that render directly to HAL.
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_RenderHALThread_H
#define INC_SF_Platform_RenderHALThread_H

#include "Render/Renderer2D.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Render/Render_Profiler.h"
#include "Platform_RTCommandQueue.h"
#include "Platform.h"

#ifdef SF_OS_ANDROID
#error Use Platform_SystemRenderThread.h instead
#endif

namespace Scaleform { namespace Platform {

class Device;
// 
    
#ifdef RENDERER_SINGLE_THREADED
#define SF_PLATFORM_THREADING_TYPE Scaleform::RTCommandQueue::SingleThreaded
#else
#define SF_PLATFORM_THREADING_TYPE Scaleform::RTCommandQueue::MultiThreaded
#endif


//------------------------------------------------------------------------
// ***** RenderHALThread

// RenderThreadBase includes render thread setup logic associated with Renderer HAL,
// without rendering the tree (the later functionality is provided by RenderThread
// derived class).


class RenderHALThread : public Thread, public Scaleform::RTCommandQueue,
                        public Render::ThreadCommandQueue
{
public:

    struct DisplayWindow : public NewOverrideBase<Stat_Default_Mem>
    {
        Device::Window*         pWindow;
        ViewConfig              VConfig;
        Size<unsigned>          ViewSize;

        DisplayWindow(Device::Window* pwin)
            : pWindow(pwin)
        {
            pWindow->GetViewConfig(&VConfig);
            ViewSize = VConfig.ViewSize;
        }

        virtual ~DisplayWindow() {}
    };

    RenderHALThread(ThreadingType threadingType = SF_PLATFORM_THREADING_TYPE);

    bool    IsSingleThreaded() const { return GetThreadingType() != MultiThreaded; }

    // Device must be applied before RenderThread is started.
    void    SetDevice(Platform::Device* device)
    { pDevice = device; }

    // Starts thread, but only if it's in multi-threaded mode.
    void    StartThread()
    {
        if (!IsSingleThreaded())
        {
            if ( Start() )
            {
                SetThreadName("Scaleform Renderer");
                SetRenderThreadId(GetThreadId());
            }
        }
        else
            SetRenderThreadId(GetCurrentThreadId());
    }

    void GetRenderStats(Render::HAL::Stats* pstats) 
    { 
        Lock::Locker lock(&RenderStatsLock);
        *pstats = RenderStats; 
    }    
    void GetMeshCacheStats(Render::MeshCache::Stats* pstats) 
    { 
        Lock::Locker lock(&RenderStatsLock);
        *pstats = MeshCacheStats; 
    }    

    // Adjusts view to match allowed HW resolutions and/or HW size.
    // Applies default size if not specified.
    bool    AdjustViewConfig(ViewConfig* config);

    bool    InitGraphics(const ViewConfig& config, Device::Window* window,
                         ThreadId renderThreadId = 0);
    void    ResizeFrame(void* layer);
    bool    ReconfigureGraphics(const ViewConfig& config);
    void    DestroyGraphics();

    // Queues up UpdateDeviceStatus call. For D3D9, this is executed
    // on the Blocked thread. For Lost devices users should
    // issue this call occasionally to see if operation can be resumed.
    void    UpdateDeviceStatus();
    void    UpdateConfiguration();

    // Returns most recent device status. This is updated by RenderThread
    // after mode configurations, DrawFrame and UpdateStatus calls.
    DeviceStatus GetDeviceStatus() const { return Status; }

    

    void    SetBackgroundColor(Render::Color bgColor)
    {
        PushCall(&RenderHALThread::setBackgroundColor, bgColor);
    }

    void    UpdateCursor(const Point<int>& mousePos, SystemCursorState state)
    {
        PushCall(&RenderHALThread::updateCursor, mousePos, state);
    }
    void    ExitThread()
    {
        PushCallAndWait(&RenderHALThread::exitThread);
    }

    void    SetStereoParams(const Render::StereoParams& sparams)
    {
        PushCall(&RenderHALThread::setStereoParams, sparams);
    }

    void    DrawFrame();
    void    WaitForOutstandingDrawFrame();
    void    FinishFrame();
    
    void    ToggleWireframe()
    {
        Wireframe = !Wireframe;
    }
	void	SetWireframe(bool wireframe)
	{
		Wireframe = wireframe;
	}

    void        SetProfileMode(Render::ProfilerModes mode)
    {
        PushCall(&RenderHALThread::setProfileMode, mode);
    }

    void        SetProfileFlag(unsigned flag, bool state)
    {
        PushCall(&RenderHALThread::setProfileFlag, flag, state);
    }

    unsigned    GetProfileFlag( unsigned flag )
    {
        unsigned result;
        PushCallAndWaitResult(&RenderHALThread::getProfileFlag, &result, flag);
        return result;
    }

    unsigned GetFrames()
    {
        return Frames.Exchange_NoSync(0);
    }

    // TextureManager can be accessed after InitGraphics is called and
    // before DestroyGraphics. Texture creation should be thread-safe.
    Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager;
    }

    // Get/Set mesh cache parameters.
    Render::MeshCacheParams GetMeshCacheParams()
    {
        Render::MeshCacheParams params;
        PushCallAndWait(&RenderHALThread::getMeshCacheParams, &params);
        return params;
    }
    void SetMeshCacheParams(const Render::MeshCacheParams& params)
    {
        PushCall(&RenderHALThread::setMeshCacheParams, params);
    }

    virtual bool TakeScreenShot(const String& filename)
    {
        bool returnValue;
        PushCallAndWaitResult(&RenderHALThread::takeScreenShot, &returnValue, filename );
        return returnValue;
    }

    virtual int Run();

    // Render::ThreadCommandQueue implementation.
    virtual void PushThreadCommand(Render::ThreadCommand* command);

protected:

    bool        adjustViewConfig(ViewConfig* config);
    void        updateDeviceStatus()
    { Status = pDevice->GetStatus(); }

    virtual void updateConfiguration()
    {
        pWindow->GetViewConfig(&VConfig);
        if (!VConfig.HasFlag(View_Stereo))
            getHAL()->SetStereoDisplay(Render::StereoCenter, 1);
    }
    
    virtual bool initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId);
    virtual void resizeFrame(void* layer);
    virtual bool reconfigureGraphics(const ViewConfig& config);
    virtual void destroyGraphics();
    void         blockForGraphicsInit();
    void         exitThread();
    void         setBackgroundColor(Render::Color bgColor) { BGColor = bgColor; }
    void         setStereoParams(Render::StereoParams sparams);
    void         setProfileMode(Render::ProfilerModes mode);
    void         setProfileFlag(unsigned flag, bool state);
    unsigned     getProfileFlag(unsigned flag);
    Render::Color getBackgroundColor() const { return BGColor; }

    Render::HAL* getHAL() const { return pDevice->GetHAL(); }   

    virtual void drawFrame() = 0;
    virtual void finishFrame() = 0;
    virtual void createCursorPrimitives(Render::HAL*) {};
    virtual void updateCursor(const Point<int> mousePos, SystemCursorState state);
    virtual void getMeshCacheParams(Render::MeshCacheParams* params);
    virtual void setMeshCacheParams(const Render::MeshCacheParams& params);
    virtual bool takeScreenShot(const String& filename);

    // Overload to hide Thread::Start method.
    virtual bool  Start(ThreadState initialState = Running)
    { 
        if ( !Thread::Start(initialState))
            return false;
        SetThreadName("Scaleform Renderer");
        return true;
    }

    void    executeThreadCommand(const Ptr<Render::ThreadCommand>& command);

    // Helper class used to block RenderThread for duration of its scope.
    class RTBlockScope
    {
        RenderHALThread* pThread;
    public:
        RTBlockScope(RenderHALThread* thread);
        ~RTBlockScope();
    };
    friend class RTBlockScope;

    //--------------------------------------------------------------------
    // ***** Members

    Platform::Device*       pDevice;
    Platform::Device::Window* pWindow;

    // TextureManger intended to be accessed from Advance thread,
    // between the initGraphics/destroyGraphics calls.
    Ptr<Render::TextureManager> pTextureManager;
    volatile DeviceStatus   Status;

    bool                    Wireframe;
	
    // Real-time render stats, not synchronized.
    Lock                    RenderStatsLock;
    Render::HAL::Stats      RenderStats;
    Render::MeshCache::Stats MeshCacheStats;
    AtomicInt<unsigned>     Frames;
    
    Size<unsigned>          ViewSize;
    Render::Color           BGColor;
    ViewConfig              VConfig;

    Event                   DrawFrameDone;
    bool                    DrawFrameEnqueued;

    // These events are used for blocking/resuming render thread
    // during maind-thread graphics configuration.
    Event                   RTBlocked, RTResume;
    bool                    RTBlockedFlag;    

    SystemCursorState       CursorState;                    // Holds current cursor state (copied from main thread's manager).
    Ptr<Render::Primitive>  CursorPrims[Cursor_Type_Count]; // Primitives for rendering software cursors.
    Render::HMatrix         CursorMats[Cursor_Type_Count];  // Matrices for cursors


    volatile unsigned       WatchDogTrigger;                // Indicates whether the watchdog is satisfied (false == unsatisfied)
    static const int        WatchDogInterval = 5000;        // Time between watchdog checks (ms)
    static const int        WatchDogMaxFailureCount = 12;   // Maximum number of watchdog failures before killing.
    Thread                  WatchDogThread;                 // Thread object performing watchdog checks.

    static int              watchDogThreadFn(Thread* thread, void* trigger);
};


}} // Scaleform::Platform

#endif //INC_SF_Platform_RenderHALThread_H
