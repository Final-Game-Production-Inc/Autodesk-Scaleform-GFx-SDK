/**********************************************************************

PublicHeader:   Render
Filename    :   Render_TextureCacheGeneric.cpp
Content     :   Implementation of the generic LRU texture cache.
Created     :   2012/09/13
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/Render_TextureCacheGeneric.h"
#include "Render/Render_Image.h"

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

TextureCacheGeneric::TextureCacheGeneric(UPInt textureSizeLimit, TrackingMethod trackMethod) :
    CurrentFrame(0),
    EvictionMemoryLimit(0),
    CurrentMemoryUsage(0),
    TextureTrackingMethod(trackMethod)
{
    SetLimitSize(textureSizeLimit);
}

TextureCacheGeneric::~TextureCacheGeneric()
{
    // Cleanup, if there are still textures in the cache (which there really shouldn't be).
    SF_DEBUG_ASSERT(TextureEvictionList.IsEmpty(), "TextureCache contains texture references on destruction.");
    SF_DEBUG_ASSERT(TextureTrackedList.IsEmpty(), "TextureCache contains texture references on destruction.");
    while (!TextureEvictionList.IsEmpty())
        TextureDestroyed(TextureEvictionList.GetFirst()->pTexture);
    while (!TextureTrackedList.IsEmpty())
        TextureDestroyed(TextureTrackedList.GetFirst()->pTexture);
}

void TextureCacheGeneric::EndFrame()
{
    CurrentFrame++;
}

void TextureCacheGeneric::SetLimitSize(UPInt limitSize)
{
    if (limitSize == 0)
    {
        // If they did not provide a texture size limit, choose mobile suggestion as the default.
        limitSize = DefaultLimitSize;
    }

    // TODOBM: per-platform limits.
    EvictionMemoryLimit = limitSize;

    // If the limit has been lowered, we may be required to evict something now, so check.
    PerformEvictionCheck();
}

UPInt TextureCacheGeneric::GetLimitSize() const
{
    return EvictionMemoryLimit;
}

UPInt TextureCacheGeneric::GetUsageSize() const
{
    return CurrentMemoryUsage;
}

bool TextureCacheGeneric::CanEvictTexture(Render::Texture* ptexture) const
{
    if (!ptexture)
        return false;

    // If there's no image to reinitialize the texture from, it can't be evicted.
    if (!ptexture->pImage)
        return false;

    // If it's not a 'true' image, then it cannot be evicted.
    if (ptexture->pImage->GetImageType() != ImageBase::Type_Image)
        return false;

    return true;
}

void TextureCacheGeneric::TextureCreation(Render::Texture* ptexture)
{
    SF_DEBUG_WARNING1(ptexture == 0, "NULL texture passed to %s", __FUNCTION__);
    if (ptexture == 0)
        return;

    // If the texture is not evictable, and we are not tracking non-evictable textures, don't even add it to the list.
    bool evictable = CanEvictTexture(ptexture);
    if (!evictable && GetTrackingMethod() == TrackingMethod_OnlyEvictable)
        return;

    TextureReferenceNode* refNode = SF_NEW TextureReferenceNode(ptexture, CurrentFrame, evictable);
    if (evictable)
        TextureEvictionList.PushBack(refNode);
    else
        TextureTrackedList.PushBack(refNode);

    // Add the texture to the usage cache.
    SF_DEBUG_ASSERT(!TextureUsageHash.Get(ptexture), "TextureCreation called for a texture already in the cache.");
    TextureUsageHash.Add(ptexture, refNode);

    // Add the size of the texture to the total count.
    switch(GetTrackingMethod())
    {
    case TrackingMethod_All:
        CurrentMemoryUsage += ptexture->GetBytes(0);
        break;

    default:
    case TrackingMethod_OnlyEvictable:
        if (evictable)
        {
            CurrentMemoryUsage += ptexture->GetBytes(0);
        }
        break;
    }

    // If we exceeded the usage limit, try to do eviction.
    PerformEvictionCheck();
}

void TextureCacheGeneric::TextureDestroyed(Render::Texture* ptexture)
{
    SF_DEBUG_WARNING1(ptexture == 0, "NULL texture passed to %s", __FUNCTION__);
    if (ptexture == 0 || ptexture->State != Texture::State_Valid)
        return;
    TextureReferenceNode** node = TextureUsageHash.Get(ptexture);
    if (node && *node)
    {
        if ((*node)->Evictable)
            CurrentMemoryUsage -= ptexture->GetBytes(0);
        (*node)->RemoveNode();
        delete *node;
        TextureUsageHash.Remove(ptexture);
    }
}

void TextureCacheGeneric::TextureReference(Render::Texture* ptexture)
{
    SF_DEBUG_WARNING1(ptexture == 0, "NULL texture passed to %s", __FUNCTION__);
    if (ptexture == 0)
        return;
    TextureReferenceNode** node = TextureUsageHash.Get(ptexture);    
    if (node && *node)
    {
        (*node)->RemoveNode();
        (*node)->LastFrameReference = CurrentFrame;
        if ((*node)->Evictable)
            TextureEvictionList.PushBack(*node);
        else
            TextureTrackedList.PushBack(*node);
    }
}

void TextureCacheGeneric::TextureEvicted(Render::Texture* ptexture)
{
    // Do the same thing as if the texture were destroyed.
    TextureDestroyed(ptexture);
}

void TextureCacheGeneric::PerformEvictionCheck()
{
    // While we are using too much size, evict the textures.
    UPInt currentHashSize = TextureUsageHash.GetSize();
    TextureReferenceNode* refNode = TextureEvictionList.GetFirst();
    while (!TextureEvictionList.IsNull(refNode) && GetUsageSize() >= EvictionMemoryLimit && EvictionMemoryLimit != 0)
    {
        // The LRU texture should be the last one on the list.
        SF_DEBUG_WARNING1(!refNode->pTexture, "NULL TextureReference in %s", __FUNCTION__);
        if (refNode->LastFrameReference >= CurrentFrame -1)
        {
            SF_DEBUG_WARNONCE(1, "TextureCache limit exceeded, but all textures in use.");
            break;
        }
        // Can only evict the texture, if it has an associated image, otherwise it wouldn't be able to
        // reinitialize itself.
        if (refNode->pTexture && refNode->pTexture->pImage)
        {
            //SF_DEBUG_WARNING2(1, "Texture evicted by cache overflow: 0x%08x (not used for %d frames)", refNode->pTexture, CurrentFrame - refNode->LastFrameReference);
            refNode->pTexture->LoseTextureData();
            refNode = TextureEvictionList.GetFirst();
        }
        else
        {
            refNode = TextureEvictionList.GetNext(refNode);
            continue;
        }

        // Ensure that we're not going in an infinite loop.
        if (currentHashSize <= TextureUsageHash.GetSize())
        {
            SF_DEBUG_WARNING(1, "LoseTextureData did not cause a TextureEviction notification.");
            break;
        }
    }
}


}}; // Scaleform::Render

