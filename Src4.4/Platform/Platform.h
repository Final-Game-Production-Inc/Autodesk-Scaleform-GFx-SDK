/**************************************************************************

Filename    :   Platform.h
Content     :   Base appplication class Platform::AppBase, implemented
                differently one every platform.
Created     :   Jan 10, 2008
Authors     :   Maxim Didenko, Dmitry Polenur, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_App_H
#define INC_SF_Platform_App_H

#include "GFxConfig.h"

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_KeyCodes.h"
#include "Kernel/SF_System.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_HAL.h"

#include "Platform/Platform_CommandLine.h"
#include "Platform/Platform_SystemCursorManager.h"
#include "GFx/GFx_Player.h"
//#include "GFx/IME/GFx_IMEManager.h"

#ifdef SF_OS_PS3
#include <cell/spurs/types.h>
#endif

#ifdef SF_OS_3DS
#include <nn/os.h>
#include <nn/fnd.h>
#endif

//------------------------------------------------------------------------
// ***** SF_PLATFORM_APP(AppPackageId,AppClass) macro

// SF_PLATFORM_APP(AppPackageId,AppClass) macro is used instead of main(), providing the
// startup application class for the program. The package Id is used as a unique identifier for the application
// on some platforms. It should match the name of the executable.
// SF_PLATFORM_APP_ARGS macro takes optimal application class constructor
// arguments, in parenthesis as follows: SF_PLATFORM_APP_ARGS(MyApp, MyApp, ("Title", 800, 600)).

#if defined(SF_OS_3DS)
    #define SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, SystemClass, AppClass, args)             \
    static Scaleform::Platform::SystemInitWrapper<SystemClass> SF_SystemClassInstance;                \
    extern void Platform_3DS_InitExtra();                                                      \
    Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance() \
    { SF_SystemClassInstance.Init(); Platform_3DS_InitExtra(); return new AppClass args; } \
    void Scaleform::Platform::AppBase::DestroyInstance(Scaleform::Platform::AppBase* app) \
    { delete app; SF_SystemClassInstance.Destroy(); }
#elif defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
// To fix Unicode command line arguments handling
// wmain needs to be in here and not in Platform_WinAPI.cpp, otherwise we have 
// ambiguous entry point error (because Platform_WinAPI.cpp is in the lib)
    #if  defined(UNICODE)
        extern int Platform_WinAPI_MainW(int argc, wchar_t** argv);
        #define SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, SystemClass, AppClass, args)             \
            static Scaleform::Platform::SystemInitWrapper<SystemClass> SF_SystemClassInstance;                \
            Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance() \
                { SF_SystemClassInstance.Init(); return new AppClass args; }                 \
                void Scaleform::Platform::AppBase::DestroyInstance(Scaleform::Platform::AppBase* app) \
                { delete app; SF_SystemClassInstance.Destroy(); } \
                int wmain(int argc, wchar_t **argv) \
                { return Platform_WinAPI_MainW(argc, argv); }
    #else
        extern int Platform_WinAPI_MainA(int argc, char** argv);
        #define SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, SystemClass, AppClass, args)             \
            static Scaleform::Platform::SystemInitWrapper<SystemClass> SF_SystemClassInstance;                \
            Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance() \
            { SF_SystemClassInstance.Init(); return new AppClass args; }                 \
            void Scaleform::Platform::AppBase::DestroyInstance(Scaleform::Platform::AppBase* app) \
            { delete app; SF_SystemClassInstance.Destroy(); } \
            int main(int argc, char** argv) \
            { return Platform_WinAPI_MainA(argc, argv); }
    #endif
#elif !defined(SF_OS_ANDROID)
    #define SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, SystemClass, AppClass, args)             \
    static Scaleform::Platform::SystemInitWrapper<SystemClass> SF_SystemClassInstance;                \
    Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance() \
    { SF_SystemClassInstance.Init(); return new AppClass args; }                 \
    void Scaleform::Platform::AppBase::DestroyInstance(Scaleform::Platform::AppBase* app) \
    { delete app; SF_SystemClassInstance.Destroy(); }
#endif

// Initialize application to use specified AppClass class as well as
// a custom system class and/or allocator, such as Scaleform::GFx::System.
#define SF_PLATFORM_SYSTEM_APP(AppPackageId, SystemClass, AppClass) \
        SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, SystemClass, AppClass, ())
// Initialize application with specified AppClass and arguments, while using
// the default Scaleform::System class and allocator for Kernel startup.
#define SF_PLATFORM_APP_ARGS(AppPackageId, AppClass, args) \
        SF_PLATFORM_SYSTEM_APP_ARGS(AppPackageId, Scaleform::System, AppClass, args)
#define SF_PLATFORM_APP(AppPackageId, AppClass) SF_PLATFORM_APP_ARGS(AppPackageId, AppClass, ())

//------------------------------------------------------------------------

namespace Scaleform { namespace Platform {

using Render::Size;
using Render::Point;
using Render::PointF;

class AppImpl;
class RenderHALThread;


// TBD: Configure window and Device separately?
//   - AppBase: SetupWindow
//   - RenderThread: InitGraphics(Device(window))


enum ViewFlagConstants
{
    View_FullScreen         = 0x00001,
    View_VSync              = 0x00002,
    View_FSAA               = 0x00004,
    View_ForceTiling        = 0x00008,
    // Window-only.
    View_WindowResizable    = 0x00010,
    View_WindowDropFiles    = 0x00020,
    // Force SW cursor, disabling HW one if available.
    View_WindowSWCursor     = 0x00040,

    // Application must respond to OnOrientation by setting the viewport Orientation flags.
    View_UseOrientation     = 0x00080,

    View_Stereo             = 0x00100,

    // Stereo in the display hardware only; uses the same size buffer but half for each eye.
    // Set View_Stereo also.
    View_Stereo_SplitV      = 0x00200,
    View_Stereo_SplitH      = 0x00400,
    
    // These flags must be set if size and/or pos is provided. If not specified, 
    // default location and size values will be used.
    // Defaults have special meaning when switching from and going into FullScreen mode.    
    View_Pos                = 0x01000,
    View_Size               = 0x02000,

    // If the platform compiles shaders at runtime, this flag causes all possible shaders
    // to be compiled at startup. The default causes shaders to be compiled as they are used.
    View_PrecompileShaders  = 0x04000,

    // Forces the system to use software rendering (if available). 
    //  - D3D10/11 use this flag to select a WARP device.
    View_SoftwareRendering  = 0x08000,

    // Causes the mesh caches to use static index/vertex buffers, instead of dynamic ones 
    // (if applicable for the HAL's mesh cache - currently only D3D9).
    View_StaticBuffers     = 0x10000,

    // Causes the created OpenGL context to use legacy mode rather than GL 3. Only applicable
    // to OpenGL renderers.
    View_GL20              = 0x20000,

    // Causes use of a deferred context, either supported directly by the graphics API, or a software deferred context.
    View_DeferredContext   = 0x40000,
    
    // Forces the shader system in D3D9 to use SM2.0 shaders, even if it is capable of SM3.0.
    View_ShaderModel20     = 0x80000,

    // Enables additional output by the graphics/driver layer. Only applicable to OpenGL 3.x+.
    View_DebugMessages     = 0x100000,
    // Allowed orientations
    View_Portrait          = 0x200000,
    View_ReversePortrait   = 0x400000,
    View_Landscape         = 0x800000,
    View_ReverseLandscape  = 0x1000000
};

enum StereoFormats
{
    Stereo_None,
    Stereo_Standard,            // Separate Left/Right buffers, call PresentFrame twice
    Stereo_CombinedBuffer,      // Left/Right stored in same buffer (frame packing, interlaced, etc.). Call PresentFrame once.
};

enum PresentFlags
{
    Present_StereoMask = 0xf,   // must match Render::StereoDisplay flags
    Present_LastWindow = 0x100  // set for the last window to be presented during the frame
};

enum Caps
{
    Cap_LockOrientation = 0x0001,
};

enum DeviceCaps
{
    DeviceCap_Stereo    = 0x0001,
    DeviceCap_Wireframe = 0x0002,
    DeviceCap_MSAA      = 0x0004,
};

struct ViewConfig
{
    unsigned        ViewFlags;      // Bit-flags for configuration. See ViewFlagConstants.
    int             ColorBits;      // The requested number of bits in the color buffer (-1 will use platform default).
    int             DepthBits;      // The requested number of bits in the depth buffer (-1 will use platform default).
    int             StencilBits;    // The requested number of bits in the stencil buffer (-1 will use platform default).
    Size<unsigned>  ViewSize;
    unsigned        Orientation;
    unsigned        StereoFormat;
    Point<int>      WindowPos;
    Size<unsigned>  MinSize;
    const Args*     CommandLineArgs;    // The args passed to the App on the commandline.

    ViewConfig()
        : ViewFlags(0), ColorBits(-1), DepthBits(-1), StencilBits(-1), ViewSize(0), Orientation(0), StereoFormat(Stereo_None), WindowPos(0), MinSize(0) { }
    ViewConfig(const Size<unsigned>& sz)
        : ViewFlags(View_Size), ColorBits(-1), DepthBits(-1), StencilBits(-1), ViewSize(sz), Orientation(0), StereoFormat(Stereo_None), WindowPos(0), MinSize(0) { }
    ViewConfig(const Point<int> &pos, const Size<unsigned>& sz)
        : ViewFlags(View_Pos|View_Size), ColorBits(-1), DepthBits(-1), StencilBits(-1), ViewSize(sz), Orientation(0), StereoFormat(Stereo_None), WindowPos(pos), MinSize(0) { }
    ViewConfig(unsigned width, unsigned height)
        : ViewFlags(View_Size), ColorBits(-1), DepthBits(-1), StencilBits(-1), ViewSize(width, height), Orientation(0), StereoFormat(Stereo_None), WindowPos(0), MinSize(0) { }
    
    Size<unsigned> GetSize() const { return ViewSize; }
    void           SetSize(const Size<unsigned>& sz)
    { ViewSize = sz; ViewFlags|= View_Size; }
    void           SetSize(unsigned width, unsigned height)
    { ViewSize.SetSize(width, height); ViewFlags|= View_Size; }
    void           ClearSize() { ViewSize.SetSize(0,0); ViewFlags&= ~View_Size; }

    Point<int>     GetPos() const { return WindowPos; }
    void           SetPos(const Point<int>& pos)
    { WindowPos = pos; ViewFlags|= View_Pos; }
    void           SetPos(int x, int y)
    { WindowPos.SetPoint(x, y); ViewFlags|= View_Pos; }
    void           ClearPos() { WindowPos.SetPoint(0,0); ViewFlags&= ~View_Pos; }

    Size<unsigned> GetMinSize() const { return MinSize; }
    void           SetMinSize(unsigned width, unsigned height)
    { MinSize.SetSize(width, height); }

    bool HasFlag(ViewFlagConstants flag) const { return (ViewFlags & flag) != 0; }
    void SetFlag(ViewFlagConstants flag, bool set)
    {
        if (set) ViewFlags |= flag;
        else     ViewFlags &= ~flag;
    }

    // Converts the platform agnostic flags (only) from ViewConfig flags.
    unsigned GetInitHALConfigFlags() const
    {
        unsigned hflags = 0;

        // If View_PrecompileShaders is not set, it means that HALConfig_DynamicShaderInit should be set.
        if (!HasFlag(View_PrecompileShaders))
            hflags |= Render::HALConfig_DynamicShaderInit;

        // If the View_DeferredContext flag is set, request software deferred contexts. If software deferred contexts are not
        // available on the given platform, this flag will be ignored.
        if (HasFlag(View_DeferredContext))
            hflags |= Render::HALConfig_SoftwareDeferredContext;

        return hflags;
    }

    unsigned GetViewportFlags() const
    {
        unsigned vpflags = Orientation;
        if (ViewFlags & View_Stereo_SplitH)
            vpflags |= Render::Viewport::View_Stereo_SplitH;
        if (ViewFlags & View_Stereo_SplitV)
            vpflags |= Render::Viewport::View_Stereo_SplitV;
        return vpflags;
    }
};

//------------------------------------------------------------------------
// ***** Rendering device

// DeviceStatus report rendering status of the Device. Status should be NeedInit or Ready
// for all devices except D3D9, which requires custom handling.
enum DeviceStatus
{
    Device_Ready,           // Rendering initialized.
    Device_NeedInit,        // Rendering not initialized, must call InitGraphics.
    Device_Lost,            // (D3D9/Android) Device lost and not ready for reset.
    Device_NeedReconfigure  // (D3D9/Android) Device is ready for reset; call ReconfigureGraphics.
};

class Device : public NewOverrideBase<Stat_Default_Mem>
{
public:
    class Window : public NewOverrideBase<Stat_Default_Mem>
    {
    public:
        virtual ~Window() {};
        virtual void* GetHandle() = 0;
        virtual void  GetViewConfig(ViewConfig *config) const = 0;
        virtual void  ConfigureWindow(ViewConfig& newConfig) = 0;
    };

    Device(Render::ThreadCommandQueue *commandQueue);
    ~Device();

    Render::HAL* GetHAL() const;
    DeviceStatus GetStatus() const;

    // ***** Graphics Setup/Shutdown APIS

    // If GraphicsConfigMainThread returns true, configuration functions below
    // should run on Main (window creation) thread with RenderThread blocked (D3D9).
    // If not, configuration should run on render thread.
    bool        GraphicsConfigOnMainThread() const;

    // Adjusts ViewConfig based on device capabilities and on remembered
    // window position, if any.
    bool        AdjustViewConfig(ViewConfig* config);
    bool        InitGraphics(const ViewConfig& config, Device::Window* window,
                             ThreadId renderThreadId = 0);
    void        ResizeFrame(void* layer);
    bool        ReconfigureGraphics(const ViewConfig& config);
    void        ShutdownGraphics();

    // Reports the default view size that should be used on this platform.
    // This size will be applied by AdjustViewConfig.
    static Size<unsigned> GetDefaultViewSize();

    // Create a secondary window that can be used with SetWindow;
    // may be missing features compared to the main window.
    // Originally added for 3DS
    Device::Window*      CreateGraphicsWindow(int windowId, const ViewConfig& newConfig);
    
    // ***** Rendering Support APIs
    // Typically run on render thread.    

    void        Clear(UInt32 color);
    void        BeginFrame();
    void        SetWindow(Window* pwindow);
    void        PresentFrame(unsigned displays = Render::StereoCenter);
    bool        TakeScreenShot(const String& filename);

    UInt32      GetCaps() const;

private:
    class DeviceImpl* pImpl;
 };

// Threading configuration constants for OnInit.
// Constants must match values of RTCommandQueue::ThreadingType.
enum ThreadingType
{
    TT_MultiThreaded,
    TT_SingleThreaded,
    // AutoDetect means that causes the use of multi-threading
    // if Thread::GetCPUCount() > 1.
    TT_AutoDetect,
    TT_Unspecified,

    TT_TypeMask         = 0x0000FFF,    // Bitmask, specifying the bits used for the threading type.
    TT_WatchDogFlag     = 0x0001000,    // Enables the watchdog thread.
};

//------------------------------------------------------------------------
// A platform independent interface for a gesture manager.
class GestureManager : public RefCountBaseNTS<GestureManager, Stat_Default_Mem>
{
public:
	class AppBase& TheApp;
    GestureManager(class AppBase& app) : TheApp(app) {}
	virtual ~GestureManager() {}

	virtual void SetMovie(Ptr<Scaleform::GFx::Movie> moviePtr) = 0;

    // 'screenPt' - the point in screen (window) coordinates;
    // 'moviePt'  - the point in Movie coordinate space.
    // Either one can be used.
    virtual void ProcessDown(UInt32 id, const Point<int>& screenPt, const PointF& moviePt) =0;
    virtual void ProcessUp(UInt32 id, const Point<int>& screenPt, const PointF& moviePt)   =0;
    virtual void ProcessMove(UInt32 id, const Point<int>& screenPt, const PointF& moviePt) =0;
private:
    GestureManager& operator=(const GestureManager&) { return *this; }
};

enum ControllerIndexMasks
{
    InputWindow_Mask        = 0xff0000,
    InputWindow_Shift       = 16,
    InputController_Mask    = 0x00ffff
};

//------------------------------------------------------------------------

// ***** Class ExtensionContext
class FREFunctionBase
{
public:
	FREFunctionBase(const char* Name) : StrName(Name)							{ }
	virtual ~FREFunctionBase()													{ }

	virtual bool Call(unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
		{ SF_UNUSED3(argc, argv, result); return false; }

	// Name
	const String& GetName() const											{ return StrName; }
protected:
	String StrName;
};

// ***** Class ExtensionContext
class ExtensionContext
{
public:
	ExtensionContext(const char* Name) 
		: StrName(Name), ActionScriptDataValue(0)							{ }
	virtual ~ExtensionContext()												{ }

	virtual void Dispose()													{ }
	virtual bool GenerateFREFunctions()										{ return false; }
	virtual FREFunctionBase* GetFREFunction(const char* functionName) const	{ SF_UNUSED(functionName); return NULL; }

	// Name
	const String& GetName() const											{ return StrName; }

	// ActionScriptData
	GFx::Value* GetActionScriptData() const									{ return ActionScriptDataValue; }
	void SetActionScriptData(GFx::Value* asData)							{ ActionScriptDataValue = asData; }
protected:
	String StrName;
	GFx::Value* ActionScriptDataValue;
};

// ***** Class Extension
class Extension
{
public:
	Extension(const char* Name, const char* NativeLibraryPath) 
		: StrName(Name), StrNativeLibraryPath(NativeLibraryPath)		{ }
	virtual ~Extension()												{ }

	virtual void Initializer()											{ }	
	virtual void Finalizer()											{ }
	virtual bool CreateContext(const char* contextType)					{ SF_UNUSED(contextType); return false; }
	virtual ExtensionContext* GetContext(const char* contextType) const { SF_UNUSED(contextType); return NULL; }
	virtual bool RemoveContext(const char* contextType)					{ SF_UNUSED(contextType); return false; }

	// Properties
	const String& GetName() const										{ return StrName; }
	const String& GetNativeLibraryPath() const							{ return StrNativeLibraryPath; }
protected:
	String StrName, StrNativeLibraryPath;

};

// ***** Class Extension Manager 
class ExtensionManager
{
public:
    virtual ~ExtensionManager() { }
	virtual bool Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
		{ SF_UNUSED6(extensionID, contextID, functionName, argc, argv, result); return false; }
	
	virtual void DispatchStatusEventAsync(const ExtensionContext& context, const String& eventName, const String& eventParam)
		{ SF_UNUSED3(context, eventName, eventParam); }

	virtual void InitNativeExtension(const String& extensionName, const String& initializer)	{ SF_UNUSED2(extensionName, initializer); }
	virtual const char* GetExtensionDirectory(const char* extensionID)							{ SF_UNUSED(extensionID); return NULL; }
	virtual void InitializeExtensionContext(const char* extensionID, const char* contextID)		{ SF_UNUSED2(extensionID, contextID); }
	virtual void FinalizeExtensionContext(const char* extensionID, const char* contextID)		{ SF_UNUSED2(extensionID, contextID); }
	
	virtual GFx::Value*	GetActionScriptData(const char* extensionID, const char* contextID)		{ SF_UNUSED2(extensionID, contextID); return NULL; }
	virtual void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)	{ SF_UNUSED3(extensionID, contextID, data); }

	virtual Extension* GetExtension(const char* ExtensionID) const								{ SF_UNUSED(ExtensionID); return NULL; }
};

//------------------------------------------------------------------------
// ***** AppImplBase
// AppImpl is a base application class that provides base functionality for
// all derived AppImpl. There should be one AppImpl per platform/renderer API.
class AppImplBase
{
    friend class AppBase;
public:
    AppImplBase(AppBase* papp);
    virtual                 ~AppImplBase();
    virtual void            InitArgDescriptions(Args* args);
    virtual void            SetFrameTime(float seconds);
    virtual float           GetFrameTime() const;
    virtual const Args&     GetArgs() const;
    virtual ThreadingType   GetArgsThreadingType() const;
    virtual void            SetWindowTitle(const String& title);
    virtual const String&   GetWindowTitle() const;
    virtual bool            OnArgs(const Args& args, Args::ParseResult parseResult);
    virtual void            ApplyViewConfigArgs(ViewConfig* config, const Args& args);
    virtual bool            OnInit(ViewConfig& config);
    virtual Point<int>      GetMousePos() const;
    virtual void            killWindow()   { };
    virtual bool            setupWindow(const String& title, const ViewConfig& config) = 0;
    
    virtual bool            IsMultitouchSupported() const { return false; }
    virtual bool            IsDisplayActive() const { return true; }

	// Extension context
	virtual void InitNativeExtensionManager(ArrayLH<Extension*> *freExtensionArray)			{ SF_UNUSED(freExtensionArray); }
	virtual ExtensionManager* GetExtensionManager()		{ return NULL; }
	virtual bool Call(const char* ExtensionID, const char* ContextID, const char* FunctionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result) 
		{ SF_UNUSED6(ExtensionID, ContextID, FunctionName, argc, argv, result); return false; }

	virtual const char* GetExtensionDirectory(const char* extensionID)									{ SF_UNUSED(extensionID); return NULL; }
	virtual void FinalizeExtensionContext(const char* extensionID, const char* contextID)				{ SF_UNUSED2(extensionID, contextID); }
	virtual void InitializeExtensionContext(const char* extensionID, const char* contextID)				{ SF_UNUSED2(extensionID, contextID); }
	virtual GFx::Value*	GetActionScriptData(const char* extensionID, const char* contextID)				{ SF_UNUSED2(extensionID, contextID); return NULL; }
	virtual void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)	{ SF_UNUSED3(extensionID, contextID, data); }
	virtual void SetMovie(Ptr<Scaleform::GFx::Movie> pmovie)											{ SF_UNUSED(pmovie); }

    // Invoked when a virtual keyboard must be opened.
    // 'textBox' contains bounds of a textfield in world coordinates (stage), in pixels.
    virtual void            HandleVirtualKeyboardOpen(bool multiline, const Render::RectF& textBox) 
        { SF_UNUSED2(multiline,textBox); }

    // Invoked when a virtual keyboard must be closed 
    virtual void            HandleVirtualKeyboardClose() { }

    virtual void            ProcessUrl(const String& url) { SF_UNUSED(url); }

	// Accelerometer
	virtual bool			RegisterAccelerometer(int accelerometerId)		{ SF_UNUSED(accelerometerId); return false; }
	virtual bool			UnregisterAccelerometer(int accelerometerId)	{ SF_UNUSED(accelerometerId); return false; }
	virtual bool			IsAccelerometerMuted() const			{ return true; }
	virtual bool			IsAccelerometerSupported() const		{ return false; }
	virtual void			SetAccelerometerInterval(int accelerometerId, int interval)	{ SF_UNUSED2(accelerometerId, interval); }

	// Geolocation
	virtual bool			RegisterGeolocation(int geolocationId)		{ SF_UNUSED(geolocationId); return false; }
	virtual bool			UnregisterGeolocation(int geolocationId)	{ SF_UNUSED(geolocationId); return false; }
	virtual bool			IsGeolocationMuted() const			{ return true; }
	virtual bool			IsGeolocationSupported() const		{ return false; }
	virtual void			SetGeolocationInterval(int geolocationId, int interval)	{ SF_UNUSED2(geolocationId, interval); }

public:
    AppBase*        pApp;
    ViewConfig      Config;
    UInt64          NextUpdateTicks;
    Point<int>      MousePos;

protected:

    virtual void            updateConfig() { };
    const ViewConfig&       getConfig();
    virtual bool            dynamicShaderInitializationDefault() const { return false; }
    

    Args            Arguments;
    String          Title;
    float           FrameTime;
    UInt64          FrameTimeTicks;
};

//------------------------------------------------------------------------
// ***** AppBase

// AppBase is a base application class that sample programs using the Platform
// derive from. AppBase class handles system-specific window initialization,
// input handling and frame processing.
// 
// Application is expected to work in three steps:
//  1. OnInit() - Called on startup, expected to setup window and graphics.
//  2. OnUpdateFrame() - Called in a loop by the system, expecting to
//     both advance the frame and update display. Application should call
//     Shutdown() to begin exit procedure.    
//  3. OnShutdown() - Called when shutting down application.

class AppBase : public NewOverrideBase<Stat_Default_Mem>
{
public:
    AppBase();
    virtual ~AppBase();

    // *** Application Startup override interface
  
    // Called to do initialization, including SetupWindow, render thread
    // creation and graphics init. If false is returned, application will exit.
    // Receives default ViewConfig with command argument applied.
    virtual bool OnInit(ViewConfig& config);

    // Called in a loop to advance and update frame rendering.
    // Frequency of calls is controlled by SetFrameTime. This function may be
    // called more frequently due to updated UI.
    //  - needRepaint specifies when the UI must be repainted due to OS event,
    //    it is always set to 'true' on the first frame. On subsequent frames,
    //    applications can choose to avoid rendering if no animation update
    //    is required.    
    virtual void OnUpdateFrame(bool needRepaint) { SF_UNUSED(needRepaint); }

    // Called to perform shutdown tasks before application exit.
    virtual void OnShutdown();


    // Configures how often OnUpdateFrame will be called by default. 0 means frames should
    // be drawn as often as possible. A value of 0.03333 can be passed to target 30 fps.
    // Note that this function serves as a hint rather than a mandate.
    void        SetFrameTime(float seconds);
    // Returns the actual frame time applied to OnUpdateFrame(), may be different
    // from the value that was set.
    float       GetFrameTime() const;

    // Takes a screenshot, saving it as the provided filename. Returns true if the screenshot was
    // successful; false otherwise. By default, it is not implemented.
    virtual bool TakeScreenShot( const String& filename ) { SF_UNUSED(filename); return false; }

    // *** Command Line arguments
   
    // Called during initialization to allow user to add program-specific command line
    // argument descriptions, by calling Args::AddDesriptions.    
    virtual void InitArgDescriptions(Args* args);
    // Called after parsing the command line; parseResult should be checked
    // to determine action to take.
    virtual bool OnArgs(const Args& args, Args::ParseResult parseResult);

    // Returns command line arguments once they've been parsed;
    // this will typically have the same values as passed to OnArgs.
    virtual const Args& GetArgs() const;

    // Return threading type suggested by arguments.
    virtual ThreadingType GetArgsThreadingType() const;

    // Applies default command line arguments to ViewConfig.
    virtual void ApplyViewConfigArgs(ViewConfig* config, const Args& args);


    // *** Input Events
    virtual void OnKey(unsigned controllerIndex, KeyCode keyCode,
                       unsigned wcharCode, bool downFlag, KeyModifiers mods)
    { SF_UNUSED5(controllerIndex, keyCode, wcharCode, downFlag, mods); }        
    virtual void OnPad(unsigned controllerIndex, PadKeyCode padCode, bool downFlag)
    { SF_UNUSED3(controllerIndex, padCode, downFlag); }

    virtual void OnPadStick(unsigned controllerIndex, PadKeyCode padCode, float xpos, float ypos)
    { SF_UNUSED4(controllerIndex, padCode, xpos, ypos); }
    
    virtual void OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, 
                               const Point<int>& mousePos, KeyModifiers mods)
    { SF_UNUSED5(mouseIndex, button, downFlag, mousePos, mods); }
    virtual void OnMouseMove(unsigned mouseIndex,
                             const Point<int>& mousePos, KeyModifiers mods)
    { SF_UNUSED3(mouseIndex, mousePos, mods); }
    // Mouse wheel delta is provided in units of scrolling, with a positive value
    // indicating an up direction.
    virtual void OnMouseWheel(unsigned mouseIndex, float zdelta,
                              const Point<int>& mousePos, KeyModifiers mods)
    { SF_UNUSED4(mouseIndex, zdelta, mousePos, mods); }

    virtual void OnTouchBegin(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
    { SF_UNUSED5(window, id, pos, contact, primary); }
    virtual void OnTouchEnd(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
    { SF_UNUSED5(window, id, pos, contact, primary); }
    virtual void OnTouchMove(unsigned window, unsigned id, const Point<int>& pos, const Point<int>& contact, bool primary)
    { SF_UNUSED5(window, id, pos, contact, primary); }

    enum GestureMaskBits // bits should be the same as in GFx_Event.h, GestureEvent!
    {
        GestureBit_Unknown      = 0,
        GestureBit_Pan          = 0x01,
        GestureBit_Zoom         = 0x02,
        GestureBit_Rotate       = 0x04,
        GestureBit_Swipe        = 0x08,
        GestureBit_PressAndTap  = 0x10,
        GestureBit_TwoFingerTap = 0x20,

        GestureBit_Last         = GestureBit_TwoFingerTap,
        GestureBit_All          = 0xFF
    };
    virtual void OnGestureBegin(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
                                     const PointF& translationDelta,
                                     const PointF& scaleDelta,
                                     float rotationDelta)
    { SF_UNUSED6(window, gestureMask, pos, translationDelta, scaleDelta, rotationDelta); }

    virtual void OnGesture(unsigned window, UInt32 gestureMask, const Point<int>& pos, 
                                const PointF& translationDelta,
                                const PointF& scaleDelta,
                                float rotationDelta)
    { SF_UNUSED6(window, gestureMask, pos, translationDelta, scaleDelta, rotationDelta); }

    virtual void OnGestureEnd(unsigned window, UInt32 gestureMask, const Point<int>& pos)
    { SF_UNUSED3(window, gestureMask, pos); }

    enum GesturePhase
    {
        Gesture_Begin = 0,
        Gesture_End   = 1,
        Gesture_Upd   = 2,
        Gesture_Simple= 3
    };
    virtual void OnGestureEvent(unsigned window, unsigned eventPhase, UInt32 gestureMask, const Point<int>& pos, 
                                const PointF& translationDelta,
                                const PointF& scaleDelta,
                                float rotationDelta)
    {
        switch(eventPhase)
        {
        case Gesture_Begin:
            OnGestureBegin(window, gestureMask, pos, translationDelta, scaleDelta, rotationDelta);
            return;
        case Gesture_End:
            OnGestureEnd(window, gestureMask, pos);
            return;
        case Gesture_Upd:
        case Gesture_Simple:
            OnGesture(window, gestureMask, pos, translationDelta, scaleDelta, rotationDelta);
            return;
        default:;
        }
    }

    // Base class for platform specific IME Events passed to OnIMEEvent.
    struct IMEEvent { };
    virtual bool    OnIMEEvent(unsigned message, UPInt wParam, UPInt lParam, UPInt hWND, bool preprocess) 
	{	SF_UNUSED5(message, wParam, lParam, hWND, preprocess);
		return false; 
	}
    // OnSize is called on window resize and should re-initialize
    // renderer for the new view. May be called outside of OnSizeEnter
    // bracket for maximize/restore events.
    virtual void    OnSize(const Size<unsigned>& newSize)
    { SF_UNUSED(newSize); }
    // Called when sizing begins and ends.    
    virtual void    OnSizeEnter(bool enterFlag)
    { SF_UNUSED(enterFlag); }

    // Called to notify application about file(s) dropped into the window.
    virtual void    OnDropFiles(const String& path)
    { SF_UNUSED(path); }

    virtual bool    OnOrientation(unsigned orientation, bool force = false)
    { SF_UNUSED2(orientation, force); return false; }

	virtual void	OnAccelerometerUpdate(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ)
	{ SF_UNUSED5(idAcc, timestamp, accelerationX, accelerationY, accelerationZ); }

	virtual void OnGeolocationUpdate(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp)
	{ SF_UNUSED9(idGeo, latitude, longitude, altitude, hAccuracy, vAccuracy, speed, heading, timestamp); }

	virtual void OnStatus(String* code, String* level, String* extensionId, String* contextId)
	{ SF_UNUSED4(code, level, extensionId, contextId); }


    // Used for stereo
    virtual void    OnConfigurationChange(const ViewConfig& newConfig)
    { SF_UNUSED(newConfig); }
    
    // Called to notify application about keyboard content change.
    virtual void    OnUpdateCliboard(const wchar_t* text) { SF_UNUSED(text); }

    // if set == true then this is SET FOCUS, otherwise - KILL FOCUS
    virtual void    OnFocus(bool setFocus, KeyModifiers mods)
    { SF_UNUSED2(setFocus, mods); }

    // Called when "UI" closebox is pressed. Default behaviour triggers
    // program shutdown.
    virtual void    OnClose()
    { Shutdown(); }

    virtual void    OnPause() {}
    virtual void    OnResume() {}
    // Returns current orientation
    virtual unsigned GetOrientation() { return 0; }
    virtual bool	IsOrientationLocked() { return false; }

    // Resets current input focus to null
    virtual void    ResetInputFocus(unsigned controllerIdx) { SF_UNUSED(controllerIdx); }

    // A notification that comes from a system to the player about virtual keyboard appearance.
    // 'keyboardRect' must be specified in screen coordinates;
    // 'textBox' must be specified in stage coordinates, in pixels.
    virtual void    NotifyShowVirtualKeyboard(const Render::RectF& keyboardRect, const Render::RectF& textBox) 
    { SF_UNUSED2(keyboardRect, textBox); }

    // A notification that comes from a system to the player about virtual keyboard disappearance.
    virtual void    NotifyHideVirtualKeyboard() {}


    // *** Window Setup / Message APIs
    static const char* GetPlatformName();

    // Creates a window & initializes the application class.
    // Returns 0 if window creation/initialization failed (the app should quit).
    bool            SetupWindow(const String& title,
                                const ViewConfig& viewConfig = ViewConfig());

    // This call cleans up the application and kills the window.
    void            Shutdown();
    // Returns true once Shutdown was called of program was closed.
    bool            IsShuttingDown() const { return ShuttingDown; }
    

    // Cursor must be enabled to show up; this is an additional state that
    // overrides CursorType. Default state of EnableCursor is platform-specific;
    // on some consoles it may be disabled by default and enabled with a trigger.
    void            SetCursor(CursorType type)      { Cursor.SetCursorType(type); }
    CursorType      GetCursor() const               { return Cursor.GetCursorType(); }
    void            EnableCursor(bool enable)       { Cursor.EnableCursor(enable); }
    bool            IsCursorEnabled() const         { return Cursor.IsCursorEnabled(); }

    void            BringWindowToFront();
    // Changes/sets window title
    void            SetWindowTitle(const String& title);
    const String&   GetWindowTitle() const;

    void            SetOrientationMode(bool allowRotate);

    void            MinimizeWindow();
    void            MaximizeWindow();
    void            RestoreWindow();

    // Reports current view configuration
    void            GetViewConfig(ViewConfig * config) const;
    Size<unsigned>  GetViewSize() const;
    bool            ViewConfigHasFlag(ViewFlagConstants flag);

    Point<int>      GetMousePos() const;

    Device::Window* GetDeviceWindow() const;

    static bool     IsConsole();

    // Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
    // This is a floating point value such as 0.1 for 10% safe frame border.
    static Size<float>     GetSafeArea();

    // Returns default file path prefix for platform, including terminating
    // character if not empty.
    static const char*     GetDefaultFilePath();

    // Returns directory for additional content
    static String          GetContentDirectory();

    // Returns directory for save data
#if defined(SF_OS_ANDROID) || defined(SF_OS_IPHONE)
    String          GetSaveDataDirectory();
#else
    String          GetSaveDataDirectory() { return GetContentDirectory(); }
#endif

    virtual RenderHALThread* GetAppRenderThread() const = 0;

    // Number of mice or similar controllers available
    virtual int     GetMouseCount();
    virtual int     GetKeyboardCount();
    virtual int     GetDisplayCount();
    virtual UInt32  GetCaps() const;

    virtual bool    IsPadConnected(unsigned pad);
    //virtual void    DetectPad() {}

    // AppBase class creation function used by SF_PLATFORM_APP.
    static AppBase* CreateInstance();
    static void     DestroyInstance(AppBase* app);
    // AppBase Main function body used on some platforms.
    int             AppMain(int argc, char* argv[]);

    AppImplBase*    GetAppImpl() { return pImplBase; }
    Device*         GetDevice()  { return pDevice; }

    SystemCursorManager Cursor;

    Ptr<GestureManager> pGestureManager;
    
#ifdef SF_OS_PS3
    CellSpurs* GetSpurs();
#endif

protected:
    Device* pDevice;

private:

    AppImplBase*         pImplBase;
    AppImpl*             pImpl;

    bool                 ShuttingDown;
};

//------------------------------------------------------------------------
// App class takes RenderThread as a template, allowing custom thread
// classes to be used.


template<class RT>
class App : public AppBase
{
public:
    App() : AppBase(), OldSize(Device::GetDefaultViewSize()), OldPos(0) { }
    ~App() 
    {
        if (pDevice)
        {
            delete pDevice;
            pDevice = 0;
        }
        AppBase::OnShutdown();
    }

    // OnInitHelper is provided to simplify OnInit initialization, while
    // allowing RenderThread class to be customized. Should be called
    // first thing from OnInit with modified ViewConfig and window name.    
    bool OnInitHelper(Platform::ViewConfig& config, const char* name,
                      ThreadingType ttype = TT_Unspecified)
    {
        if (ttype == TT_Unspecified)
            ttype = GetArgsThreadingType();
        pRenderThread = *new RT((typename RT::ThreadingType)ttype);
        pDevice = new Device(pRenderThread.GetPtr());
        pRenderThread->SetDevice(pDevice);
        
        // Adjust window size based on device caps.
        if (!pDevice->AdjustViewConfig(&config) || !adjustViewConfig(&config))
            return false;
        pRenderThread->StartThread();

        if (!SetupWindow(name, config))
            return false;

        GetViewConfig(&config);
        OnRenderThreadCreated();

        if (!pRenderThread->InitGraphics(config, GetDeviceWindow(),
                                         pRenderThread->GetThreadId()))
        {
            pRenderThread->ExitThread();
            return false;
        }

        return true;
    }

    // This methods is called after RenderThread creation, but before InitGraphics.
    // It enables configuration of glyph/mesh caches.
    virtual void OnRenderThreadCreated()
    {
    }


    virtual void OnShutdown()
    {
        if (pRenderThread)
        {
            pRenderThread->DestroyGraphics();
            pRenderThread->ExitThread();
        }
    }


    
    bool ReconfigureGraphics(ViewConfig& config)
    {
        ViewConfig newConfig(config);
        ViewConfig oldConfig;
        GetViewConfig(&oldConfig);
        Device::Window* window = GetDeviceWindow();

        if (!pRenderThread->AdjustViewConfig(&newConfig) ||
            !adjustViewConfig(&newConfig, oldConfig.HasFlag(View_FullScreen)))
            return false;

        window->ConfigureWindow(newConfig);
        if (!pRenderThread->ReconfigureGraphics(newConfig))
        {
            window->ConfigureWindow(oldConfig);
            return false;
        }
        
        saveOldConfigOnFullScreen(oldConfig, newConfig.HasFlag(View_FullScreen));
        return true;
    }


    bool adjustViewConfig(ViewConfig* config, bool restoreOld = false)
    {
        if (!config->HasFlag(View_FullScreen))
        {
            // Set old and/or default size if not applied.
            if (!config->HasFlag(View_Size))
                config->SetSize(OldSize);
            if (restoreOld && !config->HasFlag(View_Pos) && !OldPos.IsNull())
                config->SetPos(OldPos);
        }
        return true;
    }

    void  saveOldConfigOnFullScreen(const ViewConfig& oldConfig, bool fullScreen)
    {        
        if (fullScreen && !oldConfig.HasFlag(View_FullScreen))
        {
            OldSize = oldConfig.GetSize();
            OldPos = oldConfig.GetPos();
        }
    }


    RT* GetRenderThread() const { return pRenderThread.GetPtr(); }
    virtual RenderHALThread* GetAppRenderThread() const { return pRenderThread.GetPtr(); }

    bool IsSingleThreaded() const
    {
        return pRenderThread->GetThreadingType() == RT::SingleThreaded;
    }

protected:
    Ptr<RT>  pRenderThread;
    // Saved old window size and position, to enabled mode switch back.
    Size<unsigned>          OldSize;
    Point<int>              OldPos;
};

//------------------------------------------------------------------------
// DirectoryImplBase - base class for Directory implementation class.
class DirectoryImplBase : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DirectoryImplBase(const String& path, const String& pattern);
    virtual ~DirectoryImplBase();

    virtual bool            GetNextFile(String* filename, bool previousFile = 0);
    virtual void            ReadDirectory(const String& path, const String& pattern) = 0;
    virtual void            SetCurrentFile(const String& filename);
    virtual int             GetFileIndex(const String& filename) const;
    virtual const char *    GetFileAtIndex(unsigned index) const;

protected:

    void                    checkDirectoryChange(const String& filename);

    String          Path;       // Current directory.
    String          Pattern;    // Extensions to match, semicolon delimited (eg. ".swf;.gfx").
    Array<String>   Filenames;  // List of filenames matching our pattern in the current directory.
    int             CurFile;    // Index within the Filenames list of the currently loaded file.

};
//------------------------------------------------------------------------
// Directory is a system-independent interface for iterating over a
// directory of files.
class Directory
{
 public:
    Directory(const String& path = ".", const String& pattern = "*");
    ~Directory();

    bool            GetNextFile(String* filename, bool order = 0);
    void            Reread(const String& path, const String& pattern) { ReadDirectory(path, pattern); }
    void            ReadDirectory(const String& path, const String& pattern);
    void            SetCurrentFile(const String& filename);
    int             GetFileIndex(const String& filename) const;
    const char *    GetFileAtIndex(unsigned index) const;

 private:
    class DirectoryImplBase* pImpl;
};

//------------------------------------------------------------------------
// SystemInitWrapper is an internal helper used by SF_PLATFORM_SYSTEM_APP_ARGS
// macro.
template<class S>
class SystemInitWrapper
{
    UPInt Buffer[sizeof(S)/sizeof(UPInt)+1];
public:
	virtual ~SystemInitWrapper() {}

    virtual void Init()     { Construct<S>(Buffer); }
    virtual void Destroy()  { ((S*)Buffer)->~S(); }
};


}} // Scaleform::Platform

#ifdef SF_OS_ANDROID
// Android requires custom SF_PLATFORM_APP() macro that defines
// native Java package functions.
#include "Platform_Android.h"
#endif

#endif //INC_SF_Platform_App_H
