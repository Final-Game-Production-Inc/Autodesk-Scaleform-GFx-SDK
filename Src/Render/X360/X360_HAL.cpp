/**************************************************************************

Filename    :   X360_HAL.cpp
Content     :   XBox360 Renderer HAL Prototype implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_HeapNew.h"

#include "Render/Render_BufferGeneric.h"
#include "Render/Render_TextureUtil.h"
#include "Render/X360/X360_HAL.h"
#include "Render/X360/X360_Events.h"

namespace Scaleform { namespace Render { namespace X360 {

// ***** RenderHAL_X360

HAL::HAL(ThreadCommandQueue* commandQueue)
:   Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    pDevice(0),
    RSync(),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::Console_Defaults, &RSync ),
    PrevBatchType(PrimitiveBatch::DP_None),
    StencilChecked(false), StencilAvailable(false), MultiBitStencil(false),
    DepthBufferAvailable(false),
    StencilOpInc(D3DSTENCILOP_REPLACE),
    pMemoryManager(0),
    OwnMemoryManager(false)
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
        SF_DEBUG_WARNING1(1, "X360::MemoryManager, unknown memory type %d", type);

    case Memory_X360_Physical:
        return XPhysicalAlloc(size, MAXULONG_PTR, align,
                              PAGE_READWRITE | PAGE_WRITECOMBINE | (size > 65536 ? MEM_LARGE_PAGES : 0));

    // Offset in number of tiles for D3DSURFACE_PARAMETERS. 'size' parameter is the number of EDRAM tiles requested.
    case Memory_X360_EDRAM:
        return 0;   
    }
}

void    MemoryManager::Free(void* pmem, UPInt size)
{
    XPhysicalFree(pmem);
}

// *** RenderHAL_X360 Implementation
    
bool HAL::InitHAL(const X360::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    if (!params.pD3DDevice)
        return 0; 

    pDevice = params.pD3DDevice;
    pDevice->AddRef();

    RSync.SetDevice(pDevice);

    if (params.pMemoryManager)
    {
        pMemoryManager   = (X360::MemoryManager*)params.pMemoryManager;
        OwnMemoryManager = false;
    }
    else
    {
        pMemoryManager = SF_NEW MemoryManager();
        OwnMemoryManager = true;
    }

    if (!SManager.Initialize(this) ||
        !Cache.Initialize(pDevice, pMemoryManager))
    {
        ShutdownHAL();
        return false;
    }

    // Create Texture manager if needed.
    if (params.pTextureManager)
        pTextureManager = (X360::TextureManager*)(params.pTextureManager.GetPtr());
    else
    {
        pTextureManager = *SF_HEAP_AUTO_NEW(this)
            TextureManager(pDevice, pMemoryManager, params.RenderThreadId, pRTCommandQueue);
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
        // Create the default render target, and manager.
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric();
        if ( !pRenderBufferManager )
        {
            ShutdownHAL();
            return false;
        }
    }

    // Initialize the render buffer manager.
    ImageSize rtSize;
    if ( !createDefaultRenderBuffer() || !GetDefaultRenderTarget() )
    {
        XVIDEO_MODE videoMode;
        XGetVideoMode(&videoMode);
        rtSize = ImageSize(videoMode.dwDisplayWidth, videoMode.dwDisplayHeight);
    }
    else
    {
        rtSize = GetDefaultRenderTarget()->GetSize();
    }

    if (!pRenderBufferManager->Initialize(pTextureManager, Image_R8G8B8A8, rtSize))
    {
        ShutdownHAL();
        return false;
    }
    
    // Initialize shaders.
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        if (VertexShaderDesc::Descs[i] && VertexShaderDesc::Descs[i]->pBinary)
        {
            if (!StaticVShaders[i].Init(pDevice, VertexShaderDesc::Descs[i]))
            {
                ShutdownHAL();
                return false;
            }
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        if (FragShaderDesc::Descs[i] && FragShaderDesc::Descs[i]->pBinary)
        {
            if ( !StaticFShaders[i].Init(pDevice, FragShaderDesc::Descs[i]) )
            {
                ShutdownHAL();
                return false;
            }
        }
    }

    // Stencil op.
    StencilOpInc = D3DSTENCILOP_INCR;

    memcpy(&PresentParams, &params.PresentParams, sizeof(D3DPRESENT_PARAMETERS));
    HALState|= HS_ModeSet;
    notifyHandlers(HAL_Initialize);
    return true;
}

// HAL Shutdown
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    // Global unset all, incase any resources we are about to destroy are set currently.
    pDevice->UnsetAll();

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
        if (VertexShaderDesc::Descs[i] && VertexShaderDesc::Descs[i]->pBinary)
            StaticVShaders[i].Shutdown();

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
        if (FragShaderDesc::Descs[i] && FragShaderDesc::Descs[i]->pBinary)
            StaticFShaders[i].Shutdown();

    destroyRenderBuffers();
    pRenderBufferManager.Clear();

    // Do TextureManager::Reset to ensure shutdown on the current thread.
    pTextureManager->Reset();
    pTextureManager.Clear();
    SManager.Reset();
    Cache.Reset();

    if ( OwnMemoryManager )
    {
        delete pMemoryManager;
        pMemoryManager = 0;
    }

    RSync.SetDevice(0);

    pDevice->Release();
    pDevice = 0;
    return true;
}

    
// ***** Rendering
bool HAL::BeginScene()
{
    if ( !Render::HAL::BeginScene())
        return false;

    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__ "-SetState");

    // Blending render states.
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);    

    // Not necessary of not alpha testing:
    pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);  // Important!

    union 
    {
        float fbias;
        DWORD d;
    } bias;
    bias.fbias = -0.75f;

    // Must reset both stages since ApplySampleMode modifies both.

    pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
    pDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, bias.d );

    // No ZWRITE by default
    pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

    // Clear more states..
    pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,  0);

    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
        D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

    // Turn off back-face culling.
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    // Start the scene
    if (!(VMCFlags&HALConfig_NoSceneCalls))
        pDevice->BeginScene();

    // Remove render targets past the first. We do not render to them, and they may have MSAA settings which
    // differ from targets allocated for filters, etc. in which case you will receive a GPU error.
    for ( unsigned i = 1; i < D3DMAXRENDERTARGETS; ++i )
        pDevice->SetRenderTarget(i, 0);

    SManager.BeginScene();
    ShaderData.BeginScene();

    return true;
}

bool HAL::EndScene()
{
    if ( !Render::HAL::EndScene())
        return false;

    pTextureManager->EndScene();
    SManager.EndScene();

    if (!(VMCFlags&HALConfig_NoSceneCalls))
        pDevice->EndScene();

    return true;
}

// Updates D3D HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    D3DVIEWPORTx vp;
    Rect<int>    vpRect;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(Viewport::View_HalfPixelOffset, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        /*
        // TBD: Prepend UserMatrix here is incorrect for nested viewport-based
        // mask clipping; what's needed is a complex combination of viewport and
        // coordinate adjustment. Until this is done, mask viewport clipping will be
        // in the wrong place for UserMatrix.
        if (UserMatrix != Matrix2F::Identity)
        {
            Rect<int> viewportRect;
            Rect<int> userViewRect(
                ViewRect.x1 + (int)UserMatrix.Tx(),
                ViewRect.y1 + (int)UserMatrix.Ty(),
                Size<int>((int)(UserMatrix.Sx() * (float)ViewRect.Width()),
                          (int)(UserMatrix.Sy() * (float)ViewRect.Height())));

            VP.GetClippedRect(&viewportRect);
            viewportRect.IntersectRect(&vpRect, userViewRect);
        }
        */

        if ( !(HALState & HS_InRenderTarget) )
            vpRect = ViewRect;
        else
            vpRect.SetRect(VP.Left, VP.Top, VP.Left + VP.Width, VP.Top + VP.Height);
    }

    vp.X        = vpRect.x1;
    vp.Y        = vpRect.y1;

    // DX9 can't handle a vp with zero area.
    vp.Width    = (DWORD)Alg::Max<int>(vpRect.Width(), 1);
    vp.Height   = (DWORD)Alg::Max<int>(vpRect.Height(), 1);

    vp.MinZ     = 0.0f;
    vp.MaxZ     = 0.0f;
    pDevice->SetViewport(&vp);
}

