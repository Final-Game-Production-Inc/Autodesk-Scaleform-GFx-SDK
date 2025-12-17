/**************************************************************************

Filename    :   FxPlayerTextureInSWFWii.cpp
Content     :   Sample GFx SWF file player for Wii
Created     :
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


// GFx includes
#include "GFile.h"
#include "GFxPlayer.h"
#include "GFxLoader.h"
#include "GFxLog.h"
#include "GTimer.h"
#include "GImageInfo.h"
#include "GFxImageResource.h"
#include "GFxRenderConfig.h"

#include <stdlib.h>
#include <stdio.h>

#include <revolution/gx.h>

// **** Fx Player defines

#include "../Common/WiiApp.h"
#include "../Common/MathLib.h"
#define FXPLAYER_APP                WiiApp

// The path that will be searched for files
// - FXPLAYER_FILENAME must be located in this path
#define FXPLAYER_FILEDIRECTORY      "FxPlayer/"
#define FXPLAYER_FILENAME           "Window_texture.swf"
#define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
#define FXPLAYER_FONT_SIZE          10

#define FXPLAYER_APP_TITLE          "Scaleform GFxPlayer Wii v" GFC_FX_VERSION_STRING


#include "../../../Bin/FxPlayer/fxplayer.swf.h"

// ***** Player Settings class

// Settings class stores playback settings determined
// based on the comand-line parameters
class   FxPlayerSettings
{

public:
    enum AAModeType
    {
        AAMode_None,        // No anti-aliasing is used.
        AAMode_EdgeAA,      // Edge AA is used (if supported by GRenderer).
        AAMode_FSAA         // HW Full-screen AA is used (if supported by device).
    };

    UInt        BitDepth;
    Float       ScaleX, ScaleY;
    Float       TexLodBias;
    AAModeType  AAMode;
    bool        Background;
    bool        MeasurePerformance;
    bool        FullScreen;

    enum    HudStateType
    {
        Hud_Hidden,
        Hud_Stats,
        Hud_Help
    };

    // Display Hud at startup
    HudStateType    HudState;

    // Verbose options
    bool    VerboseParse;
    bool    VerboseParseShape;
    bool    VerboseParseAction;
    bool    VerboseAction;
    bool    Quiet;
    bool    NoActionErrors;

    // Rendering state
    bool    DoLoop;
    bool    DoRender;
    bool    DoSound;

    // Set to play movie as fast as possible
    bool    FastForward;

    Float   ExitTimeout;
    UInt    SleepDelay;

    // PlaybackFile
    char    FileName[256];
    char    ShortFileName[256];

    FxPlayerSettings()
    {
        // Default values
        ScaleX = ScaleY     = 1.0f;
        TexLodBias          = -0.5f;
        AAMode              = AAMode_EdgeAA;
        BitDepth            = 32;
        Background          = 1;
        MeasurePerformance  = 1;
        FullScreen          = 1;
        HudState            = Hud_Hidden;
        //HudState            = Hud_Stats;

        VerboseParse        = 0;
        VerboseParseShape   = 0;
        VerboseParseAction  = 0;
        VerboseAction       = 0;
        Quiet               = 0;
        NoActionErrors      = 0;

        DoLoop              = 1;
        DoRender            = 1;
        DoSound             = 0;

        FastForward         = 0;

        ExitTimeout         = 0.0f;
        SleepDelay          = 31;

        // Clear file
        FileName[0]         = 0;
        ShortFileName[0]    = 0;
    }

};



// ***** Player Application class

class   FxPlayerApp : public FXPLAYER_APP
{
public:
    typedef FxPlayerSettings::AAModeType AAModeType;

    // Loaded movie data
    GFxLoader           Loader;
    GFxMovieInfo        MovieInfo;
    GPtr<GFxMovieDef>   pMovieDef;
    GPtr<GFxMovieView>  pMovie;

    // Movie timing state
    float               SpeedScale;         // Advance speed, def 1.0f
    SInt                FrameCounter;       // Frames rendered, for FPS
    // Time ticks: always rely on a timer, for FPS
    UInt32              TimeStartTicks;     // Ticks during the start of playback
    UInt32              TimeTicks;          // Current ticks
    UInt32              LastLoggedFps;      // Time ticks during last FPS log
    // Movie logical ticks: either timer or setting controlled
    UInt32              MovieStartTicks;
    UInt32              MovieLastTicks;
    UInt32              MovieTicks;

    // Renderer we use
    GPtr<GFxRenderConfig> pRenderConfig;
    GPtr<GFxRenderStats>  pRenderStats;

    // Selected playback settings
    FxPlayerSettings    Settings;

    // View width and height
    SInt                ViewWidth, ViewHeight;

    // Set if wireframe ins enabled.
    bool                Wireframe;
    bool                MouseEnabled, MouseCursor;

    // Scale toggle, on by default
    bool                ScaleEnable;
    // This variable is set when the movie is paused in the player.
    bool                Paused;
    // Store playstate when paused, so that we can restore it.
    GFxMovie::PlayState PausedState;

    // Last FPS and stats
    Float               LastFPS;
    GRenderer::Stats    LastStats;
    UInt                LastFrame; // Frame reported by HUD
    // This flag is set when UpdateHudText needs to be called
    bool                NeedHudUpdate;
    // Hud text, blended over the player
    char                HudText[2048];
    char                MessageText[1024];
    GViewport           HudViewport;
    GPtr<GFxMovieView>  pHud;

    // Curve error
    Float               CurvePixelError;

    // Width, height during sizing
    SInt                SizeWidth, SizeHeight;
    bool                SizingEntered;

    GXTexObj                RenderTexture;
    void*                   pRTexData;
    SInt                    RTWidth, RTHeight;
    Float                   MeshRotation;
    UInt                    LastRotationTick;
    static bool             CubeWireframe;

    FxPlayerApp();
    ~FxPlayerApp();

    // Called from main() after settings are initialized to execute
    // most of the program logic. Responsible for setting up the window,
    // loading movies and containing the message loop.
    SInt            Run();

    // Load a new movie from a file and initialize timing
    bool            LoadMovie(char *pfilename);


    // Helper function to update HUD.
    // Uses LastFPS and LastStats; those variables must be updated separately.
    void            UpdateHudText();
    // Updates the view size based on the ScaleEnable flag and window size.
    void            UpdateViewSize();

    bool            SetupRTTexture();
    void            RenderMesh();

    // *** Overrides

    // Sizing; by default, re-initalizes the renderer
    virtual void    OnSize(SInt w, SInt h);
    virtual void    OnSizeEnter(bool enterSize);
    virtual void    OnDropFiles(char *path);

    // Input
    virtual void    OnPad(PadKeyCode keyCode, bool downFlag);
    virtual void    OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, int x, int y, 
        unsigned int mods);
    virtual void    OnMouseMove(unsigned mouseIndex, int x, int y, int unsigned mods);

    // Helper used to convert key codes and route them to GFxPlayer
    void            KeyEvent(GFxKey::Code keyCode, bool down);
};



class GFxPlayerLog : public GFxLog
{
public:
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        // Output log to console
        vprintf(pfmt, argList);
    }
};

// File opener class.
class FxPlayerFileOpener : public GFxFileOpener
{
public:
    virtual GFile* OpenFile(const char *purl)
    {
        if (!strcmp(purl, "  fxplayer.swf"))
            return new GMemoryFile(purl, fxplayer_swf, sizeof(fxplayer_swf));

        return GFxFileOpener::OpenFile(purl);
    }
};


// "fscommand" callback, handles notification callbacks from ActionScript.
class FxPlayerFSCallback : public GFxFSCommandHandler
{
public:
    virtual void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg)
    {
        GFxLog *plog = pmovie->GetLog();
        if (plog)
        {
            plog->LogMessage("FsCallback: '");
            plog->LogMessage(pcommand);
            plog->LogMessage("' '");
            plog->LogMessage(parg);
            plog->LogMessage("'\n");
        }
    }
};


class FxPlayerUserEventHandler : public GFxUserEventHandler
{
    FxPlayerApp* pApp;
public:

    FxPlayerUserEventHandler(FxPlayerApp *papp)
    {
        pApp = papp;
    }

    virtual void HandleEvent(GFxMovieView* pmovie, const GFxEvent& event)
    {
        switch(event.Type)
        {
        case GFxEvent::DoShowMouse:
            pApp->MouseCursor = 1;
            break;
        case GFxEvent::DoHideMouse:
            pApp->MouseCursor = 0;
            break;

        default:
            break;
        }
    }
};

// ***** Main function implementation

int main(int argc, char *argv[])
{
    WiiApp::InitMain();

	GFxSystem gfxInit;

    int res = 1;
    {
        FxPlayerApp app;
        res = app.Run();
    }

    GMemory::DetectMemoryLeaks();
    return res;
}



// ***** FxPlayerApp Implementation

FxPlayerApp::FxPlayerApp()
{
    Wireframe           = 0;
    // Scale toggle, on by default
    ScaleEnable         = 1;
    Paused              = 0;
    PausedState         = GFxMovie::Playing;

    // Clear timing
    SpeedScale          = 1.0f;
    FrameCounter        = 0;
    TimeStartTicks      = 0;
    TimeTicks           = 0;
    LastLoggedFps       = 0;
    MovieStartTicks     = 0;
    MovieLastTicks      = 0;
    MovieTicks          = 0;

    LastFPS             = 0.0f;
    LastFrame           = 0;
    NeedHudUpdate       = 1;
    HudText[0]          = 0;

    ViewWidth           =
    ViewHeight          = 0;

    SizingEntered       = 0;

    CurvePixelError     = 1.0f;

    MouseEnabled        = 0;
    MouseCursor         = 0;

    MeshRotation            = 0;
    LastRotationTick        = 0;
    CubeWireframe           = 0;
}

bool FxPlayerApp::CubeWireframe;

FxPlayerApp::~FxPlayerApp()
{
}

struct Vertex
{
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
    {-1.0f, 1.0f,-1.0f},
    { 1.0f, 1.0f,-1.0f},
    {-1.0f,-1.0f,-1.0f},
    { 1.0f,-1.0f,-1.0f},

    {-1.0f, 1.0f, 1.0f},
    {-1.0f,-1.0f, 1.0f},
    { 1.0f, 1.0f, 1.0f},
    { 1.0f,-1.0f, 1.0f},

    {-1.0f, 1.0f, 1.0f},
    { 1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f,-1.0f},
    { 1.0f, 1.0f,-1.0f},

    {-1.0f,-1.0f, 1.0f},
    {-1.0f,-1.0f,-1.0f},
    { 1.0f,-1.0f, 1.0f},
    { 1.0f,-1.0f,-1.0f},

    { 1.0f, 1.0f,-1.0f},
    { 1.0f, 1.0f, 1.0f},
    { 1.0f,-1.0f,-1.0f},
    { 1.0f,-1.0f, 1.0f},

    {-1.0f, 1.0f,-1.0f},
    {-1.0f,-1.0f,-1.0f},
    {-1.0f, 1.0f, 1.0f},
    {-1.0f,-1.0f, 1.0f},
};

void    FxPlayerApp::RenderMesh()
{
    // Configure RT for texture
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetDstAlpha(GX_DISABLE, 0);
    GXSetCopyFilter(GX_FALSE, 0, GX_FALSE, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    Clear(0);

    UInt    ticks = GTimer::GetTicks()/1000;

    // 1/10 revolution per second
    float dt = 0.0f;
    {
        float t  = (float)((double)fmod((double)ticks, 20000.0) / 20000.0) * 360;
        float lt = (float)((double)fmod((double)LastRotationTick, 20000.0) / 20000.0) * 360;
        dt = t - lt;
    }

    LastRotationTick    = ticks;
    MeshRotation        += dt;

    if (MeshRotation > 360)
        MeshRotation -= 360;
    if (MeshRotation < 0.0f)
        MeshRotation += 360;

    GXSetViewport(0,0, RTWidth, RTHeight, 0.0f, 1.0f);
    GXSetScissor(0,0, RTWidth, RTHeight);

    Mtx44 proj;
    Mtx mv, rot, lat, lap;

    MTXPerspective(proj, 45.0, Float(RTWidth)/Float(RTHeight), 0.1f, 100);
    MTXTrans(lat, 0, -1, -5.5);

    MTXRotDeg(rot, 'x', 15);
    MTXConcat(lat, rot, lap);
    MTXRotDeg(rot, 'y', MeshRotation);
    MTXConcat(lap, rot, mv);

    GXSetProjection(proj, GX_PERSPECTIVE);
    GXLoadPosMtxImm(mv, GX_PNMTX2);
    GXSetCurrentMtx(GX_PNMTX2);

    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode( GX_CULL_FRONT );
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    GXColor colors[] = {
                {180,0,0,255},
                {180,180,0,255},
                {0,180,180,255},
                {0,180,0,255},
                {0,0,180,255},
                {180,0,180,255}};

    for (int i = 0; i < 6; i++)
    {
        GXSetChanMatColor(GX_COLOR0A0, colors[i]);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT7, 4);
            GXPosition3f32(g_cubeVertices[4*i+0].x, g_cubeVertices[4*i+0].y, g_cubeVertices[4*i+0].z);
            GXPosition3f32(g_cubeVertices[4*i+1].x, g_cubeVertices[4*i+1].y, g_cubeVertices[4*i+1].z);
            GXPosition3f32(g_cubeVertices[4*i+2].x, g_cubeVertices[4*i+2].y, g_cubeVertices[4*i+2].z);
            GXPosition3f32(g_cubeVertices[4*i+3].x, g_cubeVertices[4*i+3].y, g_cubeVertices[4*i+3].z);
        GXEnd();
    }

    // Copy EFB to texture
    GXDrawDone();
    GXSetTexCopySrc(0, 0, RTWidth, RTHeight);
    GXSetTexCopyDst(RTWidth, RTHeight, GX_TF_RGBA8, GX_FALSE);
    GXCopyTex(pRTexData, GX_FALSE);
    GXPixModeSync();
    GXInvalidateTexAll();

    // Restore original RT config
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetCopyFilter(pRmode->aa, pRmode->sample_pattern, GX_TRUE, pRmode->vfilter);

    GXSetScissor(0.0F, 0.0F, (float)pRmode->fbWidth, (float)pRmode->efbHeight);
}

bool    FxPlayerApp::SetupRTTexture()
{
    UInt tsize = GXGetTexBufferSize(RTWidth,RTHeight, GX_TF_RGBA8, GX_FALSE, 1);
    pRTexData = OSAlloc(tsize);
    GXInitTexObj(&RenderTexture, pRTexData, RTWidth,RTHeight, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);

    if (pMovie)
    {
        GFxResource*	  pres = pMovie->GetMovieDef()->GetResource("texture1");
        GFxImageResource* pimageRes = 0;
        if (pres && pres->GetResourceType() == GFxResource::RT_Image)
            pimageRes = (GFxImageResource*)pres;

        if (pimageRes)    
        {
            // We know that the imageInfo is GImageInfo since we didn't override image creator.
            GImageInfo* pimageInfo = (GImageInfo*)pimageRes->GetImageInfo();

            if (pimageInfo)
            {
                GPtr<GTextureWii> ptexture = *pUserRenderer->CreateTexture();
                ptexture->InitTexture(&RenderTexture);
                pimageInfo->SetTexture(ptexture);
            }
        }
    }

    return 1;
}

SInt    FxPlayerApp::Run()
{
    // Set the verbose flags.
    UInt       verboseFlags = 0;

    if (Settings.VerboseParse)
        verboseFlags |= GFxParseControl::VerboseParse;
    if (Settings.VerboseParseShape)
        verboseFlags |= GFxParseControl::VerboseParseShape;
    if (Settings.VerboseParseAction)
        verboseFlags |= GFxParseControl::VerboseParseAction;

    GPtr<GFxParseControl> pparseControl = *new GFxParseControl(verboseFlags);
    Loader.SetParseControl(pparseControl);

    // File callback.
    GPtr<GFxFileOpener> pfileOpener = *new FxPlayerFileOpener;
    Loader.SetFileOpener(pfileOpener);

    // Set log, but only if not quiet
    if (!Settings.Quiet)
        Loader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));

    // Modification of the existing font cache manager
    GFxFontCacheManager::TextureConfig fontCacheConfig;
    fontCacheConfig.TextureWidth   = 512;
    fontCacheConfig.TextureHeight  = 512;
    fontCacheConfig.MaxSlotHeight  = 32;
    //    fontCacheConfig.SlotPadding    = 2;
    //    fontCacheConfig.MaxNumTextures = 1;
    Loader.GetFontCacheManager()->SetTextureConfig(fontCacheConfig);
    Loader.GetFontCacheManager()->EnableDynamicCache(true);
    Loader.GetFontCacheManager()->SetMaxRasterScale(1.25f);

    // Only short name in settings
    strcpy(Settings.FileName, FXPLAYER_FILENAME);

    bool loadMovie = strlen(Settings.FileName)>0;

    // Get info about the width & height of the movie.
    if (!loadMovie || !Loader.GetMovieInfo(FXPLAYER_FILEPATH, &MovieInfo))
    {
        if (loadMovie)
            fprintf(stderr, "Error: failed to get info about %s\n", Settings.FileName);

        ViewWidth   = 640;
        ViewHeight  = 480;

        //return 1;
    }
    else
    {
        ViewWidth   = (SInt) (MovieInfo.Width * Settings.ScaleX);
        ViewHeight  = (SInt) (MovieInfo.Height * Settings.ScaleY);
    }

    // Leave as 0 until switching.
    // This Will cause border-dependent calc during full-screen toggle.
    OldWindowWidth  = 0;
    OldWindowHeight = 0;

    if (Settings.DoRender)
    {
        // Set options based on arguments
        FullScreen      = Settings.FullScreen;
        FSAntialias     = (Settings.AAMode == FxPlayerSettings::AAMode_FSAA) ? 1 : 0;
        BitDepth        = Settings.BitDepth;

        if (FullScreen)
        {
            Settings.ScaleX = 640.0f / ViewWidth;
            Settings.ScaleY = 480.0f / ViewHeight;
            ViewWidth       = 640;
            ViewHeight      = 480;
        }

        // Enable file drop.
        SupportDropFiles = 1;
        SizableWindow    = 1;

        if (!SetupWindow(FXPLAYER_APP_TITLE, ViewWidth, ViewHeight))
            return 1;

        // It is important to initialize these sizes, in case OnSizeEnter gets called.
        SizeWidth  = Width;
        SizeHeight = Height;

        // Create renderer
        if (!CreateRenderer())
            return 1;
        //    pRenderSync = pRenderer->CreateRenderSync_DrawSync();
        //    pRenderer->SetDependentVideoMode(pRmode, pRenderSync);

        // Set renderer on loader so that it is also applied to all children.
        pRenderConfig = *new GFxRenderConfig(GetRenderer());
        Loader.SetRenderConfig(pRenderConfig);

        // Create a renderer stats object since we will be tracking statistics.
        pRenderStats = *new GFxRenderStats();
        Loader.SetRenderStats(pRenderStats);
    }

    // Load movie for the stats display
    // GFxLoader::LoadWaitCompletion should be set from threaded loading for HUD movie. 
    // Otherwise HUD will no be shown because is Advance method gets call only once when the
    // movie is not ready yet
    GPtr<GFxMovieDef> pHudDef = *Loader.CreateMovie("  fxplayer.swf",
        GFxLoader::LoadAll|GFxLoader::LoadOrdered
        |GFxLoader::LoadKeepBindData|GFxLoader::LoadWaitCompletion);
    if (pHudDef) 
    {
        pHud = *pHudDef->CreateInstance();
        if (pHud) 
        {
            pHud->SetBackgroundAlpha(0);

#ifdef FXPLAYER_FONT_SIZE
            pHud->Invoke("_root.setHudFontSize", "%d", FXPLAYER_FONT_SIZE);
#endif
        }
    }

    RTWidth = 256;
    RTHeight = 256;
    SetupRTTexture();

    // Load movie and initialize timing.
    if (loadMovie && !LoadMovie(FXPLAYER_FILEPATH))
    {
        //return 1;
    }

    while (1)
    {
        TimeTicks = GTimer::GetTicks()/1000;

        if (Settings.DoRender && !Settings.FastForward)
            MovieTicks = TimeTicks;
        else // Simulate time.
            MovieTicks = MovieLastTicks + (UInt32) (1000.0f / MovieInfo.FPS);

        int     deltaTicks  = MovieTicks - MovieLastTicks;
        float   deltaT      = deltaTicks / 1000.f;

        MovieLastTicks = MovieTicks;

        // Check auto exit timeout counter.
        if ((Settings.ExitTimeout > 0) &&
            (MovieTicks - MovieStartTicks > (UInt32) (Settings.ExitTimeout * 1000)) )
            break;

        // Process messages and exit if necessary.

        if (!ProcessMessages())
            break;

        DisplayStatus status = CheckDisplayStatus();
        if (status == DisplayStatus_Unavailable) 
        {
            SleepMilliSecs(10);
            continue;
        }
        if (status == DisplayStatus_NeedsReset) 
        {
            RecreateRenderer();
        }

        // Potential out-of bounds range is not a problem here,
        // because it will be adjusted for inside of the player.
        if (pMovie)
        {
            pMovie->SetViewport(Width,Height, (Width-ViewWidth)/2, (Height-ViewHeight)/2,
                                ViewWidth, ViewHeight);
            pMovie->SetBackgroundAlpha(Settings.Background ? 1.0f : 0.05f);

            if (!Paused)
                pMovie->Advance(deltaT * SpeedScale);
        }

        if (Settings.DoRender)
        {
            RenderMesh();

            // Renderer-specific preparation (Disable depth testing)
            PrepareRendererForFrame();
            GetRenderer()->BeginFrame();

            // Clear the entire buffer.
            Clear(GColor::Blue);

            // Enable wireframe if requested.
            SetWireframe(Wireframe);
        }

        if (pMovie)
            pMovie->Display();
/*
        if (MouseCursor)
        {
            Push2DRenderView();
            Pop2DRenderView();
        }
*/
        FrameCounter++;

        if (Settings.DoRender)
        {
            if (pMovie && (LastFrame != pMovie->GetCurrentFrame()))
                NeedHudUpdate = 1;

            // Get stats every frame
            GRenderer::Stats    renderStats;
            pRenderer->GetRenderStats(&renderStats, 1);

            // If ballpark triangle count changed, need update
            if ((renderStats.Triangles >> 11) != (LastStats.Triangles >> 11))
                NeedHudUpdate = 1;
            LastStats = renderStats;

            if (NeedHudUpdate && pHud) 
            {
                UpdateHudText();
                HudViewport = GViewport(GetWidth(), GetHeight(),
                    SInt(GetWidth()*GetSafeArea()),
                    SInt(GetHeight()*GetSafeArea()),
                    SInt(GetWidth() - 2*GetWidth()*GetSafeArea()),
                    SInt(GetHeight() - 2*GetHeight()*GetSafeArea()));

                pHud->SetViewport(HudViewport);
                pHud->Invoke("_root.setHudText", "%s", HudText);
                pHud->Invoke("_root.setMessageText", "%s", MessageText);
                if (!pMovie)
                    pHud->Invoke("_root.setHudSize", "%d %d", 0, 0);
            }

            // Draw the HUD screen if it is displayed.
            if ((!pMovie || Settings.HudState != FxPlayerSettings::Hud_Hidden) /*&& HudText[0]*/ && pHud)
            {
                SetWireframe(0);
                pHud->Display();

                pRenderer->GetRenderStats(&renderStats, 1);
            }

            // Flip buffers to display the scene
            PresentFrame();
            GetRenderer()->EndFrame();

            if (!pMovie || (!Settings.MeasurePerformance && !Settings.FastForward))
            {
                // Don't hog the CPU.
                SleepTillMessage(200);
            }
            else
            {
                // Log the frame rate every second or so.
                if (TimeTicks - LastLoggedFps > 1000)
                {
                    float   delta = (TimeTicks - LastLoggedFps) / 1000.f;

                    LastFPS = (delta > 0) ? FrameCounter / delta : 0.0f;

                    // Log frame rate to console
                    // OSReport(" (fps:%3.1f)\n", LastFPS);

                    // Update HUD
                    NeedHudUpdate = 1;

                    LastLoggedFps = TimeTicks;
                    FrameCounter = 0;
                }
            }
        }

        // If we're reached the end of the movie, exit.
        if (!Settings.DoLoop && pMovie &&
            (pMovie->GetCurrentFrame() + 1 == pMovieDef->GetFrameCount()) )
            break;
    }


    // Release logic? -> TBD

    pMovie      = 0;

    return 0;
}



