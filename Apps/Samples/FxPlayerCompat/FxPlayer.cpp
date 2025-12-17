/**************************************************************************

Filename    :   FxPlayer.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Maxim Didenko, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "GFxCompat.h"
#include "GFx.h"
#include "GFx_AMP.h"
#include "GFx_XML.h"
#include "GFx_Render.h"
#include "../Src/Kernel/SF_MsgFormat.h"
#include "../Src/Kernel/SF_Atomic.h"
#include "FxAmpAppControlCallback.h"


// Progressive loading loads content on background threads by using
// GFxTaskManager. Enable it only on systems with thread support.
#if !defined(GFC_NO_THREADSUPPORT)
    #define FXPLAYER_PROGRESSIVE_LOADING  1   
    #include "../Src/GFx/GFx_TaskManager.h"
#endif

// FxPlayerConfig defines FxPlayerCommand enumeration with command mappings
// to keyboard/gamepad buttons and provides a family of macros based on the
// platform we are compiling for. Important macros defines here include:
//
//  - FXPLAYER_APP              - Base application class we will use.
//  - FXPLAYER_APP_TITLE        - Title-bar message.
//  - FXPLAYER_FILEDIRECTORY    - Platform specific sample file directory.
//  - FXPLAYER_FILENAME         - Initial file to load, if any.
//  - FXPLAYER_VIEWWIDTH/HEIGHT - Size of window or video-mode.
//
#include "FxPlayerConfig.h"
#include "FontConfigParser.h"

// Other player implementation helper classes.
//#include "FxPlayerSettings.h"
#include "FxPlayerLog.h"

#ifndef GFC_NO_SOUND
#include "FxSoundFMOD.h"
#include "FxSoundWwise.h"
#endif

#include "GFx_Sound_FMOD.h"
#include "GFx_Sound_WWise.h"

#ifdef GFC_USE_VIDEO
    #include "../Bin/FxPlayer/fxvideoplayer.swf.h"
    #include "GFxVideo.h"
    #if defined(GFC_OS_PS3)
        #include "GFxVideoPS3.h"
    #elif defined(GFC_OS_XBOX360)
        #include "GFxVideoXbox360.h"
    #elif defined(GFC_OS_WIN32)
        #include "GFxVideoPC.h"
    #endif
    #ifdef GFC_VIDEO_USE_SYSTEM_SOUND
        #ifdef GFC_OS_WIN32
            //#include "GFxVideoSoundSystemXA2.h"
            #include "GFxVideoSoundSystemDX8.h"
        #elif defined(GFC_OS_XBOX360)
            #include "GFxVideoSoundSystemXA2.h"
        #elif defined(GFC_OS_WII)
            #include "GFxVideoSoundSystemWii.h"
        #elif defined(GFC_OS_PS3)
            #include "GFxVideoSoundSystemPS3.h"
        #endif
    #elif defined(GFC_VIDEO_USE_WWISE)
        #include "GFxVideoSoundSystemWwise.h"
    #endif
#endif

#ifdef FXPLAYER_LOGO
#define LOGO_OFFSET 2
#define LOGO_BORDER 36
#include "GImageInfo.h"
#include "GFxLogo.cpp"
#endif

#include "FxHUD.h"

#if !defined(GFC_NO_FXPLAYER_AS_SHAREDOBJECT)
#include "FxSharedObjectManager.h"
#endif  // GFC_NO_FXPLAYER_AS_SHAREDOBJECT

#ifdef __cplusplus_cli
#using <mscorlib.dll>
using namespace System;
#endif

// ***** Player Application class

class   FxPlayerApp : public FXPLAYER_APP, public FxHUDInfoProvider, FxHUDEventHandler
{
public:

     // Loaded movie data
    GFxLoader               Loader;
    GFxMovieInfo            MovieInfo;
    GPtr<GFxMovieDef>       pMovieDef;
    GPtr<GFxMovieView>      pMovie;

    // Movie timing state
    float                   SpeedScale;         // Advance speed, def 1.0f
    SInt                    FrameCounter;       // Frames rendered, for FPS
    UInt                    TessTriangles;      // Tess triangles for log.
    // Time ticks: always rely on a timer, for FPS
    UInt64                  TimeStartTicks;     // Ticks during the start of playback
    UInt64                  TimeTicks;          // Current ticks
    UInt64                  LastLoggedFps;      // Time ticks during last FPS log
    UInt64                  NextTicksTime;      // Ticks when next advance should be called.
    // Movie logical ticks: either timer or setting controlled
    UInt64                  MovieStartTicks;
    UInt64                  MovieLastTicks;    
    UInt64                  MovieTicks;
    UInt64                  HudLastTicks;

    // Renderer we use
    GPtr<GFxRenderConfig>   pRenderConfig;
    GPtr<GFxRenderStats>    pRenderStats;

    bool        bDisplayedMasksWarning;

#ifdef FXPLAYER_APP_SOUND
    FXPLAYER_APP_SOUND*     pSoundSystem;
#endif

    // Selected playback settings

    GFxValue                TransientValue;

    // View width and height
    SInt                    ViewWidth, ViewHeight;

    // Set if wire frame ins enabled.
    bool                    Wireframe;
    bool                    AmpDirty;
    //Show safe rectangle borders
    //bool                    SafeRectEnabled;
    // Scale toggle, on by default
    bool                    ScaleEnable;

    bool                    ClippingEnable;

    GLock                                   AmpCallbackLock;
    GArray< GPtr<GFxAmpMessageAppControl> > AmpAppControlMsgs;
    UInt32                                  AmpPort;
    bool                                    AmpWait;

    // Rendering state
    bool                DoLoop;
    bool                DoRender;
    bool                DoSound;
    bool                NoControlKeys; //Disable all keyboard shortcuts
    bool                NoCtrlKey; // Disable hadnling of Ctrl key
    bool                OverdrawProfile;

    //Sound properties 
    Float                   SoundVolume;
    bool                    IsMute;
    bool                    bCLIKSound;

    // Viewport transformation
    Float                   Zoom;
    Float                   ZoomStart;
    GPointF                 Move;
    GPointF                 MoveStart;

    enum TrackingState
    {
        None,
        Zooming,
        Moving,
    };

    TrackingState           MouseTracking;
    SInt                    MouseX;
    SInt                    MouseY;
    SInt                    MouseDownX;
    SInt                    MouseDownY;

    // This variable is set when the movie is paused in the player.
    bool                    Paused;
    // Store playstate when paused, so that we can restore it.
    GFx::PlayState          PausedState;

    enum AAModeType
    {
        AAMode_None,        // No anti-aliasing is used.
        AAMode_EdgeAA,      // Edge AA is used (if supported by GRenderer).
        AAMode_FSAA         // HW Full-screen AA is used (if supported by device).
    };

    //    HudStateType    HudState;
    int                     ResolutionX, ResolutionY;
    Float                   ScaleX, ScaleY;
    AAModeType              AAMode;
    bool                    Background;
    Float                   TexLodBias;

    UInt                    BackgroundColor;

    // Set to play movie as fast as possible
    bool                    FastForward;
    bool                    MeasurePerformance;
    Float                   ExitTimeout;
    UInt                    SleepDelay; // Some consoles use this
    
    // Last FPS and stats
    Float                   LastFPS;
    Float                   LastAdvance;
    Float                   LastDisplay;

	UInt64					LastHUDUpdate;

    GRenderer::Matrix       UserMatrix;

    bool                    RedirectEventsToHud;

    GPtr<FxHUDDataProvider> pHudDataProvider;
    GPtr<FxHUD>             pHud;

    GMemoryHeap*            pHudHeap;

    GPtr<FxAmpAppControlCallback> pAppControlCallback;

    // Curve error
    Float                   CurvePixelError;

    // Width, height during sizing
    SInt                    SizeWidth, SizeHeight;
    bool                    SizingEntered;

#ifdef FXPLAYER_LOGO
    GPtr<GImageInfo>         pDummyImage;
#endif

    // PlaybackFile
    char                    FileName[1024];

    bool                    FontLibChanged;

    // FontConfigFile
    GString                 FontConfigFilePath;
    GFileStat               FontConfigFileStats;
    // Font configurations, if specified.
    FontConfigSet           FontConfigs;
    // Index of currently applied FontConfig, -1 for none.
    SInt                    FontConfigIndex;

    UInt8                   KeyboardIndex;

#ifndef GFC_NO_SOUND
    FxSoundFMOD*    pSoundFMODSystem;
    FxSoundWwise*   pSoundWwiseSystem;
#endif

    FxPlayerApp();
    ~FxPlayerApp()         { xShutdownSound(); }

    virtual bool           OnArgsParsed(const FxArgs& args);
    virtual void           InitArgDescriptions(FxArgs* args);

    // Called from main() after settings are initialized to execute
    // most of the program logic. Responsible for setting up the window,
    // loading movies and containing the message loop.
    virtual int             Run();

    // Load a new movie from a file and initialize timing
    bool                    LoadMovie(const GString& filename);
    void                    UnloadMovie();

    // Used by HUD to display statisctics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(GString* phelpStr);

    // Invoked when HUD becomes active/inactive
    virtual void            OnHudStatusChange(bool active) { GUNUSED(active); }

    // Updates the view size based on the ScaleEnable flag and window size.
    void                    UpdateViewSize();
	void					UpdateHUDViewport();

    void                    ResetUserMatrix();
    void                    UpdateUserMatrix();
    GSizeF                  GetMovieScaleSize();

    // *** Overrides

    virtual void            OnAmpAppControlToggle(const GFxAmpMessageAppControl* message);

    // Sizing; by default, re-initalizes the renderer
    virtual void            OnSize(int w, int h);
    virtual void            OnSizeEnter(bool enterSize);
    virtual void            OnDropFiles(const GString& path);
    

    // Input
    virtual void            OnKey(UInt controllerIdx, KeyCode, unsigned char asciiCode, unsigned int wcharCode, 
                                  unsigned int mods, bool downFlag);
    virtual void            OnPad(UInt controllerIdx, PadKeyCode, bool downFlag);
    virtual bool            OnIMEEvent(const FxAppIMEEvent&);
    virtual void            OnChar(UInt controllerIdx, UInt32 wcharCode, UInt info);
    virtual void            OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, 
                                          int x, int y, unsigned int mods);
    virtual void            OnMouseWheel(unsigned mouseIndex, int zdelta, int x, int y, unsigned int mods);
    virtual void            OnMouseMove(unsigned mouseIndex, int x, int y, int unsigned mods);
    // if set == true then this is SET FOCUS, otherwise - KILL FOCUS
    virtual void            OnFocus(bool set, unsigned int mods);
    
    virtual void            OnPause(bool paused);

    virtual void            InitializeSound();
    virtual void            ShutdownSound() { xShutdownSound(); }
    void                    xShutdownSound(); // avoid call to a virtual method from the destructor
    virtual GSoundRenderer* GetSoundRenderer();

    // Helper used to convert key codes and route them to GFxPlayer
    void                    KeyEvent(UInt controllerIndex, GFxKey::Code keyCode, unsigned char asciiCode, 
                                     unsigned int wcharCode, unsigned int mods, bool down);
    
    virtual void            OnUpdateSystemCliboard(const wchar_t* text);
    
    void                    LoadFontConfigs(ConfigParser *parser);
    FontConfig*             GetCurrentFontConfig();
    int                     GetFontConfigIndexByName(const char* pname);
    bool                    LoadDefaultFontConfigFromPath(const GString& path);

    void                    SetRendererProfiling(bool on);
    void                    ToggleAaMode();
    void                    ToggleStrokeType();
    void                    CycleFontConfig();
    void                    HandleAmpAppMessages();
    void                    UpdateAmpState();

    void                    ProcessCommand(UInt controllerIdx, FxPlayerCommand cmd, void* data  = NULL);
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
        GUNUSED(pmovie);
        switch(event.Type)
        {
        case GFxEvent::DoShowMouse:
            {
                const GFxMouseCursorEvent& mcEvent = static_cast<const GFxMouseCursorEvent&>(event);
                if (mcEvent.MouseIndex == 0)
                    pApp->ShowCursor(true);

            }
            break;
        case GFxEvent::DoHideMouse:
            {
                const GFxMouseCursorEvent& mcEvent = static_cast<const GFxMouseCursorEvent&>(event);
                if (mcEvent.MouseIndex == 0)
                    pApp->ShowCursor(false);
            }
            break;
#if defined(GFC_OS_WIN32)
// Older window do not define this.
#ifndef IDC_HAND
#define IDC_HAND IDC_ARROW
#endif
        case GFxEvent::DoSetMouseCursor:
            {
                const GFxMouseCursorEvent& mcEvent = static_cast<const GFxMouseCursorEvent&>(event);
                if (mcEvent.MouseIndex == 0)
                {
                    switch(mcEvent.mCursorShape)
                    {
                    case GFxMouseCursorEvent::ARROW:
                        pApp->SetCursor(::LoadCursor(NULL, IDC_ARROW));
                        break;
                    case GFxMouseCursorEvent::HAND:
                        pApp->SetCursor(::LoadCursor(NULL, IDC_HAND));
                        break;
                    case GFxMouseCursorEvent::IBEAM:
                        pApp->SetCursor(::LoadCursor(NULL, IDC_IBEAM));
                        break;
                    }
                }
            }
            break;
#endif
        default:
            break;
        }
    }

};


void    FxPlayerApp::LoadFontConfigs(ConfigParser *parser)
{
    FontConfigs.Parse(parser);
    if (FontConfigs.GetSize() > 0)
    {
        FontConfigIndex = 0;
    }
    else
    {
        FontConfigIndex = -1;
    }
}

int     FxPlayerApp::GetFontConfigIndexByName(const char* pname)
{
    for (UInt i = 0; i < FontConfigs.GetSize(); i++)
    {
        if (G_stricmp(pname, FontConfigs[i]->ConfigName) == 0)
            return i;
    }   
    return -1;
}

FontConfig*  FxPlayerApp::GetCurrentFontConfig()
{
    // we are skipping over invalid fontconfigs until one is found.
    // else return NULL.

    if (FontConfigIndex == -1)
        return NULL;

    FontConfig* fc = NULL;
    SInt sIdx = FontConfigIndex;
    bool ok = false;

    while (!ok)
    {
        ok = true;
        fc = FontConfigs[FontConfigIndex];
        // check if all fontlib files exist
        for (UInt i=0; i < fc->FontLibFiles.GetSize(); i++)
        {
            // check if file exists
            GSysFile file(fc->FontLibFiles[i]);
            if (!file.IsValid())
            {
                ok = false;
                fprintf(stderr, "Fontlib file '%s' cannot be found. Skipping config '%s'..\n", fc->FontLibFiles[i].ToCStr(), fc->ConfigName.ToCStr());
                break;
            }
        }

        if (!ok)
        {
            FontConfigIndex++;
            FontConfigIndex %= (SInt)FontConfigs.GetSize();
            if (FontConfigIndex == sIdx)
                return NULL;
        }
    }

    return FontConfigs[FontConfigIndex];
}

bool FxPlayerApp::LoadDefaultFontConfigFromPath( const GString& path )
{
    if (!Arguments["NoFontConfig"]->GetBool())
    {
        // load fontconfig.txt if it exists in the movie's path
        GString fontConfigFilePath;

        // [PPS] The absolute check is unneccessary
        // ExtractFilePath will return the correct parent path:
        // C:/folder/filename.ext - C:/folder/
        // folder/filename.ext - folder/
        // filename.ext - empty string
        //if (GFxURLBuilder::IsPathAbsolute(path))
        //{
        fontConfigFilePath.AppendString(path);
        if ( !GFxURLBuilder::ExtractFilePath(&fontConfigFilePath) )
        {
            fontConfigFilePath = "";
        }
        //}

        fontConfigFilePath += "fontconfig.txt";
        bool maintainIndex = false;

        // store font config file related info
        if (FontConfigFilePath.GetLength() == 0)   // if no file was previously loaded
        {
            GFileStat fileStats;
            if ( GSysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
            {
                FontConfigFilePath = fontConfigFilePath;
                FontConfigFileStats = fileStats;
            }
        }
        else // if the file was previously loaded and is modified
        {
            if (fontConfigFilePath == FontConfigFilePath)
            {
                // if modified time is not the same, then reload config file
                GFileStat fileStats;
                if ( GSysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
                {
                    if ( !(fileStats == FontConfigFileStats) )
                    {
                        FontConfigFileStats = fileStats;
                        maintainIndex = true;
                    }
                }
            }
        }

        // parse the config file
        ConfigParser parser(fontConfigFilePath.ToCStr());
        SInt oldIdx = FontConfigIndex;
        LoadFontConfigs(&parser);

        // try to maintain previous font config index
        if ( maintainIndex &&
            (FontConfigIndex == 0) &&
            (oldIdx != -1) )
        {
            FontConfigIndex = oldIdx;
            FontConfigIndex %= (SInt)FontConfigs.GetSize();
        }

        return true;
    }
    return false;
}


// "fscommand" callback, handles notification callbacks from ActionScript.
class FxPlayerFSCallback : public GFxFSCommandHandler
{
public:
    virtual void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg)
    {
        GPtr<GFxLog> plog = pmovie->GetLog();
        if (plog)
        {
            plog->LogMessage("FsCallback: '");
            plog->LogMessage("%s", pcommand);
            plog->LogMessage("' '");
            plog->LogMessage("%s", parg);
            plog->LogMessage("'\n");
        }

        FxPlayerApp* pApp = (FxPlayerApp*)pmovie->GetUserData();

        if (G_strcmp(pcommand, "GFxApplyLanguage") == 0)
        {
            if (parg != NULL)
            {
                int idx = pApp->GetFontConfigIndexByName(parg);
                if (idx != -1)
                {
                    pApp->FontConfigIndex = idx;
                    FontConfig* pconfig = pApp->FontConfigs[pApp->FontConfigIndex];
                    pconfig->Apply(&pApp->Loader);
                    const char* language = pconfig->ConfigName.ToCStr();
                    //pmovie->SetVariable("_global.gfxLanguage", GFxValue(language));
                    pmovie->Invoke("_root.OnLanguageChanged","%s",language);
                }
                else
                {
                    if (plog)
                    {
                        plog->LogError("\nLanguage support for '%s' not found!\n\n", parg);
                    }
                }
                pApp->FontLibChanged = true;
            }
        }
        else
           if (G_strcmp(pcommand, "wireframe") == 0)
           {
               if (parg != NULL)
               {
                    pApp->Wireframe = (atoi(parg) != 0);
                    pApp->AmpDirty = true;
               }
           }

    }
};



/** 
A sample sound event callback processor.  This class can be used as a function object
to override _global.gfxProcessSound for sound playback. Overridng the _global.gfxProcessSound
method will, by default, cause most CLIK Components to send sound events to this class. It can
also be used to trigger sound playback from ActionScript using _global.gfxProcessSound().

Note that the implementation of this function is at the discretion of the developer.
*/
#if 0   // !defined(GFC_NO_SOUND)
class GFxSoundEventCallback : public GFxFunctionContext
{
public:
    GFxSoundEventCallback (const GFxMovieView* movieView) : pMovieView(movieView)
    {   
        pApp = (FxPlayerApp*)pMovieView->GetUserData();  

    #if defined(GFC_SOUND_FMOD) && !defined(GFC_SOUND_WWISE)
        if (pApp->pSoundFMODSystem)
        {   // FMOD Ex (implementation based on GSoundRender)
            pSoundEvent = *new GSoundEventFMOD;    
            pSoundEvent->pUserData = pApp->pSoundFMODSystem->pSoundRenderer;
            // FMOD Designer
            //pSoundEvent = *new GSoundEventFMODDesigner("examples.fev", "examples/FeatureDemonstration");
            //pSoundEvent->pUserData = pApp->pSoundFMODSystem->pSoundRenderer->GetFMODEventSystem();
        }
    #elif defined(GFC_SOUND_WWISE)
        // AK Wwise SoundFrame
        pSoundEvent = *new GSoundEventWwiseSF; 
    #endif
        bCLIKSound = pApp->bCLIKSound; // Is CLIK soundEvent processing enabled ("-cksd" command line parameter)
        pLog = pMovieView->GetLog();   // Used for logging debug messages
    }

