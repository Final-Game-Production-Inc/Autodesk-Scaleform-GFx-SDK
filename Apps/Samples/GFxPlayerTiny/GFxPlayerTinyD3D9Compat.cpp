/**************************************************************************

Filename    :   GFxPlayerTinyD3D9Compat.cpp
Content     :   Tiny GFxPlayer sample for Direct3D 9, based on GFx 3.1 API,
                using the GFx 4.0 Compatibility header.
Created     :   March 13, 2008
Authors     :   Michael Antonov, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx Includes
#include "GFxCompat.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"

// System Includes
#include <windows.h>

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   "Window.swf"
// Window title to use for the application.
#define FXPLAYER_APP_TITLE  "Scaleform GFxPlayerTinyD3D9 v" GFC_FX_VERSION_STRING

#include "GFx/AS3/AS3_Global.h"

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
    LPDIRECT3D9             pD3D;           // Used to create the D3D9Device.
    LPDIRECT3DDEVICE9       pDevice;        // Our rendering device.
    D3DPRESENT_PARAMETERS   PresentParams;

    // Scaleform GFx object pointers.
    GPtr<GFxMovieDef>       pMovieDef;
    GPtr<GFxMovieView>      pMovie;

    // new for 4.0
    GPtr<SF::Render::D3D9::HAL>  pRenderHAL; 
    GPtr<SF::Render::Renderer2D> pRenderer;
    ThreadCommandQueue* pCommandQueue;

//    GPtr<GFxRenderConfig>   pRenderConfig;
    SF::GFx::MovieDisplayHandle        hMovieDisplay;

    UInt32                  MovieLastTime;

    // Toggle states.
    bool                    Wireframe; 
    bool                    ControlKeyDown;
    bool                    AAEnabled;
    bool                    Paused;         // Set when the movie is paused in the player.    

    // Playback filename and window size.
    GString               FileName;
    int                     Width;
    int                     Height;

public:
    
    FxPlayerTiny(LPSTR pfilename);    
    ~FxPlayerTiny();

    // Initialize the player and run playback loop.
    int     Run();
    // Advance playback and render a frame.
    void    AdvanceAndDisplay();
    void    Display();

    // Input handler functions. Process mouse and keyboard and pass 
    // appropriate GFxEvent notifications to GFxMovieView::HandleEvent.
    void    OnKey(UInt keyCode, bool downFlag);
    void    OnMouse(GFxEvent::EventType eventType, UInt button, SInt x, SInt y);
    void    HandleKeyEvent(UInt keyCode, bool downFlag);
   
    // System window and rendering setup.
    bool    SetupWindow(const GString& name);
    HRESULT ResetD3D();    

    // Windows message handler functions. StaticWindowProc is called by the
    // system and delegates most of its processing to MemberWndProc.
    LRESULT MemberWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
};


// A sample implementation of GFxFSCommandHandler. Users can install such a
// handler to receive notifications from fscommand() calls in ActionScript.
// Note that in many cases it is better to use GFxExternalInterface instead
// since it allows for better argument passing through ExternalInterface.call.
class FxPlayerFSCommandHandler : public GFxFSCommandHandler
{
public:
    virtual void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg)
    {
        // Do not log anything since we don't have a console.
        // printf("Command: %s, Args: %s", pcommand, parg);
        GUNUSED3(pmovie, pcommand, parg);
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

// ***** FxPlayerTiny Class Implementation

FxPlayerTiny::FxPlayerTiny(LPSTR pfilename)
{
    pD3D            = Direct3DCreate9(D3D_SDK_VERSION);
    pDevice         = NULL; 
    pCommandQueue   = 0;

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
    FileName        = pfilename;
}

FxPlayerTiny::~FxPlayerTiny()
{
    if (pDevice)
        pDevice->Release();
    if (pD3D)
        pD3D->Release();
    
    pMovie = NULL;
    pMovieDef = NULL;
    delete pCommandQueue;
    UnregisterClass(pWndClassName, hInstance);
}


// Called from main()/WindMain() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
int FxPlayerTiny::Run()
{    
    if (!SetupWindow(GString(FXPLAYER_APP_TITLE " ") + FileName))
        return 1;

    // Show the window.
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

   
    // Create and Configure GFxLoader.   
    GFxLoader loader;

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a GFxFileOpener derived class that loads files in a custom way.
    // Here GFxFileOpener and GFxFSCommandHandler are set for sample purposes.    
    GPtr<GFxFileOpener> pfileOpener = *new GFxFileOpener;
    loader.SetFileOpener(pfileOpener); 
    GPtr<GFxFSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    // For D3D, it is good to override image creator to keep image data,
    // so that it can be restored in case of a lost device.
//    GPtr<GFxImageCreator> pimageCreator = *new GFxImageCreator(1);
//    loader.SetImageCreator(pimageCreator);

    //////////////////////////////////////////////////////////////////////////
    // NEW CODE FOR GFx 4.0
    // Set Font Provider:
    SF::Ptr<SF::GFx::FontProviderWin32> fontProvider = *new SF::GFx::FontProviderWin32(::GetDC(0));
    loader.SetFontProvider(fontProvider);

    // Image file handling registry (PNG, etc).
    Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

    // Add AS2 Support:
    SF::Ptr<SF::GFx::ASSupport> pAS2Support = *new SF::GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);

    // Add AS3 Support:
    SF::Ptr<SF::GFx::ASSupport> pASSupport = *new SF::GFx::AS3Support();
    loader.SetAS3Support(pASSupport);
    //////////////////////////////////////////////////////////////////////////
    
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

    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue();
    pCommandQueue = queue;
    
    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, NULL, pCommandQueue)))
        return 1;

    pMovie->SetMouseCursorCount(1);

    // new for 4.0, create renderer
    hMovieDisplay = pMovie->GetDisplayHandle();
    pRenderHAL = *new SF::Render::D3D9::HAL(pCommandQueue);
    if (!(pRenderer = *new SF::Render::Renderer2D(pRenderHAL.GetPtr())))
        return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    if (!pRenderHAL->InitHAL(SF::Render::D3D9::HALInitParams(pDevice, PresentParams, 0, Scaleform::GetCurrentThreadId())))
        return 1;

    // Set renderer on loader so that it is also applied to all children.
//    pRenderConfig = *new GFxRenderConfig(pRenderer, GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeNormal);
//    loader.SetRenderConfig(pRenderConfig);

    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // If you wanted to use the movie as a transparent HUD overlay, you would
    // set Background Alpha to 0. We don't need to do this for player sample.
    // pMovie->SetBackgroundAlpha(0.0f);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = timeGetTime();

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
            // Check for lost D3D Devices.
            //if (pRenderer)
            //{                       
            //    Render::D3D9::Renderer::DisplayStatus status = pRenderer->CheckDisplayStatus();
            //    if (status == Render::D3D9::Renderer::DisplayStatus_Unavailable)
            //    { ::Sleep(10); continue; }
            //    if (status == Render::D3D9::Renderer::DisplayStatus_NeedsReset)
            //    {           
            //        if (!SUCCEEDED(ResetD3D()))
            //            continue;
            //    }
            //}

            // Advance movie animation and render it to screen.
            AdvanceAndDisplay();
        }
    }

    return 0;
}


// Advances GFx animation and draws the scene.
void FxPlayerTiny::AdvanceAndDisplay()
{
    // Clear the back buffer to a black color.
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(200,200,200), 1.0f, 0);

    // *** GFx Rendering    
    if (pMovie)
    {
        // GRendererD3D9 does not modify D3DRS_FILLMODE, so we can set it here.
        pDevice->SetRenderState(D3DRS_FILLMODE,
            Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

        UInt32  time    = timeGetTime();
        Float   delta   = ((Float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

        Display();

        if (pDevice)
            pDevice->Present( NULL, NULL, NULL, NULL );

        MovieLastTime = time;
    }
    SF_AMP_CODE(GFxAmpServer::GetInstance().AdvanceFrame());
}

// new for 4.0
void FxPlayerTiny::Display()
{
#ifdef SF_AMP_SERVER
    SF_AMP_SCOPE_RENDER_TIMER_DISPLAY("FxPlayerTiny::Display");
#endif
    pRenderer->BeginFrame();
    if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
    {
        pRenderer->Display(hMovieDisplay);
    }
    pRenderer->EndFrame();
}


// Helper function that converts Windows VK keyCode values
// to GFxKeyEvents and routes them to GFxPlayer.
void    FxPlayerTiny::HandleKeyEvent(UInt keyCode, bool downFlag)
{
    GFxKey::Code key(GFxKey::None /*VoidSymbol*/);

    if (keyCode >= 'A' && keyCode <= 'Z')
    {
        key = (GFxKey::Code) ((keyCode - 'A') + GFxKey::A);
    }
    else if (keyCode >= VK_F1 && keyCode <= VK_F15)
    {
        key = (GFxKey::Code) ((keyCode - VK_F1) + GFxKey::F1);
    }
    else if (keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
    {
        key = (GFxKey::Code) ((keyCode - VK_NUMPAD0) + GFxKey::KP_0);
    }
    else
    {
        // Use a look-up table for keys don't correlate in order,.
        struct {
            int          vk;
            GFxKey::Code gs;
            } table[] =
        {
                { VK_RETURN,    GFxKey::Return },
                { VK_ESCAPE,    GFxKey::Escape },
                { VK_LEFT,      GFxKey::Left },
                { VK_UP,        GFxKey::Up },
                { VK_RIGHT,     GFxKey::Right },
                { VK_DOWN,      GFxKey::Down },

                // TODO: fill this out some more
                { 0, GFxKey::None /*VoidSymbol*/ }
        };

        for (int i = 0; table[i].vk != 0; i++)
        {
            if (keyCode == (UInt)table[i].vk)
            {
                key = table[i].gs;
                break;
            }
        }
    }
    
    if (key != GFxKey::None /*VoidSymbol*/)
    {
        if (pMovie)
        {
            // Pass Key events to the movie so that can be handled in ActionScript.
            GFxKeyEvent event(downFlag ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, key);
            pMovie->HandleEvent(event);
        }
    }
}

