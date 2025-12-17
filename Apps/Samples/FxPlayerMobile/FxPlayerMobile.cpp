/**************************************************************************

Filename    :   FxPlayerMobile.cpp
Content     :
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxPlayerMobile.h"

// Scaleform Includes
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_Std.h"

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Render.h"
#include "Render/Render_ImageFiles.h"
#include "Render/Render_TessGen.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/PVR_ImageFile.h" // PVR, ETC.
#include "Render/ImageFiles/TGA_ImageFile.h"
#if defined (SF_OS_ANDROID)
  #include "Render/ImageFiles/KTX_ImageFile.h" // ETC
  #include "Render/ImageFiles/DDS_ImageFile.h" // ATC
#endif
#if defined (SF_OS_WIIU)
#include "Render/ImageFiles/GTX_ImageFile.h"
#endif
#if defined (SF_OS_PSVITA)
#include "Render/ImageFiles/GXT_ImageFile.h"
#endif

#include "GFx/AS3/AS3_Global.h"

#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
#include "FxSharedObjectManager.h"
#endif

#if defined (SF_OS_ANDROID) && !defined (SF_ANDROID_NDK_BUILD)
extern "C" {
      extern void *__dso_handle __attribute__((__visibility__ ("hidden")));
        void *__dso_handle;
}
#endif

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;
using GFx::Event;

#include "../Bin/Data/AS3/FxPlayer/fxplayermini.swf.h"

// SWF Filename we will load by default.
#define FXPLAYER_FILENAME   "flash.swf"


SF_PLATFORM_SYSTEM_APP(FxPlayerMobile, Scaleform::GFx::System, FxPlayerMobile);


FxPlayerMobile* FxPlayerMobile::pApp = 0;

FxPlayerMobile::FxPlayerMobile()
{
    FileName = GetDefaultFilePath();
    FileName += FXPLAYER_FILENAME;
    pApp = this;
#if defined(GFX_ENABLE_SOUND)
    pSoundSystem = 0;
#endif
    ExitTimeout = 0;
}

// File opener class.
class FxPlayerFileOpener : public FileOpener
{
public:
    virtual File* OpenFile(const char *purl, int flags, int mode)
    {
        if (!strcmp(purl, "  fxplayermini.swf"))
            return new MemoryFile(purl, fxplayermini_swf, sizeof(fxplayermini_swf));

        // Buffered file wrapper is faster to use because it optimizes seeks.
        return FileOpener::OpenFile(purl, flags, mode);
    }
};

class FxPlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg)
    {
        if (FxPlayerMobile::pApp)
            FxPlayerMobile::pApp->FsCommand(pmovie, pcommand, parg);
    }
};

#ifdef SF_USE_ANE
class FxPlayerExtensionContextInterface : public ExtensionContextInterface
{
public:
    FxPlayerExtensionContextInterface(FxPlayerMobile* papp) : pApp(papp) 
    {
#if defined(SF_OS_IPHONE)
        ArrayLH<Extension*> *freExtensionArray = SF_NEW ArrayLH<Extension*>();
        
        /* Native Extensions init. */
        pApp->GetAppImpl()->InitNativeExtensionManager(freExtensionArray);
#endif
    }

    bool Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
    {
        return pApp->GetAppImpl()->Call(extensionID, contextID, functionName, argc, argv, result);
    }

    const char* GetExtensionDirectory(const char* extensionID)                                    { return pApp->GetAppImpl()->GetExtensionDirectory(extensionID); }
    void FinalizeExtensionContext(const char* extensionID, const char* contextID)                { pApp->GetAppImpl()->FinalizeExtensionContext(extensionID, contextID); }
    void InitializeExtensionContext(const char* extensionID, const char* contextID)                { pApp->GetAppImpl()->InitializeExtensionContext(extensionID, contextID); }
    GFx::Value* GetActionScriptData(const char* extensionID, const char* contextID)                { return pApp->GetAppImpl()->GetActionScriptData(extensionID, contextID); }
    void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)    { pApp->GetAppImpl()->SetActionScriptData(extensionID, contextID, data); }
    void SetMovie(Movie* pmovie)                                                                { pApp->GetAppImpl()->SetMovie(pmovie);    }

protected:
    FxPlayerMobile*    pApp;
};
#endif

class FxPlayerMultitouchInterface : public MultitouchInterface
{
public:
    virtual unsigned GetMaxTouchPoints() const { return 5; }

    virtual UInt32   GetSupportedGesturesMask() const { return MTG_Pan | MTG_Zoom | MTG_Rotate | MTG_Swipe; }

    virtual bool     SetMultitouchInputMode(MultitouchInputMode) { return true; }
};

class FxPlayerVirtualKeyboardInterface : public VirtualKeyboardInterface
{
public:
    // Invoked when an input textfield recieves focus
    virtual void    OnInputTextfieldFocusIn(bool multiline, const Render::RectF& textBox)
    {
        if (FxPlayerMobile::pApp)
        {
            AppImplBase* pimpl = FxPlayerMobile::pApp->GetAppImpl();
            pimpl->HandleVirtualKeyboardOpen(multiline, textBox);
        }
    }

    // Invoked when an input textfield loses focus
    virtual void    OnInputTextfieldFocusOut()
    {
        if (FxPlayerMobile::pApp)
        {
            AppImplBase* pimpl = FxPlayerMobile::pApp->GetAppImpl();
            pimpl->HandleVirtualKeyboardClose();
        }
    }
};

// Implementation of Accelerometer interface
class FxPlayerAccelerometerInterface : public AccelerometerInterface
{
public:
    FxPlayerAccelerometerInterface(FxPlayerMobile* papp) : pApp(papp) {}

    bool    RegisterAccelerometer(int accelerometerId)        { return pApp->GetAppImpl()->RegisterAccelerometer(accelerometerId); }
    bool    UnregisterAccelerometer(int accelerometerId)    { return pApp->GetAppImpl()->UnregisterAccelerometer(accelerometerId); }
    bool    IsAccelerometerMuted() const                     { return pApp->GetAppImpl()->IsAccelerometerMuted(); }
    bool    IsAccelerometerSupported() const     { return pApp->GetAppImpl()->IsAccelerometerSupported(); };
    void    SetAccelerometerInterval(int accelerometerId, int interval)       { pApp->GetAppImpl()->SetAccelerometerInterval(accelerometerId, interval); }

protected:
    FxPlayerMobile*    pApp;
};

// Implementation of Geolocation interface
class FxPlayerGeolocationInterface : public GeolocationInterface
{
public:
    FxPlayerGeolocationInterface(FxPlayerMobile* papp) : pApp(papp) {}

    bool    RegisterGeolocation(int geolocationId)        { return pApp->GetAppImpl()->RegisterGeolocation(geolocationId); }
    bool    UnregisterGeolocation(int geolocationId)    { return pApp->GetAppImpl()->UnregisterGeolocation(geolocationId); }
    bool    IsGeolocationMuted() const                     { return pApp->GetAppImpl()->IsGeolocationMuted(); }
    bool    IsGeolocationSupported() const     { return pApp->GetAppImpl()->IsGeolocationSupported(); };
    void    SetGeolocationInterval(int geolocationId, int interval)       { pApp->GetAppImpl()->SetGeolocationInterval(geolocationId, interval); }

protected:
    FxPlayerMobile*    pApp;
};

// Default implementation of UrlNavigator
class FxPlayerUrlNavigator : public UrlNavigator
{
public:
    virtual void NavigateToUrl(const String& url)
    {
        if (FxPlayerMobile::pApp)
        {
            AppImplBase* pimpl = FxPlayerMobile::pApp->GetAppImpl();
            pimpl->ProcessUrl(url);
        }
    }
};

static const float CurveTolerances[] = {1.0f, 5.0f, 10.0f, 50.0f, 250.0f};

