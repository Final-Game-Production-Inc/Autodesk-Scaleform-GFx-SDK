/**************************************************************************

Filename    :   Platform_WinAPI.cpp
Content     :   Platform::App implementation for Win32.
Created     :   Jan 10, 2008
Authors     :   Maxim Didenko, Dmitry Polenur, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform.h"
#include "Platform_XInput.h"
#include "Kernel/SF_WString.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_UTF8Util.h"
#include "Render/Render_ShapeDataFloatMP.h"
#include "Render/Render_Color.h"
#include <crtdbg.h>
#include <DbgHelp.h>

// Icon ID from GFxPlayer.rc
#define IDI_GFXPLAYER   101

#ifdef UNICODE
#define SF_WND_CLASS_NAME L"WinAPI_App_Window_Class"
#else
#define SF_WND_CLASS_NAME "WinAPI_App_Window_Class"
#endif

// Default Window position if one is not provided in ViewConfig:
//  >0 - from left/top
//  <0 - from right/bottom
#define SF_WINDOW_X     -200
#define SF_WINDOW_Y     100

//------------------------------------------------------------------------
// #define <winwuser.h> constants that are not always available,
// including MouseWheel, MouseLeave, and XButton WM_ message support.
#ifndef WM_MOUSEWHEEL
    #define WM_MOUSEWHEEL           0x020A
    #define WHEEL_DELTA             120
#endif

#ifndef WM_MOUSELEAVE
    #define WM_MOUSELEAVE           0x02A3
#endif

#ifndef WM_XBUTTONDOWN
    #define WM_XBUTTONDOWN          0x020B
    #define WM_XBUTTONUP            0x020C
    #define WM_XBUTTONDBLCLK        0x020D
    #define WM_NCXBUTTONDOWN        0x00AB
    #define WM_NCXBUTTONUP          0x00AC
    #define WM_NCXBUTTONDBLCLK      0x00AD
    // XButton values are WORD flags
    #define GET_XBUTTON_WPARAM(wp)  (HIWORD(wp))
#endif

//------------------------------------------------------------------------

namespace Scaleform { namespace Platform {

#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
#include <ole2.h>
#include <ocidl.h>
#include <manipulations.h>

//------------------------------------------------------------------------
// ***** ManipulationEventListener
class ManipulationEventListener : public _IManipulationEvents
{
public:
    ManipulationEventListener(class AppImpl*);
    virtual ~ManipulationEventListener();

    bool Connect(IManipulationProcessor *manipProc);
    bool Disconnect();

    virtual HRESULT STDMETHODCALLTYPE ManipulationStarted(
        FLOAT x,
        FLOAT y);

    virtual HRESULT STDMETHODCALLTYPE ManipulationDelta( 
        FLOAT x,
        FLOAT y,
        FLOAT translationDeltaX,
        FLOAT translationDeltaY,
        FLOAT scaleDelta,
        FLOAT expansionDelta,
        FLOAT rotationDelta,
        FLOAT cumulativeTranslationX,
        FLOAT cumulativeTranslationY,
        FLOAT cumulativeScale,
        FLOAT cumulativeExpansion,
        FLOAT cumulativeRotation);

    virtual HRESULT STDMETHODCALLTYPE ManipulationCompleted( 
        FLOAT x,
        FLOAT y,
        FLOAT cumulativeTranslationX,
        FLOAT cumulativeTranslationY,
        FLOAT cumulativeScale,
        FLOAT cumulativeExpansion,
        FLOAT cumulativeRotation);

    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
private:
    LONG                RefCount;
    IConnectionPoint*   pConnection;
    DWORD               Cookie;
    class AppImpl*      pAppImpl;
};

// A class to access multitouch related WinAPI functions (for compatibility with pre-Windows7 systems).
class User32TouchDll
{
    typedef BOOL (WINAPI *RegisterTouchWindowProcType)(HWND hwnd, ULONG ulFlags);
    typedef BOOL (WINAPI *UnregisterTouchWindowProcType)(HWND);
    typedef BOOL (WINAPI *IsTouchWindowProcType)(HWND, PULONG);
    typedef BOOL (WINAPI *CloseTouchInputHandleProcType)(HTOUCHINPUT hTouchInput);
    typedef BOOL (WINAPI *GetTouchInputInfoProcType)(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize);

    RegisterTouchWindowProcType     RegisterTouchWindowProc;
    UnregisterTouchWindowProcType   UnregisterTouchWindowProc;
    IsTouchWindowProcType           IsTouchWindowProc;
    CloseTouchInputHandleProcType   CloseTouchInputHandleProc;
    GetTouchInputInfoProcType       GetTouchInputInfoProc;

    HMODULE                         hUser32;
    DWORD                           ErrorCode;
public:
    User32TouchDll() 
    {
        hUser32 = ::LoadLibraryA("user32.dll");
        if (hUser32)
        {
            RegisterTouchWindowProc     = (RegisterTouchWindowProcType)::GetProcAddress(hUser32, "RegisterTouchWindow");    
            UnregisterTouchWindowProc   = (UnregisterTouchWindowProcType)::GetProcAddress(hUser32, "UnregisterTouchWindow");    
            IsTouchWindowProc           = (IsTouchWindowProcType)::GetProcAddress(hUser32, "IsTouchWindow");    
            CloseTouchInputHandleProc   = (CloseTouchInputHandleProcType)::GetProcAddress(hUser32, "CloseTouchInputHandle");    
            GetTouchInputInfoProc       = (GetTouchInputInfoProcType)::GetProcAddress(hUser32, "GetTouchInputInfo");    
            ErrorCode = 0;
        }
        else
        {
            ErrorCode = GetLastError();
        }
    }
    ~User32TouchDll()
    {
        if (hUser32)
            ::FreeLibrary(hUser32);
    }
    bool IsAvailable()  const   
    { 
        return hUser32 != NULL && RegisterTouchWindowProc && UnregisterTouchWindowProc && 
            IsTouchWindowProc && CloseTouchInputHandleProc && GetTouchInputInfoProc; 
    }
    int	 GetErrorCode() const   { return (int)ErrorCode; }
    void ResetErrorCode()       { ErrorCode = 0;    }

    // User32 functions. Note, before calling any of the following function
    // make sure Imm32 was loaded by calling to IsAvailable.
    BOOL RegisterTouchWindow(HWND hwnd, ULONG ulFlags) 
    { 
        return (RegisterTouchWindowProc) ? RegisterTouchWindowProc(hwnd, ulFlags) : FALSE;
    } 
    BOOL UnregisterTouchWindow(HWND hwnd)
    {
        return (UnregisterTouchWindowProc) ? UnregisterTouchWindowProc(hwnd) : FALSE;
    }
    BOOL IsTouchWindow(HWND hwnd, PULONG pulFlags)
    {
        return (IsTouchWindowProc) ? IsTouchWindowProc(hwnd, pulFlags) : FALSE;
    }
    BOOL CloseTouchInputHandle(HTOUCHINPUT hTouchInput)
    {
        return (CloseTouchInputHandleProc) ? CloseTouchInputHandleProc(hTouchInput) : FALSE;
    }
    BOOL GetTouchInputInfo(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize)
    {
        return (GetTouchInputInfoProc) ? GetTouchInputInfoProc(hTouchInput, cInputs, pInputs, cbSize) : FALSE;
    }
};
#endif

//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public Device::Window, public AppImplBase
{
    friend class AppBase;
    friend class ManipulationEventListener;

public:

    AppImpl(AppBase* app)
        : AppImplBase(app),
          hWnd(0), hInstance(0),
          WindowAdjust(0,0),
      //    OldWindowPos(0),
     //     OldWindowSize(AppBase::GetDefaultViewSize()),
          NeedPaint(true),
          hWndNextViewer(0),
          BlockOnSize(false), MultitouchSupported(false)
    {
        Created = false;
#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
        ActiveGestureMask = 0;
#endif
    }

    ~AppImpl()
    {
        // Remove this AppImpl pointer from the window handle, just in case it hasn't quite been destroy
        // and receives more windows messages. We don't want it using the dangling pointer.
        if ( hWnd != 0 )
        {
#ifdef SF_64BIT_POINTERS
            SetWindowLongPtr(hWnd, 0, (LONG_PTR)0);
#else
            SetWindowLong(hWnd, 0, (LONG)(UPInt)0);
#endif
        }
    }

    // Window Interface
    virtual void*   GetHandle() { return hWnd; }
    virtual void    GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void    ConfigureWindow(ViewConfig& newConfig);    
    

    LRESULT         MemberWndProc(UINT message, WPARAM wParam, LPARAM lParam);
    bool            setupWindow(const String& title, const ViewConfig& config);
    void            failSetupWindow(const char* cause);

    KeyModifiers    updateModifiers(bool extendedKeyFlag = false);
    void            processKey(unsigned vkCode, bool downFlag, bool extendedKeyFlag);
    void            processMouseButton(unsigned button, bool downFlag, LPARAM l);

    void            processDrawClipboard(WPARAM wParam, LPARAM lParam);
    void            processChangeCBChain(WPARAM wParam, LPARAM lParam);

    void            processTouch(WPARAM wParam, LPARAM lParam);
    
    // AppImplBase
    virtual void    InitArgDescriptions(Args* args);
    virtual void    ApplyViewConfigArgs(ViewConfig* config, const Args& args);
    virtual void    SetWindowTitle(const String& title);
    virtual void    killWindow();

    virtual bool    OnArgs(const Args& args, Args::ParseResult parseResult);

    virtual bool    IsMultitouchSupported() const { return MultitouchSupported; }

    virtual void    ProcessUrl(const String& url);

protected:
    virtual void    updateConfig();

private:
    HWND            hWnd;
    HINSTANCE       hInstance;
    Point<int>      WindowAdjust;
    KeyModifiers    Modifiers;
    GamePad			Pad;
     // Window location saved when going full-screen.
 //   Point<int>      OldWindowPos;
 //   Size<unsigned>  OldWindowSize;

    bool            NeedPaint;
    Size<LONG>      MinWindowSize;

    // Clipboard related
    HWND            hWndNextViewer;
    bool            Created;
    bool            BlockOnSize;
    bool            MultitouchSupported;

    static LRESULT CALLBACK appWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    template <typename T>
    Point<int>      ScreenToClient(T x, T y);

    Ptr<IManipulationProcessor>     pIManipProc;    
    Ptr<ManipulationEventListener>  pManipulationEventListener;
    Point<int>                      FirstTouchPt;
    UInt32                          ActiveGestureMask;
    User32TouchDll                  User32;
#endif
};


// calcWindowStyles determines window styles based on viewFlags.
void calcWindowStyles(unsigned viewFlags, DWORD* style, DWORD* styleEx)
{
    // Set the window style depending on FullScreen state
    if (viewFlags & View_FullScreen)
    {
        // FullScreen gets no borders or title bar.
        *style     = WS_POPUP;
        *styleEx   = WS_EX_APPWINDOW;
    }
    else
    {
        *styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

        if (viewFlags & View_WindowResizable)
            *style = WS_OVERLAPPEDWINDOW;
        else
        {
            // need WS_CAPTION for GL offsetting to work correctly.
            *style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU;
        }
    }
    *style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    if (viewFlags & View_WindowDropFiles)
        *styleEx |= WS_EX_ACCEPTFILES;
}

void calcAdjustedWindowRect(RECT *r, Point<int>* adjust,
                            const ViewConfig& config, DWORD style, DWORD styleEx)
{

    // Initialize the rendering window to have width & height match the client area.
    r->left  = config.WindowPos.x;
    r->top   = config.WindowPos.y;
    r->right = r->left + config.ViewSize.Width; 
    r->bottom= r->top + config.ViewSize.Height;
    ::AdjustWindowRectEx(r, style, 0, styleEx);
    adjust->SetPoint(config.WindowPos.x - r->left, config.WindowPos.y - r->top);
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;
    
    if (Created)
        return false;
    hInstance = GetModuleHandle(NULL);

    // Load GFxPlayer icon
    HICON hGFxIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_GFXPLAYER),
                                      IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT | LR_VGACOLOR);

    // Initialize the window class structure
    ::WNDCLASSEX wc = { sizeof(WNDCLASSEX), 
                        CS_HREDRAW | CS_VREDRAW | CS_OWNDC, 
                        &appWindowProc,
                        0L, 
                        sizeof(AppImpl*),
                        hInstance, 
                        hGFxIcon, 
                        NULL, // hCursor is NULL to avoid revert on each mouse move. 
                        NULL, 
                        NULL,
                        SF_WND_CLASS_NAME, 
                        NULL };
    if (!::RegisterClassEx(&wc))
    {
        failSetupWindow("Unable to register the window class");
        return false;
    }
    
    SF_ASSERT(Config.HasFlag(View_Size));
    if (!Config.HasFlag(View_Pos))
    {        
        int screenW = ::GetSystemMetrics(SM_CXSCREEN);
        int screenH = ::GetSystemMetrics(SM_CYSCREEN);
        Config.WindowPos.x = (SF_WINDOW_X > 0) ? SF_WINDOW_X : screenW - Config.ViewSize.Width  + SF_WINDOW_X;
        Config.WindowPos.y = (SF_WINDOW_Y > 0) ? SF_WINDOW_Y : screenH - Config.ViewSize.Height + SF_WINDOW_Y;
    }
    
    // Initialize cursor state.
    pApp->Cursor.SetSoftware(config.HasFlag(View_WindowSWCursor));

    // Initialize the rendering window to have width & height match the client area.
    RECT  r;
    DWORD style, styleEx;
    calcWindowStyles(Config.ViewFlags, &style, &styleEx);
    calcAdjustedWindowRect(&r, &WindowAdjust, Config, style, styleEx);
    
#ifdef UNICODE 
    WStringBuffer wbuffer;
    wbuffer = title;
    const wchar_t* windowTitle = wbuffer.GetBuffer();
#else
    const char* windowTitle = title.ToCStr();
#endif
        
    hWnd = ::CreateWindowEx(styleEx,
                            SF_WND_CLASS_NAME,
                            windowTitle,
                            style,
                            r.left, r.top, r.right-r.left, r.bottom-r.top,
                            GetDesktopWindow(),
                            NULL,
                            hInstance,
                            (LPVOID)this);
    if (!hWnd)
    {
        failSetupWindow("Unable to create window");
        return false;
    }

    // Set the console window (if there is one) to our custom icon as well.
    HWND consoleWindow = ::GetConsoleWindow();
    if (consoleWindow != INVALID_HANDLE_VALUE)
    {
        SendMessage(consoleWindow, WM_SETICON, ICON_SMALL, (LPARAM)hGFxIcon);
        SendMessage(consoleWindow, WM_SETICON, ICON_BIG, (LPARAM)hGFxIcon);
    }

#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    // test for touch
    int value = GetSystemMetrics(SM_DIGITIZER);
    if (value & NID_MULTI_INPUT)
    {
        // digitizer is multitouch
        // !AB: should we check for if (value & NID_INTEGRATED_TOUCH) as well?
        MultitouchSupported = true;    
    }
    
    if (User32.IsAvailable())
    {
        User32.RegisterTouchWindow(hWnd, 0);

        SF_ASSERT(User32.IsTouchWindow(hWnd, NULL));

        do 
        {
            // Instantiate the ManipulationProcessor object
            HRESULT hr = ::CoCreateInstance(__uuidof(ManipulationProcessor), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pIManipProc.GetRawRef()));
            if (FAILED(hr))
            {
                //SF_ASSERT(SUCCEEDED(hr)); // failed to instantiate the ManipulationProcessor object
                continue;
            }

            // Instantiate the event sink with the manipulation processor and pointer to the rectangle object
            pManipulationEventListener = *new ManipulationEventListener(this);
            if (!pManipulationEventListener)
            {
                pIManipProc = NULL;
                continue;
            }

            // Establish the link between ManipulationEventSink and ManipulationProcessor
            if (!pManipulationEventListener->Connect(pIManipProc))
            {
                SF_ASSERT(0); // failed to connect ManipulationEventListener and ManipulationProcessor
                pIManipProc = NULL;
                pManipulationEventListener = NULL;
                continue;
            }
        } while(0);
    }

#endif

    // Show the window in the foreground, and set the keyboard focus to it.
    ::ShowWindow(hWnd, SW_SHOW);
    ::SetForegroundWindow(hWnd);
    ::SetFocus(hWnd);

    // Check the client rect was changed by Windows (too big a window was created, etc).
    // This must be done after ShowWindow to support "Run Maximized" with windows shortcut.
    RECT clientRect;
    ::GetClientRect(hWnd, &clientRect);
    if ((unsigned)(clientRect.right - clientRect.left) != Config.ViewSize.Width ||
        (unsigned)(clientRect.bottom - clientRect.top) != Config.ViewSize.Height)
    {
        // Temporarily disabled, seeing if this causes window size problems with autotest.
        //Config.SetSize(clientRect.right - clientRect.left,
        //               clientRect.bottom - clientRect.top);
    }
    updateConfig();
    return true;
}

void AppImpl::failSetupWindow(const char* cause)
{
    if (Arguments.GetBool("NoDebugPopups"))
        fprintf(stderr, "%s\n", cause);
    else
        MessageBoxA(NULL, cause, "WinAPI AppBase Platform" , MB_OK | MB_ICONEXCLAMATION);
    Config = ViewConfig();
}

void AppImpl::killWindow()
{
#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    if (User32.IsAvailable())
        User32.UnregisterTouchWindow(hWnd);

    // COM objects must be released before CoUninitialize
    // Release ManipulationEventSink object 
    if (pManipulationEventListener) 
    {
        // Terminate the connection between ManipulationEventSink and ManipulationProcessor
        pManipulationEventListener->Disconnect();
        pManipulationEventListener = NULL;
    }
    pIManipProc = NULL;
#endif
}

#define SF_WGL_STYLE_PRESERVE_MASK (WS_VISIBLE|WS_MAXIMIZE|WS_MINIMIZE)

void AppImpl::ConfigureWindow(ViewConfig& config)
{
    // Configures window for a new video mode and/or settings.    
    DWORD   oldStyle   = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    DWORD   oldStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_EXSTYLE);
    UINT    changeFlags = 0;   
    DWORD   style, styleEx;
    bool    checkWindowPos = false;

    calcWindowStyles(config.ViewFlags, &style, &styleEx);
    style |= (oldStyle & SF_WGL_STYLE_PRESERVE_MASK);

    SF_ASSERT(config.HasFlag(View_Size));
    if (!config.HasFlag(View_Pos))
    {
        config.SetPos(getConfig().GetPos());
        if (!config.HasFlag(View_FullScreen))
            checkWindowPos = true;
    }
    
    // Temporarily disable OnSize() calls.
    BlockOnSize = true; 
    
    // Apply window style.
    if (style != oldStyle)
    {
        ::SetWindowLong(hWnd, GWL_STYLE, style);
        changeFlags = SWP_FRAMECHANGED;
    }
    if (styleEx != oldStyleEx)
    {
        ::SetWindowLong(hWnd, GWL_EXSTYLE, styleEx);
        changeFlags = SWP_FRAMECHANGED;
    }

    // Apply new window location/size.
    RECT r, wr;
    calcAdjustedWindowRect(&r, &WindowAdjust, config, style, styleEx);
    ::GetWindowRect(hWnd, &wr);
    
    if (checkWindowPos)
    {
        // If returning from full-screen, ensure that we can see the window.
        if (r.left < 0) r.left = 0;
        if (r.top < 0) r.top = 0;
    }

    if ((config.HasFlag(View_FullScreen) != Config.HasFlag(View_FullScreen)) ||
        changeFlags || (memcmp(&r, &wr, sizeof(RECT)) != 0))
        ::SetWindowPos(hWnd,
            config.HasFlag(View_FullScreen) ? HWND_TOPMOST : HWND_NOTOPMOST,
            r.left, r.top, r.right - r.left, r.bottom - r.top, changeFlags);

   BlockOnSize = false;
   Config = config;
}

void AppImpl::updateConfig()
{
    RECT r = { 0, 0, 0, 0 };
    if (hWnd)
        ::GetWindowRect(hWnd, &r);
    Config.SetPos(r.left + WindowAdjust.x, r.top + WindowAdjust.y);
}

void AppImpl::InitArgDescriptions(Args* args)
{
    AppImplBase::InitArgDescriptions(args);
    ArgDesc options[] =
    {
        {"ndp",     "NoDebugPopups",  Args::Flag,           "", "Disable Windows debug popups"},
        {"dump",    "Minidump",       Args::StringOption,   "", "Creates a minidump when unhandled exceptions occur (implies -ndp)."},
        {"sbuf",    "StaticBuffers",  Args::Flag,           "", "Uses static vertex/index buffers, instead of dynamic ones."},
        {"sm20",    "ShaderModel20",  Args::Flag,           "", "Forces the use of shader model 2.0 shaders (D3D9)"},
#if defined(FXPLAYER_RENDER_OPENGL)
        {"gl20",    "OpenGL20",       Args::Flag,           "", "Use a legacy GL 2.x context instead of GL 3.x" },
        {"gldbg",   "GLDebug",        Args::Flag,           "", "Enable GL debug messages via GL_ARB_debug_output." },
#endif
        {"",        "",               Args::ArgEnd,         "", ""}
    };
    args->AddDesriptions(options);
}

void AppImpl::ApplyViewConfigArgs(ViewConfig* config, const Args& args)
{
    AppImplBase::ApplyViewConfigArgs(config, args);
    if (args.GetBool("StaticBuffers"))
        config->ViewFlags |= View_StaticBuffers;
#if defined(FXPLAYER_RENDER_OPENGL)
    if (args.GetBool("OpenGL20"))
        config->ViewFlags |= View_GL20;
    if (args.GetBool("GLDebug"))
        config->ViewFlags |= View_DebugMessages;
#endif
    if (args.GetBool("ShaderModel20"))
        config->ViewFlags |= View_ShaderModel20;
}

void AppImpl::SetWindowTitle(const String& title)
{
    AppImplBase::SetWindowTitle(title);

    if (hWnd)
    {
#ifdef UNICODE 
        WStringBuffer wbuffer;
        wbuffer = title;
        const wchar_t* windowTitle = wbuffer.GetBuffer();
#else
        const char* windowTitle = title.ToCStr();
#endif
        ::SetWindowText(hWnd, windowTitle);
    }
}

// Global callback function to be called on window create
LRESULT CALLBACK AppImpl::appWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    AppImpl* papp;  

    // The first message to ever come in sets the long value to class pointer
    if (iMsg==WM_NCCREATE)
    {
        papp = (AppImpl*) ((LPCREATESTRUCT)lParam)->lpCreateParams;

        if (!papp)
            return DefWindowProc(hwnd,iMsg,wParam,lParam);
#ifdef SF_64BIT_POINTERS
        SetWindowLongPtr(hwnd, 0, (LONG_PTR)papp);
#else
        SetWindowLong(hwnd, 0, (LONG)(UPInt)papp);
#endif
        papp->hWnd = hwnd;
    }

    // use UPInt to quiet /Wp64 warning
    if ((papp=((AppImpl*)(UPInt)GetWindowLongPtr(hwnd,0)))==0)
    {
        // It is possible that WM_GETMINMAXINFO arrives before WM_NCCREATE. If autotesting, having restrictions on the 
        // size of the window can be a problem for agents with low resolutions, because of desktop window clamping rules.
        // In this case, any window that gets created does not get a maximum size.
        if (iMsg == WM_GETMINMAXINFO)
        {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
            lpmmi->ptMaxSize.x = lpmmi->ptMaxSize.y = 65535;
            lpmmi->ptMaxTrackSize.x = lpmmi->ptMaxTrackSize.y =65535;
            return 0;
        }

        return DefWindowProc(hwnd,iMsg,wParam,lParam);
    }
    // Call member
    return papp->MemberWndProc(iMsg, wParam, lParam);
}


KeyModifiers AppImpl::updateModifiers(bool extendedKeyFlag)
{
    KeyModifiers mods;
    if (::GetKeyState(VK_SHIFT) & 0x8000)
        mods.SetShiftPressed(true);
    if (::GetKeyState(VK_CONTROL) & 0x8000)
        mods.SetCtrlPressed(true);
    if (::GetKeyState(VK_MENU) & 0x8000)
        mods.SetAltPressed(true);
    if (::GetKeyState(VK_NUMLOCK) & 0x1)
        mods.SetNumToggled(true);
    if (::GetKeyState(VK_CAPITAL) & 0x1)
        mods.SetCapsToggled(true);
    if (::GetKeyState(VK_SCROLL) & 0x1)
        mods.SetScrollToggled(true);
    if (extendedKeyFlag)
        mods.SetExtendedKey(true);
    Modifiers = mods;
    return mods;
}

void AppImpl::processKey(unsigned vkCode, bool downFlag, bool extendedKeyFlag)
{
    KeyCode kc = Key::None;

    if (vkCode >= 'A' && vkCode <= 'Z')
    {
        kc = (KeyCode) ((vkCode - 'A') + Key::A);
    }
    else if (vkCode >= VK_F1 && vkCode <= VK_F15)
    {
        kc = (KeyCode) ((vkCode - VK_F1) + Key::F1);
    }
    else if (vkCode >= '0' && vkCode <= '9')
    {
        kc = (KeyCode) ((vkCode - '0') + Key::Num0);
    }
    else if (vkCode >= VK_NUMPAD0 && vkCode <= VK_DIVIDE)
    {
        kc = (KeyCode) ((vkCode - VK_NUMPAD0) + Key::KP_0);
    }
    else 
    {
        for (int i = 0; KeyCodeMap[i].winKey != 0; i++)
        {
            if (vkCode == (unsigned)KeyCodeMap[i].winKey)
            {
                kc = KeyCodeMap[i].appKey;
                break;
            }
        }
    }

    pApp->OnKey(0, kc, 0, downFlag, updateModifiers(extendedKeyFlag));
}

void AppImpl::processMouseButton(unsigned button, bool downFlag, LPARAM l)
{
    MousePos = Point<int>((SInt16)LOWORD(l), (SInt16)HIWORD(l));
    if (downFlag)
        ::SetCapture(hWnd);
    else
        ::ReleaseCapture();
    pApp->OnMouseButton(0, button, downFlag, MousePos, updateModifiers(false));
}

#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
void AppImpl::processTouch(WPARAM wParam, LPARAM lParam)
{
    UINT cInputs = LOWORD(wParam);
    PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
    if (NULL != pInputs)
    {
        if (User32.GetTouchInputInfo((HTOUCHINPUT)lParam,
            cInputs,
            pInputs,
            sizeof(TOUCHINPUT)))
        {
            for (UINT i = 0; i < cInputs; ++i)
            {
                TOUCHINPUT ti = pInputs[i];

                if (!pApp->pGestureManager)
                {
                    // feed manipulation processor with the touch event
                    if (ti.dwFlags & TOUCHEVENTF_DOWN)
                    {
                        pIManipProc->ProcessDown(ti.dwID, (FLOAT)ti.x, (FLOAT)ti.y);
                    }
                    else if (ti.dwFlags & TOUCHEVENTF_MOVE)
                    {
                        pIManipProc->ProcessMove(ti.dwID, (FLOAT)ti.x, (FLOAT)ti.y);
                    }
                    else if (ti.dwFlags & TOUCHEVENTF_UP)
                    {
                        pIManipProc->ProcessUp(ti.dwID, (FLOAT)ti.x, (FLOAT)ti.y);
                    }
                }

                if (ti.dwID != 0)
                {     
                    Point<int> ptInput = ScreenToClient(TOUCH_COORD_TO_PIXEL(ti.x), TOUCH_COORD_TO_PIXEL(ti.y));

                    POINT ptContact;
                    ptContact.x = ptContact.y = 0;
                    if (ti.dwMask & TOUCHINPUTMASKF_CONTACTAREA)
                    {
                        ptContact.x = TOUCH_COORD_TO_PIXEL(ti.cxContact);
                        ptContact.y = TOUCH_COORD_TO_PIXEL(ti.cyContact);
                    }

                    bool primary = ((ti.dwFlags & TOUCHEVENTF_PRIMARY) != 0);

                    Point<int> contact(ptContact.x, ptContact.y);

                    if (ti.dwFlags & TOUCHEVENTF_DOWN)
                        pApp->OnTouchBegin(0, ti.dwID, ptInput, contact, primary);
                    else if (ti.dwFlags & TOUCHEVENTF_UP)
                        pApp->OnTouchEnd(0, ti.dwID, ptInput, contact, primary);
                    else if (ti.dwFlags & TOUCHEVENTF_MOVE)
                        pApp->OnTouchMove(0, ti.dwID, ptInput, contact, primary);
                }

            }
            // process pInputs
            if (!User32.CloseTouchInputHandle((HTOUCHINPUT)lParam))
            {
                // error handling
            }
        }
        else
        {
            // GetLastError() and error handling
        }
    }
    else
    {
        // error handling, presumably out of memory
    }
}
#endif

void AppImpl::processDrawClipboard(WPARAM wParam, LPARAM lParam)
{
    SF_UNUSED2(wParam,lParam);

    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) 
        return; 
    if (!OpenClipboard(hWnd)) 
        return; 

    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT); 
    if (hglb != NULL) 
    { 
        const wchar_t* wstr = (const wchar_t*)GlobalLock(hglb); 
        if (wstr != NULL) 
        {
            pApp->OnUpdateCliboard(wstr);
            GlobalUnlock(hglb); 
        } 
    } 
    CloseClipboard(); 
}

void AppImpl::processChangeCBChain(WPARAM wParam, LPARAM lParam)
{
    // If the next window is closing, repair the chain.
    // Otherwise, pass the message to the next link. 
    if ((HWND) wParam == hWndNextViewer) 
        hWndNextViewer = (HWND) lParam; 
    else if (hWndNextViewer != NULL)
        SendMessage(hWndNextViewer, WM_CHANGECBCHAIN, wParam, lParam); 
}


void    AppImpl::ProcessUrl(const String& url)
{
    ShellExecute(NULL, "open", url.ToCStr(), NULL, NULL, SW_SHOWNORMAL);
}

LRESULT AppImpl::MemberWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        pApp->OnFocus((message == WM_SETFOCUS) ? true : false, updateModifiers());
        break;

    case WM_CREATE:
        hWndNextViewer = SetClipboardViewer(hWnd);
        //OnCreate();
        break;
    case WM_DESTROY:
        if (!hWndNextViewer)
            ChangeClipboardChain(hWnd, hWndNextViewer);
        //OnDestroy();
        break;

    case WM_CLOSE:
        // Default OnClose() will call Shutdown(), triggering PostQuitMessgae(0).
        pApp->OnClose();        
        return 0;

    case WM_SYSCOMMAND:
        // Look for screen-saver and power-save modes
        switch (wParam)
        {
        case SC_SCREENSAVE:     // Screen-saver is starting
        case SC_MONITORPOWER:   // Monitor is going to power-save mode
            // Prevent either from happening by returning 0
            return 0;
        default:
            break;
        }
        break;

    case WM_DROPFILES:
        {
            unsigned itemCount = ::DragQueryFile((HDROP)wParam, 0xFFFFFFFF,0,0);
            if (itemCount)
            {
                // Get name
                wchar_t buffer[512] = { 0 };
                ::DragQueryFileW((HDROP)wParam, 0, buffer, 512);
                ::DragFinish((HDROP)wParam);

                // Inform user about the drop
                pApp->OnDropFiles(String(buffer));
            }
        }
        return 0;

    case WM_SIZE:
        if (!BlockOnSize)
            pApp->OnSize(Size<unsigned>(LOWORD(lParam), HIWORD(lParam)));
        return 0;

    case WM_ENTERSIZEMOVE:
        pApp->OnSizeEnter(true);
        return 0;
    case WM_EXITSIZEMOVE:
        pApp->OnSizeEnter(false);
        return 0;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
        {
            bool downFlag = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) ? true : false;
            UInt8 scanCode = (UInt8)((lParam >> 16) & 0xff);
            bool  extendedKeyFlag = (lParam & (1 << 24)) != 0;
            // we need a scanCode to distinguish between left and right shifts
            if (wParam == VK_SHIFT && scanCode == 54) // right shift code
            {
                extendedKeyFlag = true;
            }
            //printf("%d %d %d\n", wParam, extendedKeyFlag, scanCode); //@DBG
            processKey((unsigned)wParam, downFlag, extendedKeyFlag);

            // Pass SYSKEY to DefWndProc to handle commands such as Alt-F4.
            if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP)
                break; 
            return 0;
        }
    case WM_CHAR:
        pApp->OnKey(0, Key::None, (unsigned)wParam, true, updateModifiers());
        break;

    case WM_IME_SETCONTEXT:
		return DefWindowProc(hWnd, message, wParam, 0);

    case WM_IME_STARTCOMPOSITION:
    case WM_IME_KEYDOWN:
    case WM_IME_COMPOSITION:        
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_NOTIFY:
    case WM_INPUTLANGCHANGE:
    case WM_IME_CHAR:
        // This message should not be passed down to DefWinProc otherwise it will
        // generate WM_CHAR messages which will cause text duplication.
        {
            if (pApp->OnIMEEvent(message, wParam, lParam, (UPInt)hWnd, false))
                return 0;
        }

        break;

    case WM_MOUSEMOVE:
        MousePos = Point<int>((SInt16)LOWORD(lParam), (SInt16)HIWORD(lParam));
        pApp->OnMouseMove(0, MousePos, updateModifiers());
        return 0;

        // Mouse wheel support
    case WM_MOUSEWHEEL:
        {
            // Nonclient position must be adjusted to be inside the window
            POINT   wcl = {0,0};
            ::ClientToScreen(hWnd, &wcl);
            MousePos = Point<int>(int(SInt16(LOWORD(lParam))) - wcl.x, int(SInt16(HIWORD(lParam))) - wcl.y);
            float delta = (float)SInt16(HIWORD(wParam))/(float)WHEEL_DELTA * 3;
            pApp->OnMouseWheel(0, delta, MousePos, updateModifiers());
            return 0;
        }

    case WM_SETCURSOR:
        // Set cursor to the desired type.
        if ((HWND)wParam == hWnd)
        {
            bool clientArea = (LOWORD(lParam) == HTCLIENT);
            pApp->Cursor.OnSetCursorMessage(clientArea);
            // Pass-thorugh non-client cursors to allow DefWindowProc to apply
            // a proper one. This is important for resizable windows, as they
            // can have different cursor types.
            if (clientArea)
                return 1;
        }
        break;
        
        // Mouse button support

    case WM_LBUTTONDOWN:
        processMouseButton(0, true, lParam);
        return 0;
    case WM_LBUTTONUP:
        processMouseButton(0, false, lParam);
        return 0;
    case WM_RBUTTONDOWN:
        processMouseButton(1, true, lParam);
        return 0;
    case WM_RBUTTONUP:
        processMouseButton(1, false, lParam);
        return 0;
    case WM_MBUTTONDOWN:
        processMouseButton(2, true, lParam);
        return 0;
    case WM_MBUTTONUP:
        processMouseButton(2, false, lParam);
        return 0;
        // XButton support
    case WM_XBUTTONDOWN:
        processMouseButton(2+GET_XBUTTON_WPARAM(wParam), true, lParam);
        return 0;
    case WM_XBUTTONUP:
        processMouseButton(2+GET_XBUTTON_WPARAM(wParam), false, lParam);
        return 0;
/*
    case WM_WINDOWPOSCHANGED:
        return 0;
        break;
*/
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO pmmi = (LPMINMAXINFO) lParam;
            bool handled = false;
            if (Config.MinSize.Width != 0) 
            {
                pmmi->ptMinTrackSize.x = Config.MinSize.Width; // Width
                handled = true;
            }
            if (Config.MinSize.Height != 0)
            {
                pmmi->ptMinTrackSize.y = Config.MinSize.Height; // Height
                handled = true;
            }   

            // If in autotesting, do not clamp the size of the window.
            if (pApp->GetArgs().HasValue("AutoPlayback"))
            {
                LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
                lpmmi->ptMaxSize.x = lpmmi->ptMaxSize.y = 65535;
                lpmmi->ptMaxTrackSize.x = lpmmi->ptMaxTrackSize.y =65535;
                handled = true;
            }

            if (handled) 
                return 0;
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            NeedPaint = true;
        }
        break;

        // Clipboard related messages.
    case WM_DRAWCLIPBOARD:
        processDrawClipboard(wParam, lParam);
        break;
    case WM_CHANGECBCHAIN:
        processChangeCBChain(wParam, lParam);
        break;

