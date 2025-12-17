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
#include "Render/Renderer2D.h"

namespace Scaleform { namespace Render {

typedef Matrix2F Matrix;
class RenderQueueProcessor;


// 3D stereo support
enum StereoDisplay
{
    StereoCenter,       // the non-stereo case
    StereoLeft,         // use the left eye perspective
    StereoRight         // use the right eye perspective
};

struct StereoParams
{
    float DisplayWidthCm;           // the width of the display, computed internally
    float Distortion;               // 0 to 1, the strength of the stereo effect, defaults to 0.75
    float DisplayDiagInches;        // the diagonal size of the output display in inches, defaults to 52
    float DisplayAspectRatio;  
    float EyeSeparationCm;          // the distance between eyes in cm, defaults to 6.4
    StereoParams() : DisplayWidthCm(0), Distortion(0.75f), DisplayDiagInches(52.f), DisplayAspectRatio(9.f/16.f), EyeSeparationCm(6.4f) {  }
};

class StereoImplBase : public RefCountBase<StereoImplBase, Stat_Default_Mem>
{
protected:
    StereoParams S3DParams;

public:
    virtual void SetParams(const StereoParams& p) { S3DParams = p; }
    virtual const StereoParams& GetParams() { return S3DParams; }
    virtual void GetStereoProj(const Matrix4F& proj, float eyeZ, Matrix4F* left, Matrix4F* right, float factor = 1.0f) const;
};
 
class MatrixState : public RefCountBase<MatrixState, StatRender_Mem>
{
public:
    Matrix2F            View2D;
    Matrix3F            View3D;
    Matrix4F            Proj3D;
    mutable Matrix4F    Proj3DLeft;        // 3D stereo support
    mutable Matrix4F    Proj3DRight;       // 3D stereo support

    Matrix2F            User;
    Matrix2F            User3D;
    Matrix2F            Orient2D;
    Matrix4F            Orient3D;

    Rect<int>           ViewRectOriginal;       // 'full screen' view rectangle.
    mutable Rect<int>   ViewRect;               // Current viewport rectangle.

    mutable Matrix2F    UserView;               // 'Final' concattenated User x View x Orientation matrix (for 2D shapes).
    mutable Matrix4F    UVPO;                   // 'Final' concattenated User x View x Projection x Orientation matrix (for 3D shapes).

    mutable Matrix4F    ViewRectCompensated3D;  // Matrix compensating for cullrect changes (reduced sized viewports).

    mutable bool        UVPOChanged;
    bool                OrientationSet;

    // 3D stereo support
    StereoDisplay       S3DDisplay; 
    Ptr<StereoImplBase> S3DImpl;

    HAL*                pHAL;

    MatrixState(HAL* phal);
    MatrixState();

    const Matrix4F&     GetUVP() const;
    const Matrix4F&     GetUVP( const Rect<int> & viewRect ) const;
    virtual Matrix2F&   GetFullViewportMatrix(const Size<int>& rtSize);

    virtual void        SetUserMatrix(const Matrix2F& user);
    virtual void        SetUserMatrix(const Matrix2F& user, const Matrix2F& user3D);
    virtual void        SetViewportMatrix(const Matrix2F& vp);
    virtual Viewport    SetOrientation(const Viewport& vp);
    virtual void        CopyFrom(MatrixState* state);
    virtual void        CopyTo(MatrixState* state);

    virtual void        Copy(MatrixState* outmat, MatrixState* inmat);
    
protected:
    virtual void        recalculateUVPOC() const;
    
    // 3D stereo support
    const               Matrix4F& updateStereoProjection(float factor = 1.0f) const;

    Matrix2F            FullViewportMVP;        // MVP for a 2D quad to fill the entire viewport.
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
// as video mode changes. Overridden by Renderer2DImpl.
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
    // RenderThreadId identifies the renderer thread; it is used to identify which thread
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
    BeginDisplayData( const Color & bgcol, const Viewport & vpin) :
    BackgroundColor(bgcol), VP(vpin) {} 
    Color       BackgroundColor;
    Viewport    VP;
};

//--------------------------------------------------------------------
// HAL - Abstract renderer interface (aka. Hardware Abstraction Layer)

class HAL : public RefCountBase<HAL, StatRender_Mem>
{
public:

    // HAL tracks a command queue that allows its associated systems to post commands for the render thread.
    HAL(ThreadCommandQueue *commandQueue);