void FxPlayerMobile::FsCommand(Movie* pmovie, const char* pcommand, const char* parg)
{
    SF_UNUSED3(pmovie, pcommand, parg);
}

bool FxPlayerMobile::LoadNextMovie(bool prev)
{
    String nextFile;
    if (ContentDir.GetNextFile(&nextFile, prev))
    {
        OnDropFiles(nextFile);
        return 1;
    }
    else
        return 0;
}

bool FxPlayerMobile::PrepareMovie(MovieDef* pmoviedef, Movie* pview)
{
    SF_UNUSED(pmoviedef);

    pview->SetMultitouchInterface(Ptr<MultitouchInterface>(*new FxPlayerMultitouchInterface()));

    pview->SetVirtualKeyboardInterface(Ptr<VirtualKeyboardInterface>(*new FxPlayerVirtualKeyboardInterface()));

    pview->SetAccelerometerInterface(Ptr<AccelerometerInterface>(*new FxPlayerAccelerometerInterface(this)));

    pview->SetGeolocationInterface(Ptr<GeolocationInterface>(*new FxPlayerGeolocationInterface(this)));

    return 1;
}

void FxPlayerMobile::InitArgDescriptions(Args* args)
{
    BaseClass::InitArgDescriptions(args);
    ArgDesc options []=
    {
        //      {"","--------------spacer example------------------\n","",FxCmdOption::Spacer,""},
        {"",    "FileName",  Args::StringOption | Args::Positional, NULL, "GFX or SWF file to load at startup"},
        {"t",   "ExitTimeout",         Args::FloatOption, "0.0",
        "<sec>    Timeout and exit after the specified number of seconds."},
        {"","",Args::ArgEnd,"",""},
    };
    args->AddDesriptions(options);
}

bool FxPlayerMobile::OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult)
{
    SF_UNUSED(parseResult);

    if (args.HasValue("FileName"))
    {
        FileName = args.GetString("FileName");
    }
    ExitTimeout =     args.GetFloat("ExitTimeout");
    return true;
}

void FxPlayerMobile::OnDropFiles(const String& NextFile)
{
    Ptr<MovieDef> pNewMovieDef;
    Ptr<Movie> pNewMovie;
    MovieInfo NewMovieInfo;
    // Unload current movie. If it fails we will see blank screen.
    pMovieDef = 0;
    pMovie = 0;
#ifdef SF_USE_ANE
    mLoader.GetExtensionContextInterface()->SetMovie(0);
#endif

    if (!mLoader.GetMovieInfo(NextFile, &NewMovieInfo))
    {
    error:
        SF_DEBUG_MESSAGE1(1, "Failed to load movie %s\n", NextFile.ToCStr());
        //      pHudMovie->Invoke("_root.HideLoadAnimation", NULL, NULL, 0);
        return;
    }
    if (!(pNewMovieDef = *mLoader.CreateMovie(NextFile, Loader::LoadAll|Loader::LoadWaitFrame1)))
        goto error;
    if (!(pNewMovie = *pNewMovieDef->CreateInstance(false, 0, NULL, pRenderThread)))
        goto error;

    // Tell the directory implementation of the current filename.
    ContentDir.SetCurrentFile(NextFile);

    SF_DEBUG_MESSAGE1(1, "Loaded movie %s\n", NextFile.ToCStr());

    // implicitly disable action error reporting
    Ptr<ActionControl> pactionControl = *new ActionControl();
    pactionControl->SetActionErrorSuppress(true);
    pNewMovie->SetActionControl(pactionControl);

    if (!PrepareMovie(pNewMovieDef, pNewMovie))
        goto error;

    FileName = NextFile;
    pMovieDef = pNewMovieDef;
    pMovie = pNewMovie;
    mMovieInfo = NewMovieInfo;

#ifdef SF_USE_ANE
    mLoader.GetExtensionContextInterface()->SetMovie(pMovie);
#endif

    // init the first frame
    ResetViewport();
    pNewMovie->SetViewport(mViewport);

    pNewMovie->Advance(0, 0);
    pNewMovie->SetMouseCursorCount(1);
    pNewMovie->HandleEvent(GFx::Event::SetFocus);

    pNewMovie->SetEdgeAAMode(EdgeAA ? Render::EdgeAA_On : Render::EdgeAA_Disable);

    pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle(), Platform::RenderThread::DHCAT_Normal,
                                    true, 0, Windows[ContentWindow].pWindow);

    MovieLastTime = Timer::GetTicks()/1000;
    SetFrameTime(0.001f);

    if (pMovie != NULL && pMovie->IsValid())
    {
        const char *p = FileName.ToCStr();
        const char *pname = p;
        while (*p)
        {
            if (*p == '/' || *p == '\\')
                pname = p+1;
            p++;
        }
        FilenameText.SetText(pname);
    }
    return;
}

void FxPlayerMobile::SetHudPanelMode()
{
    if (pMovie != NULL && !pMovie->IsValid())
    {
        return;
    }

    if (ContentWindow != HudWindow)
    {
        if (HudMode != 1)
            pHudMovie->Invoke("_root.showFullScreen", 0, 0, 0);
        HudVisible = true;
    }
    else if (Windows[HudWindow].BaseViewport.Height > Windows[HudWindow].BaseViewport.Width)
    {
        if (HudMode != 2)
            pHudMovie->Invoke("_root.showPortraitPanelScreen", 0, 0, 0);
        if (HudVisible)
            pHudMovie->Invoke("_root.forceOpen", 0, 0, 0);
    }
    else
    {
        if (HudMode != 0)
            pHudMovie->Invoke("_root.showPanelScreen", 0, 0, 0);
        if (HudVisible)
            pHudMovie->Invoke("_root.forceOpen", 0, 0, 0);
    }
}

void FxPlayerMobile::SetTextInt(Value& TF, unsigned n)
{
    char buf[64];
    Format(buf, "{0}", n);
    TF.SetText(buf);
}

void FxPlayerMobile::SetTextFloat(Value& TF, float n)
{
    char buf[64];
    Format(buf, "{0:.2}", n);
    TF.SetText(buf);
}

static const char* StatLabels[] = {"FPS", "DP", "Mesh", "Memory", "Triangles", "Masks", "Filters", "Advance", "Display", "Other"};

static const char* ButtonFrames[] = {0, "next", "previous", "normal", "pause", "reset", "lockscreen", "stageclip", "display", "curve",
                                     "edgeaa", "batch", "overdraw", "wireframe"};

static const char* ButtonLabels[] = {0, "Next", "Previous", "Profile", "Pause", "Restart", "Orientation", "Stage Clip", "Display", "Tolerance",
                                     "Edge AA", "Batches", "Overdraw", "Wireframe"};