#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    case WM_TOUCH:
        processTouch(wParam, lParam);
        break;
#endif
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


#if (WINVER >= 0x0601) && defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
template <typename T>
Point<int> AppImpl::ScreenToClient(T x, T y)
{
    POINT pt;
    pt.x = (int)x;
    pt.y = (int)y;
    ::ScreenToClient(hWnd, &pt);
    return Point<int>(pt.x, pt.y);
}

ManipulationEventListener::ManipulationEventListener(AppImpl* appImpl) :
RefCount(1), pConnection(NULL), Cookie(0), pAppImpl(appImpl)
{
    SF_ASSERT(pAppImpl);
}

ManipulationEventListener::~ManipulationEventListener()
{
    SF_ASSERT(!Cookie && !pConnection); 
}

bool ManipulationEventListener::Connect(IManipulationProcessor* manipProc)
{
    // Check input arguments
    if (!manipProc)
    {
        SF_ASSERT(0); // incorrect manipProc
        return false;
    }

    // Check object state
    if (Cookie || pConnection)
    {
        SF_ASSERT(0); // connection exists
        return false;
    }

    // Get the container with the connection points.
    IConnectionPointContainer* pconnectionContainer = NULL;
    HRESULT hr = manipProc->QueryInterface(&pconnectionContainer);
    if (FAILED(hr))
    {
        SF_ASSERT(0); // can't get interface for connection point container
        return false;
    }

    // Get a connection point.
    hr = pconnectionContainer->FindConnectionPoint(__uuidof(_IManipulationEvents), &pConnection);
    if (FAILED(hr))
    {
        SF_ASSERT(0); // failed to get a connection point
        pconnectionContainer->Release();
        return false;
    }

    // Release the connection container.
    pconnectionContainer->Release();

    // Advise. Establishes an advisory connection between the connection point 
    // and the caller's sink object. 
    hr = pConnection->Advise(this, &Cookie);
    if (FAILED(hr))
    {
        SF_ASSERT(0); // failed to Advise
        pConnection->Release();
        pConnection = NULL;
        return false;
    }

    return true;
}

