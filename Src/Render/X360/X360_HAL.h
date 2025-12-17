/**************************************************************************

Filename    :   X360_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_X360_HAL_H
#define INC_SF_Render_X360_HAL_H

#include "Render/Render_HAL.h"
#include "Render/X360/X360_MeshCache.h"
#include "Render/X360/X360_Shader.h"
#include "Render/X360/X360_Texture.h"
#include "Render/X360/X360_MemoryManager.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.

namespace Scaleform { namespace Render { namespace X360 {

#define SF_RENDERER_SHADER_VERSION  0x0200


// HALConfigFlags enumeration defines system-specific HAL configuration
// flags passed into InitHAL though HALInitParams.

enum HALConfigFlags
{
    // Prevents BeginScene/EndScene from being called inside BeginDisplay/EndDisplay
    // - assumes that Direct3D is already in scene by the time BeginDisplay/EndDisplay are called
    // - allows user to manage their own begin/end scene calls 
    HALConfig_NoSceneCalls          = 0x00000001,
};


// X360::HALInitParams provides Xbox360-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    IDirect3DDeviceX*       pD3DDevice;
    D3DPRESENT_PARAMETERS   PresentParams;

    HALInitParams(IDirect3DDeviceX* device,
                  const D3DPRESENT_PARAMETERS& presentParams,
                  Render::MemoryManager* pmemManager = 0,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : pD3DDevice(device), PresentParams(presentParams),
      Render::HALInitParams(pmemManager, halConfigFlags, renderThreadId)
    { }

    // X360::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};



//------------------------------------------------------------------------
// X360::HAL implements the Render::HAL interface on Xbox 360.

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:

    // Direct3D Device we are using.
    IDirect3DDeviceX*   pDevice;

    // Presentation parameters specified to configure the mode.
    D3DPRESENT_PARAMETERS   PresentParams;

    RenderSync               RSync;
    MeshCache                Cache;

    Ptr<TextureManager>      pTextureManager;
    MemoryManager *          pMemoryManager;
    bool                     OwnMemoryManager;
    FragShader               StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader             StaticVShaders[VertexShaderDesc::VSI_Count];
    
    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    
    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();
    
    // *** HAL Initialization / Shutdown Logic

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const X360::HALInitParams& params);
    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // *** Rendering
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Updates D3D HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();

    // X360 requires a prepass rendering step, because render targets require eviction from EDRAM.
    virtual bool        IsPrepassRequired() const { return true; }
    virtual void        SetDisplayPass(DisplayPass pass);

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix>& matrices);

    // Stream Source modification

    void setLinearStreamSource(PrimitiveBatch::BatchType type);

    void setInstancedStreamSource(const ShaderManager::Shader& pso, MeshCacheItem * pmesh );
    void setInstancedStreamSource(const ShaderManager::Shader& pso, MeshCacheItem * pmesh, const ComplexMesh::FillRecord& fr );


    // *** Mask Support
    // This flag indicates whether we've checked for stencil after BeginDisplay or not.
    bool            StencilChecked;
    // This flag is stencil is available, after check.
    bool            StencilAvailable;
    bool            MultiBitStencil;
    bool            DepthBufferAvailable;    
    // Increment op we need for stencil.
    D3DSTENCILOP    StencilOpInc;    

    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    void    beginMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        StencilChecked  = 0;
        StencilAvailable= 0;
        MultiBitStencil = 0;
        DepthBufferAvailable = 0;
        HALState &= ~HS_DrawingMask;
    }
    void   endMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        MaskStackTop = 0;
        MaskStack.Clear();
    }

    bool    checkMaskBufferCaps();

    // Background clear helper, expects viewport coordinates.
    virtual void    clearSolidRectangle(const Rect<int>& r, Color color);


    // *** BlendMode
    virtual void    applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);

    IDirect3DDeviceX*   GetDevice() const { return pDevice; }


    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(IDirect3DSurface9* pcolor, IDirect3DSurface9* pdepth, Render::Texture* presolve);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(Render::RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, Render::RenderTarget* prt, unsigned flags=0);
    virtual void            PopRenderTarget(unsigned flags=0);

    virtual bool            createDefaultRenderBuffer();

    // *** Filters
    virtual void          PushFilters(FilterPrimitive* primitive);
    virtual void          PopFilters();
    virtual void          drawUncachedFilter(const FilterStackEntry& e);
    virtual void          drawCachedFilter(FilterPrimitive* primitive);

    // *** DrawableImage
    virtual void            DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                                unsigned channelMask, const UInt32* values);

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
        
    virtual Render::RenderSync* GetRenderSync() const { return (Render::RenderSync*)&RSync; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned)
    {
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced, MVF_Align | MVF_ReverseColor | MVF_EndianSwapFactors | MVF_HasInstancing );
    }

protected:
    virtual void setBatchUnitSquareVertexStream();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    void         drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UPInt indexOffset, unsigned vertexBaseIndex);
    void         drawIndexedInstanced( unsigned indexCount, unsigned meshCount, unsigned vertexBaseIndex);

    // Simply sets a quad vertex buffer and draws.
    virtual void        drawScreenQuad();

    // *** Events
    virtual Render::RenderEvent& GetEvent(EventType eventType);
};

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;
    IDirect3DSurface9*       pRenderSurface;
    IDirect3DTexture9*       pResolveTarget;        // Used only with RTs (not with DS).
    IDirect3DSurface9*       pDepthStencilSurface;

    static void UpdateData( RenderBuffer* buffer, IDirect3DSurface9* prt, IDirect3DTexture9* presolve, 
                            DepthStencilBuffer* pdsb, IDirect3DSurface9* pdss)
    {
        if ( !buffer )
            return;
        RenderTargetData* poldHD = (X360::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, prt, presolve, pdsb, pdss);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        poldHD->pDepthStencilBuffer  = pdsb;
        poldHD->pRenderSurface       = prt;
        poldHD->pResolveTarget       = presolve;
        poldHD->pDepthStencilSurface = pdss;
        poldHD->CacheID              = 0;
    }

private:
    RenderTargetData( RenderBuffer* buffer, IDirect3DSurface9* prt, IDirect3DTexture9* presolve, 
             DepthStencilBuffer* pdsb, IDirect3DSurface9* pdss) : 
      RenderBuffer::RenderTargetData(buffer, pdsb), pRenderSurface(prt), 
      pResolveTarget(presolve), pDepthStencilSurface(pdss)
    {
    }
};

}}} // Scaleform::Render::D3D9

#endif
