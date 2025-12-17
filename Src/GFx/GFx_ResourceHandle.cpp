/**************************************************************************

Filename    :   GFx_ResourceHandle.cpp
Content     :   Resource handle and resource binding support for GFx
Created     :   February 8, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "GFx/GFx_ResourceHandle.h"

#include "GFx/GFx_PlayerStats.h"

namespace Scaleform { namespace GFx {

// ***** ResourceBinding

ResourceBinding::ResourceBinding(MemoryHeap *pheap)
  : pHeap(pheap)
{    
    pResources    = 0;
    ResourceCount = 0;
    Frozen        = 0;
    pOwnerDefImpl = 0;
}
ResourceBinding::~ResourceBinding()
{
    Destroy();
}

void ResourceBinding::Destroy()
{
    if (pResources)
    {
        DestructArray<ResourceBindData>(pResources, ResourceCount);
        SF_HEAP_FREE(pHeap, pResources);
        pResources = 0;
    }
}


// TBD: Technically binding array can grow as Bind catches up
// with Read. Therefore, we might need to deal with some threading
// considerations here.
void    ResourceBinding::SetBindData(unsigned index, const ResourceBindData &bd)
{
    // If this object is frozen, we should not be calling Set.
    SF_ASSERT(Frozen == 0);

    // We can use a lock - free system later
    Lock::Locker lock(&ResourceLock);
    
    unsigned size = ((index + 1) + 15) & ~15;

    if (size > ResourceCount)
    {
        if (!pResources)
        {
            // Can't use new[] since ResourceBindData doesn't derive
            // from GNewOverrideBase.
            pResources = (ResourceBindData*)SF_HEAP_ALLOC(pHeap, size * sizeof(ResourceBindData),
                                                           StatMD_Other_Mem);
            SF_ASSERT(pResources);
            ConstructArray<ResourceBindData>(pResources, size);
            ResourceCount = size;
        }
        else
        {
            ResourceBindData* pnewRes = (ResourceBindData*)
                                           SF_HEAP_ALLOC(pHeap, size * sizeof(ResourceBindData),
                                                       StatMD_Other_Mem);
            SF_ASSERT(pnewRes);
            ConstructArray<ResourceBindData>(pnewRes, size);

            for (unsigned i=0; i< ResourceCount; i++)
                pnewRes[i] = pResources[i];

            DestructArray<ResourceBindData>(pResources, ResourceCount);
            SF_HEAP_FREE(pHeap, pResources);
            pResources    = pnewRes;
            ResourceCount = size;
        }
    }
    
    pResources[index] = bd;
}


void ResourceBinding::GetResourceData_Locked(ResourceBindData *pdata, unsigned index) const
{
    // Lock so that updates to SetBindData don't break our access.
    Lock::Locker lock(&ResourceLock);
    
    // Do bounds checks because it is possible that the table was not resized
    // to accommodate an index in a handle whose binding failed to load.
    *pdata = (index < ResourceCount) ? pResources[index] : ResourceBindData();
}

}} // Scaleform::GFx

