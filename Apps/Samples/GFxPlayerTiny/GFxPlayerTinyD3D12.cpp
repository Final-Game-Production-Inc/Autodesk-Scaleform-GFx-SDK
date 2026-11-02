/**************************************************************************

Filename    :   GFxPlayerTinyD3D12.cpp
Content     :   Tiny GFxPlayer sample for Direct3D 12.
Created     :   2026
Authors     :   Final Game Production Inc.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_D3D12.h"

#ifdef GFX_ENABLE_SOUND
#include "../Common/FxSoundFMOD.h"
#endif

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"

// System Includes
#include <windows.h>
#include <winuser.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
#endif

#include "GFx_FontProvider_Win32.h"

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   "Window.swf"

#include "Render/Renderer2D.h"

#include "GFx/AS3/AS3_Global.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

// Number of swap chain back buffers (flip model).
#define FXPLAYER_BUFFER_COUNT 2

// ***** FxPlayerTiny Application Class

// FxPlayerTiny class encapsulates all of the system setup and GFxPlayer
// program state needed for playback. FxPlayerTiny is created in main() and
// contains most of the program logic in its Run function.

class FxPlayerTiny
{
    // Win32 window state variables.
    const char*             pWndClassName;
    HWND                    hWnd;
    HINSTANCE               hInstance;

    // Direct3D 12 state variables.
    ID3D12Device*           pDevice;          // Our rendering device.
    ID3D12CommandQueue*     pCommandQueue;    // Direct command queue.
    ID3D12CommandAllocator* pCommandAllocator;
    ID3D12GraphicsCommandList* pCommandList;
    IDXGIFactory2*          pDxgi;
    IDXGISwapChain3*        pSwapChain;
    ID3D12DescriptorHeap*   pRTVHeap;         // CPU-only RTV heap for back buffers.
    UINT                    RtvDescriptorSize;
    ID3D12Resource*         pBackBuffers[FXPLAYER_BUFFER_COUNT];
    ID3D12Fence*            pFrameFence;      // App-level frame fence.
    UINT64                  FrameFenceValue;
    HANDLE                  FrameFenceEvent;

    // Scaleform GFx object pointers.
    Ptr<MovieDef>           pMovieDef;
    Ptr<Movie>              pMovie;
    Ptr<Render::D3D12::HAL> pRenderHAL;
    Ptr<Render::Renderer2D> pRenderer;
    ThreadCommandQueue*     pThreadCommandQueue;
    MovieDisplayHandle      hMovieDisplay;

    UInt32                  MovieStartTime;
    UInt32                  MovieLastTime;
    float                   ExitTimeout;

    // Toggle states.
    bool                    Wireframe;
    bool                    ControlKeyDown;
    bool                    AAEnabled;
    bool                    Paused;

    // Playback filename and window size.
    String                  FileName;
    int                     Width;
    int                     Height;

    bool                    VerboseAction;

#ifdef GFX_ENABLE_SOUND
    FxSoundFMOD*            pSoundFMODSystem;
#endif

public:

    FxPlayerTiny(int argc, char *argv[]);
    ~FxPlayerTiny();

    // Initialize the player and run playback loop.
    int     Run();
    // Advance playback and render a frame.
    void    AdvanceAndDisplay();
    void    Display();

    // Input handler functions. Process mouse and keyboard and pass
    // appropriate Event notifications to Movie::HandleEvent.
    void    OnKey(unsigned keyCode, bool downFlag);
    void    OnMouse(GFx::Event::EventType eventType, unsigned button, int x, int y);
    void    OnMouseWheel(int zdelta, int x, int y);
    void    HandleKeyEvent(unsigned keyCode, bool downFlag);

    // System window and rendering setup.
    bool    SetupWindow(const String& name);
    bool    CreateDeviceAndSwapChain();
    bool    CreateBuffers();

    // Windows message handler functions.
    LRESULT MemberWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);

    // sound-related methods
#ifdef GFX_ENABLE_SOUND
    virtual void            InitializeSound();
    virtual void            ShutdownSound() { xShutdownSound(); }
    void                    xShutdownSound(); // avoid call to a virtual method from the destructor
    virtual Sound::SoundRenderer* GetSoundRenderer();
#endif
};

// A sample implementation of FSCommandHandler. Users can install such a
// handler to receive notifications from fscommand() calls in ActionScript.
class FxPlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg)
    {
        SF_UNUSED3(pmovie, pcommand, parg);
    }
};

// Operates as a single-threaded queue, so things are just executed immediately.
class FxPlayerThreadCommandQueue : public ThreadCommandQueue
{
public:

    virtual void GetRenderInterfaces( Render::Interfaces* p )
    {
        p->pHAL = pHAL;
        p->pRenderer2D = pR2D;
        p->pTextureManager = pHAL->GetTextureManager();
        p->RenderThreadID = 0;
    }

    virtual void PushThreadCommand( ThreadCommand* command )
    {
        if (command)
            command->Execute();
    }

    HAL* pHAL;
    Renderer2D* pR2D;
};

class GFxPlayerLog : public GFx::Log
{
public:
    virtual void    LogMessageVarg(SF::LogMessageId messageId, const char* pfmt, va_list argList)
    {
        SF_UNUSED(messageId);
        vprintf(pfmt, argList);
    }
};

// ***** FxPlayerTiny Class Implementation

FxPlayerTiny::FxPlayerTiny(int argc, char *argv[])
{
    pDevice          = 0;
    pCommandQueue    = 0;
    pCommandAllocator= 0;
    pCommandList     = 0;
    pDxgi            = 0;
    pSwapChain       = 0;
    pRTVHeap         = 0;
    RtvDescriptorSize= 0;
    pFrameFence      = 0;
    FrameFenceValue  = 0;
    FrameFenceEvent  = 0;
    for (int i = 0; i < FXPLAYER_BUFFER_COUNT; ++i)
        pBackBuffers[i] = 0;

    pWndClassName   = "Direct3D12_Window_Class";
    hWnd            = 0;
    hInstance       = 0;

    Wireframe       = 0;
    ControlKeyDown  = 0;
    AAEnabled       = 1;
    Paused          = 0;

    // Window size and filename.
    Width           = 800;
    Height          = 600;
    FileName        = FXPLAYER_FILENAME;
    VerboseAction   = false;
    ExitTimeout     = 0;

#ifdef GFX_ENABLE_SOUND
    pSoundFMODSystem  = NULL;
#endif

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            const char* curArg = argv[i] + 1;
            if (_stricmp(curArg, "test") == 0)
            {
                ExitTimeout = 5.;
            }
        }
        else
            FileName = argv[i];
    }
}

FxPlayerTiny::~FxPlayerTiny()
{
    if (pDevice)
    {
        // Make sure all queued GPU work has completed before releasing objects.
        if (pCommandQueue && pFrameFence)
        {
            UINT64 fenceValue = ++FrameFenceValue;
            pCommandQueue->Signal(pFrameFence, fenceValue);
            if (pFrameFence->GetCompletedValue() < fenceValue)
            {
                pFrameFence->SetEventOnCompletion(fenceValue, FrameFenceEvent);
                WaitForSingleObject(FrameFenceEvent, INFINITE);
            }
        }
    }

    for (int i = 0; i < FXPLAYER_BUFFER_COUNT; ++i)
    {
        if (pBackBuffers[i])
            pBackBuffers[i]->Release();
    }
    if (pRTVHeap)      pRTVHeap->Release();
    if (pSwapChain)    pSwapChain->Release();
    if (pCommandList)  pCommandList->Release();
    if (pCommandAllocator) pCommandAllocator->Release();
    if (pCommandQueue) pCommandQueue->Release();
    if (pFrameFence)   pFrameFence->Release();
    if (FrameFenceEvent) CloseHandle(FrameFenceEvent);
    if (pDxgi)         pDxgi->Release();
    if (pDevice)       pDevice->Release();

    delete pThreadCommandQueue;
    UnregisterClass(pWndClassName, hInstance);
#ifdef GFX_ENABLE_SOUND
    xShutdownSound();
#endif
}

bool FxPlayerTiny::CreateBuffers()
{
    // Create the CPU-only RTV heap and a render target view per back buffer.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = FXPLAYER_BUFFER_COUNT;
    rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pRTVHeap))))
        return 0;

    RtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FXPLAYER_BUFFER_COUNT; ++i)
    {
        if (FAILED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffers[i]))))
            return 0;
        pDevice->CreateRenderTargetView(pBackBuffers[i], NULL, rtvHandle);
        rtvHandle.ptr += RtvDescriptorSize;
    }

    return 1;
}

// Called from main() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
int FxPlayerTiny::Run()
{
    if (!SetupWindow(String("Scaleform GFxPlayerTinyD3D12") + " v" GFX_VERSION_STRING + " " + FileName))
        return 1;

    // Show the window.
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    if (!CreateDeviceAndSwapChain())
        return 1;
    if (!CreateBuffers())
        return 1;

    // Create and Configure Loader.
    Loader loader;
    loader.SetLog(Ptr<GFx::Log>(*new GFxPlayerLog()));

    Ptr<ActionControl> pactControl = *new ActionControl(ActionControl::Action_ErrorSuppress);
    pactControl->SetVerboseAction(VerboseAction);
    loader.SetActionControl(pactControl);

    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener);
    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    Ptr<FontProviderWin32> fontProvider = *new FontProviderWin32(::GetDC(0));
    loader.SetFontProvider(fontProvider);

#ifdef GFX_ENABLE_SOUND
    //Sound properties
    float SoundVolume = 1;
    bool  IsMute      = false;
    InitializeSound();
    Sound::SoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        Ptr<Audio> paudioState = *new Audio(psoundRenderer);
        loader.SetAudio(paudioState);
        psoundRenderer->SetMasterVolume(SoundVolume);
        psoundRenderer->Mute(IsMute);
    }
#endif // GFX_ENABLE_SOUND

    // add support for various image handlers
    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

    Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
    loader.SetAS3Support(pASSupport);
    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);

    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pThreadCommandQueue = queue;

    // Load the movie file and create its instance.
    if (!(pMovieDef = *loader.CreateMovie(FileName)))
    {
        if (!(pMovieDef = *loader.CreateMovie("AS2\\Samples\\" + FileName)))
        {
            String errorString = "Unable to load file: ";
            errorString += FileName;
            MessageBox(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
            return 1;
        }
    }

    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pThreadCommandQueue)))
        return 1;

    pMovie->SetMouseCursorCount(1);
    hMovieDisplay = pMovie->GetDisplayHandle();

    // Create renderer.
    pRenderHAL = *new Render::D3D12::HAL(pThreadCommandQueue);
    if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
        return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Initialize the HAL with the device objects created above. The platform
    // (this sample) keeps ownership of the swap chain and its back buffers;
    // the HAL renders into them through SetMainRenderTarget.
    Render::D3D12::HALInitParams halParams(pDevice, pCommandQueue, pCommandAllocator,
                                           pCommandList, NULL, 0, Scaleform::GetCurrentThreadId());
    if (!pRenderHAL->InitHAL(halParams))
        return 1;

    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieStartTime = MovieLastTime = timeGetTime();

    // Application / Player message loop.
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while(msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Advance movie animation and render it to screen.
            AdvanceAndDisplay();
            if ((ExitTimeout > 0) && (MovieLastTime - MovieStartTime > (UInt32) (ExitTimeout * 1000)))
                break;
        }
    }
    return 0;
}

#ifdef GFX_ENABLE_SOUND
void FxPlayerTiny::InitializeSound()
{
    if (!pSoundFMODSystem)
        pSoundFMODSystem = new FxSoundFMOD;
    if (!pSoundFMODSystem->IsInitialized())
    {
        bool bIsInitFMOD = pSoundFMODSystem->Initialize();
        SF_ASSERT(bIsInitFMOD);
        SF_UNUSED(bIsInitFMOD);
    }
}

void FxPlayerTiny::xShutdownSound()
{
    if (pSoundFMODSystem && pSoundFMODSystem->IsInitialized())
        pSoundFMODSystem->Finalize();
    delete pSoundFMODSystem;
    pSoundFMODSystem = NULL;
}

Sound::SoundRenderer* FxPlayerTiny::GetSoundRenderer()
{
    return pSoundFMODSystem && pSoundFMODSystem->IsInitialized() ?
        pSoundFMODSystem->GetSoundRenderer() :
        NULL;
}
#endif // GFX_ENABLE_SOUND

// Advances GFx animation and draws the scene.
void FxPlayerTiny::AdvanceAndDisplay()
{
    // Wait for the previous frame to complete before resetting the command
    // allocator (its memory may still be in use by the GPU).
    if (pFrameFence->GetCompletedValue() < FrameFenceValue)
    {
        pFrameFence->SetEventOnCompletion(FrameFenceValue, FrameFenceEvent);
        WaitForSingleObject(FrameFenceEvent, INFINITE);
    }

    // Reset the allocator and command list for this frame.
    pCommandAllocator->Reset();
    pCommandList->Reset(pCommandAllocator, NULL);

    // The swap chain's current back buffer becomes the render target.
    UINT bufferIndex = pSwapChain->GetCurrentBackBufferIndex();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += (SIZE_T)bufferIndex * RtvDescriptorSize;

    pRenderHAL->SetCommandList(pCommandList);
    pRenderHAL->SetMainRenderTarget(pBackBuffers[bufferIndex], rtvHandle,
                                    DXGI_FORMAT_B8G8R8A8_UNORM,
                                    ImageSize(Width, Height));

    // *** GFx Rendering
    if (pMovie)
    {
        UInt32  time    = timeGetTime();
        float   delta   = ((float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

        Display();

        MovieLastTime = time;
    }

    // Close the command list and submit it to the GPU.
    pCommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { pCommandList };
    pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Signal the frame fence AFTER the submitted work so the next frame can
    // safely reuse the command allocator.
    ++FrameFenceValue;
    pCommandQueue->Signal(pFrameFence, FrameFenceValue);

    // Present the back buffer contents to the display.
    if (pSwapChain)
        pSwapChain->Present(1, 0);

    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());
}

void FxPlayerTiny::Display()
{
    pRenderer->BeginFrame();
    if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
    {
        pRenderer->Display(hMovieDisplay);
    }
    pRenderer->EndFrame();
}

// Helper function that converts Windows VK keyCode values
// to GFxKeyEvents and routes them to GFxPlayer.
void    FxPlayerTiny::HandleKeyEvent(unsigned keyCode, bool downFlag)
{
    Key::Code key(Key::None);

    if (keyCode >= 'A' && keyCode <= 'Z')
    {
        key = (Key::Code) ((keyCode - 'A') + Key::A);
    }
    else if (keyCode >= VK_F1 && keyCode <= VK_F15)
    {
        key = (Key::Code) ((keyCode - VK_F1) + Key::F1);
    }
    else if (keyCode >= '0' && keyCode <= '9')
    {
        key = (Key::Code) ((keyCode - '0') + 48);
    }
    else if (keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
    {
        key = (Key::Code) ((keyCode - VK_NUMPAD0) + Key::KP_0);
    }
    else
    {
        // Use a look-up table for keys don't correlate in order.
        static struct {
            int          vk;
            Key::Code gs;
            } table[] =
        {
            { VK_SHIFT,     Key::Shift },
            { VK_RSHIFT,    Key::Shift },
            { VK_CONTROL,   Key::Control },
            { VK_RCONTROL,  Key::Control },
            { VK_MENU,      Key::Alt },
            { VK_RMENU,     Key::Alt },
            { VK_RETURN,    Key::Return },
            { VK_ESCAPE,    Key::Escape },
            { VK_LEFT,      Key::Left },
            { VK_UP,        Key::Up },
            { VK_RIGHT,     Key::Right },
            { VK_DOWN,      Key::Down },
            { VK_SPACE,     Key::Space },
            { VK_BACK,      Key::Backspace },
            { VK_DELETE,    Key::Delete },
            { VK_INSERT,    Key::Insert },
            { 0, Key::None }
        };

        for (int i = 0; table[i].vk != 0; i++)
        {
            if (keyCode == (unsigned)table[i].vk)
            {
                key = table[i].gs;
                break;
            }
        }
    }

    if (key != Key::None)
    {
        if (pMovie)
        {
            KeyEvent event(downFlag ? GFx::Event::KeyDown : KeyEvent::KeyUp, key);
            pMovie->HandleEvent(event);
        }
    }
}

// OnKey handles system keys by first interpreting some Ctrl+Key combinations
// in a special way and passing the rest of them to the movie.
void    FxPlayerTiny::OnKey(unsigned keyCode, bool downFlag)
{
    if (!pMovie)
        return;

    bool    ctrl = ControlKeyDown;

    if (keyCode == VK_CONTROL)
    {
        ControlKeyDown = downFlag;
        return;
    }

    if (keyCode == VK_MENU && downFlag)
        ControlKeyDown = false; // to enable Ctrl-Alt-... combinations to work

    if (ctrl && downFlag)
    {
        switch(keyCode)
        {
        case 'W':
            Wireframe = !Wireframe;
            break;

        case 'P':
            Paused = !Paused;
            break;
        }
    }

    else if (!ctrl)
    {
        HandleKeyEvent(keyCode, downFlag);
    }
}

void    FxPlayerTiny::OnMouse(GFx::Event::EventType eventType, unsigned button, int x, int y)
{
    if (pMovie)
    {
        MouseEvent mevent(eventType, button, (float)x, (float)y);
        pMovie->HandleEvent(mevent);
    }
}

void FxPlayerTiny::OnMouseWheel(int zdelta, int x, int y)
{
    if (pMovie)
    {
        MouseEvent mevent(GFx::Event::MouseWheel, 0, (float)x, (float)y, (float)((zdelta/WHEEL_DELTA)*3));
        pMovie->HandleEvent(mevent);
    }
}

// Setup Application Window and initialize Direct3D 12.
bool FxPlayerTiny::SetupWindow(const String& name)
{
    hInstance = GetModuleHandle(NULL);
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, StaticWindowProc,
                      0L, sizeof(FxPlayerTiny*),
                      hInstance, NULL, NULL, NULL, NULL,
                      pWndClassName, NULL };
    RegisterClassEx(&wc);

    // Create the application's window.
    RECT r = { 100,100, 100 + Width, 100 + Height };
    ::AdjustWindowRect(&r, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0);

    HWND hWnd_C = CreateWindow(pWndClassName, name.ToCStr(),
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
        r.left, r.top, r.right-r.left, r.bottom - r.top,
        GetDesktopWindow(), NULL, wc.hInstance, (LPVOID) this );

    return hWnd_C != NULL;
}

bool FxPlayerTiny::CreateDeviceAndSwapChain()
{
    // Create the D3D12 device (feature level 11.0 is required for D3D12).
    HRESULT hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));
    if (FAILED(hr))
        return 0;

    // Create the direct command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue))))
        return 0;

    // Command allocator and (initially closed) command list.
    if (FAILED(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator))))
        return 0;
    if (FAILED(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator,
                                          NULL, IID_PPV_ARGS(&pCommandList))))
        return 0;
    pCommandList->Close();

    // Frame fence + event used to synchronize CPU/GPU.
    if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFrameFence))))
        return 0;
    FrameFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!FrameFenceEvent)
        return 0;

    // Create the DXGI factory and flip-model swap chain.
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgi))))
        return 0;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width              = Width;
    swapChainDesc.Height             = Height;
    swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo             = FALSE;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = FXPLAYER_BUFFER_COUNT;
    swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;

    IDXGISwapChain1* pSwapChain1 = NULL;
    hr = pDxgi->CreateSwapChainForHwnd(pCommandQueue, hWnd, &swapChainDesc,
                                       NULL, NULL, &pSwapChain1);
    if (FAILED(hr))
        return 0;
    if (FAILED(pSwapChain1->QueryInterface(IID_PPV_ARGS(&pSwapChain))))
    {
        pSwapChain1->Release();
        return 0;
    }
    pSwapChain1->Release();

    return 1;
}

// Window Message handler. Interprets windows messages and dispatches
// mouse/keyboard input to the OnMouse/OnKeyboard functions.
LRESULT FxPlayerTiny::MemberWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SETCURSOR:
            if (LOWORD(lParam)==HTCLIENT)
            {
                ::SetCursor(::LoadCursor(0, IDC_ARROW));
                return 0;
            }
            break;

        case WM_MOUSEMOVE:
            OnMouse(GFx::Event::MouseMove, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            {
                POINT   wcl = {0,0};
                ::ClientToScreen(hWnd, &wcl);
                int x = int(SInt16(LOWORD(lParam))) - wcl.x;
                int y = int(SInt16(HIWORD(lParam))) - wcl.y;
                OnMouseWheel((int(SInt16(HIWORD(wParam)))*128)/WHEEL_DELTA, x, y);
                return 0;
            }
        case WM_LBUTTONDOWN:
            ::SetCapture(hWnd);
            OnMouse(GFx::Event::MouseDown, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_LBUTTONUP:
            ::ReleaseCapture();
            OnMouse(GFx::Event::MouseUp, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            OnKey((unsigned)wParam, 1);
            return 0;
        case WM_KEYUP:
            OnKey((unsigned)wParam, 0);
            return 0;
        case WM_CHAR:
            {
                UInt32 wcharCode = (UInt32)wParam;
                if (pMovie && wcharCode)
                {
                    CharEvent event(wcharCode);
                    pMovie->HandleEvent(event);
                }
            }
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Static window function called by Windows for message processing. Implementation
// delegates message processing to MemberWndProc.
LRESULT CALLBACK FxPlayerTiny::StaticWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    FxPlayerTiny *papp;

    // The first message to ever come in sets the long value to class pointer.
    if (iMsg == WM_NCCREATE)
    {
        papp = (FxPlayerTiny*) ((LPCREATESTRUCT)lParam)->lpCreateParams;
        if (!papp)
            return DefWindowProcW(hwnd,iMsg,wParam,lParam);

#ifdef SF_64BIT_POINTERS
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)papp);
#else
        SetWindowLong(hwnd, 0, (LONG)(size_t)papp);
#endif
        papp->hWnd = hwnd;
    }

    if ( (papp=((FxPlayerTiny*)(size_t)GetWindowLongPtr(hwnd,0))) == 0 )
        return DefWindowProcW(hwnd,iMsg,wParam,lParam);

    return papp->MemberWndProc(iMsg, wParam, lParam);
}

// ***** main() -  Application entry point.

// The main function implementation creates the FxPlayerTiny class,
// passes it the filename and calls Run. FxPlayerTiny::Run is responsible
// for all of the rendering setup and movie playback.

int SF_CDECL main(int argc, char *argv[])
{
    #if 1
        // New malloc-friendly "Magic Header" allocator.
        SF::SysAllocMalloc a;
        SF::GFx::System gfxInit(&a);
    #else
        SF::GFx::System gfxInit;
    #endif

    int result = 1;
    {
        FxPlayerTiny player(argc, argv);
        result = player.Run();
    }

    return result;
}
