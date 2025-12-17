/**************************************************************************

Filename    :   FxPlayerSWFToTexturePS3.cpp
Content     :   Sample GFx SWF file player for PS3
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
#include "GFxRenderConfig.h"
#include "GStd.h"

#include <stdlib.h>
#include <stdio.h>
#include <cell/fs/cell_fs_file_api.h>
#include <sys/paths.h>
#include <sys/timer.h>
#include <cell/sysmodule.h>

// **** Fx Player defines

#include "../Common/MathLib.h"

#ifdef FXPLAYER_RENDER_GCM
#include "../Common/GcmPS3App.h"
#define FXPLAYER_APP                GcmPS3App
#else
#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
#include "../Common/OpenGLPS3App.h"
#define FXPLAYER_APP                OpenGLPS3App
#endif

// The path that will be searched for files 
// - FXPLAYER_FILENAME must be located in this path
#define FXPLAYER_FILEDIRECTORY      SYS_HOST_ROOT "/usr/local/cell/FxPlayer/"
#define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY "3DWindow.swf"
#define FXPLAYER_BGMOVIEPATH        FXPLAYER_FILEDIRECTORY "3DWindowBackground.swf"


#define FXPLAYER_APP_TITLE          "Scaleform GFx Render Texture Sample PS3 v" GFC_FX_VERSION_STRING

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
        HudState          = Hud_Hidden;

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
    GFxMovieInfo        BGInfo;
    GPtr<GFxMovieDef>   pBGDef;
    GPtr<GFxMovieView>  pBG;

    // Movie timing state
    float               SpeedScale;         // Advance speed, def 1.0f
    SInt                FrameCounter;       // Frames rendered, for FPS
    // Time ticks: always rely on a timer, for FPS
    UInt64              TimeStartTicks;     // Ticks during the start of playback
    UInt64              TimeTicks;          // Current ticks
    UInt64              LastLoggedFps;      // Time ticks during last FPS log
    // Movie logical ticks: either timer or setting controlled
    UInt64              MovieStartTicks;
    UInt64              MovieLastTicks;
    UInt64              MovieTicks;
    
    // Renderer we use
    GPtr<GFxRenderConfig> pRenderConfig;
    GPtr<GFxRenderStats>  pRenderStats;

    // Selected playback settings
    FxPlayerSettings    Settings;

    // View width and height
    SInt                ViewWidth, ViewHeight;

    // Set if wireframe ins enabled.
    bool                Wireframe;

    enum TrackingState
    {
        None,
        Tilting,
        Centering,
    };
    TrackingState       MouseTracking;
    TrackingState       TextureTilt;

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

#ifdef FXPLAYER_RENDER_GCM
    UPInt                   BaseMain, BaseLocal;
    CellGcmSurface          RenderSurface;
    CellGcmTexture          RenderTexture;
#else
    GLuint                  RenderTextureId;
    GLuint                  RenderTextureFbo;
#endif
    SInt                    RTWidth, RTHeight;
    Float                   MeshRotationX, MeshRotationZ;
    UInt                    LastRotationTick;
    Float                   InvMV[16], Proj[16], InvProj[16];
    CGprogram               TextureFp;
    CGparameter             TextureFpT1;
    static bool             CubeWireframe;

    static FxPlayerApp      *pApp;

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
    void            RenderMovie();
    void            RenderMovieTexture();

    
    // *** Overrides

    // Sizing; by default, re-initalizes the renderer
    virtual void    OnSize(SInt w, SInt h);     
    virtual void    OnSizeEnter(bool enterSize);
    virtual void    OnDropFiles(char *path);

    // Input
    virtual void    OnPad(PadKeyCode, bool downFlag);
    virtual void    OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, int x, int y, 
        unsigned int mods);
    virtual void    OnMouseMove(unsigned mouseIndex, int x, int y, int unsigned mods);

    // Helper used to convert key codes and route them to GFxPlayer
    void            KeyEvent(GFxKey::Code key, bool down);
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
        if (!strcmp(purl, "fxplayer.swf"))
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

        if (!strcmp(pcommand, "wireframe") && FxPlayerApp::pApp)
            FxPlayerApp::pApp->Wireframe = !strcmp(parg, "1");
        else if (!strcmp(pcommand, "center") && FxPlayerApp::pApp)
            FxPlayerApp::pApp->TextureTilt = FxPlayerApp::Centering;
        else if (!strcmp(pcommand, "quit") && FxPlayerApp::pApp)
            FxPlayerApp::pApp->QuitFlag = 1;
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
            //pApp->MouseCursor = 1;
            break;
        case GFxEvent::DoHideMouse:
            //pApp->MouseCursor = 0;
            break;

        default:
            break;
        }
    }
};

// ***** Main function implementation

int main(int argc, char *argv[])
{
#if (CELL_SDK_VER >= 93)
    cellSysmoduleLoadModule(CELL_SYSMODULE_RESC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
    cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);
    cellSysmoduleLoadModule(CELL_SYSMODULE_GCM);
#endif

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
    MessageText[0]      = 0;

    ViewWidth           = 
    ViewHeight          = 0;

    SizingEntered       = 0;

    CurvePixelError     = 1.0f;
    CursorDisabled      = 0;
    CursorHidden        = 0;

    TextureTilt             = None;
    MeshRotationX           = 0;
    MeshRotationZ           = 0;
    LastRotationTick        = 0;
    CubeWireframe           = 0;
    pApp = this;
}

bool FxPlayerApp::CubeWireframe;
FxPlayerApp *FxPlayerApp::pApp = 0;


FxPlayerApp::~FxPlayerApp()
{
}

struct Vertex
{
    float x, y, z;
    float tu, tv;
};

Vertex g_cubeVertices[] =
{
    {-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
    { 1.0f, 1.0f, 0.0f,  1.0f,0.0f },
    {-1.0f,-1.0f, 0.0f,  0.0f,1.0f },
    { 1.0f,-1.0f, 0.0f,  1.0f,1.0f },
};

void    FxPlayerApp::RenderMovie()
{
#ifdef FXPLAYER_RENDER_GCM
    cellGcmSetSurface(GcmCtx, &RenderSurface);

    float scale[4], offset[4];
    scale[0] = RTWidth * 0.5f;
    scale[1] = RTHeight * 0.5f;
    offset[0] = RTWidth * 0.5f;
    offset[1] = RTHeight * 0.5f;
    scale[2] = 0.5;
    offset[2] = 0.5;
    scale[3] = offset[3] = 0;
    cellGcmSetViewport(GcmCtx, 0,0, RTWidth, RTHeight, 0, 1, scale, offset);
    cellGcmSetScissor(GcmCtx, 0,0, 4096,4096);

    cellGcmSetClearColor(GcmCtx, 0);
    cellGcmSetClearDepthStencil(GcmCtx, 0xffffff00);
    cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A|CELL_GCM_CLEAR_S|CELL_GCM_CLEAR_Z);

    pMovie->SetViewport(RTWidth,RTHeight,0,0,RTWidth,RTHeight, GViewport::View_RenderTextureAlpha);
    pMovie->SetBackgroundAlpha(0);
    pMovie->Display();

    cellGcmSetSurface(GcmCtx, &Surface);

#else
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, RenderTextureFbo);

    GLenum  x = glCheckFramebufferStatusOES (GL_FRAMEBUFFER_OES);
    if (x != GL_FRAMEBUFFER_COMPLETE_OES)
        abort();

    glViewport(0,0, RTWidth, RTHeight);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pMovie->SetViewport(RTWidth,RTHeight,0,0,RTWidth,RTHeight, GViewport::View_RenderTextureAlpha);
    pMovie->SetBackgroundAlpha(0);
    pMovie->Display();

    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
#endif
}

void    FxPlayerApp::RenderMovieTexture()
{
    UInt    ticks = GTimer::GetTicks()/1000;

    if (TextureTilt == Centering)
    {
        float dt = 0.0f;
        float t  = (float)((double)fmod((double)ticks, 7500.0) / 7500.0) * 360;
        float lt = (float)((double)fmod((double)LastRotationTick, 7500.0) / 7500.0) * 360;
        dt       = t - lt;

        LastRotationTick    = ticks;
        if (MeshRotationX > 0.0f)
        {
            MeshRotationX        -= dt;
            if (MeshRotationX < 0.0f)
                MeshRotationX = 0.0f;
        }
        else if (MeshRotationX < 0.0f)
        {
            MeshRotationX        += dt;
            if (MeshRotationX > 0.0f)
                MeshRotationX = 0.0f;
        }

        if (MeshRotationZ > 0.0f)
        {
            MeshRotationZ        -= dt;
            if (MeshRotationZ < 0.0f)
                MeshRotationZ = 0.0f;
        }
        else if (MeshRotationZ < 0.0f)
        {
            MeshRotationZ        += dt;
            if (MeshRotationZ > 0.0f)
                MeshRotationZ = 0.0f;
        }

        if (MeshRotationZ == 0.0f && MeshRotationX == 0.0f)
        {
            TextureTilt = None;
        }
    }
    else if (TextureTilt == None && (ticks - LastRotationTick) >= 20)
    {
        float tiltMax   = 10.5f;
        float tiltDelta = 0.2f;

        if (MeshRotationZ > -tiltMax && MeshRotationX == 0 || MeshRotationX < -tiltMax)
            MeshRotationZ -= tiltDelta;

        if (MeshRotationX < tiltMax && MeshRotationZ < -tiltMax)
            MeshRotationX += tiltDelta;

        if (MeshRotationZ < tiltMax && MeshRotationX > tiltMax)
            MeshRotationZ += tiltDelta;

        if (MeshRotationX > -tiltMax && MeshRotationZ > tiltMax)
            MeshRotationX -= tiltDelta;

        LastRotationTick = ticks;
    }
    else if (TextureTilt != None)
        LastRotationTick = ticks;

    MeshRotationX = fmod(MeshRotationX, 360.0f);
    MeshRotationZ = fmod(MeshRotationZ, 360.0f);

    if (MeshRotationX > 45.0f)
        MeshRotationX = 45.0f;
    if (MeshRotationX < -45.0f)
        MeshRotationX = -45.0f;

    if (MeshRotationZ > 45.0f)
        MeshRotationZ = 45.0f;
    if (MeshRotationZ < -45.0f)
        MeshRotationZ = -45.0f;

    Float rot[16], mv[16], mat[16], lap[16];

    MakeIdentity(lap);
    Translate(lap, 0,0, -2.5f);
    MakeRotateX(rot, MeshRotationX * 3.141592f * 0.00555555f);
    MatrixMult(mat, rot, lap);
    MakeRotateZ(rot, MeshRotationZ * 3.141592f * 0.00555555f);
    MatrixMult(mv, rot, mat);

#ifdef FXPLAYER_RENDER_GCM

    float scale[4], offset[4];
    scale[0] = Width * 0.5f;
    scale[1] = Height * 0.5f;
    offset[0] = Width * 0.5f;
    offset[1] = Height * 0.5f;
    scale[2] = 0.5;
    offset[2] = 0.5;
    scale[3] = offset[3] = 0;
    cellGcmSetViewport(GcmCtx, 0,0, Width, Height, 0, 1, scale, offset);
    cellGcmSetScissor(GcmCtx, 0,0, 4096,4096);

    Float proj[16], mvp[16], mvpt[16];

    MakePerspective(proj, 3.141592f * 0.25f, Float(RTWidth)/Float(RTHeight), 0.1f, 100);
    MatrixMult(mvp, mv, proj);
    MatrixTranspose(mvpt, mvp);

    cellGcmSetVertexProgramParameter(GcmCtx, VProgMvp, mvpt);

    cellGcmSetCullFaceEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetDepthTestEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetAlphaTestEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetBlendEnable(GcmCtx, CELL_GCM_TRUE);
    cellGcmSetBackPolygonMode(GcmCtx, CELL_GCM_POLYGON_MODE_FILL);
    cellGcmSetFrontPolygonMode(GcmCtx, CELL_GCM_POLYGON_MODE_FILL);

    cellGcmSetBlendEquation(GcmCtx, CELL_GCM_FUNC_ADD, CELL_GCM_FUNC_ADD);
    cellGcmSetBlendFunc(GcmCtx, CELL_GCM_ONE, CELL_GCM_ONE_MINUS_SRC_ALPHA,
                        CELL_GCM_ONE, CELL_GCM_ONE_MINUS_SRC_ALPHA);

    cellGcmSetVertexDataArray(GcmCtx, VProgPos, 0, sizeof(Vertex), 3, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
    cellGcmSetVertexDataArray(GcmCtx, VProgTc0, 0, sizeof(Vertex), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);

    SetTex2dShader(&RenderTexture);
    cellGcmSetDrawInlineArray(GcmCtx, CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, 20, g_cubeVertices);

    cellGcmSetVertexDataArray(GcmCtx, VProgPos, 0,0,0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
    cellGcmSetVertexDataArray(GcmCtx, VProgTc0, 0,0,0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);

#else
    glViewport(0,0,Width,Height);

    MakePerspective(Proj, 3.141592f * 0.25f, Float(RTWidth)/Float(RTHeight), 0.1f, 100);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(Proj);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mv);

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    cgGLEnableProfile (CG_PROFILE_SCE_FP_RSX);
    cgGLDisableProfile (CG_PROFILE_SCE_VP_RSX);
    glColorMask(1,1,1,1);
    glDepthRangef(0.1, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    cgGLBindProgram(TextureFp);
    glActiveTexture(cgGLGetTextureEnum(TextureFpT1));
    glBindTexture(GL_TEXTURE_2D, RenderTextureId);
    cgGLSetTextureParameter (TextureFpT1, RenderTextureId);
    cgGLEnableTextureParameter (TextureFpT1);
    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1,1,1,1);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), g_cubeVertices);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &g_cubeVertices[0].tu);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

extern int _binary_p_tex2d_fpo_cgelf_start;

bool    FxPlayerApp::SetupRTTexture()
{
#ifdef FXPLAYER_RENDER_GCM

    UInt32 pitch = (RTWidth * 4 + 63) & ~63;

    GAllocatorPS3::Handle hColor = Alloc.Allocate(CELL_GCM_LOCATION_LOCAL, pitch * RTHeight, 0x10000);
    GAllocatorPS3::Handle hDepth = Alloc.Allocate(CELL_GCM_LOCATION_LOCAL, pitch * RTHeight, 0x10000);

    RenderSurface.colorFormat         = CELL_GCM_SURFACE_A8R8G8B8;
    RenderSurface.colorTarget         = CELL_GCM_SURFACE_TARGET_0;
    RenderSurface.colorLocation[0]    = CELL_GCM_LOCATION_LOCAL;
    RenderSurface.colorOffset[0]      = Alloc.GetOffset(hColor);
    RenderSurface.colorPitch[0]       = 64;

    RenderSurface.colorLocation[1]    = CELL_GCM_LOCATION_LOCAL;
    RenderSurface.colorLocation[2]    = CELL_GCM_LOCATION_LOCAL;
    RenderSurface.colorLocation[3]    = CELL_GCM_LOCATION_LOCAL;
    RenderSurface.colorOffset[1]      = 0;
    RenderSurface.colorOffset[2]      = 0;
    RenderSurface.colorOffset[3]      = 0;
    RenderSurface.colorPitch[1]       = 64;
    RenderSurface.colorPitch[2]       = 64;
    RenderSurface.colorPitch[3]       = 64;

    RenderSurface.depthFormat         = CELL_GCM_SURFACE_Z24S8;
    RenderSurface.depthLocation       = CELL_GCM_LOCATION_LOCAL;
    RenderSurface.depthOffset         = Alloc.GetOffset(hDepth);
    RenderSurface.depthPitch          = pitch;

    RenderSurface.type                = CELL_GCM_SURFACE_SWIZZLE;
    RenderSurface.antialias           = CELL_GCM_SURFACE_CENTER_1;

    RenderSurface.width               = RTWidth;
    RenderSurface.height              = RTHeight;
    RenderSurface.x                   = 0;
    RenderSurface.y                   = 0;

    RenderTexture.offset    = RenderSurface.colorOffset[0];
    RenderTexture.location  = RenderSurface.colorLocation[0];
    RenderTexture.format    = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;
    RenderTexture.mipmap    = 1;
    RenderTexture.pitch     = 0;
    RenderTexture.width     = RTWidth;
    RenderTexture.height    = RTHeight;
    RenderTexture.depth     = 1;
    RenderTexture.cubemap   = CELL_GCM_FALSE;
    RenderTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
    RenderTexture.remap  = CELL_GCM_REMAP_MODE (CELL_GCM_TEXTURE_REMAP_ORDER_XYXY, 
                                                CELL_GCM_TEXTURE_REMAP_FROM_A,
                                                CELL_GCM_TEXTURE_REMAP_FROM_R,
                                                CELL_GCM_TEXTURE_REMAP_FROM_G,
                                                CELL_GCM_TEXTURE_REMAP_FROM_B,
                                                CELL_GCM_TEXTURE_REMAP_REMAP,
                                                CELL_GCM_TEXTURE_REMAP_REMAP,
                                                CELL_GCM_TEXTURE_REMAP_REMAP,
                                                CELL_GCM_TEXTURE_REMAP_REMAP);
#else
    GLuint rbs[2];

    // Create texture and framebuffer
    glGenFramebuffersOES(1, &RenderTextureFbo);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, RenderTextureFbo);

    glGenTextures(1, &RenderTextureId);
    glBindTexture(GL_TEXTURE_2D, RenderTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_ALLOCATION_HINT_SCE, GL_TEXTURE_TILED_GPU_SCE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ARGB_SCE, RTWidth,RTHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, RenderTextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffersOES(2, rbs);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbs[0]);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT, RTWidth,RTHeight);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, rbs[0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);

    CGcontext ctx = cgCreateContext();

    cgGLSetManageTextureParameters (ctx, 1);
    TextureFp = cgCreateProgram(ctx, CG_BINARY, (const char *) &_binary_p_tex2d_fpo_cgelf_start, CG_PROFILE_SCE_FP_RSX, NULL, NULL);
    TextureFpT1 = cgGetNamedParameter (TextureFp, "tex");

    if (glGetError()) abort();

#endif
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

    if(strlen(Settings.FileName) == 0)
        G_strcpy(Settings.FileName, sizeof(Settings.FileName), FXPLAYER_FILEPATH);

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
        Pads[0].MouseX = Width/2;
        Pads[0].MouseY = Height/2;

        // Create renderer      
        if (!CreateRenderer())
            return 1;

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
    GPtr<GFxMovieDef> pHudDef = *Loader.CreateMovie("fxplayer.swf",
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

    RTWidth = 1024;
    RTHeight = 1024;
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
        
        int     deltaTicks  = int(MovieTicks - MovieLastTicks);
        float   deltaT      = deltaTicks / 1000.f;
        
        MovieLastTicks = MovieTicks;

        // Check auto exit timeout counter.
        if ((Settings.ExitTimeout > 0) &&
            (MovieTicks - MovieStartTicks > (UInt32) (Settings.ExitTimeout * 1000)) )   
            break;
        
        // Process messages and exit if necessary.

        if (!ProcessMessages())
            break;

        // *** Advance/Render movie

        // Potential out-of bounds range is not a problem here,
        // because it will be adjusted for inside of the player.
        if (pMovie)
        {
            pMovie->SetViewport(Width,Height, (Width-ViewWidth)/2, (Height-ViewHeight)/2,
                                ViewWidth, ViewHeight);
            pMovie->SetBackgroundAlpha(Settings.Background ? 1.0f : 0.05f);

            if (!Paused)
            {
                pMovie->Advance(deltaT * SpeedScale);
                if (pBG)
                    pBG->Advance(deltaT * SpeedScale, 0);
            }
        }

        if (Settings.DoRender)
        {
            if (Wireframe)
                SetWireframe(1);
            RenderMovie();
            SetWireframe(0);

            // Renderer-specific preparation (Disable depth testing)
            PrepareRendererForFrame();
            GetRenderer()->BeginFrame();
            
            // Clear the entire buffer.
            Clear(GColor(233,236,226,255).ToColor32());

            if (pBG)
            {
                pBG->SetViewport(Width, Height, 0,0, Width, Height);
                pBG->SetBackgroundAlpha(0);
                pBG->Display();
            }

            RenderMovieTexture();
        }

        FrameCounter++;

        if (Settings.DoRender)
        {           
        //  SetWireframe(0);

            if (pMovie && (LastFrame != pMovie->GetCurrentFrame()))
                NeedHudUpdate = 1;

            // Get stats every frame
            GRenderer::Stats    renderStats;
            GetRenderer()->GetRenderStats(&renderStats, 1);

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
            if ((!pMovie || Settings.HudState != FxPlayerSettings::Hud_Hidden) /*&& HudText[0]*/ && pHud )
            {
                SetWireframe(0);
                pHud->Display();

                GetRenderer()->GetRenderStats(&renderStats, 1);
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
                    //printf(" (fps:%3.1f)\n", LastFPS);

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
    pMovie = 0;

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

    pMovieDef = 0;
    pMovie = 0;

    // Load the actual new movie and crate instance.
    // Don't use library: this will ensure that the memory is released.
    UInt loadConstants = GFxLoader::LoadAll;
    pBGDef = *Loader.CreateMovie(FXPLAYER_BGMOVIEPATH, loadConstants);
    pnewMovieDef = *Loader.CreateMovie(pfilename, loadConstants|GFxLoader::LoadOrdered|GFxLoader::LoadKeepBindData);

    if (!pnewMovieDef)
    {
        fprintf(stderr, "Error: failed to create a movie from '%s'\n", pfilename);
        return 0;
    }
    pnewMovie = *pnewMovieDef->CreateInstance();    
    if (pBGDef)
        pBG = *pBGDef->CreateInstance(false);
    if (!pnewMovie)
    {
        fprintf(stderr, "Error: failed to create movie instance\n");
        return 0;
    }

    // If this succeeded, replace the old movie with the new one.
    pMovieDef   = pnewMovieDef;
    pMovie      = pnewMovie;
    memcpy(&MovieInfo, &newMovieInfo, sizeof(GFxMovieInfo));

    // Set ActionScript verbosity / etc.
    GPtr<GFxActionControl> pactionControl = *new GFxActionControl();
    pactionControl->SetVerboseAction(Settings.VerboseAction);
    pactionControl->SetActionErrorSuppress(Settings.NoActionErrors);
    pMovie->SetActionControl(pactionControl);
 
    pMovie->SetMouseCursorCount(GetMouseCount());

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
        SInt width = G_Max(Width, (SInt)4);
        SInt height= G_Max(Height, (SInt)4);

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
        "Playback Info (Square)\n\n"
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
            "  Triangle   Toggle wireframe\n"           
            "  Square     Toggle HUD\n"
            "  Circle     Toggle Fast Forward\n"
            "  Start      Toggle pause\n"
            "  Select     Restart the movie\n"
            "  L1         Previous file\n"
            "  R1         Next file\n"
            "  L2         Toggle anti-aliasing\n"

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
    case FxApp::Pad_X:
        KeyEvent(GFxKey::Return, downFlag);
        return;
    default:
    	break;
    }

    if (!downFlag)
        return;

    switch(keyCode)
    {
    case FxApp::Pad_Start:
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
    case FxApp::Pad_S:           
        switch(Settings.HudState)
        {
        case FxPlayerSettings::Hud_Hidden:  Settings.HudState = FxPlayerSettings::Hud_Stats;    break;
        case FxPlayerSettings::Hud_Stats:   Settings.HudState = FxPlayerSettings::Hud_Help;     break;
        case FxPlayerSettings::Hud_Help:    Settings.HudState = FxPlayerSettings::Hud_Hidden;   break;
        }
        NeedHudUpdate = 1;
        break;

    case FxApp::Pad_T:
        // Toggle wireframe.
        Wireframe = !Wireframe;
        break;

    case FxApp::Pad_O:
        Settings.FastForward = !Settings.FastForward;
        break;

    
    case FxApp::Pad_L2:
        {
            bool    renderChange = 0;
            bool    edgeChange   = 0;

            switch(Settings.AAMode)
            {
            case FxPlayerSettings::AAMode_None:
                Settings.AAMode = FxPlayerSettings::AAMode_EdgeAA;
                edgeChange = 1;
                break;
            case FxPlayerSettings::AAMode_EdgeAA:
                Settings.AAMode = FxPlayerSettings::AAMode_FSAA;
                edgeChange = renderChange = 1;
                break;
            case FxPlayerSettings::AAMode_FSAA:
                Settings.AAMode = FxPlayerSettings::AAMode_None;
                renderChange = 1;
                break;
            }
            NeedHudUpdate = 1;
       
            // FSAA toggle - Left Thumb
            if (FSAASupported && renderChange)
            {           
                SwitchFSAA(Settings.AAMode == FxPlayerSettings::AAMode_FSAA);
            }           
                
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
    case FxApp::Pad_L1:
    case FxApp::Pad_R1:
        if(UpdateFiles(Settings.FileName, keyCode == FxApp::Pad_L1))
            LoadMovie(Settings.FileName);
        break;
    default:
    	break;
    }

}


void    FxPlayerApp::OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, 
                                   int x, int y, unsigned int mods)
{
    if (!pMovie)
    return;

    // Adjust x, y to viewport.
    x = (int) ((x - (Width-RTWidth)/2));
    y = (int) ((y - (Height-RTHeight)/2));

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
                    (int) ((x - (Width-RTWidth)/2)),
                    (int) ((y - (Height-RTHeight)/2)), 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }   
}
