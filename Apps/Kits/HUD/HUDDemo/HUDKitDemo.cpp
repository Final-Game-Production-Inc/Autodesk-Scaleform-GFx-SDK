/**************************************************************************

Filename    :   HUDKitDemo.cpp
Content     :   HUD demo using the complex objects API (leverages
                 the SWF/GFX file player code)
Created     :
Authors     :   Michael Antonov, Maxim Didenko, Boris Rayskiy, 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


//#define SF_NO_SOUND
#undef GFX_ENABLE_XML 
//#define SF_NO_IME_SUPPORT

#include "Kernel/SF_KeyCodes.h"
#include "FxGameDelegate.h"
#include "HUDSimulation.h"
#include "HUDEntityController.h"
#include "HUDAdapter.h"
#include "Kernel/SF_MsgFormat.h"
#include "../Common/FxMinimapView.h"

#include "../Common/FxHUDView.h"
#include "../Common/FxPlayerAppBase.h"

#include "../FxPlayer/FxHUD.h"

#include "GFx/AS3/AS3_Global.h"

#ifdef SF_OS_WIN32
#include <tchar.h>
#endif

using namespace Scaleform;
using namespace Render;
using namespace GFx;

class FxPlayerApp;
class FxPlayerAppDelegateHandler : public FxDelegateHandler
{
public:
    FxPlayerApp *theApp;

    FxPlayerAppDelegateHandler() : theApp(NULL) { }

        // FxDelegateHandler implementation
    void                    Accept(FxDelegateHandler::CallbackProcessor* cbreg);
};

class   FxPlayerApp : public FxPlayerAppBase, public FxHUDInfoProvider
{
public:
    FxPlayerAppDelegateHandler delegateHandler;
    HUDSimulation*          pSimulation;
    HUDEnvironmentAdapter   HUDAdapter;
    Ptr<FxDelegate>         pGameDelegate;

    FxHUDView::Config       HUDViewConfig;
    Ptr<FxHUDView>          pHUDView;

    bool                    bCaptureStats;
	bool					bShowingMinimapControls;
    
    // Statistic tracking variables.
    UInt64                  MMUpdateTimeAccumulator;
    UInt64                  HUDUpdateTimeAccumulator;
    int                     HUDFrameCounter;
    FxHUDView::Stats        HUDstats;

    FxPlayerApp();
    virtual ~FxPlayerApp() { }


    // ** FxDelegate callbacks

    // HUD specific    
    static void             RegisterHUDView(const FxDelegateArgs& params);	
    static void             RegisterMiniMapView(const FxDelegateArgs& params);

    // Test controls specific
    static void             EnableSimulation(const FxDelegateArgs& params);
    static void             CaptureStats(const FxDelegateArgs& params);
    
    bool                    ProcessHUDKeyEvent(KeyCode keyCode, bool downFlag);

    virtual String          GetAppTitle() const;
    virtual String          GetFilePath() const;
    virtual void            UpdateWindowTitle();

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnShutdown();

#ifdef SF_AMP_SERVER
	virtual void			GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const;
#endif

    virtual void            SetMovieVariables(String& argString);
//    virtual bool            DoRender();
    virtual bool            AdvanceMovie(float deltaT);
    virtual void            InstallHandlers();
    virtual bool            LoadDefaultFontConfigFromPath(const String& path);
    virtual void            OnDropFiles(const String& path);
    virtual void            OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag);
    virtual void            OnKey(unsigned controllerIndex, KeyCode keyCode,
                                unsigned wcharCode, bool downFlag, KeyModifiers mods);

    // No system font
    virtual void            SetFontProvider()   {}


    // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
    virtual void            OnHudStatusChange(bool active) { SF_UNUSED(active); }  

    void                    UpdatePerformanceStats();

    virtual void            UpdateStatReports();
    virtual void            UpdateFpsDisplay();
    virtual void            UpdateViewSize();
    void                    UpdateHUDViewport();

    virtual void            InitArgDescriptions(Platform::Args* args);

    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);

    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;

    Hash<UInt32, Ptr<FxPlayerCommand> >     HudKeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > HudPadKeyCommandMap;
};

class FxPlayerCommandHudReport : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHudDataProvider)
        {
            playerApp->UpdatePerformanceStats();

            bool fileWritten = false;
            bool xmlLog = playerApp->GetArgs().GetBool("XmlLog");
            if (playerApp->GetArgs().HasValue("RecordStatistics"))
            {
                fileWritten = playerApp->pHudDataProvider->SaveStatistics(playerApp->GetArgs().GetString("RecordStatistics"), xmlLog);
            }

            if (!fileWritten)
            {
                playerApp->pHudDataProvider->PrintStatistics(xmlLog);
            }
        }
    }
    virtual String GetHelpString() const
    {
        return "Print HUD report to console";
    }
};

class FxPlayerCommandInfoHelp : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->ToggleTab(FxHUD::Help);
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle Help";
    }
};

class FxPlayerCommandInfoSummary : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->ToggleTab(FxHUD::Summary);
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle Summary";
    }
};

class FxPlayerCommandHudNextTab : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->NextTab();
        }
    }
    virtual String GetHelpString() const
    {
        return "Cycle HUD Tabs (next)";
    }
};

class FxPlayerCommandHudPrevTab : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->PreviousTab();
        }
    }
    virtual String GetHelpString() const
    {
        return "Cycle HUD Tabs (prev)";
    }
};

class FxPlayerCommandHudOpen : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->SetVisible(true);
        }
    }
    virtual String GetHelpString() const
    {
        return "Open HUD";
    }
};

class FxPlayerCommandHudClose : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->SetVisible(false);
        }
    }
    virtual String GetHelpString() const
    {
        return "Close HUD";
    }
};
//------------------------------------------------------------------------
// ***** Main APP implementation

SF_PLATFORM_SYSTEM_APP(HUDKit, Scaleform::GFx::System, FxPlayerApp)

// ***** FxPlayerApp Implementation

FxPlayerApp::FxPlayerApp()
{
    pHudHeap = NULL;
    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);

    KeyCommandMap.Set(Key::F5, *SF_NEW FxPlayerCommandHudReport());
    KeyCommandMap.Set(Key::F1, *SF_NEW FxPlayerCommandInfoHelp());
    KeyCommandMap.Set(Key::F2, *SF_NEW FxPlayerCommandInfoSummary());
    KeyCommandMap.Set(Key::F12, *SF_NEW FxPlayerCommandHudClose());
    KeyCommandMap.Set(Key::H | ctrlMask, *SF_NEW FxPlayerCommandHudNextTab());
#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32)
    PadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PS3) || defined(SF_OS_PS2)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PSP)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_WII)
    PadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_WIIU)
    PadKeyCommandMap.Set(Pad_Plus, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_Plus, *SF_NEW FxPlayerCommandHudNextTab());
#endif

    Background          = 0;

    pGameDelegate   = *new FxDelegate();

    // Enable measure perf mode
    MeasurePerformance = 1;

    HUDSimulation::Config   simConfig;
    simConfig.PhysicalMapDimensions.SetRect(100, 100);
    simConfig.InboundsDimensions.SetRect(25, 25, 75, 75);   // 124px border
    simConfig.InitialLocations.BlueSpawnPoints.PushBack(PointF(30, 30));
    simConfig.InitialLocations.BlueSpawnPoints.PushBack(PointF(40, 30));
    simConfig.InitialLocations.BlueSpawnPoints.PushBack(PointF(30, 40));
    simConfig.InitialLocations.RedSpawnPoints.PushBack(PointF(70, 70));
    simConfig.InitialLocations.RedSpawnPoints.PushBack(PointF(60, 70));
    simConfig.InitialLocations.RedSpawnPoints.PushBack(PointF(70, 60));
    simConfig.InitialSettings.NumRedPlayers        = 16;
    simConfig.InitialSettings.NumBluePlayers     = 16;
    simConfig.InitialSettings.NumCapturePoints  = 6;
    simConfig.InitialSettings.NumPowerups     = 15;
    pSimulation = new HUDSimulation(simConfig);

    bCaptureStats   = true;
	bShowingMinimapControls = false;

    pSimulation->Start();
    HUDAdapter.SetSimulation(pSimulation);

    MMUpdateTimeAccumulator = 0;
    HUDUpdateTimeAccumulator = 0;
    HUDFrameCounter = 0;

    // Disable some GFxPlayer controls 
	KeyCommandMap.Remove(Key::F3 | ctrlMask);
	KeyCommandMap.Remove(Key::F4 | ctrlMask);
	KeyCommandMap.Remove(Key::F5 | ctrlMask);
    KeyCommandMap.Remove(Key::R | ctrlMask);      // Restart
    KeyCommandMap.Remove(Key::Right | ctrlMask);  // Step forward
    KeyCommandMap.Remove(Key::Left | ctrlMask);  // Step back
    KeyCommandMap.Remove(Key::PageDown | ctrlMask);  // Step back 10
    KeyCommandMap.Remove(Key::PageUp | ctrlMask);  // Step forward 10
    KeyCommandMap.Remove(Key::B | ctrlMask);  // Background
    KeyCommandMap.Remove(Key::BracketRight | ctrlMask);  // Next file
    KeyCommandMap.Remove(Key::BracketLeft | ctrlMask);  // Prev file
#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32)
    PadKeyCommandMap.Remove(Pad_Y);// Restart
    PadKeyCommandMap.Remove(Pad_R2);  // Next file
    PadKeyCommandMap.Remove(Pad_L2);  // Prev file
#elif defined(SF_OS_PS3) || defined(SF_OS_PS2)
    PadKeyCommandMap.Remove(Pad_T);// Restart
    PadKeyCommandMap.Remove(Pad_R2);  // Next file
    PadKeyCommandMap.Remove(Pad_L2);  // Prev file
#elif defined(SF_OS_PSP)
    PadKeyCommandMap.Remove(Pad_T);// Restart
    PadKeyCommandMap.Remove(Pad_R1);  // Next file
    PadKeyCommandMap.Remove(Pad_L1);  // Prev file
#elif defined(SF_OS_WII) || defined (SF_OS_WIIU)
    //PadKeyCommandMap.Remove(Pad_Plus);  // Next file
    PadKeyCommandMap.Remove(Pad_Minus);  // Prev file
#endif
}

void FxPlayerApp::GetExtraInfo(FxHUDExtraInfo* pinfo) 
{ 
    pinfo->LastFPS = LastFPS;
    pinfo->TessTriangles = 0; //TessTriangles;
    pinfo->Zoom = Zoom;   
    pinfo->Paused = Paused;
    pinfo->AppRes = GetViewSize();
    switch (AAMode) 
    {
    case AAMode_None:
        pinfo->AAMode = "None";
        break;
    case AAMode_EdgeAA:
        // Display a custom message if edge AA is #ifdef-ed out.
#ifndef SF_NO_FXPLAYER_EDGEAA
        pinfo->AAMode = "Edge AA";
        {
            // Report if EdgeAA is not supported by renderer.
            //@TODO
            //             Renderer::RenderCaps caps;
            //             caps.CapBits = 0;
            //             if (GetRenderer())
            //                 GetRenderer()->GetRenderCaps(&caps);
            //             if (!(caps.CapBits & Renderer::Cap_FillGouraud))
            //                 pinfo->AAMode = "Edge AA (Not Supported)";
            //             else if (!(caps.CapBits & Renderer::Cap_FillGouraudTex))
            //                 pinfo->AAMode = "Edge AA (Limited)";
        }
#else
        pinfo->AAMode = "Edge AA [#disabled]";
#endif
        break;
    case AAMode_FSAA:
        if (ViewConfigHasFlag(Platform::View_FSAA))
            pinfo->AAMode = "HW FSAA";
        else
            pinfo->AAMode = "HW FSAA (Not Supported)";
        break;
    }

    pinfo->CurvePixelError = CurvePixelError;


    // Display a custom message if stroke is #ifdef-ed out.
    //#ifndef SF_NO_FXPLAYER_STROKER
    //    pinfo->StrokeType = "Correct";
    //    if (stroke == RenderConfig::RF_StrokeNormal)
    //        pinfo->StrokeType = "Normal";
    //#else
    //    pinfo->StrokeType = "Correct [#disabled]";
    //    if (stroke == RenderConfig::RF_StrokeNormal)
    //        pinfo->StrokeType = "Normal [#disabled]";
    //#endif

    pinfo->FontConfig = (FontConfigIndex == -1) ? "Default"
        : FontConfigs[FontConfigIndex]->ConfigName.ToCStr();
    pinfo->RasCount = pRenderThread->GetGlyphRasterizationCount();
}

void FxPlayerApp::GetHelpStr(String* phelpStr)
{
    phelpStr->Clear(); 
    if (!IsConsole())
    {
        *phelpStr += "Keys:\n";
        Hash<UInt32, Ptr<FxPlayerCommand> >::Iterator it;
        for (it = KeyCommandMap.Begin(); it != KeyCommandMap.End(); ++it)
        {
            KeyCode keyCode = (KeyCode)(UInt16)(it->First);
            KeyModifiers keyModifiers((UInt8)(it->First >> 16));

            *phelpStr += "  ";
            if (keyModifiers.IsCtrlPressed())
            {
                *phelpStr += "CTRL ";
            }
            if (keyModifiers.IsAltPressed())
            {
                *phelpStr += "ALT ";
            }
            if (keyModifiers.IsShiftPressed())
            {
                *phelpStr += "SHIFT ";
            }
            if (keyModifiers.IsNumToggled())
            {
                *phelpStr += "NUM ";
            }
            if (keyModifiers.IsCapsToggled())
            {
                *phelpStr += "CAPS ";
            }
            if (keyModifiers.IsScrollToggled())
            {
                *phelpStr += "SCROLL ";
            }

            String keyDesc;
            switch (keyCode)
            {
            case Key::A: keyDesc = "A"; break;
            case Key::B: keyDesc = "B"; break;
            case Key::C: keyDesc = "C"; break;
            case Key::D: keyDesc = "D"; break;
            case Key::E: keyDesc = "E"; break;
            case Key::F: keyDesc = "F"; break;
            case Key::G: keyDesc = "G"; break;
            case Key::H: keyDesc = "H"; break;
            case Key::I: keyDesc = "I"; break;
            case Key::J: keyDesc = "J"; break;
            case Key::K: keyDesc = "K"; break;
            case Key::L: keyDesc = "L"; break;
            case Key::M: keyDesc = "M"; break;
            case Key::N: keyDesc = "N"; break;
            case Key::O: keyDesc = "O"; break;
            case Key::P: keyDesc = "P"; break;
            case Key::Q: keyDesc = "Q"; break;
            case Key::R: keyDesc = "R"; break;
            case Key::S: keyDesc = "S"; break;
            case Key::T: keyDesc = "T"; break;
            case Key::U: keyDesc = "U"; break;
            case Key::V: keyDesc = "V"; break;
            case Key::W: keyDesc = "W"; break;
            case Key::X: keyDesc = "X"; break;
            case Key::Y: keyDesc = "Y"; break;
            case Key::Z: keyDesc = "Z"; break;
            case Key::F1: keyDesc = "F1"; break;
            case Key::F2: keyDesc = "F2"; break;
            case Key::F3: keyDesc = "F3"; break;
            case Key::F4: keyDesc = "F4"; break;
            case Key::F5: keyDesc = "F5"; break;
            case Key::F6: keyDesc = "F6"; break;
            case Key::F7: keyDesc = "F7"; break;
            case Key::F8: keyDesc = "F8"; break;
            case Key::F9: keyDesc = "F9"; break;
            case Key::F10: keyDesc = "F10"; break;
            case Key::F11: keyDesc = "F11"; break;
            case Key::F12: keyDesc = "F12"; break;
            case Key::Backspace: keyDesc = "Backspace"; break;
            case Key::Tab: keyDesc = "Tab"; break;
            case Key::Clear: keyDesc = "Clear"; break;
            case Key::Return: keyDesc = "Return"; break;
            case Key::Shift: keyDesc = "Shift"; break;
            case Key::Control: keyDesc = "Control"; break;
            case Key::Alt: keyDesc = "Alt"; break;
            case Key::Pause: keyDesc = "Pause"; break;
            case Key::CapsLock: keyDesc = "Caps Lock"; break;
            case Key::Escape: keyDesc = "Escape"; break;
            case Key::Space: keyDesc = "Space"; break;
            case Key::PageUp: keyDesc = "Page Up"; break;
            case Key::PageDown: keyDesc = "Page Down"; break;
            case Key::End: keyDesc = "End"; break;
            case Key::Home: keyDesc = "Home"; break;
            case Key::Left: keyDesc = "Left"; break;
            case Key::Up: keyDesc = "Up"; break;
            case Key::Right: keyDesc = "Right"; break;
            case Key::Down: keyDesc = "Down"; break;
            case Key::Insert: keyDesc = "Insert"; break;
            case Key::Delete: keyDesc = "Delete"; break;
            case Key::Help: keyDesc = "Help"; break;
            case Key::NumLock: keyDesc = "NumLock"; break;
            case Key::ScrollLock: keyDesc = "ScrollLock"; break;
            case Key::Semicolon: keyDesc = ";"; break;
            case Key::Equal: keyDesc = "="; break;
            case Key::Comma: keyDesc = ","; break;
            case Key::Minus: keyDesc = "-"; break;
            case Key::Period: keyDesc = "."; break;
            case Key::Slash: keyDesc = "/"; break;
            case Key::Bar: keyDesc = "|"; break;
            case Key::BracketLeft: keyDesc = "["; break;
            case Key::Backslash: keyDesc = "\\"; break;
            case Key::BracketRight: keyDesc = "]"; break;
            case Key::Quote: keyDesc = "'"; break;
            default: keyDesc = ""; break;
            }
            *phelpStr += keyDesc;
            for (UPInt i = 0; i < 16 - keyDesc.GetLength(); ++i)
            {
                *phelpStr += ' ';
            }

            *phelpStr += it->Second->GetHelpString();
            *phelpStr += '\n';
        }
        *phelpStr += '\n';
    }

    if (IsPadConnected(0) || IsConsole())
    {
        *phelpStr += "Pad:\n";
        Hash<PadKeyCode, Ptr<FxPlayerCommand> >::Iterator it;
        for (it = PadKeyCommandMap.Begin(); it != PadKeyCommandMap.End(); ++it)
        {
            *phelpStr += "  ";
            String keyDesc;
            switch (it->First)
            {
            case Pad_Back: keyDesc = "Back"; break;
            case Pad_Start: keyDesc = "Start"; break;
            case Pad_A: keyDesc = "A"; break;
            case Pad_B: keyDesc = "B"; break;
            case Pad_X: keyDesc = "X"; break;
            case Pad_Y: keyDesc = "Y"; break;
            case Pad_R1: keyDesc = "RB"; break;
            case Pad_L1: keyDesc = "LB"; break;
            case Pad_R2: keyDesc = "RT"; break;
            case Pad_L2: keyDesc = "LT"; break;
            case Pad_Up: keyDesc = "Up"; break;
            case Pad_Down: keyDesc = "Down"; break;
            case Pad_Right: keyDesc = "Right"; break;
            case Pad_Left: keyDesc = "Left"; break;
            case Pad_Plus: keyDesc = "+"; break;
            case Pad_Minus: keyDesc = "-"; break;
            case Pad_1: keyDesc = "1"; break;
            case Pad_2: keyDesc = "2"; break;
            case Pad_H: keyDesc = "H"; break;
            case Pad_C: keyDesc = "C"; break;
            case Pad_Z: keyDesc = "Z"; break;
            case Pad_O: keyDesc = "Back"; break;
            case Pad_T: keyDesc = "Triangle"; break;
            case Pad_S: keyDesc = "Square"; break;
            case Pad_Select: keyDesc = "Select"; break;
            case Pad_Home: keyDesc = "Home"; break;
            case Pad_RT: keyDesc = "RS"; break;
            case Pad_LT: keyDesc = "LS"; break;
            default: keyDesc = ""; break;
            }
            *phelpStr += keyDesc;
            for (UPInt i = 0; i < 11 - keyDesc.GetLength(); ++i)
            {
                *phelpStr += ' ';
            }
            *phelpStr += it->Second->GetHelpString();
            *phelpStr += '\n';
        }
        *phelpStr += '\n';
    }
}


//
// GameDelegate callback registration for HUD and Minimap.
//
void FxPlayerAppDelegateHandler::Accept(FxDelegateHandler::CallbackProcessor* cbreg)
{    
    cbreg->Process("registerHUDView", FxPlayerApp::RegisterHUDView);
    cbreg->Process("registerMiniMapView", FxPlayerApp::RegisterMiniMapView);

    cbreg->Process("enableSimulation", FxPlayerApp::EnableSimulation);
    cbreg->Process("captureStats", FxPlayerApp::CaptureStats);
}

//
// Initialize and configure the HUD View with the HUD movieClip.
// Register the HUD View with the HUD Adapter.
//
void FxPlayerApp::RegisterHUDView(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // HUD movieclip
    FxPlayerApp* pthis = ((FxPlayerAppDelegateHandler*)params.GetHandler())->theApp;
    
    Value hud        = params[0];
    SF_ASSERT(hud.IsDisplayObject());

    pthis->HUDViewConfig.HUDMC = hud;
	pthis->HUDViewConfig.pMovie = pthis->pMovie;

    pthis->pHUDView = *new FxHUDView(pthis->HUDViewConfig);
    pthis->pGameDelegate->RegisterHandler(pthis->pHUDView);

    pthis->HUDAdapter.RegisterHUDEventListener(pthis->pHUDView);
}

//
// Initialize and configure the Minimap View with the Minimap movieClip.
// Register the Minimap View with the HUD View.
//
void FxPlayerApp::RegisterMiniMapView(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==2);   // minimap movieclip, view radius, 
    FxPlayerApp* pthis = ((FxPlayerAppDelegateHandler*)params.GetHandler())->theApp;

    Value minimap        = params[0];
    SF_ASSERT(minimap.IsDisplayObject());
    float viewRadius        = (params.GetMovie()->GetAVMVersion() == 1) ? (float)params[1].GetNumber() : (float)params[1].GetInt();

    FxMinimapView::Config   config;
    config.ViewRadius = viewRadius;

    // Define zoom levels and zoom level indices
    config.MinimapMC = minimap;
    config.ZoomFactors.PushBack(1.f);
    config.ZoomFactors.PushBack(2.f);
    config.ZoomFactors.PushBack(4.f);
    config.ZoomFactors.PushBack(8.f);
    config.InitialZoomIndex = 1;
    config.CurrMapZoomIndex = 0;    

    SF_ASSERT(pthis->pHUDView); // Register HUDView first
    pthis->pHUDView->RegisterMinimap(config);
    pthis->pGameDelegate->RegisterHandler(pthis->pHUDView->GetMinimapView());
}

////

//
// Enable/disable the simulation. 
//
void FxPlayerApp::EnableSimulation(const FxDelegateArgs &params)
{
    SF_ASSERT(params.GetArgCount()==1);   // enable sim?
    FxPlayerApp* pthis = ((FxPlayerAppDelegateHandler*)params.GetHandler())->theApp;

    if (params[0].GetBool())
        pthis->pSimulation->Start();
    else
        pthis->pSimulation->Stop();
}


//
// Enable/disable capturing display and update statistics.
//
void FxPlayerApp::CaptureStats(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // capture flag
    FxPlayerApp* pthis = ((FxPlayerAppDelegateHandler*)params.GetHandler())->theApp;

    pthis->bCaptureStats = params[0].GetBool();
}

//
// Input handling for HUD Demo.
//		W - Move forward
//		S - Move backward
//		A - Turn left
//		D - Turn right
//
//		SPACE - Fire weapon
//		R - Reload weapon
//		G - Throw grenade
//		1 - Switch to secondary weapon
//		2 - Switch to primary weapon
//
//		B - Toggle AI control of player
//		M - Toggle minimap control panel
//
bool FxPlayerApp::ProcessHUDKeyEvent(KeyCode keyCode, bool downFlag)
{
    switch (keyCode)
    {
    case Key::W:
        {
            pSimulation->PlayerMovingForward(downFlag);
            return true;
        }
        break;
    case Key::S:
        {
            pSimulation->PlayerMovingBackward(downFlag);
            return true;
        }
        break;
    case Key::A:
        {
            pSimulation->PlayerTurningLeft(downFlag);
            return true;
        }
        break;
    case Key::D:
        {
            pSimulation->PlayerTurningRight(downFlag);
            return true;
        }
        break;
    case Key::B:
        {
            if (!downFlag)
            {
                pSimulation->SwitchUserController();
                return true;
            }
        }
        break;
	case Key::Space:
        {
            pSimulation->PlayerShooting(downFlag);
            return true;
        }
        break;
	case Key::R:
        {
            if (!downFlag)
            {
                pSimulation->PlayerReloadWeapon();
                return true;
            }
        }
        break;
	case Key::G:
        {
            if (!downFlag)
            {
				pSimulation->PlayerThrowGrenade();
                return true;
            }
        }
        break;
	case Key::Num1:
		{
			if (!downFlag)
			{
				pSimulation->GetPlayer()->ChangeWeaponByIndex(0);
				return true;
			}
		}
        break;
	case Key::Num2:
		{
			if (!downFlag)
			{
				pSimulation->GetPlayer()->ChangeWeaponByIndex(1);
				return true;
			}
		}
        break;
	case Key::Num3:
		{
			if (!downFlag)
			{
				pSimulation->GetPlayer()->ChangeWeaponByIndex(2);
				return true;
			}
		}
		break;
	case Key::Num4:
		{
			if (!downFlag)
			{
				pSimulation->GetPlayer()->ChangeWeaponByIndex(3);
				return true;
			}
		}
		break;
	case Key::M:
		{
			if (!downFlag)
			{
				pHUDView->ToggleMinimapLock();              
				return true;
			}
		}
		break;
	case Key::Q:
		{
			if (!downFlag)
			{
				pSimulation->GetPlayer()->CycleWeapons();  		
				return true;
			}
		}
		break;
	default:
		break;
	}
    return false;
}


String FxPlayerApp::GetAppTitle() const
{
    String title("Scaleform HUD Kit Demo v ");

    // Version
    title += GFX_VERSION_STRING;

    // Add the word "Debug" to the application
    // title if in debug build mode
#ifdef SF_BUILD_DEBUG
    title += " " SF_BUILD_STRING;
#endif

    return title;
}

String FxPlayerApp::GetFilePath() const
{
    String filePath;

#if defined(SF_OS_XBOX360) && !defined(SF_BUILD_SHIPPING)
    filePath = "D:\\HUDKit";
#elif defined(SF_OS_XBOX360) && defined(SF_BUILD_SHIPPING)
    filePath = "D:\\data\\HUDKit";
#elif defined (SF_OS_PS3) && !defined(SF_BUILD_SHIPPING)
    filePath = "/app_home/HUDKit";
#elif defined (SF_OS_PS3) && defined(SF_BUILD_SHIPPING)
	filePath = "/app_home/data/HUDKit";
#elif defined (SF_OS_WIN32) && !defined(SF_BUILD_SHIPPING)
    filePath = "HUDKit";
#elif defined (SF_OS_WIN32) && defined(SF_BUILD_SHIPPING)
	filePath = "data/HUDKit"; 
#elif defined (SF_OS_WIIU) && !defined(SF_BUILD_SHIPPING)
    filePath = "/vol/content/Kits/HUD/HUDKit";
#elif defined (SF_OS_WIIU) && defined(SF_BUILD_SHIPPING)
	filePath = "/vol/content/Kits/data/HUDKit"; 
#else
    filePath = "HUDKit";
#endif

#ifdef SF_BUILD_SHIPPING 
    filePath += ".gfx";
#else
    filePath += ".swf";
#endif
    return filePath;
}



bool FxPlayerApp::OnInit(Platform::ViewConfig& config)
{
    if (!FxPlayerAppBase::OnInit(config))
        return false;

#ifdef SF_OS_WIN32
    if (!pMovie)
    {
        char temp[128];
        sprintf_s(temp, 128, "Failed loading %s!\nPlease check your working directory and try again.\n", FileName.ToCStr());
        MessageBoxA(NULL, temp, _T("HUDKit Error"), MB_OK);

        OnShutdown();
        return false;
    }
#endif

    pHudHeap = Memory::GetGlobalHeap()->CreateHeap("HUD", MemoryHeap::Heap_UserDebug);

    pHudDataProvider = 
        *SF_HEAP_NEW(pHudHeap) FxHUDDataProvider(pMovie, pRenderThread, this, IsConsole());

    if (!GetArgs().GetBool("NoHud") && pHudDataProvider) 
    {
        pHud = *FxHUD::CreateHUD(pHudDataProvider);
        UpdateHUDViewport();
    }

    return true;
}

#ifdef SF_AMP_SERVER
void			FxPlayerApp::GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const
{
    caps.SetCurveToleranceDown(true);
    caps.SetCurveToleranceUp(true);
    caps.SetNextFont(true);
    caps.SetRestartMovie(false);
    caps.SetToggleAaMode(true);
    caps.SetToggleAmpRecording(true);
    caps.SetToggleFastForward(true);
    caps.SetToggleInstructionProfile(true);
    caps.SetToggleOverdraw(true);
    caps.SetToggleBatch(true);
    caps.SetToggleBlending(true);
    caps.SetToggleTextureDensity(true);
    caps.SetToggleStrokeType(false);
    caps.SetTogglePause(true);
    caps.SetToggleWireframe(true);
}
#endif

void FxPlayerApp::SetMovieVariables(String& argString)
{
    FxPlayerAppBase::SetMovieVariables(argString);

	if (pMovie->GetAVMVersion() == 1) {
#ifdef SF_OS_WIN32
		pMovie->SetVariable("_global.addSafeRect", Value("0,0,0,0"));
#elif defined SF_OS_XBOX360
		pMovie->SetVariable("_global.addSafeRect", Value("40,40,40,40"));
#elif defined SF_OS_PS3
		pMovie->SetVariable("_global.addSafeRect", Value("40,40,40,40"));
#elif defined SF_OS_WII || defined (SF_OS_WIIU)
        pMovie->SetVariable("_global.addSafeRect", Value("40,40,40,40"));
#endif
	}
}

void FxPlayerApp::UpdateStatReports()
{
    FxPlayerAppBase::UpdateStatReports();

    if (pHud && (!pMovie || pHud->IsVisible() /* || SafeRectEnabled*/))
    {   				
        pHud->Update();
        pHudDataProvider->UpdateRendererStats();
        UpdatePerformanceStats();
    }

    char buff[16];
    StringDataPtr str(buff, sizeof(buff));
    if (bCaptureStats && (LastAdvanceTicks - LastFPSUpdatedTicks > TIME_UNITS))
    {
        MMUpdateTimeAccumulator = 0;
        HUDUpdateTimeAccumulator = 0;
    }
}

