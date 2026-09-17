/**********************************************************************

PublicHeader:   Render
Filename    :   Render_TextureCacheGeneric.h
Content     :   Generic implementation of the TextureCache interface. It uses
                an LRU system, with a size limit which determines when textures
                will start to be evicted.
Created     :   2012/09/12
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_TextureCacheGeneric_H
#define INC_SF_Render_TextureCacheGeneric_H

#include "Render/Render_TextureCache.h"
#include "Kernel/SF_Hash.h"

namespace Scaleform { namespace Render {

class TextureCacheGeneric : public TextureCache
{
public:
    // Enumeration, different methods of tracking the current memory usage by textures (and thus, when they start to be evicted).
    enum TrackingMethod
    {
        TrackingMethod_All,             // All textures are included in the total, incl. the glyph textures, etc.
        TrackingMethod_OnlyEvictable,   // Only textures that can be evicted are tracked in the total.
        TrackingMethod_Count,
    };

    // The suggested default limit size. Generally, the texture cache system is geared towards
    // mobile systems (highly memory constrained), so on Desktop or Console systems, so this suggestion
    // is very low. Only HALs on mobiles provide a the texture cache by default, however, a texture cache
    // can still be used with other platforms.
    static const unsigned Megabyte = 1024*1024;
    enum DefaultLimitSizes
    {
        DefaultLimitSize        =   8 * Megabyte,
    };

    // Constructor, textureSizeLimit indicates the maximum desired size in bytes of the textures
    // used by GFx. If 0 is passed, the default (DefaultLimitSize) chosen. trackMethod 
    // specifies the type of tracking used to determine used texture memory.
    TextureCacheGeneric(UPInt textureSizeLimit = TextureCacheGeneric::DefaultLimitSize, 
                        TrackingMethod trackMethod = TrackingMethod_OnlyEvictable);
    virtual ~TextureCacheGeneric();

    // End the frame, incrementing the frame counter.
    virtual void EndFrame();

    // Sets the limit size. If it is reduced below the current usage, eviction may occur immediately.
    virtual void  SetLimitSize(UPInt limitSize = TextureCacheGeneric::DefaultLimitSize);

    // Returns the size limit (in bytes) after which eviction will begin to occur.
    virtual UPInt  GetLimitSize() const;
    // Returns the size (in bytes) of the current usage of textures in the system.
    virtual UPInt  GetUsageSize() const;
    // Returns the current frame that the texture cache is on (used for last-usage of textures).
    virtual UInt64 GetCurrentFrame() const { return CurrentFrame; };
    // Returns the texture tracking method.
    TrackingMethod GetTrackingMethod() const { return TextureTrackingMethod; }
    // Returns whether ptexture can be evicted by the texture cache.
    virtual bool   CanEvictTexture(Render::Texture* ptexture) const;

    // *** TextureCache
    virtual void TextureCreation(Render::Texture* ptexture);
    virtual void TextureDestroyed(Render::Texture* ptexture);
    virtual void TextureReference(Render::Texture* ptexture);
    virtual void TextureEvicted(Render::Texture* ptexture);
    virtual void PerformEvictionCheck();

protected:

    // For tracking texture references internally.
    struct TextureReferenceNode : public ListNode<TextureReferenceNode>, NewOverrideBase<StatRender_TextureManager_Mem>
    {
    public:
        TextureReferenceNode(Render::Texture* tex, UInt64 frame, bool evictable) : 
          ListNode<TextureReferenceNode>(), Evictable(evictable), pTexture(tex), LastFrameReference(frame)
          {
              pPrev = 0;
              pNext = 0;
          }
          bool                Evictable;           // If true, the texture is evictable by the system.
          Render::Texture*    pTexture;            // Texture that this node references
          UInt64              LastFrameReference;  // The last frame (TextureCacheGeneric::CurrentFrame) this texture was referenced.
    };

    typedef List<TextureReferenceNode> ReferenceListType;
    typedef HashLH<Render::Texture*, TextureReferenceNode*> UsageHashType;

    UInt64            CurrentFrame;            // Frame counter, to determine last usage frame.
    UPInt             EvictionMemoryLimit;     // The memory limit, after which textures will start to be evicted.
    UPInt             CurrentMemoryUsage;      // The amount of memory currently used by textures.
    UsageHashType     TextureUsageHash;        // Keeps track of texture to texture reference nodes (for faster finding).
    ReferenceListType TextureEvictionList;     // A list of textures, in order of their eviction priority (LRU first).
    ReferenceListType TextureTrackedList;      // A list of textures, which the system is tracking, but cannot evict.
    TrackingMethod    TextureTrackingMethod;   // The method used for tracking memory used by textures (see TrackingMethod).
};

}}; // Scaleform::Render

#endif // INC_SF_Render_TextureCacheGeneric_H
