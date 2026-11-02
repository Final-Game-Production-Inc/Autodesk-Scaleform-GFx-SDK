/**************************************************************************

Filename    :   Vulkan_HAL.cpp
Content     :   Vulkan HAL implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Render/Vulkan/Vulkan_HAL.h"
#include "Render/Render_DebugEnv.h"
#include "Render/Render_BufferGeneric.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Vulkan/Vulkan_Events.h"
#include "Render/Render_TextureCacheGeneric.h"

// NOTE: this translation unit must not include C++ standard-library headers
// (<thread>, <mutex>, <string>, <vector>, ...). Every project here is compiled
// with OmitDefaultLibName (/Zl) and links against release-built third-party
// archives (FFmpeg/x265/CRI...), which works only because the engine's own
// objects emit no CRT "#pragma detect_mismatch" records. The STL headers do
// emit them (_ITERATOR_DEBUG_LEVEL / RuntimeLibrary), and that single object is
// then flagged by the linker as LNK2038 against the release archives.
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

namespace Scaleform { namespace Render { namespace Vulkan {

// *** Filter debugging helpers (VKFILTER project)

namespace {

// Maps the names accepted by GFX_VK_FILTER_ALLOW to FilterType values, so a filter
// bisect can be driven from the environment instead of rebuilding per FilterType.
struct VkFilterName
{
    const char* pName;
    unsigned    Type;
};

static const VkFilterName VkFilterNames[] =
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

// Returns a bit mask of allowed FilterType values. All-ones means "every type allowed"
// (the default); GFX_VK_FILTER_ALLOW=none yields 0, i.e. no filtered primitive is drawn.
static unsigned GetVkFilterAllowMask()
{
    static unsigned sMask = 0xFFFFFFFFu;
    static bool     sInit = false;
    if (sInit)
        return sMask;

    sInit = true;

    char envValue[256];
    if (!GetDebugEnvValue("GFX_VK_FILTER_ALLOW", envValue, sizeof(envValue)) || !envValue[0])
        return sMask;
    const char* pEnv = envValue;

    sMask = 0;
    const char* p = pEnv;
    while (*p)
    {
        const char* pEnd = p;
        while (*pEnd && *pEnd != ',' && *pEnd != ';' && *pEnd != ' ')
            ++pEnd;
        const size_t len = (size_t)(pEnd - p);

        if (len == 4 && !_strnicmp(p, "none", 4))
            return sMask;

        for (size_t i = 0; i < sizeof(VkFilterNames) / sizeof(VkFilterNames[0]); ++i)
        {
            if (strlen(VkFilterNames[i].pName) == len &&
                !_strnicmp(p, VkFilterNames[i].pName, len))
                sMask |= (1u << VkFilterNames[i].Type);
        }

        p = pEnd;
        while (*p == ',' || *p == ';' || *p == ' ')
            ++p;
    }
    return sMask;
}

} // anonymous namespace


// Static members for deferred framebuffer deletion
Array<VkFramebuffer> RenderTargetData::PendingDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];
Array<Ptr<DepthStencilBuffer> > RenderTargetData::PendingDSDeletes[SF_VK_MAX_FRAMES_IN_FLIGHT];
VkDevice             RenderTargetData::PendingDeleteDevice = VK_NULL_HANDLE;
VkQueue              RenderTargetData::PendingDeleteQueue = VK_NULL_HANDLE;
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
  pOffscreenResumeRenderPass(VK_NULL_HANDLE),
  pOffscreenResumeRenderPassDS(VK_NULL_HANDLE),
  InRenderPass(false),
  MainPassActive(false),
  pMainRenderPass(VK_NULL_HANDLE),
  pMainResumeRenderPass(VK_NULL_HANDLE),
  pMainFramebuffer(VK_NULL_HANDLE),
  MainRenderExtent{0, 0},
  FrameSlot(0),
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
    MainPassActive     = false;
    RenderTargetData::PendingDeleteQueue = pGraphicsQueue;

    Vulkan::RenderEvent::InitializeEvents(pInstance, pCommandBuffer);

    if (!createPipelineLayout()) { ShutdownHAL(); return false; }
    if (!createOffscreenRenderPass()) { ShutdownHAL(); return false; }
    if (!createOffscreenRenderPassDS()) { ShutdownHAL(); return false; }
    if (!createOffscreenResumeRenderPass()) { ShutdownHAL(); return false; }
    if (!createOffscreenResumeRenderPassDS()) { ShutdownHAL(); return false; }
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

    // Vulkan destroys a temporary target's depth/stencil buffer one frame after it was
    // detached (see HAL::PopRenderTarget / PendingTargetReleases): its image view is still
    // referenced by a framebuffer that is only destroyed once the frame slot's fence
    // has been waited on. Tell the buffer manager that such a buffer is legitimately
    // still in its 'in-use' list at EndFrame().
    if (pRenderBufferManager)
        pRenderBufferManager->AllowsDeferredBufferRelease = true;

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
    RenderTargetData::DrainAllPendingDSDeletes();
    PendingTargetReleases.Clear();

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
    RenderTargetData::DrainAllPendingDSDeletes();

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

static void VkTrace(const char* what);

// Debug watchdog: when GFX_VK_WATCHDOG is set and no frame has started for a few
// seconds, break into the debugger so the (otherwise silent) render-thread hang can
// be inspected. Only armed for the GFX_VK_FILTER debug sessions.
static volatile LONG gVkFrameTick = 0;
// _beginthreadex entry point: keeps the CRT's per-thread state valid without
// pulling in <thread> (see the include comment at the top of this file).
static unsigned __stdcall VkWatchdogMain(void*)
{
    LONG last = 0;
    int  strikes = 0;
    for (;;)
    {
        Sleep(1000);
        LONG now = gVkFrameTick;
        if (now == last)
        {
            if (++strikes >= 5)
            {
                fprintf(stderr, "[WATCHDOG] no frame start for 5s - breaking\n");
                fflush(stderr);
                DebugBreak();
                strikes = 0;
            }
        }
        else
        {
            last = now;
            strikes = 0;
        }
    }
}

bool HAL::BeginFrame()
{
    VkTrace("begin");
    static const bool sWatchdogEnabled = IsDebugEnvSet("GFX_VK_WATCHDOG");
    if (sWatchdogEnabled)
    {
        static bool sWatchdogStarted = false;
        if (!sWatchdogStarted)
        {
            sWatchdogStarted = true;
            // Detached: it only exists to break into the debugger on a render hang.
            uintptr_t hThread = _beginthreadex(0, 0, VkWatchdogMain, 0, 0, 0);
            if (hThread)
                CloseHandle((HANDLE)hThread);
        }
        InterlockedIncrement(&gVkFrameTick);
    }
    if (!BaseHAL::BeginFrame())
        return false;

    // Advance the frame counter once per presented frame, and switch to the GPU
    // frame slot the platform is recording into. BeginFrame is called once per
    // presented frame, whereas BeginScene/EndScene may run several times within
    // it (prepass, normal, overlay). Using the platform's slot keeps the
    // descriptor pool, UBO ring and deferred-delete queues in lock-step with the
    // per-frame fences; using a counter of our own would lag by one frame and
    // let the CPU overwrite resources the GPU is still reading.
    ++CurrentFrame;
    unsigned frameIdx = FrameSlot % SF_VK_MAX_FRAMES_IN_FLIGHT;
    vkResetDescriptorPool(pDevice, FrameDescriptorPools[frameIdx], 0);
    UBORingOffset = 0;
    return true;
}

void HAL::DrainFrameResources()
{
    // Safe point: the platform has just waited on this frame slot fence.
    unsigned frameIdx = FrameSlot % SF_VK_MAX_FRAMES_IN_FLIGHT;
    RenderTargetData::SetPendingDeleteSlot(frameIdx);

    // 1) Framebuffers first: they reference the colour/depth image views of the targets
    //    below, so they must be gone before those views may be destroyed. The delete
    //    queue is per frame slot, and DrainPendingDeletes() has just waited for the
    //    queue to go idle, so drain every slot here as well: a framebuffer queued for
    //    slot S must not outlive a target retired from a different slot.
    RenderTargetData::DrainPendingDeletes();
    RenderTargetData::DrainAllPendingDeletes();

    // 2) Depth/stencil buffers that were detached by UpdateData() (or by a pop) while
    //    their framebuffer was still pending. The framebuffers are gone now, so the
    //    views may finally die.
    RenderTargetData::DrainAllPendingDSDeletes();

    // 3) Then retire the temporary targets themselves, in the same order the GPU saw
    //    them: depth/stencil buffer, then the colour texture (dropped with the target).
    for (unsigned i = 0; i < PendingTargetReleases.GetSize(); ++i)
    {
        DeferredTargetRelease& r = PendingTargetReleases[i];        r.pDepthStencil = 0;
        if (r.pTarget)
            r.pTarget->SetInUse(RTUse_Unused);
        r.pTarget = 0;
    }
    PendingTargetReleases.Clear();

    Vulkan::TextureManager* ptm = (Vulkan::TextureManager*)pTextureManager.GetPtr();
    if (ptm)
    {
        ptm->SetKillSlot(frameIdx);
        ptm->DrainDeferredKills();
        ptm->DeferKills = true;
    }
}

bool HAL::BeginScene()
{
    if (!BaseHAL::BeginScene())
        return false;

    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__);
    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);

    // Temporary targets popped during this scene stay in the buffer manager's 'in-use'
    // list until DrainFrameResources() (after the slot fence) - see
    // PendingTargetReleases. RenderBufferManager::EndFrame() is told about that through
    // AllowsDeferredBufferRelease.
    if (!BaseHAL::EndScene())
        return false;

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
	SF_UNUSED(pbatch);
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
	SF_UNUSED(formatIndex);

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
	SF_UNUSED(numExtraBindings);

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
    VkTrace("pipe");
    VkResult r = vkCreateGraphicsPipelines(pDevice, pPipelineCache, 1, &pipelineCI, nullptr, &pipeline);
    VkTrace("pipe-done");
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
        // Same ownership rule as CreateTempRenderTarget: the returned buffer already
        // carries a reference, so it has to be held by a Ptr or the manager keeps it in
        // its 'in-use' list forever.
        Ptr<DepthStencilBuffer> pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
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

    // Bisect aid: GFX_VK_FILTER_NODS=1 renders filtered primitives without a
    // depth/stencil attachment, to isolate problems in the masked-filter path.
    static const bool sNoDS = IsDebugEnvSet("GFX_VK_FILTER_NODS");
    if (sNoDS)
        needsStencil = false;

    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(safeSize);
    if (!prt) return 0;
    Render::Texture* pt = prt->GetTexture();
    if (!pt) return 0;

    if (needsStencil)
    {
        // Use the size the render buffer manager actually allocated for the color
        // texture: it may round the extent up (e.g. 30x42 -> 32x64), and creating the
        // depth buffer from the unrounded request made vkCreateFramebuffer fail with
        // "depth attachment is smaller than the framebuffer" - the filter output then
        // stayed black.
        ImageSize rtSize = pt->GetSize();
        if (rtSize.Width <= 0 || rtSize.Height <= 0)
            rtSize = safeSize;
        // Borrow the DS the same way D3D11 does (D3D1x_HAL::CreateTempRenderTarget):
        // CreateDepthStencilBuffer returns an already-AddRef'ed buffer, and the caller
        // owns that reference. Holding it in a plain raw pointer dropped the reference
        // on the floor: a *reused* DS stays in RenderBufferManager's RBCL_InUse list
        // forever, so the next RenderBufferManager::EndFrame trips the debug assert
        // "there are render targets still 'in-use'" (and release builds recycle a
        // buffer that is still attached to a live render target).
        Ptr<DepthStencilBuffer> pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(rtSize);
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

// Debug trace: enabled while the file named by GFX_VK_FP exists. Used to find the
// exact HAL call the render thread is stuck in when the picture freezes.
static bool VkTraceOn()
{
    // The path is constant for the process; the marker *file* is re-checked every call so
    // a harness can arm/disarm the trace while the player is running.
    static char sTracePath[512];
    static const bool sConfigured = GetDebugEnvValue("GFX_VK_FP", sTracePath, sizeof(sTracePath));
    if (!sConfigured || !sTracePath[0]) return false;
    FILE* f = 0;
    fopen_s(&f, sTracePath, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}
static void VkTrace(const char* what)
{
    if (!VkTraceOn()) return;
    FILE* f = 0;
    fopen_s(&f, "vktrace.log", "a");
    if (f) { fprintf(f, "%s\n", what); fclose(f); }
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags, Color clearColor)
{
	SF_UNUSED(flags);

    VkTrace("push");
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

    // Refresh the color attachment from the render target's current texture.
    // Temporary render targets are recycled from a pool, and their backing
    // texture's VkImageView can be destroyed and recreated (deferred kills);
    // a cached RenderTargetData would then hand a stale/nulled VkImageView to
    // vkCreateFramebuffer ("Invalid VkImageView Object ..."), which the
    // validation layer reports and can crash inside its own bookkeeping.
    if (phd)
    {
        Render::Texture* ptex = prt->GetTexture();
        if (ptex && ((Vulkan::Texture*)ptex)->pTextures)
        {
            VkImageView curView = ((Vulkan::Texture*)ptex)->pTextures[0].View;
            if (curView != phd->pRenderSurface)
            {
                RenderTargetData::QueueFramebufferDelete(phd->pFramebuffer);
                phd->pFramebuffer   = VK_NULL_HANDLE;
                phd->pRenderSurface = curView;
            }
        }
        if (phd->pRenderSurface == VK_NULL_HANDLE)
            phd->pFramebuffer = VK_NULL_HANDLE;   // nothing to render into
    }

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

        // The cached framebuffer is only valid for the render pass it was
        // created with (the DS pass has an extra attachment), so recreate it if
        // this target is now used with/without a depth-stencil buffer.
        if (phd->pFramebuffer != VK_NULL_HANDLE && phd->FramebufferRenderPass != selectedRP)
        {
            RenderTargetData::QueueFramebufferDelete(phd->pFramebuffer);
            phd->pFramebuffer = VK_NULL_HANDLE;
        }

        if (phd->pFramebuffer == VK_NULL_HANDLE)
        {
            phd->pFramebuffer = createFramebuffer(phd->pRenderSurface, phd->pDSSurface,
                                                  bs.Width, bs.Height, selectedRP);
            phd->FramebufferRenderPass = (phd->pFramebuffer != VK_NULL_HANDLE) ? selectedRP : VK_NULL_HANDLE;
        }

        static const bool sRtTrace = IsDebugEnvSet("GFX_VKRT");
    if (sRtTrace)
            fprintf(stderr, "[VKRT] fb=%p rp=%p rect=%.0fx%.0f temp=%d\n",
                    (void*)phd->pFramebuffer, (void*)selectedRP, frameRect.Width(), frameRect.Height(),
                    (int)(prt->GetType() == RBuffer_Temporary)), fflush(stderr);
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
    VkTrace("push-done");
}

// Debug-only helper: copy a render-target image (already in SHADER_READ_ONLY layout)
// into a host buffer and write it as a 24-bit BMP. Used by GFX_VK_DUMP_RT to see what
// a filter pass actually produced.
static void DumpVkImageToBmp(VkDevice device, VkPhysicalDevice physDevice, VkQueue queue,
                             VkCommandPool pool, VkImage image, UInt32 w, UInt32 h, const char* path)
{
    if (!w || !h) return;
    fprintf(stderr, "[RTDUMP] writing %s (%ux%u) image=%p\n", path, w, h, (void*)image);
    fflush(stderr);

    VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bci.size = (VkDeviceSize)w * h * 4;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer buf = VK_NULL_HANDLE;
    if (vkCreateBuffer(device, &bci, nullptr, &buf) != VK_SUCCESS) return;

    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(device, buf, &req);
    VkMemoryAllocateInfo mai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = FindMemoryType(physDevice, req.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory mem = VK_NULL_HANDLE;
    if (mai.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(device, &mai, nullptr, &mem) != VK_SUCCESS)
    {
        vkDestroyBuffer(device, buf, nullptr);
        return;
    }
    vkBindBufferMemory(device, buf, mem, 0);

    VkCommandBufferAllocateInfo cai = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cai.commandPool = pool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &cai, &cmd) == VK_SUCCESS)
    {
        VkCommandBufferBeginInfo bi = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &bi);

        VkImageMemoryBarrier b = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        b.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = image;
        b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        b.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        b.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &b);

        VkBufferImageCopy region = {};
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageExtent = { w, h, 1 };
        vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf, 1, &region);

        b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        b.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        b.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &b);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo si = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, pool, 1, &cmd);

        void* pData = nullptr;
        if (vkMapMemory(device, mem, 0, bci.size, 0, &pData) == VK_SUCCESS && pData)
        {
            const UInt32 rowSize = (w * 3 + 3) & ~3u;
            unsigned char header[54] = { 0 };
            const UInt32 fileSize = 54 + rowSize * h;
            header[0] = 'B'; header[1] = 'M';
            memcpy(header + 2, &fileSize, 4);
            UInt32 off = 54;          memcpy(header + 10, &off, 4);
            UInt32 dib = 40;          memcpy(header + 14, &dib, 4);
            memcpy(header + 18, &w, 4); memcpy(header + 22, &h, 4);
            unsigned short planes = 1, bpp = 24;
            memcpy(header + 26, &planes, 2); memcpy(header + 28, &bpp, 2);

            FILE* f = 0;
            fopen_s(&f, path, "wb");
            if (f)
            {
                fwrite(header, 1, 54, f);
                Array<unsigned char> row;
                row.Resize(rowSize);
                const unsigned char* pSrc = (const unsigned char*)pData;
                for (int y = (int)h - 1; y >= 0; --y)
                {
                    memset(row.GetDataPtr(), 0, rowSize);
                    for (UInt32 x = 0; x < w; ++x)
                    {
                        const unsigned char* p = pSrc + ((size_t)y * w + x) * 4;
                        row[x * 3 + 0] = p[2];
                        row[x * 3 + 1] = p[1];
                        row[x * 3 + 2] = p[0];
                    }
                    fwrite(row.GetDataPtr(), 1, rowSize, f);
                }
                fclose(f);
            }
            vkUnmapMemory(device, mem);
        }
    }

    vkFreeMemory(device, mem, nullptr);
    vkDestroyBuffer(device, buf, nullptr);
}

// GFX_VK_KEEP_DS=1 disables the pop-time detach of a temporary target's depth/stencil
// buffer (bisect aid for DS lifetime problems).
static bool KeepDsEnabled()
{
    static const bool sKeepDs = IsDebugEnvSet("GFX_VK_KEEP_DS");
    return sKeepDs;
}

void HAL::PopRenderTarget(unsigned flags)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_RenderTarget), 0, false);
    VkTrace("pop");

    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget*      pPoppedRT = entry.pRenderTarget;

    {   // temporary probe: confirm the dump path is reached and what it sees
        static int sPopDbg = 0;
        if (sPopDbg < 20)
        {
            ++sPopDbg;
            const ImageSize dbgSize = pPoppedRT ? pPoppedRT->GetBufferSize() : ImageSize(0,0);
            fprintf(stderr, "[POP] rt=%p type=%d size=%dx%d env=%d\n",
                    (void*)pPoppedRT, pPoppedRT ? (int)pPoppedRT->GetType() : -1,
                    (int)dbgSize.Width, (int)dbgSize.Height,
                    IsDebugEnvSet("GFX_VK_DUMP_RT") ? 1 : 0);
            fflush(stderr);
        }
    }

    if ((flags & PRT_NoSet) == 0)
    {
        if (InRenderPass)
        {
            vkCmdEndRenderPass(pCommandBuffer);
            InRenderPass = false;
        }
    }
    VkTrace("pop-a");

    // Debug: dump small temporary filter targets right after their pass ended, so the
    // image is still in SHADER_READ_ONLY layout (the offscreen passes' finalLayout).
    // GFX_VK_DUMP_RT names a marker file: dumping happens only while it exists, so the
    // harness can arm the probe right after the click of interest.
    static char sDumpMarker[512];
    static const bool sDumpConfigured = GetDebugEnvValue("GFX_VK_DUMP_RT", sDumpMarker, sizeof(sDumpMarker));
    const char* pDumpMarker = sDumpMarker;
    bool bDumpArmed = false;
    if (sDumpConfigured && pDumpMarker[0])
    {
        static bool sMarkerReported = false;
        FILE* pMarker = 0;
        fopen_s(&pMarker, pDumpMarker, "rb");
        if (pMarker) { fclose(pMarker); bDumpArmed = true; }
        if (!sMarkerReported)
        {
            sMarkerReported = true;
            fprintf(stderr, "[RTDUMP] marker '%s' -> armed=%d\n", pDumpMarker, (int)bDumpArmed);
            fflush(stderr);
        }
    }
    if (bDumpArmed && pPoppedRT && pPoppedRT->GetType() == RBuffer_Temporary)
    {
        Render::Texture* ptex = pPoppedRT->GetTexture();
        if (ptex)
        {
            Vulkan::Texture* pvt = (Vulkan::Texture*)ptex;
            const ImageSize rtSize = pPoppedRT->GetBufferSize();
            const UInt32 w = (UInt32)rtSize.Width;
            const UInt32 h = (UInt32)rtSize.Height;
            static int sDumped = 0;
            if (w && h && w <= 512 && h <= 512 && sDumped < 600)
            {
                char path[128];
                snprintf(path, sizeof(path), "rtdump_%03d_%ux%u.bmp", sDumped++, w, h);
                DumpVkImageToBmp(pDevice, pPhysicalDevice, pGraphicsQueue, pCommandPool,
                                 pvt->pTextures[0].Image, w, h, path);
            }
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
        // Only valid when the platform has actually begun the main pass for this
        // frame (MainPassActive); otherwise the swapchain image is still in
        // PRESENT_SRC_KHR and beginning a pass whose initialLayout is
        // COLOR_ATTACHMENT_OPTIMAL would violate the layout-tracking rules.
        // The resume pass keeps LOAD_OP_LOAD for color, but its depth attachment
        // is CLEAR, so a matching pClearValues array is mandatory.
        if ((flags & PRT_NoSet) == 0 &&
            MainPassActive &&
            pMainResumeRenderPass != VK_NULL_HANDLE && pMainFramebuffer != VK_NULL_HANDLE)
        {
            // No image layout transition is required here: the platform's main
            // render pass ends in VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, which
            // is exactly what pMainResumeRenderPass declares as its
            // initialLayout. (Emitting a PRESENT_SRC_KHR -> COLOR_ATTACHMENT
            // barrier here would report the wrong oldLayout to the driver and
            // trip VUID-vkCmdBeginRenderPass-initialLayout-00900.)
            VkClearValue resumeClear[2] = {};
            resumeClear[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
            rpBI.renderPass  = pMainResumeRenderPass;
            rpBI.framebuffer = pMainFramebuffer;
            rpBI.renderArea.extent = MainRenderExtent;
            rpBI.clearValueCount = 2;
            rpBI.pClearValues    = resumeClear;
            vkCmdBeginRenderPass(pCommandBuffer, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
            InRenderPass = true;
            // Key pipelines by the pass that is actually recording. Using the main pass
            // here made every draw after a filter resume bind a VkPipeline created for
            // a different VkRenderPass (validation: color format / depth attachment /
            // dependencyCount incompatible), which is undefined behaviour and made the
            // driver report VK_ERROR_DEVICE_LOST on the first vkQueueSubmit.
            pCurrentRenderPass = pMainResumeRenderPass;
        }
    }
    else if ((flags & PRT_NoSet) == 0 && !InRenderPass && RenderTargetStack.GetSize() > 1)
    {
        // Back to a parent *temporary* target that already holds rendered content (e.g. a
        // filter pass nested inside the full-scene blend target). The pass for that
        // framebuffer was ended when the child was pushed, so it has to be re-opened -
        // with the LOAD variant, or its content would be cleared. Without this the
        // following composite draws were issued with no active render pass: the driver
        // dropped them (validation: VUID-vkCmdDraw-renderpass), which is why e.g. the
        // masked gradient text of the "顺利通过" banner disappeared.
        RenderTarget* pParent = RenderTargetStack.Back().pRenderTarget;
        RenderTargetData* phdParent = pParent ? (RenderTargetData*)pParent->GetRenderTargetData() : 0;
        if (phdParent && phdParent->pFramebuffer != VK_NULL_HANDLE &&
            phdParent->pRenderSurface != VK_NULL_HANDLE)
        {
            bool hasDS = (phdParent->pDSSurface != VK_NULL_HANDLE);
            VkRenderPass rp = hasDS ? pOffscreenResumeRenderPassDS : pOffscreenResumeRenderPass;
            if (rp != VK_NULL_HANDLE)
            {
                const ImageSize& bs = pParent->GetBufferSize();
                VkClearValue resumeClear[2] = {};
                resumeClear[1].depthStencil = { 1.0f, 0 };
                VkRenderPassBeginInfo rpBI = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
                rpBI.renderPass        = rp;
                rpBI.framebuffer       = phdParent->pFramebuffer;
                rpBI.renderArea.extent = { bs.Width, bs.Height };
                rpBI.clearValueCount   = hasDS ? 2 : 1;
                rpBI.pClearValues      = resumeClear;
                vkCmdBeginRenderPass(pCommandBuffer, &rpBI, VK_SUBPASS_CONTENTS_INLINE);
                InRenderPass = true;
                pCurrentRenderPass = rp;
            }
        }
    }

    VkTrace("pop-b");
    // Pop-time cleanup of a temporary target: detach its depth/stencil buffer (a fresh
    // one is attached when the target is next used with stencil content), then park the
    // whole target - target + DS - in PendingTargetReleases. Everything that the
    // framebuffer for this target references stays alive until DrainFrameResources()
    // has destroyed that framebuffer, so no image view can die under a framebuffer that
    // is still bound to the recording command buffer.
    if (pPoppedRT && pPoppedRT->GetType() == RBuffer_Temporary)
    {
        Ptr<DepthStencilBuffer> pDs;
        RenderTargetData* phd  = (RenderTargetData*)pPoppedRT->GetRenderTargetData();
        Render::Texture*  ptex = pPoppedRT->GetTexture();
        if (phd && phd->pDepthStencilBuffer && ptex && ((Vulkan::Texture*)ptex)->pTextures &&
            !KeepDsEnabled())
        {
            pDs = phd->pDepthStencilBuffer;
            RenderTargetData::UpdateData(pPoppedRT, pDevice,
                ((Vulkan::Texture*)ptex)->pTextures[0].View, 0, VK_NULL_HANDLE, VK_NULL_HANDLE);
        }
        PendingTargetReleases.PushBack(DeferredTargetRelease(pPoppedRT, pDs));
    }
    VkTrace("pop-c");

    ++AccumulatedStats.RTChanges;
    StencilChecked = false;

    if ((flags & PRT_NoSet) == 0)
    {
        HALState |= HS_ViewValid;
        updateViewport();
    }
    VkTrace("pop-done");
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

void HAL::setBatchUnitSquareVertexStreamPerDraw()
{
    // Same binding as above: shared code calls only this variant right before
    // drawPrimitive(6,1) for filter quads, and the base implementation is empty.
    setBatchUnitSquareVertexStream();
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    // Every draw must be inside a render pass; the mask/filter passes re-open the
    // parent target's pass on pop (see PopRenderTarget), so this should never fire.
    SF_DEBUG_ASSERT(InRenderPass, "Vulkan: draw issued without an active render pass.");
    VkTrace("drawprim");
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
    VkTrace("quad");
    // Pipeline is bound by ShaderInterface::Finish, which is called before drawScreenQuad.
    setBatchUnitSquareVertexStream();
    drawPrimitive(6, 1);
    VkTrace("quad-done");
}


bool HAL::IsFilterCachingSupported() const
{
    // Cached filter results are not recycled correctly on Vulkan yet: the cached
    // render targets are re-acquired every frame by the cacheable prepare pass, but
    // the cached draw (ShaderHAL::drawCachedFilter) never consumes them, so they are
    // still 'in use' at RenderBufferManager::EndFrame (debug assert) - and in release
    // builds those targets are moved back into the frame cache and recycled while the
    // FilterPrimitive still references them.
    //
    // Rebuilding filters every frame is correct and costs one extra pass per filtered
    // primitive. Set GFX_VK_FILTER_CACHE=1 to re-enable the cached path when debugging it.
    static const bool sFilterCache = IsDebugEnvSet("GFX_VK_FILTER_CACHE");
    return sFilterCache;
}
bool HAL::shouldRenderFilters(const FilterPrimitive* prim) const
{
    // Filters are on by default: the temp-RT/framebuffer lifetime and the nested-pass
    // resume were fixed (see PopRenderTarget / PendingTargetReleases), and all four
    // reference SWFs run with a clean validation log. Set GFX_VK_NOFILTERS=1 to fall
    // back to the old "no filters" baseline.
    static const bool sNoFilters = IsDebugEnvSet("GFX_VK_NOFILTERS");
    if (sNoFilters)
        return false;

    const FilterSet* pFilters = prim ? prim->GetFilters() : 0;
    const unsigned   filterCount = pFilters ? (unsigned)pFilters->GetFilterCount() : 0u;

    // Optional per-type whitelist, used for the E2 bisect:
    //   GFX_VK_FILTER_ALLOW=shadow            -> only shadow-filtered primitives
    //   GFX_VK_FILTER_ALLOW=shadow,blur       -> either of the two
    //   GFX_VK_FILTER_ALLOW=none              -> no filters at all
    // A primitive is rendered filtered only if *every* filter it carries is allowed;
    // otherwise the content is drawn unfiltered (the normal "profile has no filters" path).
    const unsigned allowMask = GetVkFilterAllowMask();
    if (allowMask != 0xFFFFFFFFu)
    {
        for (unsigned i = 0; i < filterCount; ++i)
        {
            const unsigned type = (unsigned)pFilters->GetFilter(i)->GetFilterType();
            if (type < 32 && (allowMask & (1u << type)) == 0)
                return false;
        }
    }

    // Temporary bisect aid: keep only filters whose area width falls inside
    // [GFX_VK_FILTER_MINW, GFX_VK_FILTER_MAXW]. Used to find which filtered primitive
    // breaks the frame; remove once the remaining issue is fixed.
    if (prim && prim->GetAreaMatrixCount() > 0)
    {
        const float areaW = prim->GetAreaMatrix().GetMatrix2D().Sx();
        static char sMinWBuf[32];
        static const bool sHasMinW = GetDebugEnvValue("GFX_VK_FILTER_MINW", sMinWBuf, sizeof(sMinWBuf));
        if (sHasMinW && areaW < (float)atoi(sMinWBuf)) return false;
        static char sMaxWBuf[32];
        static const bool sHasMaxW = GetDebugEnvValue("GFX_VK_FILTER_MAXW", sMaxWBuf, sizeof(sMaxWBuf));
        if (sHasMaxW && areaW > (float)atoi(sMaxWBuf)) return false;
    }

    // One line per filtered primitive (capped), to see which filter type / area is in
    // play and whether a single filter covers the whole stage - the prime suspect for
    // a "whole screen is black" symptom. Cached primitives may not carry an area
    // matrix, so only read it when present (GetAreaMatrix() indexes without a check).
    static const bool sFilterLog = IsDebugEnvSet("GFX_VK_FLTLOG");
    if (sFilterLog && prim)
    {
        static unsigned sLogged = 0;
        if (sLogged < 20000)
        {
            const bool      hasArea = prim->GetAreaMatrixCount() > 0;
            const Matrix2F* pArea   = hasArea ? &prim->GetAreaMatrix().GetMatrix2D() : 0;
            fprintf(stderr, "[VKFLT] n=%u t0=%d t1=%d t2=%d area=%.0fx%.0f at=%.0f,%.0f cache=%d\n",
                    filterCount,
                    (filterCount > 0) ? (int)pFilters->GetFilter(0)->GetFilterType() : -1,
                    (filterCount > 1) ? (int)pFilters->GetFilter(1)->GetFilterType() : -1,
                    (filterCount > 2) ? (int)pFilters->GetFilter(2)->GetFilterType() : -1,
                    pArea ? pArea->Sx() : 0.0f, pArea ? pArea->Sy() : 0.0f,
                    pArea ? pArea->Tx() : 0.0f, pArea ? pArea->Ty() : 0.0f,
                    (int)prim->GetCacheState());
            fflush(stderr);
            ++sLogged;
        }
    }

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
    if (pOffscreenResumeRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(pDevice, pOffscreenResumeRenderPass, nullptr);
        pOffscreenResumeRenderPass = VK_NULL_HANDLE;
    }
    if (pOffscreenResumeRenderPassDS != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(pDevice, pOffscreenResumeRenderPassDS, nullptr);
        pOffscreenResumeRenderPassDS = VK_NULL_HANDLE;
    }
}

// Resume variants of the offscreen passes: LOAD_OP_LOAD, entering from the layout the
// clear pass leaves behind (SHADER_READ_ONLY / DEPTH_STENCIL_ATTACHMENT). They are used
// when a nested pass is popped and drawing continues in a parent temporary target that
// already holds content (e.g. a masked gradient drawn into the full-scene blend target).
// Framebuffers created for the CLEAR passes are compatible with these (same formats and
// attachment layouts), so no second framebuffer cache is needed.
bool HAL::createOffscreenResumeRenderPass()
{
    VkAttachmentDescription colorAtt = {};
    colorAtt.format         = VK_FORMAT_R8G8B8A8_UNORM;
    colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorAtt.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;

    VkSubpassDependency deps[2] = {};
    deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass    = 0;
    // Dependencies must match createOffscreenRenderPass() exactly: render-pass
    // compatibility (VUID-VkRenderPassBeginInfo-renderPass-00904) includes them, so the
    // resume pass may only differ in loadOp/initialLayout.
    deps[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

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

    SF_VK_CHECK_RETURN(vkCreateRenderPass(pDevice, &rpCI, nullptr, &pOffscreenResumeRenderPass), false);
    return true;
}

bool HAL::createOffscreenResumeRenderPassDS()
{
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

    VkAttachmentDescription attachments[2] = {};
    attachments[0].format         = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format         = dsFormat;
    attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

    SF_VK_CHECK_RETURN(vkCreateRenderPass(pDevice, &rpCI, nullptr, &pOffscreenResumeRenderPassDS), false);
    return true;
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
    VkResult r = vkCreateFramebuffer(pDevice, &fbCI, nullptr, &fb);    if (r != VK_SUCCESS)
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



















