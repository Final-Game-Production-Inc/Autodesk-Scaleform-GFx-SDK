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

#include "../Common/FxPlayerAppBase.h"
#include "FxHUD.h"

#include "GFx/AS3/AS3_Global.h"


class   FxPlayerApp : public FxPlayerAppBase, public FxHUDInfoProvider
{
public:
    FxPlayerApp();
    // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
    // Invoked when HUD becomes active/inactive
    virtual void            OnHudStatusChange(bool active) { SF_UNUSED(active); }   

    virtual bool            OnInit(Platform::ViewConfig& config);
 //   virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnShutdown();

    virtual void            UpdateStatReports();
    virtual void            UpdateFpsDisplay();
    virtual void            UpdateViewSize();
    void                    UpdateHUDViewport();
   
    virtual void            InitArgDescriptions(Platform::Args* args);
    
	virtual bool            LoadMovie(const String& filename);

    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);
        
    void                    UpdatePerformanceStats();

    virtual void            InstallHandlers();

    bool                    RedirectEventsToHud;
    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;

    Hash<UInt32, Ptr<FxPlayerCommand> >     HudKeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > HudPadKeyCommandMap;

};

class DemoImageCreator : public GFx::ImageCreator
{
    FxPlayerApp *pApp;
public:
    DemoImageCreator(Render::TextureManager* texMgr, FxPlayerApp *papp) : GFx::ImageCreator(texMgr), pApp(papp) {SF_ASSERT(pApp);}
    virtual ~DemoImageCreator(){}
    virtual  Render::Image* LoadProtocolImage(const GFx::ImageCreateInfo& info, const String& url)
    {
        String path = pApp->FileName;
        if ( !URLBuilder::ExtractFilePath(&path) )
        {
            path = "";
        }
        String filename = url;
        filename.Remove(0,6); //remove img://
        path = path + filename;
        Render::ImageFileHandlerRegistry* pregistry = info.GetImageFileHandlerRegistry();
        return pregistry ? pregistry->ReadImage<SysFile>(path) : 0;
    }
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

class FxPlayerCommandToggleStickMode : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        app->StickMode++;
        app->StickMode %= FxPlayerAppBase::PSM_Count;
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        if (playerApp->pHud)
        {
            playerApp->pHud->SetStickMode(app->StickMode);
        }
    }
    virtual String GetHelpString() const
    {
        return "Toggle Pad Stick Mode";
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
 
FxPlayerApp::FxPlayerApp()
{    
    RedirectEventsToHud = false;
    pHudHeap = NULL;

    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);

    KeyCommandMap.Set(Key::F5, *SF_NEW FxPlayerCommandHudReport());
    KeyCommandMap.Set(Key::F1, *SF_NEW FxPlayerCommandInfoHelp());
    KeyCommandMap.Set(Key::F2, *SF_NEW FxPlayerCommandInfoSummary());
    KeyCommandMap.Set(Key::K | ctrlMask, *SF_NEW FxPlayerCommandToggleStickMode());
    KeyCommandMap.Set(Key::F12, *SF_NEW FxPlayerCommandHudClose());
    KeyCommandMap.Set(Key::H | ctrlMask, *SF_NEW FxPlayerCommandHudNextTab());

#if defined(SF_OS_XBOX360) || defined(SF_OS_WIN32) || defined(_DURANGO)
    PadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PS3) || defined(SF_OS_PS2) || defined(SF_OS_ORBIS)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_PSP)
    PadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_S, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_WII)
    PadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
#elif defined(SF_OS_WIIU)
    PadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_X, *SF_NEW FxPlayerCommandHudNextTab());
    PadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
    HudPadKeyCommandMap.Set(Pad_1, *SF_NEW FxPlayerCommandHudNextTab());
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

//------------------------------------------------------------------------