bool ManipulationEventListener::Disconnect()
{
    // Check object state
    if (!Cookie || !pConnection)
    {
        SF_ASSERT(0); // connection does not exist
        return false;
    }

    // Unadvise. Terminate the connection.
    HRESULT hr = pConnection->Unadvise(Cookie);
    SF_ASSERT(SUCCEEDED(hr)); // failed to Unadvise?
    SF_UNUSED(hr);

    pConnection->Release();
    pConnection = NULL;
    Cookie = 0;

    return true;
}

// IManipulationEvents implementation

// This event is called by the ManipulationProcessor when manipulation 
// is detected (starts). Not used by this application.
// in:
//     x - x coordinate of the initial point of manipulation 
//         (1/100 of pixel)
//     y - y coordinate of the initial point of manipulation 
//         (1/100 of pixel)
HRESULT STDMETHODCALLTYPE ManipulationEventListener::ManipulationStarted(
    FLOAT x,
    FLOAT y)
{
    Point<int> pt = pAppImpl->ScreenToClient(TOUCH_COORD_TO_PIXEL(x), TOUCH_COORD_TO_PIXEL(y));
    //pApp->OnGestureBegin(pt.x, pt.y);
    pAppImpl->FirstTouchPt = pt;
    return S_OK;
}

// This event is called by the ManipulationProcessor during the movement of 
// the fingers.
// in:
//                      x - x coordiante of the initial point of manipulation 
//                          (1/100 of pixel)
//                      y - y coordiante of the initial point of manipulation 
//                          (1/100 of pixel)
//      translationDeltaX - shift of the x-coordinate (1/100 of pixel)
//      translationDeltaY - shift of the y-coordinate (1/100 of pixel)
//             scaleDelta - scale factor (zoom in/out)
//         expansionDelta - the current rate of scale change
//          rotationDelta - rotation angle in radians
// cumulativeTranslationX - cumulative shift of x-coordinate (1/100 of pixel)
// cumulativeTranslationY - cumulative shift of y-coordinate (1/100 of pixel)
//        cumulativeScale - cumulative scale factor (zoom in/out)
//    cumulativeExpansion - cumulative rate of scale change
//     cumulativeRotation - cumulative rotation angle in radians
HRESULT STDMETHODCALLTYPE ManipulationEventListener::ManipulationDelta(
    FLOAT x,
    FLOAT y,
    FLOAT translationDeltaX,
    FLOAT translationDeltaY,
    FLOAT scaleDelta,
    FLOAT /* expansionDelta */,
    FLOAT rotationDelta,
    FLOAT /* cumulativeTranslationX */,
    FLOAT /* cumulativeTranslationY */,
    FLOAT /* cumulativeScale */,
    FLOAT /* cumulativeExpansion */,
    FLOAT /* cumulativeRotation */)
{
    UInt32 newMask = 0;
    if (translationDeltaX != 0 || translationDeltaY != 0)
        newMask |= AppBase::GestureBit_Pan;
    if (scaleDelta != 1.0f)
        newMask |= AppBase::GestureBit_Zoom;
    if (rotationDelta != 0)
        newMask |= AppBase::GestureBit_Rotate;
    UInt32 m = pAppImpl->ActiveGestureMask ^ newMask;
    
    PointF translation(TOUCH_COORD_TO_PIXEL(translationDeltaX), TOUCH_COORD_TO_PIXEL(translationDeltaY));
    PointF scale(scaleDelta, scaleDelta);

    if (m)
    {
        pAppImpl->pApp->OnGestureBegin(0, m, pAppImpl->FirstTouchPt, translation, scale,
                                            float(rotationDelta * 180 / SF_MATH_PI));
        pAppImpl->ActiveGestureMask |= m;
    }
    else
    {
        Point<int> pt = pAppImpl->ScreenToClient(TOUCH_COORD_TO_PIXEL(x), TOUCH_COORD_TO_PIXEL(y));
        pAppImpl->pApp->OnGesture(0, newMask, pt, translation, scale, 
                                       float(rotationDelta * 180 / SF_MATH_PI));
    }

    return S_OK;
}