    struct Stats
    {
        unsigned Primitives;    // Number of actual graphics API 'draw' calls performed (eg. DrawIndexedPrimitive)
        unsigned Meshes;        // Number of meshes drawn (including the number of instances for instanced draws)
        unsigned Triangles;     // Number of triangles rendered in all draw calls.
        unsigned Masks;         // Number of masks rendered.
        unsigned RTChanges;     // Number of internal render target changes needed; does not count HAL::SetRenderTarget itself
        unsigned Filters;       // Number of filters rendered, does not include cached filter rendering.

        Stats() : Primitives(0), Meshes(0), Triangles(0), Masks(0), RTChanges(0), Filters(0) { }

        void Clear() { Primitives = Meshes = Triangles = Masks = RTChanges = Filters = 0; }
    };


protected:

    // initHAL - a platform-specific function provided in every back end. Must be 
    // called before initialization. The base class implementation does very basic 
    // initialization.
    virtual bool initHAL(const HALInitParams& params);

    // initHAL - initializes the HAL's matrix state. Normally just the base class
    // implementation is fine, but on systems that support stereo 3D (3DS) or that have
    // non-standard orientation in render targets (Wii, OpenGL), the derived version
    // must be allocated.
    virtual void initMatrices();
    
    // Platform-specific function called to shutdown an initialized HAL;
    // implemented in every back-end.
    virtual bool shutdownHAL();

public:


    // Returns true if HAL has been initialized through a call to InitHAL.
    virtual bool        IsInitialized() const { return (HALState & HS_ModeSet) != 0; }


    // ***** Rendering

    // Begins rendering frame, which is a caching unit. BeginFrame/EndFrame pair
    // must always be called around BeginScene/EndScene.
    virtual bool        BeginFrame();
    virtual void        EndFrame();
    virtual void        FinishFrame();

    // Flush performs all queued rendering. This must be called for any rendering results
    // to be displayed (generally before Present is called). It is called automatically from
    // EndScene, usually an explicit call is not required. This allows for multiple 
    // BeginDisplay/EndDisplay to be queued between calls to BeginScene/EndScene.
    virtual void        Flush();

    // BeginScene begins scene rendering, initializing various render states needed
    // externally; EndScene completes scene rendering. In Direct3D, this will call
    // BeginScene/EndScene flags on the device if the 
    // If not called explicitly, these functions will be automatically called from
    // BeginDisplay and EndDisplay, explicitly. Calling them externally is more
    // efficient if multiple BeginDisplay/EndDisplay blocks will take place, as it
    // optimizes state changes and eliminates queue flush.
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Bracket rendering of a display unit that potentially has its own
    // viewport, such Render::TreeNode (GFx::MovieView).
    // Fill the background color, and set up default transforms, etc.
    void                BeginDisplay(Color backgroundColor,
                                     const Viewport& viewport);
    void                EndDisplay();
    virtual void        beginDisplay(BeginDisplayData* data);
    virtual void        endDisplay();

    // Set "User" matrix that is applied to shift the view just before viewport.
    // Should be called before BeginDisplay.
    virtual void        SetUserMatrix(const Matrix& m) { Matrices->SetUserMatrix(m); }
    virtual void        SetUserMatrix(const Matrix2F& user, const Matrix2F& user3D) { Matrices->SetUserMatrix(user, user3D); }

    // Caclulates the 2D view/projection matrix, given a view rectangle, and a window offset.
    virtual void        CalcHWViewMatrix(unsigned vpFlags, Matrix* pmatrix, const Rect<int>& viewRect, int dx, int dy);

    // Get the matrix used to adjust the viewport. The inverse of this is useful for translating
    // input mouse/touch position.
    virtual Matrix2F    GetOrientationMatrix() const { return Matrices->Orient2D; }

    // Updates the hardware viewport based on current settings of HALState and VP/ViewRect.
    virtual void        updateViewport() = 0;

    // Clears a rectangle (in screen coordinates) to the given color. If blend is true, the rectangle is not
    // actually cleared, it is 'normal' blended over the current buffer contents, with the alpha in the 'color'.
    virtual void        clearSolidRectangle(const Rect<int>& r, Color color, bool blend) = 0;


    // *** Render target state management

    // Render Buffer Interfaces
    virtual RenderBufferManager* GetRenderBufferManager() const { return pRenderBufferManager; };