    /** Called when _global.gfxProcessSound() is called in ActionScript */
    void Call(const Params& params)
    {
        bool bCallProcessed = ProcessSoundEvent(params);
        if (!bCallProcessed && bCLIKSound) 
        {
            if (params.ArgCount >= 1)
            {
                if (params.pArgs[0].IsDisplayObject() && params.ArgCount == 3)
                    ProcessCLIKSoundEvent(params);
            }            
        } 
    }

    /** FMOD-based sample for standard sound processing in FxPlayer. */     
    bool ProcessSoundEvent(const Params& params)
    {
        if (params.pArgs[0].GetType() != GFxValue::VT_String)
            return false;

        const GString methodName = params.pArgs[0].GetString();        

        // Usage:   _global.gfxProcessSound(Method:String, Command:String, Filename:String)
        // Example: _global.gfxProcessSound("GFxSound_PostEvent", "Play", "electric.wav");
        if (G_stricmp(methodName, "GFxSound_PostEvent") == 0)
        {
            if (params.ArgCount >= 2 && params.pArgs[1].GetType() == GFxValue::VT_String)
            {
                const GString command = params.pArgs[1].GetString();
                GString filename;
                if (params.ArgCount >= 3 && params.pArgs[2].GetType() == GFxValue::VT_String)
                    filename = params.pArgs[2].GetString();  

                // This log message can be commented or uncommented at the user's discretion.
                pLog->LogMessage("gfxProcessSound Callback - \n"
                                    "\tCommand: %s\n"
                                    "\tFilename: %s\n\n", 
                                    command.ToCStr(), filename.ToCStr());

                pSoundEvent->PostEvent(command, filename);
                return true;
            } 
        }
        // Usage:   _global.gfxProcessSound(Method:String, Parameter:String, Value:Number, Filename:String)
        // Example: _global.gfxProcessSound("GFxSound_SetParam", "Pan", -1.0, "electric.wav");
        else if (G_stricmp(methodName, "GFxSound_SetParam") == 0)
        {
            if (params.ArgCount >= 2 &&
                params.pArgs[1].GetType() == GFxValue::VT_String &&
                params.pArgs[2].GetType() == GFxValue::VT_Number)
            {
                const GString parameter = params.pArgs[1].GetString();
                const Double paramvalue = params.pArgs[2].GetNumber();
                GString filename;
                if (params.ArgCount >= 2 && params.pArgs[3].GetType() == GFxValue::VT_String)
                    filename = params.pArgs[3].GetString();  

                // This log message can be commented or uncommented at the user's discretion.
                pLog->LogMessage("gfxProcessSound Callback -\n"
                                    "\tParameter: %s\n" 
                                    "\tParameter Value: %f\n"
                                    "\tFilename: %s\n\n",
                                    parameter.ToCStr(), paramvalue, filename.ToCStr());

                pSoundEvent->SetParam(parameter, (Float)paramvalue, filename);
                return true;
            }
        }

        // If we're here, the call could not be processed.
        return false;
    }

    /** 
    Sample internal sound processing for CLIK components. CLIK Component events are identified by their
    first parameter, a GFxValue reference to themselves from which extra data can be retrieved. This event is
    fired by all CLIK components who inherit from UIComponent (see UIComponent::dispatchEventAndSound()).

    This method is disabled by default. It can be enabled using the -cksd command line option.

    By default, this method will search for SoundTheme+SoundName.wav (eg. defaultRollOver.wav). It is designed
    to hook into to a more advanced engine sound tool where a theme and name can easily be mapped to a particular
    sound file.

    Usage: _global.gfxProcessSound(CLIK Component:MovieClip, SoundTheme:String, SoundName:String);
    */
    bool ProcessCLIKSoundEvent(const Params& params)
    {
        // Retrieve arguments from the function parameters
        const GString componentName = params.pArgs[0].ToString();
        const GString soundThemeName = params.pArgs[1].GetString();
        const GString soundEventName = params.pArgs[2].GetString(); 
        GString soundName = GString(soundThemeName.ToCStr(), soundEventName.ToCStr(), ".wav");

        // This log message can be commented or uncommented at the user's discretion.
        if (pLog)
            pLog->LogMessage("gfxProcessSound Callback - \n"
                            "\tComponent: %s\n"
                            "\tSound Theme: %s\n"
                            "\tSoundEvent: %s\n"
                            "\tAttempting to play file: %s\n\n",
                            componentName.ToCStr(), soundThemeName.ToCStr(), 
                            soundEventName.ToCStr(), soundName.ToCStr());     

        if (pSoundEvent)
        {
            // Play the sound with the default parameters.
            pSoundEvent->SetParam("Volume", 0.4f, soundName);                   
            pSoundEvent->SetParam("Pan", 0.f, soundName);
            pSoundEvent->PostEvent("Play", soundName);
        }
        return true;
    }

private:
    const GFxMovieView* pMovieView;
    FxPlayerApp* pApp;
    GPtr<GFxLog> pLog;

    bool bCLIKSound;
    GPtr<GSoundEvent> pSoundEvent;
};
#endif // GFC_NO_SOUND


// File opener class.
class FxPlayerFileOpener : public GFxFileOpener 
{
public:
    virtual GFile* OpenFile(const char *purl, SInt flags, SInt mode) 
    {
#if !defined(GFC_NO_VIDEO) && defined(GFC_USE_VIDEO)
        if (!G_strcmp(purl, "_internal_video_player"))
            return new GMemoryFile(purl, fxvideoplayer_swf, sizeof(fxvideoplayer_swf));
#endif
        // Buffered file wrapper is faster to use because it optimizes seeks.
        return GFxFileOpener::OpenFile(purl, flags, mode);
    }
};


// ***** Main function implementation

// For UNICODE convert command line arguments to UTF8
#if defined(UNICODE) && defined(GFC_OS_WIN32)

#define main __main
int GCDECL main(int argc, char *argv[]);

int GCDECL wmain(int argc, wchar_t *wargv[])
{
    char **argv = (char**)malloc(argc * sizeof (char*));
    for (int i = 0; i < argc; i++)
    {
        argv[i] = (char*)malloc( (GUTF8Util::GetEncodeStringSize(wargv[i]) + 1) * sizeof(char) );
        GUTF8Util::EncodeString(argv[i], wargv[i]);
    }

    int res = main(argc, argv);

    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);

    return res;
}
#endif

int GCDECL main(int argc, char *argv[])
{
    // System specific startup used by the sample. This is not required to use GFx, as
    // long as the required system features are initialized by your application.
    FxPlayerApp::InitMain();

    // Create GFxSystem object to initialize GFx core memory allocator
    // and other system components. Developers can declare a custom GSysAlloc
    // implementation here and pass it to GFxSystem if necessary.


    int res = 1;

    {   // Unified SysAlloc. The local scope is used to free 
        // all memory before returning from main() 

    #if 0
        // New malloc-friendly "Magic Header" allocator.  
        GSysAllocMalloc a;
        GFxSystem gfxInit(&a);
    #else
        GFxSystem gfxInit;
    #endif
        {
            FxPlayerApp app;
            res = app.AppMain(argc, argv);
            // Wait for all threads to finish; this must be done so that memory
            // allocator and all destructors finalize correctly.
            // Note that we MUST call it here before app instance is destroyed,
            // so that we pRenderer is destroyed from the main thread only.
#ifndef GFC_NO_THREADSUPPORT
            GThread::FinishAllThreads();
#endif
        }
    }


//    // Static SysAlloc
//    UPInt segSize = 64*1024*1024;
//    void* p1 = malloc(segSize);
//    {
//        // Testing GSysAllocStatic with more than one memory segments.
//        // The player may crash due to the lack of memory.
//        GSysAllocStatic alloc;
//        alloc.AddMemSegment(p1, segSize);
//        GFxSystem gfxInit(&alloc);
//        {
//            FxPlayerApp app;
//            app.AppMain(argc, argv);
//#ifndef GFC_NO_THREADSUPPORT
//            GThread::FinishAllThreads();
//#endif
//        }
//    }
//    free(p1);

//    // Malloc SysAlloc
//    // Include "GSysAllocPagedMalloc.h"
//    {
//        GSysAllocPagedMalloc sysAlloc(64*1024);
//        GFxSystem gfxInit(&sysAlloc);
//        {
//            FxPlayerApp app;
//            app.AppMain(argc, argv);
//#ifndef GFC_NO_THREADSUPPORT
//            GThread::FinishAllThreads();
//#endif
//        }
//    }

    return res;
}


// ***** FxPlayerApp Implementation

FxPlayerApp::FxPlayerApp()
{
    Wireframe       = 0;
    AmpDirty        = true;
    //SafeRectEnabled = 0;
    // Scale toggle, on by default
    ScaleEnable     = 1;
    ClippingEnable  = 1;
    Paused          = 0;
    PausedState     = GFx::State_Playing;

    ResolutionX         = -1;    // Use SWF resolution
    ResolutionY         = -1;    // "
    ScaleX = ScaleY     = 1.0f;
    TexLodBias          = -0.5f;
    AAMode              = AAMode_EdgeAA;
    BitDepth            = 32;
    Background          = 1;

    BackgroundColor     = 0x555555;

#if defined(GFC_OS_PS2) || defined(GFC_OS_PS3) || defined(GFC_OS_WII) || defined(GFC_OS_XBOX360) || defined(GFC_OS_PSP)
    MeasurePerformance  = 1;
    FullScreen          = 1;
#else 
    MeasurePerformance  = 0;
    FullScreen          = 0;
#endif

    // No font config used by default.
    FontConfigIndex     = -1;

    FastForward         = 0;

    ExitTimeout         = 0.0f;
    SleepDelay          = 31;

    // Clear file
    FileName[0]         = 0;
    //ShortFileName[0]    = 0;
    // Clear timing
    SpeedScale      = 1.0f;
    FrameCounter    = 0;
    TessTriangles   = 0;

    TimeStartTicks  = 0;
    TimeTicks       = 0;
    NextTicksTime   = 0;
    LastLoggedFps   = 0;
    MovieStartTicks = 0;
    MovieLastTicks  = 0;
    HudLastTicks    = 0;
    MovieTicks      = 0;

    LastFPS         = 0.0f;
    LastAdvance       = 0;
    LastDisplay       = 0;
    RedirectEventsToHud = false;

	LastHUDUpdate	= 0;

    DoLoop          = 1;
    DoRender        = 1;
    DoSound         = 0;
    NoControlKeys   = 0;
    NoCtrlKey       = 0;

    SoundVolume     = 1.0f;
    IsMute          = 0;
    bCLIKSound      = false;

    AmpPort         = 7534;
    AmpWait         = false;

    ViewWidth       = 0;
    ViewHeight      = 0;


    Zoom = 1.0;
    Move = GPointF(0.0);

    MouseX = 0;
    MouseY = 0;
    MouseTracking = None;

    SizingEntered = 0;

    CurvePixelError = 1.0f;
    FontLibChanged = false;

    bDisplayedMasksWarning = false;

    pHudHeap = NULL;

    pAppControlCallback = NULL;

#ifndef GFC_NO_SOUND
    pSoundFMODSystem  = NULL;
    pSoundWwiseSystem = NULL;
#endif

    OverdrawProfile = 0;

    KeyboardIndex = 0;

}

