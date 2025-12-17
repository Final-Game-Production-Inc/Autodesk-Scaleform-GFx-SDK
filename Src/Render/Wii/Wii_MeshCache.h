/**************************************************************************

Filename    :   Wii_MeshCache.h
Content     :   Wii Mesh Cache header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Wii_MeshCache_H
#define INC_SF_Render_Wii_MeshCache_H

#include "Render/Render_SimpleMeshCache.h"
#include "Render/Render_MemoryManager.h"
#include "Wii_Shader.h"
#include "Render/Wii/Wii_Sync.h"

#define SF_RENDER_WII_INSTANCE_MATRICES 8

namespace Scaleform { namespace Render { namespace Wii {

typedef SimpleMeshBuffer MeshBuffer;
class MeshCache;
class HAL;


class MeshCacheItem : public Render::MeshCacheItem
{
    friend class MeshCache;
    friend class HAL;

public:
    SimpleMeshBuffer* pBuffer;

    UPInt             AllocAddress, AllocSize;
    UPInt             VertexOffset;

    // Display list in buffer
    UPInt    DListOffset;
    UPInt    DListSize;
    UPInt    PrimOffset;         // Location of primitive data (relative to start of display list)

    // Plain indices for streaming
    UPInt    IndexOffset;

    const VertexFormat* pFormat;


    static MeshCacheItem* Create(MeshType type,
                                 MeshCacheListSet* pcacheList,
                                 MeshBaseContent& mc, SimpleMeshBuffer* pbuffer,
                                 UPInt allocAddress, UPInt allocSize,
                                 UPInt vertexOffset, unsigned vertexCount, unsigned indexCount,
                                 UPInt dlistOffset, UPInt dlistSize, const VertexFormat * pfmt )
    {
        MeshCacheItem* p = (MeshCacheItem*)
            Render::MeshCacheItem::Create(type, pcacheList, sizeof(MeshCacheItem),
                                          mc, allocSize, vertexCount, indexCount);
        if (p)
        {
            p->pBuffer      = pbuffer;
            p->AllocAddress = allocAddress;
            p->AllocSize    = allocSize;
            p->VertexOffset = vertexOffset;
            p->DListOffset  = dlistOffset;
            p->DListSize    = dlistSize;
            p->pFormat      = pfmt;
            p->IndexOffset  = 0xffffffff;
        }
        return p;
    }

    static MeshCacheItem* Create(MeshType type,
                                 MeshCacheListSet* pcacheList,
                                 MeshBaseContent& mc, SimpleMeshBuffer* pbuffer,
                                 UPInt allocAddress, UPInt allocSize,
                                 UPInt vertexOffset, unsigned vertexCount, unsigned indexOffset, unsigned indexCount,
                                 const VertexFormat* pfmt)
    {
        SF_ASSERT(type == Mesh_Complex);

        MeshCacheItem* p = (MeshCacheItem*)
            Render::MeshCacheItem::Create(type, pcacheList, sizeof(MeshCacheItem),
                                          mc, allocSize, vertexCount, indexCount);
        if (p)
        {
            p->pBuffer      = pbuffer;
            p->AllocAddress = allocAddress;
            p->AllocSize    = allocSize;
            p->VertexOffset = vertexOffset;
            p->IndexOffset  = indexOffset;
            p->DListSize    = 0xffffffff;
            p->DListOffset  = 0xffffffff;
            p->pFormat      = pfmt;
        }
        return p;
    }

    SimpleMeshBuffer*   GetBuffer() const { return pBuffer; }
    UPInt		GetAllocAddr() const { return AllocAddress; }
    UPInt	        GetVertexOffset() const { return VertexOffset; }
    const VertexFormat* GetVertexFormat() const { return pFormat; }

};


class MeshCache : public SimpleMeshCache
{       
    MemoryManager*  pMemManager;

    // SimpleMeshCache implementation
    virtual SimpleMeshBuffer* createHWBuffer(UPInt size, AllocType atype, unsigned arena);
    virtual void              destroyHWBuffer(SimpleMeshBuffer* pbuffer);

    void            adjustMeshCacheParams(MeshCacheParams* p);    

public:
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params);
    ~MeshCache();

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(MemoryManager* pmem, RenderSync* psync);
    // Resets MeshCache, releasing all buffers.
    void            Reset();       

    virtual bool    SetParams(const MeshCacheParams& params);

    virtual AllocResult AllocCacheItem(Render::MeshCacheItem** pdata,
        UByte** pvertexDataStart, IndexType** pindexDataStart,
        MeshCacheItem::MeshType meshType,
        MeshCacheItem::MeshBaseContent &mc,
        UPInt vertexBufferSize,
        unsigned vertexCount, unsigned indexCount,
        bool waitForCache,
        const VertexFormat* pDestFormat);

    virtual bool    PreparePrimitive(PrimitiveBatch* pbatch, MeshCacheItem::MeshContent &mc, bool waitForCache);

    virtual bool    SetLargeMeshVertices(Render::MeshCacheItem* pcacheItem,
                                         const VertexFormat* pSourceFormat, UPInt vertexOffset, UByte* pvertices, unsigned vertexCount,
                                         const VertexFormat* pDestFormat, UByte* pdest);
    virtual bool    SetLargeMeshIndices( Render::MeshCacheItem* pcacheItem,
                                         const VertexFormat* pSourceFormat, UPInt indexOffset, const UInt16* pindices, unsigned indexCount,
                                         const VertexFormat* pDestFormat, UByte* pdestIndex);

    virtual void    PostUpdateMesh(Render::MeshCacheItem * pcacheItem );
};

}}};  // namespace Scaleform::Render::Wii

#endif
