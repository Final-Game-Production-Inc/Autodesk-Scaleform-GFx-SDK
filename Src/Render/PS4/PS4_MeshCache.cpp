/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_MeshCache.cpp
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/PS4/PS4_MeshCache.h"
#include "Render/PS4/PS4_ShaderDescs.h"
#include "Render/PS4/PS4_MemoryManager.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"


namespace Scaleform { namespace Render { namespace PS4 {

// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync)
    : SimpleMeshCache(pheap, params, rsync),
    RSync(rsync)
{
}

MeshCache::~MeshCache()
{
    Reset(); 
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool    MeshCache::Initialize(MemoryManager* pmem)
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
    if (pMaskEraseBatchVertexBuffer[0].isBuffer())
    {
        void * gpuPtr = reinterpret_cast<void*>(pMaskEraseBatchVertexBuffer[0].getBaseAddress());
        pMemManager->Free(gpuPtr, sizeof(VertexXY16iAlpha) * 6 * SF_RENDER_MAX_BATCHES);
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
    void *gpubuffer = pMemManager->Alloc(sizeof(VertexXY16fAlpha) * 6 * SF_RENDER_MAX_BATCHES, sce::Gnm::kAlignmentOfBufferInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE); 
    if (UPInt(gpubuffer) == ~UPInt(0))
        return false;
    VertexXY16fAlpha* buffer = reinterpret_cast<VertexXY16fAlpha*>(gpubuffer);

    pMaskEraseBatchVertexBuffer[0].initAsVertexBuffer(gpubuffer, sce::Gnm::kDataFormatR32G32Float, sizeof(VertexXY16fAlpha), 6 * SF_RENDER_MAX_BATCHES);
    pMaskEraseBatchVertexBuffer[1].initAsVertexBuffer(reinterpret_cast<UByte*>(gpubuffer) + 2*sizeof(float), sce::Gnm::kDataFormatR8G8B8A8Unorm, 
        sizeof(VertexXY16fAlpha), 6 * SF_RENDER_MAX_BATCHES);
    SF_DEBUG_ASSERT(pMaskEraseBatchVertexBuffer[0].isBuffer(), "Vertex Buffer failed isBuffer test.");
    SF_DEBUG_ASSERT(pMaskEraseBatchVertexBuffer[1].isBuffer(), "Vertex Buffer failed isBuffer test.");

    fillMaskEraseVertexBuffer<VertexXY16fAlpha>(buffer, SF_RENDER_MAX_BATCHES);
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

    void *gpuPtr = pMemManager->Alloc(pbuffer->Size, 16, Memory_Orbis_WC_GARLIC_NONVOLATILE, arena);
    pbuffer->pData = gpuPtr;
    if (UPInt(pbuffer->pData) == ~UPInt(0))
    {
        delete pbuffer;
        return 0;
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
    // TODOBM: do we need to do anything here?
}


}}}; // namespace Scaleform::Render::Orbis
