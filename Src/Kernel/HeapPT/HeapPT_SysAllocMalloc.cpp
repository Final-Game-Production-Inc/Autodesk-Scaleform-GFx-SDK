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
#include <stdlib.h>

namespace Scaleform {

//#include <stdio.h>  // DBG
//#include "SF_Memory.h" // DBG
//#include <string.h> // DBG

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
void* SysAllocPagedMalloc::Alloc(UPInt size, UPInt)
{
    void* ptr = malloc(size);
    if (ptr)
    {
        Footprint += size;

        if (UPInt(ptr) < Base) // DBG
            Base = UPInt(ptr);

        //memset(ptr, 0xC0, size); // DBG
    }

    //if (Memory::pGlobalHeap)             // DBG
    //    printf("+%u fp:%uK used:%f%%\n", 
    //        size/Granularity, Footprint/1024, 
    //        Memory::pGlobalHeap->GetTotalUsedSpace()/double(Footprint)*100.0);

    return ptr;
}

//------------------------------------------------------------------------
bool SysAllocPagedMalloc::Free(void* ptr, UPInt size, UPInt)
{
    free(ptr);
    Footprint -= size;

    //// With this DBG GFx crashes at exit. The crash is expected and normal.
    //if (Memory::pGlobalHeap)             // DBG
    //    printf("-%u fp:%uK used:%f%%\n",
    //        size/Granularity, Footprint/1024, 
    //        Memory::pGlobalHeap->GetTotalUsedSpace()/double(Footprint)*100.0);

    return true;
}

UPInt SysAllocPagedMalloc::GetBase() const // DBG
{
    return Base;
}

} // Scaleform

