/**************************************************************************

Filename    :   D3D9_MeshCache.cpp
Content     :   D3D9 Mesh Cache implementation
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D9/D3D9_MeshCache.h"
#include "Render/D3D9/D3D9_ShaderDescs.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_HeapNew.h"

//#define SF_RENDER_LOG_CACHESIZE

#include <stdio.h>

namespace Scaleform { namespace Render { namespace D3D9 {


// Helpers used to initialize default granularity sizes,
// splitting VB/Index size by 5/9.
inline UPInt calcVBGranularity(UPInt granularity)
{
    return (((granularity >> 4) * 5) / 9) << 4;
}
inline UPInt calcIBGranularity(UPInt granularity, UPInt vbGranularity)
{
    return (((granularity >> 4) - (vbGranularity >> 4)) << 4);
}


// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap,
                     const MeshCacheParams& params)
 : Render::MeshCache(pheap, params),
   pDevice(0), CacheList(getThis()), BufferCreateFlags(0),
   VertexBuffers(pheap, calcVBGranularity(params.MemGranularity)),
   IndexBuffers(pheap, calcIBGranularity(params.MemGranularity,
                                         VertexBuffers.GetGranularity())),
   Locked(false), VBSizeEvictedInLock(0)
{
    adjustMeshCacheParams(&Params);
}

MeshCache::~MeshCache()
{
    Reset();
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool MeshCache::Initialize(IDirect3DDevice9* pdevice, bool dynamicMeshes)
{
    SF_ASSERT(!pDevice);    
    adjustMeshCacheParams(&Params, pdevice);

    BufferCreateFlags = dynamicMeshes ? Buffer_Dynamic : 0;

    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;

    if (!createStaticVertexBuffers(pdevice))
    {
        Reset();
        return false;
    }

    pDevice = pdevice;
    if (Params.MemReserve &&
        !allocCacheBuffers(Params.MemReserve, MeshBuffer::AT_Reserve))
    {
        Reset();
        return false;
    }
    
    return true;
}

void MeshCache::Reset()
{
    if (pDevice)
        destroyBuffers();
    // Unconditional to simplify Initialize fail logic:
    pInstancingVertexBuffer.Clear();
    pMaskEraseBatchVertexBuffer.Clear();
    pDevice.Clear();
    StagingBuffer.Reset();
}


void MeshCache::ClearCache()
{
    destroyBuffers(MeshBuffer::AT_Chunk);
    StagingBuffer.Reset();
    StagingBuffer.Initialize(pHeap, Params.StagingBufferSize);
    SF_ASSERT(BatchCacheItemHash.GetSize() == 0);
}

void MeshCache::destroyBuffers(MeshBuffer::AllocType at)
{
    // TBD: Evict everything first!
    CacheList.EvictAll();
    VertexBuffers.DestroyBuffers(at);
    IndexBuffers.DestroyBuffers(at);
    ChunkBuffers.Clear();
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    MeshCacheParams params(argParams);
    adjustMeshCacheParams(&params, pDevice);

    if (pDevice)
    {
        CacheList.EvictAll();

        if (Params.StagingBufferSize != params.StagingBufferSize)
        {
            if (!StagingBuffer.Initialize(pHeap, params.StagingBufferSize))
            {
                if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
                {
                    SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore StagingBuffer after fail");
                }
                return false;
            }
        }

        if ((Params.MemReserve != params.MemReserve) ||
            (Params.MemGranularity != params.MemGranularity))
        {
            destroyBuffers();

            // Allocate new reserve. If not possible, restore previous one and fail.
            if (params.MemReserve &&
                !allocCacheBuffers(params.MemReserve, MeshBuffer::AT_Reserve))
            {
                if (Params.MemReserve &&
                    !allocCacheBuffers(Params.MemReserve, MeshBuffer::AT_Reserve))
                {
                    SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore Reserve after fail");
                }
                return false;
            }

            VertexBuffers.SetGranularity(calcVBGranularity(params.MemGranularity));
            IndexBuffers.SetGranularity(calcIBGranularity(params.MemGranularity,
                                        VertexBuffers.GetGranularity()));
        }
    }
    Params = params;
    return true;
}

void MeshCache::adjustMeshCacheParams(MeshCacheParams* p, IDirect3DDevice9* pdevice)
{    
    D3DCAPS9 caps;
    if ( pdevice && SUCCEEDED(pdevice->GetDeviceCaps(&caps) ))
    {
        // Hardware instancing must be supported on cards supporting SM3.0+. If the card supports
        // less than that, remove instancing support, because the hardware essentially does the
        // same thing as batching, and not all drivers have good support for it.
        if ( caps.VertexShaderVersion < D3DVS_VERSION(3, 0) )
        {
            p->InstancingThreshold = 0;
        }

        // Determine the maximum number of batches/instances we can have, depending on the number of shader
        // constants we have. It should not exceed SF_RENDER_MAX_BATCHES, otherwise we may create
        // batches which are too large.
        if (p->MaxBatchInstances > SF_RENDER_MAX_BATCHES)
            p->MaxBatchInstances = SF_RENDER_MAX_BATCHES;
    }

    if (p->VBLockEvictSizeLimit < 1024 * 256)
        p->VBLockEvictSizeLimit = 1024 * 256;

    UPInt maxStagingItemSize = p->MaxVerticesSizeInBatch +
                               sizeof(UInt16) * p->MaxIndicesInBatch;
    if (maxStagingItemSize * 2 > p->StagingBufferSize)
        p->StagingBufferSize = maxStagingItemSize * 2;
}


void MeshCache::destroyPendingBuffers()
{
    // Destroy any pending buffers that are waiting to be destroyed (if possible).
    List<Render::MeshBuffer> remainingBuffers;
    MeshBuffer* p = (MeshBuffer*)PendingDestructionBuffers.GetFirst();
    while (!PendingDestructionBuffers.IsNull(p))
    {
        MeshCacheListSet::ListSlot& pendingFreeList = CacheList.GetSlot(MCL_PendingFree);
        MeshCacheItem* pitem = (MeshCacheItem*)pendingFreeList.GetFirst();
        bool itemsRemaining = false;
        MeshBuffer* pNext = (D3D9::MeshBuffer*)p->pNext;
        p->RemoveNode();
        while (!pendingFreeList.IsNull(pitem))
        {
            if ((pitem->pVertexBuffer == p) || (pitem->pIndexBuffer == p))
            {
                // If the fence is still pending, cannot destroy the buffer.
                if ( pitem->GPUFence && pitem->GPUFence->IsPending(FenceType_Vertex) )
                {
                    itemsRemaining = true;
                    remainingBuffers.PushFront(p);
                    break;
                }
            }
            pitem = (MeshCacheItem*)pitem->pNext;
        }
        if ( !itemsRemaining )
        {
            delete p;
        }
        p = pNext;
    }
    PendingDestructionBuffers.PushListToFront(remainingBuffers);
}

void MeshCache::EndFrame()
{
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_Medium);

    CacheList.EndFrame();

    // Try and reclaim memory from items that have already been destroyed, but not freed.
    CacheList.EvictPendingFree(IndexBuffers.Allocator);
    CacheList.EvictPendingFree(VertexBuffers.Allocator);

    destroyPendingBuffers();


    // Simple Heuristic used to shrink cache. Shrink is possible once the
    // (Total_Frame_Size + LRUTailSize) exceed the allocated space by more then
    // one granularity unit. If this case, we destroy the cache buffer in the
    // order opposite to that of which it was created.

    // TBD: This may have a side effect of throwing away the current frame items
    // as well. Such effect is undesirable and can perhaps be avoided on consoles
    // with buffer data copies (copy PrevFrame content into other buffers before evict).

    UPInt totalFrameSize = CacheList.GetSlotSize(MCL_PrevFrame);
    UPInt lruTailSize    = CacheList.GetSlotSize(MCL_LRUTail);
    UPInt expectedSize   = totalFrameSize + Alg::PMin(lruTailSize, Params.LRUTailSize);
    expectedSize += expectedSize / 4; // + 25%, to account for fragmentation.

    SPInt extraSpace = getTotalSize() - (SPInt)expectedSize;
    if (extraSpace > (SPInt)Params.MemGranularity)
    {        
        while (!ChunkBuffers.IsEmpty() && (extraSpace > (SPInt)Params.MemGranularity))
        {
            MeshBuffer* p = (MeshBuffer*)ChunkBuffers.GetLast();
            p->RemoveNode();
            extraSpace -= (SPInt)p->GetSize();

            MeshBufferSet&  mbs = (p->GetBufferType() == MeshBuffer::Buffer_Vertex) ?
                                  (MeshBufferSet&)VertexBuffers : (MeshBufferSet&)IndexBuffers;

            // Evict first! This may fail if a query is pending on a mesh inside the buffer. In that case,
            // simply store the buffer to be destroyed later.
            bool allEvicted = evictMeshesInBuffer(CacheList.GetSlots(), MCL_ItemCount, p);
            mbs.DestroyBuffer(p, allEvicted);
            if ( !allEvicted )
                PendingDestructionBuffers.PushBack(p);


#ifdef SF_RENDER_LOG_CACHESIZE
            LogDebugMessage(Log_Message,
                           "Cache shrank to %dK. Start FrameSize = %dK, LRU = %dK\n",
                           getTotalSize() / 1024, totalFrameSize/1024, lruTailSize/1024);
#endif
        }
    }    
}



// Adds a fixed-size buffer to cache reserve; expected to be released at Release.
/*
bool    MeshCache::AddReserveBuffer(unsigned size, unsigned arena)
{
}
bool    MeshCache::ReleaseReserveBuffer(unsigned size, unsigned arena)
{
}
*/


