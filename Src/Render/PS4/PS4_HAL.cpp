/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_HAL.cpp
Content     :   
Created     :   2012/09/19
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "PS4_HAL.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_BufferGeneric.h"
#if !defined(SF_BUILD_SHIPPING)
#include "Render/PS4/PS4_Events.h"
#endif

namespace Scaleform { namespace Render { namespace PS4 {

//--------------------------------------------------------------------
HAL::HAL(ThreadCommandQueue* commandQueue)
:   Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    GnmxCtx(0),
    GnmxAllocatedContext(0),
    CueCpRamShadowBuffer(0),
    RSync(getThis()),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::Console_Defaults, &RSync),
    PrevBatchType(PrimitiveBatch::DP_None)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

bool HAL::InitHAL(const PS4::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    pMemoryManager = reinterpret_cast<MemoryManager*>(params.pMemoryManager);
    SF_DEBUG_ASSERT(pMemoryManager, "Error, must provide a memory manager in Orbis::HALInitParams");

    // If not GnmxContext is provided, allocate one now.
    if (params.GnmxCtx == 0)
        allocateGnmxContext(params);
    else
        GnmxCtx = params.GnmxCtx;

    RSync.SetContext(GnmxCtx, *pMemoryManager);

    if ( SManager.Initialize(this, *params.pMemoryManager) &&
         Cache.Initialize(reinterpret_cast<PS4::MemoryManager*>(params.pMemoryManager)))
    {
        // Create Texture manager if needed.
        if (params.pTextureManager)
            pTextureManager = params.GetTextureManager();
        else
        {
            pTextureManager = 
                *SF_HEAP_AUTO_NEW(this) TextureManager(GnmxCtx, params.pMemoryManager);
            if (!pTextureManager)
            {
                Cache.Reset();
                SManager.Reset(*params.pMemoryManager);
            }
        }

        // Create RenderBufferManager if needed.
        if (params.pRenderBufferManager)
            pRenderBufferManager = params.pRenderBufferManager;
        else
        {
            // Create the default render target, and manager.
            pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_Exact);
            if ( !pRenderBufferManager || !createDefaultRenderBuffer())
            {
                ShutdownHAL();
                return false;
            }
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
    return false;
}

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    destroyRenderBuffers();
    pRenderBufferManager.Clear();


    // Do TextureManager::Reset to ensure shutdown on the current thread.
    pTextureManager->Reset();
    pTextureManager.Clear();
    SManager.Reset(*pMemoryManager);
    Cache.Reset();

    freeGnmxCtx();

    return true;
}

void HAL::freeGnmxCtx()
{
    // If the Gnmx::GfxContext was allocated by GFx, free it now.
    if (isGnmxCtxAllocatedInternally())
    {
        // TBD: determine whether destruction of internal DCB/CCB resources are handled in ~GfxContext.
        if (GnmxAllocatedContext)
        {
#ifndef SF_PS4_USE_LCUE
            GnmxAllocatedContext->~GfxContext();
#else
            GnmxAllocatedContext->~GfxContext();
#endif
            SF_FREE((void*)GnmxAllocatedContext);
            GnmxAllocatedContext = 0;
        }
    }
}

bool ContextOutOfSpaceCallback(sce::Gnm::CommandBuffer * cb, uint32_t sizeInDwords, void* userData)
{
    // Attempt to submit the current context.
    PS4::HAL* hal = reinterpret_cast<PS4::HAL*>(userData);
    hal->SubmitAndResetStates();
    return true;
}

void HAL::allocateGnmxContext( const PS4::HALInitParams &params )
{
    // If they did not provide a Gnmx::GfxContext, then allocate one.
    /*if (GnmxAllocatedContext == 0)
    {
#ifndef SF_PS4_USE_LCUE
        // Note that if CueCpRamShadowBuffer is non-NULL, this indicates that GFx created this Gnmx::GfxContext, so that it will know to delete it on Shutdown.
        void* gnmxCtxMem = SF_ALLOC(sizeof(sce::Gnmx::GfxContext), StatRender_Mem);
        GnmxAllocatedContext = new(gnmxCtxMem) sce::Gnmx::GfxContext();

        // Initialzie the Gnmx::GfxContext.
        const uint32_t cueCpRamShadowSize = sce::Gnmx::ConstantUpdateEngine::computeCpRamShadowSize();
        CueCpRamShadowBuffer = SF_ALLOC(cueCpRamShadowSize, StatRender_Mem);

        void * dcbBuffer     = pMemoryManager->Alloc(params.DCBSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION);
        void * ccbBuffer     = pMemoryManager->Alloc(params.CCBSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION);
        const uint32_t numRingEntries = 16;
        const uint32_t cueHeapSize = sce::Gnmx::ConstantUpdateEngine::computeHeapSize(numRingEntries);
        void * cueHeapBuffer = pMemoryManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WC_GARLIC);
        GnmxAllocatedContext->init(CueCpRamShadowBuffer, cueHeapBuffer, numRingEntries, dcbBuffer, params.DCBSize, ccbBuffer, params.CCBSize);

        // Set the out-of-space callback.
        GnmxAllocatedContext->m_bufferFullCallback = { ContextOutOfSpaceCallback, this };
#else
        const uint32_t bufferSize  = params.DCBSize;
        const uint32_t cueHeapSize = params.CCBSize;

        void * dcbBuffer     = pMemoryManager->Alloc(bufferSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_WB_ONION_NONVOLATILE);
        void * cueHeapBuffer = pMemoryManager->Alloc(cueHeapSize, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);

        GnmxAllocatedContext->init(reinterpret_cast<uint32_t*>(dcbBuffer), bufferSize / sizeof(uint32_t), 
            reinterpret_cast<uint32_t*>(cueHeapBuffer), cueHeapSize / sizeof(uint32_t),
            0, ContextOutOfSpaceCallback, this);
#endif

		// Set the current context.
        GnmxCtx = GnmxAllocatedContext;
    }*/
}

// This call submits the current command buffers, and resets all render states, such that rendering can continue. This method
// is required for the Orbis::RenderSync, as it must submit the current context for the GPU to finish using resources.
void HAL::SubmitAndResetStates()
{
    PS4::RenderTargetData* prtdata = reinterpret_cast<PS4::RenderTargetData*>(RenderTargetStack.Back().pRenderTarget->GetRenderTargetData());

    // Submit what has been queued up to this point, and wait for the render target to be written to.
    GnmxCtx->submit();

    // Reset the context, and reinitialize.
    GnmxCtx->reset();
    GnmxCtx->initializeDefaultHardwareState();

    // Set the render target again.
    GnmxCtx->setRenderTarget(0, prtdata->pRenderSurface);
    GnmxCtx->setDepthRenderTarget(prtdata->pDSSurface);

    // Set depth/stencil states to the correct ones, based on the current mask state.
    DepthStencilMode currentMode = CurrentDepthStencilState;
    unsigned currentRef = CurrentStencilRef;
    CurrentDepthStencilState = DepthStencil_Invalid;
    CurrentStencilRef = (unsigned)-1;
    applyDepthStencilMode(currentMode, currentRef);

    // We always use 16-bit indices, and trilists.
    GnmxCtx->setIndexSize(sce::Gnm::IndexSize::kIndexSize16);
    GnmxCtx->setPrimitiveType(sce::Gnm::PrimitiveType::kPrimitiveTypeTriList);
    GnmxCtx->setIndexOffset(0);

    // Uncache previous batch type (in case instancing was used externally).
    PrevBatchType = PrimitiveBatch::DP_None;

    // Reset any cached shaders.
    SManager.BeginScene();
    ShaderData.BeginScene();

    // Update viewport/blend and raster modes.
    updateViewport();
    applyBlendModeImpl(CurrentBlendState.Mode, CurrentBlendState.SourceAc, CurrentBlendState.ForceAc);
    applyRasterModeImpl(CurrentSceneRasterMode);
}

// ***** Rendering
bool HAL::BeginFrame()
{
    if (!BaseHAL::BeginFrame())
        return false;

    // If the GnmxContext is allocated internally to Gfx, call its reset functions now.
    if (isGnmxCtxAllocatedInternally())
    {
        GnmxCtx->reset();
        GnmxCtx->initializeDefaultHardwareState();
    }
    return true;
}

void HAL::EndFrame()
{
    BaseHAL::EndFrame();
    
    // Must call the MemoryManager's EndFrame, so it can process pending memory frees.
    pMemoryManager->EndFrame();
}

bool HAL::BeginScene()
{
    if ( !ShaderHAL::BeginScene() )
        return false;

    // Render event for anything that is done immediately within BeginScene (but not the entire scene).
    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), "HAL::BeginScene-SetState");