void HAL::SetDisplayPass(DisplayPass pass)
{
    CurrentPass = pass;

    // If on the final display pass, ensure that the current target is set to the top level
    // render target on the stack (prepass/user code might have switched this).
    if ( CurrentPass == Display_Final && RenderTargetStack.GetSize() > 0 )
    {
        RenderTarget* prt = RenderTargetStack.Back().pRenderTarget;
        RenderTargetData* phd = (X360::RenderTargetData*)prt->GetRenderTargetData();
        pDevice->SetRenderTarget(0, phd->pRenderSurface);
        if ( phd->pDepthStencilSurface )
            pDevice->SetDepthStencilSurface(phd->pDepthStencilSurface);
        else 
            pDevice->SetDepthStencilSurface(0);
        ++AccumulatedStats.RTChanges;

        notifyHandlers(HAL_FinalPassBegin);

        if (VP.GetClippedRect(&ViewRect))
            HALState |= HS_ViewValid;
        else
            HALState &= ~HS_ViewValid;
        updateViewport();
    }
}

// Draws a range of pre-cached and preprocessed primitives
void HAL::DrawProcessedPrimitive(Primitive* pprimitive,
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
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
        pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE );
        DrawProcessedPrimitive(pprimitive, pstart, pend);
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0 );
        pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE );
        drawingMask = false;
    }

    SF_ASSERT(pend != 0);

    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    ShaderData.BeginPrimitive();

    unsigned bidx = 0;
    while (pbatch != pend)
    {
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        MeshCacheItem* pmesh = (MeshCacheItem*)pbatch->GetCacheItem();
        unsigned       meshIndex = pbatch->GetMeshIndex();
        unsigned       batchMeshCount = pbatch->GetMeshCount();

        if (pmesh)
        {
            Profiler.SetBatch((UPInt)pprimitive, bidx);

            unsigned fillFlags = FillFlags | FF_LeaveOpen;
            if ( batchMeshCount > 0 )
                fillFlags |= pprimitive->Meshes[0].M.Has3D() ? FF_3DProjection : 0;

            const ShaderManager::Shader& pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, pbatch->pFormat, batchMeshCount, Matrices,
                &pprimitive->Meshes[meshIndex], &ShaderData);

            if ((HALState & HS_ViewValid) && pShader ) 
            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                    (pbatch->Type != PrimitiveBatch::DP_Virtual));

                UPInt   indexOffset = pmesh->IndexOffset / sizeof (IndexType);
                pDevice->SetIndices(pmesh->GetIndexBuffer());
                pDevice->SetStreamSource(0, pmesh->GetVertexBuffer(),
                    (UINT)pmesh->VertexOffset, pbatch->pFormat->Size);

                // Draw the object with cached mesh.
                if (pbatch->Type != PrimitiveBatch::DP_Instanced)
                {
                    AccumulatedStats.Meshes += pmesh->MeshCount;
                    AccumulatedStats.Triangles += pmesh->IndexCount / 3;
                    setLinearStreamSource(pbatch->Type);
                    ShaderData.Finish(batchMeshCount);

                    drawIndexedPrimitive(pmesh->IndexCount, pmesh->MeshCount, indexOffset, 0);
                }
                else
                {
                    AccumulatedStats.Meshes += batchMeshCount;
                    AccumulatedStats.Triangles += (pmesh->IndexCount / 3) * batchMeshCount;
                    setInstancedStreamSource(pShader, pmesh);
                    ShaderData.Finish(batchMeshCount);
                    drawIndexedInstanced(pmesh->IndexCount, pbatch->GetMeshCount(), 0);
                }
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
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_High);

    typedef ComplexMesh::FillRecord   FillRecord;
    typedef PrimitiveBatch::BatchType BatchType;

    MeshCacheItem* pmesh = (MeshCacheItem*)complexMesh->GetCacheItem();
    if (!checkState(HS_InDisplay, __FUNCTION__) || !pmesh)
        return;

    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
        pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE );
        DrawProcessedComplexMeshes(complexMesh, matrices);
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0 );
        pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE );
        drawingMask = false;
    }

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;
    unsigned    formatIndex;
    unsigned    maxDrawCount = 1;
    unsigned    vertexBaseIndex = 0;
    unsigned    vertexSize = 0;
    unsigned    indexBufferOffset = (unsigned)(pmesh->IndexOffset / sizeof(IndexType));

    if (instanceCount > 1 && SManager.HasInstancingSupport())
    {
        maxDrawCount = Alg::Min(instanceCount, Cache.GetParams().MaxBatchInstances);
        batchType = PrimitiveBatch::DP_Instanced;
        formatIndex = 1;
    }
    else
    {
        batchType = PrimitiveBatch::DP_Single;
        formatIndex = 0;
        setLinearStreamSource(batchType);
    }

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();
    pDevice->SetIndices(pmesh->GetIndexBuffer());
    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        Profiler.SetBatch((UPInt)complexMesh, fillIndex);

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
        const ShaderManager::Shader& pso = SManager.SetFill(fillType, fillFlags, batchType, fr.pFormats[formatIndex], &ShaderData);

        vertexSize      = fr.pFormats[formatIndex]->Size;
        vertexBaseIndex = 0;
        pDevice->SetStreamSource(0, pmesh->GetVertexBuffer(),
                                 (UINT)(fr.VertexByteOffset + pmesh->VertexOffset),
                                 vertexSize);

        UByte textureCount = fr.pFill->GetTextureCount();
        bool solid = (fillType == PrimFill_None || fillType == PrimFill_Mask || fillType == PrimFill_SolidColor);

        for (unsigned i = 0; i < instanceCount; i++)
        {            
            const HMatrix& hm = matrices[startIndex + i];

            ShaderData.SetMatrix(pso, Uniform::SU_mvp, complexMesh->GetVertexMatrix(), hm, Matrices, 0, i%maxDrawCount);
            if (solid)
                ShaderData.SetColor(pso, Uniform::SU_cxmul, Profiler.GetColor(fr.pFill->GetSolidColor()), 0, i%maxDrawCount);
            else if (fillFlags & FF_Cxform)
                ShaderData.SetCxform(pso, Profiler.GetCxform(hm.GetCxform()), 0, i);

            for (unsigned tm = 0, stage = 0; tm < textureCount; tm++)
            {
                ShaderData.SetMatrix(pso, Uniform::SU_texgen, textureMatrices[fr.FillMatrixIndex[tm]], tm, i%maxDrawCount);
                Texture* ptex = (Texture*)fr.pFill->GetTexture(tm);
                ShaderData.SetTexture(pso, Uniform::SU_tex, ptex, fr.pFill->GetFillMode(tm), stage);
                stage += ptex->GetPlaneCount();
            }

            bool lastPrimitive = (i == instanceCount-1);
            if ( batchType != PrimitiveBatch::DP_Instanced )
            {
                ShaderData.Finish(1);
                drawIndexedPrimitive(fr.IndexCount, 1, fr.IndexOffset + indexBufferOffset, vertexBaseIndex);
                pDevice->DrawIndexedPrimitive(
                    D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, fr.VertexCount,
                    fr.IndexOffset + indexBufferOffset, fr.IndexCount / 3 );
                AccumulatedStats.Primitives++;
                if ( !lastPrimitive )
                    ShaderData.BeginPrimitive();
            }
            else if (( (i+1) % maxDrawCount == 0 && i != 0) || lastPrimitive )
			{
                unsigned drawCount = maxDrawCount;
                if ( lastPrimitive && (i+1) % maxDrawCount != 0)
                    drawCount = (i+1) % maxDrawCount;
                setInstancedStreamSource(pso, pmesh, fr);
                ShaderData.Finish(drawCount);
                drawIndexedInstanced(fr.IndexCount, instanceCount, vertexBaseIndex);
                AccumulatedStats.Primitives++;
                if ( !lastPrimitive )
                    ShaderData.BeginPrimitive();
            }
        }

        vertexBaseIndex += fr.VertexCount;

    } // for (fill record)

    pmesh->GPUFence = Cache.GetRenderSync()->InsertFence();
    pmesh->MoveToCacheListFront(MCL_ThisFrame);
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
    ScopedRenderEvent GPUEvent(GetEvent(Event_Mask), __FUNCTION__);

    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    Profiler.SetDrawMode(1);

    if (!StencilAvailable && !DepthBufferAvailable)
    {
        if (!checkMaskBufferCaps())
            return;
    }
    
    // These states are applicable to most stencil operations below.
    if (StencilAvailable)
    {
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
        pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
        pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    }

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        // Erase rectangles of these matrices; must be done even for clipped masks.
        if (StencilAvailable && (MultiBitStencil && (StencilOpInc != D3DSTENCILOP_REPLACE)))
        {
            // Any stencil of value greater then MaskStackTop should be set to it;
            // i.e. replace when (MaskStackTop < stencil value).
            DWORD maxStencilValue = (DWORD)MaskStackTop;
            pDevice->SetRenderState(D3DRS_STENCILREF, maxStencilValue);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

            MaskPrimitive* erasePrim = MaskStack[MaskStackTop].pPrimitive;
            drawMaskClearRectangles(erasePrim->GetMaskAreaMatrices(), erasePrim->GetMaskCount());
        }
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
        // Apply new viewport clipping.
        const Matrix2F &m = prim->GetMaskAreaMatrix(0).GetMatrix2D();
        // Clipped matrices are always in View coordinate space, to allow
        // matrix to be use for erase operation above. This means that we don't
        // have to do an EncloseTransform.
        SF_ASSERT((m.Shx() == 0.0f) && (m.Shy() == 0.0f));
        Rect<int>  boundClip((int)m.Tx(), (int)m.Ty(),
                             (int)(m.Tx() + m.Sx()), (int)(m.Ty() + m.Sy()));
        boundClip.Offset(VP.Left, VP.Top);
        
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
            if (StencilAvailable)
                pDevice->ClearF(D3DCLEAR_STENCIL, 0, 0, 0.0f, 0);
            else if (DepthBufferAvailable)
                pDevice->ClearF(D3DCLEAR_ZBUFFER, 0, 0, 1.0f, 0); // Z -> 1.0f
        }
    }

    else if ((MaskStackTop == 1) && viewportValid)
    {
        // Clear view rectangles.
        if (StencilAvailable)
        {
            // Unconditionally overwrite stencil rectangles with REF value of 0.
            pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE);
            pDevice->SetRenderState(D3DRS_STENCILREF, 0);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

            drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
            pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        }
        else
        {
            // Depth clears bounds. Don't use drawMaskClearRectangles yet as it doesn't
            // set proper Z.
            UPInt maskCount = prim->GetMaskCount();
            for (UPInt i = 0; i< maskCount; i++)
            {
                const Matrix2F &m = prim->GetMaskAreaMatrix(i).GetMatrix2D();
                RectF           bounds(m.EncloseTransform(RectF(1.0f)));
                Rect<int>       boundClip((int)bounds.x1, (int)bounds.y1,
                                          (int)bounds.x2, (int)bounds.y2);
                boundClip.Offset(VP.Left, VP.Top);

                if (boundClip.IntersectRect(&boundClip, ViewRect))
                {
                    D3DRECT r = { boundClip.x1, boundClip.y1, boundClip.x2, boundClip.y2 };
                    pDevice->Clear(1, &r, D3DCLEAR_ZBUFFER, 0, 1.0f, 0); // Z -> 1.0f
                }
            }
        }
    }


    if (StencilAvailable)
    {
        bool canIncDec = (MultiBitStencil && (StencilOpInc != D3DSTENCILOP_REPLACE));

        // Increment only if we support it.
        if (canIncDec)
        {
            pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)(MaskStackTop-1));
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
            pDevice->SetRenderState(D3DRS_STENCILPASS, StencilOpInc);
        }
        else if (MaskStackTop == 1)
        {
            pDevice->SetRenderState(D3DRS_STENCILREF, 1);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
        }
        else
        {   // If not supported, no change.
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        }
    }
    else if (DepthBufferAvailable)
    {
        if (MaskStackTop == 1)
        {
            // Set the correct render states in order to not modify the color buffer
            // but write the default Z-value everywhere. According to the shader code: should be 0.
            pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
            pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
            pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
            pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        }
        else
        {
            // No color write. Incr/Decr not supported.
            pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        }
    }
    ++AccumulatedStats.Masks;
}


