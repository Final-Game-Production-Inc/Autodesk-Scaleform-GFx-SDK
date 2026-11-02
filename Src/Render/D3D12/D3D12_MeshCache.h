/**************************************************************************

    PublicHeader:   Render
    Filename    :   D3D12_MeshCache.h
    Content     :   D3D12 mesh cache implementation.
                    Buffers are created on the D3D12_HEAP_TYPE_UPLOAD heap and
                    are therefore CPU-writable while remaining readable by the
                    GPU, which mirrors the "extended locks" behavior of the
                    D3D1x backend.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_MESHCACHE_H
#define INC_SF_D3D12_MESHCACHE_H
#pragma once

#include "Render/D3D12/D3D12_Config.h"
#include "Render/D3D12/D3D12_Sync.h"
#include "Render/Render_MeshCache.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace D3D12 {

class MeshBuffer;
class MeshBufferSet;
class VertexBuffer;
class IndexBuffer;
class MeshCache;
class HAL;
class ShaderManager;

// D3D12 version of MeshCacheItem. In D3D12 index and vertex buffers
// are allocated separately.
class MeshCacheItem : public Render::MeshCacheItem
{
    friend class MeshCache;
    friend class MeshBuffer;
    friend class HAL;

    VertexBuffer* pVertexBuffer;
    IndexBuffer* pIndexBuffer;
    // Ranges of allocation in vertex and index buffers.
    UPInt        VBAllocOffset, VBAllocSize;
    UPInt        IBAllocOffset, IBAllocSize;

public:

    static MeshCacheItem* Create(MeshType type,
                                 MeshCacheListSet* pcacheList, MeshBaseContent& mc,
                                 VertexBuffer* pvb, IndexBuffer* pib,
                                 UPInt vertexOffset, UPInt vertexAllocSize, unsigned vertexCount,
                                 UPInt indexOffset, UPInt indexAllocSize, unsigned indexCount)
    {
        MeshCacheItem* p = (D3D12::MeshCacheItem*)
            Render::MeshCacheItem::Create(type, pcacheList, sizeof(MeshCacheItem), mc,
                                          vertexAllocSize + indexAllocSize, vertexCount, indexCount);
        if (p)
        {
            p->pVertexBuffer = pvb;
            p->pIndexBuffer  = pib;
            p->VBAllocOffset = vertexOffset;
            p->VBAllocSize   = vertexAllocSize;
            p->IBAllocOffset = indexOffset;
            p->IBAllocSize   = indexAllocSize;
        }
        return p;
    }
};


enum {
    MeshCache_MaxBufferCount      = 256,
    MeshCache_AddressToIndexShift = 24,
    // A multi-byte allocation unit is used in MeshBufferSet::Allocator.
    MeshCache_AllocatorUnitShift  = 4,
    MeshCache_AllocatorUnit       = 1 << MeshCache_AllocatorUnitShift
};


class MeshBuffer : public Render::MeshBuffer
{
protected:
    UPInt       Index; // Index in MeshBufferSet::Buffers array.
    MeshBuffer* pNextLock;

public:

    enum BufferType
    {
        Buffer_Vertex,
        Buffer_Index,
    };

    MeshBuffer(UPInt size, AllocType type, unsigned arena)
        : Render::MeshBuffer(size, type, arena)
    { }
    virtual ~MeshBuffer() { }

    inline  UPInt   GetIndex() const { return Index; }

    virtual bool    allocBuffer(ID3D12Device* pdevice) = 0;
    virtual bool    DoLock(ID3D12Device* pdevice) = 0;
    virtual void    Unlock(ID3D12Device* pdevice) = 0;
    virtual BufferType GetBufferType() const = 0;

    // Simple LockList class is used to track all MeshBuffers that were locked.
    struct LockList
    {
        MeshBuffer *pFirst;
        LockList() : pFirst(0) { }

        void Add(MeshBuffer *pbuffer)
        {
            pbuffer->pNextLock = pFirst;
            pFirst = pbuffer;
        }
        void UnlockAll(ID3D12Device* pdevice)
        {
            MeshBuffer* p = pFirst;
            while(p)
            {
                p->Unlock(pdevice);
                p = p->pNextLock;
            }
            pFirst = 0;
        }
    };

    inline  UByte*  Lock(LockList& lockedBuffers, ID3D12Device* pdevice)
    {
        if (!pData)
        {
            if (!DoLock(pdevice))
            {
                SF_DEBUG_WARNING(1, "Render::MeshCache - Vertex/IndexBuffer lock failed");
                return 0;
            }
            lockedBuffers.Add(this);
        }
        return (UByte*)pData;
    }
};


class MeshBufferSet
{
public:
    typedef MeshBuffer::AllocType AllocType;

    // Buffers are addressable; index is stored in the upper bits of allocation address,
    // as tracked by the allocator. Buffers array may contain null pointers for freed buffers.
    ArrayLH<MeshBuffer*>  Buffers;
    AllocAddr             Allocator;
    UPInt                 Granularity;
    UPInt                 TotalSize;

    MeshBufferSet(MemoryHeap* pheap, UPInt granularity)
        : Allocator(pheap), Granularity(granularity),  TotalSize(0)
    { }

    virtual ~MeshBufferSet()
    { }

    inline void         SetGranularity(UPInt granularity)
    { Granularity = granularity; }

    inline AllocAddr&   GetAllocator()          { return Allocator; }
    inline UPInt        GetGranularity() const  { return Granularity; }
    inline UPInt        GetTotalSize() const    { return TotalSize; }

    virtual MeshBuffer* CreateBuffer(UPInt size, AllocType type, unsigned arena,
                                     MemoryHeap* pheap, ID3D12Device* pdevice) = 0;

    // Destroys all buffers of specified type; all types are destroyed if AT_None is passed.
    void        DestroyBuffers(AllocType type = MeshBuffer::AT_None)
    {
        for (UPInt i = 0; i< Buffers.GetSize(); i++)
        {
            if (Buffers[i] && ((type == MeshBuffer::AT_None) || (Buffers[i]->GetType() == type)))
                DestroyBuffer(Buffers[i]);
        }
    }

    static inline UPInt SizeToAllocatorUnit(UPInt size)
    {
        return (size + MeshCache_AllocatorUnit - 1) >> MeshCache_AllocatorUnitShift;
    }

    void        DestroyBuffer(MeshBuffer* pbuffer, bool deleteBuffer = true)
    {
        Allocator.RemoveSegment(pbuffer->GetIndex() << MeshCache_AddressToIndexShift,
                                SizeToAllocatorUnit(pbuffer->GetSize()));
        TotalSize -= pbuffer->GetSize();
        Buffers[pbuffer->GetIndex()] = 0;
        if ( deleteBuffer )
        {
            delete pbuffer;
        }
    }

    // Alloc
    inline bool    Alloc(UPInt size, MeshBuffer** pbuffer, UPInt* poffset)
    {
        UPInt offset = Allocator.Alloc(SizeToAllocatorUnit(size));
        if (offset == ~UPInt(0))
            return false;
        *pbuffer = Buffers[offset >> MeshCache_AddressToIndexShift];
        *poffset = (offset & ((1 << MeshCache_AddressToIndexShift) - 1)) << MeshCache_AllocatorUnitShift;
        return true;
    }

    inline UPInt    Free(UPInt size, MeshBuffer* pbuffer, UPInt offset)
    {
        return Allocator.Free(
            (pbuffer->GetIndex() << MeshCache_AddressToIndexShift) | (offset >> MeshCache_AllocatorUnitShift),
            SizeToAllocatorUnit(size)) << MeshCache_AllocatorUnitShift;
    }
};


template<class Buffer>
class MeshBufferSetImpl : public MeshBufferSet
{
public:
    MeshBufferSetImpl(MemoryHeap* pheap, UPInt granularity)
        : MeshBufferSet(pheap, granularity)
    { }
    virtual MeshBuffer* CreateBuffer(UPInt size, AllocType type, unsigned arena,
                                     MemoryHeap* pheap, ID3D12Device* pdevice)
    {
        // Single buffer can't be bigger then (1<<MeshCache_AddressToIndexShift)
        // size due to the way addresses are masked.
        SF_ASSERT(size <= (1<<(MeshCache_AddressToIndexShift+MeshCache_AllocatorUnitShift)));
        return  Buffer::Create(size, type, arena, pheap, pdevice, *this);
    }
};


template<class BType, class Derived>
class MeshBufferImpl : public MeshBuffer
{
protected:
    typedef MeshBufferImpl      Base;
    Ptr<BType>                  pBuffer;
    D3D12_RANGE                 MappedRange;
public:

    MeshBufferImpl(UPInt size, AllocType type, unsigned arena)
        : MeshBuffer(size, type, arena)
    {
        MappedRange.Begin = 0;
        MappedRange.End = (SIZE_T)size;
    }

    inline BType* GetHWBuffer() const { return pBuffer.GetPtr(); }

    bool   DoLock(ID3D12Device* pdevice)
    {
        SF_UNUSED(pdevice);
        SF_ASSERT(!pData);
        if ( SUCCEEDED( GetHWBuffer()->Map(0, &MappedRange, &pData)) )
        {
            return true;
        }
        return false;
    }
    void    Unlock(ID3D12Device* pdevice)
    {
        SF_UNUSED(pdevice);
        GetHWBuffer()->Unmap(0, &MappedRange);
        pData = 0;
    }

    static Derived* Create(UPInt size, AllocType type, unsigned arena,
                           MemoryHeap* pheap, ID3D12Device* pdevice,
                           MeshBufferSet& mbs)
    {
        // Determine which address index we'll use in the allocator.
        UPInt index = 0;
        while ((index < mbs.Buffers.GetSize()) && mbs.Buffers[index])
            index++;
        if (index == MeshCache_MaxBufferCount)
            return 0;

        // Round-up to Allocator unit.
        size = ((size + MeshCache_AllocatorUnit-1) >> MeshCache_AllocatorUnitShift) << MeshCache_AllocatorUnitShift;

        Derived* p = SF_HEAP_NEW(pheap) Derived(size, type, arena);
        if (!p) return 0;

        if (!p->allocBuffer(pdevice))
        {
            delete p;
            return 0;
        }
        p->Index = index;

        mbs.Allocator.AddSegment(index << MeshCache_AddressToIndexShift, size >> MeshCache_AllocatorUnitShift);
        mbs.TotalSize += size;

        if (index == mbs.Buffers.GetSize())
            mbs.Buffers.PushBack(p);
        else
            mbs.Buffers[index] = p;

        return p;
    }
};

class VertexBuffer : public MeshBufferImpl<ID3D12Resource, VertexBuffer>
{
public:
    VertexBuffer(UPInt size, AllocType atype, unsigned arena)
        : Base(size, atype, arena)
    { }

    virtual bool allocBuffer(ID3D12Device* pdevice)
    {
        D3D12_HEAP_PROPERTIES heapProps;
        memset(&heapProps, 0, sizeof(heapProps));
        heapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask     = 1;
        heapProps.VisibleNodeMask      = 1;

        D3D12_RESOURCE_DESC resDesc;
        memset(&resDesc, 0, sizeof(resDesc));
        resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment          = 0;
        resDesc.Width              = Size;
        resDesc.Height             = 1;
        resDesc.DepthOrArraySize   = 1;
        resDesc.MipLevels          = 1;
        resDesc.Format             = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count   = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        return SUCCEEDED( pdevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                           &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                           NULL, IID_PPV_ARGS(&pBuffer.GetRawRef())) );
    }
    virtual BufferType GetBufferType() const { return Buffer_Vertex; }
};

class IndexBuffer : public MeshBufferImpl<ID3D12Resource, IndexBuffer>
{
public:
    IndexBuffer(UPInt size, AllocType atype, unsigned arena)
        : Base(size, atype, arena)
    { }

    virtual bool allocBuffer(ID3D12Device* pdevice)
    {
        D3D12_HEAP_PROPERTIES heapProps;
        memset(&heapProps, 0, sizeof(heapProps));
        heapProps.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask     = 1;
        heapProps.VisibleNodeMask      = 1;

        D3D12_RESOURCE_DESC resDesc;
        memset(&resDesc, 0, sizeof(resDesc));
        resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Alignment          = 0;
        resDesc.Width              = Size;
        resDesc.Height             = 1;
        resDesc.DepthOrArraySize   = 1;
        resDesc.MipLevels          = 1;
        resDesc.Format             = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count   = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        return SUCCEEDED( pdevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                           &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                           NULL, IID_PPV_ARGS(&pBuffer.GetRawRef())) );
    }
    virtual BufferType GetBufferType() const { return Buffer_Index; }
};

typedef MeshBufferSetImpl<VertexBuffer> VertexBufferSet;
typedef MeshBufferSetImpl<IndexBuffer>  IndexBufferSet;



// D3D12 mesh cache implementation with following characteristics:
//  - Multiple cache lists and allocators; one per Buffer.
//  - Relies on 'Extended Locks'; buffers live on the D3D12 UPLOAD heap and
//    are mapped/unmapped around lock/unlock operations.

class MeshCache : public Render::MeshCache
{
    friend class HAL;

    enum {
        MinSupportedGranularity = 16*1024,
    };

    Ptr<ID3D12Device>           pDevice;
    Ptr<ID3D12GraphicsCommandList> pCommandList;
    Ptr<RenderSync>             pRenderSync;
    ShaderManager*              pShaderManager;
    MeshCacheListSet            CacheList;

    // Allocators managing the buffers.
    VertexBufferSet             VertexBuffers;
    IndexBufferSet              IndexBuffers;

    // Locked buffer info.
    bool                        Locked;
    UPInt                       VBSizeEvictedInLock;
    MeshBuffer::LockList        LockedBuffers;
    // A list of all buffers in the order they were allocated. Used to allow
    // freeing them in the opposite order (to support proper IB/VB balancing).
    // NOTE: Must use base MeshBuffer type for List<> to work with internal casts.
    List<Render::MeshBuffer>    ChunkBuffers;
    // If a MeshBuffer could not be immediately destroyed (the GPU is using one of its meshes)
    // It is temporarily stored in this list until it is no longer in use.
    List<Render::MeshBuffer>    PendingDestructionBuffers;

    Ptr<ID3D12Resource>         pMaskEraseBatchVertexBuffer;

    inline MeshCache* getThis() { return this; }

    inline UPInt    getTotalSize() const
    {
        return VertexBuffers.GetTotalSize() + IndexBuffers.GetTotalSize();
    }

    bool            createStaticVertexBuffers(ID3D12Device* pdevice);
    bool            createMaskEraseBatchVertexBuffer(ID3D12Device* pdevice);

    // Allocates Vertex/Index buffer of specified size and adds it to free list.
    bool            allocCacheBuffers(UPInt size, MeshBuffer::AllocType type, unsigned arena = 0);

    bool            evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                        MeshBuffer* pbuffer);

    // Allocates a number of bytes in the specified buffer, while evicting LRU data.
    // Buffer can contain either vertex and/or index data.
    bool            allocBuffer(UPInt* poffset, MeshBuffer** pbuffer,
                                MeshBufferSet& mbs, UPInt size, bool waitForCache);

    void            destroyBuffers(MeshBuffer::AllocType at = MeshBuffer::AT_None);

    void            adjustMeshCacheParams(MeshCacheParams* p);

    // If buffers could not be deleted immediately (their meshes were still in use), they are stored
    // in PendingDestructionBuffers and destroyed in this function (if possible).
    void            destroyPendingBuffers();

public:

    MeshCache(MemoryHeap* pheap,
              const MeshCacheParams& params);
    ~MeshCache();

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from HAL::InitHAL.
    bool            Initialize(ID3D12Device* pdevice, RenderSync* rsync, ShaderManager* psm,
                               ID3D12GraphicsCommandList* initCommandList);
    // Resets MeshCache, releasing all buffers.
    void            Reset();

    // Sets the command list used for any GPU copies (currently unused because
    // all buffers are CPU-writable UPLOAD resources, but kept for API parity).
    void            SetCommandList(ID3D12GraphicsCommandList* commandList);

    virtual QueueMode GetQueueMode() const { return QM_ExtendLocks; }


    // *** MeshCacheConfig Interface
    virtual void    ClearCache();
    virtual bool    SetParams(const MeshCacheParams& params);

    virtual void    BeginFrame();
    virtual void    EndFrame();

    virtual bool    LockBuffers();
    virtual void    UnlockBuffers();
    virtual bool    AreBuffersLocked() const { return Locked; }

    virtual UPInt   Evict(Render::MeshCacheItem* p, AllocAddr* pallocator = 0,
                          MeshBase* pmesh = 0);
    virtual bool    PreparePrimitive(PrimitiveBatch* pbatch,
                                     MeshCacheItem::MeshContent &mc, bool waitForCache);

    virtual AllocResult AllocCacheItem(Render::MeshCacheItem** pdata,
                                      UByte** pvertexDataStart, IndexType** pindexDataStart,
                                      MeshCacheItem::MeshType meshType,
                                      MeshCacheItem::MeshBaseContent &mc,
                                      UPInt vertexBufferSize,
                                      unsigned vertexCount, unsigned indexCount,
                                      bool waitForCache, const VertexFormat* pDestFormat);

    // Returns the vertex/index data pointers of a previously allocated item,
    // without re-locking the buffers (the buffers are locked for the duration
    // of the extended lock).
    void            LockMeshCacheItem(Render::MeshCacheItem* pdata,
                                      UByte** pvertexDataStart, IndexType** pindexDataStart);

    virtual void GetStats(Stats* stats);
};



// Vertex structures used by both MeshCache and HAL.

class Render_InstanceData
{
public:
    // Color format for instance, Alpha expected to have index.
    UInt32 Instance;
};


}}}  // namespace Scaleform::Render::D3D12

#endif // INC_SF_D3D12_MESHCACHE_H