    // We always use 16-bit indices, and trilists.
    GnmxCtx->setIndexSize(sce::Gnm::IndexSize::kIndexSize16);
    GnmxCtx->setPrimitiveType(sce::Gnm::PrimitiveType::kPrimitiveTypeTriList);
    GnmxCtx->setIndexOffset(0);

    // Uncache previous batch type (in case instancing was used externally).
    PrevBatchType = PrimitiveBatch::DP_None;
    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);

    if ( !Render::HAL::EndScene())
        return false;
    SManager.EndScene();

    return true;
}

void HAL::SetGfxContext(sceGnmxContextType* context)
{
    SF_DEBUG_ASSERT((HALState & HS_InScene) == 0, "Cannot switch GfxContext within a scene.");
    if (HALState & HS_InScene)
        return;

    // Setting context to 'NULL' is valid.
    if (!context)
    {
        if (GnmxAllocatedContext)
        {
            GnmxCtx = GnmxAllocatedContext;
        }
        else
        {
            // If no context is provided, and there isn't an allocated one, allocate a new one (but give a warning).
            SF_DEBUG_WARNING(1, "Calling HAL::SetGfxContext, with context = NULL, and no current GfxContext. Allocating internal Gnmx::GfxContext, with default parameteters");
            HALInitParams params(0);
            allocateGnmxContext(params);
        }
    }
    else
    {
        GnmxCtx = context; 
    }


    RSync.SetContext(GnmxCtx, *pMemoryManager); 
    pTextureManager->SetGfxContext(context);
}

