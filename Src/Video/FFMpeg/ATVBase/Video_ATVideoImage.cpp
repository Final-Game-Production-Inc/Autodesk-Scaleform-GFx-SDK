/**************************************************************************

Filename    :   Video_ATVideoImage.cpp
Content     :   Video image texture update (FFmpeg backend)
Created     :   October 2026
Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/FFMpeg/ATVBase/Video_ATVideoImage.h"
#include "Video/FFMpeg/ATVBase/Video_ATVVideoPlayerImpl.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Timer.h"
#include "Render/Render_DebugEnv.h"

namespace Scaleform { namespace GFx { namespace Video {

    static const unsigned char GClearYUV[3] = { 0x10, 0x80, 0x80 }; // black

using namespace Render;

//////////////////////////////////////////////////////////////////////////
//

Texture* ATVVideoImage::GetTexture(TextureManager* ptexman)
{
    if (pTexture)
    {
        SF_ASSERT(pTexture->GetTextureManager() == ptexman);
        return pTexture;
    }

    Texture* ptex = ptexman->CreateTexture(
        GetFormat(), GetMipmapCount(), GetSize(), GetUse(), this);
    { static int n=0; if (n++ < 3) { fprintf(stderr, "[VIDEO] GetTexture fmt=%d size=%dx%d -> %p stages=%u\n", (int)GetFormat(), GetSize().Width, GetSize().Height, (void*)ptex, ptex? ptex->GetTextureStageCount():0); fflush(stderr);} }
    SF_ASSERT(ptex);
    initTexture_NoAddRef(ptex);
    return ptex;
}

bool ATVVideoImage::Decode(ImageData* pdest, CopyScanlineFunc func, void* parg) const
{
    SF_UNUSED2(func, parg);

    if (!pVideoPlayer)
    {
        return false;
    }

    pVideoPlayer->UpdateTextureCalled = false;

    { static int n=0; if (n++ < 3) { fprintf(stderr, "[VIDEO] Decode planes=%u init=%d\n", pdest->GetPlaneCount(), (int)pVideoPlayer->FrameNotInitialized); fflush(stderr);} }

    bool result;
    if (pVideoPlayer->FrameNotInitialized)
    {
        clearImageData(pdest);
        pVideoPlayer->FrameNotInitialized = false;
        result = true;
    }
    else
    {
        // Pull the staged YUV frame from the player; returns false when no new
        // frame is ready (the previous texture content is kept on screen).
        result = pVideoPlayer->CopyFrameData(pdest);
    }

    // GFX_VIDEOPERF: consumption cadence - how often the renderer asks for a frame and
    // how often a staged frame is actually there (the rest means the decoder fell behind).
    static const bool sPerf = Render::IsDebugEnvSet("GFX_VIDEOPERF");
    if (sPerf)
    {
        static UInt64 sStart = 0;
        static unsigned sCalls = 0, sHits = 0;
        const UInt64 now = Timer::GetTicks();
        if (!sStart)
            sStart = now;
        ++sCalls;
        if (result)
            ++sHits;
        if (now - sStart >= 1000000)
        {
            fprintf(stderr, "[ATVPERF] image decode %.1f calls/s, %.1f hits/s\n",
                    (double)sCalls * 1000000.0 / (double)(now - sStart),
                    (double)sHits  * 1000000.0 / (double)(now - sStart));
            fflush(stderr);
            sStart = now;
            sCalls = sHits = 0;
        }
    }
    return result;
}

void ATVVideoImage::clearImageData(ImageData* pdest) const
{
    if (pdest->GetPlaneCount() >= 3)
    {
        ImagePlane y, u, v, a;

        // YUV black = 16,128,128
        pdest->GetPlane(0, &y);
        pdest->GetPlane(1, &u);
        pdest->GetPlane(2, &v);
        Alg::MemUtil::Set(y.pData, GClearYUV[0], y.Pitch * y.Height);
        Alg::MemUtil::Set(u.pData, GClearYUV[1], u.Pitch * u.Height);
        Alg::MemUtil::Set(v.pData, GClearYUV[2], v.Pitch * v.Height);

        if (pdest->GetPlaneCount() >= 4)
        {
            pdest->GetPlane(3, &a);
            Alg::MemUtil::Set(a.pData, 0, a.Pitch * a.Height);
        }
    }
    else
    {
        ImagePlane argb;
        pdest->GetPlane(0, &argb);
        Alg::MemUtil::Set(argb.pData, 0, argb.Pitch * argb.Height);
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

