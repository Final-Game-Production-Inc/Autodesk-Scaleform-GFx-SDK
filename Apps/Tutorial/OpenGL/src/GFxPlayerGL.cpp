///**************************************************************************
//
//Filename    :   GFxPlayer.cpp
//Content     :   Integration sample for OpenGL.
//Created     :   November 1, 2012
//Authors     :   Dmitry Polenur
//
//Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.
//
//Use of this software is subject to the terms of the Autodesk license
//agreement provided at the time of installation or download, or which
//otherwise accompanies this software in either electronic or hard copy form.
//
//**************************************************************************/

#include "GFxPlayerGL.h"
#include "GFx_Renderer_GL.h"

// This file must be included exactly once in every AS3 enabled application.
#include "GFx/AS3/AS3_Global.h"

#ifdef SF_OS_WIN32
#include "GFx_FontProvider_Win32.h"
#endif

// SWF Filename, which will load by default.
static const char*      FXPLAYER_FILENAME = "OGLguideAS3.swf";

// These parameters have been extracted from the existing demo, and are toggled via the GFx UI.
extern bool s_drawWireframe;
extern bool s_drawControlMesh;
extern bool s_drawGlutTeapot;
extern bool s_animate;

namespace Scaleform {

// This class implements callback handlers from Actionscript. Inside the OpenGLHUD.as file, the callbacks are performed,
// and caught here for handling. 
class DemoExternalInterfaceHandler : public GFx::ExternalInterface
{
public:
    virtual void Callback(GFx::Movie* pmovieView,
        const char* methodName,
        const GFx::Value* args,
        unsigned argCount)
    {
        // First, print out all information about the event that has been received.
        printf("ExternalInterface: %s, %d args: ", methodName, argCount);
        for(unsigned i = 0; i < argCount; i++)
        {
            switch(args[i].GetType())
            {
            case GFx::Value::VT_Null:
                printf("NULL");
                break;
            case GFx::Value::VT_Boolean:
                printf("%s", args[i].GetBool() ? "true" : "false");
                break;
            case GFx::Value::VT_Int:
                printf("%s", args[i].GetInt());
                break;
            case GFx::Value::VT_Number:
                printf("%3.3f", args[i].GetNumber());
                break;
            case GFx::Value::VT_String:
                printf("%s", args[i].GetString());
                break;
            default:
                printf("unknown");
                break;
            }
            printf("%s", (i == argCount - 1) ? "" : ", ");
        }
        printf("\n");

        // Now, perform handling, based on the method name.
        if(strcmp(methodName, "Wireframe") == 0 && argCount == 1 && args[0].GetType() == GFx::Value::VT_Boolean)
        {
            s_drawWireframe = args[0].GetBool();
        }
        else if(strcmp(methodName, "ControlMesh") == 0 && argCount == 1 && args[0].GetType() == GFx::Value::VT_Boolean)
        {
            s_drawControlMesh = args[0].GetBool();
        }
        else if(strcmp(methodName, "Teapot") == 0 && argCount == 1 && args[0].GetType() == GFx::Value::VT_Boolean)
        {
            s_drawGlutTeapot = args[0].GetBool();
        }
        else if(strcmp(methodName, "Pause") == 0)
        {
            s_animate = !s_animate;
        }
        else if(strcmp(methodName, "Quit") == 0)
        {
            exit(0);
        }
        else
        {
            SF_DEBUG_MESSAGE1(true, "Unexpected callback method received: %s\n", methodName);
        }
    }
};

// Operates as a single-threaded queue, so things are just executed immediately.
class FxPlayerThreadCommandQueue : public Render::ThreadCommandQueue
{
public:

    virtual void GetRenderInterfaces( Render::Interfaces* p ) 
    {
        p->pHAL = pHAL;
        p->pRenderer2D = pR2D;
        p->pTextureManager = pHAL->GetTextureManager();
        p->RenderThreadID = 0;
    }

    virtual void PushThreadCommand( Render::ThreadCommand* command ) 
    {
        if (command)
            command->Execute();
    }