    // Returns render target that was configured on the device at the time of InitHAL call.
    // The returned RenderTarget should have the RBuffer_Default type.
    virtual RenderTarget* GetDefaultRenderTarget();

    // The returned RenderTarget should have the RBuffer_Texture type.
    virtual RenderTarget* CreateRenderTarget(Texture* texture, bool needsStencil) = 0;

    // Target_Temporary, created through delegation to RenderBufferManager.
    virtual RenderTarget* CreateTempRenderTarget(const ImageSize& size, bool needsStencil) = 0;

    // *** Render target state management

    // Flags available to the PushRenderTarget method (defaults to none).
    enum PushRenderTargetFlags
    {
        PRT_NoClear     = 0x01,     // Do not clear the render target being pushed (clear is done by default).
        PRT_Resolve     = 0x02,     // Resolve the current backbuffer to a texture before rendering (X360).
        PRT_NoSet       = 0x04,     // Do not actually set the render target on the device (optimization for filter drawing)
    };

    // Applies render target; should be called before BeginDisplay, If called successfully 
    // before InitHAL, the HAL will not attempt to create a default render target by querying 
    // the target API during InitHAL (when the platform allows this). 
    virtual bool    SetRenderTarget(RenderTarget* target, bool setState = 1) = 0;

    // Begin rendering to the specified target; frameRect is the ortho projection.
    // Texture referenced by prt must not be used until PopRenderTarget. Flags are available,
    // see PushRenderTargetFlags. If the PRT_NoClear flag is not set, the target will be cleared to 'clearColor'.
    virtual void    PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags = 0, Color clearColor = 0) = 0;

    // Restore previous render target. Contents of Texture in popped render target are now available
    // for rendering. Flags are available, see PushRenderTargetFlags.
    virtual void    PopRenderTarget(unsigned flags = 0) = 0;

    // Creates the default RenderTarget, on platforms that can query the current render surface(s) from
    // the device directly. On platforms where this is not possible, an explicit call to SetRenderTarget
    // is also required. Also initializes the RenderBufferManager. Returns true if successful.
    virtual bool    createDefaultRenderBuffer() = 0;

    // Destroys the default render targets.
    virtual void    destroyRenderBuffers();

    // Allocates a PrimitiveFill initialized with the given data.
    virtual PrimitiveFill*  CreatePrimitiveFill(const PrimitiveFillData& data);    

    // Most platforms do not require the prepass step (except X360/Wii), their HALs override this function. 
    // Otherwise, if both passes are requested at once, (and prepass is not required), the prepass rendering 
    // is done inline with main display, instead of consecutively. On platforms that do not require prepasses,
    // one can still be acheived by calling Renderer2D::Display multiple times with individual pass arguments.
    virtual bool        IsPrepassRequired() const { return false; }
    virtual void        SetDisplayPass(DisplayPass pass) { CurrentPass = pass; }
    virtual DisplayPass GetDisplayPass() const { return CurrentPass; }
    virtual void        DrawBundleEntries(BundleIterator ibundles, Renderer2DImpl* r2d);

    // Draw a DP where every mesh element is transformed by the specified matrix.
    // Note: matrixCount isn't technically necessary.
    virtual void        Draw(const RenderQueueItem& item);
    
    inline  void        Draw(RenderQueueItem::Interface* i, void* data = 0)
    {
        Draw(RenderQueueItem(i, data));
    }
    
    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend) = 0;
    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix> &matrices) = 0;

    // Mask support is implemented as a stack, with three possible operations doing
    // the following:
    //  PushMask_BeginSubmit - pushes a new mask on stack and begins "submit mask" rendering.
    //  EndMaskSubmit - Ends submit mask and begins content rendering, clipped by the mask.
    //  PopMask - pops the mask from stack; further rendering will use previous masks, if any.
    virtual void        PushMask_BeginSubmit(MaskPrimitive* primitive);
    virtual void        EndMaskSubmit();
    virtual void        PopMask();

    // Used with render states which can be enabled and disabled, but may also be ignored.
    enum EnableIgnoreValue
    {
        EnableIgnore_Off    = 0,
        EnableIgnore_On     = 1,
        EnableIgnore_Ignore = 2
    };

    // *** BlendMode
    enum BlendOp
    {
        BlendOp_ADD,
        BlendOp_MAX,
        BlendOp_MIN,
        BlendOp_REVSUBTRACT,
        BlendOp_Count
    };
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
    struct BlendModeDescriptor
    {
        BlendOp         Operator;
        BlendFactor     SourceColor;
        BlendFactor     DestColor;
        BlendOp         AlphaOperator;
        BlendFactor     SourceAlpha;
        BlendFactor     DestAlpha;
    };
    static BlendModeDescriptor BlendModeTable[Blend_Count];

    struct HALBlendState
    {
        HALBlendState() : Mode(Blend_None), SourceAc(false), ForceAc(false), BlendEnable(false) { }
        BlendMode Mode;
        bool      SourceAc;
        bool      ForceAc;
        bool      BlendEnable;
    };

    virtual void          PushBlendMode(BlendPrimitive* prim);
    virtual void          PopBlendMode();
    void                  SetFullSceneTargetBlend(bool enable);

    void                  applyBlendMode(BlendMode mode, bool sourceAc = false, bool forceAc = false);
    void                  applyBlendMode(const HALBlendState& state);
    virtual void          applyBlendModeEnable(bool enabled);

