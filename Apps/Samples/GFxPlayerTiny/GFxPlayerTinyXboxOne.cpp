/**************************************************************************

Filename    :   GFxPlayerTinyDurango.cpp
Content     :   Tiny GFxPlayer sample for Durango D3D11
Created     :   May, 2012
Authors     :   Vladislav Merker

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_D3D1x.h"

#include "Render/Renderer2D.h"
#include "GFx/AS3/AS3_Global.h"

#include <wrl.h>
#include <agile.h>

// SWF movie file name
#define SWFMOVIE_FILENAME	"flash.swf"

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Xbox;

using namespace Scaleform;
using namespace Scaleform::GFx;
using namespace Scaleform::Render;

using Microsoft::WRL::ComPtr;

//------------------------------------------------------------------------

ref class PlayerTiny sealed : Windows::ApplicationModel::Core::IFrameworkView
{
public:
    PlayerTiny::PlayerTiny() : FileName(SWFMOVIE_FILENAME), Width(1920), Height(1080),
                               Paused(false), Wireframe(false), ShutdownRequested(false),
                               pCommandQueue(0) {}

    virtual void Initialize(CoreApplicationView^ view);
    virtual void SetWindow(CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

protected:
    void    OnActivated(CoreApplicationView^ view, Activation::IActivatedEventArgs^ args);
    void    OnShutdown(CoreWindow^ window, CoreWindowEventArgs^ args);
    void    OnSuspending(Platform::Object^ s, SuspendingEventArgs^ args);
    void    OnResuming(Platform::Object^ s, Platform::Object^ args);

    bool    ConfigureGraphics();
    void    AdvanceAndDisplay();
	void    HandleGamepadReading();

private:
    D3D_FEATURE_LEVEL               FeatureLevel;
	ComPtr<ID3D11Device>            pDevice;
	ComPtr<ID3D11DeviceContext>     pContext;
	ComPtr<IDXGISwapChain1>         pSwapChain;
	ComPtr<ID3D11RenderTargetView>  pRenderTargetView;
	ComPtr<ID3D11Texture2D>         pDepthStencil;
	ComPtr<ID3D11DepthStencilView>  pDepthStencilView;

    Platform::Agile<Windows::UI::Core::CoreWindow> Window;

    Ptr<MovieDef>               pMovieDef;
    Ptr<Movie>                  pMovie;
    MovieDisplayHandle          hMovieDisplay;
    UInt64                      MovieLastTime;
    Ptr<Render::D3D1x::HAL>     pHAL;
    Ptr<Render::Renderer2D>     pRenderer;
	Ptr<Render::RenderTarget>   pRenderTarget;
    ThreadCommandQueue*         pCommandQueue;

    String  FileName;
    int     Width, Height;
    bool    Paused;
    bool    Wireframe;
    bool    ShutdownRequested;

    void    HandleKeyEvent(bool down, unsigned idx, KeyCode kc);
};

//------------------------------------------------------------------------

class PlayerThreadCommandQueue : public ThreadCommandQueue
{
public:
    virtual void GetRenderInterfaces(Render::Interfaces* pint) 
    {
        pint->pHAL = pHAL;
        pint->pRenderer2D = pR2D;
        pint->pTextureManager = pHAL->GetTextureManager();
        pint->RenderThreadID = 0;
    }

    virtual void PushThreadCommand(ThreadCommand* pcmd)
    {
        if (pcmd)
            pcmd->Execute();
    }

    HAL* pHAL;
    Renderer2D* pR2D;
};

class PlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcmd, const char* parg)
    {
        SF_UNUSED3(pmovie, pcmd, parg);
    }
};

class PlayerLog : public GFx::Log
{
public:
    virtual void LogMessageVarg(Scaleform::LogMessageId msgId, const char* pfmt, va_list argList)
    {
        // Use system default output
        DefaultLogMessageVarg(msgId, pfmt, argList);
    }
};

//------------------------------------------------------------------------

void PlayerTiny::Initialize(CoreApplicationView^ view)
{
    view->Activated	+= ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &PlayerTiny::OnActivated);
    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &PlayerTiny::OnSuspending);
    CoreApplication::Resuming   += ref new EventHandler<Platform::Object^>(this, &PlayerTiny::OnResuming);
}

void PlayerTiny::SetWindow(CoreWindow^ window)
{
	Window = window;
    window->Closed  += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &PlayerTiny::OnShutdown);

    // Configure D3D11 device settings
    ConfigureGraphics();
}

void PlayerTiny::Load(Platform::String^ entryPoint)
{
    // Create and configure loader
    Loader loader;
	loader.SetLog(Ptr<GFx::Log>(*new PlayerLog()));
    
    // Verbosity and runtime behavior of ActionScript. Pass "true" to SetVerboseAction() for verbose output
    Ptr<ActionControl> pactControl = *new ActionControl(ActionControl::Action_ErrorSuppress);
    pactControl->SetVerboseAction(false);
    loader.SetActionControl(pactControl);

    // Developers set states on the loader to modify loading and playback behavior
    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener); 
    Ptr<FSCommandHandler> pcommandHandler = *new PlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    // ActionScript support
    Ptr<ASSupport> pASSupport = *new AS3Support();
    loader.SetAS3Support(pASSupport);
	Ptr<ASSupport> pAS2Support = *new AS2Support();
	loader.SetAS2Support(pAS2Support);
    
    // Image file handling registry (PNG, etc).
    Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

    // Load the movie file
    if (!(pMovieDef = *loader.CreateMovie(FileName)))
    {
        if (!(pMovieDef = *loader.CreateMovie(FileName)))
        {
            loader.GetLog()->LogError("Unable to load file: %s\n", FileName);
            return;
        }
    }

    // RenderThread command queue interface
    PlayerThreadCommandQueue* queue = new PlayerThreadCommandQueue;
    pCommandQueue = queue;

    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return;
	hMovieDisplay = pMovie->GetDisplayHandle();

    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // Create renderer
    pHAL = *new Render::D3D1x::HAL(pCommandQueue);
    if (!(pRenderer = *new Render::Renderer2D(pHAL.GetPtr())))
        return;
    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally configured device settings
    if (!pHAL->InitHAL(Render::D3D1x::HALInitParams(pDevice.Get(), pContext.Get(), 0, Scaleform::GetCurrentThreadId())))
        return;
    if (pHAL->IsInitialized())
		pRenderTarget = pHAL->CreateRenderTarget(pRenderTargetView.Get(), pDepthStencilView.Get());

    MovieLastTime = Timer::GetTicks() / 1000;
}

void PlayerTiny::Run()
{
    while (!ShutdownRequested)
    {
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

        // Advance movie animation and render it to screen
		AdvanceAndDisplay();
        // Handle events from Gamepad
        HandleGamepadReading();
    }
}

void PlayerTiny::Uninitialize()
{
    if (pHAL)
        pHAL->ShutdownHAL();

    delete pCommandQueue;
}

//------------------------------------------------------------------------

bool PlayerTiny::ConfigureGraphics()
{
	unsigned flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, 2, D3D11_SDK_VERSION, 
								 &pDevice, &FeatureLevel, &pContext)))
		return false;

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
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    ComPtr<IDXGIDevice1> pdxgiDevice;
    ComPtr<IDXGIAdapter> pdxgiAdapter;
    ComPtr<IDXGIFactory2> pdxgiFactory;
    if (FAILED(pDevice.As(&pdxgiDevice)))
        return false;
    if (FAILED(pdxgiDevice->GetAdapter(&pdxgiAdapter)))
        return false;
    if (FAILED(pdxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &pdxgiFactory)))
        return false;

    Windows::UI::Core::CoreWindow^ window = Window.Get();
    if (FAILED(pdxgiFactory->CreateSwapChainForCoreWindow(
        pDevice.Get(), reinterpret_cast<IUnknown*>(window), &swapChainDesc, nullptr, &pSwapChain)))
        return false;

    ComPtr<ID3D11Texture2D> pbackBuffer;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pbackBuffer)))
        return false;

    D3D11_TEXTURE2D_DESC backBufferDesc;
    pbackBuffer->GetDesc(&backBufferDesc);
	if (FAILED(pDevice->CreateRenderTargetView(pbackBuffer.Get(), 0, &pRenderTargetView)))
		return false;

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
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

    ID3D11RenderTargetView* prtView = pRenderTargetView.Get();
    pContext->OMSetRenderTargets(1, &prtView, pDepthStencilView.Get());

    return true;
}

//------------------------------------------------------------------------

void PlayerTiny::AdvanceAndDisplay()
{
    // Clear the frame buffer to black
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pContext->ClearRenderTargetView(pRenderTargetView.Get(), clearColor);

    if (pMovie)
    {
        // Set wireframe (based on the current setting)
        pHAL->SetRasterMode(Wireframe ? Render::D3D1x::HAL::RasterMode_Wireframe :
                                        Render::D3D1x::HAL::RasterMode_Default);

        // Calculate the time update time for this frame
        UInt64 time = Timer::GetTicks() / 1000;
        float delta = ((float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie
        if (!Paused)
            pMovie->Advance(delta);

        pRenderer->BeginFrame();
        if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
            pRenderer->Display(hMovieDisplay);
        pRenderer->EndFrame();

        MovieLastTime = time;
    }

#ifdef SF_DURANGO_MONOLITHIC
    if (pSwapChain)
    {
        IDXGISwapChain1* swapChains[1];
        swapChains[0] = pSwapChain.Get();

        DXGIX_PRESENTARRAY_PARAMETERS presentParams[1];
        presentParams[0].Disable = FALSE;
        presentParams[0].UsePreviousBuffer = FALSE;
        presentParams[0].SourceRect.left = 0;
        presentParams[0].SourceRect.top = 0;
        presentParams[0].SourceRect.right = Width;
        presentParams[0].SourceRect.bottom = Height;
        presentParams[0].DestRectUpperLeft.x = 0;
        presentParams[0].DestRectUpperLeft.y = 0;
        presentParams[0].ScaleFactorVert = 1.0f;
        presentParams[0].ScaleFactorHorz = 1.0f;

        DXGIXPresentArray(1, 0, 0, 1, swapChains, presentParams);
    }
#else
    if (pSwapChain)
        pSwapChain->Present(0, 0);
#endif
}

//------------------------------------------------------------------------

void PlayerTiny::OnActivated(CoreApplicationView^ view, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void PlayerTiny::OnShutdown(CoreWindow^ window, CoreWindowEventArgs^ args)
{
    ShutdownRequested = true;
}

void PlayerTiny::OnSuspending(Platform::Object^ s, SuspendingEventArgs^ args)
{
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

    // Suspend the D3D device (required). Since we are rendering on the main thread, no extra synchronization logic is required.
#ifdef SF_DURANGO_MONOLITHIC
    ComPtr<ID3D11DeviceContextX> context;
#else
    ComPtr<ID3DXboxPerformanceContext>  context;
#endif
    pContext.As(&context);
    context->Suspend(0);
    
    // No game-state data to save, indicate that the suspend operation is complete.
    deferral->Complete();
    deferral = nullptr;
}

void PlayerTiny::OnResuming(Platform::Object^ s, Platform::Object^ args)
{
#ifdef SF_DURANGO_MONOLITHIC
    ComPtr<ID3D11DeviceContextX> context;
#else
    ComPtr<ID3DXboxPerformanceContext>  context;
#endif
    pContext.As(&context);
    context->Resume();
}

//------------------------------------------------------------------------

void PlayerTiny::HandleKeyEvent(bool down, unsigned idx, KeyCode kc)
{
    const int buttons = 16;
    static bool processed[buttons] = { true };

    if (kc != Key::None && pMovie)
    {
        if (down && processed[idx])
        {
            KeyEvent event(GFx::Event::KeyDown, kc);
            pMovie->HandleEvent(event);
            processed[idx] = false;
        }
        if (!down && !processed[idx])
        {
            KeyEvent event(KeyEvent::KeyUp, kc);
            pMovie->HandleEvent(event);
            processed[idx] = true;
        }
    }
}

void PlayerTiny::HandleGamepadReading()
{ 
    Collections::IVectorView<Input::IGamepad^>^ gamepads = Input::Gamepad::Gamepads;
    if (gamepads->Size <= 0)
        return;

    Input::IGamepadReading^ reading = gamepads->GetAt(0)->GetCurrentReading();
    // A and DPad
    HandleKeyEvent(reading->IsAPressed, 0, Key::Return);
    HandleKeyEvent(reading->IsDPadDownPressed, 1, Key::Down);
    HandleKeyEvent(reading->IsDPadLeftPressed, 2, Key::Left);
    HandleKeyEvent(reading->IsDPadRightPressed, 3, Key::Right);
    HandleKeyEvent(reading->IsDPadUpPressed, 4, Key::Up);

    // X: toggle pause
    static bool pauseProcessed = true;
    if (reading->IsXPressed && pauseProcessed)
    {
        Paused = !Paused;
        pauseProcessed = false;
    }
    if (!reading->IsXPressed && !pauseProcessed)
        pauseProcessed = true;

    // Y: toggle wireframe mode
    static bool wireframeProcessed = true;
    if (reading->IsYPressed && wireframeProcessed)
    {
        Wireframe = !Wireframe;
        wireframeProcessed = false;
    }
    if (!reading->IsYPressed && !wireframeProcessed)
        wireframeProcessed = true;
}

//------------------------------------------------------------------------

ref class ScaleformAppSource : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
	{
		return ref new PlayerTiny;
	}
};

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ params)
{
	SysAllocMalloc sysAlloc;
	GFx::System gfxInit(&sysAlloc);

    auto appSource = ref new ScaleformAppSource();
    CoreApplication::Run(appSource);
    return 0;
}
