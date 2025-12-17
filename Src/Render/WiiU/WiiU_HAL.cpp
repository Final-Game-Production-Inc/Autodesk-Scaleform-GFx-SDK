/**************************************************************************

Filename    :   WiiU_HAL.cpp
Content     :   WiiU Renderer HAL Prototype implementation.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <cafe/os.h>
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/WiiU/WiiU_HAL.h"
#include "Render/Render_BufferGeneric.h"

#if defined(SF_USE_GPU_PROFILER)
#include <nn/save.h>
#endif

namespace Scaleform { namespace Render { namespace WiiU {


// ***** RenderHAL_WiiU

HAL::HAL(ThreadCommandQueue* commandQueue) :   
    Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    RSync(),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults, &RSync),
    PrevBatchType(PrimitiveBatch::DP_None),
    TemporaryTextureFence(0)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

bool HAL::InitHAL(const WiiU::HALInitParams& params)
{
#if defined(SF_USE_GPU_PROFILER)
    GX2DebugCaptureInit(NULL);
    SAVEInit();
    CaptureIndex = 0;
#endif

    pMemManager = params.pMemoryManager;
    
    if ( !initHAL(params))
        return false;

    pContext = params.pContext;
    
    pTextureManager = params.GetTextureManager();
    if (!pTextureManager)
    {
        pTextureManager = 
            *SF_HEAP_AUTO_NEW(this) TextureManager(params.pMemoryManager, &RSync);
    }

    // Allocate our matrix state
    Matrices = *SF_HEAP_AUTO_NEW(this) MatrixState(this);

    pRenderBufferManager = params.pRenderBufferManager;
    if (!pRenderBufferManager)
    {
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric();
        if ( !pRenderBufferManager )
        {
            ShutdownHAL();
            return false;
        }

        if (!pRenderBufferManager->Initialize(pTextureManager, Image_R8G8B8A8, ImageSize(1920,1080)))
        {
            ShutdownHAL();
            return false;
        }
    }

    if (!Cache.Initialize(pMemManager))
        return false;

    ShaderData.Initialize(pMemManager);

    HALState|= HS_ModeSet;    
    notifyHandlers(HAL_Initialize);
    return true;
}

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    destroyTemporaryTextures();
    destroyRenderBuffers();
    pRenderBufferManager.Clear();
    
    pTextureManager.Clear();
    Cache.Reset();

    return true;
}

// ***** Rendering

bool HAL::BeginFrame()
{
    if ( !Render::HAL::BeginFrame() )
        return false;
    
    destroyTemporaryTextures();
    
    return true;
}

// Set states not changed in our rendering, or that are reset after changes
bool HAL::BeginScene()
{
    if ( !Render::HAL::BeginScene())
        return false;

    GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_REGISTER);
    GX2SetPrimitiveRestartIndex(0xffff);

    GX2SetTargetChannelMasks(GX2_CHANNEL_MASK_RGBA, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, 
                             GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE, GX2_CHANNEL_MASK_NONE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);
    GX2SetAlphaTest(GX2_DISABLE, GX2_COMPARE_ALWAYS, 0);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CW, GX2_DISABLE, GX2_DISABLE);
    GX2SetPolygonOffset(0, 0, 0, 0, 0);
    GX2SetLineWidth(8);

    return true;
}

#if defined(SF_USE_GPU_PROFILER)
void HAL::endDisplay()
{
    Render::HAL::endDisplay();
    GX2DebugCaptureEnd(GX2_DEBUG_CAPTURE_DEFAULT);
}
#endif

void HAL::beginDisplay(BeginDisplayData* data)
{
#if defined(SF_USE_GPU_PROFILER)
    char buf[32];
    SFsprintf(buf, 32, "gpu%d.4mp", CaptureIndex);
    CaptureIndex++;
    GX2DebugCaptureStart(buf, GX2_DEBUG_CAPTURE_DEFAULT);
#endif

    CurStencilState = StencilState();
    CurStencilState.Set();

    Render::HAL::beginDisplay(data);
}

// Updates HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    Viewport vp;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(VP.Flags &~ Viewport::View_IsRenderTexture, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        if ( HALState & HS_InRenderTarget )
        {
            GX2SetViewport(VP.Left, VP.Top, VP.Width, VP.Height, 0, 1);
            GX2SetScissor(VP.Left, VP.Top, VP.Width, VP.Height);
        }
        else
        {
            vp.Left     = ViewRect.x1;
            vp.Top      = ViewRect.y1;
            vp.Width    = ViewRect.Width();
            vp.Height   = ViewRect.Height();
            GX2SetViewport(vp.Left, vp.Top, vp.Width, vp.Height, 0,1);
            if (VP.Flags & Viewport::View_UseScissorRect)
                GX2SetScissor(VP.ScissorLeft, VP.ScissorTop, VP.ScissorWidth, VP.ScissorHeight);
            else
                GX2SetScissor(vp.Left, vp.Top, vp.Width, vp.Height);
        }
    }
    else
    {
        GX2SetViewport(0,0,0,0, 0,0);
    }
}

void*   MemoryManager::Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena)
{
    void* pdata = SF_MEMALIGN(size, align, StatRender_Mem);
    GX2NotifyMemAlloc(pdata, size, align);
    return pdata;
}
void    MemoryManager::Free(void* pmem, UPInt size)
{
    SF_FREE(pmem);
    GX2NotifyMemFree(pmem);
}


void   HAL::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single,
                            const VertexFormat** batch, const VertexFormat** instanced, unsigned)
{
    SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced, MVF_AlignVertexStride);

    if (!(*single)->pSysFormat)
        const_cast<VertexFormat*>(*single)->pSysFormat = *(Render::SystemVertexFormat*)SF_NEW SysVertexFormat(pMemManager, *single);
    if (*batch && !(*batch)->pSysFormat)
        const_cast<VertexFormat*>(*batch)->pSysFormat = *(Render::SystemVertexFormat*)SF_NEW SysVertexFormat(pMemManager, *batch);
}

// Draws a range of pre-cached and preprocessed primitives
void        HAL::DrawProcessedPrimitive(Primitive* pprimitive,
                                        PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    SF_AMP_SCOPE_RENDER_TIMER("HAL::DrawProcessedPrimitive", Amp_Profile_Level_High);
    if (!checkState(HS_InDisplay, __FUNCTION__) ||
        !pprimitive->GetMeshCount() )
        return;

    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);

        StencilState().Set();
        DrawProcessedPrimitive(pprimitive, pstart, pend );

        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);
        CurStencilState.Set();
        drawingMask = false;
    }

    SF_ASSERT(pend != 0);
    
    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    ShaderData.BeginPrimitive();

    unsigned bidx = 0;
    while (pbatch != pend)
    {        
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        WiiU::MeshCacheItem* pmesh = reinterpret_cast<WiiU::MeshCacheItem*>(pbatch->GetCacheItem());
        unsigned             meshIndex = pbatch->GetMeshIndex();
        unsigned             batchMeshCount = pbatch->GetMeshCount();

        if (pmesh)
        {
            Profiler.SetBatch((UPInt)pprimitive, bidx);

            UInt32 fillFlags = FillFlags;
            if ( batchMeshCount > 0 )
                fillFlags |= pprimitive->Meshes[0].M.Has3D() ? FF_3DProjection : 0;

            const ShaderManager::Shader& pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, pbatch->pFormat, batchMeshCount, Matrices,
                                          &pprimitive->Meshes[meshIndex], &ShaderData);

            if ((HALState & HS_ViewValid))
            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                          (pbatch->Type != PrimitiveBatch::DP_Virtual) &&
                          (pbatch->Type != PrimitiveBatch::DP_Instanced));

                UByte* pbufferStart = (UByte*)pmesh->pBuffer->pData;
                GX2SetAttribBuffer(0, pmesh->VertexCount*pbatch->pFormat->Size, pbatch->pFormat->Size, pbufferStart+pmesh->VertexOffset);
                drawIndexedPrimitive(pmesh->IndexCount, pmesh->MeshCount, pbufferStart+pmesh->IndexOffset);
            }

            pmesh->GPUFence = Cache.GetRenderSync()->InsertFence();
            pmesh->MoveToCacheListFront(MCL_ThisFrame);
        }

        pbatch = pbatch->GetNext();
        bidx++;
    }
}


void HAL::DrawProcessedComplexMeshes(ComplexMesh* complexMesh,
                                     const StrideArray<HMatrix>& matrices)
{    
    typedef ComplexMesh::FillRecord   FillRecord;
    typedef PrimitiveBatch::BatchType BatchType;
    
    WiiU::MeshCacheItem* pmesh = reinterpret_cast<WiiU::MeshCacheItem*>(complexMesh->GetCacheItem());
    if (!checkState(HS_InDisplay, __FUNCTION__) || !pmesh)
        return;
    
    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);
        StencilState().Set();

        DrawProcessedComplexMeshes(complexMesh, matrices);

        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);
        CurStencilState.Set();
        drawingMask = false;
    }

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;
    UByte*      pbufferStart = (UByte*)pmesh->pBuffer->pData;

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();

    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        Profiler.SetBatch((UPInt)complexMesh, fillIndex);
        ShaderData.BeginPrimitive();

        unsigned fillFlags = FillFlags;
        unsigned startIndex = 0;
        if ( instanceCount > 0 )
        {
            const HMatrix& hm = matrices[0];
            fillFlags |= hm.Has3D() ? FF_3DProjection : 0;

            for (unsigned i = 0; i < instanceCount; i++)
            {
                const HMatrix& hm = matrices[startIndex + i];
                if (!(Profiler.GetCxform(hm.GetCxform()) == Cxform::Identity))
                    fillFlags |= FF_Cxform;
            }
        }

        // Apply fill.
        PrimitiveFillType fillType = Profiler.GetFillType(fr.pFill->GetType());
        const ShaderManager::Shader& pso = SManager.SetFill(fillType, fillFlags, batchType, fr.pFormats[0], &ShaderData);

        GX2SetAttribBuffer(0, fr.VertexCount*fr.pFormats[0]->Size, fr.pFormats[0]->Size,
                           pbufferStart + pmesh->VertexOffset + fr.VertexByteOffset);

        UByte textureCount = fr.pFill->GetTextureCount();
        bool solid = (fillType == PrimFill_None || fillType == PrimFill_Mask || fillType == PrimFill_SolidColor);

        for (unsigned i = 0; i < instanceCount; i++)
        {            
            const HMatrix& hm = matrices[startIndex + i];

            ShaderData.SetMatrix(pso, Uniform::SU_mvp, complexMesh->GetVertexMatrix(), hm, Matrices);
            if (solid)
                ShaderData.SetColor(pso, Uniform::SU_cxmul, Profiler.GetColor(fr.pFill->GetSolidColor()));
            else if (fillFlags & FF_Cxform)
                ShaderData.SetCxform(pso, Profiler.GetCxform(hm.GetCxform()));

            for (unsigned tm = 0, stage = 0; tm < textureCount; tm++)
            {
                ShaderData.SetMatrix(pso, Uniform::SU_texgen, textureMatrices[fr.FillMatrixIndex[tm]], tm);
                Texture* ptex = (Texture*)fr.pFill->GetTexture(tm);
                ShaderData.SetTexture(pso, Uniform::SU_tex, ptex, fr.pFill->GetFillMode(tm), stage);
                stage += ptex->GetPlaneCount();
            }

            ShaderData.Finish(0);

            drawIndexedPrimitive(fr.IndexCount, 1, pbufferStart + pmesh->IndexOffset + sizeof(IndexType) * fr.IndexOffset);
        }

    } // for (fill record)
  
    pmesh->GPUFence = Cache.GetRenderSync()->InsertFence();
    pmesh->MoveToCacheListFront(MCL_ThisFrame);
}


//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color)
{
    color = Profiler.GetClearColor(color);

    float colorf[4];
    color.GetRGBAFloat(colorf, colorf+1, colorf+2, colorf+3);
    Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
               0.0f, (float)r.Height(), (float)r.y1);

    Matrix2F  mvp(m, Matrices->UserView);

    unsigned fillflags = 0;
    const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags, PrimitiveBatch::DP_Single, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
    ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
    ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
    ShaderData.Finish(1);

    GX2SetAttribBuffer(0, sizeof(VertexXY16iInstance) * 6,
                       MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);
    drawPrimitive(6, 1);
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------

// Mask support is implemented as a stack, enabling for a number of optimizations:
//
// 1. Large "Clipped" marks are clipped to a custom viewport, allowing to save on
//    fill-rate when rendering both the mask and its content. The mask area threshold
//    that triggers this behavior is determined externally.
//      - Clipped masks can be nested, but not batched. When erased, clipped masks
//        clear the clipped intersection area.
// 2. Small masks can be Batched, having multiple mask areas with multiple mask
//    content items inside.
//      - Small masks can contain clipped masks either regular or clipped masks.
// 3. Mask area dimensions are provided as HMatrix, which maps a unit rectangle {0,0,1,1}
//    to a mask bounding rectangle. This rectangle can be rotated (non-axis aligned),
//    allowing for more efficient fill.
// 4. PopMask stack optimization is implemented that does not erase nested masks; 
//    Stencil Reference value is changed instead. Erase of a mask only becomes
//    necessary if another PushMask_BeginSubmit is called, in which case previous
//    mask bounding rectangles are erased. This setup avoids often unnecessary erase 
//    operations when drawing content following a nested mask.
//      - To implement this MaskStack keeps a previous "stale" MaskPrimitive
//        located above the MaskStackTop.


void HAL::PushMask_BeginSubmit(MaskPrimitive* prim)
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    Profiler.SetDrawMode(1);

    GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        CurStencilState = StencilState(GX2_ENABLE, MaskStackTop, GX2_COMPARE_LEQUAL, GX2_STENCIL_REPLACE);
        CurStencilState.Set();

        MaskPrimitive* erasePrim = MaskStack[MaskStackTop].pPrimitive;
        drawMaskClearRectangles(erasePrim->GetMaskAreaMatrices(), erasePrim->GetMaskCount());
    }

    MaskStack.Resize(MaskStackTop+1);
    MaskStackEntry &e = MaskStack[MaskStackTop];
    e.pPrimitive       = prim;
    e.OldViewportValid = viewportValid;
    e.OldViewRect      = ViewRect; // TBD: Must assign
    MaskStackTop++;

    HALState |= HS_DrawingMask;

    if (prim->IsClipped() && viewportValid)
    {
        Rect<int> boundClip;

        // Apply new viewport clipping.
        if (!Matrices->OrientationSet)
        {
            const Matrix2F& m = prim->GetMaskAreaMatrix(0).GetMatrix2D();

            // Clipped matrices are always in View coordinate space, to allow
            // matrix to be use for erase operation above. This means that we don't
            // have to do an EncloseTransform.
            SF_ASSERT((m.Shx() == 0.0f) && (m.Shy() == 0.0f));
            boundClip = Rect<int>(VP.Left + (int)m.Tx(), VP.Top + (int)m.Ty(),
                                  VP.Left + (int)(m.Tx() + m.Sx()), VP.Top + (int)(m.Ty() + m.Sy()));
        }
        else
        {
            Matrix2F m = prim->GetMaskAreaMatrix(0).GetMatrix2D();
            m.Append(Matrices->Orient2D);

            RectF rect = m.EncloseTransform(RectF(0,0,1,1));
            boundClip = Rect<int>(VP.Left + (int)rect.x1, VP.Top + (int)rect.y1,
                                  VP.Left + (int)rect.x2, VP.Top + (int)rect.y2);
        }

        if (!ViewRect.IntersectRect(&ViewRect, boundClip))
        {
            ViewRect.Clear();
            HALState &= ~HS_ViewValid;
            viewportValid = false;
        }
        updateViewport();
        
        // Clear full viewport area, which has been resized to our smaller bounds.
        if ((MaskStackTop == 1) && viewportValid)
        {
            CurStencilState = StencilState(GX2_ENABLE, 0, GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE);
            CurStencilState.Set();

            clearSolidRectangle(ViewRect - Point<int>(VP.Left, VP.Top), Color(0,64,64,128));
        }
    }
    else
        if ((MaskStackTop == 1) && viewportValid)
    {
        CurStencilState = StencilState(GX2_ENABLE, 0, GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE);
        CurStencilState.Set();

        drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
    }

    CurStencilState = StencilState(GX2_ENABLE, MaskStackTop-1, GX2_COMPARE_EQUAL, GX2_STENCIL_INCR);
    CurStencilState.Set();
    ++AccumulatedStats.Masks;
}


void HAL::EndMaskSubmit()
{
    Profiler.SetDrawMode(0);

    if (!checkState(HS_InDisplay|HS_DrawingMask, __FUNCTION__))
        return;

    HALState &= ~HS_DrawingMask;    
    SF_ASSERT(MaskStackTop);

    CurStencilState = StencilState(GX2_ENABLE, MaskStackTop, GX2_COMPARE_LEQUAL);
    CurStencilState.Set();

    GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);
}


void HAL::PopMask()
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    SF_ASSERT(MaskStackTop);
    MaskStackTop--;

    if (MaskStack[MaskStackTop].pPrimitive->IsClipped())
    {
        // Restore viewport
        ViewRect      = MaskStack[MaskStackTop].OldViewRect;

        if (MaskStack[MaskStackTop].OldViewportValid)
            HALState |= HS_ViewValid;
        else
            HALState &= ~HS_ViewValid;
        updateViewport();
    }

    if (MaskStackTop == 0)
    {
        CurStencilState = StencilState();
        CurStencilState.Set();
    }
    else
    {
        CurStencilState = StencilState(GX2_ENABLE, MaskStackTop, GX2_COMPARE_LEQUAL);
        CurStencilState.Set();
    }
}


//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

// Structure describing color combines applied for a given blend mode.
struct BlendModeDesc
{
    GX2BlendCombine  op;
    GX2BlendFunction src, dest;
};

struct BlendModeDescAlpha
{
    GX2BlendCombine  op;
    GX2BlendFunction srcc, srca, destc, desta;
};

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc)
{    
    static const GX2BlendCombine BlendOps[BlendOp_Count] = 
    {
        GX2_BLEND_COMBINE_ADD,              // BlendOp_ADD
        GX2_BLEND_COMBINE_MAX,              // BlendOp_MAX
        GX2_BLEND_COMBINE_MIN,              // BlendOp_MIN
        GX2_BLEND_COMBINE_DST_MINUS_SRC,    // BlendOp_REVSUBTRACT
    };

    static const GX2BlendFunction BlendFactors[BlendFactor_Count] = 
    {
        GX2_BLEND_ZERO,                 // BlendFactor_ZERO
        GX2_BLEND_ONE,                  // BlendFactor_ONE
        GX2_BLEND_SRC_ALPHA,            // BlendFactor_SRCALPHA
        GX2_BLEND_ONE_MINUS_SRC_ALPHA,  // BlendFactor_INVSRCALPHA
        GX2_BLEND_DST_COLOR,            // BlendFactor_DESTCOLOR
        GX2_BLEND_ONE_MINUS_DST_COLOR,  // BlendFactor_INVDESTCOLOR
    };

    GX2BlendCombine  op    = BlendOps[BlendModeTable[mode].Operator];
    GX2BlendFunction srcc  = BlendFactors[BlendModeTable[mode].SourceColor];
    GX2BlendFunction destc = BlendFactors[BlendModeTable[mode].DestColor];
    GX2BlendFunction srca  = BlendFactors[BlendModeTable[mode].SourceAlpha];
    GX2BlendFunction desta = BlendFactors[BlendModeTable[mode].DestAlpha];

    if ( sourceAc && srcc == GX2_BLEND_SRC_ALPHA )
        srcc = GX2_BLEND_ONE;

    if (VP.Flags & Viewport::View_AlphaComposite || forceAc)
        GX2SetBlendControl(GX2_RENDER_TARGET_0, srcc, destc, op, GX2_ENABLE, srca, desta, op);
    else
        GX2SetBlendControl(GX2_RENDER_TARGET_0, srcc, destc, op, GX2_DISABLE, srcc, destc, op);
}

RenderTarget* HAL::CreateRenderTarget(const GX2ColorBuffer& color, const GX2DepthBuffer& depth)
{
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(ImageSize(color.surface.width, color.surface.height),
        RBuffer_User, Image_R8G8B8A8, 0);
    if ( !prt )
        return 0;
    if ( prt->GetRenderTargetData() != 0 )
        return prt;

    RenderTargetData::UpdateData(prt, color, depth);
    return prt;
}


RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    WiiU::Texture* pt = (WiiU::Texture*)texture;

    if ( !pt || pt->TextureCount != 1 )
        return 0;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;
    Ptr<DepthStencilBuffer> pdsb;

    GX2ColorBuffer color;
    memset(&color, 0, sizeof(color));
    color.surface = pt->pTextures[0].Tex.surface;
    color.viewMip = 0;
    color.viewFirstSlice = 0;
    color.viewNumSlices = 1;
    GX2InitColorBufferRegs(&color);
    GX2Invalidate(GX2_INVALIDATE_CPU, color.surface.imagePtr, color.surface.imageSize);

    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize());
    }

    RenderTargetData::UpdateData(prt, color, pdsb);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;
    Texture* pt = (Texture*)prt->GetTexture();
    if ( !pt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilBuffer != 0 ))
        return prt;
    Ptr<DepthStencilBuffer> pdsb;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 

    GX2ColorBuffer color;
    memset(&color, 0, sizeof(color));
    color.surface = pt->pTextures[0].Tex.surface;
    color.viewMip = 0;
    color.viewFirstSlice = 0;
    color.viewNumSlices = 1;
    GX2InitColorBufferRegs(&color);
    GX2Invalidate(GX2_INVALIDATE_CPU, color.surface.imagePtr, color.surface.imageSize);

    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
    }

    RenderTargetData::UpdateData(prt, color, pdsb);
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
    RenderTargetData* prtdata = (WiiU::RenderTargetData*)ptarget->GetRenderTargetData();
    if ( setState )
    {
        GX2SetColorBuffer(&prtdata->ColorBuffer, GX2_RENDER_TARGET_0);
        if (prtdata->pDepthStencilBuffer)
            GX2SetDepthBuffer(&((DepthStencilSurface*)prtdata->pDepthStencilBuffer->GetSurface())->DepthBuffer);
        else if (prtdata->DepthBuffer.surface.width)
            GX2SetDepthBuffer(&prtdata->DepthBuffer);
    }

    entry.pRenderTarget = ptarget;

    // Replace the stack entry at the bottom, or if the stack is empty, add one.
    if ( RenderTargetStack.GetSize() > 0 )
        RenderTargetStack[0] = entry;
    else
        RenderTargetStack.PushBack(entry);
    return true;
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags)
{
    // flush framebuffer cache on old render target
    {
        RenderTargetEntry& old = RenderTargetStack.Back();
        RenderTargetData* pOldRtdata = (WiiU::RenderTargetData*)old.pRenderTarget->GetRenderTargetData();

        GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, pOldRtdata->ColorBuffer.surface.imagePtr, pOldRtdata->ColorBuffer.surface.imageSize);
    }

    // Setup the render target/depth stencil on the device.
    HALState |= HS_InRenderTarget;
    RenderTargetEntry entry;
    entry.pRenderTarget = prt;
    entry.OldViewport = VP;
    entry.OldViewRect = ViewRect;
    entry.OldMatrixState.CopyFrom(Matrices);
    Matrices->SetUserMatrix(Matrix2F::Identity);

    // Setup the render target/depth stencil.
    if ( !prt )
    {
        SF_DEBUG_WARNING(1, "HAL::PushRenderTarget - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }
    RenderTargetData* prtdata = (WiiU::RenderTargetData*)prt->GetRenderTargetData();

    const ImageSize& bs = prt->GetBufferSize();

    GX2SetColorBuffer(&prtdata->ColorBuffer, GX2_RENDER_TARGET_0);
    if (prtdata->pDepthStencilBuffer)
        GX2SetDepthBuffer(&((DepthStencilSurface*)prtdata->pDepthStencilBuffer->GetSurface())->DepthBuffer);
    else if (prtdata->DepthBuffer.surface.width)
        GX2SetDepthBuffer(&prtdata->DepthBuffer);

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        GX2ClearColor(&prtdata->ColorBuffer, 0,0,0,0);
    }
    GX2SetContextState(pContext);
    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, prtdata->ColorBuffer.surface.imagePtr, prtdata->ColorBuffer.surface.imageSize);
    ++AccumulatedStats.RTChanges;

    // Setup viewport.
    Rect<int> viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
    VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height(), Viewport::View_IsRenderTexture);
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

void HAL::PopRenderTarget(unsigned)
{
    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;
    WiiU::RenderTargetData* plasthd = (WiiU::RenderTargetData*)prt->GetRenderTargetData();
    prt->SetInUse(false);
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        plasthd->pDepthStencilBuffer = 0;
    }

    GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, plasthd->ColorBuffer.surface.imagePtr, plasthd->ColorBuffer.surface.imageSize);
    GX2Invalidate(GX2_INVALIDATE_TEXTURE,      plasthd->ColorBuffer.surface.imagePtr, plasthd->ColorBuffer.surface.imageSize);

    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();
    RenderTargetData* prtdata = 0;

    const ImageSize& bs = prt->GetBufferSize();

    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        prtdata = (WiiU::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();

        // Restore the old render target on the device.
        GX2SetColorBuffer(&prtdata->ColorBuffer, GX2_RENDER_TARGET_0);
        if (prtdata->pDepthStencilBuffer)
            GX2SetDepthBuffer(&((DepthStencilSurface*)prtdata->pDepthStencilBuffer->GetSurface())->DepthBuffer);
        else if (prtdata->DepthBuffer.surface.width)
            GX2SetDepthBuffer(&prtdata->DepthBuffer);

        ++AccumulatedStats.RTChanges;
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Reset the viewport to the last render target on the stack.
    HALState |= HS_ViewValid;
    updateViewport();
}


void HAL::PushFilters(FilterPrimitive* prim)
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    FilterStackEntry e = {prim, 0};

    // Queue the profiler off of whether masks should be draw or not.
    if ( !Profiler.ShouldDrawMask() )
    {
        Profiler.SetDrawMode(2);

        unsigned fillflags = 0;
        float colorf[4];
        Profiler.GetColor(0xFFFFFFFF).GetRGBAFloat(colorf);
        const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags, 
                                                            PrimitiveBatch::DP_Single, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
        Matrix2F mvp(prim->GetFilterAreaMatrix().GetMatrix2D(), Matrices->UserView);
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        ShaderData.Finish(1);

        GX2SetAttribBuffer(0, sizeof(VertexXY16iInstance) * 6,
                           MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);
        drawPrimitive(6,1);
        FilterStack.PushBack(e);
        return;
    }

    if ( (HALState & HS_CachedFilter) )
    {
        FilterStack.PushBack(e);
        return;
    }

    HALState |= HS_DrawingFilter;

    if ( prim->GetCacheState() ==  FilterPrimitive::Cache_Uncached )
    {
        // Draw the filter from scratch.
        const Matrix2F& m = e.pPrimitive->GetFilterAreaMatrix().GetMatrix2D();
        e.pRenderTarget = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), prim->GetMaskPresent());
        RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());
        PushRenderTarget(frameRect, e.pRenderTarget);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, true);

        // If this primitive has masking, then clear the entire area to the current mask level, because 
        // the depth stencil target may be different, and thus does not contain the previously written values.
        if ( prim->GetMaskPresent())
        {
            GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);

            StencilState(GX2_ENABLE, MaskStackTop, GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE).Set();
            clearSolidRectangle(Matrices->ViewRect, Color(64,0,0,128));

            GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);
        }
    }
    else
    {
        // Drawing a cached filter, ignore all draw calls until the corresponding PopFilters.
        // Keep track of the level at which we need to draw the cached filter, by adding entries to the stack.
        HALState |= HS_CachedFilter;
        CachedFilterIndex = FilterStack.GetSize();
        GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_Filters);
    }

    // Disable masking from previous target, if this filter primitive doesn't have any masking.
    if ( MaskStackTop != 0 && !prim->GetMaskPresent() && prim->GetCacheState() != FilterPrimitive::Cache_Target )
    {
        GX2SetDepthStencilControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS, GX2_DISABLE, GX2_DISABLE,
                                  GX2_COMPARE_ALWAYS, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP,
                                  GX2_COMPARE_ALWAYS, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP, GX2_STENCIL_KEEP);
    }
    else if (prim->GetCacheState() == FilterPrimitive::Cache_Uncached)
        CurStencilState.Set();

    FilterStack.PushBack(e);
}

void HAL::drawUncachedFilter(const FilterStackEntry& e)
{
    const FilterSet* filters = e.pPrimitive->GetFilters();
    unsigned filterCount = filters->GetFilterCount();
    const Filter* filter = 0;
    unsigned pass = 0, passes = 0;

    // Invalid primitive or rendertarget.
    if ( !e.pPrimitive || !e.pRenderTarget )
        return;

    // Bind the render target.
    SF_ASSERT(RenderTargetStack.Back().pRenderTarget == e.pRenderTarget);
    const int MaxTemporaryTextures = 3;
    Ptr<RenderTarget> temporaryTextures[MaxTemporaryTextures];
    memset(temporaryTextures, 0, sizeof temporaryTextures);

    // Fill our the source target.
    ImageSize size = e.pRenderTarget->GetSize();
    temporaryTextures[0] = e.pRenderTarget;

    GX2SetAttribBuffer(0, sizeof(VertexXY16iInstance) * 6,
                       MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);

    StencilState().Set();
    // Overlay mode isn't actually supported, it contains the blend mode for filter sub-targets.
    applyBlendMode(Blend_Overlay, true, false);

    // Render filter(s).
    unsigned shaders[ShaderManager::MaximumFilterPasses];
    for ( unsigned i = 0; i < filterCount; ++i )
    {
        filter = filters->GetFilter(i);
        passes = SManager.GetFilterPasses(filter, FillFlags, shaders);

        // All shadows (except those hiding the object) need the original texture.
        bool requireSource = false;
        if ( filter->GetFilterType() >= Filter_Shadow &&
             filter->GetFilterType() <= Filter_Blur_End )
        {
            temporaryTextures[Target_Original] = temporaryTextures[Target_Source];
            requireSource = true;
        }

        // Now actually render the filter.
        for (pass = 0; pass < passes; ++pass)
        {
            // Render the final pass directly to the target surface.
            if (pass == passes-1 && i == filterCount-1)
                break;

            // Create a destination texture if required.
            if ( !temporaryTextures[1] )
            {
                temporaryTextures[1] = *CreateTempRenderTarget(size, false);
            }

            RenderTarget*    prt = temporaryTextures[1];
            RenderTargetData*         prtdata = (RenderTargetData*)prt->GetRenderTargetData();

            GX2SetColorBuffer(&prtdata->ColorBuffer, GX2_RENDER_TARGET_0);
            GX2ClearColor(&prtdata->ColorBuffer, 0,0,0,0);
            GX2SetContextState(pContext);
            GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, prtdata->ColorBuffer.surface.imagePtr, prtdata->ColorBuffer.surface.imageSize);
            ++AccumulatedStats.RTChanges;

            // Scale to the size of the destination.
            const Rect<int>& viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
            const ImageSize& bs = prt->GetBufferSize();
            VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());    
            ViewRect = Rect<int>(viewRect.x1, viewRect.y1, viewRect.x2, viewRect.y2);
            HALState |= HS_ViewValid;
            updateViewport();

            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);                          

            SManager.SetFilterFill(mvp, Cxform::Identity, filter, temporaryTextures, shaders, pass, passes,
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
            drawPrimitive(6,1);

            GX2Invalidate(GX2_INVALIDATE_COLOR_BUFFER, prtdata->ColorBuffer.surface.imagePtr, prtdata->ColorBuffer.surface.imageSize);
            GX2Invalidate(GX2_INVALIDATE_TEXTURE,      prtdata->ColorBuffer.surface.imagePtr, prtdata->ColorBuffer.surface.imageSize);

            // If we require the original source, create a new target for the source.
            if ( requireSource && pass == 0)
                temporaryTextures[0] = *CreateTempRenderTarget(size, false);

            // Setup for the next pass.
            Alg::Swap(temporaryTextures[0], temporaryTextures[1]);
        }
    }

    // If there were no passes, assume we were doing a cacheAsBitmap.
    bool cacheAsBitmap = passes == 0;
    SF_DEBUG_ASSERT(!cacheAsBitmap || filterCount == 1, "Expected exactly one cacheAsBitmap filter.");

    // Cache the 2nd last step so it might be available as a cached filter next time.
    if (temporaryTextures[Target_Source] && (Profiler.IsFilterCachingEnabled() || cacheAsBitmap))
	{
        RenderTarget* cacheResults[2] = { temporaryTextures[0], temporaryTextures[2] };
        e.pPrimitive->SetCacheResults(cacheAsBitmap ? FilterPrimitive::Cache_Target : FilterPrimitive::Cache_PreTarget, cacheResults, cacheAsBitmap ? 1 : 2);
        ((WiiU::RenderTargetData*)cacheResults[0]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
        if ( cacheResults[1] )
            ((WiiU::RenderTargetData*)cacheResults[1]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
	}
    else
    {
        // This is required, or else disabling filter caching may produce incorrect results.
        e.pPrimitive->SetCacheResults(FilterPrimitive::Cache_Uncached, 0, 0);
    }

    // Pop the temporary target, begin rendering to the previous surface.
    PopRenderTarget();

    // Re-[en/dis]able masking from previous target, if available.
    if ( MaskStackTop != 0 )
    {
        CurStencilState.Set();
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);
    }

    // Now actually draw the filtered sub-scene to the target below.
    if (passes != 0)
    {
        // 'Real' filter.
        const Matrix2F& mvp = Matrices->UserView * e.pPrimitive->GetFilterAreaMatrix().GetMatrix2D();
        const Cxform&   cx  = e.pPrimitive->GetFilterAreaMatrix().GetCxform();
        SManager.SetFilterFill(mvp, cx, filter, temporaryTextures, shaders, pass, passes,
            MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
        drawPrimitive(6,1);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget) != 0);
    }
    else
    {
        // CacheAsBitmap
        drawCachedFilter(e.pPrimitive);
    }

    // Re-[en/dis]able masking from previous target, if available.
    if ( HALState & HS_DrawingMask )
        GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);

    // Cleanup.
    for ( unsigned i = 0; i < MaxTemporaryTextures; ++i )
    {
        if (temporaryTextures[i])
            temporaryTextures[i]->SetInUse(false);
    }
    AccumulatedStats.Filters += filters->GetFilterCount();
}

void HAL::drawCachedFilter(FilterPrimitive* primitive)
{
    GX2SetAttribBuffer(0, sizeof(VertexXY16iAlpha) * 6,
                       MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);

    const int MaxTemporaryTextures = 3;
    switch(primitive->GetCacheState())
    {
        // We have one-step from final target. Render it to a final target now.
        case FilterPrimitive::Cache_PreTarget:
        {
            const FilterSet* filters = primitive->GetFilters();
            UPInt filterIndex = filters->GetFilterCount()-1;
            const Filter* filter = filters->GetFilter(filterIndex);
            unsigned shaders[ShaderManager::MaximumFilterPasses];
            unsigned passes = SManager.GetFilterPasses(filter, FillFlags, shaders);

            // Fill out the temporary textures from the cached results.
            Ptr<RenderTarget> temporaryTextures[MaxTemporaryTextures];
            memset(temporaryTextures, 0, sizeof temporaryTextures);
            RenderTarget* results[2];
            primitive->GetCacheResults(results, 2);
            temporaryTextures[0] = results[0];
            ImageSize size = temporaryTextures[0]->GetSize();
            temporaryTextures[1] = *CreateTempRenderTarget(size, false);
            temporaryTextures[2] = results[1];
            PushRenderTarget(RectF((float)size.Width,(float)size.Height), temporaryTextures[1]);

            // Render to the target.
            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
            SManager.SetFilterFill(mvp, Cxform::Identity, filter, temporaryTextures, shaders, passes-1, passes,
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
            drawPrimitive(6,1);
            PopRenderTarget();

            // Re-[en/dis]able masking from previous target, if available.
            if ( MaskStackTop != 0 )
            {
                CurStencilState.Set();
            }

            // Re-[en/dis]able masking from previous target, if available.
            if ( HALState & HS_DrawingMask )
                GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_DISABLE);
            else
                GX2SetColorControl(GX2_LOGIC_OP_COPY, 1, GX2_DISABLE, GX2_ENABLE);

            // Set this as the final cache result, and then render it.
            RenderTarget* prt = temporaryTextures[1];
            primitive->SetCacheResults(FilterPrimitive::Cache_Target, &prt, 1);
            ((WiiU::RenderTargetData*)prt->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(primitive);
            drawCachedFilter(primitive);

            // Cleanup.
            for ( unsigned i = 0; i < MaxTemporaryTextures; ++i )
            {
                if (temporaryTextures[i])
                    temporaryTextures[i]->SetInUse(false);
            }
            break;
        }

        // We have a final filtered texture. Just apply it to a screen quad.
        case FilterPrimitive::Cache_Target:
        {
            unsigned fillFlags = (FillFlags|FF_Cxform|FF_AlphaWrite);
            const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Single,
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);

            RenderTarget* results;
            primitive->GetCacheResults(&results, 1);
            Texture* ptexture = (WiiU::Texture*)results->GetTexture();
            const Matrix2F& mvp = Matrices->UserView * primitive->GetFilterAreaMatrix().GetMatrix2D();
            const Rect<int>& srect = results->GetRect();
            Matrix2F texgen;
            texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
            texgen.AppendScaling((float)srect.Width() / ptexture->GetSize().Width, (float)srect.Height() / ptexture->GetSize().Height);

            const Cxform & cx = primitive->GetFilterAreaMatrix().GetCxform();
            ShaderData.SetCxform(pso, cx);
            ShaderData.SetUniform(pso, Uniform::SU_mvp, &mvp.M[0][0], 8 );
            ShaderData.SetUniform(pso, Uniform::SU_texgen, &texgen.M[0][0], 8 );
            ShaderData.SetTexture(pso, Uniform::SU_tex, ptexture, ImageFillMode(Wrap_Clamp, Sample_Linear));
            ShaderData.Finish(1);

            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
            drawPrimitive(6,1);
            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget)!=0);

            // Cleanup.
            results->SetInUse(false);
			if ( !Profiler.IsFilterCachingEnabled() )
				primitive->SetCacheResults(FilterPrimitive::Cache_Uncached, 0, 0);
            break;
        }

        // Should have been one of the other two caching types.
        default: SF_ASSERT(0); break;
    }
}

void HAL::setBatchUnitSquareVertexStream()
{
    GX2SetAttribBuffer(0, sizeof(VertexXY16iInstance) * 6 * MeshCache::MaxEraseBatchCount,
        MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    GX2Draw(GX2_PRIMITIVE_TRIANGLES, indexCount);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UByte* indexPtr)
{
    GX2DrawIndexed(GX2_PRIMITIVE_TRIANGLES, indexCount, GX2_INDEX_FORMAT_U16, indexPtr);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, unsigned channelMask, const UInt32* values)
{
    // Create a temporary texture with the palette map. There may be a more efficient way to do this; however, using
    // uniforms seems unworkable, due to shader constant slot constraints.
    ImageData data;
    Ptr<Render::Texture> ptex = *pTextureManager->CreateTexture(Image_R8G8B8A8, 1, ImageSize(256, 4), ImageUse_Map_Mask, 0);
    if ( !ptex->Map(&data, 0, 1) )
        return;
    for ( int channel = 0; channel < 4; ++channel )
    {
        UInt32* dataPtr = (UInt32*)data.GetScanline(channel);
        if ( channelMask & (1<<channel))
        {
            for (int entry = 0; entry < 256; entry++)
            {
                // Incoming data is in ARGB
                // Outgoing data is in RGBA
                // The line below shifts the incoming RGB triplet up 8 bits, then shifts the incoming A value into the vacant least significant 8 bits.
                *dataPtr++ = (values[channel*256 + entry] << 8) | (values[channel*256 + entry] >> 24);
            }
        }
        else
        {
            // Channel was not provided, just do a straight mapping.
            for (unsigned i = 0; i < 256; ++i)
                *dataPtr++ = (i << ((3 - channel) * 8));
        }
    }
    
    DCStoreRange((void*)data.GetScanline(0), 256*4*4);

    if (!ptex->Unmap())
        return;

    // First pass overwrites everything.
    applyBlendMode(Blend_OverwriteAll, true, true);
    SManager.SetDrawablePaletteMap(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), mvp,
        ptex, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData, 0 );

    // Store the texture, otherwise it would be deleted when it goes out of scope at the end of this function.
    // The GPU most likely hasn't had a chance to use it yet, so this would be bad.
    TemporaryTexture temptex;
    temptex.Tex = ptex;
    TemporaryTextureList.PushBack(temptex);
    TemporaryTextureFence = GetRenderSync()->InsertFence();

    drawScreenQuad();
}

void HAL::drawScreenQuad()
{
    // Set the vertices and Draw
    GX2SetAttribBuffer(0, sizeof(VertexXY16iInstance) * 6,
                       MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]->Size, Cache.pMaskEraseBatchVertexBuffer);
    GX2Draw(GX2_PRIMITIVE_TRIANGLES, 6);
}

void HAL::destroyTemporaryTextures()
{
    if (TemporaryTextureList.GetSize() >= MaximumTempTextures)
        TemporaryTextureFence->WaitFence(FenceType_Fragment);
    if (TemporaryTextureFence && !TemporaryTextureFence->IsPending(FenceType_Fragment))
        TemporaryTextureList.Clear();
}

}}} // Scaleform::Render::WiiU