void FxPlayerApp::InitializeSound()
{
#if !defined(GFC_NO_SOUND)

#if defined(GFC_SOUND_FMOD)
    if (!pSoundFMODSystem)
        pSoundFMODSystem = new FxSoundFMOD;
    if (!pSoundFMODSystem->IsInitialized())
    {
        bool bIsInitFMOD = pSoundFMODSystem->Initialize(
#ifdef GFC_OS_PS3		
            GetSpurs()
#endif
        );
	    if (!bIsInitFMOD)
	       xShutdownSound();
    }
#endif // GFC_SOUND_FMOD

#if defined(GFC_SOUND_WWISE)
    if (!pSoundWwiseSystem)
        pSoundWwiseSystem = new FxSoundWwise;
    if (!pSoundWwiseSystem->IsInitialized())
    {
        bool bIsInitWwise = pSoundWwiseSystem->Initialize(
#ifdef GFC_OS_PS3
            GetSpurs()
#endif			
        );
        if (!bIsInitWwise)
            xShutdownSound();
    }
#endif // GFC_SOUND_WWISE

#endif // GFC_NO_SOUND
}

void FxPlayerApp::xShutdownSound()
{
#if !defined(GFC_NO_SOUND)

#if defined(GFC_SOUND_FMOD)
    if (pSoundFMODSystem && pSoundFMODSystem->IsInitialized())
        pSoundFMODSystem->Finalize();
    delete pSoundFMODSystem;
    pSoundFMODSystem = NULL;
#endif

#if defined(GFC_SOUND_WWISE)
    if (pSoundWwiseSystem && pSoundWwiseSystem->IsInitialized())
        pSoundWwiseSystem->Finalize();
    delete pSoundWwiseSystem;
    pSoundWwiseSystem = NULL;
#endif

#endif // GFC_NO_SOUND
}

GSoundRenderer* FxPlayerApp::GetSoundRenderer()
{
#if defined(GFC_NO_SOUND) || !defined(GFC_SOUND_FMOD)
    return NULL;
#else
    return pSoundFMODSystem && pSoundFMODSystem->IsInitialized() ? pSoundFMODSystem->GetSoundRenderer() : NULL;
#endif
}


//  Sample translator implementation that can be used for testing.
/*
class TranslatorImpl : public GFxTranslator
{
public:
    // wwMode can be GFxTranslator::WWT_NoHangulWrap (Korean),
    // GFxTranslator::WWT_Prohibition (used in Japanese) or
    // GFxTranslator::WWT_Asian (Chinese). Also GFxTranslator::WWT_Hyphenation
    // might be used for very simple hyphenation. OnWordWrapping virtual 
    // method might be overridden to implement custom word-wrapping rules.
    TranslatorImpl(UInt wwMode = GFxTranslator::WWT_Default) :
      GFxTranslator(wwMode) {}

    virtual UInt    GetCaps() const         
    { 
        return Cap_StripTrailingNewLines; 
    }
    virtual void    Translate(TranslateInfo* ptranslateInfo)
    {
        // Translated strings start with $$sign
        const wchar_t* pkey = ptranslateInfo->GetKey();
        if (pkey[0] == '$')
        {
            GFxWStringBuffer buffer;
            size_t l = G_wcslen(pkey);
            buffer.Resize(l + 100);
            wchar_t* pb = buffer.GetBuffer();

            //G_wcscpy(pb, buffer.GetLength(), pkey+1);
            //ptranslateInfo->SetResult(buffer.ToWStr());

            G_wcscpy(pb, buffer.GetLength(), L"<font color='#00FF33'>*TR[<font color='#FF3300'>");
            G_wcscat(pb, buffer.GetLength(), pkey+1);
            G_wcscat(pb, buffer.GetLength(), L"</font>]</font>");
            ptranslateInfo->SetResultHtml(buffer.ToWStr());
        }
        // example of using GetInstanceName()
        else if (G_strcmp(ptranslateInfo->GetInstanceName(), "plainText") == 0)
        {
            ptranslateInfo->SetResult("PLAIN");
        }
    }
};
*/

#if defined(GFC_OS_WIN32)

// Install system-specific clipboard implementation on Win32. If this is not done
// the clipboard will still work in FxPlayer, but it will be impossible to paste
// text to external applications.
class FxPlayerTextClipboard : public GFxTextClipboard
{
public:
    void OnTextStore(const wchar_t* ptext, UPInt len)
    {
        // store the text in a system clipboard.
        if (OpenClipboard(NULL))
        {
            // Empty the Clipboard. This also has the effect
            // of allowing Windows to free the memory associated
            // with any data that is in the Clipboard
            EmptyClipboard();

            HGLOBAL hClipboardData;
            hClipboardData = GlobalAlloc(GMEM_DDESHARE, (len+1)*sizeof(wchar_t));
            GASSERT(sizeof(wchar_t) == 2);

            // Calling GlobalLock returns a pointer to the 
            // data associated with the handle returned from 
            // GlobalAlloc
            wchar_t * pchData;
            pchData = (wchar_t*)GlobalLock(hClipboardData);
            G_wcscpy(pchData, len+1, ptext);

            // Once done, unlock the memory. 
            // don't call GlobalFree because Windows will free the 
            // memory automatically when EmptyClipboard is next 
            // called. 
            GlobalUnlock(hClipboardData);

            // Now, set the Clipboard data by specifying that 
            // Unicode text is being used and passing the handle to
            // the global memory.
            SetClipboardData(CF_UNICODETEXT, hClipboardData);

            // Finally, close the Clipboard which has the effect of 
            // unlocking it so that other  apps can examine or modify its contents.
            CloseClipboard();
        }
    }
};

#endif
  
void FxPlayerApp::InitArgDescriptions(FxArgs* args)
{
    FXPLAYER_APP::InitArgDescriptions(args);

    FxArgDesc options []=
    {
//      {"","--------------spacer example------------------\n","",FxCmdOption::Spacer,""},
        {"",    "FileName",            FxArgs::StringOption | FxArgs::Positional, NULL, //"gfx_cri.swf", 
                                                            "GFX or SWF file to load at startup"},
        {"arg", "Arg",                 FxArgs::StringOption, NULL, //"gfx_cri.swf", 
                                                            "Value for _global.gfxArg variable in ActionScript."},
        {"s",   "ScaleFactor",         FxArgs::FloatOption, "1.0", 
                                                            "<factor> Scale the movie window size by the specified factor."},

        {"vol", "SoundVolume",         FxArgs::IntOption, "80", "Sound Volume. 0-100 (default 80)"},
        {"mute", "SoundMute",          FxArgs::Flag, "", "Mute sound"},
        {"cksd", "CLIKSound",          FxArgs::Flag, "", "Enable default CLIK sound events."},
        {"na",  "noAA",                FxArgs::Flag, "", "Use no anti-aliasing."},
        {"fsa", "FSAA",                FxArgs::Flag, "", "Use use fullscreen HW AA."}, 
        {"nsf", "NoSystemFont",        FxArgs::Flag, "", "No system font - disables GFxFontProviderWin32."},
        {"nfc", "NoFontConfig",        FxArgs::Flag, "", "Disable autoloading of font config file in movie's path."},
        {"fc",  "FontConfigFile",      FxArgs::StringOption, "", "<fname> Load a font config file."},
        {"fl",  "FontLibFile",         FxArgs::StringOption, NULL, 
                                                            "<fname>  Specifies a SWF/GFX file to load into GFxFontLib."},
//        {"i",   "DisplayHudOnStartup", FxArgs::Flag, "", "Display info HUD on startup.",},
        {"vp",  "VerboseParse",        FxArgs::Flag, "", "Verbose parse - print SWF parse log."},
        {"vps", "VerboseParseShape",   FxArgs::Flag, "" ,"Verbose parse shape - print SWF shape parse log."},
        {"vpa", "VerboseParseAction",  FxArgs::Flag, "", "Verbose parse action - print SWF actions during parse."},
        {"vfq", "NoLogChildFilnames",  FxArgs::Flag, "", "Do not print SWF filename for ActionScript errors in child movies."},
        {"vfr", "LogRootFilenames",    FxArgs::Flag, "","Print SWF filename for ActionScript errors in root movies."},
        {"vfp", "LogFilePath",         FxArgs::Flag, "", "Print full file path for ActionScript errors."},
        {"va",  "VerboseActions",      FxArgs::Flag, "", "Verbose actions - display ActionScript execution log."},
        {"q",   "Quiet",               FxArgs::Flag, "", "Quiet. Do not display errors or trace statements."},
        {"qae", "SuppressASErrors",    FxArgs::Flag, "", "Quiet. Suppress ActionScript errors."},
        {"ml",  "LodBias",             FxArgs::FloatOption, "-0.5", 
                                                             "<bias>  Specify the texture LOD bias (float, default -0.5)"},
        {"rs",  "RecordStatistics",    FxArgs::StringOption, NULL, "<filename> Save current performance/memory statistics at exit."},
        {"p",   "PerfomanceTest",      FxArgs::Flag, "", "Performance test - run without delay and log FPS."},
        {"ff",  "FastForward",         FxArgs::Flag, "", "Fast forward - run one frame per update."},
        {"1",   "PlayOnce",            FxArgs::Flag, "", "Play once; exit when/if movie reaches the last frame."},
        {"nr",  "NoRendering",         FxArgs::Flag, "", "Disable rendering (for batch tests)"},
        {"ns",  "NoSound",             FxArgs::Flag, "", "Disable sound (for batch tests)"},
        {"nck", "NoControlKeys",       FxArgs::Flag, "", "Disable all player related control keys."},
        {"nh",  "NoHud",               FxArgs::Flag, "", "Do not load HUD SWF."},
        {"t",   "ExitTimeout",         FxArgs::FloatOption, "0.0",
                                                             "<sec>    Timeout and exit after the specified number of seconds."},
#if defined(FXPLAYER_RENDER_DIRECT3D) && (GFC_D3D_VERSION < 10)
        {"dyn", "DynamicTextures" ,    FxArgs::Flag, "", "Use dynamic textures (D3D)."},
#endif
        {"res", "Resolution",          FxArgs::StringOption, "", "Override GFxPlayer resolution with custom values."},
#ifdef GFX_AMP_SERVER
        {"amp", "AmpPort",             FxArgs::IntOption, "7534", "Port for AMP socket connection. (default 7534)"},
        {"wait","AmpWait",             FxArgs::Flag, "", "Wait until an AMP connection has been established"},
#endif
        {"","",FxArgs::ArgEnd,"",""},
    };
        args->AddDesriptions(options);
}
bool FxPlayerApp::OnArgsParsed(const FxArgs& args) 
{
    if (!FXPLAYER_APP::OnArgsParsed(args))
        return false;

    if (args.HasValue("FileName"))
    {
        if (args.GetString("FileName").GetSize() > sizeof(FileName) - 1)
        {
            fprintf(stderr, "Error: File path must be less then %d\n", UInt(sizeof(FileName)) - 1);
            return false;
        }
        else
        {
            G_strcpy(FileName, sizeof(FileName), args.GetString("FileName").ToCStr());
        }
        
    }
    GString customRes = args.GetString("Resolution");
    if (customRes.GetSize() > 0)
    {
        const char* restr = customRes.ToCStr();
        ResolutionX = atoi(restr);
        restr = G_strchr(restr, ':');
        if (!restr) 
        {
            ResolutionX = ResolutionY = -1;
        }
        else 
        {
            ResolutionY = atoi(restr+1);
        }
    }
    ScaleX = ScaleY = args.GetFloat("ScaleFactor");
    DoRender = !args.GetBool("NoRendering");
    DoSound = !args.GetBool("NoSound");
    FastForward =  args.GetBool("FastForward");
    ExitTimeout =  args.GetFloat("ExitTimeout");

    DoLoop = !args.GetBool("PlayOnce");

    NoControlKeys = args.GetBool("NoControlKeys");

    SoundVolume = args.GetInt("SoundVolume") / 100.0f;
    if(SoundVolume < 0.0f)
        SoundVolume = 0.0f;
    else if (SoundVolume > 1.0f)
        SoundVolume = 1.0f;
    
    IsMute = args.GetBool("SoundMute");
    bCLIKSound = args.GetBool("CLIKSound");

#ifdef GFX_AMP_SERVER
    AmpPort = args.GetInt("AmpPort");
    AmpWait = args.GetBool("AmpWait");
#endif
    return true;
}

#if !defined(GFC_NO_VIDEO) && defined(GFC_USE_VIDEO)
static bool IsVideoFile(const char* pfilename)
{
    return strstr(pfilename, ".usm") != 0;
}
#endif


int FxPlayerApp::Run() 
{
    // Set the verbose flags.
    UInt verboseFlags = 0;

    if (Arguments.GetBool("VerboseParse"))//Settings.VerboseParse
        verboseFlags |= GFxParseControl::VerboseParse;
    if (Arguments.GetBool("VerboseParseShape")) //Settings.VerboseParseShape
        verboseFlags |= GFxParseControl::VerboseParseShape;
    if (Arguments.GetBool("VerboseParseAction")) //Settings.VerboseParseAction
        verboseFlags |= GFxParseControl::VerboseParseAction;

    // File callback.
    GPtr<GFxFileOpener> pfileOpener = *new FxPlayerFileOpener;

    // Uncomment this line and add libpng to linker's settings to enable
    // PNG support.
    Loader.SetPNGSupport(GPtr<GFxPNGSupport>(*new GFxPNGSupport()));

    Loader.SetFileOpener(pfileOpener);

    Ptr<ASSupport> pAS2Support = *new AS2Support();
    Loader.SetAS2Support(pAS2Support);
    Ptr<ASSupport> pAS3Support = *new AS3Support();
    Loader.SetAS3Support(pAS3Support);

#ifdef GFC_OS_MAC
    // Keep images for renderer changes
    GPtr<GFxImageCreator> pImageCreator = *new GFxImageCreator(1);
    Loader.SetImageCreator(pImageCreator);
#endif

    // Task Manager.
#ifdef FXPLAYER_PROGRESSIVE_LOADING
    GPtr<GFxThreadedTaskManager> ptaskManager = *new GFxThreadedTaskManager;
#if defined(GFC_OS_XBOX360)
    ptaskManager->AddWorkerThreads(GFxTask::Type_IO, 3, 128 * 1024, 1);
#endif
    Loader.SetTaskManager(ptaskManager);
#endif

    //Loader.SetGradientParams(GPtr<GFxGradientParams>(*new GFxGradientParams(16)));

    // Set log, but only if not quiet
    if (!Arguments.GetBool("Quiet")) //!Settings.Quiet
        Loader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));
#ifdef GFC_OS_WIN32    
    Loader.SetTextClipboard(GPtr<GFxTextClipboard>(*new FxPlayerTextClipboard()));