void HAL::EndMaskSubmit()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Mask), 0, false);
    Profiler.SetDrawMode(0);

    if (!checkState(HS_InDisplay|HS_DrawingMask, __FUNCTION__))
        return;
    HALState &= ~HS_DrawingMask;    
    SF_ASSERT(MaskStackTop);

    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
    
    if (StencilAvailable)
    {
        // We draw only where the (MaskStackTop <= stencil), i.e. where the latest mask was drawn.
        // However, we don't change the stencil buffer
        pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
        pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP); 
        // Stencil counter.
        pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)MaskStackTop);
        pDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write and write only where the mask had written
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
    }
}


void HAL::PopMask()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_PopMask), __FUNCTION__);

    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    if (!StencilAvailable && !DepthBufferAvailable)
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
    if (StencilAvailable)
    {
        if (MaskStackTop == 0)
            pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        else
        {
            // Change ref value down, so that we can draw using previous mask.
            pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)MaskStackTop);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        }
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write and write only where the mask had written
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    }
}


bool HAL::checkMaskBufferCaps()
{
    if (!StencilChecked)
    {
        // Test for depth-stencil presence.
        IDirect3DSurfaceX *pdepthStencilSurface = 0;
        pDevice->GetDepthStencilSurface(&pdepthStencilSurface);
        if (pdepthStencilSurface)
        {
            D3DSURFACE_DESC sd;
            pdepthStencilSurface->GetDesc(&sd);

            switch(sd.Format)
            {
            case D3DFMT_D24S8:
            case D3DFMT_D24FS8:
                MultiBitStencil = 1;
                StencilAvailable = 1;
                break;
            }

            pdepthStencilSurface->Release();
            pdepthStencilSurface = 0;
            DepthBufferAvailable = 1;
        }
        else
            StencilAvailable = 0;

        StencilChecked = 1;
    }

    if (!StencilAvailable && !DepthBufferAvailable)
    {
#ifdef SF_BUILD_DEBUG
        static bool StencilWarned = 0;
        if (!StencilWarned)
        {
            SF_DEBUG_WARNING(1, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
            StencilWarned = 1;
        }
#endif
        return false;
    }
    return true;
}

//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Clear), __FUNCTION__);
    color = Profiler.GetClearColor(color);
    if (color.GetAlpha() == 0xFF)
    {
        // Do more efficient HW clear. Device::Clear expects render-target coordinates.
        D3DRECT  cr = { r.x1 + VP.Left, r.y1 + VP.Top,
                        r.x2 + VP.Left, r.y2 + VP.Top };
        D3DCOLOR d3dc = D3DCOLOR_XRGB(color.GetRed(), color.GetGreen(), color.GetBlue());
        pDevice->Clear(1, &cr, D3DCLEAR_TARGET, d3dc, 1.0f,  0);
    }
    else
    {
        float colorf[4];
        color.GetRGBAFloat(colorf, colorf+1, colorf+2, colorf+3);
        Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
            0.0f, (float)r.Height(), (float)r.y1);

        Matrix2F  mvp(m, Matrices->UserView);

        unsigned fillflags = 0;
        const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags, PrimitiveBatch::DP_Single, 
            MappedXY16iAlphaSolid[PrimitiveBatch::DP_Single], &ShaderData);
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        ShaderData.Finish(1);

        setLinearStreamSource(PrimitiveBatch::DP_Batch);
        pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer, 0, sizeof(VertexXY16iAlpha));
        drawPrimitive(6, 1);
    }
}