// This function is a copy of what happens internally in GfxContext::submit, but without the actual submit.
unsigned HAL::AppendCommandBuffersToList(unsigned currentIndex, unsigned listTotalSize, 
                                         void** dcbGpuAddrs, uint32_t* dcbSizes, 
                                         void** ccbGpuAddrs, uint32_t* ccbSizes)
{
    return AppendCommandBuffersToList(*GnmxCtx, currentIndex, listTotalSize, dcbGpuAddrs, dcbSizes, ccbGpuAddrs, ccbSizes);
}

unsigned HAL::AppendCommandBuffersToList(sceGnmxContextType& ctx,
                                         unsigned currentIndex, unsigned listTotalSize, 
                                         void** dcbGpuAddrs, uint32_t* dcbSizes, 
                                         void** ccbGpuAddrs, uint32_t* ccbSizes)
{
    using namespace sce::Gnmx;

#ifndef SF_PS4_USE_LCUE
    // Check that the input lists are of the appropriate size.
    unsigned commandBuffers = ctx.m_submissionCount+1;

    if (currentIndex + commandBuffers > listTotalSize)
    {
        SF_DEBUG_ASSERT2(0, "Command buffers not appended to list - reported size is not large enough (inputSize=%d, requiredSize=%d)", listTotalSize, 
            currentIndex + commandBuffers);
        return 0;
    }

    // Submit each previously stored range
    for(uint32_t iSub=currentIndex; iSub<ctx.m_submissionCount+currentIndex; ++iSub)
    {
        dcbSizes[iSub]    = ctx.m_submissionRanges[iSub].m_dcbSizeInDwords*sizeof(uint32_t);
        dcbGpuAddrs[iSub] = ctx.m_dcb.m_beginptr + ctx.m_submissionRanges[iSub].m_dcbStartDwordOffset;
        ccbSizes[iSub]    = ctx.m_submissionRanges[iSub].m_ccbSizeInDwords*sizeof(uint32_t);
        ccbGpuAddrs[iSub] = (ccbSizes[iSub] > 0) ? ctx.m_ccb.m_beginptr + ctx.m_submissionRanges[iSub].m_ccbStartDwordOffset : 0;
    }
    // Submit anything left over after the final stored range
    dcbSizes[currentIndex + ctx.m_submissionCount]    = static_cast<uint32_t>(ctx.m_dcb.m_cmdptr - ctx.m_currentDcbSubmissionStart)*4;
    dcbGpuAddrs[currentIndex + ctx.m_submissionCount] = (void*)ctx.m_currentDcbSubmissionStart;
    ccbSizes[currentIndex + ctx.m_submissionCount]    = static_cast<uint32_t>(ctx.m_ccb.m_cmdptr - ctx.m_currentCcbSubmissionStart)*4;
    ccbGpuAddrs[currentIndex + ctx.m_submissionCount] = (ccbSizes[ctx.m_submissionCount] > 0) ? (void*)ctx.m_currentCcbSubmissionStart : 0;

    return currentIndex + commandBuffers;
#else
    // Only ever one submit buffer.
    dcbGpuAddrs[currentIndex] = ctx.m_dcb.m_beginptr;
    dcbSizes[currentIndex] = (ctx.m_dcb.m_cmdptr - ctx.m_dcb.m_beginptr) * sizeof(uint32_t);

    // Set these to zero/NULL, they should not be submitted when using the LCUE anyhow.
    ccbSizes[currentIndex] = 0;
    ccbGpuAddrs[currentIndex] = 0;
    return currentIndex + 1;
#endif
}

