/**************************************************************************

Filename    :   MenuKitDemo.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Maxim Didenko, Boris Rayskiy,

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../Common/FxPlayerAppBase.h"

#include "GFx/AS3/AS3_Global.h"

// *** MK Begin
#include "Game.h"
// *** MK End

class   MenuKitApp : public FxPlayerAppBase
{
public:
    MenuKitApp();

    virtual String          GetAppTitle() const;

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);

    // Input
    virtual void            OnKey(unsigned controllerIndex, KeyCode keyCode,
                                  unsigned wcharCode, bool downFlag, KeyModifiers mods);
    virtual void            OnPad(unsigned controllerIndex, PadKeyCode padCode, bool downFlag);   
    virtual void            OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
                                          const Point<int>& mousePos, KeyModifiers mods);    
    virtual void            OnMouseMove(unsigned mouseIndex,
                                        const Point<int>& mousePos, KeyModifiers mods);
    virtual void            OnMouseWheel(unsigned mouseIndex, float zdelta,
                                         const Point<int>& mousePos, KeyModifiers mods);

    virtual void            OnPause(bool paused);

    virtual bool            AdvanceMovie(float deltaT);

    virtual void            UpdateViewSize();

    // No system font
    virtual void            SetFontProvider()   {}
   
    virtual void            InitArgDescriptions(Platform::Args* args);
    
//    virtual FxPlayerCommand* PadKeyToCommand(PadKeyCode key);
    virtual String          GetFilePath() const { return ""; }

    // *** MK Begin
    Game                    TheGame;
    // *** MK End

};


MenuKitApp::MenuKitApp() : FxPlayerAppBase()
{    
}

//------------------------------------------------------------------------

String MenuKitApp::GetAppTitle() const
{
    String title("Scaleform Menu Kit Demo v ");

    // Version
    title += GFX_VERSION_STRING;

    // Add the word "Debug" to the application
    // title if in debug build mode
#ifdef SF_BUILD_DEBUG
    title += " " SF_BUILD_STRING;
#endif

    return title;
}

bool MenuKitApp::OnInit(Platform::ViewConfig& config)
{
    // *** MK Begin

    UInt32 ctrlMask = (KeyModifiers::Key_CtrlPressed << 16);
    // No need for previous/next file
    KeyCommandMap.Remove(Key::BracketLeft | ctrlMask);
    KeyCommandMap.Remove(Key::BracketRight | ctrlMask);
    // No need for scaled display
    KeyCommandMap.Remove(Key::S | ctrlMask);

    // Enabled measure perf mode
    MeasurePerformance = 1;

    ArgResolution.SetSize(1280, 720);
    //ArgResolution.SetSize(1024, 768);

    // Make the background color black
    BackgroundColor     = 0x000000;

    // *** MK End

    if (!FxPlayerAppBase::OnInit(config))
        return false;

    // *** MK Begin
    
    bool ok = false;
#ifdef GFX_ENABLE_SOUND
    ok = TheGame.Initialize(pRenderThread, GetSoundRenderer());
#else
    ok = TheGame.Initialize(pRenderThread);
#endif // GFX_ENABLE_SOUND
    if (!ok) 
    {
        OnShutdown();
        return false;
    }

    Ptr<GFx::UserEventHandler> pueh = *new FxPlayerUserEventHandler(this);
    TheGame.SetGFxUserEventHandler(pueh);
    TheGame.UpdateViewSize(GetViewSize());

    // *** MK End

    return true;
}

void MenuKitApp::OnPause(bool paused)
{
    // Overriden
    SF_UNUSED(paused);
}

bool MenuKitApp::AdvanceMovie(float deltaT)
{
    SF_UNUSED(deltaT);

    TheGame.UpdateViewSize(GetViewSize());
    UInt64 startAdvanceTicks = Timer::GetProfileTicks();
    GameUIAdvanceStats  advStats;
    advStats.AdvanceTicks = 0;
    TheGame.Update(&advStats, startAdvanceTicks);
    if (!TheGame.IsRunning()) { return false; }
    LastAdvanceTicks = Timer::GetProfileTicks() - startAdvanceTicks;

    return true;
}

void MenuKitApp::OnUpdateFrame(bool needRepaint)
{
    // TBD: Make use of 'needRepaint' once refresh is conditional on change.
    SF_UNUSED(needRepaint);

#if !defined (SF_BUILD_SHIPPING)
    HandleAmpAppMessages();
#endif

    // Update timing.
    UInt64 timer = Timer::GetTicks();
    UInt64 delta = timer - PrevTicks;
    PrevTicks = timer;
    AccumClockTicks += delta;

    if (Paused && Rendering)
        delta = 0;
    else if (FastForward || !Rendering)
        delta = (UInt64)(1000000.0f / mMovieInfo.FPS);

    // Check to see if exit timeout was hit.
    if ((ExitTimeout > 0.0f) && (AccumClockTicks >= (ExitTimeout * 1000.0f)))
    {
        Shutdown();
        return;
    }  

#ifdef GFC_VIDEO_USE_WWISE
    wwise->Update();
#endif

    if (!AdvanceMovie((float)delta / 1000000.f))
    {
        // If movie signaled exit, we are done.
        Shutdown();
        return;
    }

    // Nothing more to do if we can't render.
    if (!ValidateDeviceStatus())
        return;

    if (Rendering)
    {
        /*
    #ifdef FXPLAYER_LOGO        
        GFx_DisplayLogo(GetRenderer(), Viewport(GetWidth(), GetHeight()+FXPLAYER_LOGO_BORDER, 
                        0,0, GetWidth(), GetHeight()+LOGO_BORDER), pDummyImage);
    #endif
            */

        // Capture is done by Advance.
        pRenderThread->WaitForOutstandingDrawFrame();
        pRenderThread->DrawFrame();
    }

    // Update timing
    FrameCounter++;    
    if ((LastFPSUpdatedTicks + 1000000) <= AccumClockTicks)
    {
        LastFPS = 1000000.f * FrameCounter / (float)(AccumClockTicks - LastFPSUpdatedTicks);
        AccumFPS += LastFPS;
        LastFPSUpdatedTicks = AccumClockTicks;
        AccumFPSSecondCount++;
        FrameCounter = 0;
        //if (pRenderThread)
       //     LastDisplayFrames = pRenderThread->GetFrames();
        
        if (GetArgs().GetBool("LogFPS"))
        {
            float avgFPS = AccumFPS / AccumFPSSecondCount;
            printf("FPS: %6.2f, AVG FPS: %6.2f\n", LastFPS, avgFPS);
        }
    }
    
    // Update stats more frequently; each 1/5 of a second.
    if ((LastStatUpdatedTicks + 1000000/5) <= AccumClockTicks)
    {
        LastStatUpdatedTicks = AccumClockTicks;
        UpdateStatReports();
    }
}