// Allocates Vertex/Index buffer of specified size and adds it to free list.
bool MeshCache::allocCacheBuffers(UPInt size, MeshBuffer::AllocType type, unsigned arena)
{
    // Assign (5/9) of space for VB; need to experiment with fractions in the future.
    // Round to 16-byte units for now...   
    UPInt vbsize = calcVBGranularity(size);
    UPInt ibsize = calcIBGranularity(size, vbsize);

    VertexBuffer *pvb = (VertexBuffer*)VertexBuffers.CreateBuffer(vbsize, type, arena,
                                                                  pHeap, pDevice, BufferCreateFlags);
    if (!pvb)
        return false;
    MeshBuffer   *pib = IndexBuffers.CreateBuffer(ibsize, type, arena, pHeap, pDevice, BufferCreateFlags);
    if (!pib)
    {
        VertexBuffers.DestroyBuffer(pvb);
        return false;
    }
#ifdef SF_RENDER_LOG_CACHESIZE
    LogDebugMessage(Log_Message, "Cache grew to %dK\n", getTotalSize() / 1024);
#endif
    return true;
}


bool MeshCache::createStaticVertexBuffers(IDirect3DDeviceX* pdevice)
{
    return createInstancingVertexBuffer(pdevice) && 
           createMaskEraseBatchVertexBuffer(pdevice);
}