// Load a new movie from a file and initialize timing
bool    FxPlayerApp::LoadMovie(char *pfilename)
{
    // Try to load the new movie
    GPtr<GFxMovieDef>   pnewMovieDef;
    GPtr<GFxMovieView>  pnewMovie;
    GFxMovieInfo        newMovieInfo;

    // Get info about the width & height of the movie.
    if (!Loader.GetMovieInfo(pfilename, &newMovieInfo, 0, GFxLoader::LoadKeepBindData))
    {
        fprintf(stderr, "Error: failed to get info about %s\n", pfilename);
        return 0;
    }
    // Load the actual new movie and crate instance.
    // Don't use library: this will ensure that the memory is released.
    UInt loadConstants = GFxLoader::LoadAll;
    pnewMovieDef = *Loader.CreateMovie(pfilename, loadConstants|GFxLoader::LoadOrdered|GFxLoader::LoadKeepBindData);

    if (!pnewMovieDef)
    {
        fprintf(stderr, "Error: failed to create a movie from '%s'\n", pfilename);
        return 0;
    }
    pnewMovie = *pnewMovieDef->CreateInstance();
    if (!pnewMovie)
    {
        fprintf(stderr, "Error: failed to create movie instance\n");
        return 0;
    }

    // If this succeeded, replace the old movie with the new one.
    pMovieDef   = pnewMovieDef;
    pMovie      = pnewMovie;
    memcpy(&MovieInfo, &newMovieInfo, sizeof(GFxMovieInfo));

    // This should only be true if this is the GFxPlayer application
    // Make sure to comment this out or set the value to false in your game
    pMovie->SetVariable("_global.gfxPlayer", GFxValue(true));

    if (MouseEnabled)
        pMovie->SetMouseCursorCount(GetMouseCount());

    // Set ActionScript verbosity / etc.
    GPtr<GFxActionControl> pactionControl = *new GFxActionControl();
    pactionControl->SetVerboseAction(Settings.VerboseAction);
    pactionControl->SetActionErrorSuppress(Settings.NoActionErrors);
    pMovie->SetActionControl(pactionControl);

    // Install handlers.
    pMovie->SetFSCommandHandler(GPtr<GFxFSCommandHandler>(*new FxPlayerFSCallback()));
    pMovie->SetUserEventHandler(GPtr<GFxUserEventHandler>(*new FxPlayerUserEventHandler(this)));

    // Renderer
    if (Settings.DoRender)
    {
        if (Settings.AAMode == FxPlayerSettings::AAMode_EdgeAA)
            pRenderConfig->SetRenderFlags(pRenderConfig->GetRenderFlags() | GFxRenderConfig::RF_EdgeAA);
    }

    if (Settings.DoSound)
    {   // TBD:
        //  GFxSoundPlayer* psound = NULL;
        //      psound = GFxSoundPlayer::CreatePlayer ();
        //      pMovie->SetSoundPlayer(psound);
    }

    // Copy short filename (i.e. after last '/'),
    {
        G_strcpy(Settings.FileName, sizeof(Settings.FileName), pfilename);
        SPInt len = strlen(pfilename);
        for (SPInt i=len; i>0; i--) 
        {
            if (pfilename[i]=='/' || pfilename[i]=='\\') 
            {
                pfilename = pfilename+i+1;
                break;
            }
        }
        G_strcpy(Settings.ShortFileName, sizeof(Settings.ShortFileName), pfilename);
    }

    // Disable pause.
    Paused          = 0;

    // Init timing for the new piece.
    FrameCounter    = 0;
    // Time ticks: always rely on a timer
    TimeStartTicks  = GTimer::GetTicks()/1000;
    LastLoggedFps   = TimeStartTicks;
    // Movie logical ticks: either timer or setting controlled
    MovieStartTicks = (Settings.DoRender && !Settings.FastForward) ? TimeStartTicks : 0;
    MovieLastTicks  = MovieStartTicks;
    // Set the Hud to update
    NeedHudUpdate = 1;

    // Update the view
    UpdateViewSize();

    GFxResource*	  pres = pMovie->GetMovieDef()->GetResource("texture1");
    GFxImageResource* pimageRes = 0;
    if (pres && pres->GetResourceType() == GFxResource::RT_Image)
        pimageRes = (GFxImageResource*)pres;

    if (pimageRes)    
    {
        GPtr<GTextureWii> pTexture = *pUserRenderer->CreateTexture();
        pTexture->InitTexture(&RenderTexture);

        // Store the original image info's width and height.
        GImageInfo* pimageInfo = (GImageInfo*)pimageRes->GetImageInfo();
        // Convert image to texture; keep image dimensions as target size.
        pimageInfo->SetTexture(pTexture.GetPtr(),
            pimageInfo->GetWidth(), pimageInfo->GetHeight());
    }

    return 1;
}