#endif    
    //Loader.SetTranslator(GPtr<GFxTranslator>(*new TranslatorImpl()));

    // Disabling FontCacheManager
    //---------------------------
    //Loader.SetFontCacheManager(0);
    //---------------------------

    //// Configuring the font cache manager
    ////---------------------------
    //GFxFontCacheManager::TextureConfig fontCacheConfig;
    //fontCacheConfig.TextureWidth   = 1024;
    //fontCacheConfig.TextureHeight  = 1024;
    //fontCacheConfig.MaxNumTextures = 1;
    //fontCacheConfig.MaxSlotHeight  = 48;
    //fontCacheConfig.SlotPadding    = 2;
    //Loader.GetFontCacheManager()->SetTextureConfig(fontCacheConfig);
    //Loader.GetFontCacheManager()->EnableDynamicCache(true);
    //Loader.GetFontCacheManager()->SetMaxRasterScale(1.0f);
    //---------------------------

#ifdef FXPLAYER_FONTCACHE_SMALL
    GFxFontCacheManager::TextureConfig fontCacheConfig;
    fontCacheConfig.TextureWidth   = 512;
    fontCacheConfig.TextureHeight  = 256;
    fontCacheConfig.MaxNumTextures = 1;
    fontCacheConfig.MaxSlotHeight  = 32;
    //fontCacheConfig.SlotPadding    = 2;
    Loader.GetFontCacheManager()->SetTextureConfig(fontCacheConfig);
    Loader.GetFontCacheManager()->EnableDynamicCache(true);
    Loader.GetFontCacheManager()->SetMaxRasterScale(1.0f);
#endif


    // Configuring the glyph packer
    //-----------------------------
    //// Creating the packer
    //Loader.SetFontPackParams(GPtr<GFxFontPackParams>(*new GFxFontPackParams()));
    //
    //// Optional TextureConfig
    ////-----------------------------
    //GFxFontPackParams::TextureConfig fontPackConfig;
    //fontPackConfig.NominalSize = 32;
    //fontPackConfig.PadPixels   = 2;
    //fontPackConfig.TextureWidth  = 512;
    //fontPackConfig.TextureHeight = 512;
    //Loader.GetFontPackParams()->SetTextureConfig(fontPackConfig);

    //// Optional restriction: If the font contains more than 1000 glyphs it will not be packed.
    //// Instead, this font will be displayed with the dynamic cache.
    ////-----------------------------
    //Loader.GetFontPackParams()->SetGlyphCountLimit(1000);


    // Preprocess parameters (Pre-tessellation)
    //-----------------------------
    //Loader.SetPreprocessParams(GPtr<GFxPreprocessParams>(*new GFxPreprocessParams()));

    // Image Packer. Enable in Player only for testing; for production use packing in gfxexport.
    //Loader.SetImagePackParams(GPtr<GFxImagePackParams>(*new GFxImagePackParams));

    // Configure Font compactor
    // GFxFontCompactorParams contains the font compactor configuration options 
    // There are two parameters that can be set on the contractor of this class
    // 1. NominalSize - The nominal glyph size that corresponds with the input 
    //    coordinates. Small nominal size results in smaller total data size, 
    //    but less accurate glyphs. For example, NominalSize=256 can typically 
    //    save about 25% of memory, compared with NominalSize=1024, but really 
    //    big glyphs, like 500 pixels may have visual inaccuracy. For typical game 
    //    UI text NominalSize=256 is still good enough. In SWF it is typically 1024.
    // 2. MergeContours - A Boolean flag that tells, whether or not the 
    //    FontCompactor should merge the same contours and glyphs. When merging 
    //    the data can be more compact and save 10-70% of memory, depending on fonts.
    //    But if the font contains too many glyphs, the hash table may consume 
    //    additional memory, 12 (32-bit) or 16 (64-bit) bytes per each unique path, 
    //    plus 12 (32-bit) or 16 (64-bit) bytes per each unique glyph.
    //Loader.SetFontCompactorParams(GPtr<GFxFontCompactorParams>(*new GFxFontCompactorParams(256, true)));

    // Configuring GFxMeshCacheManager
    //Loader.GetMeshCacheManager()->SetMeshCacheMemLimit(4*1024*1024);
    //Loader.GetMeshCacheManager()->SetRenderGenMemLimit(2*1024*1024);
    //Loader.GetMeshCacheManager()->SetNumLockedFrames(1);
    //Loader.GetMeshCacheManager()->EnableCacheAllMorph(false);
    //Loader.GetMeshCacheManager()->EnableCacheAllScale9Grid(false);

    // Create a system font provider on Windows. If this is not done,
    // system font characters will be displayed as squares.
    if (!Arguments.GetBool("NoSystemFont")) 
    {
#ifdef GFC_OS_WIN32
        GPtr<GFxFontProviderWin32> fontProvider = *new GFxFontProviderWin32(::GetDC(0));
        Loader.SetFontProvider(fontProvider);
#endif        

        // An example of using font provider with Scaleform packed fonts 
        //GPtr<GFxFontProviderPacked> fontProvider = *new GFxFontProviderPacked(); 
        //fontProvider->LoadFonts(pfileOpener, "C:\\Scaleform\\GFx\\2.2\\Apps\\Tools\\GFxFontCompactorWin32\\FontData.gfxf");
        ////fontProvider->LoadFonts(pfileOpener, . . .);
        //Loader.SetFontProvider(fontProvider);

        // An example of using FreeType-2 font provider
        //GPtr<GFxFontProviderFT2> fontProvider = *new GFxFontProviderFT2;
        //fontProvider->MapFontToFile("Times New Roman", 0,                      "C:\\WINDOWS\\Fonts\\times.ttf");
        //fontProvider->MapFontToFile("Times New Roman", GFxFont::FF_Bold,       "C:\\WINDOWS\\Fonts\\timesbd.ttf");
        //fontProvider->MapFontToFile("Times New Roman", GFxFont::FF_Italic,     "C:\\WINDOWS\\Fonts\\timesi.ttf");
        //fontProvider->MapFontToFile("Times New Roman", GFxFont::FF_BoldItalic, "C:\\WINDOWS\\Fonts\\timesbi.ttf");

        //fontProvider->MapFontToFile("Arial",           0,                     "C:\\WINDOWS\\Fonts\\arial.ttf");
        //fontProvider->MapFontToFile("Arial",           GFxFont::FF_Bold,      "C:\\WINDOWS\\Fonts\\arialbd.ttf");
        //fontProvider->MapFontToFile("Arial",           GFxFont::FF_Italic,    "C:\\WINDOWS\\Fonts\\ariali.ttf");
        //fontProvider->MapFontToFile("Arial",           GFxFont::FF_BoldItalic,"C:\\WINDOWS\\Fonts\\arialbi.ttf");

        //fontProvider->MapFontToFile("Verdana",         0,                     "C:\\WINDOWS\\Fonts\\verdana.ttf");
        //fontProvider->MapFontToFile("Verdana",         GFxFont::FF_Bold,      "C:\\WINDOWS\\Fonts\\verdanab.ttf");
        //fontProvider->MapFontToFile("Verdana",         GFxFont::FF_Italic,    "C:\\WINDOWS\\Fonts\\verdanai.ttf");
        //fontProvider->MapFontToFile("Verdana",         GFxFont::FF_BoldItalic,"C:\\WINDOWS\\Fonts\\verdanaz.ttf");
        //Loader.SetFontProvider(fontProvider);

    }

    if(strlen(FileName) == 0)
        G_strcpy(FileName, sizeof(FileName), FXPLAYER_FILEPATH);

    bool loadMovie = strlen(FileName)>0;
    if (loadMovie)
    {
        // if filename is set, then we try to load the font config
        LoadDefaultFontConfigFromPath(FileName);
    }
    // Apply font configuration on a loader
    if (FontConfigIndex != -1) 
    {
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
            pconfig->Apply(&Loader);
    } 
    else 
    {
        // Create and load a file into GFxFontLib if requested.
        if (Arguments.HasValue("FontLibFile")) 
        {
            GPtr<GFxFontLib> fontLib = *new GFxFontLib;
            Loader.SetFontLib(fontLib);
            GPtr<GFxMovieDef> pmovieDef = *Loader.CreateMovie(Arguments.GetString("FontLibFile").ToCStr());
            fontLib->AddFontsFrom(pmovieDef);
        }
    }

    // Load movie for the stats display
    // GFxLoader::LoadWaitCompletion should be set from threaded loading for HUD movie. 
    // Otherwise HUD will no be shown because is Advance method gets call only once when the
    // movie is not ready yet

    GPtr<GFxParseControl> pparseControl = *new GFxParseControl(verboseFlags);
    Loader.SetParseControl(pparseControl);

#ifndef GFC_NO_SOUND
    if (DoSound)
        // Initialize sound system(s)
        InitializeSound();
#endif

#ifndef GFC_NO_VIDEO
    // To initialize a video playback you need to install an instance of GFxVideo 
    // to the Loader
#ifdef GFC_USE_VIDEO
#if defined(GFC_OS_PS3)
	// The video player state for PS3 has some initialization parameters:
    // 1. pointer to CellSpurs structure
    // 2. number of PPU threads which can be used for video decoding
    // 3. priority of PPU threads
    // 4. number of SPUs which can be used for video decoding
    // 5. SPURS task priorities
    UInt8 spurs_priorites[] = {0,0,1,1,1,0,0,0};
    GPtr<GFxVideo> pvc = *new GFxVideoPS3(GetSpurs(), 1, GThread::NormalPriority, 3, spurs_priorites);
#elif defined(GFC_OS_XBOX360)
    // The video player state for Xbox360 has three optional initialization parameters: 
    // 1. a mask which specifies which hardware threads will be used for video decoding
    // 2. priority of video decoding threads
    // 3. a hardware thread which will be used for the video reader
    GPtr<GFxVideo> pvc = *new GFxVideoXbox360(GFxXbox360_Proc1|GFxXbox360_Proc3|GFxXbox360_Proc5,
                                              GThread::NormalPriority,
                                              GFxXbox360_Proc2);
#elif defined(GFC_OS_WIN32)
    // The video player state for Win32 has three optional initialization parameters: 
    // 1. priority of video decoding threads
    // 2. number of video deciding threads (maximum is 3)
    // 3. an array with affinity masks for each decoding thread.
    UInt32 affinities[] = {
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK
    };
    GPtr<GFxVideo> pvc = *new GFxVideoPC(GThread::NormalPriority, MAX_VIDEO_DECODING_THREADS, affinities);
#else
    // An optional parameter allows specifying of the priority of the video decoding thread.
    GPtr<GFxVideo> pvc = *new GFxVideo(GThread::NormalPriority);
#endif

#ifdef GFC_VIDEO_USE_SYSTEM_SOUND
#if defined(GFC_OS_WIN32)
    pvc->SetSoundSystem(GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemDX8(0)));
    //pvc->SetSoundSystem(GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemXA2(0,0)));
#elif defined(GFC_OS_XBOX360)
    pvc->SetSoundSystem(GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemXA2(0,0)));
#elif defined(GFC_OS_WII)
    pvc->SetSoundSystem(GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemWii(true)));
#elif defined(GFC_OS_PS3)
    pvc->SetSoundSystem(GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemPS3(true,GetSpurs())));
#endif
#elif defined(GFC_VIDEO_USE_WWISE)
    GPtr<GFxVideoSoundSystem> wwise = GPtr<GFxVideoSoundSystem>(*new GFxVideoSoundSystemWwise());
    pvc->SetSoundSystem(wwise);
#endif // GFC_VIDEO_USE_SYSTEM_SOUND

    Loader.SetVideo(pvc);
#endif // GFC_USE_VIDEO
#endif // GFC_NO_VIDEO

#ifndef GFC_NO_SOUND
    GSoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        // Sound renderer and parameters for playing swf streaming sounds are provided to
        // to the loader by GFxAudio. Those parameters control when and how the synchronization
        // between swf frames and sound frames should be done.
        GPtr<GFxAudio> paudioState = *new GFxAudio(psoundRenderer);
        Loader.SetAudio(paudioState);
        psoundRenderer->SetMasterVolume(SoundVolume);
        psoundRenderer->Mute(IsMute);

#ifndef GFC_NO_VIDEO
#ifdef GFC_USE_VIDEO
#if !defined(GFC_VIDEO_USE_SYSTEM_SOUND) && !defined(GFC_VIDEO_USE_WWISE)
        // if a video has audio tracks and you want to play them through GSoundRenderer 
        // object you need to set this object to GFxVideo state
        pvc->SetSoundSystem(psoundRenderer);
#endif // GFC_VIDEO_USE_SYSTEM_SOUND
#endif // GFC_USE_VIDEO
#endif // GFC_NO_VIDEO
    }

#endif // GFC_NO_SOUND

    // 
    // SharedObject support
    //
#if !defined(GFC_NO_FXPLAYER_AS_SHAREDOBJECT) && !defined(GFC_NO_XML_SUPPORT)

    GString SOPath;
    SOPath.AppendString(FileName);
    GFxURLBuilder::ExtractFilePath(&SOPath);
    if (SOPath.CompareNoCase(FileName) == 0)
    {
        SOPath = "";
    } 
    // GFx currently does not support creation of directories
    //SOPath.AppendString(".soCache/");
    GPtr<GFxSharedObjectManagerBase> psomgr = *new FxSharedObjectManager(SOPath);
    Loader.SetSharedObjectManager(psomgr);

#endif  // GFC_NO_FXPLAYER_AS_SHAREDOBJECT

    if (loadMovie)
    {
#if !defined(GFC_NO_VIDEO) && defined(GFC_USE_VIDEO)
        // Get info about the width & height of the movie.
        if (IsVideoFile(FileName))
        {
            GFxVideoPlayer::VideoInfo vinfo;
            if (GFxVideoPlayer::LoadVideoInfo(FileName, &vinfo, Loader.GetFileOpener()))
            {
                ViewWidth = (SInt) (vinfo.Width * ScaleX);
                ViewHeight = (SInt) (vinfo.Height * ScaleY);
            }
            else 
            {
                if (Arguments.GetBool("Quiet"))
                    fprintf(stderr, "Error: Failed to get info about '%s'\n", FileName);

                loadMovie = 0;
            }
        }
        else
#endif

        {
            if (!Loader.GetMovieInfo(FileName, &MovieInfo)) 
            {
                // Loader.GetMovieInfo will print error message unless we are in quiet mode
                if (Arguments.GetBool("Quiet"))
                    fprintf(stderr, "Error: Failed to get info about '%s'\n", FileName);

                loadMovie = 0;
                //return 1;
            } 
            else
            {
                ViewWidth = (SInt) (MovieInfo.Width * ScaleX);
                ViewHeight = (SInt) (MovieInfo.Height * ScaleY);
            }
        }
    }
    if (!loadMovie)
    {
        ViewWidth = FXPLAYER_VIEWWIDTH;
        ViewHeight = FXPLAYER_VIEWHEIGHT;
    }

    if (DoRender) 
    {

        FSAntialias = (AAMode == AAMode_FSAA) ? 1 : 0;
#if defined(GFC_OS_WIN32) && defined(FXPLAYER_RENDER_DIRECT3D) && (GFC_D3D_VERSION < 10)
        if (Arguments.GetBool("DynamicTextures"))
        {
            VMCFlags |= RENDERER::VMConfig_UseDynamicTex;
        }
#endif

        if (FullScreen) 
        {
            ScaleX = ((Float)FXPLAYER_VIEWWIDTH)  / ViewWidth;
            ScaleY = ((Float)FXPLAYER_VIEWHEIGHT) / ViewHeight;
            ViewWidth       = FXPLAYER_VIEWWIDTH;
            ViewHeight      = FXPLAYER_VIEWHEIGHT;
        }

        if (ResolutionX != -1 && ResolutionY != -1) 
        {
            ViewWidth = ResolutionX;
            ViewHeight = ResolutionY;
        }

        // Enable file drop.
        SupportDropFiles = 1;
        SizableWindow = 1;
        SInt w = ViewWidth, h = ViewHeight;
        if (!SetupWindow(FXPLAYER_APP_TITLE, ViewWidth, 
#ifdef FXPLAYER_LOGO
            ViewHeight+LOGO_BORDER))
#else
            ViewHeight))