bool MeshCache::createInstancingVertexBuffer(IDirect3DDeviceX* pdevice)
{
    HRESULT              createResult;
    DWORD                lockFlags = 0;
    unsigned             bufferSize = sizeof(Render_InstanceData) * SF_RENDER_MAX_BATCHES;
    Render_InstanceData* pbuffer = 0;

    createResult = pdevice->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY,
                                               0, D3DPOOL_DEFAULT,
                                               &pInstancingVertexBuffer.GetRawRef(), 0);
    if (createResult != D3D_OK)
        return false;

    if (FAILED(pInstancingVertexBuffer->Lock(0, bufferSize, (void**)&pbuffer, lockFlags)))
    {
        SF_DEBUG_WARNING(1, "D3D9::MeshCache - VertexBuffer lock failed");
        pInstancingVertexBuffer = 0;
        return false;        
    }

    for(unsigned i = 0; i< SF_RENDER_MAX_BATCHES; i++)
        pbuffer[i].Instance = i; // low byte

    pInstancingVertexBuffer->Unlock();
    return true;
}

bool MeshCache::createMaskEraseBatchVertexBuffer(IDirect3DDeviceX* pdevice)
{
    HRESULT      createResult;
    DWORD        lockFlags = 0;
    unsigned     bufferSize = sizeof(VertexXY16iAlpha) * 6 * SF_RENDER_MAX_BATCHES;
    VertexXY16iAlpha* pbuffer = 0;

    createResult = pdevice->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY,
                                               0, D3DPOOL_DEFAULT,
                                               &pMaskEraseBatchVertexBuffer.GetRawRef(), 0);
    if (createResult != D3D_OK)
        return false;

    if (FAILED(pMaskEraseBatchVertexBuffer->Lock(0, bufferSize, (void**)&pbuffer, lockFlags)))
    {
        SF_DEBUG_WARNING(1, "D3D9::MeshCache - VertexBuffer lock failed");
        pMaskEraseBatchVertexBuffer = 0;
        return false;        
    }

    fillMaskEraseVertexBuffer<VertexXY16iAlpha>(pbuffer, SF_RENDER_MAX_BATCHES);

    pMaskEraseBatchVertexBuffer->Unlock();
    return true;
}

