/**************************************************************************

Filename    :   MMOKitDemo.cpp
Content     :   Sample SWF/GFX file player for MMO Kit leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Maxim Didenko, Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../Common/FxPlayerAppBase.h"

#include "FxHUD.h"

#include "GFx/AS3/AS3_Global.h"

#ifdef SF_OS_WIN32
#include <tchar.h>
#endif

#include "Game.h"


class   MMOKitApp : public FxPlayerAppBase, public FxHUDInfoProvider
{
public:
    MMOKitApp();
    // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
    // Invoked when HUD becomes active/inactive
    virtual void            OnHudStatusChange(bool active) { SF_UNUSED(active); }   

    virtual String          GetAppTitle() const;
    virtual String          GetFilePath() const;

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            SetFileOpener();
    virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnShutdown();

#ifdef SF_AMP_SERVER
	virtual void			GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const;
#endif

    void                    UpdateStatReports();
    virtual void            UpdateViewSize();
    void                    UpdateHUDViewport();
   
    virtual void            InitArgDescriptions(Platform::Args* args);
    
	virtual bool            LoadMovie(const String& filename);

    // No system font
    virtual void            SetFontProvider()   {}

    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);
        
    bool                    RedirectEventsToHud;
    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;

    Hash<UInt32, Ptr<FxPlayerCommand> >     HudKeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > HudPadKeyCommandMap;

    // *** Begin MMO Kit

    MMOGame                    TheGame;

    // *** End MMO Kit
};

class FxPlayerCommandHudReport : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        MMOKitApp* playerApp = (MMOKitApp*)app;
        if (playerApp->pHudDataProvider)
        {
            if (playerApp->GetArgs().HasValue("RecordStatistics"))
            {
                playerApp->pHudDataProvider->SaveStatistics(playerApp->GetArgs().GetString("RecordStatistics"), playerApp->GetArgs().GetBool("XmlLog"));
            }
            else
            {
                playerApp->pHudDataProvider->PrintStatistics(playerApp->GetArgs().GetBool("XmlLog"));
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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
        MMOKitApp* playerApp = (MMOKitApp*)app;
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

MMOKitApp::MMOKitApp()
{    
	UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);

	// *** Begin MMO Kit

	KeyCommandMap.Remove(Key::F3 | ctrlMask);
	KeyCommandMap.Remove(Key::F4 | ctrlMask);
	KeyCommandMap.Remove(Key::F5 | ctrlMask);
    KeyCommandMap.Remove(Key::R | ctrlMask);

    // *** End MMO Kit

    RedirectEventsToHud = false;
    pHudHeap = NULL;


    KeyCommandMap.Set(Key::F5, *SF_NEW FxPlayerCommandHudReport());
    KeyCommandMap.Set(Key::F1, *SF_NEW FxPlayerCommandInfoHelp());
    KeyCommandMap.Set(Key::F2, *SF_NEW FxPlayerCommandInfoSummary());
    KeyCommandMap.Set(Key::F12, *SF_NEW FxPlayerCommandHudClose());
    KeyCommandMap.Set(Key::H | ctrlMask, *SF_NEW FxPlayerCommandHudNextTab());

#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32)
    PadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudOpen());
    HudPadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudClose());
    HudPadKeyCommandMap.Set(Pad_L1, *SF_NEW FxPlayerCommandHudPrevTab());
    HudPadKeyCommandMap.Set(Pad_R1, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PS3) || defined(SF_OS_PS2)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudOpen());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudClose());
    HudPadKeyCommandMap.Set(Pad_L1, *SF_NEW FxPlayerCommandHudPrevTab());
    HudPadKeyCommandMap.Set(Pad_R1, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PSP)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudOpen());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudClose());
    HudPadKeyCommandMap.Set(Pad_O, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_WII)
    PadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudOpen());
    HudPadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudClose());
    HudPadKeyCommandMap.Set(Pad_2, *SF_NEW FxPlayerCommandHudNextTab());
#endif

}

void MMOKitApp::GetExtraInfo(FxHUDExtraInfo* pinfo) 
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

void MMOKitApp::GetHelpStr(String* phelpStr)
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

    if (IsPadConnected(0))
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

//------------------------------------------------------------------------


String MMOKitApp::GetAppTitle() const
{
    String title("Scaleform MMO Kit Demo v ");

    // Version
    title += GFX_VERSION_STRING;

    // Add the word "Debug" to the application
    // title if in debug build mode
#ifdef SF_BUILD_DEBUG
    title += " " SF_BUILD_STRING;
#endif

    return title;
}

String MMOKitApp::GetFilePath() const
{
    String filePath;

#if defined (NDEBUG) && !defined (SF_OS_PS3)
    filePath = "data/MMOKit.gfx";
#elif defined (NDEBUG) && defined (SF_OS_PS3)
    filePath = "/app_home/MMO/data/MMOKit.gfx"; 
#elif !defined (NDEBUG) && !defined (SF_OS_PS3)
    filePath = "MMOKit.swf";
#elif !defined (NDEBUG) && defined (SF_OS_PS3)
    filePath = "/app_home/MMO/MMOKit.swf";
#endif

    return filePath;
}

static String  BuildPath(const char* pfilename)
{
    String path;
#if defined(SF_OS_XBOX360)
    path = "D:\\";
#elif defined(SF_OS_PS3)
    path = "/app_home/MMO/";
#elif defined(SF_OS_WIIU)
    path = "/vol/content/Kits/MMO/";
#endif
    path.AppendString(pfilename);
    return path;
}

// File opener class.
class MMOKitFileOpener : public FileOpener 
{
public:
    virtual File* OpenFile(const char *purl, int flags, int mode) 
    {
#ifdef GFX_ENABLE_VIDEO
        if (!strcmp(purl, "_internal_video_player"))
            return new MemoryFile(purl, fxvideoplayer_swf, sizeof(fxvideoplayer_swf));
#endif
    
        String path;
        // PPS: We need to build path for external assets (icons)
        if (strstr(purl, "icons"))
        {
            path = BuildPath(purl);
        }
        else
        {
            path = purl;
        }

        // Buffered file wrapper is faster to use because it optimizes seeks.
        return FileOpener::OpenFile(path, flags, mode);
    }
};

void MMOKitApp::SetFileOpener()
{
    FxPlayerAppBase::SetFileOpener();

    // File callback.
    Ptr<FileOpener> pfileOpener = *new MMOKitFileOpener;	
    mLoader.SetFileOpener(pfileOpener);
}

bool MMOKitApp::OnInit(Platform::ViewConfig& config)
{
    if (!FxPlayerAppBase::OnInit(config))
        return false;

    // *** Begin MMO Kit

    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);
    // No need for previous/next file
    KeyCommandMap.Remove(Key::BracketLeft | ctrlMask);
    KeyCommandMap.Remove(Key::BracketRight | ctrlMask);
    // No need for prev/next frame
    KeyCommandMap.Remove(Key::Left | ctrlMask);
    KeyCommandMap.Remove(Key::Right | ctrlMask);
    // No need for pause file
    KeyCommandMap.Remove(Key::P | ctrlMask);

#if defined SF_OS_WIN32
    if (!pMovie)
    {
        char temp[128];
        sprintf_s(temp, 128, "Failed loading %s!\nPlease check your working directory and try again.\n", FileName.ToCStr());
        MessageBoxA(NULL, temp, _T("MMOKit Error"), MB_OK);
        
        TheGame.Shutdown();
        FxPlayerAppBase::OnShutdown();
        return false;
    }
#elif defined SF_OS_PS3
    if (!pMovie)
    {
        SF_DEBUG_MESSAGE1(1, "Failed loading %s!\nPlease check your working directory and try again.\n", FileName.ToCStr());
        TheGame.Shutdown();
        FxPlayerAppBase::OnShutdown();
        return false;
    }
#endif


    TheGame.Initialize(pMovie, new GameUIRuntimeAtlasManager(pRenderThread->GetTextureManager()));

    // *** End MMO Kit

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
void			MMOKitApp::GetAmpServerCapabilities(GFx::AMP::MessageAppControl &caps) const
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

void    MMOKitApp::OnUpdateFrame(bool needRepaint)
{
    TheGame.Update();

    FxPlayerAppBase::OnUpdateFrame(needRepaint);
}

void MMOKitApp::OnShutdown()
{
    TheGame.Shutdown();

    FxPlayerAppBase::OnShutdown();

    if (pHudDataProvider && GetArgs().HasValue("RecordStatistics"))
        pHudDataProvider->SaveStatistics(GetArgs().GetString("RecordStatistics"), GetArgs().GetBool("XmlLog"));
  
    // Releasing the movie will automatically shut down its
    // background loading task, if any.
    pHud.Clear();
    pHudDataProvider.Clear();
    pHudHeap->Release();
}

void MMOKitApp::UpdateStatReports()
{
    FxPlayerAppBase::UpdateStatReports();

    if (pHud && (!pMovie || pHud->IsVisible() /* || SafeRectEnabled*/))
    {   				
        pHud->Update();
        pHudDataProvider->UpdateRendererStats();
        pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks,
                                                 pRenderThread->GetLastDisplayTicks(),
                                                 AccumFPS / AccumFPSSecondCount);
    }
}

void MMOKitApp::UpdateViewSize() 
{
    FxPlayerAppBase::UpdateViewSize();
    UpdateHUDViewport();
}

void MMOKitApp::UpdateHUDViewport()
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



FxPlayerCommand* MMOKitApp::PadKeyToCommand(PadKeyCode key)
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
    else
    {
        it = PadKeyCommandMap.Find(key);
        if (it != PadKeyCommandMap.End())
        {
            return it->Second;
        }
    }
    return NULL;
}


void MMOKitApp::InitArgDescriptions(Platform::Args* args)
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

bool MMOKitApp::LoadMovie(const String& filename)
{
	bool rv = FxPlayerAppBase::LoadMovie(filename);
	if (pHudDataProvider)
	{
		pHudDataProvider->SetMovie(pMovie);
	}
	if (pHud)
	{
		pHud->ClearMsgText();
		pHud->Update();
	}
	return rv;
}

//------------------------------------------------------------------------
// ***** Main APP implementation

SF_PLATFORM_SYSTEM_APP(MMOKit, Scaleform::GFx::System, MMOKitApp)