void MenuKitApp::UpdateViewSize() 
{
    FxPlayerAppBase::UpdateViewSize();

    TheGame.UpdateViewSize(GetViewSize());
}


// FxPlayerCommand* MenuKitApp::PadKeyToCommand(PadKeyCode key)
// {
//     Hash< PadKeyCode, Ptr<FxPlayerCommand> >::Iterator it;
//     {
//         it = PadKeyCommandMap.Find(key);
//         if (it != PadKeyCommandMap.End())
//         {
//             return it->Second;
//         }
//     }
//     return NULL;
// }


void MenuKitApp::InitArgDescriptions(Platform::Args* args)
{
    FxPlayerAppBase::InitArgDescriptions(args);

    Platform::ArgDesc options []=
    {
        {"","", Platform::Args::ArgEnd,"",""},
    };
    args->AddDesriptions(options);
}

void MenuKitApp::OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag)
{
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
    case SF::Pad_A:
    case SF::Pad_X:
        OnKey(controllerIdx, Key::Return, 0, downFlag, 0);
        return;
    case SF::Pad_B:
    case SF::Pad_O:
        OnKey(controllerIdx, Key::Escape, 0, downFlag, 0);
        return;
    case SF::Pad_L2:
        OnKey(controllerIdx, Key::Pause, 0, downFlag, 0);
        return;
    default:
        break;
    }

    FxPlayerCommand* command = PadKeyToCommand(keyCode);
    if (command != NULL)
    {
        if (downFlag || command->HandleKeyUp())
        {
            ProcessCommand(controllerIdx, *command, downFlag);
        }
    }
}


void MenuKitApp::OnKey(unsigned controllerIndex, KeyCode keyCode,
                            unsigned wcharCode, bool downFlag, KeyModifiers mods) 
{
    bool ctrl = false;

	UInt32 keyPlusMods = mods.States & (KeyModifiers::Key_CtrlPressed |
								        KeyModifiers::Key_AltPressed |
								        KeyModifiers::Key_ShiftPressed);
    keyPlusMods <<= 16;
    keyPlusMods |= keyCode;

    if (!NoControlKeys) 
    {
        ctrl = mods.IsCtrlPressed();

        if (keyCode == Key::Control && !NoCtrlKey) 
        {
            return;
        }

        // TODO ???
        //if (keyCode == VK_MENU && downFlag)
        //    ControlKeyDown = false; // to enable Ctrl-Alt-... combinations to work

        if (downFlag && keyCode == Key::Escape)
        {
            // Cancel mouse manipulation
            if (MouseTracking != None) 
            {
                MouseTracking = None;           
                Zoom = ZoomStart;
                Move = MoveStart;
                UpdateUserMatrix();
                return;
            }
        }

        FxPlayerCommand* command = KeyToCommand(keyPlusMods);
        if (command != NULL)
        {
            if (downFlag || command->HandleKeyUp())
            {
                ProcessCommand(controllerIndex, *command, downFlag);
            }
        }
    }

    if ((keyCode == Key::None) && wcharCode)
    {
        CharEvent event(wcharCode);
        TheGame.OnCharEvent(event);
    }

    // Inform the player about keystroke
    if (!ctrl || NoCtrlKey)
    {
        KeyEvent event(downFlag ? GFx::Event::KeyDown : KeyEvent::KeyUp,
            keyCode, 0, wcharCode, mods, (UInt8)controllerIndex);
        TheGame.OnKeyEvent(event);
    }
}

