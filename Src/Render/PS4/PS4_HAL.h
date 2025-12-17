/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_HAL.h
Content     :   HAL implementation for Orbis platform.
Created     :   2012/09/19
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_HAL_H
#define INC_SF_PS4_HAL_H

#include "Render/Render_HAL.h"
#include "Render/PS4/PS4_Config.h"
#include "Render/PS4/PS4_MeshCache.h"
#include "Render/PS4/PS4_Shader.h"
#include "Render/PS4/PS4_Texture.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.

namespace Scaleform { namespace Render { namespace PS4 {

class RenderEvent;

// Orbis::HALInitParems provides Orbis specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    // Use this constructor if you are using Gnmx, and would like GFx to submit its commands
    // via the Gnmx::GfxContext in the first parameter (or LCUE::GraphicsContext if compiling with LCUE support).
    HALInitParams(sceGnmxContextType& context,
                  Render::MemoryManager* memoryManager,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : Render::HALInitParams(memoryManager, halConfigFlags, renderThreadId),
      GnmxCtx(&context), DCBSize(0), CCBSize(0), CUERingBuffers(0)
    { }

    // Use this constructor, if you are not using Gnmx (eg. using simply Gnm), or you would
    // like Gfx to generate separate DCB/CCBs. Note that you may control the allocations sizes for 
    // the DCB/CCB GFx uses. However, using sizes too small for these may result in bad performance.
    HALInitParams(  Render::MemoryManager* memoryManager,
                    UInt32 halConfigFlags = 0,
                    ThreadId renderThreadId = ThreadId(),
                    unsigned dcbSize = sce::Gnm::kIndirectBufferMaximumSizeInBytes,
                    unsigned ccbSize = sce::Gnm::kIndirectBufferMaximumSizeInBytes,
                    unsigned cueRingBuffers = 16) :
        Render::HALInitParams(memoryManager, halConfigFlags, renderThreadId),
        GnmxCtx(0), DCBSize(dcbSize), CCBSize(ccbSize), CUERingBuffers(cueRingBuffers)
    { }


    // Orbis::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }

    sceGnmxContextType*                 GnmxCtx;                // The Gnmx context used for rendering (may be NULL).
    unsigned                            DCBSize;                // Size in bytes of the DCB GFx will allocate (if GnmxCtx is NULL).
    unsigned                            CCBSize;                // Size in bytes of the CCB GFx will allocate or size of Garlic resource buffer with SF_PS4_USE_LCUE (if GnmxCtx is NULL).
    unsigned                            CUERingBuffers;         // Number of CUE ring buffers GFx will allocate, not used with SF_PS4_USE_LCUE (if GnmxCtx is NULL ).
};


//------------------------------------------------------------------------

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
    friend class ShaderInterface;
    friend class PS4::RenderEvent;
    friend class PS4::RenderSync;
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

protected:

    sceGnmxContextType*      GnmxCtx;
    sceGnmxContextType*      GnmxAllocatedContext;
    void*                    CueCpRamShadowBuffer;
    RenderSync               RSync;
    MeshCache                Cache;
    Ptr<TextureManager>      pTextureManager;
    MemoryManager*           pMemoryManager;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

    // Determines whether the Gnmx::GfxContext was allocated internally in GFx, or externally provided.
    bool      isGnmxCtxAllocatedInternally() const { return GnmxAllocatedContext != 0; }
    void      allocateGnmxContext( const PS4::HALInitParams &params );
    void      freeGnmxCtx();

public:    
    
    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();
    
    // *** HAL Initialization / Shutdown Logic

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const PS4::HALInitParams& params);

    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // *** Rendering
    virtual bool        BeginFrame();
    virtual void        EndFrame();
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Allows the modification of the GfxContext in between scenes. You may want to call this
    // to use multiple contexts in a 'double-buffering' of state changes.
    void                SetGfxContext(sceGnmxContextType* context);

    // This submits the current context's buffers and resets the states currently set within the HAL.
    // This is used when the context buffers are full, or when a sync requires a wait on queued commands.
    void                SubmitAndResetStates();

    // If you are submitting command buffers via sce::Gnm::submitCommandBuffers, this function will add the command
    // buffers GFx has generated to the lists that are input to that function. It returns the number total number of command 
    // buffers contained in the list, after the function executes. The currentIndex should point to the location (in both lists), 
    // where the command buffers should start to be added.
    unsigned            AppendCommandBuffersToList(unsigned currentIndex, unsigned listTotalSize, 
                                                   void** dcbGpuAddrs, uint32_t* dcbSizes, 
                                                   void** ccbGpuAddrs, uint32_t* ccbSizes);