// HUD Handler
void FxPlayerMobile::Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount)
{
    if (pMovie != NULL && !pMovie->IsValid())
    {
        return;
    }
    
    if (pmovieView == pHudMovie)
    {
        if (argCount > 0)
            SF_DEBUG_MESSAGE2(1, "%s: %s\n", methodName, args[0].ToString().ToCStr());
        else
            SF_DEBUG_MESSAGE1(1, "%s\n", methodName);

        if (!strcmp(methodName, "controlButtonClick"))
        {
            int button = args[0].GetInt();
            switch (ButtonFunctions[button])
            {
            case Button_Next:           LoadNextMovie(0);     break;
            case Button_Prev:           LoadNextMovie(1);     break;
            case Button_Restart:        if (pMovie) pMovie->Restart();    break;

            case Button_Pause:
                Paused = !Paused;
                if (pMovie)
                    pMovie->SetPause(Paused);
                Buttons[button].SetMember("toggled", Value(Paused));
                break;

            case Button_Batch:
                ProfileMode = (ProfileMode == Render::Profile_Batch
                    ? Render::Profile_None : Render::Profile_Batch);
                Buttons[ButtonIndex[Button_Overdraw]].SetMember("toggled", false);
                Buttons[button].SetMember("toggled", (ProfileMode == Render::Profile_Batch));
                pRenderThread->SetProfileMode(ProfileMode);
                break;
            case Button_Overdraw:
                ProfileMode = (ProfileMode == Render::Profile_Overdraw
                    ? Render::Profile_None : Render::Profile_Overdraw);
                Buttons[ButtonIndex[Button_Batch]].SetMember("toggled", false);
                Buttons[button].SetMember("toggled", (ProfileMode == Render::Profile_Overdraw));
                pRenderThread->SetProfileMode(ProfileMode);
                break;

            case Button_StageClip:
                StageClipping = !StageClipping;
                Buttons[button].SetMember("toggled", StageClipping);
                ResetViewport();
                break;

            case Button_EdgeAA:
                EdgeAA = !EdgeAA;
                Buttons[button].SetMember("toggled", EdgeAA);
                if (pMovie)
                    pMovie->SetEdgeAAMode(EdgeAA ? Render::EdgeAA_On : Render::EdgeAA_Disable);
                break;

            case Button_CurveTol:
                ButtonIcons[ButtonIndex[Button_CurveTol]].GetMember("curve", &CurveText);

                CurveTolerancePosition++;
                if (CurveTolerancePosition >= 5)
                    CurveTolerancePosition = 0;
                    
                pRenderThread->GetToleranceParams(&tolParams);
                tolParams.CurveTolerance = CurveTolerances[CurveTolerancePosition];
                pRenderThread->SetToleranceParams(tolParams);

                char buf[64];
                Format(buf, "{0}", CurveTolerancePosition + 1);
                CurveText.SetText(buf);
                break;

            case Button_LockOrient:
                LockOrientation = !LockOrientation;
                Buttons[button].SetMember("toggled", Value(LockOrientation));
                SetOrientationMode(!LockOrientation);
                break;
            case Button_Wireframe:
                Wireframe = !Wireframe;
                Buttons[button].SetMember("toggled", Wireframe);
                pRenderThread->SetWireframe(Wireframe);
                break;

            case Button_Profile:
                {
                    if (MeasurePerformance)
                    {
                        if (FastForward)
                        {
                            FastForward = 0;
                            MeasurePerformance = 0;

                            ButtonIcons[button].GotoAndStop("normal");
                        }
                        else
                        {
                            FastForward = 1;
                            ButtonIcons[button].GotoAndStop("fastforward");
                        }
                    }
                    else
                    {
                        MeasurePerformance = 1;
                        ButtonIcons[button].GotoAndStop("profile");
                    }

                    Value::DisplayInfo disp;
                    disp.SetVisible(MeasurePerformance && !HudVisible);
                    HiddenFpsText.SetDisplayInfo(disp);

                    HudUpdate = 1;
                }
                break;

            case Button_Display:
                if (pHudWindow)
                {
                    ContentWindow = !ContentWindow;
                    ResetViewport();

                    if (pMovie)
                    {
                        pRenderThread->RemoveDisplayHandle(pMovie->GetDisplayHandle());
                        pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle(), Platform::RenderThread::DHCAT_Normal,
                                                        true, 0, Windows[ContentWindow].pWindow);
                    }

                    SetHudPanelMode();
                }
                break;
            }
        }
        else if (!strcmp(methodName, "hideClick"))
        {
            HudVisible = 0;
            Value::DisplayInfo disp;
            disp.SetVisible(MeasurePerformance);
            HiddenFpsText.SetDisplayInfo(disp);
        }
        else if (!strcmp(methodName, "restoreClick"))
        {
            HudVisible = 1;
            Value::DisplayInfo disp;
            disp.SetVisible(false);
            HiddenFpsText.SetDisplayInfo(disp);
        }
        else if (!strcmp(methodName, "controlSliderChange"))
        {
            int slider = args[0].GetInt();
            float val = (float)args[1].GetNumber();

            switch (slider)
            {
            case 0:
                StereoParam.DisplayDiagInches = StereoParam0.DisplayDiagInches + val * StereoParamM.DisplayDiagInches;
                SetTextFloat(SliderValues[0], StereoParam.DisplayDiagInches);
                break;
            case 1:
                StereoParam.Distortion = StereoParam0.Distortion + val * StereoParamM.Distortion;
                SetTextFloat(SliderValues[1], StereoParam.Distortion);
                break;
            case 2:
                StereoParam.EyeSeparationCm = StereoParam0.EyeSeparationCm + val * StereoParamM.EyeSeparationCm;
                SetTextFloat(SliderValues[2], StereoParam.EyeSeparationCm);
                break;
            }
            pRenderThread->SetStereoParams(StereoParam);
        }
        else if (!strcmp(methodName, "register"))
        {
            HudMode = args[0].GetInt();
            Panel = args[1];
            HiddenFpsText = args[2];
            FilenameText = args[3];

            //Hide the FPS textfield so it doesn't interfere mouse events
            Value::DisplayInfo disp;
            disp.SetVisible(false);
            HiddenFpsText.SetDisplayInfo(disp);

            Value(args[4]).SetText(GFX_VERSION_STRING);
            for (int i = 0; i < 10; i++)
            {
                Value label;
                args[5+i].GetMember("label", &label);
                args[5+i].GetMember("value", &StatText[i]);
                label.SetText(StatLabels[i]);
            }

            memset(ButtonFunctions, 0, sizeof(ButtonFunctions));
            for (int i = 0; i < 16; i++)
                ButtonIndex[i] = -1;

            if (HudMode == 1) // Full screen view
            {
                SetButton(0, Button_Prev);
                SetButton(1, Button_Restart);
                SetButton(2, Button_Pause);
                SetButton(3, Button_Next);

                SetButton(4, Button_Profile);
                if (GetCaps() & Platform::Cap_LockOrientation)
                    SetButton(5, Button_LockOrient);
                SetButton(6, Button_Display);
                SetButton(7, Button_StageClip);

                SetButton(8, Button_Batch);
                SetButton(9, Button_Overdraw);
                SetButton(10, Button_EdgeAA);
                
                SetButton(11, Button_CurveTol);
            }
            else // Dashboard view
            {
                SetButton(0, Button_Profile);
                SetButton(1, Button_Pause);

                // General tab
                SetButton(2, Button_Prev);
                SetButton(3, Button_Restart);
                SetButton(4, Button_Next);

                if (GetCaps() & Platform::Cap_LockOrientation)
                    SetButton(5, Button_LockOrient);
                SetButton(6, Button_StageClip);

                if (pHudWindow)
                    SetButton(7, Button_Display);

                // Vis tab
                SetButton(8, Button_Batch);
                SetButton(9, Button_Overdraw);
                if (pDevice->GetCaps() & Platform::DeviceCap_Wireframe)
                    SetButton(10, Button_Wireframe);

                SetButton(11, Button_EdgeAA);
                SetButton(12, Button_CurveTol);
            }

            for (int i = 0; i < (HudMode == 1 ? 12 : 14); i++)
            {
                Buttons[i] = args[15+i];
                args[15+i].GetMember("icon", &ButtonIcons[i]);

                if (ButtonFunctions[i] != Button_None)
                {
                    ButtonIcons[i].GotoAndStop(ButtonFrames[ButtonFunctions[i]]);
                    Value label;
                    Buttons[i].GetMember("label", &label);
                    label.SetText(ButtonLabels[ButtonFunctions[i]]);
                }
                else
                {
                    Value::DisplayInfo disp;
                    disp.SetVisible(false);
                    Buttons[i].SetDisplayInfo(disp);
                }
            }

            Buttons[ButtonIndex[Button_EdgeAA]].SetMember("toggled", EdgeAA);
            Buttons[ButtonIndex[Button_Pause]].SetMember("toggled", Paused);
            if (ButtonIndex[Button_LockOrient] >= 0)
                Buttons[ButtonIndex[Button_LockOrient]].SetMember("toggled", LockOrientation);
            if (ButtonIndex[Button_Wireframe] >= 0)
                Buttons[ButtonIndex[Button_Pause]].SetMember("toggled", Wireframe);
            Buttons[ButtonIndex[Button_StageClip]].SetMember("toggled", StageClipping);

            Buttons[ButtonIndex[Button_Batch]].SetMember("toggled", (ProfileMode == Render::Profile_Batch));
            Buttons[ButtonIndex[Button_Overdraw]].SetMember("toggled", (ProfileMode == Render::Profile_Overdraw));

            if (FastForward)
                ButtonIcons[ButtonIndex[Button_Profile]].GotoAndStop("fastforward");
            else if (MeasurePerformance)
                ButtonIcons[ButtonIndex[Button_Profile]].GotoAndStop("profile");

            if (ButtonIndex[Button_CurveTol] >= 0)
            {
                Value tfCurve;
                ButtonIcons[ButtonIndex[Button_CurveTol]].GetMember("curve", &CurveText);

                char buf[64];
                Format(buf, "{0}", CurveTolerancePosition + 1);
                CurveText.SetText(buf);
            }

            Value label;
            // Tabs
            if (HudMode != 1)
            {
                args[32].GetMember("label", &label);
                label.SetText("General");
                args[33].GetMember("label", &label);
                label.SetText("Visualization");
            }

            if (pDevice->GetCaps() & Platform::DeviceCap_Stereo)
            {
                if (HudMode != 1)
                {
                    args[34].GetMember("label", &label);
                    label.SetText("Stereo");
                }

                for (int i = 0; i < 3; i++)
                {
                    Sliders[i] = args[29+i];
                    args[29+i].GetMember("value", &SliderValues[i]);
                }
                Value label;
                args[29].GetMember("label", &label);
                label.SetText("Display Size");
                args[30].GetMember("label", &label);
                label.SetText("Distortion");
                args[31].GetMember("label", &label);
                label.SetText("Eye Separation");

                Value val ((StereoParam.DisplayDiagInches - StereoParam0.DisplayDiagInches) / StereoParamM.DisplayDiagInches);
                Sliders[0].SetMember("position", val);
                SetTextFloat(SliderValues[0], StereoParam.DisplayDiagInches);
                val.SetNumber((StereoParam.Distortion - StereoParam0.Distortion) / StereoParamM.Distortion);
                Sliders[1].SetMember("position", val);
                SetTextFloat(SliderValues[1], StereoParam.Distortion);
                val.SetNumber((StereoParam.EyeSeparationCm - StereoParam0.EyeSeparationCm) / StereoParamM.EyeSeparationCm);
                Sliders[2].SetMember("position", val);
                SetTextFloat(SliderValues[2], StereoParam.EyeSeparationCm);
            }
            else
            {
                if (HudMode != 1)
                {
                    Value::DisplayInfo disp;
                    disp.SetVisible(false);
                    Value(args[34]).SetDisplayInfo(disp);
                }
                else
                {
                    Value::DisplayInfo disp;
                    disp.SetVisible(false);
                    Value(args[29]).SetDisplayInfo(disp);
                    Value(args[30]).SetDisplayInfo(disp);
                    Value(args[31]).SetDisplayInfo(disp);
                }
            }

            if (GetKeyboardCount())
            {
                Value::DisplayInfo disp;
                disp.SetVisible(false);
                Value(args[35]).SetDisplayInfo(disp);
            }
        }
        else if (!strcmp(methodName, "initialize"))
        {
            SetHudPanelMode();
        }
    }
}

