/**************************************************************************

Filename    :   Platform_RenderHALThread.cpp
Content     :   Default RenderThread implementation used by applications.
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform_RenderHALThread.h"
#include "Render/Render_MeshCache.h"

namespace Scaleform { namespace Platform {

enum WatchDogState
{
    WatchDog_Unsignaled  = 0,
    WatchDog_Signaled,
    WatchDog_ExitThread
};

//------------------------------------------------------------------------
// ***** RenderHALThread

RenderHALThread::RenderHALThread(RTCommandQueue::ThreadingType threadingType)
: Thread(256 * 1024, 1), // 256k stack size, create on processor #1.
  RTCommandQueue((RTCommandQueue::ThreadingType)(threadingType & TT_TypeMask)),
  pDevice(0),
  pTextureManager(0),
  Status(Device_NeedInit),
  Wireframe(false),
  Frames(0),
  BGColor(0),
  DrawFrameEnqueued(false),
  RTBlocked(false), RTResume(false), RTBlockedFlag(false),
  CursorState(true),
  WatchDogTrigger(WatchDog_Signaled),
  WatchDogThread(0)
{        
    memset(CursorPrims, 0, sizeof CursorPrims);

    if (threadingType & TT_WatchDogFlag)
    {
        WatchDogThread = *new Scaleform::Thread(watchDogThreadFn, (void*)&WatchDogTrigger);
        WatchDogThread->Start();
    }
}

RenderHALThread::~RenderHALThread()
{
    // Wait for the watchdog to die.
    if (WatchDogThread && WatchDogThread->GetThreadState() == Running)
    {
        WatchDogThread->SetExitFlag(true);
        WatchDogTrigger = WatchDog_ExitThread;
        WatchDogThread->Wait();
        WatchDogThread.Clear();
    }
}

int RenderHALThread::Run()
{
    RTCommandBuffer cmd;

    do {
        if (PopCommand(&cmd))
        {
            cmd.Execute(*this);
            if (cmd.NeedsWait())
                cmd.GetNotifier()->Notify();
        }   

        // Ticket the watchdog.
        AtomicOps<unsigned>::Store_Release(&WatchDogTrigger, WatchDog_Signaled);

    } while(!IsProcessingStopped());
    
    return 0;
}

// RTBlockScope helper class used to block RenderThread for duration of its scope.
RenderHALThread::RTBlockScope::RTBlockScope(RenderHALThread* thread)
: pThread(thread)
{
    // If the RT is already blocked, don't ask for it to block again.
    if ( !pThread->RTBlockedFlag )
    {
        pThread->PushCall(&RenderHALThread::blockForGraphicsInit);
        pThread->RTBlocked.Wait();
    }
    pThread->RTBlockedFlag = true;
}

RenderHALThread::RTBlockScope::~RTBlockScope()
{
    pThread->RTBlockedFlag = false;
    pThread->RTResume.PulseEvent();
}


bool RenderHALThread::AdjustViewConfig(ViewConfig* config)
{
    SF_ASSERT(pDevice);
    bool ok = false;
    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        PushCallAndWaitResult(&RenderHALThread::adjustViewConfig, &ok, config);
    }
    else
    {
        RTBlockScope rtblock(this);
        ok = adjustViewConfig(config);
    }
    return ok;
}

bool RenderHALThread::InitGraphics(const Platform::ViewConfig& config, Device::Window* window,
                                   ThreadId renderThreadId)
{
    SF_ASSERT(pDevice);
    bool ok = false;
    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        PushCallAndWaitResult(&RenderHALThread::initGraphics, &ok, config, window, renderThreadId);
    }
    else
    {
        RTBlockScope rtblock(this);
        ok = initGraphics(config, window, renderThreadId);
    }

    return ok;
}

void RenderHALThread::ResizeFrame(void* layer)
{
    SF_ASSERT(pDevice);
    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        PushCallAndWait(&RenderHALThread::resizeFrame, layer);
    }
    else
    {
        RTBlockScope rtblock(this);
        resizeFrame(layer);
    }
}

bool RenderHALThread::ReconfigureGraphics(const Platform::ViewConfig& config)
{
    SF_ASSERT(pDevice);
    bool ok = false;
    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        PushCallAndWaitResult(&RenderHALThread::reconfigureGraphics, &ok, config);
    }
    else
    {
        RTBlockScope rtblock(this);
        ok = reconfigureGraphics(config);
    }
    return ok;
}

void RenderHALThread::DestroyGraphics()
{
    SF_ASSERT(pDevice);

    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        PushCallAndWait(&RenderHALThread::destroyGraphics);
    }
    else
    {
        RTBlockScope rtblock(this);
        destroyGraphics();
    }
}

void RenderHALThread::UpdateDeviceStatus()
{ 
    if (!pDevice->GraphicsConfigOnMainThread() || IsSingleThreaded())
    {
        // TestCooperativeLevel must be called on the same thread
        // in D3D9 to detect Restores.
        PushCallAndWait(&RenderHALThread::updateDeviceStatus);
    }
    else
    {
        RTBlockScope rtblock(this);
        updateDeviceStatus();
    }
}


void RenderHALThread::UpdateConfiguration()
{ 
    if (GetCurrentThreadId() == GetThreadId())
    {
        updateConfiguration();
    }
    else
    {
        PushCallAndWait(&RenderHALThread::updateConfiguration);
    }
}


void RenderHALThread::DrawFrame()
{
    PushCall(&RenderHALThread::drawFrame);
    DrawFrameEnqueued = true;
}

void RenderHALThread::CaptureFrameWithoutDraw()
{
    PushCall(&RenderHALThread::captureFrameWithoutDraw);
    DrawFrameEnqueued = true;
}

void RenderHALThread::WaitForOutstandingDrawFrame()
{
    if (DrawFrameEnqueued && !IsSingleThreaded())
    {
        DrawFrameDone.Wait();
        DrawFrameEnqueued = false;
    }
}

bool RenderHALThread::adjustViewConfig(Platform::ViewConfig* config)
{
    return pDevice->AdjustViewConfig(config);
}

bool RenderHALThread::initGraphics(const Platform::ViewConfig& config, Device::Window* window,
                                   ThreadId renderThreadId)
{
    if (pDevice->InitGraphics(config, window, renderThreadId))
    {
        ViewSize = config.ViewSize;
        pTextureManager = GetHAL()->GetTextureManager();
        updateDeviceStatus();
        VConfig = config;
        pWindow = window;
        return true;
    }
    return false;
}

void RenderHALThread::resizeFrame(void* layer)
{
    pDevice->ResizeFrame(layer);
}

bool RenderHALThread::reconfigureGraphics(const Platform::ViewConfig& config)
{
    if (pDevice->ReconfigureGraphics(config))
    {
        ViewSize = config.ViewSize;
        updateDeviceStatus();
        VConfig = config;
        return true;
    }
    updateDeviceStatus();
    return false;
}

void RenderHALThread::destroyGraphics()
{       
    pTextureManager.Clear();
    pDevice->ShutdownGraphics();
    updateDeviceStatus();
    VConfig = ViewConfig();
}

void RenderHALThread::blockForGraphicsInit()
{
    RTBlocked.PulseEvent();
    RTResume.Wait();
}

void RenderHALThread::exitThread()
{   // Just in case.
    destroyGraphics();
    StopQueueProcessing();
}

void RenderHALThread::updateCursor(const Point<int> mousePos, SystemCursorState state)
{
    CursorState = state;
    for ( int i = 0; i < Cursor_Type_Count; i++ )
    {
        if ( !CursorMats[i].IsNull() )
            CursorMats[i].SetMatrix2D(Render::Matrix2F::Translation((float)mousePos.x, (float)mousePos.y));
    }
}

void RenderHALThread::setStereoParams(Render::StereoParams sparams)
{
    GetHAL()->SetStereoParams(sparams);
}

void RenderHALThread::setProfileMode(Render::ProfilerModes mode)
{
    GetHAL()->GetProfiler().SetProfileMode(mode);
}
void RenderHALThread::setProfileFlag(unsigned flag, bool state)
{
    GetHAL()->GetProfiler().SetProfileFlag(flag, state);
}

unsigned RenderHALThread::getProfileFlag(unsigned flag)
{
    return GetHAL()->GetProfiler().GetProfileFlag(flag);
}

void RenderHALThread::getMeshCacheParams(Render::MeshCacheParams* params)
{
    *params = GetHAL()->GetMeshCache().GetParams();
}

void RenderHALThread::setMeshCacheParams(const Render::MeshCacheParams& params )
{
    GetHAL()->GetMeshCache().SetParams(params);
}

bool RenderHALThread::takeScreenShot(const String& filename)
{
    return pDevice->TakeScreenShot(filename);
}

// ***** Render::ThreadCommandQueue implementation.

void RenderHALThread::PushThreadCommand(Render::ThreadCommand* command)
{
    Ptr<Render::ThreadCommand> cmd = command;
    // Execute commands directly if RT is blocked for init/shutdown service.
    if (RTBlockedFlag)
        cmd->Execute();
    else
        PushCall(&RenderHALThread::executeThreadCommand, cmd);
}
void RenderHALThread::executeThreadCommand(const Ptr<Render::ThreadCommand>& command)
{
    command->Execute();
}

int RenderHALThread::watchDogThreadFn(Thread*, void* triggerAddr)
{
    unsigned* trigger = reinterpret_cast<unsigned*>(triggerAddr);
    int failureCount = 0;
    while (AtomicOps<unsigned>::Load_Acquire(trigger) != WatchDog_ExitThread)
    {
        if (!AtomicOps<unsigned>::CompareAndSet_Sync(trigger, WatchDog_Signaled, WatchDog_Unsignaled))
        {
            failureCount++;

            // Print to stderr, so we will know that if this dies in an autotest, it will come out in the log.
            SF_DEBUG_WARNING1(1, "Watchdog thread unsatisfied (for %d seconds)", (failureCount * WatchDogInterval) / 1000);
            if (failureCount >= WatchDogMaxFailureCount)
            {
                // Print to stderr, so we will know that if this dies in an autotest, it will come out in the log.
                fprintf(stderr, "Watchdog thread unsatisfied for too long (for %d seconds)\n", (failureCount * WatchDogInterval) / 1000);

                // Cause a crash, so that if this is an autotest, we will get a dump that we can debug.
                int * crash = 0;
                *crash = 0xDEADDEAD;
                return -1;
            }
        }
        else
            failureCount = 0;

        // Wait for the internal, but do it in 'steps', so that the main thread doesn't potentially wait for 5 seconds for the thread to exit.
        const unsigned msecSleepInterval = 100; // number of msec to wait before checking exit flag.
        unsigned totalSleepTime = 0;
        while (totalSleepTime < RenderHALThread::WatchDogInterval)
        {
            if (AtomicOps<unsigned>::Load_Acquire(trigger) == WatchDog_ExitThread)
                break;
            MSleep(msecSleepInterval);
            totalSleepTime += msecSleepInterval;
        }
    }
    return 0;
}

void RenderHALThread::StartThread()
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

void RenderHALThread::GetRenderStats( Render::HAL::Stats* pstats )
{
    Lock::Locker lock(&RenderStatsLock);
    *pstats = RenderStats;
}

void RenderHALThread::GetMeshCacheStats( Render::MeshCache::Stats* pstats )
{
    Lock::Locker lock(&RenderStatsLock);
    *pstats = MeshCacheStats;
}

void RenderHALThread::SetBackgroundColor( Render::Color bgColor )
{
    PushCall(&RenderHALThread::setBackgroundColor, bgColor);
}

void RenderHALThread::UpdateCursor( const Point<int>& mousePos, SystemCursorState state )
{
    PushCall(&RenderHALThread::updateCursor, mousePos, state);
}

void RenderHALThread::ExitThread()
{
    PushCallAndWait(&RenderHALThread::exitThread);
}

void RenderHALThread::SetStereoParams( const Render::StereoParams& sparams )
{
    PushCall(&RenderHALThread::setStereoParams, sparams);
}

void RenderHALThread::ToggleWireframe()
{
    Wireframe = !Wireframe;
}

void RenderHALThread::SetWireframe( bool wireframe )
{
    Wireframe = wireframe;
}

void RenderHALThread::SetProfileMode( Render::ProfilerModes mode )
{
    PushCall(&RenderHALThread::setProfileMode, mode);
}

void RenderHALThread::SetProfileFlag( unsigned flag, bool state )
{
    PushCall(&RenderHALThread::setProfileFlag, flag, state);
}

unsigned RenderHALThread::GetProfileFlag( unsigned flag )
{
    unsigned result;
    PushCallAndWaitResult(&RenderHALThread::getProfileFlag, &result, flag);
    return result;
}

unsigned RenderHALThread::GetFrames()
{
    return Frames.Exchange_NoSync(0);
}

Render::MeshCacheParams RenderHALThread::GetMeshCacheParams()
{
    Render::MeshCacheParams params;
    PushCallAndWait(&RenderHALThread::getMeshCacheParams, &params);
    return params;
}

void RenderHALThread::SetMeshCacheParams( const Render::MeshCacheParams& params )
{
    PushCall(&RenderHALThread::setMeshCacheParams, params);
}

bool RenderHALThread::TakeScreenShot( const String& filename )
{
    bool returnValue;
    PushCallAndWaitResult(&RenderHALThread::takeScreenShot, &returnValue, filename );
    return returnValue;
}

void RenderHALThread::updateDeviceStatus()
{
    Status = pDevice->GetStatus();
}

void RenderHALThread::updateConfiguration()
{
    pWindow->GetViewConfig(&VConfig);
    if (!VConfig.HasFlag(View_Stereo))
        GetHAL()->SetStereoDisplay(Render::StereoCenter, 1);
}

bool RenderHALThread::Start( ThreadState initialState )
{
    if ( !Thread::Start(initialState))
        return false;
    SetThreadName("Scaleform Renderer");
    return true;
}

}} // Scaleform::Platform
