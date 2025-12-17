/**************************************************************************

Filename    :   Platform_RenderThread.cpp
Content     :   Default RenderThread implementation used by applications.
Created     :   Jan 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform_RenderThread.h"
#include "Render/Renderer2DImpl.h"
#include "Render/Render_GlyphCache.h"
#include "Render/Render_ShapeDataFloatMP.h"

namespace Scaleform { namespace Platform {

//------------------------------------------------------------------------
// ***** RenderThread

RenderThread::RenderThread(RTCommandQueue::ThreadingType threadingType)
    : RenderHALThread(threadingType), ViewportFlags(0)
{
}

int RenderThread::Run()
{
    int result = RenderHALThread::Run();
    pRenderer.Clear();
    return result;
}

bool RenderThread::initGraphics(const Platform::ViewConfig& config, Device::Window* window,
                                ThreadId renderThreadId)
{
    // Create Renderer2D before HAL initialization to allow cache configs.
    pRenderer = *new Render::Renderer2D(getHAL());    
    pRenderer->GetGlyphCacheConfig()->SetParams(GCParams);
    pRenderer->SetToleranceParams(TolParams);

    if (!RenderHALThread::initGraphics(config, window, renderThreadId))
    {
        pRenderer.Clear();
        return false;
    }    
    createCursorPrimitives(getHAL());
    Windows.PushBack(createWindowData(window));
    return true;    
}

void RenderThread::notifyShutdown(DHContainerType& arr) const
{
    for (unsigned i = 0; i < arr.GetSize(); i++)
    {
        DisplayHandleDesc& desc = arr[i];
        if (desc.pOnDisplay)
        {
            desc.pOnDisplay->OnShutdown(pRenderer);
            desc.pOnDisplay = 0;
        }
    }
}

void RenderThread::destroyGraphics()
{   
    for ( unsigned i = 0; i < Cursor_Type_Count; ++i )
    {
        if ( !CursorMats[i].IsNull())
            CursorMats[i].Clear();
        if ( CursorPrims[i] )
            CursorPrims[i].Clear();
    }

    for (unsigned i = 0; i < Windows.GetSize(); i++)
    {
        notifyShutdown(((DisplayWindow*)Windows[i])->NormalHandles);
        notifyShutdown(((DisplayWindow*)Windows[i])->OverlayHandles);
        delete Windows[i];
    }

    Windows.Clear();
    pRenderer.Clear();
    RenderHALThread::destroyGraphics();
}

void RenderThread::setGlyphCacheParams(const Render::GlyphCacheParams& params)
{
    GCParams = params;
    if (pRenderer)
        pRenderer->GetGlyphCacheConfig()->SetParams(GCParams);
}

void RenderThread::setToleranceParams(const Render::ToleranceParams& params)
{
    TolParams = params;
    if (pRenderer)
	{
        pRenderer->SetToleranceParams(TolParams);
		pRenderer->GetHAL()->GetMeshCache().ClearCache();
	}
}

void RenderThread::getToleranceParams(Render::ToleranceParams* params)
{
	if (pRenderer && params)
		*params = pRenderer->GetToleranceParams();
}

void RenderThread::addDisplayHandle(const DisplayHandleType& root, DisplayHandleCategory cat, bool clearBeforeAdd,
                                    DisplayCallBack* dcb, Device::Window* pwindow)
{
    DisplayHandleDesc desc(cat, root, dcb, pwindow ? pwindow : pWindow);
    DisplayWindow* pwin = 0;
    for (unsigned i = 0; i < Windows.GetSize(); i++)
        if (Windows[i]->pWindow == desc.pWindow)
        {
            pwin = (DisplayWindow*)Windows[i];
            break;
        }
    if (!pwin)
    {
        pwin = new DisplayWindow(desc.pWindow);
        Windows.PushBack(pwin);
    }
    else if (clearBeforeAdd)
        clearDisplayHandles(pwin, cat);

    if (cat == DHCAT_Overlay)
        pwin->OverlayHandles.PushBack(desc);
    else
        pwin->NormalHandles.PushBack(desc);
}

void RenderThread::removeDisplayHandle(const DisplayHandleType& root,
                                       DisplayHandleCategory cat)
{
    for (unsigned j = 0; j < Windows.GetSize(); j++)
    {
        DHContainerType& container = ((DisplayWindow*)Windows[j])->getDHContainer(cat);

        int remidx = -1;
        // Iterate over container to find the handle and remove it
        for (unsigned i = 0; i < container.GetSize(); i++)
        {
            const DisplayHandleDesc& desc = container[i];
            if (desc.hRoot == root)
            {
                remidx = i;
                break;
            }
        }
        if (remidx != -1)
        {
            const DisplayHandleDesc& desc = container[remidx];
            if (desc.pOnDisplay)
                desc.pOnDisplay->OnShutdown(pRenderer);
            container.RemoveAt(remidx);
        }
    }
}

void RenderThread::popDisplayHandle(DisplayHandleCategory cat)
{
    DHContainerType& container = ((DisplayWindow*)Windows[0])->getDHContainer(cat);

    if (container.GetSize() > 0)
    {
        const DisplayHandleDesc& desc = container[container.GetSize() - 1];
        if (desc.pOnDisplay)
            desc.pOnDisplay->OnShutdown(pRenderer);
        container.PopBack();
    }
}

void RenderThread::clearDisplayHandles(DisplayHandleCategory cat)
{
    for (unsigned j = 0; j < Windows.GetSize(); j++)
    {
        DHContainerType& container = ((DisplayWindow*)Windows[j])->getDHContainer(cat);

        for (unsigned i = 0; i < container.GetSize(); i++)
        {
            const DisplayHandleDesc& desc = container[i];
            if (desc.pOnDisplay)
                desc.pOnDisplay->OnShutdown(pRenderer);
        }
        container.Clear();
    }
}

void RenderThread::clearDisplayHandles(DisplayWindow* pDispWin, DisplayHandleCategory cat)
{
    DHContainerType& container = pDispWin->getDHContainer(cat);

    for (unsigned i = 0; i < container.GetSize(); i++)
    {
        const DisplayHandleDesc& desc = container[i];
        if (desc.pOnDisplay)
            desc.pOnDisplay->OnShutdown(pRenderer);
    }
    container.Clear();
}

RenderThread::DHContainerType& RenderThread::DisplayWindow::getDHContainer(DisplayHandleCategory cat)
{
    switch (cat)
    {
    case DHCAT_Overlay: return OverlayHandles;
    default:            return NormalHandles;
    }
}

void RenderThread::finishFrame()
{
    pRenderer->FinishFrame();
}

void RenderThread::drawFrame()
{
    {   // Scope timer should not include AmpServer::AdvanceFrame, where stats are reported
        SF_AMP_SCOPE_RENDER_TIMER("RenderThread::drawFrame", Amp_Profile_Level_Low);

        DrawFrameDone.PulseEvent();

        updateDeviceStatus();
        if (Status != Device_Ready)
            return;

        pDevice->BeginFrame();
        pRenderer->BeginFrame();

        for (unsigned j = 0; j < Windows.GetSize(); j++)
        {
            PresentMode = (j == Windows.GetSize()-1) ? Present_LastWindow : 0;
            drawFrame((DisplayWindow*)Windows[j]);
        }

        pRenderer->EndFrame();

        Frames++;
    }

    // Placing the AMP update call here avoids the need 
    // to explicitly call it every frame from the main loop
    SF_AMP_CODE(AmpServer::GetInstance().AdvanceFrame();)
}

void RenderThread::drawFrame(DisplayWindow* pDispWin)
{
    pDevice->SetWindow(pDispWin->pWindow);

    if (pDispWin->VConfig.HasFlag(View_Stereo))
    {
        Render::HAL* pHal = pRenderer->GetHAL();
        pHal->SetStereoDisplay(Render::StereoLeft, 0);
        drawFrame1(pDispWin, false);
        if (pDispWin->VConfig.StereoFormat == Stereo_Standard)
        {
            pDevice->PresentFrame(Render::StereoLeft);
            PresentMode |= Render::StereoRight;
        }
        else
            PresentMode |= Render::StereoLeft | Render::StereoRight;

        pHal->SetStereoDisplay(Render::StereoRight, 1);
        drawFrame1(pDispWin, true);
    }
    else
    {
        PresentMode |= Render::StereoCenter;
        drawFrame1(pDispWin, false);
    }

    // Present the back buffer contents to the display.
    pDevice->PresentFrame(PresentMode);
}

void RenderThread::drawFrame1(DisplayWindow* pDispWin, bool capture)
{
    Render::HAL * pHal = pRenderer->GetHAL();

	Render::Viewport vp(pDispWin->ViewSize.Width, pDispWin->ViewSize.Height,
		0, 0, pDispWin->ViewSize.Width, pDispWin->ViewSize.Height, ViewportFlags);

    pHal->GetProfiler().SetProfileMode(getProfileMode());
    pHal->GetProfiler().SetProfileFlags(getProfileFlags());
    pHal->GetProfiler().SetHighlightedBatch(getProfileBatchHighlight());
    pHal->SetRasterMode(!Wireframe ? Render::HAL::RasterMode_Solid : Render::HAL::RasterMode_Wireframe);

    // If prepass is required, render it now. Assume that overlays and cursor handles do
    // not require a prepass, and thus they will not be rendered here, only in the final pass.
    // If they used render targets, they would be required to have a prepass on systems that 
    // require it.
    if ( pHal->IsPrepassRequired() )
    {
        pHal->SetDisplayPass(Render::Display_Prepass);
        pHal->BeginScene();

        // Normal handles
        for (unsigned i = 0; i < pDispWin->NormalHandles.GetSize(); i++)
        {
            DisplayHandleDesc& desc = pDispWin->NormalHandles[i];
            drawDisplayHandle(desc, vp, capture);
        }
        pHal->EndScene();

        // Set the final display pass to render in the next pass.
        pHal->SetDisplayPass(Render::Display_Final);
    }

    pHal->BeginScene();
	{
		if (!(ViewportFlags & Render::Viewport::View_Stereo_AnySplit) || pHal->GetMatrices()->S3DDisplay != Render::StereoRight)
        {
            if (!(ViewportFlags & Render::Viewport::View_NoClear))
            {
                pDevice->Clear(getBackgroundColor().ToColor32());
            }
        }

		// Normal handles
		for (unsigned i = 0; i < pDispWin->NormalHandles.GetSize(); i++)
		{
			DisplayHandleDesc& desc = pDispWin->NormalHandles[i];
			drawDisplayHandle(desc, vp, capture);
		}
	}
	pHal->EndScene();

	// Get stats after rendering all 'regular' display handles.
    if (pDispWin->NormalHandles.GetSize())
	{
		bool resetStats = true;
		SF_AMP_CODE(resetStats = !AmpServer::GetInstance().IsValidConnection());
		Lock::Locker lock(&RenderStatsLock);
		pHal->GetStats(&RenderStats, resetStats);
        pHal->GetMeshCache().GetStats(&MeshCacheStats);
	}

	// Draw cursors and overlays (HUD).
    pHal->GetProfiler().SetProfileMode(Render::Profile_None);    // Do not apply profile views to the overlays.
    pHal->SetRasterMode(Render::HAL::RasterMode_Solid);          // Always render overlays in solid.
	pHal->BeginScene();
	{
		// Render cursors.
		CursorType ctype = (CursorState.IsSoftware() && CursorState.IsCursorEnabled()) ? CursorState.GetCursorType() : Cursor_Hidden;
		if ( ctype != Cursor_Hidden && CursorPrims[ctype] )
		{
			pHal->BeginDisplay(0, vp);
			pHal->Draw(CursorPrims[ctype]);
			pHal->EndDisplay();
		}

		// Overlay handles
		for (unsigned i = 0; i < pDispWin->OverlayHandles.GetSize(); i++)
		{
			DisplayHandleDesc& desc = pDispWin->OverlayHandles[i];
			drawDisplayHandle(desc, vp, capture);
		}
	}
	pHal->EndScene();

    // To clear stats after cursors/overlays
    {
        bool resetStats = true;
        SF_AMP_CODE(resetStats = !AmpServer::GetInstance().IsValidConnection());
        Render::HAL::Stats s;
        pHal->GetStats(&s, resetStats);
    }
}

void RenderThread::drawDisplayHandle(DisplayHandleDesc& desc, const Render::Viewport& vp, bool capture)
{
    SF_AMP_SCOPE_RENDER_TIMER("RenderThread::drawDisplayHandle", Amp_Profile_Level_Low);
    bool captureHasData = capture || desc.hRoot.NextCapture(pRenderer->GetContextNotify());
    
    if (captureHasData && (desc.hRoot.GetRenderEntry() != 0))
    {
        pDevice->SetWindow(desc.pWindow);

        // If the TreeRoot has a viewport, BeginDisplay was called twice, once here, and once in TreeCacheRoot::Draw.
        const Render::TreeRoot::NodeData* nodeData = desc.hRoot.GetRenderEntry()->GetDisplayData();
        bool hasViewport = nodeData->HasViewport();

        // BGColor alpha 0 => no clear in BeginDisplay().
        if ( !hasViewport )
            pRenderer->BeginDisplay(0, vp);
        pRenderer->Display(desc.hRoot);
        if ( !hasViewport )
            pRenderer->EndDisplay();

        if (desc.pOnDisplay)
        {
            pRenderer->BeginDisplay(0, vp);
            desc.pOnDisplay->OnDisplay(pRenderer);
            pRenderer->EndDisplay();
        }
    }
}

void RenderThread::createCursorPrimitives(Render::HAL* pHal )
{
    using namespace Render;

    if ( !pHal )
    {
        SF_ASSERT(0);
        return;
    }

    for ( int cursor = 0; cursor < Platform::Cursor_Type_Count; cursor++ )
    {
        // Hidden cursor is just 'NULL'.
        if ( cursor == Platform::Cursor_Hidden )
            continue;

        PrimitiveFillData       fillData(PrimFill_VColor, &VertexXY16iCF32::Format );
        Ptr<PrimitiveFill>      fill = *pHal->CreatePrimitiveFill(fillData);
        Ptr<Primitive>          prim = *SF_NEW Primitive(pHal, fill);
        Ptr<ShapeDataFloatMP> pshapeData = *SF_NEW ShapeDataFloatMP;
        pshapeData->StartLayer();
        pshapeData->StartPath(1, 1, 1);

        switch(cursor)
        {
        case Platform::Cursor_Arrow:
            pshapeData->MoveTo( 0, 0);
            pshapeData->LineTo( 0, 16);
            pshapeData->LineTo( 5, 12);
            pshapeData->LineTo( 8, 20);
            pshapeData->LineTo(12, 19);
            pshapeData->LineTo( 7, 12);
            pshapeData->LineTo(12, 12);
            pshapeData->ClosePath();
            break;

        case Platform::Cursor_Hand:
            pshapeData->MoveTo( 0, 9);
            pshapeData->LineTo( 0,12);
            pshapeData->LineTo( 5,19);
            pshapeData->LineTo( 5,22);
            pshapeData->LineTo(14,22);
            pshapeData->LineTo(16,16);
            pshapeData->LineTo(16, 9);
            pshapeData->LineTo(14, 7);
            pshapeData->LineTo(13, 7);
            pshapeData->LineTo(13,11);
            pshapeData->LineTo(13, 7);
            pshapeData->LineTo(12, 6);
            pshapeData->LineTo(10, 6);
            pshapeData->LineTo(10,10);
            pshapeData->LineTo(10, 6);
            pshapeData->LineTo( 9, 5);
            pshapeData->LineTo( 7, 5);
            pshapeData->LineTo( 7,10);
            pshapeData->LineTo( 7, 1);
            pshapeData->LineTo( 6, 0);
            pshapeData->LineTo( 5, 0);
            pshapeData->LineTo( 4, 1);
            pshapeData->LineTo( 4,13);
            pshapeData->LineTo( 4,10);
            pshapeData->LineTo( 3,10);
            pshapeData->LineTo( 2, 9);
            pshapeData->ClosePath();
            break;

        case Platform::Cursor_IBeam:
            pshapeData->MoveTo( 0,  0);
            pshapeData->LineTo( 2,  0);
            pshapeData->EndPath();
            pshapeData->StartPath(1, 1, 1);
            pshapeData->MoveTo( 4,  0);
            pshapeData->LineTo( 6,  0);
            pshapeData->EndPath();
            pshapeData->StartPath(1, 1, 1);
            pshapeData->MoveTo( 3,  1);
            pshapeData->LineTo( 3, 14);
            pshapeData->EndPath();
            pshapeData->StartPath(1, 1, 1);
            pshapeData->MoveTo( 0, 15);
            pshapeData->LineTo( 2, 15);
            pshapeData->EndPath();
            pshapeData->StartPath(1, 1, 1);
            pshapeData->MoveTo( 4, 15);
            pshapeData->LineTo( 6, 15);
            break;
        default:
            SF_ASSERT(0);
            break;
        }
        pshapeData->EndPath();

        FillStyleType cursorFill;
        cursorFill.Color = (UInt32)(Color::Gray|Color::Alpha100);
        cursorFill.pFill = 0;
        pshapeData->AddFillStyle(cursorFill);
        pshapeData->AddStrokeStyle(1.0f, 0, 0, (UInt32)(Color::Red|Color::Alpha100));
        pshapeData->CountLayers();

        Ptr<Mesh> mesh = *SF_NEW Render::Mesh(pRenderer->GetImpl(), pshapeData, Matrix2F());
        CursorPrims[cursor] = prim;
        CursorMats[cursor]  = pRenderer->GetImpl()->GetMatrixPool().CreateMatrix();
        prim->Insert(0, mesh, CursorMats[cursor] );
    }
}

unsigned RenderThread::GetGlyphRasterizationCount() const
{
    return pRenderer->GetImpl()->GetGlyphCache()->GetRasterizationCount();
}

void RenderThread::ResetRasterizationCount()
{
    pRenderer->GetImpl()->GetGlyphCache()->ResetRasterizationCount();
}


void RenderThread::GetRenderInterfaces(Render::Interfaces* p)
{
    p->Clear();
    p->pRenderer2D = pRenderer;
    if (pRenderer)
    {
        p->pHAL = pRenderer->GetHAL();
        p->pTextureManager = p->pHAL->GetTextureManager();
        p->RenderThreadID = p->pHAL->GetRenderThreadId();
    }
}

}} // Scaleform::Platform
