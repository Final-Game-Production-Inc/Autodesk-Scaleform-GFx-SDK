/**************************************************************************

Filename    :   X360_MeshCache.cpp
Content     :   XBox360 Mesh Cache implementation
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/X360/X360_MeshCache.h"
#include "Render/X360/X360_MemoryManager.h"
#include "Render/X360/X360_ShaderDescs.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include <xgraphics.h>

namespace Scaleform { namespace Render { namespace X360 {

// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync )
 : SimpleMeshCache(pheap, params, rsync),
   pMemoryManager(NULL)
{
}

MeshCache::~MeshCache()
{
    Reset();
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool MeshCache::Initialize(IDirect3DDevice9* pdevice, MemoryManager * pmemmgr )
{
    pMemoryManager = pmemmgr;

    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;

    if (!createStaticVertexBuffers(pdevice) ||
        !allocateReserve())
    {
        return false;
    }

    return true;
}

void MeshCache::Reset()
{
    releaseAllBuffers();
}

SimpleMeshBuffer*  MeshCache::createHWBuffer(UPInt size, AllocType atype, unsigned arena)
{
    MeshBuffer* pbuffer = SF_HEAP_NEW(pHeap) MeshBuffer(size, atype, arena);
    if (!pbuffer)
        return 0;

    pbuffer->pData = (UByte*)pMemoryManager->Alloc(pbuffer->Size, VBAlignment, Memory_X360_Physical, arena);
    if (!pbuffer->pData)
    {
        delete pbuffer;
        return false;        
    }

    XGSetVertexBufferHeader(size, 0, 0, 0, &pbuffer->VertexBuffer);
    XGOffsetResourceAddress(&pbuffer->VertexBuffer, pbuffer->pData); 
    XGSetIndexBufferHeader(size, 0, SF_RENDER_X360_INDEX_FMT, 0, 0, &pbuffer->IndexBuffer);
    XGOffsetResourceAddress(&pbuffer->IndexBuffer, pbuffer->pData); 

    return pbuffer;
}

void        MeshCache::destroyHWBuffer(SimpleMeshBuffer* pbuffer)
{
    // Unset the Vertex/Index buffers on the device. If the buffer we are destroying is currently set
    // then this will cause a D3D assertion when setting the next buffers. Even though the buffer may
    // be set, the sync system should ensure that it is not in use.
    if (getRenderSync() && getRenderSync()->GetDevice())
    {
        getRenderSync()->GetDevice()->SetStreamSource(0, 0, 0, 0);
        getRenderSync()->GetDevice()->SetStreamSource(1, 0, 0, 0);
        getRenderSync()->GetDevice()->SetIndices(0);
    }
    pMemoryManager->Free(((MeshBuffer*)pbuffer)->pData, 0);
    delete pbuffer;
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    MeshCacheParams oldParams(Params);
    CacheList.EvictAll();
    Params = argParams;
    adjustMeshCacheParams(&Params);

    if (getRenderSync() && getRenderSync()->GetDevice())
    {
        if (Params.StagingBufferSize != oldParams.StagingBufferSize)
        {
            if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
            {
                if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
                {
                    SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore StagingBuffer after fail");
                }
                return false;
            }
        }

        if ((Params.MemReserve != oldParams.MemReserve) ||
            (Params.MemGranularity != oldParams.MemGranularity))
        {
            releaseAllBuffers();

            // Allocate new reserve. If not possible, restore previous one and fail.
            if (Params.MemReserve && !allocateReserve())
            {
                SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore Reserve after fail");
            }
        }
    }
    return true;
}

void MeshCache::adjustMeshCacheParams(MeshCacheParams* p)
{
    p->MaxBatchInstances    = Alg::Clamp<unsigned>(p->MaxBatchInstances, 1, SF_RENDER_MAX_BATCHES);
    p->VBLockEvictSizeLimit = Alg::Clamp<UPInt>(p->VBLockEvictSizeLimit, 0, p->VBLockEvictSizeLimit = 1024 * 256);
}

bool MeshCache::createStaticVertexBuffers(IDirect3DDeviceX* pdevice)
{
    return createMaskEraseBatchVertexBuffer(pdevice);
}

bool MeshCache::createMaskEraseBatchVertexBuffer(IDirect3DDeviceX* pdevice)
{
    unsigned     bufferSize = sizeof(VertexXY16iAlpha) * 6 * SF_RENDER_MAX_BATCHES;
    VertexXY16iAlpha* pbuffer = 0;

    pbuffer = (VertexXY16iAlpha*)pMemoryManager->Alloc( bufferSize, VBAlignment, Memory_X360_Physical );
    if ( pbuffer == NULL )
        return false;

    fillMaskEraseVertexBuffer<VertexXY16iAlpha>(pbuffer, SF_RENDER_MAX_BATCHES);
    XGSetVertexBufferHeader( bufferSize, 0, 0, 0, &MaskEraseBatchVertexBuffer);
    XGOffsetResourceAddress( &MaskEraseBatchVertexBuffer, pbuffer );
    return true;
}

void MeshCache::PostUpdateMesh(Render::MeshCacheItem * pcacheItem )
{
    X360::MeshCacheItem * pitem = (X360::MeshCacheItem *)pcacheItem;
    void * allocAddress = reinterpret_cast<void*>(pitem->GetAllocAddr());
    UPInt allocSize = pitem->AllocSize;

    // Invalidate the CPU cache. This does not actually appear to be necessary.
    //unsigned k;
    //for ( k = 0; k < allocSize; k+= 32 )
    //    __dcbf(k, (const void*)allocAddress);

    // We invalidate GPU cache after we've written data to it, so that stale content
    // does not accidentally get used when rendering.
    if (getRenderSync() && getRenderSync()->GetDevice())
        getRenderSync()->GetDevice()->InvalidateGpuCache(allocAddress, allocSize, 0);
}

}}}; // namespace Scaleform::Render::X360

