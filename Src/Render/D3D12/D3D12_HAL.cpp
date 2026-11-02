/**************************************************************************

Filename    :   D3D12_HAL.cpp
Content     :   D3D12 Hardware Abstraction Layer implementation.
Created     :   Mar 2026
Authors     :   FINALZ-27980

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Render/D3D12/D3D12_HAL.h"
#include "Render/D3D12/D3D12_Events.h"
#include "Render/Render_BufferGeneric.h"
#include "Render/Render_TextureCacheGeneric.h"
#include "Render/Render_DebugEnv.h"
#include "Kernel/SF_HeapNew.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Scaleform { namespace Render { namespace D3D12 {

// Records a resource barrier on the command list.
static void D3D12_RecordBarrier(ID3D12GraphicsCommandList* pCommandList,
                                ID3D12Resource* pResource,
                                D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    if (!pCommandList || !pResource || before == after)
        return;
    D3D12_RESOURCE_BARRIER barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter  = after;
    pCommandList->ResourceBarrier(1, &barrier);
}

// Default (disabled) render-target blend description for unused RT slots.
static D3D12_RENDER_TARGET_BLEND_DESC D3D12_DisabledRTBlend()
{
    D3D12_RENDER_TARGET_BLEND_DESC d;
    memset(&d, 0, sizeof(d));
    d.BlendEnable           = FALSE;
    d.LogicOpEnable         = FALSE;
    d.SrcBlend              = D3D12_BLEND_ONE;
    d.DestBlend             = D3D12_BLEND_ZERO;
    d.BlendOp               = D3D12_BLEND_OP_ADD;
    d.SrcBlendAlpha         = D3D12_BLEND_ONE;
    d.DestBlendAlpha        = D3D12_BLEND_ZERO;
    d.BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    d.LogicOp               = D3D12_LOGIC_OP_NOOP;
    d.RenderTargetWriteMask = 0;
    return d;
}

// ***** HAL_D3D12

HAL::HAL(ThreadCommandQueue* commandQueue)
:   Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    pDevice(0),
    pCommandQueue(0),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    PrevBatchType(PrimitiveBatch::DP_None),
    pRootSignature(0),
    pUniformBuffer(0), pUniformData(0),
    UniformBufferSize(0), UniformBufferOffset(0),
    UniformBufferGPUBase(0),
    StageTextureCount(0),
    LastStageValid(false),
    SRVScratchCursor(0),
    CurrentVSAddress(0), CurrentFSAddress(0),
    DescriptorHeapsSet(false),
    CurrentStencilRefOnList(0),
    CurrentRTFormat(DXGI_FORMAT_UNKNOWN), CurrentDSFormat(DXGI_FORMAT_UNKNOWN),
    pBoundPipeline(0),
    PipelineDirty(true),
    CurrentBlendType(0),
    CurrentStencilMode(DepthStencil_Invalid),
    CurrentRasterMode(0)
{
    memset(StageTextures, 0, sizeof(StageTextures));
    memset(&BoundPipelineKey, 0, sizeof(BoundPipelineKey));
}

HAL::~HAL()
{
    ShutdownHAL();
}

// *** RenderHAL_D3D12 Implementation

bool HAL::InitHAL(const D3D12::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    if (!params.pD3DDevice)
        return 0;

    pDevice           = params.pD3DDevice;
    pCommandQueue     = params.pCommandQueue;
    pCommandAllocator = params.pCommandAllocator;
    pCommandList      = params.pCommandList;
    pDescriptorHeap   = params.pDescriptorHeap;

    pDevice->AddRef();
    if (pCommandQueue)
        pCommandQueue->AddRef();

    if ( !RSync.Init(pDevice, pCommandQueue) )
    {
        SF_DEBUG_ASSERT(0, "Failed to create D3D12 fence.");
        ShutdownHAL();
        return false;
    }

    if ( createRootSignature() &&
         createUniformBuffer() &&
         SManager.Initialize(this, params.ConfigFlags) &&
         Cache.Initialize(pDevice, &RSync, &SManager, pCommandList))
    {
        initBlendDescriptions();
        initDepthStencilDescriptions();
        initRasterDescriptions();

        // Create Texture manager if needed.
        if (params.pTextureManager)
            pTextureManager = params.GetTextureManager();
        else
        {
            pTextureManager =
                *SF_HEAP_AUTO_NEW(this) TextureManager(pDevice, &RSync,
                                                       params.RenderThreadId, pRTCommandQueue);
            if (!pTextureManager)
            {
                Cache.Reset();
                SManager.Reset();
            }
        }

        if (pTextureManager)
        {
            pTextureManager->SetOwnerHAL(this);
            pTextureManager->SetCommandList(pCommandList);
        }

        // Create RenderBufferManager if needed.
        if (params.pRenderBufferManager)
            pRenderBufferManager = params.pRenderBufferManager;
        else
        {
            // D3D12 cannot free a temporary render target while the frame's
            // command list still references it (the debug layer reports id=921
            // and the GPU can hang on the recycled memory). The default
            // "screen area" reuse budget is far too small for multi-pass
            // filters: the 9-1-B banner alone needs several 1088x512 targets,
            // so the pool evicted - and destroyed - targets in the middle of a
            // frame. Give the pool a real budget so eviction (and destruction)
            // only happens for genuinely stale targets. Override with
            // GFX_D3D12_RT_REUSE_MB=<megabytes> if needed.
            UPInt reuseLimit = 64 * 1024 * 1024;
            char  reuseEnv[32];
            if (GetDebugEnvValue("GFX_D3D12_RT_REUSE_MB", reuseEnv, sizeof(reuseEnv)))
            {
                const int mb = atoi(reuseEnv);
                if (mb > 0)
                    reuseLimit = (UPInt)mb * 1024 * 1024;
            }

            // Create the default render target, and manager.
            pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(
                RBGenericImpl::DSSM_Exact, reuseLimit);
            if ( !pRenderBufferManager || !createDefaultRenderBuffer())
            {
                ShutdownHAL();
                return false;
            }

            // D3D12 (like Vulkan) must not reuse a temporary render target in the
            // same frame it was released: the GPU has not executed the command
            // list yet, and that texture is still referenced by draws that
            // rendered into / sampled it. Reusing it for the next filter pass
            // (the "second generation" of the feedback rebuild) created a
            // read/write hazard that hung the GPU. Keep same-frame targets out
            // of the reuse match; they become available next frame.
            pRenderBufferManager->AllowsSameFrameReuse = false;
            // Targets retired during a frame stay 'in use' until the next
            // BeginFrame (whole-target retirement, see PopRenderTarget), so the
            // buffer manager's end-of-frame consistency check must tolerate them.
            pRenderBufferManager->AllowsDeferredBufferRelease = true;
        }

        // Success.
        if ( pTextureManager && pRenderBufferManager )
        {
            HALState|= HS_ModeSet;
            notifyHandlers(HAL_Initialize);
            return true;
        }
    }

    // Failure.
    pDevice->Release();
    pDevice = 0;
    if (pCommandQueue)
    {
        pCommandQueue->Release();
        pCommandQueue = 0;
    }
    return false;
}

// Updates the command list used for all rendering and resource operations.
// Called by the platform before each frame; the same list is forwarded to
// the mesh cache, shader manager and texture manager.
void HAL::SetCommandList(ID3D12GraphicsCommandList* pcl)
{
    pCommandList = pcl;
    Cache.SetCommandList(pcl);
    SManager.SetCommandList(pcl);
    if (pTextureManager)
        pTextureManager->SetCommandList(pcl);
}

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    destroyPipelines();
    destroyRootSignature();
    destroyUniformBuffer();

    // Release the targets retired during the last frame before the buffer
    // manager's consistency check (Reset asserts if targets are still 'in use').
    PendingTargetReleases.Clear();

    destroyRenderBuffers();
    pRenderBufferManager.Clear();

    // This must happen before destroying mesh buffers. If the device is lost, then the WaitFence implementation
    // depends on the RenderSync to be reset before any meshes are destroy. Otherwise an infinite wait may occur.
    RSync.SetDevice(0, 0);

    // Do TextureManager::Reset to ensure shutdown on the current thread.
    if (pTextureManager)
    {
        pTextureManager->Reset();
        pTextureManager.Clear();
    }
    SManager.Reset();
    Cache.Reset();

    pCommandAllocator.Clear();
    pCommandList.Clear();
    pDescriptorHeap.Clear();
    MainRT.pResource.Clear();
    MainRT.Set = false;

    if (pCommandQueue)
    {
        pCommandQueue->Release();
        pCommandQueue = 0;
    }
    pDevice->Release();
    pDevice = 0;
    return true;
}

void HAL::PrepareForReset()
{
    SF_ASSERT(HALState & HS_ModeSet);
    if (HALState & HS_ReadyForReset)
        return;

    // The frame is being torn down / the swap chain resized; any target retired
    // during it must not stay referenced (Reset would otherwise assert).
    PendingTargetReleases.Clear();

    // Release the default rendertarget, and depth stencil references.
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& entry = RenderTargetStack[0];
        if ( entry.pRenderTarget )
        {
            SF_ASSERT( entry.pRenderTarget->GetType() == RBuffer_Default);
            D3D12::RenderTargetData* phd = (D3D12::RenderTargetData*)entry.pRenderTarget->GetRenderTargetData();
            if ( phd )
            {
                phd->pRenderSurface = 0;
                phd->pDSSurface = 0;
            }
        }
    }

    // Release the swap-chain back-buffer reference held by MainRT; otherwise
    // IDXGISwapChain::ResizeBuffers fails with DXGI_ERROR_INVALID_CALL.
    MainRT.pResource.Clear();
    MainRT.pDSResource.Clear();
    MainRT.Set = false;

    notifyHandlers(HAL_PrepareForReset);
    HALState |= HS_ReadyForReset;
}

bool HAL::RestoreAfterReset()
{
    if (!IsInitialized())
        return false;
    if (!(HALState & HS_ReadyForReset))
        return true;

    // Reobtain the default rendertarget references.
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& entry = RenderTargetStack[0];
        if ( entry.pRenderTarget && MainRT.Set && MainRT.pResource )
        {
            SF_ASSERT( entry.pRenderTarget->GetType() == RBuffer_Default);
            D3D12::RenderTargetData* phd = (D3D12::RenderTargetData*)entry.pRenderTarget->GetRenderTargetData();
            if ( phd )
            {
                phd->pRenderSurface = MainRT.pResource;
                phd->RTV = MainRT.RTV;
                phd->Format = MainRT.Format;
                phd->pDSSurface = MainRT.pDSResource;
                phd->DSV = MainRT.DSV;
            }
        }
    }

    notifyHandlers(HAL_RestoreAfterReset);
    HALState &= ~HS_ReadyForReset;
    return true;
}

// ***** Rendering

bool HAL::BeginFrame()
{
    // The platform has already waited for the previous frame's fence before
    // calling us (see DeviceImpl::beginFrame), so this is the first safe point
    // to promote/evict the previous frame's temporary render targets and to
    // release the targets retired in that frame. Doing it in EndFrame destroyed
    // textures the just-recorded command list still referenced (debug layer
    // id=921), which made the GPU hang on the 9-1-B feedback banner.
    //
    // Retirement order mirrors Vulkan::DrainFrameResources():
    //   1) depth/stencil buffer first (it is referenced by the target's DSV),
    //   2) then mark the target available (SetInUse) so the buffer manager moves
    //      it out of the in-use list deterministically,
    //   3) and finally drop the target (its colour texture goes with it).
    // Releasing the colour target before its DS leaves the DSV/DS alive for a
    // moment while the target is already back in the pool - the next generation
    // of the feedback rebuild could then attach a new DS to a target whose old
    // DSV was still in flight.
    for (unsigned i = 0; i < PendingTargetReleases.GetSize(); ++i)
    {
        DeferredTargetRelease& r = PendingTargetReleases[i];
        r.pDepthStencil = 0;
        if (r.pTarget)
            r.pTarget->SetInUse(RTUse_Unused);
        r.pTarget = 0;
    }
    PendingTargetReleases.Clear();

    if (RenderBufferManager* prbm = GetRenderBufferManager())
        prbm->EndFrame();

    if (!BaseHAL::BeginFrame())
        return false;

    // Per-frame uniform ring reset. The platform has already waited for the
    // previous frame's fence, so no draw of the previous frame can still be
    // reading these constants.
    UniformBufferOffset = 0;

    // Descriptor staging is per frame, not per scene: every scene recorded into
    // the frame's command list may still be waiting for the descriptors it bound,
    // so the scratch area may only be recycled once the whole frame is done.
    SRVScratchCursor = 0;
    LastStageValid = false;
    // A reset command list starts with stencil reference 0.
    CurrentStencilRefOnList = 0;
    return true;
}

bool HAL::BeginScene()
{
    // Render event for anything that is done immediately within BeginScene (but not the entire scene).
    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__ "-SetState");

    if ( !BaseHAL::BeginScene() )
        return false;

    // Reset per-scene state. The descriptor scratch cursor and the staged-table
    // cache are reset per *frame* (see HAL::BeginFrame): a frame can contain
    // several scenes, and reusing scratch ranges across them would make earlier
    // draws pick up the descriptors staged by later ones (descriptor tables are
    // resolved when the command list executes, not when it is recorded).
    StageTextureCount = 0;
    DescriptorHeapsSet = false;
    PipelineDirty = true;
    pBoundPipeline = 0;
    CurrentBlendType = (unsigned)-1;
    CurrentStencilMode = DepthStencil_Invalid;
    CurrentRasterMode = (unsigned)-1;
    // NOTE: UniformBufferOffset is NOT reset here. It is reset once per frame in
    // BeginFrame, after the frame fence. Resetting per scene let a later scene
    // overwrite the constant buffers an earlier scene's draws still referenced
    // (D3D12 resolves constants when the list executes), producing garbage
    // uniforms - and, for filter shaders, unbounded blur loops -> GPU hang.

    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Transition the default render target into the render state.
    if (RenderTargetStack.GetSize() > 0)
    {
        transitionRenderTarget(RenderTargetStack[0].pRenderTarget, true);

        // D3D12 has no implicit default render target binding (unlike D3D11),
        // so bind the swap-chain RTV for the whole scene here.
        RenderTargetData* phd = (D3D12::RenderTargetData*)
            RenderTargetStack[0].pRenderTarget->GetRenderTargetData();
        if (phd && phd->RTV.ptr)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = phd->RTV;
            D3D12_CPU_DESCRIPTOR_HANDLE* pdsv = phd->DSV.ptr ? &phd->DSV : 0;
            pCommandList->OMSetRenderTargets(1, &rtv, FALSE, pdsv);
            CurrentRTFormat = phd->Format;
            CurrentDSFormat = pdsv ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN;
        }
    }

    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);

    if ( !BaseHAL::EndScene())
        return false;

    // Transition the default render target back to PRESENT so it can be presented.
    if (RenderTargetStack.GetSize() > 0)
        transitionRenderTarget(RenderTargetStack[0].pRenderTarget, false);

    return true;
}

// Updates D3D12 HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    D3D12_VIEWPORT vp;
    Rect<int>      vpRect;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;

        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(0, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        if ( !(HALState & HS_InRenderTarget))
            vpRect = ViewRect;
        else
            vpRect.SetRect(VP.Left, VP.Top, VP.Left + VP.Width, VP.Top + VP.Height);
    }

    vp.TopLeftX = (FLOAT)vpRect.x1;
    vp.TopLeftY = (FLOAT)vpRect.y1;
    vp.Width    = (FLOAT)Alg::Max<int>(vpRect.Width(), 1);
    vp.Height   = (FLOAT)Alg::Max<int>(vpRect.Height(), 1);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    pCommandList->RSSetViewports(1, &vp);

    // D3D12 always applies a scissor rectangle (there is no "scissor off"
    // rasterizer state). It defaults to an empty rect, which would discard
    // every draw, so keep it in sync with the viewport.
    D3D12_RECT scissor;
    scissor.left   = (LONG)vp.TopLeftX;
    scissor.top    = (LONG)vp.TopLeftY;
    scissor.right  = (LONG)(vp.TopLeftX + vp.Width);
    scissor.bottom = (LONG)(vp.TopLeftY + vp.Height);
    pCommandList->RSSetScissorRects(1, &scissor);
}

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmeshBase)
{
    D3D12::MeshCacheItem* pmesh = reinterpret_cast<D3D12::MeshCacheItem*>(pmeshBase);
    ID3D12Resource* pvb          = pmesh->pVertexBuffer->GetHWBuffer();
    ID3D12Resource* pib          = pmesh->pIndexBuffer->GetHWBuffer();

    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pvb->GetGPUVirtualAddress() + pmesh->VBAllocOffset;
    vbView.SizeInBytes    = (UINT)(pmesh->pVertexBuffer->GetSize() - pmesh->VBAllocOffset);
    vbView.StrideInBytes  = pbatch->pFormat->Size;
    pCommandList->IASetVertexBuffers(0, 1, &vbView);

    D3D12_INDEX_BUFFER_VIEW ibView;
    ibView.BufferLocation = pib->GetGPUVirtualAddress();
    ibView.SizeInBytes    = (UINT)pmesh->pIndexBuffer->GetSize();
    ibView.Format         = sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    pCommandList->IASetIndexBuffer(&ibView);
    return pmesh->IBAllocOffset / sizeof(IndexType);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{
    D3D12::MeshCacheItem* pmesh         = reinterpret_cast<D3D12::MeshCacheItem*>(pmeshBase);
    ID3D12Resource* pvb                  = pmesh->pVertexBuffer->GetHWBuffer();
    ID3D12Resource* pib                  = pmesh->pIndexBuffer->GetHWBuffer();
    UPInt byteOffset                     = (UPInt)(fr.VertexByteOffset + pmesh->VBAllocOffset);
    const Render::VertexFormat* pformat  = fr.pFormats[formatIndex];

    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pvb->GetGPUVirtualAddress() + byteOffset;
    vbView.SizeInBytes    = (UINT)(pmesh->pVertexBuffer->GetSize() - byteOffset);
    vbView.StrideInBytes  = pformat->Size;
    pCommandList->IASetVertexBuffers(0, 1, &vbView);

    D3D12_INDEX_BUFFER_VIEW ibView;
    ibView.BufferLocation = pib->GetGPUVirtualAddress();
    ibView.SizeInBytes    = (UINT)pmesh->pIndexBuffer->GetSize();
    ibView.Format         = sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    pCommandList->IASetIndexBuffer(&ibView);
    return (unsigned)(pmesh->IBAllocOffset / sizeof(IndexType));
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------
void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyDepthStencil), __FUNCTION__);

    // The stencil reference is dynamic state in D3D12 (the depth/stencil state
    // itself is baked into the PSO), so it must be recorded on the command list.
    // It used to never be set: masks live in stencil bits 1..N and are tested
    // against the mask level, so with the reference stuck at the default 0 the
    // test always passed and masked content was drawn unmasked - e.g. "gradient
    // text" (a gradient rectangle masked by glyph shapes) showed up as a solid
    // gradient box.
    if (pCommandList && CurrentStencilRefOnList != stencilRef)
    {
        pCommandList->OMSetStencilRef(stencilRef);
        CurrentStencilRefOnList = stencilRef;
    }

    if (mode != CurrentDepthStencilState || CurrentStencilRef != stencilRef)
    {
        CurrentStencilRef = stencilRef;

        if (DepthStencilModeTable[mode].ColorWriteEnable != DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable)
        {
            // applyBlendModeImpl will check the CurrentDepthStencilState, and set CWE appropriately, so set it before calling.
            // Note that we are calling the Impl directly, as applyBlendMode may detect this as a duplicate setting of the same state.
            CurrentDepthStencilState = mode;
            applyBlendModeImpl(CurrentBlendState.Mode, CurrentBlendState.SourceAc, CurrentBlendState.ForceAc);
        }
        CurrentDepthStencilState = mode;
    }
}

bool HAL::checkDepthStencilBufferCaps()
{
    if (!StencilChecked)
    {
        StencilAvailable = 0;
        DepthBufferAvailable = 0;

        if ( RenderTargetStack.GetSize() > 0 )
        {
            RenderTarget* prt = RenderTargetStack.Back().pRenderTarget;
            if (prt)
            {
                D3D12::RenderTargetData* phd = (D3D12::RenderTargetData*)prt->GetRenderTargetData();
                if (phd && phd->DSV.ptr)
                {
                    StencilAvailable = 1;
                    MultiBitStencil  = 1;
                    DepthBufferAvailable = 1;
                }
            }
        }
        StencilChecked = 1;
    }

    if (!StencilAvailable && !DepthBufferAvailable)
    {
        SF_DEBUG_WARNONCE(1, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
        return false;
    }
    return true;
}

void HAL::applyRasterModeImpl(RasterModeType mode)
{
    // The raster mode is part of the pipeline state key; it is resolved lazily
    // at the next draw call. Just record it.
    CurrentRasterMode = (unsigned)mode;
}

//--------------------------------------------------------------------
// *** Blend / DepthStencil / Raster descriptions (for PSO creation)
//--------------------------------------------------------------------

// Structure describing color combines applied for a given blend mode.
struct D3D12_BlendModeDesc
{
    D3D12_BLEND_OP  BlendOp;
    D3D12_BLEND     SrcArg, DestArg;
    D3D12_BLEND     SrcAlphaArg, DestAlphaArg;
};

void HAL::initBlendDescriptions()
{
    static const D3D12_BLEND_OP BlendOps[BlendOp_Count] =
    {
        D3D12_BLEND_OP_ADD,            // BlendOp_ADD
        D3D12_BLEND_OP_MAX,            // BlendOp_MAX
        D3D12_BLEND_OP_MIN,            // BlendOp_MIN
        D3D12_BLEND_OP_REV_SUBTRACT,   // BlendOp_REVSUBTRACT
    };

    static const D3D12_BLEND BlendFactors[BlendFactor_Count] =
    {
        D3D12_BLEND_ZERO,              // BlendFactor_ZERO
        D3D12_BLEND_ONE,               // BlendFactor_ONE
        D3D12_BLEND_SRC_ALPHA,         // BlendFactor_SRCALPHA
        D3D12_BLEND_INV_SRC_ALPHA,     // BlendFactor_INVSRCALPHA
        D3D12_BLEND_DEST_COLOR,        // BlendFactor_DESTCOLOR
        D3D12_BLEND_INV_DEST_COLOR,    // BlendFactor_INVDESTCOLOR
    };

    for (unsigned i = 0; i < BlendTypeCount; ++i )
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rt0 = BlendDescs[i];
        rt0 = D3D12_DisabledRTBlend();
        unsigned mode = i;
        bool sourceAc = false;

        if ( i >= Blend_Count*2 )
        {
            // Anything above normal blends has write mask disabled.
            rt0.BlendEnable           = FALSE;
            rt0.RenderTargetWriteMask = 0;
        }
        else
        {
            rt0.BlendEnable           = TRUE;
            rt0.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            if ( i >= Blend_Count )
            {
                mode -= Blend_Count;
                sourceAc = true;
            }
        }
        mode %= Blend_Count;

        rt0.BlendOp        = BlendOps[BlendModeTable[mode].Operator];
        rt0.SrcBlend       = BlendFactors[BlendModeTable[mode].SourceColor];
        rt0.DestBlend      = BlendFactors[BlendModeTable[mode].DestColor];
        rt0.BlendOpAlpha   = BlendOps[BlendModeTable[mode].AlphaOperator];
        rt0.SrcBlendAlpha  = BlendFactors[BlendModeTable[mode].SourceAlpha];
        rt0.DestBlendAlpha = BlendFactors[BlendModeTable[mode].DestAlpha];

        if ( sourceAc && rt0.SrcBlend == D3D12_BLEND_SRC_ALPHA )
            rt0.SrcBlend = D3D12_BLEND_ONE;
    }
}

void HAL::initDepthStencilDescriptions()
{
    // Mapping tables.
    static const D3D12_COMPARISON_FUNC ComparisonFunctions[DepthStencilFunction_Count] =
    {
        D3D12_COMPARISON_FUNC_ALWAYS,           // DepthStencilFunction_Ignore,
        D3D12_COMPARISON_FUNC_NEVER,            // DepthStencilFunction_Never,
        D3D12_COMPARISON_FUNC_LESS,             // DepthStencilFunction_Less,
        D3D12_COMPARISON_FUNC_EQUAL,            // DepthStencilFunction_Equal,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,       // DepthStencilFunction_LessEqual,
        D3D12_COMPARISON_FUNC_GREATER,          // DepthStencilFunction_Greater,
        D3D12_COMPARISON_FUNC_NOT_EQUAL,        // DepthStencilFunction_NotEqual,
        D3D12_COMPARISON_FUNC_GREATER_EQUAL,    // DepthStencilFunction_GreaterEqual,
        D3D12_COMPARISON_FUNC_ALWAYS,           // DepthStencilFunction_Always
    };

    static const D3D12_STENCIL_OP StencilOperations[StencilOp_Count] =
    {
        D3D12_STENCIL_OP_KEEP,     // StencilOp_Ignore,
        D3D12_STENCIL_OP_KEEP,     // StencilOp_Keep,
        D3D12_STENCIL_OP_REPLACE,  // StencilOp_Replace,
        D3D12_STENCIL_OP_INCR,     // StencilOp_Increment
    };

    for ( unsigned state = 0; state < DepthStencil_Count; ++state)
    {
        D3D12_DEPTH_STENCIL_DESC& desc = DepthStencilDescs[state];
        memset(&desc, 0, sizeof(desc));

        const HALDepthStencilDescriptor& mode = DepthStencilModeTable[state];

        desc.StencilEnable                = mode.StencilEnable ? TRUE : FALSE;
        desc.DepthEnable                  = (mode.DepthTestEnable || mode.DepthWriteEnable) ? TRUE : FALSE;
        desc.DepthWriteMask               = mode.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc                    = ComparisonFunctions[mode.DepthFunction];
        desc.StencilReadMask              = 0xFF;
        desc.StencilWriteMask             = 0xFF;
        desc.FrontFace.StencilFunc        = ComparisonFunctions[mode.StencilFunction];
        desc.FrontFace.StencilPassOp      = StencilOperations[mode.StencilPassOp];
        desc.FrontFace.StencilFailOp      = StencilOperations[mode.StencilFailOp];
        desc.FrontFace.StencilDepthFailOp = StencilOperations[mode.StencilZFailOp];

        // Make backface identical to front face.
        desc.BackFace = desc.FrontFace;
    }
}

void HAL::initRasterDescriptions()
{
    for ( unsigned state = 0; state < RasterMode_Count; ++state )
    {
        D3D12_RASTERIZER_DESC& rsdesc = RasterDescs[state];
        memset(&rsdesc, 0, sizeof(rsdesc));
        rsdesc.FillMode              = (state == RasterMode_Wireframe) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
        rsdesc.CullMode              = D3D12_CULL_MODE_NONE;
        rsdesc.FrontCounterClockwise = FALSE;
        rsdesc.DepthBias             = 0;
        rsdesc.DepthBiasClamp        = 0.0f;
        rsdesc.SlopeScaledDepthBias  = 0.0f;
        rsdesc.DepthClipEnable       = FALSE;
        rsdesc.MultisampleEnable     = FALSE;
        rsdesc.AntialiasedLineEnable = FALSE;
        rsdesc.ForcedSampleCount     = 0;
        rsdesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }
}

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc )
{
    SF_UNUSED3(mode, sourceAc, forceAc);
    // The blend state is part of the pipeline state object; it is resolved
    // lazily at the next draw call through the PSO key.
    PipelineDirty = true;
}

// *** Render targets

RenderTarget*   HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    D3D12::Texture* pt = (D3D12::Texture*)texture;

    // Cannot render to textures which have multiple HW representations.
    if ( !pt || pt->TextureCount != 1 )
        return 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;

    Ptr<DepthStencilBuffer> pdsb = 0;
    ID3D12Resource* pdsResource  = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE dsView;
    dsView.ptr = 0;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize(), false); // false == not temporary
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D12::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
            {
                dsView = surf->DSV;
                pdsResource = surf->pDepthStencilSurface;
            }
        }
    }

    DXGI_FORMAT format = pt->GetTextureFormat() ? pt->GetTextureFormat()->GetD3DFormat()
                                                : DXGI_FORMAT_R8G8B8A8_UNORM;
    RenderTargetData::UpdateData(prt, pt->pTextures[0].pTexture, pt->pTextures[0].RTV,
                                 format, pdsb, pdsResource, dsView);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( IsDebugEnvSet("GFX_D3D12_RTDBG") )
    {
        static int sLog = 0;
        if ((sLog++ % 20) == 0)
        {
            fprintf(stderr, "[RTDBG] %ux%u stencil=%d rtv=%llu srv=%llu dsv=%llu\n",
                    size.Width, size.Height, (int)needsStencil,
                    (unsigned long long)pTextureManager->RTVHeapCursor,
                    (unsigned long long)pTextureManager->SRVHeapCursor,
                    (unsigned long long)pTextureManager->DSVHeapCursor);
            fflush(stderr);
        }
    }
    if ( !prt )
    {
        static int sWarned = 0;
        if (sWarned++ < 16)
        {
            fprintf(stderr, "[D3D12RT] temp target alloc failed %ux%u stencil=%d\n",
                    size.Width, size.Height, (int)needsStencil);
            fflush(stderr);
        }
        return 0;
    }

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilBuffer))
        return prt;

    D3D12::Texture* pt = (D3D12::Texture*)prt->GetTexture();
    SF_ASSERT(pt->TextureCount == 1);

    Ptr<DepthStencilBuffer> pdsb = 0;
    ID3D12Resource* pdsResource  = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE dsView;
    dsView.ptr = 0;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D12::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
            {
                dsView = surf->DSV;
                pdsResource = surf->pDepthStencilSurface;
            }
        }
    }

    DXGI_FORMAT format = pt->GetTextureFormat() ? pt->GetTextureFormat()->GetD3DFormat()
                                                : DXGI_FORMAT_B8G8R8A8_UNORM;
    RenderTargetData::UpdateData(prt, pt->pTextures[0].pTexture, pt->pTextures[0].RTV,
                                 format, pdsb, pdsResource, dsView);
    return prt;
}

void HAL::transitionRenderTarget(RenderTarget* ptarget, bool toRenderTarget)
{
    if (!ptarget || !pCommandList)
        return;
    RenderTargetData* phd = (D3D12::RenderTargetData*)ptarget->GetRenderTargetData();
    if (!phd || !phd->pRenderSurface)
        return;

    D3D12_RESOURCE_STATES after;
    if (ptarget->GetType() == RBuffer_Default)
        after = toRenderTarget ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT;
    else
        after = toRenderTarget ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    if (phd->ResourceState == after)
        return;

    D3D12_RecordBarrier(pCommandList, phd->pRenderSurface, phd->ResourceState, after);
    phd->ResourceState = after;
}

bool HAL::SetRenderTarget(RenderTarget* ptarget, bool setState)
{
    // When changing the render target while in a scene, we must flush all drawing.
    if ( HALState & HS_InScene)
        Flush();

    // Cannot set the bottom level render target if already in display.
    if ( HALState & HS_InDisplay )
        return false;

    RenderTargetEntry entry;
    RenderTargetData* prtdata = (D3D12::RenderTargetData*)ptarget->GetRenderTargetData();
    if ( !prtdata )
        return false;

    if ( setState )
    {
        transitionRenderTarget(ptarget, true);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = prtdata->RTV;
        D3D12_CPU_DESCRIPTOR_HANDLE* pdsv = prtdata->DSV.ptr ? &prtdata->DSV : 0;
        pCommandList->OMSetRenderTargets(1, &rtv, FALSE, pdsv);
        CurrentRTFormat = prtdata->Format;
        CurrentDSFormat = pdsv ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN;
        PipelineDirty = true;
    }

    entry.pRenderTarget = ptarget;

    // Replace the stack entry at the bottom, or if the stack is empty, add one.
    if ( RenderTargetStack.GetSize() > 0 )
        RenderTargetStack[0] = entry;
    else
        RenderTargetStack.PushBack(entry);
    return true;
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags, Color clearColor)
{
    GetEvent(Event_RenderTarget).Begin(__FUNCTION__);

    HALState |= HS_InRenderTarget;
    RenderTargetEntry entry;
    entry.pRenderTarget = prt;
    entry.OldViewport = VP;
    entry.OldViewRect = ViewRect;
    entry.OldMatrixState.CopyFrom(Matrices);
    Matrices->Orient2D.SetIdentity();
    Matrices->Orient3D.SetIdentity();
    Matrices->SetUserMatrix(Matrix2F::Identity);

    // Setup the render target/depth stencil on the device.
    if ( !prt )
    {
        SF_DEBUG_WARNING(1, __FUNCTION__ " - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }
    RenderTargetData* phd = (D3D12::RenderTargetData*)prt->GetRenderTargetData();
    if ( !phd )
    {
        RenderTargetStack.PushBack(entry);
        return;
    }

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    StageTextureCount = 0;

    transitionRenderTarget(prt, true);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = phd->RTV;
    D3D12_CPU_DESCRIPTOR_HANDLE* pdsv = phd->DSV.ptr ? &phd->DSV : 0;
    pCommandList->OMSetRenderTargets(1, &rtv, FALSE, pdsv);
    CurrentRTFormat = phd->Format;
    CurrentDSFormat = pdsv ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN;
    PipelineDirty = true;
    StencilChecked = false;
    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        float clear[4];
        clearColor.GetRGBAFloat(clear);
        pCommandList->ClearRenderTargetView(rtv, clear, 0, 0);
        if (pdsv)
            pCommandList->ClearDepthStencilView(*pdsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                                1.0f, 0, 0, 0);
    }

    // Setup viewport.
    Rect<int> viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
    const ImageSize& bs = prt->GetBufferSize();
    VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());
    ViewRect.x1 = (int)frameRect.x1;
    ViewRect.y1 = (int)frameRect.y1;
    ViewRect.x2 = (int)frameRect.x2;
    ViewRect.y2 = (int)frameRect.y2;

    // Must offset the 'original' viewrect, otherwise the 3D compensation matrix will be offset.
    Matrices->ViewRectOriginal.Offset(-entry.OldViewport.Left, -entry.OldViewport.Top);
    Matrices->UVPOChanged = true;
    HALState |= HS_ViewValid;
    updateViewport();

    RenderTargetStack.PushBack(entry);
}

void HAL::PopRenderTarget(unsigned flags)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_RenderTarget), 0, false);

    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;
    Ptr<DepthStencilBuffer> retiredDS;
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        D3D12::RenderTargetData* plasthd = (D3D12::RenderTargetData*)prt->GetRenderTargetData();
        if ( plasthd )
        {
            // Keep the DS buffer alive until the frame is fenced (see the
            // whole-target retirement below); the resource itself is also parked
            // in the texture kill list.
            retiredDS = plasthd->pDepthStencilBuffer;
            // The depth/stencil resource was referenced by the command list that
            // is being recorded for this frame (clears, masked draws). Releasing
            // it here destroys it before the list is executed - the debug layer
            // reports id=921 and the GPU can hang on the stale resource. Park the
            // resource in the texture kill list, which BeginFrame drains only
            // after the frame fence (same lifetime rule as Vulkan's
            // PendingDSDeletes).
            if ( plasthd->pDepthStencilBuffer && pTextureManager )
            {
                DepthStencilSurface* psurf =
                    (DepthStencilSurface*)plasthd->pDepthStencilBuffer->GetSurface();
                if ( psurf && psurf->pDepthStencilSurface )
                    pTextureManager->D3DTextureKillList.PushBack(psurf->pDepthStencilSurface);
            }
            plasthd->pDSSurface = 0;
            plasthd->pDepthStencilBuffer = 0;
        }
    }
    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();

    // Whole-target retirement: keep the temporary target (and its DS) out of the
    // buffer manager's reuse lists until the next frame's fence. BeginFrame
    // clears this list; only then may the manager hand the target out again.
    if (prt && prt->GetType() == RBuffer_Temporary)
        PendingTargetReleases.PushBack(DeferredTargetRelease(prt, retiredDS));

    // The popped target is still bound to the output-merger stage at this point.
    // It must be unbound *before* it is transitioned to PIXEL_SHADER_RESOURCE:
    // leaving it bound as an RTV while its resource state is a shader-resource
    // state is invalid (and made the NVIDIA driver hang intermittently when the
    // 9-1-B filter passes ping-ponged their temporary targets). The parent target
    // is re-bound below when flags does not include PRT_NoSet.
    if (pCommandList && prt)
        pCommandList->OMSetRenderTargets(0, NULL, FALSE, NULL);

    // Transition the popped target back so it can be sampled as a texture.
    // This must also happen for PRT_NoSet: the filter passes pop their
    // intermediate targets with that flag (to skip rebinding the parent
    // target), yet those targets are sampled by the next pass and by the
    // final composition. Leaving them in RENDER_TARGET state made the
    // shader read undefined/compressed data - the glow came out empty.
    if (prt)
        transitionRenderTarget(prt, false);

    D3D12_CPU_DESCRIPTOR_HANDLE* pds = 0;
    RenderTargetData* phd = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        phd = (D3D12::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
        if ( phd && phd->DSV.ptr )
            pds = &phd->DSV;
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    StageTextureCount = 0;

    // Restore the old render target on the device.
    if ((flags & PRT_NoSet) == 0)
    {
        if ( phd )
        {
            transitionRenderTarget(RenderTargetStack.Back().pRenderTarget, true);
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = phd->RTV;
            pCommandList->OMSetRenderTargets(1, &rtv, FALSE, pds);
            CurrentRTFormat = phd->Format;
            CurrentDSFormat = pds ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_UNKNOWN;
            PipelineDirty = true;
            ++AccumulatedStats.RTChanges;
        }

        // Reset the viewport to the last render target on the stack.
        HALState |= HS_ViewValid;
        updateViewport();
    }
}

void HAL::drawFilter(const Matrix2F& mvp, const Cxform& cx, const Filter* filter,
                     Ptr<RenderTarget>* targets, unsigned* shaders, unsigned pass,
                     unsigned passCount, const VertexFormat* pvf, BlurFilterState& leBlur)
{
    // D3D12 analogue of the Vulkan "resume render pass / image layout" fix:
    // every target that this pass samples must be in the shader-resource state.
    // The destination was just pushed and is the current render target, so it is
    // deliberately skipped. The filter source (the sprite's own target) is still
    // in RENDER_TARGET state on the first pass; without this barrier the shader
    // sampled undefined data and the glow (and the blurred copy) came out empty.
    RenderTarget* pCurrent = RenderTargetStack.GetSize() ? RenderTargetStack.Back().pRenderTarget.GetPtr() : 0;
    for (unsigned i = 0; i < Target_Count; ++i)
    {
        RenderTarget* prt = targets[i];
        if (!prt || prt == pCurrent)
            continue;
        transitionRenderTarget(prt, false);
    }

    // A feedback loop (sampling the render target that is currently bound) hangs
    // the GPU. Target_Destination is the current target by design and is not
    // sampled, but Source / Original must never alias it.
    for (unsigned i = 0; i < Target_Count; ++i)
    {
        if (i == Target_Destination)
            continue;
        RenderTarget* prt = targets[i];
        if (prt && prt == pCurrent)
        {
            static int sFbWarned = 0;
            if (sFbWarned++ < 16)
            {
                fprintf(stderr, "[D3D12] filter feedback loop: target %u is the current RT (%ux%u)\n",
                        i, prt->GetBufferSize().Width, prt->GetBufferSize().Height);
                fflush(stderr);
            }
        }
    }

    BaseHAL::drawFilter(mvp, cx, filter, targets, shaders, pass, passCount, pvf, leBlur);
}

void HAL::SetMainRenderTarget(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                              DXGI_FORMAT format, const ImageSize& size,
                              ID3D12Resource* dsResource, D3D12_CPU_DESCRIPTOR_HANDLE dsv)
{
    MainRT.pResource   = resource;
    MainRT.RTV         = rtv;
    MainRT.Format      = format;
    MainRT.Size        = size;
    MainRT.pDSResource = dsResource;
    MainRT.DSV         = dsv;
    MainRT.Set         = true;

    RenderTarget* prt = GetDefaultRenderTarget();
    if (prt && prt->GetType() == RBuffer_Default)
    {
        Ptr<DepthStencilBuffer> pdsb = 0;
        RenderTargetData::UpdateData(prt, resource, rtv, format, pdsb, dsResource, dsv);
        // The swap-chain back buffer is in PRESENT state until the first
        // transition to RENDER_TARGET.
        D3D12::RenderTargetData* phd = (D3D12::RenderTargetData*)prt->GetRenderTargetData();
        if (phd)
            phd->ResourceState = D3D12_RESOURCE_STATE_PRESENT;
    }
}

bool HAL::createDefaultRenderBuffer()
{
    ImageSize rtSize;

    if ( GetDefaultRenderTarget() )
    {
        RenderTarget* prt = GetDefaultRenderTarget();
        rtSize = prt->GetSize();
    }
    else
    {
        // The platform provides the swapchain back buffer through SetMainRenderTarget.
        if ( MainRT.Set && MainRT.pResource )
        {
            rtSize = MainRT.Size;
            Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize);
            if ( !ptarget )
                return false;

            Ptr<DepthStencilBuffer> pdsb = 0;
            RenderTargetData::UpdateData(ptarget, MainRT.pResource, MainRT.RTV, MainRT.Format,
                                         pdsb, MainRT.pDSResource, MainRT.DSV);

            if ( !SetRenderTarget(ptarget))
                return false;
        }
        else
        {
            // No swapchain target provided yet; create an offscreen default target.
            ImageSize size(Alg::Max<UInt32>(1, ViewRect.Width()), Alg::Max<UInt32>(1, ViewRect.Height()));
            if (size.Width <= 1 || size.Height <= 1)
                size = ImageSize(256, 256);

            Ptr<Render::Texture> ptexture =
                *pTextureManager->CreateTexture(Image_R8G8B8A8, 1, size, ImageUse_RenderTarget, 0);
            if (!ptexture)
                return false;

            Ptr<RenderTarget> ptarget = *pRenderBufferManager->CreateRenderTarget(
                size, RBuffer_Default, Image_R8G8B8A8, ptexture);
            if (!ptarget)
                return false;

            D3D12::Texture* pt = (D3D12::Texture*)ptexture.GetPtr();
            Ptr<DepthStencilBuffer> pdsb = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE dsv;
            dsv.ptr = 0;
            RenderTargetData::UpdateData(ptarget, pt->pTextures[0].pTexture, pt->pTextures[0].RTV,
                                         pt->GetTextureFormat()->GetD3DFormat(), pdsb, 0, dsv);
            if ( !SetRenderTarget(ptarget))
                return false;
        }
    }

    return pRenderBufferManager->Initialize(pTextureManager, Image_R8G8B8A8, rtSize);
}

void HAL::setBatchUnitSquareVertexStream()
{
    static const UINT stride = sizeof(VertexXY16fAlpha);
    ID3D12Resource* pb = Cache.pMaskEraseBatchVertexBuffer.GetPtr();
    if (!pb)
        return;

    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pb->GetGPUVirtualAddress();
    vbView.SizeInBytes    = (UINT)pb->GetDesc().Width;
    vbView.StrideInBytes  = stride;
    pCommandList->IASetVertexBuffers(0, 1, &vbView);
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    preparePipeline();
    pCommandList->DrawInstanced(indexCount, 1, 0, 0);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    preparePipeline();
    pCommandList->DrawIndexedInstanced(indexCount, 1, (UINT)indexPtr, (INT)vertexOffset, 0);

    SF_UNUSED2(meshCount, vertexCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    preparePipeline();
    pCommandList->DrawIndexedInstanced(indexCount, (UINT)meshCount, (UINT)indexPtr, (INT)vertexOffset, 0);

    SF_UNUSED(vertexCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}

bool HAL::IsFilterCachingSupported() const
{
    // Same fix as Vulkan (CheckPoint 2026/10/29 22:05, root cause 1): the cached
    // filter results are re-acquired by the cacheable prepare pass every frame,
    // but ShaderHAL::drawCachedFilter never consumes them on D3D12 either. The
    // cached target therefore stays "in use", the glow never reaches the screen
    // and the temporary targets are still referenced at frame end.
    // Rebuilding every filter every frame is correct and matches the other
    // back-ends; caching stays opt-in only for debugging it.
    static const bool sFilterCache = Render::IsDebugEnvSet("GFX_D3D12_FILTER_CACHE");
    return sFilterCache;
}

namespace {

struct D3D12FilterName
{
    const char* pName;
    FilterType  Type;
};

// Same names as the Vulkan bisect switch (CheckPoint 2026/10/29 22:05) so both
// back-ends can be compared with identical GFX_*_FILTER_ALLOW values.
static const D3D12FilterName D3D12FilterNames[] =
{
    { "blur",            Filter_Blur },
    { "shadow",          Filter_Shadow },
    { "glow",            Filter_Glow },
    { "bevel",           Filter_Bevel },
    { "gradientglow",    Filter_GradientGlow },
    { "gradientbevel",   Filter_GradientBevel },
    { "gradglow",        Filter_GradientGlow },
    { "gradbevel",       Filter_GradientBevel },
    { "colormatrix",     Filter_ColorMatrix },
    { "cmatrix",         Filter_ColorMatrix },
    { "convolution",     Filter_Convolution },
    { "displacementmap", Filter_DisplacementMap },
    { "displacement",    Filter_DisplacementMap },
    { "cacheasbitmap",   Filter_CacheAsBitmap },
};

// Bit mask of allowed FilterType values; all-ones means every type is allowed.
// GFX_D3D12_FILTER_ALLOW=none disables filtering entirely, a comma separated
// list (e.g. "shadow,glow") restricts it to those types for bisecting.
static unsigned GetD3D12FilterAllowMask()
{
    static unsigned sMask = 0xFFFFFFFFu;
    static bool     sInit = false;
    if (sInit)
        return sMask;
    sInit = true;

    char envValue[256];
    if (!GetDebugEnvValue("GFX_D3D12_FILTER_ALLOW", envValue, sizeof(envValue)) || !envValue[0])
        return sMask;

    sMask = 0;
    const char* p = envValue;
    while (*p)
    {
        const char* pEnd = p;
        while (*pEnd && *pEnd != ',' && *pEnd != ';' && *pEnd != ' ')
            ++pEnd;
        const size_t len = (size_t)(pEnd - p);

        if (len == 4 && !_strnicmp(p, "none", 4))
            return sMask;

        for (size_t i = 0; i < sizeof(D3D12FilterNames) / sizeof(D3D12FilterNames[0]); ++i)
        {
            if (strlen(D3D12FilterNames[i].pName) == len &&
                !_strnicmp(p, D3D12FilterNames[i].pName, len))
                sMask |= (1u << D3D12FilterNames[i].Type);
        }

        p = pEnd;
        while (*p == ',' || *p == ';' || *p == ' ')
            ++p;
    }
    return sMask;
}

} // anonymous namespace

bool HAL::shouldRenderFilters(const FilterPrimitive* prim) const
{
    // Mirrors the Vulkan Decision (CheckPoint 2026/10/29 22:12): filters are on
    // by default now that the multi-pass temp-RT path is fixed. GFX_D3D12_NOFILTERS=1
    // restores the old "no filters" baseline, GFX_D3D12_FILTER_ALLOW restricts the
    // set of rendered filter types for bisecting.
    static const bool sNoFilters = Render::IsDebugEnvSet("GFX_D3D12_NOFILTERS");
    if (sNoFilters)
        return false;

    const FilterSet* pfilters = prim ? prim->GetFilters() : 0;
    if (!pfilters || pfilters->GetFilterCount() == 0)
        return false;

    const unsigned filterCount = (unsigned)pfilters->GetFilterCount();
    // Debug A/B: GFX_D3D12_NOCACHEBMP=1 skips cacheAsBitmap primitives (the
    // drawCachedFilter path), to isolate it from the uncached multi-pass path.
    static const bool sNoCacheBmp = Render::IsDebugEnvSet("GFX_D3D12_NOCACHEBMP");
    if (sNoCacheBmp)
    {
        for (unsigned i = 0; i < filterCount; ++i)
            if (pfilters->GetFilter(i)->GetFilterType() == Filter_CacheAsBitmap)
                return false;
    }
    const unsigned allowMask   = GetD3D12FilterAllowMask();
    if (allowMask != 0xFFFFFFFFu)
    {
        for (unsigned i = 0; i < filterCount; ++i)
        {
            const Filter* pf = pfilters->GetFilter(i);
            if (!pf)
                return false;
            const unsigned type = (unsigned)pf->GetFilterType();
            if (type < 32 && (allowMask & (1u << type)) == 0)
                return false;
        }
    }

    // One line per filtered primitive (capped), so a black/garbled frame can be
    // traced back to the exact filter type and area size. Same idea as [VKFLT].
    static const bool sFilterLog = Render::IsDebugEnvSet("GFX_D3D12_FLTLOG");
    if (sFilterLog && prim)
    {
        static unsigned sLogged = 0;
        if (sLogged < 20000)
        {
            const bool      hasArea = prim->GetAreaMatrixCount() > 0;
            const Matrix2F* pArea   = hasArea ? &prim->GetAreaMatrix().GetMatrix2D() : 0;
            float blurX = 0.0f, blurY = 0.0f, strength = 0.0f;
            unsigned passes = 0, mode = 0;
            const Filter* pf0 = pfilters->GetFilter(0);
            if (pf0 && pf0->GetFilterType() <= Filter_Blur_End)
            {
                const BlurFilterParams& bp = ((const BlurFilter*)pf0)->GetParams();
                blurX    = TwipsToPixels(bp.BlurX);
                blurY    = TwipsToPixels(bp.BlurY);
                strength = bp.Strength;
                passes   = bp.Passes;
                mode     = bp.Mode;
            }
            fprintf(stderr, "[D3D12FLT] n=%u t0=%d t1=%d area=%.0fx%.0f at=%.0f,%.0f cache=%d"
                            " blur=%.0fx%.0f str=%.1f passes=%u mode=0x%X\n",
                    filterCount,
                    (filterCount > 0) ? (int)pfilters->GetFilter(0)->GetFilterType() : -1,
                    (filterCount > 1) ? (int)pfilters->GetFilter(1)->GetFilterType() : -1,
                    pArea ? pArea->Sx() : 0.0f, pArea ? pArea->Sy() : 0.0f,
                    pArea ? pArea->Tx() : 0.0f, pArea ? pArea->Ty() : 0.0f,
                    (int)prim->GetCacheState(),
                    blurX, blurY, strength, passes, mode);
            fflush(stderr);
            ++sLogged;
        }
    }

    return true;
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
    return Events[eventType];
}

void HAL::drawScreenQuad()
{
    ID3D12Resource* pb = Cache.pMaskEraseBatchVertexBuffer.GetPtr();
    static const unsigned stride = sizeof(VertexXY16fAlpha);
    if (!pb)
        return;

    D3D12_VERTEX_BUFFER_VIEW vbView;
    vbView.BufferLocation = pb->GetGPUVirtualAddress();
    vbView.SizeInBytes    = (UINT)pb->GetDesc().Width;
    vbView.StrideInBytes  = stride;
    pCommandList->IASetVertexBuffers(0, 1, &vbView);
    drawPrimitive(6,1);
}

// *** Pipeline management

bool HAL::createRootSignature()
{
    // Root parameters:
    //   0: CBV b0 (vertex constant buffer)  - root descriptor
    //   1: CBV b0 (fragment constant buffer) - root descriptor
    //   2: Descriptor table t0..t7 (texture SRVs)
    //   3: Descriptor table s0..s7 (samplers)
    D3D12_ROOT_PARAMETER rootParams[4];
    memset(rootParams, 0, sizeof(rootParams));

    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;

    rootParams[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 1;
    rootParams[1].Descriptor.RegisterSpace  = 0;
    rootParams[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_DESCRIPTOR_RANGE srvRange;
    srvRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors                    = 8;
    srvRange.BaseShaderRegister                = 0;
    srvRange.RegisterSpace                     = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges   = &srvRange;
    rootParams[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_DESCRIPTOR_RANGE samplerRange;
    samplerRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    samplerRange.NumDescriptors                    = 8;
    samplerRange.BaseShaderRegister                = 0;
    samplerRange.RegisterSpace                     = 0;
    samplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[3].DescriptorTable.pDescriptorRanges   = &samplerRange;
    rootParams[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
    memset(&rootSigDesc, 0, sizeof(rootSigDesc));
    rootSigDesc.NumParameters     = 4;
    rootSigDesc.pParameters       = rootParams;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers   = 0;
    rootSigDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // Serialize and create the root signature.
    Ptr<ID3DBlob> pSignatureBlob;
    Ptr<ID3DBlob> pErrorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             &pSignatureBlob.GetRawRef(), &pErrorBlob.GetRawRef());
    if (FAILED(hr))
    {
        if (pErrorBlob && pErrorBlob->GetBufferSize())
            SF_DEBUG_ERROR1(1, "Failed to serialize D3D12 root signature: %s",
                            (const char*)pErrorBlob->GetBufferPointer());
        return false;
    }

    hr = pDevice->CreateRootSignature(0, pSignatureBlob->GetBufferPointer(),
                                      pSignatureBlob->GetBufferSize(),
                                      IID_PPV_ARGS(&pRootSignature));
    return SUCCEEDED(hr);
}

void HAL::destroyRootSignature()
{
    if (pRootSignature)
    {
        pRootSignature->Release();
        pRootSignature = 0;
    }
}

ID3D12PipelineState* HAL::GetPipeline(unsigned vsIndex, unsigned fsIndex,
                                      unsigned blendType, unsigned stencilMode, unsigned rasterMode,
                                      const SysVertexFormat* pvf, DXGI_FORMAT rtFormat, DXGI_FORMAT dsFormat)
{
    PipelineKey key;
    key.VSIndex     = vsIndex;
    key.FSIndex     = fsIndex;
    key.BlendType   = blendType;
    key.StencilMode = stencilMode;
    key.RasterMode  = rasterMode;
    key.pVFormat    = pvf;
    key.RTFormat    = rtFormat;
    key.DSFormat    = dsFormat;

    ID3D12PipelineState** existing = Pipelines.Get(key);
    if (existing)
        return *existing;

    if (!pRootSignature || vsIndex >= (unsigned)VertexShaderDesc::VSI_Count ||
        fsIndex >= (unsigned)FragShaderDesc::FSI_Count || !pvf)
        return 0;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.pRootSignature = pRootSignature;
    desc.VS             = SManager.StaticVShaders[vsIndex].Bytecode;
    desc.PS             = SManager.StaticFShaders[fsIndex].Bytecode;
    desc.InputLayout    = { pvf->VertexElements, pvf->VertexElementCount };
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0]    = rtFormat;
    desc.DSVFormat        = dsFormat;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.SampleMask         = 0xFFFFFFFF;
    desc.BlendState.AlphaToCoverageEnable  = FALSE;
    desc.BlendState.IndependentBlendEnable = FALSE;
    desc.BlendState.RenderTarget[0]        = (blendType < BlendTypeCount) ? BlendDescs[blendType] : BlendDescs[0];
    for (unsigned i = 1; i < 8; i++)
        desc.BlendState.RenderTarget[i] = D3D12_DisabledRTBlend();
    desc.DepthStencilState = (stencilMode < DepthStencil_Count) ? DepthStencilDescs[stencilMode] : DepthStencilDescs[0];
    desc.RasterizerState   = (rasterMode < RasterMode_Count) ? RasterDescs[rasterMode] : RasterDescs[0];

    ID3D12PipelineState* pso = 0;
    HRESULT hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
    if (FAILED(hr) || !pso)
    {
        SF_DEBUG_WARNING1(1, "D3D12: Failed to create graphics pipeline state (hr=0x%08x)", (unsigned)hr);
        return 0;
    }

    Pipelines.Set(key, pso);
    return pso;
}

void HAL::destroyPipelines()
{
    for (PipelineMap::Iterator it = Pipelines.Begin(); it != Pipelines.End(); ++it)
    {
        if (it->Second)
            it->Second->Release();
    }
    Pipelines.Clear();
    pBoundPipeline = 0;
    PipelineDirty = true;
}

// *** Uniform ring buffer

bool HAL::createUniformBuffer()
{
    UniformBufferSize = 16 * 1024 * 1024; // 16 MB ring: 2048 draws per frame.
    UniformBufferOffset = 0;

    D3D12_HEAP_PROPERTIES heapProps;
    memset(&heapProps, 0, sizeof(heapProps));
    heapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment          = 0;
    resDesc.Width              = UniformBufferSize;
    resDesc.Height             = 1;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = 1;
    resDesc.Format             = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count   = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                NULL, IID_PPV_ARGS(&pUniformBuffer))))
        return false;

    pUniformBuffer->SetName(L"GFx_UniformRing");
    UniformBufferGPUBase = pUniformBuffer->GetGPUVirtualAddress();

    D3D12_RANGE range;
    range.Begin = 0;
    range.End   = (SIZE_T)UniformBufferSize;
    return SUCCEEDED(pUniformBuffer->Map(0, &range, (void**)&pUniformData));
}

void HAL::destroyUniformBuffer()
{
    if (pUniformBuffer)
    {
        if (pUniformData)
        {
            pUniformBuffer->Unmap(0, 0);
            pUniformData = 0;
        }
        pUniformBuffer->Release();
        pUniformBuffer = 0;
    }
    UniformBufferGPUBase = 0;
    UniformBufferSize = 0;
    UniformBufferOffset = 0;
}

UByte* HAL::MapUniformBuffer(D3D12_GPU_VIRTUAL_ADDRESS* pGPU, UPInt size)
{
    size = (size + 255) & ~(UPInt)255;
    if (!pUniformData || !pUniformBuffer)
        return 0;

    // Ring allocation. The ring is reset once per frame (after the frame fence),
    // so a wrap here means the frame recorded more draws than the ring can hold.
    // That is a correctness hazard (constants are read when the list executes),
    // so make it loud instead of silently corrupting earlier draws.
    UPInt offset = UniformBufferOffset;
    UniformBufferOffset += size;
    if (UniformBufferOffset > UniformBufferSize)
    {
        static int sWrapWarned = 0;
        if (sWrapWarned++ < 8)
        {
            fprintf(stderr, "[D3D12] uniform ring wrapped (%llu draws/frame needed); "
                            "raise UniformBufferSize\n",
                    (unsigned long long)(UniformBufferSize / 8192));
            fflush(stderr);
        }
        UniformBufferOffset = size;
        offset = 0;
    }

    if (pGPU)
        *pGPU = UniformBufferGPUBase + offset;
    return pUniformData + offset;
}

void HAL::SetCurrentConstantBuffers(D3D12_GPU_VIRTUAL_ADDRESS vs, D3D12_GPU_VIRTUAL_ADDRESS fs)
{
    CurrentVSAddress = vs;
    CurrentFSAddress = fs;
}

// *** Stage texture binding
void HAL::SetStageTexture(unsigned stage, D3D12_CPU_DESCRIPTOR_HANDLE srvCPU,
                          D3D12_GPU_DESCRIPTOR_HANDLE srvGPU,
                          D3D12_CPU_DESCRIPTOR_HANDLE samplerCPU,
                          D3D12_GPU_DESCRIPTOR_HANDLE samplerGPU)
{
    // srvGPU is not needed: the SRV is copied into the per-draw scratch range
    // and the table is bound from there.
    SF_UNUSED(srvGPU);
    if (stage >= 4)
        return;
    StageTextures[stage].SRV        = srvCPU;
    StageTextures[stage].Sampler    = samplerCPU;
    StageTextures[stage].SamplerGPU = samplerGPU;
    if (stage + 1 > StageTextureCount)
        StageTextureCount = stage + 1;
}

void HAL::bindStageTextures()
{
    if (!pTextureManager || !pCommandList)
        return;
    if (StageTextureCount == 0)
        return;

    TextureManager* pmgr  = pTextureManager;
    const unsigned  count = StageTextureCount < 4 ? StageTextureCount : 4;

    // Pick a stage that carries a texture; stages without one alias it so that
    // every entry of the staged table is a valid, initialized SRV.
    unsigned validStage = count;
    for (unsigned i = 0; i < count; i++)
    {
        if (StageTextures[i].SRV.ptr && StageTextures[i].SamplerGPU.ptr)
        {
            validStage = i;
            break;
        }
    }
    if (validStage == count)
        return;

    // The root signature declares the SRV table t0..t7, so the table has to be
    // staged in full. Binding a partially written table (the old code copied
    // only the bound stage count) feeds the shader uninitialized descriptors,
    // which is undefined behavior on the GPU.
    D3D12_CPU_DESCRIPTOR_HANDLE srvSrcs[StageTableSize];
    for (unsigned i = 0; i < StageTableSize; i++)
    {
        unsigned src = (i < count && StageTextures[i].SRV.ptr) ? i : validStage;
        srvSrcs[i] = StageTextures[src].SRV;
    }

    // Batches share their texture set, so reusing the previously staged range
    // avoids almost all of the descriptor copies.
    bool sameAsLast = LastStageValid;
    for (unsigned i = 0; sameAsLast && i < StageTableSize; i++)
    {
        if (LastStagedSRVs[i].ptr != srvSrcs[i].ptr)
            sameAsLast = false;
    }

    if (!sameAsLast)
    {
        // One range per distinct texture set; the ranges are recycled per scene
        // (BeginScene resets the cursor), never within a scene, because draws
        // already recorded in the command list still reference them.
        if (SRVScratchCursor + StageTableSize > TextureManager::SRVScratchDescriptorCount)
            SRVScratchCursor = 0;

        UINT srvIncrement = pDevice->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        ID3D12DescriptorHeap* psrvHeap = pmgr->GetSRVHeap();
        UINT srvTotal = psrvHeap->GetDesc().NumDescriptors;

        D3D12_CPU_DESCRIPTOR_HANDLE dst = psrvHeap->GetCPUDescriptorHandleForHeapStart();
        dst.ptr += (srvTotal - TextureManager::SRVScratchDescriptorCount + SRVScratchCursor) *
                   srvIncrement;

        D3D12_CPU_DESCRIPTOR_HANDLE dsts[StageTableSize];
        for (unsigned i = 0; i < StageTableSize; i++)
        {
            dsts[i] = dst;
            dsts[i].ptr += i * srvIncrement;
        }

        pDevice->CopyDescriptors(StageTableSize, dsts, NULL,
                                 StageTableSize, srvSrcs, NULL,
                                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        LastSRVTableGPU = pmgr->GetSRVScratchBase();
        LastSRVTableGPU.ptr += SRVScratchCursor * srvIncrement;
        for (unsigned i = 0; i < StageTableSize; i++)
            LastStagedSRVs[i] = srvSrcs[i];
        LastStageValid = true;

        SRVScratchCursor += StageTableSize;
    }

    pCommandList->SetGraphicsRootDescriptorTable(2, LastSRVTableGPU);
    // Samplers are persistent: one prebuilt s0..s7 group per fill mode, so the
    // whole sampler table is always valid and no per-draw copy is needed.
    pCommandList->SetGraphicsRootDescriptorTable(3, StageTextures[validStage].SamplerGPU);
}

// *** Draw preparation

void HAL::preparePipeline()
{
    const ShaderInterface::Shader& curShaders = ShaderData.GetCurrentShaders();
    if (!curShaders.pVFormat || !curShaders.pVS || !curShaders.pFS)
    {
        return;
    }

    const SysVertexFormat* pvf = (const SysVertexFormat*)curShaders.pVFormat->pSysFormat.GetPtr();
    if (!pvf)
        return;

    unsigned vsIndex = (unsigned)(curShaders.pVS - SManager.StaticVShaders);
    unsigned fsIndex = (unsigned)(curShaders.pFS - SManager.StaticFShaders);
    unsigned blendType = GetBlendType(CurrentBlendState.Mode,
                                      DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable ? Write_All : Write_None,
                                      CurrentBlendState.SourceAc);
    unsigned stencilMode = (unsigned)CurrentDepthStencilState;
    unsigned rasterMode  = (unsigned)AppliedSceneRasterMode;

    // Create/retrieve the pipeline state object.
    ID3D12PipelineState* pso = GetPipeline(vsIndex, fsIndex, blendType, stencilMode,
                                           rasterMode, pvf, CurrentRTFormat, CurrentDSFormat);
    if (!pso)
        return;

    // Set the descriptor heaps (both must be set before any descriptor table use).
    if (!DescriptorHeapsSet)
    {
        ID3D12DescriptorHeap* heaps[2] = { pTextureManager->GetSRVHeap(), pTextureManager->GetSamplerHeap() };
        pCommandList->SetDescriptorHeaps(2, heaps);
        DescriptorHeapsSet = true;
    }

    pCommandList->SetGraphicsRootSignature(pRootSignature);

    if (pso != pBoundPipeline)
    {
        pCommandList->SetPipelineState(pso);
        pBoundPipeline = pso;
        PipelineDirty = false;
    }

    // Bind the constant buffers for the current draw.
    if (CurrentVSAddress)
        pCommandList->SetGraphicsRootConstantBufferView(0, CurrentVSAddress);
    if (CurrentFSAddress)
        pCommandList->SetGraphicsRootConstantBufferView(1, CurrentFSAddress);

    bindStageTextures();
}

}}} // namespace Scaleform::Render::D3D12
