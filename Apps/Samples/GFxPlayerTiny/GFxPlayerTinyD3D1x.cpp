/**************************************************************************

Filename    :   GFxPlayerTinyD3D1x.cpp
Content     :   Tiny GFxPlayer sample for Direct3D 10/11.
Created     :   March 13, 2008
Authors     :   Michael Antonov, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_D3D1x.h"

#ifdef GFX_ENABLE_SOUND
#include "../Common/FxSoundFMOD.h"
#endif

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"

// System Includes
#include <windows.h>
// Mousewheel support
#include    <winuser.h>
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
#endif

#if SF_D3D_VERSION == 10
#include <d3d10.h>
#define FXPLAYER_APP_TITLE  "Scaleform GFxPlayerTinyD3D10" " v" GFX_VERSION_STRING

#elif SF_D3D_VERSION == 11
#include <d3d11.h>
#define FXPLAYER_APP_TITLE  "Scaleform GFxPlayerTinyD3D11" " v" GFX_VERSION_STRING

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

// ***** FxPlayerTiny Application Class

// FxPlayerTiny class encapsulates all of the system setup and GFxPlayer
// program state needed for playback. FxPlayerTiny is created in main() /
// WinMain() and contains most of the program logic in its Run function.

class FxPlayerTiny
{
    // Win32 window state variables.
    const char*             pWndClassName;
    HWND                    hWnd;
    HINSTANCE               hInstance;

    // Direct3D state variables.

    ID3D1x(Device)*           pDevice;          // Our rendering device.
    ID3D1x(DeviceContext)*    pDeviceContext;   // Device context, in D3D10, this is just a reference to the device itself.
    IDXGIFactory*             pDxgi;
    IDXGISwapChain*           pSwapChain;
    ID3D1x(RenderTargetView)* pRenderTarget;
    ID3D1x(DepthStencilView)* pDepthStencil;
    ID3D1x(Texture2D)*        pDepthStencilBuf;
    DXGI_SWAP_CHAIN_DESC      SwapChainDesc;

    // Scaleform GFx object pointers.
    Ptr<MovieDef>           pMovieDef;
    Ptr<Movie>              pMovie;
	Ptr<Render::D3D1x::HAL> pRenderHAL; 
	Ptr<Render::Renderer2D> pRenderer;
    ThreadCommandQueue*     pCommandQueue;
	// Context to draw

	MovieDisplayHandle        hMovieDisplay;

    //Ptr<RenderConfig>       pRenderConfig;
    UInt32                  MovieStartTime;
    UInt32                  MovieLastTime;
    float                   ExitTimeout;


    // Toggle states.
    bool                    Wireframe; 
    bool                    ControlKeyDown;
    bool                    AAEnabled;
    bool                    Paused;         // Set when the movie is paused in the player.    

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
    bool    CreateBuffers();

    // Windows message handler functions. StaticWindowProc is called by the
    // system and delegates most of its processing to MemberWndProc.
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
// Note that in many cases it is better to use GFxExternalInterface instead
// since it allows for better argument passing through ExternalInterface.call.
class FxPlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg)
    {
        // Do not log anything since we don't have a console.
        // printf("Command: %s, Args: %s", pcommand, parg);
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
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(SF::LogMessageId messageId, const char* pfmt, va_list argList)
    {
        SF_UNUSED(messageId);
        // Output log to console
        vprintf(pfmt, argList);
    }
};


// ***** FxPlayerTiny Class Implementation

FxPlayerTiny::FxPlayerTiny(int argc, char *argv[])
{
    pDevice          = 0;
    pRenderTarget    = 0;
    pDevice          = 0;
    pDeviceContext   = 0;
    pSwapChain       = 0;
    pDepthStencil    = 0;
    pDepthStencilBuf = 0;

    pWndClassName   = "Direct3D_Window_Class";
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
    if (pDeviceContext)
        pDeviceContext->Release();

    if (pDevice)
        pDevice->Release();
    delete pCommandQueue;
    UnregisterClass(pWndClassName, hInstance);
#ifdef GFX_ENABLE_SOUND
    xShutdownSound();
#endif
}

bool FxPlayerTiny::CreateBuffers()
{
    ID3D1x(Texture2D)* pBackBuffer;
    HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D1x(Texture2D)), (void**)&pBackBuffer);
    if( FAILED(hr) )
        return 0;

    hr = pDevice->CreateRenderTargetView(pBackBuffer, 0, &pRenderTarget);
    pBackBuffer->Release();
    if( FAILED(hr) )
        return 0;

    D3D1x(TEXTURE2D_DESC) dsd;
    dsd.Width = Width;
    dsd.Height = Height;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = SwapChainDesc.SampleDesc.Count;
    dsd.SampleDesc.Quality = SwapChainDesc.SampleDesc.Quality;
    dsd.Usage = D3D1x(USAGE_DEFAULT);
    dsd.BindFlags = D3D1x(BIND_DEPTH_STENCIL);
    dsd.CPUAccessFlags = 0;
    dsd.MiscFlags = 0;
    hr = pDevice->CreateTexture2D(&dsd, NULL, &pDepthStencilBuf);
    if( FAILED(hr) )
        return 0;

    hr = pDevice->CreateDepthStencilView( pDepthStencilBuf, 0, &pDepthStencil);
    if( FAILED(hr) )
        return 0;

    pDeviceContext->ClearDepthStencilView(pDepthStencil, D3D1x(CLEAR_DEPTH) | D3D1x(CLEAR_STENCIL), 0, 0);
    pDeviceContext->OMSetRenderTargets(1, &pRenderTarget, pDepthStencil);

    // Setup the viewport
    D3D1x(VIEWPORT) vp;
    vp.Width = D3D11((FLOAT))Width;
    vp.Height = D3D11((FLOAT))Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pDeviceContext->RSSetViewports(1, &vp);

    return 1;
}

// Called from main()/WindMain() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
int FxPlayerTiny::Run()
{    
    if (!SetupWindow(String(FXPLAYER_APP_TITLE " ") + FileName))
        return 1;

    // Show the window.
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

   
    // Create and Configure Loader.   
    Loader loader;

	loader.SetLog(Ptr<GFx::Log>(*new GFxPlayerLog()));
    
  //  Ptr<ParseControl> pparseControl = *new ParseControl(ParseControl::VerboseParse);
  //  loader.SetParseControl(pparseControl);

    Ptr<ActionControl> pactControl = *new ActionControl(ActionControl::Action_ErrorSuppress);
//      Ptr<ActionControl> pactControl = *new ActionControl(ActionControl::Action_Verbose);
     pactControl->SetVerboseAction(VerboseAction);
     loader.SetActionControl(pactControl);

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FileOpener derived class that loads files in a custom way.
    // Here FileOpener and FSCommandHandler are set for sample purposes.    
    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener); 
    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    Ptr<FontProviderWin32> fontProvider = *new FontProviderWin32(::GetDC(0));
    loader.SetFontProvider(fontProvider);

    // For D3D, it is good to override image creator to keep image data,
    // so that it can be restored in case of a lost device.
	//Ptr<GFx::ImageCreator> pimageCreator = *new GFx::ImageCreator(pRenderHAL->GetTextureManager());
 //   loader.SetImageCreator(pimageCreator);

#ifdef GFX_ENABLE_SOUND
    //Sound properties 
    float SoundVolume = 1;
    bool  IsMute      = false;
    InitializeSound();
    Sound::SoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        // Sound renderer and parameters for playing swf streaming sounds are provided to
        // to the loader by GFxAudio. Those parameters control when and how the synchronization
        // between swf frames and sound frames should be done.
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
    pCommandQueue = queue;
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


    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return 1;

    pMovie->SetMouseCursorCount(1);
	
	hMovieDisplay = pMovie->GetDisplayHandle();

    // Create renderer.
	pRenderHAL = *new Render::D3D1x::HAL(pCommandQueue);
    if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
        return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    if (!pRenderHAL->InitHAL(D3D1x::HALInitParams(pDevice D3D11(, pDeviceContext), 0, Scaleform::GetCurrentThreadId())))
        return 1;

    // Set renderer on loader so that it is also applied to all children.
    //pRenderConfig = *new RenderConfig(pRenderer, RenderConfig::RF_EdgeAA | RenderConfig::RF_StrokeNormal);
	//pRenderConfig = *new RenderConfig();
	//loader.SetRenderConfig(pRenderConfig);

    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // If you wanted to use the movie as a transparent HUD overlay, you would
    // set Background Alpha to 0. We don't need to do this for player sample.
    // pMovie->SetBackgroundAlpha(0.0f);

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
        bool bIsInitFMOD = pSoundFMODSystem->Initialize(
#ifdef SF_OS_PS3		
            GetSpurs()
#endif
            );
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

    // Present the back buffer contents to the display.
    if (pSwapChain)
        pSwapChain->Present(0, 0);

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
        // Use a look-up table for keys don't correlate in order,.
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

                // TODO: fill this out some more
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
            // Pass Key events to the movie so that can be handled in ActionScript.
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

        //case 'A':
        //    AAEnabled = ! AAEnabled;
        //    {
        //        UInt32 rendererFlags = pRenderConfig->GetRenderFlags() &
        //                               ~RenderConfig::RF_EdgeAA;
        //        if (AAEnabled)
        //            rendererFlags |= RenderConfig::RF_EdgeAA;
        //        pRenderConfig->SetRenderFlags(rendererFlags);
        //    }
        //    break;

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

// Setup Application Window and initialize Direct3D.
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

    UInt32 flags = 0;
#ifdef SF_BUILD_DEBUG
    flags |= D3D1x(CREATE_DEVICE_DEBUG);
#endif

    memset(&SwapChainDesc, 0, sizeof(SwapChainDesc));
    SwapChainDesc.BufferCount = 1;
    SwapChainDesc.BufferDesc.Width = Width;
    SwapChainDesc.BufferDesc.Height = Height;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 85;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = hWnd_C;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.Windowed = TRUE;

#if (SF_D3D_VERSION == 10)
    HRESULT hr = D3D10CreateDeviceAndSwapChain1(NULL, D3D1x(DRIVER_TYPE_HARDWARE), NULL, flags, 
        D3D10_FEATURE_LEVEL_10_1, D3D10_1_SDK_VERSION, &SwapChainDesc, &pSwapChain, &pDevice);
    if (FAILED(hr))
    {
        hr = D3D10CreateDeviceAndSwapChain1(NULL, D3D1x(DRIVER_TYPE_HARDWARE), NULL, flags, D3D10_FEATURE_LEVEL_10_0, 
            D3D10_1_SDK_VERSION, &SwapChainDesc, &pSwapChain, &pDevice);
        if (FAILED(hr))
            return 0;
    }
    pDeviceContext = pDevice;
    pDeviceContext->AddRef();

#elif (SF_D3D_VERSION == 11)
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, 3, D3D1x(SDK_VERSION), 
        &SwapChainDesc, &pSwapChain, &pDevice,  0, &pDeviceContext);
    if (FAILED(hr))
        return 0;
#endif

    if ( !CreateBuffers() )
        return 0;

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
          
        // *** The following logic is used to route input to GFxPlayer.
        
        case WM_MOUSEMOVE:
            OnMouse(GFx::Event::MouseMove, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            {
                // Nonclient position must be adjusted to be inside the window
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

    // Use size_t to quiet /Wp64 warning.
    if ( (papp=((FxPlayerTiny*)(size_t)GetWindowLongPtr(hwnd,0))) == 0 )
        return DefWindowProcW(hwnd,iMsg,wParam,lParam);

    // Call non-static member to Windows handle messages.
    return papp->MemberWndProc(iMsg, wParam, lParam);
}


// ***** WinMain() -  Application entry point.

// The main function implementation creates the FxPlayerTiny class,
// passes it the filename and calls Run. FxPlayerTiny::Run is responsible
// for all of the rendering setup and movie playback.

int SF_CDECL main(int argc, char *argv[])
//INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, INT)
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
