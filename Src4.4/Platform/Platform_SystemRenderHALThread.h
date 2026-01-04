/**************************************************************************

Filename    :   Platform_SystemRenderHALThread.h
Content     :   
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_SystemRenderHALThread_H
#define INC_SF_Platform_SystemRenderHALThread_H

#include "Render/Render_ThreadCommandQueue.h"
#include "Platform_RTCommandQueue.h"
#include "Platform.h"


namespace Scaleform { namespace Platform {

class Device;

#define SF_PLATFORM_THREADING_TYPE Scaleform::Platform::RTCommandQueue::MultiThreaded

//------------------------------------------------------------------------
// ***** RenderHALThread

// RenderThreadBase includes render thread setup logic associated with Renderer HAL,
// without rendering the tree (the later functionality is provided by RenderThread
// derived class).


class RenderHALThread : public Render::ThreadCommandQueue,
                        public RTCommandQueue,
                        public RefCountBase<RenderHALThread, Stat_Default_Mem>
{
public:
    RenderHALThread(ThreadingType threadingType = MultiThreaded);
    virtual ~RenderHALThread();

    bool    IsSingleThreaded() const { return GetThreadingType() != MultiThreaded; }

    // Device must be applied before RenderThread is started.
    void    SetDevice(Platform::Device* device)
    { pDevice = device; }

    void    StartThread() {}
    int     Run();

    ThreadId       GetThreadId() const { return 0; }


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
    void    SetConfigAndWindow(const ViewConfig& config, Device::Window* window);
    bool    InitGraphics(const ViewConfig& config, Device::Window* window,
                         ThreadId renderThreadId = 0);
    bool    ReconfigureGraphics(const Platform::ViewConfig& config) { return 1; }
    void    DestroyGraphics();

    bool    InitGraphics_RenderThread();
    bool    OnSize_RenderThread(unsigned w, unsigned h);

    // Queues up UpdateDeviceStatus call. For D3D9, this is executed
    // on the Blocked thread. For Lost devices users should
    // issue this call occasionally to see if operation can be resumed.
    void    UpdateDeviceStatus() {}
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

    void    SetStereoParams(const Render::StereoParams& sparams)
    {
        PushCall(&RenderHALThread::setStereoParams, sparams);
    }

    void    ExitThread()
    {
        PushCallAndWait(&RenderHALThread::exitThread);
    }


    void    DrawFrame() {}
    void    CaptureFrameWithoutDraw();
    void    DrawFrame_RenderThread();
    void    WaitForOutstandingDrawFrame();

    void    ToggleWireframe()
    {
        Wireframe = !Wireframe;
    }
    void	SetWireframe(bool wireframe)
    {
        Wireframe = wireframe;
    }

    bool    TakeScreenShot(const String& filename)
    {
        return false;
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

    // Creates an HMatrix, through the render thread's matrix pool.
    virtual Render::HMatrix CreateMatrix() { return Render::HMatrix(); }

    // Creates a mesh. Must be executed on the render thread.
    virtual Render::Mesh*   CreateMesh(Render::MeshProvider*, const Render::Matrix2F&)   { return 0; }

    // Render::ThreadCommandQueue implementation.
    virtual void PushThreadCommand(Render::ThreadCommand* command);

    void AbortFrame();
    

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

    Render::HAL* GetHAL() { return pDevice->GetHAL(); }

protected:
    virtual bool initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId);
    virtual bool reconfigureGraphics(const ViewConfig& config);
    virtual void destroyGraphics();
    bool        adjustViewConfig(ViewConfig* config);
    void        updateDeviceStatus()
    { Status = pDevice->GetStatus(); }

    virtual void        updateConfiguration()
    {
        pWindow->GetViewConfig(&VConfig);
        if (!VConfig.HasFlag(View_Stereo))
            getHAL()->SetStereoDisplay(Render::StereoCenter, 1);
    }

    void         blockForGraphicsInit();
    void         runCommands();
    void         exitThread();
    void         setBackgroundColor(Render::Color bgColor) { BGColor = bgColor; }
    void         setStereoParams(Render::StereoParams sparams);
    Render::Color getBackgroundColor() const { return BGColor; }

    Render::HAL* getHAL() const { return pDevice->GetHAL(); }   

    virtual void drawFrame() = 0;
    virtual void captureFrameWithoutDraw() = 0;
    virtual void createCursorPrimitives() {};
    virtual void updateCursor(const Point<int> mousePos, SystemCursorState state);

    void    executeThreadCommand(const Ptr<Render::ThreadCommand>& command);

    virtual void getMeshCacheParams(Render::MeshCacheParams* params);
    virtual void setMeshCacheParams(const Render::MeshCacheParams& params);

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
    unsigned                GlyphRasterCount;
    bool                    ResetGlyphRasterCount;
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
};

}}
#endif
