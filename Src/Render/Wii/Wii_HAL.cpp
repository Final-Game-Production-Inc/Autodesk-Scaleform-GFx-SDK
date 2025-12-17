/**************************************************************************

Filename    :   Wii_HAL.cpp
Content     :   Wii Renderer HAL Prototype implementation.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Wii_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Wii_HAL.h"
#include "Render/Wii/Wii_MemoryManager.h"
#include "Render/Render_FilterParams.h"
#include "Render/Render_Twips.h"
#include "Render/Render_BufferGeneric.h"

#include <revolution/vi.h>

namespace Scaleform { namespace Render { namespace Wii {



// ***** RenderHAL_Wii
static const float widescreenWidthAdjust = ( 640.0f / 832.0f );

HAL::HAL(ThreadCommandQueue* commandQueue)
	: Render::HAL(commandQueue),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::Console_Defaults),
    PrevBatchType(PrimitiveBatch::DP_None)
{
	// NCT_HACK 16:9
	float orthoWidth = 1.0f / widescreenWidthAdjust;
    ProjMatrix2D.OrthoOffCenterRH(-orthoWidth,orthoWidth,-1.0f,1.0f,   0.f, 255.f);
    ProjMatrix2D.M[2][2] = -1.f/255.f;
    ProjMatrix2D.M[2][3] = -1.f;

    pDummyDList = (UByte*)SF_MEMALIGN(96, 32, StatRender_Mem);
}

HAL::~HAL()
{
    ShutdownHAL();
}

bool HAL::InitHAL(const Wii::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    if (!params.pRMode)
        return false;

#ifdef SF_BUILD_DEBUG
    GXSetVerifyLevel(GX_WARN_MEDIUM);
#endif
    
    // Memory manager required on Wii!
    SF_ASSERT(params.pMemoryManager);

    // Allocate/Initialize texture manager
    if (params.pTextureManager)
        pTextureManager = params.GetTextureManager();
    else
    {
        pTextureManager = *SF_HEAP_AUTO_NEW(this) TextureManager(params.pMemoryManager, params.RenderThreadId,
            params.RenderThreadId != 0 ? pRTCommandQueue : 0);
        if (!pTextureManager)
        {
            SF_ASSERT(pTextureManager);
            return false;
        }
    }
    pRMode = params.pRMode;

    // Allocate our matrix state
    Matrices = *SF_HEAP_AUTO_NEW(this) MatrixState(this);

    // Create RenderBufferManager if needed.
    if (params.pRenderBufferManager)
        pRenderBufferManager = params.pRenderBufferManager;
    else
    {
        // Create the default render target manager.
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_EqualOrBigger);
        if ( !pRenderBufferManager || !createDefaultRenderBuffer())
        {
            ShutdownHAL();
            return false;
        }
    }

    SManager.Initialize(this, &Profiler);
    if (!Cache.Initialize(params.pMemoryManager, params.pRSync))
    {
        pTextureManager.Clear();
        pRMode = 0;
        return false;
    }

    HALState|= HS_ModeSet;
    notifyHandlers(HAL_Initialize);
    return true;
}

// Shutdown rendering.
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    destroyRenderBuffers();
    pTextureManager.Clear();
    Cache.Reset();
    pRMode = 0;
    return true;
}


    
// ***** Rendering

// Set states not changed in our rendering, or that are reset after changes
bool HAL::BeginScene()
{
    if ( !Render::HAL::BeginScene())
        return false;

    SManager.BeginScene();
    ProjMatrixSet = 0;

    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_BLUE,GX_CH_BLUE,  GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE,  GX_CH_BLUE,  GX_CH_BLUE,  GX_CH_BLUE);

    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    ZTest = ZWrite = 0;
    ZCompare = GX_ALWAYS;

    const GXColor BLACK = {0,0,0,0};
    const GXColor WHITE = {255,255,255,255};
    GXSetFog(GX_FOG_NONE, 0, 1, 0, 1, BLACK);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetAlphaCompare( GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetCullMode( GX_CULL_NONE );
//    GXSetCopyClear(BLACK, 0);

    GXLoadPosMtxImm(Matrix3F::Identity.M, GX_IDENTITY);

    // color
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, WHITE);
    GXSetChanAmbColor(GX_COLOR1A1, WHITE);
    GXSetChanMatColor(GX_COLOR0A0, WHITE);
    GXSetChanMatColor(GX_COLOR1A1, WHITE);

    // tex env
    GXSetNumTexGens(0);
    GXSetNumIndStages(0);

    for (int i = 0; i < 8; i++)
    {
        GXSetTevDirect((GXTevStageID)(GX_TEVSTAGE0 + i));
        GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXEnableTexOffsets((GXTexCoordID)(GX_TEXCOORD0 + i), GX_DISABLE, GX_DISABLE);
        GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0 + i), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

        GXSetTexCoordScaleManually((GXTexCoordID)(GX_TEXCOORD0 + i), GX_FALSE, 1, 1);
        GXSetTexCoordCylWrap((GXTexCoordID)(GX_TEXCOORD0 + i), GX_FALSE, GX_FALSE);
    }

    return true;
}

//--------------------------------------------------------------------
RenderTarget* HAL::CreateRenderTarget(RenderTargetParams params)
{
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(ImageSize(640,448), RBuffer_User, Image_R8G8B8A8);
    RenderTargetData::UpdateData(prt, params);
    return prt;
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    SF_UNUSED(needsStencil);
    Wii::Texture* pt = (Wii::Texture*)texture;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 

    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    RenderTargetParams rtParams;

    rtParams.FbMode = GX_PF_RGBA6_Z24;
    rtParams.FbZMode = GX_ZC_LINEAR;
    rtParams.pRMode = 0;

    RenderTargetData::UpdateData(prt, rtParams, pt);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;
    if ( prt->GetRenderTargetData() )
        return prt;

    Wii::Texture* pt = (Wii::Texture*)prt->GetTexture();

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 

    RenderTargetParams rtParams;
    rtParams.FbMode = GX_PF_RGBA6_Z24;
    rtParams.FbZMode = GX_ZC_LINEAR;
    rtParams.pRMode = 0;

    RenderTargetData::UpdateData(prt, rtParams, pt);
    return prt;
}

bool HAL::SetRenderTarget(RenderTarget* ptarget, bool setState)
{
#if 1
    if ( HALState & HS_InDisplay )
        return false;
    return true;
#else
    // When changing the render target while in a scene, we must flush all drawing.
    if ( HALState & HS_InScene)
        Flush();

    // Cannot set the bottom level render target if already in display.
    if ( HALState & HS_InDisplay )
        return false;

    RenderTargetEntry entry;
    RenderTargetData* prtdata = (Wii::RenderTargetData*)ptarget->GetRenderTargetData();

    if ( setState )
    {
        GXSetPixelFmt(prtdata->RT.FbMode, prtdata->RT.FbZMode);
        if (prtdata->RT.FbMode == GX_PF_RGBA6_Z24)
        {
            GXSetAlphaUpdate(GX_TRUE);
            GXSetDstAlpha(GX_DISABLE, 0);
        }
        else
            GXSetAlphaUpdate(GX_FALSE);
    }

    entry.pRenderTarget = ptarget;
    pRMode = prtdata->RT.pRMode;

    // Replace the stack entry at the bottom, or if the stack is empty, add one.
    if ( RenderTargetStack.GetSize() > 0 )
        RenderTargetStack[0] = entry;
    else
        RenderTargetStack.PushBack(entry);
    return true;
#endif    
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags)
{
#if 0
    // Wii requires render targets to be done in a prepass.
    SF_ASSERT(CurrentPass == Display_Prepass);

    HALState |= HS_InRenderTarget;
    RenderTargetEntry entry;
    entry.pRenderTarget = prt;
    entry.OldViewport = VP;
    entry.OldViewRect = ViewRect;
    entry.OldMatrixState.CopyFrom(Matrices);
    Matrices->SetUserMatrix(Matrix2F::Identity);

    if ( !prt )
    {
        RenderTargetStack.PushBack(entry);
        return;
    }

    RenderTargetData* phd = (Wii::RenderTargetData*)prt->GetRenderTargetData();

    pRMode = phd->RT.pRMode;

    GXSetPixelFmt(phd->RT.FbMode, phd->RT.FbZMode);
    if (phd->RT.FbMode == GX_PF_RGBA6_Z24)
    {
        GXSetAlphaUpdate(GX_TRUE);
        GXSetDstAlpha(GX_DISABLE, 0);
    }
    else
    {
        GXSetAlphaUpdate(GX_FALSE);
    }

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        clearBuffer(phd);
    }

    // Setup viewport.
    Rect<int> viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
    const ImageSize& bs = prt->GetBufferSize();
    VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height()); 
    if (phd->pTexture)
        VP.Flags |= Viewport::View_RenderTextureAlpha;
    ViewRect.x1 = (int)frameRect.x1;
    ViewRect.y1 = (int)frameRect.y1;
    ViewRect.x2 = (int)frameRect.x2;
    ViewRect.y2 = (int)frameRect.y2;

    // Must offset the 'original' viewrect, otherwise the 3D compensation matrix will be offset.
    Matrices->ViewRectOriginal.Offset(-entry.OldViewport.Left, -entry.OldViewport.Top);
    Matrices->UVPOChanged = true;

    HALState |= HS_ViewValid;
    updateViewport();
    applyBlendMode(BlendModeStack.GetSize() ? BlendModeStack.Back() : Blend_Normal);

    RenderTargetStack.PushBack(entry);
#endif    
}

void HAL::copyRTTexture(RenderTarget* prt)
{
/*
    RenderTargetData* phd = (Wii::RenderTargetData*)prt->GetRenderTargetData();
    Texture::HWTextureDesc& tdesc = phd->pTexture->pTextures[0];

    GXDrawDone();
    GXSetTexCopySrc(0, 0, tdesc.Width, tdesc.Height);
    GXSetTexCopyDst(tdesc.Width, tdesc.Height, GX_TF_RGBA8, GX_FALSE);
//    GXColor color = {0,0,0,0};
//    GXSetCopyClear(color, 0);
    GXCopyTex(tdesc.pTexData, GX_FALSE); // NCT_COMMENT GX_TRUE);
*/    
}