protected:
    BlendMode             getLastBlendModeOrDefault() const;
    virtual void          applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false) = 0;
    virtual void          applyBlendModeEnableImpl(bool enabled) { SF_UNUSED(enabled); };
    virtual bool          shouldRenderTargetBlend(const BlendPrimitive*) const { return true; }

    Ptr<RenderTarget>     FullSceneBlendTarget;

public:
    // *** DepthStencil modes

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

    // *** Rasterization
    enum RasterModeType
    {
        RasterMode_Solid,       // Primitives are solid.
        RasterMode_Wireframe,   // Primitives are rendered in wireframe.
        RasterMode_Point,       // Primitives are rendered as points.
        RasterMode_Count,
        RasterMode_Default = RasterMode_Solid
    };

    // Sets the raster mode to be applied to the next scene. 
    void SetRasterMode(RasterModeType mode)                         { NextSceneRasterMode = mode; }
    // Returns whether this platform supports the given raster mode.
    virtual bool IsRasterModeSupported(RasterModeType mode) const   { SF_UNUSED(mode); return true; }

public:

    // *** Cacheable
    virtual void        PrepareCacheable(CacheablePrimitive*, bool unprepare);

    // *** Filters
    struct FilterStackEntry;
    virtual void        PushFilters(FilterPrimitive*);
    virtual void        PopFilters();
    virtual void        drawUncachedFilter(const FilterStackEntry&)  { };
    virtual void        drawCachedFilter(FilterPrimitive*)           { };

    // *** DrawableImage

    // Retrieves the capabilities of this HAL to perform the DICommand wrt the GPU (See DICommand::RenderCaps).
    // The default implementation reports the most common HAL implementations capabilities.
    virtual unsigned    DrawableCommandGetFlags(const DICommand* pcmd) const;

    // DrawableImage hardware commands
    virtual void        DrawableCxform( Texture** tex, const Matrix2F* texgen, const Cxform* cx) 
                        { SF_UNUSED3( tex, texgen, cx ); }
    virtual void        DrawableCompare( Texture** tex, const Matrix2F* texgen) 
                        { SF_UNUSED2( tex, texgen ); }
    virtual void        DrawableCopyChannel( Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul ) 
                        { SF_UNUSED3( tex, cxmul, texgen ); }
    virtual void        DrawableMerge( Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul ) 
                        { SF_UNUSED3( tex, cxmul, texgen ); }
    virtual void        DrawableCopyPixels( Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                            bool mergeAlpha, bool destAlpha ) 
                        { SF_UNUSED5( tex, texgen, mvp, mergeAlpha, destAlpha ); }
    virtual void        DrawablePaletteMap( Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                            unsigned channelMask, const UInt32* values) 
                        { SF_UNUSED5( tex, texgen, mvp, channelMask, values ); }
    virtual void        DrawableCopyback( Render::Texture* tex, const Matrix2F& mvp, const Matrix2F& texgen, unsigned flagMask = 0xFFFFFFFF ) 
                        { SF_UNUSED4(tex, mvp, texgen, flagMask); }

    virtual void        GetStats(Stats* pstats, bool clear = true);

    // *** Events

    // Generic rendering event API - default implementation does nothing.
    // Override this in a HAL to provide overloaded event types.
    virtual RenderEvent& GetEvent(EventType);

    ProfileViews& GetProfiler()
    {
        return Profiler;
    }

    // Texture manager
    virtual TextureManager*           GetTextureManager() const = 0;
    virtual class MeshCache&          GetMeshCache() = 0;
    virtual RQCacheInterface&         GetRQCacheInterface() { return QueueProcessor.GetQueueCachesRef(); }
    virtual RenderQueueProcessor&     GetRQProcessor()      { return QueueProcessor; }

    // Returns the RenderSync object, used for synchronizing the CPU and GPU. Not all platforms require
    // this object, it is valid for this function to return 0.
    virtual RenderSync*               GetRenderSync() { return 0; }

    // Obtains formats that renderer will use for single, batches and instanced rendering of
    // the specified source format.
    //   - Filled in pointer may be the same as sourceFormat.
    //   - 'batch' format may be reported as 0, in which case batching is disabled.
    //   - 'instanced' format may be reported as 0, in which instancing is not supported for format.
    virtual void     MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                     const VertexFormat** single,
                                     const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType) = 0;

    // Returns the maximum number of batches, based on the number of available uniforms, and the number required per-batch
    // (on shader-platforms). The MeshCacheParams can lower the return value of this function further.
    // If the user modifies the shader type, during the emit step (eg. with the PushUserData API), and this changes uniform usage,
    // they must account for this, to return the minimum possible batch count, based on any possible modification.
    virtual unsigned GetMaximumBatchCount(Primitive* prim) = 0;
                                     
    // Adds an external notification object that will be told about HAL
    // events such as SetVideoMode/ResetVideoMode. Should be removed with RemoveNotify.
    void AddNotify(HALNotify *notify)
    {
        SF_ASSERT(notify->pPrev == 0);
        NotifyList.PushBack(notify);
    }
    void RemoveNotify(HALNotify *notify)
    {
        notify->RemoveNode();
    }    

     // 3D stereo support
    virtual void SetStereoImpl(StereoImplBase* simpl)
    {
        Matrices->S3DImpl = simpl;
    }
    
    // 3D stereo support
    virtual void SetStereoParams(StereoParams sParams)  
    { 
        if (sParams.DisplayWidthCm == 0)
        {
            sParams.DisplayWidthCm = sParams.DisplayDiagInches / sqrtf(1.0f + 1.f/sParams.DisplayAspectRatio *
            1.f/sParams.DisplayAspectRatio) * 2.54f /* inches to cm */; 
        }
        if (Matrices->S3DImpl)
        {
            Matrices->S3DImpl->SetParams(sParams);
        }
    }

    virtual void SetStereoDisplay(StereoDisplay sDisplay, bool setstate = 0) 
    { Matrices->S3DDisplay = sDisplay;  Matrices->UVPOChanged = 1; SF_UNUSED(setstate); }

    // Pushes a 3D view matrix onto the stack when rendering.  Generally used along with 3D perspective.
    void PushView3D(const Matrix3F &m);
    // Pushes a 3D projection matrix onto the stack when rendering.  Generally used for 3D perspective.
    void PushProj3D(const Matrix4F &m);
    // Restore previous 3D View matrix.
    void PopView3D();
    // Restore previous 3D projection matrix.
    void PopProj3D();

    void SetFullViewRect(const Rect<int> &r) { Matrices->ViewRectOriginal = r; Matrices->UVPOChanged = 1; }

    const Matrix3F &GetView3D() const { return Matrices->View3D; }
    const Matrix4F &GetProj3D() const { return Matrices->Proj3D; }

    // Most platforms require a negative scale when mapping a quad to the screen. Others (GL based)
    // which do not should override this function and return 1.0f.
    virtual float GetViewportScaling() const { return -1.0f; }

    MatrixState* GetMatrices() const { return Matrices; }
    unsigned GetHALState() const { return HALState; }

    UInt32 GetConfigFlags() const { return VMCFlags; }
    ThreadId GetRenderThreadId() const { return RenderThreadID; }

    // Returns render thread command queue, if any, used to post commands that must be
    // executed by the render thread.
    ThreadCommandQueue* GetThreadCommandQueue() const { return pRTCommandQueue; }

    // *** UserData. The following Push/Pop user data set in the render tree into the HAL.
    // The default implementation does nothing with this data, except store it. The expectation 
    // is that users will use the data contained within the stack, to affect the fills in some way.
    virtual void PushUserData(const UserDataState::Data* data);
    virtual void PopUserData();

    // HAL State flags are checked for during most API calls to ensure that
    // state is valid for the operation. Check is done by checkState(bits),
    // which will emit error message for some cases. Not all of these are applicable
    // on all platforms.
    enum HALStateFlags
    {
        HS_ModeSet          = 0x00001,  // InitHAL has been called.
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
        HS_BlendTarget      = 0x00800,  // Set if a fullscreen target is required to perform an BlendMode requiring a target.
        HS_InCachedTarget   = HS_InCachedFilter | HS_InCachedBlend,  // Set when a filter or target blend from a cached result.

        HS_DeviceValid      = 0x01000,  // If not valid, device is Lost (3DS/D3D9 only).
        HS_ReadyForReset    = 0x02000,  // Set when reset-dependent resources are released (D3D9/D3D1x only).
        HS_InGxmScene       = 0x10000,  // Currently in a gxm scene (PSVITA only).
        HS_SceneChanged     = 0x20000,  // The gxm scene was changed after BeginDisplay (PSVITA only).
    };

