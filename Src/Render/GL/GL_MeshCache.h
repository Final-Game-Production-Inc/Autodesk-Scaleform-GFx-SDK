/**************************************************************************

Filename    :   GL_MeshCache.h
Content     :   GL Mesh Cache implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GL_MeshCache_H
#define INC_SF_Render_GL_MeshCache_H

#include "Render/Render_MeshCache.h"
#include "Render/GL/GL_Common.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace GL {

class MeshBuffer;
class MeshBufferSet;
class MeshCache;
class HAL;


class MeshCacheItem : public Render::MeshCacheItem
{
    friend class MeshCache;
    friend class MeshBuffer;
    friend class HAL;
    friend class VertexBuilder_Legacy;
    friend class VertexBuilder_Core30;
    friend class VertexBuilder_VAOES;

    MeshBuffer*     pVertexBuffer;
    MeshBuffer*     pIndexBuffer;
    UPInt           VBAllocOffset, VBAllocSize;
    UPInt           IBAllocOffset, IBAllocSize;

    GLuint              VAO;            // The vertex array object name this item uses. 
    const VertexFormat* VAOFormat;      // The vertex format used to construct the current VAO.
    UByte*              VAOOffset;      // The base vertex offset to construct the current VAO.

public:

    static MeshCacheItem* Create(MeshType type,
                                 MeshCacheListSet* pcacheList, MeshBaseContent& mc,
                                 MeshBuffer* pvb, MeshBuffer* pib,
                                 UPInt vertexOffset, UPInt vertexAllocSize, unsigned vertexCount,
                                 UPInt indexOffset, UPInt indexAllocSize, unsigned indexCount)
    {
        MeshCacheItem* p = (GL::MeshCacheItem*)
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
            p->VAO           = 0;
            p->VAOFormat     = 0;
            p->VAOOffset     = 0;
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
    HAL*        pHal;
    GLuint      Buffer;
    UByte*      BufferData;
    GLenum      Type;
    UPInt       Index; // Index in MeshBufferSet::Buffers array.
    MeshBuffer* pNextLock;

    friend class MeshBufferSet;

    struct MeshBufferUpdateEntry
    {
        MeshBufferUpdateEntry() : Offset(0), Size(0) { }
        MeshBufferUpdateEntry(UPInt o, UPInt s) : Offset(o), Size(s) { }
        UPInt Offset;
        UPInt Size;
    };
    typedef ArrayConstPolicy<0, 8, true> NeverShrinkPolicy;
    ArrayLH<MeshBufferUpdateEntry, StatRender_MeshCacheMgmt_Mem, NeverShrinkPolicy> MeshBufferUpdates;

public:
    MeshBuffer(HAL* phal, GLenum btype, UPInt size, AllocType type, unsigned arena)
        : Render::MeshBuffer(size, type, arena)
    { 
        pHal = phal; 
        Buffer = 0; 
        BufferData = 0; 
        pNextLock = 0; 
        Type = btype; 
        CurrentBuffer = 0xffffffff; 
    }
    ~MeshBuffer();

    inline  UPInt   GetIndex() const { return Index; }
    inline  HAL* GetHAL() const { return pHal; }

    bool    allocBuffer();
    bool    DoMap(UPInt offset, UPInt size);
    void    Unmap();
    GLuint  GetBuffer() { return Buffer; }
    UByte*  GetBufferBase() const;
    GLenum  GetBufferType() const { return Type; }

    static GLuint CurrentBuffer;

    // Simple LockList class is used to track all MeshBuffers that were locked.
    struct MapList
    {
        MeshBuffer *pFirst;
        MapList() : pFirst(0) { }

        void Add(MeshBuffer *pbuffer)
        {
            pbuffer->pNextLock = pFirst;
            pFirst = pbuffer;
        }
        void UnmapAll()
        {
            MeshBuffer* p = pFirst;
            while(p)
            {
                p->Unmap();
                p = p->pNextLock;
            }
            pFirst = 0;
        }
    };

    inline  UByte*  Map(MapList& lockedBuffers, UPInt offset, UPInt size)
    {
        if (!pData)
        {  
            lockedBuffers.Add(this);
        }
        if (!DoMap(offset, size))
            {
            SF_DEBUG_WARNONCE(1, "Render::MeshCache - Vertex/IndexBuffer map failed");
                return 0;
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
    GLenum                 BufferType;
    ArrayLH<MeshBuffer*>   Buffers;
    AllocAddr              Allocator;
    UPInt                  Granularity;
    UPInt                  TotalSize;

    MeshBufferSet(GLenum type, MemoryHeap* pheap, UPInt granularity)
        : BufferType(type), Allocator(pheap), Granularity(granularity),  TotalSize(0)
    { }

    inline void         SetGranularity(UPInt granularity)
    { Granularity = granularity; }

    inline AllocAddr&   GetAllocator()          { return Allocator; }
    inline UPInt        GetGranularity() const  { return Granularity; }
    inline UPInt        GetTotalSize() const    { return TotalSize; }

    MeshBuffer* CreateBuffer(UPInt size, AllocType type, unsigned arena, MemoryHeap* pheap, HAL* phal)
    {
        // Single buffer can't be bigger then (1<<MeshCache_AddressToIndexShift)
        // size due to the way addresses are masked.
        SF_ASSERT(size <= (1<<(MeshCache_AddressToIndexShift+MeshCache_AllocatorUnitShift)));

        // Determine which address index we'll use in the allocator.
        UPInt index = 0;
        while ((index < Buffers.GetSize()) && Buffers[index])
            index++;
        if (index == MeshCache_MaxBufferCount)
            return 0;

        // Round-up to Allocator unit.
        size = ((size + MeshCache_AllocatorUnit-1) >> MeshCache_AllocatorUnitShift) << MeshCache_AllocatorUnitShift;

        MeshBuffer* p = SF_HEAP_NEW(pheap) MeshBuffer(phal, BufferType, size, type, arena);
        if (!p) return 0;

        if (!p->allocBuffer())
        {
            delete p;
            return 0;
        }
        p->Index = index;

        Allocator.AddSegment(index << MeshCache_AddressToIndexShift, size >> MeshCache_AllocatorUnitShift);
        TotalSize += size;

        if (index == Buffers.GetSize())
            Buffers.PushBack(p);
        else
            Buffers[index] = p;

        return p;

    }

    // Destroys all buffers of specified type; all types are destroyed if AT_None is passed.
    void        DestroyBuffers(AllocType type = MeshBuffer::AT_None, bool lost = false)
    {        
        SF_UNUSED(lost);
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

    inline bool         CheckAllocationSize(UPInt size, const char* bufferType)
    {
        SF_UNUSED(bufferType);
        UPInt largestSize = 0;
        for (UPInt i = 0; i< Buffers.GetSize(); i++)
        {
            if (!Buffers[i])
                continue;
            if (Buffers[i]->Size > size)
                return true;
            if (Buffers[i]->Size > largestSize)
                largestSize = Buffers[i]->Size;
        }
        SF_DEBUG_WARNING3(1, "Render %s buffer allocation failed. Size = %zd, MaxBuffer = %zd",
                          bufferType, size, largestSize);
        return false;
    }

    void        DestroyBuffer(MeshBuffer* pbuffer, bool lost = false, bool deleteBuffer = true)
    {
        Allocator.RemoveSegment(pbuffer->GetIndex() << MeshCache_AddressToIndexShift,
            SizeToAllocatorUnit(pbuffer->GetSize()));
        TotalSize -= pbuffer->GetSize();
        Buffers[pbuffer->GetIndex()] = 0;
        if (lost)
            pbuffer->Buffer = 0;
        if (deleteBuffer)
        delete pbuffer;
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


// GL mesh cache implementation with following characteristics:
//  - Multiple cache lists and allocators; one per Buffer.
//  - Relies on 'Extended Locks'.

class MeshCache : public Render::MeshCache
{      
public:
    // List of possible strategies used to update buffers within the mesh cache. They are listed in order of preference,
    // however, all system may not support all of these methods.
    enum BufferUpdateType
    {
        BufferUpdate_MapBufferUnsynchronized,   // Use glMapBufferRange, with no synchronization. glFenceSync is used to ensure consistency.
        BufferUpdate_ClientBuffers,             // Use client-memory buffers. Client memory is modified directly, and reissued to the GPU every draw.
        BufferUpdate_MapBuffer,                 // Use glMapBuffer to update buffers. VBOs are used, without client backing. GPU may stall.
        BufferUpdate_UpdateBuffer,              // Use glBuffer[Sub]Data to update the buffers. VBOs are used, with client memory backing. GPU may stall.
        BufferUpdate_Count
    };

protected:
    
    friend class HAL;    

    enum {
        MinSupportedGranularity = 16*1024
    };

    HAL *                       pHal;
    MeshCacheListSet            CacheList;

    // Allocators managing the buffers. 
    MeshBufferSet               VertexBuffers, IndexBuffers;
    bool                        UseSeparateIndexBuffers;
    BufferUpdateType            BufferUpdate;

    // Mapped buffer into.
    bool                        Mapped;
    UPInt                       VBSizeEvictedInMap;
    MeshBuffer::MapList         MappedBuffers;
    // A list of all buffers in the order they were allocated. Used to allow
    // freeing them in the opposite order (to support proper IB/VB balancing).
    // NOTE: Must use base MeshBuffer type for List<> to work with internal casts.
    List<Render::MeshBuffer>    ChunkBuffers;
    // If a MeshBuffer could not be immediately destroyed (the GPU is using one of its meshes)
    // It is temporarily stored in this list until it is no longer in use.
    List<Render::MeshBuffer>    PendingDestructionBuffers;

    GLuint                      MaskEraseBatchVertexBuffer;
    GLuint                      MaskEraseBatchVAO;

    inline MeshCache* getThis() { return this; }
    inline HAL* GetHAL() const { return pHal; }

    inline UPInt    getTotalSize() const
    {
        return VertexBuffers.GetTotalSize() + IndexBuffers.GetTotalSize();
    }

    bool            createStaticVertexBuffers();
    bool            createInstancingVertexBuffer();
    bool            createMaskEraseBatchVertexBuffer();

    // Allocates Vertex/Index buffer of specified size and adds it to free list.
    bool            allocCacheBuffers(UPInt size, MeshBuffer::AllocType type, unsigned arena = 0);

    bool            evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                        MeshBuffer* pbuffer);

    // Allocates a number of bytes in the specified buffer, while evicting LRU data.
    // Buffer can contain either vertex and/or index data.
    bool            allocBuffer(UPInt* poffset, MeshBuffer** pbuffer,
                                MeshBufferSet& mbs, UPInt size, bool waitForCache);
    
    void            destroyBuffers(MeshBuffer::AllocType at = MeshBuffer::AT_None, bool lost = false);

    // adjustMeshCacheParams can be called with either valid or null device.
    // Valid device is specified once MeshCache is initialized.
    void            adjustMeshCacheParams(MeshCacheParams* p); 

    // If buffers could not be deleted immediately (their meshes were still in use), they are stored
    // in PendingDestructionBuffers and destroyed in this function (if possible).
    void            destroyPendingBuffers();

public:

    
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params);    
    ~MeshCache();

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(HAL* phal);
    // Resets MeshCache, releasing all buffers.
    void            Reset(bool lost = false);    

    virtual QueueMode GetQueueMode() const { return QM_ExtendLocks; }

    // *** MeshCacheConfig Interface    
    virtual void    ClearCache();
    virtual bool    SetParams(const MeshCacheParams& params);    

    virtual void    EndFrame();      

    // Adds a fixed-size buffer to cache reserve; expected to be released at Release.
    //virtual bool    AddReserveBuffer(unsigned size, unsigned arena = 0);
    //virtual bool    ReleaseReserveBuffer(unsigned size, unsigned arena = 0);

    virtual bool    LockBuffers();
    virtual void    UnlockBuffers();
    virtual bool    AreBuffersMapped() const { return Mapped; }

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

    virtual void GetStats(Stats* stats);
    
    virtual BufferUpdateType GetBufferUpdateType() const { return BufferUpdate; }
};


}}};  // namespace Scaleform::Render::GL

#endif