//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc)
{    
    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyBlend), __FUNCTION__);

    static const UInt32 BlendOps[BlendOp_Count] = 
    {
        D3DBLENDOP_ADD,         // BlendOp_ADD
        D3DBLENDOP_MAX,         // BlendOp_MAX
        D3DBLENDOP_MIN,         // BlendOp_MIN
        D3DBLENDOP_REVSUBTRACT, // BlendOp_REVSUBTRACT
    };

    static const UInt32 BlendFactors[BlendFactor_Count] = 
    {
        D3DBLEND_ZERO,          // BlendFactor_ZERO
        D3DBLEND_ONE,           // BlendFactor_ONE
        D3DBLEND_SRCALPHA,      // BlendFactor_SRCALPHA
        D3DBLEND_INVSRCALPHA,   // BlendFactor_INVSRCALPHA
        D3DBLEND_DESTCOLOR,     // BlendFactor_DESTCOLOR
        D3DBLEND_INVDESTCOLOR,  // BlendFactor_INVDESTCOLOR
    };

    if (!pDevice)
        return;

    pDevice->SetRenderState(D3DRS_BLENDOP, BlendOps[BlendModeTable[mode].Operator]);
    pDevice->SetRenderState(D3DRS_DESTBLEND, BlendFactors[BlendModeTable[mode].DestColor]);
    if ( sourceAc && BlendFactors[BlendModeTable[mode].SourceColor] == D3DBLEND_SRCALPHA )
        pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    else
        pDevice->SetRenderState(D3DRS_SRCBLEND, BlendFactors[BlendModeTable[mode].SourceColor]);        

    if (VP.Flags & Viewport::View_AlphaComposite || forceAc)
    {
        pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_BLENDOPALPHA, BlendOps[BlendModeTable[mode].Operator]);
        pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, BlendFactors[BlendModeTable[mode].SourceAlpha]);
        pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, BlendFactors[BlendModeTable[mode].DestAlpha]);
    }
    else
    {
        pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    }
}

