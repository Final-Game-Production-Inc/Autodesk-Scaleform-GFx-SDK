/**************************************************************************

PublicHeader:   Render
Filename    :   Render_MeshCacheConfig.h
Content     :   Mesh Cache parameters and Configuration APIs
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_MeshCacheConfig_H
#define INC_SF_Render_MeshCacheConfig_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** MeshCacheParams

// MeshCacheParams holds parameters that control buffer sizes, batching
// and allocation policies of MeshCache.

struct MeshCacheParams
{
    // How much is reserved ahead, maximum, and default block size for growth.
    // IB/VB size distribution will be determined if necessary.
    UPInt    MemReserve;    
    UPInt    MemLimit;
    UPInt    MemGranularity;

    // Size up to which LRU Tail will be allowed to grow without data being
    // discarded, assuming that MemLimit wasn't yet reached.
    // Dynamic size of the cache will be then roughly 
    //  [sizeof(LastFrame) + LRUTailSize] * 1.125,
    // where 1.125 is a fragmentation adjustment factor.
    UPInt    LRUTailSize;

    // Staging buffer size; must be at least 2 * (MaxVerticesSizeInBatch + Inidices).
    // Serves as secondary cache on PC. Meshes larger thenNoBatchVerticesSizeThreshold
    // are not placed in a staging buffer.
    UPInt    StagingBufferSize;

    // Maximum VBSize evicted during a single lock. This should be set low
    // to avoid unnecessary thrashing that reduces MRU effectiveness.
    UPInt    VBLockEvictSizeLimit;

    // Maximum number of instances in a batch; 
    unsigned MaxBatchInstances;

    // The number of consecutive meshes that forces the use of instancing.
    unsigned InstancingThreshold;  

    // If mesh vertex size is exceeded, it can not be batched. Note that it
    // can still be instanced.
    unsigned NoBatchVerticesSizeThreshold;
    // The maximum size of vertices in a single batch; once this threshold
    // is exceeded new batch is created.
    unsigned MaxVerticesSizeInBatch;    
    unsigned MaxIndicesInBatch;


    // Default PC parameters constructor tag.
    enum PCDefaultsType      { PC_Defaults };
    enum ConsoleDefaultsType { Console_Defaults };

    
    // Default MeshCacheParams for PC.
    // On PC we can occupy more memory and need a large staging buffer
    // for rebuilding batches, as we don't have read access to VBs.

    MeshCacheParams(PCDefaultsType = PC_Defaults)
      : MemReserve(1024 * 1024 * 3),
        MemLimit(1024 * 1024 * 16),
        MemGranularity(1024 * 1024 * 3),
        LRUTailSize(1024 * 1024 * 10),
        StagingBufferSize(1024 * 1024 * 2),     
        VBLockEvictSizeLimit(1024 * 256),
        MaxBatchInstances(24),
        InstancingThreshold(5),
        NoBatchVerticesSizeThreshold(1024 * 8),
        MaxVerticesSizeInBatch(1024 * 16),
        MaxIndicesInBatch(1024 * 6)
    { }

    // Default MeshCacheParams for Consoles.
    // Due to tighter memory conditions use smaller blocks by default,
    // also with a smaller limit. Staging buffer can be small since
    // video memory is typically readable.
    MeshCacheParams(ConsoleDefaultsType)
      : MemReserve(1024 * 768),
        MemLimit(1024 * 1024 * 12),
        MemGranularity(1024 * 768),
        LRUTailSize(1024 * 768),
        StagingBufferSize(1024 * 64),
        VBLockEvictSizeLimit(1024 * 256),
        MaxBatchInstances(24),
        InstancingThreshold(5),
        NoBatchVerticesSizeThreshold(1024 * 8) ,
        MaxVerticesSizeInBatch(1024 * 16),
        MaxIndicesInBatch(1024 * 6)
    { }

    MeshCacheParams(const MeshCacheParams& src)
      : MemReserve(src.MemReserve),
        MemLimit(src.MemLimit),
        MemGranularity(src.MemGranularity),
        LRUTailSize(src.LRUTailSize),
        StagingBufferSize(src.StagingBufferSize),
        VBLockEvictSizeLimit(src.VBLockEvictSizeLimit),
        MaxBatchInstances(src.MaxBatchInstances),
        InstancingThreshold(src.InstancingThreshold),
        NoBatchVerticesSizeThreshold(src.NoBatchVerticesSizeThreshold),
        MaxVerticesSizeInBatch(src.MaxVerticesSizeInBatch),
        MaxIndicesInBatch(src.MaxIndicesInBatch)
    { }

    // Constructs MeshCacheParams based on previous value, overriding
    // specified non-zero arguments.
    MeshCacheParams(const MeshCacheParams& src,
                    UPInt reserve, UPInt limit = 0, UPInt granulariy = 0,
                    UPInt lruTail = 0, UPInt stagingSize = 0)
      : MemReserve(reserve ? reserve : src.MemReserve),
        MemLimit(limit ? limit : src.MemLimit),
        MemGranularity(granulariy ? granulariy : src.MemGranularity),
        LRUTailSize(lruTail ? lruTail : src.LRUTailSize),
        StagingBufferSize(stagingSize ? stagingSize : src.StagingBufferSize),
        VBLockEvictSizeLimit(src.VBLockEvictSizeLimit),
        MaxBatchInstances(src.MaxBatchInstances),
        InstancingThreshold(src.InstancingThreshold),
        NoBatchVerticesSizeThreshold(src.NoBatchVerticesSizeThreshold),
        MaxVerticesSizeInBatch(src.MaxVerticesSizeInBatch),
        MaxIndicesInBatch(src.MaxIndicesInBatch)
    { }
};


//------------------------------------------------------------------------
// ***** MeshCacheConfig

// MeshCacheConfig defined external configuration API of a MeshCache
// used by Renderer/HAL.

class MeshCacheConfig
{   
public:
    virtual ~MeshCacheConfig() {}

    // Sets new MeshCache parameters, re-creating mesh buffers if
    // cache is already initialized. Returns if buffer resize failed.
    // Note that some parameters applied may be modified by HAL to
    // match its capabilities.
    virtual bool    SetParams(const MeshCacheParams& params) = 0;
    
    // Returns last applied MeshCacheParams value, potentially adjusted
    // based on HAL capabilities. If SetParams wasn't called,    
    // default values for HAL are returned.
    virtual const MeshCacheParams& GetParams() const = 0;

    // Clears mesh cache, releasing all buffers except for reserve.
    virtual void    ClearCache() = 0;

};


}};  // namespace Scaleform::Render

#endif