bool FxPlayerMobile::OnInit(Platform::ViewConfig& config)
{
    Move.x = Move.y = 0;
    Zoom = ZoomCur = 1;
    ButtonDown = TouchDown = 0;
    Paused = 0;
    EdgeAA = 1;
    MeasurePerformance = 0;
    FastForward = 0;
    HudUpdate = 0;
    HudVisible = false;
    HudMode = 3;
    LockOrientation = 1;
    StageClipping = 0;
    Wireframe = 0;
    ProfileMode = Render::Profile_None;
    Width = config.ViewSize.Width;
    Height = config.ViewSize.Height;
    NeedsOrientation = config.HasFlag(View_UseOrientation);
    Orientation = config.Orientation;
    CurveTolerancePosition = 0;
    Windows[1].pWindow = 0;

    config.SetFlag(View_Stereo, 1);
    SetOrientationMode(!LockOrientation);

    // Enable progressive loading
#ifdef SF_ENABLE_THREADS
    Ptr<ThreadedTaskManager> pTaskManager = *new ThreadedTaskManager;
    mLoader.SetTaskManager(pTaskManager);
#endif

    Ptr<FileOpener> pfileOpener = *new FxPlayerFileOpener;
    mLoader.SetFileOpener(pfileOpener);

    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    mLoader.SetFSCommandHandler(pcommandHandler);

    mLoader.SetExternalInterface(this);

#ifdef SF_USE_ANE
    mLoader.SetExtensionContextInterface(Ptr<ExtensionContextInterface>(*new FxPlayerExtensionContextInterface(this)));
#endif

    Ptr<FxPlayerUrlNavigator> purlnav = *new FxPlayerUrlNavigator();
    mLoader.SetUrlNavigator(purlnav);

    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    mLoader.SetImageFileHandlerRegistry(pimgReg);

#if defined(GFX_ENABLE_SOUND)
    SoundVolume = 1.0;
    IsMute = false;
    InitializeSound();
    Sound::SoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        Ptr<Audio> paudioState = *new Audio(psoundRenderer);
        mLoader.SetAudio(paudioState);
        psoundRenderer->SetMasterVolume(SoundVolume);
        psoundRenderer->Mute(IsMute);
    }
#endif // GFX_ENABLE_SOUND

#ifdef GFX_AS2_SUPPORT
    Ptr<ASSupport> pAS2Support = *new AS2Support();
    mLoader.SetAS2Support(pAS2Support);
#endif
#ifdef GFX_AS3_SUPPORT
    Ptr<ASSupport> pAS3Support = *new AS3Support();
    mLoader.SetAS3Support(pAS3Support);
#endif
    //pRenderer2D->GetTolerances().StrokeLowerScale = 0.99f;
    //pRenderer2D->GetTolerances().StrokeUpperScale = 1.01f;

#if defined(GFX_AS2_ENABLE_SHAREDOBJECT)
    String savePath = GetSaveDataDirectory();
    Ptr<SharedObjectManagerBase> psharedobjs = *new FxSharedObjectManager(savePath);
    mLoader.SetSharedObjectManager(psharedobjs);
#endif  // GFX_AS2_ENABLE_SHAREDOBJECT

    OnInitHelper(config, "FxPlayer");
    ContentDir.Reread(GetContentDirectory(), "*.swf");

    Windows[0].pWindow = GetDeviceWindow();
    Windows[0].UpdateViewport(config);
    ContentWindow = 0;

    if (GetDisplayCount() == 2)
        pHudWindow = pDevice->CreateGraphicsWindow(1, Platform::ViewConfig());
    else
        pHudWindow = 0;

    if (pHudWindow)
    {
        Platform::ViewConfig hwconfig;
        pHudWindow->GetViewConfig(&hwconfig);

        Windows[1].pWindow = pHudWindow;
        Windows[1].UpdateViewport(hwconfig);
        ResetViewport();

        HudWindow = 1;
    }
    else
        HudWindow = 0;

    /*
    StereoParam.DisplayDiagInches = 10.73625f;
    StereoParam.Distortion = 0.4296f;
    StereoParam.EyeSeparationCm = 1.4f;
    */
    StereoParam.DisplayDiagInches = 15.5f;
    StereoParam.Distortion = 0.4f;
    StereoParam.EyeSeparationCm = 1.4f;

    StereoParam0.DisplayDiagInches = 1.5f;
    StereoParamM.DisplayDiagInches = 14.5f;
    StereoParam0.Distortion = 0;
    StereoParamM.Distortion = 1.2f;
    StereoParam0.EyeSeparationCm = 0;
    StereoParamM.EyeSeparationCm = 8;

    if (config.HasFlag(View_Stereo))
        UseStereo = config.StereoFormat;
    else
        UseStereo = Platform::Stereo_None;

    OnOrientation(Orientation, true);

    GlyphCacheParams fontCacheConfig;
