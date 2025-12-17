/**************************************************************************

Filename    :   Platform_D3D9.cpp
Content     :   Platform Device Support for D3D9
Created     :   2009
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D9/D3D9_HAL.h"
#include "Platform.h"
#include <WinReg.h>
#if SF_CC_MSVC < 1700
#include <d3dx9tex.h>
#endif
#include <ShlObj.h>

// Uncomment this to use a IDirect3DDevice9Ex device.
//#define SF_D3D9_USE_D3D9EX

namespace Scaleform { namespace Platform {

// Define this to enable NVidia Perf HUD support.
#define SF_ENABLE_NV_PERF_HUD

#ifdef SF_BUILD_DEBUG
#define SF_PLATFORM_D3D_SDK_VERSION     (32 | 0x80000000)
#else
#define SF_PLATFORM_D3D_SDK_VERSION     32
#endif

#define  SF_PLATFORM_APP_COLOR_FORMAT           D3DFMT_UNKNOWN
#define  SF_PLATFORM_APP_DEPTHSTENCIL_FORMAT    D3DFMT_D24S8


class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    enum D3DRuntime
    {
        D3DRT_Unknown,
        D3DRT_Release,
        D3DRT_Debug
    };

    D3DRuntime detectD3DRuntime();
    D3DFORMAT determineFormat(const ViewConfig& config) const;
    D3DFORMAT determineDepthStencilFormat(const ViewConfig& config) const;

    void      calcPresentParameters(D3DPRESENT_PARAMETERS* params, const ViewConfig& config);    

    bool adjustFullScreenRes(D3DFORMAT format, Size<unsigned>* size);
    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();
    bool takeScreenShot(const String& filename);

    void updateStatus();

    Device::Window*         pWindow;
    Ptr<Render::D3D9::HAL>  pHal;
    DeviceStatus            Status;

    // D3D Variables
    HWND                    hWnd;
#ifdef SF_D3D9_USE_D3D9EX
    LPDIRECT3D9EX           pD3DEx;         // Used to create the D3D9DeviceEx.
#endif // SF_D3D9_USE_D3D9EX
    LPDIRECT3D9             pD3D;           // Used to create the D3D9Device.
    LPDIRECT3DDEVICE9       pDevice;        // Our rendering device.
    D3DPRESENT_PARAMETERS   PresentParams;
    // Detected FSAA capability.
    bool                    FSAASupported;
};


DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0),
#ifdef SF_D3D9_USE_D3D9EX
   pD3DEx(0),
#endif
   pD3D(0),
   pDevice(0),
   FSAASupported(false)   
{
    bool createD3D9Ex = false;

#ifdef SF_D3D9_USE_D3D9EX
    // Check to see whether Direct3DCreate9Ex is present in the runtime.
    HMODULE libHandle = LoadLibraryA("d3d9.dll");
    if (libHandle != 0)
        createD3D9Ex = GetProcAddress( libHandle, "Direct3DCreate9Ex" ) != 0;

    if (createD3D9Ex)
    {
        Direct3DCreate9Ex(SF_PLATFORM_D3D_SDK_VERSION, &pD3DEx);

        // The D3DEx interface is a subclass of D3D, so it can be reused without changing other code,
        // except for those places where it must be used explicitly for new functionality.
        pD3D = pD3DEx;
    }
#endif

    if (!createD3D9Ex)
    {
        pD3D = Direct3DCreate9(SF_PLATFORM_D3D_SDK_VERSION); 
    }


    if (pD3D)
        pHal = *SF_NEW Render::D3D9::ProfilerHAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
    if (pD3D)
        pD3D->Release();
}


// This detects if D3D9 used Debug Runtime (Enabled in DirectX Control Panel);
// we need to do this force MULTITHREADED device mode.
DeviceImpl::D3DRuntime DeviceImpl::detectD3DRuntime()
{
    HKEY        regD3DKey = 0;
    DWORD       debugRuntimeVal = 0;
    DWORD       debugRuntimeValSize = sizeof(DWORD);
    D3DRuntime  runtime = D3DRT_Unknown;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Direct3D\\"), 0,
        KEY_READ, &regD3DKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(regD3DKey, TEXT("LoadDebugRuntime"), 0, 0,
                            (BYTE*)&debugRuntimeVal, &debugRuntimeValSize) == ERROR_SUCCESS)
        {
            if (debugRuntimeVal != 0)
                runtime = D3DRT_Debug;
            else
                runtime = D3DRT_Release;
        }
        RegCloseKey(regD3DKey);
    }
    return runtime;
}


D3DFORMAT DeviceImpl::determineFormat(const ViewConfig& config) const
{    
    D3DFORMAT format = SF_PLATFORM_APP_COLOR_FORMAT;
    bool      fullScreen = config.HasFlag(View_FullScreen);

    if (fullScreen && (SF_PLATFORM_APP_COLOR_FORMAT == D3DFMT_UNKNOWN))
        format = D3DFMT_X8R8G8B8;

    // Use 16-bit bit depth if requested and supported.
    if (config.ColorBits== 16)
    {        
        if (pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                  D3DFMT_R5G6B5, D3DFMT_R5G6B5, !fullScreen) == D3D_OK)
            format = D3DFMT_R5G6B5;
        else if (pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                       D3DFMT_X1R5G5B5, D3DFMT_X1R5G5B5, !fullScreen) == D3D_OK)
            format = D3DFMT_X1R5G5B5;
    }

    if (format == D3DFMT_UNKNOWN)
    {
        D3DDISPLAYMODE mode;
        pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
        format = mode.Format;
    }

    return format;
}

D3DFORMAT DeviceImpl::determineDepthStencilFormat(const ViewConfig& config) const
{
    unsigned stencilBits = config.StencilBits == -1 ? 8 : config.StencilBits;
    unsigned depthBits = config.DepthBits == -1 ? 24 : config.DepthBits;

    bool stencilValid = stencilBits == 0 || stencilBits == 1 || stencilBits == 4 || stencilBits == 8;
    SF_DEBUG_WARNING1(!stencilValid, "Stencil buffers must be either 0, 1, 4 or 8 bits (requested %d). Using 8-bit stencil.", stencilBits);
    if (!stencilValid)
        stencilBits = 8;    
    bool depthValid = depthBits == 0 || depthBits == 15 || depthBits == 16 || depthBits == 24 || depthBits == 32;
    SF_DEBUG_WARNING1(!depthValid, "Depth buffers must be either 0, 15, 16, 24 or 32 bits (requested %d). Using 24-bit depth.", depthBits);
    if (!depthValid)
        depthBits = 24;

    // Figure out what possible formats we have, based on how many stencil bits are requested.
    switch(stencilBits)
    {
    case 0:
        switch(depthBits)
        {
        case 0:     return D3DFMT_UNKNOWN;                 // No depth or stencil.
        case 16:    return D3DFMT_D16;
        case 24:    return D3DFMT_D24X8;
        case 32:    return D3DFMT_D32;
        }
        break;
    case 1:
        SF_DEBUG_WARNING1(depthBits != 15, "Requested D%dS1, but no matching format exists. Using D15S1.", depthBits);
        return D3DFMT_D15S1;
    case 4:
        SF_DEBUG_WARNING1(depthBits != 24, "Requested D%dS4, but no matching format exists. Using D24X4S4.", depthBits);
        return D3DFMT_D24X4S4;
    case 8:
        SF_DEBUG_WARNING1(depthBits != 24, "Requested D%dS8, but no matching format exists. Using D24S8.", depthBits);
        return D3DFMT_D24S8;
    }
    SF_DEBUG_ASSERT(0, "Unhandled combination of depth/stencil bits.");
    return D3DFMT_D24S8;
}

void DeviceImpl::calcPresentParameters(D3DPRESENT_PARAMETERS* params, const ViewConfig& config)
{
    D3DFORMAT format = determineFormat(config);
    bool      fullScreen = config.HasFlag(View_FullScreen);
    bool      vsync = config.HasFlag(View_VSync);

    D3DPRESENT_PARAMETERS &d3dpp = *params;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed              = !fullScreen;
    d3dpp.BackBufferFormat      = format;
    d3dpp.AutoDepthStencilFormat= determineDepthStencilFormat(config);
    d3dpp.EnableAutoDepthStencil= d3dpp.AutoDepthStencilFormat != D3DFMT_UNKNOWN;
    d3dpp.BackBufferHeight      = config.ViewSize.Height;
    d3dpp.BackBufferWidth       = config.ViewSize.Width;
    d3dpp.PresentationInterval  = vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.SwapEffect            = fullScreen ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_COPY;

    // Enable multi-sampling if it is supported.
    if (SUCCEEDED(pD3D->CheckDeviceMultiSampleType(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, !fullScreen, D3DMULTISAMPLE_4_SAMPLES, 0)))
        FSAASupported = true;
    else
        FSAASupported = false;

    if (config.HasFlag(View_FSAA) && FSAASupported)
    {
        // Discard required for FSAA.
        d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
        d3dpp.SwapEffect      = D3DSWAPEFFECT_DISCARD;
    }
}


bool DeviceImpl::adjustFullScreenRes(D3DFORMAT format, Size<unsigned>* size)
{
    UINT           modeCount = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, format);
    UINT           i;
    D3DDISPLAYMODE mode;
    D3DDISPLAYMODE bestMode = { 0, 0, 0, format} ;
    D3DDISPLAYMODE biggestMode = { 0, 0, 0, format} ;

    if (!modeCount)
        return false;

    // Find the best matching mode resolution according to the following criteria:
    // a) Use exact match if possible, else
    // b) Use the smallest resolution that is greater than requested size, else
    // c) Use the largest available resolution.

    for (i = 0; i< modeCount; i++)
    {
        pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, format, i, &mode);

        // Matching mode found; it's a perfect fit so we are done.
        if ((mode.Width == size->Width) && (mode.Height == size->Height))
            return true;

        if ((mode.Width >= size->Width) && (mode.Height >= size->Height))
        {
            if (!bestMode.Width ||
                ((mode.Width < bestMode.Width) && (mode.Height < bestMode.Height)))
                bestMode = mode;
        }
        else
        {
            if (!biggestMode.Width ||
                ((mode.Width >= biggestMode.Width) && (mode.Height >= biggestMode.Height)))
                biggestMode = mode;
        }
    }

    if (bestMode.Width)
        size->SetSize(bestMode.Width, bestMode.Height);
    else
        size->SetSize(biggestMode.Width, biggestMode.Height);
    
    return true;
}


bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    if (config->HasFlag(View_FullScreen))
    {
        if (!config->HasFlag(View_Size))
        {
            int screenW = ::GetSystemMetrics(SM_CXSCREEN);
            int screenH = ::GetSystemMetrics(SM_CYSCREEN);
            config->SetSize(screenW, screenH);
        }

        D3DFORMAT format = determineFormat(*config);
        if (!adjustFullScreenRes(format, &config->ViewSize))
            return false;
        config->SetPos(0, 0);
    }
    else
    {
        if (config->HasFlag(View_Size))
        {
            if (config->ViewSize.Width < 1)
                config->ViewSize.Width = 1;
            if (config->ViewSize.Height< 1)
                config->ViewSize.Height= 1;
        }
    }
    config->SetFlag(View_Stereo, false);

    return true;
}


bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId)
{
    if (!pD3D || !pHal || !window)
        return false;

    pWindow = window;
    hWnd    = (HWND)window->GetHandle();

    // Set up the structure used to create the D3DDevice.
    calcPresentParameters(&PresentParams, config);

    // Support both SW and HW vertex processing (SW processing needed for Intel boards).
    DWORD    vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    D3DCAPS9 caps;
    pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
    if ((caps.VertexShaderVersion < D3DVS_VERSION(1,1)) ||
        !(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
        vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    // Although our framework uses D3D9 only from one thread at a time and thus doesn't
    // require MULTITHREADED flag, it may create and use D3D Device on different threads.
    // Such use results in uncontrollable warning spam from D3D Debug runtime, so we
    // force creation of MULTITHREADED device when Debug Runtime is enabled.
    if (detectD3DRuntime() != D3DRT_Release)
        vertexProcessing |= D3DCREATE_MULTITHREADED;


    // This logic enables NVidia PerfHUD, by looking at 'PerfHUD' adapter.
    UINT       adapterToUse = D3DADAPTER_DEFAULT;
    D3DDEVTYPE deviceType   = config.HasFlag(View_SoftwareRendering) ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

#ifdef SF_ENABLE_NV_PERF_HUD
    for (UINT adapter = 0; adapter < pD3D->GetAdapterCount(); adapter++)
    {
        D3DADAPTER_IDENTIFIER9 identifier;
        pD3D->GetAdapterIdentifier(adapter, 0, &identifier);
        if (strstr(identifier.Description, "PerfHUD") != 0)
        {
            adapterToUse = adapter;
            deviceType   = D3DDEVTYPE_REF;
            break;
        }
    }
#endif

#ifdef SF_D3D9_USE_D3D9EX
    SF_DEBUG_WARNING(pD3DEx == 0, "SF_D3D9_USE_D3D9EX was defined, but D3D9Ex it is not available (running on Windows pre-Vista?).");

    // Create the D3DDeviceEx.
    IDirect3DDevice9Ex* pdevice;
    if (!pD3DEx || FAILED(pD3DEx->CreateDeviceEx(adapterToUse, deviceType, hWnd,
        vertexProcessing, &PresentParams, 0, &pdevice)))
    {        
        pWindow = 0;
        hWnd = 0;
        return false;
    }
    pDevice = pdevice;
#else
    // Create the D3DDevice.
    if (FAILED(pD3D->CreateDevice(adapterToUse, deviceType, hWnd,
                                  vertexProcessing, &PresentParams, &pDevice)))
    {        
        pWindow = 0;
        hWnd = 0;
        return false;
    }
#endif

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    unsigned flags = 0;
    
    if (config.HasFlag(View_StaticBuffers))
        flags |= Render::D3D9::HALConfig_StaticBuffers;
    if (config.HasFlag(View_ShaderModel20))
        flags |= Render::D3D9::HALConfig_ShaderModel20;


    if (!pHal->InitHAL(Render::D3D9::HALInitParams(pDevice, PresentParams, flags, renderThreadId)))
    {
        shutdownGraphics();        
        pWindow = 0;
        hWnd = 0;
        return false;
    }

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
	if (!pDevice)
		return false;

//    SF_ASSERT(pDevice);    

    // If device reset is not possible yet, don't try.
    HRESULT hr = pDevice->TestCooperativeLevel();    
    if (hr == D3DERR_DEVICELOST)
    {
        Status = Device_Lost;
        return false;
    }
    
    // Custom D3D9::HAL function is used here to handle reset
    // instead of pHal->ResetVideoMode().
    pHal->PrepareForReset();   

    D3DPRESENT_PARAMETERS newParams;
    calcPresentParameters(&newParams, config);

    if (pDevice->Reset(&newParams) != D3D_OK)
    {
configureGraphics_Restore:
        if (hr != D3DERR_DEVICENOTRESET)
        {
            // If device was valid first, try to restore its state.
            if (pDevice->Reset(&PresentParams) == D3D_OK)
            {
                if (pHal->RestoreAfterReset())
                    Status = Device_Ready;
            }
        }
        return false;
    }

    if (!pHal->RestoreAfterReset())
        goto configureGraphics_Restore;

    Status = Device_Ready;
    PresentParams = newParams;
    return true;
}


void DeviceImpl::shutdownGraphics()
{
    if (!pDevice)
        return;
    if (pHal)
        pHal->ShutdownHAL();

    pDevice->Release();
    pDevice = 0;
    pWindow = 0;
    hWnd = 0;
    Status = Device_NeedInit;
}

void DeviceImpl::updateStatus()
{
    if (Status != Device_NeedInit)
    {
        HRESULT hr = pDevice->TestCooperativeLevel();
        if (hr == D3DERR_DEVICELOST)
            Status = Device_Lost;
        else if (hr == D3DERR_DEVICENOTRESET)
            Status = Device_NeedReconfigure;
    }
}


//------------------------------------------------------------------------
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
    pImpl->updateStatus();
    return pImpl->Status;
}

//------------------------------------------------------------------------
bool Device::GraphicsConfigOnMainThread() const
{
    // D3D9 requires Device configuration to run on the main window creation
    // thread. If this is not done, full-screen Mode Reset/Restore after lost
    // device calls will block in windows message hook.
    // This also means that we should use multi-threaded device with debug
    // libraries to avoid log spam.
    return true;
}

void Device::ResizeFrame(void*) { }
    
bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

bool Device::InitGraphics(const ViewConfig& config, Device::Window* window,
                          ThreadId renderThreadId)
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
    return Size<unsigned>(1024, 768);
}


//------------------------------------------------------------------------
void Device::Clear(UInt32 color)
{
    //D3DVIEWPORT9 vps;
    //pImpl->pDevice->GetViewport(&vps);
    const D3DPRESENT_PARAMETERS &params = pImpl->PresentParams;
    D3DVIEWPORT9 vp = { 0, 0, params.BackBufferWidth, params.BackBufferHeight, 0.0f, 0.0f };
    pImpl->pDevice->SetViewport(&vp);
    pImpl->pDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0); // Normal
    pImpl->pDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_STENCIL, color, 1.0f, 255); // Debug Masks (dirty stencil)

    //pImpl->pDevice->SetViewport(&vps);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    SF_UNUSED(WindowId);
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);
    pImpl->pDevice->Present(NULL, NULL, NULL, NULL);
}

void Device::BeginFrame()
{
}

void Device::SetWireframe(bool flag)
{
    pImpl->pDevice->SetRenderState(D3DRS_FILLMODE, flag ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
}

bool Device::TakeScreenShot(const String& filename)
{
    pImpl->takeScreenShot(filename);
    return true;
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}


bool DeviceImpl::takeScreenShot(const String& filename)
{
	#if SF_CC_MSVC < 1700
    String actualFilename = filename;
    Ptr<IDirect3DSurface9> pbackBuffer;
    if ( FAILED( pDevice->GetRenderTarget(0, &pbackBuffer.GetRawRef()) ) )
        return false;

    D3DSURFACE_DESC surfaceDesc;
    pbackBuffer->GetDesc(&surfaceDesc);

    // SHCreateDirectoryEx only accepts backslashes as separators, and must be an absolute path.
    // D3DXSaveSurfaceToFile will not create directories if they don't exist.
    String relpath = filename.GetPath();
    TCHAR workingDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, workingDir);
    String path(workingDir);
    path += '/';
    path += relpath;
    StringBuffer sb;
    for ( UPInt i = 0; i < path.GetLength(); ++i )
    {
        char c = path[i];
        if ( c != '/' )
            sb.AppendChar(c);
        else
            sb.AppendChar('\\');
    }
    path = sb;

    // If the filename doesn't have .png on the end, append it, because that is the format.
    if ( filename.GetExtension().CompareNoCase(".png") != 0 )
        actualFilename.AppendString(".png");

    SHCreateDirectoryExA(0, path, NULL);
    HRESULT hr = D3DXSaveSurfaceToFileA(actualFilename, D3DXIFF_PNG, pbackBuffer, 0, 0 );

    return SUCCEEDED(hr);
	#else

	SF_UNUSED(filename);
	return false;
	#endif
}

}} // Scaleform::Platform

