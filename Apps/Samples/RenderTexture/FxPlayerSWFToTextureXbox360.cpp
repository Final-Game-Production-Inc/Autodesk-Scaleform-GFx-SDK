/**************************************************************************

Filename    :   FxPlayerSWFToTextureXbox360.cpp
Content     :   Sample GFx SWF file player with RTT for XBox 360
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

// Direct3D application class
#include "Direct3DXbox360App.h"

// Standard includes
#include <stdlib.h>
#include <stdio.h>


// **** Fx Player defines

#define FXPLAYER_APP                Direct3DXboxApp

// Resolution at which the player will start, normally 640x480.
// Specify 1280 and 720 for 720p res.
#define FXPLAYER_VIEWWIDTH          640
#define FXPLAYER_VIEWHEIGHT         480

// The path that will be searched for files
// - FXPLAYER_FILENAME must be located in this path
#define FXPLAYER_FILEDIRECTORY      "D:\\Samples\\"
#define FXPLAYER_FILENAME           "3DWindow.swf"
#define FXPLAYER_FILEPATH           FXPLAYER_FILEDIRECTORY FXPLAYER_FILENAME
#define FXPLAYER_FILEMASK           "*.*"
#define FXPLAYER_FILEEXTENSIONLIST  {".swf", ".gfx", 0}


#define FXPLAYER_APP_TITLE          "Scaleform GFx Render Texture Sample XBox 360 v" GFC_FX_VERSION_STRING

#define FXPLAYER_RENDER_DIRECT3D

#define GFC_2PI (2.0f*3.141592f)


#include "../Bin/FxPlayer/fxplayer.swf.h"

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

        VerboseParse        = 0;
        VerboseParseShape   = 0;
        VerboseParseAction  = 0;
        VerboseAction       = 0;
#if defined(GFC_BUILD_DEBUG)
        Quiet               = 0;
        NoActionErrors      = 0;
#else
        Quiet               = 1;
        NoActionErrors      = 1;
#endif
        DoLoop              = 1;
        DoRender            = 1;
        DoSound             = 0;

        FastForward         = 0;

        ExitTimeout         = 0.0f;
        SleepDelay          = 31;

        // Clear file
        FileName[0]         = 0;
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

    GPtr<IDirect3DVertexShader9>    pVShader3D;
    GPtr<IDirect3DPixelShader9>     pPShaderTex2D;
    GPtr<IDirect3DTexture9>         pRenderTexture;
    GPtr<IDirect3DSurface9>         pRenderSurface;
    GPtr<IDirect3DSurface9>         pStencilSurface;
    GPtr<IDirect3DVertexBuffer9>    pCubeVertexBuffer;
    SInt                            RTWidth, RTHeight;
    Float                           MeshRotationX;
    Float                           MeshRotationY;
    UInt                            LastRotationTick;
    bool                            CubeWireframe;

	enum TiltState
	{
		None,
		Tilting,
		Centering,
	};

	TiltState TextureTilt;

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
    void            SetupMatrices();
    void            RenderMovie();
    void            RenderMovieTexture();

    // *** Overrides

    // Sizing; by default, re-initalizes the renderer
    virtual void    OnSize(int w, int h);
    virtual void    OnSizeEnter(bool enterSize);

    // Input
    virtual void    OnKey(KeyCode, unsigned char asciiCode, unsigned int wcharCode, unsigned int mods, bool downFlag);
    virtual void    OnPad(PadKeyCode, bool downFlag);

    // Helper used to convert key codes and route them to GFxPlayer
    void            KeyEvent(GFxKey::Code keyCode, unsigned char asciiCode, unsigned int wcharCode, unsigned int mods, bool down);
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

// "fscommand" handler class, handles notification callbacks from ActionScript.
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
		else if ((!strcmp(pcommand, "quit") || !strcmp(pcommand, "exit")) && FxPlayerApp::pApp)
			FxPlayerApp::pApp->QuitFlag = 1;
    }
};

// ***** Main function implementation

int main(int argc, char *argv[])
{
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

    MeshRotationX = MeshRotationY = 0;
	TextureTilt				= None;
    LastRotationTick        = 0;
    CubeWireframe           = 0;
    pApp = this;
}

FxPlayerApp *FxPlayerApp::pApp = 0;

FxPlayerApp::~FxPlayerApp()
{
    if (pDevice)
        pDevice->SetStreamSource( 0, 0, 0, 0);
    pApp = 0;
}


struct Vertex
{
    enum
    {
        FVF = D3DFVF_XYZ | D3DFVF_TEX1
    };

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


void    FxPlayerApp::SetupMatrices()
{
    // For our world matrix, we will just leave it as the identity
    D3DXMATRIXA16 matWorld;
    D3DXMATRIXA16 rotX;
    D3DXMATRIXA16 rotY;
    D3DXMatrixRotationX( &rotX, MeshRotationX );
    D3DXMatrixRotationY( &rotY, MeshRotationY );
	D3DXMatrixMultiply(&matWorld, &rotX, &rotY);

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.

    D3DXVECTOR3 vEyePt( 0.0f, 0.0f, -2.5f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( -0.0f, 1.0f, 0.0f );

    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );


    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ),
        Float(RTWidth)/Float(RTHeight), 0.1f, 100.0f );

    D3DXMATRIX mvp;
    D3DXMatrixMultiply(&mvp, &matWorld, &matView);
    D3DXMatrixMultiply(&mvp, &mvp, &matProj);
    D3DXMatrixTranspose(&mvp, &mvp);
    pDevice->SetVertexShaderConstantF(0, (const float *)&mvp, 4);
}

// Rendering
void    FxPlayerApp::RenderMovie()
{
    IDirect3DSurface9 *poldSurface      = 0;
    IDirect3DSurface9 *poldDepthSurface = 0;

    // Save both RT and depth-stencil.
    pDevice->GetRenderTarget(0, &poldSurface);
    pDevice->GetDepthStencilSurface(&poldDepthSurface);

    pPlatformHAL->SetRenderTarget(pRenderTarget);

    D3DVIEWPORT9 vp;
    vp.X        = 0;
    vp.Y        = 0;
    vp.Width    = RTWidth;
    vp.Height   = RTHeight;
    vp.MinZ     = 0.0f;
    vp.MaxZ     = 1.0f;
    pDevice->SetViewport(&vp);

    pDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
        D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0 );

    if (pMovie)
    {
        pMovie->SetViewport(RTWidth, RTHeight, 0,0, RTWidth, RTHeight, GViewport::View_RenderTextureAlpha);
        pMovie->SetBackgroundAlpha(0);
        pMovie->Display();
    }

    // Copy RT Surface to Texture
    pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, NULL, pRenderTexture, 0, 0, 0, 0, 0, 0, 0);

    // Restore the render target
    pDevice->SetRenderTarget(0, poldSurface);
    pDevice->SetDepthStencilSurface(poldDepthSurface);

    if (poldSurface)
        poldSurface->Release();
    if (poldDepthSurface)
        poldDepthSurface->Release();
}

    void    FxPlayerApp::RenderMovieTexture()
{
	static bool bCentering = false;
    UInt    ticks = GetTickCount();
	MeshRotationY -= InputState.Gamepad.sThumbRX * 0.0000001f;
	MeshRotationX += InputState.Gamepad.sThumbRY * 0.0000001f;
	MeshRotationX = fmod(MeshRotationX, GFC_2PI);
	MeshRotationY = fmod(MeshRotationY, GFC_2PI);

    if (TextureTilt == Centering)
    {
		if (bCentering == false)
		{
			LastRotationTick = ticks;
			bCentering = true;
		}
        float dt = 0.0f;
		float t  = (float)ticks;
		float lt = (float)LastRotationTick;
		dt = t - lt;
		dt *= 0.0005f;
    
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

        if (MeshRotationY > 0.0f)
        {
            MeshRotationY        -= dt;
            if (MeshRotationY < 0.0f)
                MeshRotationY = 0.0f;
        }
        else if (MeshRotationY < 0.0f)
        {
            MeshRotationY        += dt;
            if (MeshRotationY > 0.0f)
                MeshRotationY = 0.0f;
        }

        if (MeshRotationY == 0.0f && MeshRotationX == 0.0f)
        {
            TextureTilt = None;
			bCentering = false;
        }
    }
    else if (TextureTilt != None)
        LastRotationTick = ticks;

    if (MeshRotationX > GFC_2PI / 8.0f)
        MeshRotationX = GFC_2PI / 8.0f;
    if (MeshRotationX < -GFC_2PI / 8.0f)
        MeshRotationX = -GFC_2PI / 8.0f;

    if (MeshRotationY >  GFC_2PI / 8.0f)
        MeshRotationY =  GFC_2PI / 8.0f;
    if (MeshRotationY < -GFC_2PI / 8.0f)
        MeshRotationY = -GFC_2PI / 8.0f; 
	
    // Setup the world, view, and projection matrices
    SetupMatrices();

    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
    //pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);


    D3DVIEWPORT9 vp;
    vp.X        = 0;
    vp.Y        = 0;
    vp.Width    = RTWidth;
    vp.Height   = RTHeight;
    vp.MinZ     = 0.0f;
    vp.MaxZ     = 1.0f;
    pDevice->SetViewport(&vp);

    pDevice->BeginScene();

    pDevice->SetPixelShader(pPShaderTex2D);
    pDevice->SetVertexShader(pVShader3D);

    // Blending
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

    pDevice->SetTexture( 0, pRenderTexture );
    pDevice->SetStreamSource( 0, pCubeVertexBuffer, 0, sizeof(Vertex) );
    pDevice->SetFVF( Vertex::FVF );

    pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );

    pDevice->EndScene();
}

// also recreates if lost
bool    FxPlayerApp::SetupRTTexture()
{
    if (FAILED( pDevice->CreateTexture(
        RTWidth,RTHeight,1,
        0, D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT, &pRenderTexture.GetRawRef(), 0) ))
        return 0;

    if (FAILED( pDevice->CreateRenderTarget(
        RTWidth,RTHeight, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, 0, &pRenderSurface.GetRawRef(), 0) ))
        return 0;

    pDevice->CreateDepthStencilSurface( RTWidth,RTHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0,
        TRUE, &pStencilSurface.GetRawRef(), 0);

    pRenderTarget = pPlatformHAL->CreateRenderTarget(pRenderSurface, pStencilSurface, 0);

    return 1;
}

static const char *g_strVertexShaderProgram3D =
"float4x4 mvp : register(c0);\n"
"void main(float4     pos  : POSITION,\n"
"          float2     tc0  : TEXCOORD0,\n"
"          out float4 opos : POSITION,\n"
"          out float2 otc0 : TEXCOORD0)\n"
"{\n"
"  opos = mul(pos, mvp);\n"
"  otc0 = tc0;\n"
"}\n"
;

static const char *g_strPShaderProgramTex2D =
"sampler tex0 : register(s0);\n"
"void main(float2 tc0        : TEXCOORD0,\n"
"          out float4 ocolor : COLOR)\n"
"{\n"
"  ocolor = tex2D(tex0, tc0);\n"
"}\n"
;

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

    // Only short name in settings
    strcpy(Settings.FileName, FXPLAYER_FILENAME);

    bool loadMovie = strlen(Settings.FileName)>0;

    // Get info about the width & height of the movie.
    if (!loadMovie || !Loader.GetMovieInfo(FXPLAYER_FILEPATH, &MovieInfo))
    {
        if (loadMovie)
            fprintf(stderr, "Error: failed to get info about %s\n", Settings.FileName);

        ViewWidth   = FXPLAYER_VIEWWIDTH;
        ViewHeight  = FXPLAYER_VIEWHEIGHT;

        //return 1;
    }
    else
    {
        ViewWidth   = (SInt) (MovieInfo.Width * Settings.ScaleX);
        ViewHeight  = (SInt) (MovieInfo.Height * Settings.ScaleY);
    }

    if (Settings.DoRender)
    {
        // Set options based on arguments
        FullScreen      = Settings.FullScreen;
        FSAntialias     = (Settings.AAMode == FxPlayerSettings::AAMode_FSAA) ? 1 : 0;
        BitDepth        = Settings.BitDepth;

        if (FullScreen)
        {
            Settings.ScaleX = ((Float)FXPLAYER_VIEWWIDTH)  / ViewWidth;
            Settings.ScaleY = ((Float)FXPLAYER_VIEWHEIGHT) / ViewHeight;
            ViewWidth       = FXPLAYER_VIEWWIDTH;
            ViewHeight      = FXPLAYER_VIEWHEIGHT;
        }

        // Enable file drop.
        SupportDropFiles = 1;
        SizableWindow    = 1;

        FxApp::SetupWindowParams extraParams;
        extraParams.ForceTiling=false;
        if (!SetupWindow(FXPLAYER_APP_TITLE, ViewWidth, ViewHeight, extraParams))
            return 1;

        // It is important to initialize these sizes, in case OnSizeEnter gets called.
        SizeWidth  = Width;
        SizeHeight = Height;

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

    // Compile simple 3d vertex shader & simple texture shader.
    GPtr<ID3DXBuffer> pVertexShaderCode;
    GPtr<ID3DXBuffer> pVertexErrorMsg;
    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram3D,
        (UINT)strlen( g_strVertexShaderProgram3D ),
        NULL,
        NULL,
        "main",
        "vs_2_0",
        0,
        &pVertexShaderCode.GetRawRef(),
        &pVertexErrorMsg.GetRawRef(),
        NULL );
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return 0;
    }

    pDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(),
        &pVShader3D.GetRawRef() );

    GPtr<ID3DXBuffer> pPixelShaderCode;
    GPtr<ID3DXBuffer> pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPShaderProgramTex2D,
        (UINT)strlen( g_strPShaderProgramTex2D ),
        NULL,
        NULL,
        "main",
        "ps_2_0",
        0,
        &pPixelShaderCode.GetRawRef(),
        &pPixelErrorMsg.GetRawRef(),
        NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return 0;
    }

    pDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(),
        &pPShaderTex2D.GetRawRef() );

    RTWidth = 512;
    RTHeight = 512;
    SetupRTTexture();

#ifdef FXPLAYER_RENDER_DIRECT3D
    pDevice->CreateVertexBuffer( 4*sizeof(Vertex),0, Vertex::FVF,
        D3DPOOL_MANAGED, &pCubeVertexBuffer.GetRawRef(), NULL );
    void *pVertices = NULL;
    pCubeVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    pCubeVertexBuffer->Unlock();
#endif

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

    // Load movie and initialize timing.
    if (loadMovie && !LoadMovie(FXPLAYER_FILEPATH))
    {
        //return 1;
    }


    while (1)
    {
        TimeTicks = GetTickCount();

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

        // This is technically necessary only for D3D
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
        if (pBG)
        {
            if (!Paused)
                pBG->Advance(deltaT * SpeedScale);
        }

        if (Settings.DoRender)
        {
            pRenderer->BeginFrame();

            SetWireframe(Wireframe);
            RenderMovie();

            // Renderer-specific preparation (Disable depth testing)
            PrepareRendererForFrame();
            GetRenderer()->BeginFrame();

            // Clear the entire buffer.
            Clear(GColor::Blue);
            if (pBG)
            {
                pBG->SetViewport(Width, Height, 0,0, Width, Height);
                pBG->SetBackgroundAlpha(0);
                pBG->Display();
            }
            SetWireframe(0);
            RenderMovieTexture();
        }

        FrameCounter++;


        if (Settings.DoRender)
        {
            SetWireframe(0);

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
            if ((!pMovie || Settings.HudState != FxPlayerSettings::Hud_Hidden) && pHud)
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

                    char buff[512];

                    LastFPS = (delta > 0) ? FrameCounter / delta : 0.0f;

                    // Display frame rate in title
                    sprintf(buff, FXPLAYER_APP_TITLE " (fps:%3.1f)", LastFPS);
                    SetWindowTitle(buff);

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


    pDevice->SetTexture( 0, 0 );
    pDevice->SetPixelShader(0);
    pDevice->SetVertexShader(0);

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
    if (!Loader.GetMovieInfo(pfilename, &newMovieInfo))
    {
        fprintf(stderr, "Error: failed to get info about %s\n", pfilename);
        return 0;
    }

     // Load the actual new movie and crate instance.
    pnewMovieDef = *Loader.CreateMovie(pfilename, GFxLoader::LoadAll);
	pBGDef = *Loader.CreateMovie("D:\\Samples\\3DWindowBackground.swf", GFxLoader::LoadAll);
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

    pMovie->SetFSCommandHandler(GPtr<GFxFSCommandHandler>(*new FxPlayerFSCallback()));

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
    // but only if the source isn't the same buffer.
    if (Settings.FileName != pfilename)
    {
        SInt len = strlen(pfilename);
        for (SInt i=len; i>0; i--)
        {
            if (pfilename[i]=='/' || pfilename[i]=='\\')
            {
                pfilename = pfilename+i+1;
                break;
            }
        }
        strcpy(Settings.FileName, pfilename);
    }


    // Disable pause.
    Paused          = 0;

    // Init timing for the new piece.
    FrameCounter    = 0;
    // Time ticks: always rely on a timer
    TimeStartTicks  = GetTickCount();
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
    if (pRenderer)
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
        float hw = (Float) MovieInfo.Height / (Float) MovieInfo.Width;
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

        if ((ViewWidth <= 640) && (ViewHeight <= 480))
            pAAType = "HW FSAA";
        else
            pAAType = "HW FSAA (Not Supported)";
        break;
    }


    // Update hud text
    sprintf(HudText,
        "Playback Info (X)\n\n"
        "Filename:  %s\n"
        "           SWF %d (%dx%d)\n"
        "           %d/%d frames%s\n"
        "FPS:       %3.1f\n"
        "Triangles: %d\n"
        "Lines:     %d\n"
        "CurveErr:  %3.1f (Ctrl - or +)\n"
        "AA Mode:   %s",

        Settings.FileName,
        MovieInfo.Version,
        MovieInfo.Width, MovieInfo.Height,
        LastFrame, MovieInfo.FrameCount, Paused ? " (Paused)" : "",
        LastFPS,
        LastStats.Triangles,
        LastStats.Lines,
        CurvePixelError,
        pAAType);

    if (Settings.HudState == FxPlayerSettings::Hud_Help)
    {
        strcat(HudText,
            "\n\n"
            "Keys:\n"
            "  Y          Toggle wireframe\n"
            "  X          Toggle HUD\n"
            "  B          Toggle Fast Forward\n"
            "  Start      Toggle pause\n"
            "  Back       Restart the movie\n"
            "  LB         Previous file\n"
            "  RB         Next file\n"
            "  L-Thumb    Toggle anti-aliasing\n"

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


// Helper function to update HUD.
// Uses LastFPS and LastStats; those variables must be updated separately.

void    FxPlayerApp::OnKey(KeyCode keyCode, unsigned char asciiCode, unsigned int wcharCode, 
                           unsigned int mods, bool downFlag)
{
    switch(keyCode)
    {
    case FxApp::Left:
    case FxApp::Right:
    case FxApp::Up:
    case FxApp::Down:
    case FxApp::Return:
        KeyEvent((GFxKey::Code)keyCode, asciiCode, wcharCode, mods, downFlag);
        return;
    case FxApp::F1:
        OnPad(FxApp::Pad_X, downFlag);
        return;
    default:
        break;
    }
    bool contol = mods & KM_Control;
    if (contol)
    {
        switch(keyCode)
        {
            // A key is 'return'
        case 'Q':
        case 'R':
            KeyEvent(GFxKey::Tab,  0, 0, 0, downFlag);
            return;
        case 'A':
            OnPad(FxApp::Pad_L2, downFlag);
            return;
        default:
            break;
        }
    } else {
        KeyEvent((GFxKey::Code)keyCode,  asciiCode, wcharCode, mods, downFlag);
    }
}

void FxPlayerApp::OnPad(PadKeyCode keyCode, bool downFlag)
{
    if (!pMovie || !downFlag)
        return;
    switch(keyCode)
    {
    case FxApp::Pad_Up:
        KeyEvent(GFxKey::Up, 0, keyCode, 0, downFlag);
        return;
    case FxApp::Pad_Right:
        KeyEvent(GFxKey::Right, 0, keyCode, 0, downFlag);
        return;
    case FxApp::Pad_Left:
        KeyEvent(GFxKey::Left, 0, keyCode, 0, downFlag);
        return;
    case FxApp::Pad_Down:
        KeyEvent(GFxKey::Down, 0, keyCode, 0, downFlag);
        return;
    case FxApp::Pad_A:
        KeyEvent(GFxKey::Return, 0, keyCode, 0, downFlag);
        return;
    case FxApp::Pad_R2:
        KeyEvent(GFxKey::Tab, 0, keyCode, 0, downFlag);
        return;

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

    case FxApp::Pad_Back:
        Paused = 0;
        pMovie->GotoFrame(0);
        // Restart the movie, does not seem to work quite right.
        pMovie->Restart();
        break;

        // Toggle info hud.
    case FxApp::Pad_X:
        switch(Settings.HudState)
        {
            case FxPlayerSettings::Hud_Hidden:  Settings.HudState = FxPlayerSettings::Hud_Stats;    break;
            case FxPlayerSettings::Hud_Stats:   Settings.HudState = FxPlayerSettings::Hud_Help;     break;
            case FxPlayerSettings::Hud_Help:    Settings.HudState = FxPlayerSettings::Hud_Hidden;   break;
        }
        NeedHudUpdate = 1;
        break;

    case FxApp::Pad_Y:
        // Toggle wireframe.
        Wireframe = !Wireframe;
        break;

    case FxApp::Pad_B:
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
                Settings.AAMode = FxPlayerSettings::AAMode_None;
                edgeChange = 1;
                //Settings.AAMode = FxPlayerSettings::AAMode_FSAA;
                //edgeChange = renderChange = 1;
                break;
                // We don's support FSAA in the demo because there isn't enough
                // EDRAM to do so easily.
                /*
                case FxPlayerSettings::AAMode_FSAA:
                Settings.AAMode = FxPlayerSettings::AAMode_None;
                renderChange = 1;
                break;
                */
            }
            NeedHudUpdate = 1;


            // FSAA toggle - Left Thumb
            if (renderChange)
            {
                SwitchFSAA(Settings.AAMode == FxPlayerSettings::AAMode_FSAA);
            }

            if (edgeChange && pRenderer && pMovie)
            {
                UInt32 rendererFlags = pRenderConfig->GetRenderFlags() & ~GFxRenderConfig::RF_EdgeAA;
                if (Settings.AAMode == FxPlayerSettings::AAMode_EdgeAA)
                    rendererFlags |= GFxRenderConfig::RF_EdgeAA;
                pRenderConfig->SetRenderFlags(rendererFlags);
            }
        }
        break;

    }
}

// Helper used to convert key codes and route them to GFxPlayer
void FxPlayerApp::KeyEvent(GFxKey::Code keyCode, unsigned char asciiCode,
                           unsigned int wcharCode, unsigned int mods, bool down)
{
    if (keyCode != GFxKey::VoidSymbol) 
    {
        if (pMovie) 
        {
            GFxKeyEvent event(down ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, keyCode, asciiCode, wcharCode);
            event.SpecialKeysState.SetShiftPressed(mods & FxApp::KM_Shift ? 1 : 0);
            event.SpecialKeysState.SetCtrlPressed(mods & FxApp::KM_Control ? 1 : 0);
            event.SpecialKeysState.SetAltPressed(mods & FxApp::KM_Alt ? 1 : 0);
            event.SpecialKeysState.SetNumToggled(mods & FxApp::KM_Num ? 1 : 0);
            event.SpecialKeysState.SetCapsToggled(mods & FxApp::KM_Caps ? 1 : 0);
            event.SpecialKeysState.SetScrollToggled(mods & FxApp::KM_Scroll ? 1 : 0);
            pMovie->HandleEvent(event);
        }
    }
}