#if defined(SF_OS_ANDROID) || defined(SF_OS_IPHONE)
    fontCacheConfig.TextureWidth   = 512;
    fontCacheConfig.TextureHeight  = 512;
    fontCacheConfig.NumTextures = 4;
#else
    fontCacheConfig.TextureWidth   = 1024;
    fontCacheConfig.TextureHeight  = 1024;
    fontCacheConfig.NumTextures = 1;
#endif
    //Temporarily changing value from 32 too 100 as Vector Fonts aren't supported yet
    //If the value is kept at 32, then large fonts will be blurry
    fontCacheConfig.MaxSlotHeight  = 100;
    //fontCacheConfig.SlotPadding    = 2;

    pRenderThread->SetGlyphCacheParams(fontCacheConfig);

    // Multithreaded loading with single threading rendering is not properly supported
    // because of RTCommandQueue. Disable creating textures on load.
    SF::Ptr<GFx::ImageCreator> pimageCreator =
        *new GFx::ImageCreator(
#if defined(SF_OS_3DS) || defined(SF_OS_WIIU) || defined(SF_OS_PSVITA) || defined(SF_OS_ANDROID)
        pRenderThread->GetTextureManager()
#else
        NULL
#endif
        );
    mLoader.SetImageCreator(pimageCreator);

    pRenderThread->SetStereoParams(StereoParam);

    Ptr<MovieDef> pHudDef = *mLoader.CreateMovie("  fxplayermini.swf",
        Loader::LoadAll/*|Loader::LoadKeepBindData*/|Loader::LoadWaitCompletion);

    if (pHudDef)
    {
        pHudMovie = *pHudDef->CreateInstance();

        if (pHudMovie && pHudMovie->IsValid())
        {
            pRenderThread->AddDisplayHandle(pHudMovie->GetDisplayHandle(), Platform::RenderThread::DHCAT_Overlay,
                                            true, 0, pHudWindow);

            pHudMovie->SetBackgroundAlpha(0);
            pHudMovie->SetViewport(GFx::Viewport(800,480,0,0,800,480));
            pHudMovie->Advance(0);
            pHudMovie->SetViewport(Windows[HudWindow].BaseViewport);
            pHudMovie->Advance(1);
            pHudMovie->SetMouseCursorCount(1);
        }
    }

    // Load the movie file and create its instance.
    if (mLoader.GetMovieInfo(FileName, &mMovieInfo))
    {
        pMovieDef = *mLoader.CreateMovie(FileName, Loader::LoadAll|Loader::LoadWaitFrame1);
        if (pMovieDef)
        {
            pMovie = *pMovieDef->CreateInstance(false, 0, NULL, pRenderThread);
            if (pMovie)
            {
#ifdef SF_USE_ANE
                mLoader.GetExtensionContextInterface()->SetMovie(pMovie);
#endif
                // implicitly disable action error reporting
                Ptr<ActionControl> pactionControl = *new ActionControl();
                pactionControl->SetActionErrorSuppress(true);
                pMovie->SetActionControl(pactionControl);

                PrepareMovie(pMovieDef, pMovie);

                // init the first frame
                ResetViewport();
                pMovie->SetViewport(mViewport);
                pMovie->Advance(0, 0);
                pMovie->HandleEvent(GFx::Event::SetFocus);
                pMovie->SetMouseCursorCount(1);
                
                // init EdgeAA and CurveTolerance
                pMovie->SetEdgeAAMode(EdgeAA ? Render::EdgeAA_On : Render::EdgeAA_Disable);
                pRenderThread->GetToleranceParams(&tolParams);
                tolParams.CurveTolerance = CurveTolerances[CurveTolerancePosition];
                pRenderThread->SetToleranceParams(tolParams);

                pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle());

                if (pMovie->IsValid())
                {
                    const char *p = FileName.ToCStr();
                    const char *pname = p;
                    while (*p)
                    {
                        if (*p == '/' || *p == '\\')
                            pname = p+1;
                        p++;
                    }
                    FilenameText.SetText(pname);

                    // Tell the directory implementation of the current filename.
#if !defined (SF_OS_IPHONE)
                    // Using FileName for iOS will break document loading via iTunes
                    ContentDir.SetCurrentFile(FileName);
#else
                    ContentDir.SetCurrentFile(pname);
#endif //SF_OS_IPHONE
                }
            }
        }
        else
            SF_DEBUG_WARNING1(1, "Failed to load %s", FileName.ToCStr());
    }
    else
        SF_DEBUG_WARNING1(1, "Failed to load %s", FileName.ToCStr());

    ResetViewport();

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    LastLoggedFps = MovieLastTime = Timer::GetTicks()/1000;
    FrameCount = -1;


    // pGestureManager = 0;        // uncomment to disable RECOGNIZE_GESTURES (use native gesture handlers instead)

    return 1;
}

void FxPlayerMobile::OnShutdown()
{
    if ((pHudMovie != NULL && pHudMovie->IsValid()) ||
        (pMovie != NULL && pMovie->IsValid()))
    {
        HiddenFpsText = Panel = FilenameText = CurveText = Value();
        for (int i = 0; i < 14; i++)
        {
            Buttons[i] = Value();
            ButtonIcons[i] = Value();
        }
        for (int i = 0; i < 10; i++)
            StatText[i] = Value();
        for (int i = 0; i < 3; i++)
        {
            Sliders[i] = Value();
            SliderValues[i] = Value();
        }
    }
    
#ifdef SF_USE_ANE
    mLoader.GetExtensionContextInterface()->SetMovie(0);
#endif

    pHudMovie = 0;
    pMovie = 0;
    pMovieDef = 0;

#if defined(GFX_ENABLE_SOUND)
    ShutdownSound();
#endif

    BaseClass::OnShutdown();
}

void FxPlayerMobile::UpdateViewSize()
{
    int width = Alg::Max(Windows[ContentWindow].BaseViewport.Width, 4);
    int height= Alg::Max(Windows[ContentWindow].BaseViewport.Height, 4);

    if (StageClipping)
    {
        // Determine movie size and location based on the aspect ratio
        float hw;
        if (mMovieInfo.Height != 0 && mMovieInfo.Width != 0)
            hw = (float) mMovieInfo.Height / (float) mMovieInfo.Width;
        else
            hw = (float) height / (float) width;
        if (width * hw > height)
        {
            // Use height as the basis for aspect ratio
            ViewWidth = (int)((float) height / hw);
            ViewHeight = height;
        }
        else
        {
            // Use width
            ViewWidth = width;
            ViewHeight = (int) (width * hw);
        }
    }
    else
    {
        ViewWidth = width;
        ViewHeight = height;
    }
}

SizeF FxPlayerMobile::GetMovieScaleSize()
{
    SizeF scale;
    scale.Width = (pMovie->GetVisibleFrameRect().Width() / ViewWidth);
    scale.Height = (pMovie->GetVisibleFrameRect().Height() / ViewHeight);
    return scale;
}

