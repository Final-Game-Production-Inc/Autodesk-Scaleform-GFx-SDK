/**************************************************************************

Filename    :   Vulkan_MeshCache.cpp
Content     :   Vulkan mesh cache implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Render/Vulkan/Vulkan_MeshCache.h"
#include "Render/Vulkan/Vulkan_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_Primitive.h"

namespace Scaleform { namespace Render { namespace Vulkan {

// *** MeshBuffer

MeshBuffer::~MeshBuffer()
{
    SF_ASSERT(Buffer == VK_NULL_HANDLE && "MeshBuffer destroyed without calling Destroy()");
}

bool MeshBuffer::Create(VkDevice device, VkPhysicalDevice physDevice)
{
    VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCI.size  = Size;
    bufCI.usage = Usage;
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    SF_VK_CHECK_RETURN(vkCreateBuffer(device, &bufCI, nullptr, &Buffer), false);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, Buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(physDevice, memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (allocInfo.memoryTypeIndex == UINT32_MAX)
        return false;

    SF_VK_CHECK_RETURN(vkAllocateMemory(device, &allocInfo, nullptr, &Memory), false);
    SF_VK_CHECK_RETURN(vkBindBufferMemory(device, Buffer, Memory, 0), false);

    void* mapped = nullptr;
    SF_VK_CHECK_RETURN(vkMapMemory(device, Memory, 0, Size, 0, &mapped), false);
    pData = (UByte*)mapped;

    return true;
}

void MeshBuffer::Destroy(VkDevice device)
{
    if (pData && Memory != VK_NULL_HANDLE)
    {
        vkUnmapMemory(device, Memory);
        pData = nullptr;
    }
    if (Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, Buffer, nullptr);
        Buffer = VK_NULL_HANDLE;
    }
    if (Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, Memory, nullptr);
        Memory = VK_NULL_HANDLE;
    }
}


// *** MeshBufferSet

MeshBuffer* MeshBufferSet::CreateBuffer(VkDevice device, VkPhysicalDevice physDevice,
                                        UPInt size, MeshBuffer::AllocType type)
{
    MeshBuffer* pbuf;
    if (Usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        pbuf = SF_HEAP_AUTO_NEW(this) VertexBuffer(size, type, Arena);
    else
        pbuf = SF_HEAP_AUTO_NEW(this) IndexBuffer(size, type, Arena);

    if (!pbuf->Create(device, physDevice))
    {
        delete pbuf;
        return nullptr;
    }
    Buffers.PushBack(pbuf);
    return pbuf;
}

void MeshBufferSet::DestroyAll()
{
    for (unsigned i = 0; i < Buffers.GetSize(); i++)
        delete Buffers[i];
    Buffers.Clear();
}


// *** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params)
: Render::MeshCache(pheap, params),
  CacheList(this),
  VertexBuffers(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, 0),
  IndexBuffers(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, 0),
  Mapped(false),
  pCurrentVB(nullptr), CurrentVBFreeOffset(0),
  pCurrentIB(nullptr), CurrentIBFreeOffset(0),
  pDevice(VK_NULL_HANDLE), pPhysicalDevice(VK_NULL_HANDLE),
  DestroyFrameCount(0),
  MaskEraseBatchVertexBuffer(VK_NULL_HANDLE),
  MaskEraseBatchVertexMemory(VK_NULL_HANDLE)
{
}

MeshCache::~MeshCache()
{
    Reset();
}

bool MeshCache::Initialize(VkDevice device, VkPhysicalDevice physDevice, RenderSync* psync)
{
    pDevice = device;
    pPhysicalDevice = physDevice;
    pRQCaches = 0;

    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;
    if (!RSync.SetDevice(device, VK_NULL_HANDLE))
        return false;

    if (!createMaskEraseBatchVertexBuffer())
        return false;

    return true;
}

void MeshCache::Reset()
{
    pCurrentVB = nullptr;
    CurrentVBFreeOffset = 0;
    pCurrentIB = nullptr;
    CurrentIBFreeOffset = 0;

    destroyMaskEraseBatchVertexBuffer();
    destroyAllPendingBuffers();

    for (unsigned i = 0; i < VertexBuffers.Buffers.GetSize(); i++)
        VertexBuffers.Buffers[i]->Destroy(pDevice);
    VertexBuffers.DestroyAll();

    for (unsigned i = 0; i < IndexBuffers.Buffers.GetSize(); i++)
        IndexBuffers.Buffers[i]->Destroy(pDevice);
    IndexBuffers.DestroyAll();
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    Params = argParams;
    if (StagingBuffer.GetBuffer())
        StagingBuffer.Initialize(pHeap, Params.StagingBufferSize);
    return true;
}

UPInt MeshCache::Evict(Render::MeshCacheItem* p, AllocAddr* pallocator, MeshBase* pskipMesh)
{
    SF_UNUSED(pallocator);
    MeshCacheItem* pitem = (MeshCacheItem*)p;
    UPInt freedSize = pitem->VBAllocSize + pitem->IBAllocSize;

    // With suballocation, multiple items share the same buffer.
    // We cannot destroy the buffer until all items in it are gone.
    // Buffers are cleaned up in Reset() / ClearCache() instead.
    pitem->pVertexBuffer = 0;
    pitem->pIndexBuffer = 0;

    pitem->Destroy(pskipMesh, true);
    return freedSize;
}

void MeshCache::ClearCache()
{
    CacheList.EvictAll();

    pCurrentVB = nullptr;
    CurrentVBFreeOffset = 0;
    pCurrentIB = nullptr;
    CurrentIBFreeOffset = 0;

    if (pDevice != VK_NULL_HANDLE)
        vkDeviceWaitIdle(pDevice);
    destroyAllPendingBuffers();

    for (unsigned i = 0; i < VertexBuffers.Buffers.GetSize(); i++)
        VertexBuffers.Buffers[i]->Destroy(pDevice);
    VertexBuffers.DestroyAll();

    for (unsigned i = 0; i < IndexBuffers.Buffers.GetSize(); i++)
        IndexBuffers.Buffers[i]->Destroy(pDevice);
    IndexBuffers.DestroyAll();
}

void MeshCache::destroyReadyPendingBuffers()
{
    // Walk backwards so RemoveAt doesn't skip entries.
    for (int i = (int)PendingDestroyBuffers.GetSize() - 1; i >= 0; i--)
    {
        if (PendingDestroyBuffers[i].SafeAfterFrame <= DestroyFrameCount)
        {
            PendingDestroyBuffers[i].pBuffer->Destroy(pDevice);
            delete PendingDestroyBuffers[i].pBuffer;
            PendingDestroyBuffers.RemoveAt(i);
        }
    }
}

void MeshCache::destroyAllPendingBuffers()
{
    // Caller must ensure the GPU is idle (e.g. vkDeviceWaitIdle was called).
    for (UPInt i = 0; i < PendingDestroyBuffers.GetSize(); i++)
    {
        PendingDestroyBuffers[i].pBuffer->Destroy(pDevice);
        delete PendingDestroyBuffers[i].pBuffer;
    }
    PendingDestroyBuffers.Clear();
}

bool MeshCache::createMaskEraseBatchVertexBuffer()
{
    unsigned vertexCount = 6 * MaxEraseBatchCount;
    UPInt bufSize = vertexCount * sizeof(VertexXY16iAlpha);

    VkBufferCreateInfo bufCI = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCI.size  = bufSize;
    bufCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SF_VK_CHECK_RETURN(vkCreateBuffer(pDevice, &bufCI, nullptr, &MaskEraseBatchVertexBuffer), false);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(pDevice, MaskEraseBatchVertexBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX)
    {
        SF_DEBUG_WARNING(1, "Vulkan: No suitable memory type for mask erase batch VB");
        return false;
    }
    SF_VK_CHECK_RETURN(vkAllocateMemory(pDevice, &allocInfo, nullptr, &MaskEraseBatchVertexMemory), false);
    SF_VK_CHECK_RETURN(vkBindBufferMemory(pDevice, MaskEraseBatchVertexBuffer, MaskEraseBatchVertexMemory, 0), false);

    void* pdata = nullptr;
    SF_VK_CHECK_RETURN(vkMapMemory(pDevice, MaskEraseBatchVertexMemory, 0, bufSize, 0, &pdata), false);

    fillMaskEraseVertexBuffer<VertexXY16iAlpha>((VertexXY16iAlpha*)pdata, MaxEraseBatchCount);
    vkUnmapMemory(pDevice, MaskEraseBatchVertexMemory);

    return true;
}

void MeshCache::destroyMaskEraseBatchVertexBuffer()
{
    if (MaskEraseBatchVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(pDevice, MaskEraseBatchVertexBuffer, nullptr);
        MaskEraseBatchVertexBuffer = VK_NULL_HANDLE;
    }
    if (MaskEraseBatchVertexMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(pDevice, MaskEraseBatchVertexMemory, nullptr);
        MaskEraseBatchVertexMemory = VK_NULL_HANDLE;
    }
}

MeshCache::AllocResult MeshCache::AllocCacheItem(
    Render::MeshCacheItem** pdata,
    UByte** pvertexDataStart, IndexType** pindexDataStart,
    MeshCacheItem::MeshType meshType,
    MeshCacheItem::MeshBaseContent& mc,
    UPInt vertexBufferSize,
    unsigned vertexCount, unsigned indexCount,
    bool waitForCache, const VertexFormat* pDestFormat)
{
    SF_UNUSED(waitForCache);

    if (!pDevice)
        return Alloc_StateError;

    UPInt vbSize = vertexBufferSize;
    UPInt ibSize = indexCount * sizeof(IndexType);

    // Align VB to 16 bytes, IB to sizeof(IndexType)
    UPInt vbAligned = (vbSize + 15) & ~(UPInt)15;
    UPInt ibAligned = (ibSize + (sizeof(IndexType) - 1)) & ~(UPInt)(sizeof(IndexType) - 1);

    // Suballocate from current buffers; create a new one only when full.
    VertexBuffer* pvb = nullptr;
    IndexBuffer* pib = nullptr;
    UPInt vbOffset = 0, ibOffset = 0;

    if (!pCurrentVB || CurrentVBFreeOffset + vbAligned > pCurrentVB->GetSize())
    {
        UPInt newBufSize = Alg::Max<UPInt>(Params.MemGranularity, vbAligned);
        MeshBuffer* nb = VertexBuffers.CreateBuffer(pDevice, pPhysicalDevice, newBufSize, MeshBuffer::AT_Chunk);
        if (!nb) return Alloc_Fail;
        pCurrentVB = (VertexBuffer*)nb;
        CurrentVBFreeOffset = 0;
    }

    if (!pCurrentIB || CurrentIBFreeOffset + ibAligned > pCurrentIB->GetSize())
    {
        UPInt newBufSize = Alg::Max<UPInt>(Params.MemGranularity, ibAligned);
        MeshBuffer* nb = IndexBuffers.CreateBuffer(pDevice, pPhysicalDevice, newBufSize, MeshBuffer::AT_Chunk);
        if (!nb) return Alloc_Fail;
        pCurrentIB = (IndexBuffer*)nb;
        CurrentIBFreeOffset = 0;
    }

    pvb = pCurrentVB;
    vbOffset = CurrentVBFreeOffset;
    CurrentVBFreeOffset += vbAligned;

    pib = pCurrentIB;
    ibOffset = CurrentIBFreeOffset;
    CurrentIBFreeOffset += ibAligned;

    MeshCacheItem* pitem = (MeshCacheItem*)
        MeshCacheItem::Create(meshType, &CacheList, sizeof(MeshCacheItem), mc, vertexBufferSize + ibSize, vertexCount, indexCount);
    if (!pitem)
        return Alloc_Fail;

    pitem->pVertexBuffer = pvb;
    pitem->pIndexBuffer  = pib;
    pitem->VBAllocOffset = vbOffset;
    pitem->VBAllocSize   = vbSize;
    pitem->IBAllocOffset = ibOffset;
    pitem->IBAllocSize   = ibSize;

    *pvertexDataStart = pvb->pData + vbOffset;
    *pindexDataStart  = (IndexType*)(pib->pData + ibOffset);
    *pdata = pitem;

    return Alloc_Success;
}

bool MeshCache::LockBuffers()
{
    // Buffers are persistently mapped - nothing to do
    Mapped = true;
    return true;
}

void MeshCache::UnlockBuffers()
{
    Mapped = false;
}

void MeshCache::BeginFrame()
{
    // Advance the frame counter first so that the threshold comparison in
    // destroyReadyPendingBuffers() uses the updated count. By the time BeginFrame
    // is called, the main loop has already called vkWaitForFences for the oldest
    // in-flight frame, so buffers tagged SafeAfterFrame <= DestroyFrameCount are
    // guaranteed to no longer be referenced by any submitted command buffer.
    DestroyFrameCount++;
    destroyReadyPendingBuffers();
}

void MeshCache::EndFrame()
{
    CacheList.EndFrame();
    // Buffer destruction is now deferred to BeginFrame (after GPU fence wait).
    // Do NOT call destroyAllPendingBuffers() here.
}

void MeshCache::EvictAll()
{
    CacheList.EvictAll();
}

bool MeshCache::PreparePrimitive(PrimitiveBatch* pbatch,
                                  MeshCacheItem::MeshContent& mc, bool waitForCache)
{
    Render::MeshCacheItem* pmesh = (Render::MeshCacheItem*)pbatch->GetCacheItem();
    if (pmesh)
    {
        pmesh->MoveToCacheListFront(MCL_ThisFrame);
        return true;
    }

    Primitive* prim = pbatch->GetPrimitive();

    if (mc.IsLargeMesh())
    {
        SF_ASSERT(mc.GetMeshCount() == 1);
        MeshResult mr = GenerateMesh(mc[0], prim->GetVertexFormat(),
                                     pbatch->pFormat, 0, waitForCache);
        if (mr.Succeded())
            pbatch->SetCacheItem(mc[0]->CacheItems[0]);
        if (mr == MeshResult::Fail_LargeMesh_NeedCache)
            return false;
        return true;
    }

    unsigned totalVertexCount, totalIndexCount;
    pbatch->CalcMeshSizes(&totalVertexCount, &totalIndexCount);

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
        return (allocResult == Alloc_Fail) ? false : true;

    pbatch->SetCacheItem(batchData);

    StagingBufferPrep meshPrep(this, mc, prim->GetVertexFormat(), false);

    UByte*      pstagingBuffer   = StagingBuffer.GetBuffer();
    const VertexFormat* pvf      = prim->GetVertexFormat();
    const VertexFormat* pdvf     = pbatch->pFormat;

    unsigned    i;
    unsigned    indexStart = 0;

    for(i = 0; i < mc.GetMeshCount(); i++)
    {
        Mesh* pmesh2 = mc[i];
        SF_ASSERT(pmesh2->StagingBufferSize != 0);

        void*   convertArgArray[1] = { &i };
        ConvertVertices_Buffered(*pvf, pstagingBuffer + pmesh2->StagingBufferOffset,
                                 *pdvf, pvertexDataStart,
                                 pmesh2->VertexCount, &convertArgArray[0]);
        ConvertIndices(pindexDataStart,
                       (IndexType*)(pstagingBuffer + pmesh2->StagingBufferIndexOffset),
                       pmesh2->IndexCount, (IndexType)indexStart);

        pvertexDataStart += pmesh2->VertexCount * destVertexSize;
        pindexDataStart  += pmesh2->IndexCount;
        indexStart       += pmesh2->VertexCount;
    }

    return true;
}

}}} // Scaleform::Render::Vulkan
