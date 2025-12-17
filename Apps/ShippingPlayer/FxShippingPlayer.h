/**************************************************************************

Filename    :   FxShippingPlayer.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxPlayerMobile_H
#define INC_FxPlayerMobile_H

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

#ifndef SF_BUILD_SHIPPING
#define FXSMP_ENABLE_HUD
#endif

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Platform;
using namespace Render;
using namespace GFx;
using GFx::Event;

class FxShippingPlayer : public App<FxRenderThread>, public ExternalInterface
{
public:
    typedef App<FxRenderThread> BaseClass;

    static FxShippingPlayer*    pApp;

    SF_MEMORY_REDEFINE_NEW_IMPL(App<FxRenderThread>, SF_MEMORY_CHECK_DELETE_NONE, Stat_Default_Mem)

        Loader                  mLoader;
    MovieInfo               mMovieInfo;
    Ptr<MovieDef>           pMovieDef;
    Ptr<Movie>              pMovie;

#ifdef FXSMP_ENABLE_HUD
    Ptr<Movie>              pHudMovie;

    Platform::Device::Window* pHudWindow;

    enum {
        Stat_FPS,
        Stat_DP,
        Stat_Mesh,
        Stat_Memory,
        Stat_Tri,
        Stat_Masks,
        Stat_Filters,
        Stat_Advance,
        Stat_Display,
        Stat_Other
    };
    enum {
        Button_None,
        Button_Next,
        Button_Prev,
        Button_Profile,
        Button_Pause,
        Button_Restart,
        Button_LockOrient,
        Button_StageClip,
        Button_Display,
        Button_CurveTol,
        Button_EdgeAA,
        Button_Batch,
        Button_Overdraw,
        Button_Wireframe,
    };

    int                     HudMode;
    Value                   Panel, FilenameText, HiddenFpsText, CurveText;
    Value                   StatText[10];
    Value                   Buttons[15], ButtonIcons[15];
    int                     ButtonFunctions[15], ButtonIndex[16];
    Value                   Sliders[3], SliderValues[3];

    void SetButton(int i, int f)
    {
        ButtonFunctions[i] = f;
        ButtonIndex[f] = i;
    }
#endif

    unsigned                Width, Height;
    bool                    NeedsOrientation;
    int                     Orientation, OrientationFlip;
    int                     ViewWidth, ViewHeight;
    PointF                  Move, MoveStart, ZoomStart;
    float                   Zoom, ZoomCur;
    UInt64                  MovieLastTime;
    String                  FileName;
    bool                    Paused;
    bool                    EdgeAA, Wireframe;
    bool                    LockOrientation, StageClipping;
    bool                    MeasurePerformance, FastForward;
    Render::ProfilerModes   ProfileMode;
    int                     CurveTolerancePosition;
    bool                    ButtonDown, TouchDown;
    GFx::Viewport           mViewport;
	Render::ToleranceParams tolParams;

    struct Window
    {
        Device::Window*     pWindow;
        GFx::Viewport       BaseViewport;
        Matrix2F            MouseMatrix;

        void                UpdateViewport(const Platform::ViewConfig& vc);
    };
    Window                  Windows[2];
    int                     HudWindow, ContentWindow;

    UInt64                  LastLoggedFps;
    int                     FrameCount;
    UInt64                  DisplayTime, AdvanceTime;
    bool                    HudVisible;
    bool                    HudUpdate;

    Directory               ContentDir;

    int                     UseStereo;
    Render::StereoParams    StereoParam, StereoParam0, StereoParamM;

#if defined(GFX_ENABLE_SOUND)
    float                   SoundVolume;
    bool                    IsMute;
    FxSoundFMOD*            pSoundSystem;
#endif

    float                   ExitTimeout;

    FxShippingPlayer();

    virtual bool OnInit(ViewConfig& config);
    virtual void OnShutdown();

    virtual void OnUpdateFrame(bool needRepaint);

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

    virtual void OnTouchBegin(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);
    virtual void OnTouchEnd(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);
    virtual void OnTouchMove(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary);

    virtual void            OnGestureBegin(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
        const PointF& translationDelta,
        const PointF& scaleDelta,
        float rotationDelta);
    virtual void            OnGesture(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
        const PointF& translationDelta,
        const PointF& scaleDelta,
        float rotationDelta);
    virtual void            OnGestureEnd(unsigned window, UInt32 gestureMask, const Point<int>& pos);

    virtual bool OnOrientation(unsigned /*orientation*/, bool force = 0);
    virtual void OnSize(const Size<unsigned>& size);
    virtual void OnConfigurationChange(const ViewConfig& newConfig);

    virtual void OnDropFiles(const String& filename);

	virtual void OnStatus(String* code, String* level, String* extensionId, String* contextId);

    virtual void OnAccelerometerUpdate(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ);

	virtual void OnGeolocationUpdate(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp);

    virtual bool PrepareMovie(MovieDef* pdef, Movie* pview);

    virtual void InitArgDescriptions(Args* args);
    virtual bool OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult);
    virtual void ResetInputFocus(unsigned controllerIdx);
    virtual void NotifyShowVirtualKeyboard(const Render::RectF& keyboardRect, const Render::RectF& textBox);
    virtual void NotifyHideVirtualKeyboard();

    virtual void OnFocus(bool setFocus, KeyModifiers mods);
    virtual void OnPause();
    virtual void OnResume();
    // Returns current orientation
    virtual unsigned GetOrientation() { return Orientation; }

    virtual bool IsOrientationLocked() { return LockOrientation; }

    //void Display(MovieDisplayHandle& movie, bool setTime = false);

    // for HUD
    void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount);

#ifdef FXSMP_ENABLE_HUD
    void SetTextInt(Value& TF, unsigned n);
    void SetTextFloat(Value& TF, float n);

    void AdvanceAndDisplayHud(UInt64 time);
    void SetHudPanelMode();
#endif

    SizeF GetMovieScaleSize();
    void UpdateViewSize();
    void FsCommand (Movie* pmovie, const char* pcommand, const char* parg);
    void ResetViewport();
    PointF AdjustInputPoint(int x, int y);

#if defined(GFX_ENABLE_SOUND)
    void InitializeSound();
    void ShutdownSound();
    Sound::SoundRenderer* GetSoundRenderer();
#endif
};

#endif
