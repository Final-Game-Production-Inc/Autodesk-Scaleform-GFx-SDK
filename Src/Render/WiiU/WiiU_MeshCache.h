/**************************************************************************

Filename    :   WiiU_MeshCache.h
Content     :   WiiU Mesh Cache header
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_WiiU_MeshCache_H
#define INC_SF_Render_WiiU_MeshCache_H

#include "Render/Render_SimpleMeshCache.h"
#include "Render/Render_MemoryManager.h"
#include "Render/WiiU/WiiU_Sync.h"

#include <cafe/gx2.h>

namespace Scaleform { namespace Render { namespace WiiU {

class MeshCache;
class HAL;

// WiiU version of MeshCacheItem. 
// We define this class primarily to allow HAL member access through friendship. 

class MeshCacheItem : public SimpleMeshCacheItem
{
    friend class MeshCache;
    friend class HAL;
};


// WiiU MeshCache implementation is simple, relying on SimpleMeshCache to do
// most of the work.

class MeshCache : public SimpleMeshCache
{       
    friend class HAL;    

    enum {
        MaxEraseBatchCount = 10
    };

    RenderSync*     RSync;
    MemoryManager*  pMemManager;

    // SimpleMeshCache implementation
    virtual SimpleMeshBuffer* createHWBuffer(UPInt size, AllocType atype, unsigned arena);
    virtual void              destroyHWBuffer(SimpleMeshBuffer* pbuffer);

    VertexXY16iInstance* pMaskEraseBatchVertexBuffer;

    bool            createStaticVertexBuffers();
    bool            createInstancingVertexBuffer();
    bool            createMaskEraseBatchVertexBuffer();

    void            adjustMeshCacheParams(MeshCacheParams* p);    

public:
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync);
    ~MeshCache();

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(MemoryManager* pmem);
    // Resets MeshCache, releasing all buffers.
    void            Reset();       

    virtual bool    SetParams(const MeshCacheParams& params);
    virtual void    PostUpdateMesh(Render::MeshCacheItem * pcacheItem );
};

}}};  // namespace Scaleform::Render::WiiU

#endif