unsigned HAL::GetCommandBufferCount() const
{
#ifndef SF_PS4_USE_LCUE
    return GnmxCtx->m_submissionCount+1;
#else
    return 1;
#endif
}

// Updates D3D HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
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

    GnmxCtx->setupScreenViewport(vpRect.x1, vpRect.y1, vpRect.x2, vpRect.y2, 0.5f, 0.5f);
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------

bool HAL::checkDepthStencilBufferCaps()
{
    RenderTarget* prt = GetDefaultRenderTarget();
    SF_DEBUG_ASSERT(prt, "NULL Default RenderTarget, call SetRenderTarget before rendering.");
    PS4::RenderTargetData* phd = reinterpret_cast<PS4::RenderTargetData*>(prt->GetRenderTargetData());

    StencilAvailable     = phd->pDSSurface != NULL && phd->pDSSurface->getStencilFormat() != sce::Gnm::kStencilInvalid;
    MultiBitStencil      = StencilAvailable; // if it's not invalid, must be stencil8
    DepthBufferAvailable = phd->pDSSurface != NULL && phd->pDSSurface->getZFormat() != sce::Gnm::kZFormatInvalid;
    StencilChecked       = true;

    SF_DEBUG_WARNONCE(!StencilAvailable && !DepthBufferAvailable, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
    return StencilAvailable || DepthBufferAvailable;
}

struct OrbisVertexDeclBuilder
{
    static const unsigned   MaxElements = VertexShaderDesc::MaxVertexAttributes;
    sce::Gnm::Buffer        Buffers[MaxElements];
    unsigned                Count;
    void*                   BaseVertexAddress;
    unsigned                VertexCount;
    unsigned                VertexStride;

    OrbisVertexDeclBuilder(void* vertexAddress, unsigned vertexCount, unsigned stride) : 
    Count(0), BaseVertexAddress(vertexAddress), VertexCount(vertexCount), VertexStride(stride) { }

    void    Add(int, unsigned attr, int componentCount, int offset)
    {
        sce::Gnm::DataFormat format = getOrbisBufferFormat(attr, componentCount);
        Buffers[Count].initAsVertexBuffer(reinterpret_cast<UByte*>(BaseVertexAddress) + offset, format, VertexStride, VertexCount);
        SF_DEBUG_ASSERT(Buffers[Count].isBuffer(), "Vertex Buffer failed isBuffer test.");
        Count++;
    }

    sce::Gnm::DataFormat getOrbisBufferFormat(unsigned attr, unsigned componentCount)
    {
        sce::Gnm::DataFormat format = sce::Gnm::kDataFormatInvalid;
        switch(attr & VET_CompType_Mask)
        {
        case VET_U8N:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR8Unorm; break;
            case 2: format = sce::Gnm::kDataFormatR8G8Unorm; break;
            case 4: format = sce::Gnm::kDataFormatR8G8B8A8Unorm; break;
            default: case 3: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;
        case VET_U8:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR8Sint; break;
            case 2: format = sce::Gnm::kDataFormatR8G8Sint; break;
            case 4: format = sce::Gnm::kDataFormatR8G8B8A8Sint; break;
            default: case 3: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;
        case VET_S16:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR16Sint; break;
            case 2: format = sce::Gnm::kDataFormatR16G16Sint; break;
            case 4: format = sce::Gnm::kDataFormatR16G16B16A16Sint; break;
            default: case 3: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;
        case VET_U16:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR16Uint; break;
            case 2: format = sce::Gnm::kDataFormatR16G16Uint; break;                
            case 4: format = sce::Gnm::kDataFormatR16G16B16A16Uint; break;
            default: case 3: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;
        case VET_U32:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR32Uint; break;
            case 2: format = sce::Gnm::kDataFormatR32G32Uint; break;
            case 3: format = sce::Gnm::kDataFormatR32G32B32Uint; break;
            case 4: format = sce::Gnm::kDataFormatR32G32B32A32Uint; break;
            default: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;
        case VET_F32:
            switch(componentCount)
            {
            case 1: format = sce::Gnm::kDataFormatR32Float; break;
            case 2: format = sce::Gnm::kDataFormatR32G32Float; break;
            case 3: format = sce::Gnm::kDataFormatR32G32B32Float; break;
            case 4: format = sce::Gnm::kDataFormatR32G32B32A32Float; break;
            default: SF_DEBUG_ASSERT(0, "Not supported.");
            }
            break;

            // These types are ignored if present, because the indicate an instance index that will be provided by S_INSTANCE_ID.
        case VET_I8:
        case VET_I16:
            break;

        default:
            SF_DEBUG_ASSERT1(0, "Invalid attribute data format: %d\n", attr & VET_CompType_Mask);
            break;
        }
        return format;
    }

    void    Finish(int)
    {
    }
};

void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    sce::Gnm::DepthStencilControl dsc;
    sce::Gnm::StencilOpControl    soc;
    bool setDsc = false; // set to true, if something in the DSC is actually modified.
    bool setSoc = false; // set to true, if something in the SOC is actually modified.

    dsc.init();
    soc.init();

    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyDepthStencil), __FUNCTION__);
    static sce::Gnm::CompareFunc DepthStencilCompareFunctions[DepthStencilFunction_Count] =
    {
        sce::Gnm::kCompareFuncNever,           // Ignore
        sce::Gnm::kCompareFuncNever,           // Never
        sce::Gnm::kCompareFuncLess,            // Less
        sce::Gnm::kCompareFuncEqual,           // Equal
        sce::Gnm::kCompareFuncLessEqual,       // LessEqual
        sce::Gnm::kCompareFuncGreater,         // Greater
        sce::Gnm::kCompareFuncNotEqual,        // NotEqual
        sce::Gnm::kCompareFuncGreaterEqual,    // GreaterEqual
        sce::Gnm::kCompareFuncAlways,          // Always
    };
    static sce::Gnm::StencilOp StencilOps[StencilOp_Count] =
    {
        sce::Gnm::kStencilOpKeep,        // Ignore
        sce::Gnm::kStencilOpKeep,        // Keep
        sce::Gnm::kStencilOpReplaceTest, // Replace
        sce::Gnm::kStencilOpAddWrap,     // Increment        
    };

    const HALDepthStencilDescriptor& oldState = DepthStencilModeTable[CurrentDepthStencilState];
    const HALDepthStencilDescriptor& newState = DepthStencilModeTable[mode];

    // Apply the modes now.
    if (oldState.ColorWriteEnable != newState.ColorWriteEnable)
        GnmxCtx->setRenderTargetMask(newState.ColorWriteEnable ? 0xF : 0x0);

    // NOTE: settings are recorded in dsc, but only actually set if a change is detected. Any change
    // requires an entire set of parameters, which must be correct for the current mode, even if certain
    // parameters have not changed.
    dsc.setStencilEnable(newState.StencilEnable != 0);

    if (oldState.StencilEnable != newState.StencilEnable)
    {
        setDsc = true;
    }

    // Only need to set stencil pass/fail ops if stenciling is actually enabled.
    if (newState.StencilEnable)
    {
        if (stencilRef != CurrentStencilRef)
        {
            sce::Gnm::StencilControl sctrl = {stencilRef, 0xFF, 0xFF, 1}; // Op value 1 used only for increment.
            GnmxCtx->setStencil(sctrl);
            CurrentStencilRef = stencilRef;
        }

        dsc.setStencilFunction(DepthStencilCompareFunctions[newState.StencilFunction]);
        if (oldState.StencilFunction != newState.StencilFunction &&
            newState.StencilFunction != HAL::DepthStencilFunction_Ignore)
        {            
            setDsc = true;
        }

        soc.setStencilOps(StencilOps[newState.StencilFailOp], StencilOps[newState.StencilPassOp], StencilOps[newState.StencilZFailOp]);
        if ((oldState.StencilFailOp != newState.StencilFailOp &&
            newState.StencilFailOp != HAL::StencilOp_Ignore) || 
            (oldState.StencilPassOp != newState.StencilPassOp &&
            newState.StencilPassOp!= HAL::StencilOp_Ignore) || 
            (oldState.StencilZFailOp != newState.StencilZFailOp &&
            newState.StencilZFailOp != HAL::StencilOp_Ignore))
        {
            setSoc = true;
        }
    }

    // If the value of depth test/write change, we may have to change the value of ZEnable.
    dsc.setDepthControl(newState.DepthWriteEnable ? sce::Gnm::kDepthControlZWriteDisable : sce::Gnm::kDepthControlZWriteEnable,
        DepthStencilCompareFunctions[newState.DepthFunction]);
    if (newState.DepthTestEnable &&
        ((oldState.DepthWriteEnable != newState.DepthWriteEnable) ||
        (oldState.DepthFunction != newState.DepthFunction)))
    {
        setDsc = true;
    }

    dsc.setDepthEnable(newState.DepthTestEnable != 0);
    if (oldState.DepthTestEnable != newState.DepthTestEnable)
    {
        setDsc = true;
    }

    if (setDsc)
        GnmxCtx->setDepthStencilControl(dsc);

    if (setSoc)
        GnmxCtx->setStencilOpControl(soc);

    CurrentDepthStencilState = mode;
}

