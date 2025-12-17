/**************************************************************************

Filename    :   GFxPlayerTinyXBox360.cpp
Content     :   Tiny GFxPlayer sample for XBox360.
Created     :   March 27, 2008
Authors     :   Michael Antonov, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// GFx Includes
#include "Kernel/SF_File.h"
#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"
//#include "GFx_Renderer_X360.h"
#include "GFxCompat.h"

#include "Render/X360/X360_HAL.h"
#include "Render/Renderer2D.h"

#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/DDS_ImageFile.h"

#include "GFx/AS3/AS3_Global.h"

// System Includes
#include "xtl.h"

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   "D:\\Samples\\Window.swf"
// Window title to use for the application.
#define FXPLAYER_APP_TITLE  "Scaleform GFxPlayerTinyXBox360 v" GFX_VERSION_STRING

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
    // Direct3D state variables.
    LPDIRECT3D9             pD3D;
    IDirect3DDevice9*       pDevice;
    D3DPRESENT_PARAMETERS   PresentParams;
    IDirect3DTexture9*      pFrameBuffer[2];
    UInt                    CurBuffer;
    IDirect3DTexture9*      pBackBuffer;

    // Scaleform GFx object pointers.
    GPtr<GFxMovieDef>       pMovieDef;
    GPtr<GFxMovieView>      pMovie;
	GPtr<Render::X360::HAL> pRenderHAL;
    Ptr<Render::Renderer2D> pRenderer;
	ThreadCommandQueue*     pCommandQueue;
    UInt32                  MovieLastTime;

	// Context to draw
	MovieDisplayHandle        hMovieDisplay;

    // Toggle states.
    bool                    Wireframe; 
    bool                    AAEnabled;
    bool                    Paused;         // Set when the movie is paused in the player.    

    // Input state and pad, if any.
    XINPUT_STATE            InputState;

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
    bool    ProcessMessages();
    void    OnKey(UInt keyCode, bool downFlag);
    void    HandleKeyEvent(UInt keyCode, bool downFlag);
    // System window and rendering setup.
    bool    SetupWindow(const GString& name);
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

    Wireframe       = 0; 
    AAEnabled       = 1;
    Paused          = 0;

    // Window size and filename.
    Width           = 640;
    Height          = 480;
    FileName        = pfilename;
}

FxPlayerTiny::~FxPlayerTiny()
{
    if (pDevice)
        pDevice->Release();
    if (pD3D)
        pD3D->Release();

	delete pCommandQueue;
}


// Called from main()/WindMain() after settings are initialized to execute
// most of the program logic. Responsible for setting up the window,
// loading movies and containing the playback/message loop.
int FxPlayerTiny::Run()
{
    if (!SetupWindow(GString(FXPLAYER_APP_TITLE " ") + FileName))
        return 1;
   
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
    //GPtr<GFxImageCreator> pimageCreator = *new GFxImageCreator(1);
    //loader.SetImageCreator(pimageCreator);

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

	// set states to add AS2 and AS3 support
	Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
	loader.SetAS2Support(pAS2Support);
    
	Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
	loader.SetAS3Support(pASSupport);

    // Load the movie file and create its instance.
    if (!(pMovieDef = *loader.CreateMovie(FileName)))
        return 1;
 
    
	FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
	pCommandQueue = queue;

	if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
		return 1;

    //Enable/Disable mouse support
    pMovie->SetMouseCursorCount(1);
    
	hMovieDisplay = pMovie->GetDisplayHandle();

     // Create renderer.
	pRenderHAL = *new Render::X360::HAL();
	if (!(pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr())))
		return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.

    if (!pRenderHAL->InitHAL(Render::X360::HALInitParams(pDevice, PresentParams, 0, 0, 0)))
        return 1;

    // Set renderer on loader so that it is also applied to all children.
    //pRenderConfig = *new GFxRenderConfig(pRenderer, GFxRenderConfig::RF_EdgeAA);
    //loader.SetRenderConfig(pRenderConfig);

    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0,0, Width, Height);

    // If you wanted to use the movie as a transparent HUD overlay, you would
    // set Background Alpha to 0. We don't need to do this for player sample.
    // pMovie->SetBackgroundAlpha(0.0f);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = GetTickCount();

    // Application / Player message loop.

    while(1)
    {
        if (!ProcessMessages())
            break;

       // Advance movie animation and render it to screen.
       AdvanceAndDisplay();

    }

    return 0;
}


// Advances GFx animation and draws the scene.
void FxPlayerTiny::AdvanceAndDisplay()
{
    // Clear the back buffer to a black color.
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

    // *** GFx Rendering    

    if (pMovie)
    {
        // GRendererD3D9 does not modify D3DRS_FILLMODE, so we can set it here.
        pDevice->SetRenderState(D3DRS_FILLMODE,
            Wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

        UInt32  time    = GetTickCount();
        Float   delta   = ((Float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

        Display();

        if (pDevice)
			pDevice->Present( NULL, NULL, NULL, NULL );

        MovieLastTime = time;
    }

    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());

    // If using predicated tiling, this would be needed to release resources
    // that cannot be modified while rendering the same frame
    // pRenderer->EndFrame();
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
void    FxPlayerTiny::HandleKeyEvent(UInt keyCode, bool downFlag)
{
    GFxKey::Code key(GFxKey::None);

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
        { 0, GFxKey::None }
    };

    for (int i = 0; table[i].vk != 0; i++)
    {
        if (keyCode == (UInt)table[i].vk)
        {
            key = table[i].gs;
            break;
        }
    }

    if (key != GFxKey::None)
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
    switch(keyCode)
    {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
        HandleKeyEvent(keyCode, downFlag);
        return;

        // A key is 'return'
    case 'A':
        HandleKeyEvent(VK_RETURN, downFlag);
        return;
    }

    if (!downFlag)
        return;

        switch(keyCode)
        {
        case 'Y':
            Wireframe = !Wireframe;
            break;

        //case 'L':
        //    AAEnabled = ! AAEnabled;
        //    {
        //        UInt32 rendererFlags = pRenderConfig->GetRenderFlags() &
        //                               ~GFxRenderConfig::RF_EdgeAA;
        //        if (AAEnabled)
        //            rendererFlags |= GFxRenderConfig::RF_EdgeAA;
        //        pRenderConfig->SetRenderFlags(rendererFlags);
        //    }
        //    break;

        case VK_PAUSE:
            Paused = !Paused;
            break;
        }

}


// Setup Application Window and initialize Direct3D.
bool FxPlayerTiny::SetupWindow(const GString& name)
{

    if (!pD3D)
        return 0;

    // Set up the structure used to create the D3DDevice    
    D3DPRESENT_PARAMETERS &d3dpp = PresentParams;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed                  = 0;    
    d3dpp.BackBufferFormat          = D3DFMT_A8R8G8B8;
    d3dpp.BackBufferCount           = 1;
    d3dpp.EnableAutoDepthStencil    = TRUE;
    d3dpp.AutoDepthStencilFormat    = D3DFMT_D24S8;
    d3dpp.BackBufferHeight          = Height;
    d3dpp.BackBufferWidth           = Width;
    d3dpp.SwapEffect                = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval      = D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.MultiSampleType           = D3DMULTISAMPLE_NONE;

    // Create the D3DDevice
    if( FAILED(pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice ) ) )
    {
        return 0;
    }

    return 1;
}

// Button bit to key maping
struct XBoxButtonToKey
{
    DWORD   Button;
    UInt    Key;
};

XBoxButtonToKey XBoxButtonToKey_Map[] =
{
    { XINPUT_GAMEPAD_BACK,          VK_BACK     },
    { XINPUT_GAMEPAD_START,         VK_PAUSE    },
    { XINPUT_GAMEPAD_A,             'A'         },
    { XINPUT_GAMEPAD_B,             'B'         },
    { XINPUT_GAMEPAD_X,             'X'         },
    { XINPUT_GAMEPAD_Y,             'Y'         },
    { XINPUT_GAMEPAD_RIGHT_SHOULDER,'N'         },
    { XINPUT_GAMEPAD_LEFT_SHOULDER, 'P'         },
    { XINPUT_GAMEPAD_RIGHT_THUMB,	'W'			},
    { XINPUT_GAMEPAD_LEFT_THUMB,	'Q'			},
    { XINPUT_GAMEPAD_DPAD_UP,       VK_UP       },
    { XINPUT_GAMEPAD_DPAD_DOWN,     VK_DOWN     },
    { XINPUT_GAMEPAD_DPAD_LEFT,     VK_LEFT     },
    { XINPUT_GAMEPAD_DPAD_RIGHT,    VK_RIGHT    },
    { 0, 0 }
};

// Message processing function to be called in the 
// application loops until this returns 0.
bool FxPlayerTiny::ProcessMessages()
{
    XINPUT_STATE newState;
    SInt mbutton = -1;
    SInt rthumbbutton = -1;

    if (XInputGetState(0, &newState) == ERROR_SUCCESS)
    {
        // Send out fake keystrokes for buttons
        for (int i=0; XBoxButtonToKey_Map[i].Button != 0; i++)
        {
            if (!(InputState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button) &&
                (newState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button))
            {
                OnKey(XBoxButtonToKey_Map[i].Key, 1);
            }
            else if ((InputState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button) &&
                !(newState.Gamepad.wButtons & XBoxButtonToKey_Map[i].Button))
            {
                OnKey(XBoxButtonToKey_Map[i].Key, 0);
            }
        }       

        if (newState.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
            InputState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            OnKey('L', 1);
        else if (newState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
            InputState.Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            OnKey('L', 0);

        if (newState.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
            InputState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            OnKey('R', 1);
        else if (newState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD &&
            InputState.Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            OnKey('R', 0);

        if ((newState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) !=
            (InputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB))
            mbutton = (newState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0;

        if ((newState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) !=
            (InputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB))
            rthumbbutton = (newState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0;

        if (newState.Gamepad.sThumbRX > 0)
        {
            newState.Gamepad.sThumbRX -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbRX < 0)
                newState.Gamepad.sThumbRX = 0;
        }
        else if (newState.Gamepad.sThumbRX < 0)
        {
            newState.Gamepad.sThumbRX += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbRX > 0)
                newState.Gamepad.sThumbRX = 0;
        }
        if (newState.Gamepad.sThumbRY > 0)
        {
            newState.Gamepad.sThumbRY -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbRY < 0)
                newState.Gamepad.sThumbRY = 0;
        }
        else if (newState.Gamepad.sThumbRY < 0)
        {
            newState.Gamepad.sThumbRY += XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbRY > 0)
                newState.Gamepad.sThumbRY = 0;
        }
        if (newState.Gamepad.sThumbLX > 0)
        {
            newState.Gamepad.sThumbLX -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbLX < 0)
                newState.Gamepad.sThumbLX = 0;
        }
        else if (newState.Gamepad.sThumbLX < 0)
        {
            newState.Gamepad.sThumbLX += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbLX > 0)
                newState.Gamepad.sThumbLX = 0;
        }
        if (newState.Gamepad.sThumbLY > 0)
        {
            newState.Gamepad.sThumbLY -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbLY < 0)
                newState.Gamepad.sThumbLY = 0;
        }
        else if (newState.Gamepad.sThumbLY < 0)
        {
            newState.Gamepad.sThumbLY += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + 200;
            if (newState.Gamepad.sThumbLY > 0)
                newState.Gamepad.sThumbLY = 0;
        }

        memcpy(&InputState, &newState, sizeof(XINPUT_STATE));
    }           
    XINPUT_KEYSTROKE key;
    XInputGetKeystroke(0, XINPUT_FLAG_KEYBOARD, &key);
    if( key.Flags & XINPUT_KEYSTROKE_KEYDOWN )
    {
        OnKey(key.Unicode, 1);
    }
    return 1;
}


// ***** main() -  Application entry point.

// The main function implementation creates the FxPlayerTiny class,
// passes it the filename and calls Run. FxPlayerTiny::Run is responsible
// for all of the rendering setup and movie playback.

int main(int argc, char *argv[])
{
    GFxSystem gfxInit;

    int result = 1;
    {  
        FxPlayerTiny player(FXPLAYER_FILENAME);   
        result = player.Run();
    }

    return result;
}