    // Static version, which will append all commands in the input Gnmx::GfxContext to the given lists (this function is provided for
    // convenience, if GFx is using a Gnmx::GfxContext separate from the one in the application.
    static unsigned     AppendCommandBuffersToList(sceGnmxContextType& ctx,
                                                   unsigned currentIndex, unsigned listTotalSize, 
                                                   void** dcbGpuAddrs, uint32_t* dcbSizes, 
                                                   void** ccbGpuAddrs, uint32_t* ccbSizes);

    // Returns the number of command buffers that GFx generated (used to determine the required size of the lists input to AppendCommandBuffersToList.
    unsigned            GetCommandBufferCount() const;

    // Public method to clear, since clearSolidRectangle is protected, and there is no easy method to clear on Orbis.
    void                ClearSolidRectangle(const Rect<int>& r, Color color, bool blend) { clearSolidRectangle(r, color, blend); }

    // Updates D3D HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();

    // *** BlendMode
    void            applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);

    // *** Rasterization
    void                    applyRasterModeImpl(RasterModeType mode);

    virtual Render::TextureManager* GetTextureManager() const { return pTextureManager.GetPtr(); }

    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags =0, Color clearColor =0);
    virtual void            PopRenderTarget(unsigned flags = 0);

    virtual bool            createDefaultRenderBuffer();

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*    GetRenderSync()       { return &RSync; }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, 
                                    unsigned flags = 0)
    {
        SF_UNUSED(flags);
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced);
    }

protected:

    virtual void  setBatchUnitSquareVertexStream();
    virtual void  setBatchUnitSquareVertexStreamPerDraw();
    virtual UPInt setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    virtual void  setVertexArrayPerDraw(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual void  setVertexArrayPerDraw(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    void          setVertexArrayImpl(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    virtual void  setLinearStreamSource(PrimitiveBatch::BatchType type);
    virtual void  setInstancedStreamSource(UPInt drawCount, UPInt indicesPerMesh);

    virtual void  drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void  drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );
    virtual void  drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );
    void          drawScreenQuad();

    virtual void  drawUncachedFilter(const FilterStackEntry&);

    // *** Mask Support
    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual bool        checkDepthStencilBufferCaps();
    
    // *** Instancing
    PrimitiveBatch::BatchType   PrevBatchType; // Previous batching mode

    // *** Events
    virtual Render::RenderEvent& GetEvent(EventType eventType);
};


// If profile modes are required, use this class instead of HAL itself. If SF_RENDERER_PROFILE is not
// defined, profile modes will be disabled (by switching this class to be the HAL class).
#if defined(SF_RENDERER_PROFILE)
class ProfilerHAL : public Render::ProfilerHAL<HAL>
{
public:
    ProfilerHAL(ThreadCommandQueue* commandQueue) : Render::ProfilerHAL<HAL>(commandQueue)
    {
    }
};
#else
typedef HAL ProfilerHAL;
#endif

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;
    sce::Gnm::RenderTarget*         pRenderSurface;         // The render target.
    sce::Gnm::DepthRenderTarget*    pDSSurface;             // The depth stencil surface (0 if not required).

    static void UpdateData( RenderBuffer* buffer, sce::Gnm::RenderTarget* prt, 
        DepthStencilBuffer* pdsb, sce::Gnm::DepthRenderTarget* pdss )
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (PS4::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, prt, pdsb, pdss);
            buffer->SetRenderTargetData(poldHD);
            return;
        }

        poldHD->pDepthStencilBuffer = pdsb;
        poldHD->pDSSurface          = pdss;
        poldHD->pRenderSurface      = prt;
    }

    virtual ~RenderTargetData()
    {
    }

private:
    RenderTargetData( RenderBuffer* buffer, sce::Gnm::RenderTarget* prt, DepthStencilBuffer* pdsb, sce::Gnm::DepthRenderTarget* pdss) : 
       RenderBuffer::RenderTargetData(buffer, pdsb), pRenderSurface(prt), pDSSurface(pdss)
       {
       }
};

}}} // Scaleform::Render::D3D1x

#endif // INC_SF_Orbis_HAL_H