void FxPlayerApp::UpdateFpsDisplay()
{
    if (GetArgs().GetBool("LogFPS"))
    {
        if (pHudDataProvider)
        {
            UpdatePerformanceStats();
            pHudDataProvider->PrintStatistics(false);
        }
        else
        {
            FxPlayerAppBase::UpdateFpsDisplay();
        }
    }
}

void FxPlayerApp::UpdateViewSize() 
{
    FxPlayerAppBase::UpdateViewSize();
    UpdateHUDViewport();
}

void FxPlayerApp::UpdateHUDViewport()
{
    if (pHud)
    {
        Render::Size<unsigned> viewSize = GetViewSize();
        Render::Viewport hudViewport(viewSize.Width, viewSize.Height,
            int(viewSize.Width * GetSafeArea().Width),
            int(viewSize.Height * GetSafeArea().Height),
            int(viewSize.Width - 2 * viewSize.Width * GetSafeArea().Width),
            int(viewSize.Height - 2 * viewSize.Height * GetSafeArea().Height));

        pHud->SetViewport(hudViewport);
    }
}

FxPlayerCommand* FxPlayerApp::PadKeyToCommand(PadKeyCode key)
{
    Hash< PadKeyCode, Ptr<FxPlayerCommand> >::Iterator it;
    if (pHud && pHud->IsVisible())
    {
        it = HudPadKeyCommandMap.Find(key);
        if (it != HudPadKeyCommandMap.End())
        {
            return it->Second;
        }
    }

    // Return the player's command by default, if no matching HUD command was found.
    it = PadKeyCommandMap.Find(key);
    if (it != PadKeyCommandMap.End())
    {
        return it->Second;
    }

    return NULL;
}

