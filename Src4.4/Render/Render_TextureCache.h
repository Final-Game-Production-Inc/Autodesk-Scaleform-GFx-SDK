/**********************************************************************

PublicHeader:   Render
Filename    :   Render_TextureCache.h
Content     :   Interface for the texture caching system.
Created     :   2012/09/06
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_TextureCache_H
#define INC_SF_Render_TextureCache_H

#include "Kernel/SF_RefCount.h"
#include "Render/Render_Stats.h"

namespace Scaleform { namespace Render {

class Texture;

class TextureCache : public RefCountBase<TextureCache, StatRender_TextureManager_Mem>
{
public:
    virtual ~TextureCache() { } 

    // Generally a TextureCache will do processing during either a BeginFrame or EndFrame (or both)
    // So make these overrideable, and make sure that they are called consistently.
    virtual void BeginFrame()   { };
    virtual void EndFrame()     { };

    // Called when a texture is created.
    virtual void TextureCreation(Render::Texture* ptexture) = 0;

    // Called when a texture is destroyed by the system, and therefore no longer needs to be checked for eviction.
    virtual void TextureDestroyed(Render::Texture* ptexture) = 0;

    // Called when a texture is set in the pipeline to be used for rendering.
    virtual void TextureReference(Render::Texture* ptexture) = 0;

    // Called when a texture is evicted by the caching system. This is provided as a
    // convenience, so that users can use the default-implementation, and simply be notified
    // when textures are evicted (the base class implementation should still be called).
    virtual void TextureEvicted(Render::Texture* ptexture) { SF_UNUSED(ptexture); };

    // Does a pass on the current state of the texture caching system, evicting textures if
    // the implementation deems that it is necessary to do so. 
    virtual void PerformEvictionCheck() = 0;
};


}}; // Scaleform::Render

#endif // INC_SF_Render_TextureCache_H
