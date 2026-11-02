/**************************************************************************

Filename    :   Vulkan_HAL.h
Content     :   Vulkan Hardware Abstraction Layer.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_VULKAN_HAL_H
#define INC_SF_VULKAN_HAL_H
#pragma once

#include "Render/Vulkan/Vulkan_Config.h"
#include "Render/Render_DebugEnv.h"
#include "Render/Render_HAL.h"
#include "Render/Vulkan/Vulkan_MeshCache.h"
#include "Render/Vulkan/Vulkan_Shader.h"
#include "Render/Vulkan/Vulkan_Texture.h"
#include "Render/Render_ShaderHAL.h"

namespace Scaleform { namespace Render { namespace Vulkan {

enum HALConfigFlags
{
};

struct HALInitParams : public Render::HALInitParams
{
    VkDevice            pDevice;
    VkPhysicalDevice    pPhysicalDevice;
    VkQueue             pGraphicsQueue;
    uint32_t            GraphicsQueueFamily;
    VkCommandPool       pCommandPool;
    VkCommandBuffer     pCommandBuffer;
    VkRenderPass        pRenderPass;
    VkFormat            SwapchainFormat;
    VkInstance          pInstance;

    HALInitParams(VkDevice device, VkPhysicalDevice physDevice,
                  VkQueue graphicsQueue, uint32_t queueFamily,
                  VkCommandPool cmdPool, VkCommandBuffer cmdBuf,
                  VkRenderPass renderPass, VkFormat swapFormat,
                  VkInstance instance,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : Render::HALInitParams(0, halConfigFlags, renderThreadId),
      pDevice(device), pPhysicalDevice(physDevice),
      pGraphicsQueue(graphicsQueue), GraphicsQueueFamily(queueFamily),
      pCommandPool(cmdPool), pCommandBuffer(cmdBuf),
      pRenderPass(renderPass), SwapchainFormat(swapFormat),
      pInstance(instance)
    { }

    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*)pTextureManager.GetPtr(); }
};


// Pipeline state key for the pipeline cache map.
// pRenderPass is included so RGBA (offscreen) and BGRA (main) pipelines are kept separate.
struct PipelineKey
{
    ShaderDesc::ShaderType  Shader;
    ShaderDesc::ShaderType  FragShaderType;
    unsigned                BlendType;
    unsigned                StencilMode;
    unsigned                RasterMode;
    const SysVertexFormat*  pVFormat;
    VkRenderPass            pRenderPass;

    bool operator==(const PipelineKey& o) const
    {
        return Shader == o.Shader && FragShaderType == o.FragShaderType &&
               BlendType == o.BlendType &&
               StencilMode == o.StencilMode && RasterMode == o.RasterMode &&
               pVFormat == o.pVFormat && pRenderPass == o.pRenderPass;
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
        UPInt h = (UPInt)k.Shader * 2654435761u;
        h ^= RotateLeft((UPInt)k.FragShaderType * 2246822519u, 5);
        h ^= RotateLeft((UPInt)k.BlendType * 2246822519u, 7);
        h ^= RotateLeft((UPInt)k.StencilMode * 3266489917u, 13);
        h ^= RotateLeft((UPInt)k.RasterMode * 668265263u, 19);
        h ^= RotateLeft((UPInt)k.pVFormat * 374761393u, 11);
        h ^= RotateLeft((UPInt)k.pRenderPass * 2654435761u, 17);
        return h;
    }
};


class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

    VkDevice                pDevice;
    VkPhysicalDevice        pPhysicalDevice;
    VkQueue                 pGraphicsQueue;
    uint32_t                GraphicsQueueFamily;
    VkCommandPool           pCommandPool;
    VkCommandBuffer         pCommandBuffer;
    VkRenderPass            pCurrentRenderPass;
    VkInstance              pInstance;

    // Tracks whether the platform has actually begun the main swapchain render
    // pass for the current frame. SetMainRenderTarget() is called both after
    // BeginFrame (pass active) and after swapchain recreation (pass not yet
    // begun); PopRenderTarget must only resume the main pass when it is active.
    bool                    MainPassActive;

    // Sets whether the main swapchain render pass is currently active. Called
    // by the platform right after vkCmdBeginRenderPass for the swapchain pass,
    // and cleared after the pass is ended / swapchain is recreated.
    // This mirrors the state into InRenderPass, which is what the drawing code
    // uses to decide whether a graphics pipeline may be bound.
    void SetMainPassActive(bool active) { MainPassActive = active; InRenderPass = active; }

    MeshCache               Cache;
    Ptr<TextureManager>     pTextureManager;
    PrimitiveBatch::BatchType PrevBatchType;

    HAL*    getThis() { return this; }

public:
    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();

    // *** Initialization
    virtual bool        InitHAL(const Vulkan::HALInitParams& params);
    virtual bool        ShutdownHAL();
    void                PrepareForReset();
    bool                RestoreAfterReset();

    // *** Rendering
    virtual bool        BeginScene();

    // Called by the platform right after it waited on this frame in-flight fence.
    // Reclaiming GPU resources any earlier destroys objects that in-flight command
    // buffers still reference (validation reported destroyed VkImageView objects).
    void                    DrainFrameResources();
    virtual bool        EndScene();
    virtual bool        BeginFrame();
    virtual void        beginDisplay(BeginDisplayData* data);
    virtual void        updateViewport();

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

    // *** Stencil modes (for Vulkan pipeline depth/stencil state lookup)
    enum StencilModes
    {
        StencilMode_Disabled,
        StencilMode_Available_ClearMasks,
        StencilMode_Available_ClearMasksAbove,
        StencilMode_Available_WriteMask,
        StencilMode_DepthOnly_WriteMask,
        StencilMode_Available_TestMask,
        StencilMode_DepthOnly_TestMask,
        StencilMode_Unavailable,
        StencilMode_Count
    };

    // *** Raster modes
    virtual bool IsRasterModeSupported(RasterModeType mode) const { return mode != RasterMode_Point; }
    virtual void applyRasterModeImpl(RasterModeType mode);

    // *** Vertex arrays (pure virtuals from ShaderHAL)
    virtual UPInt setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);

    // *** Pipeline management
    VkPipelineLayout    pPipelineLayout;
    VkPipelineCache     pPipelineCache;
    VkDescriptorSetLayout pDescriptorSetLayout;

    VkPipeline  GetPipeline(ShaderDesc::ShaderType vsShader, ShaderDesc::ShaderType fsShader,
                            unsigned blendType, unsigned stencilMode, const SysVertexFormat* pvf);
    bool        createPipelineLayout();
    void        destroyPipelineLayout();

    // Per-frame descriptor pool
    VkDescriptorPool    FrameDescriptorPools[SF_VK_MAX_FRAMES_IN_FLIGHT];
    unsigned            CurrentFrame;

    // UBO ring buffer
    VkBuffer            UBORingBuffer;
    VkDeviceMemory      UBORingMemory;
    UByte*              pUBORingData;
    UPInt               UBORingOffset;
    bool                createUBORing();
    void                destroyUBORing();
    UPInt               allocUBOSpace(UPInt size);

    VkRenderPass    pOffscreenRenderPass;
    VkRenderPass    pOffscreenRenderPassDS;  // with depth/stencil attachment
    // Resume variants (LOAD_OP_LOAD): used when a nested offscreen pass is popped and
    // rendering continues in a parent temporary target that already has content.
    VkRenderPass    pOffscreenResumeRenderPass;
    VkRenderPass    pOffscreenResumeRenderPassDS;
    bool            InRenderPass;
    bool            createOffscreenRenderPass();
    bool            createOffscreenResumeRenderPass();
    bool            createOffscreenResumeRenderPassDS();
    bool            createOffscreenRenderPassDS();
    void            destroyOffscreenRenderPass();
    VkFramebuffer   createFramebuffer(VkImageView colorView, VkImageView dsView,
                                      unsigned width, unsigned height, VkRenderPass rp);

    // Main render pass info, set each frame by the application so PopRenderTarget
    // can resume the swapchain render pass after returning from an offscreen filter pass.
    VkRenderPass    pMainRenderPass;        // original main pass (owns the pipeline compatibility key)
    VkRenderPass    pMainResumeRenderPass;  // same format, LOAD_OP_LOAD - used for restart
    VkFramebuffer   pMainFramebuffer;
    VkExtent2D      MainRenderExtent;

    // No explicit swapchain-image layout tracking is needed: the platform's main
    // render pass ends in COLOR_ATTACHMENT_OPTIMAL (which is exactly what the
    // resume pass expects), and presentFrame() transitions the image to
    // PRESENT_SRC_KHR once, right before submit.

    // Call once per frame before Display(), providing the current swapchain framebuffer.
    void SetMainRenderTarget(VkRenderPass mainPass, VkRenderPass resumePass,
                             VkFramebuffer framebuffer, VkExtent2D extent)
    {
        pMainRenderPass       = mainPass;
        pMainResumeRenderPass = resumePass;
        static const bool sImgTrace = IsDebugEnvSet("GFX_VKIMG");
        if (sImgTrace)
            fprintf(stderr, "[VKHAL] SetMainRT fb=%p ext=%ux%u\n", (void*)framebuffer, extent.width, extent.height), fflush(stderr);
        pMainFramebuffer      = framebuffer;
        MainRenderExtent      = extent;
        // NOTE: InRenderPass is intentionally not touched here. This is called
        // from swapchain (re)creation, where no render pass is recording yet;
        // the platform marks the pass active right after vkCmdBeginRenderPass
        // through SetMainPassActive(true).
    }

    // Frame slot (0..SF_VK_MAX_FRAMES_IN_FLIGHT-1) for the frame being recorded.
    // The platform owns the fence/semaphore slot index, so it hands the same
    // slot to the HAL; otherwise the per-slot descriptor pools, UBO ring and
    // deferred-delete queues drift out of sync with the GPU by one frame.
    unsigned    FrameSlot;
    void        SetFrameSlot(unsigned slot) { FrameSlot = slot; }

    VkDevice    GetDevice() const { return pDevice; }
    VkCommandBuffer GetCommandBuffer() const { return pCommandBuffer; }
    void        SetCommandBuffer(VkCommandBuffer cmd) { pCommandBuffer = cmd; }

    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags = 0, Color clearColor = 0);
    virtual void            PopRenderTarget(unsigned flags = 0);
    virtual bool            createDefaultRenderBuffer();

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*    GetRenderSync()       { return &Cache.RSync; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned = 0)
    {
        SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced);
    }

protected:
    virtual void setBatchUnitSquareVertexStream();
    // Overridden: the shared drawFilter() only calls the "PerDraw" variant, whose base
    // implementation is empty. Without binding the unit-square buffer here the filter
    // quad reuses whatever vertex buffer the last mesh bound - it renders nothing.
    virtual void setBatchUnitSquareVertexStreamPerDraw();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void drawMaskClearRectangles(const Matrix2F* matrices, UPInt count);
    virtual void drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);
    virtual void drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);

    bool         shouldRenderFilters(const FilterPrimitive* prim) const;
    bool         IsFilterCachingSupported() const;
    virtual Render::RenderEvent& GetEvent(EventType eventType);

    void drawScreenQuad();

public:
    // Pipeline state - public so ShaderInterface::Finish can bind pipelines
    unsigned        CurrentStencilMode;
    unsigned        CurrentBlendType;

private:
    // Pipeline cache map
    typedef Hash<PipelineKey, VkPipeline, PipelineKeyHashF> PipelineMap;
    PipelineMap     Pipelines;

    // Blend state descriptions (reusable for pipeline creation)
    VkPipelineColorBlendAttachmentState BlendDescs[BlendTypeCount];
    VkPipelineDepthStencilStateCreateInfo DepthStencilDescs[StencilMode_Count];
    void    initBlendDescriptions();
    void    initDepthStencilDescriptions();

    // Map from base-class DepthStencilMode to Vulkan-specific StencilModes
    static StencilModes MapDepthStencilMode(DepthStencilMode mode);

    RenderEvent Events[Event_Count];

    // One temporary target that has been popped but whose GPU resources are still
    // referenced by a framebuffer that is queued for deferred destruction
    // (RenderTargetData::PendingDeletes). The whole set is retired together, in
    // DrainFrameResources(), right after that framebuffer has been destroyed and the
    // frame slot's fence has been waited on:
    //     framebuffer -> depth/stencil buffer -> colour texture (via the target)
    // Until then the target is kept out of the buffer manager's eviction lists (it stays
    // 'in use'), because evicting it would destroy the very image views the framebuffer
    // references - the validation layer then reports "VkImageView ... was destroyed,
    // which was used by VkFramebuffer ..." and invalidates the recording command buffer.
    struct DeferredTargetRelease
    {
        Ptr<RenderTarget>       pTarget;
        Ptr<DepthStencilBuffer> pDepthStencil;

        DeferredTargetRelease() { }
        DeferredTargetRelease(RenderTarget* target, DepthStencilBuffer* ds)
            : pTarget(target), pDepthStencil(ds) { }
    };
    Array<DeferredTargetRelease> PendingTargetReleases;
};


class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;
    VkDevice        pDevice;
    VkImageView     pRenderSurface;
    VkImageView     pDSSurface;
    VkFramebuffer   pFramebuffer;
    // Render pass the cached pFramebuffer was created for. A temporary target
    // can be re-used with or without a depth/stencil attachment, which selects
    // a different render pass (and attachment count); the framebuffer must be
    // recreated when that choice changes.
    VkRenderPass    FramebufferRenderPass;
    UPInt           CacheID;

    // Per-slot deferred deletion queues for framebuffers.  With N frames in
    // flight, the app waits on slot S's fence before reusing slot S, so
    // PendingDeletes[S] is safe to drain at that point.  Using a single shared
    // list would destroy framebuffers from frame N-1 before its GPU work
    // completes (the app only waited on the fence for slot S, not N-1's slot).
    static Array<VkFramebuffer> PendingDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];
    static VkDevice             PendingDeleteDevice;
    static VkQueue              PendingDeleteQueue;
    static unsigned             PendingDeleteSlot;

    static void SetPendingDeleteSlot(unsigned frameIndex)
    {
        PendingDeleteSlot = frameIndex % SF_VK_MAX_FRAMES_IN_FLIGHT;
    }

    static void DrainPendingDeletes()
    {
        if (PendingDeleteDevice == VK_NULL_HANDLE) return;
        // Only drain the slot whose fence the app waited on this frame.  With
        // multithreaded rendering (FxRenderThread plus a main-thread HUD pass)
        // the enqueue slot can lag the drain slot, so guard the destruction
        // with a queue wait whenever any framebuffer is pending: a framebuffer
        // referenced by a command buffer that has not completed execution yet
        // may not be destroyed.
        UPInt total = 0;
        for (unsigned s = 0; s < SF_VK_MAX_FRAMES_IN_FLIGHT; ++s)
            total += PendingDeletes[s].GetSize();
        if (total == 0)
            return;
        if (PendingDeleteQueue != VK_NULL_HANDLE)
            vkQueueWaitIdle(PendingDeleteQueue);
        Array<VkFramebuffer>& list = PendingDeletes[PendingDeleteSlot];
        for (unsigned i = 0; i < list.GetSize(); ++i)
            vkDestroyFramebuffer(PendingDeleteDevice, list[i], nullptr);
        list.ClearAndRelease();
    }

    static void DrainAllPendingDeletes()
    {
        if (PendingDeleteDevice == VK_NULL_HANDLE) return;
        for (unsigned s = 0; s < SF_VK_MAX_FRAMES_IN_FLIGHT; ++s)
        {
            for (unsigned i = 0; i < PendingDeletes[s].GetSize(); ++i)
                vkDestroyFramebuffer(PendingDeleteDevice, PendingDeletes[s][i], nullptr);
            PendingDeletes[s].ClearAndRelease();
        }
    }

    // Depth/stencil buffers that were detached from a render target while the
    // framebuffer that used their VkImageView was still alive (the framebuffer is only
    // destroyed later, from PendingDeletes). Releasing the buffer here would destroy
    // the view first; it is parked per frame slot and released together with the
    // framebuffers by DrainAllPendingDeletes()/DrainAllPendingDSDeletes().
    static Array<Ptr<DepthStencilBuffer> > PendingDSDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];

    static void ParkDS(const Ptr<DepthStencilBuffer>& ds)
    {
        if (ds)
            PendingDSDeletes[PendingDeleteSlot].PushBack(ds);
    }

    static void DrainAllPendingDSDeletes()
    {
        for (unsigned s = 0; s < SF_VK_MAX_FRAMES_IN_FLIGHT; ++s)
            PendingDSDeletes[s].Clear();
    }

    // Queues a framebuffer for destruction once the current frame slot's fence
    // has been waited on. Used when a cached framebuffer becomes invalid (its
    // attachment view was recreated) while it may still be referenced by an
    // in-flight command buffer.
    static void QueueFramebufferDelete(VkFramebuffer fb)
    {
        if (fb != VK_NULL_HANDLE)
            PendingDeletes[PendingDeleteSlot].PushBack(fb);
    }

    static void UpdateData(RenderBuffer* buffer, VkDevice device, VkImageView rtView,
                           DepthStencilBuffer* pdsb, VkImageView dsView,
                           VkFramebuffer fb)
    {
        if (!buffer) return;
        PendingDeleteDevice = device;
        RenderTargetData* poldHD = (Vulkan::RenderTargetData*)buffer->GetRenderTargetData();
        if (!poldHD)
        {
            poldHD = SF_NEW RenderTargetData(buffer, device, rtView, pdsb, dsView, fb);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        // Framebuffer first: it references the old DS view we are about to replace.
        if (poldHD->pFramebuffer != fb && poldHD->pFramebuffer != VK_NULL_HANDLE)
        {
            PendingDeletes[PendingDeleteSlot].PushBack(poldHD->pFramebuffer);
            poldHD->FramebufferRenderPass = VK_NULL_HANDLE;
        }
        if (poldHD->pDepthStencilBuffer.GetPtr() != pdsb)
        {
            Ptr<DepthStencilBuffer> pOld = poldHD->pDepthStencilBuffer;
            poldHD->pDepthStencilBuffer = pdsb;
            ParkDS(pOld);
        }
        poldHD->pDevice = device;
        poldHD->pDSSurface = dsView;
        poldHD->pRenderSurface = rtView;
        poldHD->pFramebuffer = fb;
    }

    virtual ~RenderTargetData()
    {
        if (pFramebuffer != VK_NULL_HANDLE)
            PendingDeletes[PendingDeleteSlot].PushBack(pFramebuffer);
    }

private:
    RenderTargetData(RenderBuffer* buffer, VkDevice device, VkImageView rtView,
                     DepthStencilBuffer* pdsb, VkImageView dsView,
                     VkFramebuffer fb)
    : RenderBuffer::RenderTargetData(buffer, pdsb),
      pDevice(device), pRenderSurface(rtView), pDSSurface(dsView), pFramebuffer(fb),
      FramebufferRenderPass(VK_NULL_HANDLE), CacheID(0) {}
};

}}} // Scaleform::Render::Vulkan

#endif // INC_SF_VULKAN_HAL_H








