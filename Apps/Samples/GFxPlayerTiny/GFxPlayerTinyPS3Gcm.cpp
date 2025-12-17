/**************************************************************************

Filename    :   GFxPlayerTinyPS3Gcm.cpp
Content     :   Tiny FxPlayer sample for Playstation 3.
Created     :   March 27, 2008
Authors     :   Michael Antonov, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx includes
#include "GFx_Kernel.h"
#include "GFx.h"

#include "Kernel/SF_Timer.h"
#include "GFx_Renderer_PS3.h"
#include "Render/Renderer2D.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"

#include "GFx/AS3/AS3_Global.h"

// PS3 includes
#include <sys/paths.h>
#include <cell/gcm.h>
#include <cell/pad.h>
#include <sys/process.h>
#include <sys/spu_initialize.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>

#include <sys/process.h>
SYS_PROCESS_PARAM(1001, 0x40000)

#include <stdlib.h>
#include <string.h>

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Scaleform::Render;
using namespace Scaleform::GFx;

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   SYS_APP_HOME "/Window.swf"
// Window title to use for the application.
#define FXPLAYER_APP_TITLE  "Scaleform FxPlayerTinyPS3 v" GFX_VERSION_STRING

#define VK_RETURN '\n'
#define VK_ESCAPE 257
#define VK_LEFT   258
#define VK_RIGHT  259
#define VK_UP     260
#define VK_DOWN   261

/////////////////////////////////////////////////////////////////////////////
// ***** FxPlayerTiny Application Class

// FxPlayerTiny class encapsulates all of the system setup and FxPlayer
// program state needed for playback. FxPlayerTiny is created in main() /
// WinMain() and contains most of the program logic in its Run function.
//
class FxPlayerTiny
{

    // Scaleform Fx object pointers.
    Ptr<MovieDef>               pMovieDef;
    Ptr<Movie>                  pMovie;
	Ptr<Render::PS3::HAL>       pRenderHAL; 
	Ptr<Render::Renderer2D>     pRenderer;
    ThreadCommandQueue*         pCommandQueue;
    UInt32                      MovieLastTime;
    Ptr<Render::RenderTarget>   pDisplayRT;


	// Context to draw

	MovieDisplayHandle        hMovieDisplay;

    // Toggle states.
    bool                    AAEnabled;
    bool                    Paused;
    bool                    QuitFlag;
    bool                    Wireframe;

    // Device state and allocator
    PS3::MemoryManager*     pMemManager;
    CellGcmContextData*     GcmCtx;

    // Frame buffers
    UInt32                  Buffers[2];
    CellGcmSurface          Surface;
    int                     Frame;

    // Last pad state read in ProcessMessages.
    CellPadData             pad0;

    // Playback filename and window size.
    String                  FileName;
    int                     Width;
    int                     Height;

public:    
    FxPlayerTiny(const char* pfilename);    
    ~FxPlayerTiny();

    // Initialize the player and run playback loop.
    int     Run();
    // Advance playback and render a frame.
    void    AdvanceAndDisplay();
    void    Display();

    // Input handler functions. Process mouse and keyboard and pass 
    // appropriate GFxEvent notifications to FxMovieView::HandleEvent.
    bool    ProcessMessages();
    void    OnKey(unsigned keyCode, bool downFlag);
    void    HandleKeyEvent(unsigned keyCode, bool downFlag);

    // System window and rendering setup.
    bool    SetupWindow(const String& name); 
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

/////////////////////////////////////////////////////////////////////////////
// ***** FxPlayerTiny Class Implementation

FxPlayerTiny::FxPlayerTiny(const char* pfilename)
{
#if (CELL_SDK_VER >= 93)
    cellSysmoduleLoadModule(CELL_SYSMODULE_RESC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
    cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_GCM);
#endif
    
    sys_spu_initialize (6, 0);

    AAEnabled       = 1;
    Paused          = 0;
    FileName        = pfilename;
    Wireframe       = 0;

    memset (&pad0, 0, sizeof (pad0));
}

FxPlayerTiny::~FxPlayerTiny()
{
    delete pCommandQueue;
}


/////////////////////////////////////////////////////////////////////////////
// File opener class.
//
class FxPlayerFileOpener : public FileOpener 
{
public:
    virtual File* OpenFile(const char *purl) 
    {
        return new SysFile(purl);
    }
};


/////////////////////////////////////////////////////////////////////////////
// Called from main() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
//
int FxPlayerTiny::Run()
{
    if (!SetupWindow(String(FXPLAYER_APP_TITLE " ") + FileName))
        return 1;
   
    // Create and Configure GFxLoader.   
    Loader loader;

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FxFileOpener derived class that loads files in a custom way.
    // Here FxFileOpener and FxFSCommandHandler are set for sample purposes.    
    Ptr<FileOpener> pfileOpener = *new FxPlayerFileOpener;
    loader.SetFileOpener(pfileOpener); 
    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    // add support for various image handlers
    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry();
#ifdef SF_ENABLE_LIBJPEG
    pimgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
    pimgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif
    pimgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
    pimgReg->AddHandler(&SF::Render::DDS::FileReader::Instance);
    loader.SetImageFileHandlerRegistry(pimgReg);

    // AS2 support
    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);

    // AS3 support
    Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
    loader.SetAS3Support(pASSupport);

    // Load the movie file and create its instance.
    if (!(pMovieDef = *loader.CreateMovie(FileName)))
        return 1; 

    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pCommandQueue = queue;

    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return 1;

    pMovie->SetMouseCursorCount(1);

    hMovieDisplay = pMovie->GetDisplayHandle();

    pRenderHAL = *new Render::PS3::HAL();

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    // See RendererPS3.h for more parameters.
    if (!pRenderHAL->InitHAL(Render::PS3::HALInitParams(GcmCtx, pMemManager )))
        return 1;

    // Create renderer.
	if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
		return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Create the default rendertarget. This is required on PS3 (if you want to use render targets),
    // because there is no way to query the render surface in GCM. If this is not done, Push/PopRenderTarget
    // in HAL will all fail. One instance with each buffer address is needed to swap buffers each frame
    pDisplayRT = SF_NEW RenderTarget(0, RBuffer_Default, ImageSize(Surface.width, Surface.height) );
    Surface.colorOffset[0] = Buffers[Frame];

    PS3::RenderTargetData::UpdateData(pDisplayRT, Surface, 0, Surface.height);
    if ( !pRenderHAL->SetRenderTarget(pDisplayRT))
        return 1;

    // Set renderer on loader so that it is also applied to all children.
    //pRenderConfig = *new RenderConfig(pRenderer, RenderConfig::RF_EdgeAA);
    //loader.SetRenderConfig(pRenderConfig);
    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // If you wanted to use the movie as a transparent HUD overlay, you would
    // set Background Alpha to 0. We don't need to do this for player sample.
    // pMovie->SetBackgroundAlpha(0.0f);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = Timer::GetTicks()/1000;

    // Application / Player message loop.
    while(1)
    {
        if (!ProcessMessages())
            break;

        AdvanceAndDisplay();
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Advances Fx animation and draws the scene.
//
void FxPlayerTiny::AdvanceAndDisplay()
{
    cellGcmSetSurface(GcmCtx, &Surface);

    // Clear the frame buffer to black.
    cellGcmSetClearColor(GcmCtx, 0);
    cellGcmSetClearDepthStencil(GcmCtx, 0);
    cellGcmSetClearSurface(GcmCtx,
        CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A|CELL_GCM_CLEAR_S|CELL_GCM_CLEAR_Z);

    // *** Fx Rendering    
    if (pMovie)
    {
        // Set Wireframe
        cellGcmSetBackPolygonMode(GcmCtx,  Wireframe ? CELL_GCM_POLYGON_MODE_LINE : CELL_GCM_POLYGON_MODE_FILL);
        cellGcmSetFrontPolygonMode(GcmCtx, Wireframe ? CELL_GCM_POLYGON_MODE_LINE : CELL_GCM_POLYGON_MODE_FILL);

        UInt32  time    = Timer::GetTicks()/1000;
        float   delta   = ((float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

        Display();

        MovieLastTime = time;
    }

    while (cellGcmGetFlipStatus() != 0)
        sys_timer_usleep(100);

    cellGcmResetFlipStatus();
    cellGcmSetFlip(GcmCtx, Frame);
    cellGcmFlush(GcmCtx);
    cellGcmSetWaitFlip(GcmCtx);

    Frame = !Frame;
    Surface.colorOffset[0] = Buffers[Frame];
    PS3::RenderTargetData::UpdateData(pDisplayRT, Surface, 0, Surface.height);

    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());
}

void FxPlayerTiny::Display()
{
    pRenderer->BeginFrame();
    if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
    {
        pRenderer->Display(hMovieDisplay);
    }
    // Release resources that cannot be modified while rendering the same frame.
    // Needed for VMConfig_FastTexUpdate
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
#if 0
    else if (keyCode >= VK_F1 && keyCode <= VK_F15)
    {
        key = (Key::Code) ((keyCode - VK_F1) + Key::F1);
    }
#endif
    else if (keyCode >= '0' && keyCode <= '9')
    {
        key = (Key::Code) ((keyCode - '0') + 48);
    }
#if 0
    else if (keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
    {
        key = (Key::Code) ((keyCode - VK_NUMPAD0) + Key::KP_0);
    }
#endif
    else
    {
        // Use a look-up table for keys don't correlate in order,.
        static struct {
            int          vk;
            Key::Code gs;
        } table[] =
        {
#if 0
            { VK_SHIFT,     Key::Shift },
            { VK_RSHIFT,    Key::Shift },
            { VK_CONTROL,   Key::Control },
            { VK_RCONTROL,  Key::Control },
            { VK_MENU,      Key::Alt },
            { VK_RMENU,     Key::Alt },
#endif
            { VK_RETURN,    Key::Return },
            { VK_ESCAPE,    Key::Escape },
            { VK_LEFT,      Key::Left },
            { VK_UP,        Key::Up },
            { VK_RIGHT,     Key::Right },
            { VK_DOWN,      Key::Down },
#if 0
            { VK_SPACE,     Key::Space },
            { VK_BACK,      Key::Backspace },
            { VK_DELETE,    Key::Delete },
            { VK_INSERT,    Key::Insert },
#endif
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
    switch(keyCode)
    {
    case 'w':
    case 'a':
    case 's':
    case 'd':
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
        HandleKeyEvent(keyCode, downFlag);
        return;

    case 'X':
        HandleKeyEvent(VK_RETURN, downFlag);
        return;

    case 'O':
        HandleKeyEvent(VK_ESCAPE, downFlag);
        return;
    }

    if (!downFlag)
        return;

    switch(keyCode)
    {
    case 'T':
        // Toggle wireframe.
        Wireframe = !Wireframe;
        break;

    case 'L':
#if 0
        if (--CurFile<0)
            CurFile = numFiles-1;
        LoadFile(CurFile);
#endif
#if 0
        AAEnabled = ! AAEnabled;
        {
            UInt32 rendererFlags = pRenderConfig->GetRenderFlags() &
                ~GFxRenderConfig::RF_EdgeAA;
            if (AAEnabled)
                rendererFlags |= GFxRenderConfig::RF_EdgeAA;
            pRenderConfig->SetRenderFlags(rendererFlags);
        }
#endif
        break;

    case '\n':
        Paused = !Paused;
        break;

    case 'R':
#if 0
        if (++CurFile>=numFiles)
            CurFile = 0;
        LoadFile(CurFile);
#endif
        break;
    }

}




/////////////////////////////////////////////////////////////////////////////
// Setup Application Window
//
bool FxPlayerTiny::SetupWindow(const String& name)
{
    UPInt cmdsize = 262144;
    UPInt mainsize = 32*1024*1024;
    void *mm = memalign(1 << 20, mainsize);

    if (CELL_OK != cellGcmInit(cmdsize, mainsize, mm))
        return 0;

    CellGcmConfig cfg;
    cellGcmGetConfiguration(&cfg);

    if (cfg.localAddress == 0 || cfg.localSize == 0)
        return 0;

    pMemManager = new PS3::MemoryManager(Memory::GetGlobalHeap(), cfg.localAddress, cfg.localSize,
        ((UByte*)mm)+cmdsize, mainsize-cmdsize);

    CellVideoOutState         cvs;
    CellVideoOutResolution    cvr;
    CellVideoOutConfiguration cvc;

    cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &cvs);
    cellVideoOutGetResolution(cvs.displayMode.resolutionId, &cvr);

    memset(&cvc, 0, sizeof(cvc));
    cvc.resolutionId = cvs.displayMode.resolutionId;
    cvc.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
    cvc.pitch = (cvr.width * 4 + 63) & ~63;
    unsigned pitch = cvc.pitch;
//    int width = cvr.width;
    int height = cvr.height;

    if (CELL_OK != cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &cvc, 0, 0))
        return 0;

    cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);

    UInt32 depthofs;

    void *hColor0 = pMemManager->Alloc(pitch * height, 0x100, Memory_PS3_RSX);
    void *hColor1 = pMemManager->Alloc(pitch * height, 0x100, Memory_PS3_RSX);
    void *hDepth  = pMemManager->Alloc(pitch * height, 0x100, Memory_PS3_RSX);

    cellGcmAddressToOffset(hColor0, &Buffers[0]);
    cellGcmAddressToOffset(hColor1, &Buffers[1]);
    cellGcmAddressToOffset(hDepth, &depthofs);

    if (CELL_OK != cellGcmSetDisplayBuffer(0, Buffers[0], cvc.pitch, cvr.width, cvr.height))
        return 0;
    if (CELL_OK != cellGcmSetDisplayBuffer(1, Buffers[1], cvc.pitch, cvr.width, cvr.height))
        return 0;

    Surface.colorFormat         = CELL_GCM_SURFACE_A8R8G8B8;
    Surface.colorTarget         = CELL_GCM_SURFACE_TARGET_0;
    Surface.colorLocation[0]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorOffset[0]      = Buffers[0];
    Surface.colorPitch[0]       = cvc.pitch;

    Surface.colorLocation[1]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorLocation[2]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorLocation[3]    = CELL_GCM_LOCATION_LOCAL;
    Surface.colorOffset[1]      = 0;
    Surface.colorOffset[2]      = 0;
    Surface.colorOffset[3]      = 0;
    Surface.colorPitch[1]       = 64;
    Surface.colorPitch[2]       = 64;
    Surface.colorPitch[3]       = 64;

    Surface.depthFormat         = CELL_GCM_SURFACE_Z24S8;
    Surface.depthLocation       = CELL_GCM_LOCATION_LOCAL;
    Surface.depthOffset         = depthofs;
    Surface.depthPitch          = cvc.pitch;

    Surface.type                = CELL_GCM_SURFACE_PITCH;
    Surface.antialias           = CELL_GCM_SURFACE_CENTER_1;

    Surface.width               = cvr.width;
    Surface.height              = cvr.height;
    Surface.x                   = 0;
    Surface.y                   = 0;

    Width = cvr.width;
    Height = cvr.height;

    GcmCtx = gCellGcmCurrentContext;

    cellPadInit (1);

    cellGcmSetSurface(GcmCtx, &Surface);

    float scale[4], offset[4];
    scale[0]    = Width * 0.5f;
    scale[1]    = Height * 0.5f;
    offset[0]   = Width * 0.5f;
    offset[1]   = Height * 0.5f;
    scale[2]    = offset[2] = 0.5f;
    scale[3]    = offset[3] = 0;

    cellGcmSetViewport(GcmCtx, 0,0, Width, Height, 0, 1, scale, offset);
    cellGcmSetScissor(GcmCtx, 0,0, 4096,4096);

    cellGcmSetClearColor(GcmCtx, 0x808080FF);
    cellGcmSetClearDepthStencil(GcmCtx, 0);
    cellGcmSetClearSurface(GcmCtx,
        CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A|CELL_GCM_CLEAR_S|CELL_GCM_CLEAR_Z);

    cellGcmResetFlipStatus();
    cellGcmSetFlip(GcmCtx, 0);
    cellGcmFlush(GcmCtx);
    cellGcmSetWaitFlip(GcmCtx);
    cellGcmFinish(GcmCtx, 0);
    Frame = 1;

    Surface.colorOffset[0] = Buffers[Frame];
    cellGcmSetSurface(GcmCtx, &Surface);

    return 1;
}


/////////////////////////////////////////////////////////////////////////////
// Message processing function to be called in the  application loops.
//

static int keymap[2][8] = {{' ', 'c', 'v', '\n', VK_UP, VK_RIGHT, VK_DOWN, VK_LEFT}, {'L', 'R', 'l', 'r', 'T', 'O', 'X', 'S'}};

bool FxPlayerTiny::ProcessMessages()
{
#define GFX_GAMEPAD_DEADZONE 16
    CellPadData pad;

    if (!cellPadGetData (0, &pad) && pad.len >= 3)
    {
        for (int i = 2; i <= 3; i++)
            for (int j = 0; j < 8; j++)
                if ((pad0.button[i] & (1 << j)) != (pad.button[i] & (1 << j)))
                    OnKey (keymap[i-2][j], (pad.button[i] & (1 << j)) ? 1 : 0);

        // map left stick to WASD

        // LEFT STICK HORIZ
        bool bDKey      = ((pad.button[6]  & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
        bool bDKeyPrev  = ((pad0.button[6] & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
        if ( bDKey != bDKeyPrev )
            OnKey('d', bDKey);

        bool bAKey      = ((pad.button[6]  & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE);
        bool bAKeyPrev  = ((pad0.button[6] & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE); 
        if (bAKey != bAKeyPrev)
            OnKey('a', bAKey);

        // LEFT STICK VERT
        bool bWKey      = ((pad.button[7]  & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
        bool bWKeyPrev  = ((pad0.button[7] & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
        if ( bWKey != bWKeyPrev  )
            OnKey('s', bWKey);

        bool bSKey      = ((pad.button[7]  & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE);
        bool bSKeyPrev  = ((pad0.button[7] & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE); 
        if ( bSKey != bSKeyPrev)
            OnKey('w', bSKey);

        pad0 = pad;
    }

    return 1;

}


/////////////////////////////////////////////////////////////////////////////
// ***** main() -  Application entry point.

// The main function implementation creates the FxPlayerTiny class,
// passes it the filename and calls Run. FxPlayerTiny::Run is responsible
// for all of the rendering setup and movie playback.
//
int main(int argc, char *argv[])
{
    GFx::System gfxInit;

    int result = 1;
    {  
        FxPlayerTiny player(FXPLAYER_FILENAME);   
        result = player.Run();
    }

    return result;
}


