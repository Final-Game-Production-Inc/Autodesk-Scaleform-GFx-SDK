/**************************************************************************

Filename    :   Platform_D3D1x.cpp
Content     :   
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D1x/D3D1x_HAL.h"
#include "Platform.h"
#include <WinReg.h>
#include <ShlObj.h>

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Kernel/SF_SysFile.h"
#include <D3DCommon.h>

namespace Scaleform { namespace Platform {

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    DXGI_FORMAT determineFormat(const ViewConfig& config) const;
    DXGI_FORMAT determineDepthStencilFormat(const ViewConfig& config) const;

    void      calcSwapChainParams(DXGI_SWAP_CHAIN_DESC* params, const ViewConfig& config);    

    bool adjustFullScreenRes(DXGI_FORMAT format, Size<unsigned>* size);
    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);

    bool createResourceViews(const ViewConfig& config);
    bool destroyResourceViews();

    bool createSwapChain(const ViewConfig& config);
    bool destroySwapChain();

    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();
    bool takeScreenShot(const String& filename);

    void updateStatus();
    void presentFrame();
    void clear(Render::Color clearColor);

    Device::Window*             pWindow;
    Ptr<Render::D3D1x::HAL>     pHal;
    DeviceStatus                Status;

    // D3D Variables
    HWND                            hWnd;
    DXGI_SWAP_CHAIN_DESC            SwapChainDesc;              // Desc of the main swap chain.
    Ptr<IDXGISwapChain>             pDXGISwapChain;             // Rendering swap chain.
    IDXGIFactory*                   pDXGIFactory;               // DXGIFactory
    IDXGIAdapter*                   pDXGIAdapter;               // DXGIAdapter
    IDXGIDevice*                    pDXGIDevice;                // DXGIDevice
    ID3D1x(Device) *                pDevice;                    // Our rendering device.
    ID3D1x(DeviceContext) *         pDeviceContext;             // Rendering context.
    Ptr<ID3D1x(Texture2D)>          pDepthStencil;              // Depth/Stencil buffer.
    Ptr<ID3D1x(DepthStencilView)>   pDepthStencilView;          // Depth/Stencil view.
    Ptr<ID3D1x(RenderTargetView)>   pRenderTargetView;          // Render target view.

    D3D_FEATURE_LEVEL           FeatureLevel;   // Feature level of the created device.

    // Detected FSAA capability.
    Scaleform::ThreadId         RenderThreadId;
    bool                        FSAAEnabled;
    bool                        VSync;
};


DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
 : pWindow(0), pHal(0), Status(Device_NeedInit),
   hWnd(0),
   pDXGISwapChain(0),
   pDXGIFactory(0),
   pDXGIAdapter(0),
   pDevice(0),
   pDeviceContext(0),
   pDepthStencil(0),
   pDepthStencilView(0),
   pRenderTargetView(0),
   FeatureLevel(D3D_FEATURE_LEVEL_9_1),
   RenderThreadId(0),
   FSAAEnabled(false),
   VSync(false)
{
        pHal = *SF_NEW Render::D3D1x::ProfilerHAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    destroySwapChain();
    pHal.Clear();
}


DXGI_FORMAT DeviceImpl::determineFormat(const ViewConfig& config) const
{    
    IDXGIOutput* poutput;
    if ( pDXGIAdapter == 0 || FAILED(pDXGIAdapter->EnumOutputs(0, &poutput)))
	{
		SF_DEBUG_WARNING(1, "Could not enumerate output modes. Using DXGI_FORMAT_R8G8B8A8_UNORM.");
        return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
    
    // Use 128/64-bit bit depth if requested and supported.
    static const int MaximumModes = 512;
    DXGI_MODE_DESC modeDesc[MaximumModes];
    ZeroMemory(&modeDesc, sizeof modeDesc);
    UINT modeCount = 0;
    unsigned startFormat, endFormat;
    switch(config.ColorBits)
    {
    default: 
        SF_DEBUG_WARNING1(1, "Only 32 and 64 bit color buffers are supported (requested %d bits). Using 32bit.", config.ColorBits);
        // Intentional fallthrough.
    case -1:
    case 32:
        startFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS; 
        endFormat = DXGI_FORMAT_R8G8B8A8_SINT;
        break;
    case 64: 
        startFormat = DXGI_FORMAT_R16G16B16A16_TYPELESS; 
        endFormat = DXGI_FORMAT_R16G16B16A16_SINT;
                break;
        }

    unsigned format;
    for ( format = startFormat; format <= endFormat; ++format )
    {
        HRESULT hr = poutput->GetDisplayModeList((DXGI_FORMAT)format, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &modeCount, 0);
        if ( SUCCEEDED(hr) && modeCount > 0)
        {
            SF_DEBUG_ASSERT2(modeCount < MaximumModes, "Exceeded maximum expected display modes for a format (max=%d, actual=%d)", MaximumModes, modeCount);
            hr = poutput->GetDisplayModeList((DXGI_FORMAT)format, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &modeCount, modeDesc);
            if ( (SUCCEEDED(hr) || hr == DXGI_ERROR_MORE_DATA) && modeCount > 0)
                break;
        }
    }

    if ( modeCount > 0 )
	{
        return modeDesc[0].Format;
	}
    else
	{
		SF_DEBUG_WARNING(1, "Could find suitable buffer mode. Trying DXGI_FORMAT_R8G8B8A8_UNORM.");
        return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

DXGI_FORMAT DeviceImpl::determineDepthStencilFormat(const ViewConfig& config) const
{
    unsigned stencilBits = config.StencilBits == -1 ? 8 : config.StencilBits;
    unsigned depthBits = config.DepthBits == -1 ? 24 : config.DepthBits;

    bool stencilValid = stencilBits == 0 || stencilBits == 8;
    SF_DEBUG_WARNING1(!stencilValid, "Stencil buffers must be either 0 or 8 bits (requested %d). Using 8-bit stencil.", stencilBits);
    if (!stencilValid)
        stencilBits = 8;    
    bool depthValid = depthBits == 0 || depthBits == 16 || depthBits == 24 || depthBits == 32;
    SF_DEBUG_WARNING1(!depthValid, "Depth buffers must be either 0, 16, 24 or 32 bits (requested %d). Using 24-bit depth.", depthBits);
    if (!depthValid)
        depthBits = 24;

    // Figure out what possible formats we have, based on how many stencil bits are requested.
    switch(stencilBits)
    {
    case 0:
        switch(depthBits)
        {
        case 0:     return DXGI_FORMAT_UNKNOWN;                 // No depth or stencil.
        case 16:    return DXGI_FORMAT_D16_UNORM;
        case 24:    SF_DEBUG_WARNING(1, "Requested D24S0, but no matching format exists. Using D24S8.");
                    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case 32:    return DXGI_FORMAT_D32_FLOAT;
        }
        break;
    case 8:
        switch(depthBits)
        {
        case 0:     SF_DEBUG_WARNING(1, "Requested D0S8, but no matching format exists. Using D24S8.");
                    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case 16:    SF_DEBUG_WARNING(1, "Requested D16S8, but no matching format exists. Using D24S8.");
                    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case 24:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case 32:    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        }
        break;
    }
    SF_DEBUG_ASSERT(0, "Unhandled combination of depth/stencil bits.");
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void DeviceImpl::calcSwapChainParams(DXGI_SWAP_CHAIN_DESC* params, const ViewConfig& config)
{
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc,sizeof(desc));

    bool      fullScreen = config.HasFlag(View_FullScreen);
    VSync                = config.HasFlag(View_VSync);

    desc.BufferCount                        = 1;
    desc.BufferDesc.Width                   = config.ViewSize.Width;
    desc.BufferDesc.Height                  = config.ViewSize.Height;
    desc.BufferDesc.Format                  = determineFormat(config);
    desc.BufferDesc.RefreshRate.Numerator   = VSync ? 1  : 0;
    desc.BufferDesc.RefreshRate.Denominator = VSync ? 60 : 0;
    desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow                       = hWnd;
    desc.SampleDesc.Count                   = config.HasFlag(View_FSAA) ? 4 : 1;
    desc.SampleDesc.Quality                 = 0;
    desc.Windowed                           = !fullScreen;
    desc.Flags                              = 0;

    memcpy(params, &desc, sizeof desc);
}


bool DeviceImpl::adjustFullScreenRes(DXGI_FORMAT format, Size<unsigned>* size)
{
    // Find the best matching mode resolution.
    IDXGIOutput* poutput;
    if ( pDXGIAdapter == 0 || FAILED(pDXGIAdapter->EnumOutputs(0, &poutput)))
        return false;

    DXGI_MODE_DESC modeDesc, outModeDesc;
    modeDesc.Format = format;
    modeDesc.Width  = size->Width;
    modeDesc.Height = size->Height;
    modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    modeDesc.RefreshRate.Denominator = 0;
    modeDesc.RefreshRate.Numerator = 0;

    if ( FAILED(poutput->FindClosestMatchingMode(&modeDesc, &outModeDesc, pDevice) ) )
        return false;

    size->SetSize(outModeDesc.Width, outModeDesc.Height);
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

        DXGI_FORMAT format = determineFormat(*config);
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
    if (!pHal || !window)
        return false;

    pWindow         = window;
    hWnd            = (HWND)window->GetHandle();
    RenderThreadId  = renderThreadId;
    FSAAEnabled     = config.HasFlag(View_FSAA);

    // Set up the structure used to create the D3DDevice.
#ifdef SF_BUILD_DEBUG
    UINT flags = D3D1x(CREATE_DEVICE_DEBUG);
#else
    UINT flags = 0;
#endif

#if (SF_D3D_VERSION == 11)
    static const int maxFeatures = 6;
    D3D_FEATURE_LEVEL featureLevels[maxFeatures] = 
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // If a software device is requested, use the WARP driver.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/gg615082%28v=vs.85%29.aspx#how_to_use_warp
    D3D_DRIVER_TYPE driverType = config.HasFlag(View_SoftwareRendering) ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;

    // Create the D3DDevice.
    if (FAILED(D3D11CreateDevice( 0, driverType, 0, flags, featureLevels, maxFeatures, 
        D3D1x(SDK_VERSION), &pDevice, &FeatureLevel, &pDeviceContext ) ))
    {        
        pWindow = 0;
        hWnd = 0;
        return false;
    }
    SF_DEBUG_MESSAGE1(FeatureLevel != D3D_FEATURE_LEVEL_11_0, "Created D3D11 with a lower feature level (0x%x)\n", FeatureLevel);
#elif (SF_D3D_VERSION == 10)
    static const int maxFeatures = 5;
    D3D10_FEATURE_LEVEL1 featureLevels[maxFeatures] = 
    {
        D3D10_FEATURE_LEVEL_10_1,
        D3D10_FEATURE_LEVEL_10_0,
        D3D10_FEATURE_LEVEL_9_3,
        D3D10_FEATURE_LEVEL_9_2,
        D3D10_FEATURE_LEVEL_9_1,
    };

    // If a software device is requested, use the WARP driver.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/gg615082%28v=vs.85%29.aspx#how_to_use_warp
    D3D10_DRIVER_TYPE driverType = config.HasFlag(View_SoftwareRendering) ? D3D10_DRIVER_TYPE_WARP : D3D10_DRIVER_TYPE_HARDWARE;

    // Create the D3DDevice.
    unsigned bestFeature = 0;
    while (bestFeature < maxFeatures &&
           FAILED(D3D10CreateDevice1( 0, driverType, 0, flags, featureLevels[bestFeature],
           D3D10_1_SDK_VERSION, &pDevice ) ))
    {        
        bestFeature++;
    }

    // Check for failure.
    if ( bestFeature >= maxFeatures )
    {
        pWindow = 0;
        hWnd = 0;
        return false;
    }
    SF_DEBUG_MESSAGE1(featureLevels[bestFeature] != D3D_FEATURE_LEVEL_10_1, "Created D3D10.1 with a lower feature level (0x%x)\n", featureLevels[bestFeature]);
    pDeviceContext = pDevice;
#endif

    if ( !createSwapChain(config) )
        return false;

    if ( !createResourceViews(config))
        return false;

    unsigned hflags = 0;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    if (!pHal->InitHAL(Render::D3D1x::HALInitParams(pDevice, D3D11(pDeviceContext,) hflags, renderThreadId)))
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

    // This function may be recursive, so only allow one entry.
    static bool reconfigureActive = false;
    if ( reconfigureActive )
        return true;
    reconfigureActive = true;

    DXGI_SWAP_CHAIN_DESC newParams;
    calcSwapChainParams(&newParams, config);

    pHal->PrepareForReset();
    if (!destroyResourceViews())
        return false;
    Status = Device_NeedInit;


    // If changing FSAA mode, we need to recreate everything.
    if ( FSAAEnabled != config.HasFlag(View_FSAA) )
    {
        if ( !createSwapChain(config) )
            return false;
    }
    else
    {
        DXGI_MODE_DESC modeDesc = newParams.BufferDesc;

        BOOL fullScreen;
        pDXGISwapChain->GetFullscreenState(&fullScreen, NULL);
        if (config.HasFlag(View_FullScreen) != (fullScreen != 0))
        {
            if (FAILED(pDXGISwapChain->SetFullscreenState(config.HasFlag(View_FullScreen), 0)))
                return false;
            if (FAILED(pDXGISwapChain->ResizeTarget(&modeDesc)))
                return false;
            if (FAILED(pDXGISwapChain->ResizeBuffers(1, modeDesc.Width, modeDesc.Height, modeDesc.Format, newParams.Flags)))
                return false;
        }
        else
        {
            if (FAILED(pDXGISwapChain->ResizeBuffers(1, modeDesc.Width, modeDesc.Height, modeDesc.Format, newParams.Flags)))
                return false;
        }
    }

    if ( !createResourceViews(config))
        return false;
    pHal->RestoreAfterReset();
    Status = Device_Ready;

    VSync = config.HasFlag(View_VSync);
    SwapChainDesc = newParams;
    reconfigureActive = false;
    return true;
}

bool DeviceImpl::createResourceViews(const ViewConfig& config)
{
    // Create the views for the backbuffer and depthstencil.
    ID3D1x(Texture2D)* pviewTexture;
    HRESULT hr;

    if ( FAILED( pDXGISwapChain->GetBuffer(0, __uuidof(*pviewTexture), (LPVOID*)&pviewTexture ) ) )
        return false;

    D3D1x(TEXTURE2D_DESC) bbdesc;
    pviewTexture->GetDesc(&bbdesc);
    hr = pDevice->CreateRenderTargetView(pviewTexture, 0, &pRenderTargetView.GetRawRef() );
    pviewTexture->Release();
    if ( FAILED( hr ) )
        return false;

    D3D1x(TEXTURE2D_DESC) dsdesc;
    dsdesc.Width = bbdesc.Width;
    dsdesc.Height = bbdesc.Height;
    dsdesc.MipLevels = 1;
    dsdesc.ArraySize = 1;

    dsdesc.Format = determineDepthStencilFormat(config);

    // UNKNOWN == no depth/stencil buffer.
    if (dsdesc.Format != DXGI_FORMAT_UNKNOWN)
    {
    dsdesc.SampleDesc.Count = bbdesc.SampleDesc.Count;
    dsdesc.SampleDesc.Quality = bbdesc.SampleDesc.Quality; 
    dsdesc.Usage = D3D1x(USAGE_DEFAULT);
    dsdesc.BindFlags = D3D1x(BIND_DEPTH_STENCIL);
    dsdesc.CPUAccessFlags = 0;
    dsdesc.MiscFlags = 0;
    hr = pDevice->CreateTexture2D(&dsdesc, 0, &pDepthStencil.GetRawRef());
    if ( FAILED(hr) )
        return false;
    pDevice->CreateDepthStencilView(pDepthStencil, 0, &pDepthStencilView.GetRawRef());
    }

    // Set it in the context.
    ID3D1x(RenderTargetView)* prtView = pRenderTargetView.GetPtr();
    pDeviceContext->OMSetRenderTargets(1, &prtView, pDepthStencilView.GetPtr() );
    return true;
}

bool DeviceImpl::destroyResourceViews()
{
    if ( pDeviceContext )
        pDeviceContext->OMSetRenderTargets(0, 0, 0);

    pRenderTargetView = 0;
    pDepthStencilView = 0;
    pDepthStencil     = 0;
    return true;
}

bool DeviceImpl::createSwapChain(const ViewConfig& config)
{
    if (!destroySwapChain())
        return false;

    // Can't create the swap chain without a device.
    if ( pDevice == 0 )
        return false;

    // Query everything we can get at this point.
    IDXGIDevice*        pdevice;
    IDXGIFactory*       pfactory;
    IDXGIAdapter*       padapter;
    if ( FAILED(pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdevice)) ||
        FAILED(pdevice->GetParent(__uuidof(IDXGIAdapter), (void **)&padapter)) ||
        FAILED(padapter->GetParent(__uuidof(IDXGIFactory), (void **)&pfactory)))
    {
        return false;
    }
    pDXGIDevice = pdevice;
    pDXGIFactory = pfactory;
    pDXGIAdapter = padapter;
    calcSwapChainParams(&SwapChainDesc, config);

    Ptr<IDXGISwapChain> pswapChain;

    if (FAILED(pfactory->CreateSwapChain(pdevice, &SwapChainDesc, &pswapChain.GetRawRef())))
    {
        pDXGIDevice = 0;
        pDXGIFactory = 0;
        pDXGIAdapter = 0;
        return false;
    }

    // Disable DXGI automatic Alt-Enter to enter fullscreen. Use Alt+U instead.
    pfactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES);
    pDXGISwapChain = pswapChain;

    FSAAEnabled = config.HasFlag(View_FSAA);
    return true;
}

bool DeviceImpl::destroySwapChain()
{
    if ( pDXGISwapChain )
        pDXGISwapChain->SetFullscreenState(FALSE, 0);
    pDXGIDevice = 0;
    pDXGIFactory = 0;
    pDXGIAdapter = 0;
    pDXGISwapChain = 0;
    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (!pDevice)
        return;

    destroySwapChain();

    pDepthStencil       = 0;
    pDepthStencilView   = 0;
    pRenderTargetView   = 0;

    if (pHal)
        pHal->ShutdownHAL();

    pDeviceContext->ClearState();
    pDeviceContext->Flush();
    pDeviceContext->Release();
    pDeviceContext = 0;

    pDevice->Release();
    pDevice = 0;

    pWindow = 0;
    hWnd = 0;
    Status = Device_NeedInit;
}

void DeviceImpl::updateStatus()
{
}

void DeviceImpl::presentFrame()
{
    pDXGISwapChain->Present( VSync ? 1 : 0, 0 );
}

void DeviceImpl::clear(Render::Color clearColor)
{
    float rgba[4];
    clearColor.GetRGBAFloat(&rgba[0], &rgba[1], &rgba[2], &rgba[3] );
    pDeviceContext->ClearRenderTargetView(pRenderTargetView, rgba );
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
    pImpl->clear(color);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);
    pImpl->presentFrame();
}

void Device::BeginFrame()
{
}

void Device::SetWireframe(bool flag)
{
    pImpl->pHal->SetRasterMode(flag ? Render::D3D1x::HAL::RasterMode_Wireframe :
        Render::D3D1x::HAL::RasterMode_Default);
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}

bool Device::TakeScreenShot(const String& filename)
{
    return pImpl->takeScreenShot(filename);
}

bool DeviceImpl::takeScreenShot(const String& filename)
{
    String actualFilename = filename;

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

    Ptr<ID3D1x(RenderTargetView)> pbackBuffer;
    Ptr<ID3D1x(DepthStencilView)> pdsBuffer;
    Ptr<ID3D1x(Resource)>         presource;

    pDeviceContext->OMGetRenderTargets(1, &pbackBuffer.GetRawRef(), &pdsBuffer.GetRawRef());
    pbackBuffer->GetResource(&presource.GetRawRef());    

    Ptr<ID3D1x(Texture2D)> ptexture;
    Ptr<ID3D1x(Texture2D)> pstaging;
    if (FAILED(presource->QueryInterface(__uuidof(ID3D1x(Texture2D)), (void**)&ptexture.GetRawRef())))
        return false;
    D3D1x(TEXTURE2D_DESC) textureDesc;
    ptexture->GetDesc(&textureDesc);

    D3D1x(TEXTURE2D_DESC) desc;
    memset(&desc, 0, sizeof desc);
    desc.Width              = textureDesc.Width;
    desc.Height             = textureDesc.Height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UINT;
    desc.Usage              = D3D1x(USAGE_STAGING);
    desc.BindFlags          = 0;
    desc.CPUAccessFlags     = D3D1x(CPU_ACCESS_READ);
    desc.SampleDesc.Count   = 1;

    if (FAILED(pDevice->CreateTexture2D(&desc, 0, &pstaging.GetRawRef())))
        return false;

    pDeviceContext->CopyResource(pstaging, presource);
    D3D1x(MAPPED_TEXTURE2D) mappedResource;
    if (FAILED(D3D1xMapTexture2D(pDeviceContext, pstaging, 0, D3D1x(MAP_READ), 0, &mappedResource)))
        return false;

    // Now actually save it.
    Render::ImageData data;
    data.Initialize(Render::Image_R8G8B8A8, (UInt32)textureDesc.Width, (UInt32)textureDesc.Height, 
        (UPInt)mappedResource.RowPitch, (UByte*)mappedResource.pData);

#ifdef SF_ENABLE_LIBPNG
    bool success = Render::PNG::FileWriter::WriteImage<SysFile>(actualFilename, data);
#else
	bool success = false;
#endif
    D3D1xUnmapTexture2D(pDeviceContext, pstaging, 0);

    return success;
}

}} // Scaleform::Platform

