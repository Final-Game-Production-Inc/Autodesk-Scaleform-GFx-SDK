/**************************************************************************

Filename    :   FxRenderThread.cpp
Content     :   RenderThread class implementation for GFxPlayer. Includes
                extra HUD support and timing logic.
Created     :   Dec, 2010
Authors     :   Art Bolgar, Mike Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxRenderThread.h"


FxRenderThread::FxRenderThread(Scaleform::RTCommandQueue::ThreadingType ttype) 
: Platform::RenderThread(ttype),  
  LastDisplayTicks(0),
  LastDrawFrameTicks(0),
  ProfileMode(Render::Profile_None),
  ProfileFlags(0),
  BatchHighlight(-1)
{
}

void FxRenderThread::drawFrame()
{
    LastDisplayTicksAccum   = 0;
    LastDrawFrameTicksAccum = 0;

    RenderThread::drawFrame();

    LastDisplayTicks = LastDisplayTicksAccum;
    LastDrawFrameTicks = LastDrawFrameTicksAccum;
}

void FxRenderThread::finishFrame()
{
    RenderThread::finishFrame();
}

void FxRenderThread::drawDisplayHandle(Platform::RenderThread::DisplayHandleDesc& desc,
                                       const Render::Viewport& vp, bool capture)
{
    SF_AMP_SCOPE_RENDER_TIMER("FxRenderThread::drawDisplayHandle", Amp_Profile_Level_Low);
    SF_UNUSED(vp);

    bool isOverlay = (desc.Category == Platform::RenderThread::DHCAT_Overlay);

    UInt64 endTicks, startTicks = Timer::GetProfileTicks();
    UInt64 captureTicks;
    UInt64 diplayStartTicks = 0, displayEndTicks = 0;

    Render::HAL* hal = pRenderer->GetHAL();

    bool captureHasData = capture || desc.hRoot.NextCapture(pRenderer->GetContextNotify());
    captureTicks = Timer::GetProfileTicks();

    if (captureHasData && desc.hRoot.GetRenderEntry())
    {
        if (isOverlay) 
        {
            hal->SetUserMatrix(HUDUserMatrix);
        }
        else
        {
            hal->SetUserMatrix(UserMatrix);
        }

        diplayStartTicks = Timer::GetProfileTicks();
        pRenderer->Display(desc.hRoot);
        displayEndTicks = Timer::GetProfileTicks();
    }

    endTicks = Timer::GetProfileTicks();
    if (!isOverlay)
    {
        LastDisplayTicksAccum += (UInt32) ((captureTicks-startTicks) +
            (displayEndTicks-diplayStartTicks));
    }
    LastDrawFrameTicksAccum += (UInt32)(endTicks - startTicks);
}

void FxRenderThread::presentOnResize()
{
    // Present the back buffer contents to the display.
    updateDeviceStatus();
    if ((Status == Platform::Device_Ready) && !VConfig.HasFlag(Platform::View_FSAA))
        pDevice->PresentFrame();
}

/*
void FxRenderThread::RecreateRenderer2D()
{
    pRenderer = NULL;
    pRenderer = *new Render::Renderer2D(pRenderHAL.GetPtr());

#ifndef FXPLAYER_FONTCACHE_SMALL
    Render::GlyphCacheParams fontCacheConfig;
    //Temporarily changing value from defalut of 48 to 100 as Vector Fonts aren't supported yet
    //Otherwise large fonts will be blurry
    fontCacheConfig.MaxSlotHeight  = 100;
    pRenderer->GetGlyphCacheConfig()->SetParams(fontCacheConfig);
#endif

    //pRenderer->GetTolerances().StrokeLowerScale = 0.99f;
    //pRenderer->GetTolerances().StrokeUpperScale = 1.01f;
}
*/