// OnKey handles system keys by first interpreting some Ctrl+Key combinations
// in a special way and passing the rest of them to the movie.
void    FxPlayerTiny::OnKey(UInt keyCode, bool downFlag)
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

void    FxPlayerTiny::OnMouse(GFxEvent::EventType eventType, UInt button, SInt x, SInt y)
{
    if (pMovie)
    {
        GFxMouseEvent mevent(eventType, button, (Float)x, (Float)y);
        pMovie->HandleEvent(mevent);
    }
}


// Setup Application Window and initialize Direct3D.
bool FxPlayerTiny::SetupWindow(const GString& name)
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

    HWND hiWnd = CreateWindow(pWndClassName, name.ToCStr(),
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
        r.left, r.top, r.right-r.left, r.bottom - r.top,
        GetDesktopWindow(), NULL, wc.hInstance, (LPVOID) this );

    if (!pD3D)
        return 0;

    // Set up the structure used to create the D3DDevice    
    ZeroMemory(&PresentParams, sizeof(PresentParams));
    PresentParams.Windowed                = TRUE;
    PresentParams.SwapEffect              = D3DSWAPEFFECT_DISCARD;
    PresentParams.BackBufferFormat        = D3DFMT_UNKNOWN;  
    PresentParams.BackBufferWidth         = Width;
    PresentParams.BackBufferHeight        = Height;
    PresentParams.EnableAutoDepthStencil  = 1;
    PresentParams.AutoDepthStencilFormat  = D3DFMT_D24S8;
    //PresentParams.MultiSampleQuality        = D3DMULTISAMPLE_4_SAMPLES;

    // Create the D3DDevice
    if (FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hiWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &PresentParams, &pDevice ) ))
    {
        return 0;
    }

    return 1;
}

