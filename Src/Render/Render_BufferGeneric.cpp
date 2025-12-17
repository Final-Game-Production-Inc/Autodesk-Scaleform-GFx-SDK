/**********************************************************************

Filename    :   Render_BufferGeneric.cpp
Content     :   Generic RenderBufferManager implementation.
Created     :   April 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render_BufferGeneric.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace RBGenericImpl {

//------------------------------------------------------------------------

inline ImageSize RenderBufferManager::RoundUpImageSize(const ImageSize& size) const
{
    if (RequirePow2)
    {
        ImageSize roundedSize(1,1);
        while (roundedSize.Width  < size.Width)  { roundedSize.Width <<= 1; }
        while (roundedSize.Height < size.Height) { roundedSize.Height <<= 1; }
        return roundedSize;
    }
    else
    {
        return ImageSize(Alg::Max<UInt32>(32, (size.Width + 31) & ~31), 
                         Alg::Max<UInt32>(32, (size.Height + 31) & ~31));
    }
}

// Matches image size; inputs should be rounded.
inline bool MatchSize(RenderBufferType type, DSSizeMode sizeMode, const ImageSize& cachedSize, const ImageSize& size)
{
    // Some platforms requires the depth stencil buffer to be exactly the same size as
    // the render buffer. In this case, do not use an area calculation, ensure that the 
    // are exactly the same size.
    switch (sizeMode)
    {
    case DSSM_None:
        // Do nothing
        break;

    case DSSM_Exact:
        return size.Width == cachedSize.Width && size.Height == cachedSize.Height;

    case DSSM_EqualOrBigger:
        return cachedSize.Width >= size.Width && cachedSize.Height >= size.Height;
    }

    SF_UNUSED(type);
    if ((size.Width > cachedSize.Width) || (size.Height > cachedSize.Height))
        return false;

    // We should use at least 85% of image area; if not, rely on re-allocation, unless this is a temporary
    // buffer, in which case we can always have it.
    if (((cachedSize.Area() * 27)/32) > size.Area() )
        return false;
    
    return true;
}

bool CacheData::Match(const ImageSize& size, DSSizeMode sizeMode, RenderBufferType type, ImageFormat format) const
{
    if ((pBuffer->GetType() == type) && (Format == format))
        return MatchSize(type, sizeMode, pBuffer->GetBufferSize(), size);
    return false;
}


//------------------------------------------------------------------------
// ***** RenderBufferManager

RenderBufferManager::RenderBufferManager(DSSizeMode depthStencilSizeMode,
    UPInt memReuseLimit, UPInt memAbsoluteLimit)
    : CtorReuseLimit(memReuseLimit),
      ReuseLimit(0), AbsoluteLimit(memAbsoluteLimit),
      AllocSize(0), DefImageFormat(Image_None), 
      RequirePow2(0),
      DepthStencilSizeMode(depthStencilSizeMode)
{
}

RenderBufferManager::~RenderBufferManager()
{
    Destroy();
}

bool RenderBufferManager::Initialize(TextureManager* manager,
                                     ImageFormat format, const ImageSize& screenSize)
{
    // Should we clear out caches if being re-initialized?

    pTextureManager = manager;
    DefImageFormat = format;
    RequirePow2 = !manager->IsNonPow2Supported(format, ImageUse_RenderTarget);

    if (CtorReuseLimit == ReuseLimit_ScreenSize)
        ReuseLimit = screenSize.IsNull() ? 0 : screenSize.Area();
    else
        ReuseLimit = CtorReuseLimit;

    return true;
}

void RenderBufferManager::Destroy()
{
    Reset();
    DefImageFormat = Image_None;
    pTextureManager.Clear();
}

void RenderBufferManager::EndFrame()
{
    evictOverReuseLimit(RBCL_Reuse_LRU);
    evictOverReuseLimit(RBCL_LRU);

    SF_DEBUG_ASSERT(BufferCache[RBCL_Uncached].IsEmpty(), "RenderBufferManager::EndFrame - there are render targets still 'uncached'.");
    SF_DEBUG_ASSERT(BufferCache[RBCL_InUse].IsEmpty(), "RenderBufferManager::EndFrame - there are render targets still 'in-use'.");
    if (!BufferCache[RBCL_Uncached].IsEmpty() || !BufferCache[RBCL_InUse].IsEmpty())
        DumpUsage();
    BufferCache[RBCL_ThisFrame].PushListToFront(BufferCache[RBCL_InUse]);
    BufferCache[RBCL_LRU].PushListToFront(BufferCache[RBCL_PrevFrame]);
    BufferCache[RBCL_PrevFrame].PushListToFront(BufferCache[RBCL_ThisFrame]);
    BufferCache[RBCL_Reuse_LRU].PushListToFront(BufferCache[RBCL_Reuse_ThisFrame]);
}

void RenderBufferManager::Reset()
{
    // Clear out caches.
    SF_DEBUG_ASSERT(BufferCache[RBCL_Uncached].IsEmpty(), "RenderBufferManager::Reset - there are render targets still 'uncached'.");
    SF_DEBUG_ASSERT(BufferCache[RBCL_InUse].IsEmpty(), "RenderBufferManager::Reset - there are render targets still 'in-use'.");
    evictAll(RBCL_ThisFrame);
    evictAll(RBCL_PrevFrame);
    evictAll(RBCL_LRU);
    evictAll(RBCL_Reuse_ThisFrame);
    evictAll(RBCL_Reuse_LRU);
}

void RenderBufferManager::DumpUsage()
{
    SF_DEBUG_MESSAGE(1,"RenderBufferManager::DumpUsage()\n");
    for (unsigned i = 0; i < RBCL_ItemCount; ++i)
    {
        SF_DEBUG_MESSAGE1(1,"\tLIST(%d)\n", i);
        const List<CacheData>& cacheList = BufferCache[i];
        const CacheData* cacheData = 0;
        for (cacheData = cacheList.GetFirst(); !cacheList.IsNull(cacheData); cacheData = cacheList.GetNext(cacheData) )
            SF_DEBUG_MESSAGE3(1,"\t\tEntry: ptr=%p fmt=%d, size=%d\n", cacheData->pBuffer, cacheData->Format, cacheData->DataSize);
    }
}

Render::RenderTarget*
RenderBufferManager::CreateRenderTarget(const ImageSize& size, RenderBufferType type,
                                        ImageFormat format, Texture* texture)
{
    if (!pTextureManager)
        return 0;
    SF_ASSERT(type != RBuffer_Temporary);
    return createRenderTarget(size, type, format, texture);
}

RenderTarget*
RenderBufferManager::createRenderTarget(const ImageSize& size, RenderBufferType type,
                                        ImageFormat format, Texture* texture)
{
    SF_ASSERT(type != RBuffer_DepthStencil);

    // TBD: Will we need to round-up size on certain platforms?
    // Shouldn't be necessary if that is done internally by TextureManager anyway.

    RenderTarget* target = SF_HEAP_AUTO_NEW(this) RenderTarget(this, type, size);
    if (target)
    {
        // Apply a texture if relevant.
        target->Format = format;
        target->initTexture(texture);
        target->initViewRect(Rect<int>(size.Width, size.Height));
    }

    // By default, render targets don't get added to the list,
    // as it is up to end user to free them.
    return target;  
}



Render::RenderTarget*
RenderBufferManager::CreateTempRenderTarget(const ImageSize& size)
{
    if (!pTextureManager)
        return 0;

    CacheData*    data = 0;
    RenderTarget* target = 0;    
    ImageSize     roundedSize = RoundUpImageSize(size);
    ImageFormat   format = DefImageFormat;
    UPInt         requestSize = roundedSize.Area() * ImageData::GetFormatBitsPerPixel(format) / 8;
       
    switch(reserveSpace(&data, roundedSize, RBuffer_Temporary, format, requestSize))
    {
    case RS_Match:
    {
        // Reuse this item.
        target = data->GetRenderTarget();
        target->SetInUse(RTUse_InUse); // Moves to RBCL_InUse; Status = InUse.
        target->initViewRect(Rect<int>(size.Width, size.Height)); 
        target->AddRef();

        // When reusing an old render target, make sure that the CacheID is unset. In rare circumstances,
        // a primitive with the same address could be allocated, and match with its previous render target,
        // causing it not to re-cache.
        RenderBuffer::RenderTargetData* prt = target->GetRenderTargetData();
        if (prt)
            prt->CacheID = 0;
        break;
    }

    case RS_Alloc:
    {
        // Allocate new texture.
        {
            Ptr<Texture> texture = 
                *pTextureManager->CreateTexture(format, 1, roundedSize, ImageUse_RenderTarget);
            if (texture)
            {
                SF_ASSERT(texture->GetSize() == roundedSize);
                target = createRenderTarget(roundedSize, RBuffer_Temporary, format, texture);
                if (target)
                {
                    pushFront(RBCL_InUse, target);
                    target->initViewRect(Rect<int>(size.Width, size.Height));
                    target->DataSize = requestSize;
                    AllocSize += requestSize;
                }
            }
        }
        break;
    }

    case RS_Fail:
        break;
    }

    return target;
}


Render::DepthStencilBuffer*
RenderBufferManager::CreateDepthStencilBuffer(const ImageSize& size, bool temporary)
{
    if (!pTextureManager)
        return 0;

    CacheData*          data = 0;
    DepthStencilBuffer* buffer = 0;
    ImageSize           roundedSize = (DepthStencilSizeMode == DSSM_Exact) ? size : RoundUpImageSize(size);
    UPInt               requestSize = roundedSize.Area() * 4;

    // If not temporary, do not attempt to match.
    RenderBufferManager::ReserveSpaceResult reserveResult = temporary ? 
        reserveSpace(&data, roundedSize, RBuffer_DepthStencil, Image_None, requestSize) : RS_Alloc;

    switch(reserveResult)
    {
    case RS_Match:
        // Reuse this item.
        moveToFront(RBCL_InUse, data);
        buffer = data->GetDepthStencilBuffer();
        buffer->AddRef();
        break;

    case RS_Alloc:
        // Allocate new depth-stencil.
        {
            Ptr<Render::DepthStencilSurface> surface =
                *pTextureManager->CreateDepthStencilSurface(roundedSize);

            if (surface)
            {
                buffer = SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(this, roundedSize, temporary);
                if (buffer)
                {
                    buffer->initSurface(surface);

                    // Only put into the 'in-use' list if it is temporary. If it is a user target, then
                    // don't track it within the reuse lists. Also don't track it as allocated space.
                    if (temporary)
                    {
                        pushFront(RBCL_InUse, buffer);
                        AllocSize += requestSize;
                    }
                    buffer->DataSize = requestSize;
                }
            }
        }
        break;

    case RS_Fail:
        break;
    }   

    return buffer;
}



/*
***** Allocation / Shrinking Strategy

Goals:
 1. Don't grow over ReuseLimit unless necessary (do Reuse/LRU eviction first).
 2. Try to reuse buffers from previous frames without recreating them frequently,
    as this is costly on D3D and other APIs.

To accommodate (2) we don't to eagerly free buffers on release even if they
exceeded Reuse limit, preferring to keep them for one frame in case they
might be reused. This means that a released item always gets placed in a Reuse
buffer with actual free happening on:
 - Further allocation over ReuseLimit, if there was no reuse match.
 - End frame, if item wasn't used in this frame.
 - Explicit clear request.

General allocation then proceeds as follows:
 a) Try to match from ReuseLRU.
 b) Try to match from ReuseThis.
 c) Evict ReuseLRU until allocation is possible, allocate if fitness reached.
 d) Try to match from LRU, replace item if matched.
 e) Evict LRU until allocation is possible, allocate if fitness reached.
      - This is done before freeing ReuseThis because likelihood
        of old-frame LRU being needed is lower.
 f) Evict ReuseThis until allocation is possible, allocate if fitness reached.
 g) Try to match from Prev.
 h) Evict Prev until allocation is possible, allocate if fitness reached.
 i) Try to match from This.
 j) Evict This until allocation is possible, allocate if fitness reached.
 k) Allocated if allowed under AbsoluteLimit.
 l) Fail.

*/

