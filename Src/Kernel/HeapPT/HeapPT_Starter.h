/**************************************************************************

Filename    :   HeapPT_Starter.h
Content     :   Starter allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Internal allocator used for the initial bootstrapping.
                It allocates/frees the page mapping arrays.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Heap_Starter_H
#define INC_SF_Kernel_Heap_Starter_H

#include "HeapPT_Granulator.h"

namespace Scaleform { namespace HeapPT {

// ***** Starter
//
// Internal allocator used for the initial bootstrapping. 
// It allocates/frees the page mapping arrays, but can also be used
// as a general purpose allocator that requires the allocation size 
// when freeing memory.
//
// The starter is just optimized for pages of the same size. On 64-bit 
// platforms the pages can have different sizes, but definitely there 
// will not be a great variety of sizes. Most of the requests will be 
// of the same size, only few of them will have a bigger size.
//
// On 32-bit platforms, the detailed situation looks like this:
// Assume HeaderPageSize=4k, Granularity=16K, and allocation requests 
// for exactly 1K PageTables.
// One PageTable (1K) can serve 1MB, one header can serve 16 PageTables,
// that is, 16 MB. One header page can keep information about 204 headers,
// that is, all in all it can serve space of 3264 MB. Only after exceeding
// it (if anything is left available) the starter has to allocate the second 
// header page to serve the rest of physically available 4 GB of address 
// space (not to mention on 32-bit WinXP/Vista only 3GB is available). 
//
// On 64-bit platforms, the situation is heavier, but still, remains 
// quite acceptable. You probably have to increase the "HeaderPageSize" 
// and "Granularity". In 64-bit case HeaderPageSize=8K and Granularity=32K
// will serve the requests perfectly.
//
// The only restriction is: HeaderPageSize and Granularity cannot be
// less than Heap_PageSize.
//------------------------------------------------------------------------
class Starter
{
public:
    Starter(SysAllocPaged* sysAlloc, 
            UPInt granularity    = 4*Heap_PageSize, 
            UPInt headerPageSize = 1*Heap_PageSize);

    SysAllocPaged* GetSysAlloc() { return Allocator.GetSysAlloc(); }

    void*   Alloc(UPInt size, UPInt alignSize);
    void    Free(void* ptr, UPInt size, UPInt alignSize); // Size is required!

    void    VisitMem(MemVisitor* visitor) const;
    void    VisitSegments(class SegVisitor* visitor) const;

    UPInt   GetFootprint() const { return Allocator.GetFootprint(); }
    UPInt   GetUsedSpace() const { return Allocator.GetUsedSpace(); }

private:
    Granulator Allocator;
};

}} // Scaleform::Heap

#endif