protected:

    void notifyHandlers(HALNotifyType type)
    {
        HALNotify *p = NotifyList.GetFirst();
        while (!NotifyList.IsNull(p))
        {
            // Allow item itself to be removed during notification.
            HALNotify *next = p->pNext;
            p->OnHALEvent(type);
            p = next;
        }
    }

    // Outputs debug warnings for missing states.
    void        checkState_EmitWarnings(unsigned stateFlags, const char* funcName);

    // Checks that the required HALState bit flags are present; called in front of
    // various rendering calls.
    inline bool checkState(unsigned stateFlags, const char* funcName)
    {
        if ((HALState & stateFlags) != stateFlags)
        {
            checkState_EmitWarnings(stateFlags, funcName);
            return false;
        }
        return true;
    }

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:
    // Returns whether the profile can render any of the filters contained in the FilterPrimitive
    // By default, always return true. Platforms can override this if they have conditional support
    // for filters.
    virtual bool shouldRenderFilters(const FilterPrimitive*) const { return true; }


protected:

    // Simply sets a quad vertex buffer and draws (uniforms, etc, need to be set already).
    virtual void drawScreenQuad() = 0;

    // *** Profiler
    friend class ProfileModifierTDensity;
    virtual void profilerApplyUniform(ProfilerUniform uniform, unsigned components, float* values) { SF_UNUSED3(uniform, components, values); }
    virtual void profilerDrawCacheablePrimArea(const CacheablePrimitive* prim)                     { SF_UNUSED(prim); }


    unsigned                 HALState;       // See HALStateFlags above
    DisplayPass              CurrentPass;    // Current display pass.
    List<HALNotify>          NotifyList;
    Ptr<MatrixState>         Matrices;
    ProfileViews             Profiler;
    Stats                    AccumulatedStats;

    
    UInt32                   VMCFlags;      // Video Mode Configuration Flags (HALConfigFlags - varies per platform)
    UInt32                   FillFlags;     // Flags applicable to the current fill being rendered (see PrimitiveFillFlags)

    // ThreadId of the render thread (may be 0, or equal to the main thread's ID in single threaded mode).
    ThreadId                 RenderThreadID;    

    MemoryHeap*              pHeap;

    // Queue used for posting commands to the rendering thread; used by D3D9 TextureManagers
    // and for context shutdown notify.
    ThreadCommandQueue*      pRTCommandQueue;

    RenderQueue              Queue;

    Ptr<RenderBufferManager> pRenderBufferManager;
    RenderQueueProcessor     QueueProcessor;

    // stack of 3D View and Projection matrices
    typedef ArrayConstPolicy<0, 8, true> NeverShrinkPolicy;
    typedef ArrayLH<Matrix3F, Stat_Default_Mem, NeverShrinkPolicy> ViewMatrix3DStackType;
    ViewMatrix3DStackType ViewMatrix3DStack;

    typedef ArrayLH<Matrix4F, Stat_Default_Mem, NeverShrinkPolicy> ProjectionMatrix3DStackType;
    ProjectionMatrix3DStackType ProjectionMatrix3DStack;

    // The current state of alpha blending. Used for redundancy checking.
    HALBlendState CurrentBlendState;

    // *** DepthStencil modes
    static HALDepthStencilDescriptor DepthStencilModeTable[DepthStencil_Count];

    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef) = 0;
    virtual bool        checkDepthStencilBufferCaps() = 0;
    virtual void        drawMaskClearRectangles(const HMatrix* matrices, UPInt count);
    virtual void        drawMaskClearRectangles(const Matrix2F* matrices, UPInt count) = 0;
    virtual void        beginMaskDisplay();
    virtual void        endMaskDisplay();

    DepthStencilMode            CurrentDepthStencilState;   // The current values of the depth stencil 
    unsigned                    CurrentStencilRef;          // The current value of the stencil ref.
    bool                        StencilChecked;             // true if the depth/stencil capabilities have been checked this frame.
    bool                        StencilAvailable;           // true if there is a stencil buffer available.
    bool                        MultiBitStencil;            // true if the available stencil buffer has more than 1 bit available.
    bool                        DepthBufferAvailable;       // true if the depth buffer is available.

    // *** Rasterization 

    // Applies the given raster mode, with redundancy checking.
    void applyRasterMode(RasterModeType mode);
    // Platform specific method which unconditionally applies the given raster mode.
    virtual void applyRasterModeImpl(RasterModeType mode) = 0;

    RasterModeType NextSceneRasterMode;     // The raster mode to be used for the next scene (applied in BeginScene)
    RasterModeType CurrentSceneRasterMode;  // The raster mode for the current scene
    RasterModeType AppliedSceneRasterMode;  // The currently applied raster mode (may differ from the current raster mode).