void HAL::clearBuffer(RenderTargetData* /*phd*/)
{
/*
    Texture::HWTextureDesc& tdesc = phd->pTexture->pTextures[0];

    GXSetViewport(0,0, tdesc.Width, tdesc.Height, 0, 1);
    GXSetScissor(0,0, tdesc.Width, tdesc.Height);

    GXSetCurrentMtx(GX_IDENTITY);

    //GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ZERO, GX_LO_COPY);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
    GXColor gc = {0,0,0,0};
    GXSetChanMatColor(GX_COLOR0A0, gc);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);

    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    GXBegin(GX_QUADS, GX_VTXFMT7, 4);
    GXPosition2f32(1, 1);
    GXPosition2f32(1, -1);
    GXPosition2f32(-1, -1);
    GXPosition2f32(-1, 1);
    GXEnd();

    GXSetCurrentMtx(0);
*/    
}

void HAL::PopRenderTarget(unsigned)
{
#if 0
    // Wii requires render targets to be done in a prepass.
    SF_ASSERT(CurrentPass == Display_Prepass);

    RenderTargetEntry& entry = RenderTargetStack.Back();

    // Copy framebuffer to texture
    RenderTarget* poldRT = entry.pRenderTarget;
    copyRTTexture(poldRT);

    RenderTargetParams OldRTFmt = ((Wii::RenderTargetData*)poldRT->GetRenderTargetData())->RT;

    entry.pRenderTarget->SetInUse(false);
    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();
    RenderTargetEntry& back = RenderTargetStack.Back();
    RenderTargetData* phd = (Wii::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();

    {
        GXPixModeSync();
        GXInvalidateTexAll();

        GXSetPixelFmt(phd->RT.FbMode, phd->RT.FbZMode);
        if (phd->RT.FbMode == GX_PF_RGBA6_Z24)
        {
            GXSetAlphaUpdate(GX_TRUE);
            GXSetDstAlpha(GX_DISABLE, 0);
        }
        else
            GXSetAlphaUpdate(GX_FALSE);
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    ++AccumulatedStats.RTChanges;

    // Reset the viewport to the last render target on the stack.
    HALState |= HS_ViewValid;
    updateViewport();
    applyBlendMode(BlendModeStack.GetSize() ? BlendModeStack.Back() : Blend_Normal);
#endif
}

bool HAL::createDefaultRenderBuffer()
{
    ImageSize rtSize(1640, 1480);
    if ( GetDefaultRenderTarget() )
    {
        RenderTarget* prt = GetDefaultRenderTarget();
        rtSize = prt->GetSize();
    }
    return pRenderBufferManager->Initialize(pTextureManager, Image_B8G8R8A8, rtSize);
}

// Updates HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    Viewport vp;
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

        if (HALState & HS_InRenderTarget)
        {
            vp = VP;
        }
        else
        {
            vp.Left     = ViewRect.x1;
            vp.Top      = ViewRect.y1;
            vp.Width    = ViewRect.Width();
            vp.Height   = ViewRect.Height();
        }

        if (pRMode)
        {
            if (pRMode->field_rendering && 0 == (VP.Flags & Viewport::View_IsRenderTexture))
                GXSetViewportJitter(vp.Left, vp.Top, vp.Width, vp.Height, 0, 1, VIGetNextField());
            else
                GXSetViewport(vp.Left, vp.Top, vp.Width, vp.Height, 0, 1);

            GXSetCopyFilter(pRMode->aa, pRMode->sample_pattern, GX_TRUE, pRMode->vfilter);
        }
        else
        {
            GXSetViewport(vp.Left, vp.Top, vp.Width, vp.Height, 0, 1);
            GXSetCopyFilter(GX_FALSE, 0, GX_FALSE, 0);
        }

        GXSetScissor(vp.Left, vp.Top, vp.Width, vp.Height);
        if (ProjMatrixSet != 2)
            ProjMatrixSet = 0;
    }
    else
    {
        GXSetScissor(0,0,0,0);
    } 
}

void HAL::SetUserMatrix(const Matrix& userMatrix)
{
}

void MatrixState::recalculateUVPOC() const
{
    if (UVPOChanged)
    {
        // Recalculated the view compensation matrix.
        if ( ViewRect != ViewRectOriginal && !ViewRectOriginal.IsNull())
        {
            Point<int> dc = ViewRect.Center() - ViewRectOriginal.Center();
            float      dx = ((float)ViewRectOriginal.Width()) / ViewRect.Width();
            float      dy = ((float)ViewRectOriginal.Height()) / ViewRect.Height();
            float      ox = 2.0f * dc.x / ViewRect.Width();
            float      oy = 2.0f * dc.y / ViewRect.Height();
            ViewRectCompensated3D.MultiplyMatrix(Matrix4F::Translation(-ox, oy, 0), Matrix4F::Scaling(dx, dy, 1));
        }
        else
        {
            ViewRectCompensated3D = Matrix4F::Identity;
        }

        // UPO = User3D * Orient3D * ViewRectCompensated3D * Proj3D;
        Matrix4F UO(User3D, Orient3D);
        updateStereoProjection();
        Matrix4F VRP(ViewRectCompensated3D, Proj3D);
        UVPO = Matrix4F(UO, VRP);
        UVPOChanged = 0;
    }  
}

