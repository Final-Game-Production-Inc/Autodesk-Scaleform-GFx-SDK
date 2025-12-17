/**************************************************************************

Filename    :   Platform_Wii.cpp
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

#include "Render/Wii/Wii_HAL.h"
#include "Render/Wii/Wii_MemoryManager.h"

#include <revolution.h>
#include <revolution/kpad.h>


#define GX_FIFO_SIZE (256 * 1024)


namespace Scaleform { namespace Platform {


using namespace Render;

//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public Device::Window, public AppImplBase
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
        int                MouseX, MouseY;

        Controller() { Enabled = 0; }
    };
    Controller              Controllers[4];
    
    bool              Created;
    bool              BlockOnSize;
};


//------------------------------------------------------------------------

AppImpl::AppImpl(AppBase* app) :         
    AppImplBase(app),
    Created(false)
{
}

AppImpl::~AppImpl() 
{
}

static void *mem2alloc(unsigned long size)
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
        pAppImpl->Controllers[chan].Enabled = (result == WPAD_ERR_NONE);
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

    for (int i = 0; i < 4; i++)
    {
#if (__SDKVERNUMERIC__ <= 2006113006L)
        WPADSetConnectCallback(i, WPADConnectFunc);
#else
        KPADSetConnectCallback(i, WPADConnectFunc);
#endif

        UInt32 type;
        if (WPAD_ERR_NONE == WPADProbe(i, &type))
            Controllers[i].Enabled = 1;

        Controllers[i].MouseX      = 100;
        Controllers[i].MouseY      = 100;
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

bool AppImpl::processMessages()
{
    for (int i = 0; i < 4; i++)
    {
        if (!Controllers[i].Enabled)
            continue;

        KPADStatus kpad[KPAD_MAX_READ_BUFS];

        unsigned n = KPADRead(i, kpad, KPAD_MAX_READ_BUFS);

        if (n > 0)
        {
            int mx = Config.ViewSize.Width*0.5f + kpad[0].pos.x * Config.ViewSize.Width*0.50f;
            int my = Config.ViewSize.Height*0.5f + kpad[0].pos.y * Config.ViewSize.Height*0.50f;

            if (Controllers[i].MouseX != mx ||
                Controllers[i].MouseY != my)
            {
                Controllers[i].MouseX = mx;
                Controllers[i].MouseY = my;

                pApp->OnMouseMove(i, Point<int>(mx, my), 0);
            }

            for (int j = 0; j < 13; j++)
                if (kpad[0].trig & keymap[j][0])
                    pApp->OnPad(i, (PadKeyCode)keymap[j][1], 1);
                else if (kpad[0].release & keymap[j][0])
                    pApp->OnPad(i, (PadKeyCode)keymap[j][1], 0);

            if (kpad[0].trig & KPAD_BUTTON_A)
                pApp->OnMouseButton(i, 0, 1, Point<int>(Controllers[i].MouseX, Controllers[i].MouseY), 0);
            else if (kpad[0].release & KPAD_BUTTON_A)
                pApp->OnMouseButton(i, 0, 0, Point<int>(Controllers[i].MouseX, Controllers[i].MouseY), 0);
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

Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.0f);
}

const char* AppBase::GetDefaultFilePath()
{
    return "FxPlayer/";
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

    virtual void   ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Path = path;
    Pattern = pattern;

    String readPath = Path;
    while (readPath.GetLength() > 0 && readPath[readPath.GetLength()-1] == '/')
        readPath.Remove(readPath.GetLength()-1);

    DVDDir fd;
    if (DVDOpenDir(readPath, &fd))
    {
        DVDDirEntry dir;
        UInt64      size = sizeof(dir);
        while (DVDReadDir(&fd, &dir))
        {
            String name = dir.name;
            if (name.GetExtension() == pattern.GetExtension())
                Filenames.PushBack(name);
        }
    }
    DVDCloseDir(&fd);
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

    Device::Window*         pView;
    Ptr<Wii::HAL>           pHal;
    DeviceStatus            Status;
    static Wii::MemoryManager*     pMemManager;

    GXRenderModeObj*        pRmode;
    GXRenderModeObj         Rmode;
    UInt32                  TVMode;
    GXFifoObj*              pGXFifo;
    void*                   pGXFifoBuf;
    UPInt                   FBSize;
    void*                   pFB[2];
    int                     CurFb;
    RenderTarget*           pDisplayRT;

    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId);
    void shutdownGraphics();
    void beginFrame();
    void presentFrame();
};

Wii::MemoryManager* DeviceImpl::pMemManager;

DeviceImpl::DeviceImpl(ThreadCommandQueue *commandQueue)
    : pView(0), pHal(0), Status(Device_NeedInit)
{
    pHal = *SF_NEW Wii::HAL(commandQueue);
    pRmode = 0;
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
    pView = 0;
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    if (!pRmode)
    {
        bool isdtv = VIGetDTVStatus();
        UInt32 tvf = VIGetTvFormat();
        TVMode = VIGetScanMode();

        switch (tvf)
        {
        case VI_NTSC:
            pRmode = isdtv ? &GXNtsc480Prog : &GXNtsc480IntDf;
            break;
        case VI_PAL:
            pRmode = &GXPal528IntDf;
            break;
        case VI_EURGB60:
            pRmode = isdtv ? &GXEurgb60Hz480Prog : &GXEurgb60Hz480IntDf;
            break;
        case VI_MPAL:
            pRmode = isdtv ? &GXMpal480Prog : &GXMpal480IntDf;
            break;
        default:
            OSHalt("unknown TV format\n");
            break;
        }

        GXAdjustForOverscan(pRmode, &Rmode, 0, 16);
        pRmode = &Rmode;
    }

    if (config)
    {
        config->ViewSize.Width     = pRmode->fbWidth;
        config->ViewSize.Height    = pRmode->efbHeight;
        config->SetFlag(View_FullScreen, true);
        config->SetFlag(View_Stereo, false);
    }
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    if (!pHal || !view)
        return false;

    pView = view;

    pGXFifoBuf = pMemManager->Alloc(GX_FIFO_SIZE, 32, Memory_Wii_MEM1);
    pGXFifo = GXInit(pGXFifoBuf, GX_FIFO_SIZE);

    if (!pRmode)
        adjustViewConfig(0);

    FBSize = VIPadFrameBufferWidth(pRmode->fbWidth) * pRmode->xfbHeight * VI_DISPLAY_PIX_SZ;
    pFB[0] = pMemManager->Alloc(FBSize, 32, Memory_Wii_MEM1);
    pFB[1] = pMemManager->Alloc(FBSize, 32, Memory_Wii_MEM1);
    CurFb = 0;

    VIConfigure(pRmode);
    VISetBlack(1);
    VISetNextFrameBuffer(pFB[0]);
    VIFlush();
    VIWaitForRetrace();
    if (pRmode->viTVmode & 3 != TVMode || TVMode == VI_INTERLACE)
    {
        VIWaitForRetrace();
    }

    GXSetViewport(0, 0, pRmode->fbWidth, pRmode->efbHeight, 0, 1.0f);
    GXSetScissor(0, 0, pRmode->fbWidth, pRmode->efbHeight);

    float yScale = GXGetYScaleFactor(pRmode->efbHeight, pRmode->xfbHeight);
    int xfbHeight = GXSetDispCopyYScale(yScale);
    GXSetDispCopySrc(0, 0, pRmode->fbWidth, pRmode->efbHeight);
    GXSetDispCopyDst(pRmode->fbWidth, xfbHeight);
    GXSetCopyFilter(pRmode->aa, pRmode->sample_pattern, GX_TRUE, pRmode->vfilter);
    GXSetDispCopyGamma(GX_GM_1_0);

    GXColor black = {0,0,0,255};
    GXSetCopyClear(black, 0);

    if (pRmode->aa)
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

    GXCopyDisp(pFB[1], GX_TRUE);

    if (!pHal->InitHAL(Render::Wii::HALInitParams(pRmode, new Wii::RenderSync_DrawDone(), pMemManager, 0, renderThreadId)))
    {
        shutdownGraphics();
        return false;
    }

    // Create the default rendertarget.
    Wii::RenderTargetParams rtParams;
    rtParams.FbMode = GX_PF_RGB8_Z24;
    rtParams.FbZMode = GX_ZC_LINEAR;
    rtParams.pRMode = pRmode;
    pDisplayRT = pHal->CreateRenderTarget(rtParams);
    pHal->SetRenderTarget(pDisplayRT, false);

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
    if (pRmode->field_rendering)
        GXSetViewportJitter(
        0.0F, 0.0F, (float)pRmode->fbWidth, (float)pRmode->efbHeight, 
        0.0F, 1.0F, VIGetNextField());
    else
        GXSetViewport(
        0.0F, 0.0F, (float)pRmode->fbWidth, (float)pRmode->efbHeight, 
        0.0F, 1.0F);

    GXInvalidateVtxCache();
    GXInvalidateTexAll();
}

void DeviceImpl::presentFrame()
{
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    GXCopyDisp(pFB[CurFb], GX_TRUE);
    GXDrawDone();

    VISetNextFrameBuffer(pFB[CurFb]);
    VISetBlack(0);
    VIFlush();
    VIWaitForRetrace();
    CurFb = !CurFb;
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
    return Size<unsigned>(640,448);
}

void Device::Clear(UInt32 c)
{
    Render::Color color (c);
    GXColor gc = {color.GetRed(), color.GetGreen(), color.GetBlue(), 255};
    GXSetCopyClear(gc, 0);
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

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

    pImpl->presentFrame();
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
    // Scaleform::System initSFSystem;
    UPInt size = 14 << 20;
    UByte *m1arenaLo = (UByte*) OSGetMEM1ArenaLo();
    UByte *m1arenaHi = (UByte*) OSGetMEM1ArenaHi();

    DeviceImpl::pMemManager = new Wii::MemoryManager(Memory::GetGlobalHeap(), m1arenaLo, m1arenaLo + size);

    m1arenaLo += size;

    m1arenaLo = (UByte*)OSInitAlloc(m1arenaLo, m1arenaHi, 1);
    OSSetCurrentHeap(OSCreateHeap((void*)OSRoundUp32B(m1arenaLo), (void*)OSRoundDown32B(m1arenaHi)));
    OSSetMEM1ArenaLo(m1arenaHi);

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
    DVDInit();
    VIInit();

    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int  result = app->AppMain(argc, argv);
    Scaleform::Platform::AppBase::DestroyInstance(app);
    return result;
}