// Advances GFx animation and draws the scene.
void FxPlayerMobile::OnUpdateFrame(bool needDraw)
{
    SF_UNUSED(needDraw);

    UInt64  time    = Timer::GetTicks()/1000;

    // Check to see if exit timeout was hit.
    if ((ExitTimeout > 0.0f) && (time >= (ExitTimeout)))
    {
        Shutdown();
        return;
    }

    if (pMovie)
    {
        // Advance time and display the movie.
        if (!Paused)
        {
            float delta;

            if (FastForward)
                delta = (1000.0f / pMovieDef->GetFrameRate());
            else
                delta = ((float)(time - MovieLastTime)) / 1000.0f;

            UInt64 b = SF::Timer::GetProfileTicks();
            {
                pMovie->Advance(delta, 0);
            }
            AdvanceTime += (SF::Timer::GetProfileTicks() - b);
        }

        FrameCount++;
    }

    AdvanceAndDisplayHud(time);

    MovieLastTime = time;
    
    AppImplBase* pimpl = FxPlayerMobile::pApp->GetAppImpl();
    if (pimpl->IsDisplayActive())
    {
        pRenderThread->WaitForOutstandingDrawFrame();
        pRenderThread->DrawFrame();
    }
}

void FxPlayerMobile::AdvanceAndDisplayHud(UInt64 time)
{
    if (pHudMovie && pHudMovie->IsValid())
    {
        if ((time - LastLoggedFps > 1000) && FrameCount && ((HudVisible) || MeasurePerformance))
        {
            Render::HAL::Stats rstats;
            pRenderThread->GetRenderStats(&rstats);
            UInt64 displayTicks = pRenderThread->GetLastDisplayTicks();
            UInt64 otherTicks   = pRenderThread->GetLastDrawFrameTicks() - displayTicks;

            if (MeasurePerformance)
            {
                float delta = (time - LastLoggedFps) / 1000.0f;
                char buf[256];
                Format(buf, "{0:.0f}", int(delta > 0 ? (FrameCount / delta) : 0));
                if (HudVisible)
                    StatText[Stat_FPS].SetText(buf);
                else
                    HiddenFpsText.SetText(buf);
            }
            else
            {
                if (HudVisible)
                    StatText[Stat_FPS].SetText("");
                else
                    HiddenFpsText.SetText("");
            }
            SetTextFloat(StatText[Stat_Advance], (AdvanceTime / FrameCount) * 0.001f);
            SetTextFloat(StatText[Stat_Display], displayTicks * 0.001f);
            SetTextFloat(StatText[Stat_Other], otherTicks * 0.001f);

            FrameCount = 0;
            AdvanceTime = 0;

            SetTextInt(StatText[Stat_DP],      rstats.Primitives);
            SetTextInt(StatText[Stat_Mesh],    rstats.Meshes);
            SetTextInt(StatText[Stat_Tri],     rstats.Triangles);
            SetTextInt(StatText[Stat_Masks],   rstats.Masks);
            SetTextInt(StatText[Stat_Filters], rstats.Filters);

            MemoryHeap::RootStats stats;
            Memory::pGlobalHeap->GetRootStats(&stats);

            char buf[128];
            Format(buf, "{0}K", (stats.SysMemUsedSpace - stats.UserDebugFootprint - stats.DebugInfoFootprint + 1023) / 1024);
            StatText[Stat_Memory].SetText(buf);

            LastLoggedFps = time;
        }
    }

    if (pHudMovie)
    {
        pHudMovie->Advance(/*((float)(time - MovieLastTime)) / 1000.0f*/ 0.5f);
    }
}

void FxPlayerMobile::OnMouseButton(unsigned inputSource, unsigned button, bool downFlag, const Point<int> &pos, KeyModifiers modifiers)
{
    SF_UNUSED(modifiers);

    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;
    const Window& win = (pHudWindow ? Windows[windowIndex] : Windows[0]);

    PointF p = win.MouseMatrix.Transform(PointF((float)pos.x,(float)pos.y));

    if (pHudMovie && (int)windowIndex == HudWindow && (HudWindow != ContentWindow || ButtonDown || pHudMovie->HitTest(p.x, p.y, Movie::HitTest_ShapesNoInvisible)))
    {
        if (windowIndex == 0)
            ButtonDown = downFlag;

        MouseEvent event(downFlag ? GFx::Event::MouseDown : GFx::Event::MouseUp, button, p.x, p.y, 0.0f, mouseIndex);
        pHudMovie->HandleEvent(event);
    }
    else if (pMovie && (int)windowIndex == ContentWindow)
    {
        if (windowIndex == 0)
            ButtonDown = 0;

        MouseEvent event(downFlag ? GFx::Event::MouseDown : GFx::Event::MouseUp, button,
                            p.x - (win.BaseViewport.Width-ViewWidth)/2, p.y - (win.BaseViewport.Height-ViewHeight)/2, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }
}

void FxPlayerMobile::OnMouseMove(unsigned inputSource, const Point<int> &pos, KeyModifiers modifiers)
{
    SF_UNUSED(modifiers);

    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;
    const Window& win = (pHudWindow ? Windows[windowIndex] : Windows[0]);

    PointF p = win.MouseMatrix.Transform(PointF((float)pos.x,(float)pos.y));

    if (pHudMovie && (int)windowIndex == HudWindow && (HudWindow != ContentWindow || ButtonDown || pHudMovie->HitTest(p.x, p.y, Movie::HitTest_ShapesNoInvisible)))
    {
        MouseEvent event(GFx::Event::MouseMove, 0, p.x, p.y, 0.0f, mouseIndex);
        pHudMovie->HandleEvent(event);
    }
    else if (pMovie && (int)windowIndex == ContentWindow)
    {
        MouseEvent event(GFx::Event::MouseMove, 0,
                            p.x - (win.BaseViewport.Width-ViewWidth)/2, p.y - (win.BaseViewport.Height-ViewHeight)/2, 0.0f, mouseIndex);
        pMovie->HandleEvent(event);
    }
}


void FxPlayerMobile::OnMouseWheel(unsigned inputSource, float zdelta,
                                  const Point<int>& pos, KeyModifiers mods)
{
    SF_UNUSED(mods);

    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;
    const Matrix2F& MouseMatrix = (pHudWindow ? Windows[windowIndex] : Windows[0]).MouseMatrix;

    if ((int)windowIndex == ContentWindow)
    {
        PointF p = MouseMatrix.Transform(PointF((float)pos.x,(float)pos.y));

        MouseEvent event(GFx::Event::MouseWheel, 0, p.x, p.y, zdelta, mouseIndex);
        pMovie->HandleEvent(event);
    }
}

void FxPlayerMobile::OnKey(unsigned inputSource, KeyCode key, unsigned wcharCode, bool down, KeyModifiers modifiers)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned kbIndex = inputSource & Platform::InputController_Mask;

    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    if (!down)
        switch (key)
        {
        case SF::Key::Pause:
        case SF::Key::F15:
            {
                pHudMovie->Invoke(HudVisible ? "_root.forceClose" : "_root.forceOpen", 0, 0, 0);
                HudVisible = !HudVisible;
                Value::DisplayInfo disp;
                disp.SetVisible(!HudVisible);
                HiddenFpsText.SetDisplayInfo(disp);
                return;
            }

        default:
            break;
        }

    if ((int)windowIndex == ContentWindow && pMovie)
    {
        KeyEvent event(down ? GFx::Event::KeyDown : GFx::Event::KeyUp, key, (UByte)(wcharCode > 128 ? 0 : wcharCode), wcharCode, modifiers, (UInt8)kbIndex);
        pMovie->HandleEvent(event);
    }
}

