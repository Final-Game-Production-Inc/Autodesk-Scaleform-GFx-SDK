/**************************************************************************

Filename    :   X360_MeshCache.h
Content     :   XBox360 Mesh Cache header
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_X360_MeshCache_H
#define INC_SF_Render_X360_MeshCache_H

#include "Render/Render_SimpleMeshCache.h"
#include "Render/X360/X360_Types.h"
#include "Render/X360/X360_Sync.h"

#define SF_RENDER_X360_INDEX_FMT         D3DFMT_INDEX16

namespace Scaleform { namespace Render { namespace X360 {

class MeshBuffer;
class MeshCache;
class HAL;

// On XBox360 vertex and index buffers cover the same memory range.
// Our MeshBuffer adds defines IDirect3DVertexBuffer9/IndexBuffer9
// declarations to span that memory range.

class MeshBuffer : public SimpleMeshBuffer
{
    friend class MeshCache;
    // Vertex/Index buffers; on XBox360 these can be declared directly.    
    IDirect3DVertexBuffer9  VertexBuffer;
    IDirect3DIndexBuffer9   IndexBuffer;

public:
    MeshBuffer(UPInt size, AllocType type, unsigned arena)
        : SimpleMeshBuffer(size, type, arena)
    { }

    inline IDirect3DVertexBuffer9* GetVertexBuffer() { return &VertexBuffer; }
    inline IDirect3DIndexBuffer9*  GetIndexBuffer()  { return &IndexBuffer; }
};


// XBox360 version of MeshCacheItem. 
// We define this class primarily to allow HAL member access through friendship. 

class MeshCacheItem : public SimpleMeshCacheItem
{
    friend class MeshCache;
    friend class HAL;

public:
    inline IDirect3DVertexBuffer9* GetVertexBuffer() const
    {
        return ((MeshBuffer*)pBuffer)->GetVertexBuffer();
    }
    inline IDirect3DIndexBuffer9*  GetIndexBuffer() const
    {
        return ((MeshBuffer*)pBuffer)->GetIndexBuffer();
    }
};


// XBox360 MeshCache implementation is simple, relying on SimpleMeshCache to do
// most of the work. createHWBuffer()/destroyHWBuffer() functions map to XPhysicalAlloc,
// while PreparePrimitive is responsible for packaging up the data.
//
// Since XBox360 mesh buffers are located in physical memory, memory copies and
// re-indexing is used to access cached MeshBatchData objects when they are no
// longer in the staging buffer.
class MemoryManager;
class MeshCache : public SimpleMeshCache
{   
    friend class HAL;

    RenderSync*               getRenderSync() const { return (X360::RenderSync*)GetRenderSync(); }
    
    // SimpleMeshCache implementation
    virtual SimpleMeshBuffer* createHWBuffer(UPInt size, AllocType atype, unsigned arena);
    virtual void              destroyHWBuffer(SimpleMeshBuffer* pbuffer);

    bool                      createStaticVertexBuffers(IDirect3DDeviceX* pdevice);
    bool                      createMaskEraseBatchVertexBuffer(IDirect3DDeviceX* pdevice);
    void                      adjustMeshCacheParams(MeshCacheParams* p);

    MemoryManager * pMemoryManager;
    IDirect3DVertexBufferX MaskEraseBatchVertexBuffer;
    IDirect3DVertexBufferX SquareVertexBuffer;

public:
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync);
    ~MeshCache();    

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(IDirect3DDevice9* pdevice, MemoryManager * pmemmgr );
    // Resets MeshCache, releasing all buffers.
    void            Reset();    

    virtual bool    SetParams(const MeshCacheParams& params);
    virtual void    PostUpdateMesh(Render::MeshCacheItem * pcacheItem );
};

// Vertex structures used by both MeshCache and HAL.
class Render_InstanceData
{
public:
    // Color format for instance, Alpha expected to have index.
    UInt32 Instance;
};


}}};  // namespace Scaleform::Render::X360

#endif