bool MeshCache::LockBuffers()
{
    SF_ASSERT(!Locked);
    Locked = true;
    VBSizeEvictedInLock = 0;
    if (pRQCaches)
        pRQCaches->SetCacheLocked(Cache_Mesh);
    return true;
}

void MeshCache::UnlockBuffers()
{
    SF_ASSERT(Locked != 0); 
    LockedBuffers.UnlockAll();
    Locked = false;
    if (pRQCaches)
        pRQCaches->ClearCacheLocked(Cache_Mesh);
}


bool MeshCache::evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                    MeshBuffer* pbuffer)
{
    bool evictionFailed = false;
    for (unsigned i=0; i< count; i++)
    {
        MeshCacheItem* pitem = (MeshCacheItem*)plist[i].GetFirst();
        while (!plist[i].IsNull(pitem))
        {
            if ((pitem->pVertexBuffer == pbuffer) || (pitem->pIndexBuffer == pbuffer))
            {
                // Evict returns the number of bytes released. If this is zero, it means the mesh
                // was still in use. 
                if ( Evict(pitem) == 0 )
                {
                    evictionFailed = true;
                    SF_ASSERT(pitem->Type == MeshCacheItem::Mesh_Destroyed);

                    // We still need to delete all the addresses allocated in the buffer, because it is 
                    // going to be deleted, and AllocAddr will break otherwise.
                    if ( pitem->pVertexBuffer == pbuffer)
                    {
                        VertexBuffers.Free(pitem->VBAllocSize, pitem->pVertexBuffer, pitem->VBAllocOffset);
                        pitem->pVertexBuffer = 0;
                    }
                    if ( pitem->pIndexBuffer == pbuffer)
                    {
                        IndexBuffers.Free(pitem->IBAllocSize, pitem->pIndexBuffer, pitem->IBAllocOffset);
                        pitem->pIndexBuffer = 0;
                    }
                }

                // Evict may potentially modify the cache items, so start again.
                // This is less than ideal, but better than accessing a dangling pointer.
                pitem = (MeshCacheItem*)plist[i].GetFirst();
                continue;
            }
            pitem = (MeshCacheItem*)pitem->pNext;
        }
    }
    return !evictionFailed;
}


UPInt MeshCache::Evict(Render::MeshCacheItem* pbatch, AllocAddr* pallocator, MeshBase* pskipMesh)
{
    MeshCacheItem* p = (MeshCacheItem*)pbatch;

    // If a fence is not pending, then the memory for the item can be reclaimed immediately.
    if ( !UsesDynamicMeshes() || !p->GPUFence || !p->GPUFence->IsPending(FenceType_Vertex))
    {
        // - Free allocator data.
        UPInt vbfree = p->pVertexBuffer ? VertexBuffers.Free(p->VBAllocSize, p->pVertexBuffer, p->VBAllocOffset) : 0;
        UPInt ibfree = p->pIndexBuffer  ? IndexBuffers.Free(p->IBAllocSize, p->pIndexBuffer, p->IBAllocOffset) : 0;
        UPInt freedSize = pallocator ? ((&VertexBuffers.GetAllocator() == pallocator) ? vbfree : ibfree) : vbfree + ibfree;
    
        VBSizeEvictedInLock += (unsigned)  p->VBAllocSize;
        p->Destroy(pskipMesh, true);
        return freedSize;
    }
    else
    {
        // Still in use, push it on the pending to delete list.
        // It should be valid for this to be called multiple times for a single mesh (for example, in a PendingFree situation).
        p->Destroy(pskipMesh, false);
        CacheList.PushFront(MCL_PendingFree, p);
        return 0;
    }

}