void FxPlayerMobile::OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag)
{
    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    if (!pMovie)
        return;
    switch(keyCode)
    {
    case SF::Pad_Left:
        OnKey(controllerIdx, Key::Left, 0, downFlag, 0);
        return;
    case SF::Pad_Right:
        OnKey(controllerIdx, Key::Right, 0, downFlag, 0);
        return;
    case SF::Pad_Up:
        OnKey(controllerIdx, Key::Up, 0, downFlag, 0);
        return;
    case SF::Pad_Down:
        OnKey(controllerIdx, Key::Down, 0, downFlag, 0);
        return;

    case SF::Pad_L1:
        if (downFlag)
            LoadNextMovie(true);
        return;
    case SF::Pad_R1:
        if (downFlag)
            LoadNextMovie(false);
        return;

#ifdef SF_OS_PSVITA
    case SF::Pad_X:
        OnKey(controllerIdx, Key::Return, 0, downFlag, 0);
        return;
    case SF::Pad_O:
        OnKey(controllerIdx, Key::Escape, 0, downFlag, 0);
        return;
    case SF::Pad_T:
        OnKey(controllerIdx, Key::Space, 0, downFlag, 0);
        return;
#else
    case SF::Pad_A:
        OnKey(controllerIdx, Key::Return, 0, downFlag, 0);
        return;
    case SF::Pad_B:
        OnKey(controllerIdx, Key::Escape, 0, downFlag, 0);
        return;
    case SF::Pad_X:
        OnKey(controllerIdx, Key::Space, 0, downFlag, 0);
        return;
#endif

    case SF::Key::Home:
    case SF::Pad_Start:
    case SF::Pad_Select:
        if (downFlag)
        {
            pHudMovie->Invoke(HudVisible ? "_root.forceClose" : "_root.forceOpen", 0, 0, 0);
            HudVisible = !HudVisible;
            Value::DisplayInfo disp;
            disp.SetVisible(!HudVisible);
            HiddenFpsText.SetDisplayInfo(disp);
        }
        return;

    default:
        break;
    }
}

void FxPlayerMobile::OnPadStick(unsigned inputSource, PadKeyCode padCode, float xpos, float ypos)
{
    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned controllerIdx = inputSource & Platform::InputController_Mask;

    if ((int)windowIndex == ContentWindow)
    {
        GamePadAnalogEvent event(padCode, xpos, ypos, (UInt8)controllerIdx);
        if (pMovie)
            pMovie->HandleEvent(event);
    }
}

PointF FxPlayerMobile::AdjustInputPoint(int x, int y)
{
    PointF p = Windows[0].MouseMatrix.Transform(PointF((float)x,(float)y));
    return PointF(p.x - (Windows[0].BaseViewport.Width-ViewWidth)/2,
                  p.y - (Windows[0].BaseViewport.Height-ViewHeight)/2);
}

void FxPlayerMobile::OnTouchBegin(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
{
    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    TouchEvent event(GFx::Event::TouchBegin, id, p.x, p.y, (float)contact.x, (float)contact.y, primary);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) &&
        (TouchDown || pHudMovie->HitTest(p.x, p.y, Movie::HitTest_ShapesNoInvisible)))
    {
        pHudMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pHudMovie);
            pGestureManager->ProcessDown(id, pos, p);
        }
        TouchDown = true;
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
    {
        pMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pMovie);
            pGestureManager->ProcessDown(id, pos, p);
        }
    }
}
void FxPlayerMobile::OnTouchEnd(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
{
    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    TouchEvent event(GFx::Event::TouchEnd, id, p.x, p.y, (float)contact.x, (float)contact.y, primary);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) && TouchDown)
    {
        pHudMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pHudMovie);
            pGestureManager->ProcessUp(id, pos, p);
        }
        if (primary)
            TouchDown = false;
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
    {
        pMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pMovie);
            pGestureManager->ProcessUp(id, pos, p);
        }
    }
}
void FxPlayerMobile::OnTouchMove(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
{
    if ((pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    TouchEvent event(GFx::Event::TouchMove, id, p.x, p.y, (float)contact.x, (float)contact.y, primary);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) && TouchDown)
    {
        pHudMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pHudMovie);
            pGestureManager->ProcessMove(id, pos, p);
        }
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
    {
        pMovie->HandleEvent(event);
        if (pGestureManager)
        {
            pGestureManager->SetMovie(pMovie);
            pGestureManager->ProcessMove(id, pos, p);
        }
    }
}

