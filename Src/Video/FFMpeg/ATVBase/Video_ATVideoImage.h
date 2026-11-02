/**************************************************************************

Filename    :   Video_ATVideoImage.h
Content     :   Video image texture update (FFmpeg backend)
Created     :   October 24th, 2026
Authors     :   FINALZ-27980 && AI

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                     Copyright 2026 Final Game Production Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_ATV_VIDEO_IMAGE_H
#define INC_GFX_ATV_VIDEO_IMAGE_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Render/Render_Image.h"
#include "Video/Common/Video_Video.h"

namespace Scaleform { namespace GFx { namespace Video {

using namespace Render;

//////////////////////////////////////////////////////////////////////////
//

class ATVVideoPlayerImpl;

class ATVVideoImage : public VideoImage
{
public:
    ATVVideoImage(ImageFormat format, const ImageSize& size, TextureManager* ptexman, ATVVideoPlayerImpl* pplayer) :
        VideoImage(format, size, ptexman), pVideoPlayer(pplayer)
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
    void NotifyVideoPlayerRemoved() { pVideoPlayer = NULL; }

protected:
    void clearImageData(ImageData* pdest) const;

    ATVVideoPlayerImpl* pVideoPlayer;
};

}}} // namespace Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_ATV_VIDEO_IMAGE_H