public:

    // BlendModeStack (holds BlendPrimitives).
    struct BlendStackEntry
    {
        Ptr<BlendPrimitive>     pPrimitive;         // The blend primitive, at this level of the blend-stack.
        Ptr<RenderTarget>       pRenderTarget;      // The render target, holding the actual content
        Ptr<RenderTarget>       pLayerAlpha;        // Layer alpha channel. Note: this target contains the layer alpha, 
                                                    // which is different from the alpha channel of pRenderTarget.
        bool                    LayerAlphaCleared;  // true if the pLayerAlpha member has been cleared, false otherwise.
        bool                    NoLayerParent;      // true if this is an Alpha or Erase blend mode, but there was no layer parent.
    };
    typedef ArrayLH<BlendStackEntry, Stat_Default_Mem, NeverShrinkPolicy> BlendStackType;
    BlendStackType  BlendModeStack;

    // *** Mask Support   
    struct MaskStackEntry
    {
        Ptr<MaskPrimitive> pPrimitive;
        bool               OldViewportValid;
        Rect<int>          OldViewRect;
    };
    typedef ArrayLH<MaskStackEntry, Stat_Default_Mem, NeverShrinkPolicy> MaskStackType;

    MaskStackType   MaskStack;
    // Active stack top; 0 for empty stack. We track this separately from stack size
    // to allow PopMask optimization. Last item above top may be needed to 
    // erased previous mask bounds when entering a new nested mask.
    unsigned        MaskStackTop;

    // *** Render Target Support
    struct RenderTargetEntry
    {
        Ptr<RenderTarget>           pRenderTarget;
        MatrixState                 OldMatrixState;
        Rect<int>                   OldViewRect;
        Viewport                    OldViewport;
    };
    typedef ArrayLH<RenderTargetEntry, Stat_Default_Mem, NeverShrinkPolicy> RenderTargetStackType;
    RenderTargetStackType RenderTargetStack;

    // *** FilterStack (holds filter descs).
    struct FilterStackEntry
    {
        Ptr<FilterPrimitive> pPrimitive;
        Ptr<RenderTarget>    pRenderTarget;
    };
    typedef ArrayLH<FilterStackEntry, Stat_Default_Mem, NeverShrinkPolicy> FilterStackType;
    FilterStackType FilterStack;

    int CacheableIndex;         // Holds the level of cached filter on the FilterStack being drawn (-1 for none).
    int CacheablePrepIndex;     // Holds the level of cacheable primitive being prepared (-1 for none).
    int CacheablePrepStart;     // Holds the level at which cacheable primitives started to be prepared (-1 for none)

    typedef ListAllocLH_POD<BeginDisplayData> BeginDisplayDataType;
    BeginDisplayDataType BeginDisplayDataList;  // Holds data passed to BeginScene.

    typedef ArrayLH<const UserDataState::Data*, Stat_Default_Mem, NeverShrinkPolicy> UserDataStackType;
    UserDataStackType UserDataStack;    // Holds the UserDataState::Data objects passed to the HAL.

    Viewport                 VP;        // Output size.
    Rect<int>                ViewRect;    
};


}} // Scaleform::Render

#endif
