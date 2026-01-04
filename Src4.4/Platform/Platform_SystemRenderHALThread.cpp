/**************************************************************************

Filename    :   Platform_SystemRenderHALThread.cpp
Content     :   Default RenderThread implementation used by applications.
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform_SystemRenderHALThread.h"

namespace Scaleform { namespace Platform {

//------------------------------------------------------------------------
// ***** RenderHALThread

RenderHALThread::RenderHALThread(RTCommandQueue::ThreadingType threadingType) : 
  RTCommandQueue(MultiThreaded),
  pDevice(0),
  pWindow(0),
  pTextureManager(0),
  Status(Device_NeedInit),
  Wireframe(false),
  GlyphRasterCount(0),
  ResetGlyphRasterCount(false),
  Frames(0),
  BGColor(0),
  DrawFrameEnqueued(false),
  RTBlocked(false), RTResume(false), RTBlockedFlag(false),
  CursorState(true)
{        
    memset(CursorPrims, 0, sizeof CursorPrims);
}

RenderHALThread::~RenderHALThread()
{
}

// RTBlockScope helper class used to block RenderThread for duration of its scope.
RenderHALThread::RTBlockScope::RTBlockScope(RenderHALThread* thread)
: pThread(thread)
{
    pThread->PushCall(&RenderHALThread::blockForGraphicsInit);
    pThread->RTBlocked.Wait();
    pThread->RTBlockedFlag = true;
}

RenderHALThread::RTBlockScope::~RTBlockScope()
{
    pThread->RTBlockedFlag = false;
    pThread->RTResume.PulseEvent();
}

int RenderHALThread::Run()
{
    return 0;
}

void RenderHALThread::runCommands()
{
    RTCommandBuffer cmd;

    while (PopCommand(&cmd, 0))
    {
        cmd.Execute(*this);
        if (cmd.NeedsWait())
            cmd.GetNotifier()->Notify();
    }       
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

void RenderHALThread::SetConfigAndWindow(const ViewConfig& config, Device::Window* window)
{
    VConfig = config;
    pWindow = window;
}

bool    RenderHALThread::InitGraphics(const ViewConfig& config, Device::Window* window,
                                            ThreadId renderThreadId)
{
    SF_DEBUG_ASSERT(window == pWindow, "pWindow does not match in InitGraphics and SetConfigAndWindow.");

    // InitGraphics really takes place on the render thread, and is called by the system.
    blockForGraphicsInit();
    return 1;
}

bool RenderHALThread::InitGraphics_RenderThread()
{
    runCommands();
    bool ok = false;
    {
        ok = initGraphics(VConfig, pWindow, GetCurrentThreadId());
    }
    Status = Device_Ready;
    return ok;
}

void RenderHALThread::DestroyGraphics()
{
    SF_ASSERT(pDevice);
    Status = Device_NeedReconfigure;
}

void RenderHALThread::DrawFrame_RenderThread()
{
    runCommands();
    drawFrame();
}

void RenderHALThread::CaptureFrameWithoutDraw()
{
    runCommands();
    captureFrameWithoutDraw();
}

bool RenderHALThread::OnSize_RenderThread(unsigned w, unsigned h)
{
    runCommands();
    AbortFrame();
    return 1;
}

void RenderHALThread::WaitForOutstandingDrawFrame()
{
    DrawFrameDone.Wait();
}

void RenderHALThread::AbortFrame()
{
}

bool RenderHALThread::adjustViewConfig(Platform::ViewConfig* config)
{
    return pDevice->AdjustViewConfig(config);
}

bool RenderHALThread::initGraphics(const ViewConfig& config, Device::Window* window,
                                         ThreadId renderThreadId)
{
    if (pDevice->InitGraphics(config, window, renderThreadId))
    {
        ViewSize = config.ViewSize;
        pTextureManager = getHAL()->GetTextureManager();
        //updateDeviceStatus();
        VConfig = config;
        RTResume.PulseEvent();
        return true;
    }
    RTResume.PulseEvent();
    return false;
}

bool RenderHALThread::reconfigureGraphics(const Platform::ViewConfig& config)
{
    if (pDevice->ReconfigureGraphics(config))
    {
        ViewSize = config.ViewSize;
        //updateDeviceStatus();
        VConfig = config;
        return true;
    }
    //updateDeviceStatus();
    return false;
}

void RenderHALThread::destroyGraphics()
{       
    pTextureManager.Clear();
    pDevice->ShutdownGraphics();
    //updateDeviceStatus();
    VConfig = ViewConfig();
}

void RenderHALThread::blockForGraphicsInit()
{
    RTBlocked.PulseEvent();
    RTResume.Wait();
}

void RenderHALThread::exitThread()
{
    //destroyGraphics();
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
    getHAL()->SetStereoParams(sparams);
}

// ***** Render::ThreadCommandQueue implementation.
void RenderHALThread::PushThreadCommand(Render::ThreadCommand* command)
{
    Ptr<Render::ThreadCommand> cmd = command;
    PushCall(&RenderHALThread::executeThreadCommand, cmd);
}

void RenderHALThread::executeThreadCommand(const Ptr<Render::ThreadCommand>& command)
{
    command->Execute();
}

void RenderHALThread::getMeshCacheParams(Render::MeshCacheParams* params)
{
    *params = getHAL()->GetMeshCache().GetParams();
}

void RenderHALThread::setMeshCacheParams(const Render::MeshCacheParams& params )
{
    getHAL()->GetMeshCache().SetParams(params);
}

}} // Scaleform::Platform