// This event is called by the ManipulationProcessor when manipulation is 
// completed. Not used by this application. 
// in:
//                      x - x coordinate of the initial point of manipulation 
//                          (1/100 of pixel)
//                      y - y coordinate of the initial point of manipulation 
//                          (1/100 of pixel)
// cumulativeTranslationX - cumulative shift of x-coordinate (1/100 of pixel)
// cumulativeTranslationY - cumulative shift of y-coordinate (1/100 of pixel)
//        cumulativeScale - cumulative scale factor (zoom in/out)
//    cumulativeExpansion - cumulative rate of scale change
//     cumulativeRotation - cumulative rotation angle in radians
HRESULT STDMETHODCALLTYPE ManipulationEventListener::ManipulationCompleted(
    FLOAT x,
    FLOAT y,
    FLOAT /* cumulativeTranslationX */,
    FLOAT /* cumulativeTranslationY */,
    FLOAT /* cumulativeScale */,
    FLOAT /* cumulativeExpansion */,
    FLOAT /* cumulativeRotation */)
{
    Point<int> pt = pAppImpl->ScreenToClient(TOUCH_COORD_TO_PIXEL(x), TOUCH_COORD_TO_PIXEL(y));
    pAppImpl->pApp->OnGestureEnd(0, pAppImpl->ActiveGestureMask, pt);
    pAppImpl->ActiveGestureMask = 0;
    pAppImpl->FirstTouchPt.x = pAppImpl->FirstTouchPt.y = 0;
    return S_OK;
}

