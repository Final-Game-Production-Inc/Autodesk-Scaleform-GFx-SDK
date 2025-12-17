/**********************************************************************

PublicHeader:   Render
Filename    :   X360_BufferMemory.h
Content     :   RenderBufferManager implementation, which allocates all 
                rendering surfaces from user provided memory block(s). 
Created     :   Sept 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_X360_BufferMemory_H
#define INC_SF_X360_BufferMemory_H

#include "Render/Render_Buffer.h"
#include "Render/X360/X360_Texture.h"
#include <xgraphics.h>

namespace Scaleform { namespace Render { namespace X360 { 

class RenderBufferManager;
typedef X360::RenderBufferManager RenderBufferManagerMemory;

//------------------------------------------------------------------------
class RenderTarget;
class DepthStencilBuffer;

static const int X360_TEXTURE_MEMORY_ALIGN = 4096;
static const int X360_TEXTURE_MEMORY_SHIFT = 12;

class RenderBufferManager : public Render::RenderBufferManager
{
    friend class RenderTarget;
    friend class DepthStencilBuffer;    

public:

    RenderBufferManager( IDirect3DDevice9* pdevice, MemoryHeap* pheap = Memory::GetGlobalHeap() );
    virtual ~RenderBufferManager();

    virtual bool Initialize( Render::TextureManager* manager, ImageFormat format, const ImageSize& screenSize = ImageSize(0) );

    // Adds a block of video memory from a texture to be used for render buffer allocation.
    bool AddVideoMemory( IDirect3DTexture9* ptexture );

    // Adds an block of video memory to be used for render buffer allocation. 
    // Returns false if the memory provided was invalid.
    virtual bool AddVideoMemory( UPInt videoAddr, UPInt videoSize );

    // Removes a block of video memory. This can only be done 
    virtual void RemoveVideoMemory( UPInt videoAddr, UPInt videoSize );

    virtual void Destroy();
    virtual void EndFrame();
    virtual void Reset();

    virtual Render::RenderTarget* CreateRenderTarget( const ImageSize& size, RenderBufferType type, ImageFormat format, Render::Texture* texture = 0 );
    virtual Render::RenderTarget* CreateTempRenderTarget( const ImageSize& size );
    virtual Render::DepthStencilBuffer* CreateDepthStencilBuffer( const ImageSize& size );

protected:

    IDirect3DSurface9* allocateSurface( const ImageSize& size, D3DFORMAT format, SInt32& edramLocation, unsigned& edramSize );
    IDirect3DTexture9* allocateRenderTargetResolve( const ImageSize &size );

    void clearRenderTargetSurface( RenderTarget& rt );
    void clearRenderTargetResolve( RenderTarget& rt );
    void clearDepthStencilSurface( DepthStencilBuffer& dsb );

    IDirect3DDevice9*               pDevice;             // D3D Device
    X360::TextureManager*           pTextureManager;     // Texture manager
    Ptr<TextureManagerLocks>        pManagerLocks;       // Hiding access to the locking mechanisms within TextureManager.
    MemoryHeap*                     pMainHeap;           // Heap from which main memory allocations are made.
    AllocAddr                       VideoAlloc;          // Allocator managing video memory blocks.
    AllocAddr                       EDRAMAlloc;          // Allocator for managing EDRAM.
    UPInt                           AvailableMemory;     // Total amount of available memory.
    List<X360::RenderTarget>        RenderTargets;       // List of currently allocated render targets.
    List<X360::DepthStencilBuffer>  DepthStencilBuffers; // List of currently allocated depth stencil buffers.
};

//------------------------------------------------------------------------
class RenderTarget : public Render::RenderTarget, public ListNode<RenderTarget>
{
    friend class RenderBufferManager;
public:
    RenderTarget(RenderBufferManager* manager, RenderBufferType type,
        const ImageSize& bufferSize)
        : Render::RenderTarget(manager, type, bufferSize),
          ListNode<RenderTarget>(),
          EDRAMTile(-1),
          EDRAMSize(0),
          pTexture(0),
          RTStatus(RTS_Lost)
    { }

    virtual ~RenderTarget()
    {
        Render::RenderTarget::SetInUse(false);
    }

    virtual Texture*    GetTexture() const  
    { 
        return pTexture; 
    }

    virtual RenderTargetStatus GetStatus() const 
    { 
        return RTStatus; 
    }

    virtual void        SetInUse(RenderTargetUse inUse);

    virtual void      AddRef()  { RefCount++; }
    virtual void      Release() 
    { 
        RefCount--; 
        if ( RefCount == 0 ) 
            delete this; 
    } 

protected:
    // Return derived-class version of manager.
    RenderBufferManager* getManager() const { return (RenderBufferManager*)pManager; }

    void initSurface(SInt32 edramTile, unsigned edramSize )
    {
        EDRAMTile = edramTile;
        EDRAMSize = edramSize;
    }

    void initTexture(X360::Texture* texture)
    { 
        pTexture = texture; 
        RTStatus = RTS_InUse;
    }

    void initViewRect(const Rect<int>& viewRect)
    {
        ViewRect = viewRect;
    }    

    SInt32              EDRAMTile;
    unsigned            EDRAMSize;

    Ptr<X360::Texture>  pTexture;
    RenderTargetStatus  RTStatus;
};


// DepthStencilBuffer implementation with swapping support through CacheData;
// holds DepthStencilSurface.

class DepthStencilBuffer : public Render::DepthStencilBuffer, public ListNode<DepthStencilBuffer>
{
    friend class RenderBufferManager;
public:
    DepthStencilBuffer(RenderBufferManager* manager, const ImageSize& bufferSize)
        : Render::DepthStencilBuffer(manager, bufferSize), ListNode<DepthStencilBuffer>(),
          DSStatus(RTS_Lost), EDRAMTile(-1), EDRAMSize(0), pSurface(0)
    { }

    virtual void      AddRef()  { RefCount++; }
    virtual void      Release() { RefCount--; if ( RefCount == 0 ) delete this; } 
    
    virtual ~DepthStencilBuffer()
    {
        if ( getManager() )
            getManager()->clearDepthStencilSurface(*this);
    }

    virtual DepthStencilSurface* GetSurface() const  { return pSurface; }

protected:
    void initSurface(DepthStencilSurface* surface, SInt32 edramTile, unsigned edramSize )
    { 
        pSurface = surface; 
        EDRAMTile = edramTile;
        EDRAMSize = edramSize;
    }

    RenderBufferManager* getManager() const { return (RenderBufferManager*)pManager; }

    RenderTargetStatus          DSStatus;
    SInt32                      EDRAMTile;
    unsigned                    EDRAMSize;
    Ptr<DepthStencilSurface>    pSurface;
};

}}} // Scaleform::Render::X360

#endif // INC_SF_X360_BufferMemory_H