RenderBufferManager::ReserveSpaceResult
RenderBufferManager::reserveSpace(CacheData **pdata,
                                  const ImageSize& size, RenderBufferType type,
                                  ImageFormat format, UPInt requestSize)
{
    if (AbsoluteLimit && (requestSize > AbsoluteLimit))
        return RS_Fail;

    // 1. Try to find available compatible-size buffer for reuse.

    *pdata = findMatch(RBCL_Reuse_LRU, size, type, format);
    if (!*pdata)
        *pdata = findMatch(RBCL_Reuse_ThisFrame, size, type, format);
    if (*pdata)
        return RS_Match;


    // 2. Evict 

    //  Evict ReuseLRU until allocation is possible.
    if (evictUntilAvailable(RBCL_Reuse_LRU, requestSize))
        return RS_Alloc;

    *pdata = findMatch(RBCL_LRU, size, type, format);
    if (*pdata)
        return RS_Match;

    if (evictUntilAvailable(RBCL_LRU, requestSize))
        return RS_Alloc;
    if (evictUntilAvailable(RBCL_Reuse_ThisFrame, requestSize))
        return RS_Alloc;

    // Prev Frame
    *pdata = findMatch(RBCL_PrevFrame, size, type, format);
    if (*pdata)
        return RS_Match;
    if (evictUntilAvailable(RBCL_PrevFrame, requestSize))
        return RS_Alloc;

    // This Frame
    *pdata = findMatch(RBCL_ThisFrame, size, type, format);
    if (*pdata)
        return RS_Match;
    if (evictUntilAvailable(RBCL_ThisFrame, requestSize))
        return RS_Alloc;

    // At this point, there is not enough space under ReuseLimit,
    // so check AbsoluteLimit.
    if (AbsoluteLimit && ((AllocSize + requestSize) > AbsoluteLimit))
        return RS_Fail;

    return RS_Alloc;
}


