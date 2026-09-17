/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_MeshCache.h
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_MeshCache_H
#define INC_SF_PS4_MeshCache_H

#include "Render/PS4/PS4_Config.h"
#include "Render/PS4/PS4_Sync.h"
#include "Render/PS4/PS4_MemoryManager.h"
#include "Render/Render_SimpleMeshCache.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace PS4 {

class MeshCache;
class HAL;

// Orbis version of MeshCacheItem. 
// We define this class primarily to allow HAL member access through friendship. 

class MeshCacheItem : public SimpleMeshCacheItem
{
    friend class MeshCache;
    friend class HAL;
};

// Orbis MeshCache implementation is simple, relying on SimpleMeshCache to do
// most of the work.

class MeshCache : public SimpleMeshCache
{       
    friend class HAL;    

    RenderSync*     RSync;
    MemoryManager*  pMemManager;

    // SimpleMeshCache implementation
    virtual SimpleMeshBuffer* createHWBuffer(UPInt size, AllocType atype, unsigned arena);
    virtual void              destroyHWBuffer(SimpleMeshBuffer* pbuffer);

    sce::Gnm::Buffer pMaskEraseBatchVertexBuffer[2];
    
    bool            createStaticVertexBuffers();
    bool            createInstancingVertexBuffer();
    bool            createMaskEraseBatchVertexBuffer();

    void            adjustMeshCacheParams(MeshCacheParams* p);    

public:
    MeshCache(MemoryHeap* pheap, const MeshCacheParams& params, RenderSync* rsync);
    ~MeshCache();

    // Initializes MeshCache for operation, including allocation of the reserve
    // buffer. Typically called from SetVideoMode.
    bool            Initialize(PS4::MemoryManager* pmem);
    // Resets MeshCache, releasing all buffers.
    void            Reset();       

    virtual bool    SetParams(const MeshCacheParams& params);
    virtual void    PostUpdateMesh(Render::MeshCacheItem * pcacheItem );
};

}}};  // namespace Scaleform::Render::Orbis

#endif // INC_SF_Orbis_MeshCache_H