// IUnknown implementation
ULONG ManipulationEventListener::AddRef(void)
{
    return InterlockedIncrement(&RefCount);
}

ULONG ManipulationEventListener::Release(void)
{
    ULONG newRefCount = InterlockedDecrement(&RefCount);
    if (newRefCount == 0)
    {
        delete this;
    }
    return newRefCount;
}

HRESULT ManipulationEventListener::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if ((riid == __uuidof(_IManipulationEvents)) || (riid == IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}
#endif

// Handles invalid parameters passed to secure CRT functions (eg. printf_s), when -nerr option is passed on the command line.
void InvalidParameterHandler(const wchar_t * expression,
                             const wchar_t * function, 
                             const wchar_t * file, 
                             unsigned int line,
                             uintptr_t pReserved )
{
    SF_UNUSED(pReserved);
    wprintf(L"Invalid parameter detected in function %s."
        L" File: %s Line: %d\n", function, file, line);
    wprintf(L"Expression: %s\n", expression);

    // The default invalid parameter handler exits the process, so do that as well.
    abort();
}

// Writes minidump files when -dump is passed on the command line.
static char MinidumpFilename[_MAX_PATH];
LONG WINAPI WriteMinidumpUnhandledExceptionFilter( __in struct _EXCEPTION_POINTERS* ExceptionInfo )
{
    // Construct the dump filename, and open the handle.
    HANDLE dumpFile = INVALID_HANDLE_VALUE;
    if ( SFstrlen(MinidumpFilename) == 0)
    {
        GetModuleFileName(0, MinidumpFilename, _MAX_PATH );
        SFstrcat(MinidumpFilename, _MAX_PATH, ".dmp");
    }

    // Now actually write the dump file.
    dumpFile = CreateFile(MinidumpFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
    MINIDUMP_EXCEPTION_INFORMATION exception = { ::GetCurrentThreadId(), ExceptionInfo, TRUE };
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, MiniDumpWithIndirectlyReferencedMemory, &exception, 0, 0 );
    CloseHandle(dumpFile);
    fprintf_s(stderr, "Encountered unhandled exception! Writing minidump to %s\n", MinidumpFilename);
    return EXCEPTION_CONTINUE_SEARCH;
}

// Called after parsing the command line; parseResult should be checked
// to determine action to take.
bool AppImpl::OnArgs(const Args& args, Args::ParseResult parseResult)
{
    if ( !AppImplBase::OnArgs(args, parseResult) )
        return false;

    // Check for disabling error dialogs.
    if ( args.GetBool("NoDebugPopups") || args.GetString("Minidump").GetLength() > 0 )
    {
        // Redirects asserts and errors to stderr (in debug mode only).
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
        _CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);

        // Prints CRT invalid parameter errors to stderr, instead of an error dialog.
        _set_invalid_parameter_handler(InvalidParameterHandler);

        // Disables all windows error dialogs.
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX );
    }

    // Check for creating minidumps.
    MinidumpFilename[0] = 0;
    if (args.GetString("Minidump").GetLength() > 0)
    {
        String minidumpName = args.GetString("Minidump"); 
        SFstrcpy(MinidumpFilename, _MAX_PATH, minidumpName.ToCStr());
        SetUnhandledExceptionFilter(WriteMinidumpUnhandledExceptionFilter);
    }
    
    // Test various ways the program can exit with an error dialog/minidump.
    //int * i = 0;
    //*i = 5;
    //SF_ASSERT(0);
    //sprintf_s(0, 1, "%s", "a");

    return true;
}

