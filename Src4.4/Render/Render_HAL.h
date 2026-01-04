/**************************************************************************

PublicHeader:   Render
Filename    :   Render_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_RenderHAL_H
#define INC_SF_Render_RenderHAL_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_AllocAddr.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_ListAlloc.h"
#include "Kernel/SF_ThreadCheck.h"

#include "Render/Render_Types2D.h"
#include "Render/Render_Color.h"
#include "Render/Render_Vertex.h"
#include "Render/Render_Containers.h"
#include "Render/Render_Primitive.h"
#include "Render/Render_Viewport.h"
#include "Render/Render_States.h"
#include "Render/Render_Buffer.h"
#include "Render/Render_DrawableImage.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Render/Render_Bundle.h"
#include "Render/Render_Queue.h"
#include "Render/Render_Events.h"
#include "Render/Render_Profiler.h"
#include "Render/Render_MatrixState.h"
#include "Render/Render_GlyphCache.h"
#include "Render/Render_TessGen.h"

namespace Scaleform { namespace Render {

typedef Matrix2F Matrix;
class RenderQueueProcessor;
class TreeRoot;
struct ToleranceParams;
struct MeshCacheParams;
struct GlyphCacheParams;
class MeshKeyManager;
class GraphicsDevice;

//------------------------------------------------------------------------
// Generic configuration flags, that apply to multiple platforms.
enum HALConfigFlagsType
{
    // The HAL will not submit commands directly to the graphics API. Instead, they will
    // be queued in a command buffer, allowing the majority of CPU-side rendering execution
    // to happen on a thread other than the one that owns the device. The queued commands
    // should then be executed on the device thread using the HAL::Submit command.
    HALConfig_SoftwareDeferredContext   = 0x80000000,
    
    // Disable the generation of GPU events used in GPU profiling tools. These events may be useful
    // in a tool, to debug a GPU frame, or examine performance (dependent on the specific tool).
    // These events do take a certain amount of performance themselves, so disabling them may
    // provide a speedup. These events are always disabled unless SF_RENDERER_PROFILE is defined.
    HALConfig_DisableRenderEvents       = 0x40000000,

    // Increase the frequency of GPU fence insertion when accessing resources (if possible). 
    // In general, the system will only fence resources once per scene. If resource caches are
    // over-utilized, the system will have to wait for the GPU to finish rendering the entire scene
    // before reclaiming these resources. This flag allows reclamation of resources intra-scene.
    // This flag may have no effect, depending on the fencing mechanism on the platform, for instance,
    // in D3D9/D3D1x, only one query is ever used per scene, as they are expensive to issue.
    HALConfig_FencePerDrawCall          = 0x20000000,

    // If the viewport does not match the dimensions of the content, filter parameters are scaled
    // appropriately so that they appear in the same ratios they do when viewed at the regular viewport size.
    // For example, the blur filter will increase the size of the blur kernel, based on the scaling
    // of the content into the viewport. If this flag is used, this scaling is disabled, and filter
    // parameters remain constant, even if the viewport changes.
    HALConfig_DisableFilterScaling      = 0x10000000,

    // Backends that support deferred context rendering will automatically call the HAL::Submit method
    // on EndScene, if the current thread is executing in the immediate device's thread. It is possible
    // that the user would like to have Submit called at another time, passing this flag will disable
    // the implicit call to Submit, and the user must call this explictly.
    HALConfig_DisableImplicitSubmit     = 0x08000000,

    // By default, GFx initializes all shaders during InitHAL. This flag disables that behavior, and
    // only initializes shaders when they are referenced. In general, the subset of shaders used by
    // any content is much lower than the total number of available shaders. Depending on the platform
    // and content, this may save significant memory and/or startup time. This is particularly true 
    // in GL where shaders cannot be compiled offline. However, this may have performance implications
    // when referencing initializing shaders during playback.
    HALConfig_DynamicShaderInit         = 0x04000000,
};

// Parameter to Display, specifying which pass to render.
enum DisplayPass
{
    Display_Prepass =   0x0001, // Prepass rendering renders all render targets in the scene. This is required on some platforms (X360/Wii).
    Display_Final   =   0x0002, // Final pass rendering, using cached results of the prepass (if required).
    Display_All     =   0x0003  // Render both passes concurrently.
};

//------------------------------------------------------------------------
// 
enum HALNotifyType
{
    HAL_Initialize,
    HAL_Shutdown,
    HAL_PrepareForReset,
    HAL_RestoreAfterReset,
    HAL_FinalPassBegin,
};

// HALNotify interface can be installed on HAL to be informed of events such
// as video mode changes.
class HALNotify : public ListNode<HALNotify>
{
public:
    HALNotify() { pPrev = pNext = 0; }
    virtual ~HALNotify() {}

    virtual void OnHALEvent(HALNotifyType type) = 0;
};


//------------------------------------------------------------------------

// HALInitParams contains common arguments passed to HAL::InitHAL function.
// Typically this structure is not used directly, with a system-specific
// derived structure such as D3D9::HALInitParams or PS3::HALInitParams
// being passed instead.

struct HALInitParams
{
    // MemoryManager to use for video memory allocations, applicable to consoles.
    // This value is ignored with D3D and OpenGL and can be null.
    MemoryManager*              pMemoryManager;
    // ConfigFlags contains platform-specific settings defined by HALConfig
    // enumerations in the appropriate platform-specific back ends,
    // such as D3D9::HALConfig_NoSceneCalls.
    unsigned                    ConfigFlags;
    // RenderThreadId identifies the main renderer thread; it is used to identify which thread
    // is currently executing, and is used by the texture manager, and DrawableImage systems.
    // Should not be null.
    ThreadId                    RenderThreadId;
    // Texture manager can be created early and specified here. Early texture manager
    // creation allows resource loading to take place before HAL init.
    // If null, default texture manager will be created by InitHAL.
    Ptr<TextureManager>         pTextureManager;
    // RenderBufferManager controls creation and lifetime of render target buffers;
    // this class can be substituted to alter buffer memory management strategy.
    Ptr<RenderBufferManager>    pRenderBufferManager;
    // RenderQueueSize controls the size of the internal queue of RenderQueueItems 
    // used by the renderer. Once this limit is reached, a rendering flush will be required.
    unsigned                    RenderQueueSize;
    
    HALInitParams(MemoryManager* mmanager = 0, UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : pMemoryManager(mmanager), ConfigFlags(halConfigFlags), RenderThreadId(renderThreadId), 
      pTextureManager(0), pRenderBufferManager(0), RenderQueueSize(RenderQueue::DefaultQueueSize)
    { }
};

//---------------------------------------------------------------------------------------
class BeginDisplayData : public ListNode<BeginDisplayData>
{
public:
    BeginDisplayData() { }
    BeginDisplayData( const Color & bgcol, const Viewport & vpin, bool fullSceneBlendTarget, const Matrix2F& viewMatrix) :
    BackgroundColor(bgcol), VP(vpin), FullSceneBlendTarget(fullSceneBlendTarget), ViewMatrix(viewMatrix) {} 
    Color       BackgroundColor;
    Viewport    VP;
    bool        FullSceneBlendTarget;
    Matrix2F    ViewMatrix;
};

//--------------------------------------------------------------------
// HAL - Abstract renderer interface (aka. Hardware Abstraction Layer)

class HAL : public RefCountBase<HAL, StatRender_Mem>, public Context::RenderNotify
{
    friend class ComplexMesh;
    friend class HALBeginDisplayItem;
    friend class HALEndDisplayItem;
    friend class ProfileModifierTDensity;
public:

    // *** Public Enums

    // HAL State flags are checked for during most API calls to ensure that
    // state is valid for the operation. Check is done by checkState(bits),
    // which will emit error message for some cases. Not all of these are applicable
    // on all platforms.
    enum HALStateFlags
    {
        HS_Initialized      = 0x00001,  // InitHAL has been called.
        HS_InFrame          = 0x00002,  // BeginFrame called.
        HS_InScene          = 0x00004,  // BeginScene called.
        HS_InDisplay        = 0x00008,  // Rendering fails if not in display.
        HS_InRenderTarget   = 0x00010,  // Rendering into a render target (not the main target).
        HS_ViewValid        = 0x00020,  // Non-empty viewport; culls rendering.
        HS_DrawingMask      = 0x00040,  // Set when drawing a mask
        HS_DrawingFilter    = 0x00080,  // Set when drawing a filter (from scratch)
        HS_InCachedFilter   = 0x00100,  // Set when drawing a filter from a cached result.
        HS_InCachedBlend    = 0x00200,  // Set when drawing a filter from a cached result.
        HS_SceneInDisplay   = 0x00400,  // Set if BeginScene was called by BeginDisplay
        HS_InCachedTarget   = HS_InCachedFilter | HS_InCachedBlend,  // Set when a filter or target blend from a cached result.

        HS_DeviceValid      = 0x01000,  // If not valid, device is Lost (3DS/D3D9 only).
        HS_ReadyForReset    = 0x02000,  // Set when reset-dependent resources are released (D3D9/D3D1x only).
        HS_InGxmScene       = 0x10000,  // Currently in a gxm scene (PSVITA only).
        HS_SceneChanged     = 0x20000,  // The gxm scene was changed after BeginDisplay (PSVITA only).
        HS_GnmNeedsInit     = 0x40000,  // true, if the Gnmx context needs reset and initializeHardwareStates called on it (PS4 only).
    };

    // Flags available to the Push/PopRenderTarget method (defaults to none).
    enum PushRenderTargetFlags
    {
        PRT_NoClear     = 0x01,     // Do not clear the render target being pushed (clear is done by default).
        PRT_Resolve     = 0x02,     // Resolve the current backbuffer to a texture before rendering (X360).
        PRT_NoSet       = 0x04,     // Do not actually set the render target on the device (optimization for filter drawing). Implies PRT_NoClear.
        PRT_Subscene    = 0x08,     // The content will be rendered to target is a subscene. This implies HS_InRenderTarget, and removes UserMatrix transformations.
        PRT_NoEvent     = 0x10,     // Disables RenderEvents for the Push/Pop operation.
    };

    // Specifies the type of rasterization to perform. Not all of these are available on all platforms. See IsRasterModeSupported.
    enum RasterModeType
    {
        RasterMode_Solid,       // Primitives are solid.
        RasterMode_Wireframe,   // Primitives are rendered in wireframe.
        RasterMode_Point,       // Primitives are rendered as points.
        RasterMode_Count,
        RasterMode_Default = RasterMode_Solid
    };


    // *** Public structs

    // Holds stats as recorded by the HAL during rendering.
    struct Stats
    {
        unsigned Primitives;    // Number of actual graphics API 'draw' calls performed (eg. DrawIndexedPrimitive)
        unsigned Meshes;        // Number of meshes drawn (including the number of instances for instanced draws)
        unsigned Triangles;     // Number of triangles rendered in all draw calls.
        unsigned Masks;         // Number of masks rendered.
        unsigned RTChanges;     // Number of render target changes performed (between HAL::BeginScene/EndScene).
        unsigned Filters;       // Number of filters rendered, does not include cached filter rendering.

        Stats() : Primitives(0), Meshes(0), Triangles(0), Masks(0), RTChanges(0), Filters(0) { }

        void Clear() { Primitives = Meshes = Triangles = Masks = RTChanges = Filters = 0; }
    };

    // *** Public API

    // HAL tracks a command queue that allows its associated systems to post commands for the render thread.
    HAL(ThreadCommandQueue *commandQueue);
    virtual ~HAL();

    // Configuration options of subsystems. For optimum performance, these should be set before calling InitHAL.
    // Calling them after InitHAL is possible, but may require significant memory reallocation.
    const ToleranceParams&  GetToleranceParams() const;
    void                    SetToleranceParams(const ToleranceParams& params);
    const MeshCacheParams&  GetMeshCacheParams();
    void                    SetMeshCacheParams(const MeshCacheParams& params);
    const GlyphCacheParams& GetGlyphCacheParams();
    void                    SetGlyphCacheParams(const GlyphCacheParams& param);

    // Adds an external notification object that will be told about HAL
    // events such as InitHAL/ShutdownHAL. Should be removed with RemoveNotify.
    void                    AddNotify(HALNotify *notify);
    void                    RemoveNotify(HALNotify *notify);    

    // Creates a user RenderTarget, or a temporary RenderTarget (which may be evicted with subsequent render target allocations).
    // Note: Platform specific versions of CreateRenderTarget exist in platform-based derived HAL.
    virtual RenderTarget*   CreateRenderTarget(Texture* texture, bool needsStencil) = 0;
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil) = 0;

    // Applies a render target. This should be called before BeginDisplay, to have contents appear within the target.
    // If not called before InitHAL, the HAL will attempt to create a default RenderTarget by querying the graphics API
    // (when the platform allows this). 
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = true);

    // Begin rendering to the specified target.
    // Texture referenced by prt must not be used as an input until PopRenderTarget. Flags are available,
    // see PushRenderTargetFlags. If the PRT_NoClear flag is not set, the target will be cleared to 'clearColor'.
    // If PRT_Subscene is in the flags, the frameRect represents the scene space area that contains the content to be rendered.
    virtual bool            PushRenderTarget(RenderTarget* prt, unsigned flags = 0, const Rect<int> frameRect = Rect<int>(), Color clearColor = 0);

    // Restore previous render target. Contents of Texture in popped render target are now available
    // for rendering. Flags are available, see PushRenderTargetFlags.
    virtual bool            PopRenderTarget(unsigned flags = 0);

    // Returns true if HAL has been initialized through a call to InitHAL.
    virtual bool            IsInitialized() const { return (HALState & HS_Initialized) != 0; }

    // Initialization and shutdown.
    virtual bool            InitHAL(const HALInitParams& params);
    virtual bool            ShutdownHAL();

    // PrepareForReset and RestoreAfterReset deal with the situation in which the rendering context is lost.
    // This function does not need an implementation in most backends, only D3D9, D3D1x and Android, as those
    // are the only platforms which have a concept of losing rendering resources (to varying degrees).
    virtual bool            PrepareForReset();
    virtual bool            RestoreAfterReset();

    // Begins rendering frame, which is a caching unit. BeginFrame/EndFrame pair
    // must always be called around BeginScene/EndScene.
    virtual bool            BeginFrame();
    virtual void            EndFrame();

    // Flush performs all queued rendering. This must be called for any rendering results
    // to be displayed (generally before Present is called). It is called automatically from
    // EndScene, usually an explicit call is not required. This allows for multiple 
    // BeginDisplay/EndDisplay to be queued between calls to BeginScene/EndScene.
    virtual void            Flush();

    // If using a deferred context (either supported by the graphics API as with D3D11 deferred contexts), or 
    // software deferred contexts (using the HALConfig_SoftwareDeferredContext flag), this call will submit 
    // queued commands.
    virtual bool            Submit();

    // BeginScene begins scene rendering, initializing various render states needed
    // externally; EndScene completes scene rendering. In Direct3D, this will call
    // BeginScene/EndScene flags on the device if the 
    // If not called explicitly, these functions will be automatically called from
    // BeginDisplay and EndDisplay, explicitly. Calling them externally is more
    // efficient if multiple BeginDisplay/EndDisplay blocks will take place, as it
    // optimizes state changes and eliminates queue flush.
    virtual bool            BeginScene();
    virtual bool            EndScene();

    // Bracket rendering of a display unit that potentially has its own
    // viewport, such Render::TreeNode (GFx::MovieView).
    // Fill the background color, and set up default transforms, etc.
    void                    BeginDisplay(Color backgroundColor, const Viewport& viewport, 
                                         bool fullsceneBlendTarget = false, const Matrix2F& viewMatrix = Matrix2F::Identity);
    void                    EndDisplay();

    // Renders a display tree starting with TreeRoot. TreeRoot must've been allocated from the specified context.
    void                    Display(const DisplayHandle<TreeRoot>& hroot)   { Draw(hroot.GetRenderEntry()); }
    void                    Draw(const DisplayHandle<TreeRoot>& hroot)      { Draw(hroot.GetRenderEntry()); }
    void                    Draw(TreeRoot *pnode);
    virtual void            ClearSolidRectangle(const Rect<int>& r, Color color, bool blend) = 0;

    // Forces update of render tree, in order to apply changes in TreeNodes.
    // If pcontext is specified it will update only roots corresponding to the
    // context; otherwise all roots will be used.
    void                    ForceUpdateImages(Context* pcontext = 0);

    // Subsystem accessors.
    virtual Context::RenderNotify*  GetContextNotify()          { return this; }
    virtual ThreadCommandQueue*     GetThreadCommandQueue()     { return pRTCommandQueue; }
    virtual Render::MemoryManager*  GetMemoryManager()          { return pMemoryManager; }
    virtual RenderBufferManager*    GetRenderBufferManager()    { return pRenderBufferManager; };
    virtual ProfileViews&           GetProfiler()               { return Profiler; }
    virtual const ProfileViews&     GetProfiler() const         { return Profiler; }
    virtual RQCacheInterface&       GetRQCacheInterface()       { return QueueProcessor.GetQueueCachesRef(); }
    virtual RenderQueueProcessor&   GetRQProcessor()            { return QueueProcessor; }
    virtual RenderSync*             GetRenderSync()             { return 0; }
    virtual PrimitiveFillManager&   GetPrimitiveFillManager()   { return FillManager; }
    virtual GlyphCache*             GetGlyphCache()             { return pGlyphCache; }
    virtual MeshKeyManager*         GetMeshKeyManager()         { return pMeshKeyManager; }
    virtual MeshGenerator*          GetMeshGen()                { return &MeshGen; }
    virtual StrokeGenerator*        GetStrokeGen()              { return &StrokeGen; }
    virtual MatrixPool&             GetMatrixPool()             { return MPool; }
    virtual MatrixStateFactory&     GetMatrixStateFactory()     { return *pMatrixFactory; }
    virtual MatrixState*            GetMatrices()               { return Matrices; }
    virtual RenderEvents&           GetEvents();
    virtual void                    GetStats(Stats* pstats, bool clear = true);
    virtual TextureManager*         GetTextureManager() = 0;
    virtual MeshCache&              GetMeshCache() = 0;
    virtual Render::GraphicsDevice* GetGraphicsDeviceBase()     { return 0; };

    // HAL State accessors.
    unsigned                        GetHALState() const                                { return HALState; }
    UInt32                          GetConfigFlags() const                             { return ConfigFlags; }
    ThreadId                        GetRenderThreadId() const                          { return RenderThreadID; }
    virtual float                   GetViewportScaling() const                         { return -1.0f; }
    virtual const Viewport&         GetVP() const                                      { return VP; }
    virtual bool                    IsRasterModeSupported(RasterModeType mode) const   { SF_UNUSED(mode); return true; }
    virtual bool                    IsPrepassRequired() const { return false; }
    virtual DisplayPass             GetDisplayPass() const { return CurrentPass; }

    // HAL output control functions.
    void                            SetRasterMode(RasterModeType mode)                 { NextSceneRasterMode = mode; }
    virtual void                    SetStereoImpl(StereoImplBase* simpl);
    virtual void                    SetStereoParams(StereoParams sParams);
    virtual void                    SetStereoDisplay(StereoDisplay sDisplay, bool setstate = false);
    virtual void                    SetDisplayPass(DisplayPass pass) { CurrentPass = pass; }

    // *** Uncommonly used Public API functions.
    virtual void                    DrawBundleEntries(BundleIterator ibundles);
    virtual void                    Draw(const RenderQueueItem& item);    
    void                            Draw(RenderQueueItem::Interface* i, void* data = 0);   
    virtual void                    DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend) = 0;
    virtual void                    DrawProcessedComplexMeshes(ComplexMesh* p, const StrideArray<HMatrix> &matrices) = 0;

    virtual void                    PushMask_BeginSubmit(MaskPrimitive* primitive);
    virtual void                    EndMaskSubmit();
    virtual void                    PopMask();

    virtual void                    PushBlendMode(BlendPrimitive* prim);
    virtual void                    PopBlendMode();
    void                            ApplyBlendMode(BlendMode mode, bool sourceAc = false, bool forceAc = false);

    virtual void                    PrepareCacheable(CacheablePrimitive*, bool unprepare);
    virtual void                    PushFilters(FilterPrimitive*);
    virtual void                    PopFilters();

    void                            PushView3D(const Matrix3F &m);
    void                            PushProj3D(const Matrix4F &m);
    void                            PopView3D();
    void                            PopProj3D();

    virtual void                    PushUserData(const UserDataState::Data* data);
    virtual void                    PopUserData();

    virtual unsigned                DrawableCommandGetFlags(const DICommand* pcmd) const;
    virtual void                    DrawableCxform( Texture** tex, const Matrix2F* texgen, const Cxform* cx);
    virtual void                    DrawableCompare( Texture** tex, const Matrix2F* texgen);
    virtual void                    DrawableCopyChannel( Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul );
    virtual void                    DrawableMerge( Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul );
    virtual void                    DrawableCopyPixels( Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                                        bool mergeAlpha, bool destAlpha );
    virtual void                    DrawablePaletteMap( Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                                        unsigned channelMask, const UInt32* values);
    virtual void                    DrawableCopyback( Render::Texture* tex, const Matrix2F& mvp, const Matrix2F& texgen, unsigned flagMask = 0xFFFFFFFF );

    // Obtains formats that renderer will use for single, batches and instanced rendering of
    // the specified source format.
    //   - Filled in pointer may be the same as sourceFormat.
    //   - 'batch' format may be reported as 0, in which case batching is disabled.
    //   - 'instanced' format may be reported as 0, in which instancing is not supported for format.
    virtual void                    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                                    const VertexFormat** single, const VertexFormat** batch, 
                                                    const VertexFormat** instanced, unsigned meshType) = 0;

    // Returns the maximum number of batches, based on the number of available uniforms, and the number required per-batch
    // (on shader-platforms). The MeshCacheParams can lower the return value of this function further.
    // If the user modifies the shader type, during the emit step (eg. with the PushUserData API), and this changes uniform usage,
    // they must account for this, to return the minimum possible batch count, based on any possible modification.
    virtual unsigned                GetMaximumBatchCount(Primitive* prim) = 0;

    virtual bool                    ShouldRenderFilters(const FilterPrimitive*) const      { return true; }
    virtual bool                    ShouldRenderTargetBlend(const BlendPrimitive*) const   { return false; }

    SF_AMP_CODE(void GetExternalMemory(MemItem* root);)

protected:

    // *** Private Enums

    // Used with render states which can be enabled and disabled, but may also be ignored.
    enum EnableIgnoreValue
    {
        EnableIgnore_Off    = 0,
        EnableIgnore_On     = 1,
        EnableIgnore_Ignore = 2
    };

    // Enumeration of possible blend operations.
    enum BlendOp
    {
        BlendOp_ADD,
        BlendOp_MAX,
        BlendOp_MIN,
        BlendOp_REVSUBTRACT,
        BlendOp_Count
    };

    // Enumeration of possible blend factors.
    enum BlendFactor
    {
        BlendFactor_ZERO,
        BlendFactor_ONE,
        BlendFactor_SRCALPHA,
        BlendFactor_INVSRCALPHA,
        BlendFactor_DESTCOLOR,
        BlendFactor_INVDESTCOLOR,
        BlendFactor_Count
    };


    // The depth/stencil modes. Generally used to perform masking operations.
    enum DepthStencilMode
    {
        DepthStencil_Invalid,                   // Used to invalidate these states at during BeginFrame.
        DepthStencil_Disabled,                  // All depth and stencil testing/writing is disabled.

        // Used for writing stencil values (depth operations disabled, color write enable off)
        DepthStencil_StencilClear,              // Clears all stencil values to the stencil reference.
        DepthStencil_StencilClearHigher,        // Clears all stencil values lower than the reference, to the reference.
        DepthStencil_StencilIncrementEqual,     // Increments any stencil values that match the reference.

        // Used for testing stencil values (depth operations disabled).
        DepthStencil_StencilTestLessEqual,      // Allows rendering where the stencil value is less than the reference.

        // Used for writing depth values (stencil operations disabled, color write enable off).
        DepthStencil_DepthWrite,                // Depth is written (but not tested), stenciling is disabled.

        // Used for testing depth values (stencil operations disabled).
        DepthStencil_DepthTestEqual,            // Allows rendering where the depth values are equal, stenciling and depth writing are disabled.

        DepthStencil_Count
    };

    // The depth/stencil test functions used by the renderer
    enum DepthStencilFunction
    {
        DepthStencilFunction_Ignore,
        DepthStencilFunction_Never,
        DepthStencilFunction_Less,
        DepthStencilFunction_Equal,
        DepthStencilFunction_LessEqual,
        DepthStencilFunction_Greater,
        DepthStencilFunction_NotEqual,
        DepthStencilFunction_GreaterEqual,
        DepthStencilFunction_Always,
        DepthStencilFunction_Count
    };

    // The stencil operations used by the renderer
    enum StencilOp
    {
        StencilOp_Ignore,
        StencilOp_Keep,
        StencilOp_Replace,
        StencilOp_Increment,
        StencilOp_Count
    };

    // *** Private structs
    struct HALDepthStencilDescriptor
    {
        EnableIgnoreValue           DepthTestEnable;    // true if depth testing should be performed, false otherwise (DepthFunction ignored).
        EnableIgnoreValue           DepthWriteEnable;   // true if depth writing should be performed, false otherwise.
        EnableIgnoreValue           StencilEnable;      // true if stenciling should be performed, false otherwise (StencilFunction ignored).
        EnableIgnoreValue           ColorWriteEnable;   // true if color writing should be enabled, false otherwise.
        DepthStencilFunction        DepthFunction;      // The function to perform when doing depth testing, determining whether a fragment passes or fails.
        DepthStencilFunction        StencilFunction;    // The function to perform when doing stencil testing, determining whether a fragment passes or fails.
        StencilOp                   StencilPassOp;      // The operation to perform on the stencil buffer if a fragment passes the stencil test.
        StencilOp                   StencilFailOp;      // The operation to perform on the stencil buffer if a fragment fails the stencil test.
        StencilOp                   StencilZFailOp;     // The operation to perform on the stencil buffer if a fragment passes stencil test, but fails depth testing.
    };

    struct HALBlendModeDescriptor
    {
        BlendOp         Operator;
        BlendFactor     SourceColor;
        BlendFactor     DestColor;
        BlendOp         AlphaOperator;
        BlendFactor     SourceAlpha;
        BlendFactor     DestAlpha;
    };

    struct HALBlendState
    {
        HALBlendState() : Mode(Blend_None), SourceAc(false), ForceAc(false), BlendEnable(false) { }
        BlendMode Mode;
        bool      SourceAc;
        bool      ForceAc;
        bool      BlendEnable;
    };

    struct RenderTargetEntry
    {
        Ptr<RenderTarget>           pRenderTarget;              // The render target itself.
        MatrixState                 OldMatrixState;
        Rect<int>                   OldViewRect;
        Viewport                    OldViewport;
        unsigned                    PushFlags;                  // Flags used when the RenderTarget was pushed on the stack.
        bool                        StencilChecked;             // true if the depth/stencil capabilities have been checked for this target.
        bool                        StencilAvailable;           // true if there is a stencil buffer available.
        bool                        MultiBitStencil;            // true if the available stencil buffer has more than 1 bit available.
        bool                        DepthBufferAvailable;       // true if the depth buffer is available.
    };

    // BlendModeStack (holds BlendPrimitives).
    struct BlendModeStackEntry
    {
        Ptr<BlendPrimitive>     pPrimitive;         // The blend primitive, at this level of the blend-stack.
        Ptr<RenderTarget>       pRenderTarget;      // The render target, holding the actual content
        Ptr<RenderTarget>       pLayerAlpha;        // Layer alpha channel. Note: this target contains the layer alpha, 
                                                    // which is different from the alpha channel of pRenderTarget.
        bool                    LayerAlphaCleared;  // true if the pLayerAlpha member has been cleared, false otherwise.
        bool                    NoLayerParent;      // true if this is an Alpha or Erase blend mode, but there was no layer parent.
    };

    struct MaskStackEntry
    {
        Ptr<MaskPrimitive> pPrimitive;
        bool               OldViewportValid;
        Rect<int>          OldViewRect;
    };

    struct FilterStackEntry
    {
        Ptr<FilterPrimitive> pPrimitive;
        Ptr<RenderTarget>    pRenderTarget;
    };

    // Stack and list typedefs.
    typedef ArrayConstPolicy<0, 8, true> NeverShrinkPolicy;
    typedef ListAllocLH_POD<BeginDisplayData> BeginDisplayDataType;
    typedef ArrayLH<Matrix3F, Stat_Default_Mem, NeverShrinkPolicy> ViewMatrix3DStackType;
    typedef ArrayLH<Matrix4F, Stat_Default_Mem, NeverShrinkPolicy> ProjectionMatrix3DStackType;
    typedef ArrayLH<BlendModeStackEntry, Stat_Default_Mem, NeverShrinkPolicy> BlendStackType;
    typedef ArrayLH<MaskStackEntry, Stat_Default_Mem, NeverShrinkPolicy> MaskStackType;
    typedef ArrayLH<RenderTargetEntry, Stat_Default_Mem, NeverShrinkPolicy> RenderTargetStackType;
    typedef ArrayLH<FilterStackEntry, Stat_Default_Mem, NeverShrinkPolicy> FilterStackType;
    typedef ArrayLH<const UserDataState::Data*, Stat_Default_Mem, NeverShrinkPolicy> UserDataStackType;

    HAL*                getThis() { return this; }
    void                notifyHandlers(HALNotifyType type);
    void                checkState_EmitWarnings(unsigned stateFlags, const char* funcName);
    bool                checkState(unsigned stateFlags, const char* funcName);

    // *** Private API.
    virtual void        beginDisplay(BeginDisplayData* data);
    virtual void        endDisplay();

    virtual void        updateViewport() = 0;
    virtual void        calcHWViewMatrix(unsigned vpFlags, Matrix* pmatrix, const Rect<int>& viewRect, int dx, int dy);

    virtual bool        createDefaultRenderBuffer();
    virtual void        destroyDefaultRenderBuffer();
    virtual void        setRenderTargetImpl(RenderTargetData* data, unsigned flags, const Color &clearColor) = 0; 

    BlendMode           getLastBlendModeOrDefault() const;
    void                applyBlendMode(const HALBlendState& state);
    virtual void        applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false) = 0;
    virtual void        applyBlendModeEnable(bool enabled);
    virtual void        applyBlendModeEnableImpl(bool enabled) { SF_UNUSED(enabled); };

    virtual void        profilerApplyUniform(ProfilerUniform uniform, unsigned components, float* values) { SF_UNUSED3(uniform, components, values); }
    virtual void        profilerDrawCacheablePrimArea(const CacheablePrimitive* prim)                     { SF_UNUSED(prim); }

    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef) = 0;
    virtual bool        checkDepthStencilBufferCaps() = 0;
    virtual void        drawMaskClearRectangles(const HMatrix* matrices, UPInt count);
    virtual void        drawMaskClearRectangles(const Matrix2F* matrices, UPInt count) = 0;
    virtual void        beginMaskDisplay();
    virtual void        endMaskDisplay();

    virtual void        drawUncachedFilter(const FilterStackEntry&);
    virtual void        drawCachedFilter(FilterPrimitive*);
    virtual void        drawScreenQuad() = 0;

    void                applyRasterMode(RasterModeType mode);
    virtual void        applyRasterModeImpl(RasterModeType mode) = 0;

    virtual void        entryChanges(Context& context, Context::ChangeBuffer& cb, bool forceUpdateImages = false);
    virtual void        entryDestroy(Context::Entry* p);
    virtual void        entryFlush(Context::Entry* p);
    void                addComplexMeshToUpdateList(ComplexMesh::UpdateNode& meshNode);
    void                updateComplexMeshes();

    // *** Private Data Members
    MemoryHeap*                         pHeap;                      // Heap that the HALContext allocated, and in which it allocates memory for child data.
    ThreadId                            RenderThreadID;             // Thread identifier for the thread that holds the graphics device.
    unsigned                            ConfigFlags;                // HALConfigFlags (see HALConfigFlagsType).
    volatile unsigned                   HALState;                   // See HALStateFlags above.
    DisplayPass                         CurrentPass;                // Current display pass.

    ToleranceParams                     ToleranceParamsStore;       // Holds current values of ToleranceParams. Used to buffer settings before InitHAL.
    MeshCacheParams                     MeshCacheParamsStore;       // Holds current values of MeshCacheParams. Used to buffer settings before InitHAL.
    GlyphCacheParams                    GlyphCacheParamsStore;      // Holds current values of GlyphCacheParams. Used to buffer settings before InitHAL.

    List<ComplexMesh::UpdateNode>       ComplexMeshUpdateList;      // List of ComplexMeshes that need to be updated (for example, if ForceUpdateImages is called).
    List<TreeCacheNode>                 RenderRoots;                // List of all created TreeCacheRoot nodes.
    List<HALNotify>                     NotifyList;                 // List of clients to be notified when HAL events occur (see HALNotifyType).

    BlendStackType                      BlendModeStack;             // Holds HALBlendState, managed by Push/PopBlendMode. NOTE: ApplyBlendMode does not affect this stack.
    FilterStackType                     FilterStack;                // The stack of filters, managed by Push/PopFilters.
    MaskStackType                       MaskStack;                  // The stack of masks, managed by PushMask_BeginSubmit/PopMask.
    ProjectionMatrix3DStackType         ProjectionMatrix3DStack;    // The stack of 3D projection matrices, managed by Push/PopProj3D.
    RenderTargetStackType               RenderTargetStack;          // The stack of render targets, managed by Push/PopRenderTarget and SetRenderTarget.
    UserDataStackType                   UserDataStack;              // Stack of UserDataState::Data objects, managed by Push/PopUserData
    ViewMatrix3DStackType               ViewMatrix3DStack;          // The stack of 3D view matrices, managed by Push/PopProj3D.

    Ptr<RenderTarget>     				FullSceneBlendTarget;		// Blend Target containing the entire scene (NULL, unless a top-level target blend exists).
    DepthStencilMode                    CurrentDepthStencilState;   // The current values of the depth stencil 
    unsigned                            CurrentStencilRef;          // The current value of the stencil ref.
    RasterModeType                      NextSceneRasterMode;        // The raster mode to be used for the next scene (applied in BeginScene)
    RasterModeType                      CurrentSceneRasterMode;     // The raster mode for the current scene
    RasterModeType                      AppliedSceneRasterMode;     // The currently applied raster mode (may differ from the current raster mode).
    HALBlendState                       CurrentBlendState;          // The current state of alpha blending. Used for redundancy checking.
    UInt32                              FillFlags;                  // Flags applicable to the current fill being rendered (see PrimitiveFillFlags)
    
    ThreadCommandQueue*                 pRTCommandQueue;            // Controls access to providing rendering commands to the render thread.
    PrimitiveFillManager                FillManager;                // Manages primitive fills.
    Ptr<MeshKeyManager>                 pMeshKeyManager;            // Meshkey manager, organization for skipping generation of already-tesselated meshes.
    Ptr<GlyphCache>                     pGlyphCache;                // The Glyph Cache, handles rasterized text glyphs.
    MatrixStateFactory*                 pMatrixFactory;             // The object the controls creating MatrixStates.
    Ptr<MatrixState>                    Matrices;                   // The current state of the matrices.
    Ptr<RenderBufferManager>            pRenderBufferManager;       // Manages allocations of render targets.
    Ptr<Render::MemoryManager>          pMemoryManager;             // Handles allocation of video memory, if directly allocatable (varies per platform, may be NULL).
    Stats                               AccumulatedStats;           // Stats (see GetStats).
    ProfileViews                        Profiler;                   // Modifies rendering output for special data views (eg. batch and overdraw modes).

    MeshGenerator                       MeshGen;                    // Mesh generator (the fill tessellator).
    StrokeGenerator                     StrokeGen;                  // Stroke generator (the stroke tessellator).
    MatrixPool                          MPool;                      // Pool of matrices.

    RenderQueue                         Queue;                      // Queue of RenderQueueItems.
    RenderQueueProcessor                QueueProcessor;             // Used to process the RenderQueue.
    BeginDisplayDataType                BeginDisplayDataList;       // Holds (queued) data passed to BeginScene.

    unsigned                            MaskStackTop;               // Active stack top; 0 for empty stack. We track this separately from stack size to allow PopMask optimization.
    int                                 CacheableIndex;             // Holds the level of cached filter on the FilterStack being drawn (-1 for none).
    int                                 CacheablePrepIndex;         // Holds the level of cacheable primitive being prepared (-1 for none).
    int 								CacheablePrepStart;     	// Holds the level at which cacheable primitives started to be prepared (-1 for none)
    Viewport                            VP;                         // Output size.
    Rect<int>                           ViewRect;                   // "Viewport" on the content, in screen space.
    Matrix2F                            ViewportMatrix;             // The viewport matrix, determining how the content is scaled/translated on screen.

    // *** Static Private Data Members
    static HALDepthStencilDescriptor    DepthStencilModeTable[DepthStencil_Count];
    static HALBlendModeDescriptor       BlendModeTable[Blend_Count];
};


}} // Scaleform::Render

#endif