void FxPlayerApp::UpdatePerformanceStats()
{
    float fps = AccumFPSSecondCount > 0 ? AccumFPS / AccumFPSSecondCount : 0.0f;
    pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks, pRenderThread->GetLastDisplayTicks(), fps);
}

void FxPlayerApp::InitArgDescriptions(Platform::Args* args)
{
    FxPlayerAppBase::InitArgDescriptions(args);

    Platform::ArgDesc options []=
    {
        //        {"i",   "DisplayHudOnStartup", FxArgs::Flag, "", "Display info HUD on startup.",},
        {"nh",  "NoHud",  Platform::Args::Flag, "", "Do not load HUD SWF."},
        {"","", Platform::Args::ArgEnd,"",""},
    };
    args->AddDesriptions(options);
}

void FxPlayerApp::OnShutdown()
{
    // Release the minimap view and HUD view to drop the Values holding AS object references.
    // If not released before the movie is released, then we get memory leaks.
    HUDViewConfig.HUDMC.SetUndefined();

    if (pSimulation)
        delete pSimulation;

    if (pHUDView)
    {
        if (pHUDView->GetMinimapView())
        {
            pGameDelegate->UnregisterHandler(pHUDView->GetMinimapView());
        }
        pGameDelegate->UnregisterHandler(pHUDView);
    }

    pGameDelegate->UnregisterHandler(&delegateHandler);
    pHUDView = NULL;

    if (pHudDataProvider)
    {
        bool xmlLog = GetArgs().GetBool("XmlLog");
        UpdatePerformanceStats();
        if (GetArgs().HasValue("RecordStatistics"))
        {
            if (!pHudDataProvider->SaveStatistics(GetArgs().GetString("RecordStatistics"), xmlLog))
            {
                pHudDataProvider->PrintStatistics(xmlLog); 
            }
        }
        else if (xmlLog)
        {
            pHudDataProvider->PrintStatistics(true); 
        }

        // Releasing the movie will automatically shut down its
        // background loading task, if any.
        pHud.Clear();
        pHudDataProvider.Clear();
        pHudHeap->Release();
    }

    FxPlayerAppBase::OnShutdown();
}

