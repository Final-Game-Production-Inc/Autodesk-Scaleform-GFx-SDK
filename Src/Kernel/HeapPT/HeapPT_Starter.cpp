/**************************************************************************

Filename    :   HeapPT_Starter.cpp
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

#include "HeapPT_Starter.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
Starter::Starter(SysAllocPaged* sysAlloc, 
                 UPInt granularity,
                 UPInt headerPageSize) : 
    Allocator(sysAlloc, 256, granularity, headerPageSize)
{}

//------------------------------------------------------------------------
void* Starter::Alloc(UPInt size, UPInt alignSize)
{
    return Allocator.Alloc(size, alignSize);
}

//------------------------------------------------------------------------
void Starter::Free(void* ptr, UPInt size, UPInt alignSize)
{
    Allocator.Free(ptr, size, alignSize);
}

//------------------------------------------------------------------------
void Starter::VisitMem(MemVisitor* visitor) const
{
    Allocator.VisitMem(visitor, 
                       MemVisitor::Cat_Starter, 
                       MemVisitor::Cat_StarterFree);
}

void Starter::VisitSegments(class SegVisitor* visitor) const
{
    Allocator.VisitSegments(visitor, SegVisitor::Seg_PageMap,
                                     SegVisitor::Seg_PageMapUnused);
}

}} // Scaleform::Heap