// Called when sizing begins and ends.
void    FxPlayerApp::OnSizeEnter(bool enterSize)
{

    // When leaving size, adjust to new width/height.
    if (!enterSize)
    {
        SizingEntered = 0;

        if (GetRenderer())
        {
            ResizeWindow(SizeWidth, SizeHeight);
            UpdateViewSize();
        }
    }
    else
    {
        // Set SizingEntered flag so that we can differentiate size-grip
        // resize from instantaneous resize (maximize).
        SizingEntered = 1;
    }
}

// Sizing; by default, re-initalizes the renderer
void    FxPlayerApp::OnSize(SInt w, SInt h)
{
    if (GetRenderer())
    {
        SizeWidth = w;
        SizeHeight= h;

        if (!SizingEntered && CheckDisplayStatus() == DisplayStatus_Ok) 
        {
            // Commit sizing immediately if it was due to maximize.
            OnSizeEnter(0);
        }
        else
        {
            if (!(FSAntialias && FSAASupported))
                PresentFrame();           
        }       
    }
}

// Updates the view size based on the ScaleEnable flag and window size.
void    FxPlayerApp::UpdateViewSize()
{
    if (ScaleEnable)
    {
        SInt width = G_Max(Width, 4);
        SInt height= G_Max(Height, 4);

        // Determine movie size and location based on the aspect ratio
        float hw;
        if (MovieInfo.Height != 0 && MovieInfo.Width != 0)
            hw = (Float) MovieInfo.Height / (Float) MovieInfo.Width;
        else
            hw = (Float) height / (Float) width;
        if (width * hw > height)
        {
            // Use height as the basis for aspect ratio
            ViewWidth   = (SInt)((float) height / hw);
            ViewHeight  = height;
        }
        else
        {
            // Use width
            ViewWidth   = width;
            ViewHeight  = (SInt) (width * hw);
        }

        // Compute input scale
        Settings.ScaleX = (Float) ViewWidth / (Float) MovieInfo.Width;
        Settings.ScaleY = (Float) ViewHeight / (Float) MovieInfo.Height;
    }
    else
    {
        // No scaling, just center the image
        ViewWidth   = MovieInfo.Width;
        ViewHeight  = MovieInfo.Height;
        Settings.ScaleX = Settings.ScaleY = 1.0f;
    }
}


