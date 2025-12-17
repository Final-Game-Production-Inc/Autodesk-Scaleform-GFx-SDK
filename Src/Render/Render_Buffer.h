/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Buffer.h
Content     :   Render Buffer target description and manager
Created     :   April 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_Buffer_H
#define INC_SF_Render_Buffer_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Debug.h"
#include "Render_Image.h"


namespace Scaleform { namespace Render {

class RenderBufferManager;


//------------------------------------------------------------------------
// RenderBufferType describes the type of RenderBuffer.

enum RenderBufferType
{
    RBuffer_None,
    // Default back-buffer render target.
    RBuffer_Default,
    // User-specified platform-specific render target.
    RBuffer_User,           
    RBuffer_Temporary,          // Created with CreateTempRenderTarget.
    RBuffer_Texture,            // Created from user texture.
    RBuffer_DepthStencil,       // Created with CreateDepthStencilBuffer (temporary).
    RBuffer_UserDepthStencil    // Created with CreateDepthStencilBuffer (not temporary).
};


//------------------------------------------------------------------------
// ***** RenderBuffer

// RenderBuffer is a type of buffer created and managed by the RenderBufferManager,
// with implementations including RenderTarget and DepthStencilBuffer.
// Features:
// - RenderBuffer reference counting is custom due to potential caching,
//   allowing released buffers to survive and be reused when applicable.
// - Each RenderBuffer may have associated RenderTargetData allocated, which will
//   get deleted whenever RenderBuffer actually dies or its associated
//   HW resource (texture, etc) is lost.
// - ListNode base and RBCacheListType are stored in base class for
//   convenient implementation (TBD: Could be moved to derived-class
//   template in the future if necessary).

class RenderBuffer : public NewOverrideBase<StatRender_TextureManager_Mem>, public RefCountVImpl
{
    friend class RenderBufferManager;
public:
    RenderBuffer(RenderBufferManager* manager, RenderBufferType type,
                 const ImageSize& bufferSize)
        : RefCountVImpl(), Type(type), pManager(manager), pRenderTargetData(0), BufferSize(bufferSize) { }
    virtual ~RenderBuffer()
    { destroyRenderTargetData(); }

    virtual void AddRef() 
    {
        AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
    }
    virtual void Release()
    {
        if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
            delete this;
    }

    // RenderTargetData represents HW resource associated with render buffer,
    // allocated explicitly by the relevant HAL back end.
    // RenderTargetData will be deleted when RenderBuffer or its data destroyed;
    // back end detects this in destructor.
    class RenderTargetData : public NewOverrideBase<StatRender_TextureManager_Mem>
    {
    protected:
        RenderBuffer*           pBuffer;
    public:
        Ptr<DepthStencilBuffer> pDepthStencilBuffer;    // Depth stencil buffer used with this render target (0 if not required).
        UPInt                   CacheID;                // Stores information about cached results.

        RenderTargetData(RenderBuffer* buffer, DepthStencilBuffer* pdsb);
        virtual ~RenderTargetData();
    };

    // SetRenderTargetData assigns ownership to RenderBuffer which will destroy
    // it when no longer relevant.
    void        SetRenderTargetData(RenderTargetData* data)  { pRenderTargetData = data; }
    RenderTargetData*    GetRenderTargetData() const         { return pRenderTargetData; }

    //RenderBufferManager* GetBufferManager() const { return pManager; }
    RenderBufferType     GetType() const          { return Type; }
    
    // Returns underlying texture buffer size; needed when computing
    // U/V coordinates for rendering from the buffer texture.
    const ImageSize&    GetBufferSize() const { return BufferSize; }

protected:
    // Hide getManager() implementation since pointer may be
    // invalid during cleanup. If we made these public, we'd need
    // to null them out on RenderBufferManager::Destroy.
    RenderBufferManager* getManager() const { return pManager; }

    inline void destroyRenderTargetData();
    // avoid warning.
    void operator = (const RenderBuffer&) { }
    
