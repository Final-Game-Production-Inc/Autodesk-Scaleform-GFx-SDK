/**************************************************************************

PublicHeader:   Render
Filename    :   Render_GlyphCacheConfig.h
Content     :   GlyphCache parameters and configuration API
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GlyphCacheConfig_H
#define INC_SF_Render_GlyphCacheConfig_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
// ***** GlyphCacheParams


struct GlyphCacheParams
{
    unsigned TextureWidth;
    unsigned TextureHeight; 
    unsigned NumTextures; 
    unsigned MaxSlotHeight; 
    unsigned SlotPadding;
    unsigned TexUpdWidth;
    unsigned TexUpdHeight;
    float    MaxRasterScale;
    unsigned MaxVectorCacheSize;
    float    FauxItalicAngle;
    float    FauxBoldRatio;
    float    OutlineRatio;
    float    ShadowQuality;         // Must be 0.25...1.0
    bool     UseAutoFit;
    bool     UseVectorOnFullCache;

    // If true, the glyph cache wait until the GPU is no longer using slots, so they can be evicted.
    // Otherwise, full cache is reported. This only occurs when using more than the entire glyph space
    // during a single frame of rendering.
    bool     FenceWaitOnFullCache;

    // Configures dynamic GlyphCache rendering.
    // Pass NumTextures == 0 to disable dynamic cache.
    GlyphCacheParams(unsigned numTextures = 1,
                     unsigned textureWidth = 1024, unsigned textureHeight = 1024,
                     unsigned maxSlotHeight = 48)
        : 
        TextureWidth(textureWidth),
        TextureHeight(textureHeight),

        NumTextures(numTextures),
        MaxSlotHeight(maxSlotHeight),
        SlotPadding(2),
        TexUpdWidth(256),
        TexUpdHeight(512),
        MaxRasterScale(1.0f),
        MaxVectorCacheSize(500),
        FauxItalicAngle(0.25f),
        FauxBoldRatio(0.045f),
        OutlineRatio(0.01f),
        ShadowQuality(1.0f),
        UseAutoFit(true),
        UseVectorOnFullCache(true),
        FenceWaitOnFullCache(true)
    {}

};


//------------------------------------------------------------------------
// ***** GlyphCacheConfig

// GlyphCacheConfig defined external configuration API for GlyphCache.

class GlyphCacheConfig
{   
public:
    virtual ~GlyphCacheConfig() {}

    // Sets new GlyphCache parameters, re-creating texture buffers if
    // cache is already initialized. Returns if false buffer resize failed.
    // Note that some parameters applied may be modified by HAL/Renderer to
    // match its capabilities.
    virtual bool    SetParams(const GlyphCacheParams& params) = 0;

    // Returns last applied GlyphCacheParams value, potentially adjusted
    // based on HAL/Renderer capabilities. If SetParams wasn't called,    
    // default values used are returned.
    virtual const GlyphCacheParams& GetParams() const = 0;

    // Clears glyph cache.
    virtual void    ClearCache() = 0;
};

}}

#endif
