/**************************************************************************

Filename    :   HeapPT_SysAllocMapper.h
Content     :   System Memory allocator based on memory mapping interface
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Win32 System Allocator that uses VirtualAlloc
                and VirualFree.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_SysAllocMapper_H
#define INC_SF_Kernel_HeapPT_SysAllocMapper_H

#include "Kernel/SF_SysAlloc.h"

namespace Scaleform {

// ***** SysAllocMapper
//
// SysAllocMapper provides a system allocator implementation for 
// systems that allow to reserve address space and then map and unmap
// arbitrary memory pages. O
//
// This version differs from SysAllocWinAPI and SysAllocMMAP in such a way 
// that it takes a full advantage of memory mapping. The allocator reserves 
// address space segments of 128 megabytes and then commits and de-commits 
// 4K memory pages as necessary. The restrictions are: 
// 1) The total address space by default cannot exceed 32*128 megabytes. 
// 2) A single allocation cannot exceed 128 megabytes.
// It's good enough in practice, but might be not suitable in some extreme 
// cases, especially on 64-bit platforms.
//
// If possible, we recommend that developers use this interface directly instead
// of providing their own SysAlloc implementation. Since SysAllocMapper relies on
// the low-level OS functions, it intelligently considers both system page size and
// granularity, allowing it to be particularly alignment friendly. Due to this alignment
// efficiency, SysAllocMapper does not lose any memory on 4K alignment required
// internally by MemoryHeap. No alignment overhead means that SysAllocMapper uses
// memory and address space with maximum efficiency, making it the best choice for
// the Scaleform system allocator on Microsoft platforms.
//------------------------------------------------------------------------
class SysAllocMapper : public SysAllocPaged
{
    struct Segment
    {
        UByte*  Memory;
        UPInt   Size;
        UPInt   PageCount;
    };

    enum { MaxSegments = 32 };

public:
    // Initializes system allocator.
    SysAllocMapper(SysMemMapper* mapper, UPInt segSize, 
                   UPInt granularity, bool bestFit = true);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual UPInt   GetFootprint() const { return Footprint; }
    virtual UPInt   GetUsedSpace() const { return Footprint; }

    const UInt32*   GetBitSet(UPInt numSeg) const 
    {
        if (numSeg >= NumSegments)
        {
            return 0;
        }
        return (const UInt32*)(Segments[numSeg].Memory + SegmSize - PageSize);
    }

    virtual UPInt   GetBase() const; // DBG

private:
    UPInt       getBitSize(UPInt size) const;
    UInt32*     getBitSet(const UByte* start, UPInt size) const;
    UPInt       getEndBit(UPInt size) const;
    UByte*      getAlignedPtr(const UByte* ptr, UPInt alignment) const;
    bool        alignmentIsOK(const UByte* ptr, UPInt size, UPInt alignment, UPInt limit) const;
    void*       allocMem(UPInt pos, UPInt size, UPInt alignment);
    void*       allocMem(UPInt size, UPInt alignment);
    UPInt       freeMem(void* ptr, UPInt size);

    bool        inRange(const UByte* ptr, const Segment* seg) const;
    UPInt       binarySearch(const UByte* ptr) const;
    UPInt       findSegment(const UByte* ptr) const;
    bool        reserveSegment(UPInt size);
    void        releaseSegment(UPInt pos);

    SysMemMapper*   pMapper;
    UPInt           PageSize;
    UPInt           PageShift;
    UPInt           SegmSize;
    UPInt           Granularity;
    UPInt           Footprint;
    Segment         Segments[MaxSegments];
    UPInt           NumSegments;
    UPInt           LastSegment;
    bool            BestFit;
    UByte*          LastUsed;
};

} // Scaleform

#endif
