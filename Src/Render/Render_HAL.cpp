/**************************************************************************

Filename    :   Render_HAL.cpp
Content     :   Non-platform specific HAL implementated
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Render/Render_Bundle.h"
#include "Render/Render_Queue.h"
#include "Render/Render_DrawableImage.h"
#include "Render/Render_DrawableImage_Queue.h"

namespace Scaleform { namespace Render {

MatrixState::MatrixState( HAL* phal ) : UVPOChanged(0), OrientationSet(0), S3DDisplay(StereoCenter), pHAL(phal)
{
    // All platforms (except GL) use this as their 'full viewport' quad. 
    // Note that D3D9 and X360 require a half-pixel offset applied to match pixel centers, but
    // this depends on the size of the target so it can't be done here.
    FullViewportMVP = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
}

MatrixState::MatrixState() : UVPOChanged(0), OrientationSet(0), S3DDisplay(StereoCenter), pHAL(NULL)
{
    FullViewportMVP = Matrix2F::Scaling(2,-2) * Matrix2F::Translation(-0.5f, -0.5f);
}

void MatrixState::SetUserMatrix(const Matrix2F& user, const Matrix2F& user3D)
{
    UVPOChanged = 1;
    User = user;
    UserView = View2D * (User * Orient2D);
    User3D = user3D;

    // Offset the 3D content, so that it will match the transforms made to 2D content.
    if (ViewRect.Width() > 0 && ViewRect.Height() > 0)
    {
        float w = (float)(ViewRect.Width());
        float h = (float)(ViewRect.Height());
        PointF center = PointF(w * 0.5f, h * 0.5f);
        PointF dc = User3D * center - center;
        User3D.M[0][3] = ( 2.0f * dc.x / w);
        User3D.M[1][3] = (-2.0f * dc.y / h);
    }
    else
    {
        User3D.M[0][3] = 0.0f;
        User3D.M[1][3] = 0.0f;
    }
}

void MatrixState::SetUserMatrix(const Matrix2F& user)
{
    SetUserMatrix(user, user);
}

void MatrixState::SetViewportMatrix(const Matrix2F& vp)
{
    View2D = vp;
    UserView = View2D * (User * Orient2D);
}

Viewport MatrixState::SetOrientation(const Viewport& vp)
{
    OrientationSet = 0;
    UVPOChanged = 1;

    switch (vp.Flags & Viewport::View_Orientation_Mask)
    {
    case Viewport::View_Orientation_Normal:
        Orient2D.SetIdentity();
        Orient3D.SetIdentity();
        break;

    case Viewport::View_Orientation_R90:
    case Viewport::View_Orientation_L90:
        {
            OrientationSet = 1;

            float flipm = ((vp.Flags & Viewport::View_Orientation_Mask) == Viewport::View_Orientation_L90 ? -1.0f : 1.0f);
            Orient2D.SetMatrix(0, -flipm, flipm < 0 ? 0 : (float)vp.BufferWidth,
                               flipm, 0,  flipm < 0 ? (float)vp.BufferHeight : 0);
            Orient3D.M[0][0] = 0;
            Orient3D.M[0][1] = flipm;
            Orient3D.M[1][1] = 0;
            Orient3D.M[1][0] = -flipm;
        }
        break;
    }

    UserView = View2D * (User * Orient2D);

    Viewport viewport;
    PointF tl = Orient2D.Transform(PointF((float)vp.Left, (float)vp.Top));
    PointF br = Orient2D.Transform(PointF(float(vp.Left+vp.Width), float(vp.Top+vp.Height)));
    viewport.Flags = vp.Flags;
    viewport.Left = (int)ceilf(Alg::Min(tl.x, br.x));
    viewport.Top = (int)ceilf(Alg::Min(tl.y, br.y));
    viewport.Width = (int)ceilf(fabsf(tl.x - br.x));
    viewport.Height = (int)ceilf(fabsf(tl.y - br.y));
    if (viewport.Flags & Viewport::View_UseScissorRect)
    {
        PointF sctl = Orient2D.Transform(PointF((float)vp.ScissorLeft, (float)vp.ScissorTop));
        PointF scbr = Orient2D.Transform(PointF(float(vp.ScissorLeft+vp.ScissorWidth),
                                                float(vp.ScissorTop+vp.ScissorHeight)));
        viewport.Flags = vp.Flags;
        viewport.ScissorLeft = (int)ceilf(Alg::Min(sctl.x, scbr.x));
        viewport.ScissorTop = (int)ceilf(Alg::Min(sctl.y, scbr.y));
        viewport.ScissorWidth = (int)ceilf(fabsf(sctl.x - scbr.x));
        viewport.ScissorHeight = (int)ceilf(fabsf(sctl.y - scbr.y));
    }
    viewport.BufferWidth = vp.BufferWidth;
    viewport.BufferHeight = vp.BufferHeight;

    return viewport;
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

        const Matrix4F& Projection = updateStereoProjection();
        Matrix4F UO(User3D, ViewRectCompensated3D);
        Matrix4F VRP(Orient3D, Projection);
        UVPO = Matrix4F(Matrix4F(UO, VRP), View3D);
        UVPOChanged = 0;
    }
}

void MatrixState::Copy(MatrixState* outmat, MatrixState* inmat)
{
    outmat->View2D = inmat->View2D;
    outmat->View3D = inmat->View3D;
    outmat->Proj3D = inmat->Proj3D;
    outmat->Proj3DLeft = inmat->Proj3DLeft;
    outmat->Proj3DRight = inmat->Proj3DRight;

    outmat->User = inmat->User;
    outmat->User3D = inmat->User3D;
    outmat->Orient2D = inmat->Orient2D;
    outmat->Orient3D = inmat->Orient3D;

    outmat->ViewRectOriginal = inmat->ViewRectOriginal;
    outmat->ViewRect = inmat->ViewRect;

    outmat->UserView = inmat->UserView;
    outmat->UVPO = inmat->UVPO;

    outmat->ViewRectCompensated3D = inmat->ViewRectCompensated3D; 

    // Always set "change" to be true, in case there are any caching dependencies in the HALs.
    // This will just cause the UVPO to be recalculated anytime it is copied from another source.
    outmat->UVPOChanged = true; 
    outmat->OrientationSet = inmat->OrientationSet;

    if (inmat->S3DImpl)
    {
        outmat->S3DImpl = inmat->S3DImpl;
        outmat->S3DImpl->SetParams(inmat->S3DImpl->GetParams());
    }
    
    outmat->S3DDisplay = inmat->S3DDisplay;
    
    // Seems a bit squirrelly, but done so that we can have a default constructor for the class
    // in order for RenderTargetEntry to be able to directly instantiate a MatrixState instance.
    outmat->pHAL = inmat->pHAL;
}

void MatrixState::CopyFrom(MatrixState* state)
{
    Copy(this, state);
}

void MatrixState::CopyTo(MatrixState* state)
{
    Copy(state, this);
}

const Matrix4F& MatrixState::updateStereoProjection(float factor) const
{
    if (S3DDisplay != StereoCenter && S3DImpl)
    {
        float eyeZ = -View3D.M[2][3];
        if (S3DDisplay == StereoLeft)
        {
            S3DImpl->GetStereoProj(Proj3D, eyeZ, &Proj3DLeft, NULL, factor);
            return Proj3DLeft;
        }
        else if (S3DDisplay == StereoRight)
        {
            S3DImpl->GetStereoProj(Proj3D, eyeZ, NULL, &Proj3DRight, factor);
            return Proj3DRight;
        }
    }
    return Proj3D;
}

void Viewport::SetStereoViewport(unsigned display)
{
	switch (Flags & View_Stereo_AnySplit)
	{
	case View_Stereo_SplitV:
		Height = Height >> 1;
		Top = Top >> 1;
		if (display == StereoRight)
			Top += BufferHeight >> 1;
		break;

	case View_Stereo_SplitH:
		Width = Width >> 1;
		Left = Left >> 1;
		if (display == StereoRight)
			Left += BufferWidth >> 1;
		break;
	}
}

//
// Stereo projection matrix is a horizontally offset version of regular mono 
// projection matrix, shifted in X.
//
// Essentially, the camera separation  = interaxial / screen width;  (scaled 
// by a distortion scale factor)
//
void StereoImplBase::GetStereoProj(
                                    const Matrix4F &original, 	// original (mono) perspective matrix
                                    float screenDist,           // eye distance to screen, eye Z 
                                    Matrix4F *left, 		    // dest perspective matrix ptr for left eye
                                    Matrix4F *right, 	        // dest perspective matrix ptr for right eye
                                    float factor)   const

{
    Matrix4F postProjectionMove;
    Matrix4F preProjectionMove;
    Matrix4F tmpMat;
    float postParam, preParam;

    postParam = S3DParams.Distortion * factor * S3DParams.EyeSeparationCm / S3DParams.DisplayWidthCm;
    preParam = - postParam * screenDist * original.M[3][2] / original.M[0][0];
    if (preParam < 0)
        preParam = -preParam;

    if (left)
    {
        postProjectionMove.M[0][3]  = -postParam;
        preProjectionMove.M[0][3] =  preParam;

        tmpMat = original * preProjectionMove;
        *left = postProjectionMove * tmpMat;
    }

    if (right)
    {
        postProjectionMove.M[0][3]  =  postParam;
        preProjectionMove.M[0][3] = -preParam;

        tmpMat = original * preProjectionMove;
        *right = postProjectionMove * tmpMat;
    }
}

Matrix2F& MatrixState::GetFullViewportMatrix(const Size<int>& rtSize)
{
    SF_UNUSED(rtSize);
    return FullViewportMVP;
}

const Matrix4F& MatrixState::GetUVP() const
{
    recalculateUVPOC();
    return UVPO;
}

const Matrix4F& MatrixState::GetUVP( const Rect<int> & viewRect ) const
{
    if ( viewRect != ViewRect )
    {
        ViewRect = viewRect;
        UVPOChanged = true;
    }
    recalculateUVPOC();
    return UVPO;
}

//---------------------------------------------------------------------------------------
HAL::HAL(ThreadCommandQueue *commandQueue)  : 
    HALState(0),
    CurrentPass(Display_All),
    VMCFlags(0),
    FillFlags(0),
    RenderThreadID(0),
    pHeap(0),
    pRTCommandQueue(commandQueue),
    pRenderBufferManager(0),
    QueueProcessor(Queue, getThis()),
    CurrentDepthStencilState(DepthStencil_Disabled),
    CurrentStencilRef(0),
    StencilChecked(false), 
    StencilAvailable(false), 
    MultiBitStencil(false),
    DepthBufferAvailable(false),
    NextSceneRasterMode(RasterMode_Solid),
    CurrentSceneRasterMode(RasterMode_Solid),
    AppliedSceneRasterMode(RasterMode_Solid),
    MaskStackTop(0),
    CacheableIndex(-1),
	CacheablePrepIndex(-1),
    CacheablePrepStart(-1)
{ 
    pHeap = Memory::GetGlobalHeap();
    
    Link_RenderStats(); 
}

bool HAL::initHAL(const HALInitParams& params)
{
    initMatrices();
    Matrices->S3DImpl = *SF_NEW StereoImplBase;

    VMCFlags = params.ConfigFlags;
    RenderThreadID = params.RenderThreadId;

    // If no RendeThreadID is supplied, assume that the render thread is the one calling InitHAL.
    if ( RenderThreadID == 0 )
    {
        SF_DEBUG_WARNING(RenderThreadID == 0, "HALInitParams::RenderThreadId is NULL - assuming current thread is the render thread.");
        RenderThreadID = GetCurrentThreadId();
    }

    // When starting up, assume that all profiler modes and flags are available. Derived HALs may disable specific
    // modes in their InitHAL function (assumes that this method is called first).
    GetProfiler().SetModeAvailability((unsigned)Profile_All);
    GetProfiler().SetFlagAvailability((unsigned)ProfileFlag_All);

    return Queue.Initialize(params.RenderQueueSize);
}

void HAL::initMatrices()
{
    // Allocate our matrix state
    Matrices = *SF_HEAP_AUTO_NEW(this) MatrixState(this);
}

bool HAL::shutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;
    notifyHandlers(HAL_Shutdown);

    Queue.Shutdown();

    // Remove ModeSet and other state flags.
    HALState = 0;    

    // Remove availability of all profiling.
    GetProfiler().SetModeAvailability(0);
    GetProfiler().SetFlagAvailability(0);

    return true;
}

bool HAL::BeginFrame()
{
    GetEvent(Event_Frame).Begin(__FUNCTION__);
    if (!checkState(HS_ModeSet, __FUNCTION__))
        return false;

    if (HALState & HS_ReadyForReset)
        return false;

    HALState |= HS_InFrame;

    if (GetRenderSync())
        GetRenderSync()->BeginFrame();

    GetRQProcessor().BeginFrame();
    GetMeshCache().BeginFrame();
    GetTextureManager()->BeginFrame();

    return true;
}

void HAL::EndFrame()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Frame), 0, false);
    if (!checkState(HS_ModeSet|HS_InFrame, __FUNCTION__))
        return;

    // RenderSync::EndFrame should come before MeshCache::EndFrame - as it function will do fence pending checks,
    // which may be marked as 'quick' reclaimed, during RenderSync::EndFrame.
    if (GetRenderSync())
        GetRenderSync()->EndFrame();

    RenderBufferManager* prbm = GetRenderBufferManager();
    if ( prbm )
        prbm->EndFrame();
    GetMeshCache().EndFrame();
    GetTextureManager()->EndFrame();
    HALState &= ~HS_InFrame;

}

void HAL::FinishFrame()
{
    // Nothing by default
}

class HALBeginDisplayItem : public RenderQueueItem::Interface
{
public:
    static HALBeginDisplayItem Instance;
    virtual void EmitToHAL(RenderQueueItem&item, RenderQueueProcessor&qp) 
    {
        BeginDisplayData* data = (BeginDisplayData*)item.GetData();
        qp.GetHAL()->beginDisplay(data);
    }
};
class HALEndDisplayItem : public RenderQueueItem::Interface
{
public:
    static HALEndDisplayItem Instance;
    virtual void EmitToHAL(RenderQueueItem&, RenderQueueProcessor&qp) 
    {
        qp.GetHAL()->endDisplay();
    }
};
HALBeginDisplayItem HALBeginDisplayItem::Instance;
HALEndDisplayItem   HALEndDisplayItem::Instance;

void HAL::BeginDisplay(Color backgroundColor, const Viewport& vpin)
{
    if (!checkState(HS_InFrame, __FUNCTION__))
        return;

    BeginDisplayData entry(backgroundColor, vpin);
    BeginDisplayData* data = BeginDisplayDataList.Alloc(entry);

    // If BeginScene has not be called, BeginDisplay cannot be queued, because it is 
    // presumed that there is no matching EndScene call. This would not flush rendering
    // once it is completed, and thus Present would display no content. This is valid
    // in the case of a single movie rendering within a scene. Otherwise, Begin/EndScene
    // should be called.
    if ( !(HALState & HS_InScene) )
        beginDisplay(data);
    else
        Draw(&HALBeginDisplayItem::Instance, data);
}


void HAL::EndDisplay()
{
    // If BeginDisplay called BeginScene, we must call EndDisplay directly (not queued), because
    // otherwise rendering will not be flushed (because EndScene would not be called). This would
    // cause no display if Present were called.
    if (!(HALState & HS_SceneInDisplay))
        Draw(&HALEndDisplayItem::Instance);
    else
    {
        Flush();
        endDisplay();
    }
}


//---------------------------------------------------------------------------------------
void HAL::beginDisplay(BeginDisplayData* data)
{
    GetEvent(Event_Display).Begin(__FUNCTION__);
    if (!checkState(HS_InFrame, __FUNCTION__))
        return;
    HALState |= HS_InDisplay;

    Color backgroundColor = data->BackgroundColor;
    const Viewport& vpin  = data->VP;
    BeginDisplayDataList.Free(data);

    // BeginScene automatically calls BeginScene if necessary.
    if (!(HALState & HS_InScene))
    {
        BeginScene();
        HALState |= HS_SceneInDisplay;
    }

    applyBlendMode(CurrentBlendState);

    beginMaskDisplay();

    VP = Matrices->SetOrientation(vpin);
    if (VP.GetClippedRect(&ViewRect))
        HALState |= HS_ViewValid;
    else
        HALState &= ~HS_ViewValid;

    updateViewport();

    // If there is a child that has a BlendMode that requires a target, and we are not currently in a RenderTarget
    // (eg. the buffer currently being rendered to cannot be sampled), then we must create a target that can be sampled
    // and render the entire scene into that.
    if ((GetHALState() & (HS_BlendTarget|HS_InRenderTarget)) == HS_BlendTarget)
    {
        FullSceneBlendTarget = *CreateTempRenderTarget(ViewRect.GetSize(), true);
        PushRenderTarget(Rect<int>(ViewRect.GetSize()), FullSceneBlendTarget);
    }

    // Clear the background with a solid quad, if background has alpha > 0.
    if (backgroundColor.GetAlpha() > 0 && !(vpin.Flags & Viewport::View_NoClear))
    {
        clearSolidRectangle(Rect<int>(vpin.Width, vpin.Height), backgroundColor, true);
    }
}

void HAL::endDisplay()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Display), 0, false);
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;    

    endMaskDisplay();
    SF_ASSERT(BlendModeStack.GetSize() == 0);

    if (GetHALState() & HS_BlendTarget &&
        RenderTargetStack.GetSize() > 0 &&
        RenderTargetStack.Back().pRenderTarget == FullSceneBlendTarget)
    {
        RenderTargetEntry& rte = RenderTargetStack.Back();
        PopRenderTarget();

        // Render to the backbuffer.
        applyBlendMode(Blend_Normal, true, true);
        Render::Texture* ptex = rte.pRenderTarget->GetTexture();
        Matrix2F fullViewportMatrix = Matrices->GetFullViewportMatrix(ptex->GetSize());
        Matrix2F texgen(Matrix2F::Identity);
        texgen.AppendScaling(SizeF(rte.pRenderTarget->GetRect().GetSize()) / SizeF(ptex->GetSize()));
        texgen.AppendScaling(1.0f, -GetViewportScaling());
        DrawableCopyback(ptex, fullViewportMatrix, texgen, 0); // disable 1/2 pixel offset
        rte.pRenderTarget->SetInUse(RTUse_Unused);
    }

    if (HALState & HS_SceneInDisplay)
    {
        EndScene();
        HALState &= ~HS_SceneInDisplay;
    }

    // Must clear - but clear after EndScene, because it may do a Flush. Clearing this
    // flag beforehand would cause all the flushed items to fail, because we were not
    // currently in display.
    HALState &= ~HS_InDisplay;
}

void HAL::CalcHWViewMatrix(unsigned vpFlags, Matrix* pmatrix, const Rect<int>& viewRect, int dx, int dy)
{
    float       vpWidth = (float)viewRect.Width();
    float       vpHeight= (float)viewRect.Height();

    float       xhalfPixelAdjust = 0.0f;
    float       yhalfPixelAdjust = 0.0f;
    if (vpFlags & Viewport::View_HalfPixelOffset)
    {
        xhalfPixelAdjust = (viewRect.Width() > 0) ? (1.0f / vpWidth) : 0.0f;
        yhalfPixelAdjust = (viewRect.Height()> 0) ? (1.0f / vpHeight) : 0.0f;
    }

    pmatrix->SetIdentity();
    if (vpFlags & Viewport::View_IsRenderTexture)
    {
        pmatrix->Sx() = 2.0f  / vpWidth;
        pmatrix->Sy() = 2.0f /  vpHeight;
        pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx) - xhalfPixelAdjust; 
        pmatrix->Ty() = -1.0f - pmatrix->Sy() * ((float)dy) - yhalfPixelAdjust;
    }
    else
    {
        pmatrix->Sx() = 2.0f  / vpWidth;
        pmatrix->Sy() = -2.0f / vpHeight;
        pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx) - xhalfPixelAdjust; 
        pmatrix->Ty() = 1.0f  - pmatrix->Sy() * ((float)dy) + yhalfPixelAdjust;
    }
}


void HAL::Flush()
{
    GetRQProcessor().Flush();
}

bool HAL::BeginScene()
{
    GetEvent(Event_Scene).Begin(__FUNCTION__);
    if (!checkState(HS_InFrame, __FUNCTION__))
        return false;

    if ( GetTextureManager() )
        GetTextureManager()->BeginScene();

    // Set the blend mode state Force setting of states, by changing the cached values, and then applying different settings.
    CurrentBlendState.Mode = Blend_Count;   
    CurrentBlendState.BlendEnable = false;
    applyBlendMode(Blend_None, false, false);
    applyBlendModeEnable(true);

    // Set the depth/stencil state, to off. Set the redundancy checks to a mode that ignores everything.
    // When we set the mode to disabled, it will only set the modes it actually needs.
    CurrentDepthStencilState = DepthStencil_Invalid;
    CurrentStencilRef        = (unsigned)-1;
    applyDepthStencilMode(DepthStencil_Disabled, 0);
    
    // Set the rasterization state.
    CurrentSceneRasterMode = NextSceneRasterMode;
    AppliedSceneRasterMode = RasterMode_Count; // invalidate to force set.
    applyRasterMode(CurrentSceneRasterMode);

    HALState |= HS_InScene;
    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);
    if (!checkState(HS_InFrame|HS_InScene, __FUNCTION__))
        return false;

    // Flush all rendering on EndScene.
    Flush();

    if ( GetTextureManager() )
        GetTextureManager()->EndScene();

    // Finally, clear the flag.
    HALState &= ~HS_InScene;
    return true;
}

RenderTarget* HAL::GetDefaultRenderTarget()
{
    if (RenderTargetStack.GetSize() == 0 )
        return 0;
    return RenderTargetStack[0].pRenderTarget;
}

void HAL::destroyRenderBuffers()
{
    RenderTargetStack.Clear();
}

PrimitiveFill*  HAL::CreatePrimitiveFill(const PrimitiveFillData &data)
{
    return SF_HEAP_NEW(pHeap) PrimitiveFill(data);
}

void HAL::checkState_EmitWarnings(unsigned stateFlags, const char* funcName)
{    
    SF_UNUSED2(stateFlags, funcName);
#ifdef SF_BUILD_DEBUG
    // Outputs debug warnings for missing states.
    struct WarnHelper
    {
        unsigned CheckFlags, HALState;
        WarnHelper(unsigned cf, unsigned hs) : CheckFlags(cf), HALState(hs) { }

        bool operator ()(unsigned checkBit, unsigned requiredBits) const
        {
            return (CheckFlags & checkBit) &&
                   ((HALState & (requiredBits | checkBit)) == requiredBits);
        }
    };

    WarnHelper needWarning(stateFlags, HALState);

    // TBD: WE need a better solution then secondary mask for when to NOT display warnings.
    //      Once BeginFrame fails, for example, there is no need to warn on all other calls.
    SF_DEBUG_WARNING1(needWarning(HS_ModeSet, 0),
                      "%s failed - Mode Not Set", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InFrame, 0),
                      "%s failed - Begin/EndFrame missing/failed.", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InDisplay, HS_InFrame|HS_ModeSet),
                      "%s failed - Begin/EndDisplay missing/failed.", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_DeviceValid, 0),
                      "%s failed - Device Lost or not valid", funcName);

    SF_DEBUG_WARNING1(needWarning(HS_DrawingMask, HS_InFrame|HS_InDisplay),
                      "%s failed - PushMask_BeginSubmit call missing", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_DrawingFilter, HS_InFrame|HS_InDisplay),
                      "%s failed - PushFilter call missing", funcName);
#endif
}

void HAL::DrawBundleEntries( BundleIterator ibundles, Renderer2DImpl* r2d )
{
    if ( CurrentPass == Display_All && IsPrepassRequired() )
    {
        SetDisplayPass(Display_Prepass);
        DrawBundleEntries(ibundles, r2d);
        SetDisplayPass(Display_Final);
        DrawBundleEntries(ibundles, r2d);
        SetDisplayPass(Display_All);
    }
    else
    {
        switch(CurrentPass)
        {
        case Display_All:
        case Display_Final:
			GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);
			GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_All);
            break;
        case Display_Prepass:
			GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_CacheableOnly);
            GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_CacheableOnly);
            break;
        }

        while(ibundles)
        {
            ibundles->DrawBundleEntry(r2d);
            ibundles++;
        }
    }
}

// Draws a RenderQueueItem by placing it into a queue.

void HAL::Draw(const RenderQueueItem& item)
{    
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_Medium);

    // Must be in display, unless this is a queued BeginDisplay render item.
    if (item.GetInterface() != &HALBeginDisplayItem::Instance && 
        !checkState(HS_InDisplay, __FUNCTION__))
        return;

    RenderQueueProcessor& qp = GetRQProcessor();
    RenderQueueItem* pitem = Queue.ReserveHead();
    if (!pitem)
    {
        qp.ProcessQueue(RenderQueueProcessor::QPM_One);
        pitem = Queue.ReserveHead();
        SF_ASSERT(pitem);
    }

    // We can add our primitive.
    // On Consoles, this may end up building up multiple primitives since cache
    // eviction will not rely on lock but will wait instead.
    *pitem = item;    
    Queue.AdvanceHead();

    // Process as many items as possible.
    qp.ProcessQueue(RenderQueueProcessor::QPM_Any);
}

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
    GetEvent(Event_Mask).Begin(__FUNCTION__);
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    if (!checkDepthStencilBufferCaps())
        return;

    // Draw masking primitives in 'solid' rasterization mode, otherwise the masking may not affect the stencil buffer.
    applyRasterMode(RasterMode_Solid);

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        // Erase rectangles of these matrices; must be done even for clipped masks.
        if (StencilAvailable && MultiBitStencil)
        {
            // Any stencil of value greater then MaskStackTop should be set to it;
            // i.e. replace when (MaskStackTop < stencil value).
            unsigned maxStencilValue = MaskStackTop;
            applyDepthStencilMode(DepthStencil_StencilClearHigher, maxStencilValue);
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

    if ((MaskStackTop == 1) && viewportValid)
    {
        // Clear view rectangles.
        if (StencilAvailable)
        {
            // Unconditionally overwrite stencil rectangles with REF value of 0.
            applyDepthStencilMode(DepthStencil_StencilClear, 0);
        }
        else
        {
            // Depth clears bounds. 
            applyDepthStencilMode(DepthStencil_DepthWrite, 0);
        }
        drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
    }

    // Prepare for primitives rendering masks.
    if (StencilAvailable)
    {
        if (MultiBitStencil)
        {
            // Increment only if we support it.
            applyDepthStencilMode(DepthStencil_StencilIncrementEqual, (MaskStackTop-1));
        }
        else
        {
            // If we cannot increment, setup for 1-bit masks (no nested masking).
            applyDepthStencilMode(DepthStencil_StencilClear, 1);
            SF_DEBUG_WARNONCE(MaskStackTop > 1, "Nested-masks used, but only single-bit stencil buffer available. Nested masks will be incorrect.");
        }
    }
    else if (DepthBufferAvailable)
    {
        // Set the correct render states in order to not modify the color buffer
        // but write the default Z-value everywhere.
        applyDepthStencilMode(DepthStencil_DepthWrite, 0);
        SF_DEBUG_WARNONCE(MaskStackTop > 1, "Nested-masks used, but only depth buffer available. Nested masks will be incorrect.");
    }
    ++AccumulatedStats.Masks;
}


void HAL::EndMaskSubmit()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Mask), 0, false);

    if (!checkState(HS_InDisplay|HS_DrawingMask, __FUNCTION__))
        return;
    HALState &= ~HS_DrawingMask;    
    SF_DEBUG_ASSERT(MaskStackTop, "EndMaskSubmit called, but not masks on the MaskStack.");

    if (StencilAvailable)
    {
        // We draw only where the (MaskStackTop <= stencil), i.e. where the latest mask was drawn.
        // However, we don't change the stencil buffer
        applyDepthStencilMode(DepthStencil_StencilTestLessEqual, MaskStackTop);
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write and write only where the mask had written
        applyDepthStencilMode(DepthStencil_DepthTestEqual, 0);
    }

    // Re-apply the current raster mode, it may have been set to solid in PushMask_BeginSubmit.
    applyRasterMode(CurrentSceneRasterMode);
}


void HAL::PopMask()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_PopMask), __FUNCTION__);
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    if (!checkDepthStencilBufferCaps())        
        return;

    SF_DEBUG_ASSERT(MaskStackTop, "No items on the MaskStack, during HAL::PopMask");
    MaskStackTop--;

    // Disable mask or decrement stencil reference value.
    if (StencilAvailable)
    {
        if (MaskStackTop == 0)
        {
            applyDepthStencilMode(DepthStencil_Disabled, 0);
        }
        else
        {
            // Change ref value down, so that we can draw using previous mask.
            applyDepthStencilMode(DepthStencil_StencilTestLessEqual, MaskStackTop);
        }
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write/test.
        applyDepthStencilMode(DepthStencil_Disabled, 0);
    }
}

#define BO(x) HAL::BlendOp_##x
#define BF(x) HAL::BlendFactor_##x

HAL::BlendModeDescriptor HAL::BlendModeTable[Blend_Count] =
{
    { BO(ADD),         BF(SRCALPHA),      BF(INVSRCALPHA), BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // None
    { BO(ADD),         BF(SRCALPHA),      BF(INVSRCALPHA), BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // Normal
    { BO(ADD),         BF(SRCALPHA),      BF(INVSRCALPHA), BO(ADD), BF(SRCALPHA),  BF(INVSRCALPHA)  }, // Layer
                                                                                          
    { BO(ADD),         BF(DESTCOLOR),     BF(INVSRCALPHA), BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // Multiply
    { BO(ADD),         BF(INVDESTCOLOR),  BF(ONE),         BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // Screen

#if defined(SF_RENDER_DARKEN_LIGHTEN_OLD_BEHAVIOR)
    { BO(MAX),         BF(SRCALPHA),      BF(ONE),         BO(MAX), BF(SRCALPHA),  BF(ONE)          }, // Lighten
    { BO(MIN),         BF(SRCALPHA),      BF(ONE),         BO(MIN), BF(SRCALPHA),  BF(ONE)          }, // Darken
#else
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // Lighten (Same as overwrite all)
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // Darken (Same as overwrite all)
#endif

    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // Difference (Same as overwrite all)
                                                                                          
    { BO(ADD),         BF(SRCALPHA),      BF(ONE),         BO(ADD), BF(ONE),       BF(ONE)          }, // Add
    { BO(REVSUBTRACT), BF(SRCALPHA),      BF(ONE),         BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // Subtract
                                                           
    { BO(ADD),         BF(INVDESTCOLOR),  BF(INVSRCALPHA), BO(ADD), BF(SRCALPHA),  BF(INVSRCALPHA)  }, // Invert
                                                                                          
    { BO(ADD),         BF(ZERO),          BF(ONE),         BO(ADD), BF(ZERO),      BF(SRCALPHA)     }, // Alpha
    { BO(ADD),         BF(ZERO),          BF(ONE),         BO(ADD), BF(ZERO),      BF(INVSRCALPHA)  }, // Erase
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // Overlay (Same as overwrite all)
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // Hardlight (Same as overwrite all)

    // The following are used internally.
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ZERO),      BF(ONE)          }, // Overwrite - overwrite the destination.
    { BO(ADD),         BF(ONE),           BF(ZERO),        BO(ADD), BF(ONE),       BF(ZERO)         }, // OverwriteAll - overwrite the destination (including alpha).
    { BO(ADD),         BF(ONE),           BF(ONE),         BO(ADD), BF(ONE),       BF(ONE)          }, // FullAdditive - add all components together, without multiplication.
    { BO(ADD),         BF(SRCALPHA),      BF(INVSRCALPHA), BO(ADD), BF(ONE),       BF(INVSRCALPHA)  }, // FilterBlend
    { BO(ADD),         BF(ZERO),          BF(ONE),         BO(ADD), BF(ZERO),      BF(ONE)          }, // Ignore - leaves dest unchanged.
};                                           

#undef BO
#undef BF

void HAL::PushBlendMode(BlendPrimitive* prim)
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    BlendMode mode = prim->GetBlendMode();
    BlendStackEntry e = { prim, 0 };
    SF_DEBUG_ASSERT(prim != 0, "Unexpected NULL BlendPrimitive in HAL::PushBlendMode.");
    BlendModeStack.PushBack(e);

    applyBlendMode(mode, false, (HALState& HS_InRenderTarget) != 0 );
}

void HAL::PopBlendMode()
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    SF_DEBUG_ASSERT(BlendModeStack.GetSize() != 0, "Blend mode stack was unexpectedly empty during PopBlendMode.");
    BlendStackEntry e = BlendModeStack.Back();
    BlendModeStack.PopBack();

    applyBlendMode(getLastBlendModeOrDefault(), false, (HALState& HS_InRenderTarget) != 0 );
}

void HAL::SetFullSceneTargetBlend(bool enable)
{
    HALState &= ~HS_BlendTarget;
    if (enable)
        HALState |= HS_BlendTarget;
}

void HAL::applyBlendMode(BlendMode mode, bool sourceAc, bool forceAc)
{
    // Check for redundant setting of blend states.
    if (CurrentBlendState.Mode == mode &&
        CurrentBlendState.SourceAc == sourceAc &&
        CurrentBlendState.ForceAc == forceAc )
    {
        return;
    }

    SF_DEBUG_ASSERT1(((unsigned) mode) < Blend_Count, "Invalid blend mode set (%d)", mode);

    // For release, just set a default blend mode
    if (((unsigned) mode) >= Blend_Count)
        mode = Blend_None;

#if !defined(SF_BUILD_SHIPPING)
    const char* eventString = 0;
    switch(mode)
    {
       default:
        case Blend_None:        eventString = "HAL::applyBlendMode(None)"; break;
        case Blend_Normal:      eventString = "HAL::applyBlendMode(Normal)"; break;
        case Blend_Layer:       eventString = "HAL::applyBlendMode(Layer)"; break;
        case Blend_Multiply:    eventString = "HAL::applyBlendMode(Multiply)"; break;
        case Blend_Screen:      eventString = "HAL::applyBlendMode(Screen)"; break;
        case Blend_Lighten:     eventString = "HAL::applyBlendMode(Lighten)"; break;
        case Blend_Darken:      eventString = "HAL::applyBlendMode(Darken)"; break;
        case Blend_Difference:  eventString = "HAL::applyBlendMode(Difference)"; break;
        case Blend_Add:         eventString = "HAL::applyBlendMode(Add)"; break;       
        case Blend_Subtract:    eventString = "HAL::applyBlendMode(Subtract)"; break;
        case Blend_Invert:      eventString = "HAL::applyBlendMode(Invert)"; break;
        case Blend_Alpha:       eventString = "HAL::applyBlendMode(Alpha)"; break;
        case Blend_Erase:       eventString = "HAL::applyBlendMode(Erase)"; break;
        case Blend_Overlay:     eventString = "HAL::applyBlendMode(Overlay)"; break;
        case Blend_HardLight:   eventString = "HAL::applyBlendMode(HardLight)"; break;
        case Blend_Overwrite:   eventString = "HAL::applyBlendMode(Overwrite)"; break;
        case Blend_OverwriteAll:eventString = "HAL::applyBlendMode(OverwriteAll)"; break;
        case Blend_FullAdditive:eventString = "HAL::applyBlendMode(FullAdditive)"; break;
        case Blend_FilterBlend: eventString = "HAL::applyBlendMode(FilterBlend)"; break;
        case Blend_Ignore:      eventString = "HAL::applyBlendMode(Ignore)"; break;
    }
    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyBlend), eventString);
#endif

    // Check for overriding
    mode = Profiler.GetBlendMode(mode);

    // Multiply requires different fill mode, save it in the HAL's fill flags.
    FillFlags &= ~(FF_BlendMask);
    if ( mode == Blend_Multiply || mode == Blend_Screen )
        FillFlags |= FF_Multiply;
    else if (mode == Blend_Invert)
        FillFlags |= FF_Invert;

    // Apply or remove blending fill flag.
    if (mode > Blend_Normal)
        FillFlags |= FF_Blending;
    else
        FillFlags &= ~FF_Blending;

    // Now actually apply it.
    CurrentBlendState.Mode = mode;
    CurrentBlendState.SourceAc = sourceAc;
    CurrentBlendState.ForceAc = forceAc;
    applyBlendModeImpl(mode, sourceAc, forceAc);
}

void HAL::applyBlendMode(const HALBlendState& state)
{
    applyBlendMode(state.Mode, state.SourceAc, state.ForceAc);
}

void HAL::applyBlendModeEnable(bool enabled)
{
    if (CurrentBlendState.BlendEnable != enabled)
    {
        applyBlendModeEnableImpl(enabled);
        CurrentBlendState.BlendEnable = enabled;
    }
}

BlendMode HAL::getLastBlendModeOrDefault() const
{
    if (BlendModeStack.GetSize()>=1)
        return BlendModeStack.Back().pPrimitive->GetBlendMode();
    return Blend_Normal;
}

// Just to make things easier to read:
#define IV(x) EnableIgnore_##x
#define DSF(x) DepthStencilFunction_##x
#define SO(x) StencilOp_##x
HAL::HALDepthStencilDescriptor HAL::DepthStencilModeTable[DepthStencil_Count] =
{
    // ZTest,      ZWrite,      Stencil,      ColorW,       DepthFn,     StencilFn,      StencilPassOp, StencilFailOp, StencilZFailOp
    { IV(Ignore),  IV(Ignore),  IV(Ignore),   IV(Ignore),   DSF(Ignore), DSF(Ignore),    SO(Ignore),    SO(Ignore),    SO(Ignore)  },  // Invalid - must be all ignores (used as a 'force')
    { IV(Off),     IV(Off),     IV(Off),      IV(On),       DSF(Ignore), DSF(Ignore),    SO(Ignore),    SO(Ignore),    SO(Ignore)  },  // Disabled
    { IV(Off),     IV(Off),     IV(On),       IV(Off),      DSF(Ignore), DSF(Always),    SO(Replace),   SO(Ignore),    SO(Ignore)  },  // StencilClear
    { IV(Off),     IV(Off),     IV(On),       IV(Off),      DSF(Ignore), DSF(LessEqual), SO(Replace),   SO(Keep),      SO(Ignore)  },  // StencilClearHigher
    { IV(Off),     IV(Off),     IV(On),       IV(Off),      DSF(Ignore), DSF(Equal),     SO(Increment), SO(Keep),      SO(Ignore)  },  // StencilIncrementEqual
    { IV(Off),     IV(Off),     IV(On),       IV(On),       DSF(Ignore), DSF(LessEqual), SO(Keep),      SO(Keep),      SO(Ignore)  },  // StencilTestLessEqual
    { IV(Off),     IV(On),      IV(Off),      IV(On),       DSF(Always), DSF(Ignore),    SO(Ignore),    SO(Ignore),    SO(Ignore)  },  // DepthWrite
    { IV(On),      IV(Off),     IV(Off),      IV(On),       DSF(Equal),  DSF(Ignore),    SO(Ignore),    SO(Ignore),    SO(Ignore)  },  // DepthTestEqual
};
#undef IV
#undef DSF
#undef SO

void HAL::drawMaskClearRectangles(const HMatrix* matrices, UPInt count)
{
    UPInt baseCount = 0;
    while (count > 0)
    {
        static const int MaximumClearRects = 32;
        UPInt passCount = Alg::Min<UPInt>(count, MaximumClearRects);
        Matrix2F convertedMatrices[MaximumClearRects];
        for(unsigned mtx = 0; mtx < passCount; ++mtx)
        {
            convertedMatrices[mtx].SetToAppend(matrices[baseCount + mtx].GetMatrix2D(), GetMatrices()->UserView);
        }
        drawMaskClearRectangles(convertedMatrices, passCount);
        count -= passCount;
        baseCount += passCount;
    }
}

void HAL::beginMaskDisplay()
{
    SF_DEBUG_ASSERT(MaskStackTop == 0, "No masks should be on the stack before calling beginMaskDisplay." );
    StencilChecked  = 0;
    StencilAvailable= 0;
    MultiBitStencil = 0;
    DepthBufferAvailable = 0;
    HALState &= ~HS_DrawingMask;
}

void HAL::endMaskDisplay()
{
    SF_DEBUG_ASSERT(MaskStackTop == 0, "All masks should be on removed the mask stack when calling endMaskDisplay.");
    MaskStackTop = 0;
    MaskStack.Clear();
}

void HAL::applyRasterMode(RasterModeType mode)
{
    if (mode != AppliedSceneRasterMode)
    {
        applyRasterModeImpl(mode);
        AppliedSceneRasterMode = mode;
    }
}

void HAL::PrepareCacheable(CacheablePrimitive* prim, bool unprepare)
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    // If we are cached in some way (or we are 'unpreparing').
    if ( prim->GetCacheState() != CacheablePrimitive::Cache_Uncached || unprepare)
    {
        if ( !unprepare )
        {
            // The filter is being prepared, determine whether the results are cached.
            RenderTarget* results[CacheablePrimitive::MaximumCachedResults];
            prim->GetCacheResults(results, CacheablePrimitive::MaximumCachedResults);

            // Make sure that the first result (which is always required) is valid, and has the CacheID relating to
            // this CacheablePrimitive. If other results exist, make sure that they are valid as well.
            bool validCache = true;
            for ( unsigned cr = 0; cr < CacheablePrimitive::MaximumCachedResults; ++cr )
            {
                if ( !results[cr] )
                {
                    validCache = (cr != 0);
                    break;
                }

                if ( results[cr]->GetStatus() == RTS_Lost || results[cr]->GetStatus() == RTS_Unresolved ||
                    results[cr]->GetRenderTargetData()->CacheID != reinterpret_cast<UPInt>(prim) )
                {
                    validCache = false;
                    break;
                }
            }

            // Must increase the prep index always, so we know when to remove the prepare filter.
            CacheablePrepIndex++;

            // Cache isn't valid, uncache the results.
            if (!validCache)
            {
                prim->SetCacheResults(CacheablePrimitive::Cache_Uncached, 0, 0);
                return;
            }

            // If there are valid cache results, then we must set the queue processor to skip
            // anything in the queue, except other cacheable prepares. Because the primitive is cached,
            // we do not need to prepare any geometry inside it, as it should be contained within 
            // the cached results. 
            if (CacheablePrepStart < 0)
            {
                for ( unsigned cr = 0; cr < CacheablePrimitive::MaximumCachedResults; ++cr )
                {
                    if ( results[cr] )
                        results[cr]->SetInUse(RTUse_InUse);
                }
                GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_CacheableOnly);
                CacheablePrepStart = CacheablePrepIndex;
            }
        }
        else if ( CacheablePrepIndex >= 0 )
        {
            // When cacheables are 'unprepared' we need to set the render queue to continue preparing
            // primitives from that point on. However, if we are in the prepass, make sure it is
            // still only processing cacheables.
            if (CacheablePrepIndex == CacheablePrepStart)
            {
                CacheablePrepStart = -1;
                if (CurrentPass != Display_Prepass)
                    GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_All);
                else
                    GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_CacheableOnly);
            }

            // Make sure that cache results have their SetInUse flags set properly
            --CacheablePrepIndex;
        }
    }
    else if (prim->GetCacheState() == CacheablePrimitive::Cache_Uncached)
    {
        if (CurrentPass == Display_Prepass)
        {
            // Uncached, and in the prepass, or 'both', need to prepare everything.
            GetRQProcessor().SetQueuePrepareFilter(RenderQueueProcessor::QPF_All);
        }

        // If we're currently within a cached filter, and we hit an uncached primitive, we still increase the
        // cached prep index, just so we know where the real cached primitive ends.
        if (!unprepare && CacheablePrepIndex >= 0)
            CacheablePrepIndex++;
    }
}

void HAL::PushFilters(FilterPrimitive* prim)
{
    GetEvent(Event_Filter).Begin(__FUNCTION__);
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    FilterStackEntry e = {prim, 0};

    // Do not render filters if the profile does not support it (unfiltered content will be rendered).
    if (!shouldRenderFilters(prim))
    {
        FilterStack.PushBack(e);
        return;
    }

    if ( (HALState & HS_InCachedTarget) )
    {
        FilterStack.PushBack(e);
        return;
    }

    // Disable masking from previous target, if this filter primitive doesn't have any masking.
    if ( MaskStackTop != 0 && !prim->GetMaskPresent() && prim->GetCacheState() != CacheablePrimitive::Cache_Target )
        applyDepthStencilMode(HAL::DepthStencil_Disabled, MaskStackTop);

    // Apply the solid raster mode, we do not want to draw filtered content in wireframe/point, as if it gets cached, it will stay that way
    applyRasterModeImpl(RasterMode_Solid);

    HALState |= HS_DrawingFilter;

    if ( prim->GetCacheState() ==  CacheablePrimitive::Cache_Uncached )
    {
        // Draw the filter from scratch.
        const Matrix2F& m = e.pPrimitive->GetAreaMatrix().GetMatrix2D();
        e.pRenderTarget = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), prim->GetMaskPresent());
        RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());
        PushRenderTarget(frameRect, e.pRenderTarget);
        applyBlendMode(Blend_Normal, false, true);

        // If this primitive has masking, then clear the entire area to the current mask level, because 
        // the depth stencil target may be different, and thus does not contain the previously written values.
        if (prim->GetMaskPresent() && checkDepthStencilBufferCaps())
        {
            if (StencilAvailable)
                applyDepthStencilMode(HAL::DepthStencil_StencilClear, MaskStackTop);
            else if (DepthBufferAvailable)
                applyDepthStencilMode(HAL::DepthStencil_DepthWrite, MaskStackTop);

            const Matrix2F& viewMtx = GetMatrices()->GetFullViewportMatrix(e.pRenderTarget->GetBufferSize());
            drawMaskClearRectangles(&viewMtx, 1);


            // Although we cleared the mask, we might not be rendering it immediately, so put depth/stencil back to normal.
            applyDepthStencilMode(HAL::DepthStencil_Disabled, MaskStackTop);
        }
    }
    else
    {
        // Drawing a cached filter, ignore all draw calls until the corresponding PopFilters.
        // Keep track of the level at which we need to draw the cached filter, by adding entries to the stack.
        HALState |= HS_InCachedFilter;
        CacheableIndex = (int)FilterStack.GetSize();
        GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_CacheableOnly);
    }
    FilterStack.PushBack(e);
}

void HAL::GetStats(Stats* pstats, bool clear)
{
	// Cannot call GetStats within a scene - rendering may not be flushed, primitives
	// will not be drawn, and stats will be inaccurate.
	SF_ASSERT( (HALState & HS_InScene) == 0 );
	*pstats = AccumulatedStats;
	if (clear)
		AccumulatedStats.Clear();
}


RenderEvent& HAL::GetEvent( EventType )
{
    static RenderEvent defaultEvent;
    return defaultEvent;
}

void HAL::PopFilters()
{
    // Do not render filters if the profile does not support it.
    ScopedRenderEvent GPUEvent(GetEvent(Event_Filter), __FUNCTION__, false);
    if (!shouldRenderFilters(FilterStack.Back().pPrimitive))
    {
        FilterStack.Pop();
        return;
    }

    FilterStackEntry e;
    e = FilterStack.Pop();

    // If doing a cached filter, and haven't reached the level at which it will be displayed, ignore the pop.
    // Also, if doing a cached blend, ignore all filter calls.
    if ( (HALState&HS_InCachedBlend) ||
         ((HALState&HS_InCachedFilter) && (CacheableIndex < (int)FilterStack.GetSize())) )
    {
        return;
    }

    CacheableIndex = -1;
    if ( HALState & HS_InCachedTarget )
    {
        drawCachedFilter(e.pPrimitive);
        GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);
        HALState &= ~HS_InCachedTarget;
    }
    else
    {
        drawUncachedFilter(e);
    }

    if ( FilterStack.GetSize() == 0 )
        HALState &= ~HS_DrawingFilter;
}


void HAL::PushView3D( const Matrix3F &m )
{
    Matrices->View3D = m; 
    Matrices->UVPOChanged = 1; 
    ViewMatrix3DStack.PushBack(m);
}


void HAL::PushProj3D( const Matrix4F &m )
{
    Matrices->Proj3D = m; 
    Matrices->UVPOChanged = 1; 
    ProjectionMatrix3DStack.PushBack(m);
}

void HAL::PopView3D()
{
    ViewMatrix3DStack.PopBack(); 
    Matrices->View3D = ViewMatrix3DStack.GetSize() ? ViewMatrix3DStack.Back() : Matrix3F::Identity;  
    Matrices->UVPOChanged = 1;
}

void HAL::PopProj3D()
{
    ProjectionMatrix3DStack.PopBack(); 
    Matrices->Proj3D = ProjectionMatrix3DStack.GetSize() ? ProjectionMatrix3DStack.Back() : Matrix4F::Identity;  
    Matrices->UVPOChanged = 1;
}

unsigned HAL::DrawableCommandGetFlags(const DICommand* pcmd) const
{
    if ( !pcmd )
        return 0;
    switch(pcmd->GetType())
    {
        case DICommandType_Map:             return DICommand::RC_GPU | DICommand::RC_GPU_NoRT;
        case DICommandType_Unmap:           return DICommand::RC_GPU | DICommand::RC_GPU_NoRT;
        case DICommandType_CreateTexture:   return DICommand::RC_GPU | DICommand::RC_GPU_NoRT;

        case DICommandType_Clear:           return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_ApplyFilter:     return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_Draw:            return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_CopyChannel:     return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_CopyPixels:      return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_ColorTransform:  return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_Compare:         return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_FillRect:        return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_FloodFill:       return 0;
        case DICommandType_Merge:           return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_Noise:           return 0;
        case DICommandType_PaletteMap:      return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_PerlinNoise:     return 0;
        case DICommandType_Scroll:          return DICommand::RC_GPU | DICommand::RC_GPUPreference;
        case DICommandType_Threshold:       return DICommand::RC_GPU;
    default:
        return 0;
    }
}

void HAL::PushUserData(const UserDataState::Data* data)
{
    UserDataStack.PushBack(data);
}

void HAL::PopUserData()
{
    UserDataStack.Pop();
}

}} // Scaleform::Render