// Resets the D3D device and recreates triangle geometry.
HRESULT FxPlayerTiny::ResetD3D()
{   
    // Set new mode
    if( FAILED(pDevice->Reset(&PresentParams) ))
        return E_FAIL;
    return S_OK;
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
            OnMouse(GFxEvent::MouseMove, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_LBUTTONDOWN:
            ::SetCapture(hWnd);
            OnMouse(GFxEvent::MouseDown, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_LBUTTONUP:
            ::ReleaseCapture();
            OnMouse(GFxEvent::MouseUp, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            OnKey((UInt)wParam, 1);
            return 0;
        case WM_KEYUP:          
            OnKey((UInt)wParam, 0);
            return 0;
        case WM_CHAR:
            {
                UInt32 wcharCode = (UInt32)wParam;
                if (pMovie && wcharCode) 
                {
                    GFxCharEvent event(wcharCode);
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

#ifdef GFC_64BIT_POINTERS
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

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, INT)
{
#if 1
    // New malloc-friendly "Magic Header" allocator.  
    SF::SysAllocMalloc a;
    GFxSystem gfxInit(&a);
#else
    GFxSystem gfxInit;
#endif
    int result = 1;
    {
        // If command line is empty, use the default filename.
        LPSTR pfilename = (!lpCmdLine || !lpCmdLine[0]) ? 
                            FXPLAYER_FILENAME :lpCmdLine;
        
        FxPlayerTiny player(pfilename);
        
        result = player.Run();
    }

    return result;
}