void FxPlayerApp::OnUpdateFrame(bool needRepaint)
{
    UInt64 mMstartDisplay = 0;
    if (bCaptureStats)
    {
        mMstartDisplay = Timer::GetTicks();
    }
    
    FxPlayerAppBase::OnUpdateFrame(needRepaint);   
}

bool FxPlayerApp::AdvanceMovie(float deltaT)
{
    UInt64 mMstartAdvance = 0;
    if (bCaptureStats)
    {
         mMstartAdvance = Timer::GetTicks();
    }

    //////////////////////////////////////////////////////////////////////////
    // UPDATE SIM
    HUDstats.UpdateTime = 0;
    HUDstats.MinimapStats.PushedObjects = 0;

    // Update the simulation
    if (pSimulation->IsRunning())
        pSimulation->Update(AccumClockTicks);

    // Update the HUD Demo HUD View
    if ( pHUDView )
    {
        if (pSimulation->IsRunning())
            pHUDView->UpdateView(&HUDAdapter, pMovie, bCaptureStats ? &HUDstats : NULL);

        if (bCaptureStats)
        {
            MMUpdateTimeAccumulator += HUDstats.MinimapStats.UpdateTime;
            HUDUpdateTimeAccumulator += HUDstats.UpdateTime + HUDstats.MinimapStats.UpdateTime;
        }
    }

    if (bCaptureStats) HUDFrameCounter++;
    //////////////////////////////////////////////////////////////////////////

    bool result = FxPlayerAppBase::AdvanceMovie(deltaT);
    if (pMovie)
        pMovie->SetBackgroundAlpha(Background ? 1.0f : 0.05f);
    return result;
}


