/**************************************************************************

Filename    :   FxPlayerTextureInSWFWin32.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Andrew Reise

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "../Common/FxPlayerAppBase.h"
#include "../FxPlayer/FxHUD.h"
#include "GFx/AS3/AS3_Global.h"


#if defined(FXPLAYER_RENDER_DIRECT3D)
#if SF_D3D_VERSION < 10
#define APP_BASE  TextureInSWFD3D9App
#include "Render/D3D9/D3D9_HAL.h"
#include "d3dx9math.h"
#include "FxPlayerTextureInSWFD3D9.h"
#else 
#define APP_BASE  TextureInSWFD3D1xApp
//#include "Render/D3D1x/D3D1x_HAL.h"
#include "FxPlayerTextureInSWFD3D1x.h"
#endif
class DrawFrameThreadCommand;



class RenderMeshThreadCommand;

class   FxPlayerApp : public APP_BASE, public FxHUDInfoProvider
{
public:
    FxPlayerApp();
    // Used by HUD to display statistics
    virtual void            GetExtraInfo(FxHUDExtraInfo* pinfo);
    virtual void            GetHelpStr(String* phelpStr);
    // Invoked when HUD becomes active/inactive
    virtual void            OnHudStatusChange(bool active) { SF_UNUSED(active); }   

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnShutdown();       

    virtual void            UpdateStatReports();
    virtual void            UpdateFpsDisplay();
    virtual void            UpdateViewSize();
    void                    UpdateHUDViewport();
   
    virtual void            InitArgDescriptions(Platform::Args* args);
    
	virtual bool            LoadMovie(const String& filename);


    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);

    virtual void OnHALEvent(HALNotifyType type);

    bool                    RedirectEventsToHud;
    Ptr<FxHUDDataProvider>  pHudDataProvider;
    Ptr<FxHUD>              pHud;
    MemoryHeap*             pHudHeap;

    Hash<UInt32, Ptr<FxPlayerCommand> >     HudKeyCommandMap;
    Hash<PadKeyCode, Ptr<FxPlayerCommand> > HudPadKeyCommandMap;


    Ptr<RenderMeshThreadCommand>    pRenderMeshCommand;
};

class RenderMeshThreadCommand : public ThreadCommand
{
public:
    RenderMeshThreadCommand(FxPlayerApp* papp) : pApp(papp) { }
    virtual void Execute()
    {
        pApp->RenderMesh();
    }
    FxPlayerApp* pApp;
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
 
FxPlayerApp::FxPlayerApp()
{    
    pRenderMeshCommand  = *SF_HEAP_AUTO_NEW(this) RenderMeshThreadCommand(this);
    RedirectEventsToHud = false;
    pHudHeap = NULL;

    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);

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

void FxPlayerApp::GetExtraInfo(FxHUDExtraInfo* pinfo) 
{ 
    pinfo->LastFPS = LastFPS;
    pinfo->TessTriangles = 0; //TessTriangles;
    pinfo->Zoom = Zoom;   
    pinfo->Paused = Paused;
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

bool FxPlayerApp::OnInit(Platform::ViewConfig& config)
{
    if (!FxPlayerAppBase::OnInit(config))
        return false;

    APP_BASE::OnInit(config);

    SetupRTTexture();


    //D3D9::TextureManager * pmanager = (D3D9::TextureManager*)pPlatformHAL->GetTextureManager(); 

    //Ptr<D3D9::Texture> pMyHWTexture = *(D3D9::Texture*)pmanager->CreateTexture( pRenderTexture, ImageSize(RTWidth, RTHeight));
    //Ptr<TextureImage> pti = * SF_NEW TextureImage(Image_R8G8B8, pMyHWTexture->GetSize(), 0, pMyHWTexture);


    //Ptr<DemoImageCreator> pimageCreator = * SF_NEW DemoImageCreator(pmanager, pti);
    //mLoader.SetImageCreator(pimageCreator);

    FileName = "Window_Texture.swf";
    
    String filenameStr = FileName;
    String argString;

    // Try to load the new movie
    Ptr<MovieDef> pnewMovieDef;
    Ptr<Movie> pnewMovie;
    MovieInfo newMovieInfo;

    MovieCursorType = Platform::Cursor_Arrow;
    SetCursor(MovieCursorType);
    ResetUserMatrix();

    // Get info about the width & height of the movie.
    if (!mLoader.GetMovieInfo("AS2\\Samples\\RenderTexture\\" + filenameStr, &newMovieInfo, 0, Loader::LoadKeepBindData)) 
    {
        if (!mLoader.GetMovieInfo(filenameStr, &newMovieInfo, 0, Loader::LoadKeepBindData))
        {
            fprintf(stderr, "Error: Failed to get info about %s\n", filenameStr.ToCStr());
#ifdef SF_OS_WIN32
            String errorString = "Unable to load  " + filenameStr;
            MessageBoxA(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
            return 0;
        }

    }
    else
    {
        filenameStr = "AS2\\Samples\\RenderTexture\\" + filenameStr;
    }

    unsigned loadConstants = Loader::LoadAll;
	const Platform::Args& args = GetArgs();
    /*
    Ptr<ImageCreator> imageCreator = *new CustomImageCreator;
    mLoader.SetImageCreator(imageCreator);
    */

    // Set ActionScript verbosity / etc.
    Ptr<ActionControl> pactionControl = *new ActionControl();
    pactionControl->SetVerboseAction(args["VerboseActions"]->GetBool());