#endif
        return 1;

#ifdef FXPLAYER_LOGO
        Height -= LOGO_BORDER;
#endif

        // It is important to initialize these sizes, in case OnSizeEnter gets called.
        SizeWidth = GetWidth();
        SizeHeight = GetHeight();

        // Width & Height might be changed by SetupWindow call above
        if (w != GetWidth() || h != GetHeight())
            UpdateViewSize();

        // Create renderer
        if (!CreateRenderer()) 
            return 1;

        // Set renderer on loader so that it is also applied to all children.
        pRenderConfig = *new GFxRenderConfig(GetRenderer(), GFxRenderConfig::RF_StrokeNormal);
        Loader.SetRenderConfig(pRenderConfig);

        // Create a renderer stats object since we will be tracking statistics.
        pRenderStats = *new GFxRenderStats();
        Loader.SetRenderStats(pRenderStats);
    }

#ifndef GFC_NO_XML_SUPPORT
    
    GPtr<GFxXMLParser> pexpatXmlParser = *new GFxXMLParserExpat;
    GPtr<GFxXMLSupportBase> pxmlSupport = *new GFxXMLSupport(pexpatXmlParser);
    Loader.SetXMLSupport(pxmlSupport);

    //
    // Using the default parser for document processing from C++
    // ---------------------------------------------------------
    // (Not available in eval/lib distributions)
    //
    //#include "XML/GFxXML.h"
    //
    // -- Create a DOM builder that processes whitespace
    //GFxXMLDOMBuilder domBuilder(pxmlSupport);
    // -- Create a DOM builder that ignores whitespace
    //GFxXMLDOMBuilder domBuilder2(pxmlSupport, true);
    //
    // -- Process the xml file and return the root of the DOM tree
    //GPtr<GFxXMLDocument> pdoc = domBuilder.ParseFile("inputfile.xml", Loader.GetFileOpener());
    // -- Process the xml file and return the root of the DOM tree (use provided object manager)
    //GPtr<GFxXMLObjectManager> pobjMgr = *new GFxXMLObjectManager();
    //GPtr<GFxXMLDocument> pdoc2 = domBuilder.ParseFile("inputfile.xml", Loader.GetFileOpener(), pobjMgr);
    // 

#endif

#ifdef GFC_USE_IME
    //!Create IMEManager after the application window has been set up since 
    // IME manager needs valid window handle as parameter.
    GFxIMEManager* pimemanager = IMEHelper::CreateManager(this);
    if (pimemanager)
    {
        // For error logging. If error logging not desired, just pass null to Init. User must 
        // call Init though!
        pimemanager->Init(Loader.GetLog());
        pimemanager->SetIMEMoviePath("IME.swf");
        Loader.SetIMEManager(pimemanager);
        // Loader keeps the object from this point
        pimemanager->Release();
    }
#endif


#ifdef GFC_USE_IME
    if (pimemanager)
    {
        if(pMovie)
            pMovie->HandleEvent(GFxEvent::SetFocus);
    }
#endif

    pHudHeap = GMemory::GetGlobalHeap()->CreateHeap(
        "HUD", GMemoryHeap::Heap_UserDebug);

    pHudDataProvider = *GHEAP_NEW(pHudHeap) FxHUDDataProvider(
        pMovie, 
        GetRenderer(),
        Loader.GetFontCacheManager(),
        Loader.GetMeshCacheManager(), 
        this, 
        IsConsole()
        );

#ifdef GFX_AMP_SERVER
    GFxAmpMessageAppControl caps;
    caps.SetCurveToleranceDown(true);
    caps.SetCurveToleranceUp(true);
    caps.SetNextFont(true);
    caps.SetRestartMovie(true);
    caps.SetToggleAaMode(true);
    caps.SetToggleAmpRecording(true);
    caps.SetToggleFastForward(true);
    caps.SetToggleInstructionProfile(true);
    caps.SetToggleOverdraw(true);
    caps.SetToggleStrokeType(true);
    caps.SetTogglePause(true);
    caps.SetToggleWireframe(true);
    GFxAmpServer::GetInstance().SetAppControlCaps(&caps);

    pAppControlCallback = *GHEAP_AUTO_NEW(&GFxAmpServer::GetInstance()) FxAmpAppControlCallback(this);
    GFxAmpServer::GetInstance().SetAppControlCallback(pAppControlCallback);
    GFxAmpServer::GetInstance().AddAmpRenderer(GetAmpRenderer());
    if (AmpWait)
    {
        GFxAmpServer::GetInstance().SetConnectionWaitTime(GFC_WAIT_INFINITE);
    }
    GFxAmpServer::GetInstance().SetListeningPort(AmpPort);
    AmpDirty = true;

#endif

    if (!Arguments.GetBool("NoHud")) // DBG && !Arguments.GetBool("NoRendering"))
    {
        pHud = *FxHUD::CreateHUD(pHudDataProvider, this);      
    }

#ifdef GFX_AMP_SERVER
    GFxAmpServer::GetInstance().OpenConnection();
#endif
  
    // Load movie and initialize timing.
    if (loadMovie && !LoadMovie(FileName)) 
    {
        //return 1;
    }

    while (1)
    {
        HandleAmpAppMessages();

        TimeTicks = GTimer::GetTicks() / 1000;

        if (DoRender && !FastForward)
            MovieTicks = TimeTicks;
        else
            // Simulate time.
            MovieTicks = MovieLastTicks + (UInt32) (1000.0f / MovieInfo.FPS);

        int deltaTicks = (int)(MovieTicks - MovieLastTicks);
        float deltaT = deltaTicks / 1000.f;
        //float deltaTHud = (TimeTicks - HudLastTicks) / 1000.f; 
        MovieLastTicks  = MovieTicks;
        HudLastTicks    = TimeTicks;

        // Check auto exit timeout counter.
        if ((ExitTimeout > 0) && (MovieTicks - MovieStartTicks > (UInt32) (ExitTimeout * 1000)))
            break;

        // Process messages and exit if necessary.
        if (!ProcessMessages())
            break;

#ifdef GFC_VIDEO_USE_WWISE
        wwise->Update();
#endif

        // *** Advance/Render movie

        if (DoRender)
        {
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
        }

//#if defined(FXPLAYER_RENDER_DIRECT3D) && !defined(GFC_OS_XBOX360)
//      // prevent lost device from stopping resize handling
    
        if (!GetWidth() && !GetHeight() && SizeHeight && SizeWidth)
        {
            OnSize(SizeWidth, SizeHeight);
        }
//#endif

        // Potential out-of bounds range is not a problem here,
        // because it will be adjusted for inside of the player.
        if (pMovie) 
        {
            pMovie->SetViewport(GetWidth(), GetHeight(), (GetWidth()-ViewWidth)/2, 
                               (GetHeight()-ViewHeight)/2, ViewWidth, ViewHeight);
            pMovie->SetBackgroundAlpha(Background ? 1.0f : 0.0f);

            Float timeTillNextTicks = pMovie->Advance(deltaT * SpeedScale, 0);            

            NextTicksTime = TimeTicks + (UInt32)(timeTillNextTicks * 1000.0f);
            if (NextTicksTime < TimeTicks) // wrap-around check.
                NextTicksTime = TimeTicks;
        }

        if (DoRender) 
        {
            // Renderer-specific preparation (Disable depth testing)
            PrepareRendererForFrame();
            GetRenderer()->BeginFrame();
 //           if (pMovie)
 //               pMovie->DisplayPrePass();

            // Clear the entire buffer with dark gray.
            Clear(BackgroundColor);

            // Enable wireframe if requested.
            SetWireframe(Wireframe);
            SetRendererProfiling(OverdrawProfile);
        }

        if (pMovie)
        {
            pMovie->Display();
        }

#ifdef FXPLAYER_LOGO        
        GFx_DisplayLogo(GetRenderer(), GViewport(GetWidth(), GetHeight()+LOGO_BORDER, 
            0,0,
            GetWidth(), GetHeight()+LOGO_BORDER), pDummyImage);
#endif
        FrameCounter++;

        if (DoRender) 
        {
            // User matrix should be set before hud is updated.
            GRenderer::Matrix m;
            GetRenderer()->SetUserMatrix(m);

            SetWireframe(false);
            SetRendererProfiling(false);

            if (pHudDataProvider)
            {
#ifdef GFX_AMP_SERVER
                if (!GFxAmpServer::GetInstance().IsProfiling())
                {
                    pHudDataProvider->UpdateRendererStats();
                }
#endif
                if (!bDisplayedMasksWarning)
                {
                    UInt numMasks = pHudDataProvider->GetLastRenderMasksCount();
                    if (numMasks > 0)
                    {
                        GFxLog* plog = Loader.GetLog();
                        if (plog)
                            plog->LogMessage("Performance warning: Flash content uses masks. Scaleform does not recommend using masks as they degrade performance, especially in HUDs\n");
                        bDisplayedMasksWarning = true;
                    }
                }
            }

            if (pHud && (!pMovie || pHud->IsVisible() /* || SafeRectEnabled*/))
            {   				
				if (!pMovie)
					UpdateHUDViewport();

				if (!pMovie || pHud->IsVisible())
				{
					float deltaHud = (TimeTicks - LastHUDUpdate) / 1000.f;
					if (deltaHud > 1.f)
					{
						pHud->Update();
						LastHUDUpdate = TimeTicks;
					}
				}
			

                

                pHud->Display();            
                // Clear Renderer stats, so HUD stats will be excluded.
                if (!pMovie || pHud->IsVisible())
#ifdef GFX_AMP_SERVER
                    GetRenderer()->GetStats(0, !GFxAmpServer::GetInstance().IsValidConnection());
#else
                    GetRenderer()->GetStats(0);
#endif
            }
            GetRenderer()->SetUserMatrix(UserMatrix);  
            
            RedrawMouseCursor();

            // Flip buffers to display the scene.
            PresentFrame();

            GString strTitle;

            if (!pMovie || (!MeasurePerformance && !FastForward)) 
            {
                if (pMovie) 
                {
                    TimeTicks = GTimer::GetTicks() / 1000;
                    if (!VSync && (TimeTicks < NextTicksTime))
                        SleepTillMessage((unsigned int)(NextTicksTime - TimeTicks));
                } 
                else 
                {
                    // Don't hog the CPU.
                    SleepTillMessage(200);
                }
            } 
            else 
            {
                // Log the frame rate every second or so.
                if (TimeTicks - LastLoggedFps > 1000) 
                {
                    float delta = (TimeTicks - LastLoggedFps) / 1000.f;
                    LastFPS = (delta > 0) ? FrameCounter / delta : 0.0f;
                    TessTriangles = pRenderStats->GetTessStatistics();

                    // Display frame rate in title
                    G_Format(strTitle, FXPLAYER_APP_TITLE " (fps:{0:3.1})", LastFPS);

                    if(VSync)
                    {
                        strTitle += " VSync";
                    }
                }
            }
                   
            if (TimeTicks - LastLoggedFps > 1000) 
            {
                if (strTitle.IsEmpty())
                {
                    strTitle = FXPLAYER_APP_TITLE;
                }
#ifdef GFX_AMP_SERVER
                GString portString;
                if (GFxAmpServer::GetInstance().IsValidConnection())
                {
                    G_Format(portString, " Connected to AMP ({0})", AmpPort);
                }
                else if (GFxAmpServer::GetInstance().IsEnabled())
                {
                    G_Format(portString, " Listening for AMP ({0})", AmpPort);
                }
                strTitle += portString;
#endif
                SetWindowTitle(strTitle);
            }

            pHudDataProvider->UpdatePerformanceStats(TimeTicks - LastLoggedFps < 1000);

            if (TimeTicks - LastLoggedFps > 1000) 
            {
                LastLoggedFps = TimeTicks;
                FrameCounter = 0;
            }

            // Inform the renderer that our frame ended, this is required on some
            // platforms such as XBox 360 with predicated tiling so that start
            // reusing resources locked in the previous frame (dynamic font cache, etc).
            GetRenderer()->EndFrame();
        }

        // If we're reached the end of the movie, exit.
        if (!DoLoop && pMovie &&
            (pMovie->GetCurrentFrame() + 1 == pMovieDef->GetFrameCount()) )
        break;
    }

#ifdef GFX_AMP_SERVER
    GFxAmpServer::GetInstance().CloseConnection();
#endif

    if (pHudDataProvider && Arguments.HasValue("RecordStatistics"))
        pHudDataProvider->SaveStatistics(Arguments.GetString("RecordStatistics"));   
    // Releasing the movie will automatically shut down its
    // background loading task, if any.
    pHud = NULL;
    pHudDataProvider = NULL;
    pMovie = 0;
    pMovieDef = 0;
    pHudHeap->Release();
    // If we started worker threads in the renderer we must end them here;
    // otherwise FinishAllThreads will lock up waiting indefinitely.
#ifdef FXPLAYER_PROGRESSIVE_LOADING
    ptaskManager->RequestShutdown();    
#endif

#ifndef GFC_NO_SOUND
    ShutdownSound();
#endif

    return 0;
}

/*
// Example of installing custom image creator. You may want to do this if you want to
// create a custom GFxImageInfoBase derived class, such as the one which loads files
// directly on demand instead of pre-loading them on startup. Not that the actual
// implementation of default GFxImageCreator is more involved since it creates texture
// directly for renderers that don't require backup image.

#include "GFxImageResource.h"
class CustomImageCreator : public GFxImageCreator
{
public:
    GImageInfoBase* CreateImage(const GFxImageCreateInfo &info)
    {   
        GPtr<GImage> pimage;

        switch(info.Type)
        {
        case GFxImageCreateInfo::Input_Image:
            // We have to pass correct size; it is required at least
            // when we are initializing image info with a texture.
            return new GImageInfo(info.pImage, info.pImage->Width, info.pImage->Height);

        case GFxImageCreateInfo::Input_File:
            {
                // If we got here, we are responsible for loading an image file.
                GPtr<GFile> pfile  = *info.pFileOpener->OpenFile(info.pFileInfo->FileName.ToCStr());
                if (!pfile)
                    return 0;

                // Load an image into GImage object.
                pimage = *GFxImageCreator::LoadBuiltinImage(pfile, info.pFileInfo->Format, info.Use,
                                                            info.pLog, info.pJpegSupport, info.pPNGSupport);
                if (!pimage)
                    return 0;
                return new GImageInfo(pimage, info.pFileInfo->TargetWidth, 
                                              info.pFileInfo->TargetHeight);
            }        
            break;

            // No input - empty image info.
        case GFxImageCreateInfo::Input_None:
        default:
            break;
        }
        return new GImageInfo();
    }
};
*/


 // An example of ExternalInterface handler implementation
 class CustomEIHandler : public GFxExternalInterface
 {
 public:
	 CustomEIHandler() { }

     void Callback(GFxMovieView* pmovieView, const char* methodName, const GFxValue* args, UInt argCount)
     {
        GPtr<GFxLog> pLog = pmovieView->GetLog();
        pLog->LogMessage("\nCallback! %s, nargs = %d\n", (methodName)?methodName:"(null)", argCount);
        for (UInt i = 0; i < argCount; ++i)
        {
            pLog->LogMessage("  arg(%d) = ", i);
            switch(args[i].GetType())
            {
            case GFxValue::VT_String: pLog->LogMessage("%s", args[i].GetString()); break;
            case GFxValue::VT_Number: pLog->LogMessage("%f", args[i].GetNumber()); break;
            case GFxValue::VT_Boolean: pLog->LogMessage("%s", (args[i].GetBool())?"true":"false"); break;
            // etc...
            default:
                break;
            }
            pLog->LogMessage("\n");
        }
     }
 };
 