RenderTarget* HAL::CreateRenderTarget(IDirect3DSurface9* pcolor, IDirect3DSurface9* pdepth, Render::Texture* presolve)
{
    D3DSURFACE_DESC sdesc;
    ImageSize dsSize(0), rtSize(0);

    if (SUCCEEDED(pcolor->GetDesc(&sdesc)))
        rtSize.SetSize(sdesc.Width, sdesc.Height);

    Ptr<RenderTarget> ptarget = pRenderBufferManager->CreateRenderTarget(rtSize, RBuffer_User, Image_R8G8B8A8, 0);

    Ptr<DepthStencilBuffer> pdsb = 0;
    if (SUCCEEDED(pdepth->GetDesc(&sdesc)))
    {
        dsSize.SetSize(sdesc.Width, sdesc.Height);
        pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsSize);
    }

    RenderTargetData::UpdateData(ptarget, pcolor, presolve ? ((X360::Texture*)presolve)->pTextures[0].Tex : 0, pdsb, pdepth);
    return ptarget;
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    X360::Texture* pt = (X360::Texture*)texture;

    if ( !pt || pt->TextureCount != 1 )
        return 0;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    SF_ASSERT(pt->pTextures[0].Surface);

    IDirect3DSurface9* prenderSurface = 0, *pdsSurface = 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;
    prenderSurface = pt->pTextures[0].Surface;

    Ptr<DepthStencilBuffer> pdsb;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (X360::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsSurface = surf->pDepthStencilSurface;
        }
    }

    RenderTargetData::UpdateData(prt, prenderSurface, pt->pTextures[0].Tex, pdsb, pdsSurface);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilSurface))
        return prt;

    Ptr<IDirect3DSurface9> prenderSurface = 0;
    IDirect3DSurface9* pdsSurface = 0;
    X360::Texture* pt = (X360::Texture*)prt->GetTexture();

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    SF_ASSERT(pt->pTextures[0].Surface);   
    prenderSurface = pt->pTextures[0].Surface;

    Ptr<DepthStencilBuffer> pdsb;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(size);
        if ( pdsb )
        {
            DepthStencilSurface* surf = (X360::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsSurface = surf->pDepthStencilSurface;
        }
    }

    RenderTargetData::UpdateData(prt, prenderSurface, pt->pTextures[0].Tex, pdsb, pdsSurface);
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
    RenderTargetData* prtdata = (X360::RenderTargetData*)ptarget->GetRenderTargetData();
    IDirect3DSurface9* pd3dsurface= prtdata->pRenderSurface;

    if ( setState )
    {
        pDevice->SetRenderTarget(0, pd3dsurface);
        if ( prtdata->pDepthStencilSurface )
            pDevice->SetDepthStencilSurface(prtdata->pDepthStencilSurface);
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
        SF_DEBUG_WARNING(1, __FUNCTION__ " - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }

    // Resolve the texture if required. This makes the assuption that the default render target is large
    // enough to hold the current contents of EDRAM.
    if (flags & HAL::PRT_Resolve)
    {
        static bool resolveWarning = false;
        bool resolved = false;
        if (RenderTargetStack.GetSize() != 0)
        {
            RenderTargetData* plasthd = (RenderTargetData*)RenderTargetStack.Back().pRenderTarget->GetRenderTargetData();
            if (plasthd && plasthd->pResolveTarget)
            {
                resolved = true;
                pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, 0, plasthd->pResolveTarget, 0, 0, 0, 0, 1.0f, 0, 0 );        
            }
        }
        SF_DEBUG_MESSAGE(!resolved && !resolveWarning, "Error resolving rendering target (resolve target wasn't available).");
        if (!resolved && !resolveWarning)
            resolveWarning = true;
    }

    RenderTargetData* phd = (X360::RenderTargetData*)prt->GetRenderTargetData();
    pDevice->SetRenderTarget(0, phd->pRenderSurface);
    if ( phd->pDepthStencilSurface )
        pDevice->SetDepthStencilSurface(phd->pDepthStencilSurface);

    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        pDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 1.0f,  0);
    }
    else
    {
        // Restore the resolve target at this level if no clear was requested.
        applyBlendMode(Blend_OverwriteAll, true, true);
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        DrawableCopyback(prt->GetTexture(), mvp, Matrix2F::Identity );
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
    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;

    // If the results of the render target need to be resolved, do that now.
    if (flags & PRT_Resolve)
    {
        X360::RenderTargetData* plasthd = (X360::RenderTargetData*)prt->GetRenderTargetData();
        SF_DEBUG_ASSERT(plasthd->pResolveTarget != 0, "Cannot have NULL resolve target when requesting resolve.");
        pDevice->Resolve(D3DRESOLVE_RENDERTARGET0, 0, plasthd->pResolveTarget, 0, 0, 0, 0, 1.0f, 0, 0 );        
    }

    // Note: we do not set the target to be finished here, as in most other platforms, because
    // X360 requires a prepass. When the filter is drawn in the final pass as a cached filter,
    // it will be released then.
    // entry.pRenderTarget->SetInUse(false);

    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        X360::RenderTargetData* plasthd = (X360::RenderTargetData*)prt->GetRenderTargetData();
        plasthd->pDepthStencilBuffer = 0;
        plasthd->pDepthStencilSurface = 0;
    }
    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();
    RenderTargetData* phd = 0;
    Render::Texture* poldTex = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        poldTex = back.pRenderTarget->GetTexture();
        phd = (X360::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Restore the old render target on the device, but not to the final target, if in prepass
    if ( CurrentPass != Display_Prepass || HALState & HS_InRenderTarget )
    {
        pDevice->SetRenderTarget(0, phd->pRenderSurface);
        if ( phd->pDepthStencilSurface )
            pDevice->SetDepthStencilSurface(phd->pDepthStencilSurface);
        else 
            pDevice->SetDepthStencilSurface(0);
        ++AccumulatedStats.RTChanges;

        if (flags & PRT_Resolve && poldTex)
        {
            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
            DrawableCopyback(poldTex, mvp, Matrix2F::Identity );
        }
    }

    // Reset the viewport to the last render target on the stack.
    HALState |= HS_ViewValid;
    updateViewport();
}

