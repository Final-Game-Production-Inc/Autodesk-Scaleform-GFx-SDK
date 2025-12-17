/**************************************************************************

Filename    :   FxPlayerAppBase.h
Content     :   Defines FxPlayerAppBase class, which excludes the HUD.

Created     :   November, 2010
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko, 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef FX_PlayerAppBase_H
#define FX_PlayerAppBase_H

#include "Kernel/SF_Types.h"
#include "Platform/Platform.h"
#include "Platform/Platform_RenderThread.h"
#include "GFx.h"
#include "GFx/GFx_TaskManager.h"
#ifdef GFX_ENABLE_XML
  #include "GFx/XML/XML_Document.h"
#endif

#ifdef GFX_ENABLE_VIDEO
#include "../Bin/Data/AS2/FxPlayer/fxvideoplayer.swf.h"
#ifdef GFX_VIDEO_USE_WWISE
#include "Video/Video_SystemSoundInterface.h"
#endif
#endif

// Adds the word "Debug" to the application
// title if in debug build mode
#ifdef SF_BUILD_DEBUG
    #define SF_DEBUG_STRING    " " SF_BUILD_STRING
#else
    #define SF_DEBUG_STRING
#endif

//#define FXPLAYER_LOGO
#ifdef FXPLAYER_LOGO
#define FXPLAYER_LOGO_BORDER 40
#else
#define FXPLAYER_LOGO_BORDER 0
#endif

//------------------------------------------------------------------------
// TBD: Strip/Cleanup

#if defined(FXPLAYER_X11)
    #undef None
    #undef Status
    #include <unistd.h>

#elif defined(SF_OS_WIN32)
    #ifdef  FXPLAYER_RENDER_DIRECT3D
      #ifndef SF_D3D_VERSION
      #define SF_D3D_VERSION 9
      #endif
      #undef FXPLAYER_RENDER      
    #else
        // OpenGL application class
    #endif

#elif defined(SF_OS_MAC)
    // OpenGL application class
    #undef None
    #include <unistd.h>

#elif defined(SF_OS_XBOX360)
    // Direct3D application class
    #define FXPLAYER_RENDER_DIRECT3D
    #define FXPLAYER_FONT_SIZE          10

    #if _XDK_VER >= 9328
        #ifdef SF_BUILD_DEBUG
            #pragma comment(lib, "xmahald")
            #pragma comment(lib, "xuirund")
            #pragma comment(lib, "xuirenderd")
        #else
            #pragma comment(lib, "xmahal")
            #pragma comment(lib, "xuirun")
            #pragma comment(lib, "xuirender")
        #endif
    #else
        #ifdef SF_BUILD_DEBUG
            #pragma comment(lib, "xactd")
            #pragma comment(lib, "xaudiod")
        #else
            #pragma comment(lib, "xact")
            #pragma comment(lib, "xaudio")
        #endif
    #endif

#elif defined (SF_OS_PS3)
    #include <sys/paths.h>
    #include <sys/spu_initialize.h>
    #include <sys/spu_thread.h>
    #include <sys/spu_thread_group.h>
    #include <sys/ppu_thread.h>
    #include <sys/process.h>
    #include <cell/sysmodule.h>
    #include <cell/spurs.h>
    #include <cell/spurs/control.h>
    #define FXPLAYER_FONT_SIZE          10

#elif defined (SF_OS_WII)
   #define FXPLAYER_FONTCACHE_SMALL
   #define FXPLAYER_FONT_SIZE          10

#endif

#define DEFAULT_FONT_LIB_NAME "fonts_en.swf"

#include "GFx_AMP.h"
#include "FxRenderThread.h"
#include "FontConfigParser.h"
#include "FxSoundFMOD.h"
#include "FxSoundWwise.h"


// MA: For now, Consider separate header.
//bool FindNextSiblingFile(char* filename, bool prev);


namespace SF = Scaleform;
using namespace Scaleform;
//using namespace Render;
using namespace GFx;

#if defined(GFX_ENABLE_SOUND)
using namespace Sound;
#endif

// ***** Forward Declarations
class FxPlayerCommand;
class FxRenderThread;
namespace Scaleform {
class FxPlayerAutoTest;
}

#include "GFx/AMP/Amp_Interfaces.h"


//------------------------------------------------------------------------
// ***** FxPlayerAppBase

class FxPlayerAppBase : public Platform::App<FxRenderThread>
#ifdef SF_AMP_SERVER
    , public AMP::AppControlInterface
#endif
{
public:
    typedef Platform::App<FxRenderThread> BaseClass;

    // Loaded movie data
    Loader              mLoader;
    MovieInfo           mMovieInfo;
    Ptr<MovieDef>       pMovieDef;
    Ptr<Movie>          pMovie;


    // ***** Movie Timing State

    // Accumulated timing us used to enable FastForward, Pause and other
    // time adjusted values without wrap-around.
    UInt64              AccumClockTicks;
    UInt64              PrevTicks; 

    // Current FPS and update tracking.
    // LastFPSUpdatedTicks and LastStatUpdatedTicks work off AccumClockTicks.    
    // We allow stats to be updated at higer rate then FPS.
    UInt64              LastFPSUpdatedTicks;
    UInt64              LastStatUpdatedTicks;
    int                 FrameCounter;
    float               LastFPS;
    float               AccumFPS;
    int                 AccumFPSSecondCount;

    // Advance timings recorded for last frame.
    UInt64              LastAdvanceTicks;    


    // Movie viewport width and height, updated based on scale and/or state.
    // This value is often smaller then the GetViewSize(), as the later covers
    // all of the window client area.
    Render::Size<unsigned>  MViewSize;
    Render::Size<unsigned>  MinSize;

    // Set if wire frame ins enabled.
    bool                    Wireframe;
    bool                    AmpDirty;
    // Scale toggle, on by default
    bool                    ScaleEnable;
    bool                    ClippingEnable;

#ifdef SF_AMP_SERVER
    Lock                                        AmpCallbackLock;
    Array< Ptr<GFx::AMP::MessageAppControl> >   AmpAppControlMsgs;
    UInt32                                      AmpPort;
#endif
    // Rendering state
    bool                  PlayOnce;
    bool                  Rendering;
    bool                  DoSound;
    bool                  NoControlKeys;    //Disable all keyboard shortcuts
    bool                  NoCtrlKey;        // Disable handling of Ctrl key
    Render::ProfilerModes ProfileMode;      // The current profile mode.
    int                   BatchHighlight;   // In Batch profile mode, specifies the index of the batch that should be highlighted (-ve to disable highlighting).

    //Sound properties 
    float               SoundVolume;
    bool                IsMute;
    bool                bCLIKSound;

    // Viewport transformation
    float               Zoom;
    float               ZoomStart;
    PointF              Move;
    PointF              MoveStart;


    enum TrackingState
    {
        None,
        Zooming,
        Moving,
    };

    TrackingState           MouseTracking;
    Point<int>              MouseDownPos;
    Point<int>              MousePrevPos;
    
    enum PadStickMode
    {
        PSM_NoStick,
        PSM_NoPad,
        PSM_StickAndPad,
        PSM_Count
    };
    
    int                     StickMode;
    
    // This variable is set when the movie is paused in the player.
    bool                    Paused;
    // Store playstate when paused, so that we can restore it.
    PlayState               PausedState;

    enum AAModeType
    {
        AAMode_None,        // No anti-aliasing is used.
        AAMode_EdgeAA,      // Edge AA is used (if supported by Renderer).
        AAMode_FSAA         // HW Full-screen AA is used (if supported by device).
    };

    

    // Command line argument resolution; not specified if Null {0,0}.
    Size<unsigned>          ArgResolution;
    float                   ScaleX, ScaleY;
    AAModeType              AAMode;
    bool                    Background;
    float                   TexLodBias;

    unsigned                BackgroundColor;

    // Set to play movie as fast as possible
    bool                    FastForward;
    bool                    MeasurePerformance;
    float                   ExitTimeout;

    // Automated Testing (recording/playback)
    FxPlayerAutoTest*       AutoTest;

    Render::Matrix2F        UserMatrix;

    // Curve error
    float                   CurvePixelError;

    // Width, height during sizing
    bool                    SizingEntered;
    Size<unsigned>          SizingSize;

    // Last cursor type applied by Flash movie.
    Platform::CursorType    MovieCursorType;

    // PlaybackFile
    Platform::Directory     ContentDir;
    String                  FileName;
    bool                    SkipLoadMovie;

    bool                    FontLibChanged;
    bool                    PadPassthrough;
    // FontConfigFile
    String                  FontConfigFilePath;
    FileStat                FontConfigFileStats;
    // Font configurations, if specified.
    FontConfigSet           FontConfigs;
    // Index of currently applied FontConfig, -1 for none.
    int                     FontConfigIndex;
    UInt8                   KeyboardIndex;

    unsigned ResIndex;

#ifdef GFX_ENABLE_SOUND
#ifdef GFX_SOUND_FMOD
    FxSoundFMOD*    pSoundFMODSystem;
#endif
#ifdef GFX_SOUND_WWISE
    FxSoundWwise*   pSoundWwiseSystem;
#endif
#endif

#ifdef GFX_VIDEO_USE_WWISE
    Ptr<Video::VideoSoundSystem> pWwise;
#endif

#ifdef SF_ENABLE_THREADS
    Ptr<ThreadedTaskManager> pTaskManager;
#endif
    Hash<UInt32, Ptr<FxPlayerCommand> >     KeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > PadKeyCommandMap;
    Hash<PadKeyCode, Key::Code > PadPassthroughMap;

    FxPlayerAppBase();
    ~FxPlayerAppBase();

    virtual String          GetAppTitle() const;
    virtual String          GetFilePath() const;
    virtual void            UpdateWindowTitle();
    // Overriden to support HUD.
    virtual void            UpdateStatReports() { UpdateWindowTitle(); }
    virtual void            UpdateFpsDisplay();

    //------------------------------------------------------------------------
    virtual bool            OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult);
    virtual void            InitArgDescriptions(Platform::Args* args);

    // Main Program logic used by App.
    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnAutoUpdateFrame();
    virtual void            OnShutdown();
    virtual void            OnPause() {}

    virtual void            OnRenderThreadCreated();

#ifdef SF_AMP_SERVER
	virtual void			GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const;
#endif
    
    // *** App Event Overrides
 
    // Sizing; by default, re-initalizes the renderer
    virtual void            OnSize(const Size<unsigned>& newSize);
    virtual void            OnSizeEnter(bool enterFlag);
    virtual void            OnDropFiles(const String& path);
    virtual void            OnConfigurationChange(const Platform::ViewConfig& config);

    // Input
    virtual void            OnKey(unsigned controllerIndex, KeyCode keyCode,
                                  unsigned wcharCode, bool downFlag, KeyModifiers mods);
    virtual void            OnPad(unsigned controllerIndex, PadKeyCode padCode, bool downFlag);
    virtual void            OnPadStick(unsigned controllerIndex, PadKeyCode padCode, float xpos, float ypos);
    virtual bool            OnIMEEvent(unsigned message, UPInt wParam, UPInt lParam, UPInt hWND, bool preprocess);
    virtual void            OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
                                          const Point<int>& mousePos, KeyModifiers mods);    
    virtual void            OnMouseMove(unsigned mouseIndex,
                                        const Point<int>& mousePos, KeyModifiers mods);
    virtual void            OnMouseWheel(unsigned mouseIndex, float zdelta,
                                         const Point<int>& mousePos, KeyModifiers mods);

    virtual void            OnTouchBegin(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);
    virtual void            OnTouchEnd(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);
    virtual void            OnTouchMove(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);

    virtual void            OnGestureBegin(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
                                                const PointF& translationDelta,
                                                const PointF& scaleDelta,
                                                float rotationDelta);
    virtual void            OnGesture(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
                                           const PointF& translationDelta,
                                           const PointF& scaleDelta,
                                           float rotationDelta);
    virtual void            OnGestureEnd(unsigned window, UInt32 gestureMask, const Point<int>& pos);

    virtual void            OnUpdateCliboard(const wchar_t* text);
    // if set == true then this is SET FOCUS, otherwise - KILL FOCUS
    virtual void            OnFocus(bool setFocus, KeyModifiers mods);

    //------------------------------------------------------------------------
    
    virtual void            SetMovieVariables(String& argString);
    bool                    ValidateDeviceStatus();
    virtual bool            AdvanceMovie(float deltaT);

    //virtual void            ResetStats();
    virtual FxPlayerCommand* KeyToCommand(UInt32 key);
    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);

    // Load a new movie from a file and initialize timing
    virtual bool            LoadMovie(const String& filename);

    // Sets application Frame time based on settings such as FastForward,
    // MesuerPerformance Pause and VSync.
    void                    AdjustFrameTime();
#ifdef FXPLAYER_LOGO
    Size<unsigned>  GetViewSize() const;
#endif
    // Updates the view size based on the ScaleEnable flag and window size.
    virtual void            UpdateViewSize();

    void                    ResetUserMatrix();
    void                    UpdateUserMatrix();
    //  SizeF                   GetMovieScaleSize();
    Render::PointF          AdjustToViewPort(const Point<int>& pos);
    Render::Point<int>      AdjustMovieDefToViewPort(const Point<int>& pos);
    // *** Overrides
#ifdef SF_AMP_SERVER
    virtual bool            HandleAmpRequest(const GFx::AMP::MessageAppControl* message);
#endif

    virtual void            OnPause(bool paused);
    virtual void            OnObjectsReport();

#ifdef GFX_ENABLE_SOUND
    virtual void            InitializeSound();
    virtual void            ShutdownSound() { xShutdownSound(); }
    void                    xShutdownSound(); // avoid call to a virtual method from the destructor
    virtual Sound::SoundRenderer* GetSoundRenderer();
#endif

    // Helper used to convert key codes and route them to GFxPlayer
    virtual void            OnKeyEvent(unsigned controllerIndex, KeyCode keyCode,
                                       unsigned wcharCode, bool downFlag, KeyModifiers mods = KeyModifiers());


    void                    LoadFontConfigs(ConfigParser *parser);
    FontConfig*             GetCurrentFontConfig();
    int                     GetFontConfigIndexByName(const char* pname);
    virtual bool            LoadDefaultFontConfigFromPath(const String& path);


    void                    SetRendererProfiling(bool on);
    virtual bool            TakeScreenShot( const String& filename );
    void                    ToggleAaMode();
    void                    ToggleStrokeType();
    void                    CycleFontConfig();
#ifdef SF_AMP_SERVER
    void                    HandleAmpAppMessages();
    void                    UpdateAmpState();
#endif


    void                    ProcessCommand(unsigned controllerIdx, const FxPlayerCommand& cmd, bool downFlag);
    void                    OnKeyFinishSeek();
    void                    ApplyLanguage(const char* name);
    virtual void            InstallHandlers();
    virtual void            SetFileOpener();
    virtual void            SetFontProvider();
    virtual bool            HandleFsCommand(const char* pcommand, const char* parg);
    virtual bool            HandleEvent(const GFx::Event& event);
    virtual bool            IsImeEnabled() const;

#ifdef GFX_ENABLE_VIDEO
    bool                    IsVideoFile(const char* pfn) const { return strstr(pfn, ".usm") != 0; }
#endif

    // Restarts the current movie and registers variables before first advance0
    void                    SafeMovieRestart();
};



//------------------------------------------------------------------------

class FxPlayerUserEventHandler : public UserEventHandler
{
    FxPlayerAppBase* pApp;

public:

    FxPlayerUserEventHandler(FxPlayerAppBase *papp)
    {
        pApp = papp;
    }

    virtual void HandleEvent(Movie* pmovie, const GFx::Event& event)
    {
        SF_UNUSED(pmovie);
        pApp->HandleEvent(event);
    }

};

// File opener class.
class FxPlayerFileOpener : public FileOpener 
{
public:
    virtual File* OpenFile(const char *purl, int flags, int mode) 
    {
#ifdef GFX_ENABLE_VIDEO
        if (!strcmp(purl, "_internal_video_player"))
            return new MemoryFile(purl, fxvideoplayer_swf, sizeof(fxvideoplayer_swf));
#endif
        // Buffered file wrapper is faster to use because it optimizes seeks.
        return FileOpener::OpenFile(purl, flags, mode);
    }
};

// "fscommand" callback, handles notification callbacks from ActionScript.
class FxPlayerFSCallback : public FSCommandHandler
{
public:
    FxPlayerFSCallback(FxPlayerAppBase* app) : App(app) { }
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg);
protected:
    FxPlayerAppBase* App;
};


//------------------------------------------------------------------------
#define FXPLAYERCOMMAND_GETCOMMANDNAME(x) \
    virtual const char* GetCommandName() const \
{ \
    static const char* name = #x; \
    return name; \
}

class FxPlayerCommand : public RefCountBase<FxPlayerCommand, Stat_Default_Mem>
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const = 0;
    virtual String GetHelpString() const = 0;
    virtual const char *GetCommandName() const { return ""; }
    virtual bool HandleKeyUp() const { return false; }
#ifdef GFX_ENABLE_XML
    // Alternate execute, which can take parameters via XML attributes.
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, GFx::XML::ElementNode* parameters) const
    {
        // Default implementation just calls regular execute.
        SF_UNUSED(parameters);
        Execute(app, controllerIdx, keyDown);
    }

    virtual void RecordAndExecute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, GFx::XML::ElementNode* parameters = 0) const;
#endif

};

#endif // FX_PlayerAppBase_H
