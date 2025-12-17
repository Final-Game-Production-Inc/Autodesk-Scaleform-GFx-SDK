/**********************************************************************

PublicHeader:   Render
Filename    :   Render_BufferMemory.cpp
Content     :   
Created     :   Sep 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/X360/X360_BufferMemory.h"
#include "Render/X360/X360_HAL.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace X360 { 

RenderBufferManager::RenderBufferManager( IDirect3DDevice9* pdevice, MemoryHeap* pheap ) :
    Render::RenderBufferManager(),
    pDevice(pdevice),
    pTextureManager(0),
    pManagerLocks(0),
    pMainHeap(pheap),
    VideoAlloc(pMainHeap),
    EDRAMAlloc(pMainHeap, 0, GPU_EDRAM_TILES),
    AvailableMemory(0)
{
    SF_ASSERT(pdevice);
    pDevice->AddRef();
}

RenderBufferManager::~RenderBufferManager()
{
    Destroy();
}


bool RenderBufferManager::Initialize( Render::TextureManager* manager, ImageFormat format, const ImageSize& screenSize)
{
    pTextureManager = (X360::TextureManager*)manager;
    pManagerLocks = *SF_HEAP_NEW(pMainHeap) TextureManagerLocks((X360::TextureManager*)manager);
    SF_UNUSED2(format, screenSize);
    return true;
}

Render::RenderTarget* RenderBufferManager::CreateTempRenderTarget( const ImageSize& size )
{
    SInt32 edramTile;
    unsigned edramSize;
    IDirect3DSurface9* surface = allocateSurface(size, D3DFMT_A8R8G8B8, edramTile, edramSize );
    if ( !surface )
        return 0;
    IDirect3DTexture9* texture = allocateRenderTargetResolve(size);
    if ( !texture )
        return 0;

    // Allocate texture/render target. NOTE: this is outside the texture manager's control.
    Ptr<X360::Texture> pt = *SF_HEAP_NEW(pMainHeap) X360::Texture(pManagerLocks, texture, size, 0, 0);
    X360::RenderTarget* target = SF_HEAP_NEW(pMainHeap) X360::RenderTarget(this, RBuffer_Temporary, size);

    pt->pTextures[0].Surface  = surface;
    pt->pTextures[0].Tex      = texture;
    pt->pTextures[0].Size     = size;
    pt->pTextures[0].pTexData = 0;  // don't give texture manager the pointer, we manage it manually.

    target->initSurface(edramTile, edramSize);
    target->initTexture(pt);
    target->initViewRect(Rect<int>(size.Width, size.Height));
    RenderTargets.PushBack(target);
    target->AddRef();

    return target;
}

bool RenderBufferManager::AddVideoMemory( IDirect3DTexture9* ptexture )
{
    UINT baseData, baseSize;
    XGGetTextureLayout(ptexture, &baseData, &baseSize, 0, 0, 0, 0, 0, 0, 0, 0);
    return AddVideoMemory(baseData, baseSize);
}

bool RenderBufferManager::AddVideoMemory( UPInt videoAddr, UPInt videoSize )
{
    // Must pass a valid address and a minimum of 4k.
    if ( videoAddr == 0 || videoSize < X360_TEXTURE_MEMORY_ALIGN )
        return false;

    // NOTE: we shift down the allocation and size by the required alignment of textures. This ensure we are
    // always allocating aligned memory, without any extra work during allocation time.
    VideoAlloc.AddSegment(videoAddr >> X360_TEXTURE_MEMORY_SHIFT, 
        videoSize >> X360_TEXTURE_MEMORY_SHIFT);
    AvailableMemory += videoSize;
    return true;
}

void RenderBufferManager::RemoveVideoMemory( UPInt videoAddr, UPInt videoSize )
{
    // Find cached Rendertargets which fall within this range. These must
    // be evicted, because this memory will no longer be available. If this is done
    // between the prepass and final pass, the content will not be filtered, as all
    // filtered content on X360 must be cached.
    X360::RenderTarget* prt = RenderTargets.GetFirst();
    while ( !RenderTargets.IsNull(prt) )
    {
        UPInt textData = (UPInt)prt->GetTexture()->pTextures[0].pTexData;
        X360::RenderTarget* prtNext = prt->pNext;
        if ( textData >= videoAddr && textData < (videoAddr+videoSize))
        {
            SF_ASSERT(prt->GetStatus() != RTS_InUse );
            clearRenderTargetResolve(*prt);
        }
        prt = prtNext;
    }

    // Remove the segment from the allocator.
    VideoAlloc.RemoveSegment(videoAddr >> X360_TEXTURE_MEMORY_SHIFT, 
        videoSize >> X360_TEXTURE_MEMORY_SHIFT);
    // All memory in the segment should be unused, therefore we do not reduce AvailableMemory.
}

void RenderBufferManager::Destroy()
{
    Reset();

    AvailableMemory = 0;

    if (pDevice)
        pDevice->Release();
    pDevice = 0;

    pManagerLocks = 0;
}

void RenderBufferManager::clearRenderTargetSurface( RenderTarget& rt )
{
    X360::Texture* pt = rt.GetTexture();
    if ( !pt || !pt->pTextures[0].Surface )
        return;

    // If the texture has no remaining texture pointer, it is lost.
    if ( !pt->pTextures[0].Tex )
        rt.RTStatus = RTS_Lost;

    // Make sure that the surface isn't set on the device before destroying it.
    if ( pt->pTextures[0].Surface->IsSet(pDevice) )
        pDevice->SetRenderTarget(0, 0);

    //printf("Free (surface): 0x%x\n", pt->pTextures[0].Surface );
    EDRAMAlloc.Free(rt.EDRAMTile, rt.EDRAMSize);
    XGOffsetSurfaceAddress(pt->pTextures[0].Surface, 0, 0);
    SF_HEAP_FREE(pMainHeap, pt->pTextures[0].Surface);
    pt->pTextures[0].Surface = 0;

    // Clear the render surface from the RenderTargetData.
    RenderTargetData* phd = (X360::RenderTargetData*)rt.GetRenderTargetData();
    phd->pRenderSurface = 0;
}

void RenderBufferManager::clearRenderTargetResolve( RenderTarget& rt )
{
    X360::Texture* pt = rt.GetTexture();
    if ( !pt || !pt->pTextures[0].Tex )
        return;

    SF_ASSERT(pt->pTextures[0].Tex);
    //printf("Free (texture): 0x%x\n", pt->pTextures[0].Tex );

    // If the texture still have a surface, it could be resolved again.
    if ( pt->pTextures[0].Surface )
        rt.RTStatus = RTS_Unresolved;
    else
        rt.RTStatus = RTS_Lost;

    // Must unset the textures on the device and wait for the resource to be not-busy before deleting it.
    TextureManager* pmgr = pt->GetManager();
    for ( unsigned i = 0; i < pmgr->MaximumStages; ++i)
        pmgr->SetSamplerState(i, 0);
    pt->pTextures[0].Tex->BlockUntilNotBusy();

    UINT baseData, baseSize;
    XGGetTextureLayout(pt->pTextures[0].Tex, &baseData, &baseSize, 0, 0, 0, 0, 0, 0, 0, 0);
    XGOffsetResourceAddress(pt->pTextures[0].Tex, 0);
    VideoAlloc.Free(baseData>>X360_TEXTURE_MEMORY_SHIFT, baseSize>>X360_TEXTURE_MEMORY_SHIFT);
    SF_HEAP_FREE(pMainHeap, pt->pTextures[0].Tex);
    pt->pTextures[0].Tex = 0;

    // Clear the render target from the RenderTargetData
    RenderTargetData* phd = (X360::RenderTargetData*)rt.GetRenderTargetData();
    phd->pResolveTarget = 0;

    AvailableMemory += baseSize;
}

void RenderBufferManager::clearDepthStencilSurface( DepthStencilBuffer& ds )
{
    if ( !ds.GetSurface() || !ds.GetSurface()->pDepthStencilSurface )
        return;

    DepthStencilSurface* pdss = ds.GetSurface();
    //printf("Free (surface): 0x%x\n", pdss->pDepthStencilSurface );

    // Make sure that the surface isn't set on the device before destroying it.
    if ( pdss->pDepthStencilSurface->IsSet(pDevice) )
        pDevice->SetDepthStencilSurface(0);

    XGOffsetSurfaceAddress(pdss->pDepthStencilSurface, 0, 0);
    EDRAMAlloc.Free(ds.EDRAMTile, ds.EDRAMSize);
    SF_HEAP_FREE(pMainHeap, pdss->pDepthStencilSurface);
    pdss->pDepthStencilSurface = 0;

}

Render::DepthStencilBuffer* RenderBufferManager::CreateDepthStencilBuffer( const ImageSize& size )
{
    DepthStencilBuffer* pdsb = SF_HEAP_NEW(pMainHeap) DepthStencilBuffer(this, size);
    DepthStencilSurface* pdss = SF_HEAP_NEW(pMainHeap) DepthStencilSurface(0, size);
    pdsb->pSurface = pdss;

    IDirect3DSurface9* surface = allocateSurface( size, D3DFMT_D24S8, pdsb->EDRAMTile, pdsb->EDRAMSize );
    if ( !surface )
    {
        // pdss is deleted automatically, smart pointer within pdsb.
        delete pdsb;
        return 0;   
    }
    pdss->pDepthStencilSurface = surface;
    return pdsb;    
}

IDirect3DSurface9* RenderBufferManager::allocateSurface( const ImageSize& size, D3DFORMAT format, SInt32& edramLocation, unsigned& edramSize )
{
    if ( size.Width == 0 || size.Height == 0 )
        return 0;

    edramSize = XGSurfaceSize( size.Width, size.Height, format, D3DMULTISAMPLE_NONE );
    edramLocation = EDRAMAlloc.Alloc(edramSize);
    if ( edramLocation == ~UPInt(0) )
    {
        X360::RenderTarget* prt = RenderTargets.GetFirst();
        X360::RenderTarget* prtNext;

        // Remove all other render target surfaces in EDRAM by resolving them.
        while ( !RenderTargets.IsNull(prt) )
        {
            prtNext = prt->pNext;
            if (prt->GetStatus() == RTS_Unresolved ||
                prt->GetStatus() == RTS_Available )
            {
                X360::RenderTargetData* phd = (X360::RenderTargetData*)prt->GetRenderTargetData();            
                if ( prt->GetStatus() == RTS_Unresolved && phd->pResolveTarget)
                {
                    pDevice->Resolve(D3DRESOLVE_RENDERTARGET0 | D3DRESOLVE_CLEARRENDERTARGET, 0, phd->pResolveTarget, 
                        0, 0, 0, 0, 1.0f, 0, 0 );
                }
                clearRenderTargetSurface(*prt);
                if ( prt->GetStatus() == RTS_Lost )
                {
                    prt->RemoveNode();
                    prt->Release();
                }
            }
            prt = prtNext;
        }

        // Try to allocate again.
        edramLocation = EDRAMAlloc.Alloc(edramSize);
        if ( edramLocation == ~UPInt(0))
            return 0;
    }
    // Allocate CPU-side strutures.
    IDirect3DSurface9* surface = (IDirect3DSurface9*)SF_HEAP_ALLOC(pMainHeap, sizeof(IDirect3DSurface9), Stat_Default_Mem);
    if ( !surface )
    {
        EDRAMAlloc.Free(edramLocation, edramSize);
        return 0;
    }

    //printf("Allocate (surface): 0x%x\n", surface );

    // Set the surface information, and return it.
    D3DSURFACE_PARAMETERS surfParams;
    memset(&surfParams, 0, sizeof(D3DSURFACE_PARAMETERS));
    surfParams.Base = edramLocation;
    surfParams.HierarchicalZBase = 0xFFFFFFFF;
    XGSetSurfaceHeader( size.Width, size.Height, format, D3DMULTISAMPLE_NONE, &surfParams, surface, 0);
    return surface;

}

IDirect3DTexture9* RenderBufferManager::allocateRenderTargetResolve( const ImageSize &size )
{
    if ( size.Width == 0 || size.Height == 0 )
        return 0;

    IDirect3DTexture9* texture = (IDirect3DTexture9*)SF_HEAP_ALLOC(pMainHeap, sizeof(IDirect3DTexture9), Stat_Default_Mem);
    if ( !texture ) 
        return 0;

    UINT baseSize;
    XGSetTextureHeader( size.Width, size.Height, 1, 0, D3DFMT_A8R8G8B8, 0, 0, XGHEADER_CONTIGUOUS_MIP_OFFSET, 0, texture, &baseSize, 0);
    const unsigned shift = X360_TEXTURE_MEMORY_SHIFT;
    UPInt textureMemory = VideoAlloc.Alloc((baseSize+(1<<(shift-1)))>>shift);
    if ( textureMemory != ~UPInt(0) )
    {
        //printf("Allocate (resolve): 0x%x\n", texture );
        AvailableMemory -= (baseSize+(1<<(shift-1)));
        XGOffsetResourceAddress(texture, reinterpret_cast<void*>(textureMemory<<shift));
        return texture;
    }

    // Try to evict any available render targets, and allocate again.
    X360::RenderTarget* prt = RenderTargets.GetFirst();
    while ( !RenderTargets.IsNull(prt) )
    {
        X360::RenderTarget* prtNext = prt->pNext;
        if ( prt->GetStatus() == RTS_Available )
        {
            clearRenderTargetResolve(*prt); 
            if ( prt->GetStatus() == RTS_Lost )
            {
                prt->RemoveNode();
                prt->Release();
            }
        }
        prt = prtNext;
    }

    // Try to allocate again (pass or fail).
    textureMemory = VideoAlloc.Alloc((baseSize+(1<<(shift-1)))>>shift);
    if ( textureMemory != ~UPInt(0))
    {
        //printf("Allocate (resolve): 0x%x\n", texture );
        AvailableMemory -= (baseSize+(1<<(shift-1)));
        XGOffsetResourceAddress(texture, reinterpret_cast<void*>(textureMemory<<shift));
        return texture;
    }
    SF_HEAP_FREE(pMainHeap, texture);
    return 0;
}

void RenderBufferManager::EndFrame()
{
#ifdef SF_BUILD_DEBUG
    // Make sure no render targets are in use (debug only)
    X360::RenderTarget* prt = RenderTargets.GetFirst();
    while ( !RenderTargets.IsNull(prt) )
    {
        SF_ASSERT(prt->RTStatus != RTS_InUse);
        prt = prt->pNext;
    }
#endif
}

void RenderBufferManager::Reset()
{
    // Evict all render targets.
    X360::RenderTarget* prt = RenderTargets.GetFirst();
    while ( !RenderTargets.IsNull(prt) )
    {
        clearRenderTargetSurface(*prt);
        clearRenderTargetResolve(*prt);
        X360::RenderTarget* prtNext = RenderTargets.GetNext(prt);
        prt->RemoveNode();
        prt->Release();
        prt = prtNext;
    }
    SF_ASSERT(RenderTargets.IsEmpty());
}

Render::RenderTarget* RenderBufferManager::CreateRenderTarget( const ImageSize& size, RenderBufferType type, ImageFormat format, Render::Texture* texture )
{
    SF_ASSERT(type != RBuffer_DepthStencil);
    RenderTarget* target = SF_HEAP_AUTO_NEW(this) RenderTarget(this, type, size);
    if (target)
    {
        // Apply a texture if relevant.
        target->initTexture((X360::Texture*)texture);
        target->initViewRect(Rect<int>(size.Width, size.Height));
    }

    // By default, render targets don't get added to the list,
    // as it is up to end user to free them.
    return target;
}

void RenderTarget::SetInUse( RenderTargetUse inUse)
{
    // If we are not already lost, and unused, kill the render target immediately.
    bool release = false;
    if ( RTStatus != RTS_Lost && inUse == RTUse_Unused )
    {
        if ( GetType() == RBuffer_Temporary )
        {
            RenderBufferManager* mgr = getManager();
            mgr->clearRenderTargetResolve(*this);
            mgr->clearRenderTargetSurface(*this);
            RemoveNode();
            release = true;
        }
        pTexture = 0;
    }

    // If we are cacheable, set it to be available, which may mean it could be evicted.
    if ( inUse == RTUse_Unused_Cacheable )
        RTStatus = RTS_Available;

    // Must release last, just in case we're going to be deleted.
    if ( release )
        Release();
}

}}}; // Scaleform::Render::X360