//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase()
 : 
    Cursor(true),
    ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}

AppBase::~AppBase()
{
    delete pImpl;
}

void AppBase::SetOrientationMode(bool)
{
}

void AppBase::MinimizeWindow()
{

}

void AppBase::MaximizeWindow()
{

}

void AppBase::RestoreWindow()
{

}

//------------------------------------------------------------------------

// This call cleans up the application and kills the window.
void AppBase::Shutdown()
{
    ::PostQuitMessage(0);
    ShuttingDown = true;
}

void AppBase::BringWindowToFront()
{
    if (pImpl->hWnd)
    {        
        ::BringWindowToTop(pImpl->hWnd);
        ::SetForegroundWindow(pImpl->hWnd);
    }
}

Device::Window* AppBase::GetDeviceWindow() const
{
    return pImpl;
}

bool AppBase::IsConsole()
{
    return false;
}

bool    AppBase::IsPadConnected(unsigned pad) 
{
	return pImpl->Pad.IsConnected(pad);
}
// Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
// This is a floating point value such as 0.1 for 10% safe frame border.
Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.0f);
}

const char* AppBase::GetDefaultFilePath()
{
    return "";
}

String AppBase::GetContentDirectory()
{
    return "";
}

int AppBase::GetMouseCount()
{
    return 1;
}
int AppBase::GetKeyboardCount()
{
    int cnt = 0;
    for (unsigned i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        if (pImpl->Pad.IsConnected(i))
            ++cnt;
    }
    return Alg::Max(1, cnt);
}

