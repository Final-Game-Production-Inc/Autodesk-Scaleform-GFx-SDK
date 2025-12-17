/**********************************************************************

PublicHeader:   Render
Filename    :   Render_BufferGeneric.h
Content     :   Generic RenderBufferManager implementation, relying
                on texture allocation and user-set limits.
Created     :   April 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_BufferGeneric_H
#define INC_SF_Render_BufferGeneric_H

#include "Render_Buffer.h"

namespace Scaleform { namespace Render {

namespace RBGenericImpl {
    class RenderBufferManager;
};

// RenderBufferManagerGeneric is a general-purpose implementation of
// RenderBufferManager that delegates to TextureManager and performs
// buffer eviction based on memory counters.
typedef RBGenericImpl::RenderBufferManager RenderBufferManagerGeneric;


//------------------------------------------------------------------------
namespace RBGenericImpl {


// RenderBuffer RBCacheListType; may be used by RenderBufferManager as needed.
enum RBCacheListType
{
    RBCL_Uncached,   // The item isn't cached yet.    
    RBCL_InUse,      // The item is queued up for rendering and not complete yet.
    RBCL_ThisFrame,  // The item is used in the current frame.
    RBCL_PrevFrame,  // The item was used in previous frame.
    RBCL_LRU,
    RBCL_Reuse_ThisFrame,
    RBCL_Reuse_LRU,
    // Total number of cache lists
    RBCL_ItemCount
};

// RenderBuffer DSSizeMode; used to indicate depth-stencil size preference.
enum DSSizeMode
{
    DSSM_None,          // Use the default depth stencil size mode
    DSSM_Exact,         // Depth stencil requests must be identical to potential matches
    DSSM_EqualOrBigger  // Depth stencil requests must be identical to color sizes
};


class RenderTarget;
class DepthStencilBuffer;

// CacheData is a mix-in base class for RenderBuffer implementations used by
// RenderBufferManager. This class provides LRU/MRU cache element tracking
// through different linked list types.

class CacheData : public ListNode<CacheData>
{
public:
    RenderBuffer*   pBuffer;
    RBCacheListType ListType;
    ImageFormat     Format;
    UPInt           DataSize;

    CacheData(RenderBuffer* buffer)
        : ListNode<CacheData>(),
          pBuffer(buffer), ListType(RBCL_Uncached),
          Format(Image_None), DataSize(0)
    { }

    inline RenderTarget*       GetRenderTarget() const;
    inline DepthStencilBuffer* GetDepthStencilBuffer() const;

    bool Match(const ImageSize& size, DSSizeMode sizeMode, RenderBufferType type, ImageFormat format) const;
};


//------------------------------------------------------------------------
// RenderBufferManager allocates textures 

class RenderBufferManager : public Render::RenderBufferManager
{
    friend class RenderTarget;
    friend class DepthStencilBuffer;    
public:

    // These special value of ReuseLimit dictate limit policy.
    // If a different value is passed, it it interpreted directly.
    enum {
        // ReuseLimit of 0 means that no reuse memory accumulation takes place.
        ReuseLimit_Zero        = 0,
        // ReuseLimit_ScreenSize mens that default reuse limit will be set to
        // one full screen size, provided by Initialize.
        ReuseLimit_ScreenSize  = 0xFFFFFFFF
    };

    RenderBufferManager(DSSizeMode depthStencilSizeMode = DSSM_None,
                        UPInt memReuseLimit = ReuseLimit_ScreenSize,
                        UPInt memAbsoluteLimit = 0);

    virtual ~RenderBufferManager();

    virtual bool    Initialize(TextureManager* manager, ImageFormat format,
                               const ImageSize& screenSize = ImageSize(0));
    virtual void    Destroy();

    virtual void    EndFrame();

    virtual void    Reset();

    virtual void    DumpUsage();

    virtual Render::RenderTarget* CreateRenderTarget(const ImageSize& size, RenderBufferType type,
                                                     ImageFormat format, Texture* texture = 0);
    
    // Allocates a temporary render target of specified type
    //  - call TextureManager to allocate texture
    virtual Render::RenderTarget* CreateTempRenderTarget(const ImageSize& size);

    virtual Render::DepthStencilBuffer* CreateDepthStencilBuffer(const ImageSize& size, bool temporary=true);


protected:
    RenderTarget* createRenderTarget(const ImageSize& size, RenderBufferType type,
                                     ImageFormat format, Texture* texture = 0);

    enum ReserveSpaceResult {
        RS_Match,
        RS_Alloc,
        RS_Fail
    };

    ImageSize RoundUpImageSize(const ImageSize& size) const;

    // Reserves space for allocation by either:
    //  - Finding a mactching reusable buffer,
    //  - Evicting bufffers until enough space is available.
    ReserveSpaceResult reserveSpace(CacheData **pdata,
                                    const ImageSize& size, RenderBufferType type,
                                    ImageFormat format, UPInt requestSize);

    CacheData* findMatch(RBCacheListType ltype, const ImageSize& size,
                         RenderBufferType bufferType, ImageFormat format);

    void pushFront(RBCacheListType ltype, CacheData* p)
    {
        p->ListType = ltype;        
        BufferCache[ltype].PushFront(p);
    }
    void moveToFront(RBCacheListType ltype, CacheData* p)
    {                
        p->RemoveNode();
        pushFront(ltype, p);     
    }

    void moveListToFront(RBCacheListType to, RBCacheListType from)
    {
        List<CacheData>& src = BufferCache[from];
        BufferCache[to].PushListToFront(src);        
    }

    void evict(CacheData* p);
    void evictAll(RBCacheListType ltype);
    void evictOverReuseLimit(RBCacheListType ltype);
    bool evictUntilAvailable(RBCacheListType ltype, UPInt allocSize);

    Ptr<TextureManager> pTextureManager;
    // ReuseLimit value passed in constructor; may be a special tag.
    UPInt               CtorReuseLimit;
    UPInt               ReuseLimit, AbsoluteLimit;
    UPInt               AllocSize; // Currently allocated size
    ImageFormat         DefImageFormat;
    bool                RequirePow2;
    DSSizeMode          DepthStencilSizeMode;

    List<CacheData> BufferCache[RBCL_ItemCount];
};


//------------------------------------------------------------------------

// RenderTarget implementation with swapping support through CachaData.
// Holds Texture.

class RenderTarget : public Render::RenderTarget, public CacheData
{
    friend class RenderBufferManager;
public:
    RenderTarget(RenderBufferManager* manager, RenderBufferType type,
                 const ImageSize& bufferSize)
        : Render::RenderTarget(manager, type, bufferSize),
          CacheData(getThis()), RTStatus(RTS_InUse)
    { }

    RenderTarget* getThis() { return this; }

    virtual Texture*    GetTexture() const  { return pTexture; }
    virtual RenderTargetStatus GetStatus() const { return RTStatus; }
    void                SetInUse(bool inUse) { SetInUse(inUse ? RTUse_InUse : RTUse_Unused); }
    virtual void        SetInUse(RenderTargetUse inUse);

    virtual void AddRef();
    virtual void Release();

    void onEvict();

protected:
    // Return derived-class version of manager.
    RenderBufferManager* getManager() const { return (RenderBufferManager*)pManager; }

    void initTexture(Texture* texture)
    { pTexture = texture; }

    void initViewRect(const Rect<int>& viewRect)
    {
        ViewRect = viewRect;
    }    

    Ptr<Texture>        pTexture;
    RenderTargetStatus  RTStatus;
};


// DepthStencilBuffer implementation with swapping support through CacheData;
// holds DepthStencilSurface.

class DepthStencilBuffer : public Render::DepthStencilBuffer, public CacheData
{
    friend class RenderBufferManager;
public:
    DepthStencilBuffer(RenderBufferManager* manager, const ImageSize& bufferSize, bool temporary=true)
        : Render::DepthStencilBuffer(manager, bufferSize, temporary), CacheData(getThis())
    { }

    DepthStencilBuffer* getThis() { return this; }

    virtual DepthStencilSurface* GetSurface() const  { return pSurface; }
    virtual void AddRef();
    virtual void Release();

    void onEvict();

protected:
    void initSurface(DepthStencilSurface* surface)
    { pSurface = surface; }

    RenderBufferManager* getManager() const { return (RenderBufferManager*)pManager; }

    Ptr<DepthStencilSurface> pSurface;
};


inline RenderTarget* CacheData::GetRenderTarget() const
{
    SF_ASSERT((pBuffer->GetType() > RBuffer_None) &&
              (pBuffer->GetType() <= RBuffer_Texture));
    return (RenderTarget*)pBuffer;
}
inline DepthStencilBuffer* CacheData::GetDepthStencilBuffer() const
{
    SF_ASSERT(pBuffer->GetType() == RBuffer_DepthStencil);
    return (DepthStencilBuffer*)pBuffer;
}


}}}; // namespace Scaleform::Render::RBGenericImpl

#endif // INC_SF_Render_BufferGeneric_H