    Render::HAL*        pHAL;
    Render::Renderer2D* pR2D;

};

class GFxPlayerLog : public GFx::Log
{
public:
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(Scaleform::LogMessageId messageId, const char* pfmt, va_list argList)
    {
        SF_UNUSED(messageId);
        // Output log to console
        vprintf(pfmt, argList);
    }
};



// ***** GFxPlayer Class Implementation
GFxPlayer::GFxPlayer()
{
}

GFxPlayer::~GFxPlayer()
{
    // The HAL must be shutdown prior to deleting the GL context. It will make calls to destroy resources, which cannot
    // be processed properly if the context no longer exists.
    if ( pRenderHAL )
    {
        Render::GL::HAL* phal = reinterpret_cast<Render::GL::HAL*>(pRenderHAL.GetPtr());
        phal->ShutdownHAL();
    }

    // Destroy the ThreadCommandQueue. It is the only non-POD member which is not reference counted, and must be destroyed
    // explicitly. Other members will have their reference counts decremented, and be subsequently destroyed automatically.
    delete pCommandQueue;
    pCommandQueue = 0;
}


int GFxPlayer::Init()
{       
    // Configure Loader.   
    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FileOpener derived class that loads files in a custom way.
    // Here FileOpener is set for demonstration purposes, but a default instance of this FileOpener
    // is installed in the loader already.
    Ptr<GFx::FileOpener> pfileOpener = *new GFx::FileOpener;
    mLoader.SetFileOpener(pfileOpener); 

    // Install the customized logging function, which prints output to the console.
    mLoader.SetLog(Ptr<GFx::Log>(*new GFxPlayerLog()));

#ifdef SF_OS_WIN32
    // Install the Win32 font providier, which can pull font definitions from installed system fonts.
	Ptr<GFx::FontProviderWin32> fontProvider = *new GFx::FontProviderWin32(::GetDC(0));
	mLoader.SetFontProvider(fontProvider);
#endif

    // Install the image handler libraries (with all available format handlers), which allows loading images from different types.
    Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    mLoader.SetImageFileHandlerRegistry(pimgReg);

    // Install the custom DemoExternalInterfaceHandler, which will process callbacks from the GFx content.
    Ptr<GFx::ExternalInterface> pEIHandler = *new DemoExternalInterfaceHandler;
    mLoader.SetExternalInterface(pEIHandler);

    // Install support for both AS2 and AS3 script. If only targetting one language, you can omit the unused one. Also, GFx can be
    // recompiled with GFX_AS2/AS3_SUPPORT undefined in GFxConfig.h, to reduce copmiled library size (source customers only).
	Ptr<GFx::ASSupport> pAS2Support = *new GFx::AS2Support();
	mLoader.SetAS2Support(pAS2Support);
	Ptr<GFx::ASSupport> pASSupport = *new GFx::AS3Support();
	mLoader.SetAS3Support(pASSupport);

    // Allocate the ThreadCommandQueue, which is an interface for sending rendering commands to the rendering thread. This sample is
    // single threaded, so the ThreadCommandQueue is very simple, executing all commands immediately. For multi-threaded execution,
    // the implementation of this class must be thread safe.
    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pCommandQueue = queue;

    // Create renderer.
	Ptr<Render::GL::HAL> phal = *new Render::GL::HAL(pCommandQueue);
    pRenderHAL = phal;
    pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr());
	if (!pRenderer)
		return 1;

    // Give the command queue pointers to the HAL and R2D
    queue->pHAL = pRenderHAL;
    queue->pR2D = pRenderer;

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    if (!phal->InitHAL(Render::GL::HALInitParams()))
        return 1;

    // Load the movie file into a GFx::MovieDef.
    pMovieDef = *mLoader.CreateMovie(FXPLAYER_FILENAME);
    if (!pMovieDef)
    {
        String alternateFilename = "../bin/";
        alternateFilename += FXPLAYER_FILENAME;
        if (!(pMovieDef = *mLoader.CreateMovie(alternateFilename)))
        {
            String errorString = "Unable to load file: ";
            errorString += FXPLAYER_FILENAME;
            MessageBoxA(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
            return 1;
        }
    }

    // Now create the GFx::Movie from the loaded GFx::MovieDef.
    pMovie = *pMovieDef->CreateInstance(GFx::MemoryParams(), 0, 0, pCommandQueue);
    if (!pMovie)
    {
        return 1;
    }

	hMovieDisplay = pMovie->GetDisplayHandle();

    // Indicate to the Movie that there is exactly one mouse.
    pMovie->SetMouseCursorCount(1);

    // If you wanted to use the movie as a transparent HUD overlay, you would set background alpha to zero. If you want to
    // see the Stage, then the background alpha should be higher than zero. In this sample, we do not want the stage to
    // obscure the 3D rendering, so we set its alpha to zero.
    pMovie->SetBackgroundAlpha(0.0f);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = timeGetTime();

    return 0;
}


//// Advances GFx animation and draws the scene.
void GFxPlayer::AdvanceAndDisplay()
{
    // Main GFx loop.
    if (pMovie)
    {
        // Advance time and display the movie.
        UInt32  time    = timeGetTime();
        float   delta   = ((float)(time - MovieLastTime)) / 1000.0f;

        pMovie->Advance(delta);

        // Render the GFx content.
        pRenderer->BeginFrame();
        if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
        {
            pRenderer->Display(hMovieDisplay);
        }
        pRenderer->EndFrame();

        // Store the last update time
        MovieLastTime = time;
    }

    // If AMP is connected, let it know the GFx frame has completed.
    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());
}

void    GFxPlayer::OnMouseEvent(GFx::Event::EventType eventType, unsigned button, int x, int y)
{
    // Pass mouse events (movement + clicks) to the GFx::Movie for processing.
    if (pMovie)
    {
        GFx::MouseEvent mevent(eventType, button, (float)x, (float)y);
        pMovie->HandleEvent(mevent);
    }
}

void GFxPlayer::OnResize(int w, int h)
{
    // On resizing of the window, make the GFx content fill the entire window.
    if (!pMovie)
        return;

    pMovie->SetViewport(w, h, 0,0, w, h);
}

bool GFxPlayer::IsHit( int x, int y )
{
    // Perform a hit-test on the movie's shapes, for the given coordinates.
    if (!pMovie)
        return false;

    return pMovie->HitTest((float)x, (float)y, GFx::Movie::HitTest_Shapes);
}

}; // namespace Scaleform.