    const RenderBufferType Type;
    RenderBufferManager*   pManager;
    RenderTargetData*               pRenderTargetData;
    ImageSize              BufferSize;
};



//------------------------------------------------------------------------

// TargetStatus describes the current "lifetime" status of a RenderTarget.
enum RenderTargetStatus
{
    RTS_InUse,       // Target has content that will be used in this frame.
    RTS_Available,   // Target is allocated and holds earlier content, if any.
    RTS_Unresolved,  // Target exists in video memory, but is not resolved to a texture.
    RTS_Lost         // Target has been lost or swapped out; must be recreated.
};

enum RenderTargetUse
{
    RTUse_Unused,           // Render Target is unused, and not useful as a cached result.
    RTUse_InUse,            // Render Target is current needed for rendering.
    RTUse_Unused_Cacheable, // Render Target is unused, but results may be useful for caching.
};


// Need linked list to keep track of these.
class RenderTarget : public RenderBuffer
{    
public:
    RenderTarget(RenderBufferManager* manager, RenderBufferType type,
                 const ImageSize& bufferSize)
        : RenderBuffer(manager, type, bufferSize), 
          ViewRect(bufferSize.Width, bufferSize.Height)
    { }

    // *** Dimensions

    // Allocated Render Target may be a part of a larger texture buffer,
    // potentially managed through packing for efficiency. To allow
    // for this, view rectangle and size is stored in addition to a
    // general buffer size.

    
    // GetRect returns the allocated rectangle for the render target.
    // This is an area in pixels that we should draw to. This rectangle
    // determines the rendering viewport when Target is applied to HAL.
    const Rect<int>& GetRect() const { return ViewRect; }

    // SetRect allows changing of the renderer modifiable area of the render target.
    // For example, if you want to render a scene to only a section of a full target.
    // This is really only valid for user-created types; Default and Temporary should 
    // be managed by the RenderBufferManager and set in the target directly.
    void SetRect(const Rect<int>& viewRect) 
    { 
        SF_ASSERT(GetType() == RBuffer_Texture || GetType() == RBuffer_User); 
        ViewRect = viewRect; 
    }

    // Obtain the size of the render-modifiable area in pixels. 
    // Note that this is different from the full texture size.
    const ImageSize     GetSize() const { return ImageSize(ViewRect.Width(), ViewRect.Height()); }

    // Return texture associated with the render target; available
    // for User textures. Will return null (0) for default back-buffer.
    virtual Texture*    GetTexture() const  { return 0; }

    // *** Rendering Status

    virtual RenderTargetStatus GetStatus() const { return RTS_Lost; }

    // Marks a render target as in use/not in use for this frame. SetInUse
    // is called automatically when render target is applied to the HAL; it
    // is also cleared automatically on EndFrame.
    virtual void        SetInUse(RenderTargetUse inUse) { SF_UNUSED(inUse); SF_DEBUG_ASSERT(0, "This should only be called for derived instances"); }

protected:
    // Rectangle that is a part of render target
    Rect<int>       ViewRect;
};




/*
namespace D3D9 {

class Target : public Render::Target
{
    IDirect3DTexture9* pTexture;
    IDirect3DSurface9* pDepthStencil;
public:
    Target(IDirect3DTexture9*, IDirect3DSurface9*, Rect<int> &viewRect)
    { }
};

};

*/

// DepthStencilBuffer is created internally by HAL but is managed through
// RenderBufferManager to enable reuse. Unlike RenderTarget, which is
// expected to service longer and thus has SetInUse function to control
// caching, DeptStencilBuffer is seen a temporary. Thus its Release()
// function play a role of making it available for use.

class DepthStencilBuffer : public RenderBuffer
{   
public:
    DepthStencilBuffer(RenderBufferManager* manager, const ImageSize& bufferSize, bool temporary=true)
        : RenderBuffer(manager, temporary ? RBuffer_DepthStencil : RBuffer_UserDepthStencil, bufferSize)
    { }

    virtual ~DepthStencilBuffer() { }
    virtual DepthStencilSurface*  GetSurface() const  { return 0; }
};

//------------------------------------------------------------------------

// RenderBufferManager is an abstract class used by Render::HAL implementations
// to manage cachable render targets.
//  - RenderBufferCache

class RenderBufferManager : public RefCountBase<RenderBufferManager, StatRender_TextureManager_Mem>
{
    friend class RenderBuffer;
public:

