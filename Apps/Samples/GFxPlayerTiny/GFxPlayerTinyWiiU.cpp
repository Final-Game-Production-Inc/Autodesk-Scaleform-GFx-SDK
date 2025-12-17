/**************************************************************************

Filename    :   GFxPlayerTinyWiiU.cpp
Content     :   Tiny GFxPlayer sample for WiiU.
Created     :   March 13, 2008
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"

#include "GFx/AS3/AS3_Global.h"

#include "Render/Renderer2D.h"
#include "Render/WiiU/WiiU_HAL.h"

#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#include "Render/ImageFiles/GTX_ImageFile.h"

// WiiU includes
#include <cafe/fs.h>
#include <cafe/vpad.h>
#include <cafe/pads/kpad/kpad.h>

#define FXPLAYER_TV_FILENAME    "/vol/content/FxPlayer/3DWindow.swf"
#define FXPLAYER_DRC_FILENAME   "/vol/content/FxPlayer/Window.swf"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

static void *mem2alloc(u32 size)     { return SF_ALLOC(size, Stat_Default_Mem); }
static u8    mem2free(void *ptr)     { SF_FREE(ptr); return 1; }


// ***** FxPlayerTiny Application Class

// FxPlayerTiny class encapsulates all of the system setup and GFxPlayer
// program state needed for playback. FxPlayerTiny is created in main() /
// WinMain() and contains most of the program logic in its Run function.

class FxPlayerTiny
{
public:
    GX2ColorBuffer          ColorBuffer;
    void*                   pColorBuffer;
    GX2ColorBuffer          DRCColorBuffer;
    void*                   pDRCColorBuffer;
    GX2DepthBuffer          DepthBuffer;
    void*                   pDepthBuffer;
    GX2ContextState *       pContext;
    void*                   pTVDispBuffers;
    UPInt                   DispBufferSize;
    void*                   pDRCDispBuffer;
    RenderTarget*           pDisplayRT;
    RenderTarget*           pDRCRT;

    // Input
    struct Controller
    {
        bool                Enabled;
        int                 Touch, TouchPosValid;
        int                 MouseX, MouseY;
        float               LastLx, LastLy, LastRx, LastRy;

        Controller() { Enabled = 0; Touch = 0; TouchPosValid = 0; MouseX = MouseY = -100; LastLx = LastRx = LastLy = LastRy = -1e6; }
    };
    Controller              Controllers[4];
    Controller              DisplayControllers[2];
    int                     ControllerCount;

    // Scaleform GFx objects.
    Ptr<MovieDef>           pTVMovieDef;
    Ptr<Movie>              pTVMovie;
    Ptr<MovieDef>           pDRCMovieDef;
    Ptr<Movie>              pDRCMovie;
	Ptr<Render::WiiU::HAL>  pRenderHAL;
	Ptr<Render::Renderer2D> pRenderer;
	ThreadCommandQueue*     pCommandQueue;
    Render::MemoryManager  *pMemManager;

	MovieDisplayHandle      hTVMovieDisplay;
    MovieDisplayHandle      hDRCMovieDisplay;
    UInt32                  MovieLastTime;

    // Toggle states.
    bool    Wireframe; 
    bool    ControlKeyDown;
    bool    AAEnabled;
    bool    Paused;

    // Playback filename and window size.
    String  FileName[2];
    int     TVWidth, TVHeight;
    int     DRCWidth, DRCHeight;

    bool    VerboseAction;

public:
    FxPlayerTiny(const char* pfilename1, const char* pfilename2, bool verboseAction);
    ~FxPlayerTiny();

    // Initialize the player and run playback loop.
    int     Run();
    // Advance playback and render a frame.
    void    AdvanceAndDisplay();
    void    Display();

    // Input handler functions. Process mouse and keyboard and pass 
    // appropriate Event notifications to Movie::HandleEvent.
    bool    ProcessMessages();

    void OnKey(int movie, int controller, unsigned keyCode, bool downFlag);
    void OnMouse(int movie, int controller, GFx::Event::EventType eventType, unsigned button, int x, int y);
    void OnMouseMove(int movie, int controller, int x, int y);
    void OnMouseButton(int movie, int controller, unsigned button, bool downFlag, int x, int y);
    void OnPadStick(int movie, unsigned controllerIndex, PadKeyCode padCode, float xpos, float ypos);

    // System window and rendering setup.
    bool    SetupWindow();
};

FxPlayerTiny* pApp;

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
    virtual void LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        SF_UNUSED(messageType);
        // Output log to console
        vprintf(pfmt, argList);
    }
};


// ***** FxPlayerTiny Class Implementation

FxPlayerTiny::FxPlayerTiny(const char* pfilename, const char* pfilename2, bool verboseAction)
{
    AAEnabled     = 1;
    Paused        = 0;
    FileName[0]   = pfilename;
    FileName[1]   = pfilename2;
    VerboseAction = verboseAction;

    pApp = this;
}

FxPlayerTiny::~FxPlayerTiny() 
{
    if ( pMemManager )
    {
        pMemManager->Free(pColorBuffer,ColorBuffer.surface.imageSize);
        pMemManager->Free(pDepthBuffer,DepthBuffer.surface.imageSize);
        pMemManager->Free(pContext, sizeof(GX2ContextState));
        pMemManager->Free(pTVDispBuffers,DispBufferSize);
        pMemManager->Free(pDRCColorBuffer,DRCColorBuffer.surface.imageSize);
        pMemManager->Free(pDRCDispBuffer,DispBufferSize);
        delete pMemManager;
    }
	
	delete pCommandQueue;
}


// Called from main()/WinMain() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
int FxPlayerTiny::Run()
{    
    if (!SetupWindow())
        return 1;

    // Create and Configure Loader.   
    Loader loader;
	loader.SetLog(Ptr<GFx::Log>(*new GFxPlayerLog()));

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FileOpener derived class that loads files in a custom way.
    // Here FileOpener and FSCommandHandler are set for sample purposes.    
    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener); 
    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry();
#ifdef SF_ENABLE_LIBJPEG
    pimgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
    pimgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif
    pimgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
    pimgReg->AddHandler(&SF::Render::GTX::FileReader::Instance);
	loader.SetImageFileHandlerRegistry(pimgReg);

    // AS2 support
	Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
	loader.SetAS2Support(pAS2Support);
    // AS3 support
    Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
    loader.SetAS3Support(pASSupport);

	FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pCommandQueue = queue;
	
    // Load the movies file and create instances.
    if (!(pTVMovieDef = *loader.CreateMovie(FileName[0])))
        return 1;
		
    if (!(pTVMovie = *pTVMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return 1;
		
    if (!(pDRCMovieDef = *loader.CreateMovie(FileName[1])))
        return 1;

	if (!(pDRCMovie = *pDRCMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return 1;
		
    pTVMovie->SetMouseCursorCount(4);
    pDRCMovie->SetMouseCursorCount(1);
	hTVMovieDisplay = pTVMovie->GetDisplayHandle();
    hDRCMovieDisplay = pDRCMovie->GetDisplayHandle();

	pRenderHAL = *new Render::WiiU::HAL();

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.    
    if (!pRenderHAL->InitHAL(WiiU::HALInitParams(pContext, pMemManager)))
        return 1;

    // Create renderer.
    if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
        return 1;

	// Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;
		
    // Create the default render targets.
    pDisplayRT = pRenderHAL->CreateRenderTarget(ColorBuffer, DepthBuffer);
    pRenderHAL->SetRenderTarget(pDisplayRT, false);

    pDRCRT = pRenderHAL->CreateRenderTarget(DRCColorBuffer, DepthBuffer);

    // Set playback view to span the entire window.
    pTVMovie->SetViewport(TVWidth, TVHeight, 0, 0, TVWidth, TVHeight);
    pDRCMovie->SetViewport(DRCWidth, DRCHeight, 0, 0, DRCWidth, DRCHeight);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = Timer::GetTicks() / 1000;

    // Application / Player message loop.
    while (1)
    {
        if (!ProcessMessages())
            break;            
        AdvanceAndDisplay();
    }

    return 0;
}


// Advances GFx animation and draws the scene.
void FxPlayerTiny::AdvanceAndDisplay()
{
    // Advance time
    UInt32 time  = Timer::GetTicks() / 1000;
    float  delta = ((float)(time - MovieLastTime)) / 1000.0f;

    if (!Paused)
    {
        pTVMovie->Advance(delta);
        pDRCMovie->Advance(delta);
    }

    // GFx rendering
    Display();

    MovieLastTime = time;

    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());
}

void FxPlayerTiny::Display()
{
    pRenderer->BeginFrame();

    pRenderHAL->SetRenderTarget(pDisplayRT, true);
    GX2ClearColor(&ColorBuffer, 0,0,0,0);
    GX2SetContextState(pContext);

    if (hTVMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
        pRenderer->Display(hTVMovieDisplay);

    GX2SwapBuffers(&ColorBuffer);
    GX2SetContextState(pContext);
    GX2WaitForVsync();

    // Rendering for DRC
    if (DisplayControllers[0].Enabled)
    {
        pRenderHAL->SetRenderTarget(pDRCRT, true);

        GX2ClearColor(&DRCColorBuffer, 0,0,0,0);
        GX2SetContextState(pContext);

        if (hDRCMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
            pRenderer->Display(hDRCMovieDisplay);

        GX2CopyColorBufferToScanBuffer(&DRCColorBuffer, GX2_SCAN_TARGET_DRC_FIRST);
        GX2SetContextState(pContext);
    }

    pRenderer->EndFrame();
}

void FxPlayerTiny::OnKey(int movie, int controller, unsigned keyCode, bool downFlag)
{
    KeyEvent kevent(downFlag ? GFx::Event::KeyDown : GFx::Event::KeyUp, (Key::Code)keyCode, 0,0, controller);
    (movie ? pDRCMovie : pTVMovie)->HandleEvent(kevent);
}

void FxPlayerTiny::OnMouseMove(int movie, int controller, int x, int y)
{
    MouseEvent event(GFx::Event::MouseMove, 0, (float)x, (float)y, 0.0f, controller);
    (movie ? pDRCMovie : pTVMovie)->HandleEvent(event);
}

void FxPlayerTiny::OnMouseButton(int movie, int controller, unsigned button, bool downFlag, int x, int y)
{
    MouseEvent event(downFlag ? GFx::Event::MouseDown : GFx::Event::MouseUp, button, (float)x, (float)y, 0.0f, controller);
    (movie ? pDRCMovie : pTVMovie)->HandleEvent(event);
}

void FxPlayerTiny::OnPadStick(int movie, unsigned controller, PadKeyCode padCode, float xpos, float ypos)
{
    GamePadAnalogEvent event(padCode, xpos, ypos, controller);
    (movie ? pDRCMovie : pTVMovie)->HandleEvent(event);
}

static void WPADConnectFunc(SInt32 chan, SInt32 result)
{
    if (chan >= 0 && chan < 4)
    {
        pApp->Controllers[chan].Enabled = (result == WPAD_ERR_NONE);

        KPADSetButtonProcMode(chan, KPAD_BUTTON_PROC_MODE_TIGHT);
    }
}

static void DRCConnectFunc(SInt32 chan, SInt32 result)
{
    if (chan >= 0 && chan < 2)
    {
        pApp->DisplayControllers[chan].Enabled = (result == GX2_DRC_ATTACH);

        VPADSetButtonProcMode(chan, VPAD_BUTTON_PROC_MODE_TIGHT);
    }
}

bool FxPlayerTiny::SetupWindow()
{
    // On WiiU rendering just uses normal memory (MEM2).
    pMemManager = SF_NEW WiiU::MemoryManager();

    GX2Init(0);

    TVWidth = 1280;
    TVHeight = 720;
    DRCWidth = 854;
    DRCHeight = 480;

    GX2InitColorBuffer(&ColorBuffer, TVWidth, TVHeight,
                       GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_AA_MODE_1X);
    pColorBuffer = pMemManager->Alloc(ColorBuffer.surface.imageSize, ColorBuffer.surface.alignment, Memory_Normal);
    GX2InitColorBufferPtr(&ColorBuffer, pColorBuffer);
    GX2Invalidate(GX2_INVALIDATE_CPU, pColorBuffer, ColorBuffer.surface.imageSize);

    GX2InitDepthBuffer(&DepthBuffer, TVWidth, TVHeight,
                       GX2_SURFACE_FORMAT_D_D24_S8_UNORM, GX2_AA_MODE_1X);
    pDepthBuffer = pMemManager->Alloc(DepthBuffer.surface.imageSize, DepthBuffer.surface.alignment, Memory_Normal);
    GX2Invalidate(GX2_INVALIDATE_CPU, pDepthBuffer, DepthBuffer.surface.imageSize);
    GX2InitDepthBufferPtr(&DepthBuffer, pDepthBuffer);

    pContext = (GX2ContextState*) pMemManager->Alloc(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT, Memory_Normal);
    GX2SetupContextState(pContext);

    GX2SetColorBuffer(&ColorBuffer, GX2_RENDER_TARGET_0);
    GX2SetDepthBuffer(&DepthBuffer);

    GX2TVRenderMode tvmode = GX2_TV_RENDER_720;
    GX2Boolean      scale;

    GX2CalcTVSize(tvmode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &DispBufferSize, &scale);

    pTVDispBuffers = pMemManager->Alloc(DispBufferSize, GX2_SCAN_BUFFER_ALIGNMENT, Memory_Normal);
    GX2Invalidate(GX2_INVALIDATE_CPU, pTVDispBuffers, DispBufferSize);

    GX2SetTVBuffer(pTVDispBuffers, DispBufferSize, tvmode, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);

    GX2SetViewport(0, 0, TVWidth, TVHeight, 0.0f, 1.0f);
    GX2SetScissor(0, 0, TVWidth, TVHeight);
    GX2SetTVScale(TVWidth, TVHeight);

    GX2SetSwapInterval(1);
    GX2ClearColor(&ColorBuffer, 0,0,0,0);
    GX2SwapBuffers(&ColorBuffer);
    GX2WaitForVsync();
    GX2SwapBuffers(&ColorBuffer);
    GX2WaitForVsync();

    GX2SetContextState(pContext);

    GX2SetTVEnable(GX2_ENABLE);

    // DRC
    GX2InitColorBuffer(&DRCColorBuffer, DRCWidth, DRCHeight,
                       GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_AA_MODE_1X);
    pDRCColorBuffer = pMemManager->Alloc(DRCColorBuffer.surface.imageSize, DRCColorBuffer.surface.alignment, Memory_Normal);
    GX2InitColorBufferPtr(&DRCColorBuffer, pDRCColorBuffer);
    GX2Invalidate(GX2_INVALIDATE_CPU, pDRCColorBuffer, DRCColorBuffer.surface.imageSize);

    GX2CalcDRCSize(GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE, &DispBufferSize, &scale);
    pDRCDispBuffer = pMemManager->Alloc(DispBufferSize, GX2_SCAN_BUFFER_ALIGNMENT, Memory_Normal);

    GX2SetDRCBuffer(pDRCDispBuffer, DispBufferSize, GX2_DRC_SINGLE, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_BUFFERING_DOUBLE);
    GX2SetDRCScale(DRCWidth, DRCHeight);

    GX2CopyColorBufferToScanBuffer(&DRCColorBuffer, GX2_SCAN_TARGET_DRC_FIRST);
    GX2SetContextState(pContext);
    GX2SetDRCEnable(GX2_ENABLE);

    // input
    WPADRegisterAllocator(mem2alloc, mem2free);
    KPADInit();
    VPADInit();

    for (int i = 0; i < 4; i++)
    {
        KPADSetConnectCallback(i, WPADConnectFunc);

        UInt32 type;
        if (WPAD_ERR_NONE == WPADProbe(i, &type))
            Controllers[i].Enabled = 1;

        Controllers[i].MouseX      = 100;
        Controllers[i].MouseY      = 100;
    }

    for (int i = 0; i < 2; i++)
    {
        GX2SetDRCConnectCallback(i, DRCConnectFunc);
    }

    GX2DRCMode drc = GX2GetSystemDRCMode();
    if (drc != GX2_DRC_NONE)
    {
        DisplayControllers[0].Enabled = true;
        VPADSetButtonProcMode(0, VPAD_BUTTON_PROC_MODE_TIGHT);
    }

    return 1;
}

static int keymap[13][2] = {
    {KPAD_BUTTON_LEFT,  Key::Left},
    {KPAD_BUTTON_RIGHT, Key::Right},
    {KPAD_BUTTON_UP,    Key::Up},
    {KPAD_BUTTON_DOWN,  Key::Down},
    {KPAD_BUTTON_PLUS,  Key::KP_Add},
    {KPAD_BUTTON_MINUS, Key::KP_Subtract},
    {KPAD_BUTTON_A,     Key::Return},
    {KPAD_BUTTON_B,     Key::Escape},
    {KPAD_BUTTON_1,     Key::Num1},
    {KPAD_BUTTON_2,     Key::Num2},
    {KPAD_BUTTON_HOME,  Key::Home},
    {KPAD_BUTTON_C,     Key::C},
    {KPAD_BUTTON_Z,     Key::Z}};

static int vpad_keymap[15][2] = {
    {VPAD_BUTTON_LEFT,  Key::Left},
    {VPAD_BUTTON_RIGHT, Key::Right},
    {VPAD_BUTTON_UP,    Key::Up},
    {VPAD_BUTTON_DOWN,  Key::Down},
    {VPAD_BUTTON_PLUS,  Key::KP_Add},
    {VPAD_BUTTON_MINUS, Key::KP_Subtract},
    {VPAD_BUTTON_A,     Key::Return},
    {VPAD_BUTTON_B,     Key::Escape},
    {VPAD_BUTTON_X,     Key::Space},
    {VPAD_BUTTON_Y,     Key::Y},
    {VPAD_BUTTON_HOME,  Key::Home},
    {VPAD_TRIGGER_ZL,   Key::PageDown},
    {VPAD_TRIGGER_ZR,   Key::PageUp},
    {VPAD_TRIGGER_L,    Key::Num1},
    {VPAD_TRIGGER_R,    Key::Num2}};

// Message processing function to be called in the application loops.
bool FxPlayerTiny::ProcessMessages()
{
    for (int drc = 0; drc < 2; drc++)
    {
        if (!DisplayControllers[drc].Enabled)
            continue;

        VPADStatus vpad[VPAD_MAX_READ_BUFS];
        int        err;

        unsigned n = VPADRead(drc, vpad, VPAD_MAX_READ_BUFS, &err);

        for (int i = n-1; i >= 0; i--)
        {
            for (int j = 0; j < 17; j++)
                if (vpad[i].trig & vpad_keymap[j][0])
                    OnKey(1, 0, vpad_keymap[j][1], 1);
                else if (vpad[i].release & vpad_keymap[j][0])
                    OnKey(1, 0, vpad_keymap[j][1], 0);

            bool change = false;

            if (vpad[i].tpData.validity == VPAD_TP_VALIDITY_VALID && (vpad[i].tpData.x || vpad[i].tpData.y))
            {
                VPADTPData cpt;
                VPADGetTPCalibratedPoint(drc, &cpt, &vpad[i].tpData);
                int mx = int( cpt.x * DRCWidth/float(TVWidth) + 0.5f );
                int my = int( cpt.y * DRCHeight/float(TVHeight) + 0.5f );

                if (!DisplayControllers[drc].TouchPosValid || mx != DisplayControllers[drc].MouseX || my != DisplayControllers[drc].MouseY)
                {
                    DisplayControllers[drc].MouseX = mx;
                    DisplayControllers[drc].MouseY = my;
                    change = true;
                }
                DisplayControllers[drc].TouchPosValid = true;
            }
            else
                DisplayControllers[drc].TouchPosValid = false;

            if (DisplayControllers[drc].Touch != vpad[i].tpData.touch)
            {
                if (DisplayControllers[drc].TouchPosValid)
                {
                    pApp->OnMouseButton(1, 0, 0, vpad[i].tpData.touch,
                                        DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY);
                    DisplayControllers[drc].Touch = vpad[i].tpData.touch;
                }
                else if (vpad[i].tpData.touch)
                    DisplayControllers[drc].Touch = 2;
                else
                {
                    pApp->OnMouseButton(1, 0, 0, vpad[i].tpData.touch,
                                        DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY);
                    DisplayControllers[drc].Touch = vpad[i].tpData.touch;
                }
            }
            else if (change)
                pApp->OnMouseMove(1, 0, DisplayControllers[drc].MouseX, DisplayControllers[drc].MouseY);

            float padlx = vpad[i].lStick.x;
            float padly = vpad[i].lStick.y;
            float padrx = vpad[i].rStick.x;
            float padry = vpad[i].rStick.y;

            if (padlx != DisplayControllers[drc].LastLx || padly != DisplayControllers[drc].LastLy)
            {
                pApp->OnPadStick(1, 0, Pad_LT, padlx, padly);
                DisplayControllers[drc].LastLx = padlx; DisplayControllers[drc].LastLy = padly;
            }
            if (padrx != DisplayControllers[drc].LastRx || padry != DisplayControllers[drc].LastRy)
            {
                pApp->OnPadStick(1, 0, Pad_RT, padrx, padry);
                DisplayControllers[drc].LastRx = padrx; DisplayControllers[drc].LastRy = padry;
            }
        }
    }

    for (int wim = 0; wim < 4; wim++)
    {
        if (!Controllers[wim].Enabled)
            continue;

        KPADStatus kpad[KPAD_MAX_READ_BUFS];

        unsigned n = KPADRead(wim, kpad, KPAD_MAX_READ_BUFS);

        for (int i = n-1; i >= 0; i--)
        {
            int mx = TVWidth*0.5f + kpad[i].pos.x * TVWidth*0.50f;
            int my = TVHeight*0.5f + kpad[i].pos.y * TVHeight*0.50f;

            if (Controllers[wim].MouseX != mx ||
                Controllers[wim].MouseY != my)
            {
                Controllers[wim].MouseX = mx;
                Controllers[wim].MouseY = my;

                pApp->OnMouseMove(0, i, mx, my);
            }

            for (int j = 0; j < 13; j++)
                if (kpad[i].trig & keymap[j][0])
                    pApp->OnKey(0, i, keymap[j][1], 1);
                else if (kpad[i].release & keymap[j][0])
                    pApp->OnKey(0, i, keymap[j][1], 0);

            if (kpad[i].trig & KPAD_BUTTON_A)
                pApp->OnMouseButton(0, i, 0, 1, Controllers[wim].MouseX, Controllers[wim].MouseY);
            else if (kpad[i].release & KPAD_BUTTON_A)
                pApp->OnMouseButton(0, i, 0, 0, Controllers[wim].MouseX, Controllers[wim].MouseY);
        }
    }
    return 1;
}


// The main function implementation creates the FxPlayerTiny class,
// passes it the filename and calls Run. FxPlayerTiny::Run is responsible
// for all of the rendering setup and movie playback.
int SF_CDECL main(int argc, char *argv[])
{
    FSInit();

    SF::GFx::System gfxInit;

    int result = 1;
    {
        bool verboseAction = false;
        const char *filename1 = FXPLAYER_TV_FILENAME;
        const char *filename2 = FXPLAYER_DRC_FILENAME;
        if (argc >= 2)
            filename1 = argv[1];
        if (argc >= 3)
            filename2 = argv[2];

        FxPlayerTiny player(filename1, filename2, verboseAction);
        result = player.Run();
    }

    return result;
}
