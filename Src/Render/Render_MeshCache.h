/**************************************************************************

PublicHeader:   Render
Filename    :   Render_MeshCache.h
Content     :   New renderer Mesh Cache implementation
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_MeshCache_H
#define INC_SF_Render_MeshCache_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_Hash.h"
#include "Render/Render_Primitive.h"
#include "Render/Render_MeshCacheConfig.h"
#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render {

class MeshCacheItem;
class MeshCache;
class MeshCacheListSet;

typedef UInt16      IndexType;

// Identifies the type of cache the item is in.
enum MeshCacheListType
{
    MCL_Uncached,           // The item isn't cached yet.    
    MCL_InFlight,           // The item is queued up for rendering and not complete yet.
    MCL_ThisFrame,          // The item is used in the current frame.
    MCL_PrevFrame,          // The item was used in previous frame. During frame rendering,
                            // part of frame content will come from MCL_PrevFrame.
                            // TBD: It may make sense to apply MRU to this MCL_PrevFrame.
    MCL_LRUTail,
    MCL_PendingFree,      // The item is queued for deletion, but was in use.

    // Total number of cache lists
    MCL_ItemCount
};


// MeshCacheItem represents a chunk of data cached in VB/IB buffers of the MeshCache.
// This can be either a set of consecutive meshes describing a PrimitiveBatch, or a single
// mesh used during instancing.
//  - MeshCacheItem items are shuffled based on the caching strategy.
//  - Allocated with GHEAP_ALLOC due to variable size.
//  - The combination of meshes stored here is used as a hash table key.

class MeshCacheItem : public ListNode<MeshCacheItem>
{
    friend class MeshCacheListSet;

    MeshCacheListSet*   pCacheList;    
    MeshCacheListType   ListType;

    inline MeshCache* GetCache() const;

public:
    
    enum MeshType
    {
        Mesh_Regular,
        Mesh_Complex,
        Mesh_Destroyed,
    };

    MeshType   Type;

    // A list of PrimitiveBatch that use us; there can be multiple due to sharing.
    //  We need this list so that we can clear their pointers to us when MeshCacheItem
    //  is swapped out. 
    List<MeshCacheItemUseNode> PrimitiveBatches;    

    // Hash table data, used for hashing this batch.
    unsigned   HashKey;
    unsigned   MeshCount;
    MeshBase** pMeshes; // Meshes start past the end of the buffer.

    // A range of Vertices in cache.
    UPInt       AllocSize;
    unsigned    VertexCount;
    unsigned    IndexCount;
    // Fence is used on consoles for synchronization.
    Ptr<Fence>  GPUFence;

    
    // *** Create/Destroy helpers

    // MeshCacheItem doesn't have a constructor as it is allocated together
    // with its MeshData* structure; instead Create takes 'classSize', which should
    // be a size of a derived class.
    struct MeshBaseContent;
    static MeshCacheItem*  Create(MeshType type,
                                  MeshCacheListSet* pcacheList, UPInt classSize,
                                  MeshBaseContent& content, UPInt allocSize,
                                  unsigned vertexCount, unsigned indexCount);

    // Removes all references to us from MeshCache and PrimitiveBatch objects and frees
    // batch memory. Exception: Doesn't try to remove reference from our pskipMesh. 
    void                    Destroy(MeshBase* pskipMesh = 0, bool freeMemory = true);


    // Find MeshCacheItem from which mesh data can be copied; used on consoles where
    // video memory is readable.
    static MeshCacheItem* FindMeshSourceBatch(Mesh *pmesh,
                                              unsigned *pprevVertexCount,
                                              unsigned *pprevIndexCount,
                                              MeshCacheItem* pskipBatch);

    // Informs us that a mesh containing us is deleted. This call
    // results in this MeshCacheItem also being deleted.
    //  - pmesh specifies the mesh that was deleted, so that we
    //    don't try removing us from its list a second time.
    void            NotifyMeshRelease(MeshBase* pmesh);

    inline void     MoveToCacheListFront(MeshCacheListType list);

    MeshUseStatus   GetUseStatus() const;

    inline MeshCacheListType GetListType() const { return ListType; }


    // *** Hashing Support

    // Back-end specific data structure goes here.    
    static UPInt    CalcHashKey(MeshBase** pmeshes, unsigned meshCount)
    {
        UPInt i, key = 0;
        for (i = 0; i< meshCount; i++)
            key ^= ((UPInt)pmeshes[i]) >> 5;
        return key;
    }
    static UPInt    CalcHashKey(const StrideArray<MeshBase*>& meshes)
    {
        UPInt i, key = 0;
        for (i = 0; i< meshes.GetSize(); i++)
            key ^= ((UPInt)meshes[i]) >> 5;
        return key;
    }


    struct MeshBaseContent
    {
        StrideArray<MeshBase*> Meshes;
        unsigned   HashKey;

        MeshBaseContent(MeshBase** pmeshes, unsigned meshCount)
          : Meshes(pmeshes, meshCount),
            HashKey((unsigned)MeshCacheItem::CalcHashKey(pmeshes, meshCount))
        { }
        MeshBaseContent(const StrideArray<MeshBase*>& meshes)
          : Meshes(meshes),
            HashKey((unsigned)MeshCacheItem::CalcHashKey(meshes))
        { }

        unsigned  GetMeshCount() const          { return (unsigned) Meshes.GetSize(); }
        MeshBase* operator[] (unsigned i) const { return Meshes[i]; }
    };

    struct MeshContent : public MeshBaseContent
    {
        MeshContent(Mesh** pmeshes, unsigned meshCount)
          : MeshBaseContent((MeshBase**)pmeshes, meshCount)
        { }
        MeshContent(PrimitiveBatch* pbatch)
          : MeshBaseContent(pbatch->GetMeshes_InstanceAdjusted())
        { }        

        Mesh* operator[] (unsigned i) const { return (Mesh*)Meshes[i]; }

        bool IsLargeMesh() const { return  GetMeshCount() && (*this)[0]->LargeMesh; }
    };

    class HashFunctor
    {
    public:
        UPInt operator()(MeshCacheItem* pdata) const { return pdata->HashKey; }
        UPInt operator()(const MeshBaseContent &c) const { return c.HashKey; }
    };

    typedef MeshBaseContent AltLookupKey;

    inline bool    Equals(const StrideArray<MeshBase*>& meshes)
    {
        if (MeshCount != (unsigned)meshes.GetSize())
            return false;
        for (unsigned i = 0; i < MeshCount; i++)
            if (pMeshes[i] != meshes[i])
                return false;
        return true;
    }

    // Location is system memory cache.
    // Initialization data for the array of matrices that use us.

};


inline bool operator == (MeshCacheItem* pmeshData, const MeshCacheItem::AltLookupKey& key)
{
    return pmeshData->Equals(key.Meshes);
}



// MeshCacheListSet is a set of linked lists used to enable LRU/MRU caching.
// The set has fixed-designation list slots identified by named constants,
// such as MCL_ThisFrame and PreviousFrame slots, which enable tracking
// of meshes that were used in previous frames.

class  MeshCacheListSet
{
public:
    struct ListSlot : public List<MeshCacheItem>
    {
        // Total AllocSize of all entries in the slot.
        UPInt Size; 
        ListSlot() : Size(0) { }
    };

private:
    MeshCache*   pCache;
    ListSlot     Slots[MCL_ItemCount];

public:
    MeshCacheListSet(MeshCache* pcache) : pCache(pcache) { }
    ~MeshCacheListSet() { }
    
    
    void    PushFront(MeshCacheListType type, MeshCacheItem* p)
    {
        p->ListType = type;
        Slots[type].PushFront(p);
        Slots[type].Size += p->AllocSize;
    }
    void    RemoveNode(MeshCacheItem* p)
    { 
        p->RemoveNode();
        Slots[p->ListType].Size -= p->AllocSize; 
    }
    void    PushListToFront(MeshCacheListType to, MeshCacheListType from)
    {
        ListSlot&      src = Slots[from];
        MeshCacheItem* p = src.GetFirst();
        while (!src.IsNull(p))
        {
            p->ListType = to;
            p = p->pNext;
        }
        Slots[to].PushListToFront(src);
        Slots[to].Size += src.Size;
        src.Size = 0;
    }

    inline MeshCache* GetCache() const                         { return pCache; }
    inline ListSlot& GetSlot(MeshCacheListType type)           { return Slots[type]; }
    inline ListSlot* GetSlots()                                { return Slots; }
    inline UPInt     GetSlotSize(MeshCacheListType type) const { return Slots[type].Size; }
  
    // Evicts items in list until block of 'size' becomes available.
    // Returns true if succeeded, false if no such block resulted.
    bool    EvictLRU(ListSlot& list, AllocAddr& a, UPInt size);
    bool    EvictLRUTillLimit(ListSlot& list, AllocAddr& a, UPInt size, UPInt limit);
    bool    EvictPendingFree(AllocAddr& a);
    
    bool    EvictLRU(MeshCacheListType ltype, AllocAddr& a, UPInt size)
    {
        // Cannot call this function on MCL_PendingFree, as it may requeue items.
        SF_ASSERT(ltype != MCL_PendingFree && ltype < MCL_ItemCount);
        return EvictLRU(Slots[ltype], a, size);
    }
    bool    EvictLRUTillLimit(MeshCacheListType ltype, AllocAddr& a, UPInt size, UPInt limit)
    {
        // Cannot call this function on MCL_PendingFree, as it may requeue items.
        SF_ASSERT(ltype != MCL_PendingFree && ltype < MCL_ItemCount);
        return EvictLRUTillLimit(Slots[ltype], a, size, limit);
    }
      
    void    EvictAll();

    // Finalized the frame by moving MCL_ThisFrame items to PrevFrames
    // and then the LRU. PrevFrames contents are properly shifted.
    void    EndFrame();
};

inline MeshCache* MeshCacheItem::GetCache() const
{
    return pCacheList->GetCache();
}
inline void       MeshCacheItem::MoveToCacheListFront(MeshCacheListType list)
{ 
    pCacheList->RemoveNode(this);
    pCacheList->PushFront(list, this);
    //RemoveNode();
    //pCacheList->Slots[list].PushFront(this);
}



// Mesh Staging Buffer acts as a short-term cache that stores generate meshes
// before they are instanced and copied into the Vertex/Index buffers. Typically
// the staging buffer will accumulate all of the meshes in the Batch before they
// are uploaded into the VBs.
//
//   - Right now, it is implemented as a simple circular buffer; this is fast
//     for updates but not ideal for swapping. We could potentially implement
//     it as an LRU in the future(?).
//
//   - PCs have a large staging buffer; consoles, however, can get away with a
//     smaller one since they can copy mesh data directly from the primitive instead.
//
//   - Meshes in the buffer can be pinned to ensure they are not swapped out.
//     There is a limit on the total size of pinned items, which should correspond
//     to the maximum total mesh size for the largest allowed batch in BatchProcessor.


class MeshStagingBuffer
{
    UByte*  pBuffer;
    UPInt   BufferSize;
    UPInt   TotalPinnedSize;
    // Should depend on maximum allowed batch
    UPInt   PinSizeLimit;

    // A linked list of meshes in buffer that is rotated as meshes are brought in/swapped out.
    // The first item in the list is head (most recently allocated), while the
    // last item is tail (least recently allocated).
    List<MeshStagingNode>  MeshList;

    // Allocated space in the circular buffer, while potentially swapping out
    // unpinned meshes.
    bool    AllocateBufferSpace(UPInt* paddr, UPInt size);


public:
    MeshStagingBuffer();
    ~MeshStagingBuffer();

    bool    Initialize(MemoryHeap* pheap, UPInt size);
    void    Reset();

    UByte*  GetBuffer() const { return pBuffer; }
    UPInt   GetBufferSize() const { return BufferSize; }
    
    // Determines that the mesh in
    bool    IsCached(MeshBase* pmesh) { return pmesh->StagingBufferSize != 0; }

    // Pins a mesh so that it cannot be swapped out by future Generate call.
    // Typically all meshes in a batch are pinned before they are uploaded
    // to the vertex/index buffers.
    void    PinMesh(MeshBase* pmesh)
    {
        SF_ASSERT(IsCached(pmesh));
        if (!pmesh->PinCount)
            TotalPinnedSize += pmesh->StagingBufferSize;
        pmesh->PinCount++;
    }
    void    UnpinMesh(MeshBase* pmesh)
    {
        SF_ASSERT(pmesh->PinCount != 0);
        pmesh->PinCount--;
        if (!pmesh->PinCount)
            TotalPinnedSize -= pmesh->StagingBufferSize;
    }

    // Allocates buffer space for mesh and initializes it,
    bool    AllocateMesh(Mesh *pmesh, UPInt vertexCount, UPInt vertexSize,
                                      UPInt indexCount);

};


// MeshBuffer - Base class for buffer implementations on different platforms.
// Mesh cache grows and shrinks by allocating and freeing MeshBuffer objects, 
// each buffer will typically have enough space for many meshes.

class MeshBuffer : public ListNode<MeshBuffer>, public NewOverrideBase<Stat_Default_Mem>
{
public:
    enum AllocType
    {
        AT_None,
        AT_Reserve,
        AT_Chunk,
        AT_Direct
    };

    MeshBuffer(UPInt size, AllocType type, unsigned arena)
        : Arena(arena), Type(type), Size(size), pData(0)
    { }
    virtual ~MeshBuffer() { }

    inline  AllocType GetType() const { return Type; }
    inline  UPInt     GetSize() const { return Size; }
    inline  unsigned  GetArena() const { return Arena; }

//protected:
    unsigned    Arena;
    AllocType   Type;
    UPInt       Size;
    // Pointer to the start of the buffer. On platforms that require mapping/locking,
    // such as D3D9, this pointer may be null until mapped.
    void*       pData;
};



// MeshCache maintains hardware Vertex and Index buffers cached
// objects lists, providing an API used to manage those.

class MeshCache : public CacheBase, public MeshCacheConfig
{
public:
    typedef HashSetLH<MeshCacheItem*,
                      MeshCacheItem::HashFunctor> CacheItemHashType;    
   
    MemoryHeap*         pHeap;    
    RQCacheInterface*   pRQCaches;
    MeshCacheParams     Params;

    MeshStagingBuffer   StagingBuffer;  
    CacheItemHashType   BatchCacheItemHash;

    SF_AMP_CODE(UInt32 Thrashing;)
    
    // QueueMode reported by GetQueueMode, reports the type of queue handling
    // expected by MeshCache from the RenderQueueProcessor. There are two modes
    // handled distinctly differently:
    // 
    //  QM_WaitForFences - This mode, applicable on consoles, does not perform
    //                     Lock/UnlockBuffers operations on the MeshCache and instead
    //                     relies on HW fences to know when old meshes can be swapped
    //                     out. If swapping candidates fences are not passed, 
    //                     commands should be queued up instead.
    //
    //  QM_ExtendLocks   - Doesn't use fences and instead requires LockBuffers calls
    //                     to upload data. For optimization, this mode tries to minimize
    //                     locks by extending each one for as long as possible and
    //                     queuing up data for as long as the lock is held. This is
    //                     is used with D3D9.
    enum QueueMode
    {
        QM_WaitForFences,
        QM_ExtendLocks
    };


    // StagingBufferPrep prepares the argument content in the staging buffer
    // and Pins it there until destructor is called.
    class StagingBufferPrep
    {
        MeshCache*                   pCache;
        MeshCacheItem::MeshContent&  MC;
        bool                         CanCopyData;   // True, if the system is capable of copying vertex/index data from existing MeshCacheItems.
        bool                         PinMeshes;     // True, if the GenerateMeshes call should pin the meshes in the staging buffer (set to false after first iteration).
        const VertexFormat*          pVertexFormat; // The required vertex format of the data to be prepared.

        // PinedFlagArray is used to coordinate the number of Pins/Unpins; this is
        // necessary in case multiple identical meshes are in the list.
        // TBD: Instead of maintaining array, we could just modify UnpinMesh() to clear
        // out PinCount to 0 instead of decrementing; however, that may not allow
        // for more complex usage patterns in the future...
        bool                       PinnedFlagArray[256];

        // Avoid warnings.
        StagingBufferPrep(const StagingBufferPrep& s)
            : pCache(s.pCache), MC(s.MC), CanCopyData(s.CanCopyData), PinMeshes(s.PinMeshes), pVertexFormat(s.pVertexFormat) { }
        void operator = (StagingBufferPrep&) { SF_ASSERT(0); }

    public:

        StagingBufferPrep(MeshCache* cache,
                          MeshCacheItem::MeshContent &mc,
                          const VertexFormat* format,
                          bool canCopyData);

        // This is called to actually generate the meshes, or alternatively copy them from an existing buffer.
        // It is called both before (by the constructor) and after the new MeshCacheItem is allocated, because 
		// this allocation may cause eviction of the previous buffer (and then it cannot be copied from). 
        void GenerateMeshes(MeshCacheItem * skipBatch = 0);

        ~StagingBufferPrep();
    };


    MeshCache(MemoryHeap* pheap,
              const MeshCacheParams& params);
    virtual ~MeshCache();

    // Set cache interface used by the rendering queue.
    void  SetRQCacheInterface(RQCacheInterface* rqCaches);

    virtual QueueMode GetQueueMode() const { return QM_WaitForFences; }

    virtual void    BeginFrame() { };

    // Tells cache that the frame has ended. Causes cache eviction queue update.
    virtual void    EndFrame()      = 0;

    // MeshCacheConfig - partial implementation.
    const MeshCacheParams& GetParams() const { return Params; }

    // Adds a fixed-size buffer to cache reserve; expected to be released at Release.
    //virtual bool    AddReserveBuffer(unsigned size, unsigned arena = 0) = 0;
    //virtual bool    ReleaseReserveBuffer(unsigned size, unsigned arena = 0) = 0;

    // Locking APIs are used only with QM_ExtendLocks.
    virtual bool      LockBuffers() { return false; }
    virtual void      UnlockBuffers() { }
    virtual bool      AreBuffersLocked() const { return false; }

    virtual MeshUseStatus GetItemUseStatus(const MeshCacheItem* item);

    // Moves MeshCacheItem to the front of the specified cache list. Ignores
    // calls with null argument.
    void            MoveToCacheListFront(MeshCacheListType list, MeshCacheItem* p)
    {        
        if (p) { p->MoveToCacheListFront(list); }
    }

    bool            GetHashPrimitive(PrimitiveBatch* pbatch, MeshCacheItem::MeshContent &mc)
    {
        // a) Try to lookup mesh combination in cache. 
        //    If this combination of meshes was already used, nothing else to do.
        MeshCacheItem* pmesh;
        if (BatchCacheItemHash.GetAlt(mc, &pmesh))
        {
            pbatch->SetCacheItem(pmesh);
            return true;
        }
        return false;
    }


    // Evicts MeshCacheItem; used by MeshCacheListSet::Evict functions.
    // Evicts MeshCacheItem from mesh/index caches and frees it. Returns the resulting free
    // block size within the specified allocator. If 'pmesh' argument is provided, that
    // meshes list of batched is not touched (it assumed to be cleared externally).
    // Limitation: required AllocAddr.... meaning that all mesh caches should use it.
    virtual UPInt   Evict(MeshCacheItem* p, AllocAddr* pallocator = 0, MeshBase* pskipMesh = 0) = 0; 

    // Generates meshes and uploads them to buffers.
    // Returns 'false' if there is not enough space in Cache, so Unlock and
    // flush needs to take place.
    //   - Pass 'false' for firstCall is PreparePrimitive was already called once
    //     with this data and failed.
    virtual bool    PreparePrimitive(PrimitiveBatch* pbatch,
                                     MeshCacheItem::MeshContent &mc, bool waitForCache) = 0;

    virtual bool    SetLargeMeshVertices(MeshCacheItem* pcacheItem,
                                         const VertexFormat* pSourceFormat, UPInt vertexOffset,
                                         UByte* pvertices, unsigned vertexCount,
                                         const VertexFormat* pDestFormat, UByte* pdest);
    virtual bool    SetLargeMeshIndices( MeshCacheItem* pcacheItem,
                                         const VertexFormat* pSourceFormat, UPInt indexOffset,
                                         const UInt16* pindices, unsigned indexCount,
                                         const VertexFormat* pDestFormat, UByte* pdestIndex);

    struct MeshResult
    {
        enum ResultType
        {
            Success_Staging          = 0x0,
            Success_LargeMesh        = 0x1,
            Fail_Staging_NoBuffer    = 0x2,
            Fail_LargeMesh_NeedCache = 0x3,
            Fail_LargeMesh_TooBig    = 0x5,
            Fail_LargeMesh_ThisFrame = 0x7, // Should try again in future frames.
            Fail_General             = 0x6
        };

        MeshResult(ResultType val) : Value(val) { }
        MeshResult(const MeshResult& arg) : Value(arg.Value) { }

        bool operator == (const MeshResult& other) { return Value == other.Value; }
        bool operator != (const MeshResult& other) { return Value != other.Value; }
        bool operator == (ResultType val)          { return Value == val; }
        bool operator != (ResultType val)          { return Value != val; }

        bool IsLargeMesh() const    { return (Value & Success_LargeMesh) != 0; }
        bool Succeded() const       { return Value <= Success_LargeMesh; }

        ResultType Value;
    };

    // Generates mesh data, placing it either is StagingBuffer(for smaller meshes)
    // or directly into cache (large mesh).
    // - If mesh is known to fit in staging buffer, singleFormat and singleFormat 
    //   may be null.
    // - When re-generating large mesh that is tagged so, batchFormat may be null.
    MeshResult       GenerateMesh(Mesh* mesh,
                                  const VertexFormat* sourceFormat,
                                  const VertexFormat* singleFormat,
                                  const VertexFormat* batchFormat,
                                  bool waitForCache);

    bool            PrepareComplexMesh(ComplexMesh* pprim, bool waitForCache);


    enum AllocResult
    {
        Alloc_Fail,         // Allocation failed - need more cache space.
        Alloc_Fail_TooBig,  // Failed due to excessively big allocation size.
        Alloc_Fail_ThisFrame, // Allocation failed in this frame, but may succeed in the future;
                              // this is useful for caches that need to keep several frames alive.
        Alloc_Success,      // Allocation succeeded.
        Alloc_StateError    // Buffers were in wrong state, or other/system mem error.
    };    

    // Allocates back-end specific MeshCacheItem and its buffers.
    // Used for PrepareComplexMesh, but also potentially by internal PreparePrimitive.    
    virtual AllocResult AllocCacheItem(MeshCacheItem** pdata,
                                      UByte** pvertexDataStart, IndexType** pindexDataStart,
                                      MeshCacheItem::MeshType meshType,
                                      MeshCacheItem::MeshBaseContent &mc,
                                      UPInt vertexBufferSize,
                                      unsigned vertexCount, unsigned indexCount,
                                      bool waitForCache,
                                      const VertexFormat* pDestFormat) = 0;

	// Called after a mesh has been written. On some platforms (such as X360), there is
	// some extra calls that need to be made to signal that the GPU can safely use the mesh.
	virtual void PostUpdateMesh(MeshCacheItem * pcacheItem ) { SF_UNUSED(pcacheItem); };


    // Stats
    enum StatType
    {
        MeshBuffer_Common = 0,
        MeshBuffer_Vertex = 1,
        MeshBuffer_Index  = 2,
        MeshBuffer_GpuMem = 4,  // On non console platforms, this means it was allocated in the driver, so it could be any kind of memory.

        MeshBuffer_StatCount  = 1+MeshBuffer_GpuMem+MeshBuffer_Index,
    };
    struct Stats
    {
        UPInt TotalSize[MeshBuffer_StatCount];
        UPInt UsedSize[MeshBuffer_StatCount];

        Stats()
        {
            for (int i = 0; i < MeshBuffer_StatCount; i++)
                TotalSize[i] = UsedSize[i] = 0;
        }

        inline UPInt GetTotal(int stat) const
        {
            return TotalSize[stat] + TotalSize[stat + MeshBuffer_GpuMem];
        }
        inline UPInt GetUsed(int stat) const
        {
            return UsedSize[stat] + UsedSize[stat + MeshBuffer_GpuMem];
        }
    };

    virtual void GetStats(Stats*) {};

protected:
    template<class VFormat>
    static void fillMaskEraseVertexBuffer(VFormat* buffer, unsigned count);
};

template<class VFormat>
inline void MeshCache::fillMaskEraseVertexBuffer(VFormat* pbuffer, unsigned count)
{
    for(unsigned i = 0; i< count; i++)
    {
        // Set all four components of the alpha to the batch index. This way,
        // we don't need to worry about swizzling in the VB/shader.
        for ( UByte j =0; j < 6; j++)
        {
            for (UByte k = 0; k < 4; k++)
                pbuffer[i * 6 + j].Alpha[k] = (UByte)i;
        }

        pbuffer[i * 6 + 0].x  = 0;
        pbuffer[i * 6 + 0].y  = 1;
        pbuffer[i * 6 + 1].x  = 0;
        pbuffer[i * 6 + 1].y  = 0;
        pbuffer[i * 6 + 2].x  = 1;
        pbuffer[i * 6 + 2].y  = 0;

        pbuffer[i * 6 + 3].x  = 0;
        pbuffer[i * 6 + 3].y  = 1;
        pbuffer[i * 6 + 4].x  = 1;
        pbuffer[i * 6 + 4].y  = 0;
        pbuffer[i * 6 + 5].x  = 1;
        pbuffer[i * 6 + 5].y  = 1;
    }
}

}};  // namespace Scaleform::Render

#endif

