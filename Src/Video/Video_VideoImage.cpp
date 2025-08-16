/**************************************************************************

Filename    :   Video_VideoImage.cpp
Content     :   
Created     :   Feb, 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Video/Video_VideoImage.h"
#include "Video/Video_VideoPlayerImpl.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace GFx { namespace Video {

	unsigned char GClearYUV[3] = { 0x10, 0x80, 0x80 }; // black

// When CRI skips or we get too far behind in a frame, then CRI resets the frame time
// Scaleform can try to decode the video image multiple times and take a frame meant
// for later. So we try to keep the renderer and decoder in sync to avoid that as much
// as possible.
Uint64 GDecodeTick;
Uint64 GRenderTick;

using namespace Render;

//////////////////////////////////////////////////////////////////////////
//

Texture* VideoImage::GetTexture(TextureManager* ptexman)
{
    if (pTexture)
    {
        SF_ASSERT(pTexture->GetTextureManager() == ptexman);
        return pTexture;
    }

    Texture* ptex = ptexman->CreateTexture(
        GetFormat(), GetMipmapCount(), GetSize(), GetUse(), this);
    SF_ASSERT(ptex);
    initTexture_NoAddRef(ptex);
    return ptex;
}

bool VideoImage::Decode(ImageData* pdest, CopyScanlineFunc func, void* parg) const
{
    SF_UNUSED2(func, parg);

    if (!pVideoPlayer)
	{
        return false;
	}

	pVideoPlayer->UpdateTextureCalled = false;

    if (pVideoPlayer->FrameNotInitialized)
    {
        clearImageData(pdest);
        pVideoPlayer->FrameNotInitialized = false;
        return true;
    }

	if ( GDecodeTick >= GRenderTick )
	{
		return false;
	}
	++GDecodeTick;

    if (pVideoPlayer->GetCriPlayer()->IsNextFrameOnTime())
    {
        if (pdest->GetPlaneCount() >= 3)
        {
            ImagePlane y, u, v, a;
            CriMvYuvBuffers yuvBufs;

            pdest->GetPlane(0, &y);
            yuvBufs.y_imagebuf  = (CriUint8*)y.pData;
            yuvBufs.y_pitch     = (CriUint32)y.Pitch;
            yuvBufs.y_bufsize   = (CriUint32)y.DataSize;
            pdest->GetPlane(1, &u);
            yuvBufs.u_imagebuf  = (CriUint8*)u.pData;
            yuvBufs.u_pitch     = (CriUint32)u.Pitch;
            yuvBufs.u_bufsize   = (CriUint32)u.DataSize;
            pdest->GetPlane(2, &v);
            yuvBufs.v_imagebuf  = (CriUint8*)v.pData;
            yuvBufs.v_pitch     = (CriUint32)v.Pitch;
            yuvBufs.v_bufsize   = (CriUint32)v.DataSize;

            if (pdest->GetPlaneCount() >= 4)
            {
                pdest->GetPlane(3, &a);
                if (pVideoPlayer->IsAlphaVideo())
                {
                    yuvBufs.a_imagebuf  = (CriUint8*)a.pData;
                    yuvBufs.a_pitch     = (CriUint32)a.Pitch;
                    yuvBufs.a_bufsize   = (CriUint32)a.DataSize;
                }
            }
            pVideoPlayer->GetCriPlayer()->GetFrameOnTimeAsYUVBuffers(
                &yuvBufs, pVideoPlayer->FrameInfo);
            checkAndRequestSkipDecoding();
        }
        else
        {
            ImagePlane argb;
            pdest->GetPlane(0, &argb);

            pVideoPlayer->GetCriPlayer()->GetFrameOnTimeAs32bitARGB(
                (CriUint8*)argb.pData, (CriUint32)argb.Pitch, (CriUint32)argb.DataSize,
                pVideoPlayer->FrameInfo);
            checkAndRequestSkipDecoding();
        }
        pVideoPlayer->FrameOnTime = true;

        if (pVideoPlayer->PausedStartup)
        {
            pVideoPlayer->GetCriPlayer()->Pause(TRUE);
            pVideoPlayer->Paused = TRUE;
            pVideoPlayer->PausedStartup = false;
        }
        return true;
    }

    return false;
}

void VideoImage::clearImageData(ImageData* pdest) const
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

void VideoImage::checkAndRequestSkipDecoding() const
{
    if (!pVideoPlayer)
        return;

    CriMvEasyPlayer *mvp = pVideoPlayer->GetCriPlayer();
    CriMvFrameInfo &info = pVideoPlayer->FrameInfo;

    CriUint64 count, unit;
    mvp->GetTime(count, unit);
    CriFloat32 timePlayback = (CriFloat32)count / unit * 1000;

    CriFloat32 timeFrame = (CriFloat32)info.time / info.tunit * 1000;
    CriFloat32 frameInterval = 1000.0f / ((CriFloat32)info.framerate / 1000);

	// Empirically gives the best results with v-sync.
	// Skip too early and we keep skipping, don't skip and we start getting decode
	// time outs.
	frameInterval *= 2.f;

    // When the movie frame is delayed, request to skip decoding
	if ((timePlayback - timeFrame) > frameInterval )
    {
		// This almost *always* leads to more stuttering
		// and it resets the frame time to the current time
        criMvPly_SkipFrame(mvp->mvply);

#ifdef GFX_VIDEO_DIAGS
        pVideoPlayer->pLog->LogMessage("[Video] Decode: Skipped frame: %d\n",
            info.frame_id);
#endif
    }
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