CacheData* RenderBufferManager::findMatch(RBCacheListType ltype, const ImageSize& size,
                                          RenderBufferType bufferType, ImageFormat format)
{
    CacheData* data = BufferCache[ltype].GetLast();
    while(!BufferCache[ltype].IsNull(data))
    {
        if (data->Match(size, (bufferType == RBuffer_DepthStencil) ? DepthStencilSizeMode : DSSM_None, bufferType, format))
        {
            return data;
        }
        data = data->pPrev;
    }
    return 0;
}


void RenderBufferManager::evict(CacheData* p)
{
    // Evict data:

    // - remove data from list
    // - If RenderTarget
    //       - release associated texture
    //       - If referenced outside, set status to lost
    //       - Else delete RenderTarget
    //       - free data if external RefCount == 0.
    // - else (depth stencil)
    //       - ASSERT(no references)
    //       - delete DepthStencil, this should free HW buffers.
    //       - free data

    p->RemoveNode();
    p->ListType = RBCL_Uncached;
    AllocSize -= p->DataSize;
    p->DataSize = 0;


    if (p->pBuffer->GetType() == RBuffer_Temporary)
    {
        p->GetRenderTarget()->onEvict();
    }
    else if (p->pBuffer->GetType() == RBuffer_DepthStencil)
    {
        p->GetDepthStencilBuffer()->onEvict();
    }
    else
    {
        SF_DEBUG_ASSERT1(0, "Cannot evict buffers of type (%d)", p->pBuffer->GetType());
    }
}


