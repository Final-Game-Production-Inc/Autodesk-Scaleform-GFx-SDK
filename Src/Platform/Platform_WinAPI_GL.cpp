/**************************************************************************

Filename    :   Platform_WinAPI_GL.cpp
Content     :   OpenGL Platform Support under Win32
Created     :   2009
Authors     :   Mustafa Thamer

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/GL/GL_HAL.h"
#include "Platform.h"
#include "wglext.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Kernel/SF_SysFile.h"
#include <ShlObj.h>

#define     OVERSIZE            1.0f

namespace Scaleform { namespace Platform {

 // Video mode
#define  SF_APP_GL_COLOR_BITS         32
#define  SF_APP_GL_DEPTH_BITS         24
#define  SF_APP_GL_STENCIL_BITS       8
#define  SF_APP_GL_ACCUM_BITS         0

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue* commandQueue);
    ~DeviceImpl();

    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId );
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();

    void setVSync(bool isEnabled);
    void setFSAA( bool isEnabled);
    bool setupPixelFormat(const ViewConfig& config);
    bool setFullScreenMode(const ViewConfig& config);
    bool checkFSAASupport(const ViewConfig& config);
    void calculatePixelFormat(const ViewConfig& config, PIXELFORMATDESCRIPTOR & pfd ) const;
    void calculatePixelFormat(const ViewConfig& config, GLint * intDest, GLfloat * floatDest ) const;
    bool takeScreenShot(const String& filename);

    Device::Window*       pWindow;
    Ptr<Render::GL::HAL>  pHal;
    DeviceStatus          Status;
    class AppBase*        pApp;

    // OpenGL Variables
    HWND     hWnd;
    HDC      hDC;
    HGLRC    hGLRC;
    bool     FSAASupported;

    // These are used in calculatePixelFormat (wgl version).
    static const int MAX_PIXELFORMAT_INTS = 32;
    static const int MAX_PIXELFORMAT_FLOATS = 2;
};


DeviceImpl::DeviceImpl(Render::ThreadCommandQueue* commandQueue)
 : pWindow(0), 
   pHal(0),
   Status(Device_NeedInit),
   hWnd(0),
   hDC(0), 
   hGLRC(0), 
   FSAASupported(false)
{
    pHal = *SF_NEW Render::GL::ProfilerHAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
}



void DeviceImpl::setVSync(bool isEnabled)
{
    PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = NULL;
    wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapInterval) wglSwapInterval(isEnabled ? 1 : 0);
}

void DeviceImpl::setFSAA( bool isEnabled)
{
    if ( FSAASupported && isEnabled )
        glEnable(GL_MULTISAMPLE_ARB);
    else
        glDisable(GL_MULTISAMPLE_ARB);
}

bool DeviceImpl::setFullScreenMode(const ViewConfig& config)
{
    // If we're in fullscreen mode, set up the display
    if (config.ViewFlags & View_FullScreen)
    {
        // Set up the device mode structure
        DEVMODE screenSettings;
        memset(&screenSettings,0,sizeof(screenSettings));

        screenSettings.dmSize       = sizeof(screenSettings);
        screenSettings.dmPelsWidth  = config.ViewSize.Width;
        screenSettings.dmPelsHeight = config.ViewSize.Height;
        screenSettings.dmBitsPerPel = config.ColorBits == -1 ? SF_APP_GL_COLOR_BITS : config.ColorBits;    // bits per pixel
        screenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Attempt to switch to the resolution and bit depth we've selected
        if (ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            return false;
        }
    }
    return true;
}

class ScopedTempWindow
{
public:
    ScopedTempWindow(HWND wnd) : hWnd(wnd) { }
    ~ScopedTempWindow() { ::DestroyWindow(hWnd); }
    HWND hWnd;
};

// We need to create a temporary window, and GL context, to query whether FSAA
// is available or not. The window is destroyed immediately afterwards.
bool DeviceImpl::checkFSAASupport(const ViewConfig& config)
{
    FSAASupported = false;
    PIXELFORMATDESCRIPTOR pfd;

#ifdef UNICODE
    #define STATIC_WINDOW_CLASS L"STATIC"
    #define TEMP_WINDOW_NAME    L"TEMP"
#else
    #define STATIC_WINDOW_CLASS "STATIC"
    #define TEMP_WINDOW_NAME    "TEMP"
#endif

    HWND tempWnd = ::CreateWindow(STATIC_WINDOW_CLASS, TEMP_WINDOW_NAME,
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 16, 16, 
        NULL, NULL, GetModuleHandle(NULL), NULL );
    ScopedTempWindow tempScope(tempWnd);

    // Couldn't create the window, bail.
    if ( !tempWnd )
        return FSAASupported;

    HDC tempDC = GetDC(tempWnd);
    calculatePixelFormat(config, pfd);
    int pixelFormat = ChoosePixelFormat(tempDC, &pfd);
    if ( pixelFormat == 0 )
        return FSAASupported;

    if(!SetPixelFormat(tempDC, pixelFormat, &pfd))
        return FSAASupported;

    // Create OpenGL rendering context and make it active.
    HGLRC hglrc;
    if ((hglrc = wglCreateContext(tempDC))==0)
        return false;

    if (!wglMakeCurrent(tempDC, hglrc))
        return false;

    // Get the extension function.
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
        return FSAASupported;

    // Now actually make the call.
    GLint format;
    GLuint numFormats;
    int intAttributes[MAX_PIXELFORMAT_INTS];
    float floatAttributes[MAX_PIXELFORMAT_FLOATS];
    calculatePixelFormat(config, intAttributes, floatAttributes );
    FSAASupported = wglChoosePixelFormatARB(tempDC,intAttributes, 
        floatAttributes, 1, &format, &numFormats ) && numFormats >= 1;

    return FSAASupported;
}

void DeviceImpl::calculatePixelFormat(const ViewConfig& config, PIXELFORMATDESCRIPTOR & pfd ) const
{
    memset (&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize =         sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion =      1;
    pfd.dwFlags =       PFD_DRAW_TO_WINDOW |  // Window drawing support
                        PFD_SUPPORT_OPENGL |  // OpenGL support
                        PFD_DOUBLEBUFFER;
    pfd.iPixelType =    PFD_TYPE_RGBA;
    pfd.cColorBits   =  (BYTE)(config.ColorBits == -1 ? SF_APP_GL_COLOR_BITS : config.ColorBits);
    pfd.cDepthBits   =  (BYTE)(config.DepthBits == -1 ? SF_APP_GL_DEPTH_BITS : config.DepthBits);
    pfd.cStencilBits =  (BYTE)(config.StencilBits == -1 ? SF_APP_GL_STENCIL_BITS : config.StencilBits);
    pfd.cAccumBits =    SF_APP_GL_ACCUM_BITS;
    pfd.iLayerType =    PFD_MAIN_PLANE;
}

void DeviceImpl::calculatePixelFormat(const ViewConfig& config, GLint * intDest, GLfloat * floatDest ) const
{
    static GLfloat floatAttributes[] = 
    {0, 0};

    GLint intAttributes[] = 
    {
        WGL_COLOR_BITS_ARB,     config.ColorBits   == -1 ? SF_APP_GL_COLOR_BITS : config.ColorBits,
        WGL_DEPTH_BITS_ARB,     config.DepthBits   == -1 ? SF_APP_GL_DEPTH_BITS : config.DepthBits,
        WGL_STENCIL_BITS_ARB,   config.StencilBits == -1 ? SF_APP_GL_STENCIL_BITS : config.StencilBits,
        WGL_ACCUM_BITS_ARB,     SF_APP_GL_ACCUM_BITS,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB,        4,       // Just use 4x FSAA.
        0, 0
    };

    memcpy( intDest, intAttributes, sizeof intAttributes );
    memcpy( floatDest, floatAttributes, sizeof floatAttributes );
}

// Sets up the pixel format according to the settings. Note that, FSAA is supported
// then the pixel format is always choosen to be an FSAA compatible one, so that,
// we can toggle FSAA by simply changing the GL_MULTISAMPLE_ARB state.
bool DeviceImpl::setupPixelFormat(const ViewConfig& config)
{   
    GLint  pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;  
    calculatePixelFormat(config, pfd);

    if ( !FSAASupported )
    {
        // Find the closest available pixel format
        if ((pixelFormat = ChoosePixelFormat(hDC, &pfd))==0)
        {
            if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
                fprintf(stderr, "Can't choose an appropriate pixel format\n");
            else
	            MessageBoxA(NULL, "Can't choose an appropriate pixel format",
                        "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
            return false;
        }
    }
    else
    {
        // Get the extension function.
        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        if (!wglChoosePixelFormatARB)
        {
            if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
                fprintf(stderr, "Unable to choose appropriate FSAA pixel format\n");
            else
                MessageBoxA(NULL, "Unable to choose appropriate FSAA pixel format",
                    "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION );   
            return false;
        }

        HDC hDCGL = GetDC(hWnd);
        GLuint numFormats;
        int intAttributes[MAX_PIXELFORMAT_INTS];
        float floatAttributes[MAX_PIXELFORMAT_FLOATS];
        calculatePixelFormat(config, intAttributes, floatAttributes );
        if (!wglChoosePixelFormatARB(hDCGL,intAttributes, floatAttributes, 1, 
            &pixelFormat, &numFormats ) || numFormats <= 0 )
        {
            if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
                fprintf(stderr, "Unable to choose appropriate FSAA pixel formatt\n");
            else
                MessageBoxA(NULL, "Unable to choose appropriate FSAA pixel format",
                    "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION );
            return false;
        }
    }

    // Compare what we got, with what we requested.
    PIXELFORMATDESCRIPTOR chosenPFD;
    if (DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &chosenPFD))
    {
        SF_DEBUG_WARNING2(chosenPFD.cColorBits   != pfd.cColorBits,   "Using %dbit color buffer (%dbit was requested)",   chosenPFD.cColorBits,   pfd.cColorBits);
        SF_DEBUG_WARNING2(chosenPFD.cDepthBits   != pfd.cDepthBits,   "Using %dbit depth buffer (%dbit was requested)",   chosenPFD.cDepthBits,   pfd.cDepthBits);
        SF_DEBUG_WARNING2(chosenPFD.cStencilBits != pfd.cStencilBits, "Using %dbit stencil buffer (%dbit was requested)", chosenPFD.cStencilBits, pfd.cStencilBits);
    }

    // Set pixel format to device context
    if(!SetPixelFormat(hDC, pixelFormat, &pfd))
    {
        if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
            fprintf(stderr, "Unable to set pixel format\n");
        else
            MessageBoxA(NULL, "Unable to set pixel format",
                    "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    return true;
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    if ( config->HasFlag(View_FullScreen) )
    {
        if ( !config->HasFlag(View_Size) )
        {
            int screenW = ::GetSystemMetrics(SM_CXSCREEN);
            int screenH = ::GetSystemMetrics(SM_CYSCREEN);
            config->SetSize(screenW, screenH);
        }


        config->SetPos(0,0);
    }
    else
    {
        if ( config->HasFlag(View_Size))
        {
            config->ViewSize = config->ViewSize.ExpandedTo(1,1);
        }
    }
    config->SetFlag(View_Stereo, false);

    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId )
{
    if (!pHal || !window)
        return false;

    pApp    = ((AppImplBase*)window)->pApp;     // for popup control
    pWindow = window;
    hWnd    = (HWND)window->GetHandle();

    if ((hDC = GetDC(hWnd)) == 0)
    {
        if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
            fprintf(stderr, "Unable to create device context\n");
        else
            MessageBoxA(NULL,"Unable to create device context",
                    "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);      
        return false;
    }

    // Determine FSAA support.
    FSAASupported = checkFSAASupport(config);

    // Setup fullscreen (if requested).
    if (!setFullScreenMode(config))
    {
        if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
            fprintf(stderr, "Unable to set fullscreen mode\n");
        else
            MessageBoxA(NULL, "Unable to set fullscreen mode",
                "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    // Setup pixel format.
    if (!setupPixelFormat(config))
    {
        if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
            fprintf(stderr, "Unable to set pixel format\n");
        else
            MessageBoxA(NULL, "Unable to set pixel format",
                "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    hGLRC = 0;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
    // Don't create a context with Core (3.x) attributes if the user has requested a Legacy (2.x) context.
    if (config.ViewFlags & View_GL20)
    {
        wglCreateContextAttribsARB = NULL;
    }

    if (wglCreateContextAttribsARB)
    {
        const int MaxCreateAttributes = 10;
        int currentAttribute = 0;
        int createAttributes[MaxCreateAttributes];

        // wglCreateContextAttribsARB will always return us a with all the features of the context we request.
        // Request 3.2, as it will return us a core profile, without the legacy features.
        createAttributes[currentAttribute++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
        createAttributes[currentAttribute++] = 3;
        createAttributes[currentAttribute++] = WGL_CONTEXT_MINOR_VERSION_ARB;
        createAttributes[currentAttribute++] = 2;
        createAttributes[currentAttribute++] = WGL_CONTEXT_PROFILE_MASK_ARB;
        createAttributes[currentAttribute++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

        // If GL debug_output is desired, the context must be created with the debug flag.
        if (config.ViewFlags & View_DebugMessages)
        {
            createAttributes[currentAttribute++] = WGL_CONTEXT_FLAGS_ARB;
            createAttributes[currentAttribute++] = WGL_CONTEXT_DEBUG_BIT_ARB;
        }
        createAttributes[currentAttribute++] = 0;
        hGLRC = wglCreateContextAttribsARB(hDC, NULL, createAttributes);
    }
    
    if (hGLRC == 0)
    {
        // Create OpenGL rendering context and make it active.
        if ((hGLRC = wglCreateContext(hDC))==0)
        {
            if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
                fprintf(stderr, "Unable to create OpenGL rendering context\n");
            else
                MessageBoxA(NULL, "Unable to create OpenGL rendering context",
                "OpenGLApp Error",MB_OK | MB_ICONEXCLAMATION);
            return false;
        }
    }
    // Get the FSAA check context.
    HGLRC holdglrc = wglGetCurrentContext();

    // Make the rendering context current.
    if (!wglMakeCurrent(hDC, hGLRC))
    {       
        if (pApp && pApp->GetArgs().GetBool("NoDebugPopups"))
            fprintf(stderr, "Unable to activate OpenGL rendering context\n");
        else
            MessageBoxA(NULL,"Unable to activate OpenGL rendering context",
                "OpenGLApp Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    // Delete the FSAA context (if it exists).
    if ( holdglrc )
        wglDeleteContext(holdglrc);

    // Set VSync, depending on the flag
    setVSync(config.HasFlag(View_VSync));

    // Set FSAA, if requested (and available).
    setFSAA(config.HasFlag(View_FSAA));

    // Configure renderer in "Dependent mode", honoring externally
    // configured device settings.
    unsigned flags = 0;

    if (!config.HasFlag(View_PrecompileShaders))
        flags |= Render::GL::HALConfig_DynamicShaderCompile;

    if (config.HasFlag(View_DebugMessages))
        flags |= Render::GL::HALConfig_DebugMessages;

    if (!pHal->InitHAL(Render::GL::HALInitParams(flags, renderThreadId)))
    {
        shutdownGraphics();
        return false;
    }

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
    // Set VSync, depending on the flag
    setVSync(config.HasFlag(View_VSync));

    // Set FSAA, if requested (and available).
    setFSAA(config.HasFlag(View_FSAA));

    return true;
}

void DeviceImpl::shutdownGraphics()
{
    if (pHal)
        pHal->ShutdownHAL();

    Status = Device_NeedInit;
    pWindow = 0;
    hWnd = 0;

    if ( hGLRC )
    {
        wglMakeCurrent( NULL, NULL );
        wglDeleteContext( hGLRC );
        hGLRC = 0;
    }
}

//------------------------------------------------------------------------
Device::Device(Render::ThreadCommandQueue *commandQueue)
{
	pImpl = SF_NEW DeviceImpl(commandQueue);
}

Device::~Device()
{
    delete pImpl;
}

Render::HAL* Device::GetHAL() const
{
    return pImpl->pHal;
}

DeviceStatus Device::GetStatus() const
{
    return pImpl->Status;
}

void Device::ResizeFrame(void*) { }
    
bool Device::GraphicsConfigOnMainThread() const
{
    // GL does not require that the window config happens on the main thread.
    return false;
}

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

bool Device::InitGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId )
{
    return pImpl->initGraphics(config, window, renderThreadId);
}

bool Device::ReconfigureGraphics(const ViewConfig& config)
{
    return pImpl->reconfigureGraphics(config);
}

void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

Size<unsigned> Device::GetDefaultViewSize()
{
    return Size<unsigned>(1024, 768);
}

//------------------------------------------------------------------------

void Device::Clear(UInt32 color)
{
    float   rgba[4];
    GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
    if (scissorEnabled)
        glDisable(GL_SCISSOR_TEST);
    Render::Color c(color);
    c.GetRGBAFloat(rgba);
    glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnabled)
        glEnable(GL_SCISSOR_TEST);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    SF_UNUSED(WindowId);
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::BeginFrame()
{
}

void Device::SetWireframe(bool flag)
{
    glPolygonMode(GL_FRONT_AND_BACK, flag ? GL_LINE : GL_FILL);
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);
    ::SwapBuffers(pImpl->hDC);
}

UInt32 Device::GetCaps() const
{
    return DeviceCap_Wireframe;
}

bool Device::TakeScreenShot(const String& filename)
{
    pImpl->takeScreenShot(filename);
    return true;
}

bool DeviceImpl::takeScreenShot(const String& filename)
{
#ifdef SF_ENABLE_LIBPNG
    ::SwapBuffers(hDC);

    // SHCreateDirectoryEx only accepts backslashes as separators, and must be an absolute path.
    String relpath = filename.GetPath();
    TCHAR workingDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, workingDir);
    String path(workingDir);
    path += '/';
    path += relpath;
    StringBuffer sb;
    for ( UPInt i = 0; i < path.GetLength(); ++i )
    {
        char c = path[i];
        if ( c != '/' )
            sb.AppendChar(c);
        else
            sb.AppendChar('\\');
    }
    path = sb;
    
    SHCreateDirectoryEx(0, path, NULL);

    GLint viewportInfo[4];
    glGetIntegerv(GL_VIEWPORT, viewportInfo);
    UByte* pixelMemory = (UByte*)SF_ALLOC(sizeof(UByte)*viewportInfo[2]*(viewportInfo[3]+1)*3, Stat_Default_Mem);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(viewportInfo[0], viewportInfo[1], viewportInfo[2], viewportInfo[3], GL_RGB, GL_UNSIGNED_BYTE, pixelMemory );
    if ( glGetError() != 0 )
    {
        SF_FREE(pixelMemory);
        return false;
    }

    // Reverse the scanlines, so they're not upsidedown.
    UByte* extraLine = pixelMemory + viewportInfo[2]*viewportInfo[3]*3;
    unsigned linesize = viewportInfo[2]*3;
    for (int sl = 0; sl < viewportInfo[3]/2; sl++ )
    {
        memcpy(extraLine, pixelMemory + sl* linesize, linesize );
        memcpy(pixelMemory + sl* linesize, pixelMemory + (viewportInfo[3]-sl-1) * linesize, linesize );
        memcpy(pixelMemory + (viewportInfo[3]-sl-1) * linesize, extraLine, linesize );
    }
    
    // Now actually save it.
    Render::ImageData data;
    data.Initialize(Render::Image_R8G8B8, (UInt32)viewportInfo[2], (UInt32)viewportInfo[3], (UPInt)viewportInfo[2]*3, (UByte*)pixelMemory);
    String actualFilename = filename;
    if ( actualFilename.GetExtension().CompareNoCase(".png") )
        actualFilename.AppendString(".png");

    bool success = Render::PNG::FileWriter::WriteImage<SysFile>(actualFilename, data);
    SF_FREE(pixelMemory);
    return success;
#else
    SF_UNUSED(filename);
    return false;
#endif
}

}} // Scaleform::Platform