void FxPlayerApp::InstallHandlers()
{
    FxPlayerAppBase::InstallHandlers();

    pMovie->SetExternalInterface(NULL);
    mLoader.SetExternalInterface(pGameDelegate);
    delegateHandler.theApp = this;
    pGameDelegate->RegisterHandler(&delegateHandler);
}


void FxPlayerApp::UpdateWindowTitle()
{
    String strTitle;

    if (pMovie && (MeasurePerformance || FastForward)) 
    {
        // Display frame rate in title
        //Format(strTitle, "{1} (fps:{0:3.1})", LastFPS, GetAppTitle());

        GFx::Double invDiv = 1.0 / ((double)HUDFrameCounter*1000);   // display per frame numbers in ms, so convert from seconds to milliseconds

        Render::HAL::Stats rstats;
        pRenderThread->GetRenderStats(&rstats);
        UInt64 displayTicks = pRenderThread->GetLastDisplayTicks();
        UInt64 otherTicks   = pRenderThread->GetLastDrawFrameTicks() - displayTicks;

        //@TODO - change sprintf to Format
        char buffer[512]; 
        if (pRenderThread->IsSingleThreaded())
        {
            SF::SFsprintf(buffer, sizeof(buffer),
                "%s - FPS:%d (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f - Kit: Objs=%d, Update: %.5f, MMUpdate: %.5f",
                GetAppTitle().ToCStr(),
                (int)LastFPS,
                rstats.Meshes, rstats.Primitives, rstats.Triangles,
                double(LastAdvanceTicks)/1000.0,
                double(displayTicks)/1000.0,
                double(otherTicks)/1000.0,
                HUDstats.MinimapStats.PushedObjects, (double)HUDUpdateTimeAccumulator * invDiv, (double)MMUpdateTimeAccumulator * invDiv);
        }
        else
        {
            // TBD: Add Render thread FPS once they are de-synchronized.
            //" - A_FPS:%d, R_FPS:%d  (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f",

            SF::SFsprintf(buffer, sizeof(buffer),
                "%s MT - FPS:%d (M=%d, P=%d, T=%d) A=%.3f, D=%.3f, O=%.3f - Kit: Objs=%d, Update: %.5f, MMUpdate: %.5f",
                GetAppTitle().ToCStr(),
                (int)LastFPS, // LastDisplayFrames,
                rstats.Meshes, rstats.Primitives,rstats.Triangles,
                double(LastAdvanceTicks)/1000.0,
                double(displayTicks)/1000.0,
                double(otherTicks)/1000.0,
                HUDstats.MinimapStats.PushedObjects, (double)HUDUpdateTimeAccumulator * invDiv, (double)MMUpdateTimeAccumulator * invDiv);
        }
        strTitle = buffer;
    }    

    if (strTitle.IsEmpty())
    {
        strTitle = GetAppTitle();
        if (pRenderThread->IsSingleThreaded())
            strTitle += " ST ";
        else
            strTitle +="  MT ";
    }

    if (ViewConfigHasFlag(Platform::View_VSync))
        strTitle += " VSync";

#ifdef SF_AMP_SERVER
    String portString;
    if (AmpServer::GetInstance().IsValidConnection())
    {
        Format(portString, " Connected to AMP ({0})", AmpPort);
    }
    else if (AmpServer::GetInstance().IsEnabled())
    {
        Format(portString, " Listening for AMP ({0})", AmpPort);
    }
    strTitle += portString;
#endif

    SetWindowTitle(strTitle);
}