    virtual ~RenderBufferManager() { };

    // Initialize initialized the buffer manager to used specified TextureManager
    // for buffer creation.
    // - format is the default image format that should be requested for temp RT surfaces.
    // - screenSize is a hint to guide caching thresholds when applicable.
    // - On consoles, internal MemoryManager, if any can take precedence over
    //   the one associated with the texture manager.
    virtual bool    Initialize(TextureManager* manager, ImageFormat format,
                               const ImageSize& screenSize = ImageSize(0)) = 0;
    virtual void    Destroy() = 0;

    virtual void    EndFrame() = 0;

    virtual void    Reset() = 0;

    // Generic constructor for RenderTargets.
    // Texture* should be provided only for RBuffer_Texture type.
    virtual RenderTarget* CreateRenderTarget(const ImageSize& size, RenderBufferType type,
                                             ImageFormat format, Texture* texture = 0) = 0;

    // Allocates a temporary render target of specified type
    //  - call TextureManager to allocate texture
    virtual RenderTarget* CreateTempRenderTarget(const ImageSize& size) = 0;

    // Depth buffer
    //  - Finds existing buffer and/or creates a new one.
    //  - We would like to abstract this away on memory systems.
    //  - Managed separately from RTs
    //  - May need extra flags to distinguish between matching buffers, such as MSAA.
    virtual DepthStencilBuffer* CreateDepthStencilBuffer(const ImageSize& size, bool temporary=true) = 0;
};


//---------------------------------------------------------------------------------------
void RenderBuffer::destroyRenderTargetData()
{
    if (pRenderTargetData)
    {
        delete pRenderTargetData;
        pRenderTargetData = 0;
    }
}
inline RenderBuffer::RenderTargetData::RenderTargetData(RenderBuffer* buffer, DepthStencilBuffer* pdsb) : 
pBuffer(buffer), pDepthStencilBuffer(pdsb), CacheID(0) 
{
}

inline RenderBuffer::RenderTargetData::~RenderTargetData() 
{

}



//------------------------------------------------------------------------
/*

class HAL
{

    // *** Render Target creation

    // D3D9/Some other platform only. Returns render target that was configured
    // on the device at the time of SetDependentVideoMode call.
    // Target_Default
    virtual RenderTarget* GetDefaultRenderTarget() = 0;

    // Target_Texture
    virtual RenderTarget* CreateRenderTarget(Texture* texture, bool needsStencil) = 0;

    // Target_Temporary, created through delegation to RenderBufferManager.
    virtual RenderTarget* CreateTempRenderTarget(const ImageSize& size, bool needsStencil) = 0;

    // + system specific version:
    //virtual D3D9::RenderTarget* CreateRenderTarget(IDirect3DSurface9* textureSurface,
    //                                               IDirect3DSurface9* depthSurface)


    // *** Render target state management

    // 
    // Applies render target; should be called before BeginDisplay, but after
    // SetDependentVideoMode.
    virtual bool    SetRenderTraget(RenderTarget* target) = 0;


    // Begin rendering to the specified target; frameRect is the ortho projection.
    // Texture referenced by pRT must not be used until PopRenderTarget.
    virtual void    PushRenderTarget(const GRectF& frameRect, RenderTarget* prt) = 0;

    // Restore previous render target. Contents of Texture in popped render target are now available
    // for rendering.
    virtual void    PopRenderTarget() = 0;


};

*/




// Console RenderBufferManager works with provided memory manager directly,
// controlling allocations up to specified limit.
//  - Custom memory manager is passed to the TextureManager for allocations.
/*
class ConsoleRenderBufferManager : public MemoryManager
{
public:
    RenderBufferManager(TextureManager* manager, MemoryManager *mem,
                        UPInt memLimit = 0)
    {
    }

    // MemoryManager delegate overrides; provided to support overriding memory
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0);
    virtual void    Free(void* pmem, UPInt size);

};
*/

}}; // namespace Scaleform::Render

#endif // INC_SF_Render_Target_H