UInt32 AppBase::GetCaps() const
{
    return 0;
}

// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{    
   // Scaleform::System initSFSystem;

    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return -1;	// ERROR
    
    // Initialize application and its window.
    ViewConfig config(Device::GetDefaultViewSize());
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return -2;	// ERROR
    }

    // Application / Player message loop.
    MSG msg;
    memset(&msg, 0, sizeof(msg));
    
    pImpl->NextUpdateTicks = Timer::GetTicks() + pImpl->FrameTimeTicks;

    while(msg.message != WM_QUIT)
    {
        XInputEventAdapter adapter(this);
		pImpl->Pad.Update(adapter);

        if (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
#if defined(SF_ENABLE_IME) 
			//HWND    hwndIME     = ImmGetDefaultIMEWnd(hWND);

			bool imeUIMsg = (ImmIsUIMessage(NULL, msg.message, msg.wParam, msg.lParam) == 0)? FALSE: TRUE;
			if((msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP) || imeUIMsg
				|| (msg.message == WM_LBUTTONDOWN) || (msg.message == WM_LBUTTONUP))
			{
				OnIMEEvent(msg.message, msg.wParam, msg.lParam, (UPInt)pImpl->hWnd, true);
			}

#endif		
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (!IsShuttingDown())
        {
            // Wait for Update time if necessary; consider timing wrap-around.
            
            UInt64  ticks = Timer::GetTicks();
            if (pImpl->FrameTime != 0.0f)
            {                   
                if ((pImpl->NextUpdateTicks > ticks) && !pImpl->NeedPaint)
                {
                    UInt64   waitTicks = pImpl->NextUpdateTicks - ticks;
                    unsigned waitMs = (unsigned)(waitTicks / 1000);
                    DWORD dww = ::MsgWaitForMultipleObjects(0,0,0, waitMs, QS_ALLEVENTS);
                    // WAIT_OBJECT_0 is returned when message is fired. Force
                    // Advance to be called next iteration via setting pImpl->NeedPaint 
                    // to true.
                    if (dww == WAIT_OBJECT_0)
                        pImpl->NeedPaint = true;
                    continue;
                }
            }
            OnUpdateFrame(pImpl->NeedPaint);
            if (pImpl->FrameTime != 0.0f)
                pImpl->NextUpdateTicks = ticks + pImpl->FrameTimeTicks;
            pImpl->NeedPaint = false;
        }
    }

    OnShutdown();

    return 0;	// OK
}