PrimitiveFill*  HAL::CreatePrimitiveFill(const PrimitiveFillData &data)
{
    return SF_HEAP_NEW(pHeap) PrimitiveFill(data);
}

void   HAL::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single,
                            const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType)
{
    VertexElement        outElements[8];
    VertexFormat         outFormat;
    outFormat.Size = sourceFormat->Size;
    outFormat.pElements = outElements;

    const VertexElement* pve = sourceFormat->pElements;
    VertexElement* pveo = outElements;
    for (; pve->Attribute != VET_None; pve++, pveo++)
    {
        *pveo = *pve;
        if ((pve->Attribute & (VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask)) == VET_ColorARGB8)
            pveo->Attribute = VET_ColorRGBA8 | (pve->Attribute & ~(VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask));
    }

    return SManager.MapVertexFormat(fill, &outFormat, single, batch, instanced, meshType);
}

// Draws a range of pre-cached and preprocessed primitives
void        HAL::DrawProcessedPrimitive(Primitive* pprimitive,
                                        PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    SF_ASSERT(pend != 0);

    if (!pprimitive->GetMeshCount())
        return;

    // If in overdraw profile mode, and this primitive is part of a mask, draw it in color mode.
    static bool drawingMask = false;
    if ( !Profiler.ShouldDrawMask() && !drawingMask && (HALState & HS_DrawingMask) )
    {
        drawingMask = true;
        GXSetColorUpdate(GX_TRUE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        DrawProcessedPrimitive(pprimitive, pstart, pend);
        GXSetColorUpdate(GX_FALSE);
        GXSetZMode(ZTest, ZCompare, ZWrite);
        drawingMask = false;
    }

    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    SManager.BeginPrimitive();

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

            unsigned fillFlags = FillFlags;
            if (pprimitive->GetMeshCount() && pprimitive->Meshes[0].M.Has3D())
                fillFlags |= FF_3DProjection;

            Shader pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, pbatch->pFormat, batchMeshCount, Matrices,
                                          &pprimitive->Meshes[meshIndex]);

            if ((fillFlags & FF_Cxform) && batchMeshCount > 1)
            {
                // Streaming
                unsigned  vcount = SManager.SetVertexArray(GX_VTXFMT7, pbatch->pFormat, (UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset);
                UByte*    pIndexData = (UByte*)(pmesh->pBuffer->pData) + pmesh->DListOffset + pmesh->PrimOffset + pShader.NumMatrices;
                unsigned  indexSize = (vcount >> 16) + (vcount & 0xffff) * 2;

                GXBeginDisplayList(pDummyDList, 96);
                GXEndDisplayList();

                UPInt indexOffset = 0;
                for (int i = 0; i < batchMeshCount; i++)
                {
                    MeshBase* prealmesh = pmesh->pMeshes[i];
                    SManager.SetCxform(pShader, Profiler.GetCxform(pprimitive->Meshes[meshIndex+i].M.GetCxform()));

                    SManager.SetStreamedBatch(pShader, i);

                    //GXBegin(GX_TRIANGLES, GX_VTXFMT7, prealmesh->IndexCount);
                    GXColor1x8(GX_DRAW_TRIANGLES | GX_VTXFMT7);
                    GXColor1x16(prealmesh->IndexCount);

                    for (unsigned j = 0; j < prealmesh->IndexCount; j++, pIndexData += indexSize + pShader.NumMatrices)
                    {
                        switch (indexSize)
                        {
                        case 8: GXWGFifo.u64 = *((UInt64*)pIndexData); break;
                        case 7: GXColor1u32(*(UInt32*)pIndexData); GXColor1u16(*(UInt16*)(pIndexData+4)); GXColor1x8(*(pIndexData+6)); break;
                        case 6: GXColor1u32(*(UInt32*)pIndexData); GXColor1u16(*(UInt16*)(pIndexData+4)); break;
                        case 5: GXColor1u32(*(UInt32*)pIndexData); GXColor1x8(*(pIndexData+4)); break;
                        case 4: GXColor1u32(*(UInt32*)pIndexData); break;
                        case 3: GXColor1u16(*(UInt16*)pIndexData); GXColor1x8(*(pIndexData+2)); break;
                        case 2: GXColor1u16(*(UInt16*)pIndexData); break;
                        default: SF_ASSERT(0);
                        }
                    }
                    //GXEnd();

                    AccumulatedStats.Primitives++;
                    AccumulatedStats.Meshes++;

                    indexOffset += prealmesh->IndexCount;
                }

                AccumulatedStats.Triangles += pmesh->IndexCount / 3;
            }
            else if (batchMeshCount > 0)
            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                    (pbatch->Type != PrimitiveBatch::DP_Virtual));

                SManager.SetCxform(pShader, Profiler.GetCxform(pprimitive->Meshes[meshIndex].M.GetCxform()));
                GXCallDisplayList((UByte*)pmesh->pBuffer->pData + pmesh->DListOffset, pmesh->DListSize);

                AccumulatedStats.Meshes += pmesh->MeshCount;
                AccumulatedStats.Triangles += pmesh->IndexCount / 3;
                AccumulatedStats.Primitives++;
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
        GXSetColorUpdate(GX_TRUE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

        DrawProcessedComplexMeshes(complexMesh, matrices);

        GXSetColorUpdate(GX_FALSE);
        GXSetZMode(ZTest, ZCompare, ZWrite);
        drawingMask = false;
    }

    MeshCacheItem* pmesh = (MeshCacheItem*)complexMesh->GetCacheItem();
    if (!pmesh)
        return;

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();

    UByte* pvertexData = (UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset;
    UInt16* pIndexData = (UInt16*) ((UByte*)pmesh->pBuffer->pData + pmesh->IndexOffset);

    UInt32 fillFlags = FillFlags | (matrices[0].Has3D() ? FF_3DProjection : 0);

    for (unsigned i = 0; i < instanceCount; i++)
    {
        const HMatrix& hm = matrices[i];
        if (!(Profiler.GetCxform(hm.GetCxform()) == Cxform::Identity))
            fillFlags |= FF_Cxform;
    }

    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        SManager.BeginPrimitive();

        UByte textureCount = fr.pFill->GetTextureCount();

        // Apply fill.
        PrimitiveFillType fillType = Profiler.GetFillType(fr.pFill->GetType());
        Shader pso = SManager.SetFill(fr.pFill, fillType, fillFlags, batchType, fr.pFormats[0]);

        unsigned  vcount = SManager.SetVertexArray(GX_VTXFMT7, fr.pFormats[0], pvertexData + fr.VertexByteOffset);
        SF_ASSERT((vcount >> 16) == 0);
        SF_ASSERT(vcount <= 4 && vcount >= 1);

        //SManager.SetStreamedBatch(pso, 0);
        GXSetCurrentMtx(0);
        SF_ASSERT(((const SysVertexFormat*)fr.pFormats[0]->pSysFormat.GetPtr())->IsComplexMesh);

        bool solid = (fillType == PrimFill_None || fillType == PrimFill_Mask || fillType == PrimFill_SolidColor);

        for (unsigned i = 0; i < instanceCount; i++)
        {            
            const HMatrix& hm = matrices[i];

            SManager.SetMatrix(pso, Uniform::SU_mvp, complexMesh->GetVertexMatrix(), hm, Matrices);
            if (solid)
                SManager.SetColor(pso, Profiler.GetColor(fr.pFill->GetSolidColor()));
            else
                SManager.SetCxform(pso, Profiler.GetCxform(hm.GetCxform()));

            for (unsigned tm = 0, stage = 0; tm < textureCount; tm++)
            {
                SManager.SetMatrix(pso, Uniform::SU_texgen, textureMatrices[fr.FillMatrixIndex[tm]], tm);
                Texture* ptex = (Texture*)fr.pFill->GetTexture(tm);
                SManager.SetTexture(pso, stage, ptex, fr.pFill->GetFillMode(tm));
                stage += ptex->GetTextureStageCount();
            }

            SManager.Finish(1);

            GXBegin(GX_TRIANGLES, GX_VTXFMT7, fr.IndexCount);

            UInt16* pFillIndexData = pIndexData + fr.IndexOffset;
            for (unsigned j = 0; j < fr.IndexCount; j++)
            {
                UInt16 index = pFillIndexData[j];

                switch (vcount)
                {
                case 4: GXPosition1x16(index);
                case 3: GXPosition1x16(index);
                case 2: GXPosition1x16(index);
                case 1: GXPosition1x16(index);
                }
            }
            GXEnd();
        }

        AccumulatedStats.Triangles += (fr.IndexCount / 3) * instanceCount;
        AccumulatedStats.Meshes += instanceCount;
        AccumulatedStats.Primitives++;

    } // for (fill record)

    pmesh->GPUFence = Cache.GetRenderSync()->InsertFence();
    pmesh->MoveToCacheListFront(MCL_ThisFrame);   
}

