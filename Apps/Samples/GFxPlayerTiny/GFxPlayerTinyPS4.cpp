/**************************************************************************

Filename    :   GFxPlayerTinyOrbis.cpp
Content     :   Tiny FxPlayer sample for ORBIS
Created     :   September, 2012
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx.h"
#include "GFx/AS3/AS3_Global.h"

// Renderer includes
#include "Render/Renderer2D.h"
#include "Render/PS4/PS4_HAL.h"

// System includes
#include <scebase_common.h>
#include <gnm.h>
#include <gnmx.h>
#include <pad.h>
#include <video_out.h>
#include <sdk_version.h>

// If we are compiling with LCUE support, include the LCUE implementation source. Otherwise,
// it won't be linked in anywhere. In SDK 1.600, the LCUE was included in the general release,
// so this file is not required anymore.
#if defined(SF_PS4_USE_LCUE) && SCE_ORBIS_SDK_VERSION < 0x01600051u
#include "Platform/PS4/lcue.cpp"
#endif

// Specify the C-library heap size (256MB).
size_t	sceLibcHeapSize	= 256 << 20;
unsigned int sceLibcHeapExtendedAlloc = 1; // Enable

// For convenience
using namespace Scaleform;
using namespace Scaleform::Render;
using namespace Scaleform::GFx;

// Fake Windows key identifiers
#define VK_RETURN '\n'
#define VK_ESCAPE 257
#define VK_LEFT   258
#define VK_RIGHT  259
#define VK_UP     260
#define VK_DOWN   261

// SWF Filename we will load by default.
#define SWFMOVIE_FILENAME "/hostapp/flash.swf"

/////////////////////////////////////////////////////////////////////////////
//

class PlayerTiny
{
public:    
    PlayerTiny(const char* pfilename);    
    ~PlayerTiny();

    int     Run();

    bool    ConfigureGraphics();
    void    AdvanceAndDisplay();

    // Input handler functions. Process mouse and keyboard and pass 
    // appropriate GFxEvent notifications to FxMovieView::HandleEvent.
    bool    ProcessMessages();
    void    OnKey(unsigned keyCode, bool downFlag);
    void    HandleKeyEvent(unsigned keyCode, bool downFlag);

    bool    OpenPads();

protected:
    Ptr<MovieDef>               pMovieDef;                      // Contains movie definition data.
    Ptr<Movie>                  pMovie;                         // Instance of the movie.

    Ptr<Render::Renderer2D>     pRenderer;                      // The renderer (consumes MovieDisplayHandles).
    Ptr<Render::PS4::HAL>       pHAL;                           // HAL-backend, which does the actual rendering work.
    PS4::MemoryManager*         pMemManager;                    // The memory manager, used to manage video memory.
    ThreadCommandQueue*         pCommandQueue;                  // Command queue, used to communicate between Advance and Render.
    Ptr<Render::RenderTarget>   pDisplayRT;                     // GFx's structure defining the current render target.

    UInt32                      MovieLastTime;                  // Used to keep track of the current Advance time for the movie.

	MovieDisplayHandle          hMovieDisplay;                  // The display handle from pMovie, which is used to render.

    String                      FileName;                       // Playback filename
    int                         Width, Height;                  // Framebuffer dimensions.

    bool                        Wireframe;                      // true, if the rendering should be done in wireframe (solid if false).
    bool                        Paused;                         // Whether movie playback is paused.

    static const unsigned       FRAME_BUFFERS = 2;              // The number of framebuffers.
    uint32_t                    VideoOutHandle;                 // The handle to the opened video output.
    SceKernelEqueue             FlipEventQueue;                 // Event queue, which handles the flipping synchronization.
    sce::Gnm::RenderTarget      DispSurface[FRAME_BUFFERS];     // The framebuffers (double buffered).
    UInt64*                     DispSurfaceLabels;                  // Labels written by the GPU, to determine rendering has completed for the given buffer.
    UInt64                      ExpectedFrameLabels[FRAME_BUFFERS]; // The expected frame number to exist in DispSurfaceLables entry, to indicate when the frame is finished rendering.
    UInt64                      CurrentFrame;                       // The index of the current frame.
    sce::Gnm::DepthRenderTarget DepthTarget;                    // The depth/stencil target.
    sceGnmxContextType          GnmxCtx;                        // The Gnmx context (used for rendering).
    unsigned                    BackBuffer, FrontBuffer;        // Indices for which entries in DispSurface are the current front/back buffers.
    void*                       CueCpRamShadowBuffer;           // The shadow buffer used by the Gnmx::ConstantUpdateEngine (see Orbis docs).
    
    static const unsigned       MAX_PAD_NUM = 4;
    int                         PadHandle[MAX_PAD_NUM];         // Pad handles
    ScePadData                  LastPadData[MAX_PAD_NUM];       // Last pad state read in ProcessMessages.
    SceUserServiceUserId        UserId;
};

class FxPlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg)
    {
        SF_UNUSED3(pmovie, pcommand, parg);
    }
};

class FxPlayerThreadCommandQueue : public ThreadCommandQueue
{
public:

    virtual void GetRenderInterfaces( Render::Interfaces* p ) 
    {
        p->pHAL = pHAL;
        p->pRenderer2D = pR2D;
        p->pTextureManager = pHAL->GetTextureManager();
        p->RenderThreadID = 0;
    }

    virtual void PushThreadCommand( ThreadCommand* command ) 
    {
        if (command)
            command->Execute();
    }

    HAL* pHAL;
    Renderer2D* pR2D;
};


/////////////////////////////////////////////////////////////////////////////
//

PlayerTiny::PlayerTiny(const char* pfilename) :
    pMovieDef(0),
    pMovie(0),
    pRenderer(0),
    pHAL(0),
    pMemManager(0),
    pCommandQueue(0),
    pDisplayRT(0),
    MovieLastTime(0),
    hMovieDisplay(),
    FileName(pfilename),
    Width(1920),
    Height(1080),
    Wireframe(false),
    Paused(false),
    VideoOutHandle(0),
    DispSurfaceLabels(0),
    CurrentFrame(0),
    BackBuffer(0),
    FrontBuffer(0),
    CueCpRamShadowBuffer(0)
{
    memset(DispSurface, 0, sizeof(DispSurface));
    memset (&LastPadData, 0, sizeof (LastPadData));
}

PlayerTiny::~PlayerTiny()
{
    // Note: HAL/Renderer2D must be destroyed before the MemoryManager/CommandQueue, because they reference them.
    pRenderer = 0;
    pHAL      = 0;

    delete pCommandQueue;
    delete pMemManager;

    free(CueCpRamShadowBuffer);
    CueCpRamShadowBuffer = 0;

    sceKernelDeleteEqueue(FlipEventQueue);
    sceVideoOutClose(VideoOutHandle);
}


/////////////////////////////////////////////////////////////////////////////
//

int PlayerTiny::Run()
{
    // First, setup the graphics.
    if (!ConfigureGraphics())
        return 1;

    // Create and Configure GFxLoader.   
    Loader loader;

    // Developers set states on the loader to modify loading and playback behavior.
    // If you need to load files from a custom package, for example, you can 
    // create a FxFileOpener derived class that loads files in a custom way.
    // Here FxFileOpener and FxFSCommandHandler are set for sample purposes.    
    Ptr<FileOpener> pfileOpener = *new FileOpener;
    loader.SetFileOpener(pfileOpener); 
    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    loader.SetFSCommandHandler(pcommandHandler);

    // Add support for all GFx provided image handlers
    Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry(GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

    // AS2 support
    Ptr<ASSupport> pAS2Support = *new GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);

    // AS3 support
    Ptr<ASSupport> pASSupport = *new GFx::AS3Support();
    loader.SetAS3Support(pASSupport);


    // Load the movie file and create its instance.
    if (!(pMovieDef = *loader.CreateMovie(FileName)))
        return 1; 

    if (!(pMovie = *pMovieDef->CreateInstance(false, 0, 0, pCommandQueue)))
        return 1;

    // Force one 'mouse' (actually a pad) to be used by the movie that we loaded.
    pMovie->SetMouseCursorCount(1);


    // Set playback view to span the entire window.
    pMovie->SetViewport(Width, Height, 0, 0, Width, Height);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    MovieLastTime = Timer::GetTicks() / 1000;

    // Get the display handle of the movie we loaded (for rendering in AdvanceAndDisplay).
    hMovieDisplay = pMovie->GetDisplayHandle();

    // Open pads
    if (!OpenPads())
        return 1;

    // Application / Player message loop.
    while (true)
    {
        if (!ProcessMessages())
            break;

        AdvanceAndDisplay();
    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
//

bool PlayerTiny::ConfigureGraphics()
{
    // First, open the video handle, and get the display resolution.
    VideoOutHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, 0);
    SF_DEBUG_ASSERT(VideoOutHandle >= 0, "Unable to open video output handle.");

    // Configure based on TV mode
    SceVideoOutResolutionStatus resolutionInfo;
    sceVideoOutGetResolutionStatus(VideoOutHandle, &resolutionInfo);
    Height = resolutionInfo.fullHeight;
    Width  = resolutionInfo.fullWidth;

    // Create the memory manager/command queue. 
    pMemManager = SF_NEW PS4::MemoryManager();

    FxPlayerThreadCommandQueue* queue = new FxPlayerThreadCommandQueue;
    pCommandQueue = queue;

    // Setup the framebuffers
    FrontBuffer = FRAME_BUFFERS-1;
    BackBuffer  = 0;

    void* renderTargetAddr[FRAME_BUFFERS];
    for( unsigned buffer=0; buffer<FRAME_BUFFERS; ++buffer )
    {
        // Currently, display render targets must be BGRA 8888 UNORM, LinearAligned
        sce::Gnm::DataFormat format = sce::Gnm::kDataFormatB8G8R8A8Unorm;
        sce::Gnm::TileMode tileMode;
        sce::GpuAddress::computeSurfaceTileMode(&tileMode, sce::GpuAddress::kSurfaceTypeColorTargetDisplayable, format, 1);
        const sce::Gnm::SizeAlign sizeAlign = DispSurface[buffer].init( Width, Height, 1, format, tileMode, sce::Gnm::kNumSamples1, sce::Gnm::kNumFragments1, NULL, NULL );
        renderTargetAddr[buffer] = pMemManager->Alloc(sizeAlign, Memory_Orbis_UC_GARLIC_NONVOLATILE);
        DispSurface[buffer].setAddresses( renderTargetAddr[buffer], 0, 0 );
    }

    // Depth/Stencil surface setup (use both).
    sce::Gnm::SizeAlign stencilSizeAlign;
    const sce::Gnm::StencilFormat stencilFormat = sce::Gnm::kStencil8;
    sce::Gnm::TileMode depthTileMode;
    sce::Gnm::DataFormat depthFormat = sce::Gnm::DataFormat::build(sce::Gnm::kZFormat32Float);
    sce::GpuAddress::computeSurfaceTileMode(&depthTileMode, sce::GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);

    const sce::Gnm::SizeAlign depthTargetSizeAlign = DepthTarget.init( Width, Height, depthFormat.getZFormat(), stencilFormat, 
        depthTileMode, sce::Gnm::kNumFragments1, &stencilSizeAlign, 0 );

    void* stencilAddr = pMemManager->Alloc(stencilSizeAlign, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    void* depthBufferBaseAddr = pMemManager ->Alloc(depthTargetSizeAlign, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    DepthTarget.setAddresses( depthBufferBaseAddr, stencilAddr );

    // Now that the render targets have been created, setup the video mode.
    SceVideoOutBufferAttribute attribute;
    sceVideoOutSetBufferAttribute(&attribute,
        SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
        SCE_VIDEO_OUT_TILING_MODE_TILE,
        SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
        Width, Height, Width);

    int returnValue = sceVideoOutRegisterBuffers(VideoOutHandle, 0, renderTargetAddr, FRAME_BUFFERS, &attribute );
    if (returnValue < 0)
    {
        SF_DEBUG_ASSERT1(0, "sceVideoOutRegisterBuffers failed (error = 0x%08x)", returnValue);
        return false;
    }
    
    // Allocate space for frame labels.
    DispSurfaceLabels = reinterpret_cast<UInt64*>(pMemManager->Alloc(sizeof(UInt64) * FRAME_BUFFERS, sizeof(UInt64), Memory_Orbis_WB_ONION_NONVOLATILE));
    memset(DispSurfaceLabels, 0, FRAME_BUFFERS * sizeof(UInt64));

    returnValue = sceKernelCreateEqueue(&FlipEventQueue, "FlipEventQueue");
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceKernelCreateEqueue failure (error = %d)", returnValue);
    sceVideoOutAddFlipEvent(FlipEventQueue, VideoOutHandle, 0);

#ifndef SF_PS4_USE_LCUE
    // Initialzie the Gnmx::GfxContext.
	const uint32_t numRingEntries = 16;
	const uint32_t cueCpRamShadowSize = sce::Gnmx::ConstantUpdateEngine::computeCpRamShadowSize();
	const uint32_t cueHeapSize = sce::Gnmx::ConstantUpdateEngine::computeHeapSize(numRingEntries);
    const uint32_t bufferSize = sce::Gnm::kIndirectBufferMaximumSizeInBytes;
        
    CueCpRamShadowBuffer = malloc(cueCpRamShadowSize);   // Note: this memory is not tracked by GFx allocators.

    void * cueHeapBuffer = pMemManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    void * dcbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);
    void * ccbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);

    // The parameter types to Gnmx::GfxContext::init were changed in 0.820.
    GnmxCtx.init(CueCpRamShadowBuffer, cueHeapBuffer, numRingEntries, dcbBuffer, bufferSize, ccbBuffer, bufferSize);

#else
    const uint32_t bufferSize = sce::Gnm::kIndirectBufferMaximumSizeInBytes;
    const uint32_t cueHeapSize = bufferSize;

    void * dcbBuffer     = pMemManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);
    void * cueHeapBuffer = pMemManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);

    GnmxCtx.init(reinterpret_cast<uint32_t*>(dcbBuffer), bufferSize / sizeof(uint32_t), 
                 reinterpret_cast<uint32_t*>(cueHeapBuffer), cueHeapSize / sizeof(uint32_t),
                 0, 0, 0);
#endif

    // Create the HAL and Renderer2D
    pHAL = *SF_NEW PS4::HAL(pCommandQueue);
    pRenderer = *SF_NEW Renderer2D(pHAL);

    // Create the default rendertarget. This is required on Orbis (if you want to use render targets),
    // because there is no way to query the render surface in Gnm. If this is not done, Push/PopRenderTarget
    // in HAL will all fail. One instance with each buffer address is needed to swap buffers each frame
    pDisplayRT = *SF_NEW RenderTarget(0, RBuffer_Default, ImageSize(Width, Height) );

    // Initialize the HAL
    if (!pHAL->InitHAL(PS4::HALInitParams(GnmxCtx, pMemManager, 0, GetCurrentThreadId())))
        return false;
    return true;
}

void PlayerTiny::AdvanceAndDisplay()
{
    // Setup the GfxContext for rendering the next frame.
    GnmxCtx.reset();
    GnmxCtx.initializeDefaultHardwareState();
    GnmxCtx.waitUntilSafeForRendering(VideoOutHandle, BackBuffer);

    // Notify the HAL of the current backbuffer, and have the HAL set it in the Gnmx::GfxContext.
    PS4::RenderTargetData::UpdateData(pDisplayRT, &DispSurface[BackBuffer], 0, &DepthTarget);
    pHAL->SetRenderTarget(pDisplayRT, true);

    // Clear the frame buffer to black. Since their is no easy hardware clear on Orbis, we
    // slightly mis-use the HAL here. Presumably game code will have its own way to clear the target.
    GnmxCtx.setupScreenViewport(0, 0, Width, Height, 0.5f, 0.5f);
    GnmxCtx.setPrimitiveType(sce::Gnm::kPrimitiveTypeTriList);
    MatrixState previousMatrixState;
    previousMatrixState.CopyFrom(pHAL->GetMatrices());
    pHAL->GetMatrices()->UserView = Matrix2F::Identity;
    pHAL->ClearSolidRectangle(Rect<int>(-1, -1, 1, 1), Color::Black, false );
    pHAL->GetMatrices()->CopyFrom(&previousMatrixState);

    if (pMovie)
    {
        // Set Wireframe (based on the current setting).
        pHAL->SetRasterMode(Wireframe ? Render::PS4::HAL::RasterMode_Wireframe : Render::PS4::HAL::RasterMode_Solid);

        // Calculate the time update time for this frame.
        UInt32 time = Timer::GetTicks() / 1000;
        float delta = ((float)(time - MovieLastTime)) / 1000.0f;

        // Advance time and display the movie.
        if (!Paused)
            pMovie->Advance(delta);

		pRenderer->BeginFrame();
		if (hMovieDisplay.NextCapture(pRenderer->GetContextNotify()))
			pRenderer->Display(hMovieDisplay);
		pRenderer->EndFrame();

        MovieLastTime = time;
    }

    // Write the frame index to the framebuffer's label (so we know when rendering to this framebuffer has completed)
    CurrentFrame++;
    GnmxCtx.writeImmediateAtEndOfPipe(sce::Gnm::EndOfPipeEventType::kEopFlushCbDbCaches, &DispSurfaceLabels[BackBuffer], 
        CurrentFrame, sce::Gnm::CacheAction::kCacheActionNone);
    ExpectedFrameLabels[BackBuffer] = CurrentFrame;

    // Kick the command buffer, and display it.
    GnmxCtx.submit();
    sce::Gnm::submitDone();

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
    int returnValue = sceVideoOutSubmitFlip(VideoOutHandle, BackBuffer, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceVideoOutSubmitFlip failure (error = %d)", returnValue);

    // Wait Flip
    SceKernelEvent flipEvent;
    int out;
    returnValue = sceKernelWaitEqueue(FlipEventQueue, &flipEvent, 1, &out, 0);
    SF_DEBUG_ASSERT1(returnValue >= 0, "sceVideoOutSubmitFlip failure (error = %d)", returnValue);

    // Switch to the next framebuffer.
    FrontBuffer = BackBuffer;
    BackBuffer = (BackBuffer + 1) % FRAME_BUFFERS;

    // Notify AMP that we have finished a frame.
    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame());
}

bool PlayerTiny::OpenPads()
{
    sceUserServiceInitialize(NULL);
    sceUserServiceGetInitialUser(&UserId);

    int ret = scePadInit();
    if (ret != SCE_OK)
    {
        SF_DEBUG_ASSERT1(0, "scePadInit failed (error = 0x%08x)", ret);
        return false;
    }
    for (unsigned pad = 0; pad < MAX_PAD_NUM; ++pad)
    {
        PadHandle[pad] = scePadOpen(UserId, SCE_PAD_PORT_TYPE_STANDARD, pad, NULL);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Message processing function to be called in the  application loops.
//

static int keymap[] = {' ', 'c', 'v', '\n', VK_UP, VK_RIGHT, VK_DOWN, VK_LEFT, 'L', 'R', 'l', 'r', 'T', 'O', 'X', 'S'};

bool PlayerTiny::ProcessMessages()
{   
    static const unsigned GFX_GAMEPAD_DEADZONE = 16;
    ScePadData newPadData;

    // Any pad can do the input, but they are all processed as the single input provider to the movie.
    for (unsigned pad = 0; pad < MAX_PAD_NUM; ++pad)
    {
        if (PadHandle[pad] > 0 && scePadReadState(PadHandle[pad], &newPadData) == SCE_OK)
        {
            // Determine if any digital buttons were pressed/released.
            unsigned button, index;
            for (button = 1, index = 0; button <= SCE_PAD_BUTTON_SQUARE; button <<= 1, ++index)
            {
                if ((LastPadData[pad].buttons & button) != (newPadData.buttons & button))
                    OnKey(keymap[index], (newPadData.buttons & button) ? true : false);
            }

            // Map left stick to WASD
            // Left Stick Horizontal checks
            bool bDKey      = ((newPadData.leftStick.x  & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
            bool bDKeyPrev  = ((LastPadData[pad].leftStick.x & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
            if ( bDKey != bDKeyPrev )
                OnKey('d', bDKey);

            bool bAKey      = ((newPadData.leftStick.x  & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE);
            bool bAKeyPrev  = ((LastPadData[pad].leftStick.x & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE); 
            if (bAKey != bAKeyPrev)
                OnKey('a', bAKey);

            // Left Stick Vertical checks
            bool bWKey      = ((newPadData.leftStick.y  & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
            bool bWKeyPrev  = ((LastPadData[pad].leftStick.y & 0xF8) > 128 + GFX_GAMEPAD_DEADZONE);
            if ( bWKey != bWKeyPrev  )
                OnKey('s', bWKey);

            bool bSKey      = ((newPadData.leftStick.y  & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE);
            bool bSKeyPrev  = ((LastPadData[pad].leftStick.y & 0xF8) < 128 - GFX_GAMEPAD_DEADZONE); 
            if ( bSKey != bSKeyPrev)
                OnKey('w', bSKey);

            LastPadData[pad] = newPadData;
        }
    }
    return 1;
}

// Helper function that converts Windows VK keyCode values
// to GFxKeyEvents and routes them to GFxPlayer.
void PlayerTiny::HandleKeyEvent(unsigned keyCode, bool downFlag)
{
    Key::Code key(Key::None);

    if (keyCode >= 'A' && keyCode <= 'Z')
    {
        key = (Key::Code) ((keyCode - 'A') + Key::A);
    }
    else if (keyCode >= '0' && keyCode <= '9')
    {
        key = (Key::Code) ((keyCode - '0') + 48);
    }
    else
    {
        // Use a look-up table for keys don't correlate in order,.
        static struct {
            int          vk;
            Key::Code gs;
        } table[] =
        {
            { VK_RETURN,    Key::Return },
            { VK_ESCAPE,    Key::Escape },
            { VK_LEFT,      Key::Left },
            { VK_UP,        Key::Up },
            { VK_RIGHT,     Key::Right },
            { VK_DOWN,      Key::Down },

            // TODO: fill this out some more
            { 0, Key::None }
        };

        for (int i = 0; table[i].vk != 0; i++)
        {
            if (keyCode == (unsigned)table[i].vk)
            {
                key = table[i].gs;
                break;
            }
        }
    }

    if (key != Key::None)
    {
        if (pMovie)
        {
            // Pass Key events to the movie so that can be handled in ActionScript.
            KeyEvent event(downFlag ? GFx::Event::KeyDown : KeyEvent::KeyUp, key);
            pMovie->HandleEvent(event);
        }
    }
}

// OnKey handles system keys by first interpreting some Ctrl+Key combinations
// in a special way and passing the rest of them to the movie.
void PlayerTiny::OnKey(unsigned keyCode, bool downFlag)
{
    switch(keyCode)
    {
    case 'w':
    case 'a':
    case 's':
    case 'd':
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
        HandleKeyEvent(keyCode, downFlag);
        return;

    case 'X':
        HandleKeyEvent(VK_RETURN, downFlag);
        return;

    case 'O':
        HandleKeyEvent(VK_ESCAPE, downFlag);
        return;
    }

    if (!downFlag)
        return;

    switch(keyCode)
    {
    case 'T':
        // Toggle wireframe.
        Wireframe = !Wireframe;
        break;

    case '\n':
        Paused = !Paused;
        break;
    }
}



/////////////////////////////////////////////////////////////////////////////
// The main function implementation initializes GFx, providing the memory allocator.
// It also creates the FxPlayerTiny class, passes it the filename and calls Run. 
// FxPlayerTiny::Run is responsible for all of the rendering setup and movie playback.

int main(int argc, char *argv[])
{
	SysAllocMalloc sysAlloc;
	GFx::System gfxInit(&sysAlloc);

	PlayerTiny player(SWFMOVIE_FILENAME);
	int result = player.Run();

	return result;
}