//------------------------------------------------------------------------
class SystemCursorManagerImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    SystemCursorManagerImpl()
    {
        CurrentType = Cursor_Hidden;
        hCursors[Cursor_Hidden] = 0;
        hCursors[Cursor_Arrow]  = LoadCursor(NULL, IDC_ARROW);
        hCursors[Cursor_Hand]   = LoadCursor(NULL, IDC_HAND);
        hCursors[Cursor_IBeam]  = LoadCursor(NULL, IDC_IBEAM);
    }
    HCURSOR     hCursors[Cursor_Type_Count];

    // Current hVursor state, to support update; NULL for hidden. 
    CursorType  CurrentType; 
};


//------------------------------------------------------------------------
SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) : 
    SystemCursorState(systemCursorAvailable, true),
    pImpl(0)
{
    pImpl = SF_NEW SystemCursorManagerImpl;
}

SystemCursorManager::~SystemCursorManager()
{
    delete pImpl;
}

void SystemCursorManager::update(bool forceSet)
{
    bool systemShown = !InClientArea || Enabled && !Software && Type != Cursor_Hidden;
    if ((pImpl->CurrentType != Cursor_Hidden) != systemShown)
    {
        // Work around ref-counted cursor state.
        if (systemShown)
        {
            do { } while(::ShowCursor(TRUE) < 0);
        }
        else
        {
            do { } while(::ShowCursor(FALSE) >= 0);
            pImpl->CurrentType = Cursor_Hidden;
            return;
        }
    }

    CursorType type = InClientArea ? Type : Cursor_Arrow;
    if ((pImpl->CurrentType != type) || forceSet)
    {
        pImpl->CurrentType = type;
        if (InClientArea)
            ::SetCursor(pImpl->hCursors[pImpl->CurrentType]);
    }
}

//------------------------------------------------------------------------
class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern) :
        DirectoryImplBase(path, pattern) { } 

    virtual void ReadDirectory(const String& path, const String& pattern);
    virtual int  GetFileIndex(const String& filename) const;
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Pattern = pattern;
    Path = path;
    if (Path == ".")
    {
        Path += "/";
    }
    unsigned start = 0;
    String winpath  = "";
    unsigned length = (unsigned)Pattern.GetLength();
    for (unsigned i = 0; i < Pattern.GetLength(); i++ )
    {
        //bool s = false;
        String sp;
        if (Pattern.GetCharAt(i) == ';') 
            sp = Pattern.Substring(start, i);
        else if (i == length - 1)
            sp = Pattern.Substring(start, i + 1);
        else
            continue;
        winpath = Path + sp;
        WIN32_FIND_DATAA dp;
        HANDLE hFind = FindFirstFileA(winpath, &dp);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            String lowerFilename = dp.cFileName;
            lowerFilename = lowerFilename.ToLower();
            Filenames.PushBack(lowerFilename);
            while (FindNextFileA(hFind, &dp))
            {
                lowerFilename = dp.cFileName;
                lowerFilename = lowerFilename.ToLower();
                Filenames.PushBack(lowerFilename);
            }
            FindClose(hFind);
        }
        start = i+1;
    }

    CurFile = 0;
}

int DirectoryImpl::GetFileIndex(const String& filename) const
{
    String fileOnly = filename.GetFilename();
    fileOnly = fileOnly.ToLower();
    for ( unsigned i = 0; i < (unsigned)Filenames.GetSize(); ++i )
    {
        if ( Filenames[i] == fileOnly )
            return i;
    }
    return -1;
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}

}} // Scaleform::Platform


//------------------------------------------------------------------------
//wmain can not be defined in platform library because of ambiguous entry point issue
//it is defined in SF_PLATFORM_SYSTEM_APP_ARGS macro
int Platform_WinAPI_MainW(int argc, wchar_t **wargv)
{
    char **argv = (char**)malloc(argc * sizeof (char*));
    for (int i = 0; i < argc; i++)
    {
        Scaleform::SPInt numChars = Scaleform::UTF8Util::GetEncodeStringSize(wargv[i]) + 1;
        argv[i] = (char*)malloc( numChars * sizeof(char) );
        Scaleform::UTF8Util::EncodeStringSafe(argv[i], numChars, wargv[i]);
    }

    int res = Platform_WinAPI_MainA(argc, argv);

    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);

    return res;
}

int Platform_WinAPI_MainA(int argc, char** argv)
{
    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int result = -1;
    if (app)
        result = app->AppMain(argc, argv);

    Scaleform::Platform::AppBase::DestroyInstance(app);
    if (result == 0 && Scaleform::System::HasMemoryLeaks)
        result = -255;

    return result;
}