bool HAL::createDefaultRenderBuffer()
{
    if ( !GetDefaultRenderTarget() )
    {
        ImageSize rtSize;
        IDirect3DSurface9* prenderTarget = 0;
        IDirect3DSurface9* pdsTarget     = 0;
        D3DSURFACE_DESC rtDesc, dsDesc;
        if (FAILED(pDevice->GetRenderTarget(0, &prenderTarget)) ||             
            FAILED(prenderTarget->GetDesc(&rtDesc)) )
        {
            return false;
        }

        if ( FAILED(pDevice->GetDepthStencilSurface(&pdsTarget)) ||
             FAILED(pdsTarget->GetDesc(&dsDesc)))
        {
            pdsTarget = 0;
            memset(&dsDesc, 0, sizeof(D3DSURFACE_DESC));
        }

        rtSize = ImageSize(rtDesc.Width, rtDesc.Height);
        ImageSize dsSize = ImageSize(dsDesc.Width, dsDesc.Height);

        Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize );
        Ptr<DepthStencilBuffer> pdsb = 0;
        if ( pdsTarget )
            pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsSize);

        RenderTargetData::UpdateData(ptarget, prenderTarget, 0, pdsb, pdsTarget );

        if ( pdsTarget )
            pdsTarget->Release();
        if ( prenderTarget )
            prenderTarget->Release();

        if ( !SetRenderTarget(ptarget) )
            return false;
    }
    return true;
}

void HAL::PushFilters(FilterPrimitive* prim)
{
    GetEvent(Event_Filter).Begin(__FUNCTION__);
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

        setLinearStreamSource(PrimitiveBatch::DP_Batch);
        pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer, 0, sizeof(VertexXY16iAlpha));
        drawPrimitive(6, 1);
        FilterStack.PushBack(e);
        return;
    }

    // If the render target failed to allocate, don't draw items within the filter.
    if ( (HALState & HS_CachedFilter) ||
         (CurrentPass == Display_Prepass && FilterStack.GetSize() >0 && !FilterStack.Back().pRenderTarget ))
    {
        FilterStack.PushBack(e);
        return;
    }

    // If there are already filters on the stack, we need to resolve them, as we may overwrite their EDRAM.
    if ( CurrentPass == Display_Prepass && FilterStack.GetSize() > 0 )
    {
        pDevice->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARRENDERTARGET, 0, 
            ((RenderTargetData*)FilterStack.Back().pRenderTarget->GetRenderTargetData())->pResolveTarget, 0, 0, 0, 0, 1.0f, 0, 0 );        
    }

    HALState |= HS_DrawingFilter;

    // Accept all rendering commands from now on (so we can draw the object that will be filtered).
    GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);

    if ( prim->GetCacheState() ==  FilterPrimitive::Cache_Uncached && CurrentPass == Display_Prepass )
    {
        // Draw the filter from scratch.
        const Matrix2F& m = e.pPrimitive->GetFilterAreaMatrix().GetMatrix2D();
        e.pRenderTarget = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), prim->GetMaskPresent());
        if ( e.pRenderTarget )
        {
            RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());
            PushRenderTarget(frameRect, e.pRenderTarget);
            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, true);

            // Disable masking from previous target, if this filter primitive doesn't have any masking.
            if ( MaskStackTop != 0 && !prim->GetMaskPresent() )
            {
                if ( StencilAvailable )
                    pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
                else if ( DepthBufferAvailable )
                    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
                pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
            }            

            // If this primitive has masking, then clear the entire area to the current mask level, because 
            // the depth stencil target may be different, and thus does not contain the previously written values.
            if ( prim->GetMaskPresent())
            {
                if (StencilAvailable)
                    pDevice->Clear(0, 0, D3DCLEAR_STENCIL, 0, 0.0f, MaskStackTop);
                else if (DepthBufferAvailable)
                    pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0); // Z -> 1.0f
            }
        }
    }
    else if ( prim->GetCacheState() != FilterPrimitive::Cache_Uncached )
    {
        // Drawing a cached filter, ignore all draw calls until the corresponding PopFilters.
        // Keep track of the level at which we need to draw the cached filter, by adding entries to the stack.
        SF_ASSERT(prim->GetCacheState() == FilterPrimitive::Cache_Target);
        HALState |= HS_CachedFilter;
        CachedFilterIndex = FilterStack.GetSize();
        GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_Filters);
    }
    else
    {
        SF_DEBUG_WARNONCE(1, "Attempt to draw uncached filter on final pass.");
    }
    FilterStack.PushBack(e);
}