//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc )
{    
    if (!GnmxCtx)
        return;

    static const sce::Gnm::BlendFunc BlendOps[BlendOp_Count] = 
    {
        sce::Gnm::BlendFunc::kBlendFuncAdd,             // BlendOp_ADD
        sce::Gnm::BlendFunc::kBlendFuncMax,             // BlendOp_MAX
        sce::Gnm::BlendFunc::kBlendFuncMin,             // BlendOp_MIN
        sce::Gnm::BlendFunc::kBlendFuncReverseSubtract, // BlendOp_REVSUBTRACT
    };

    static const sce::Gnm::BlendMultiplier BlendFactors[BlendFactor_Count] = 
    {
        sce::Gnm::BlendMultiplier::kBlendMultiplierZero,            // BlendFactor_ZERO
        sce::Gnm::BlendMultiplier::kBlendMultiplierOne,             // BlendFactor_ONE
        sce::Gnm::BlendMultiplier::kBlendMultiplierSrcAlpha,        // BlendFactor_SRCALPHA
        sce::Gnm::BlendMultiplier::kBlendMultiplierOneMinusSrcAlpha,    // BlendFactor_INVSRCALPHA
        sce::Gnm::BlendMultiplier::kBlendMultiplierDestColor,       // BlendFactor_DESTCOLOR
        sce::Gnm::BlendMultiplier::kBlendMultiplierOneMinusDestColor,   // BlendFactor_INVDESTCOLOR
    };

    sce::Gnm::BlendMultiplier sourceOp = BlendFactors[BlendModeTable[mode].SourceColor];
    if ( sourceAc && sourceOp == sce::Gnm::BlendMultiplier::kBlendMultiplierSrcAlpha )
        sourceOp = sce::Gnm::BlendMultiplier::kBlendMultiplierOne;

    sce::Gnm::BlendControl bc;
    bc.init();    
    bc.setBlendEnable(true);
    bc.setColorEquation(sourceOp, BlendOps[BlendModeTable[mode].Operator], BlendFactors[BlendModeTable[mode].DestColor]);

    if (VP.Flags & Viewport::View_AlphaComposite || forceAc)
    {
        bc.setSeparateAlphaEnable(true);
        bc.setAlphaEquation(BlendFactors[BlendModeTable[mode].SourceAlpha], BlendOps[BlendModeTable[mode].AlphaOperator],
            BlendFactors[BlendModeTable[mode].DestAlpha]);
    }
    GnmxCtx->setBlendControl(0, bc);
}

