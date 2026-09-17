/**********************************************************************

PublicHeader:   Render
Filename    :   Platform_Orbis.cpp
Content     :   Platform Support for Orbis.
Created     :   2012/10/06
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Platform_PS4_H
#define INC_SF_Platform_PS4_H

#include "Platform.h"
#include "Platform/Platform_SystemCursorManager.h"

#include "Kernel/SF_KeyCodes.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_Debug.h"

#include "Render/PS4/PS4_HAL.h"
#include "Render/PS4/PS4_MemoryManager.h"

#include <unistd.h>
#include <stdlib.h>

#include <scebase_common.h>
#include <pad.h>
#include <video_out.h>
#include <sdk_version.h>

// If we are compiling with LCUE support, include the LCUE implementation source. Otherwise,
// it won't be linked in anywhere. In SDK 1.600, the LCUE was included in the general release,
// so this file is not required anymore.
#if defined(SF_PS4_USE_LCUE) && SCE_ORBIS_SDK_VERSION < 0x01600051u
#include "PS4/lcue.cpp"
#endif

// No define for in pad.h
static const int MAX_PADS_SUPPORTED = 4;
static const int GAMEPAD_DEADZONE   = 16;
    
// Specify the C-library heap size (256MB).
size_t	sceLibcHeapSize	= 256 << 20;

namespace Scaleform { namespace Platform {

using namespace Render:: PS4;
using namespace Render;

//------------------------------------------------------------------------
// ***** AppImpl

class AppImpl : public Device::Window, public AppImplBase
{
    friend class AppBase;

    AppImpl(AppBase* app);
	~AppImpl();

    // Window Interface
    virtual void* GetHandle() { return 0; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);    
    
private:
    bool          setupWindow(const String& title, const ViewConfig& config);
    bool          processMessages();
    void          setMousePosition(const Point<unsigned> & p );

    struct Pad
    {
        int         PadHandle;
        ScePadData  LastPadData;
        int         MouseX, MouseY;
        float       MouseXadj, MouseYadj;

        bool    LStickLeft, LStickRight, LStickUp, LStickDown;
        bool    nLStickLeft, nLStickRight, nLStickUp, nLStickDown;

        UInt64  LastMsgTime, LastRepeatTime, RepeatDelay;

        Pad() : PadHandle(-1),
            MouseX(0), MouseY(0),
            MouseXadj(0), MouseYadj(0)
        {
            memset(&LastPadData, 0, sizeof(LastPadData));

            // Avoid 'ghost' first frame movement.
            LastPadData.leftStick.x  = 0x80;
            LastPadData.leftStick.y  = 0x80;
            LastPadData.rightStick.x = 0x80;
            LastPadData.rightStick.y = 0x80;

            LStickLeft = LStickRight = LStickUp = LStickDown = false;
            nLStickLeft = nLStickRight = nLStickUp = nLStickDown = false;
            LastRepeatTime = RepeatDelay = 0;
            LastMsgTime = Timer::GetTicks();
        }

        void OpenPort(int index)
        {
            static SceUserServiceUserId UserId = 0;
            if (!UserId)
            {
                sceUserServiceInitialize(NULL);
                sceUserServiceGetInitialUser(&UserId);
                scePadInit();
            }

            SceUserServiceLoginUserIdList userList;
            sceUserServiceGetLoginUserIdList(&userList);
            if (userList.userId[index] == SCE_USER_SERVICE_USER_ID_INVALID)
                return;

            PadHandle = scePadOpen(userList.userId[index], SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
        }
    } Pads[MAX_PADS_SUPPORTED];    
};


//------------------------------------------------------------------------

static PadKeyCode keymap[16] = 
{
    Pad_Select, Pad_LT, Pad_RT, Pad_Start,
    Pad_Up, Pad_Right, Pad_Down, Pad_Left,
    Pad_L2, Pad_R2, Pad_L1, Pad_R1, Pad_T,
    Pad_O, Pad_X, Pad_S
};


//------------------------------------------------------------------------

AppImpl::AppImpl(AppBase* app) :         
    AppImplBase(app)
{
}

AppImpl::~AppImpl() 
{
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;

    Point<unsigned> center(config.ViewSize.Width/2, config.ViewSize.Height/2);
    setMousePosition(center);
    return true;
}

void AppImpl::ConfigureWindow(ViewConfig& newConfig)
{
    // Configures window for a new video mode.
    Config = newConfig;
}

bool AppImpl::processMessages()
{
    for (unsigned k = 0; k < MAX_PADS_SUPPORTED; ++k)
    {
        ScePadData newPadData;
        int mbutton = -1;
        UInt64 msgtime = Timer::GetTicks();
        float delta = (msgtime - Pads[k].LastMsgTime) / 1000000.0f;
        bool didrepeat = 0;

        // Returns non-zero if an error code occurs. In which case, just skip this pad.
        if (scePadReadState(Pads[k].PadHandle, &newPadData) != SCE_OK)
            continue;

        // Determine if any digital buttons were pressed/released.
        unsigned button, index;

        for (button = 1, index = 0; button <= SCE_PAD_BUTTON_SQUARE; button <<= 1, ++index)
        {
            if ((Pads[k].LastPadData.buttons & button) != (newPadData.buttons & button))
            {
                if (keymap[index] == Pad_X && pApp->IsCursorEnabled())
                {
                    // 'X' button counts as mouse click when the cursor is enabled.
                    mbutton = (newPadData.buttons & button) ? 1 : 0;
                }
                else
                {
                    // Press/Release the button.
                    pApp->OnPad(k, keymap[index], (newPadData.buttons & button) ? 1 : 0);
                    Pads[k].RepeatDelay = 350000;
                    didrepeat = 1;
                }
            }
            else if ((Pads[k].LastPadData.buttons & button) && (newPadData.buttons & button)
                && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay
                && (!pApp->IsCursorEnabled() || keymap[index] != Pad_X))
            {
                // Detect repeat, and push the button again.
                Pads[k].RepeatDelay = 100000;
                pApp->OnPad(k, keymap[index], 1);
                didrepeat = 1;
            }
        }

        bool curR = ((newPadData.leftStick.x & 0xF8) >= 192);
        bool oldR = ((Pads[k].LastPadData.leftStick.x & 0xF8) >= 192);
        bool curL = ((newPadData.leftStick.x & 0xF8) <= 64);
        bool oldL = ((Pads[k].LastPadData.leftStick.x & 0xF8) <= 64);
        bool curD = ((newPadData.leftStick.y & 0xF8) >= 192);
        bool oldD = ((Pads[k].LastPadData.leftStick.y & 0xF8) >= 192);
        bool curU = ((newPadData.leftStick.y & 0xF8) <= 64);
        bool oldU = ((Pads[k].LastPadData.leftStick.y & 0xF8) <= 64);

        Pads[k].nLStickLeft = Pads[k].nLStickRight = Pads[k].nLStickUp = Pads[k].nLStickDown = false;
        if (curR)
        {
            if (oldR && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Right, 0);
                pApp->OnPad(k, Pad_Right, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldL)
                pApp->OnPad(k, Pad_Left, 0);
            if (oldL || !oldR)
            {
                pApp->OnPad(k, Pad_Right, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickRight = true;
        }
        if (curL)
        {
            if (oldL && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Left, 0);
                pApp->OnPad(k, Pad_Left, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldR)
                pApp->OnPad(k, Pad_Right, 0);
            if (oldR || !oldL)
            {
                pApp->OnPad(k, Pad_Left, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickLeft = true;
        }
        if (curU)
        {
            if (oldU && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Up, 0);
                pApp->OnPad(k, Pad_Up, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldD)
                pApp->OnPad(k, Pad_Down, 0);
            if (oldD || !oldU)
            {
                pApp->OnPad(k, Pad_Up, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickUp = true;
        }
        if (curD)
        {
            if (oldD && msgtime >= Pads[k].LastRepeatTime + Pads[k].RepeatDelay)
            {
                //pApp->OnPad(k, Pad_Down, 0);
                pApp->OnPad(k, Pad_Down, 1);
                Pads[k].RepeatDelay = 100000;
                didrepeat = 1;
            }
            else if (oldU)
                pApp->OnPad(k, Pad_Up, 0);
            if (oldU || !oldD)
            {
                pApp->OnPad(k, Pad_Down, 1);
                Pads[k].RepeatDelay = 350000;
                didrepeat = 1;
            }
            Pads[k].nLStickDown = true;
        }

        if (Pads[k].LStickDown && !Pads[k].nLStickDown)
            pApp->OnPad(k, Pad_Down, 0);

        if (Pads[k].LStickUp && !Pads[k].nLStickUp)
            pApp->OnPad(k, Pad_Up, 0);

        if (Pads[k].LStickLeft && !Pads[k].nLStickLeft)
            pApp->OnPad(k, Pad_Left, 0);

        if (Pads[k].LStickRight && !Pads[k].nLStickRight)
            pApp->OnPad(k, Pad_Right, 0);

        Pads[k].LStickDown = Pads[k].nLStickDown;
        Pads[k].LStickLeft = Pads[k].nLStickLeft;
        Pads[k].LStickRight = Pads[k].nLStickRight;
        Pads[k].LStickUp = Pads[k].nLStickUp;

        if ((newPadData.rightStick.x & 0xF8) > 128 + GAMEPAD_DEADZONE)
            Pads[k].MouseXadj += (float(newPadData.rightStick.x & 0xF8) - 128.0f - GAMEPAD_DEADZONE) * 4.0f * delta;
        else if ((newPadData.rightStick.x & 0xF8) < 128 - GAMEPAD_DEADZONE)
            Pads[k].MouseXadj += (float(newPadData.rightStick.x & 0xF8) - 128.0f + GAMEPAD_DEADZONE) * 4.0f * delta;

        if ((newPadData.rightStick.y & 0xF8) > 128 + GAMEPAD_DEADZONE)
            Pads[k].MouseYadj += (float(newPadData.rightStick.y & 0xF8) - 128.0f - GAMEPAD_DEADZONE) * 4.0f * delta;
        else if ((newPadData.rightStick.y & 0xF8) < 128 - GAMEPAD_DEADZONE)
            Pads[k].MouseYadj += (float(newPadData.rightStick.y & 0xF8) - 128.0f + GAMEPAD_DEADZONE) * 4.0f * delta;

        Pads[k].LastPadData = newPadData;

        if (fabs(Pads[k].MouseXadj) >= 1 || fabs(Pads[k].MouseYadj) >= 1) 
        {
            Pads[k].MouseX += int(Pads[k].MouseXadj);
            Pads[k].MouseY += int(Pads[k].MouseYadj);
            Pads[k].MouseXadj -= int(Pads[k].MouseXadj);
            Pads[k].MouseYadj -= int(Pads[k].MouseYadj);
            if (Pads[k].MouseX < 0)
                Pads[k].MouseX = 0;
            if (Pads[k].MouseX >= int(Config.ViewSize.Width))
                Pads[k].MouseX = Config.ViewSize.Width;
            if (Pads[k].MouseY < 0)
                Pads[k].MouseY = 0;
            if (Pads[k].MouseY >= int(Config.ViewSize.Height))
                Pads[k].MouseY = Config.ViewSize.Height;

            pApp->OnMouseMove(k, Point<int>(Pads[k].MouseX, Pads[k].MouseY), 0);

            // Change the mouse position in the impl.
            MousePos = Point<int>(Pads[k].MouseX, Pads[k].MouseY);
        }
        if (mbutton != -1)
            pApp->OnMouseButton(k, 0, mbutton, Point<int>(Pads[k].MouseX, Pads[k].MouseY), 0);

        Pads[k].LastMsgTime = msgtime;
        if (didrepeat)
            Pads[k].LastRepeatTime = msgtime;

    }
    return 1;
}

void AppImpl::setMousePosition(const Point<unsigned> & p ) 
{
    for ( int i = 0; i < MAX_PADS_SUPPORTED; i++ )
    {
        Pads[i].MouseX = p.x;
        Pads[i].MouseY = p.y;
    }
}

//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase() :
    Cursor(false),
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

void AppBase::Shutdown()
{
    ShuttingDown = true;
}

void AppBase::BringWindowToFront()
{
}

Device::Window* AppBase::GetDeviceWindow() const
{
    return pImpl;
}

bool AppBase::IsConsole()
{
    return true;
}

Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.05f);
}

const char* AppBase::GetDefaultFilePath()
{
    return "/hostapp/";
}

String AppBase::GetContentDirectory()
{
    return GetDefaultFilePath();
}

int AppBase::GetMouseCount()
{
    // No way to query how many controllers are connected? Assume only 1.
    return 1;
}

int AppBase::GetKeyboardCount()
{
    return 1;
}

UInt32 AppBase::GetCaps() const
{
    return 0;
}

bool AppBase::IsPadConnected(unsigned pad)
{
    // No way to query how many controllers are connected? Return the first is always connected.
	return pad == 0;
}

// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{    
    // Scaleform::System initSFSystem;

    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return 0;

    // Initialize application and its window.
    ViewConfig config(Device::GetDefaultViewSize());
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return 0;
    }

    // Application / Player message loop.
    pImpl->NextUpdateTicks = Timer::GetTicks() + pImpl->FrameTimeTicks;

    // Open pads
    for (unsigned i = 0; i < MAX_PADS_SUPPORTED; ++i)
        pImpl->Pads[i].OpenPort(i);

    while(!IsShuttingDown())
    {
        // Wait for Update time if necessary; consider timing wrap-around.
        pImpl->processMessages();

        if (pImpl->FrameTime != 0.0f)
        {
            UInt64  ticks = Timer::GetTicks();
            if (pImpl->NextUpdateTicks > ticks)
            {
                useconds_t waitUs = pImpl->NextUpdateTicks - ticks;
                usleep(waitUs);
                continue;
            }
            pImpl->NextUpdateTicks = ticks + pImpl->FrameTimeTicks;
        }
        OnUpdateFrame(true);
    }

    OnShutdown();

    return 0;
}

//------------------------------------------------------------------------
// ***** Directory

class DirectoryImpl : public DirectoryImplBase
{
public:
    DirectoryImpl(const String& path, const String& pattern)
        : DirectoryImplBase(path, pattern) { }

    virtual void ReadDirectory(const String& path, const String& pattern);
};

void DirectoryImpl::ReadDirectory(const String& path, const String& pattern)
{
    Path = path;
    Pattern = pattern;

    SceKernelMode mode = (SceKernelMode)SCE_KERNEL_O_DIRECTORY;
	int fd = sceKernelOpen(path.ToCStr(), SCE_KERNEL_O_RDONLY, mode);
	if (fd > 0)
	{
        SceKernelStat sb;
        sceKernelFstat(fd, &sb);
        static const int DIRECTORY_BUFFER_SIZE = Alg::Align<512>(sizeof(SceKernelDirent));
        char directoryBuffer[DIRECTORY_BUFFER_SIZE];

        int readBytes = sceKernelGetdents(fd, directoryBuffer, DIRECTORY_BUFFER_SIZE);
		while (readBytes > 0)
		{
            SceKernelDirent* dir = reinterpret_cast<SceKernelDirent*>(directoryBuffer);
            int bytesLeft = readBytes;
            while (bytesLeft > 0)
            {
			    String name = dir->d_name;
			    if (name.GetExtension() == pattern.GetExtension())
				    Filenames.PushBack(name);
                if (dir->d_reclen == 0)
                    break;
                dir = reinterpret_cast<SceKernelDirent*>((char*)dir + dir->d_reclen);
                bytesLeft -= dir->d_reclen;
            }
            readBytes = sceKernelGetdents(fd, directoryBuffer, DIRECTORY_BUFFER_SIZE);
		}
        sceKernelClose(fd);
	}

	CurFile = 0;
}

Directory::Directory(const String& path, const String& pattern)
{
    pImpl = new DirectoryImpl(path, pattern);
}

Directory::~Directory()
{
    delete pImpl;
}


//------------------------------------------------------------------------
// ***** SystemCursorManager

SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) :
    SystemCursorState(systemCursorAvailable, false),
    pImpl(0)
{
}
SystemCursorManager::~SystemCursorManager()
{
}

void SystemCursorManager::update(bool forceSet)
{
    // System cursors update. Since there are none, do nothing.
}

//------------------------------------------------------------------------

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    sce::Gnm::DataFormat        determineFormat(const ViewConfig& config) const;

	bool                        adjustViewConfig(ViewConfig* config);
    bool                        initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId);
    void                        shutdownGraphics();
    void                        beginFrame();
    void                        clear(UInt32 color);
    void                        presentFrame();

    Device::Window*             pView;                          // The 'window' to the device. Placebo on Orbis.
    Ptr< PS4::HAL>             pHal;                           // The backend renderer
    DeviceStatus                Status;                         // Status of the device.
    static const unsigned       FRAME_BUFFERS = 3;              // The number of framebuffers.
    uint32_t                    VideoOutHandle;                     // The handle to the opened video output.
    SceKernelEqueue             FlipEventQueue;                     // Event queue, which handles the flipping synchronization.
    sce::Gnm::RenderTarget      DispSurface[FRAME_BUFFERS];     // The framebuffers (double buffered).
    sce::Gnm::SizeAlign         DispSurfaceSize[FRAME_BUFFERS]; // Allocated sizes of the framebuffers.
    UInt64*                     DispSurfaceLabels;                  // Labels written by the GPU, to determine rendering has completed for the given buffer.
    UInt64                      ExpectedFrameLabels[FRAME_BUFFERS]; // The expected frame number to exist in DispSurfaceLables entry, to indicate when the frame is finished rendering.
    UInt64                      CurrentFrame;                       // The index of the current frame.
    sce::Gnm::DepthRenderTarget DepthTarget;                    // The depth/stencil target.
    sce::Gnm::SizeAlign         DepthSize, StencilSize;         // Sizes of the depth/stencil targets.
    sceGnmxContextType          GnmxCtx;                        // Either a sce::Gnmx::GfxContext OR sce::Gnmx::LCUE::GraphicsContext (used for rendering).
    unsigned                    BackBuffer, FrontBuffer;        // Indices for which entries in DispSurface are the current front/back buffers.
    void*                       CueCpRamShadowBuffer;           // The shadow buffer used by the Gnmx::ConstantUpdateEngine (see Orbis docs).
    PS4::MemoryManager*       pMemManager;                    // The memory manager, used to manage video memory.
    Ptr<Render::RenderTarget>   pDisplayRT;                     // GFx's structure defining the current render target.
};

DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue) : 
    pView(0), 
    pHal(0), 
    Status(Device_NeedInit),
    VideoOutHandle(0),
    DispSurfaceLabels(0),
    CurrentFrame(0),
    BackBuffer(0),
    FrontBuffer(FRAME_BUFFERS-1),
    CueCpRamShadowBuffer(0),
    pMemManager(0),
    pDisplayRT(0)
{
    pHal = *SF_NEW PS4::ProfilerHAL(commandQueue);
    memset(DispSurface, 0, sizeof(DispSurface));
    memset(DispSurfaceSize, 0, sizeof(DispSurfaceSize));
    memset(ExpectedFrameLabels, 0, sizeof(ExpectedFrameLabels));

    // Open the video output handle immediately.
    VideoOutHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, 0);
    SF_DEBUG_ASSERT(VideoOutHandle >= 0, "Unable to open video output handle.");

}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();

    if ( pMemManager )
    {
        // Free the framebuffers.
        for ( unsigned i = 0; i < FRAME_BUFFERS; ++i )
            pMemManager->Free(DispSurface[i].getBaseAddress(), DispSurfaceSize[i].m_size);

        // And the depth/stencil targets.
        pMemManager->Free(DepthTarget.getZReadAddress(), DepthSize.m_size);
        pMemManager->Free(DepthTarget.getStencilReadAddress(), StencilSize.m_size);

        // And the labels.
        pMemManager->Free(DispSurfaceLabels, sizeof(UInt64) * FRAME_BUFFERS);

        // Kill the shadow buffer.
        free(CueCpRamShadowBuffer);

        // And the memory manager.
        delete pMemManager;
        pMemManager = 0;
    }

	pView = 0;

    // Close the video output handle.
    sceVideoOutClose(VideoOutHandle);
}

sce::Gnm::DataFormat DeviceImpl::determineFormat(const ViewConfig& config) const
{    
    // Currently, display render targets must be BGRA 8888 UNORM, LinearAligned, so ignore whatever the ViewConfig says.
    SF_DEBUG_WARNING1(config.ColorBits != 32 && config.ColorBits != -1, 
        "Cannot specify a bit-depth for the display surface other than 32 (passed %d)", config.ColorBits);
    return sce::Gnm::kDataFormatB8G8R8A8Unorm;
}

bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    // If you pass zero, you get the current resolution of the output.
    if (config->ViewSize.Width == 0 || config->ViewSize.Height == 0)
    {
        SceVideoOutResolutionStatus resolutionInfo;
        sceVideoOutGetResolutionStatus(VideoOutHandle, &resolutionInfo);
        config->ViewSize = Size<unsigned>(resolutionInfo.fullWidth, resolutionInfo.fullHeight);
    }

    config->ViewFlags |= View_FullScreen;
	return true;
}

bool ContextOutOfSpaceCallback(sce::Gnm::CommandBuffer *, uint32_t, void* userData)
{
    // Attempt to submit the current context.
    PS4::HAL* hal = reinterpret_cast<PS4::HAL*>(userData);
    hal->SubmitAndResetStates();
    return true;
}

bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
	return true;
#if 0
    if (!pHal || !view)
        return false;

	pView = view;

    // TODOBM: Stereo support has been added in 0.915. Support it.
    if (config.HasFlag(View_Stereo))
    {
    }

    // Create the flip handler
    int returnValue = sceKernelCreateEqueue(&FlipEventQueue, "FlipEventQueue");
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceKernelCreateEqueue failure (error = %d)", returnValue);
    sceVideoOutAddFlipEvent(FlipEventQueue, VideoOutHandle, 0);

    // Create the memory manager.
    pMemManager = SF_NEW PS4::MemoryManager();

    // Setup the framebuffers
    FrontBuffer = FRAME_BUFFERS-1;
    BackBuffer  = 0;

    sce::Gnm::DataFormat format = determineFormat(config);
    void* renderTargetAddr[FRAME_BUFFERS];
    for( unsigned buffer=0; buffer<FRAME_BUFFERS; ++buffer )
    {
        // Currently, display render targets must be BGRA 8888 UNORM, LinearAligned
        sce::Gnm::TileMode tileMode;
        sce::GpuAddress::computeSurfaceTileMode(&tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, format, 1);
        DispSurfaceSize[buffer] = DispSurface[buffer].init( config.ViewSize.Width, config.ViewSize.Height, 1, 
            format, tileMode, sce::Gnm::kNumSamples1, sce::Gnm::kNumFragments1, NULL, NULL );
        renderTargetAddr[buffer] = pMemManager->Alloc(DispSurfaceSize[buffer], Memory_Orbis_UC_GARLIC_NONVOLATILE);
        DispSurface[buffer].setAddresses( renderTargetAddr[buffer], 0, 0 );
    }

    // Allocate space for frame labels.
    DispSurfaceLabels = reinterpret_cast<UInt64*>(pMemManager->Alloc(sizeof(UInt64) * FRAME_BUFFERS, sizeof(UInt64), Memory_Orbis_WB_ONION_NONVOLATILE));
    memset(DispSurfaceLabels, 0, FRAME_BUFFERS * sizeof(UInt64));

    // Depth/Stencil surface setup (use both).
    const sce::Gnm::StencilFormat stencilFormat = sce::Gnm::kStencil8;
    sce::Gnm::TileMode depthTileMode;
    sce::Gnm::DataFormat depthFormat = sce::Gnm::DataFormat::build(sce::Gnm::kZFormat32Float);
    sce::GpuAddress::computeSurfaceTileMode(&depthTileMode, sce::GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);

    DepthSize = DepthTarget.init( config.ViewSize.Width, config.ViewSize.Height, 
        depthFormat.getZFormat(), stencilFormat, depthTileMode, sce::Gnm::kNumFragments1, &StencilSize, 0 );

    void* stencilAddr = pMemManager->Alloc(StencilSize, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    void* depthBufferBaseAddr = pMemManager->Alloc(DepthSize, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    DepthTarget.setAddresses( depthBufferBaseAddr, stencilAddr );

    // Now that the render targets have been created, setup the video mode.
    SceVideoOutBufferAttribute attribute;
    sceVideoOutSetBufferAttribute(&attribute,
        SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
        SCE_VIDEO_OUT_TILING_MODE_TILE,
        SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
        config.ViewSize.Width, config.ViewSize.Height, config.ViewSize.Width);

    returnValue = sceVideoOutRegisterBuffers(VideoOutHandle, 0, renderTargetAddr, FRAME_BUFFERS, &attribute );
    if (returnValue < 0)
    {
        SF_DEBUG_ASSERT1(0, "sceVideoOutRegisterBuffers failed (error = 0x%08x)", returnValue);
        return false;
    }

    returnValue = sceKernelCreateEqueue(&FlipEventQueue, "FlipEventQueue");
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceKernelCreateEqueue failure (error = %d)", returnValue);
    sceVideoOutAddFlipEvent(FlipEventQueue, VideoOutHandle, 0);

    const uint32_t bufferSize = sce::Gnm::kIndirectBufferMaximumSizeInBytes;
#ifndef SF_PS4_USE_LCUE
    // Initialzie the Gnmx::GfxContext.
    const uint32_t numRingEntries = 16;
    const uint32_t cueCpRamShadowSize = sce::Gnmx::ConstantUpdateEngine::computeCpRamShadowSize();
    const uint32_t cueHeapSize = sce::Gnmx::ConstantUpdateEngine::computeHeapSize(numRingEntries);

    CueCpRamShadowBuffer = malloc(cueCpRamShadowSize);   // Note: this memory is not tracked by GFx allocators.
    void * cueHeapBuffer = pMemManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    void * dcbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);
    void * ccbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);

    // The parameter types to Gnmx::GfxContext::init were changed in 0.820.
    GnmxCtx.init( CueCpRamShadowBuffer, cueHeapBuffer, numRingEntries, dcbBuffer, bufferSize, ccbBuffer, bufferSize);
    
    // Set the buffer-full callback.
    GnmxCtx.m_bufferFullCallback = { ContextOutOfSpaceCallback, pHal.GetPtr()};

#else
    const uint32_t cueHeapSize = bufferSize;

    void * dcbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);
    void * cueHeapBuffer = pMemManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);

    GnmxCtx.init(reinterpret_cast<uint32_t*>(dcbBuffer), bufferSize / sizeof(uint32_t), 
                 reinterpret_cast<uint32_t*>(cueHeapBuffer), cueHeapSize / sizeof(uint32_t),
                 0, ContextOutOfSpaceCallback, pHal.GetPtr());
#endif

    if (!pHal->InitHAL(Render::PS4::HALInitParams(GnmxCtx, pMemManager, 0, renderThreadId)))
    {
        shutdownGraphics();
        return false;
    }

    // Create the default render target. This is required on Orbis (if you want to use render targets),
    // because there is no way to query the render surface in Gnm. If this is not done, Push/PopRenderTarget
    // in HAL will all fail. One instance with each buffer address is needed to swap buffers each frame
    pDisplayRT = *SF_NEW RenderTarget(0, RBuffer_Default, ImageSize(config.ViewSize.Width, config.ViewSize.Height) );

    // Notify the HAL of the current backbuffer, and have the HAL set it in the Gnmx::GfxContext.
    PS4::RenderTargetData::UpdateData(pDisplayRT, &DispSurface[BackBuffer], 0, &DepthTarget);
    pHal->SetRenderTarget(pDisplayRT, true);

    Status = Device_Ready;
    return true;
#endif
}

void DeviceImpl::shutdownGraphics()
{
    if (pHal)
        pHal->ShutdownHAL();

    Status = Device_NeedInit;

    sceKernelDeleteEqueue(FlipEventQueue);
}

void DeviceImpl::beginFrame()
{
    // Setup the GfxContext for rendering the next frame.
    GnmxCtx.reset();
    GnmxCtx.initializeDefaultHardwareState();
    GnmxCtx.waitUntilSafeForRendering(VideoOutHandle, BackBuffer);

    // Notify the HAL of the current backbuffer, and have the HAL set it in the Gnmx::GfxContext.
    PS4::RenderTargetData::UpdateData(pDisplayRT, &DispSurface[BackBuffer], 0, &DepthTarget);
    pHal->SetRenderTarget(pDisplayRT, true);
}

void DeviceImpl::clear(UInt32 color)
{
    // Clear the frame buffer to the given color. Since their is no easy hardware clear on Orbis, we
    // slightly mis-use the HAL here. Presumably game code will have its own way to clear the target.
    ViewConfig config;
    pView->GetViewConfig(&config);

    GnmxCtx.setupScreenViewport(0, 0, config.ViewSize.Width, config.ViewSize.Height, 0.5f, 0.5f);
    GnmxCtx.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);

    MatrixState previousMatrixState;
    previousMatrixState.CopyFrom(pHal->GetMatrices());
    pHal->GetMatrices()->UserView = Matrix2F::Identity;
    pHal->ClearSolidRectangle(Rect<int>(-1, -1, 1, 1), color|Color::Alpha100, false );
    pHal->GetMatrices()->CopyFrom(&previousMatrixState);
}

void DeviceImpl::presentFrame()
{
    // Write the frame index to the framebuffer's label (so we know when rendering to this framebuffer has completed)
    CurrentFrame++;
    GnmxCtx.writeImmediateAtEndOfPipe(sce::Gnm::EndOfPipeEventType::kEopFlushCbDbCaches, &DispSurfaceLabels[BackBuffer], 
        CurrentFrame, sce::Gnm::CacheAction::kCacheActionNone);
    ExpectedFrameLabels[BackBuffer] = CurrentFrame;

    // Kick the command buffer, and display it.
    GnmxCtx.submit();
    sce::Gnm::submitDone();
    SceVideoOutFlipMode flipMode = SCE_VIDEO_OUT_FLIP_MODE_VSYNC; 

    // Check VSync
    ViewConfig config;
    pView->GetViewConfig(&config);
    // TODOBM: HSync has terrible tearing artifacts, and sometimes the EOP label doesn't get written properly, so disable it for now.
    //if (!config.HasFlag(View_VSync))
    //    flipMode = SCE_VIDEO_OUT_FLIP_MODE_HSYNC;

    // Make sure that the next backbuffer is available to be written on (GPU is not still processing it).
    unsigned waitCount = 0;
    while (DispSurfaceLabels[BackBuffer] != ExpectedFrameLabels[BackBuffer])
    {
        waitCount++;
        usleep(100);

        // If we have waited one second for the rendering to complete, something has probably gone wrong.
        if (100*waitCount > 100*1000)
            break;
    }
    SF_DEBUG_ASSERT2(DispSurfaceLabels[BackBuffer] == ExpectedFrameLabels[BackBuffer], 
        "Expected frame label to be '%ld' (but it was %ld)", ExpectedFrameLabels[BackBuffer], DispSurfaceLabels[BackBuffer]);


    // Set Flip Request
    int returnValue = sceVideoOutSubmitFlip(VideoOutHandle, BackBuffer, flipMode, 0);
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceVideoOutSubmitFlip failure (error = %d)", returnValue);

    // Wait Flip
    SceKernelEvent flipEvent;
    int out;
    returnValue = sceKernelWaitEqueue(FlipEventQueue, &flipEvent, 1, &out, 0);
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceVideoOutSubmitFlip failure (error = %d)", returnValue);

    // Switch to the next framebuffer.
    FrontBuffer = BackBuffer;
    BackBuffer = (BackBuffer + 1) % FRAME_BUFFERS;
}

//------------------------------------------------------------------------
// *** Device.
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

bool Device::InitGraphics(const ViewConfig& config, Device::Window* view, ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, view, renderThreadId);
}

bool Device::ReconfigureGraphics(const ViewConfig& config)
{
    return true;
}

void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

bool Device::GraphicsConfigOnMainThread() const
{
    return true;
}

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

Size<unsigned> Device::GetDefaultViewSize()
{
    // Use the output resolution by default (returning 0x0 will detect it).
    return Size<unsigned>(0, 0);
}

void Device::Clear(UInt32 color)
{
    pImpl->clear(color);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::BeginFrame()
{
    pImpl->beginFrame();
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);

    pImpl->presentFrame();
}

bool Device::TakeScreenShot(const String&)
{
    return false;
}

UInt32 Device::GetCaps() const
{
    // Although Orbis has a rendering mode for wireframe, it doesn't appear to work.
    return DeviceCap_Wireframe;
}


void Device::ResizeFrame(void*)
{

}


}} // Scaleform::Platform

//------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Scaleform::Platform::AppBase* app = Scaleform::Platform::AppBase::CreateInstance();
    int  result = app->AppMain(argc, argv);
    Scaleform::Platform::AppBase::DestroyInstance(app);
    return result;
}

#endif // INC_SF_Platform_Orbis_H
