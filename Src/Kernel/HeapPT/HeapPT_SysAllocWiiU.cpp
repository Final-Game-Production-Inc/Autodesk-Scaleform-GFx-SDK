/**************************************************************************

Filename    :   HeapPT_SysAllocMalloc.cpp
Content     :   Malloc System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   System Allocator that uses regular malloc/free

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_SysAllocMalloc.h"
#include <cafe/mem.h>

namespace Scaleform {


//------------------------------------------------------------------------
SysAllocPagedMalloc::SysAllocPagedMalloc(UPInt granularity) :
    Granularity((granularity + MinGranularity-1) / MinGranularity * MinGranularity),
    Footprint(0),
    Base(~UPInt(0))
{
    SF_ASSERT(Granularity >= MinGranularity);
}

//------------------------------------------------------------------------
void SysAllocPagedMalloc::GetInfo(Info* i) const
{
    i->MinAlign    = 1;
    i->MaxAlign    = 1;
    i->Granularity = Granularity;
    i->SysDirectThreshold = 32768;
    i->MaxHeapGranularity = 8192;
    i->HasRealloc  = false;
}

//------------------------------------------------------------------------
void* SysAllocPagedMalloc::Alloc(UPInt size, UPInt align)
{
    void* ptr = MEMAllocFromDefaultHeapEx(size, align);
    if (ptr)
    {
        Footprint += size;

        if (UPInt(ptr) < Base) // DBG
            Base = UPInt(ptr);

        //memset(ptr, 0xC0, size); // DBG
    }

    return ptr;
}

//------------------------------------------------------------------------
bool SysAllocPagedMalloc::Free(void* ptr, UPInt size, UPInt)
{
    MEMFreeToDefaultHeap(ptr);
    Footprint -= size;

    return true;
}

UPInt SysAllocPagedMalloc::GetBase() const // DBG
{
    return Base;
}

} // Scaleform
