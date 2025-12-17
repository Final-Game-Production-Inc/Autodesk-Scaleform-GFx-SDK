/**************************************************************************

Filename    :   D3D1x_HAL.h
Content     :   
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D1X_HAL_H
#define INC_SF_D3D1X_HAL_H
#pragma once

#include "Render/D3D1x/D3D1x_Config.h"
#include "Render/Render_HAL.h"
#include "Render/D3D1x/D3D1x_MeshCache.h"
#include "Render/D3D1x/D3D1x_Shader.h"
#include "Render/D3D1x/D3D1x_Texture.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.
#include "Render/Render_Profiler.h"

namespace Scaleform { namespace Render { namespace D3D1x {

//------------------------------------------------------------------------
enum HALConfigFlags
{
};

// D3D1x::HALInitParems provides D3D10/D3D11 specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    ID3D1x(Device)*               pD3DDevice;
    D3D11(ID3D1x(DeviceContext)* pD3DContext);

    HALInitParams(ID3D1x(Device)* device,
                  D3D11(ID3D1x(DeviceContext)* pD3DContext,)
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : pD3DDevice(device),
      D3D11(pD3DContext(pD3DContext),)
      Render::HALInitParams(0, halConfigFlags, renderThreadId)
    { }

    // D3D1x::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};


//------------------------------------------------------------------------

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

    // Direct3D Device we are using.
    ID3D1x(Device)*               pDevice;
    ID3D1x(DeviceContext)*        pDeviceContext;
    Ptr<ID3D1x(RenderTargetView)> pRenderTargetView;
    Ptr<ID3D1x(DepthStencilView)> pDepthStencilView;

    MeshCache                     Cache;
    RenderSync                    RSync;
    Ptr<TextureManager>           pTextureManager;
    
    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    
    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();
    
    // *** HAL Initialization / Shutdown Logic

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const D3D1x::HALInitParams& params);
    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // - PrepareForReset should be called before the device's SwapChain is resized.
    // The HAL holds a reference to the default back and depthstencil buffers, which need
    // to be released and reobtained.
    void                PrepareForReset();
    // - RestoreAfterReset called after reset to restore needed variables.
    bool                RestoreAfterReset();


    // *** Rendering
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Updates D3D HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();

    virtual UPInt       setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt       setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);

    // *** Mask Support
    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual bool        checkDepthStencilBufferCaps();

    // *** Rasterization
    virtual bool        IsRasterModeSupported(RasterModeType mode) const;
    virtual void        applyRasterModeImpl(RasterModeType mode);

    // *** BlendMode
    void            applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);


    // *** Device States
    enum ColorWriteMode
    {
        Write_All,
        Write_None,
        Write_Count
    };

    static const unsigned BlendTypeCount = Blend_Count*2 + 1;
    static unsigned GetBlendType( BlendMode blendMode, ColorWriteMode writeMode, bool sourceAc = false) 
    { 
        unsigned base = 0;
        if ( writeMode == Write_None )
        {
            base = Blend_Count*2;
            blendMode = (BlendMode)0;
        }
        else if ( sourceAc )
            base += Blend_Count;
        return base + (unsigned)blendMode;
    }
    ID3D1x(BlendState)* BlendStates[BlendTypeCount];
    bool createBlendStates();
    void destroyBlendStates();

    ID3D1x(DepthStencilState)* DepthStencilStates[DepthStencil_Count];
    bool createDepthStencilStates();
    void destroyDepthStencilStates();

    ID3D1x(RasterizerState)* RasterStates[RasterMode_Count];
    bool createRasterStates();
    void destroyRasterStates();


    ID3D1x(Device)*   GetDevice() const { return pDevice; }

    static const unsigned   ConstantBufferCount = 8;
    ID3D1x(Buffer)*         ConstantBuffers[ConstantBufferCount];
    unsigned                CurrentConstantBuffer;

    bool                  createConstantBuffers();
    void                  destroyConstantBuffers();
    ID3D1x(Buffer)*       getNextConstantBuffer();

    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(ID3D1x(View)* pRenderTarget, ID3D1x(View)* pDepthStencil);
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
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned = 0)
    {
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced);
    }

protected:

    virtual void setBatchUnitSquareVertexStream();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );
    virtual void drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );

    // Returns whether the profile can render any of the filters contained in the FilterPrimitive.
    // If a profile does not support dynamic looping (eg. using SM2.0), no blur/shadow type filters
    // can be rendered, in which case this may return false, however, ColorMatrix filters can still be rendered.
    bool        shouldRenderFilters(const FilterPrimitive* prim) const;

    // *** Events
    virtual Render::RenderEvent& GetEvent(EventType eventType);

    void drawScreenQuad();
};

// Use this HAL if you want to use profile modes.
class ProfilerHAL : public Render::ProfilerHAL<HAL>
{
public:
    ProfilerHAL(ThreadCommandQueue* commandQueue) : Render::ProfilerHAL<HAL>(commandQueue)
    {
    }
};

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;
    ID3D1x(View)*           pRenderSurface;         // View of the render target.
    ID3D1x(View)*           pDSSurface;             // View of the depth stencil surface (0 if not required).

    static void UpdateData( RenderBuffer* buffer, ID3D1x(View)* prt, 
                            DepthStencilBuffer* pdsb, ID3D1x(View)* pdss )
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (D3D1x::RenderTargetData*)buffer->GetRenderTargetData();
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
    }

    virtual ~RenderTargetData()
    {
        if ( pRenderSurface )
            pRenderSurface->Release();
        if ( pDSSurface )
            pDSSurface->Release();
    }

private:
    RenderTargetData( RenderBuffer* buffer, ID3D1x(View)* prt, DepthStencilBuffer* pdsb, ID3D1x(View)* pdss) : 
      RenderBuffer::RenderTargetData(buffer, pdsb), pRenderSurface(prt), pDSSurface(pdss)
    {
        if ( pRenderSurface )
            pRenderSurface->AddRef();
        if ( pDSSurface )
            pDSSurface->AddRef();
    }
};

}}} // Scaleform::Render::D3D1x

#endif // INC_SF_D3D1X_HAL_H
