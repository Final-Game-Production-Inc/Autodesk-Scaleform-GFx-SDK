/**************************************************************************

Filename    :   PS3_MeshCache.cpp
Content     :   PS3 Mesh Cache implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/PS3/PS3_MeshCache.h"
#include "Render/PS3/PS3_ShaderDescs.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"


namespace Scaleform { namespace Render { namespace PS3 {

// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync)
    : SimpleMeshCache(pheap, params, rsync),
      RSync(rsync),
      pMaskEraseBatchVertexBuffer(0)
{
}

MeshCache::~MeshCache()
{
    Reset(); 
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool    MeshCache::Initialize(CellGcmContextData* gcm, MemoryManager* pmem)
{
    pMemManager = pmem;

    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;

    if (!allocateReserve())
        return false;

    if (!createStaticVertexBuffers())
        return false;

    return true;
}

void    MeshCache::Reset()
{
    if (pMaskEraseBatchVertexBuffer)
    {
        pMemManager->Free(pMaskEraseBatchVertexBuffer, sizeof(VertexXY16iInstance) * 6 * SF_RENDER_MAX_BATCHES);
        pMaskEraseBatchVertexBuffer = 0;
    }
    if (RSync->GetContext())
        releaseAllBuffers();
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    MeshCacheParams oldParams(Params);
    CacheList.EvictAll();
    Params = argParams;
    adjustMeshCacheParams(&Params);

    if (RSync->GetContext())
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

bool MeshCache::createStaticVertexBuffers()
{
    return createInstancingVertexBuffer() && 
           createMaskEraseBatchVertexBuffer();
}

bool MeshCache::createInstancingVertexBuffer()
{
    return true;
}

bool MeshCache::createMaskEraseBatchVertexBuffer()
{
    pMaskEraseBatchVertexBuffer = (VertexXY16iInstance*)
        pMemManager->Alloc(sizeof(VertexXY16iInstance) * 6 * SF_RENDER_MAX_BATCHES, 16, Memory_PS3_MainVideo);
    if (UPInt(pMaskEraseBatchVertexBuffer) == ~UPInt(0))
        return false;

    fillMaskEraseVertexBuffer<VertexXY16iAlpha>(
        reinterpret_cast<VertexXY16iAlpha*>(pMaskEraseBatchVertexBuffer), SF_RENDER_MAX_BATCHES);
    return true;
}

void MeshCache::adjustMeshCacheParams(MeshCacheParams* p)
{
    p->MaxBatchInstances    = Alg::Clamp<unsigned>(p->MaxBatchInstances, 1, SF_RENDER_MAX_BATCHES);
    p->VBLockEvictSizeLimit = Alg::Clamp<UPInt>(p->VBLockEvictSizeLimit, 0, p->VBLockEvictSizeLimit = 1024 * 256);
    p->InstancingThreshold  = 1<<30;
}

SimpleMeshBuffer*  MeshCache::createHWBuffer(UPInt size, AllocType atype, unsigned arena)
{
    SimpleMeshBuffer* pbuffer = SF_HEAP_NEW(pHeap) SimpleMeshBuffer(size, atype, arena);
    if (!pbuffer)
        return 0;

    pbuffer->pData = (UByte*)pMemManager->Alloc(pbuffer->Size, 16, Memory_PS3_MainVideo, arena);
    if (UPInt(pbuffer->pData) == ~UPInt(0))
    {
        delete pbuffer;
        return false;
    }

    return pbuffer;
}

void        MeshCache::destroyHWBuffer(SimpleMeshBuffer* pbuffer)
{
    pMemManager->Free(pbuffer->pData, pbuffer->Size);
    delete pbuffer;
}

void MeshCache::PostUpdateMesh(Render::MeshCacheItem * pcacheItem )
{
    SF_UNUSED(pcacheItem);

    // We invalidate GPU cache after we've written data to it, so that stale content
    // does not accidentally get used when rendering.
    cellGcmSetInvalidateVertexCache(RSync->GetContext());
}


}}}; // namespace Scaleform::Render::PS3

