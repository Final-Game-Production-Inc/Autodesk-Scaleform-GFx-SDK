/**************************************************************************

Filename    :   D3D12_HAL.h
Content     :   D3D12 Hardware Abstraction Layer.
Created     :   Mar 2026
Authors     :   FINALZ-27980

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_HAL_H
#define INC_SF_D3D12_HAL_H
#pragma once

#include "Render/Render_HAL.h"
#include "Render/D3D12/D3D12_MeshCache.h"
#include "Render/D3D12/D3D12_Shader.h"
#include "Render/D3D12/D3D12_Texture.h"
#include "Render/D3D12/D3D12_Sync.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.
#include "Render/Render_Profiler.h"

#include <d3d12.h>

namespace Scaleform { namespace Render { namespace D3D12 {

//------------------------------------------------------------------------
enum HALConfigFlags
{
};

// D3D12::HALInitParems provides D3D12 specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    ID3D12Device*               pD3DDevice;
    ID3D12CommandQueue*         pCommandQueue;
    ID3D12CommandAllocator*     pCommandAllocator;
    ID3D12GraphicsCommandList*  pCommandList;        // 命令列表（用于记录命令）
    ID3D12DescriptorHeap*       pDescriptorHeap;     // 描述符堆（CBV/SRV/UAV）

    HALInitParams(ID3D12Device* device,
                  ID3D12CommandQueue* commandQueue,
                  ID3D12CommandAllocator* commandAllocator,
                  ID3D12GraphicsCommandList* commandList,
                  ID3D12DescriptorHeap* descriptorHeap,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : Render::HALInitParams(0, halConfigFlags, renderThreadId),
      pD3DDevice(device),
      pCommandQueue(commandQueue),
      pCommandAllocator(commandAllocator),
      pCommandList(commandList),
      pDescriptorHeap(descriptorHeap)
    { }

    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*)pTextureManager.GetPtr(); }
};

// Pipeline state key for the pipeline state object cache. The render target
// and depth-stencil formats are included so pipelines for the swapchain
// (BGRA) and offscreen (RGBA) targets are kept separate.
struct PipelineKey
{
    unsigned                VSIndex;
    unsigned                FSIndex;
    unsigned                BlendType;
    unsigned                StencilMode;
    unsigned                RasterMode;
    const SysVertexFormat*  pVFormat;
    DXGI_FORMAT             RTFormat;
    DXGI_FORMAT             DSFormat;

    bool operator==(const PipelineKey& o) const
    {
        return VSIndex == o.VSIndex && FSIndex == o.FSIndex &&
               BlendType == o.BlendType && StencilMode == o.StencilMode &&
               RasterMode == o.RasterMode && pVFormat == o.pVFormat &&
               RTFormat == o.RTFormat && DSFormat == o.DSFormat;
    }
};

struct PipelineKeyHashF
{
    static UPInt RotateLeft(UPInt val, unsigned bits)
    {
        return (val << bits) | (val >> (sizeof(UPInt) * 8 - bits));
    }
    UPInt operator()(const PipelineKey& k) const
    {
        UPInt h = (UPInt)k.VSIndex * 2654435761u;
        h ^= RotateLeft((UPInt)k.FSIndex * 2246822519u, 5);
        h ^= RotateLeft((UPInt)k.BlendType * 2246822519u, 7);
        h ^= RotateLeft((UPInt)k.StencilMode * 3266489917u, 13);
        h ^= RotateLeft((UPInt)k.RasterMode * 668265263u, 19);
        h ^= RotateLeft((UPInt)k.pVFormat * 374761393u, 11);
        h ^= RotateLeft((UPInt)k.RTFormat * 2654435761u, 17);
        h ^= RotateLeft((UPInt)k.DSFormat * 40503u, 23);
        return h;
    }
};


class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

    // *** D3D12 device objects (AddRef'd in InitHAL).
    ID3D12Device*                   pDevice;
    ID3D12CommandQueue*             pCommandQueue;
    Ptr<ID3D12CommandAllocator>     pCommandAllocator;
    Ptr<ID3D12GraphicsCommandList>  pCommandList;
    Ptr<ID3D12DescriptorHeap>       pDescriptorHeap; // Platform-provided CBV/SRV/UAV heap (kept for parity).

    MeshCache               Cache;
    RenderSync              RSync;
    Ptr<TextureManager>     pTextureManager;
    PrimitiveBatch::BatchType PrevBatchType;

    HAL*    getThis() { return this; }

public:
    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();

    // *** Initialization
    virtual bool        InitHAL(const D3D12::HALInitParams& params);
    virtual bool        ShutdownHAL();
    void                PrepareForReset();
    bool                RestoreAfterReset();

    // *** Rendering
    virtual bool        BeginFrame();
    virtual bool        BeginScene();
    virtual bool        EndScene();
    virtual void        updateViewport();
    virtual void        drawScreenQuad();

    // *** Mask / Stencil
    virtual void    applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual bool    checkDepthStencilBufferCaps();

    // *** Blend modes
    void    applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);

    enum ColorWriteMode
    {
        Write_All,
        Write_None,
        Write_Count
    };

    static const unsigned BlendTypeCount = Blend_Count * 2 + 1;
    static unsigned GetBlendType(BlendMode blendMode, ColorWriteMode writeMode, bool sourceAc = false)
    {
        unsigned base = 0;
        if (writeMode == Write_None)
        {
            base = Blend_Count * 2;
            blendMode = (BlendMode)0;
        }
        else if (sourceAc)
            base += Blend_Count;
        return base + (unsigned)blendMode;
    }

    // *** Raster modes
    virtual bool IsRasterModeSupported(RasterModeType mode) const { return mode != RasterMode_Point; }
    virtual void applyRasterModeImpl(RasterModeType mode);

    // *** Vertex arrays (pure virtuals from ShaderHAL)
    virtual UPInt setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);

    // *** Pipeline management
    ID3D12RootSignature*    pRootSignature;
    bool        createRootSignature();
    void        destroyRootSignature();

    // PSO cache lookup; creates the pipeline state object if needed.
    ID3D12PipelineState* GetPipeline(unsigned vsIndex, unsigned fsIndex,
                                     unsigned blendType, unsigned stencilMode, unsigned rasterMode,
                                     const SysVertexFormat* pvf, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat);
    void        destroyPipelines();

    // *** Uniform ring buffer
    ID3D12Resource*             pUniformBuffer;
    UByte*                      pUniformData;
    UPInt                       UniformBufferSize;
    UPInt                       UniformBufferOffset;
    D3D12_GPU_VIRTUAL_ADDRESS   UniformBufferGPUBase;
    bool        createUniformBuffer();
    void        destroyUniformBuffer();

    // Allocates 'size' bytes (256-byte aligned) from the uniform ring buffer.
    // Returns the CPU pointer and outputs the GPU virtual address.
    UByte*      MapUniformBuffer(D3D12_GPU_VIRTUAL_ADDRESS* pGPU, UPInt size);
    // Records the vertex/fragment constant buffer addresses for the next draw.
    void        SetCurrentConstantBuffers(D3D12_GPU_VIRTUAL_ADDRESS vs, D3D12_GPU_VIRTUAL_ADDRESS fs);

    // *** Stage texture binding
    // Records a texture stage (SRV descriptor + sampler descriptor group).
    // bindStageTextures() turns the recorded stages into the t0..t7 / s0..s7
    // descriptor tables the root signature declares.
    void        SetStageTexture(unsigned stage, D3D12_CPU_DESCRIPTOR_HANDLE srvCPU,
                                D3D12_GPU_DESCRIPTOR_HANDLE srvGPU,
                                D3D12_CPU_DESCRIPTOR_HANDLE samplerCPU,
                                D3D12_GPU_DESCRIPTOR_HANDLE samplerGPU);
    // Stages the SRV descriptor table and binds both descriptor tables.
    void        bindStageTextures();

    // *** Render targets
    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags = 0, Color clearColor = 0);
    virtual void            PopRenderTarget(unsigned flags = 0);
    virtual void            drawFilter(const Matrix2F& mvp, const Cxform& cx, const Filter* filter,
                                       Ptr<RenderTarget>* targets, unsigned* shaders, unsigned pass,
                                       unsigned passCount, const VertexFormat* pvf, BlurFilterState& leBlur);
    virtual bool            createDefaultRenderBuffer();

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*    GetRenderSync()       { return &RSync; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned = 0)
    {
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced);
    }

    // Provides the swapchain back buffer as the default render target.
    // Called by the platform after InitHAL (and per frame, if using a
    // flip-model swapchain where the current back buffer index changes).
    void        SetMainRenderTarget(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                                    DXGI_FORMAT format, const ImageSize& size,
                                    ID3D12Resource* dsResource = 0,
                                    D3D12_CPU_DESCRIPTOR_HANDLE dsv = D3D12_CPU_DESCRIPTOR_HANDLE());

    ID3D12Device* GetDevice() const { return pDevice; }
    ID3D12GraphicsCommandList* GetCommandList() const { return pCommandList; }
    void        SetCommandList(ID3D12GraphicsCommandList* pcl);

protected:
    virtual void setBatchUnitSquareVertexStream();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);
    virtual void drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);

    bool         IsFilterCachingSupported() const;
    bool         shouldRenderFilters(const FilterPrimitive* prim) const;
    virtual bool DefersBufferManagerEndFrame() const { return true; }
    virtual Render::RenderEvent& GetEvent(EventType eventType);

    // Prepares the pipeline state for the current draw call: binds the root
    // signature, PSO, constant buffers and stage descriptors.
    void        preparePipeline();
    // Transitions the current render target resource to the RENDER_TARGET state.
    void        transitionRenderTarget(RenderTarget* ptarget, bool toRenderTarget);

public:
    // Pipeline state - public so ShaderInterface::Finish can bind pipelines.
    unsigned        CurrentBlendType;
    unsigned        CurrentStencilMode;
    unsigned        CurrentRasterMode;

    // Main (swapchain) render target info, provided by the platform.
    struct MainRenderTargetInfo
    {
        Ptr<ID3D12Resource>         pResource;
        D3D12_CPU_DESCRIPTOR_HANDLE RTV;
        Ptr<ID3D12Resource>         pDSResource;
        D3D12_CPU_DESCRIPTOR_HANDLE DSV;
        DXGI_FORMAT                 Format;
        ImageSize                   Size;
        bool                        Set;

        MainRenderTargetInfo() : Format(DXGI_FORMAT_UNKNOWN), Set(false)
        {
            RTV.ptr = 0;
            DSV.ptr = 0;
        }
    };
    MainRenderTargetInfo MainRT;

private:
    // Pipeline cache map.
    typedef Hash<PipelineKey, ID3D12PipelineState*, PipelineKeyHashF> PipelineMap;
    PipelineMap     Pipelines;

    // Blend / depth-stencil / raster state descriptions (reusable for PSO creation).
    D3D12_RENDER_TARGET_BLEND_DESC BlendDescs[BlendTypeCount];
    D3D12_DEPTH_STENCIL_DESC       DepthStencilDescs[DepthStencil_Count];
    D3D12_RASTERIZER_DESC          RasterDescs[RasterMode_Count];
    void    initBlendDescriptions();
    void    initDepthStencilDescriptions();
    void    initRasterDescriptions();

    // Current stage texture state (descriptor handles per stage).
    struct StageTextureState
    {
        D3D12_CPU_DESCRIPTOR_HANDLE SRV;
        D3D12_CPU_DESCRIPTOR_HANDLE Sampler;
        // GPU handle of the persistent s0..s7 sampler group for this stage.
        D3D12_GPU_DESCRIPTOR_HANDLE SamplerGPU;
    };
    StageTextureState           StageTextures[4];
    unsigned                    StageTextureCount;
    // Descriptor table staging. The root signature declares t0..t7 and s0..s7,
    // so every draw stages a full 8-entry SRV table (unused stages alias the
    // first valid stage). Consecutive draws usually share their texture set, so
    // the staged handles are cached and the copy is skipped when unchanged.
    static const unsigned       StageTableSize = 8;
    D3D12_CPU_DESCRIPTOR_HANDLE LastStagedSRVs[StageTableSize];
    D3D12_GPU_DESCRIPTOR_HANDLE LastSRVTableGPU;
    bool                        LastStageValid;
    unsigned                    SRVScratchCursor;
    D3D12_GPU_VIRTUAL_ADDRESS   CurrentVSAddress;
    D3D12_GPU_VIRTUAL_ADDRESS   CurrentFSAddress;
    bool                        DescriptorHeapsSet;
    // Stencil reference currently recorded on the command list. D3D12 keeps the
    // reference as dynamic state (OMSetStencilRef), separate from the PSO, and a
    // freshly reset list starts at 0. Tracked so masks (which use the mask level
    // as the reference) actually restrict masked content.
    unsigned                    CurrentStencilRefOnList;

    // Formats of the currently bound render target (part of the PSO key).
    DXGI_FORMAT                 CurrentRTFormat;
    DXGI_FORMAT                 CurrentDSFormat;

    // Cache of the last bound pipeline to avoid redundant PSO binding.
    ID3D12PipelineState*        pBoundPipeline;
    PipelineKey                 BoundPipelineKey;
    bool                        PipelineDirty;

    // Whole-target retirement for temporary render targets (same rule Vulkan
    // implements in DeferredTargetRelease / PendingTargetReleases): the target
    // and its depth/stencil buffer stay alive - and stay out of the buffer
    // manager's reuse lists - until the frame that used them has been fenced.
    // Without this, the second generation of the 9-1-B feedback rebuild could
    // re-acquire a target whose RTV/DSV belonged to a command list that had not
    // executed yet; the GPU then hung in OMSetRenderTargets.
    struct DeferredTargetRelease
    {
        Ptr<RenderTarget>       pTarget;
        Ptr<DepthStencilBuffer> pDepthStencil;

        DeferredTargetRelease() { }
        DeferredTargetRelease(RenderTarget* target, DepthStencilBuffer* ds)
            : pTarget(target), pDepthStencil(ds) { }
    };
    Array<DeferredTargetRelease> PendingTargetReleases;

    RenderEvent Events[Event_Count];
};


// RenderTargetData wraps the D3D12 color/depth-stencil resources and their
// descriptor handles for a RenderBuffer.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;

    Ptr<ID3D12Resource>         pRenderSurface;
    Ptr<ID3D12Resource>         pDSSurface;
    D3D12_CPU_DESCRIPTOR_HANDLE RTV;
    D3D12_CPU_DESCRIPTOR_HANDLE DSV;
    DXGI_FORMAT                 Format;
    // Last recorded D3D12 resource state of pRenderSurface. D3D12 - unlike
    // D3D1x - needs an explicit transition before a render target can be
    // sampled as a texture; tracking it here makes those barriers exact
    // (no redundant barriers, no RENDER_TARGET -> SRV sampling hazards).
    D3D12_RESOURCE_STATES       ResourceState;

    static void UpdateData(RenderBuffer* buffer, ID3D12Resource* rtRes,
                           const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, DXGI_FORMAT format,
                           DepthStencilBuffer* pdsb, ID3D12Resource* dsRes,
                           const D3D12_CPU_DESCRIPTOR_HANDLE& dsv)
    {
        if (!buffer) return;
        RenderTargetData* poldHD = (D3D12::RenderTargetData*)buffer->GetRenderTargetData();
        if (!poldHD)
        {
            poldHD = SF_NEW RenderTargetData(buffer, rtRes, rtv, format, pdsb, dsRes, dsv);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        // Only a *different* resource starts in RENDER_TARGET state. UpdateData is
        // also called when a pooled temporary target gets a stencil buffer again;
        // resetting the tracked state unconditionally there made
        // transitionRenderTarget skip the SRV -> RENDER_TARGET barrier and the next
        // ClearRenderTargetView/draw operated on a resource still in
        // PIXEL_SHADER_RESOURCE state (validation id=538/527, GPU hang).
        const bool newTarget = (poldHD->pRenderSurface.GetPtr() != rtRes);
        poldHD->pRenderSurface = rtRes;
        poldHD->pDSSurface = dsRes;
        poldHD->RTV = rtv;
        poldHD->DSV = dsv;
        poldHD->Format = format;
        poldHD->pDepthStencilBuffer = pdsb;
        if (newTarget)
            poldHD->ResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

private:
    RenderTargetData(RenderBuffer* buffer, ID3D12Resource* rtRes,
                     const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, DXGI_FORMAT format,
                     DepthStencilBuffer* pdsb, ID3D12Resource* dsRes,
                     const D3D12_CPU_DESCRIPTOR_HANDLE& dsv)
    : RenderBuffer::RenderTargetData(buffer, pdsb),
      pRenderSurface(rtRes), pDSSurface(dsRes), RTV(rtv), DSV(dsv), Format(format),
      ResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET) {}
};

}}} // Scaleform::Render::D3D12

#endif // INC_SF_D3D12_HAL_H