// Load a new movie from a file and initialize timing
bool FxPlayerApp::LoadMovie( const GString& filename)
{

    GString filenameStr = filename;
    GString argString;
#if !defined(GFC_NO_VIDEO) && defined(GFC_USE_VIDEO)
    if (IsVideoFile(filenameStr))
    {
        // a video file to a swf file as an argument.
        argString = filenameStr;
        filenameStr = "_internal_video_player";
        ClippingEnable = false;
    }
#endif
    // Try to load the new movie
    GPtr<GFxMovieDef> pnewMovieDef;
    GPtr<GFxMovieView> pnewMovie;
    GFxMovieInfo newMovieInfo;

    ResetCursor();
    ResetUserMatrix();

    // Get info about the width & height of the movie.
    if (!Loader.GetMovieInfo(filenameStr, &newMovieInfo, 0,GFxLoader::LoadKeepBindData)) 
    {
        fprintf(stderr, "Error: Failed to get info about %s\n", filenameStr.ToCStr());
        return 0;
    }

    UInt loadConstants = GFxLoader::LoadAll;
	/*
    GPtr<GFxImageCreator> imageCreator = *new CustomImageCreator;
    Loader.SetImageCreator(imageCreator);
    */

    // Load the actual new movie and crate instance.
    // Don't use library: this will ensure that the memory is released.
    pnewMovieDef = *Loader.CreateMovie(filenameStr, loadConstants
                                                  |GFxLoader::LoadKeepBindData
                                                  |GFxLoader::LoadWaitFrame1);
    if (!pnewMovieDef) 
    {
        fprintf(stderr, "Error: Failed to create a movie from '%s'\n", filenameStr.ToCStr());
        return 0;
    }

    pnewMovie = *pnewMovieDef->CreateInstance(GFx::MemoryParams(), false);
    if (!pnewMovie) 
    {
        fprintf(stderr, "Error: Failed to create movie instance\n");
        return 0;
    }

    // If this succeeded, replace the old movie with the new one.
    pMovieDef = pnewMovieDef;
    pMovie = pnewMovie;
    memcpy(&MovieInfo, &newMovieInfo, sizeof(GFxMovieInfo));

    //pMovie->SetFocusGroup(1,1);


    if (filenameStr.GetSize() > sizeof(FileName) - 1)
    {
        fprintf(stderr, "Error: File path '%s' is longer then %d \n", filenameStr.ToCStr(), UInt(sizeof(FileName)) - 1);
        return 0;
    }
    G_strcpy(FileName, sizeof(FileName), filenameStr.ToCStr());

    // Release cached memory used by previous file.
    if (pMovie->GetMeshCacheManager())
        pMovie->GetMeshCacheManager()->ClearCache();

    // Set a reference to the app
    pMovie->SetUserData(this);

    // This should only be true if this is the GFxPlayer application
    // Make sure to comment this out or set the value to false in your game
    pMovie->SetVariable("_global.gfxPlayer", GFxValue(true));

#if 0 //ndef GFC_NO_SOUND
    GFxValue _global, soundFunc;
    pMovie->GetVariable(&_global, "_global");
    GPtr<GFxSoundEventCallback> soundEventCallback = *GHEAP_NEW(GMemory::GetGlobalHeap()) GFxSoundEventCallback(pMovie);               
    pMovie->CreateFunction(&soundFunc, soundEventCallback);            
    _global.SetMember("gfxProcessSound", soundFunc);
#endif // GFC_NO_SOUND

    // Pass command line argument to ActionScript if necessary.
    if (Arguments.HasValue("Arg") && argString.IsEmpty())
        argString = Arguments["Arg"]->GetString();
    if (!argString.IsEmpty())
        pMovie->SetVariable("_global.gfxArg", argString.ToCStr());

    if (!CursorDisabled)
    {
        pMovie->SetMouseCursorCount(GetMouseCount());
    }
//    pMovie->SetControllerCount(GetKeyboardCount());

    const char* language = (FontConfigIndex == -1) ? "Default"
        : FontConfigs[FontConfigIndex]->ConfigName.ToCStr();
    pMovie->SetVariable("_global.gfxLanguage", GFxValue(language));

    // Set ActionScript verbosity / etc.
    GPtr<GFxActionControl> pactionControl = *new GFxActionControl();
    pactionControl->SetVerboseAction(Arguments["VerboseActions"]->GetBool());
#ifdef GFC_NO_FXPLAYER_VERBOSE_ACTION
    if (Arguments["VerboseActions"]->GetBool())
        fprintf(stderr, "VerboseAction is disabled by the GFC_NO_FXPLAYER_VERBOSE_ACTION macro in GConfig.h\n");
#endif

    pactionControl->SetActionErrorSuppress(Arguments["SuppressASErrors"]->GetBool());
    pactionControl->SetLogRootFilenames(Arguments["LogRootFilenames"]->GetBool());
    pactionControl->SetLogChildFilenames(!Arguments["NoLogChildFilnames"]->GetBool());
    pactionControl->SetLongFilenames(Arguments["LogFilePath"]->GetBool());
    pMovie->SetActionControl(pactionControl);

    // Install handlers.
    pMovie->SetFSCommandHandler(GPtr<GFxFSCommandHandler>(*new FxPlayerFSCallback()));
    pMovie->SetUserEventHandler(GPtr<GFxUserEventHandler>(*new FxPlayerUserEventHandler(this)));

    // setting ExternalInterface handler
    GPtr<CustomEIHandler> pei = *new CustomEIHandler();
    pMovie->SetExternalInterface(pei);


    //!AB, set active movie..this causes OnMovieFocus to get called on the IMEManager
    pMovie->HandleEvent(GFxEvent::SetFocus);

    // init first frame

    pMovie->Advance(0.0f, 0);
    
    // Renderer
    if (DoRender) 
    {
        if (AAMode == AAMode_EdgeAA)
            pRenderConfig->SetRenderFlags(pRenderConfig->GetRenderFlags()| GFxRenderConfig::RF_EdgeAA);
    }

    if (DoSound) 
    { 
        // No built-in sound support currently in GFx. Customers
        // can play their own sound through fscommand() callbacks.
    }

    // Disable pause.
    Paused = 0;

    // Init timing for the new piece.
    FrameCounter = 0;
    // Time ticks: always rely on a timer
    TimeStartTicks = GTimer::GetTicks()/1000;
    NextTicksTime = TimeStartTicks;
    LastLoggedFps = TimeStartTicks;
    // Movie logical ticks: either timer or setting controlled
    MovieStartTicks = (DoRender && !FastForward) ? TimeStartTicks : 0;
    MovieLastTicks = MovieStartTicks;

    if (pHudDataProvider)
        pHudDataProvider->SetMovie(pMovie);

    //if (pHud)
    //    pHud->Update(true, true);

    // Update the view
    UpdateViewSize();

    AmpDirty = true;

    return 1;
}

