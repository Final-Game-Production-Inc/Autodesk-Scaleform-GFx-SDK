/**********************************************************************

Filename    :   Platform_XboxOne.cpp
Content     :   Durango D3D11 platform specific
Created     :   May, 2012
Authors     :   

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Platform.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_WString.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_UTF8Util.h"

#define SF_D3D_VERSION 11
#include "Render/D3D1x/D3D1x_HAL.h"

#include <wrl.h>

using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Xbox;

using Microsoft::WRL::ComPtr;

using namespace Scaleform;
using namespace Scaleform::BaseTypes;
using namespace Scaleform::Platform;
using namespace Scaleform::Render;

#undef Platform


//------------------------------------------------------------------------

static Platform::String^ SFStringToWin(const Scaleform::String& in)
{
	wchar_t* buf = (wchar_t*) alloca(2*in.GetLength()+2);
	UTF8Util::DecodeString(buf, in.ToCStr(), in.GetSize());
	return ref new Platform::String(buf);
}

//------------------------------------------------------------------------
// Using system Events, allows the main thread to block execution of the rendering thread
// (and thus, usage of the D3D context) during a suspend event in the App lifecycle.
class SuspendRenderThreadCommand : public Scaleform::Render::ThreadCommand
{
public:
    SuspendRenderThreadCommand(ComPtr<ID3D11DeviceContext> pcontext) 
    { 
        pcontext.As(&pDeviceContext);

    }
    virtual ~SuspendRenderThreadCommand() { }

    // Accessors for the two events associated with suspending the render thread.
    Event& GetRenderingCompleteEvent() { return RenderingPausedEvent; }
    Event& GetResumeCompleteEvent()    { return ResumeCompletedEvent; }

    virtual void Execute()
    {
        // Set the event rendering has finished (that this command has been reached in the rendering thread).
        RenderingPausedEvent.SetEvent();

        // Suspend the device.
        pDeviceContext.Get()->Suspend(0);

        // Now, wait until the resume event is signaled by the main thread (eg. during OnResume).
        ResumeCompletedEvent.Wait();

        // Now resume the device.
        pDeviceContext.Get()->Resume();
    }

protected:
#ifdef SF_DURANGO_MONOLITHIC
    ComPtr<ID3D11DeviceContextX>        pDeviceContext;
#else
    ComPtr<ID3DXboxPerformanceContext>  pDeviceContext;
#endif
    Event                           RenderingPausedEvent;
    Event                           ResumeCompletedEvent;
};

ref class ScaleformApp sealed : Windows::ApplicationModel::Core::IFrameworkView
{
public:
    ScaleformApp();
    
    virtual void Initialize(CoreApplicationView^ view);
    virtual void SetWindow(CoreWindow^ win);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

    CoreWindow^  GetWindow() { return Window; }

protected:
    void OnActivated(CoreApplicationView^ view, Activation::IActivatedEventArgs^ args);
    void OnSuspending(Platform::Object^ s, SuspendingEventArgs^ args);
    void OnResuming(Platform::Object^ s, Platform::Object^ args);
    void OnSize(CoreWindow^ win, WindowSizeChangedEventArgs^ args);
    void OnShutdown(CoreWindow^ win, CoreWindowEventArgs^ args);

    void HandleGamepadReading();

private:
	CoreWindow^  Window;

	AppBase*                        pApp;
	AppImpl*                        pImpl;
    bool                            ShutdownRequested;
    Ptr<SuspendRenderThreadCommand> SuspendResumeCommand;
    

    void HandlePadEvent(unsigned pad, bool down, unsigned idx, PadKeyCode kc);
};


//------------------------------------------------------------------------

namespace Scaleform { namespace SFPlatform {

class AppImpl : public Device::Window, public AppImplBase
{
public:
    AppImpl(AppBase* app) : AppImplBase(app), NeedPaint(true)
    {
        Created = false;
    }

    // Window Interface
    virtual void* GetHandle() { return 0; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);    
    
    bool          setupWindow(const String& title, const ViewConfig& config);

	// AppImplBase
	void          SetFrameTime(float seconds);

    ScaleformApp^ AppView;
    bool          Created;
    bool          NeedPaint;
};

class DeviceImpl : public Scaleform::NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();

    void updateStatus();
	bool createBuffers();

    AppBase*        pApp;
	AppImpl*        pImpl;
	ScaleformApp^   pAppView;
	DeviceStatus    Status;

	D3D_FEATURE_LEVEL               FeatureLevel;
	ComPtr<ID3D11Device>            pDevice;
	ComPtr<ID3D11DeviceContext>     pContext;
	ComPtr<IDXGISwapChain1>         pSwapChain;
	ComPtr<ID3D11RenderTargetView>  pRenderTargetView;
	ComPtr<ID3D11Texture2D>         pDepthStencil;
	ComPtr<ID3D11DepthStencilView>  pDepthStencilView;
	Ptr<Render::RenderTarget>       pRenderTarget;

    Ptr<Render::D3D1x::HAL>         pHal;
    int                             Width, Height;
};

//------------------------------------------------------------------------

DeviceImpl::DeviceImpl(ThreadCommandQueue *commandQueue) :
    Width(1920), Height(1080), pHal(0), Status(Device_NeedInit)
{
    pHal = *SF_NEW Render::D3D1x::HAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    config->ViewFlags = View_FullScreen | View_VSync | View_UseOrientation;
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId)
{
    if (!pHal || !window)
        return false;

    pImpl = (AppImpl*) window;
	pAppView = pImpl->AppView;
	pApp = pImpl->pApp;

	unsigned flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // Allow PIX captures in everything but shipping.
#if !defined(SF_BUILD_SHIPPING) && defined(SF_DURANGO_MONOLITHIC)
    flags |=  D3D11_CREATE_DEVICE_INSTRUMENTED;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, 2, D3D11_SDK_VERSION, 
								 &pDevice, &FeatureLevel, &pContext)))
		return 0;

	if (!createBuffers())
		return 0;

    if (!pHal->InitHAL(Render::D3D1x::HALInitParams(pDevice.Get(), pContext.Get(), 0, renderThreadId)))
    {
        shutdownGraphics();
        return false;
    }

	pRenderTarget = pHal->CreateRenderTarget(pRenderTargetView.Get(), pDepthStencilView.Get());

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::createBuffers()
{
	if (!pSwapChain)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
        swapChainDesc.Width = Width;
        swapChainDesc.Height = Height;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false; 
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = 0;

        ID3D11Device* pdevice = pDevice.Get();
        Ptr<IDXGIDevice1> pdxgiDevice;
        Ptr<IDXGIAdapter> pdxgiAdapter;
        Ptr<IDXGIFactory2> pdxgiFactory;

        if (FAILED(pdevice->QueryInterface(__uuidof( pdxgiDevice ), reinterpret_cast< void** >( &pdxgiDevice.GetRawRef() ) )))
            return false;
        if (FAILED(pdxgiDevice->GetAdapter(&pdxgiAdapter.GetRawRef())))
            return false;
        if (FAILED(pdxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast< void** >( &pdxgiFactory.GetRawRef() ) )))
            return false;

        Windows::UI::Core::CoreWindow^ window = pAppView->GetWindow();
        if (FAILED(pdxgiFactory->CreateSwapChainForCoreWindow(
            pDevice.Get(), reinterpret_cast<IUnknown*>(window), &swapChainDesc, nullptr, &pSwapChain)))
            return false;
	}
    else {
        pSwapChain->ResizeBuffers(2, Width, Height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    }

    ComPtr<ID3D11Texture2D> pbackBuffer;
    if (FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pbackBuffer)))
        return false;

    D3D11_TEXTURE2D_DESC backBufferDesc;
    pbackBuffer->GetDesc(&backBufferDesc);
	if (FAILED(pDevice->CreateRenderTargetView(pbackBuffer.Get(), 0, &pRenderTargetView)))
		return false;

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = backBufferDesc.Width;
    depthStencilDesc.Height = backBufferDesc.Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = backBufferDesc.SampleDesc.Count;
    depthStencilDesc.SampleDesc.Quality = backBufferDesc.SampleDesc.Quality; 
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;
    if (FAILED(pDevice->CreateTexture2D(&depthStencilDesc, 0, &pDepthStencil)))
        return false;
    pDevice->CreateDepthStencilView(pDepthStencil.Get(), 0, &pDepthStencilView);

	if (pHal->IsInitialized())
		pRenderTarget = pHal->CreateRenderTarget(pRenderTargetView.Get(), pDepthStencilView.Get());

    ID3D11RenderTargetView* prtView = pRenderTargetView.Get();
    pContext->OMSetRenderTargets(1, &prtView, pDepthStencilView.Get());

	pImpl->Config.ViewSize.Width = backBufferDesc.Width;
	pImpl->Config.ViewSize.Height = backBufferDesc.Height;

	pApp->OnSize(pImpl->Config.ViewSize);

    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
	if (!createBuffers())
		return false;

    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (pHal)
        pHal->ShutdownHAL();

	pRenderTargetView = nullptr;
	pDepthStencil = nullptr;
	pDepthStencilView = nullptr;
	pDevice = nullptr;
	pContext = nullptr;

    pAppView = nullptr;
    Status = Device_NeedInit;
}

void DeviceImpl::updateStatus()
{
}

}}

//------------------------------------------------------------------------

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    // Set the mouse (driven by the pads) to be in the center of the screen.
    Size<unsigned> p = Config.GetSize();
    if ( p.IsEmpty() )
        p = Device::GetDefaultViewSize();
    p /= 2;
    MousePos.SetPoint(p.Width, p.Height);

	return true;
}

void AppImpl::ConfigureWindow(ViewConfig& config)
{
}

void AppImpl::SetFrameTime(float)
{
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

bool Device::GraphicsConfigOnMainThread() const
{
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

Render::Size<unsigned> Device::GetDefaultViewSize()
{
    return Render::Size<unsigned>(1920, 1080);
}

void Device::Clear(UInt32 color)
{
	float rgba[4];
    Render::Color c(color);
    c.GetRGBAFloat(rgba);
	pImpl->pContext->ClearRenderTargetView(pImpl->pRenderTargetView.Get(), rgba);
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
	pImpl->pHal->SetRenderTarget(pImpl->pRenderTarget, true);
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

#ifdef SF_DURANGO_MONOLITHIC
    IDXGISwapChain1* swapChains[1];
    swapChains[0] = pImpl->pSwapChain.Get();

    DXGIX_PRESENTARRAY_PARAMETERS presentParams[1];
    presentParams[0].Disable = FALSE;
    presentParams[0].UsePreviousBuffer = FALSE;
    presentParams[0].SourceRect.left = 0;
    presentParams[0].SourceRect.top = 0;
    presentParams[0].SourceRect.right = pImpl->Width;
    presentParams[0].SourceRect.bottom = pImpl->Height;
    presentParams[0].DestRectUpperLeft.x = 0;
    presentParams[0].DestRectUpperLeft.y = 0;
    presentParams[0].ScaleFactorVert = 1.0f;
    presentParams[0].ScaleFactorHorz = 1.0f;

    HRESULT hr = DXGIXPresentArray(1, 0, 0, 1, swapChains, presentParams);
#else
	HRESULT hr = pImpl->pSwapChain->Present(1, 0);
#endif

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
		ShutdownGraphics();
		InitGraphics(pImpl->pImpl->Config, pImpl->pImpl, 0);
    }
}

void Device::BeginFrame()
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


//------------------------------------------------------------------------

AppBase::AppBase() : Cursor(false), ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}

AppBase::~AppBase()
{
    delete pImpl;
}

void AppBase::SetOrientationMode(bool allowRotate)
{
}

void AppBase::Shutdown()
{
    ShuttingDown = true;
}

bool AppBase::IsPadConnected(unsigned)
{
    return true;
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

int AppBase::GetDisplayCount()
{
    return 1;
}

Render::Size<float> AppBase::GetSafeArea()
{
    return Render::Size<float>(0.0f);
}

static char ContentDir[1024], StartupContentDir[1024];
static bool ContentDirInitialized = 0;

const char* AppBase::GetDefaultFilePath()
{
    if (!ContentDirInitialized)
    {
        ContentDirInitialized = 1;
		auto pathw = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
		String path (pathw->ToString()->Data());

        SFstrcpy(StartupContentDir, sizeof(StartupContentDir), path.ToCStr());
        SFstrcat(StartupContentDir, sizeof(StartupContentDir), "\\");
        
		SFstrcpy(ContentDir, sizeof(ContentDir), "");
    }

    return StartupContentDir;
}

String AppBase::GetContentDirectory()
{
    if (!ContentDirInitialized)
        GetDefaultFilePath();

    return ContentDir;
}

int AppBase::GetMouseCount()
{
    return 1;
}
int AppBase::GetKeyboardCount()
{
    return 0;
}

UInt32 AppBase::GetCaps() const
{
    return Cap_LockOrientation;
}

int AppBase::AppMain(int argc, char* argv[])
{    
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return 1;

    ViewConfig config;
    GetViewConfig(&config);
    if (!OnInit(config))
        return 1;

    return 0;
}


//------------------------------------------------------------------------

SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) :
    SystemCursorState(systemCursorAvailable, false), pImpl(0)
{
}

SystemCursorManager::~SystemCursorManager()
{
}

void SystemCursorManager::update(bool forceSet)
{
}

//------------------------------------------------------------------------

class DirectoryImpl : public DirectoryImplBase
{

public:
    DirectoryImpl(const String& path, const String& pattern) :
        DirectoryImplBase(path, pattern) {}

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
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}

//------------------------------------------------------------------------

ScaleformApp::ScaleformApp() : ShutdownRequested(false)
{
	pApp = Scaleform::SFPlatform::AppBase::CreateInstance();
    pImpl = (AppImpl*)pApp->GetDeviceWindow();

	pImpl->AppView = this;
}

void ScaleformApp::Initialize(CoreApplicationView^ view)
{
    view->Activated	            += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ScaleformApp::OnActivated);
    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &ScaleformApp::OnSuspending);
    CoreApplication::Resuming   += ref new EventHandler<Platform::Object^>(this, &ScaleformApp::OnResuming);
}

void ScaleformApp::SetWindow(CoreWindow^ window)
{
	Window = window;
    window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &ScaleformApp::OnSize);
    window->Closed      += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ScaleformApp::OnShutdown);
}

void ScaleformApp::Load(Platform::String^)
{
}

void ScaleformApp::Run()
{
    pApp->AppMain(0, 0);

	while (!ShutdownRequested)
    {
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		pApp->OnUpdateFrame(true);

        HandleGamepadReading();
    }
}

void ScaleformApp::Uninitialize()
{
}

//------------------------------------------------------------------------

void ScaleformApp::HandlePadEvent(unsigned pad, bool down, unsigned idx, PadKeyCode kc)
{
    const int buttons = 16;
    static bool processed[buttons] = { true };

    if (kc != Key::None && pApp)
    {
        if (down && processed[idx])
        {
            if (!pApp->IsCursorEnabled() || pad != 0 || kc != Pad_A)
                pApp->OnPad(pad, kc, true);
            else
                pApp->OnMouseButton(pad, 0, true, pApp->GetMousePos(), 0);

            processed[idx] = false;
        }
        if (!down && !processed[idx])
        {
            if (!pApp->IsCursorEnabled() || pad != 0 || kc != Pad_A)
                pApp->OnPad(pad, kc, false);
            else
                pApp->OnMouseButton(pad, 0, false, pApp->GetMousePos(), 0);

            processed[idx] = true;
        }
    }
}

void ScaleformApp::HandleGamepadReading()
{ 
    Collections::IVectorView<Input::IGamepad^>^ gamepads = Input::Gamepad::Gamepads;
    if (gamepads->Size <= 0)
        return;

    for (unsigned gpidx = 0; gpidx < gamepads->Size; ++gpidx)
    {
        Input::IGamepad^ gp = gamepads->GetAt(gpidx);
        if (gp == nullptr)
            continue;

        Input::IGamepadReading^ reading = gp->GetCurrentReading();

        // Moving the 'mouse', which is always gamepad 0.
        const float RIGHT_THUMB_DEADZONE = 0.05f;
        if (gpidx == 0 && pApp->IsCursorEnabled() && (Alg::Abs(reading->RightThumbstickX) > RIGHT_THUMB_DEADZONE || Alg::Abs(reading->RightThumbstickY) > RIGHT_THUMB_DEADZONE))
        {
            const float mouseMovementScaleFactor = 5.0f;
            Render::Point<float> mouseMovement(reading->RightThumbstickX, -reading->RightThumbstickY);
            Render::Point<int>& mousePosition = pApp->GetAppImpl()->MousePos;

            mouseMovement *= mouseMovementScaleFactor;
            mousePosition.x += mouseMovement.x;
            mousePosition.y += mouseMovement.y;
            Alg::Clamp<int>(mousePosition.x, 0, pApp->GetViewSize().Width);
            Alg::Clamp<int>(mousePosition.y, 0, pApp->GetViewSize().Height);
            pApp->OnMouseMove(gpidx, mousePosition, 0);
        }

        // A, B, X and Y
        HandlePadEvent(gpidx, reading->IsAPressed, 0, Pad_A);
        HandlePadEvent(gpidx, reading->IsBPressed, 1, Pad_B);
        HandlePadEvent(gpidx, reading->IsXPressed, 2, Pad_X);
        HandlePadEvent(gpidx, reading->IsYPressed, 3, Pad_Y);
        // Right and left triggers and shoulders
        HandlePadEvent(gpidx, bool(reading->RightTrigger), 4, Pad_R2);
        HandlePadEvent(gpidx, bool(reading->LeftTrigger), 5, Pad_L2);
        HandlePadEvent(gpidx, reading->IsRightShoulderPressed, 6, Pad_R1);
        HandlePadEvent(gpidx, reading->IsLeftShoulderPressed, 7, Pad_L1);
        // DPad and thumbsticks
        HandlePadEvent(gpidx, reading->IsDPadDownPressed, 8, Pad_Down);
        HandlePadEvent(gpidx, reading->IsDPadLeftPressed, 9, Pad_Left);
        HandlePadEvent(gpidx, reading->IsDPadRightPressed, 10, Pad_Right);
        HandlePadEvent(gpidx, reading->IsDPadUpPressed, 11, Pad_Up);
        HandlePadEvent(gpidx, reading->IsLeftThumbstickPressed , 12, Pad_LT);
        HandlePadEvent(gpidx, reading->IsRightThumbstickPressed , 13, Pad_RT);
        // Menu and view
        HandlePadEvent(gpidx, reading->IsMenuPressed, 14, Pad_Start);
        HandlePadEvent(gpidx, reading->IsViewPressed, 15, Pad_Back);
    }
}

void ScaleformApp::OnSize(CoreWindow^ win, WindowSizeChangedEventArgs^ args)
{
	pApp->OnSize(Render::Size<unsigned>((unsigned)args->Size.Width, (unsigned)args->Size.Height));
}

void ScaleformApp::OnShutdown(CoreWindow^ win, CoreWindowEventArgs^ args)
{
	ShutdownRequested = 1;
	pApp->Shutdown();
}

void ScaleformApp::OnActivated(CoreApplicationView^ view, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void ScaleformApp::OnSuspending(Platform::Object^ app, SuspendingEventArgs^ args)
{
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
    pApp->OnPause();

    // Before suspending the D3D device context, ensure that everything on the rendering thread has been paused.
    Device* pdevice = pApp->GetDevice();
    Render::D3D1x::HAL* phal = reinterpret_cast<Render::D3D1x::HAL*>(pdevice->GetHAL());
    Render::ThreadCommandQueue* pqueue = phal->GetThreadCommandQueue();
    
    // Wait until rendering has completed.
    SuspendResumeCommand = *SF_NEW SuspendRenderThreadCommand(phal->pDeviceContext);
    pqueue->PushThreadCommand(SuspendResumeCommand);
    SuspendResumeCommand->GetRenderingCompleteEvent().Wait();
    
    // No game-state data to save, indicate that the suspend operation is complete.
    deferral->Complete();
    deferral = nullptr;
}
 
void ScaleformApp::OnResuming(Platform::Object^ app, Platform::Object^ args)
{
    // Signal to the SuspendResumeCommand that the render thread can resume.
    SF_DEBUG_ASSERT(SuspendResumeCommand, "Unexpected NULL SuspendResumeCommand in ScaleformApp::OnResuming");
    if (SuspendResumeCommand)
    {
        SuspendResumeCommand->GetResumeCompleteEvent().SetEvent();
        SuspendResumeCommand = 0;
    }
	pApp->OnResume();
}


//------------------------------------------------------------------------

ref class ScaleformAppSource : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
	{
		return ref new ScaleformApp;
	}
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto appsource = ref new ScaleformAppSource();
    CoreApplication::Run(appsource);
    return 0;
}
