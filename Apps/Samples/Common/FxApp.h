/**************************************************************************

Filename    :   FxApp.h
Content     :   Base FxApp class declaration. System specific app
                classes derive from this in their own files.
Created     :   Jan 10, 2008
Authors     :   Maxim Didenko, Dmitry Polenur, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_FxApp_H
#define INC_FxApp_H

#include "GFx_Kernel.h"

#include "FxSettings.h"

// Base class for IME Events passed to OnIMEEvent. The actual data
// is in platform specific derived class such as FxWin32IMEEvent.
struct FxAppIMEEvent { };


// ***** FxApp

// FxApp is a base application class which has a number of platform
// specific implementations, such as Direct3DWin32App and GcmPS3App.
// While the user application (FxPlayerApp, etc) usually derives from
// the platform specific classes, it uses functions declared here
// for portability.

namespace Scaleform
{ 
    namespace Sound
    {
        class SoundRenderer;
    }

    namespace GFx
    {
        namespace AMP
        {
            class MessageAppControl;
        }
    }
}

class FxApp 
{
public:
    FxApp();
    virtual ~FxApp();

    // *** Initialization

    // Each subclass must provide a static InitMain function, and all apps must call it 
    // before allocating memory or using graphics features (including memory
    // allocation from static constructors).
    static void     InitMain() {}

    virtual bool CreateRenderer() = 0;
    virtual bool CreateSoundRenderer() { return false; }
    virtual void ShutdownSound()      {}
    
    // Called before Run(), this is a good place to do initialization -- if
    // anything fails, return false from here to prevent the program from
    // continuing. 
    virtual bool Init();

    // This is the replacement for the normal main(): all program work should
    // be done here. When OnRun() returns, the programs starts shutting down.
    virtual int  Run();

    // This is only called if Init() returned true so it's a good place to do
    // any cleanup matching the initializations done there.
    virtual bool Exit();

    // this one is called from Init() to add all supported options
    // to the given parser (don't forget to call the base class version if you
    // override it!)
    virtual void InitArgDescriptions(FxArgs* args);

    // called after successfully parsing the command line, return true
    // to continue and false to exit (don't forget to call the base class
    // version if you override it!)
    virtual bool OnArgsParsed(const FxArgs& args);

    // called if "--help" option was specified, return true to continue
    // and false to exit
    virtual bool OnArgsHelp(const FxArgs& args);

    // called if incorrect command line options were given, return
    // false to abort and true to continue
    virtual bool OnArgsError(const FxArgs& args);

  
    //Application Main function
    int          AppMain(int argc, char* argv[]);

    // *** Window Setup / Message APIs

    // Describes extra parameters for window setup which are
    // necessary on some platforms.
    struct SetupWindowParams
    {
        SetupWindowParams()
            : X(0), Y(0),
              FullScreen (false), ForceTiling(false) {}
        int  X,Y;
        bool FullScreen;
        bool ForceTiling;
    };

    // Creates a window & initializes the application class.
    // Returns 0 if window creation/initialization failed (the app should quit).
    virtual bool    SetupWindow(const char *pname, int width, int height, 
                                const SetupWindowParams& extraParams = SetupWindowParams()) = 0;

    // Message processing function to be called in the
    // application loops until it returns 0, to indicate application being closed.
    virtual bool    ProcessMessages() { return true; }
    // Sleeps for the specified number of milliseconds or till message.
    virtual void    SleepTillMessage(unsigned int /*ms*/) { }
    virtual void    SleepMilliSecs(unsigned int ms)     = 0;

    virtual void    BringMainWindowOnTop() { }

    // This call cleans up the application and kills the window.
    // If not called explicitly, it will be called from the destructor.
    virtual void    KillWindow()                        = 0;

    // Changes/sets window title
    virtual void    SetWindowTitle(const char *ptitle)  = 0;   

    // Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
    // This is a floating point value such as 0.1 for 10% safe frame border.
    virtual float   GetSafeArea() { return 0; }

    virtual Scaleform::Render::HAL* GetRenderHAL() = 0;
    virtual Scaleform::Sound::SoundRenderer* GetSoundRenderer() { return 0; }

    // Presents the data (swaps the back and front buffers).
    virtual void    PresentFrame()                      = 0;

    // On successful reset, this function will call InitRenderer again.
    virtual bool    RecreateRenderer()                  = 0;

    // This override is called from SetupWindow to initialize OpenGL/Direct3D view
    virtual bool    InitRenderer()                      = 0;
    // Should/can be called every frame to prepare the render, user function
    virtual void    PrepareRendererForFrame()           = 0;

    // This method is used to switch between full and non-full screen mode
    virtual void    SwitchFullScreenMode() {}

    // This method handles AMP app control messages
    virtual void    OnAmpAppControlToggle(const Scaleform::GFx::AMP::MessageAppControl* message) { SF_UNUSED(message); }

    // *** Input Constants and 'On' Functions

    enum KeyModifiers
    {
        KM_Control = 0x1,
        KM_Alt     = 0x2,
        KM_Shift   = 0x4,
        KM_Num     = 0x8,
        KM_Caps    = 0x10,
        KM_Scroll  = 0x20
    };

    // Input overrides, invoked during the ProcessMessages call.
    virtual void    OnKey(unsigned /*controllerIdx*/, Key::Code /*keyCode*/, unsigned char /*asciiCode*/, unsigned int /*wcharCode*/, 
        unsigned int /*mods*/, bool /*downFlag*/)     { };
    virtual void    OnPad(unsigned /*controllerIdx*/, PadKeyCode /*PadkeyCode*/, bool /*downFlag*/) { }
    virtual void    OnPadStick(unsigned /*controllerIdx*/, PadKeyCode /*PadkeyCode*/, float /*xpos*/, float /*ypos*/) { }


    // IME events
    virtual bool    OnIMEEvent(unsigned message, UPInt wParam, UPInt lParam, UPInt hWND, bool preprocess) 
                                { SF_UNUSED5(message, wParam, lParam, hWND, preprocess); return false; }

    // Mouse events
    virtual void    OnMouseButton(unsigned int /*mouseIndex*/, unsigned int /*button*/, bool /*downFlag*/, 
                                  int /*x*/, int /*y*/, unsigned int /*mods*/) {  }
    virtual void    OnMouseMove(unsigned int /*mouseIndex*/, int /*x*/, int /*y*/, int unsigned /*mods*/) {}
    virtual void    OnMouseWheel(unsigned int /*mouseIndex*/, int /*zdelta*/, int /*x*/, int /*y*/, 
                                 unsigned int /*mods*/) { }

    virtual void    OnTouchBegin(unsigned /*id*/, int /*x*/, int /*y*/, int /*wcontact*/, int /*hcontact*/, bool /*primary*/) {}
    virtual void    OnTouchEnd(unsigned /*id*/, int /*x*/, int /*y*/, int /*wcontact*/, int /*hcontact*/, bool /*primary*/) {}
    virtual void    OnTouchMove(unsigned /*id*/, int /*x*/, int /*y*/, int /*wcontact*/, int /*hcontact*/, bool /*primary*/) {}

    virtual void    OnGestureBegin(UInt32 /*gestureMask*/, int /*x*/, int /*y*/, 
                                        float /*translationDeltaX*/, float /*translationDeltaY*/,
                                        float /*scaleDeltaX*/, float /*scaleDeltaY*/,
                                        float /*rotationDelta*/) {}
    virtual void    OnGesture(UInt32 /*gestureMask*/, int /*x*/, int /*y*/, float /*translationDeltaX*/, float /*translationDeltaY*/,
                                   float /*scaleDeltaX*/, float /*scaleDeltaY*/,
                                   float /*rotationDelta*/) {}
    virtual void    OnGestureEnd(UInt32 /*gestureMask*/, int /*x*/, int /*y*/) {}

    // Number of mice or similar controllers available
    virtual int     GetMouseCount() { return 1; }
    virtual int     GetKeyboardCount() { return 1; }

    virtual void    ResizeWindow(int /*w*/, int /*h*/) { }
    // Sizing; by default, re-initalizes the renderer
    virtual void    OnSize(int w, int h)   { ResizeWindow(w,h); }
    // Called when sizing begins and ends.
    virtual void    OnSizeEnter(bool /*enterSize*/) { }
    // Handle dropped files.
    virtual void    OnDropFiles(const String& /*path*/)     { }

    virtual void    OnUpdateSystemCliboard(const wchar_t* /*text*/) { }

    virtual bool    UpdateFiles(char* /*filename*/, bool /* prev*/) { return 0; }

    // Query display status
    enum DisplayStatus
    {
        DisplayStatus_Ok            = 0,
        DisplayStatus_NoModeSet     = 1,    // Video mode 
        DisplayStatus_Unavailable   = 2,    // Display is unavailable for rendering; check status again later
        DisplayStatus_NeedsReset    = 3     // May be returned in Dependent mode to indicate external action being required
    };
    virtual DisplayStatus  CheckDisplayStatus() const = 0;


    // *** Simple Rendering APIs

    // Clears the entire back buffer.
    virtual void    Clear(unsigned int color) = 0;

    // Initialize and restore 2D rendering view.
    // Push2DRenderView must be called before using 2D functions below,
    // while Pop2DRenderView MUST be called after it is finished.
    virtual void    Push2DRenderView()  = 0;
    virtual void    Pop2DRenderView()   = 0;    

    // Draw a filled + blended rectangle.
    virtual void    FillRect(int l, int t, int r, int b, unsigned int color) = 0;
    // Draw a text string (specify top-left corner of characters, NOT baseline)
    virtual void    DrawText(int /*x*/, int /*y*/, const char* /*ptext*/, unsigned int /*color*/)  {}
    // Compute text size that will be generated by DrawText
    virtual void    CalcDrawTextSize(int* /*pw*/, int* /*ph*/, const char* /*ptext*/) {}
    
    // API-independent toggle for wireframe rendering.
    virtual void    SetWireframe(bool wireframeEnable) = 0;
  
    virtual void    SwitchFSAA(bool /*on_off*/) {}
    virtual void    SetVSync(bool /*isEnabled*/){}  

    // if set == true then this is SET FOCUS, otherwise - KILL FOCUS
    virtual void    OnFocus(bool /*set*/, unsigned int /*mods*/) { }

    // *** Window Size / State Query

    int             GetWidth()  const { return Width; }
    int             GetHeight() const { return Height; }
    void            SetWidth(int w)   { Width=w; }
    void            SetHeight(int h)  { Height=h; }

    bool            IsConsole() const { return Console; }
    
    virtual bool    IsPadConnected(unsigned /*pad*/) {return IsConsole();}
    virtual void    DetectPad() {}

    // *** Mouse and Cursor functions

    virtual void    StartMouseCapture() {}
    virtual void    EndMouseCapture()   {}

     // Draw a mouse cursor (usually after the scene). Used only on consoles.
    virtual void    RedrawMouseCursor() {}
    
    virtual void    ShowCursorInstantly(bool /*show*/) {}
    void            ShowCursor(bool show = true);
    void            ResetCursor();


    // Cursor states
    bool            CursorHidden;
    bool            CursorHiddenSaved;
    bool            CursorDisabled;
    bool            CursorIsOutOfClientArea;

    // Boolean states for the window and app
    bool            Created;
    bool            Active;
    bool            QuitFlag;
    bool            LockOnSize;
    int             ExitCode;

    // Set to true if this platform is a console; should be
    // set in the constructor for the particular platform.
    bool            Console;
    
    // Set this if the window is to accept drag and drop
    bool            SupportDropFiles;
    bool            SizableWindow;

    // Requested 3D state
    bool            FullScreen;
    bool            FSAntialias;
    bool            VSync;
    unsigned int    BitDepth;

    // This flag will be set after SetupWindow to indicate that FSAA is available.
    bool            FSAASupported;

    int             Width;
    int             Height;

    unsigned int    VMCFlags;

    // Old width and height saved during FullScreen mode
    int             OldWindowX, OldWindowY;
    int             OldWindowWidth, OldWindowHeight;
    
    FxArgs          Arguments; 
    
    static FxApp*   pApp; 
};

#endif //INC_FxApp_H