bool FxPlayerApp::LoadDefaultFontConfigFromPath(const String& path)
{
    SF_UNUSED(path);
    String fontPath;

#ifdef SF_OS_XBOX360
    fontPath = "D:\\Localization\\";
#elif defined SF_OS_PS3
    fontPath = "/app_home/Localization//";
#else
    fontPath = "Localization/";
#endif

#ifdef SF_BUILD_SHIPPING
    fontPath += "fontconfig_shipping.txt";
#else
    fontPath += "fontconfig.txt";
#endif

    // parse the config file
    ConfigParser parser(fontPath);
    int oldIdx = FontConfigIndex;
    LoadFontConfigs(&parser);

    if ((FontConfigIndex == 0) &&
        (oldIdx != -1) )
    {
        FontConfigIndex = oldIdx;
        FontConfigIndex %= (int)FontConfigs.GetSize();
    }

    return true;
}

void FxPlayerApp::OnDropFiles(const String& path)
{
    SF_UNUSED(path);
}

void FxPlayerApp::OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag)
{

    int inputKeyCode = -1;
    switch(keyCode)
    {
    case Pad_Left:
        inputKeyCode = Key::A;
        break;
    case Pad_Right:
        inputKeyCode = Key::D;
        break;
    case Pad_Up:
        inputKeyCode = Key::W;
        break;
    case Pad_Down:
        inputKeyCode = Key::S;
        break;
    case Pad_R2:
        inputKeyCode = Key::Space;
        break;
    case Pad_L2:
        inputKeyCode = Key::G;
        break;
    case Pad_X:
    case Pad_S:
        inputKeyCode = Key::R;
        break;
    case Pad_Y:
    case Pad_T:
        inputKeyCode = Key::Q;
        break;
    default: 
        break;
    }

    // PPS: This scheme is setup for the Wiimote in horizontal configuration
    // PPS: The WiiU requires DRC and is handled above
#if defined SF_OS_WII
    switch(keyCode)
    {
    case Pad_Left:
        inputKeyCode = Key::S;
        break;
    case Pad_Right:
        inputKeyCode = Key::W;
        break;
    case Pad_Up:
        inputKeyCode = Key::A;
        break;
    case Pad_Down:
        inputKeyCode = Key::D;
        break;
    case Pad_1:
        inputKeyCode = Key::Space;
        break;
    case Pad_2:
        inputKeyCode = Key::R;
        break;
    case Pad_A:
        inputKeyCode = Key::G;
        break;
    case Pad_B:
        inputKeyCode = Key::Q;
        break;
    default: 
        break;
    }

#endif
	
    if (inputKeyCode > -1)
    {
        ProcessHUDKeyEvent((KeyCode)inputKeyCode, downFlag);
    }
    else
    {
        FxPlayerAppBase::OnPad(controllerIdx, keyCode, downFlag);
    }
}

void FxPlayerApp::OnKey(unsigned controllerIndex, KeyCode keyCode,
                              unsigned wcharCode, bool downFlag, KeyModifiers mods)
{
    if (mods.IsCtrlPressed() || !ProcessHUDKeyEvent(keyCode, downFlag))
    {
        FxPlayerAppBase::OnKey(controllerIndex, keyCode, wcharCode, downFlag, mods);
    }
}
