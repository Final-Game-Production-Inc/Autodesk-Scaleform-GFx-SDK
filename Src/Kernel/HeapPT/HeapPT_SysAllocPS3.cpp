/**************************************************************************

Filename    :   HeapPT_SysAllocPS3.cpp
Platform    :   PS3
Content     :   PS3 system allocator
Created     :   2009
Authors     :   Vlad Merker, Maxim Shemanarev

Notes       :   PS3 system allocator that uses sys_memory_* functions

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <sys/memory.h>
#include "HeapPT_SysAllocPS3.h"

namespace Scaleform {

//------------------------------------------------------------------------
SysAllocPS3::SysAllocPS3(UPInt granularity)
{
    SF_ASSERT(granularity);
    Granularity = SysAlignment = (granularity < MaxPageSize ?
        MinPageSize :
        MaxPageSize);
    Footprint = 0;
}

//------------------------------------------------------------------------
void SysAllocPS3::GetInfo(Info* i) const
{
    i->MinAlign    = i->MaxAlign = SysAlignment;
    i->Granularity = Granularity;
    i->HasRealloc  = false;
}

//------------------------------------------------------------------------
void* SysAllocPS3::Alloc(UPInt size, UPInt)
{
    size = (size + SysAlignment - 1) & ~(SysAlignment - 1);

    sys_addr_t addr = 0;
    uint64_t flags = (Granularity == MaxPageSize ?
        SYS_MEMORY_PAGE_SIZE_1M :
        SYS_MEMORY_PAGE_SIZE_64K);

    int ret = sys_memory_allocate(size, flags, &addr);
    SF_ASSERT(ret == CELL_OK);
    SF_UNUSED(ret);
    if(addr)
        Footprint += size;

    return (void *)addr;
}

//------------------------------------------------------------------------
bool SysAllocPS3::Free(void* ptr, UPInt size, UPInt)
{
    int ret = sys_memory_free((sys_addr_t)ptr);
    SF_ASSERT(ret == CELL_OK);
    SF_UNUSED(ret);
    Footprint -= size;

    return true;
}

} // Scaleform