/**************************************************************************

Filename    :   Platform_PS3_GCM.cpp
Content     :   Platform Support for D3D9
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "Render/PS3/PS3_HAL.h"
#include "Platform.h"
#include <unistd.h>
#include <stdlib.h>

#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>

namespace Scaleform { namespace Platform {

using namespace Render::PS3;
using namespace Render;

//------------------------------------------------------------------------
#define  SF_PLATFORM_APP_COLOR_FORMAT           CELL_GCM_SURFACE_A8R8G8B8;
#define  SF_PLATFORM_APP_DEPTHSTENCIL_FORMAT    D3DFMT_D24S8

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    Device::Window*         pView;
    Ptr<Render::PS3::HAL>   pHal;
    DeviceStatus            Status;

    uint8_t determineFormat(const ViewConfig& config) const;

	bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId);
    void shutdownGraphics();
    void beginFrame();

    CellGcmContextData*     GcmCtx;
    PS3::MemoryManager*     pMemManager;
    CellGcmSurface          Surface;
    unsigned                Frame;
    void*                   Surfaces[3];    // 2 backbuffers, 1 depthstencil
    UPInt                   SurfaceSize;
    UPInt                   Buffers[2];

    RenderTarget*           pDisplayRT;
};

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
: pView(0), pHal(0), Status(Device_NeedInit), SurfaceSize(0)
{
    pHal = *SF_NEW Render::PS3::HAL(commandQueue);
    memset(Surfaces, 0, sizeof Surfaces);
    memset(Buffers, 0, sizeof Buffers);
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();

    if ( pMemManager )
    {
        for ( unsigned i = 0; i < 3; ++i )
            pMemManager->Free(Surfaces[i], SurfaceSize);
        SurfaceSize = 0;
        memset(Surfaces, 0, sizeof Surfaces);
        memset(Buffers, 0, sizeof Buffers);
        delete pMemManager;
        pMemManager = 0;
    }

	pView = 0;
}

uint8_t DeviceImpl::determineFormat(const ViewConfig& config) const
{    
    uint8_t format = SF_PLATFORM_APP_COLOR_FORMAT;

    switch(config.ColorBits)
    {
    case 16: return CELL_GCM_SURFACE_R5G6B5;
    case 24: return CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8;
    default: break;
    }
    return format;
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    bool validSize = false;
    CellVideoOutDeviceInfo devInfo;
    cellVideoOutGetDeviceInfo(CELL_VIDEO_OUT_PRIMARY, 0, &devInfo);
    for (unsigned mode = 0; mode < devInfo.availableModeCount; ++mode)
    {
        if (config->HasFlag(View_Stereo) && 
            devInfo.availableModes[mode].resolutionId == CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING)
        {
            config->ViewSize.Width = 1280;
            config->ViewSize.Height= 720;
            config->ViewFlags |= View_FullScreen;
            return true;
        }
    }

    for (unsigned mode = 0; mode < devInfo.availableModeCount; ++mode)
    {
        CellVideoOutResolution cvor;
        if (config->HasFlag(View_Stereo) && 
            devInfo.availableModes[mode].resolutionId == CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING)
        {
            validSize = true;
            break;
        }
        if (cellVideoOutGetResolution(devInfo.availableModes[mode].resolutionId, &cvor) == CELL_OK)
        {
            if ( cvor.width == config->ViewSize.Width && 
                 cvor.height == config->ViewSize.Height )
            {
                validSize = true;
                config->ViewFlags &= ~View_Stereo;
                break;
            }
        }
    }

    // Default to 720p.
    if ( !validSize )
    {
        config->ViewSize.Width = 1280;
        config->ViewSize.Height= 720;
    }

    config->ViewFlags |= View_FullScreen;
	return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    if (!pHal || !view)
        return false;

	pView = view;

    uint8_t format = determineFormat(config);
    int width = config.ViewSize.Width;
    int height = config.ViewSize.Height;
    int bufheight;
    if (config.HasFlag(View_Stereo))
    {
        bufheight = height * 2 + 30;

        CellVideoOutConfiguration cvc;
        memset(&cvc, 0, sizeof(cvc));
        cvc.resolutionId = CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING;
        cvc.format = format;
        cvc.pitch = (width * 4 + 63) & ~63;	

        cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &cvc, 0, 0);
    }
    else
        bufheight = height;

    UPInt cmdsize = 262144;
    UPInt mainsize = 32*1024*1024;
    void *mm = memalign(1 << 20, mainsize);

    if (CELL_OK != cellGcmInit(cmdsize, mainsize, mm))
        return 0;

    CellGcmConfig cfg;
    cellGcmGetConfiguration(&cfg);

    if (cfg.localAddress == 0 || cfg.localSize == 0)
        return 0;

    pMemManager = new PS3::MemoryManager(Memory::GetGlobalHeap(), cfg.localAddress, cfg.localSize,
        ((UByte*)mm)+cmdsize, mainsize-cmdsize);

    unsigned pitch = (width * 4 + 63) & ~63;

    UInt32 depthofs;

    SurfaceSize = pitch * bufheight;
    Surfaces[0] = pMemManager->Alloc(SurfaceSize, 0x100, Memory_PS3_RSX);
    Surfaces[1] = pMemManager->Alloc(SurfaceSize, 0x100, Memory_PS3_RSX);
    Surfaces[2] = pMemManager->Alloc(SurfaceSize, 0x100, Memory_PS3_RSX);

    cellGcmAddressToOffset(Surfaces[0], &Buffers[0]);
    cellGcmAddressToOffset(Surfaces[1], &Buffers[1]);
    cellGcmAddressToOffset(Surfaces[2], &depthofs);

    if (CELL_OK != cellGcmSetDisplayBuffer(0, Buffers[0], pitch, width, bufheight))
        return 0;
    if (CELL_OK != cellGcmSetDisplayBuffer(1, Buffers[1], pitch, width, bufheight))
        return 0;

    Surface.colorFormat         = format;
    Surface.colorTarget         = CELL_GCM_SURFACE_TARGET_0;
    Surface.colorLocation[0]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorOffset[0]      = Buffers[0];
    Surface.colorPitch[0]       = pitch;

    Surface.colorLocation[1]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorLocation[2]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorLocation[3]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorOffset[1]      = 0;
    Surface.colorOffset[2]      = 0;
    Surface.colorOffset[3]      = 0;
    Surface.colorPitch[1]       = 64;
    Surface.colorPitch[2]       = 64;
    Surface.colorPitch[3]       = 64;

    Surface.depthFormat         = CELL_GCM_SURFACE_Z24S8;
    Surface.depthLocation       = CELL_GCM_LOCATION_LOCAL;
    Surface.depthOffset         = depthofs;
    Surface.depthPitch          = pitch;

    Surface.type                = CELL_GCM_SURFACE_PITCH;
    Surface.antialias           = CELL_GCM_SURFACE_CENTER_1;

    Surface.width               = width;
    Surface.height              = bufheight;
    Surface.x                   = 0;
    Surface.y                   = 0;

    cellGcmSetFlipMode(config.HasFlag(View_VSync) ?
        CELL_GCM_DISPLAY_VSYNC :
        CELL_GCM_DISPLAY_HSYNC);

    GcmCtx = gCellGcmCurrentContext;
    beginFrame();

    cellGcmSetClearColor(GcmCtx, 0x808080FF);
    cellGcmSetClearDepthStencil(GcmCtx, 0);
    cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A|CELL_GCM_CLEAR_S|CELL_GCM_CLEAR_Z);

    cellGcmResetFlipStatus();
    cellGcmSetFlip(GcmCtx, 0);
    cellGcmFlush(GcmCtx);
    cellGcmSetWaitFlip(GcmCtx);
    cellGcmFinish(GcmCtx, 0);
    Frame = 1;

    // setup surface for left buffer, because the first SetStereoDisplay has setState=0.
    Surface.colorOffset[0] = Buffers[Frame];
    Surface.height = height;

    cellGcmSetSurface(GcmCtx, &Surface);

    if (!pHal->InitHAL(Render::PS3::HALInitParams(GcmCtx, pMemManager)))
    {
        shutdownGraphics();
        return false;
    }

    // Create the default rendertarget. This is required on PS3 (if you want to use render targets),
    // because there is no way to query the render surface in GCM. If this is not done, Push/PopRenderTarget
    // in HAL will all fail. One instance with each buffer address is needed to swap buffers each frame
    pDisplayRT = SF_NEW RenderTarget(0, RBuffer_Default, ImageSize(Surface.width, height) );
    Surface.colorOffset[0] = Buffers[Frame];

    PS3::RenderTargetData::UpdateData(pDisplayRT, Surface, 0, config.HasFlag(View_Stereo) ? height : 0);
    if ( !pHal->SetRenderTarget(pDisplayRT))
    {
        shutdownGraphics();
        return false;
    }

    if (config.HasFlag(View_Stereo))
    {
        cellSysmoduleLoadModule(CELL_SYSMODULE_AVCONF_EXT);

        Render::StereoParams StereoParams;
        if (CELL_OK != cellVideoOutGetScreenSize(CELL_VIDEO_OUT_PRIMARY, &StereoParams.DisplayDiagInches))
        {
            StereoParams.DisplayDiagInches = 46.;
        }

        StereoParams.Distortion = 0.7f;
        StereoParams.EyeSeparationCm = 6.4f;
        pHal->SetStereoParams(StereoParams);
    }

    // reset for BeginFrame
    Surface.height = bufheight;

    Status = Device_Ready;
    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (pHal)
        pHal->ShutdownHAL();

    Status = Device_NeedInit;
}

void DeviceImpl::beginFrame()
{
    cellGcmSetSurface(GcmCtx, &Surface);

    float scale[4], offset[4];
    scale[0]    = Surface.width * 0.5f;
    scale[1]    = Surface.height * 0.5f;
    offset[0]   = Surface.width * 0.5f;
    offset[1]   = Surface.height * 0.5f;
    scale[2]    = offset[2] = 0.5f;
    scale[3]    = offset[3] = 0;

    cellGcmSetViewport(GcmCtx, 0,0, Surface.width, Surface.height, 0, 1, scale, offset);
    cellGcmSetScissor(GcmCtx, 0,0, 4096,4096);
}

//------------------------------------------------------------------------
// *** Device.
Device::Device(Render::ThreadCommandQueue *commandQueue)
{
    pImpl = SF_NEW DeviceImpl(commandQueue);
}
Device::~Device()
{
    delete pImpl;
}

Render::HAL* Device::GetHAL() const
{
    return pImpl->pHal;
}

DeviceStatus Device::GetStatus() const
{
    return pImpl->Status;
}

bool Device::InitGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, view, renderThreadId);
}

void Device::ResizeFrame(void*) { }
    
bool Device::ReconfigureGraphics(const ViewConfig& config)
{
    return true;
}

void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

bool Device::GraphicsConfigOnMainThread() const
{
    return true;
}

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

Size<unsigned> Device::GetDefaultViewSize()
{
    return Size<unsigned>(1280,720);
}

void Device::Clear(UInt32 color)
{
    cellGcmSetClearColor(pImpl->GcmCtx, color);
    cellGcmSetClearDepthStencil(pImpl->GcmCtx, 0);
    cellGcmSetClearSurface(pImpl->GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A|CELL_GCM_CLEAR_S|CELL_GCM_CLEAR_Z);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::BeginFrame()
{
    pImpl->beginFrame();
}

void Device::PresentFrame(unsigned displays)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

    while (cellGcmGetFlipStatus() != 0)
        sys_timer_usleep(100);

    cellGcmResetFlipStatus();
    cellGcmSetFlip(pImpl->GcmCtx, pImpl->Frame);
    cellGcmFlush(pImpl->GcmCtx);
    cellGcmSetWaitFlip(pImpl->GcmCtx);

    pImpl->Frame = !pImpl->Frame;
    pImpl->Surface.colorOffset[0] = pImpl->Buffers[pImpl->Frame];

    ViewConfig config;
    pImpl->pView->GetViewConfig(&config);
    PS3::RenderTargetData::UpdateData(pImpl->pDisplayRT, pImpl->Surface, 0, 
        config.HasFlag(View_Stereo) ? config.ViewSize.Height : 0);
}

void Device::SetWireframe(bool flag)
{
    cellGcmSetBackPolygonMode(pImpl->GcmCtx, flag ? CELL_GCM_POLYGON_MODE_LINE : CELL_GCM_POLYGON_MODE_FILL);
    cellGcmSetFrontPolygonMode(pImpl->GcmCtx, flag ? CELL_GCM_POLYGON_MODE_LINE : CELL_GCM_POLYGON_MODE_FILL);
}

bool Device::TakeScreenShot(const String&)
{
    return false;
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}

}}