bool FxPlayerApp::OnInit(Platform::ViewConfig& config)
{
    // Skip loading the movie until we have overriden the ImageCreator
    bool skipLoadMovie = SkipLoadMovie;
    SkipLoadMovie = true;
    if (!FxPlayerAppBase::OnInit(config))
        return false;
    SkipLoadMovie = skipLoadMovie;

    if (pRenderThread)
    {
        Ptr<ImageCreator> pimageCreator = * SF_NEW DemoImageCreator(pRenderThread->GetTextureManager(), this);
        mLoader.SetImageCreator(pimageCreator);
    }

    // Now load the movie, if needed
    if (!SkipLoadMovie && !FileName.IsEmpty())
    {
        LoadMovie(FileName);
    }

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

void FxPlayerApp::OnShutdown()
{
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
    }

    // Releasing the movie will automatically shut down its
    // background loading task, if any.
    pHud.Clear();
    pHudDataProvider.Clear();
    pHudHeap->Release();

    FxPlayerAppBase::OnShutdown();
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
    pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks, pRenderThread ? pRenderThread->GetLastDisplayTicks() : 0, fps);
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

#if 0
class CustomExternalInterface : public GFx::ExternalInterface
{
public:
    virtual void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount)
    {

    }
};

void FxPlayerApp::InstallHandlers()
{
    FxPlayerAppBase::InstallHandlers();

    Ptr<CustomExternalInterface> xi = *new CustomExternalInterface;
    pMovie->SetExternalInterface(xi);
}
#else
void FxPlayerApp::InstallHandlers()
{
    FxPlayerAppBase::InstallHandlers();
}
#endif

bool FxPlayerApp::LoadMovie(const String& filename)
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

SF_PLATFORM_SYSTEM_APP(FxPlayer, Scaleform::GFx::System, FxPlayerApp)

/* 
// SysAllocStatic allocator   
//---------------------------
static const int allocSize = 1024 * 1024 * 128;
static UByte allocBuffer[allocSize];
static SysAllocStatic sysAlloc;
static GFx::System* gfxSystem;

extern int Platform_WinAPI_MainA(int argc, char** argv);
static Scaleform::Platform::SystemInitWrapper<Scaleform::GFx::System> SF_SystemClassInstance;

Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance()
{
    sysAlloc.AddMemSegment(allocBuffer, allocSize);
    gfxSystem = new GFx::System(&sysAlloc);
    return new FxPlayerApp();
}

void Scaleform::Platform::AppBase::DestroyInstance(AppBase* app)
{
    delete app;
    delete gfxSystem;
}

int main(int argc, char** argv)
{ 
    Platform_WinAPI_MainA(argc, argv); 
}
//---------------------------
*/


/*
// SysAllocWinAPI allocator
//---------------------------
static SysAllocWinAPI sysAlloc;
static GFx::System* gfxSystem;

extern int Platform_WinAPI_MainA(int argc, char** argv);
static Scaleform::Platform::SystemInitWrapper<Scaleform::GFx::System> SF_SystemClassInstance;

Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance()
{
    gfxSystem = new GFx::System(&sysAlloc);
    return new FxPlayerApp();
}

void Scaleform::Platform::AppBase::DestroyInstance(AppBase* app)
{
    delete app;
    delete gfxSystem;
}

int main(int argc, char** argv)
{ 
    Platform_WinAPI_MainA(argc, argv); 
}
//---------------------------
*/


/*
// Direct VirtualAlloc / VirtualFree (extreme debugging only)
// Uncomment SF_MEMORY_FORCE_MALLOC
//---------------------------
static SysAllocWinAPI sysAlloc;
static GFx::System* gfxSystem;
static HeapPT_SysAlloc sysMapper;

extern int Platform_WinAPI_MainA(int argc, char** argv);
static Scaleform::Platform::SystemInitWrapper<Scaleform::GFx::System> SF_SystemClassInstance;

Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance()
{
    sysMapper.Init();
    gfxSystem = new GFx::System(&sysAlloc);
    Memory::GetGlobalHeap()->pSysMapper = &sysMapper;
    return new FxPlayerApp();
}

void Scaleform::Platform::AppBase::DestroyInstance(AppBase* app)
{
    delete app;
    delete gfxSystem;
}

int main(int argc, char** argv)
{ 
    Platform_WinAPI_MainA(argc, argv); 
}
//---------------------------
*/