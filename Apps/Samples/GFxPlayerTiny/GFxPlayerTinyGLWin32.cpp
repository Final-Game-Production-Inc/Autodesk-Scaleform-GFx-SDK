/**************************************************************************

Filename    :   GFxPlayerTinyGLWin32.cpp
Content     :   Tiny GFxPlayer sample for OpenGL.
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

#include "GFx_FontProvider_Win32.h"

// System Includes
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include "wglext.h"

#include "GFx_Renderer_GL.h"

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   "Window.swf"
// Window title to use for the application.
#define FXPLAYER_APP_TITLE  "Scaleform GFxPlayerTinyOGL v" GFX_VERSION_STRING

#include "Render/Renderer2D.h"

#include "GFx/AS3/AS3_Global.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"


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
    HDC                     hDC;
    HINSTANCE               hInstance;

    // OpenGL state variables.
    HGLRC                   hGLRC;

    // Scaleform GFx object pointers.
    Ptr<MovieDef>       pMovieDef;
    Ptr<Movie>			pMovie;
	Ptr<Render::GL::HAL>  pRenderHAL; 
	Ptr<Render::Renderer2D> pRenderer;
    ThreadCommandQueue*     pCommandQueue;

	MovieDisplayHandle        hMovieDisplay;


    UInt32                  MovieStartTime;
    UInt32                  MovieLastTime;
    float                   ExitTimeout;

    // Toggle states.
    bool                    Wireframe; 
    bool                    ControlKeyDown;
    bool                    AAEnabled;
    bool                    Paused;         // Set when the movie is paused in the player.    

    // Playback filename and window size.
    String               FileName;
    int                     Width;
    int                     Height;

	bool                    VerboseAction;
public:
    
    FxPlayerTiny(int argc, char *argv[]);    
    ~FxPlayerTiny();

    // Initialize the player and run playback loop.
    int     Run();
    // Advance playback and render a frame.
    void    AdvanceAndDisplay();
    void    Display();

    // Input handler functions. Process mouse and keyboard and pass 
    // appropriate GFx::Event notifications to Movie::HandleEvent.
    void    OnKey(unsigned keyCode, bool downFlag);
    void    OnMouse(GFx::Event::EventType eventType, unsigned button, int x, int y);
    void    HandleKeyEvent(unsigned keyCode, bool downFlag);
   
    // System window and rendering setup.
    bool    SetupWindow(const String& name);

    // Windows message handler functions. StaticWindowProc is called by the
    // system and delegates most of its processing to MemberWndProc.
    LRESULT MemberWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
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

    pWndClassName   = "OpenGL_Window_Class";
    hWnd            = 0;
    hInstance       = 0;
    hDC             = 0;
    hGLRC           = 0;
    pCommandQueue   = 0;

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
    // Must shutdown the HAL prior to deleting the OGL context.
    if ( pRenderHAL )
        pRenderHAL->ShutdownHAL();

    if (hDC)
        ReleaseDC(hWnd, hDC);

    if (hWnd)
        DestroyWindow(hWnd);
    
    if (hGLRC)
    {       
        wglMakeCurrent(NULL,NULL);                      
        wglDeleteContext(hGLRC);
    }
    delete pCommandQueue;
    UnregisterClass(pWndClassName, hInstance);
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

	Ptr<ActionControl> pactControl = *new ActionControl(ActionControl::Action_ErrorSuppress);

    VerboseAction = false;      // Set to true to dump verbose ActionScript parsing

#ifndef GFC_NO_FXPLAYER_VERBOSE_ACTION
    pactControl->SetVerboseAction(VerboseAction);
#endif
	loader.SetActionControl(pactControl);

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FileOpener derived class that loads files in a custom way.
    // Here FileOpener and FSCommandHandler are set for sample purposes.    
    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener); 
    //Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    //loader.SetFSCommandHandler(pcommandHandler);

	Ptr<FontProviderWin32> fontProvider = *new FontProviderWin32(::GetDC(0));
	loader.SetFontProvider(fontProvider);

    // Image file handling registry (PNG, etc).
    Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

	Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
	loader.SetAS2Support(pAS2Support);

	Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
	loader.SetAS3Support(pASSupport);

    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pCommandQueue = queue;

    // Create renderer.
	pRenderHAL = *new Render::GL::HAL(pCommandQueue);
	if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
		return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    if (!pRenderHAL->InitHAL(GL::HALInitParams(0, Scaleform::GetCurrentThreadId())))
        return 1;

    // Set renderer on loader so that it is also applied to all children.
    //pRenderConfig = *new RenderConfig(pRenderer, RenderConfig::RF_EdgeAA | RenderConfig::RF_StrokeNormal);
    //loader.SetRenderConfig(pRenderConfig);

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

    if (!(pMovie = *pMovieDef->CreateInstance(MemoryParams(), 0, pactControl, pCommandQueue)))
        return 1;

	hMovieDisplay = pMovie->GetDisplayHandle();

	pMovie->SetMouseCursorCount(1);


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


// Advances GFx animation and draws the scene.
void FxPlayerTiny::AdvanceAndDisplay()
{
    // Clear the back buffer to a black color.
    GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
    if (scissorEnabled)
        glDisable(GL_SCISSOR_TEST);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnabled)
        glEnable(GL_SCISSOR_TEST);

    // *** GFx Rendering    
    if (pMovie)
    {
        // Set Wireframe
        glPolygonMode(GL_FRONT_AND_BACK, 
            Wireframe ? GL_LINE : GL_FILL);

        UInt32  time    = timeGetTime();
        float   delta   = ((float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

        Display();

        MovieLastTime = time;
    }

    // Present the back buffer contents to the display.
    ::SwapBuffers(hDC);

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
    else if (keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
    {
        key = (Key::Code) ((keyCode - VK_NUMPAD0) + Key::KP_0);
    }
    else
    {
        // Use a look-up table for keys don't correlate in order,.
        struct {
            int          vk;
            Key::Code gs;
            } table[] =
        {
                { VK_RETURN,    Key::Return },
                { VK_ESCAPE,    Key::Escape },
                { VK_LEFT,      Key::Left },
                { VK_UP,        Key::Up },
                { VK_RIGHT,     Key::Right },
                { VK_DOWN,      Key::Down },

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

    HWND hiWnd = CreateWindow(pWndClassName, name.ToCStr(),
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
        r.left, r.top, r.right-r.left, r.bottom - r.top,
        GetDesktopWindow(), NULL, wc.hInstance, (LPVOID) this );
    if (!hiWnd)
    {
        MessageBox(NULL,"Unable to create window", "Error", MB_OK | MB_ICONEXCLAMATION);      
        return 0;
    }

    // get a device context
    hDC = GetDC(hiWnd);
    if (!hDC)
    {
        MessageBox(NULL,"Unable to create device context", "Error", MB_OK | MB_ICONEXCLAMATION);      
        return 0;
    }

    PIXELFORMATDESCRIPTOR pfd;  
    memset (&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize =         sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion =      1;
    pfd.dwFlags =       PFD_DRAW_TO_WINDOW |  // Window drawing support
        PFD_SUPPORT_OPENGL |  // OpenGL support
        PFD_DOUBLEBUFFER;
    pfd.iPixelType =    PFD_TYPE_RGBA;
    pfd.cColorBits =    32;
    pfd.cDepthBits =    24;
    pfd.cStencilBits =  8;
    pfd.cAccumBits =    0;
    pfd.iLayerType =    PFD_MAIN_PLANE;

    // Find the closest available pixel format
    GLuint  pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (!pixelFormat)
    {
        MessageBox(NULL, "Can't choose an appropriate pixel format", "Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    if(!SetPixelFormat(hDC, pixelFormat, &pfd))
    {
        MessageBox(NULL, "Unable to set pixel format", "Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    // Create OpenGL rendering context
    if ((hGLRC = wglCreateContext(hDC))==0)
    {
        MessageBox(NULL, "Unable to create OpenGL rendering context", "OpenGLApp Error",MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    // Make the rendering context the active one
    if (!wglMakeCurrent(hDC, hGLRC))
    {       
        MessageBox(NULL,"Unable to activate OpenGL rendering context", "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

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
