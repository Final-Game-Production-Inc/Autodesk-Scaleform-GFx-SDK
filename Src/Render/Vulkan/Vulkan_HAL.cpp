/**************************************************************************

Filename    :   Vulkan_HAL.cpp
Content     :   Vulkan HAL implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Render/Vulkan/Vulkan_HAL.h"
#include "Render/Render_BufferGeneric.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Vulkan/Vulkan_Events.h"
#include "Render/Render_TextureCacheGeneric.h"

#include <string.h>

namespace Scaleform { namespace Render { namespace Vulkan {

// Static members for deferred framebuffer deletion
Array<VkFramebuffer> RenderTargetData::PendingDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];
VkDevice             RenderTargetData::PendingDeleteDevice = VK_NULL_HANDLE;
unsigned             RenderTargetData::PendingDeleteSlot = 0;


// *** HAL

HAL::HAL(ThreadCommandQueue* commandQueue)
: Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
  pDevice(VK_NULL_HANDLE),
  pPhysicalDevice(VK_NULL_HANDLE),
  pGraphicsQueue(VK_NULL_HANDLE),
  GraphicsQueueFamily(0),
  pCommandPool(VK_NULL_HANDLE),
  pCommandBuffer(VK_NULL_HANDLE),
  pCurrentRenderPass(VK_NULL_HANDLE),
  pInstance(VK_NULL_HANDLE),
  pOffscreenRenderPass(VK_NULL_HANDLE),
  pOffscreenRenderPassDS(VK_NULL_HANDLE),
  InRenderPass(false),
  pMainRenderPass(VK_NULL_HANDLE),
  pMainResumeRenderPass(VK_NULL_HANDLE),
  pMainFramebuffer(VK_NULL_HANDLE),
  MainRenderExtent{0, 0},
  Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
  PrevBatchType(PrimitiveBatch::DP_None),
  pPipelineLayout(VK_NULL_HANDLE),
  pPipelineCache(VK_NULL_HANDLE),
  pDescriptorSetLayout(VK_NULL_HANDLE),
  CurrentFrame(0),
  UBORingBuffer(VK_NULL_HANDLE),
  UBORingMemory(VK_NULL_HANDLE),
  pUBORingData(nullptr),
  UBORingOffset(0),
  CurrentStencilMode(StencilMode_Disabled),
  CurrentBlendType(0)
{
    memset(FrameDescriptorPools, 0, sizeof(FrameDescriptorPools));
    initBlendDescriptions();
    initDepthStencilDescriptions();
}

HAL::~HAL()
{
    ShutdownHAL();
}


// *** Initialization

bool HAL::InitHAL(const Vulkan::HALInitParams& params)
{
    if (!initHAL(params))
        return false;

    if (!params.pDevice)
        return false;

    pDevice            = params.pDevice;
    pPhysicalDevice    = params.pPhysicalDevice;
    pGraphicsQueue     = params.pGraphicsQueue;
    GraphicsQueueFamily = params.GraphicsQueueFamily;
    pCommandPool       = params.pCommandPool;
    pCommandBuffer     = params.pCommandBuffer;
    pCurrentRenderPass = params.pRenderPass;
    pMainRenderPass    = params.pRenderPass;
    pInstance          = params.pInstance;

    Vulkan::RenderEvent::InitializeEvents(pInstance, pCommandBuffer);

    if (!createPipelineLayout()) { ShutdownHAL(); return false; }
    if (!createOffscreenRenderPass()) { ShutdownHAL(); return false; }
    if (!createOffscreenRenderPassDS()) { ShutdownHAL(); return false; }
    if (!createUBORing()) { ShutdownHAL(); return false; }
    if (!SManager.Initialize(this)) { ShutdownHAL(); return false; }
    if (!Cache.Initialize(pDevice, pPhysicalDevice, &Cache.RSync))
    {
        ShutdownHAL();
        return false;
    }

    Cache.GetRenderSync()->SetDevice(pDevice, pGraphicsQueue);

    // Create descriptor pools for frames in flight
    for (unsigned f = 0; f < SF_VK_MAX_FRAMES_IN_FLIGHT; f++)
    {
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         SF_VK_DESCRIPTOR_POOL_MAX_SETS * 2 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SF_VK_DESCRIPTOR_POOL_MAX_SETS * (SF_VK_MAX_TEXTURE_STAGES + SF_VK_MAX_TEXTURE_STAGES - 1) },
        };
        VkDescriptorPoolCreateInfo poolCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        poolCI.maxSets       = SF_VK_DESCRIPTOR_POOL_MAX_SETS;
        poolCI.poolSizeCount = 2;
        poolCI.pPoolSizes    = poolSizes;
        SF_VK_CHECK_RETURN(vkCreateDescriptorPool(pDevice, &poolCI, nullptr, &FrameDescriptorPools[f]), false);
    }

    // Texture manager
    if (params.pTextureManager)
        pTextureManager = params.GetTextureManager();
    else
    {
        pTextureManager = *SF_HEAP_AUTO_NEW(this) TextureManager(
            pDevice, pPhysicalDevice, pCommandPool, pGraphicsQueue,
            params.RenderThreadId, pRTCommandQueue);
        if (!pTextureManager)
        {
            Cache.Reset();
            SManager.Reset();
            return false;
        }
    }

    Matrices = *SF_HEAP_AUTO_NEW(this) MatrixState(this);

    if (params.pRenderBufferManager)
        pRenderBufferManager = params.pRenderBufferManager;
    else
    {
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_Exact);
        if (!pRenderBufferManager || !createDefaultRenderBuffer())
        {
            ShutdownHAL();
            return false;
        }
    }

    if (pTextureManager && pRenderBufferManager)
    {
        HALState |= HS_ModeSet;
        notifyHandlers(HAL_Initialize);
        return true;
    }

    return false;
}


bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    Vulkan::RenderEvent::ShutdownEvents();

    if (pDevice)
        vkDeviceWaitIdle(pDevice);

    // Drain all framebuffers queued for deferred deletion (all slots)
    RenderTargetData::DrainAllPendingDeletes();

    // Destroy pipelines
    for (PipelineMap::Iterator it = Pipelines.Begin(); it != Pipelines.End(); ++it)
        vkDestroyPipeline(pDevice, it->Second, nullptr);
    Pipelines.Clear();

    destroyPipelineLayout();
    destroyOffscreenRenderPass();
    destroyUBORing();

    for (unsigned f = 0; f < SF_VK_MAX_FRAMES_IN_FLIGHT; f++)
    {
        if (FrameDescriptorPools[f] != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(pDevice, FrameDescriptorPools[f], nullptr);
            FrameDescriptorPools[f] = VK_NULL_HANDLE;
        }
    }

    destroyRenderBuffers();
    pRenderBufferManager.Clear();

    pTextureManager->Reset();
    pTextureManager.Clear();
    SManager.Reset();

    Cache.GetRenderSync()->SetDevice(VK_NULL_HANDLE, VK_NULL_HANDLE);
    Cache.Reset();

    // Final drain: ~RenderTargetData may have pushed framebuffers during
    // destroyRenderBuffers / pRenderBufferManager.Clear above.
    RenderTargetData::DrainAllPendingDeletes();

    pDevice = VK_NULL_HANDLE;
    return true;
}


void HAL::PrepareForReset()
{
    SF_ASSERT(HALState & HS_ModeSet);
    if (HALState & HS_ReadyForReset)
        return;
    notifyHandlers(HAL_PrepareForReset);
    HALState |= HS_ReadyForReset;
}

bool HAL::RestoreAfterReset()
{
    if (!IsInitialized()) return false;
    if (!(HALState & HS_ReadyForReset)) return true;
    notifyHandlers(HAL_RestoreAfterReset);
    HALState &= ~HS_ReadyForReset;
    return true;
}


// *** Rendering

bool HAL::BeginScene()
{
    if (!BaseHAL::BeginScene())
        return false;

    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__);

    // Set the current frame slot and drain framebuffer deletions for THIS slot.
    // With N frames in flight the app waits on slot S's fence before reusing it,
    // so only slot S's accumulated deletes are guaranteed safe to destroy here.
    unsigned frameIdx = CurrentFrame % SF_VK_MAX_FRAMES_IN_FLIGHT;
    RenderTargetData::SetPendingDeleteSlot(frameIdx);
    RenderTargetData::DrainPendingDeletes();

    // Return render targets used last frame back to the pool now that the GPU is done.
    for (unsigned i = 0; i < PendingRTReleases.GetSize(); ++i)
        PendingRTReleases[i]->SetInUse(RTUse_Unused);
    PendingRTReleases.Clear();

    // Drain VkImageView/VkImage/VkDeviceMemory kills for this slot.  The app
    // waited on the in-flight fence for this slot, so its resources are safe.
    Vulkan::TextureManager* ptm = (Vulkan::TextureManager*)pTextureManager.GetPtr();
    if (ptm)
    {
        ptm->SetKillSlot(frameIdx);
        ptm->DrainDeferredKills();
        ptm->DeferKills = true;
    }

    vkResetDescriptorPool(pDevice, FrameDescriptorPools[frameIdx], 0);
    UBORingOffset = 0;

    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);

    // Release filter temp RTs from RBCL_InUse before the base class calls
    // EndFrame() which asserts that list is empty.  The strong references in
    // PendingRTReleases keep the underlying textures alive until BeginScene
    // of a later frame (after the GPU fence has been waited on).
    for (unsigned i = 0; i < PendingRTReleases.GetSize(); ++i)
        PendingRTReleases[i]->SetInUse(RTUse_Unused);

    if (!BaseHAL::EndScene())
        return false;

    CurrentFrame++;
    return true;
}

void HAL::beginDisplay(BeginDisplayData* data)
{
    GetEvent(Event_Display).Begin(__FUNCTION__);
    Render::HAL::beginDisplay(data);
}

void HAL::updateViewport()
{
    if (HALState & HS_ViewValid)
    {
        Rect<int> vpRect;

        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        CalcHWViewMatrix(0, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect  = ViewRect;
        Matrices->UVPOChanged = 1;

        if (!(HALState & HS_InRenderTarget))
            vpRect = ViewRect;
        else
            vpRect.SetRect(VP.Left, VP.Top, VP.Left + VP.Width, VP.Top + VP.Height);

        VkViewport vp = {};
        vp.x      = (float)vpRect.x1;
        vp.width  = Alg::Max(1.0f, (float)vpRect.Width());
        // Negative height flips Y axis to match D3D/GL convention where
        // CalcHWViewMatrix maps y=0 to clip Y=+1 (top) and y=height to clip Y=-1 (bottom).
        // In Vulkan clip space, Y=-1 is top and Y=+1 is bottom, so the flip is needed.
        vp.y      = (float)vpRect.y1 + Alg::Max(1.0f, (float)vpRect.Height());
        vp.height = -Alg::Max(1.0f, (float)vpRect.Height());
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { vpRect.x1, vpRect.y1 };
        scissor.extent = { (uint32_t)Alg::Max(1, vpRect.Width()),
                           (uint32_t)Alg::Max(1, vpRect.Height()) };

        vkCmdSetViewport(pCommandBuffer, 0, 1, &vp);
        vkCmdSetScissor(pCommandBuffer, 0, 1, &scissor);
    }
}


// *** Vertex arrays (ShaderHAL pure virtuals)

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmeshBase)
{
    Vulkan::MeshCacheItem* pmesh = reinterpret_cast<Vulkan::MeshCacheItem*>(pmeshBase);

    VkBuffer vb = pmesh->pVertexBuffer->GetHWBuffer();
    VkDeviceSize vbOffset = pmesh->VBAllocOffset;
    vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, &vb, &vbOffset);
    // Index buffer bound at offset 0; the allocation offset is returned and
    // passed as firstIndex to vkCmdDrawIndexed by the caller.
    vkCmdBindIndexBuffer(pCommandBuffer, pmesh->pIndexBuffer->GetHWBuffer(),
                         0, VK_INDEX_TYPE_UINT16);

    return pmesh->IBAllocOffset / sizeof(IndexType);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{
    Vulkan::MeshCacheItem* pmesh = reinterpret_cast<Vulkan::MeshCacheItem*>(pmeshBase);

    VkBuffer vb = pmesh->pVertexBuffer->GetHWBuffer();
    VkDeviceSize byteOffset = fr.VertexByteOffset + pmesh->VBAllocOffset;
    vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, &vb, &byteOffset);
    // Index buffer bound at offset 0; the allocation offset is returned and
    // passed as firstIndex to vkCmdDrawIndexed by the caller.
    vkCmdBindIndexBuffer(pCommandBuffer, pmesh->pIndexBuffer->GetHWBuffer(),
                         0, VK_INDEX_TYPE_UINT16);

    return pmesh->IBAllocOffset / sizeof(IndexType);
}


// *** Mask / Stencil

HAL::StencilModes HAL::MapDepthStencilMode(DepthStencilMode mode)
{
    switch (mode)
    {
    case DepthStencil_Disabled:                 return StencilMode_Disabled;
    case DepthStencil_StencilClear:             return StencilMode_Available_ClearMasks;
    case DepthStencil_StencilClearHigher:        return StencilMode_Available_ClearMasksAbove;
    case DepthStencil_StencilIncrementEqual:     return StencilMode_Available_WriteMask;
    case DepthStencil_StencilTestLessEqual:      return StencilMode_Available_TestMask;
    case DepthStencil_DepthWrite:                return StencilMode_DepthOnly_WriteMask;
    case DepthStencil_DepthTestEqual:            return StencilMode_DepthOnly_TestMask;
    default:                                     return StencilMode_Disabled;
    }
}

void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    StencilModes vkMode = MapDepthStencilMode(mode);
    CurrentStencilMode = (unsigned)vkMode;

    // Update the color write state: some depth/stencil modes disable color writes.
    bool colorWrite = DepthStencilModeTable[mode].ColorWriteEnable;
    if (!colorWrite)
        CurrentBlendType = GetBlendType(Blend_Normal, Write_None);

    // Set the dynamic stencil reference
    vkCmdSetStencilReference(pCommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, (uint32_t)stencilRef);

    // Update base class tracking
    if (mode != CurrentDepthStencilState || CurrentStencilRef != stencilRef)
    {
        if (DepthStencilModeTable[mode].ColorWriteEnable != DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable)
        {
            CurrentDepthStencilState = mode;
            applyBlendModeImpl(CurrentBlendState.Mode, CurrentBlendState.SourceAc, CurrentBlendState.ForceAc);
        }
        CurrentDepthStencilState = mode;
        CurrentStencilRef = stencilRef;
    }
}

bool HAL::checkDepthStencilBufferCaps()
{
    if (!StencilChecked)
    {
        StencilChecked = true;
        StencilAvailable = true;
        MultiBitStencil  = true;
        DepthBufferAvailable = false;
    }

    if (!StencilAvailable && !DepthBufferAvailable)
    {
        SF_DEBUG_WARNONCE(1, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
        return false;
    }
    return true;
}


// *** Raster modes

void HAL::applyRasterModeImpl(RasterModeType mode)
{
    // In Vulkan, raster mode is baked into the pipeline. The pipeline cache key includes
    // the raster mode, so changing it here means future GetPipeline() calls will look up
    // or create a pipeline with the appropriate polygon mode.
    // The base class tracks AppliedSceneRasterMode for redundancy checking.
    SF_UNUSED(mode);
}


// *** Blend modes

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc)
{
    SF_UNUSED(forceAc);
    bool colorWrite = DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable;
    CurrentBlendType = GetBlendType(mode, colorWrite ? Write_All : Write_None, sourceAc);
}


// *** Pipeline management

bool HAL::createPipelineLayout()
{
    // Descriptor set layout:
    //   binding 0 = VS UBO
    //   binding 1 = FS UBO
    //   binding 2 = sampler2D tex[MAX_STAGES]  (array -- for TexTGTexTG, YUV, YUVA shaders)
    //   bindings 3..MAX = sampler2D             (individual -- for filter shaders like FBox2Shadow)
    //
    // Two SPIR-V patterns coexist in the compiled shaders:
    //   (a) Array style:      layout(binding=2) uniform sampler2D tex[N];
    //   (b) Individual style: layout(binding=2) uniform sampler2D srctex;
    //                         layout(binding=3) uniform sampler2D tex;
    // The layout must satisfy both: binding 2 as an array of MAX_STAGES covers (a),
    // and separate bindings 3+ cover (b).
    const unsigned numExtraBindings = (SF_VK_MAX_TEXTURE_STAGES > 1) ? SF_VK_MAX_TEXTURE_STAGES - 1 : 0;
    VkDescriptorSetLayoutBinding bindings[3 + SF_VK_MAX_TEXTURE_STAGES] = {};
    unsigned bindingCount = 0;

    bindings[bindingCount].binding         = 0;
    bindings[bindingCount].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[bindingCount].descriptorCount = 1;
    bindings[bindingCount].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    bindingCount++;

    bindings[bindingCount].binding         = 1;
    bindings[bindingCount].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[bindingCount].descriptorCount = 1;
    bindings[bindingCount].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindingCount++;

    // Binding 2: array of samplers (satisfies array-style shaders)
    bindings[bindingCount].binding         = 2;
    bindings[bindingCount].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[bindingCount].descriptorCount = SF_VK_MAX_TEXTURE_STAGES;
    bindings[bindingCount].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindingCount++;

    // Bindings 3+ : individual samplers (satisfies individual-binding-style shaders)
    for (unsigned s = 1; s < SF_VK_MAX_TEXTURE_STAGES; s++)
    {
        bindings[bindingCount].binding         = 2 + s;
        bindings[bindingCount].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[bindingCount].descriptorCount = 1;
        bindings[bindingCount].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindingCount++;
    }

    VkDescriptorSetLayoutCreateInfo layoutCI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    layoutCI.bindingCount = bindingCount;
    layoutCI.pBindings    = bindings;
    SF_VK_CHECK_RETURN(vkCreateDescriptorSetLayout(pDevice, &layoutCI, nullptr, &pDescriptorSetLayout), false);

    VkPipelineLayoutCreateInfo plCI = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    plCI.setLayoutCount = 1;
    plCI.pSetLayouts    = &pDescriptorSetLayout;
    SF_VK_CHECK_RETURN(vkCreatePipelineLayout(pDevice, &plCI, nullptr, &pPipelineLayout), false);

    VkPipelineCacheCreateInfo cacheCI = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    SF_VK_CHECK_RETURN(vkCreatePipelineCache(pDevice, &cacheCI, nullptr, &pPipelineCache), false);

    return true;
}

void HAL::destroyPipelineLayout()
{
    if (pPipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(pDevice, pPipelineCache, nullptr);
        pPipelineCache = VK_NULL_HANDLE;
    }
    if (pPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(pDevice, pPipelineLayout, nullptr);
        pPipelineLayout = VK_NULL_HANDLE;
    }
    if (pDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(pDevice, pDescriptorSetLayout, nullptr);
        pDescriptorSetLayout = VK_NULL_HANDLE;
    }
}


VkPipeline HAL::GetPipeline(ShaderDesc::ShaderType vsShader, ShaderDesc::ShaderType fsShader,
                             unsigned blendType, unsigned stencilMode, const SysVertexFormat* pvf)
{
    PipelineKey key;
    key.Shader      = vsShader;
    key.FragShaderType = fsShader;
    key.BlendType   = blendType;
    key.StencilMode = stencilMode;
    key.RasterMode  = (unsigned)AppliedSceneRasterMode;
    key.pVFormat    = pvf;
    key.pRenderPass = pCurrentRenderPass;

    VkPipeline* existing = Pipelines.Get(key);
    if (existing)
        return *existing;

    // Create pipeline lazily
    unsigned vsIdx = VertexShaderDesc::GetShaderIndex(vsShader, SManager.ShaderModel);
    unsigned fsIdx = FragShaderDesc::GetShaderIndex(fsShader, SManager.ShaderModel);
    const VertexShader* pvs = &SManager.StaticVShaders[vsIdx];
    const FragShader* pfs   = &SManager.StaticFShaders[fsIdx];

    if (!pvs || pvs->pProg == VK_NULL_HANDLE || !pfs || pfs->pProg == VK_NULL_HANDLE)
        return VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = pvs->pProg;
    stages[0].pName  = "main";

    stages[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = pfs->pProg;
    stages[1].pName  = "main";

    VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    if (pvf)
    {
        vertexInput.vertexBindingDescriptionCount   = 1;
        vertexInput.pVertexBindingDescriptions      = &pvf->BindingDesc;
        vertexInput.vertexAttributeDescriptionCount = pvf->AttributeCount;
        vertexInput.pVertexAttributeDescriptions    = pvf->Attributes;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAsm = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vpState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vpState.viewportCount = 1;
    vpState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterState.polygonMode = (key.RasterMode == RasterMode_Wireframe) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterState.cullMode    = VK_CULL_MODE_NONE;
    rasterState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
    rasterState.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo msState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendStateCreateInfo blendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    VkPipelineColorBlendAttachmentState blendAtt = (blendType < BlendTypeCount)
        ? BlendDescs[blendType] : BlendDescs[0];
    blendState.attachmentCount = 1;
    blendState.pAttachments    = &blendAtt;

    VkPipelineDepthStencilStateCreateInfo dsState = (stencilMode < StencilMode_Count)
        ? DepthStencilDescs[stencilMode] : DepthStencilDescs[0];

    VkDynamicState dynStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE
    };
    VkPipelineDynamicStateCreateInfo dynState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynState.dynamicStateCount = 3;
    dynState.pDynamicStates    = dynStates;

    VkGraphicsPipelineCreateInfo pipelineCI = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineCI.stageCount          = 2;
    pipelineCI.pStages             = stages;
    pipelineCI.pVertexInputState   = &vertexInput;
    pipelineCI.pInputAssemblyState = &inputAsm;
    pipelineCI.pViewportState      = &vpState;
    pipelineCI.pRasterizationState = &rasterState;
    pipelineCI.pMultisampleState   = &msState;
    pipelineCI.pDepthStencilState  = &dsState;
    pipelineCI.pColorBlendState    = &blendState;
    pipelineCI.pDynamicState       = &dynState;
    pipelineCI.layout              = pPipelineLayout;
    pipelineCI.renderPass          = pCurrentRenderPass;
    pipelineCI.subpass             = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult r = vkCreateGraphicsPipelines(pDevice, pPipelineCache, 1, &pipelineCI, nullptr, &pipeline);
    if (r != VK_SUCCESS)
    {
        SF_DEBUG_WARNING1(1, "Vulkan: Failed to create graphics pipeline, VkResult=%d", (int)r);
        return VK_NULL_HANDLE;
    }

    Pipelines.Set(key, pipeline);
    return pipeline;
}


// *** UBO ring buffer

bool HAL::createUBORing()
{
    VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCI.size  = SF_VK_UBO_RING_SIZE;
    bufCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SF_VK_CHECK_RETURN(vkCreateBuffer(pDevice, &bufCI, nullptr, &UBORingBuffer), false);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(pDevice, UBORingBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX)
    {
        SF_DEBUG_WARNING(1, "Vulkan: No suitable memory type for UBO ring buffer");
        return false;
    }
    SF_VK_CHECK_RETURN(vkAllocateMemory(pDevice, &allocInfo, nullptr, &UBORingMemory), false);
    SF_VK_CHECK_RETURN(vkBindBufferMemory(pDevice, UBORingBuffer, UBORingMemory, 0), false);

    void* mapped;
    SF_VK_CHECK_RETURN(vkMapMemory(pDevice, UBORingMemory, 0, SF_VK_UBO_RING_SIZE, 0, &mapped), false);
    pUBORingData = (UByte*)mapped;

    return true;
}

void HAL::destroyUBORing()
{
    if (pUBORingData && UBORingMemory != VK_NULL_HANDLE)
    {
        vkUnmapMemory(pDevice, UBORingMemory);
        pUBORingData = nullptr;
    }
    if (UBORingBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(pDevice, UBORingBuffer, nullptr);
        UBORingBuffer = VK_NULL_HANDLE;
    }
    if (UBORingMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(pDevice, UBORingMemory, nullptr);
        UBORingMemory = VK_NULL_HANDLE;
    }
}

UPInt HAL::allocUBOSpace(UPInt size)
{
    size = (size + 255) & ~(UPInt)255;
    UPInt offset = UBORingOffset;
    UBORingOffset += size;
    if (UBORingOffset > SF_VK_UBO_RING_SIZE)
    {
        SF_DEBUG_ASSERT(0, "Vulkan: UBO ring buffer overflow - increase SF_VK_UBO_RING_SIZE");
        SF_DEBUG_WARNING(1, "Vulkan: UBO ring buffer overflow, data corruption likely");
        return 0;
    }
    return offset;
}


// *** Render targets

bool HAL::createDefaultRenderBuffer()
{
    if (!pRenderBufferManager)
        return false;

    ImageSize rtSize(0, 0);
    RenderTarget* existing = GetDefaultRenderTarget();
    if (existing)
    {
        rtSize = existing->GetSize();
    }
    else
    {
        rtSize = ImageSize(1, 1);
        Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize);
        RenderTargetData::UpdateData(ptarget, pDevice, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, VK_NULL_HANDLE);
        SetRenderTarget(ptarget);
    }

    return pRenderBufferManager->Initialize(pTextureManager, Image_R8G8B8A8, rtSize);
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    SF_UNUSED2(texture, needsStencil);
    // Create render target from texture
    Vulkan::Texture* pt = (Vulkan::Texture*)texture;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(pt->GetSize(), RBuffer_User, pt->GetFormat(), pt);

    if (needsStencil)
    {
        DepthStencilBuffer* pdsb = pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
        if (pdsb)
        {
            Vulkan::DepthStencilSurface* pdss = (Vulkan::DepthStencilSurface*)pdsb->GetSurface();
            VkImageView dsView = pdss ? pdss->pDepthStencilView : VK_NULL_HANDLE;
            RenderTargetData::UpdateData(prt, pDevice, pt->pTextures[0].View, pdsb, dsView, VK_NULL_HANDLE);
        }
    }
    else
    {
        RenderTargetData::UpdateData(prt, pDevice, pt->pTextures[0].View, 0, VK_NULL_HANDLE, VK_NULL_HANDLE);
    }
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    // Clamp to 1x1 minimum so degenerate filter areas (zero scale from collapsed
    // display objects) never reach vkCreateImage / vkCreateFramebuffer with a zero
    // extent (VUID-VkImageCreateInfo-extent-00944/00945).
    ImageSize safeSize(Alg::Max<UInt32>(1, size.Width), Alg::Max<UInt32>(1, size.Height));

    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(safeSize);
    if (!prt) return 0;
    Render::Texture* pt = prt->GetTexture();
    if (!pt) return 0;

    if (needsStencil)
    {
        DepthStencilBuffer* pdsb = pRenderBufferManager->CreateDepthStencilBuffer(safeSize);
        if (pdsb)
        {
            Vulkan::DepthStencilSurface* pdss = (Vulkan::DepthStencilSurface*)pdsb->GetSurface();
            VkImageView dsView = pdss ? pdss->pDepthStencilView : VK_NULL_HANDLE;
            RenderTargetData::UpdateData(prt, pDevice, ((Vulkan::Texture*)pt)->pTextures[0].View,
                pdsb, dsView, VK_NULL_HANDLE);
        }
    }
    else
    {
        RenderTargetData::UpdateData(prt, pDevice, ((Vulkan::Texture*)pt)->pTextures[0].View,
            0, VK_NULL_HANDLE, VK_NULL_HANDLE);
    }
    return prt;
}

bool HAL::SetRenderTarget(RenderTarget* ptarget, bool setState)
{
    if (!ptarget) return false;
    SF_UNUSED(setState);

    if (HALState & HS_InDisplay)
        return false;

    RenderTargetEntry entry;
    entry.pRenderTarget = ptarget;

    if (RenderTargetStack.GetSize() > 0)
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
    RenderTargetStack.PushBack(entry);

    if (!prt)
        return;

    StencilChecked = false;
    ++AccumulatedStats.RTChanges;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();

    if (phd && phd->pRenderSurface != VK_NULL_HANDLE && pOffscreenRenderPass != VK_NULL_HANDLE)
    {
        if (InRenderPass)
        {
            vkCmdEndRenderPass(pCommandBuffer);
            InRenderPass = false;
        }

        const ImageSize& bs = prt->GetBufferSize();
        // Guard: zero-size buffer can't produce a valid framebuffer or render area.
        if (bs.Width == 0 || bs.Height == 0)
            return;

        // Use the DS render pass when a depth/stencil view is attached.
        bool hasDS = (phd->pDSSurface != VK_NULL_HANDLE);
        VkRenderPass selectedRP = hasDS ? pOffscreenRenderPassDS : pOffscreenRenderPass;

        if (phd->pFramebuffer == VK_NULL_HANDLE)
        {
            phd->pFramebuffer = createFramebuffer(phd->pRenderSurface, phd->pDSSurface,
                                                  bs.Width, bs.Height, selectedRP);
        }

        if (phd->pFramebuffer != VK_NULL_HANDLE)
        {
            VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            rpBI.renderPass  = selectedRP;
            rpBI.framebuffer = phd->pFramebuffer;
            rpBI.renderArea.extent = { bs.Width, bs.Height };

            VkClearValue clearVals[2] = {};
            // Use the supplied clear color
            clearColor.GetRGBAFloat(&clearVals[0].color.float32[0]);
            clearVals[1].depthStencil = { 1.0f, 0 };
            rpBI.clearValueCount = hasDS ? 2 : 1;
            rpBI.pClearValues    = clearVals;

            pCurrentRenderPass = selectedRP;
            vkCmdBeginRenderPass(pCommandBuffer, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
            InRenderPass = true;
        }
    }

    Rect<int> viewRect = prt->GetRect();
    const ImageSize& bs = prt->GetBufferSize();
    VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());
    ViewRect.x1 = (int)frameRect.x1;
    ViewRect.y1 = (int)frameRect.y1;
    ViewRect.x2 = (int)frameRect.x2;
    ViewRect.y2 = (int)frameRect.y2;
    Matrices->ViewRectOriginal.Offset(-entry.OldViewport.Left, -entry.OldViewport.Top);
    Matrices->UVPOChanged = true;
    HALState |= HS_ViewValid;
    updateViewport();
}

void HAL::PopRenderTarget(unsigned flags)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_RenderTarget), 0, false);

    RenderTargetEntry& entry = RenderTargetStack.Back();

    if ((flags & PRT_NoSet) == 0)
    {
        if (InRenderPass)
        {
            vkCmdEndRenderPass(pCommandBuffer);
            InRenderPass = false;
        }
    }

    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();

    if (RenderTargetStack.GetSize() <= 1)
    {
        HALState &= ~HS_InRenderTarget;

        // Resume the main swapchain render pass so that the caller can immediately
        // composite the filter result without drawing outside a render pass.
        if ((flags & PRT_NoSet) == 0 &&
            pMainResumeRenderPass != VK_NULL_HANDLE && pMainFramebuffer != VK_NULL_HANDLE)
        {
            VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            rpBI.renderPass  = pMainResumeRenderPass;
            rpBI.framebuffer = pMainFramebuffer;
            rpBI.renderArea.extent = MainRenderExtent;
            vkCmdBeginRenderPass(pCommandBuffer, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
            InRenderPass = true;
            // Use the main render pass as the pipeline key so pipelines are shared
            // with those created during regular (non-filter) main-pass rendering.
            pCurrentRenderPass = pMainRenderPass;
        }
    }

    ++AccumulatedStats.RTChanges;
    StencilChecked = false;

    if ((flags & PRT_NoSet) == 0)
    {
        HALState |= HS_ViewValid;
        updateViewport();
    }
}


// *** Draw primitives

void HAL::drawMaskClearRectangles(const Matrix2F* matrices, UPInt count)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_MaskClear), "HAL::drawMaskClearRectangles");

    unsigned fillflags = 0;
    const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags,
        PrimitiveBatch::DP_Batch, MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch], &ShaderData);

    setBatchUnitSquareVertexStream();

    // Bind pipeline with the current stencil mode and current blend type.
    // This must happen after SetFill so the vertex format / shader pair is resolved.
    SysVertexFormat* pvf = (SysVertexFormat*)MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch]->pSysFormat.GetPtr();
    const ShaderPair& curSh = ShaderData.GetCurrentShaders();
    VkPipeline pipeline = GetPipeline(curSh.pVDesc ? curSh.pVDesc->Type : ShaderDesc::ST_None,
                                      curSh.pFDesc ? curSh.pFDesc->Type : ShaderDesc::ST_None,
                                      CurrentBlendType, CurrentStencilMode, pvf);
    if (pipeline != VK_NULL_HANDLE)
        vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    unsigned drawRangeCount = 0;
    for (UPInt i = 0; i < count; i += (UPInt)drawRangeCount)
    {
        drawRangeCount = Alg::Min<unsigned>((unsigned)(count - i), SF_RENDER_MAX_BATCHES);

        if (i != 0)
            ShaderData.BeginPrimitive();

        for (unsigned j = 0; j < drawRangeCount; j++)
        {
            ShaderData.SetMatrix(pso, Uniform::SU_mvp, matrices[i + j], 0, j);

            float colorf[4];
            Color c = Profiler.GetClearColor(0xFF00007F);
            c.GetRGBAFloat(colorf);
            ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        }
        ShaderData.Finish(drawRangeCount);
        drawPrimitive(drawRangeCount * 6, drawRangeCount);
    }
}

void HAL::setBatchUnitSquareVertexStream()
{
    VkBuffer vb = Cache.GetMaskEraseBatchVB();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, &vb, &offset);
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    vkCmdDraw(pCommandBuffer, indexCount, meshCount, 0, 0);
    SF_UNUSED(meshCount);
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
}

void HAL::drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount,
                                UPInt indexOffset, UPInt vertexOffset)
{
    vkCmdDrawIndexed(pCommandBuffer, indexCount, 1, (uint32_t)indexOffset, (int32_t)vertexOffset, 0);
    SF_UNUSED2(meshCount, vertexCount);
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
}

void HAL::drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount,
                                UPInt indexOffset, UPInt vertexOffset)
{
    vkCmdDrawIndexed(pCommandBuffer, indexCount, meshCount, (uint32_t)indexOffset, (int32_t)vertexOffset, 0);
    SF_UNUSED(vertexCount);
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
}

void HAL::drawScreenQuad()
{
    // Pipeline is bound by ShaderInterface::Finish, which is called before drawScreenQuad.
    setBatchUnitSquareVertexStream();
    drawPrimitive(6, 1);
}


bool HAL::shouldRenderFilters(const FilterPrimitive* prim) const
{
    SF_UNUSED(prim);
    return true;
}


// *** Events

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
    Vulkan::RenderEvent::UpdateCommandBuffer(pCommandBuffer);
    return Events[eventType];
}


// *** Offscreen render pass

bool HAL::createOffscreenRenderPass()
{
    VkAttachmentDescription colorAtt = {};
    colorAtt.format         = VK_FORMAT_R8G8B8A8_UNORM;
    colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
    // CLEAR + UNDEFINED: every use of this render pass starts from a clean slate.
    // Using LOAD_OP_CLEAR (not DONT_CARE) guarantees the hardware zeroes the
    // attachment at render-pass start, preventing stale data from recycled
    // render-target textures from bleeding through filter backgrounds.
    // initialLayout=UNDEFINED is compatible with both freshly-created images and
    // images that were previously used as shader resources (SHADER_READ_ONLY_OPTIMAL).
    colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;

    // dep[0]: external -> subpass: wait for prior color output before writing.
    VkSubpassDependency deps[2] = {};
    deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass    = 0;
    deps[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    // dep[1]: subpass -> external: ensure color writes are visible to subsequent
    // fragment shader reads (the render target is sampled as a texture next pass).
    deps[1].srcSubpass    = 0;
    deps[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo rpCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpCI.attachmentCount = 1;
    rpCI.pAttachments    = &colorAtt;
    rpCI.subpassCount    = 1;
    rpCI.pSubpasses      = &subpass;
    rpCI.dependencyCount = 2;
    rpCI.pDependencies   = deps;

    SF_VK_CHECK_RETURN(vkCreateRenderPass(pDevice, &rpCI, nullptr, &pOffscreenRenderPass), false);
    return true;
}

bool HAL::createOffscreenRenderPassDS()
{
    // Query supported depth/stencil format (same fallback order as DepthStencilSurface::Initialize)
    VkFormat dsCandidates[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };
    VkFormat dsFormat = VK_FORMAT_UNDEFINED;
    for (auto fmt : dsCandidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pPhysicalDevice, fmt, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            dsFormat = fmt;
            break;
        }
    }
    if (dsFormat == VK_FORMAT_UNDEFINED)
        return false;

    // Color attachment (same as the non-DS version)
    VkAttachmentDescription attachments[2] = {};
    attachments[0].format         = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth/stencil attachment
    attachments[1].format         = dsFormat;
    attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference dsRef = {};
    dsRef.attachment = 1;
    dsRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorRef;
    subpass.pDepthStencilAttachment = &dsRef;

    VkSubpassDependency deps[2] = {};
    deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass    = 0;
    deps[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    deps[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    deps[1].srcSubpass    = 0;
    deps[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo rpCI = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpCI.attachmentCount = 2;
    rpCI.pAttachments    = attachments;
    rpCI.subpassCount    = 1;
    rpCI.pSubpasses      = &subpass;
    rpCI.dependencyCount = 2;
    rpCI.pDependencies   = deps;

    SF_VK_CHECK_RETURN(vkCreateRenderPass(pDevice, &rpCI, nullptr, &pOffscreenRenderPassDS), false);
    return true;
}

void HAL::destroyOffscreenRenderPass()
{
    if (pOffscreenRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(pDevice, pOffscreenRenderPass, nullptr);
        pOffscreenRenderPass = VK_NULL_HANDLE;
    }
    if (pOffscreenRenderPassDS != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(pDevice, pOffscreenRenderPassDS, nullptr);
        pOffscreenRenderPassDS = VK_NULL_HANDLE;
    }
}

VkFramebuffer HAL::createFramebuffer(VkImageView colorView, VkImageView dsView,
                                     unsigned width, unsigned height, VkRenderPass rp)
{
    // Spec: width and height must be > 0 (VUID-VkFramebufferCreateInfo-width-00885/height-00887).
    if (width == 0 || height == 0)
    {
        SF_DEBUG_WARNING2(1, "Vulkan: skipping framebuffer creation with zero extent (%ux%u)", width, height);
        return VK_NULL_HANDLE;
    }

    VkImageView attachments[2];
    unsigned attachCount = 0;
    if (colorView != VK_NULL_HANDLE)
        attachments[attachCount++] = colorView;
    if (dsView != VK_NULL_HANDLE)
        attachments[attachCount++] = dsView;

    VkFramebufferCreateInfo fbCI = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbCI.renderPass      = rp;
    fbCI.attachmentCount = attachCount;
    fbCI.pAttachments    = attachments;
    fbCI.width           = width;
    fbCI.height          = height;
    fbCI.layers          = 1;

    VkFramebuffer fb = VK_NULL_HANDLE;
    VkResult r = vkCreateFramebuffer(pDevice, &fbCI, nullptr, &fb);
    if (r != VK_SUCCESS)
        SF_DEBUG_WARNING1(1, "Vulkan: Failed to create framebuffer, VkResult=%d", (int)r);
    return fb;
}


// *** Blend descriptions

void HAL::initBlendDescriptions()
{
    static const VkBlendOp BlendOps[BlendOp_Count] =
    {
        VK_BLEND_OP_ADD,              // BlendOp_ADD
        VK_BLEND_OP_MAX,              // BlendOp_MAX
        VK_BLEND_OP_MIN,              // BlendOp_MIN
        VK_BLEND_OP_REVERSE_SUBTRACT, // BlendOp_REVSUBTRACT
    };
    static const VkBlendFactor BlendFactors[BlendFactor_Count] =
    {
        VK_BLEND_FACTOR_ZERO,                // BlendFactor_ZERO
        VK_BLEND_FACTOR_ONE,                 // BlendFactor_ONE
        VK_BLEND_FACTOR_SRC_ALPHA,           // BlendFactor_SRCALPHA
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // BlendFactor_INVSRCALPHA
        VK_BLEND_FACTOR_DST_COLOR,           // BlendFactor_DESTCOLOR
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, // BlendFactor_INVDESTCOLOR
    };

    memset(BlendDescs, 0, sizeof(BlendDescs));

    for (unsigned i = 0; i < BlendTypeCount; i++)
    {
        VkPipelineColorBlendAttachmentState& d = BlendDescs[i];

        if (i >= (unsigned)(Blend_Count * 2))
        {
            // Write_None: disable all color writes
            d.blendEnable    = VK_FALSE;
            d.colorWriteMask = 0;
            continue;
        }

        d.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        d.blendEnable = VK_TRUE;

        bool     sourceAc = (i >= (unsigned)Blend_Count);
        unsigned mode     = i % Blend_Count;

        const BlendModeDescriptor& bmd = BlendModeTable[mode];
        d.colorBlendOp        = BlendOps[bmd.Operator];
        d.srcColorBlendFactor = BlendFactors[bmd.SourceColor];
        d.dstColorBlendFactor = BlendFactors[bmd.DestColor];
        d.alphaBlendOp        = BlendOps[bmd.Operator];
        d.srcAlphaBlendFactor = BlendFactors[bmd.SourceAlpha];
        d.dstAlphaBlendFactor = BlendFactors[bmd.DestAlpha];

        // For source-AC (premultiplied alpha): replace SRC_ALPHA with ONE for color source
        if (sourceAc && d.srcColorBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA)
            d.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    }
}


void HAL::initDepthStencilDescriptions()
{
    memset(DepthStencilDescs, 0, sizeof(DepthStencilDescs));

    for (unsigned i = 0; i < StencilMode_Count; i++)
    {
        DepthStencilDescs[i].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        DepthStencilDescs[i].depthTestEnable  = VK_FALSE;
        DepthStencilDescs[i].depthWriteEnable = VK_FALSE;
        DepthStencilDescs[i].stencilTestEnable = VK_FALSE;
    }

    // StencilMode_Available_WriteMask: write stencil
    auto& wm = DepthStencilDescs[StencilMode_Available_WriteMask];
    wm.stencilTestEnable = VK_TRUE;
    wm.front.failOp      = VK_STENCIL_OP_KEEP;
    wm.front.depthFailOp = VK_STENCIL_OP_KEEP;
    wm.front.passOp      = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    wm.front.compareOp   = VK_COMPARE_OP_ALWAYS;
    wm.front.compareMask = 0xFF;
    wm.front.writeMask   = 0xFF;
    wm.back = wm.front;

    // StencilMode_Available_TestMask: test stencil
    auto& tm = DepthStencilDescs[StencilMode_Available_TestMask];
    tm.stencilTestEnable = VK_TRUE;
    tm.front.failOp      = VK_STENCIL_OP_KEEP;
    tm.front.depthFailOp = VK_STENCIL_OP_KEEP;
    tm.front.passOp      = VK_STENCIL_OP_KEEP;
    tm.front.compareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    tm.front.compareMask = 0xFF;
    tm.front.writeMask   = 0x00;
    tm.back = tm.front;

    // StencilMode_Available_ClearMasks: unconditionally write reference value into stencil.
    // Matches D3D1x: COMPARISON_ALWAYS + STENCIL_OP_REPLACE (on both pass and depth-fail).
    auto& cm = DepthStencilDescs[StencilMode_Available_ClearMasks];
    cm.stencilTestEnable = VK_TRUE;
    cm.front.failOp      = VK_STENCIL_OP_KEEP;
    cm.front.depthFailOp = VK_STENCIL_OP_REPLACE;
    cm.front.passOp      = VK_STENCIL_OP_REPLACE;
    cm.front.compareOp   = VK_COMPARE_OP_ALWAYS;
    cm.front.compareMask = 0xFF;
    cm.front.writeMask   = 0xFF;
    cm.back = cm.front;

    // StencilMode_Available_ClearMasksAbove: write reference value only where stencil >= ref.
    // Matches D3D1x: COMPARISON_LESS_EQUAL (ref <= buffer) + STENCIL_OP_REPLACE.
    // Used to restore a nested mask level by clamping any higher stencil values back down.
    auto& cma = DepthStencilDescs[StencilMode_Available_ClearMasksAbove];
    cma.stencilTestEnable = VK_TRUE;
    cma.front.failOp      = VK_STENCIL_OP_KEEP;
    cma.front.depthFailOp = VK_STENCIL_OP_KEEP;
    cma.front.passOp      = VK_STENCIL_OP_REPLACE;
    cma.front.compareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    cma.front.compareMask = 0xFF;
    cma.front.writeMask   = 0xFF;
    cma.back = cma.front;

    // StencilMode_DepthOnly_WriteMask
    auto& dw = DepthStencilDescs[StencilMode_DepthOnly_WriteMask];
    dw = wm;
    dw.depthTestEnable  = VK_TRUE;
    dw.depthWriteEnable = VK_TRUE;
    dw.depthCompareOp   = VK_COMPARE_OP_ALWAYS;

    // StencilMode_DepthOnly_TestMask
    auto& dt = DepthStencilDescs[StencilMode_DepthOnly_TestMask];
    dt = tm;
    dt.depthTestEnable  = VK_TRUE;
    dt.depthCompareOp   = VK_COMPARE_OP_ALWAYS;
}

}}} // Scaleform::Render::Vulkan
