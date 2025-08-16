/**************************************************************************

Filename    :   Video_VideoImage.h
Content     :   
Created     :   Feb, 2011
Authors     :   Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEO_IMAGE_H
#define INC_GFX_VIDEO_IMAGE_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Render/Render_Image.h"

namespace Scaleform { namespace GFx { namespace Video {

using namespace Render;

//////////////////////////////////////////////////////////////////////////
//

class VideoPlayerImpl;

class VideoImage : public TextureImage
{
public:
    VideoImage(ImageFormat format, const ImageSize& size, TextureManager* ptexman, VideoPlayerImpl* pplayer) :
        TextureImage(format, size, ImageUse_Update, 0, ptexman), pVideoPlayer(pplayer)
    {
        SF_ASSERT(ptexman);
        SF_ASSERT(pplayer);
    }

    virtual Texture* GetTexture(TextureManager* ptexman);
    virtual bool     Decode(ImageData* pdest, CopyScanlineFunc func, void* parg) const;
    virtual void     TextureLost(TextureLossReason)
    {
        // Do not release the backing texture. Will be restored after loss.
    }
    void NotifyVideoPlayerRemoved();

protected:
    void clearImageData(ImageData* pdest) const;
    void checkAndRequestSkipDecoding() const;

    VideoPlayerImpl* pVideoPlayer;
    mutable Lock     VideoPlayerLock;
};

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEO_IMAGE_H