void RenderBufferManager::evictAll(RBCacheListType ltype)
{
    while(!BufferCache[ltype].IsEmpty())    
        evict(BufferCache[ltype].GetFirst());
}

void RenderBufferManager::evictOverReuseLimit(RBCacheListType ltype)
{
    CacheData *p;
    while(AllocSize > ReuseLimit)
    {
        p = BufferCache[ltype].GetLast();
        if (BufferCache[ltype].IsNull(p))
            break;
        evict(p);
    }
}

bool RenderBufferManager::evictUntilAvailable(RBCacheListType ltype, UPInt requestSize)
{
    CacheData *p;
    while((AllocSize + requestSize) > ReuseLimit)
    {
        p = BufferCache[ltype].GetLast();
        if (BufferCache[ltype].IsNull(p))
            break;
        evict(p);
    }
    return ((AllocSize + requestSize) <= ReuseLimit);
}


//------------------------------------------------------------------------
// ***** RenderTarget

void RenderTarget::SetInUse(RenderTargetUse inUse)
{
    if (GetType() != RBuffer_Temporary)
    {
        return;
    }

    SF_DEBUG_ASSERT1(RTStatus != RTS_Lost, "Cannot call RenderTarget::SetInUse on a lost RenderTarget (ptr=%p)", this);

    switch(inUse)
    {
    case RTUse_InUse:
        SF_ASSERT(RTStatus != RTS_Lost);
        getManager()->moveToFront(RBCL_InUse, this);
        RTStatus = RTS_InUse;
        break;

    case RTUse_Unused:
    case RTUse_Unused_Cacheable:
        if (ListType < RBCL_ThisFrame)
            getManager()->moveToFront(RBCL_ThisFrame, this);
        RTStatus = RTS_Available;
        break;

    default:
        SF_DEBUG_ASSERT1(0, "Unexpected RenderTargetUse type (%d)", inUse);
        break;
    }
}