#ifdef GFC_NO_FXPLAYER_VERBOSE_ACTION
    if (args["VerboseActions"]->GetBool())
        fprintf(stderr, "VerboseAction is disabled by the GFC_NO_FXPLAYER_VERBOSE_ACTION macro in GFxConfig.h\n");
#endif

    pactionControl->SetActionErrorSuppress(args["SuppressASErrors"]->GetBool());
    pactionControl->SetLogRootFilenames(args["LogRootFilenames"]->GetBool());
    pactionControl->SetLogChildFilenames(!args["NoLogChildFilnames"]->GetBool());
    pactionControl->SetLongFilenames(args["LogFilePath"]->GetBool());

    // Load the actual new movie and crate instance.
    // Don't use library: this will ensure that the memory is released.
    pnewMovieDef = *mLoader.CreateMovie(filenameStr, loadConstants
                                                                |Loader::LoadWaitCompletion);
    if (!pnewMovieDef) 
    {
        fprintf(stderr, "Error: Failed to create a movie from '%s'\n", filenameStr.ToCStr());
        return 0;
    }

    pnewMovie = *pnewMovieDef->CreateInstance(MemoryParams(), false, pactionControl);
    if (!pnewMovie) 
    {
        fprintf(stderr, "Error: Failed to create movie instance\n");
        return 0;
    }

    if (pnewMovie)
    {
        Resource*      pres = pnewMovie->GetMovieDef()->GetResource("texture1");
        ImageResource* pimageRes = 0;
        if (pres && pres->GetResourceType() == Resource::RT_Image)
            pimageRes = (ImageResource*)pres;


        if (pimageRes)
        {
            ImageBase* pimageOrig = pimageRes->GetImage();
            if (pimageOrig)
            {
                pMyHWTexture = *CreateHWTexture();
                Ptr<TextureImage> pti = * SF_NEW TextureImage(Image_R8G8B8, pMyHWTexture->GetSize(), 0, pMyHWTexture);

                // If the texture is different size than the image it replaces, set the image matrix to their relative scales.
                ImageSize originalSize = pimageOrig->GetSize();
                ImageSize newSize      = pMyHWTexture->GetSize();
                Size<float> scaleParameters(((float)originalSize.Width) / newSize.Width, ((float)originalSize.Height) / newSize.Height);
                Matrix2F textureMatrix = Matrix2F::Scaling(scaleParameters.Width, scaleParameters.Height );
                pti->SetMatrix(textureMatrix);

                pimageRes->SetImage(pti);
            }
        }
    }
    // If this succeeded, replace the old movie with the new one.
    pMovieDef = pnewMovieDef;
    pMovie = pnewMovie;
    memcpy(&mMovieInfo, &newMovieInfo, sizeof(MovieInfo));

    FileName = filenameStr;

    pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle());
    pRenderThread->ResetRasterizationCount();
    // Release cached memory used by previous file.