// Helper function to update HUD.
// Uses LastFPS and LastStats; those variables must be updated separately.
void    FxPlayerApp::UpdateHudText()
{
    if (!pMovie) 
    {
        G_strcpy(HudText, sizeof(HudText), "");
#ifdef FXPLAYER_FILEPATH
        if (IsConsole())
            G_strcpy(MessageText, sizeof(MessageText), "Copy a SWF/GFX file to\n" FXPLAYER_FILEPATH);
        else
#endif
            G_strcpy(MessageText, sizeof(MessageText), "Drag and drop SWF/GFX file here");
        return;
    }
    else
        MessageText[0]=0;

    if (pMovie)
        LastFrame = pMovie->GetCurrentFrame();
    else
        LastFrame = 0;

    // AA Type
    char * pAAType = "Edge AA";

    switch(Settings.AAMode)
    {
    case FxPlayerSettings::AAMode_None:
        pAAType = "None";
        break;
    case FxPlayerSettings::AAMode_EdgeAA:

        // Display a custom message if edge AA is #ifdef-ed out.
#ifndef GFC_NO_FXPLAYER_EDGEAA
        pAAType = "Edge AA";
#else
        pAAType = "Edge AA [#disabled]";
#endif
        break;
    case FxPlayerSettings::AAMode_FSAA:
        pAAType = "HW FSAA";
        break;
    }

    // Update hud text
    sprintf(HudText,
        "Playback Info (1)\n\n"
        "Filename:  %s\n"
        "           SWF %d (%dx%d)\n"
        "           %d/%d frames%s\n"
        "FPS:       %3.1f\n"
        "Triangles: %d @ %d DP\n"
        "Lines:     %d\n"
        "CurveErr:  %3.1f\n"
        "AA Mode:   %s",

        Settings.ShortFileName,
        MovieInfo.Version,
        MovieInfo.Width, MovieInfo.Height,
        LastFrame, MovieInfo.FrameCount, Paused ? " (Paused)" : "",
        LastFPS,
    LastStats.Triangles, LastStats.Primitives,
        LastStats.Lines,
        CurvePixelError,
        pAAType);

    if (Settings.HudState == FxPlayerSettings::Hud_Help)
    {
        strcat(HudText,
            "\n\n"
            "Keys:\n"
//            "  Triangle   Toggle wireframe\n"
            "  1          Toggle HUD\n"
            "  C          Toggle Fast Forward\n"
            "  Home       Toggle pause\n"
            //"  Select     Restart the movie\n"
            "  -  +       Change file\n"
            "  2          Toggle anti-aliasing\n"
            //"  R2         Enable mouse; Show/Hide cursor\n"

/*
            "  F1         Toggle fast mode (FPS)\n"

            "  Right      Step back one frame\n"
            "  Left       Step forward one frame\n"
            "  Up         Step back 10 frames\n"
            "  Down       Step forward 10 frames\n"
            "  L1,L2      Curve tolerance down, up\n"
            "  Start      Toggle Info Help\n"
            "  Select     Toggle Info Stats\n"
*/
            );
    }

    NeedHudUpdate = 0;
}