//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color)
{
/*
    color = Profiler.GetClearColor(color);

    GXLoadPosMtxImm(Matrix3F::Identity.M, GX_IDENTITY);
    GXSetCurrentMtx(GX_IDENTITY);

//    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);

    GXColor gc = {color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha()};
    GXSetChanMatColor(GX_COLOR0A0, gc);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);

    if (ProjMatrixSet != 2)
    {
        ProjMatrixSet = 2;
        GXSetProjection(ProjMatrix2D.M, GX_ORTHOGRAPHIC);
    }

    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    float x1 = r.x1 / (VP.Width*0.5f) - 1.0f;
    float x2 = r.x2 / (VP.Width*0.5f) - 1.0f;
    float y1 = r.y1 / (VP.Height*0.5f) - 1.0f;
    float y2 = r.y2 / (VP.Height*0.5f) - 1.0f;

    GXBegin(GX_QUADS, GX_VTXFMT7, 4);
    GXPosition2f32(x2, y2);
    GXPosition2f32(x2, y1);
    GXPosition2f32(x1, y1);
    GXPosition2f32(x1, y2);
    GXEnd();

    applyBlendMode(BlendModeStack.GetSize() ? BlendModeStack.Back() : Blend_Normal, false, false);
*/    
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
/*
    Profiler.SetDrawMode(1);

    // disable framebuffer writes
    GXSetColorUpdate(GX_DISABLE);
    GXSetAlphaUpdate(GX_DISABLE);

    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

        MaskPrimitive* erasePrim = MaskStack[MaskStackTop].pPrimitive;
        drawMaskClearRectangles(erasePrim->GetMaskAreaMatrices(), erasePrim->GetMaskCount());
    }

    MaskStack.Resize(MaskStackTop+1);
    MaskStackEntry &e = MaskStack[MaskStackTop];
    e.pPrimitive       = prim;
    e.OldViewportValid = viewportValid;
    e.OldViewRect      = ViewRect; // TBD: Must assign

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
        if ((MaskStackTop == 0) && viewportValid)
        {
            GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

            drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
        }
    }

    else if ((MaskStackTop == 0) && viewportValid)
    {
        GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

        drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
    }

    MaskStackTop++;

    GXSetZMode(GX_TRUE, GX_GREATER, GX_TRUE);
    ZTest = ZWrite = 1;
    ZCompare = GX_GREATER;
    ++AccumulatedStats.Masks;
*/    
}


void HAL::EndMaskSubmit()
{
/*
    Profiler.SetDrawMode(0);

    if (!checkState(HS_DrawingMask, __FUNCTION__))
        return;
    HALState &= ~HS_DrawingMask;
    SF_ASSERT(MaskStackTop);

    // draw only where primitive z matches mask z
    GXSetZMode(GX_TRUE, GX_EQUAL, GX_FALSE);
    ZTest = 1; ZWrite = 0;
    ZCompare = GX_EQUAL;

    GXSetColorUpdate(GX_ENABLE);
    if ((VP.Flags & Viewport::View_AlphaComposite))
        GXSetAlphaUpdate(GX_ENABLE);
*/        
}


void HAL::PopMask()
{
/*
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

    // Disable mask or decrement stencil reference value.
    if (MaskStackTop == 0)
    {
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        ZTest = ZWrite = 0;
        ZCompare = GX_ALWAYS;
    }
*/    
}

static VertexElement VertexXY16I_E[] =
{
    { 0, VET_XY16i },
    { 0, VET_None }
};
static VertexFormat VertexXY16I = { sizeof(VertexXY16iInstance), VertexXY16I_E };

void HAL::drawMaskClearRectangles(const HMatrix* matrices, UPInt count)
{
/*
    // This operation is used to clear bounds for masks.
    // Potential issue: Since our bounds are exact, right/bottom pixels may not
    // be drawn due to HW fill rules.
    //  - This shouldn't matter if a mask is tessellated within bounds of those
    //    coordinates, since same rules are applied to those render shapes.
    //  - EdgeAA must be turned off for masks, as that would extrude the bounds.

    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
    //GXColor gc = {0,192,0,255};
    //GXSetChanMatColor(GX_COLOR0A0, gc);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
        GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    // using Pos+Matrix doesn't work
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);

    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetCurrentMtx(0);

    SManager.BeginPrimitive();

    for (UPInt i = 0; i < count; i++)
    {
        SManager.SetMatrix(SManager.CurShader, Uniform::SU_mvp, Matrix2F::Identity, matrices[i], Matrices);
        SManager.Finish(1);

        GXBegin(GX_QUADS, GX_VTXFMT7, 4);
        GXPosition2f32(0, 0);
        GXPosition2f32(1, 0);
        GXPosition2f32(1, 1);
        GXPosition2f32(0, 1);
        GXEnd();

        AccumulatedStats.Meshes += 1;
        AccumulatedStats.Triangles += 2;
        AccumulatedStats.Primitives++;
    }

    applyBlendMode(BlendModeStack.GetSize() ? BlendModeStack.Back() : Blend_Normal);
*/    
}



//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

// Structure describing color combines applied for a given blend mode.
struct BlendModeDesc
{
    GXBlendMode     BlendOp;
    GXBlendFactor   SrcFactor, DestFactor;
    GXLogicOp       BlendLogicalOp;
};

