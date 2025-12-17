/**************************************************************************

Filename    :   D3D9_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_D3D9_HAL_H
#define INC_SF_Render_D3D9_HAL_H

#include "Render/D3D9/D3D9_MeshCache.h"
#include "Render/D3D9/D3D9_Texture.h"
#include "Render/D3D9/D3D9_Shader.h"    // ShaderManager
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.
#include "Render/Render_Profiler.h"

#include <d3d9.h>

namespace Scaleform { namespace Render { namespace D3D9 {


// HALConfigFlags enumeration defines system-specific HAL configuration
// flags passed into InitHAL though HALInitParams.

enum HALConfigFlags
{
    // Prevents BeginScene/EndScene from being called inside BeginDisplay/EndDisplay
    // - assumes that Direct3D is already in scene by the time BeginDisplay/EndDisplay are called
    // - allows user to manage their own begin/end scene calls 
    HALConfig_NoSceneCalls          = 0x00000001,

    // Use static buffers in the mesh cache. This has no visible impact, however, it may
    // have performance implications. Rendering from static buffers generally has faster 
    // throughput, but can cause significant latency issues with dynamic content. This also 
    // may be used silently, if the underlying hardware does not support D3D9 queries.
    HALConfig_StaticBuffers         = 0x00000010,

    // Forces the shader system to use ShaderModel 2.0 shaders (even if it is capable of 3.0)
    HALConfig_ShaderModel20         = 0x00000020,
};


// D3D9::HALInitParems provides D3D9-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    IDirect3DDeviceX*       pD3DDevice;
    D3DPRESENT_PARAMETERS   PresentParams;

    HALInitParams(IDirect3DDeviceX* device,
                  const D3DPRESENT_PARAMETERS& presentParams,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : pD3DDevice(device), PresentParams(presentParams),
      Render::HALInitParams(0, halConfigFlags, renderThreadId)
    {
    }

    // D3D9::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};

// D3D9 MatrixState overrides GetFullViewportMatrix to deal with 1/2 pixel center offset.
class MatrixState : public Render::MatrixState
{
public:
    MatrixState(Render::HAL* phal) : Render::MatrixState(phal) { }
    MatrixState() : Render::MatrixState() { }
    virtual Matrix2F&   GetFullViewportMatrix(const Size<int>& rtSize)
    {
        ModifiedFullViewportMVP = FullViewportMVP;
        ModifiedFullViewportMVP.Tx() -= 1.0f/rtSize.Width;   // D3D9 1/2 pixel center offset
        ModifiedFullViewportMVP.Ty() += 1.0f/rtSize.Height;
        return ModifiedFullViewportMVP;
    }
protected:
    mutable Matrix2F ModifiedFullViewportMVP;
};

class HAL: public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

    // Direct3D Device we are using.
    IDirect3DDeviceX*        pDevice;

    // Presentation parameters specified to configure the mode.
    D3DPRESENT_PARAMETERS    PresentParams;

    MeshCache                Cache;
    RenderSync               RSync;

    Ptr<TextureManager>      pTextureManager;
    
    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    
    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();
    
    // *** HAL Initialization / Shutdown Logic

    // Initialize rendering
    virtual bool        InitHAL(const D3D9::HALInitParams& params);
    // Shutdown rendering (cleanup).
    virtual bool        ShutdownHAL();

    // D3D9 device Reset and lost device support.
    // - PrepareForReset should be called before IDirect3DDevice9::Reset to release
    // caches and other system-specific references.
    void                PrepareForReset();
    // - RestoreAfterReset called after reset to restore needed variables.
    bool                RestoreAfterReset();


    // *** Rendering
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Updates D3D HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();

    
    // Stream Source modification
    virtual UPInt       setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt       setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    virtual void        setLinearStreamSource(PrimitiveBatch::BatchType type);
    virtual void        setInstancedStreamSource(UPInt instanceCount, UPInt verticesPerInstance);

    // *** Rasterization
    virtual void        applyRasterModeImpl(RasterModeType mode);

    // *** Mask Support
    // This flag indicates whether we've checked for stencil after BeginDisplay or not.
    // Increment op we need for stencil.
    D3DSTENCILOP    StencilOpInc;    

    virtual void    applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual bool    checkDepthStencilBufferCaps();

    // Background clear helper, expects viewport coordinates.
    virtual void     clearSolidRectangle(const Rect<int>& r, Color color, bool blend);


    // *** BlendMode
    virtual void        applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);
    virtual void        applyBlendModeEnableImpl(bool enabled);



    IDirect3DDeviceX*   GetDevice() const { return pDevice; }


    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(IDirect3DSurface9* pcolor, IDirect3DSurface9* pdepth);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags=0, Color clearColor=0);
    virtual void            PopRenderTarget(unsigned flags = 0);
    
    virtual bool            createDefaultRenderBuffer();

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*    GetRenderSync()       { return &RSync; }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned)
    {
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced, 
            MVF_Align | MVF_ReverseColor | (SManager.HasInstancingSupport() ? MVF_HasInstancing : 0));
    }

protected:

    virtual void        setBatchUnitSquareVertexStream();
    virtual void        drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void        drawIndexedPrimitive( unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexBaseIndex);
    virtual void        drawIndexedInstanced( unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexBaseIndex);

    // Returns whether the profile can render any of the filters contained in the FilterPrimitive.
    // If a profile does not support dynamic looping (Cap_NoDynamicLoops), no blur/shadow type filters
    // can be rendered, in which case this may return false, however, ColorMatrix filters can still be rendered.
    bool                shouldRenderFilters(const FilterPrimitive* prim) const;

    // Creates the D3D9-specific MatrixState object.
    virtual void        initMatrices();

    // Simply sets a quad vertex buffer and draws.
    virtual void        drawScreenQuad();

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
    IDirect3DSurface9*       pRenderSurface;        // Render target surface.
    IDirect3DSurface9*       pDSSurface;            // Depth stencil surface (0 if not required)

    static void UpdateData( RenderBuffer* buffer, IDirect3DSurface9* prt, 
                            DepthStencilBuffer* pdsb, IDirect3DSurface9* pdss )
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (D3D9::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, prt, pdsb, pdss);
            buffer->SetRenderTargetData(poldHD);
            return;
        }

        if ( prt )
            prt->AddRef();
        if ( pdss )
            pdss->AddRef();
        if ( poldHD->pRenderSurface ) 
            poldHD->pRenderSurface->Release();
        if ( poldHD->pDSSurface ) 
            poldHD->pDSSurface->Release();

        poldHD->pDepthStencilBuffer = pdsb;
        poldHD->pDSSurface          = pdss;
        poldHD->pRenderSurface      = prt;
        poldHD->CacheID             = 0;
    }

    virtual ~RenderTargetData()
    {
        if ( pRenderSurface )
            pRenderSurface->Release();
        if ( pDSSurface )
            pDSSurface->Release();        
        pDepthStencilBuffer = 0;
    }

private:
    RenderTargetData( RenderBuffer* buffer, IDirect3DSurface9* prt, DepthStencilBuffer* pdsb, IDirect3DSurface9* pdss) : 
      RenderBuffer::RenderTargetData(buffer, pdsb), pRenderSurface(prt), pDSSurface(pdss)
    {
        if ( pRenderSurface )
            pRenderSurface->AddRef();
        if ( pDSSurface )
            pDSSurface->AddRef();        
    }
};


}}} // Scaleform::Render::D3D9

#endif