void FxPlayerMobile::OnGestureBegin(unsigned window, UInt32 gestureMask, const Point<int>& pos,
                                    const PointF& translation,
                                    const PointF& scale,
                                    float rotation)
{
    if (pGestureManager || (pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    GestureEvent event(GFx::Event::GestureBegin, gestureMask, p.x, p.y,
                       translation.x, translation.y, scale.x, scale.y, rotation);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) &&
        (TouchDown || pHudMovie->HitTest(p.x, p.y, Movie::HitTest_ShapesNoInvisible)))
    {
        pHudMovie->HandleEvent(event);
        TouchDown = true;
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
        pMovie->HandleEvent(event);
}

void FxPlayerMobile::OnGesture(unsigned window, UInt32 gestureMask, const Point<int>& pos,
                                    const PointF& translation,
                                    const PointF& scale,
                                    float rotation)
{

    if (pGestureManager || (pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    GestureEvent event(gestureMask & GestureBit_Swipe ? GFx::Event::GestureSimple
                       : GFx::Event::Gesture, gestureMask, p.x, p.y,
                       translation.x, translation.y, scale.x, scale.y, rotation);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) && TouchDown)
    {
        pHudMovie->HandleEvent(event);
        TouchDown = true;
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
        pMovie->HandleEvent(event);
}

void FxPlayerMobile::OnGestureEnd(unsigned window, UInt32 gestureMask, const Point<int>& pos)
{

    if (pGestureManager || (pHudMovie != NULL && !pHudMovie->IsValid()) ||
        (pMovie != NULL && !pMovie->IsValid()))
    {
        return;
    }

    Render::PointF p = AdjustInputPoint(pos.x, pos.y);
    GestureEvent event(GFx::Event::GestureEnd, gestureMask, p.x, p.y);

    if (pHudMovie && ((int)(window >> Platform::InputWindow_Shift) == HudWindow) && TouchDown)
    {
        pHudMovie->HandleEvent(event);
        TouchDown = false;
    }
    else if (pMovie && ((int)(window >> Platform::InputWindow_Shift) == ContentWindow))
        pMovie->HandleEvent(event);
}

void FxPlayerMobile::OnSize(const Size<unsigned>& size)
{
    Width = size.Width;
    Height = size.Height;
    /*
    StereoParam.DisplayAspectRatio = float(Width)/float(Height);
    if (UseStereo != Stereo_None)
        pHAL->SetStereoParams(StereoParam);
    */
    if (NeedsOrientation)
        OnOrientation(Orientation);
    else
    {
        Windows[0].BaseViewport = GFx::Viewport(Width, Height, 0,0, Width, Height);
        ResetViewport();
    }

    //SF_DEBUG_MESSAGE2(1, "OnSize %d %d", x,y);
}

void FxPlayerMobile::OnConfigurationChange(const ViewConfig& config)
{
    if (config.HasFlag(View_Stereo))
        UseStereo = config.StereoFormat;
    else
        UseStereo = Platform::Stereo_None;

    pRenderThread->UpdateConfiguration();
}

void FxPlayerMobile::OnFocus(bool setFocus, KeyModifiers)
{
    // By default, if we receive or lose focus, unpause or pause the movie (eg. pause the game). 
    // On Android, this will be called from the onWindowFocusChanged callback.
    if (pMovie)
        pMovie->SetPause(!setFocus);
}

void FxPlayerMobile::OnAccelerometerUpdate(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ)
{
    if (!pMovie)
        return;
    
    AccelerometerEvent event(GFx::Event::Accelerometer, idAcc, timestamp, accelerationX, accelerationY, accelerationZ);
    pMovie->HandleEvent(event);
}

void FxPlayerMobile::OnPause()
{
    if (pMovie)
    {
        //iOS sockets do not persist when the application becomes inactive. 
        //The socket connection needs to be closed when the application is about to go inactive.
#if defined (SF_OS_IPHONE) && !defined SF_BUILD_SHIPPING
        AmpServer::GetInstance().CloseConnection();
#endif
        AppLifecycleEvent event (AppLifecycleEvent::OnPause);
        pMovie->HandleEvent(event);
        
        pMovie->SetPause(true);
    }
}

void FxPlayerMobile::OnResume()
{
    if (pMovie)
    {
		//iOS sockets do not persist when the application becomes inactive. 
		//The socket connection needs to be re-opened when the application is about resume.
#if defined (SF_OS_IPHONE) && !defined SF_BUILD_SHIPPING
        AmpServer::GetInstance().OpenConnection();
#endif
        AppLifecycleEvent event (AppLifecycleEvent::OnResume);
        pMovie->HandleEvent(event);
        
        pMovie->SetPause(false);
    }
}

void FxPlayerMobile::OnGeolocationUpdate(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp)
{
    if (!pMovie)
        return;
    
    GeolocationEvent event(GFx::Event::Geolocation, idGeo, latitude, longitude, altitude, hAccuracy, vAccuracy, speed, heading, timestamp);
    pMovie->HandleEvent(event);
}


void FxPlayerMobile::OnStatus(String* code, String* level, String* extensionId, String* contextId)
{
    if (!pMovie)
        return;

    StatusEvent event(GFx::Event::Status, code, level, extensionId, contextId);
    pMovie->HandleEvent(event);
}

void FxPlayerMobile::Window::UpdateViewport(const Platform::ViewConfig& vc)
{
    if (vc.HasFlag(View_UseOrientation))
    {
        switch (vc.Orientation)
        {
        case Render::Viewport::View_Orientation_L90:
        case Render::Viewport::View_Orientation_R90:
            {
                BaseViewport = GFx::Viewport(vc.ViewSize.Width, vc.ViewSize.Height, 0,0, vc.ViewSize.Height, vc.ViewSize.Width, vc.Orientation);
                Render::MatrixState ms;
                ms.SetOrientation(BaseViewport);
                MouseMatrix = ms.Orient2D.Invert();
            }
            break;

        default:
            BaseViewport = GFx::Viewport(vc.ViewSize.Width, vc.ViewSize.Height, 0,0, vc.ViewSize.Width, vc.ViewSize.Height, vc.Orientation);
            MouseMatrix.SetIdentity();
            break;
        }
    }
    else
    {
        BaseViewport = GFx::Viewport(vc.ViewSize.Width, vc.ViewSize.Height, 0,0, vc.ViewSize.Width, vc.ViewSize.Height, vc.Orientation);
        MouseMatrix.SetIdentity();
    }
}

void FxPlayerMobile::ResetViewport()
{
    pRenderThread->SetViewportFlags(Windows[0].BaseViewport.Flags);

    if (pHudMovie)
    {
        pHudMovie->SetViewport(Windows[HudWindow].BaseViewport);
        SetHudPanelMode();
    }
    if (pMovie)
    {
        UpdateViewSize();

        mViewport = Windows[ContentWindow].BaseViewport;
        if (mViewport.Height == (int)Windows[ContentWindow].BaseViewport.BufferHeight)
        {
            mViewport.Left = (Windows[ContentWindow].BaseViewport.BufferWidth-ViewWidth)/2;
            mViewport.Top = (Windows[ContentWindow].BaseViewport.BufferHeight-ViewHeight)/2;
            mViewport.Width = ViewWidth;
            mViewport.Height = ViewHeight;
        }
        else
        {
            mViewport.Left = (Windows[ContentWindow].BaseViewport.BufferHeight-ViewWidth)/2;
            mViewport.Top = (Windows[ContentWindow].BaseViewport.BufferWidth-ViewHeight)/2;
            mViewport.Width = ViewWidth;
            mViewport.Height = ViewHeight;
        }

        pMovie->SetViewport(mViewport);
    }
}

bool FxPlayerMobile::OnOrientation(unsigned orientation, bool force)
{
    if (LockOrientation && !force)
        return false;

	Orientation = orientation;

#if defined (SF_OS_WINMETRO)
    Platform::ViewConfig newconfig;
    Windows[0].pWindow->GetViewConfig(&newconfig);
    Windows[0].UpdateViewport(newconfig);

    ResetViewport();
#else
	if (pMovie)
    {
        switch(orientation)
        {
            case Render::Viewport::View_Orientation_L90:
            {
                OrientationEvent event(OrientationEvent::RotatedLeft);
                pMovie->HandleEvent(event);
                break;
            }
            case Render::Viewport::View_Orientation_R90:
            {
                OrientationEvent event(OrientationEvent::RotatedRight);
                pMovie->HandleEvent(event);
                break;
            }
            case Render::Viewport::View_Orientation_180:
            {
                OrientationEvent event(OrientationEvent::UpsideDown);
                pMovie->HandleEvent(event);
                break;
            }
            case Render::Viewport::View_Orientation_Normal:
            default:
            {
                OrientationEvent event(OrientationEvent::Default);
                pMovie->HandleEvent(event);
                break;
            }
        }
	}
#endif
    return true;
}

void FxPlayerMobile::ResetInputFocus(unsigned controllerIdx)
{
    SF_UNUSED(controllerIdx);
    if (pMovie)
        pMovie->ResetInputFocus(controllerIdx);
}

void FxPlayerMobile::NotifyShowVirtualKeyboard(const Render::RectF& keyboardRect,
                                               const Render::RectF& textBox)
{
    SF_UNUSED2(keyboardRect, textBox);
    if (pMovie)
    {
        GFx::Viewport vp;
        pMovie->GetViewport(&vp);
        Render::RectF screenRect(vp.Left, vp.Top, Render::SizeF(vp.Width, vp.Height));
        Render::RectF visRect;
        if (keyboardRect.y1 - screenRect.y1 > screenRect.y2 - keyboardRect.y2)
        {
            visRect.y1 = screenRect.y1;
            visRect.y2 = keyboardRect.y1;
        }
        else
        {
            visRect.y1 = keyboardRect.y2;
            visRect.y2 = screenRect.y2;
        }
        /*
        if (keyboardRect.x1 - screenRect.x1 > screenRect.x2 - keyboardRect.x2)
        {
            visRect.x1 = screenRect.x1;
            visRect.x2 = keyboardRect.x1;
         }
        else
        {
            visRect.x1 = keyboardRect.x2;
            visRect.x2 = screenRect.x2;
        }
        */
        visRect.x1 = screenRect.x1;
        visRect.x2 = screenRect.x2;
        pMovie->MakeAreaVisible(visRect, textBox, GFx::Movie::MAVF_ScaleUp50);
    }
}

// A notification that comes from a system to the player about virtual keyboard disappearance.
void FxPlayerMobile::NotifyHideVirtualKeyboard()
{
    if (pMovie)
    {
        pMovie->RestoreViewport();
    }
}

#if defined(GFX_ENABLE_SOUND)
void FxPlayerMobile::InitializeSound()
{
#if defined(GFX_SOUND_FMOD)
    if (!pSoundSystem)
        pSoundSystem = new FxSoundFMOD;
    pSoundSystem->pFileOpener = FxPlayerMobile::pApp->mLoader.GetFileOpener();
    if (!pSoundSystem->IsInitialized())
    {
        if (!pSoundSystem->Initialize(
#ifdef SF_OS_3DS
            GetDeviceMemoryHeap()
#endif
        ))
            ShutdownSound();
    }
#endif
}

void FxPlayerMobile::ShutdownSound()
{
#if defined(GFX_SOUND_FMOD)
    if (pSoundSystem && pSoundSystem->IsInitialized())
        pSoundSystem->Finalize();
    delete pSoundSystem;
    pSoundSystem = 0;
#endif
}

Sound::SoundRenderer* FxPlayerMobile::GetSoundRenderer()
{
#if defined(GFX_SOUND_FMOD)
    return pSoundSystem && pSoundSystem->IsInitialized() ?
           pSoundSystem->GetSoundRenderer() :
           0;
#else
    return 0;
#endif
}
#endif // GFX_ENABLE_SOUND

