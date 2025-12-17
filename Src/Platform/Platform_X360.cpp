/**************************************************************************

Filename    :   Platform_X360.cpp
Content     :   Platform Support for D3D9
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/X360/X360_HAL.h"
#include "Render/X360/X360_BufferMemory.h"
#include "Platform.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_KeyCodes.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Kernel/SF_MsgFormat.h"
#include "Render/Render_ShapeDataFloatMP.h"
#include "Render/Render_TreeNode.h"
#include "Render/Render_TreeShape.h"
#include "Platform/Platform_XInput.h"
#include <d3dx9.h>
#include <xbdm.h>

namespace Scaleform { namespace Platform {

//------------------------------------------------------------------------
#define  SF_PLATFORM_APP_COLOR_FORMAT           D3DFMT_A8R8G8B8;
#define  SF_PLATFORM_APP_DEPTHSTENCIL_FORMAT    D3DFMT_D24S8



class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>, public Render::HALNotify
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    D3DFORMAT determineFormat(const ViewConfig& config) const;
    void      calcPresentParameters(D3DPRESENT_PARAMETERS* params, const ViewConfig& config);    

    bool adjustFullScreenRes(D3DFORMAT format, Size<unsigned>* size);
    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();
    bool takeScreenShot(const String& filename);
    void clear(UInt32 color);

    // Render::HALNotify.
    virtual void OnHALEvent(Render::HALNotifyType type);

    Device::Window*         pWindow;
    Ptr<Render::X360::HAL>  pHal;
    DeviceStatus            Status;
    ArrayLH<String>         ScreenCaptureRequests;
    UInt32                  ClearColor;

    // D3D Variables
    HWND                                    hWnd;
    LPDIRECT3D9                             pD3D;           // Used to create the D3D9Device.
    LPDIRECT3DDEVICE9                       pDevice;        // Our rendering device.
    D3DPRESENT_PARAMETERS                   PresentParams;
    Render::X360::MemoryManager*            pMemManager;    
    Ptr<Render::X360::RenderBufferManager>  pRBManager;     // RenderBufferManager instance.
    UPInt                                   RBVideoMemory;  // Video memory for RenderBuffers.

    static const int                        RBVideoSize = 1280 * 720 * 4 * 4;   // Size of video memory block allocated for RenderBuffers.
};

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0),
   pDevice(0),
   ClearColor(0),
   pMemManager(0),
   pRBManager(0),
   RBVideoMemory(0)
{
    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D)
    {
        pHal = *SF_NEW Render::X360::HAL(commandQueue);
        pHal->AddNotify(this);
    }
}

DeviceImpl::~DeviceImpl()
{
    if ( pHal )
        pHal->RemoveNotify(this);

    pHal.Clear();
    if (pD3D)
        pD3D->Release();
}


D3DFORMAT DeviceImpl::determineFormat(const ViewConfig& config) const
{    
    D3DFORMAT format = SF_PLATFORM_APP_COLOR_FORMAT;

    // Use 16-bit bit depth if requested and supported.
    if (config.ColorBits == 16)
    {        
        if (pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                  D3DFMT_R5G6B5, D3DFMT_R5G6B5, false) == D3D_OK)
            format = D3DFMT_R5G6B5;
        else if (pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                       D3DFMT_X1R5G5B5, D3DFMT_X1R5G5B5, false) == D3D_OK)
            format = D3DFMT_X1R5G5B5;
    }
    return format;
}

void DeviceImpl::calcPresentParameters(D3DPRESENT_PARAMETERS* params, const ViewConfig& config)
{
    D3DFORMAT format = determineFormat(config);
    bool      vsync = config.HasFlag(View_VSync);

    // Set up the structure used to create the D3DDevice    
    D3DPRESENT_PARAMETERS &d3dpp = PresentParams;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed                = FALSE;
    d3dpp.SwapEffect              = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat        = format;
    d3dpp.BackBufferCount         = 1;
    d3dpp.BackBufferWidth         = config.ViewSize.Width;
    d3dpp.BackBufferHeight        = config.ViewSize.Height;
    d3dpp.EnableAutoDepthStencil  = TRUE;
    d3dpp.AutoDepthStencilFormat  = D3DFMT_D24S8;
    d3dpp.PresentationInterval    = vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.MultiSampleQuality      = D3DMULTISAMPLE_NONE;

    if (config.HasFlag(View_FSAA))
    {
        // Discard required for FSAA.
        d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
        d3dpp.SwapEffect      = D3DSWAPEFFECT_DISCARD;
    }
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    // If you've already created the device, don't allow any changes.
    if ( pDevice )
        return false;

    // If we have already created the device, don't allow resolution changes.
    if ( !config->HasFlag(View_Size) )
    {
        // Default resolution (get it from the current settings).
        XVIDEO_MODE videoMode;
        XGetVideoMode( &videoMode );
        config->SetSize(videoMode.dwDisplayWidth, videoMode.dwDisplayHeight);
    }

    // Clamp to 720p, so we don't have to deal with tiling.
    Size<unsigned> curSize = config->GetSize();
    config->SetSize(Alg::Min((int)curSize.Width, 1280), Alg::Min((int)curSize.Height, 720));

    // Must have fullscreen, at 0,0.
    config->SetFlag(View_FullScreen, true);
    config->SetFlag(View_Stereo, false);
    config->SetPos(0, 0);
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId )
{
    if (!pD3D || !pHal)
        return false;
  
    pWindow = window;

    // Set up the structure used to create the D3DDevice    
    D3DPRESENT_PARAMETERS &d3dpp = PresentParams;
    calcPresentParameters(&d3dpp, config);

    // Create the D3DDevice.
    if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                  D3DCREATE_HARDWARE_VERTEXPROCESSING, 
                                  &d3dpp, &pDevice)))
    {
        return false;
    }

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    unsigned flags = 0;

    // Create our own MemoryManager, and use it to create the RenderBufferManager, specifically for the X360.
    pMemManager = SF_NEW Render::X360::MemoryManager();
    RBVideoMemory = (UPInt)pMemManager->Alloc(RBVideoSize, Render::X360::X360_TEXTURE_MEMORY_ALIGN, Render::Memory_X360_Physical);
    pRBManager = *SF_NEW Render::X360::RenderBufferManager(pDevice);
    pRBManager->AddVideoMemory(RBVideoMemory, RBVideoSize);

    Render::X360::HALInitParams initParams(pDevice, PresentParams, pMemManager, flags, renderThreadId);
    initParams.pRenderBufferManager = pRBManager;
    if (!pHal->InitHAL(initParams))
    {
        shutdownGraphics();
        return false;
    }

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
    // Don't allow reconfigure.
    return false;
}

void DeviceImpl::shutdownGraphics()
{
    if (!pDevice)
        return;

    // RenderBufferManager must be destroyed before (or during ShutdownHAL). It has access to the
    // internally created TextureManager, and may try to call into it during its shutdown.
    pRBManager  = 0;

    if (pHal)
        pHal->ShutdownHAL();

    pMemManager->Free((void*)RBVideoMemory, RBVideoSize );
    RBVideoMemory = 0;

    delete pMemManager;
    pMemManager = 0;

    pDevice->Release();
    pDevice = 0;

    Status = Device_NeedInit;
}

bool DeviceImpl::takeScreenShot(const String& filename)
{
    // X360 only accepts backslashes as separators, and must be an absolute path.
    // D3DXSaveSurfaceToFile will not create directories if they don't exist.
    String path = "";

    // Check if there is a drive letter already, if not, get the default file path.
    if ( filename[1] != ':' )
        path = AppBase::GetDefaultFilePath();
    path += filename.GetPath();
    StringBuffer sb;
    for ( UPInt i = 0; i < path.GetLength(); ++i )
    {
        char c = path[i];
        if ( c != '/' )
            sb.AppendChar(c);
        else
            sb.AppendChar('\\');

        if ( (c == '/' || c =='\\') && !CreateDirectory(sb.ToCStr(), 0) &&
             GetLastError() != ERROR_ALREADY_EXISTS )
        {
            DWORD err = GetLastError();
            return false;
        }

    }
    path = sb;
    path += filename.GetFilename();

    Ptr<IDirect3DSurface9> pbackSurface = 0;
    if ( FAILED( pDevice->GetBackBuffer(0, 0, 0, &pbackSurface.GetRawRef()) ) )
        return false;

    D3DSURFACE_DESC desc;
    if (FAILED(pbackSurface->GetDesc(&desc)))
        return false;
        
    Ptr<IDirect3DTexture9> ptempTexture = 0;
    if ( FAILED(pDevice->CreateTexture(desc.Width, desc.Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &ptempTexture.GetRawRef(), 0)) )
        return false;

    // If the filename doesn't have .png on the end, append it, because that is the format.
    if ( path.GetExtension().CompareNoCase(".png") != 0 )
        path.AppendString(".png");

    pDevice->Resolve( D3DRESOLVE_RENDERTARGET0, 0, ptempTexture, 0, 0, 0, 0, 1.0f, 0, 0 );
    HRESULT hr = D3DXSaveTextureToFile(path, D3DXIFF_PNG, ptempTexture, 0 );
    return SUCCEEDED(hr);
}

void DeviceImpl::clear(UInt32 color)
{
    ClearColor = color;

    // Only really do a clear if it is the final pass.
    if ( pHal->GetDisplayPass() == Render::Display_Final )
        pDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0);
}

void DeviceImpl::OnHALEvent(Render::HALNotifyType type)
{
    if ( type == Render::HAL_FinalPassBegin )
        clear(ClearColor);
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

void Device::ResizeFrame(void*) { }
    
bool Device::GraphicsConfigOnMainThread() const
{
    return false;
}

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}


bool Device::InitGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, window, renderThreadId);
}

bool Device::ReconfigureGraphics(const ViewConfig& config)
{
    return pImpl->reconfigureGraphics(config);
}

void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

Size<unsigned> Device::GetDefaultViewSize()
{
    return Size<unsigned>(1280, 720);
}

void Device::Clear(UInt32 color)
{
    pImpl->clear(color);
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

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

    for (unsigned i = 0; i < pImpl->ScreenCaptureRequests.GetSize(); ++i)
        pImpl->takeScreenShot(pImpl->ScreenCaptureRequests[i]);

    pImpl->ScreenCaptureRequests.Clear();
    pImpl->pDevice->Present(NULL, NULL, NULL, NULL);
}

void Device::BeginFrame()
{
    D3DVIEWPORTx vp;
    vp.X = 0;
    vp.Y = 0;
    vp.MinZ = vp.MaxZ = 0;
    vp.Width = pImpl->PresentParams.BackBufferWidth;
    vp.Height = pImpl->PresentParams.BackBufferHeight;
    pImpl->pDevice->SetViewport(&vp);
}

void Device::SetWireframe(bool flag)
{
    pImpl->pDevice->SetRenderState(D3DRS_FILLMODE, flag ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
}

bool Device::TakeScreenShot(const String& filename)
{
    pImpl->ScreenCaptureRequests.PushBack(filename);
    return true;
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}

//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public Device::Window, public AppImplBase
{
    friend class AppBase;
    friend class XInputEventAdapter_X360;


    AppImpl(AppBase* app)
        : AppImplBase(app)
    {
        Created = false;
    }

    // Window Interface
    virtual void* GetHandle() { return 0; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);    
    bool          IsConsole() { return true; }

    bool          setupWindow(const String& title, const ViewConfig& config);
    bool          processMessages();
    bool          isPadConnected(unsigned i)
    {
        if ( i >= XUSER_MAX_COUNT ) 
            return false;
        return Pads.IsConnected(i);

    }

	virtual void  InitArgDescriptions(Args* args);

private:

    GamePad         Pads;

    void            updateGamePads();

    KeyModifiers    Modifiers;

    bool            Created;
    bool            BlockOnSize;
};

//---------------------------------------------------------------------------------------
class FxXBoxIMEEvent : public AppBase::IMEEvent
{
public:
    FxXBoxIMEEvent(): eType(0){};
    FxXBoxIMEEvent(XINPUT_KEYSTROKE& k, unsigned etype)
    {
        key.VirtualKey  = k.VirtualKey;
        key.Unicode     = k.Unicode;
        key.Flags       = k.Flags;
        key.UserIndex   = k.UserIndex;
        key.HidCode     = k.HidCode;
        eType           = etype;
        pLastKey        = 0;
    };

    FxXBoxIMEEvent(XINPUT_KEYSTROKE* plastKey, unsigned etype)
    {
        pLastKey        = plastKey;
        eType           = etype;
    };

    FxXBoxIMEEvent(XINPUT_KEYSTROKE& k, XINPUT_KEYSTROKE* plastKey, unsigned etype)
    {
        key.VirtualKey  = k.VirtualKey;
        key.Unicode     = k.Unicode;
        key.Flags       = k.Flags;
        key.UserIndex   = k.UserIndex;
        key.HidCode     = k.HidCode;
        pLastKey        = plastKey;
        eType           = etype;
    };

    XINPUT_KEYSTROKE key;
    XINPUT_KEYSTROKE* pLastKey;
    unsigned eType; // Indicates before or after event 
};

//---------------------------------------------------------------------------------------
bool AppImpl::processMessages()
{
    updateGamePads();

    // Pass empty event to XBoxIMEManager to inquire about the state of the
    // XIME queue
	bool ret = true;
	XINPUT_KEYSTROKE key;
	XINPUT_KEYSTROKE lastKey = {0};
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_X360)
    FxXBoxIMEEvent eventBefore;
    ret = pApp->OnIMEEvent(0, 0, 0, 0, false);
   
#endif
    // If there are events in the queue that need to be processed, don't remove the
    // current keystroke from keystroke queue.
    if (ret)
    {
        HRESULT hr = XInputGetKeystroke(XUSER_INDEX_ANY, XINPUT_FLAG_ANYDEVICE, &key );
        if(hr == ERROR_SUCCESS)
        {
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_X360)
            FxXBoxIMEEvent eventKey(key, 1);
            pApp->OnIMEEvent(0, eventKey.key.VirtualKey, 0, 0, false);
            lastKey = key;
#endif
        }
    }
    // Either way, get the appropriate key from IMEManager and process it. If
    // IME is not active, this will be the last keystroke.
#if defined(SF_ENABLE_IME) && defined(SF_ENABLE_IME_X360)
    FxXBoxIMEEvent eventKey(&lastKey, 2);
    ret = pApp->OnIMEEvent(0, eventKey.key.VirtualKey, 0, 0, false);
    key = lastKey;
#endif

    if(key.Flags)
    {
        KeyCode kc = Key::None;
        if (key.VirtualKey >= 'A' && key.VirtualKey <= 'Z')
        {
            kc = (KeyCode) ((key.VirtualKey - 'A') + Key::A);
        }
        else if (key.VirtualKey >= VK_F1 && key.VirtualKey <= VK_F15)
        {
            kc = (KeyCode) ((key.VirtualKey - VK_F1) + Key::F1);
        }
        else if (key.VirtualKey >= VK_NUMPAD0 && key.VirtualKey <= VK_DIVIDE)
        {
            kc = (KeyCode) ((key.VirtualKey - VK_NUMPAD0) + Key::KP_0);
        }
        else
        {
            for (int i = 0; KeyCodeMap[i].winKey != 0; i++)
            {
                if (key.VirtualKey == (unsigned)KeyCodeMap[i].winKey)
                {
                    kc = KeyCodeMap[i].appKey;
                    break;
                }
            }
        }
        KeyModifiers mods;
        if (key.Flags & XINPUT_KEYSTROKE_CTRL)
            mods.SetCtrlPressed(true);
        if (key.Flags & XINPUT_KEYSTROKE_SHIFT)
            mods.SetShiftPressed(true);
        if (key.Flags & XINPUT_KEYSTROKE_ALT)
            mods.SetAltPressed(true);
        if((key.Flags & XINPUT_KEYSTROKE_KEYDOWN) || (key.Flags & XINPUT_KEYSTROKE_REPEAT))
            pApp->OnKey(0, kc, key.Unicode, 1, mods);
        else if((key.Flags & XINPUT_KEYSTROKE_KEYUP))
            pApp->OnKey(0, kc, key.Unicode, 0, mods);
    }
    return true;
}

class XInputEventAdapter_X360 : public XInputEventAdapter
{
public:
    XInputEventAdapter_X360(AppImpl* pimpl) : XInputEventAdapter(pimpl->pApp), pAppImpl(pimpl) { }
    void OnMouseMove(unsigned mouseIndex, const Point<int>& mousePos, KeyModifiers mods)
    { 
        pAppImpl->MousePos = mousePos; 
        pApp->OnMouseMove(mouseIndex, mousePos, mods);
    }

private:
    AppImpl* pAppImpl;
};

void AppImpl::updateGamePads()
{
    XInputEventAdapter_X360 adapter(this);
    Pads.Update(adapter);
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;
    
    if (Created)
        return false;

    // Set the mouse (driven by the pads) to be in the center of the screen.
    Size<unsigned> p = Config.GetSize();
    if ( p.IsEmpty() )
        p = Device::GetDefaultViewSize();
    p /= 2;
    Point<unsigned> pos( p.Width, p.Height );
    Pads.SetMousePosition( pos );

    return true;
}

void AppImpl::ConfigureWindow(ViewConfig& newConfig)
{
    // Configures window for a new video mode.

    Config = newConfig;

}

//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase()
 :  Cursor(false),
    ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}
AppBase::~AppBase()
{
    delete pImpl;
}


void AppImpl::InitArgDescriptions(Args* args)
{
    AppImplBase::InitArgDescriptions(args);
    ArgDesc options[] =
    {
        {"ndp",   "NoDebugPopups",  Args::Flag,      "",   "Disables waiting for a debugger to attach, dumps are automatically created\n" },
        {"",      "",               Args::ArgEnd,    "",   ""}
    };
    args->AddDesriptions(options);
}

void AppBase::SetOrientationMode(bool)
{
}

//------------------------------------------------------------------------

// This call cleans up the application and kills the window.
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
    // Active title.
    return "D:\\";
}

// Number of mice or similar controllers available
int AppBase::GetMouseCount()
{
    // One mouse per pad.
    int padCount = 0;
    for ( int i = 0; i < XUSER_MAX_COUNT; ++i )
    {
        if ( pImpl->Pads.IsConnected(i) )
            padCount++;
    }
    return padCount;
}

int AppBase::GetKeyboardCount()
{
    // Only allow one keyboard for now.
    XINPUT_KEYBOARDSTATE state;
    if ( XInputGetKeyboardState(0, &state) == ERROR_DEVICE_NOT_CONNECTED )
        return 0;
    return 1;
}

bool AppBase::IsPadConnected(unsigned pad)
{
    return pImpl->isPadConnected(pad);
}


char** CommandLineToArgv(char* CmdLine, int* pargc)
{
    char** argv;
    char*  carg;
    unsigned   len;
    unsigned   narg = 0;

    bool  inquotes = false;
    bool  inarg = false;
    bool  inempty = true;

    len = strlen(CmdLine);
    unsigned strpsize = (len / 2 +1 ) * sizeof(char**);

    argv = (char**)SF_ALLOC(strpsize + (len+2)*sizeof(char), Stat_Default_Mem);

    carg = (char*)(((char*)argv) + strpsize);

    argv[0] = carg;

    unsigned   i = 0, j = 0;
    while(CmdLine[i]) 
    {
        char c = CmdLine[i];
        if(inquotes) 
        {
            if(c == '\"') 
                inquotes = false;
            else 
                carg[j++] = c;
        } 
        else 
        {
            if (c == ' ' || c == '\t' || c =='\r' || c =='\n')
            {
                if(inarg) 
                    carg[j++] = '\0';
                inarg = false;
                inempty = true;
            }
            else
            {
                inarg = true;
                if(inempty)
                    argv[narg++] = carg+j;
                inempty = false;
                if (c == '\"')
                    inquotes = true;
                else
                    carg[j++] = c;
            }
           
        }
        i++;
    }
    carg[j] = '\0';
    argv[narg] = 0;

    (*pargc) = narg;
    return argv;
}


// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{    
   // Scaleform::System initSFSystem;
    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    char* cmdln = GetCommandLine();
    char **targv = CommandLineToArgv(cmdln, &argc);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, targv)))
    {
        SF_FREE(targv);
        return 0;
    }
    SF_FREE(targv);
    // Initialize application and its window.
    ViewConfig config;
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return 0;
    }

    // Application / Player message loop.
    pImpl->NextUpdateTicks = Timer::GetTicks() + pImpl->FrameTimeTicks;
    while (!IsShuttingDown())
    {
        if ( !pImpl->processMessages() )
            break;

        // Wait for Update time if necessary; consider timing wrap-around.
        if (pImpl->FrameTime != 0.0f)
        {
            UInt64  ticks = Timer::GetTicks();

            if ((pImpl->NextUpdateTicks > ticks) )
            {
                UInt64   waitTicks = pImpl->NextUpdateTicks - ticks;
                unsigned waitMs = (unsigned)(waitTicks / 1000);
                ::Sleep(waitMs);
                continue;
            }
            pImpl->NextUpdateTicks = ticks + pImpl->FrameTimeTicks;
        }
        OnUpdateFrame(true);
    }

    OnShutdown();

    return 0;
}

class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern) :
        DirectoryImplBase(path, pattern) { }

    virtual void ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Pattern = pattern;
    Path = path;
    unsigned start = 0;
    String winpath  = "";
    unsigned length = Pattern.GetLength();
    for (unsigned i = 0; i < Pattern.GetLength(); i++ )
    {
        bool s = false;
        String sp;
        if (Pattern.GetCharAt(i) == ';') 
            sp = Pattern.Substring(start, i);
        else if (i == length - 1)
            sp = Pattern.Substring(start, i + 1);
        else
            continue;
        winpath = Path + sp;
        WIN32_FIND_DATAA dp;
        HANDLE hFind = FindFirstFileA(winpath, &dp);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            Filenames.PushBack(dp.cFileName);
            while (FindNextFileA(hFind, &dp))
                Filenames.PushBack(dp.cFileName);
            FindClose(hFind);
        }
        start = i+1;
    }

    CurFile = 0;
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = SF_NEW DirectoryImpl(path, pattern);
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


}} // Scaleform::Platform


//------------------------------------------------------------------------
int main(int argc, char** argv)
{    
    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int result = app->AppMain(argc, argv);
    Scaleform::Platform::AppBase::DestroyInstance(app);

    return result;
}

