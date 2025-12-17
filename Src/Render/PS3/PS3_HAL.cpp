/**************************************************************************

Filename    :   PS3_HAL.cpp
Content     :   PS3 Renderer HAL Prototype implementation.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/PS3/PS3_HAL.h"
#include "Render/PS3/PS3_Events.h"
#include "Render/PS3/PS3_MemoryManager.h"
#include "Render/Render_BufferGeneric.h"

namespace Scaleform { namespace Render { namespace PS3 {



// ***** RenderHAL_PS3

HAL::HAL(ThreadCommandQueue* commandQueue)
	: Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    GcmCtx(0),
    VertexArrayUsed(0),
    RSync(),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::Console_Defaults, &RSync),
    pTextureManager(0),
    TemporaryTextureFence(0)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

void*   MemoryManager::Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena)
{
    switch (type)
    {
    default:
        SF_DEBUG_WARNING1(1, "PS3::MemoryManager, unknown memory type %d, using main", type);

    case Memory_PS3_MainVideo:
        return (void*)Main.Alloc(size, align);
    case Memory_PS3_RSX:
        return (void*)Local.Alloc(size, align);
    }
}

void    MemoryManager::Free(void* pmem, UPInt size)
{
    if (pmem >= MainStart && pmem < MainEnd)
        Main.Free(((UPInt)pmem), size);
    else
        Local.Free(((UPInt)pmem), size);
}


// *** RenderHAL_PS3 Implementation

bool HAL::InitHAL(const PS3::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    if (!params.pGcm)
        return false;

    // Memory manager required on PS3!
    SF_ASSERT(params.pMemoryManager);

    // Allocate/Initialize texture manager
    if (params.pTextureManager)
        pTextureManager = params.GetTextureManager();
    else
    {
        pTextureManager = 
            *SF_HEAP_AUTO_NEW(this) TextureManager(params.pGcm, params.pMemoryManager, &RSync);
        if (!pTextureManager)
        {
            ShutdownHAL();
            return false;
        }
    }

    // Allocate our matrix state
    Matrices = *SF_HEAP_AUTO_NEW(this) MatrixState(this);

    // Create RenderBufferManager if needed.
    if (params.pRenderBufferManager)
        pRenderBufferManager = params.pRenderBufferManager;
    else
    {
        // Create the default render target manager.
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric();
        if ( !pRenderBufferManager || !createDefaultRenderBuffer())
        {
            ShutdownHAL();
            return false;
        }
    }

    GcmCtx = params.pGcm;        
    MemoryManager* pmemManager = (PS3::MemoryManager*)params.pMemoryManager;

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
        if (VertexShaderDesc::Descs[i] && VertexShaderDesc::Descs[i]->pBinary)
            StaticVShaders[i].Init(VertexShaderDesc::Descs[i]);

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
        if (FragShaderDesc::Descs[i] && FragShaderDesc::Descs[i]->pBinary)
            StaticFShaders[i].Init(FragShaderDesc::Descs[i], pmemManager);

    SF_ASSERT(!RSync.GetContext());
    RSync.SetContext(GcmCtx, params.Label);

    if (!Cache.Initialize(GcmCtx, pmemManager))
    {
        ShutdownHAL();
        return false;
    }

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

    if ( pTextureManager )
    {
        for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
            if (VertexShaderDesc::Descs[i] && VertexShaderDesc::Descs[i]->pBinary)
                StaticVShaders[i].Shutdown();

        for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
            if (FragShaderDesc::Descs[i] && FragShaderDesc::Descs[i]->pBinary)
                StaticFShaders[i].Shutdown();
    }
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

    ShaderData.BeginFrame();
    return true;
}

// Set states not changed in our rendering, or that are reset after changes
bool HAL::BeginScene()
{
    if (!Render::HAL::BeginScene())
        return false;
    for (int i = 0; i < 16; i++)
    {
        cellGcmSetVertexDataArray(GcmCtx, i, 0,0,0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
    }
    VertexArrayUsed = 0;

    cellGcmSetCullFaceEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetZMinMaxControl(GcmCtx, CELL_GCM_TRUE, CELL_GCM_FALSE, CELL_GCM_FALSE);

    cellGcmSetDepthTestEnable(GcmCtx, CELL_GCM_FALSE);
    cellGcmSetAlphaTestEnable(GcmCtx, CELL_GCM_FALSE);

    cellGcmSetTransferLocation(GcmCtx, CELL_GCM_LOCATION_LOCAL);

    return true;
}

void HAL::beginDisplay(BeginDisplayData* data)
{
    cellGcmSetBlendEnable(GcmCtx, CELL_GCM_TRUE);
    Render::HAL::beginDisplay(data);
}

// Updates HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    Viewport vpRect;
    float scale[4], offset[4];

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;

        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(0, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        vpRect.Left     = ViewRect.x1;
        vpRect.Top      = ViewRect.y1;
        vpRect.Width    = ViewRect.Width();
        vpRect.Height   = ViewRect.Height();

        if ( !(HALState & HS_InRenderTarget) )
        {
            scale[0] = vpRect.Width * 0.5f;
            scale[1] = vpRect.Height * -0.5f;
            scale[2] = 0.5;
            scale[3] = 0;
            offset[0] = vpRect.Left + vpRect.Width * 0.5f;
            offset[1] = vpRect.Top + vpRect.Height * 0.5f;
            offset[2] = 0.5;
            offset[3] = 0;
            cellGcmSetViewport(GcmCtx, vpRect.Left, vpRect.Top, vpRect.Width, vpRect.Height, 0, 1, scale, offset);           
            cellGcmSetScissor(GcmCtx, vpRect.Left, vpRect.Top, vpRect.Width, vpRect.Height);
        }
        else
        {
            scale[0] = VP.Width * 0.5f;
            scale[1] = VP.Height * -0.5f;
            scale[2] = 0.5;
            scale[3] = 0;
            offset[0] = VP.Left + VP.Width * 0.5f;
            offset[1] = VP.Top + VP.Height * 0.5f;
            offset[2] = 0.5;
            offset[3] = 0;
            cellGcmSetViewport(GcmCtx, VP.Left, VP.Top, VP.Width, VP.Height, 0, 1, scale, offset);
            cellGcmSetScissor(GcmCtx, VP.Left, VP.Top, VP.Width, VP.Height);
        }
    }
    else
    {
        cellGcmSetViewport(GcmCtx, 0,0, 0,0, 0,1, scale, offset);
        cellGcmSetScissor(GcmCtx, 0,0, 0,0);
    }
}

void HAL::SetUserMatrix(const Matrix& userMatrix)
{
}

void   HAL::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
    const VertexFormat** single,
    const VertexFormat** batch, const VertexFormat** instanced, unsigned)
{
    SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced);
}

class VertexBuilder
{
public:
    int       vertexLoc, vertexBase;
    unsigned  Size;
    int       vused;
    HAL*      pHal;
    const VertexShader* pvs;

    VertexBuilder(HAL* phal, const VertexShader* ps, int vl, int vb, unsigned s)
        : vertexLoc(vl), vertexBase(vb), Size(s), vused(0), pHal(phal), pvs(ps) {}

    void Add(int i, int attr, int size, int offset)
    {
        int vet;
        switch (attr & VET_CompType_Mask)
        {
        case VET_U8:  vet = CELL_GCM_VERTEX_UB256; size = 4; break;
        case VET_U8N: vet = CELL_GCM_VERTEX_UB; break;
        case VET_S16: vet = CELL_GCM_VERTEX_S32K; break;
        case VET_F32: vet = CELL_GCM_VERTEX_F; break;
        default: SF_ASSERT(0); break;
        }

        if ( pvs->Attributes[i] != -1 )
        {
            cellGcmSetVertexDataArray(pHal->GcmCtx, pvs->Attributes[i], 0, Size,
                                      size, vet, vertexLoc, vertexBase + offset);

            vused |= (1 << pvs->Attributes[i]);
        }
    }

    void Finish(int)
    {
        for (int i = 0; i < 16; i++)
        {
            if ((pHal->VertexArrayUsed & (1 << i)) != 0 && (vused & (1 << i)) == 0)
                cellGcmSetVertexDataArray(pHal->GcmCtx, i, 0,0,0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
        }
        pHal->VertexArrayUsed = vused;
    }
};

void   HAL::setVertexArray (const VertexShader* pvs, const VertexFormat* pvf, int vertexLoc, int vertexBase)
{
    VertexBuilder vb (this, pvs, vertexLoc, vertexBase, pvf->Size);
    BuildVertexArray(pvf, vb);
}

// Draws a range of pre-cached and preprocessed primitives
void        HAL::DrawProcessedPrimitive(Primitive* pprimitive,
                                              PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    SF_AMP_SCOPE_RENDER_TIMER("HAL::DrawProcessedPrimitive", Amp_Profile_Level_High);
    SF_ASSERT(pend != 0);

    if (!pprimitive->GetMeshCount())
        return;

    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);
        DrawProcessedPrimitive(pprimitive, pstart, pend);
        cellGcmSetColorMask(GcmCtx, 0);
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_TRUE);
        drawingMask = false;
    }

    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    ShaderData.BeginPrimitive();

    unsigned bidx = 0;
    while (pbatch != pend)
    {
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        PS3::MeshCacheItem* pmesh = reinterpret_cast<PS3::MeshCacheItem*>(pbatch->GetCacheItem());
        unsigned            meshIndex = pbatch->GetMeshIndex();
        unsigned            batchMeshCount = pbatch->GetMeshCount();

        if (pmesh)
        {
            Profiler.SetBatch((UPInt)pprimitive, bidx);

            UInt32 fillFlags = FillFlags;
            if (pprimitive->GetMeshCount() && pprimitive->Meshes[0].M.Has3D())
                fillFlags |= FF_3DProjection;

            ShaderPair pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, pbatch->pFormat, batchMeshCount, Matrices,
                &pprimitive->Meshes[meshIndex], &ShaderData);

            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                          (pbatch->Type != PrimitiveBatch::DP_Virtual) &&
                          (pbatch->Type != PrimitiveBatch::DP_Instanced));

                UInt32 vertexOffset, indexOffset;
                cellGcmAddressToOffset((UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset, &vertexOffset);
                cellGcmAddressToOffset((UByte*)pmesh->pBuffer->pData + pmesh->IndexOffset, &indexOffset);
                setVertexArray(ShaderData.GetCurrentShaders().pVS, pbatch->pFormat, CELL_GCM_LOCATION_MAIN, vertexOffset);

                drawIndexedPrimitive(pmesh->IndexCount, pmesh->MeshCount, indexOffset);
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

    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);
        DrawProcessedComplexMeshes(complexMesh, matrices);
        cellGcmSetColorMask(GcmCtx, 0);
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_TRUE);
        drawingMask = false;
    }

    PS3::MeshCacheItem* pmesh = reinterpret_cast<PS3::MeshCacheItem*>(complexMesh->GetCacheItem());
    if (!pmesh)
        return;

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();
    UInt32 vertexOffset, indexOffset;
    SInt32 result = cellGcmAddressToOffset((UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset, &vertexOffset);
	SF_ASSERT( result == CELL_OK );
    result = cellGcmAddressToOffset((UByte*)pmesh->pBuffer->pData + pmesh->IndexOffset, &indexOffset);
	SF_ASSERT( result == CELL_OK );

    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        ShaderData.BeginPrimitive();

        UByte textureCount = fr.pFill->GetTextureCount();
        unsigned startIndex = 0;
        unsigned fillFlags = FillFlags;

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
        ShaderPair pso = SManager.SetFill(fillType, fillFlags, batchType, fr.pFormats[0], &ShaderData);

        setVertexArray(ShaderData.GetCurrentShaders().pVS, fr.pFormats[0], CELL_GCM_LOCATION_MAIN, vertexOffset + fr.VertexByteOffset);

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

            ShaderData.Finish(1);

            drawIndexedPrimitive(fr.IndexCount, 1, indexOffset + sizeof(IndexType) * fr.IndexOffset);
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

    if (color.GetAlpha() == 0xFF)
    {
        cellGcmSetScissor(GcmCtx, VP.Left + r.x1, VP.Top + r.y1, r.x2-r.x1, r.y2-r.y1);
        cellGcmSetClearColor(GcmCtx, color.Raw);
        cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A);

        cellGcmSetScissor(GcmCtx, 0,0,4096,4096);
    }
    else
    {
        float colorf[4];
        color.GetRGBAFloat(colorf, colorf+1, colorf+2, colorf+3);
        Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
                   0.0f, (float)r.Height(), (float)r.y1);

        Matrix2F  mvp(m, Matrices->UserView);

        unsigned fillFlags = 0;
        ShaderPair pso = SManager.SetFill(PrimFill_SolidColor, fillFlags, PrimitiveBatch::DP_Single, &VertexXY16iInstance::Format, &ShaderData);
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        ShaderData.Finish(1);

        UInt32 offset;
		SInt32 result = cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
		SF_ASSERT( result == CELL_OK );
        setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iInstance::Format, CELL_GCM_LOCATION_MAIN, offset);

        drawPrimitive(6, 1);
    }
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

    // disable framebuffer writes
    cellGcmSetColorMask(GcmCtx, 0);
    cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_TRUE);

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        cellGcmSetStencilFunc(GcmCtx, CELL_GCM_LEQUAL, MaskStackTop, 0xFF);
        cellGcmSetStencilOp(GcmCtx, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_REPLACE);

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
            cellGcmSetClearDepthStencil(GcmCtx, 0);
            cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_S);
        }
    }

    else if ((MaskStackTop == 1) && viewportValid)
    {
        // Unconditionally overwrite stencil rectangles with REF value of 0.
        cellGcmSetStencilFunc(GcmCtx, CELL_GCM_ALWAYS, 0, 0xFF);
        cellGcmSetStencilOp(GcmCtx, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_REPLACE);

        drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
    }

    cellGcmSetStencilFunc(GcmCtx, CELL_GCM_EQUAL, MaskStackTop-1, 0xFF);
    cellGcmSetStencilOp(GcmCtx, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_INCR);
    ++AccumulatedStats.Masks;
}


void HAL::EndMaskSubmit()
{
    Profiler.SetDrawMode(0);

    if (!checkState(HS_InDisplay|HS_DrawingMask, __FUNCTION__))
        return;

    HALState &= ~HS_DrawingMask;    
    SF_ASSERT(MaskStackTop);

    cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);

    // We draw only where the (MaskStackTop <= stencil), i.e. where the latest mask was drawn.
    // However, we don't change the stencil buffer
    cellGcmSetStencilFunc(GcmCtx, CELL_GCM_LEQUAL, MaskStackTop, 0xFF);
    cellGcmSetStencilOp(GcmCtx, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_KEEP);
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
        ViewRect = MaskStack[MaskStackTop].OldViewRect;

        if (MaskStack[MaskStackTop].OldViewportValid)
            HALState |= HS_ViewValid;
        else
            HALState &= ~HS_ViewValid;
        updateViewport();
    }

    // Disable mask or decrement stencil reference value.
    if (MaskStackTop == 0)
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);
    else
    {
        // Change ref value down, so that we can draw using previous mask.
        cellGcmSetStencilFunc(GcmCtx, CELL_GCM_LEQUAL, MaskStackTop, 0xFF);
        cellGcmSetStencilOp(GcmCtx, CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_KEEP);
    }
}

//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------
void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc)
{
    static const UInt32 BlendOps[BlendOp_Count] = 
    {
        CELL_GCM_FUNC_ADD,              // BlendOp_ADD
        CELL_GCM_MAX,                   // BlendOp_MAX
        CELL_GCM_MIN,                   // BlendOp_MIN
        CELL_GCM_FUNC_REVERSE_SUBTRACT, // BlendOp_REVSUBTRACT
    };

    static const UInt32 BlendFactors[BlendFactor_Count] = 
    {
        CELL_GCM_ZERO,                  // BlendFactor_ZERO
        CELL_GCM_ONE,                   // BlendFactor_ONE
        CELL_GCM_SRC_ALPHA,             // BlendFactor_SRCALPHA
        CELL_GCM_ONE_MINUS_SRC_ALPHA,   // BlendFactor_INVSRCALPHA
        CELL_GCM_DST_COLOR,             // BlendFactor_DESTCOLOR
        CELL_GCM_ONE_MINUS_DST_COLOR,   // BlendFactor_INVDESTCOLOR
    };

    if ((VP.Flags & Viewport::View_AlphaComposite) || forceAc )
    {
        if ( sourceAc && BlendModeTable[mode].SourceColor == BlendFactor_SRCALPHA)
        {
            cellGcmSetBlendFunc(GcmCtx, CELL_GCM_ONE, BlendFactors[BlendModeTable[mode].DestColor], 
                BlendFactors[BlendModeTable[mode].SourceAlpha], BlendFactors[BlendModeTable[mode].DestAlpha]);
        }
        else
        {
            cellGcmSetBlendFunc(GcmCtx, BlendFactors[BlendModeTable[mode].SourceColor], BlendFactors[BlendModeTable[mode].DestColor], 
                BlendFactors[BlendModeTable[mode].SourceAlpha], BlendFactors[BlendModeTable[mode].DestAlpha]);
        }
    }
    else
    {
        if ( sourceAc && BlendModeTable[mode].SourceColor == BlendFactor_SRCALPHA)
        {
            cellGcmSetBlendFunc(GcmCtx, CELL_GCM_ONE, BlendFactors[BlendModeTable[mode].DestColor], 
                BlendFactors[BlendModeTable[mode].SourceAlpha], BlendFactors[BlendModeTable[mode].DestAlpha]);
        }
        else
        {
            cellGcmSetBlendFunc(GcmCtx, BlendFactors[BlendModeTable[mode].SourceColor], BlendFactors[BlendModeTable[mode].DestColor], 
                BlendFactors[BlendModeTable[mode].SourceColor], BlendFactors[BlendModeTable[mode].DestColor]);
        }
    }
    cellGcmSetBlendEquation(GcmCtx, BlendOps[BlendModeTable[mode].Operator], BlendOps[BlendModeTable[mode].Operator]);
}

uint8_t getSurfaceFormatFromTextureFormat(uint8_t format)
{
    switch(format)
    {
        default: 
        case CELL_GCM_TEXTURE_A8R8G8B8: return CELL_GCM_SURFACE_A8R8G8B8; break;
        case CELL_GCM_TEXTURE_B8:       return CELL_GCM_SURFACE_B8; break;
        case CELL_GCM_TEXTURE_D1R5G5B5: 
        case CELL_GCM_TEXTURE_A1R5G5B5: return CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5; break;
        case CELL_GCM_TEXTURE_R5G6B5:   return CELL_GCM_SURFACE_R5G6B5; break;
        case CELL_GCM_TEXTURE_G8B8:     return CELL_GCM_SURFACE_G8B8; break;
        case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT: return CELL_GCM_SURFACE_F_W16Z16Y16X16; break;
        case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT: return CELL_GCM_SURFACE_F_W32Z32Y32X32; break;
        case CELL_GCM_TEXTURE_X32_FLOAT:return CELL_GCM_SURFACE_F_X32; break;
        case CELL_GCM_TEXTURE_D8R8G8B8: return CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8; break;           
    }
}

void addDepthStencilToSurface(CellGcmSurface& surface, CellGcmSurface dssurface)
{
    surface.depthFormat   = dssurface.depthFormat;
    surface.depthLocation = dssurface.depthLocation;
    surface.depthOffset   = dssurface.depthOffset;
    surface.depthPitch    = dssurface.depthPitch;
}


RenderTarget* HAL::CreateRenderTarget(CellGcmSurface surface, unsigned frameHeightIfStereo)
{
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(ImageSize(surface.width, surface.height),
        RBuffer_User, Image_R8G8B8A8, 0);
    if ( !prt )
        return 0;
    if ( prt->GetRenderTargetData() != 0 )
        return prt;

    RenderTargetData::UpdateData(prt, surface, 0, frameHeightIfStereo);
    return prt;
}


RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    PS3::Texture* pt = (PS3::Texture*)texture;

    if ( !pt || pt->TextureCount != 1 )
        return 0;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;
    Ptr<DepthStencilBuffer> pdsb;

    const CellGcmTexture& tex = pt->pTextures[0].Tex;

    CellGcmSurface surface;
    memset(&surface, 0, sizeof surface);
    surface.type             = tex.format & CELL_GCM_TEXTURE_LN ? CELL_GCM_SURFACE_PITCH : CELL_GCM_SURFACE_SWIZZLE;
    surface.colorFormat      = getSurfaceFormatFromTextureFormat(tex.format);
	surface.antialias        = CELL_GCM_SURFACE_CENTER_1;
	surface.colorTarget      = CELL_GCM_SURFACE_TARGET_0;
	surface.colorLocation[0] = tex.location;
	surface.colorOffset[0]   = tex.offset;
	surface.colorPitch[0]    = tex.pitch;
	surface.colorPitch[1]    = 64;
	surface.colorPitch[2]    = 64;
	surface.colorPitch[3]    = 64;
	surface.width            = tex.width;
	surface.height           = tex.height;
	surface.x                = 0;
	surface.y                = 0;
    surface.depthFormat      = CELL_GCM_SURFACE_Z24S8;
    surface.depthLocation    = CELL_GCM_LOCATION_LOCAL;
    surface.depthOffset      = 0;
    surface.depthPitch       = 64;


    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (PS3::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                addDepthStencilToSurface(surface, surf->Surface);
        }
    }


    RenderTargetData::UpdateData(prt, surface, pdsb);
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

    const CellGcmTexture& tex = pt->pTextures[0].Tex;

    CellGcmSurface surface;
    memset(&surface, 0, sizeof surface);
    surface.type             = tex.format & CELL_GCM_TEXTURE_LN ? CELL_GCM_SURFACE_PITCH : CELL_GCM_SURFACE_SWIZZLE;
    surface.colorFormat      = getSurfaceFormatFromTextureFormat(tex.format);
    surface.antialias        = CELL_GCM_SURFACE_CENTER_1;
    surface.colorTarget      = CELL_GCM_SURFACE_TARGET_0;
    surface.colorLocation[0] = tex.location;
    surface.colorOffset[0]   = tex.offset;
    surface.colorPitch[0]    = tex.pitch;
    surface.colorPitch[1]    = 64;
    surface.colorPitch[2]    = 64;
    surface.colorPitch[3]    = 64;
    surface.width            = tex.width;;
    surface.height           = tex.height;
    surface.x                = 0;
    surface.y                = 0;
    surface.depthFormat      = CELL_GCM_SURFACE_Z24S8;
    surface.depthLocation    = CELL_GCM_LOCATION_LOCAL;
    surface.depthOffset      = 0;
    surface.depthPitch       = 64;

    Ptr<DepthStencilBuffer> pdsb = 0;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(size);
        if ( pdsb )
        {
            DepthStencilSurface* surf = (PS3::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                addDepthStencilToSurface(surface, surf->Surface);
        }
    }

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 



    RenderTargetData::UpdateData(prt, surface, pdsb);
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
    RenderTargetData* prtdata = (PS3::RenderTargetData*)ptarget->GetRenderTargetData();
    if ( setState )
    {
        if (prtdata->FrameHeightIfStereo)
        {
            prtdata->RenderSurface.colorOffset[0] = prtdata->ColorOffset;
            prtdata->RenderSurface.depthOffset = prtdata->DepthOffset;
            if (Matrices->S3DDisplay == StereoRight)
            {
                prtdata->RenderSurface.colorOffset[0] += prtdata->RenderSurface.colorPitch[0] * (prtdata->FrameHeightIfStereo + 30);
                prtdata->RenderSurface.depthOffset += prtdata->RenderSurface.depthPitch * (prtdata->FrameHeightIfStereo + 30);
            }
        }
        const CellGcmSurface& surface = prtdata->RenderSurface;
        cellGcmSetSurface(GcmCtx, &surface);
    }

    entry.pRenderTarget = ptarget;

    // Replace the stack entry at the bottom, or if the stack is empty, add one.
    if ( RenderTargetStack.GetSize() > 0 )
        RenderTargetStack[0] = entry;
    else
        RenderTargetStack.PushBack(entry);
    return true;
}

void HAL::SetStereoDisplay(StereoDisplay sDisplay, bool setstate)
{
    Matrices->S3DDisplay = sDisplay;
    Matrices->UVPOChanged = 1;

    SF_ASSERT(RenderTargetStack.GetSize() == 1);
    if (setstate)
    {
        RenderTargetData* prtdata = (PS3::RenderTargetData*)RenderTargetStack[0].pRenderTarget->GetRenderTargetData();

        if (prtdata->FrameHeightIfStereo)
        {
            prtdata->RenderSurface.colorOffset[0] = prtdata->ColorOffset;
            prtdata->RenderSurface.depthOffset = prtdata->DepthOffset;
            if (Matrices->S3DDisplay == StereoRight)
            {
                prtdata->RenderSurface.colorOffset[0] += prtdata->RenderSurface.colorPitch[0] * (prtdata->FrameHeightIfStereo + 30);
                prtdata->RenderSurface.depthOffset += prtdata->RenderSurface.depthPitch * (prtdata->FrameHeightIfStereo + 30);
            }

            const CellGcmSurface& surface = prtdata->RenderSurface;
            cellGcmSetSurface(GcmCtx, &surface);
        }
    }
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags)
{
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
    RenderTargetData* phd = (PS3::RenderTargetData*)prt->GetRenderTargetData();
    CellGcmSurface surface = phd->RenderSurface;

    const ImageSize& bs = prt->GetBufferSize();
    float scale[4], offset[4];
    offset[0] = scale[0] = bs.Width * 0.5f;
    scale[1] = bs.Height * -0.5f;
    offset[1] = bs.Height * 0.5f;
    offset[2] = scale[2] = 0.5;
    offset[3] = scale[3] = 0;
    cellGcmSetSurface(GcmCtx, &surface);
    cellGcmSetViewport(GcmCtx, 0, 0, bs.Width, bs.Height, 0, 1, scale, offset);           
    cellGcmSetScissor(GcmCtx, 0, 0, bs.Width, bs.Height);
    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        cellGcmSetClearColor(GcmCtx, 0x00000000);
        cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A);
    }

    // Setup viewport.
    Rect<int> viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
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

void HAL::PopRenderTarget(unsigned)
{
    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;
    prt->SetInUse(false);
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        PS3::RenderTargetData* plasthd = (PS3::RenderTargetData*)prt->GetRenderTargetData();
        plasthd->pDepthStencilBuffer = 0;
    }
    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();
    RenderTargetData* phd = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        phd = (PS3::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
        CellGcmSurface surface = phd->RenderSurface;

        // Restore the old render target on the device.
        cellGcmSetSurface(GcmCtx, &surface);
        ++AccumulatedStats.RTChanges;
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Reset the viewport to the last render target on the stack.
    HALState |= HS_ViewValid;
    updateViewport();
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
        uint8_t id;
        cellGcmGetCurrentDisplayBufferId(&id);
        const CellGcmDisplayInfo *info = cellGcmGetDisplayInfo();

        rtSize = ImageSize(info[id].width, info[id].height);
    }

    // No way to query the current rendertarget on PS3. Must call SetRenderTarget externally.
    return pRenderBufferManager->Initialize(pTextureManager, Image_PS3_A8R8G8B8, rtSize);
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
            PrimitiveBatch::DP_Single, &VertexXY16iInstance::Format, &ShaderData);
        Matrix2F mvp(prim->GetFilterAreaMatrix().GetMatrix2D(), Matrices->UserView);
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        ShaderData.Finish(1);

        UInt32 offset;
        SInt32 result = cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
		SF_ASSERT( result == CELL_OK );
        setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iInstance::Format, CELL_GCM_LOCATION_MAIN, offset);
        drawPrimitive(6, 1);
        FilterStack.PushBack(e);
        return;
    }

    if ( (HALState & HS_CachedFilter) )
    {
        FilterStack.PushBack(e);
        return;
    }

    // Disable masking from previous target, if this filter primitive doesn't have any masking.
    if ( MaskStackTop != 0 && !prim->GetMaskPresent() && prim->GetCacheState() != FilterPrimitive::Cache_Target )
    {
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);
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
            cellGcmSetClearDepthStencil(GcmCtx, MaskStackTop);
            cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_S);
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

    UInt32 offset;
    SInt32 result = cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
	SF_ASSERT( result == CELL_OK );
    setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iAlpha::Format, CELL_GCM_LOCATION_MAIN, offset);

    const VertexFormat* pvf[3];
    SManager.MapVertexFormat(PrimFill_UVTexture, &VertexXY16iAlpha::Format, &pvf[0], &pvf[1], &pvf[2] );
    cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);
    cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_FALSE);

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

            CellGcmSurface& surface = ((PS3::RenderTargetData*)temporaryTextures[1]->GetRenderTargetData())->RenderSurface;
            cellGcmSetSurface(GcmCtx, &surface);
            cellGcmSetClearColor(GcmCtx, 0x00000000);
            RenderTarget* prt = temporaryTextures[1];
            const ImageSize& bs = prt->GetBufferSize();
            float scale[4], offset[4];
            offset[0] = scale[0] = bs.Width * 0.5f;
            scale[1] = bs.Height * -0.5f;
            offset[1] = bs.Height * 0.5f;
            offset[2] = scale[2] = 0.5;
            offset[3] = scale[3] = 0;
            ++AccumulatedStats.RTChanges;

            cellGcmSetClearColor(GcmCtx, 0x00000000);
            cellGcmSetViewport(GcmCtx, 0, 0, bs.Width, bs.Height, 0, 1, scale, offset);           
            cellGcmSetScissor(GcmCtx, 0, 0, bs.Width, bs.Height);
            cellGcmSetClearSurface(GcmCtx, CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A);
            
            // Scale to the size of the destination.
            const Rect<int>& viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
            VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());    
            ViewRect = Rect<int>(viewRect.x1, viewRect.y1, viewRect.x2, viewRect.y2);
            HALState |= HS_ViewValid;
            updateViewport();

            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);                          
            SManager.SetFilterFill(mvp, Cxform::Identity, filter, temporaryTextures, shaders, pass, passes, pvf[0], &ShaderData);
            drawPrimitive(6,1);

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
	    // Cache the 2nd last step so it might be available as a cached filter next time.
	    RenderTarget* cacheResults[2] = { temporaryTextures[0], temporaryTextures[2] };
        e.pPrimitive->SetCacheResults(cacheAsBitmap ? FilterPrimitive::Cache_Target : FilterPrimitive::Cache_PreTarget, cacheResults, cacheAsBitmap ? 1 : 2);
		((PS3::RenderTargetData*)cacheResults[0]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
		if ( cacheResults[1] )
			((PS3::RenderTargetData*)cacheResults[1]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
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
        cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_TRUE);
        cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);
    }

    // Now actually draw the filtered sub-scene to the target below.
    if (passes != 0)
    {
        // 'Real' filter.
        const Matrix2F& mvp = Matrices->UserView * e.pPrimitive->GetFilterAreaMatrix().GetMatrix2D();
        const Cxform& cx = e.pPrimitive->GetFilterAreaMatrix().GetCxform();
        SManager.SetFilterFill(mvp, cx, filter, temporaryTextures, shaders, pass, passes, pvf[0], &ShaderData);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
        drawPrimitive(6,1);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget) != 0);
    }
    else
    {
        // CacheAsBitmap
        drawCachedFilter(e.pPrimitive);
    }

    if ( HALState & HS_DrawingMask )
        cellGcmSetColorMask(GcmCtx, 0);

    // Cleanup.
    for ( int i = 0; i < MaxTemporaryTextures; ++i )
    {
        if (temporaryTextures[i])
            temporaryTextures[i]->SetInUse(false);
    }
    AccumulatedStats.Filters += filters->GetFilterCount();

}

void HAL::drawCachedFilter(FilterPrimitive* primitive)
{
    UInt32 offset;
    SInt32 result = cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
	SF_ASSERT( result == CELL_OK );
    setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iAlpha::Format, CELL_GCM_LOCATION_MAIN, offset);

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
            const VertexFormat* pvf[3];
            SManager.MapVertexFormat(PrimFill_Texture, &VertexXY16iUV::Format, &pvf[0], &pvf[1], &pvf[2] );
            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
            SManager.SetFilterFill(mvp, Cxform::Identity, filter, temporaryTextures, shaders, passes-1, passes, pvf[0], &ShaderData);
            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
            drawPrimitive(6,1);
            PopRenderTarget();
            
            // Re-[en/dis]able masking from previous target, if available.
            if ( MaskStackTop != 0 )
            {
                cellGcmSetStencilTestEnable(GcmCtx, CELL_GCM_TRUE);
                cellGcmSetColorMask(GcmCtx, CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A);
            }

            // Re-[en/dis]able masking from previous target, if available.
            if ( HALState & HS_DrawingMask )
                cellGcmSetColorMask(GcmCtx, 0);

            // Set this as the final cache result, and then render it.
            RenderTarget* prt = temporaryTextures[1];
            primitive->SetCacheResults(FilterPrimitive::Cache_Target, &prt, 1);
            ((PS3::RenderTargetData*)prt->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(primitive);
            drawCachedFilter(primitive);

            // Cleanup.
            for ( int i = 0; i < MaxTemporaryTextures; ++i )
            {
                if (temporaryTextures[i])
                    temporaryTextures[i]->SetInUse(false);
            }
            break;
        }

        // We have a final filtered texture. Just apply it to a screen quad.
        case FilterPrimitive::Cache_Target:
        {
            const VertexFormat* pvf[3];
            SManager.MapVertexFormat(PrimFill_Texture, &VertexXY16iUV::Format, &pvf[0], &pvf[1], &pvf[2] );
            unsigned fillFlags = (FillFlags|FF_Cxform|FF_AlphaWrite);
            const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Single, pvf[0], &ShaderData);

            RenderTarget* results;
            primitive->GetCacheResults(&results, 1);
            Texture* ptexture = (PS3::Texture*)results->GetTexture();
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
            ShaderData.Finish();

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
    UInt32 offset;
    cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
    setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iInstance::Format, CELL_GCM_LOCATION_MAIN, offset);
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    cellGcmSetDrawArrays(GcmCtx, CELL_GCM_PRIMITIVE_TRIANGLES, 0, indexCount);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UPInt indexOffset)
{
    cellGcmSetDrawIndexArray(GcmCtx, CELL_GCM_PRIMITIVE_TRIANGLES, indexCount,
        CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, CELL_GCM_LOCATION_MAIN, indexOffset);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, unsigned channelMask, const UInt32* values)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_DIPaletteMap), __FUNCTION__);

    // Create a temporary texture with the palette map. There may be a more efficient way to do this; however, using
    // uniforms seems unworkable, due to shader constant slot constraints.
    ImageData data;
    Ptr<Render::Texture> ptex = *pTextureManager->CreateTexture(Image_PS3_A8R8G8B8, 1, ImageSize(256, 4), ImageUse_Map_Mask, 0);
    if ( !ptex->Map(&data, 0, 1) )
        return;
    for ( int channel = 0; channel < 4; ++channel )
    {
        UInt32* dataPtr = (UInt32*)data.GetScanline(channel);
        if ( channelMask & (1<<channel))
        {
            memcpy(dataPtr, values + channel*256, 256*sizeof(UInt32));
        }
        else
        {
            // Channel was not provided, just do a straight mapping.
            for ( unsigned i = 0; i < 256; ++i )
                *dataPtr++ = (i << (channel*8));
        }
    }
    if (!ptex->Unmap())
        return;

    // First pass overwrites everything.
    applyBlendMode(Blend_OverwriteAll, true, true);
    SManager.SetDrawablePaletteMap(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), mvp,
        ptex, &VertexXY16iInstance::Format, &ShaderData);

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
    // Set the vertices, and Draw
    UInt32 offset;
    SInt32 result = cellGcmAddressToOffset(Cache.pMaskEraseBatchVertexBuffer, &offset);
	SF_ASSERT( result == CELL_OK );
    setVertexArray(ShaderData.GetCurrentShaders().pVS, &VertexXY16iInstance::Format, CELL_GCM_LOCATION_MAIN, offset);
    cellGcmSetDrawArrays(GcmCtx, CELL_GCM_PRIMITIVE_TRIANGLES, 0, 6);
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
#if !defined(SF_BUILD_SHIPPING)
    static PS3::RenderEvent Events[Event_Count];
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

void HAL::destroyTemporaryTextures()
{
    if (TemporaryTextureList.GetSize() >= MaximumTempTextures)
        TemporaryTextureFence->WaitFence(FenceType_Fragment);
    if (TemporaryTextureFence && !TemporaryTextureFence->IsPending(FenceType_Fragment))
        TemporaryTextureList.Clear();
}

}}} // Scaleform::Render::PS3
