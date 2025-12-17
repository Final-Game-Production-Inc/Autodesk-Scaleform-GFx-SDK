/**************************************************************************

Filename    :   Wii_HALSetup.cpp
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Wii_HAL.h"
#include "Wii_MemoryManager.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_BufferGeneric.h"

#include <revolution/vi.h>

namespace Scaleform { namespace Render { namespace Wii {


// *** RenderHAL_Wii Implementation
    
bool HAL::InitHAL(const Wii::HALInitParams& params)
{
    if ( !Render::HAL::initHAL(params))
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
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(1 << 28);
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

    Render::HAL::shutdownHAL();

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
    GXSetCopyClear(BLACK, 0);

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
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags)
{
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
}

void HAL::copyRTTexture(RenderTarget* prt)
{
    RenderTargetData* phd = (Wii::RenderTargetData*)prt->GetRenderTargetData();
    Texture::HWTextureDesc& tdesc = phd->pTexture->pTextures[0];

    GXDrawDone();
    GXSetTexCopySrc(0, 0, tdesc.Width, tdesc.Height);
    GXSetTexCopyDst(tdesc.Width, tdesc.Height, GX_TF_RGBA8, GX_FALSE);
    GXColor color = {0,0,0,0};
    GXSetCopyClear(color, 0);
    GXCopyTex(tdesc.pTexData, GX_TRUE);
}

void HAL::clearBuffer(RenderTargetData* phd)
{
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
}

void HAL::PopRenderTarget(unsigned)
{
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
        CalcHWViewMatrix(&Matrices->View2D, ViewRect, dx, dy);
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

void HAL::CalcHWViewMatrix(Matrix* pmatrix, const Rect<int>& viewRect, int dx, int dy)
{
    float       vpWidth = (float)viewRect.Width();
    float       vpHeight= (float)viewRect.Height();

    pmatrix->SetIdentity();
    pmatrix->Sx() = 2.0f  / vpWidth;
    pmatrix->Sy() = -2.0f / vpHeight;
    pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx);
    pmatrix->Ty() = 1.0f  - pmatrix->Sy() * ((float)dy);
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

}}} // Scaleform::Render::Wii
