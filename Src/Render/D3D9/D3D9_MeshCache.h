/**************************************************************************

Filename    :   D3D9_MeshCache.h
Content     :   D3D9 Mesh Cache implementation
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_D3D9_MeshCache_H
#define INC_SF_Render_D3D9_MeshCache_H

#include "Render/Render_MeshCache.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/D3D9/D3D9_Sync.h"
#include <d3d9.h>


#define SF_RENDER_D3D9_INDEX_FMT         D3DFMT_INDEX16

// DirectX compatibility defines
#define IDirect3DDeviceX        IDirect3DDevice9
#define IDirect3DTextureX       IDirect3DTexture9
#define IDirect3DSurfaceX       IDirect3DSurface9
#define IDirect3DIndexBufferX   IDirect3DIndexBuffer9
#define IDirect3DVertexBufferX  IDirect3DVertexBuffer9
#define D3DCAPSx                D3DCAPS9
#define D3DVIEWPORTx            D3DVIEWPORT9
#define LPDIRECT3Dx             LPDIRECT3D9
#define LPDIRECT3DDEVICEx       LPDIRECT3DDEVICE9


namespace Scaleform { namespace Render { namespace D3D9 {

class MeshBuffer;
class MeshBufferSet;
class VertexBuffer;
class IndexBuffer;
class MeshCache;
class HAL;


// D3D9 version of MeshCacheItem. In D3D9 index and vertex buffers
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
        MeshCacheItem* p = (D3D9::MeshCacheItem*)
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

enum MeshBufferCreateFlags
{
    Buffer_Dynamic  = 0x1,
};

class MeshBuffer : public Render::MeshBuffer
{
protected:    
    UPInt       Index;      // Index in MeshBufferSet::Buffers array.
    MeshBuffer* pNextLock;
    unsigned    Flags;      // Flags, see MeshBufferCreateFlags

public:

    enum BufferType
    {
        Buffer_Vertex,
        Buffer_Index,
    };

    MeshBuffer(UPInt size, AllocType type, unsigned arena, unsigned flags = 0)
        : Render::MeshBuffer(size, type, arena), Flags(flags)
    { }
    virtual ~MeshBuffer() { }
    
    inline  UPInt   GetIndex() const { return Index; }
    
    virtual bool    allocBuffer(IDirect3DDeviceX* pdevice) = 0;
    virtual bool    DoLock() = 0;
    virtual void    Unlock() = 0;    
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
        void UnlockAll()
        {
            MeshBuffer* p = pFirst;
            while(p)
            {
                p->Unlock();
                p = p->pNextLock;
            }
            pFirst = 0;
        }
    };

    inline  UByte*  Lock(LockList& lockedBuffers)
    {
        if (!pData)
        {  
            if (!DoLock())
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
                                     MemoryHeap* pheap, IDirect3DDeviceX* pdevice,
                                     unsigned flags) = 0;

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
        SF_DEBUG_WARNING3(1, "Render %s buffer allocation failed. Size = %d, MaxBuffer = %d",
                         bufferType, size, largestSize);
        return false;
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
                                     MemoryHeap* pheap, IDirect3DDeviceX* pdevice,
                                     unsigned flags)
    {
        // Single buffer can't be bigger then (1<<MeshCache_AddressToIndexShift)
        // size due to the way addresses are masked.
        SF_ASSERT(size <= (1<<(MeshCache_AddressToIndexShift+MeshCache_AllocatorUnitShift)));
        return  Buffer::Create(size, type, arena, pheap, pdevice, flags, *this);
    }
};


template<class BType, class Derived>
class MeshBufferImpl : public MeshBuffer
{
protected:
    typedef MeshBufferImpl  Base;
    Ptr<BType>              pBuffer;
public:

    MeshBufferImpl(UPInt size, AllocType type, unsigned arena, unsigned flags)
        : MeshBuffer(size, type, arena, flags)
    { }

    inline BType* GetHWBuffer() const { return pBuffer.GetPtr(); }

    bool   DoLock()
    {
        SF_ASSERT(!pData);
        DWORD lockFlags = (Flags & Buffer_Dynamic) ? D3DLOCK_NOOVERWRITE : 0;
        return (!FAILED(pBuffer->Lock(0, (UINT)Size, &pData, lockFlags)));
    }
    void    Unlock()
    {
        if (pData)
        {
            pBuffer->Unlock();
            pData = 0;
        }
    }

    static Derived* Create(UPInt size, AllocType type, unsigned arena,
                           MemoryHeap* pheap, IDirect3DDeviceX* pdevice,
                           unsigned flags, MeshBufferSet& mbs)
    {
        // Determine which address index we'll use in the allocator.
        UPInt index = 0;
        while ((index < mbs.Buffers.GetSize()) && mbs.Buffers[index])
            index++;
        if (index == MeshCache_MaxBufferCount)
            return 0;

        // Round-up to Allocator unit.
        size = ((size + MeshCache_AllocatorUnit-1) >> MeshCache_AllocatorUnitShift) << MeshCache_AllocatorUnitShift;

        Derived* p = SF_HEAP_NEW(pheap) Derived(size, type, arena, flags);
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

class VertexBuffer : public MeshBufferImpl<IDirect3DVertexBufferX, VertexBuffer>
{
public:
    VertexBuffer(UPInt size, AllocType atype, unsigned arena, unsigned flags)
        : Base(size, atype, arena, flags)
    { }

    virtual bool allocBuffer(IDirect3DDeviceX* pdevice)
    {
        unsigned usage = D3DUSAGE_WRITEONLY | ((Flags & Buffer_Dynamic) ? D3DUSAGE_DYNAMIC : 0);
        return pdevice->CreateVertexBuffer((UINT)Size, usage, 0, D3DPOOL_DEFAULT, &pBuffer.GetRawRef(), 0) == D3D_OK;
    }
    virtual BufferType GetBufferType() const { return Buffer_Vertex; }
};

class IndexBuffer : public MeshBufferImpl<IDirect3DIndexBufferX, IndexBuffer>
{
public:
    IndexBuffer(UPInt size, AllocType atype, unsigned arena, unsigned flags)
        : Base(size, atype, arena, flags)
    { }

    virtual bool allocBuffer(IDirect3DDeviceX* pdevice)
    {
        unsigned usage = D3DUSAGE_WRITEONLY | ((Flags & Buffer_Dynamic) ? D3DUSAGE_DYNAMIC : 0);
        return pdevice->CreateIndexBuffer((UINT)Size, usage, SF_RENDER_D3D9_INDEX_FMT,
            D3DPOOL_DEFAULT, &pBuffer.GetRawRef(), 0) == D3D_OK;
    }
    virtual BufferType GetBufferType() const { return Buffer_Index; }
};

typedef MeshBufferSetImpl<VertexBuffer> VertexBufferSet;
typedef MeshBufferSetImpl<IndexBuffer>  IndexBufferSet;



// D3D9 mesh cache implementation with following characteristics:
//  - Multiple cache lists and allocators; one per Buffer.
//  - Relies on 'Extended Locks'.

class MeshCache : public Render::MeshCache
{      
    friend class HAL;    
    
    enum {
        MinSupportedGranularity = 16*1024
    };

    Ptr<IDirect3DDeviceX>       pDevice;    
    MeshCacheListSet            CacheList;

    unsigned                    BufferCreateFlags;

    // Allocators managing the buffers. 
    VertexBufferSet             VertexBuffers;
    IndexBufferSet              IndexBuffers;
       
    // Locked buffer into.
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

    // Vertex buffer used for instancing. Right now it is simply
    // allocated to have constant values.
    Ptr<IDirect3DVertexBufferX> pInstancingVertexBuffer;
    Ptr<IDirect3DVertexBufferX> pMaskEraseBatchVertexBuffer;
    
    inline MeshCache* getThis() { return this; }

    inline UPInt    getTotalSize() const
    {
        return VertexBuffers.GetTotalSize() + IndexBuffers.GetTotalSize();
    }
   
    bool            createStaticVertexBuffers(IDirect3DDeviceX* pdevice);
    bool            createInstancingVertexBuffer(IDirect3DDeviceX* pdevice);
    bool            createMaskEraseBatchVertexBuffer(IDirect3DDeviceX* pdevice);

    // Allocates Vertex/Index buffer of specified size and adds it to free list.
    bool            allocCacheBuffers(UPInt size, MeshBuffer::AllocType type, unsigned arena = 0);

    bool            evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                        MeshBuffer* pbuffer);

    // Allocates a number of bytes in the specified buffer, while evicting LRU data.
    // Buffer can contain either vertex and/or index data.
    bool            allocBuffer(UPInt* poffset, MeshBuffer** pbuffer,
                                MeshBufferSet& mbs, UPInt size, bool waitForCache);

    void            destroyBuffers(MeshBuffer::AllocType at = MeshBuffer::AT_None);

    // adjustMeshCacheParams can be called with either valid or null device.
    // Valid device is specified once MeshCache is initialized.
    void            adjustMeshCacheParams(MeshCacheParams* p, IDirect3DDevice9* device = 0);

    // If buffers could not be deleted immediately (their meshes were still in use), they are stored
    // in PendingDestructionBuffers and destroyed in this function (if possible).
    void            destroyPendingBuffers();

public:

    MeshCache(MemoryHeap* pheap,
              const MeshCacheParams& params);
    ~MeshCache();    

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(IDirect3DDeviceX* pdevice, bool dynamicMeshes );
    // Resets MeshCache, releasing all buffers.
    void            Reset();    

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

    virtual void GetStats(Stats* stats);

    bool            UsesDynamicMeshes() { return (BufferCreateFlags & Buffer_Dynamic) != 0; }
};



// Vertex structures used by both MeshCache and HAL.

class Render_InstanceData
{
public:
    // Color format for instance, Alpha expected to have index.
    UInt32 Instance;
};


}}};  // namespace Scaleform::Render::D3D9

#endif
