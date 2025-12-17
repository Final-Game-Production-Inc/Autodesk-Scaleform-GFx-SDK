/**************************************************************************

Filename    :   HeapPT_SysAllocStatic.cpp
Content     :   Static memory System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   System allocator that works entirely in a single 
                block of memory.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_HeapTypes.h"
#include "../SF_SysAlloc.h"
#include "HeapPT_AllocLite.h"

namespace Scaleform {

//------------------------------------------------------------------------
SysAllocStatic::SysAllocStatic(void* mem1, UPInt size1,
                               void* mem2, UPInt size2,
                               void* mem3, UPInt size3,
                               void* mem4, UPInt size4) :
    MinSize(Heap::Heap_PageSize),
    NumSegments(0), 
    pAllocator(0),
    TotalSpace(0)
{
    pAllocator = ::new(PrivateData) HeapPT::AllocLite(MinSize);
    if (mem1) AddMemSegment(mem1, size1);
    if (mem2) AddMemSegment(mem2, size2);
    if (mem3) AddMemSegment(mem3, size3);
    if (mem4) AddMemSegment(mem4, size4);
    SF_COMPILER_ASSERT(sizeof(PrivateData) >= sizeof(HeapPT::AllocLite));
    SF_COMPILER_ASSERT(sizeof(Segments[0]) >= sizeof(HeapPT::TreeSeg));
}

//------------------------------------------------------------------------
SysAllocStatic::SysAllocStatic(UPInt minSize) :
    MinSize(minSize),
    NumSegments(0), 
    pAllocator(0),
    TotalSpace(0)
{
    pAllocator = ::new(PrivateData) HeapPT::AllocLite(MinSize);
    SF_COMPILER_ASSERT(sizeof(PrivateData) >= sizeof(HeapPT::AllocLite));
    SF_COMPILER_ASSERT(sizeof(Segments[0]) >= sizeof(HeapPT::TreeSeg));
}


//------------------------------------------------------------------------
void SysAllocStatic::AddMemSegment(void* mem, UPInt size)
{
    UByte* ptr = (UByte*)((UPInt(mem) + MinSize-1) & ~(MinSize-1));
    UByte* end = (UByte*)((UPInt(mem) + size)      & ~(MinSize-1));
    if (NumSegments < MaxSegments)
    {
        HeapPT::TreeSeg* seg = (HeapPT::TreeSeg*)Segments[NumSegments];
        seg->Buffer   = ptr;
        seg->Size     = end - ptr;
        seg->UseCount = 0;
        TotalSpace   += seg->Size;
        pAllocator->InitSegment(seg);
        ++NumSegments;
    }
}

//------------------------------------------------------------------------
void SysAllocStatic::GetInfo(Info* i) const
{
    i->MinAlign     = pAllocator->GetMinSize();
    i->MaxAlign     = 0; // Any alignment
    i->Granularity  = 0; // No granularity
    i->HasRealloc   = true;
}


//------------------------------------------------------------------------
void* SysAllocStatic::Alloc(UPInt size, UPInt alignment)
{
    HeapPT::TreeSeg* seg;
    return pAllocator->Alloc(size, alignment, &seg);
}


//------------------------------------------------------------------------
bool SysAllocStatic::ReallocInPlace(void* oldPtr, UPInt oldSize, 
                                    UPInt newSize, UPInt alignment)
{
    UPInt i;
    for (i = 0; i < NumSegments; ++i)
    {
        HeapPT::TreeSeg* seg = (HeapPT::TreeSeg*)Segments[i];
        if ((UByte*)oldPtr >= seg->Buffer && 
            (UByte*)oldPtr <  seg->Buffer + seg->Size)
        {
            HeapPT::AllocLite::ReallocResult ret = 
                pAllocator->ReallocInPlace(seg, oldPtr, oldSize, 
                                           newSize, alignment);
            return ret < HeapPT::AllocLite::ReallocFailed;
        }
    }
    return false;
}


//------------------------------------------------------------------------
bool SysAllocStatic::Free(void* ptr, UPInt size, UPInt alignment)
{
    UPInt i;
    for (i = 0; i < NumSegments; ++i)
    {
        HeapPT::TreeSeg* seg = (HeapPT::TreeSeg*)Segments[i];
        if ((UByte*)ptr >= seg->Buffer && 
            (UByte*)ptr <  seg->Buffer + seg->Size)
        {
            pAllocator->Free(seg, ptr, size, alignment);
            return true;
        }
    }
    return false;
}


//------------------------------------------------------------------------
void SysAllocStatic::VisitMem(HeapPT::MemVisitor* visitor) const
{ 
    pAllocator->VisitMem(visitor, HeapPT::MemVisitor::Cat_SysAllocFree);
}



//------------------------------------------------------------------------
UPInt SysAllocStatic::GetBase() const 
{
    UPInt i;
    UPInt m = ~UPInt(0);
    for (i = 0; i < NumSegments; ++i)
    {
        HeapPT::TreeSeg* seg = (HeapPT::TreeSeg*)Segments[i];
        if (UPInt(seg->Buffer) < m)
        {
            m = UPInt(seg->Buffer);
        }
    }
    return m;
} 

//------------------------------------------------------------------------
UPInt SysAllocStatic::GetSize() const 
{ 
    UPInt i;
    UPInt s = 0;
    for (i = 0; i < NumSegments; ++i)
    {
        s += ((HeapPT::TreeSeg*)Segments[i])->Size;
    }
    return s;
}

//------------------------------------------------------------------------
UPInt SysAllocStatic::GetFootprint() const
{
    return TotalSpace;
}

//------------------------------------------------------------------------
UPInt SysAllocStatic::GetUsedSpace() const
{
    return TotalSpace - pAllocator->GetFreeBytes();
}

} // Scaleform