// Allocates the buffer, while evicting LRU data.
bool MeshCache::allocBuffer(UPInt* poffset, MeshBuffer** pbuffer,
                            MeshBufferSet& mbs, UPInt size, bool waitForCache)
{
    SF_UNUSED(waitForCache);

    if (mbs.Alloc(size, pbuffer, poffset))
        return true;

    // If allocation failed... need to apply swapping or grow buffer.
    MeshCacheItem* pitems;

    // #1. Try and reclaim memory from items that have already been destroyed, but not freed.
    //     These cannot be reused, so it is best to evict their memory first, if possible.
    if (CacheList.EvictPendingFree(mbs.Allocator))
        goto alloc_size_available;

    // #2. Then, apply LRU (least recently used) swapping from data stale in
    //    earlier frames until the total size 
  
    if ((getTotalSize() + MinSupportedGranularity) <= Params.MemLimit)
    {
        if (CacheList.EvictLRUTillLimit(MCL_LRUTail, mbs.GetAllocator(),
                                        size, Params.LRUTailSize))
            goto alloc_size_available;

        // TBD: May cause spinning? Should we have two error codes?
        SF_ASSERT(size <= mbs.GetGranularity());
        if (size > mbs.GetGranularity())
            return false;

        UPInt allocSize = Alg::PMin(Params.MemLimit - getTotalSize(), mbs.GetGranularity());
        if (size <= allocSize)
        {
            MeshBuffer* pbuff = mbs.CreateBuffer(allocSize, MeshBuffer::AT_Chunk, 0, pHeap, pDevice, BufferCreateFlags);
            if (pbuff)
            {
                ChunkBuffers.PushBack(pbuff);
#ifdef SF_RENDER_LOG_CACHESIZE
                LogDebugMessage(Log_Message, "Cache grew to %dK\n", getTotalSize() / 1024);
#endif
                goto alloc_size_available;
            }
        }
    }

    if (CacheList.EvictLRU(MCL_LRUTail, mbs.GetAllocator(), size))
        goto alloc_size_available;

    if (VBSizeEvictedInLock > Params.VBLockEvictSizeLimit)
        return false;

    // #3. Apply MRU (most recently used) swapping to the current frame content.
    // NOTE: MRU (GetFirst(), pNext iteration) gives
    //       2x improvement here with "Stars" test swapping.
    MeshCacheListSet::ListSlot& prevFrameList = CacheList.GetSlot(MCL_PrevFrame);
    pitems = (MeshCacheItem*)prevFrameList.GetFirst();
    while(!prevFrameList.IsNull(pitems))
    {
        if (!UsesDynamicMeshes() || !pitems->GPUFence || !pitems->GPUFence->IsPending(FenceType_Vertex))
        {
            if (Evict(pitems, &mbs.GetAllocator()) >= size)
                goto alloc_size_available;

            // Get the first item in the list, because and the head of the list will now be different, due to eviction.
            pitems = (MeshCacheItem*)prevFrameList.GetFirst();
        }
        else
        {
            pitems = (MeshCacheItem*)prevFrameList.GetNext(pitems);
        }
    }

    // #4. If MRU swapping didn't work for ThisFrame items due to them still
    // being processed by the GPU and we are being asked to wait, wait
    // until fences are passed to evict items.
    MeshCacheListSet::ListSlot& thisFrameList = CacheList.GetSlot(MCL_ThisFrame);
    pitems = (MeshCacheItem*)thisFrameList.GetFirst();
    while(waitForCache && !thisFrameList.IsNull(pitems))
    {
        if ( UsesDynamicMeshes() && pitems->GPUFence )
            pitems->GPUFence->WaitFence(FenceType_Vertex);
        if (Evict(pitems, &mbs.GetAllocator()) >= size)
            goto alloc_size_available;
        pitems = (MeshCacheItem*)thisFrameList.GetFirst();
    }
    return false;
    
    // At this point we know we have a large enough block either due to
    // swapping or buffer growth, so allocation shouldn't fail.
alloc_size_available:
    if (!mbs.Alloc(size, pbuffer, poffset))
    {
        SF_ASSERT(0);
        return false;
    }

    return true;
}


