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
    virtual bool        EndScene();
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
    bool            InRenderPass;
    bool            createOffscreenRenderPass();
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

    // Call once per frame before Display(), providing the current swapchain framebuffer.
    void SetMainRenderTarget(VkRenderPass mainPass, VkRenderPass resumePass,
                             VkFramebuffer framebuffer, VkExtent2D extent)
    {
        pMainRenderPass       = mainPass;
        pMainResumeRenderPass = resumePass;
        pMainFramebuffer      = framebuffer;
        MainRenderExtent      = extent;
    }

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
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void drawMaskClearRectangles(const Matrix2F* matrices, UPInt count);
    virtual void drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);
    virtual void drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset);

    bool         shouldRenderFilters(const FilterPrimitive* prim) const;
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

    // Render targets used this frame that must not be recycled until the GPU
    // finishes executing the command buffer.  Drained at the start of BeginScene,
    // after the previous frame's fence has been waited on.
    Array<Ptr<RenderTarget>> PendingRTReleases;
};


class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;
    VkDevice        pDevice;
    VkImageView     pRenderSurface;
    VkImageView     pDSSurface;
    VkFramebuffer   pFramebuffer;
    UPInt           CacheID;

    // Per-slot deferred deletion queues for framebuffers.  With N frames in
    // flight, the app waits on slot S's fence before reusing slot S, so
    // PendingDeletes[S] is safe to drain at that point.  Using a single shared
    // list would destroy framebuffers from frame N-1 before its GPU work
    // completes (the app only waited on the fence for slot S, not N-1's slot).
    static Array<VkFramebuffer> PendingDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];
    static VkDevice             PendingDeleteDevice;
    static unsigned             PendingDeleteSlot;

    static void SetPendingDeleteSlot(unsigned frameIndex)
    {
        PendingDeleteSlot = frameIndex % SF_VK_MAX_FRAMES_IN_FLIGHT;
    }

    static void DrainPendingDeletes()
    {
        if (PendingDeleteDevice == VK_NULL_HANDLE) return;
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
        poldHD->pDevice = device;
        poldHD->pDepthStencilBuffer = pdsb;
        poldHD->pDSSurface = dsView;
        poldHD->pRenderSurface = rtView;
        if (poldHD->pFramebuffer != fb && poldHD->pFramebuffer != VK_NULL_HANDLE)
            PendingDeletes[PendingDeleteSlot].PushBack(poldHD->pFramebuffer);
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
      pDevice(device), pRenderSurface(rtView), pDSSurface(dsView), pFramebuffer(fb), CacheID(0) {}
};

}}} // Scaleform::Render::Vulkan

#endif // INC_SF_VULKAN_HAL_H