// Handle dropped file
void    FxPlayerApp::OnDropFiles(char *path)
{
    LoadMovie(path);
}

// Helper used to convert key codes and route them to GFxPlayer
void FxPlayerApp::KeyEvent(GFxKey::Code keyCode, bool down)
{
    if (keyCode != GFxKey::VoidSymbol) 
    {
        if (pMovie) 
        {
            GFxKeyEvent event(down ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, keyCode);
            pMovie->HandleEvent(event);
        }
    }
}

void    FxPlayerApp::OnPad(PadKeyCode keyCode, bool downFlag)
{
    switch(keyCode)
    {
    case FxApp::Pad_Left:
        KeyEvent(GFxKey::Left, downFlag);
        return;
    case FxApp::Pad_Right:
        KeyEvent(GFxKey::Right, downFlag);
        return;
    case FxApp::Pad_Up:
        KeyEvent(GFxKey::Up, downFlag);
        return;
    case FxApp::Pad_Down:
        KeyEvent(GFxKey::Down, downFlag);
        return;

    case FxApp::Pad_A:
        KeyEvent(GFxKey::Return, downFlag);
        return;

    case FxApp::Pad_B:
        KeyEvent(GFxKey::Escape, downFlag);
        return;
    }

    if (!downFlag)
        return;

    switch(keyCode)
    {
    case FxApp::Pad_Home:
        // Toggle paused state.
        NeedHudUpdate = 1;
        Paused = !Paused;
        // Note that toggling play state through pMovie->SetPlayState would only pause root clip,
        // not children. So instead we just set the global variable above which prevents Advance.
        // However, we may still need to save/restore state in frame.
        if (Paused)
            PausedState = pMovie->GetPlayState();
        else                
            pMovie->SetPlayState(PausedState);
        break;

    case FxApp::Pad_Select:
        Paused = 0;
        pMovie->GotoFrame(0);
        // Restart the movie, does not seem to work quite right.
        pMovie->Restart();
        break;

        // Toggle info hud.
    case FxApp::Pad_1:           
        switch(Settings.HudState)
        {
        case FxPlayerSettings::Hud_Hidden:  Settings.HudState = FxPlayerSettings::Hud_Stats;    break;
        case FxPlayerSettings::Hud_Stats:   Settings.HudState = FxPlayerSettings::Hud_Help;     break;
        case FxPlayerSettings::Hud_Help:    Settings.HudState = FxPlayerSettings::Hud_Hidden;   break;
        }
        NeedHudUpdate = 1;
        break;

    case FxApp::Pad_R1: 
        MouseCursor = !MouseCursor;
        if (!MouseEnabled && pMovie)
            pMovie->SetMouseCursorCount(GetMouseCount());
        MouseEnabled = 1;
        break;

    case FxApp::Pad_C:
        Settings.FastForward = !Settings.FastForward;
        break;


    case FxApp::Pad_2:
        {
            bool    edgeChange   = 0;

            switch(Settings.AAMode)
            {
            case FxPlayerSettings::AAMode_None:
                Settings.AAMode = FxPlayerSettings::AAMode_EdgeAA;
                edgeChange = 1;
                break;
            case FxPlayerSettings::AAMode_EdgeAA:
                Settings.AAMode = FxPlayerSettings::AAMode_None;
                edgeChange = 1;
                break;
            }
            NeedHudUpdate = 1;

            if (edgeChange && pMovie)
            {
                UInt32 rendererFlags = pRenderConfig->GetRenderFlags() & ~GFxRenderConfig::RF_EdgeAA;
                if (Settings.AAMode == FxPlayerSettings::AAMode_EdgeAA)
                    rendererFlags |= GFxRenderConfig::RF_EdgeAA;
                pRenderConfig->SetRenderFlags(rendererFlags);
            }
        }
        break;

        // Next/previous SWF
    case FxApp::Pad_Plus:
    case FxApp::Pad_Minus:
        if(UpdateFiles(Settings.FileName, keyCode == FxApp::Pad_Minus))
            LoadMovie(Settings.FileName);
        break;
    }

}

void    FxPlayerApp::OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, 
                                   int x, int y, unsigned int mods)
{
    if (!pMovie)
    return;

    // Adjust x, y to viewport.
    x = (int) ((x - (Width-ViewWidth)/2));
    y = (int) ((y - (Height-ViewHeight)/2));

    // Update mouse state
    if (downFlag)
    {
    GFxMouseEvent event(GFxEvent::MouseDown, button, x, y, 0.0f, mouseIndex);
    pMovie->HandleEvent(event);
    }
    else
    {
    GFxMouseEvent event(GFxEvent::MouseUp, button, x, y, 0.0f, mouseIndex);
    pMovie->HandleEvent(event);
    }
}

void    FxPlayerApp::OnMouseMove(unsigned mouseIndex, int x, int y, unsigned int mods)
{
    // Used by NotifyMouseState in the main loop
    if (pMovie)
    {
        GFxMouseEvent event(GFxEvent::MouseMove, 0,
                            (int) ((x - (Width-ViewWidth)/2)),
                            (int) ((y - (Height-ViewHeight)/2)), 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }
}