// Generates meshes and uploads them to buffers.
bool MeshCache::PreparePrimitive(PrimitiveBatch* pbatch,
                                 MeshCacheItem::MeshContent &mc,
                                 bool waitForCache)
{
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_Medium);

    Primitive* prim = pbatch->GetPrimitive();

    if (mc.IsLargeMesh())
    {
        SF_ASSERT(mc.GetMeshCount() == 1);
        MeshResult mr = GenerateMesh(mc[0], prim->GetVertexFormat(),
                                     pbatch->pFormat, 0, waitForCache);

        if (mr.Succeded())
            pbatch->SetCacheItem(mc[0]->CacheItems[0]);
        // Return 'false' if we just need more cache, to flush and retry.
        if (mr == MeshResult::Fail_LargeMesh_NeedCache)
            return false;
        return true;
    }

    // Prepare and Pin mesh data with the StagingBuffer. NOTE: this must happen before calculating mesh sizes.
    // This stage updates the mesh vertex/index counts, so calculating them first, it could be incorrect, for
    // example in the case of MeshCache::ClearCache, and changing ToleranceParams.
    StagingBufferPrep   meshPrep(this, mc, prim->GetVertexFormat(), false);

    // NOTE: We always know that meshes in one batch fit into Mesh Staging Cache.
    unsigned totalVertexCount, totalIndexCount;
    pbatch->CalcMeshSizes(&totalVertexCount, &totalIndexCount);

    SF_ASSERT(totalVertexCount && totalIndexCount);
    SF_ASSERT(pbatch->pFormat);

    Render::MeshCacheItem* batchData = 0;
    unsigned       destVertexSize = pbatch->pFormat->Size;
    UByte*         pvertexDataStart;
    IndexType*     pindexDataStart;
    AllocResult    allocResult;

    allocResult = AllocCacheItem(&batchData, &pvertexDataStart, &pindexDataStart,
                                MeshCacheItem::Mesh_Regular, mc,
                                totalVertexCount * destVertexSize,
                                totalVertexCount, totalIndexCount, waitForCache, 0);
    if (allocResult != Alloc_Success)
    {
        // Return 'true' for state error/too big (we can't recover by swapping cache and re-trying).
        return (allocResult == Alloc_Fail) ? false : true;
    }

    pbatch->SetCacheItem(batchData);

    // This step either generates the mesh into the staging buffer, or locates an existing MeshCacheItem
    // that we can copy the mesh from. It must be done after creating the cache item, because that may
    // evict the item that would be copied from.
    meshPrep.GenerateMeshes(batchData);

    // Copy meshes into the Vertex/Index buffers.

    // All the meshes have been pinned, so we can
    // go through them and copy them into buffers
    UByte*      pstagingBuffer   = StagingBuffer.GetBuffer();
    const VertexFormat* pvf      = prim->GetVertexFormat();
    const VertexFormat* pdvf     = pbatch->pFormat;
   
    unsigned    i;
    unsigned    indexStart = 0;

    //printf("Writing to: mesh=%p vsz=%02d vc=%d AOffset=%d pdest=%p SOffset=%d\n",
    //       batchData, destVertexSize, totalVertexCount, ((MeshCacheItem*)batchData)->VBAllocOffset,
    //       pvertexDataStart, mc[0]->StagingBufferOffset); 

    for(i = 0; i< mc.GetMeshCount(); i++)
    {
        Mesh* pmesh = mc[i];
        SF_ASSERT(pmesh->StagingBufferSize != 0);
    
        // Convert vertices and initialize them to the running index
        // within this primitive.
        void*   convertArgArray[1] = { &i };
        ConvertVertices_Buffered(*pvf, pstagingBuffer + pmesh->StagingBufferOffset,
                                 *pdvf, pvertexDataStart,
                                 pmesh->VertexCount, &convertArgArray[0]);
        // Copy and assign indices.
        ConvertIndices(pindexDataStart,
                       (IndexType*)(pstagingBuffer + pmesh->StagingBufferIndexOffset),
                       pmesh->IndexCount, (IndexType)indexStart);

        pvertexDataStart += pmesh->VertexCount * destVertexSize;
        pindexDataStart  += pmesh->IndexCount;
        indexStart       += pmesh->VertexCount;
    }

    // ~StagingBufferPrep will Unpin meshes in the staging buffer.
    return true;
}