void HAL::PopFilters()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Filter), __FUNCTION__, false);

    FilterStackEntry e;
    e = FilterStack.Pop();

    if ( !Profiler.ShouldDrawMask() )
    {
        if ( FilterStack.GetSize() == 0 )
            Profiler.SetDrawMode(0);
        return;
    }

    // If doing a cached filter, and haven't reached the level at which it will be displayed, ignore the pop.
    if ( (HALState & HS_CachedFilter) && (CachedFilterIndex < (int)FilterStack.GetSize()) )
        return;

    CachedFilterIndex = -1;
    if ( HALState & HS_CachedFilter )
    {
        // Only actually draw the top-level cached filter on the final pass.
        if ( CurrentPass == Display_Final || FilterStack.GetSize() > 0 )
        {
            drawCachedFilter(e.pPrimitive);
            GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);
        }
        else
        {
            GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_Filters);
        }
        HALState &= ~HS_CachedFilter;
    }
    else
    {
        drawUncachedFilter(e);

        // If there are no filters left in the stack, then go back to only accepting filter calls.
        if ( FilterStack.GetSize() == 0 && CurrentPass == Display_Prepass )
            GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_Filters);
    }

    if ( FilterStack.GetSize() == 0 )
        HALState &= ~HS_DrawingFilter;
}

void HAL::drawUncachedFilter(const FilterStackEntry& e)
{
    // X360 requires filters to be done in a prepass. If we are not in the prepass,
    // it means that the filtered content failed caching in the prepass, and the content
    // should be rendered unfiltered.
    if (CurrentPass != Display_Prepass)
        return;

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
    temporaryTextures[Target_Source] = e.pRenderTarget;

    // Resolve the render target.
    pDevice->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARRENDERTARGET, 0, 
        ((RenderTargetData*)temporaryTextures[Target_Source]->GetRenderTargetData())->pResolveTarget, 
        0, 0, 0, 0, 1.0f, 0, 0 );

    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer, 0, sizeof(VertexXY16iAlpha));

    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
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
            // Create a destination texture if required.
            if ( !temporaryTextures[Target_Destination] )
            {
                temporaryTextures[Target_Destination] = *CreateTempRenderTarget(size, false);

                // Failed allocation. Try and quit gracefully.
                if ( !temporaryTextures[Target_Destination] )
                {
                    temporaryTextures[Target_Source]->SetInUse(false);
                    temporaryTextures[Target_Source] = 0;
                    i = filterCount;
                    break;
                }
            }

            IDirect3DSurface9* psurface = ((X360::Texture*)temporaryTextures[
                Target_Destination]->GetTexture())->pTextures[0].Surface;
            pDevice->SetRenderTarget(0, psurface);
            pDevice->ClearF(D3DCLEAR_TARGET, 0, 0, 1.0f, 0);
            ++AccumulatedStats.RTChanges;

            // Scale to the size of the destination.
            RenderTarget* prt = temporaryTextures[1];
            const Rect<int>& viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
            const ImageSize& bs = prt->GetBufferSize();
            VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());    
            ViewRect = Rect<int>(viewRect.x1, viewRect.y1, viewRect.x2, viewRect.y2);
            HALState |= HS_ViewValid;
            updateViewport();

            Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);                          
            mvp.Tx() -= 1.0f/size.Width;   // X360 1/2 pixel center offset
            mvp.Ty() += 1.0f/size.Height;

            SManager.SetFilterFill(mvp, Cxform::Identity, filter, temporaryTextures, shaders, pass, passes, 
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);
            drawPrimitive(6, 1);
            pTextureManager->SetSamplerState(0, 0);
            pTextureManager->SetSamplerState(1, 0);

            // Resolve the render target.
            IDirect3DTexture9* presolveTex = ((RenderTargetData*)temporaryTextures[Target_Destination]->GetRenderTargetData())->pResolveTarget;
            pDevice->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARRENDERTARGET, 0, presolveTex, 
                0, 0, 0, 0, 1.0f, 0, 0 );

            // If we require the original source, create a new target for the source.
            if ( requireSource && pass == 0 )
            {
                if ( passes > 1 )
                    temporaryTextures[Target_Source] = *CreateTempRenderTarget(size, false);
            }
            // On the last pass of each filter, release the original and source textures.
            if ( pass == passes -1)
            {
                if (temporaryTextures[Target_Original])
                {
                    temporaryTextures[Target_Original]->SetInUse(false);
                    temporaryTextures[Target_Original] = 0;
                }
                if (temporaryTextures[Target_Source])
                {
                    temporaryTextures[Target_Source]->SetInUse(false);
                    temporaryTextures[Target_Source] = 0;
                }
            }

            // Setup for the next pass.
            Alg::Swap(temporaryTextures[Target_Source], temporaryTextures[Target_Destination]);
        }
    }

    // Cache the last step, as it will be used to draw the filtered object in the final pass (and
    // potentially used on subsequent frames).
    if ( temporaryTextures[Target_Source] )
    {
        RenderTarget* cacheResults[1] = { temporaryTextures[Target_Source] };
        e.pPrimitive->SetCacheResults(FilterPrimitive::Cache_Target, cacheResults, 1);
        ((X360::RenderTargetData*)cacheResults[0]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
    }

    // Pop the temporary target.
    PopRenderTarget();

    // If this is not the top filter on the stack, we need to draw it into the stacked filtered target,
    // plus the content we previously resolved for the higher level stack.
    if ( FilterStack.GetSize() > 0 && temporaryTextures[Target_Source] )
    {
        // Previous content.
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        unsigned fillFlags = FillFlags;
        const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Single, 
            MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);

        RenderTarget* prt = FilterStack.Back().pRenderTarget;
        Texture* pt = (Texture*)prt->GetTexture();
        Rect<int> srect = FilterStack.Back().pRenderTarget->GetRect();
        
        Matrix2F texgen;
        texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
        texgen.AppendScaling((float)srect.Width() / pt->GetSize().Width, (float)srect.Height() / pt->GetSize().Height);

        ShaderData.SetTexture(pso, Uniform::SU_tex, pt, ImageFillMode());
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetMatrix(pso, Uniform::SU_texgen, texgen);
        ShaderData.Finish(1);

        pDevice->ClearF(D3DCLEAR_TARGET, 0, 0, 1.0f, 0 );
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
        drawPrimitive(6, 1);
        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget) != 0);

        // And then the cached filter.
        drawCachedFilter(e.pPrimitive);
    }

    if ( HALState & HS_DrawingMask )
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0 );

    // Cleanup. NOTE: We do not set the final target to not be in use, because we will require
    // it in the final pass rendering.
    for ( unsigned i = Target_Destination; i < MaxTemporaryTextures; ++i )
    {
        pDevice->SetTexture(i, 0);
        if (temporaryTextures[i])
            temporaryTextures[i]->SetInUse(false);
    }
    AccumulatedStats.Filters += filters->GetFilterCount();
}