void    HAL::applyBlendModeImpl(BlendMode mode, bool forceAc, bool sourceAc)
{
    static BlendModeDesc blendModes[] =
    {
        //{ GX_BM_BLEND,    GX_BL_ONE,          GX_BL_ZERO,         GX_LO_COPY }, // Blend_None
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Blend_Normal
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Blend_Normal
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Blend_Layer

        { GX_BM_BLEND,      GX_BL_DSTCLR,       GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Blend_Multiply

        { GX_BM_NONE,       GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Screen *??

        { GX_BM_NONE,       GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Lighten *??
        { GX_BM_NONE,       GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Darken *??

        { GX_BM_NONE,       GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Difference *??

        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_ONE,          GX_LO_COPY }, // Add
        { GX_BM_SUBTRACT,   GX_BL_ONE,          GX_BL_ONE,          GX_LO_COPY }, // Subtract

        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Invert *??

        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Alpha *??
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Erase *?? to what color?
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Overlay *??
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // HardLight *??

        // add one more mode for when we do not want blending
        { GX_BM_BLEND,      GX_BL_SRCALPHA,     GX_BL_INVSRCALPHA,  GX_LO_COPY }, // Blend_Normal
//        { GX_BM_NONE,   GX_BL_ONE,  GX_BL_ZERO,     GX_LO_COPY  },  // no blending
    };

    BlendModeDesc *m = &blendModes[(unsigned)mode];
    if ((VP.Flags & Viewport::View_AlphaComposite) || forceAc)
    {
        if (m->SrcFactor == GX_BL_SRCALPHA)
            GXSetBlendMode(m->BlendOp, GX_BL_ONE, m->DestFactor, m->BlendLogicalOp);
        else
            GXSetBlendMode(m->BlendOp, m->SrcFactor, m->DestFactor, m->BlendLogicalOp);

        if (!(HALState&HS_DrawingMask))
            GXSetAlphaUpdate(mode == Blend_Add ? GX_FALSE : GX_TRUE);
    }
    else if (sourceAc && m->SrcFactor == GX_BL_SRCALPHA)
        GXSetBlendMode(m->BlendOp, GX_BL_ONE, m->DestFactor, m->BlendLogicalOp);
    else
        GXSetBlendMode(m->BlendOp, m->SrcFactor, m->DestFactor, m->BlendLogicalOp);

    if ( mode == Blend_Multiply || mode == Blend_Darken || ((VP.Flags & Viewport::View_AlphaComposite) || forceAc) & !sourceAc)
        FillFlags |= FF_Multiply;
    else
        FillFlags &= ~FF_Multiply;
}


void HAL::PushFilters(FilterPrimitive* prim)
{
/*
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    FilterStackEntry e = {prim, 0};

    // Query the profiler off of whether masks should be draw or not.
    if ( !Profiler.ShouldDrawMask() )
    {
        Profiler.SetDrawMode(2);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);

        GXSetNumTevStages(1);
        GXSetNumTexGens(0);
        GXSetNumChans(1);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetCurrentMtx(0);

        SManager.BeginPrimitive();

        Cxform cx = Profiler.GetCxform(Cxform::Identity);
        Color c = Color(UByte(cx.M[1][0] * 1./255.f), UByte(cx.M[1][1] * 1./255.f), UByte(cx.M[1][2] * 1./255.f), UByte(cx.M[1][3] * 1./255.f));
        SManager.SetMatrix(SManager.CurShader, Uniform::SU_mvp, Matrix2F::Identity, prim->GetFilterAreaMatrix(), Matrices);
        SManager.SetColor(SManager.CurShader, c);
        SManager.Finish(1);

        GXBegin(GX_QUADS, GX_VTXFMT7, 4);
        GXPosition2f32(0, 0);
        GXPosition2f32(1, 0);
        GXPosition2f32(1, 1);
        GXPosition2f32(0, 1);
        GXEnd();

        AccumulatedStats.Meshes += 1;
        AccumulatedStats.Triangles += 2;
        AccumulatedStats.Primitives++;

        FilterStack.PushBack(e);
        return;
    }

    if ( (HALState & HS_CachedFilter) )
    {
        FilterStack.PushBack(e);
        return;
    }

    // If there are already filters on the stack, we need to resolve them
    if ( CurrentPass == Display_Prepass && FilterStack.GetSize() > 0 )
    {
        copyRTTexture(FilterStack.Back().pRenderTarget);
    }

    HALState |= HS_DrawingFilter;

    // Accept all rendering commands from now on (so we can draw the object that will be filtered).
    GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);

    if ( prim->GetCacheState() ==  FilterPrimitive::Cache_Uncached && CurrentPass == Display_Prepass )
    {
        // Draw the filter from scratch.
        const Matrix2F& m = e.pPrimitive->GetFilterAreaMatrix().GetMatrix2D();
        bool requiresStencil = false; // TODO: how to determine if stenciling is required - need this from the tree?
        e.pRenderTarget = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), requiresStencil);
        RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());
        PushRenderTarget(frameRect, e.pRenderTarget);
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
*/    
}

void HAL::PopFilters()
{
/*
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

    SManager.BeginPrimitive();
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
    {
        HALState &= ~HS_DrawingFilter;
    }
*/    
}

void HAL::drawUncachedFilter(const FilterStackEntry& e)
{
/*
    if (CurrentPass != Display_Prepass)
        return;

    const FilterSet* filters = e.pPrimitive->GetFilters();
    unsigned filterCount = filters->GetFilterCount();
    unsigned pass = 0;

    // Invalid primitive or rendertarget.
    if ( !e.pPrimitive || !e.pRenderTarget )
        return;

    if (ProjMatrixSet != 2)
    {
        ProjMatrixSet = 2;
        GXSetProjection(ProjMatrix2D.M, GX_ORTHOGRAPHIC);
    }

    SF_ASSERT(RenderTargetStack.Back().pRenderTarget == e.pRenderTarget);
    const int MaxTemporaryTextures = 3;
    Ptr<RenderTarget> temporaryTextures[MaxTemporaryTextures]; // In each pass: 0=Src 1=Dest 2=OriginalSrc(optional)
    memset(temporaryTextures, 0, sizeof temporaryTextures);

    ImageSize size = e.pRenderTarget->GetSize();
    temporaryTextures[0] = e.pRenderTarget;

    // e.pRenderTarget is still on the stack.
    RenderTargetParams OldRTFmt = ((Wii::RenderTargetData*)e.pRenderTarget->GetRenderTargetData())->RT;

    if (filterCount)
    {
        copyRTTexture(e.pRenderTarget);
        GXPixModeSync();
        GXInvalidateTexAll();
    }

    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX2, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX3, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX4, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX5, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX6, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX7, GX_TEX_ST, GX_F32, 0);
    GXSetCurrentMtx(GX_IDENTITY);

    // Render filter(s).
    for ( unsigned filteri = 0; filteri < filterCount; ++filteri )
    {
        const Filter* filter = filters->GetFilter(filteri);
        int filterType = filter->GetFilterType();

        Rect<int> srcrect = temporaryTextures[0]->GetRect();
        Rect<int> destrect = Rect<int>(-1,1,1,-1);

        temporaryTextures[1] = *CreateTempRenderTarget(ImageSize(srcrect.Width(), srcrect.Height()), false);
        if (OldRTFmt.FbMode != GX_PF_RGBA6_Z24)
        {
            GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
        }

        if (filterType >= Filter_Blur && filterType <= Filter_Blur_End)
        {
            BlurFilter* bfilter = (BlurFilter*)filter;
            const BlurFilterParams& params = bfilter->GetParams();

            bool hasCxform = false;
            bool requireSource = (filterType != Filter_Blur);
            if ( requireSource )
                temporaryTextures[Target_Original] = temporaryTextures[Target_Source];

            unsigned n = params.Passes;
            BlurFilterParams pass[3];

            pass[0] = params;
            if ((params.Mode & BlurFilterParams::Mode_FilterMask) == Filter_Glow)
                pass[0].Mode = (pass[0].Mode & BlurFilterParams::Mode_FlagsMask) | Filter_Shadow;
            pass[0].BlurX = TwipsToPixels(params.BlurX);
            pass[0].BlurY = TwipsToPixels(params.BlurY);
            pass[0].Offset.x = TwipsToPixels(params.Offset.x);
            pass[0].Offset.y = TwipsToPixels(params.Offset.y);
            pass[1] = pass[0];
            pass[2] = pass[0];

            if (pass[0].BlurX * pass[0].BlurY > 8)
            {
                n *= 2;
                pass[0].Mode &= BlurFilterParams::Mode_FlagsMask;
                pass[0].Mode |= Filter_Blur;
                pass[1].Mode &= BlurFilterParams::Mode_FlagsMask;
                pass[1].Mode |= Filter_Blur;
                pass[0].BlurY = 1;
                pass[1].BlurX = 1;
                pass[2].BlurX = 1;
            }
            else if ((params.Mode & BlurFilterParams::Mode_FilterMask) != Filter_Blur)
            {
                pass[0].Mode &= BlurFilterParams::Mode_FlagsMask;
                pass[0].Mode |= Filter_Blur;
                pass[1].Mode &= BlurFilterParams::Mode_FlagsMask;
                pass[1].Mode |= Filter_Blur;
            }

            bool   Prims = 0;

            if (pass[2].BlurX*pass[2].BlurY > (requireSource ? 7 : 8))
            {
                n++;
                Prims = 1;
                pass[2].BlurX = pass[2].BlurY = 1;
            }
            pass[0].Offset.x = pass[0].Offset.y = pass[1].Offset.x = pass[1].Offset.y = 0;

            GXSetNumChans(0);

            // Render each pass
            for (unsigned i = 0; i < n; i++)
            {
                unsigned passi = (i == n-1) ? 2 : (i&1);
                const BlurFilterParams& pparams = pass[passi];

                // RT/Viewport setup
                RenderTarget* prt = temporaryTextures[1];
                RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
                RenderTargetData* phdSrc = (RenderTargetData*)temporaryTextures[0]->GetRenderTargetData();

                const Rect<int>& viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
                const ImageSize& bs = prt->GetBufferSize();
                VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height(),
                              Viewport::View_RenderTextureAlpha);    
                ViewRect = Rect<int>(viewRect.x1, viewRect.y1, viewRect.x2, viewRect.y2);
                HALState |= HS_ViewValid;
                updateViewport();

                srcrect = temporaryTextures[0]->GetRect();

                float SizeX = unsigned(pparams.BlurX-1) * 0.5f;
                float SizeY = unsigned(pparams.BlurY-1) * 0.5f;
                unsigned  np = unsigned((SizeX*2+1)*(SizeY*2+1));
                unsigned  count = (np + 7) >> 3;
                bool  scale4 = (np > 12);
                UByte scale = scale4 ? UByte(1020.0f/np) : (np == 1 ? 255 : UByte(510.0f/np));
                unsigned  ntex = (np > 8) ? 8 : np;
                unsigned  ntex2 = 0;
                unsigned  ntev = ntex;

                GXLoadTexObj(&phdSrc->pTexture->pTextures[0].Tex, GX_TEXMAP0);
                if (count > 1)
                    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_COPY);
                else
                    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_COPY);

                GXColor scalec;
                scalec.r = scalec.g = scalec.b = scalec.a = scale; 
                GXSetTevColor(GX_TEVREG0, scalec);

                GXClearVtxDesc();
                GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

                for (unsigned i = 0; i < ntex; i++)
                {
                    GXTevScale    scale = (i == ntex-1 && np > 1) ? GX_CS_DIVIDE_2 : GX_CS_SCALE_1;

                    GXSetVtxDesc((GXAttr)(GX_VA_TEX0 + i), GX_DIRECT);

                    GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0 + i), GX_TG_MTX2x4, (GXTexGenSrc)(GX_TG_TEX0 + i), GX_IDENTITY);
                    GXSetTexCoordScaleManually((GXTexCoordID)(GX_TEXCOORD0 + i), GX_TRUE, 1, 1);
                    GXSetTexCoordBias((GXTexCoordID)(GX_TEXCOORD0 + i), GX_FALSE, GX_FALSE);

                    GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + i), (GXTexCoordID)(GX_TEXCOORD0 + i), GX_TEXMAP0, GX_COLOR_NULL);
                    GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + i), GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, i ? GX_CC_CPREV : GX_CC_ZERO);
                    GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + i), GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, i ? GX_CA_APREV : GX_CA_ZERO);
                    GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEV_ADD, GX_TB_ZERO, scale, GX_FALSE, GX_TEVPREV);
                    GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEV_ADD, GX_TB_ZERO, scale, GX_FALSE, GX_TEVPREV);
                }
                if (scale4)
                {
                    GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
                    GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
                    GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                    GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_FALSE, GX_TEVPREV);
                    GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_FALSE, GX_TEVPREV);
                    ntev++;
                }

                if ((pparams.Mode & BlurFilterParams::Mode_FilterMask) == Filter_Shadow)
                {
                    // original source
                    RenderTarget* prtOrig = temporaryTextures[2] ? temporaryTextures[2] : temporaryTextures[0];
                    RenderTargetData* phdOrig = (RenderTargetData*)prtOrig->GetRenderTargetData();

                    unsigned basetc = ntex;
                    GXSetVtxDesc((GXAttr)(GX_VA_TEX0 + basetc), GX_DIRECT);
                    GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TG_MTX2x4, (GXTexGenSrc)(GX_TG_TEX0 + ntex), GX_IDENTITY);
                    GXSetTexCoordScaleManually((GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TRUE, 1, 1);
                    GXSetTexCoordBias((GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_FALSE, GX_FALSE);
                    GXLoadTexObj(&phdOrig->pTexture->pTextures[0].Tex, GX_TEXMAP1);
                    ntex2++;

                    GXColor scolor = {pparams.Colors[0].GetRed(), pparams.Colors[0].GetGreen(), pparams.Colors[0].GetBlue(), pparams.Colors[0].GetAlpha()};
                    GXSetTevColor(GX_TEVREG1, scolor);

                    if (pparams.Mode & BlurFilterParams::Mode_Inner)
                    {
                        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), (GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TEXMAP1, GX_COLOR_NULL);
                        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_C1, GX_CC_TEXC, GX_CC_APREV, GX_CC_ZERO);
                        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_A1, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
                        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        ntev++;

                        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), (GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TEXMAP1, GX_COLOR_NULL);
                        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV, GX_CC_ZERO);
                        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
                        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        ntev++;
                    }
                    else
                    {
                        // scolor * color.a
                        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
                        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_ZERO, GX_CC_C1, GX_CC_APREV, GX_CC_ZERO);
                        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_ZERO, GX_CA_A1, GX_CA_APREV, GX_CA_ZERO);
                        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        ntev++;

                        if (!(params.Mode & BlurFilterParams::Mode_HideObject))
                        {
                            // base + (1-base.a) * scolorAc
                            GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), (GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TEXMAP1, GX_COLOR_NULL);
                            GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_CPREV, GX_CC_ZERO, GX_CC_TEXA, GX_CC_TEXC);
                            GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_APREV, GX_CA_ZERO, GX_CA_TEXA, GX_CA_TEXA);
                            GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                            GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                            ntev++;
                        }
                    }
                    if (pparams.Mode & BlurFilterParams::Mode_Knockout)
                    {
                        GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + ntev), (GXTexCoordID)(GX_TEXCOORD0 + basetc), GX_TEXMAP1, GX_COLOR_NULL);
                        GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CC_CPREV, GX_CC_ZERO, GX_CC_TEXA, GX_CC_ZERO);
                        GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_CA_APREV, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO);
                        GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + ntev), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                        ntev++;
                    }
                }

                GXSetNumTexGens(ntex + ntex2);
                GXSetNumTevStages(ntev);

                const float vertices[] = {destrect.x1, destrect.y1, srcrect.x1, srcrect.y1,
                                          destrect.x2, destrect.y1, srcrect.x2, srcrect.y1,   
                                          destrect.x2, destrect.y2, srcrect.x2, srcrect.y2,
                                          destrect.x1, destrect.y2, srcrect.x1, srcrect.y2};

                GXBegin(GX_QUADS, GX_VTXFMT7, count * 4);
                float x = -SizeX, y = -SizeY;
                for (unsigned p = 0; p < count; p++)
                {
                    float xv[8], yv[8];
                    for (unsigned tc = 0; tc < ntex; tc++)
                        if (y <= SizeY)
                        {
                            xv[tc] = x - pparams.Offset.x;
                            yv[tc] = y - pparams.Offset.y;
                            x++;
                            if (x > SizeX)
                            {
                                x = -SizeX;
                                y++;
                            }
                        }
                        else
                            xv[tc] = yv[tc] = 1e10;

                    for (unsigned j = 0; j < 4; j++)
                    {
                        GXPosition2f32(vertices[j*4], vertices[j*4+1]);
                        for (unsigned tc = 0; tc < ntex; tc++)
                            if (xv[tc] < 1e9)
                                GXTexCoord2f32(vertices[j*4+2] + xv[tc], vertices[j*4+3] + yv[tc]);
                            else
                                GXTexCoord2f32(-1, -1);
                        if ((pparams.Mode & BlurFilterParams::Mode_FilterMask) == Filter_Shadow)
                            GXTexCoord2f32(vertices[j*4+2], vertices[j*4+3]);
                    }
                }
                GXEnd();
                AccumulatedStats.Triangles += count;

                // If we require the original source, create a new target for the source.
                if (requireSource && i == 0)
                    temporaryTextures[0] = *CreateTempRenderTarget(size, false);

                if (passi != 2 || filteri != filterCount-1)
                {
                    copyRTTexture(temporaryTextures[1]);
                    GXPixModeSync();
                    GXInvalidateTexAll();
                }
                else
                {
                    // Set the new dest texture so that PopRenderTarget will copy to it.
                    RenderTargetStack.Back().pRenderTarget = temporaryTextures[1];
                }

                // Setup for the next pass.
                Alg::Swap(temporaryTextures[0], temporaryTextures[1]);
            }

            for (int i = 0; i < 8; i++)
                GXSetTexCoordScaleManually((GXTexCoordID)(GX_TEXCOORD0 + i), GX_FALSE, 1, 1);

            AccumulatedStats.RTChanges  += n;
            AccumulatedStats.Primitives += n;
            AccumulatedStats.Meshes     += n;
        }
        else if (filterType == Filter_ColorMatrix || filterType == Filter_CacheAsBitmap)
        {
            const float* matrix = 0;
            if ( filterType == Filter_ColorMatrix)
            {
                ColorMatrixFilter& matrixFilter = *(ColorMatrixFilter*)filter;
                matrix = &matrixFilter[0];
            }
            else 
            {
                // For cacheAsBitmap, just render it as a ColorMatrixFilter, with an identity transform.
                static float identityMatrix[] = { 
                    1.0f, 0, 0, 0, 
                    0, 1.0f, 0, 0,
                    0, 0, 1.0f, 0,
                    0, 0, 0, 1.0f, 
                    0, 0, 0, 0 };
                matrix = (float*)&identityMatrix;
            }

            RenderTargetData* phdSrc = (RenderTargetData*)temporaryTextures[0]->GetRenderTargetData();

            const Rect<int>& viewRect = temporaryTextures[1]->GetRect(); // On the render texture, might not be the entire surface.
            const ImageSize& bs = temporaryTextures[1]->GetBufferSize();
            VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height(),
                          Viewport::View_RenderTextureAlpha);    
            ViewRect = Rect<int>(viewRect.x1, viewRect.y1, viewRect.x2, viewRect.y2);
            HALState |= HS_ViewValid;
            updateViewport();

            GXLoadTexObj(&phdSrc->pTexture->pTextures[0].Tex, GX_TEXMAP0);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

            GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED,   GX_CH_RED,   GX_CH_RED,   GX_CH_RED);

            const UInt32        regs[]  = {0x100 + GX_KCOLOR0,        0x100 + GX_KCOLOR1,         0x100 + GX_KCOLOR2,        0x100 + GX_KCOLOR3,         GX_TEVREG0,   GX_TEVREG1,   0x200, 0x201};
            const UInt32        cregs[] = {0x100 + GX_TEV_KCSEL_K0,   0x100 + GX_TEV_KCSEL_K1,    0x100 + GX_TEV_KCSEL_K2,   0x100 + GX_TEV_KCSEL_K3,    GX_CC_C0,     GX_CC_C1,     0x200, 0x201};
            const UInt32        aregs[] = {0x100 + GX_TEV_KASEL_K0_A, 0x100 + GX_TEV_KASEL_K1_A,  0x100 + GX_TEV_KASEL_K2_A, 0x100 + GX_TEV_KASEL_K3_A,  GX_CA_A0,     GX_CA_A1,     0x200, 0x201};
            const GXTevSwapSel  swaps[] = {GX_TEV_SWAP1, GX_TEV_SWAP2, GX_TEV_SWAP3, GX_TEV_SWAP0};

            const float mult6 = 63.25f;
            GXTevOp     ops[8];
            unsigned    rows[8];
            GXColor     mrows[8];
            unsigned    nvc  = 0;
            unsigned    ntev = 0;

            for (unsigned i = 0; i < 4; i++)
            {
                SInt16 r = matrix[i+0] * mult6;
                SInt16 g = matrix[i+4] * mult6;
                SInt16 b = matrix[i+8] * mult6;
                SInt16 a = matrix[i+12] * mult6;

                if (r > 0 || g > 0 || b > 0 || a > 0 || (ntev == 0 && i == 3))
                {
                    mrows[ntev] = (GXColor){r>0?r:0,g>0?g:0,b>0?b:0,a>0?a:0};
                    ops[ntev] = GX_TEV_ADD;
                    rows[ntev] = i;
                    ntev++;
                }
                if (r < 0 || g < 0 || b < 0 || a < 0)
                {
                    mrows[ntev] = (GXColor){r<0?-r:0,g<0?-g:0,b<0?-b:0,a<0?-a:0};
                    ops[ntev] = GX_TEV_SUB;
                    rows[ntev] = i;
                    ntev++;
                }
            }

            GXColorS10 cxadd = {
                Alg::Clamp<SInt16>(matrix[16] * mult6, -1024, 1023),
                Alg::Clamp<SInt16>(matrix[17] * mult6, -1024, 1023),
                Alg::Clamp<SInt16>(matrix[18] * mult6, -1024, 1023),
                Alg::Clamp<SInt16>(matrix[19] * mult6, -1024, 1023)};

            GXSetTevColorS10(GX_TEVREG2, cxadd);

            for (unsigned i = 0; i < ntev; i++)
            {
                GXTevColorArg cprev = i ? GX_CC_CPREV : GX_CC_C2;
                GXTevAlphaArg aprev = i ? GX_CA_APREV : GX_CA_A2;
                GXTevScale    scale = (i == ntev-1) ? GX_CS_SCALE_4 : GX_CS_SCALE_1;
                GXBool        clamp = (i == ntev-1) ? GX_TRUE : GX_FALSE;
                GXChannelID   chan = GX_COLOR_NULL;
                GXTevColorArg crow, texc = (swaps[rows[i]] == GX_TEV_SWAP0) ? GX_CC_TEXA : GX_CC_TEXC;
                GXTevAlphaArg arow;

                if (cregs[i] >= 0x200)
                {
                    chan = (cregs[i] == 0x201 ? GX_COLOR1A1 : GX_COLOR0A0);
                    GXSetChanMatColor(chan, mrows[i]);
                    GXSetChanCtrl(chan, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
                    crow = GX_CC_RASC;
                    arow = GX_CA_RASA;
                    nvc++;
                }
                else if (cregs[i] >= 0x100)
                {
                    GXSetTevKColor((GXTevKColorID)(regs[i]&0xff), mrows[i]);
                    GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0 + i), (GXTevKColorSel)(cregs[i]&0xff));
                    GXSetTevKAlphaSel((GXTevStageID)(GX_TEVSTAGE0 + i), (GXTevKAlphaSel)(aregs[i]&0xff));
                    crow = GX_CC_KONST;
                    arow = GX_CA_KONST;
                }
                else
                {
                    GXSetTevColor((GXTevRegID)regs[i], mrows[i]);
                    crow = (GXTevColorArg)cregs[i];
                    arow = (GXTevAlphaArg)aregs[i];
                }

                GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEXCOORD0, GX_TEXMAP0, chan);
                GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0 + i), GX_CC_ZERO, texc, crow, cprev);
                GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0 + i), GX_CA_ZERO, GX_CA_TEXA, arow, aprev);
                GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0 + i), ops[i], GX_TB_ZERO, scale, clamp, GX_TEVPREV);
                GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0 + i), ops[i], GX_TB_ZERO, scale, clamp, GX_TEVPREV);
                GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEV_SWAP0, swaps[rows[i]]);
            }

            GXSetNumChans(nvc);
            GXSetNumTexGens(1);

            GXSetNumTevStages(ntev);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_COPY);

            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 1, 1);

            GXBegin(GX_QUADS, GX_VTXFMT7, 4);
            {
                // mvp is identity
                GXPosition2f32(-1, 1); GXTexCoord2f32(srcrect.x1, srcrect.y1);
                GXPosition2f32(1,  1); GXTexCoord2f32(srcrect.x2, srcrect.y1);
                GXPosition2f32(1, -1); GXTexCoord2f32(srcrect.x2, srcrect.y2);
                GXPosition2f32(-1,-1); GXTexCoord2f32(srcrect.x1, srcrect.y2);
            }
            GXEnd();

            GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_BLUE,GX_CH_BLUE,  GX_CH_BLUE, GX_CH_ALPHA);
            for (unsigned i = 0; i < ntev; i++)
                GXSetTevSwapMode((GXTevStageID)(GX_TEVSTAGE0 + i), GX_TEV_SWAP0, GX_TEV_SWAP0);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 1, 1);

            AccumulatedStats.Primitives++;
            AccumulatedStats.Meshes++;
            AccumulatedStats.Triangles++;

            if (filteri != filterCount-1)
            {
                copyRTTexture(temporaryTextures[1]);
                GXPixModeSync();
                GXInvalidateTexAll();
            }
            else
            {
                // Set the new dest texture so that PopRenderTarget will copy to it.
                RenderTargetStack.Back().pRenderTarget = temporaryTextures[1];
            }

            Alg::Swap(temporaryTextures[0], temporaryTextures[1]);
        }
    }

    // Cache the last step, as it will be used to draw the filtered object in the final pass (and
    // potentially used on subsequent frames).
    RenderTarget* cacheResults[1] = { temporaryTextures[0] };
    e.pPrimitive->SetCacheResults(FilterPrimitive::Cache_Target, cacheResults, 1);
    ((RenderTargetData*)cacheResults[0]->GetRenderTargetData())->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());

    PopRenderTarget();

    // If this is not the top filter on the stack, we need to draw it into the stacked filtered target,
    // plus the content we previously resolved for the higher level stack.
    if ( FilterStack.GetSize() > 0 )
    {
        clearBuffer((RenderTargetData*)RenderTargetStack.Back().pRenderTarget->GetRenderTargetData());

        // Previous content.
        Matrix2F mvp = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
        RenderTarget* prt = FilterStack.Back().pRenderTarget;
        Texture* pt = (Texture*)prt->GetTexture();
        RenderTargetData* phd = (RenderTargetData*)(prt->GetRenderTargetData());
        Rect<int> srect = prt->GetRect();

        Matrix2F texgen;
        texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
        texgen.AppendScaling((float)srect.Width() / pt->GetSize().Width, (float)srect.Height() / pt->GetSize().Height);

        SManager.SetMatrix(SManager.CurShader, Uniform::SU_mvp, mvp);
        SManager.SetMatrix(SManager.CurShader, Uniform::SU_texgen, texgen);

        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, true, true);

        GXSetNumChans(0);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, 3);
        GXSetNumTexGens(1);
        GXLoadTexObj(&phd->pTexture->pTextures[0].Tex, GX_TEXMAP0);

        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_C0);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_A0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(1);

        GXSetCurrentMtx(0);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_S16, 0);

        GXBegin(GX_QUADS, GX_VTXFMT7, 4);
        {
            GXPosition2s16(0, 0);
            GXPosition2s16(1, 0);
            GXPosition2s16(1, 1);
            GXPosition2s16(0, 1);
        }
        GXEnd();

        applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal, false, (HALState&HS_InRenderTarget) != 0);

        // And then the cached filter.
        drawCachedFilter(e.pPrimitive);
    }

    // Cleanup. NOTE: We do not set the final target to not be in use, because we will require
    // it in the final pass rendering.
    for ( unsigned i = 1; i < 3; ++i )
    {
        if (temporaryTextures[i])
            temporaryTextures[i]->SetInUse(false);
    }

    GXSetCurrentMtx(0);

    AccumulatedStats.Filters += filters->GetFilterCount();
*/    
}

void HAL::drawCachedFilter(FilterPrimitive* primitive)
{
/*
    switch(primitive->GetCacheState())
    {
        // wii should never cache pre-target.
    case FilterPrimitive::Cache_Uncached:
    case FilterPrimitive::Cache_PreTarget:
        {
            SF_DEBUG_WARNING(1, "Filter was not cached. Drawing content without filtering. ");
            break;
        }

        // We have a final filtered texture. Just apply it to a screen quad.
    case FilterPrimitive::Cache_Target:
        {
            GXSetNumChans(0);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, 3);
            GXSetNumTexGens(1);

            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_C0);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_A0);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

            applyBlendMode(BlendModeStack.GetSize() ? BlendModeStack.Back() : Blend_Normal, false, true);

            if (FillFlags & FF_Multiply)
            {
                GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
                GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
                GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                GXSetNumTevStages(2);
            }
            else
                GXSetNumTevStages(1);

            GXSetCurrentMtx(0);

            RenderTarget* results;
            primitive->GetCacheResults(&results, 1);

            Texture* ptexture = (Wii::Texture*)results->GetTexture();
            GXLoadTexObj(&ptexture->pTextures[0].Tex, GX_TEXMAP0);

            const Matrix2F& mvp = Matrices->UserView * primitive->GetFilterAreaMatrix().GetMatrix2D();
            const Rect<int>& srect = results->GetRect();
            Matrix2F texgen;
            texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
            texgen.AppendScaling((float)srect.Width() / (ptexture->GetSize().Width),
                                 (float)srect.Height() / (ptexture->GetSize().Height));

            SManager.SetMatrix(SManager.CurShader, Uniform::SU_mvp, mvp);
            SManager.SetMatrix(SManager.CurShader, Uniform::SU_texgen, texgen);

            SManager.SetCxform(SManager.CurShader, primitive->GetFilterAreaMatrix().GetCxform());

            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_S16, 0);

            GXBegin(GX_QUADS, GX_VTXFMT7, 4);
            {
                GXPosition2s16(0, 0);
                GXPosition2s16(1, 0);
                GXPosition2s16(1, 1);
                GXPosition2s16(0, 1);
            }
            GXEnd();

            applyBlendMode(BlendModeStack.GetSize()>=1 ? BlendModeStack.Back() : Blend_Normal);

            results->SetInUse(false);
            break;
        }

        // Should have been one of the other two caching types.
    default: SF_ASSERT(0); break;
    }
*/    
}

void HAL::drawScreenQuad()
{
/* NCT_COMMENT
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT7, 4);
    GXPosition2f32(0, 0);
    GXPosition2f32(1, 0);
    GXPosition2f32(1, 1);
    GXPosition2f32(0, 1);
    GXEnd();
*/
}

}}} // Scaleform::Render::Wii