MeshCache::AllocResult
MeshCache::AllocCacheItem(Render::MeshCacheItem** pdata,
                         UByte** pvertexDataStart, IndexType** pindexDataStart,
                         MeshCacheItem::MeshType meshType,
                         MeshCacheItem::MeshBaseContent &mc,
                         UPInt vertexBufferSize,
                         unsigned vertexCount, unsigned indexCount,
                         bool waitForCache, const VertexFormat*)
{

    if (!AreBuffersLocked() && !LockBuffers())
        return Alloc_StateError;

    // Compute and allocate appropriate VB/IB space.
    UPInt       vbOffset = 0, ibOffset = 0;
    MeshBuffer  *pvb = 0, *pib = 0;
    UByte       *pvdata, *pidata;
    MeshCache::AllocResult failType = Alloc_Fail;

    if (!allocBuffer(&vbOffset, &pvb, VertexBuffers,
                     vertexBufferSize, waitForCache))
    {
        if (!VertexBuffers.CheckAllocationSize(vertexBufferSize, "Vertex"))        
            failType = Alloc_Fail_TooBig;        
handle_alloc_fail:
        if (pvb) VertexBuffers.Free(vertexBufferSize, pvb, vbOffset);
        if (pib) IndexBuffers.Free(indexCount * sizeof(IndexType), pib, ibOffset);
        return failType;
    }
    if (!allocBuffer(&ibOffset, &pib, IndexBuffers,
                     indexCount * sizeof(IndexType), waitForCache))
    {
        if (!IndexBuffers.CheckAllocationSize(indexCount * sizeof(IndexType), "Index"))
            failType = Alloc_Fail_TooBig;        
        goto handle_alloc_fail;
    }

    pvdata = pvb->Lock(LockedBuffers);
    pidata = pib->Lock(LockedBuffers);

    if (!pvdata || !pidata)
        goto handle_alloc_fail;

    // Create new MeshCacheItem; add it to hash.
    *pdata = MeshCacheItem::Create(meshType,
        &CacheList, mc, (VertexBuffer*)pvb, (IndexBuffer*)pib,
        (unsigned)vbOffset, vertexBufferSize, vertexCount,
        (unsigned)ibOffset, indexCount * sizeof(IndexType), indexCount);

    if (!*pdata)
    {
        // Memory error; free buffers, skip mesh.
        SF_ASSERT(0);
        failType = Alloc_StateError;
        goto handle_alloc_fail;
    }

    *pvertexDataStart = pvdata + vbOffset;
    *pindexDataStart  = (IndexType*)(pidata + ibOffset);
    return Alloc_Success;
}

void MeshCache::GetStats(Stats* stats)
{
    *stats = Stats();
    unsigned memType = MeshBuffer_GpuMem;

    stats->TotalSize[memType + MeshBuffer_Vertex] = VertexBuffers.GetTotalSize();
    stats->UsedSize[memType + MeshBuffer_Vertex] = VertexBuffers.Allocator.GetFreeSize() << MeshCache_AllocatorUnitShift;

    stats->TotalSize[memType + MeshBuffer_Index] = IndexBuffers.GetTotalSize();
    stats->UsedSize[memType + MeshBuffer_Index] = IndexBuffers.Allocator.GetFreeSize() << MeshCache_AllocatorUnitShift;
}

}}}; // namespace Scaleform::Render::D3D9