void HAL::drawCachedFilter(FilterPrimitive* primitive)
{
    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer, 0, sizeof(VertexXY16iAlpha));

    switch(primitive->GetCacheState())
    {
        // X360 should never cache pre-target.
        case FilterPrimitive::Cache_Uncached:
        case FilterPrimitive::Cache_PreTarget:
        {
            SF_DEBUG_WARNING(1, "Filter was not cached. Drawing content without filtering. ");
            break;
        }

        // We have a final filtered texture. Just apply it to a screen quad.
        case FilterPrimitive::Cache_Target:
        {
            unsigned fillFlags = FillFlags | FF_Cxform | FF_AlphaWrite;
            const ShaderManager::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Single, 
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData);

            RenderTarget* results;
            primitive->GetCacheResults(&results, 1);
            Texture* ptexture = (X360::Texture*)results->GetTexture();
            const Matrix2F& mvp = Matrices->UserView * primitive->GetFilterAreaMatrix().GetMatrix2D();
            const Cxform& cx = primitive->GetFilterAreaMatrix().GetCxform();
            const Rect<int>& srect = results->GetRect();
            Matrix2F texgen;
            texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
            texgen.AppendScaling((float)srect.Width() / ptexture->GetSize().Width, (float)srect.Height() / ptexture->GetSize().Height);

            ShaderData.SetUniform(pso, Uniform::SU_mvp, &mvp.M[0][0], 8 );
            ShaderData.SetUniform(pso, Uniform::SU_texgen, &texgen.M[0][0], 8 );
            ShaderData.SetTexture(pso, Uniform::SU_tex, ptexture, ImageFillMode(Wrap_Clamp, Sample_Linear));
            ShaderData.SetCxform( pso, cx);
            ShaderData.Finish(1);

            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);
            drawPrimitive(6, 1);
            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget)!=0);
            pTextureManager->SetSamplerState(0, 0);
            pTextureManager->SetSamplerState(1, 0);

            // Cleanup. Keep texture around if possible; it could be used again.
            if ( Profiler.IsFilterCachingEnabled() )
                results->SetInUse(RTUse_Unused_Cacheable);
            else
                results->SetInUse(RTUse_Unused);
            break;
        }
    }
}


void HAL::setLinearStreamSource( PrimitiveBatch::BatchType type )
{
    if (PrevBatchType >= PrimitiveBatch::DP_Instanced)
    {
        pDevice->SetStreamSource(1, NULL, 0, 0);
    }
    PrevBatchType = type;
}

void HAL::setInstancedStreamSource( const ShaderManager::Shader& pso, MeshCacheItem * pmesh )
{
    pDevice->SetStreamSource(1, pmesh->GetVertexBuffer(), pmesh->IndexOffset, 4 );
    float instSize[4];
    for ( int i = 0; i < 4; i++ )
        instSize[i] = (float)pmesh->IndexCount;
    ShaderData.SetUniform(pso, Uniform::SU_instSize, instSize, 4);
}

void HAL::setInstancedStreamSource( const ShaderManager::Shader& pso, MeshCacheItem * pmesh, const ComplexMesh::FillRecord& fr )
{
    pDevice->SetStreamSource(1, pmesh->GetVertexBuffer(), 
        pmesh->IndexOffset + fr.IndexOffset * sizeof(IndexType), 4 );
    float instSize[4];
    for ( int i = 0; i < 4; i++ )
        instSize[i] = (float)fr.IndexCount;
    ShaderData.SetUniform(pso, Uniform::SU_instSize, instSize, 4);
}

void HAL::setBatchUnitSquareVertexStream()
{
    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer, 0, sizeof(VertexXY16iAlpha));
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (UINT)(indexCount/3));

#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UPInt indexOffset, unsigned vertexBaseIndex)
{
    pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, 0, (UINT)indexOffset, indexCount / 3 );

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced( unsigned indexCount, unsigned meshCount, unsigned vertexBaseIndex)
{
    pDevice->DrawVertices(D3DPT_TRIANGLELIST, vertexBaseIndex, indexCount * meshCount );

#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}


void HAL::DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, unsigned channelMask, const UInt32* values)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_DIPaletteMap), __FUNCTION__);

    // Create a temporary texture with the palette map. There may be a more efficient way to do this; however, using
    // uniforms seems unworkable, due to shader constant slot constraints.
    ImageData data;
    Ptr<Render::Texture> ptex = *pTextureManager->CreateTexture(Image_B8G8R8A8, 1, ImageSize(256, 4), ImageUse_Map_Mask, 0);
    if ( !ptex->Map(&data, 0, 1) )
        return;
    for ( int channel = 0; channel < 4; ++channel )
    {
        UInt32* dataPtr = (UInt32*)data.GetScanline(channel);
        if ( channelMask & (1<<channel))
        {
            Image_CopyScanline32_Swizzle<0,3,2,1>((UByte*)dataPtr, (UByte*)(values + channel*256), 256*sizeof(UInt32), 0, 0);
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
        ptex, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single], &ShaderData, ShaderManager::CPF_HalfPixelOffset );

    drawScreenQuad();
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
#if !defined(SF_BUILD_SHIPPING)
    static X360::RenderEvent Events[Event_Count];
    return Events[eventType];
#else
    // Shipping builds just return a default event, which does nothing.
    return Render::HAL::GetEvent(eventType);
#endif
}

void HAL::drawScreenQuad()
{
    // Set the vertices, and Draw
    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, &Cache.MaskEraseBatchVertexBuffer,
        0, sizeof(VertexXY16iAlpha));
	drawPrimitive(6,1);
}

}}} // Scaleform::Render::X360

