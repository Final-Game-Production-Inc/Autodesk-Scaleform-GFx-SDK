/**************************************************************************

Filename    :   Platform_WiiU.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform.h"
#include "Platform/Platform_SystemCursorManager.h"

#include "Kernel/SF_KeyCodes.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_Debug.h"

#include "Render/WiiU/WiiU_HAL.h"

#include <cafe.h>
#include <cafe/pads/kpad/kpad.h>
#include <cafe/vpad.h>
#include <cafe/fs.h>


namespace Scaleform {
    
extern FSClient gFSClient; // from FileWiiU

namespace Platform {


using namespace Render;


class WiiUWindow : public Device::Window
{
public:
    int                     Display;
    ViewConfig              VConfig;
    GX2ColorBuffer          ColorBuffer;
    void*                   pColorBuffer;
    UPInt                   DispBufferSize;
    void*                   pDispBuffers;
    RenderTarget*           pWindowRT;

    // Window Interface
    WiiUWindow(int disp = 0) { Display = disp; }

    virtual void* GetHandle() { return 0; }
    virtual void  ConfigureWindow(ViewConfig& newConfig) {}
    virtual void  GetViewConfig(ViewConfig *config) const { *config = VConfig; }

    void Init(Render::MemoryManager* pMemManager, const ViewConfig& vconfig);
};

//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public WiiUWindow, public AppImplBase
{
    friend class AppBase;

    AppImpl(AppBase* app);
    ~AppImpl();

public:
    // Window Interface
    virtual void* GetHandle() { return 0; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);    

    bool          setupWindow(const String& title, const ViewConfig& config);

    bool          processMessages();
    void          setMousePosition(const Point<unsigned> & p );

    struct Controller
    {
        bool                Enabled;
        int                 Index;
        int                 Touch, TouchPosValid;
        int                 MouseX, MouseY;
        float               LastLx, LastLy, LastRx, LastRy;

        Controller() { Enabled = 0; Index = -1; Touch = 0; TouchPosValid = 0; MouseX = MouseY = -100; LastLx = LastRx = LastLy = LastRy = -1e6; }
    };
    Controller              Controllers[4];
    Controller              DisplayControllers[2];
    int                     ControllerCount;

    bool              Created;
    bool              BlockOnSize;
};


//------------------------------------------------------------------------

AppImpl::AppImpl(AppBase* app) :         
    AppImplBase(app),
    ControllerCount(0),
    Created(false)
{
}

AppImpl::~AppImpl() 
{
}

static void *mem2alloc(u32 size)
{
    return SF_ALLOC(size, Stat_Default_Mem);
}
static u8 mem2free(void *ptr)
{
    SF_FREE(ptr);
    return 1;
}

static AppImpl* pAppImpl;

static void WPADConnectFunc(SInt32 chan, SInt32 result)
{
    if (chan >= 0 && chan < 4)
    {
        pAppImpl->Controllers[chan].Enabled = (result == WPAD_ERR_NONE);
        if (pAppImpl->Controllers[chan].Index < 0)
            pAppImpl->Controllers[chan].Index = chan;

        KPADSetButtonProcMode(chan, KPAD_BUTTON_PROC_MODE_TIGHT);
    }
}

static void DRCConnectFunc(SInt32 chan, SInt32 result)
{
    if (chan >= 0 && chan < 2)
    {
        pAppImpl->DisplayControllers[chan].Enabled = (result == GX2_DRC_ATTACH);
        if (pAppImpl->DisplayControllers[chan].Index < 0)
            pAppImpl->DisplayControllers[chan].Index = chan | 0x10000;

        VPADSetButtonProcMode(chan, VPAD_BUTTON_PROC_MODE_TIGHT);
    }
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;

    if (Created)
        return false;

    // controller setup
    pAppImpl = this;
    WPADRegisterAllocator(mem2alloc, mem2free);
    KPADInit();
    VPADInit();

    for (int i = 0; i < 4; i++)
    {
        KPADSetConnectCallback(i, WPADConnectFunc);

        UInt32 type;
        if (WPAD_ERR_NONE == WPADProbe(i, &type))
            Controllers[i].Enabled = 1;

        Controllers[i].MouseX      = 100;
        Controllers[i].MouseY      = 100;
    }

    for (int i = 0; i < 2; i++)
    {
        GX2SetDRCConnectCallback(i, DRCConnectFunc);
    }

    GX2DRCMode drc = GX2GetSystemDRCMode();
    switch (drc)
    {
    case GX2_DRC_DOUBLE:
        //DisplayControllers[1].Enabled = true;
    case GX2_DRC_SINGLE:
        DisplayControllers[0].Enabled = true;
        DisplayControllers[0].Index = 1 << Platform::InputWindow_Shift;
        VPADSetButtonProcMode(0, VPAD_BUTTON_PROC_MODE_TIGHT);
        break;
    }

    return true;
}

void AppImpl::ConfigureWindow(ViewConfig& newConfig)
{
    // Configures window for a new video mode.
    Config = newConfig;
}

static int keymap[13][2] = {
    {KPAD_BUTTON_LEFT,  Pad_Left},
    {KPAD_BUTTON_RIGHT, Pad_Right},
    {KPAD_BUTTON_UP,    Pad_Up},
    {KPAD_BUTTON_DOWN,  Pad_Down},
    {KPAD_BUTTON_PLUS,  Pad_Plus},
    {KPAD_BUTTON_MINUS, Pad_Minus},
    {KPAD_BUTTON_A,     Pad_A},
    {KPAD_BUTTON_B,     Pad_B},
    {KPAD_BUTTON_1,     Pad_1},
    {KPAD_BUTTON_2,     Pad_2},
    {KPAD_BUTTON_HOME,  Pad_Home},
    {KPAD_BUTTON_C,     Pad_C},
    {KPAD_BUTTON_Z,     Pad_Z}};

static int vpad_keymap[15][2] = {
    {VPAD_BUTTON_LEFT,  Pad_Left},
    {VPAD_BUTTON_RIGHT, Pad_Right},
    {VPAD_BUTTON_UP,    Pad_Up},
    {VPAD_BUTTON_DOWN,  Pad_Down},
    {VPAD_BUTTON_PLUS,  Pad_Plus},
    {VPAD_BUTTON_MINUS, Pad_Minus},
    {VPAD_BUTTON_A,     Pad_A},
    {VPAD_BUTTON_B,     Pad_B},
    {VPAD_BUTTON_X,     Pad_X},
    {VPAD_BUTTON_Y,     Pad_Y},
    {VPAD_BUTTON_HOME,  Pad_Home},
    {VPAD_TRIGGER_ZL,   Pad_L1},
    {VPAD_TRIGGER_ZR,   Pad_R1},
    {VPAD_TRIGGER_L,    Pad_L2},
    {VPAD_TRIGGER_R,    Pad_R2}};

bool AppImpl::processMessages()
{
    for (int drc = 0; drc < 2; drc++)
    {
        if (!DisplayControllers[drc].Enabled)
            continue;

        VPADStatus vpad[VPAD_MAX_READ_BUFS];
        int        err;

        unsigned n = VPADRead(drc, vpad, VPAD_MAX_READ_BUFS, &err);

        for (int i = n-1; i >= 0; i--)
        {
            for (int j = 0; j < 17; j++)
                if (vpad[i].trig & vpad_keymap[j][0])
                    pApp->OnPad(DisplayControllers[drc].Index, (PadKeyCode)vpad_keymap[j][1], 1);
                else if (vpad[i].release & vpad_keymap[j][0])
                    pApp->OnPad(DisplayControllers[drc].Index, (PadKeyCode)vpad_keymap[j][1], 0);

            bool change = false;

            if (vpad[i].tpData.validity == VPAD_TP_VALIDITY_VALID && (vpad[i].tpData.x || vpad[i].tpData.y))
            {
                VPADTPData cpt;
                VPADGetTPCalibratedPoint(drc, &cpt, &vpad[i].tpData);
                int mx = int( cpt.x * 854.0f/1280.0f + 0.5f );
                int my = int( cpt.y * 480.0f/720.0f + 0.5f );

                if (!DisplayControllers[drc].TouchPosValid || mx != DisplayControllers[drc].MouseX || my != DisplayControllers[drc].MouseY)
                {
                    DisplayControllers[drc].MouseX = mx;
                    DisplayControllers[drc].MouseY = my;
                    change = true;
                }
                DisplayControllers[drc].TouchPosValid = true;
            }
            else
                DisplayControllers[drc].TouchPosValid = false;

            if (DisplayControllers[drc].Touch != vpad[i].tpData.touch)
            {
                if (DisplayControllers[drc].TouchPosValid)
                {
                    pApp->OnMouseButton(DisplayControllers[drc].Index, 0, vpad[i].tpData.touch,
                                        Point<int>(DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY), 0);
                    DisplayControllers[drc].Touch = vpad[i].tpData.touch;
                }
                else if (vpad[i].tpData.touch)
                    DisplayControllers[drc].Touch = 2;
                else
                {
                    pApp->OnMouseButton(DisplayControllers[drc].Index, 0, vpad[i].tpData.touch,
                                        Point<int>(DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY), 0);
                    DisplayControllers[drc].Touch = vpad[i].tpData.touch;
                }
            }
            else if (change)
                pApp->OnMouseMove(DisplayControllers[drc].Index,
                                  Point<int>(DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY), 0);

            float padlx = vpad[i].lStick.x;
            float padly = vpad[i].lStick.y;
            float padrx = vpad[i].rStick.x;
            float padry = vpad[i].rStick.y;

            if (padlx != DisplayControllers[drc].LastLx || padly != DisplayControllers[drc].LastLy)
            {
                pApp->OnPadStick(DisplayControllers[drc].Index, Pad_LT, padlx, padly);
                DisplayControllers[drc].LastLx = padlx; DisplayControllers[drc].LastLy = padly;
            }
            if (padrx != DisplayControllers[drc].LastRx || padry != DisplayControllers[drc].LastRy)
            {
                pApp->OnPadStick(DisplayControllers[drc].Index, Pad_RT, padrx, padry);
                DisplayControllers[drc].LastRx = padrx; DisplayControllers[drc].LastRy = padry;
            }
        }
    }

    for (int wim = 0; wim < 4; wim++)
    {
        if (!Controllers[wim].Enabled)
            continue;

        KPADStatus kpad[KPAD_MAX_READ_BUFS];

        unsigned n = KPADRead(wim, kpad, KPAD_MAX_READ_BUFS);

        for (int i = n-1; i >= 0; i--)
        {
            int mx = Config.ViewSize.Width*0.5f + kpad[i].pos.x * Config.ViewSize.Width*0.50f;
            int my = Config.ViewSize.Height*0.5f + kpad[i].pos.y * Config.ViewSize.Height*0.50f;

            if (Controllers[wim].MouseX != mx ||
                Controllers[wim].MouseY != my)
            {
                Controllers[wim].MouseX = mx;
                Controllers[wim].MouseY = my;

                pApp->OnMouseMove(Controllers[wim].Index, Point<int>(mx, my), 0);
            }

            for (int j = 0; j < 13; j++)
                if (kpad[i].trig & keymap[j][0])
                    pApp->OnPad(Controllers[wim].Index, (PadKeyCode)keymap[j][1], 1);
                else if (kpad[i].release & keymap[j][0])
                    pApp->OnPad(Controllers[wim].Index, (PadKeyCode)keymap[j][1], 0);

            if (kpad[i].trig & KPAD_BUTTON_A)
                pApp->OnMouseButton(Controllers[wim].Index, 0, 1, Point<int>(Controllers[wim].MouseX, Controllers[wim].MouseY), 0);
            else if (kpad[i].release & KPAD_BUTTON_A)
                pApp->OnMouseButton(Controllers[wim].Index, 0, 0, Point<int>(Controllers[wim].MouseX, Controllers[wim].MouseY), 0);
        }
    }
    return 1;
}

void AppImpl::setMousePosition(const Point<unsigned> & p ) 
{
}

//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase() :
    Cursor(false),
    ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}
AppBase::~AppBase()
{
    delete pImpl;
}

void AppBase::SetOrientationMode(bool)
{
}

void AppBase::Shutdown()
{
    ShuttingDown = true;
}

void AppBase::BringWindowToFront()
{
}

Device::Window* AppBase::GetDeviceWindow() const
{
    return pImpl;
}

bool AppBase::IsConsole()
{
    return true;
}
UInt32 AppBase::GetCaps() const
{
    return 0;
}

// Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
// This is a floating point value such as 0.1 for 10% safe frame border.
Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.05f);
}

const char* AppBase::GetDefaultFilePath()
{
    return "/vol/content/FxPlayer/";
}

String AppBase::GetContentDirectory()
{
    return GetDefaultFilePath();
}

int AppBase::GetMouseCount()
{
    return 4;
}

int AppBase::GetKeyboardCount()
{
    return 0;
}

bool AppBase::IsPadConnected(unsigned pad)
{
    return pad < 4 && pImpl->Controllers[pad].Enabled;
}


//------------------------------------------------------------------------
// ***** Directory

class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern) :
        DirectoryImplBase(path, pattern) { }

    virtual void ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Path = path;
    Pattern = pattern;
    
    String readPath = Path;
    while (readPath.GetLength() > 0 && readPath[readPath.GetLength()-1] == '/')
        readPath.Remove(readPath.GetLength()-1);
    
    const char *preadPath = readPath.ToCStr();
    FSDirHandle fd;
    FSCmdBlock  commandBlock;
    FSInitCmdBlock(&commandBlock);
    FSStatus result = FSOpenDir(&gFSClient, &commandBlock, preadPath, &fd, FS_RET_NO_ERROR);
    if (result == FS_STATUS_OK)
    {
        FSDirEntry dir;
        while (FSReadDir(&gFSClient, &commandBlock, fd, &dir, FS_RET_NO_ERROR) == FS_STATUS_OK)
        {
            String name = dir.name;
            if (name.GetExtension() == pattern.GetExtension())
                Filenames.PushBack(name);
        }
    }
    FSCloseDir(&gFSClient, &commandBlock, fd, FS_RET_NO_ERROR);
    CurFile = 0;
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}

//------------------------------------------------------------------------
// ***** SystemCursorManager

SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) :
SystemCursorState(systemCursorAvailable, false),
    pImpl(0)
{
}
SystemCursorManager::~SystemCursorManager()
{
}

void SystemCursorManager::update(bool forceSet)
{
    // System cursors update. Since there are none, do nothing.
}

// ----------------------------------------------------------------

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    WiiUWindow*             pView;      // TV
    WiiUWindow*             pDrcWindow;
    WiiUWindow*             pCurWindow;
    Ptr<WiiU::HAL>          pHal;
    DeviceStatus            Status;
    Render::MemoryManager*  pMemManager;

    GX2DepthBuffer          DepthBuffer;
    void*                   pDepthBuffer;
    GX2ContextState *       pContext;
    OSTime                  LastDrcFlip;

    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId);
    void shutdownGraphics();
    void beginFrame();
    void presentTVFrame();
    void presentDrcFrame();
};

DeviceImpl::DeviceImpl(ThreadCommandQueue *commandQueue)
    : pView(0), pHal(0), Status(Device_NeedInit)
{
    pHal = *SF_NEW WiiU::HAL(commandQueue);
    pMemManager = new WiiU::MemoryManager();
    pContext = 0;
    pView = 0; pDrcWindow = 0; pCurWindow = 0;
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
    pView = 0;
}

void* SystemAllocateGX2(UPInt size, UPInt align, MemoryType)
{
    void * pmem = MEMAllocFromDefaultHeapEx(size, align);
    GX2NotifyMemAlloc(pmem, size, align);
    return pmem;
}
void SystemFreeGX2(void* pmem)
{
    GX2NotifyMemFree(pmem);
    MEMFreeToDefaultHeap(pmem);
}

void WiiUWindow::Init(Render::MemoryManager* pMemManager, const ViewConfig& config)
{
    VConfig = config;

    memset(&ColorBuffer, 0, sizeof(ColorBuffer));
    GX2InitColorBuffer(&ColorBuffer, config.ViewSize.Width, config.ViewSize.Height,
                       GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_AA_MODE_1X);
    pColorBuffer = SystemAllocateGX2(ColorBuffer.surface.imageSize, ColorBuffer.surface.alignment, Memory_Normal);
    GX2InitColorBufferPtr(&ColorBuffer, pColorBuffer);
    GX2Invalidate(GX2_INVALIDATE_CPU, pColorBuffer, ColorBuffer.surface.imageSize);
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    if (config)
    {
        config->ViewSize.Width     = 1280;
        config->ViewSize.Height    = 720;
        config->SetFlag(View_FullScreen, true);
        config->SetFlag(View_Stereo, false);
    }
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    if (!pHal || !view)
        return false;

    pView = (WiiUWindow*) view;

    GX2Init(0);

    pView->Init(pMemManager, config);
    memset(&DepthBuffer, 0, sizeof(DepthBuffer));
    GX2InitDepthBuffer(&DepthBuffer, config.ViewSize.Width, config.ViewSize.Height,
                       GX2_SURFACE_FORMAT_D_D24_S8_UNORM, GX2_AA_MODE_1X);
    pDepthBuffer = SystemAllocateGX2(DepthBuffer.surface.imageSize, DepthBuffer.surface.alignment, Memory_Normal);
    GX2Invalidate(GX2_INVALIDATE_CPU, pDepthBuffer, DepthBuffer.surface.imageSize);
    GX2InitDepthBufferPtr(&DepthBuffer, pDepthBuffer);

    pContext = (GX2ContextState*) SystemAllocateGX2(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT, Memory_Normal);
    GX2SetupContextState(pContext);

    GX2SetColorBuffer(&pView->ColorBuffer, GX2_RENDER_TARGET_0);
    GX2SetDepthBuffer(&DepthBuffer);

    GX2TVRenderMode tvmode = GX2_TV_RENDER_720;
    GX2Boolean      scale;

    GX2CalcTVSize(tvmode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &pView->DispBufferSize, &scale);

    pView->pDispBuffers = SystemAllocateGX2(pView->DispBufferSize, GX2_SCAN_BUFFER_ALIGNMENT, Memory_Normal);
    GX2Invalidate(GX2_INVALIDATE_CPU, pView->pDispBuffers, pView->DispBufferSize);

    GX2SetTVBuffer(pView->pDispBuffers, pView->DispBufferSize, tvmode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);

    GX2SetViewport(0, 0, config.ViewSize.Width, config.ViewSize.Height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, config.ViewSize.Width, config.ViewSize.Height);
    GX2SetTVScale(config.ViewSize.Width, config.ViewSize.Height);

    GX2SetSwapInterval(1);
    GX2ClearColor(&pView->ColorBuffer, 0,0,0,0);
    GX2SetContextState(pContext);

    pCurWindow = pView;
    presentTVFrame();
    presentTVFrame();
    GX2SetTVEnable(GX2_ENABLE);
    LastDrcFlip = 0;

    if (!pHal->InitHAL(Render::WiiU::HALInitParams(pContext, pMemManager)))
    {
        shutdownGraphics();
        return false;
    }

    // Create the default rendertarget.
    pView->pWindowRT = pHal->CreateRenderTarget(pView->ColorBuffer, DepthBuffer);
    pHal->SetRenderTarget(pView->pWindowRT, false);

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
}

void DeviceImpl::presentTVFrame()
{
    SF_ASSERT(pCurWindow == pView);

    GX2SwapBuffers(&pView->ColorBuffer);
    GX2SetContextState(pContext);
    GX2Flush();

    //GX2WaitForFlip();
    GX2WaitForVsync();
}

void DeviceImpl::presentDrcFrame()
{
    GX2CopyColorBufferToScanBuffer(&pCurWindow->ColorBuffer, GX2_SCAN_TARGET_DRC_FIRST);
    GX2SetContextState(pContext);
}

//------------------------------------------------------------------------
// *** Device.
Device::Device(ThreadCommandQueue *commandQueue)
{
    pImpl = SF_NEW DeviceImpl(commandQueue);
}
Device::~Device()
{
    delete pImpl;
}

HAL* Device::GetHAL() const
{
    return pImpl->pHal;
}

DeviceStatus Device::GetStatus() const
{
    return pImpl->Status;
}

void Device::ResizeFrame(void*) { }
    
bool Device::InitGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, view, renderThreadId);
}

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

void Device::Clear(UInt32 c)
{
    Render::Color color (c);
    float rgba[4];
    color.GetRGBAFloat(rgba);
    GX2ClearColor(&pImpl->pCurWindow->ColorBuffer, rgba[0], rgba[1], rgba[2], rgba[3]);
    GX2SetContextState(pImpl->pContext);
}

int AppBase::GetDisplayCount()
{
    return 1 + (pImpl->DisplayControllers[0].Enabled ? 1 : 0);
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    WiiUWindow *pnewwin = SF_NEW WiiUWindow(WindowId);

    ViewConfig newConfig;
    newConfig.ViewSize.Width = 854;
    newConfig.ViewSize.Height = 480;
    newConfig.ViewFlags = View_FullScreen | View_UseOrientation;
    newConfig.StereoFormat = Stereo_None;

    pnewwin->Init(pImpl->pMemManager, newConfig);

    GX2Boolean scale;
    GX2CalcDRCSize(GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &pnewwin->DispBufferSize, &scale);
    pnewwin->pDispBuffers = SystemAllocateGX2(pnewwin->DispBufferSize, GX2_SCAN_BUFFER_ALIGNMENT, Memory_Normal);
    GX2Invalidate(GX2_INVALIDATE_CPU, pnewwin->pDispBuffers, pnewwin->DispBufferSize);

    GX2SetDRCBuffer(pnewwin->pDispBuffers, pnewwin->DispBufferSize, GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);
    GX2SetDRCScale(newConfig.ViewSize.Width, newConfig.ViewSize.Height);
    GX2Invalidate(GX2_INVALIDATE_CPU, pnewwin->pDispBuffers, pnewwin->DispBufferSize);

    GX2CopyColorBufferToScanBuffer(&pnewwin->ColorBuffer, GX2_SCAN_TARGET_DRC_FIRST);
    GX2SetContextState(pImpl->pContext);
    GX2SetDRCEnable(GX2_ENABLE);

    pnewwin->pWindowRT = pImpl->pHal->CreateRenderTarget(pnewwin->ColorBuffer, pImpl->DepthBuffer);

    return pnewwin;
}

void Device::SetWindow(Window* pwin)
{
    pImpl->pCurWindow = (WiiUWindow*) pwin;
    pImpl->pHal->SetRenderTarget(pImpl->pCurWindow->pWindowRT, true);
}

void Device::BeginFrame()
{
    pImpl->beginFrame();
}

void Device::PresentFrame(unsigned displays)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

    if (pImpl->pCurWindow->Display)
        pImpl->presentDrcFrame();
    else
        pImpl->presentTVFrame();
}

void Device::SetWireframe(bool flag)
{
}

bool Device::TakeScreenShot(const String&)
{
    return false;
}

UInt32 Device::GetCaps() const
{
    return 0;
}

// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{    
    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return 0;

    // Initialize application and its window.
    ViewConfig config(Device::GetDefaultViewSize());
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return 0;
    }

    // Application / Player message loop.
    pImpl->NextUpdateTicks = Timer::GetTicks() + pImpl->FrameTimeTicks;

    while(!IsShuttingDown())
    {
        // Wait for Update time if necessary; consider timing wrap-around.
        pImpl->processMessages();

        if (pImpl->FrameTime != 0.0f)
        {
            UInt64  ticks = Timer::GetTicks();
            if (pImpl->NextUpdateTicks > ticks)
            {
                UPInt waitUs = pImpl->NextUpdateTicks - ticks;
                OSSleepMicroseconds(waitUs);
                continue;
            }
            pImpl->NextUpdateTicks = ticks + pImpl->FrameTimeTicks;
        }
        OnUpdateFrame(true);
    }

    OnShutdown();

    return 0;
}

}} // Scaleform::Platform


//------------------------------------------------------------------------
int main(int argc, char** argv)
{
    FSInit();

    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int  result = app->AppMain(argc, argv);
    Scaleform::Platform::AppBase::DestroyInstance(app);
    return result;
}