void HAL::applyRasterModeImpl(RasterModeType mode)
{
    sce::Gnm::PrimitiveSetup primSetupReg;
    primSetupReg.init();
    primSetupReg.setCullFace(sce::Gnm::kPrimitiveSetupCullFaceNone);
    primSetupReg.setFrontFace(sce::Gnm::kPrimitiveSetupFrontFaceCcw);

    // NOTE: it looks as though point and wireframe modes don't actually work.
    sce::Gnm::PrimitiveSetupPolygonMode polygonMode;
    switch(mode)
    {
    default:
    case RasterMode_Solid:      polygonMode = sce::Gnm::kPrimitiveSetupPolygonModeFill; break;
    case RasterMode_Wireframe:  polygonMode = sce::Gnm::kPrimitiveSetupPolygonModeLine; break;
    case RasterMode_Point:      polygonMode = sce::Gnm::kPrimitiveSetupPolygonModePoint; break;
    }
    primSetupReg.setPolygonMode(polygonMode, polygonMode);
    GnmxCtx->setPrimitiveSetup(primSetupReg);
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    PS4::Texture* pt = (PS4::Texture*)texture;

    // Cannot render to textures which have multiple HW representations.
    if ( !pt || pt->TextureCount != 1 )
        return 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);   
    if ( !prt )
        return 0;

    Ptr<DepthStencilBuffer>      pdsb =0;
    sce::Gnm::DepthRenderTarget* pdsTarget = 0;
    sce::Gnm::RenderTarget*      prtTarget = &pt->pTextures[0].Surface;

    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize(), false); // false == not temporary
        if ( pdsb )
        {
            DepthStencilSurface* surf = (PS4::DepthStencilSurface*)pdsb->GetSurface();
            pdsTarget = &surf->DepthTarget;
        }
    }
    RenderTargetData::UpdateData(prt, prtTarget, pdsb, pdsTarget);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilBuffer))
        return prt;

    Ptr<DepthStencilBuffer>      pdsb = 0;
    sce::Gnm::DepthRenderTarget* pdsTarget = 0;
    PS4::Texture*                pt = (PS4::Texture*)prt->GetTexture();
    sce::Gnm::RenderTarget*      prtTarget = &pt->pTextures[0].Surface;

    SF_DEBUG_ASSERT(pt->TextureCount == 1, "Cannot render to textures which have multiple HW representations"); 

    if ( needsStencil )
    {   
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (PS4::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsTarget = &surf->DepthTarget;
        }
    }

    RenderTargetData::UpdateData(prt, prtTarget, pdsb, pdsTarget);
    return prt;
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
    RenderTargetData* prtdata = (PS4::RenderTargetData*)ptarget->GetRenderTargetData();

    if ( setState )
    {
        GnmxCtx->setRenderTarget(0, prtdata->pRenderSurface);
        GnmxCtx->setDepthRenderTarget(prtdata->pDSSurface);
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
    Matrices->SetUserMatrix(Matrix2F::Identity);

    // Setup the render target/depth stencil on the device.
    if ( !prt )
    {
        SF_DEBUG_WARNING(1, "HAL::PushRenderTarget - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }

    RenderTargetData* phd = (PS4::RenderTargetData*)prt->GetRenderTargetData();
    GnmxCtx->setRenderTarget(0, phd->pRenderSurface);
    GnmxCtx->setDepthRenderTarget(phd->pDSSurface);
    StencilChecked = false;
    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        GnmxCtx->setupScreenViewport(0, 0, prt->GetBufferSize().Width, prt->GetBufferSize().Height, 0.5f, 0.5f);
        MatrixState previousMatrixState;
        previousMatrixState.CopyFrom(GetMatrices());
        GetMatrices()->UserView = Matrix2F::Identity;
        clearSolidRectangle(Rect<int>(-2, -2, 2, 2), clearColor, false);
        GetMatrices()->CopyFrom(&previousMatrixState);
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
     PS4::RenderTargetData* plasthd = ( PS4::RenderTargetData*)prt->GetRenderTargetData();
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        if ( plasthd->pDSSurface )
            plasthd->pDSSurface = 0;
        plasthd->pDepthStencilBuffer = 0;
    }
    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();

    sce::Gnm::DepthRenderTarget* pds = 0;
    RenderTargetData* phd = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        phd = (PS4::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
        pds = (sce::Gnm::DepthRenderTarget*)phd->pDSSurface;
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    GnmxCtx->waitForGraphicsWrites(plasthd->pRenderSurface->getBaseAddress256ByteBlocks(), plasthd->pRenderSurface->getSliceSizeInBytes()>>8, sce::Gnm::kWaitTargetSlotCb0,
        sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache,
        sce::Gnm::kStallCommandBufferParserDisable);

    // Restore the old render target on the device.
    if ((flags & PRT_NoSet) == 0)
    {
        GnmxCtx->setRenderTarget(0, phd->pRenderSurface);
        GnmxCtx->setDepthRenderTarget(pds);
        ++AccumulatedStats.RTChanges;


        // Reset the viewport to the last render target on the stack.
        HALState |= HS_ViewValid;
        updateViewport();
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
        rtSize = ImageSize(1920, 1080); // correct screen size?
    }

    // No way to query the current rendertarget on Orbis. Must call SetRenderTarget externally.
    return pRenderBufferManager->Initialize(pTextureManager, Image_B8G8R8A8, rtSize);
}

void HAL::setBatchUnitSquareVertexStream()
{
    // NOTE: LCUE will assert if you set a stream that isn't used. So, we check the current
    // shader whether it is a batched one or not, and if not, we do not set the batch stream
    // for the VB.
    bool batchStream = ShaderData.GetCurrentShaders().pVDesc->Flags & Shader_Batch;
    GnmxCtx->setVertexBuffers(sce::Gnm::kShaderStageVs, 0, batchStream ? 2 : 1, Cache.pMaskEraseBatchVertexBuffer);
}


void HAL::setBatchUnitSquareVertexStreamPerDraw()
{
#ifdef SF_PS4_USE_LCUE
    setBatchUnitSquareVertexStream();
#endif
}

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmeshBase)
{
    PS4::MeshCacheItem* pmesh = reinterpret_cast<PS4::MeshCacheItem*>(pmeshBase);

    void* bufferBaseAddress = pmesh->pBuffer->pData;
    GnmxCtx->setIndexBuffer(bufferBaseAddress);

    OrbisVertexDeclBuilder builder(reinterpret_cast<UByte*>(bufferBaseAddress) + pmesh->VertexOffset, pmesh->VertexCount, pbatch->pFormat->Size);
    BuildVertexArray(pbatch->pFormat, builder);
    GnmxCtx->setVertexBuffers(sce::Gnm::ShaderStage::kShaderStageVs, 0, builder.Count, builder.Buffers );

    return pmesh->IndexOffset / sizeof(IndexType);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{
    PS4::MeshCacheItem* pmesh = reinterpret_cast<PS4::MeshCacheItem*>(pmeshBase);
#ifndef SF_PS4_USE_LCUE
    setVertexArrayImpl(fr, formatIndex, pmesh);
#endif
    return pmesh->IndexOffset / sizeof(IndexType);
}

void HAL::setVertexArrayPerDraw(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh)
{
#ifdef SF_PS4_USE_LCUE
    setVertexArray(pbatch, pmesh);
#else
    SF_UNUSED2(pbatch, pmesh);
#endif
}

void HAL::setVertexArrayPerDraw(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh)
{
#ifdef SF_PS4_USE_LCUE
    setVertexArrayImpl(fr, formatIndex, pmesh);
#else
    SF_UNUSED3(fr, formatIndex, pmesh);
#endif
}

// NOTE: This function actually applies the vertex stream. When using the LCUE, the ComplexMesh version of setVertexArray
// cannot actually set the stream, because the shader will not be applied yet. As such, it would try to apply to the previous
// shader, which might have the incorrect SRO table, and assert.
void HAL::setVertexArrayImpl(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{
    PS4::MeshCacheItem* pmesh = reinterpret_cast<PS4::MeshCacheItem*>(pmeshBase);

    void* bufferBaseAddress = pmesh->pBuffer->pData;
    GnmxCtx->setIndexBuffer(bufferBaseAddress);

    OrbisVertexDeclBuilder builder(reinterpret_cast<UByte*>(bufferBaseAddress) + pmesh->VertexOffset + fr.VertexByteOffset, 
        fr.VertexCount, fr.pFormats[formatIndex]->Size);
    BuildVertexArray(fr.pFormats[formatIndex], builder);

    GnmxCtx->setVertexBuffers(sce::Gnm::ShaderStage::kShaderStageVs, 0, builder.Count, builder.Buffers );
}



void HAL::setLinearStreamSource(PrimitiveBatch::BatchType type)
{
    if (PrevBatchType >= PrimitiveBatch::DP_Instanced)
    {
        GnmxCtx->setNumInstances(1);
        GnmxCtx->setInstanceStepRate(1, 1);
    }
    PrevBatchType = type;
}

void HAL::setInstancedStreamSource(UPInt instanceCount, UPInt indicesPerMesh)
{
    SF_UNUSED(indicesPerMesh);
    SF_DEBUG_ASSERT(SManager.HasInstancingSupport(), "Cannot call setInstancedStreamSource if the ShaderManager does not support instancing.");
    if (PrevBatchType != PrimitiveBatch::DP_Instanced)
    {
        GnmxCtx->setNumInstances(instanceCount);
    }
    PrevBatchType = PrimitiveBatch::DP_Instanced;
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    GnmxCtx->drawIndexAuto(indexCount);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset )
{
    GnmxCtx->drawIndexOffset(indexOffset, indexCount);

    SF_UNUSED2(meshCount, vertexOffset);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexOffset )
{
    GnmxCtx->drawIndexOffset(indexOffset, indexCount);

    SF_UNUSED2(meshCount, vertexOffset);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawScreenQuad()
{
    setBatchUnitSquareVertexStream();
    drawPrimitive(6,1);
}

void HAL::drawUncachedFilter(const FilterStackEntry& e)
{
    // Invalid primitive or rendertarget.
    if ( !e.pPrimitive || !e.pRenderTarget )
        return;

    // Override drawUncachedFilter, as it must wait for the writing of the unfiltered content to complete before
    // it is used in the base implementation.
    PS4::RenderTargetData* phd = reinterpret_cast<PS4::RenderTargetData*>(e.pRenderTarget->GetRenderTargetData());
    GnmxCtx->waitForGraphicsWrites(phd->pRenderSurface->getBaseAddress256ByteBlocks(), phd->pRenderSurface->getSliceSizeInBytes()>>8, sce::Gnm::kWaitTargetSlotCb0,
        sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2, sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache,
        sce::Gnm::kStallCommandBufferParserDisable);

    BaseHAL::drawUncachedFilter(e);
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
#if !defined(SF_BUILD_SHIPPING)
    static PS4::RenderEvent Events[Event_Count];
    static bool EventsInitialized = false;
    if (!EventsInitialized)
    {
        for ( unsigned event = 0; event < Event_Count; ++event)
            Events[event].pHAL = this;
    }
    return Events[eventType];
#else
    // Shipping builds just return a default event, which does nothing.
    return Render::HAL::GetEvent(eventType);
#endif
}

}}} // Scaleform::Render::Orbis

