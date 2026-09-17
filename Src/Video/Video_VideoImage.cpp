/**************************************************************************

Filename    :   Video_VideoImage.cpp
Content     :   Video image texture update (CRI Mana 2019)
Created     :   Feb, 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

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

    CriManaPlayerHn player = pVideoPlayer->GetManaPlayer();
    CriManaFrameInfo frameInfo;
    Alg::MemUtil::Set(&frameInfo, 0, sizeof(frameInfo));

    if (criManaPlayer_ReferFrame(player, &frameInfo) == CRI_TRUE &&
        criManaPlayer_IsFrameOnTime(player, &frameInfo) == CRI_TRUE)
    {
        if (pdest->GetPlaneCount() >= 3)
        {
            ImagePlane y, u, v, a;
            CriManaTextureBuffersYUV yuvBufs;
            Alg::MemUtil::Set(&yuvBufs, 0, sizeof(yuvBufs));

            pdest->GetPlane(0, &y);
            yuvBufs.y_plane.imagebuf  = (CriUint8*)y.pData;
            yuvBufs.y_plane.pitch     = (CriUint32)y.Pitch;
            yuvBufs.y_plane.bufsize   = (CriUint32)y.DataSize;
            pdest->GetPlane(1, &u);
            yuvBufs.u_plane.imagebuf  = (CriUint8*)u.pData;
            yuvBufs.u_plane.pitch     = (CriUint32)u.Pitch;
            yuvBufs.u_plane.bufsize   = (CriUint32)u.DataSize;
            pdest->GetPlane(2, &v);
            yuvBufs.v_plane.imagebuf  = (CriUint8*)v.pData;
            yuvBufs.v_plane.pitch     = (CriUint32)v.Pitch;
            yuvBufs.v_plane.bufsize   = (CriUint32)v.DataSize;

            if (pdest->GetPlaneCount() >= 4 && pVideoPlayer->IsAlphaVideo())
            {
                pdest->GetPlane(3, &a);
                yuvBufs.a_plane.imagebuf  = (CriUint8*)a.pData;
                yuvBufs.a_plane.pitch     = (CriUint32)a.Pitch;
                yuvBufs.a_plane.bufsize   = (CriUint32)a.DataSize;
            }

            criManaPlayer_CopyFrameToBuffersYUV(player, &frameInfo, &yuvBufs);
        }
        else
        {
            ImagePlane argb;
            pdest->GetPlane(0, &argb);

            CriManaTextureBuffer argbBuf;
            argbBuf.imagebuf = (CriUint8*)argb.pData;
            argbBuf.pitch    = (CriUint32)argb.Pitch;
            argbBuf.bufsize  = (CriUint32)argb.DataSize;

            criManaPlayer_CopyFrameToBufferARGB32(player, &frameInfo, &argbBuf);
        }

        // Store frame info in player for position tracking
        pVideoPlayer->FrameInfo = frameInfo;

        // Release the frame back to the decoder
        // But When Is Loading H.264 Video File Is More,
        // It Will Raise Exception in GFxPlayer.exe: 0xC0000005 : 
        // Access violation occurred while writing to location 0x00000000000003B0.
        criManaPlayer_DiscardFrame(player, &frameInfo);

        pVideoPlayer->FrameOnTime = true;

        if (pVideoPlayer->PausedStartup)
        {
            criManaPlayer_Pause(player, CRI_TRUE);
            pVideoPlayer->Paused = CRI_TRUE;
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

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
