/**************************************************************************

Filename    :   WiiUDemo.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_WiiUDemo_H
#define INC_WiiUDemo_H

// GFx Includes
#include "GFx.h"
#include "GFx_Kernel.h"
#include "GFx_Render.h"
#include "GFx/GFx_ImageCreator.h"
#include "Render/Renderer2D.h"

#include "Platform/Platform.h"
#include "FxRenderThread.h"

#if defined(GFX_ENABLE_SOUND)
#if defined(GFX_SOUND_FMOD)
#include "FxSoundFMOD.h"
#else
#undef GFX_ENABLE_SOUND
#endif
#endif

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Platform;
using namespace Render;
using namespace GFx;
using GFx::Event;

#include "World.h"
#include "Battle.h"

#include "FxHUD.h"

class WiiUDemo : public App<FxRenderThread>, public ExternalInterface, public FxHUDInfoProvider
{
public:
    typedef App<FxRenderThread> BaseClass;

    static WiiUDemo*         pApp;

    SF_MEMORY_REDEFINE_NEW_IMPL(App<FxRenderThread>, SF_MEMORY_CHECK_DELETE_NONE, Stat_Default_Mem)

    Loader                  mLoader;
    MovieInfo               mMovieInfo;

    Platform::Device::Window* pDrcWindow;

    unsigned                Width, Height;
    int                     ViewWidth, ViewHeight;
    UInt32                  MovieLastTime;
    bool                    Paused;
    bool                    EdgeAA, Wireframe;
    bool                    MeasurePerformance, FastForward;
    FxRenderThread::ProfileMode ProfileMode;
    int                     CurveTolerance;
    bool                    ButtonDown, TouchDown;
    GFx::Viewport           mViewport;

    struct Window
    {
        Device::Window*     pWindow;
        GFx::Viewport       mViewport;
        Ptr<MovieDef>       pMovieDef;
        Ptr<Movie>          pMovie;

        void                UpdateViewport(const Platform::ViewConfig& vc);
    };
    Window                  Windows[2];

    UInt64                  LastLoggedFps;
    int                     FrameCount;
    UInt64                  DisplayTime, AdvanceTime;

#if defined(GFX_ENABLE_SOUND)
    float                   SoundVolume;
    bool                    IsMute;
    FxSoundFMOD*            pSoundSystem;
#endif

    float                   ExitTimeout;

    WiiUDemo();

    virtual bool OnInit(ViewConfig& config);
    virtual void OnShutdown();

    virtual void OnUpdateFrame(bool needRepaint);

    bool LoadMovie(int window, const char* filename);

    // Input
    virtual void OnKey(unsigned controllerIndex, KeyCode keyCode,
        unsigned wcharCode, bool downFlag, KeyModifiers mods);
    virtual void OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag);
    virtual void OnPadStick(unsigned controllerIndex, PadKeyCode padCode, float xpos, float ypos);

    virtual void OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
        const Point<int>& mousePos, KeyModifiers mods);    
    virtual void OnMouseMove(unsigned mouseIndex,
        const Point<int>& mousePos, KeyModifiers mods);
    virtual void OnMouseWheel(unsigned mouseIndex, float zdelta,
        const Point<int>& mousePos, KeyModifiers mods);

    virtual void InitArgDescriptions(Args* args);
    virtual bool OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult);

    void FsCommand (Movie* pmovie, const char* pcommand, const char* parg);
    void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount);

#if defined(GFX_ENABLE_SOUND)
    void InitializeSound();
    void ShutdownSound();
    Sound::SoundRenderer* GetSoundRenderer();
#endif


    World           CassienWorld;
    Ptr<Battle>     CassienBattle;

    Value        TvScreenMC, DrcScreenMC, LocationView, BattleUI, BattleScreen;


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

    bool                    RedirectEventsToHud;
    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;
    

     // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
  
    void                    UpdateHUDViewport();
};

#endif
