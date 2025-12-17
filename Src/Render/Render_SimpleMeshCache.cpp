/**************************************************************************

Filename    :   Render_SimpleMeshCache.cpp
Content     :   Common base for consoles MeshCache implementations.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_SimpleMeshCache.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

//#define SF_RENDER_LOG_CACHESIZE

namespace Scaleform { namespace Render {


// ***** SimpleMeshCache

SimpleMeshCache::SimpleMeshCache(MemoryHeap* pheap,
                                 const MeshCacheParams& params,
                                 RenderSync* psync, UInt32 cacheFlags)
 : Render::MeshCache(pheap, params),   
   CacheList(getThis()),
   Allocator(pheap),
   TotalSize(0),
   pRenderSync(psync),
   CacheFlags(cacheFlags),
   VBAlignment(16),
   IBAlignment(16),
   BufferAlignment(16)
{
}

SimpleMeshCache::~SimpleMeshCache()
{   
}

// Allocates reserve, if any. Intended for use in Initialize implementation.
bool    SimpleMeshCache::allocateReserve()
{
    if (Params.MemReserve)
        return allocMeshBuffer(Params.MemReserve, MeshBuffer::AT_Reserve) != 0;
    return true;
}

void    SimpleMeshCache::releaseAllBuffers()
{
    ClearCache();
    while(!Buffers.IsEmpty())
        releaseMeshBuffer((SimpleMeshBuffer*)Buffers.GetFirst());
}


void    SimpleMeshCache::ClearCache()
{
    CacheList.EvictAll();

    // Release all dynamically allocated buffers.
    SimpleMeshBuffer* pbuffer = (SimpleMeshBuffer*)Buffers.GetFirst();
    while(!Buffers.IsNull(pbuffer))
    {
        SimpleMeshBuffer *p = pbuffer;
        pbuffer = (SimpleMeshBuffer*)pbuffer->pNext;
        if (p->GetType() == MeshBuffer::AT_Chunk)
            releaseMeshBuffer(p);
    }

    StagingBuffer.Reset();
    StagingBuffer.Initialize(pHeap, Params.StagingBufferSize);

    SF_ASSERT(BatchCacheItemHash.GetSize() == 0);
}

void    SimpleMeshCache::EndFrame()
{
    CacheList.EndFrame();

    // Try and reclaim memory from items that have already been destroyed, but not freed.
    CacheList.EvictPendingFree(Allocator);

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

    SPInt extraSpace = TotalSize - (SPInt)expectedSize;
    if (extraSpace > (SPInt)Params.MemGranularity && !(CacheFlags & Cache_NeverShrink))
    {
        SimpleMeshBuffer* pbuffer = (SimpleMeshBuffer*)Buffers.GetLast();

        while (!Buffers.IsNull(pbuffer) && (extraSpace > (SPInt)Params.MemGranularity))
        {
            SimpleMeshBuffer* p = pbuffer;
            pbuffer = (SimpleMeshBuffer*)pbuffer->pPrev;
            if (p->GetType() != MeshBuffer::AT_Chunk)
                continue;

            // TBD: Conditional release likely to break shrinking due to reallocation. A way 
            // to solve this would be to tag buffers as non-allocatable until they can be freed.
            if ( releaseMeshBuffer(p) )
                extraSpace -= (SPInt)p->GetFullSize();
            
#ifdef SF_RENDER_LOG_CACHESIZE
            LogDebugMessage(Log_Message,
                "Cache shrank to %dK. Start FrameSize = %dK, LRU = %dK\n",
                TotalSize / 1024, totalFrameSize/1024, lruTailSize/1024);
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
SimpleMeshBuffer*   SimpleMeshCache::allocMeshBuffer(UPInt size, AllocType atype, unsigned arena)
{    
    SimpleMeshBuffer* pbuffer = createHWBuffer(size, atype, arena);
    if (!pbuffer)
        return 0;

    // Add the newly reserved area to the allocator.
    Allocator.AddSegment((UPInt)pbuffer->pData, pbuffer->GetSize());
    Buffers.PushBack(pbuffer);
    TotalSize += pbuffer->GetFullSize();
    return pbuffer;
}

bool SimpleMeshCache::releaseMeshBuffer(SimpleMeshBuffer* pbuffer)
{
    // Remove from Allocator; generating any necessary warnings.
    // Go through all allocation lists and Evict data items in the buffers address range.

    // TBD: There shouldn't be any MCL_InFlight items. If there are,
    //      free operations should be delayed...
    SF_ASSERT(CacheList.GetSlot(MCL_InFlight).IsEmpty());
    
    // Eviction may not complete immediately if the meshes were still in use. evictMeshesInBuffer
    // will return false in this case, meaning we can't free the Buffer.
    if ( evictMeshesInBuffer(CacheList.GetSlots(), MCL_ItemCount, pbuffer) )
    {
        Allocator.RemoveSegment((UPInt)pbuffer->pData, pbuffer->GetSize());
        pbuffer->RemoveNode();
        TotalSize -= pbuffer->GetFullSize();
        destroyHWBuffer(pbuffer);
        return true;
    }
    return false;
}


SimpleMeshBuffer* SimpleMeshCache::findBuffer(UPInt address)
{
    // TBD: This is less then ideal since it happens on every allocation....
    SimpleMeshBuffer* p = (SimpleMeshBuffer*)Buffers.GetFirst();
    while (!Buffers.IsNull(p))
    {
        UPInt bufferAddress = (UPInt)p->pData; 
        if ((address >= bufferAddress) &&
            (address < (bufferAddress + p->GetSize())))
            return p;
        p = (SimpleMeshBuffer*)p->pNext;
    }
    return 0;
}

bool SimpleMeshCache::evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                             SimpleMeshBuffer* pbuffer)
{
    for (unsigned i=0; i< count; i++)
    {
        SimpleMeshCacheItem* pitem = (SimpleMeshCacheItem*)plist[i].GetFirst();
        while (!plist[i].IsNull(pitem))
        {
            if (pitem->pBuffer == pbuffer)
            {
                if (pitem->Type == MeshCacheItem::Mesh_Destroyed)
                {
                    // If the fence is still pending, report eviction failure.
                    SF_DEBUG_ASSERT(pitem->GPUFence, "MeshCacheItem which is pending delete must have a fence.");
                    if ( pitem->GPUFence->IsPending(FenceType_Vertex) )
                        return false;
                }

                // Evict returns the number of bytes released. If this is zero, it means the mesh
                // was still in use, and we cannot destroy the buffer immediately.
                if ( Evict(pitem) == 0 )
                    return false;

                // Evict may potentially modify the cache items, so start again.
                // This is less than ideal, but better than accessing a dangling pointer.
                pitem = (SimpleMeshCacheItem*)plist[i].GetFirst();
                continue;
            }
            pitem = (SimpleMeshCacheItem*)pitem->pNext;
        }
    }
    return true;
}


bool    SimpleMeshCache::allocBuffer(UPInt* poffset, UPInt size, bool waitForCache)
{
    *poffset = Allocator.Alloc(size);
    if (*poffset != ~UPInt(0))
        return true;
   
    // If allocation failed... need to apply swapping or grow buffer.
    MeshCacheItem* pitems;
    MeshCacheListSet::ListSlot& prevFrameList = CacheList.GetSlot(MCL_PrevFrame);
    MeshCacheListSet::ListSlot& thisFrameList = CacheList.GetSlot(MCL_ThisFrame);

    // #1. Try and reclaim memory from items that have already been destroyed, but not freed.
    //     These cannot be reused, so it is best to evict their memory first, if possible.
    if (CacheList.EvictPendingFree(Allocator))
        goto alloc_size_available;

    // #2. Apply LRU (least recently used) swapping from data stale in
    //     earlier frames. Swapping done first until under LRUTailSize if
    //     cache growth is possible; if not, swapping is done until LRU is empty.
    if ((TotalSize + SimpleMeshBuffer::MinGranularity) <= Params.MemLimit)
    {
        if (CacheList.EvictLRUTillLimit(MCL_LRUTail, Allocator,
                                        size, Params.LRUTailSize))
            goto alloc_size_available;

        // TBD: May cause spinning? Should we have two error codes?
        SF_ASSERT(size <= SimpleMeshBuffer::GetUsableSize(Params.MemGranularity));
        if (size > SimpleMeshBuffer::GetUsableSize(Params.MemGranularity))
            return false;

        UPInt allocSize = Alg::PMin(Params.MemLimit - TotalSize, Params.MemGranularity);
        if (size <= SimpleMeshBuffer::GetUsableSize(allocSize))
        {
            if (allocMeshBuffer(allocSize, MeshBuffer::AT_Chunk, 0))
            {         
#ifdef SF_RENDER_LOG_CACHESIZE
                LogDebugMessage(Log_Message, "Cache grew to %dK\n", TotalSize / 1024);
#endif
                goto alloc_size_available;
            }
        }
    }

    if (CacheList.EvictLRU(MCL_LRUTail, Allocator, size))
        goto alloc_size_available;

    
    // #3. Apply MRU (most recently used) swapping to the previous frame content.
    // TBD: We'll we assume that Fence is not needed here since PrevFrame is
    // out of the HW rendering queue by now; this needs to be verified.    
    pitems = (MeshCacheItem*)prevFrameList.GetFirst();
    while(!prevFrameList.IsNull(pitems))
    {
        if (!pitems->GPUFence || !pitems->GPUFence->IsPending(FenceType_Vertex))
        {
            if (Evict(pitems, &Allocator) >= size)
                goto alloc_size_available;

            // Get the first item in the list, because and the head of the list will now be different, due to eviction.
            pitems = (MeshCacheItem*)prevFrameList.GetFirst();
        }
        else
        {
            pitems = (MeshCacheItem*)prevFrameList.GetNext(pitems);
        }
    }

    // Force command buffer kick-off in case we'll be swapping
    // out the last draw command.
    pRenderSync->KickOffFences(FenceType_Vertex);

    // #4. Apply MRU (most recently used) swapping to the current frame content.
    //     NOTE: MRU (GetFirst(), pNext iteration) gives
    //       2x improvement here with "Stars" test swapping.    
    pitems = (MeshCacheItem*)thisFrameList.GetFirst();

    while(!thisFrameList.IsNull(pitems))
    {
        if (pitems->GPUFence && pitems->GPUFence->IsPending(FenceType_Vertex))
        {   
            // Can't evict this item.
            pitems = (MeshCacheItem*)pitems->pNext;
        }
        else
        {
            if (Evict(pitems, &Allocator) >= size)
                goto alloc_size_available;
            // Restart from beginning, in case Evict removed more list items.
            pitems = (MeshCacheItem*)thisFrameList.GetFirst();
        }
    }

    // #5. If MRU swapping didn't work for ThisFrame items due to them still
    // being processed by the GPU and we are being asked to wait, wait
    // until fences are passed to evict items.
    if (waitForCache && !thisFrameList.IsEmpty())
    {            
        pitems = (MeshCacheItem*)thisFrameList.GetFirst();
        while(!thisFrameList.IsNull(pitems))
        {
            if ( pitems->GPUFence )
                pitems->GPUFence->WaitFence(FenceType_Vertex);
            if (Evict(pitems, &Allocator) >= size)
                goto alloc_size_available;
            pitems = (MeshCacheItem*)thisFrameList.GetFirst();
        }
    }

    return false;

    // At this point we know we have a large enough block either due to
    // swapping or buffer growth, so allocation shouldn't fail.
alloc_size_available:
    *poffset = Allocator.Alloc(size);
    SF_ASSERT(*poffset != ~UPInt(0));
    return true;
}


UPInt   SimpleMeshCache::Evict(Render::MeshCacheItem* p, AllocAddr* pallocator, MeshBase* pskipMesh)
{
    // If a fence is not pending, then the memory for the item can be reclaimed immediately.
    if ( !p->GPUFence || !p->GPUFence->IsPending(FenceType_Vertex) ) 
    {
        // - Free allocator data.
        AllocAddr &a = pallocator ? *pallocator : Allocator;
        UPInt freedSize = a.Free(((SimpleMeshCacheItem*)p)->AllocAddress, p->AllocSize);
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
MeshCache::AllocResult
SimpleMeshCache::AllocCacheItem(Render::MeshCacheItem** pdata,
                                UByte** pvertexDataStart, IndexType** pindexDataStart,
                                MeshCacheItem::MeshType meshType,
                                MeshCacheItem::MeshBaseContent &mc,
                                UPInt vertexBufferSize,
                                unsigned vertexCount, unsigned indexCount,
                                bool waitForCache, const VertexFormat*)
{
    UPInt allocAddress;
    UPInt vertexBufferSizeAlign = Alg::Align<UPInt>(vertexBufferSize, VBAlignment);
    UPInt indexBufferSizeAlign  = Alg::Align<UPInt>(indexCount * sizeof(IndexType), IBAlignment);
    UPInt allocSize = Alg::Align(vertexBufferSizeAlign + indexBufferSizeAlign, BufferAlignment);

    if (!allocBuffer(&allocAddress, allocSize, waitForCache))
        return Alloc_Fail;

    SimpleMeshBuffer* pbuffer  = (SimpleMeshBuffer*)findBuffer(allocAddress);
    UPInt             vbOffset = allocAddress - (UPInt)pbuffer->pData;
    UPInt             ibOffset = vbOffset + vertexBufferSizeAlign;

    // Create new MeshCacheItem; add it to hash.
    *pdata = SimpleMeshCacheItem::Create(meshType, &CacheList, sizeof(SimpleMeshCacheItem), 
        mc, (SimpleMeshBuffer*)pbuffer, allocAddress, allocSize, vbOffset, vertexCount, ibOffset, indexCount, 0 );

    if (!*pdata)
    {
        SF_DEBUG_ASSERT(0, "MeshCache Memory error; freeing buffers and skipping mesh");
        return Alloc_StateError;
    }

    *pvertexDataStart = (UByte*)allocAddress;
    *pindexDataStart = (IndexType*)(allocAddress + vertexBufferSizeAlign);
    return Alloc_Success;
}

// Generates meshes and uploads them to buffers.
// Returns 'false' if there is not enough space in Cache, so Unlock and
// flush needs to take place.
//   - Pass 'false' for firstCall is PreparePrimitive was already called once
//     with this data and failed.
bool SimpleMeshCache::PreparePrimitive(PrimitiveBatch* pbatch,
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
    // example in the case of MeshCache::ClearCache.
    StagingBufferPrep   meshPrep(this, mc, prim->GetVertexFormat(), true);

    // NOTE: We always know that meshes in one batch fit into Mesh Staging Cache.
    unsigned totalVertexCount, totalIndexCount;
    pbatch->CalcMeshSizes(&totalVertexCount, &totalIndexCount);

    // First, try to allocate the buffer space while swapping out any stale data.
    // Note that AllocBuffer takes place before mesh-gen to ensure that packing code
    // does not rely on MeshCacheItem items that may get evicted during allocation.
    // For vertex alignment purpose, we round all allocations to 16 bytes.
    UPInt   allocAddress;
    UPInt   vertexByteSize = totalVertexCount * pbatch->pFormat->Size;
	UPInt   vertexAlignedSize = Alg::Align(vertexByteSize, VBAlignment);
    UPInt   allocSize      = Alg::Align(vertexAlignedSize + Alg::Align(totalIndexCount * sizeof(IndexType),IBAlignment), BufferAlignment );

    // If a zero-size request happens, just ignore it.
    if (allocSize==0)
        return true;

    if (!allocBuffer(&allocAddress, allocSize, waitForCache))
        return false;

	SF_ASSERT(pbatch->pFormat);

	Render::MeshCacheItem* batchData = 0;

	SimpleMeshBuffer* pbuffer  = findBuffer(allocAddress);
	UPInt             vbOffset = allocAddress - (UPInt)pbuffer->pData;
	UPInt             ibOffset = vbOffset + vertexAlignedSize;

	batchData = SimpleMeshCacheItem::Create(MeshCacheItem::Mesh_Regular, &CacheList, sizeof(SimpleMeshCacheItem),
		mc, pbuffer, allocAddress, allocSize, vbOffset, totalVertexCount, ibOffset, totalIndexCount, pbatch->pFormat);

	if (!batchData)
	{
		return false;
	}

	pbatch->SetCacheItem(batchData);

    // This step is a verification that the mesh is now either in the staging buffer, in an existing MeshCacheItem
    // that we can copy the mesh from. It must be done after creating the cache item, because that may
    // evict the item that would be copied from.
    meshPrep.GenerateMeshes(batchData);

    // Copy meshes into the Vertex/Index buffers.
    // All the meshes have been pinned, so we can
    // go through them and copy them into buffers.
    UByte*      pvertexDataStart = (UByte*)allocAddress;
    IndexType*  pindexDataStart  = (IndexType*)(allocAddress + vertexAlignedSize);
    UByte*      pstagingBuffer   = StagingBuffer.GetBuffer();
	const VertexFormat* pvf      = pbatch->GetPrimitive()->GetVertexFormat();

    unsigned i;
    unsigned indexStart = 0;

    for(i = 0; i< mc.GetMeshCount(); i++)
    {
        Mesh* pmesh = mc[i];        
        //SF_ASSERT(pmesh->Data.GetVertexFormat() == pvf);

        // Convert vertices and initialize them to the running index
        // within this primitive.
        UByte   instance = (UByte)i;
        void*   convertArgArray[1] = { &instance };

        // If mesh is in staging buffer, initialize it from there.
        if (pmesh->StagingBufferSize != 0)
        {
            ConvertVertices(*pvf, pstagingBuffer + pmesh->StagingBufferOffset,
                            *pbatch->pFormat, pvertexDataStart,
                            pmesh->VertexCount, &convertArgArray[0]);

            // Copy and assign indices.
            IndexType* pindexSource = (IndexType*)(pstagingBuffer + pmesh->StagingBufferIndexOffset);
            ConvertIndices(pindexDataStart, pindexSource, pmesh->IndexCount, (IndexType)indexStart);
            pindexDataStart += pmesh->IndexCount;
        }
        else
        {
            // If mesh is not in a staging buffer, find the batch that has our mesh and location in it,
            // so vertex and index data can be copied.
            unsigned            prevVertexCount, prevIndexCount;
            SimpleMeshCacheItem*  psourceBatchMesh = (SimpleMeshCacheItem*)
                MeshCacheItem::FindMeshSourceBatch(pmesh, &prevVertexCount, &prevIndexCount,
                pbatch->GetCacheItem());

            // Now, copy the old mesh data into the new location.
            const VertexFormat * psrcFmt = psourceBatchMesh->GetVertexFormat();
            UByte* psourceBufferStart = (UByte*)(psourceBatchMesh->GetBuffer())->pData;
            UByte* pvertexSource      = psourceBufferStart + psourceBatchMesh->GetVertexOffset() +
                prevVertexCount * psrcFmt->Size;

            // If the meshes have the same format, then we can copy the mesh data directly, and just update
            // the instancing indices for the new location. If they do not have the same data, we will need to
            // convert it.
            if ( psrcFmt && pbatch->pFormat == psrcFmt )
            {
                memcpy(pvertexDataStart, pvertexSource, pmesh->VertexCount * pbatch->pFormat->Size);
                // Assign proper instance index.
                InitializeVertices(*pbatch->pFormat, pvertexDataStart, pmesh->VertexCount, &convertArgArray[0]);
            }
            else
            {
                ConvertVertices(*psrcFmt, pvertexSource,
                    *pbatch->pFormat, pvertexDataStart,
                    pmesh->VertexCount, &convertArgArray[0]);
            }

            // Convert and assign indices (always the same format).
            IndexType* pindexSource = ((IndexType*)(psourceBufferStart + psourceBatchMesh->GetIndexOffset())) + prevIndexCount;
            ConvertIndices(pindexDataStart, pindexSource, pmesh->IndexCount, -(IndexType)(prevVertexCount) + (IndexType)indexStart);
            pindexDataStart += pmesh->IndexCount;
        }

        pvertexDataStart += pmesh->VertexCount * pbatch->pFormat->Size;
        indexStart       += pmesh->VertexCount;
    }   

    // Invalidate the CPU/GPU cache
    PostUpdateMesh( batchData );

    // ~StagingBufferPrep will Unpin meshes in the staging buffer.             
    return true;
}

void SimpleMeshCache::GetStats(Stats* stats)
{
    stats->TotalSize[MeshBuffer_Common] = TotalSize;
    stats->UsedSize[MeshBuffer_Common] = Allocator.GetFreeSize();
}

}}; // namespace Scaleform::Render