// Called when sizing begins and ends.
void FxPlayerApp::OnSizeEnter(bool enterSize) 
{
    // When leaving size, adjust to new width/height.
    if (!enterSize)  
    {
        SizingEntered = 0;

        if (GetRenderer() && ((SizeWidth != GetWidth()) || (SizeHeight != GetHeight()))) 
        {
            ResizeWindow(SizeWidth, 
#ifdef FXPLAYER_LOGO
                SizeHeight+LOGO_BORDER);
            Height -= LOGO_BORDER;
#else
                SizeHeight);
#endif
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
void FxPlayerApp::OnSize(SInt w, SInt h) 
{
    if (GetRenderer()) 
    {
        SizeWidth = w;
        SizeHeight= h;
#ifdef FXPLAYER_LOGO
        SizeHeight -= LOGO_BORDER;
#endif
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


void FxPlayerApp::ResetUserMatrix() 
{
    Move = GPointF(0.0f);
    Zoom = 1.0f;
    UpdateUserMatrix();
}

void FxPlayerApp::UpdateUserMatrix() 
{
    if (!GetRenderer())
        return;
    GRenderer::Matrix m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x * GFxMovie::GetRenderPixelScale(),
                        Move.y * GFxMovie::GetRenderPixelScale());
    GetRenderer()->SetUserMatrix(m);
    UserMatrix = m;
}

GSizeF FxPlayerApp::GetMovieScaleSize() 
{
    GSizeF scale;
    scale.Width = (pMovie->GetVisibleFrameRect().Width() / ViewWidth);
    scale.Height = (pMovie->GetVisibleFrameRect().Height() / ViewHeight);
    return scale;
}

void FxPlayerApp::OnAmpAppControlToggle(const GFxAmpMessageAppControl* message)
{
    GLock::Locker locker(&AmpCallbackLock);
    AmpAppControlMsgs.PushBack(const_cast<GFxAmpMessageAppControl*>(message));
}


// Updates the view size based on the ScaleEnable flag and window size.
void FxPlayerApp::UpdateViewSize() 
{
    if (ScaleEnable) 
    {
        SInt width = G_Max(GetWidth(), 4);
        SInt height= G_Max(GetHeight(), 4);

        if (ClippingEnable) 
        {
            // Determine movie size and location based on the aspect ratio
            float hw;
            if (MovieInfo.Height != 0 && MovieInfo.Width != 0)
                hw = (Float) MovieInfo.Height / (Float) MovieInfo.Width;
            else
                hw = (Float) height / (Float) width;
            if (width * hw > height) 
            {
                // Use height as the basis for aspect ratio
                ViewWidth = (SInt)((float) height / hw);
                ViewHeight = height;
            } 
            else 
            {
                // Use width
                ViewWidth = width;
                ViewHeight = (SInt) (width * hw);
            }
        } 
        else 
        {
            ViewWidth = width;
            ViewHeight = height;
        }
        // Compute input scale
        ScaleX = (Float) ViewWidth / (Float) MovieInfo.Width;
        ScaleY = (Float) ViewHeight / (Float) MovieInfo.Height;
    } 
    else 
    {
#ifdef FXPLAYER_RENDER_DIRECT3D
        // D3D9 doesn't allow offscreen viewports, so check for that
        if (GetWidth() < MovieInfo.Width || GetHeight() < MovieInfo.Height)
        {
            ViewWidth = GetWidth();
            ViewHeight = GetHeight();

            // Compute input scale
            ScaleX = (Float) ViewWidth / (Float) MovieInfo.Width;
            ScaleY = (Float) ViewHeight / (Float) MovieInfo.Height;
        }
        else
#endif
        {
            // No scaling, just center the image
            ViewWidth = MovieInfo.Width;
            ViewHeight = MovieInfo.Height;
            ScaleX = ScaleY = 1.0f;
        }
    }

    UpdateHUDViewport();
}

void FxPlayerApp::UpdateHUDViewport()
{
	if (pHud)
	{
		GViewport hudViewport = GViewport(GetWidth(), GetHeight(),
		SInt(GetWidth() * GetSafeArea()),
		SInt(GetHeight() * GetSafeArea()),
		SInt(GetWidth() - 2 * GetWidth() * GetSafeArea()),
		SInt(GetHeight() - 2 * GetHeight() * GetSafeArea()));
		pHud->SetViewport(hudViewport);
	}
}

void FxPlayerApp::SetRendererProfiling(bool on)
{
    GAmpRenderer* pAmpRenderer = GetAmpRenderer();
    if (pAmpRenderer)
    {
        if (!on)
            pAmpRenderer->DisableProfile();
        else
            pAmpRenderer->ProfileCounters(
            (UInt64(GAmpRenderer::Profile_Fill   | 16) << GAmpRenderer::Channel_Green) |
            (UInt64(GAmpRenderer::Profile_Mask   | 32) << GAmpRenderer::Channel_Red) |
            (UInt64(GAmpRenderer::Profile_Filter | 32) << GAmpRenderer::Channel_Blue));
    }
}

void FxPlayerApp::ToggleAaMode()
{
    bool renderChange = 0;
    bool edgeChange = 0;

    switch(AAMode)
    {
    case AAMode_None:
        AAMode = AAMode_EdgeAA;
        edgeChange = 1;
        break;
    case AAMode_EdgeAA:
        if(FSAASupported)
            AAMode = AAMode_FSAA;
        else
            AAMode = AAMode_None;
        edgeChange = renderChange = 1;
        break;
    case AAMode_FSAA:
        AAMode = AAMode_None;
        renderChange = 1;
        break;
    }

    if (renderChange) 
    {
        // FSAA toggle
        SwitchFSAA(AAMode == AAMode_FSAA);
    }
    if (edgeChange && pMovie)
    {
        UInt32 rendererFlags = pRenderConfig->GetRenderFlags() & ~GFxRenderConfig::RF_EdgeAA;
        if (AAMode == AAMode_EdgeAA)
            rendererFlags |= GFxRenderConfig::RF_EdgeAA;
        pRenderConfig->SetRenderFlags(rendererFlags);
    }
    AmpDirty = true;
}

void FxPlayerApp::ToggleStrokeType()
{
    UInt32 rf = pRenderConfig->GetRenderFlags();
    UInt32 stroke = rf & GFxRenderConfig::RF_StrokeMask;

    switch (stroke) 
    {
    case GFxRenderConfig::RF_StrokeHairline:
        stroke = GFxRenderConfig::RF_StrokeNormal;
        break;
    case GFxRenderConfig::RF_StrokeNormal:
        stroke = GFxRenderConfig::RF_StrokeCorrect;
        break;
    case GFxRenderConfig::RF_StrokeCorrect:
        stroke = GFxRenderConfig::RF_StrokeHairline;
        break;
    }
    pRenderConfig->SetRenderFlags((rf & ~GFxRenderConfig::RF_StrokeMask) | stroke);
    AmpDirty = true;
}

// Switch international font.
void FxPlayerApp::CycleFontConfig()
{
    if ((FontConfigIndex != -1) && (FontConfigs.GetSize() > 1))
    {
        // Apply different settings and reload file.

        FontConfigIndex++;
        FontConfigIndex %= (SInt)FontConfigs.GetSize();
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
        {
            pconfig->Apply(&Loader);
            const char* language = (FontConfigIndex == -1) ? "Default"
                : pconfig->ConfigName.ToCStr();
            pMovie->Invoke("_root.OnLanguageChanged", "%s", language);
        }
        AmpDirty = true;
    }
}

void FxPlayerApp::HandleAmpAppMessages()
{
    AmpCallbackLock.DoLock();
    for (UPInt i = 0; i < AmpAppControlMsgs.GetSize(); ++i)
    {
        GFxAmpMessageAppControl* msg = AmpAppControlMsgs[i];
        if (msg->IsToggleWireframe())
        {
            Wireframe = !Wireframe;
        }
        if (msg->IsToggleOverdraw())
        {
            OverdrawProfile = !OverdrawProfile;
        }
        if (msg->IsTogglePause())
        {
            Paused = !Paused;
            OnPause(Paused);
        }
        if (msg->IsToggleFastForward())
        {
            FastForward = !FastForward;
        }
        if (msg->IsToggleAaMode())
        {
            ToggleAaMode();
        }
        if (msg->IsToggleStrokeType())
        {
            ToggleStrokeType();
        }
        if (msg->IsRestartMovie())
        {
            Paused = false;
            OnPause(Paused);
            pMovie->Restart();
        }
        if (msg->IsNextFont())
        {
            CycleFontConfig();
        }
        if (msg->IsCurveToleranceUp())
        {
            CurvePixelError = G_Min(10.0f, CurvePixelError + 0.5f);
            pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
        }
        if (msg->IsCurveToleranceDown())
        {
            CurvePixelError = G_Max(0.5f, CurvePixelError - 0.5f);
            pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
        }
        if (!msg->GetLoadMovieFile().IsEmpty())
        {
            LoadMovie(msg->GetLoadMovieFile());
        }
        AmpDirty = true;
    }
    AmpAppControlMsgs.Clear();
    AmpCallbackLock.Unlock();

    UpdateAmpState();
}

void FxPlayerApp::UpdateAmpState()
{
#ifdef GFX_AMP_SERVER
    if (AmpDirty)
    {
        GPtr<GFxAmpCurrentState> ampState = *GNEW GFxAmpCurrentState();

        ampState->StateFlags = GFxAmpServer::GetInstance().GetCurrentState();
        if (OverdrawProfile)
        {
            ampState->StateFlags |= Amp_RenderOverdraw;
        }
        else
        {
            ampState->StateFlags &= ~Amp_RenderOverdraw;
        }
        if (Wireframe)
        {
            ampState->StateFlags |= Amp_App_Wireframe;
        }
        else
        {
            ampState->StateFlags &= ~Amp_App_Wireframe;
        }
        if (Paused)
        {
            ampState->StateFlags |= Amp_App_Paused;
        }
        else
        {
            ampState->StateFlags &= ~Amp_App_Paused;
        }
        if (FastForward)
        {
            ampState->StateFlags |= Amp_App_FastForward;
        }
        else
        {
            ampState->StateFlags &= ~Amp_App_FastForward;
        }

        ampState->ConnectedApp = FXPLAYER_APP_TITLE;
        ampState->ConnectedFile = FileName;
        ampState->CurveToleranceMin = 0.5f;
        ampState->CurveToleranceMax = 10.0f;
        ampState->CurveToleranceStep = 0.5f;
        for (UInt i = 0; i < FontConfigs.GetSize(); ++i)
        {
            ampState->Locales.PushBack(FontConfigs[i]->ConfigName);
        }
        FxHUDExtraInfo extraInfo;
        GetExtraInfo(&extraInfo);
        ampState->AaMode = extraInfo.AAMode;
        ampState->StrokeType = extraInfo.StrokeType;
        ampState->CurrentLocale = extraInfo.FontConfig;
        ampState->CurveTolerance = CurvePixelError;

        GFxAmpServer::GetInstance().UpdateState(ampState);
        AmpDirty = false;
    }
#endif
}


void FxPlayerApp::GetExtraInfo(FxHUDExtraInfo* pinfo) 
{ 
    pinfo->LastFPS = LastFPS;
    pinfo->TessTriangles = TessTriangles;
    pinfo->Zoom = Zoom;   
    pinfo->Paused = Paused;
    switch (AAMode) 
    {
    case AAMode_None:
        pinfo->AAMode = "None";
        break;
    case AAMode_EdgeAA:
        // Display a custom message if edge AA is #ifdef-ed out.
#ifndef GFC_NO_FXPLAYER_EDGEAA
        pinfo->AAMode = "Edge AA";
        {
            // Report if EdgeAA is not supported by renderer.
            GRenderer::RenderCaps caps;
            caps.CapBits = 0;
            if (GetRenderer())
                GetRenderer()->GetRenderCaps(&caps);
            if (!(caps.CapBits & GRenderer::Cap_FillGouraud))
                pinfo->AAMode = "Edge AA (Not Supported)";
            else if (!(caps.CapBits & GRenderer::Cap_FillGouraudTex))
                pinfo->AAMode = "Edge AA (Limited)";
        }
#else
        pinfo->AAMode = "Edge AA [#disabled]";
#endif
        break;
    case AAMode_FSAA:
        if (FSAASupported)
            pinfo->AAMode = "HW FSAA";
        else
            pinfo->AAMode = "HW FSAA (Not Supported)";
        break;
    }
    
    pinfo->CurvePixelError = CurvePixelError;
    
    // Stroke type
    UInt32 stroke = GFxRenderConfig::RF_StrokeCorrect;
    bool optTriangles = false;

    if (pRenderConfig) 
    {
        stroke = (pRenderConfig->GetRenderFlags() & GFxRenderConfig::RF_StrokeMask);
        optTriangles = (pRenderConfig->GetRenderFlags() & GFxRenderConfig::RF_OptimizeTriangles) != 0;
    }

    // Display a custom message if stroke is #ifdef-ed out.
#ifndef GFC_NO_FXPLAYER_STROKER
    pinfo->StrokeType = "Correct";
    if (stroke == GFxRenderConfig::RF_StrokeNormal)
        pinfo->StrokeType = "Normal";
    else if (stroke == GFxRenderConfig::RF_StrokeHairline)
        pinfo->StrokeType = "Hairline";
#else
    pinfo->StrokeType = "Correct [#disabled]";
    if (stroke == GFxRenderConfig::RF_StrokeNormal)
        pinfo->StrokeType = "Normal [#disabled]";
#endif

   pinfo->FontConfig = (FontConfigIndex == -1) ? "Default"
        : FontConfigs[FontConfigIndex]->ConfigName.ToCStr(); 
}

void FxPlayerApp::GetHelpStr(GString* phelpStr)
{
    phelpStr->Clear(); 
    if (!IsConsole())
    {
        *phelpStr += "Keys:\n";
        for (int i = 0; FxPlayerCommandKeyMap[i].keyCode != FxApp::VoidSymbol; i++)
            *phelpStr += FxPlayerCommandKeyMap[i].helpText;
        *phelpStr +="\n";
    }
    if (IsPadConnected(0))
    {
        *phelpStr += "Pad:\n";
        for (int i = 0; FxPlayerCommandPadKeyMap[i].keyCode != FxApp::VoidPadKey; i++)
            *phelpStr += FxPlayerCommandPadKeyMap[i].helpText;
    }

}

// Handle dropped file
void FxPlayerApp::OnDropFiles(const GString& path)
{
    // unload the translator
    if (FontConfigIndex >= 0)
        Loader.SetTranslator(NULL);

    if (LoadDefaultFontConfigFromPath(path) ) 
    {
        // set the font config
        FontConfig* pconfig = GetCurrentFontConfig();
        if (pconfig)
            pconfig->Apply(&Loader);
    }

    LoadMovie(path);

    //!AB, set active movie
 //   if (pMovie)
 //       pMovie->HandleEvent(GFxEvent::SetFocus);

    BringMainWindowOnTop();
}


void FxPlayerApp::ProcessCommand(UInt controllerIdx, FxPlayerCommand cmd, void* pdata /*= NULL*/ )
{
    GStringBuffer buffer;

    switch (cmd) 
    {
        case FPC_Quit:
            QuitFlag = 1;
            return;

        case FPC_HudReport:
            if (pHudDataProvider)
            {
                if (Arguments.HasValue("RecordStatistics"))
                    pHudDataProvider->SaveStatistics(Arguments.GetString("RecordStatistics"));               
                else
                    pHudDataProvider->PrintReport(!pHud);
            }
            break;

        case FPC_Mem_Report:
            GMemory::pGlobalHeap->MemReport(buffer, GMemoryHeap::MemReportFull);
            printf("\n%s\n", buffer.ToCStr());
            break;

        case FPC_CurveToleranceUp: // 219 '['
            CurvePixelError = G_Min(10.0f, CurvePixelError + 0.5f);
            pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
            AmpDirty = true;
            break;

        case FPC_CurveToleranceDown: // 221 ']':
            CurvePixelError = G_Max(0.5f, CurvePixelError - 0.5f);
            pRenderConfig->SetMaxCurvePixelError(CurvePixelError);
            AmpDirty = true;
            break;

        case FPC_Wireframe:
            // Toggle wireframe.
            Wireframe = !Wireframe;
            AmpDirty = true;
            break;
        
        case FPC_VSync:
            // Toggle VSync
            SetVSync(!VSync);
            break;

#ifndef GFC_NO_SOUND        
        case FPC_Mute:
            //Toggle Mute
            {
                IsMute = !IsMute;
                GSoundRenderer* psoundRenderer = GetSoundRenderer();
                if(psoundRenderer)
                    psoundRenderer->Mute(IsMute);
            }
            break;

        case FPC_VolumeUp:
            //Increase volume
            {
                GSoundRenderer* psoundRenderer = GetSoundRenderer();
                if(psoundRenderer)
                {
                    if (IsMute)
                    {
                        psoundRenderer->Mute(false);
                        IsMute = false;
                    } 
                    else
                    {
                        SoundVolume+=10;
                        if (SoundVolume > 100)
                            SoundVolume = 100;
                        psoundRenderer->SetMasterVolume(SoundVolume);
                    }
                }
            }
            break;

        case FPC_VolumeDown:
            //Decrease volume
            {
                GSoundRenderer* psoundRenderer = GetSoundRenderer();
                if(psoundRenderer)
                {
                    if (IsMute)
                    {
                        psoundRenderer->Mute(false);
                        IsMute = false;
                    } 
                    else
                    {
                        SoundVolume-=10;
                        if (SoundVolume < 0)
                            SoundVolume = 0;
                        psoundRenderer->SetMasterVolume(SoundVolume);
                    }
                }
            }
            break;
#endif //#ifndef GFC_NO_SOUND

	    // Switch to a next stroke type.
        case FPC_StrokeMode: 
            ToggleStrokeType();
            break;

        case FPC_Pause:
            // Toggle paused state.
            Paused = !Paused;
            OnPause(Paused);
            AmpDirty = true;
            break;

        case FPC_Restart:
            Paused = false;
            OnPause(Paused);
            pMovie->Restart();
            AmpDirty = true;
            break;

        case FPC_StepForward_1:
             pMovie->GotoFrame(pMovie->GetCurrentFrame()+1);
        onkey_finish_seek: 
             Paused = 1;
             pMovie->SetPlayState(GFx::State_Playing);
             // Ensure tag actions are executed. This may change frame state to Stopped.
             pMovie->Advance(0.0f);
             pMovie->SetPause(Paused);
             PausedState = pMovie->GetPlayState();
             AmpDirty = true;
            break;

        case FPC_StepBack_1:
            pMovie->GotoFrame(pMovie->GetCurrentFrame()-1);
            goto onkey_finish_seek;

        case FPC_StepBack_10: //PRIOR
            pMovie->GotoFrame(G_Max<UInt>(0, pMovie->GetCurrentFrame()-10) );
            goto onkey_finish_seek;

        case FPC_StepForward_10: // NEXT
            pMovie->GotoFrame(G_Min<UInt>(pMovie->GetCurrentFrame()+10, MovieInfo.FrameCount-1));
            goto onkey_finish_seek;

        case FPC_InfoHelp:
            if (pHud)
                pHud->ToggleTab(FxHUD::Help);
            break;
        case FPC_InfoSummary:
            if (pHud)
                pHud->ToggleTab(FxHUD::Summary);
            break;       
        case FPC_HudNextTab:            
            if (pHud)
                pHud->NextTab();
            break;

        case FPC_HudPreviousTab:            
            if (pHud)
                pHud->PreviousTab();
            break;
        case FPC_HudClose:
		case FPC_HudOpen:
            if (pHud)
                pHud->SetVisible(cmd == FPC_HudOpen);
            break;

        case FPC_SafeRect:
            //SafeRectEnabled = !SafeRectEnabled;
            break;
        case FPC_AntialiasingMode:
            ToggleAaMode();
            break;

        case FPC_Fullscreen:
            pRenderConfig->SetRenderer(0);
            if (pHud)
                pHud->SetRenderer(0);
            SwitchFullScreenMode();
            pRenderConfig->SetRenderer(GetRenderer());
            if (pHud)
            {
                pHud->SetRenderer(GetRenderer());
            }

#ifdef FXPLAYER_LOGO
            Height -= LOGO_BORDER;
#endif
            UpdateViewSize();
            SetWindowTitle(FXPLAYER_APP_TITLE);
            break;

        case FPC_ScaledDisplay:
            // Toggler scale
            ScaleEnable = !ScaleEnable;
            UpdateViewSize();
            break;

        case FPC_StageClipping:
            // Toggler clipping
            ClippingEnable = !ClippingEnable;
            UpdateViewSize();
            break;

        case FPC_FastForward:
            FastForward = !FastForward;
            AmpDirty = true;
            break;

        case FPC_Background:
            Background = !Background;
            break;

        case FPC_BackgroundColor:
            switch(BackgroundColor)
            {
            case 0x555555: //Gray 
                BackgroundColor = 0x0000FF; 
                break;
            case 0x0000FF: //Blue
                BackgroundColor = 0x000000; 
                break;
            case 0x000000: //Black
                BackgroundColor = 0xFFFFFF; 
                break;
            case 0xFFFFFF: //White
                BackgroundColor = 0x555555; 
                break;
            }
            break;
        case FPC_CtrlKey:
            NoCtrlKey = !NoCtrlKey;
            break;
        case FPC_FastMode:
            MeasurePerformance = !MeasurePerformance;
            pRenderStats->GetTessStatistics(); // Clear stats
            LastFPS = 0;

            if (!MeasurePerformance)
                SetWindowTitle(FXPLAYER_APP_TITLE);
            break;

        case FPC_ResetUserMatrix:
            ResetUserMatrix();
            break;

        case FPC_FontConfig:
            CycleFontConfig();
            break;

        case FPC_StageCulling:
            // Toggle viewport culling.
            if (pMovie && pRenderConfig)
            {
                UInt32 rendererFlags = pRenderConfig->GetRenderFlags();
                pRenderConfig->SetRenderFlags(rendererFlags ^ GFxRenderConfig::RF_NoViewCull);
            }
            break;

        case FPC_TriangleOptimization:
            if (pMovie && pRenderConfig)
            {
                UInt32 rendererFlags = pRenderConfig->GetRenderFlags();
                pRenderConfig->SetRenderFlags(rendererFlags ^ GFxRenderConfig::RF_OptimizeTriangles);
            }
            break;

#if defined(FXPLAYER_RENDER_DIRECT3D) && (GFC_D3D_VERSION < 10) && !defined(GFC_OS_XBOX360)
        case FPC_DynamicTextures:
            VMCFlags = (VMCFlags & ~renderer_type::VMConfig_UseDynamicTex) |
                ((~(VMCFlags & renderer_type::VMConfig_UseDynamicTex)) & renderer_type::VMConfig_UseDynamicTex);
            pRenderer->ResetVideoMode();
            SetRendererDependentVideoMode();
            break;
#endif
        case FPC_LoadNextFile:
        case FPC_LoadPrevFile:
            if(UpdateFiles(FileName, cmd == FPC_LoadPrevFile))
                LoadMovie(FileName);
            
            break;

        case FPC_UnloadMovie:
            UnloadMovie();
            break;

        case FPC_ShowMouseCursor:
            CursorHidden = !CursorHidden;
            ShowCursor(!CursorHidden);
            if (!CursorHidden)
            {
                if (CursorDisabled && pMovie)
                    pMovie->SetMouseCursorCount(GetMouseCount());
                CursorDisabled = false;
            }
            else
            {
                if (pMovie)
                    pMovie->SetMouseCursorCount(0);
                CursorDisabled = true;
            }
            break;

        case FPC_ToggleOverdrawProfile:
            OverdrawProfile = !OverdrawProfile;
            AmpDirty = true;
            break;

        case KEY_Return:
        case KEY_Escape:
            if (pdata)
            {
                bool* pkeyDown = (bool*)pdata;
                KeyEvent(controllerIdx, cmd == KEY_Return ? GFxKey::Return : GFxKey::Escape, 0, 0, 0, *pkeyDown);   
            }            
            break;

        default:
        	break;
        } // switch(keyCode)
}


bool FxPlayerApp::OnIMEEvent(const FxAppIMEEvent& event)
{
#ifndef GFC_NO_IME_SUPPORT
    return IMEHelper::OnEvent(event, pMovie);
#else
    GUNUSED(event);
    return false;
#endif
}


void FxPlayerApp::OnPad(UInt controllerIdx, PadKeyCode keyCode, bool downFlag)
{
    if (!pMovie)
        return;
    switch(keyCode)
    {
    case FxApp::Pad_Left:
        KeyEvent(controllerIdx, GFxKey::Left, 0,0,0, downFlag);
        return;
    case FxApp::Pad_Right:
        KeyEvent(controllerIdx, GFxKey::Right, 0,0,0, downFlag);
        return;
    case FxApp::Pad_Up:
        KeyEvent(controllerIdx, GFxKey::Up, 0,0,0, downFlag);
        return;
    case FxApp::Pad_Down:
        KeyEvent(controllerIdx, GFxKey::Down, 0,0,0, downFlag);
        return;
    default:
        break;
    }
    
    for (int i = 0; FxPlayerCommandPadKeyMap[i].keyCode != FxApp::VoidPadKey; i++)
    {
        if (keyCode == FxPlayerCommandPadKeyMap[i].keyCode)
        {
            // When HUD is visible, some pad keys will have different functions
            FxPlayerCommand cmd;
            if(pHud && pHud->IsVisible())
                cmd = FxPlayerCommandPadKeyMap[i].hudCmd;
            else
                cmd = FxPlayerCommandPadKeyMap[i].cmd;
            
            if (!downFlag && cmd != KEY_Return && cmd != KEY_Escape)
                return;

            ProcessCommand(controllerIdx, cmd, &downFlag);
            break;
        }
    }
    
}


void FxPlayerApp::OnKey(UInt controllerIdx, KeyCode keyCode, unsigned char asciiCode,
                        unsigned int wcharCode, unsigned int mods, bool downFlag) 
{
    if (!pMovie)
        return;

    bool ctrl = false;

    if (!NoControlKeys) 
    {
        ctrl = mods & FxApp::KM_Control;        

        if (keyCode == FxApp::Control && !NoCtrlKey) 
            return;

        // TODO ???
        //if (keyCode == VK_MENU && downFlag)
        //    ControlKeyDown = false; // to enable Ctrl-Alt-... combinations to work

        if (keyCode == FxApp::Escape && downFlag) 
        {
            // Cancel mouse manipulation
            if (MouseTracking != None) 
            {
                MouseTracking = None;
                EndMouseCapture();
                Zoom = ZoomStart;
                Move = MoveStart;
                UpdateUserMatrix();
                return;
            }
        }

        if (keyCode == FxApp::F1 && downFlag)
            ProcessCommand(controllerIdx, FPC_InfoHelp);
        else if (keyCode == FxApp::F2 && downFlag)
            ProcessCommand(controllerIdx, FPC_InfoSummary);
        else if (keyCode == FxApp::F3 && downFlag)
            ProcessCommand(controllerIdx, FPC_RefreshHud);
        else if ((keyCode == FxApp::F9 && downFlag))
            ProcessCommand(controllerIdx, FPC_CtrlKey);
        else if (keyCode == FxApp::F12 && downFlag)
            ProcessCommand(controllerIdx, FPC_HudClose);
        else if (keyCode == FxApp::F8 && downFlag)
            ProcessCommand(controllerIdx, FPC_BackgroundColor);
/*
        else if (keyCode == GFxKey::Num1 && downFlag)
            KeyboardIndex = 0;
        else if (keyCode == GFxKey::Num2 && downFlag)
        {
            KeyboardIndex = 1;
            pMovie->SetControllerFocusGroup(1,0);
        }
        else if (keyCode == GFxKey::Num3 && downFlag)
        {
            KeyboardIndex = 1;
            pMovie->SetControllerFocusGroup(1,1);
        }*/
    }

    if (ctrl && downFlag && !NoCtrlKey) 
    {
	    // Handle Ctrl-Key combinations
        for (int i = 0; FxPlayerCommandKeyMap[i].keyCode != FxApp::VoidSymbol; i++)
        {
            if (keyCode == FxPlayerCommandKeyMap[i].keyCode)
            {
                ProcessCommand(controllerIdx, FxPlayerCommandKeyMap[i].cmd);
                break;
            }
        }
    }
    else
    {
        // @TEMP
//         bool rest = false;
//         if (keyCode >= FxApp::KP_0 && keyCode <= FxApp::KP_Divide)
//         {
//             switch(keyCode)
//             {
//             case FxApp::KP_4: keyCode = FxApp::Left; break;
//             case FxApp::KP_6: keyCode = FxApp::Right; break;
//             case FxApp::KP_8: keyCode = FxApp::Up; break;
//             case FxApp::KP_2: keyCode = FxApp::Down; break;
//             case FxApp::KP_7: keyCode = FxApp::Tab; break;
//             }
//             KeyboardIndex = 1;
//             pMovie->SetControllerFocusGroup(1,1);
//             rest = 1;
//         }

        // Inform the player about keystroke
        if (!ctrl || NoCtrlKey)
            KeyEvent(controllerIdx, (GFxKey::Code)keyCode, asciiCode, wcharCode, mods, downFlag);

        // @TEMP
//         if (rest)
//         {
//             KeyboardIndex = 0;
//         }
    }

}

// Helper used to convert key codes and route them to GFxPlayer
void FxPlayerApp::KeyEvent(UInt controllerIdx, GFxKey::Code keyCode, unsigned char asciiCode,
                           unsigned int wcharCode, unsigned int mods, bool down)
{
    if (keyCode != GFxKey::VoidSymbol) 
    {
        if (pMovie) 
        {
            GFxKeyEvent event(down ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, keyCode, asciiCode, wcharCode);
            event.mSpecialKeysState.SetShiftPressed(mods & FxApp::KM_Shift ? 1 : 0);
            event.mSpecialKeysState.SetCtrlPressed(mods & FxApp::KM_Control ? 1 : 0);
            event.mSpecialKeysState.SetAltPressed(mods & FxApp::KM_Alt ? 1 : 0);
            event.mSpecialKeysState.SetNumToggled(mods & FxApp::KM_Num ? 1 : 0);
            event.mSpecialKeysState.SetCapsToggled(mods & FxApp::KM_Caps ? 1 : 0);
            event.mSpecialKeysState.SetScrollToggled(mods & FxApp::KM_Scroll ? 1 : 0);
            event.KeyboardIndex     = (UInt8)controllerIdx;
            pMovie->HandleEvent(event);
        }
    }
}


void FxPlayerApp::OnChar(UInt controllerIdx, UInt32 wcharCode, UInt info) 
{
    GUNUSED(info);
    if (pMovie && wcharCode) 
    {
        GFxCharEvent event(wcharCode); 
        event.KeyboardIndex     = (UInt8)controllerIdx;
        pMovie->HandleEvent(event);
    }
}


void FxPlayerApp::OnMouseButton(unsigned mouseIndex, unsigned int button, bool downFlag, int x, int y, 
                                unsigned int mods)
{
    bool ControlKeyDown = mods & FxApp::KM_Control;
    if (!pMovie)
        return;

    //if (!CursorDisabled)
    //    pMovie->EnableMouseSupport(1);
    //CursorDisabled = false;

    Float mX, mY;
    MouseX = x;
    MouseY = y;

    // Adjust x, y to viewport.
    GSizeF s = GetMovieScaleSize();
    mX = ((x - (GetWidth()  - ViewWidth ) / 2)) * s.Width;
    mY = ((y - (GetHeight() - ViewHeight) / 2)) * s.Height;

    GRenderer::Matrix m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    GRenderer::Point p = m.TransformByInverse(GRenderer::Point(mX, mY));

    mX = (p.x / s.Width);
    mY = (p.y / s.Height);

    // UpdateStatBags mouse state
    if (downFlag) 
    {
        MouseDownX = MouseX;
        MouseDownY = MouseY;

        StartMouseCapture();

        if (button == 0 && ControlKeyDown)
        {
            MouseTracking = Zooming;
        }
        else if (button == 1 && ControlKeyDown)
        {
            MouseTracking = Moving;
        }
      
        if (MouseTracking != None) 
        {
            ZoomStart = Zoom;
            MoveStart = Move;
            return;
        }
        
        GFxMouseEvent event(GFxEvent::MouseDown, button, mX, mY, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    } 
    else 
    {
        EndMouseCapture();

        if (MouseTracking != None)
        {
            MouseTracking = None;
            return;
        } 
        
        GFxMouseEvent event(GFxEvent::MouseUp, button, mX, mY, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }
}


void FxPlayerApp::OnMouseWheel(unsigned mouseIndex, int zdelta, int x, int y, unsigned int mods)
{
    bool ControlKeyDown = mods & FxApp::KM_Control;
    if (ControlKeyDown)// && MouseTracking == None)
    {
        ZoomStart = Zoom;

        Float dZoom = Zoom;
        Zoom += 0.02f * (zdelta / 128) * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;

        GSizeF s = GetMovieScaleSize();
        Float mX = ((x - (GetWidth()-ViewWidth)/2)) * s.Width;
        Float mY = ((y - (GetHeight()-ViewHeight)/2)) * s.Height;
        GRenderer::Matrix m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(Move.x, Move.y);
        GRenderer::Point p = m.TransformByInverse(GRenderer::Point(mX, mY));
        mX = (Float) (int) (p.x / s.Width);
        mY = (Float) (int) (p.y / s.Height);

        Move.x += s.Width * dZoom * mX;
        Move.y += s.Height * dZoom * mY;

        UpdateUserMatrix();
        return;
    }

    if (!pMovie)
        return;

    Float mX;
    Float mY;

    GSizeF s = GetMovieScaleSize();


    // Adjust x, y to viewport.
    mX = ((x - (GetWidth()  - ViewWidth)  / 2)) * s.Width;
    mY = ((y - (GetHeight() - ViewHeight) / 2)) * s.Height;

    GRenderer::Matrix m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    GRenderer::Point p = m.TransformByInverse(GRenderer::Point(mX, mY));

    mX = (p.x / s.Width);
    mY = (p.y / s.Height);

    GFxMouseEvent event(GFxEvent::MouseWheel, 0, mX, mY, 
        (Float)((zdelta/G_Min(G_Abs(zdelta), WHEEL_DELTA))*3), mouseIndex);

    pMovie->HandleEvent(event);
}


void FxPlayerApp::OnMouseMove(unsigned mouseIndex, int x, int y, unsigned int mods)
{
    GUNUSED(mods);

    Float mX;
    Float mY;

    /*
	if (pHud && pHud->IsVisible() && MouseTracking == None)
    {
        mX = (x - GetWidth() * GetSafeArea()); 
        mY = (y  - GetHeight() * GetSafeArea());
        GFxMouseEvent event(GFxEvent::MouseMove, 0, mX, mY, 0.0f, mouseIndex);
        if (pHud->HandleEvent(event) != GFxMovieView::HE_NotHandled)
            return;
    }
	*/


    Float dX = (Float) MouseX - x;
    Float dY = (Float) MouseY - y;
    MouseX = x;
    MouseY = y;

    // Used by NotifyMouseState in the main loop
    if (!pMovie)
        return;

    GSizeF s = GetMovieScaleSize();

    if (MouseTracking == Zooming) 
    {
        Float dZoom = Zoom; 
        Zoom += 0.01f * dY * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;

        mX = ((MouseDownX - (GetWidth()-ViewWidth)/2)) * s.Width;
        mY = ((MouseDownY - (GetHeight()-ViewHeight)/2)) * s.Height;
        GRenderer::Matrix m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(MoveStart.x, MoveStart.y);
        GRenderer::Point p = m.TransformByInverse(GRenderer::Point(mX, mY));
        mX = (Float) (int) (p.x / s.Width);
        mY = (Float) (int) (p.y / s.Height);

        Move.x += s.Width * dZoom * mX;
        Move.y += s.Height * dZoom * mY;

        UpdateUserMatrix();
        return;
    }
    if (MouseTracking == Moving) 
    {
        Move.x -= s.Width * dX;
        Move.y -= s.Height * dY;

        UpdateUserMatrix();
        return;
    }

    // Adjust x, y to viewport.
    mX = ((x - (GetWidth()-ViewWidth)/2)) * s.Width;
    mY = ((y - (GetHeight()-ViewHeight)/2)) * s.Height;

    GRenderer::Matrix m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    GRenderer::Point p = m.TransformByInverse(GRenderer::Point(mX, mY));

    mX = (p.x / s.Width);
    mY = (p.y / s.Height);
    //}
    GFxMouseEvent event(GFxEvent::MouseMove, 0, mX, mY, 0.0f, mouseIndex);
    pMovie->HandleEvent(event);

    /*
    // Mouse HitTest debugging logic

    Double xmouse = pMovie->GetVariableDouble("_root._xmouse");
    Double ymouse = pMovie->GetVariableDouble("_root._ymouse");

    int hitTest = pMovie->HitTest(x, y, GFxMovieView::HitTest_Shapes);

    pMovie->GetLog()->LogMessage("pMovie->HitTest: %d\n", hitTest);

    x = (SInt)(float(x) * MovieInfo.Width) / ViewWidth;
    y = (SInt)(float(y) * MovieInfo.Height) / ViewHeight;

    const char* pi = pMovie->Invoke("_root.hitTest", "%f%f%d", xmouse, ymouse, 1);
    pMovie->GetLog()->LogMessage("_root.hitTest: %s\n", pi);
    */
}


void FxPlayerApp::OnUpdateSystemCliboard(const wchar_t* text) 
{
    GPtr<GFxTextClipboard> pclipBoard = Loader.GetTextClipboard();
    if (pclipBoard)
        pclipBoard->SetText(text);
}


void FxPlayerApp::OnFocus(bool set, unsigned int mods)
{
    if (pMovie)
    {
        if (set)
        {
            // need to update toggled keys when focus is being transferred to
            // the movie's windows.
            GFxSpecialKeysState sks;
            sks.SetNumToggled((mods & FxApp::KM_Num) ? 1 : 0);
            sks.SetCapsToggled((mods & FxApp::KM_Caps) ? 1 : 0);
            sks.SetScrollToggled((mods & FxApp::KM_Scroll) ? 1 : 0);

            pMovie->HandleEvent(GFxSetFocusEvent(sks));
        }
        else
        {
            pMovie->HandleEvent(GFxEvent::KillFocus);
        }
    }
}

void FxPlayerApp::OnPause( bool paused)
{
    // Note that toggling play state through pMovie->SetPlayState would only pause root clip,
    // not children. So instead we just set the global variable above which prevents Advance.
    // However, we may still need to save/restore state in frame.

    pMovie->SetPause(paused);

    if (paused)
        PausedState = pMovie->GetPlayState();
    else
        pMovie->SetPlayState(PausedState);
}

void FxPlayerApp::UnloadMovie()
{
    if (FontConfigIndex >= 0)
        Loader.SetTranslator(NULL);
    if (pMovie && pMovie->GetMeshCacheManager())
        pMovie->GetMeshCacheManager()->ClearCache();
    pMovie = 0;
    if (pHudDataProvider)
        pHudDataProvider->SetMovie(0);
    if (pHud)
        pHud->Update();
    AmpDirty = true;
}