//     if (pMovie->GetMeshCacheManager())
//         pMovie->GetMeshCacheManager()->ClearCache();

    // Set a reference to the app
    pMovie->SetUserData(this);

    if (IsCursorEnabled())    
        pMovie->SetMouseCursorCount(GetMouseCount());    
    pMovie->SetControllerCount(GetKeyboardCount());

    SetMovieVariables(argString);

    InstallHandlers();

    //!AB, set active movie..this causes OnMovieFocus to get called on the IMEManager
    pMovie->HandleEvent(GFx::Event::SetFocus);

    // init first frame
    pMovie->Advance(0.0f, 0);

    // Renderer
    if (Rendering) 
    {
        pMovie->SetEdgeAAMode((AAMode == AAMode_EdgeAA) ?
                              Render::EdgeAA_On : Render::EdgeAA_Disable);
    }

    if (DoSound) 
    { 
        // No built-in sound support currently in GFx. Customers
        // can play their own sound through fscommand() callbacks.
    }

    // Disable pause.
    Paused = false;
    AdjustFrameTime();    
    AccumClockTicks = 0;
    LastFPSUpdatedTicks = 0;
    LastStatUpdatedTicks = 0;
    PrevTicks = Timer::GetTicks();

    // Reset FPS counters used for FPS averaging.
    FrameCounter = 0;    
    LastFPS = 0.0f;
    AccumFPS = 0.0f;
    AccumFPSSecondCount = 0;

    // Update the view
    UpdateViewSize();
    AmpDirty = true;

    APP_BASE::CreateVertexBuffer();

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

void FxPlayerApp::OnUpdateFrame( bool needRepaint )
{
    pRenderThread->PushThreadCommand(pRenderMeshCommand);    
    FxPlayerAppBase::OnUpdateFrame(needRepaint);
}




void FxPlayerApp::OnShutdown()
{
    APP_BASE::OnShutdown();
    pRenderMeshCommand = 0;

    if (pHudDataProvider)
    {
        bool xmlLog = GetArgs().GetBool("XmlLog");
        float fps = AccumFPSSecondCount > 0 ? AccumFPS / AccumFPSSecondCount : 0.0f;
        pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks, pRenderThread->GetLastDisplayTicks(), fps);
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
        pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks,
                                                 pRenderThread->GetLastDisplayTicks(), AccumFPS / AccumFPSSecondCount);
    }
}

void FxPlayerApp::UpdateFpsDisplay()
{
    if (GetArgs().GetBool("LogFPS"))
    {
        if (pHudDataProvider)
        {
            float fps = AccumFPSSecondCount > 0 ? AccumFPS / AccumFPSSecondCount : 0.0f;
            pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks, pRenderThread->GetLastDisplayTicks(), fps);
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

void FxPlayerApp::OnHALEvent(HALNotifyType type)
{
    switch(type)
    {
    case HAL_PrepareForReset:
#if (SF_D3D_VERSION == 9)
        pRenderTexture = 0;
        pStencilSurface = 0;
#endif
        break;
    case HAL_RestoreAfterReset:
#if (SF_D3D_VERSION == 9)
        SetupRTTexture();
        pMyHWTexture->Initialize(pRenderTexture.GetRawRef());
#endif
        break;
    default:
        break;
    }
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

#else

Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance()
{
    fprintf(stderr, "Unsupported platform or renderer.\n");
    return NULL;
}

void Scaleform::Platform::AppBase::DestroyInstance(AppBase* app)
{
    SF_UNUSED(app);
}
#if  defined(SF_OS_WIN32)
    extern int Platform_WinAPI_MainA(int argc, char** argv);
    int main(int argc, char** argv) { Platform_WinAPI_MainA(argc, argv); }
#endif


#endif  // defined(FXPLAYER_RENDER_DIRECT3D) && (SF_D3D_VERSION < 10)