void MenuKitApp::OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
                                    const Point<int>& mousePos, KeyModifiers mods)
{
    bool ControlKeyDown = mods.IsCtrlPressed();

    MousePrevPos = mousePos;
	Render::PointF p = AdjustToViewPort(mousePos);

    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    // UpdateStatBags mouse state
    if (downFlag) 
    {
        MouseDownPos = mousePos;

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

        MouseEvent event(GFx::Event::MouseDown, button, p.x, p.y, 0.0f, mouseIndex);
        TheGame.OnMouseEvent(event);
    } 
    else 
    {
        if (MouseTracking != None)
        {
            MouseTracking = None;
            return;
        } 

        MouseEvent event(GFx::Event::MouseUp, button, p.x, p.y, 0.0f, mouseIndex);
        TheGame.OnMouseEvent(event);
    }
}


void MenuKitApp::OnMouseWheel(unsigned mouseIndex, float zdelta,
                                   const Point<int>& mousePos, KeyModifiers mods)
{
    bool controlDown = mods.IsCtrlPressed();

	Render::PointF p = AdjustToViewPort(mousePos);

    if (controlDown)// && MouseTracking == None)
    {
        ZoomStart = Zoom;

        float dZoom = Zoom;
        Zoom += 0.02f * zdelta * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;

        Render::Matrix m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(Move.x, Move.y);
        p = m.TransformByInverse(p);

        Move.x += dZoom * p.x;
        Move.y += dZoom * p.y;

        UpdateUserMatrix();
        return;
    }

    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    MouseEvent event(GFx::Event::MouseWheel, 0, p.x, p.y, zdelta, mouseIndex);

    TheGame.OnMouseEvent(event);
}


void MenuKitApp::OnMouseMove(unsigned mouseIndex,
                                  const Point<int>& mousePos, KeyModifiers mods)
{
    SF_UNUSED(mods);

    Point<int> delta = MousePrevPos - mousePos;
    MousePrevPos = mousePos;

    if (MouseTracking == Zooming) 
    {
        float dZoom = Zoom; 
        Zoom += 0.01f * delta.x * Zoom;

        if (Zoom < 0.02f)
            Zoom = dZoom;

        dZoom -= Zoom;
        Render::PointF p = AdjustToViewPort(MouseDownPos);
		Render::Matrix2F m;
        m.AppendScaling(ZoomStart);
        m.AppendTranslation(MoveStart.x, MoveStart.y);

        p = m.TransformByInverse(p);

        Move.x += dZoom * p.x;
        Move.y += dZoom * p.y;

        UpdateUserMatrix();
        return;
    }

    if (MouseTracking == Moving) 
    {
        Move.x -= delta.x;
        Move.y -= delta.y;

        UpdateUserMatrix();
        return;
    }

	Render::PointF p = AdjustToViewPort(mousePos);
    Render::Matrix2F m;
    m.AppendScaling(Zoom);
    m.AppendTranslation(Move.x, Move.y);
    p = m.TransformByInverse(p);

    //}
    MouseEvent event(GFx::Event::MouseMove, 0, p.x, p.y, 0.0f, mouseIndex);
    TheGame.OnMouseEvent(event);

    /*
    // Mouse HitTest debugging logic

    Double xmouse = pMovie->GetVariableDouble("_root._xmouse");
    Double ymouse = pMovie->GetVariableDouble("_root._ymouse");

    int hitTest = pMovie->HitTest(x, y, Movie::HitTest_Shapes);

    pMovie->GetLog()->LogMessage("pMovie->HitTest: %d\n", hitTest);

    x = (int)(float(x) * mMovieInfo.Width) / ViewWidth;
    y = (int)(float(y) * mMovieInfo.Height) / ViewHeight;

    const char* pi = pMovie->Invoke("_root.hitTest", "%f%f%d", xmouse, ymouse, 1);
    pMovie->GetLog()->LogMessage("_root.hitTest: %s\n", pi);
    */
}

//------------------------------------------------------------------------
// ***** Main APP implementation

SF_PLATFORM_SYSTEM_APP(MenuKit, Scaleform::GFx::System, MenuKitApp)
