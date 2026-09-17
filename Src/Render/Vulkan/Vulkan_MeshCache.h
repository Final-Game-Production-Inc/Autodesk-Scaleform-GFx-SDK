/**************************************************************************

Filename    :   Vulkan_MeshCache.h
Content     :   Vulkan mesh cache - vertex/index buffer management.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_VULKAN_MESHCACHE_H
#define INC_SF_VULKAN_MESHCACHE_H
#pragma once

#include "Render/Vulkan/Vulkan_Config.h"
#include "Render/Vulkan/Vulkan_ShaderDescs.h"
#include "Render/Render_MeshCache.h"
#include "Render/Vulkan/Vulkan_Sync.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace Vulkan {

class HAL;
class MeshCache;
class MeshBuffer;
class MeshBufferSet;

class MeshBuffer : public Render::MeshBuffer
{
public:
    VkBuffer        Buffer;
    VkDeviceMemory  Memory;
    UByte*          pData;
    VkBufferUsageFlags Usage;

    MeshBuffer(UPInt size, AllocType type, unsigned arena, VkBufferUsageFlags usage)
        : Render::MeshBuffer(size, type, arena),
          Buffer(VK_NULL_HANDLE), Memory(VK_NULL_HANDLE), pData(0), Usage(usage) {}
    ~MeshBuffer();

    bool    Create(VkDevice device, VkPhysicalDevice physDevice);
    void    Destroy(VkDevice device);

    inline  UByte* Lock() { return pData; }
    inline  void   Unlock() {}
    VkBuffer GetHWBuffer() const { return Buffer; }
};

class VertexBuffer : public MeshBuffer
{
public:
    VertexBuffer(UPInt size, AllocType type, unsigned arena)
        : MeshBuffer(size, type, arena, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {}
};

class IndexBuffer : public MeshBuffer
{
public:
    IndexBuffer(UPInt size, AllocType type, unsigned arena)
        : MeshBuffer(size, type, arena, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {}
};


class MeshBufferSet
{
public:
    typedef MeshBuffer::AllocType AllocType;

    VkBufferUsageFlags  Usage;
    unsigned            Granularity;
    unsigned            Arena;
    ArrayLH<MeshBuffer*> Buffers;

    MeshBufferSet(VkBufferUsageFlags usage, unsigned granularity, unsigned arena)
        : Usage(usage), Granularity(granularity), Arena(arena) {}
    ~MeshBufferSet() { DestroyAll(); }

    MeshBuffer* CreateBuffer(VkDevice device, VkPhysicalDevice physDevice,
                             UPInt size, AllocType type);
    void        DestroyAll();

    inline UPInt GetTotalSize() const
    {
        UPInt total = 0;
        for (unsigned i = 0; i < Buffers.GetSize(); i++)
            total += Buffers[i]->GetSize();
        return total;
    }
};


class MeshCacheItem : public Render::MeshCacheItem
{
    friend class MeshCache;

public:
    VertexBuffer*   pVertexBuffer;
    IndexBuffer*    pIndexBuffer;
    UPInt           VBAllocOffset;
    UPInt           VBAllocSize;
    UPInt           IBAllocOffset;
    UPInt           IBAllocSize;

    MeshCacheItem()
        : pVertexBuffer(0), pIndexBuffer(0),
          VBAllocOffset(0), VBAllocSize(0),
          IBAllocOffset(0), IBAllocSize(0) {}
};


class MeshCache : public Render::MeshCache
{
    friend class HAL;

    RenderSync          RSync;
    MeshCacheListSet    CacheList;
    MeshBufferSet       VertexBuffers;
    MeshBufferSet       IndexBuffers;
    bool                Mapped;

    // Suballocation state: bump-allocate from the current buffer, create
    // a new one only when it fills up.
    VertexBuffer*       pCurrentVB;
    UPInt               CurrentVBFreeOffset;
    IndexBuffer*        pCurrentIB;
    UPInt               CurrentIBFreeOffset;

    VkDevice            pDevice;
    VkPhysicalDevice    pPhysicalDevice;

    // Deferred buffer destruction: buffers are tagged with the frame after which it is
    // safe to destroy them (GPU has finished all submitted work that references them).
    struct PendingDestroyEntry
    {
        MeshBuffer* pBuffer;
        unsigned    SafeAfterFrame; // destroy when DestroyFrameCount >= SafeAfterFrame
    };
    ArrayLH<PendingDestroyEntry> PendingDestroyBuffers;
    unsigned            DestroyFrameCount; // monotonically incrementing, advances in BeginFrame

    VkBuffer            MaskEraseBatchVertexBuffer;
    VkDeviceMemory      MaskEraseBatchVertexMemory;

    inline MeshCache* getThis() { return this; }

    enum {
        MaxEraseBatchCount = SF_RENDER_MAX_BATCHES
    };

    bool                createMaskEraseBatchVertexBuffer();
    void                destroyMaskEraseBatchVertexBuffer();
    // Destroy all pending entries whose SafeAfterFrame <= DestroyFrameCount.
    void                destroyReadyPendingBuffers();
    // Destroy ALL pending entries unconditionally (caller must ensure GPU is idle).
    void                destroyAllPendingBuffers();

public:
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params);
    ~MeshCache();

    bool        Initialize(VkDevice device, VkPhysicalDevice physDevice, RenderSync* psync);
    void        Reset();

    virtual QueueMode   GetQueueMode() const { return QM_WaitForFences; }

    virtual bool        SetParams(const MeshCacheParams& params);

    virtual AllocResult AllocCacheItem(Render::MeshCacheItem** pdata,
                                        UByte** pvertexDataStart, IndexType** pindexDataStart,
                                        MeshCacheItem::MeshType meshType,
                                        MeshCacheItem::MeshBaseContent& mc,
                                        UPInt vertexBufferSize,
                                        unsigned vertexCount, unsigned indexCount,
                                        bool waitForCache, const VertexFormat* pDestFormat);

    virtual bool        LockBuffers();
    virtual void        UnlockBuffers();
    virtual bool        AreBuffersLocked() const { return Mapped; }

    virtual void        BeginFrame();
    virtual void        EndFrame();
    virtual void        EvictAll();
    virtual bool        PreparePrimitive(PrimitiveBatch* pbatch,
                                          MeshCacheItem::MeshContent &mc, bool waitForCache);

    RenderSync*         GetRenderSync()   { return &RSync; }

    virtual UPInt       Evict(Render::MeshCacheItem* p, AllocAddr* pallocator = 0, MeshBase* pskipMesh = 0);
    virtual void        ClearCache();

    VkBuffer            GetMaskEraseBatchVB() const { return MaskEraseBatchVertexBuffer; }
};


}}} // Scaleform::Render::Vulkan

#endif // INC_SF_VULKAN_MESHCACHE_H