void RenderTarget::AddRef()
{
    RefCount++;
}

void RenderTarget::Release()
{
    RefCount--;
    if (RefCount > 0)
        return;

    if ((pBuffer->GetType() != RBuffer_Temporary) ||
        (RTStatus == RTS_Lost))
    {
        delete this;
        return;
    }

    RBCacheListType newList = (ListType <= RBCL_ThisFrame) ?
                              RBCL_Reuse_ThisFrame : RBCL_Reuse_LRU;
    getManager()->moveToFront(newList, this);
    RTStatus = RTS_Available;
}

void RenderTarget::onEvict()
{
    SF_DEBUG_ASSERT1(GetType() == RBuffer_Temporary, "Cannot evict a non-temporary RenderTarget (type=%d).", GetType());
    SF_DEBUG_ASSERT1(RTStatus == RTS_Available, "Can only evict a RenderTarget that is available (current status=%d)", RTStatus);

    // Release texture.
    initTexture(0);
    destroyRenderTargetData();

    RTStatus = RTS_Lost;

    if (RefCount == 0)
        delete this;
}


//------------------------------------------------------------------------
// ***** DepthStencilBuffer

void DepthStencilBuffer::AddRef()
{
    RefCount++;
}

void DepthStencilBuffer::Release()
{
    RefCount--;
    if (RefCount > 0)
        return;

    if (Type == RBuffer_DepthStencil)
    {
        // If it is temporary, save it for potential reuse.
        RBCacheListType newList = (ListType <= RBCL_ThisFrame) ?
                                  RBCL_Reuse_ThisFrame : RBCL_Reuse_LRU;
        getManager()->moveToFront(newList, this);
    }
    else
    {
        // Destroy it.
        SF_DEBUG_ASSERT(Type == RBuffer_UserDepthStencil, "Should only be destroying UserDepthStencil surfaces here.");
        delete this;
    }
}

void DepthStencilBuffer::onEvict()
{
    SF_DEBUG_ASSERT(GetType() == RBuffer_DepthStencil, "Can only evict a temporary DepthStencilBuffer.");
    SF_DEBUG_ASSERT(RefCount == 0, "Can only evict a non-referenced DepthStencilBuffer.");   
    delete this;
}


}}} // namespace Scaleform::Render::RBGenericImpl
