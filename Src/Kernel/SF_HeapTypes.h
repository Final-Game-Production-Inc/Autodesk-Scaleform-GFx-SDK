/**************************************************************************

Filename    :   SF_HeapTypes.h
Content     :   Types, defines, and constants
            :   
Created     :   July 14, 2008
Authors     :   Maxim Shemanarev, Boris Rayskiy, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Heap_Types_H
#define INC_SF_Kernel_Heap_Types_H

#include <string.h>      // memset, memcpy, etc
#include "SF_Types.h"
#include "SF_List.h"

namespace Scaleform {

class MemoryHeap;
class MemoryHeapPT;

namespace Heap {

// Heap assertions are defined separately to improve debug performance
// and to allow debug checks in Release configuration. 
//------------------------------------------------------------------------
#define SF_HEAP_ASSERT(a) SF_ASSERT(a)

//------------------------------------------------------------------------
#ifdef SF_64BIT_POINTERS
typedef UInt32 UPIntHalf;
typedef UInt16 UPIntQuarter;
#else
typedef UInt16 UPIntHalf;
typedef UInt8  UPIntQuarter;
#endif


// ***** Page Size Breakdowns

//  32 Bit System:
//
//  [Level]  [Table Entries]  [Bits]  [Bytes]  [Table Size]
//                            +12
//  Lv1      256              +8      1M        256*8 = 2K (*)
//  Lv2      4096             +12     4G       4096*8 = 32K
//                           = 32
//
//  64 Bit System:
//
//  [Level]  [Table Entries]  [Bits]  [Bytes]  [Table Size]
//                             +12
//   Lv1      256              +8      1M        256*16  = 4K (*)
//   Lv2      256              +8      256M      256*16  = 4k (*)
//   Lv3      4096             +12     256M*4K  4096*16 = 64k (*)
//   Lv4      4096             +12     256M*16M 4096*16 = 64k (*)
//   Lv5      4096             +12     16E      4096*16 = 64k
//
// (*) Dynamically allocated pages; these must all have the same size.


//------------------------------------------------------------------------
struct HeapSegment : ListNode<HeapSegment>
{
    UPInt           SelfSize;   // Size of HeapSegment in bytes
    UPIntHalf       SegType;    // See Heap_Segment...
    UPIntHalf       Alignment;  // Alignment for direct system allocs only
    UPInt           UseCount;   // Number of allocs to detect empty segments
    MemoryHeapPT*   pHeap;      // Pointer to the heap itself
    UPInt           DataSize;   // The payload size in bytes
    UByte*          pData;      // Pointer to the data payload
};

//------------------------------------------------------------------------
enum HeapConstants
{
    // Allocator flags. 
    //--------------------------------------------------------------------
    Heap_AllowTinyBlocks           = 0x10,
    Heap_AllowDynaSize             = 0x20,

    // Starter and DebugAlloc granularity constants.
    //--------------------------------------------------------------------
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

#ifdef SF_64BIT_POINTERS
    Heap_StarterGranularity        = 64*1024,
    Heap_StarterHeaderPageSize     = 8*1024,

    Heap_DebugAllocGranularity     = 512*1024,
    Heap_DebugAllocHeaderPageSize  = 8*1024,
    Heap_DebugAllocPoolSize        = 4096,
#else  // SF_64BIT_POINTERS
    Heap_StarterGranularity        = 32*1024,
    Heap_StarterHeaderPageSize     = 4*1024,

    Heap_DebugAllocGranularity     = 256*1024,
    Heap_DebugAllocHeaderPageSize  = 4*1024,
    Heap_DebugAllocPoolSize        = 2048,
#endif // SF_64BIT_POINTERS

#else  // SF_MEMORY_ENABLE_DEBUG_INFO

#ifdef SF_64BIT_POINTERS
    Heap_StarterGranularity        = 32*1024,
    Heap_StarterHeaderPageSize     = 8*1024,
#else
    Heap_StarterGranularity        = 16*1024,
    Heap_StarterHeaderPageSize     = 4*1024,
#endif

#endif // SF_MEMORY_ENABLE_DEBUG_INFO


    // Granulator and Bookkeeper constants.
    //--------------------------------------------------------------------
#ifdef SF_64BIT_POINTERS
    Heap_GranulatorHeaderPageSize  = 8*1024,
    Heap_BookkeeperGranularity     = 32*1024,
#else
    Heap_GranulatorHeaderPageSize  = 4*1024,
    Heap_BookkeeperGranularity     = 16*1024,
#endif


#ifndef SF_64BIT_POINTERS

    //--------------------------------------------------------------------
    Heap_Lv1_PageShift     = 12,
    Heap_Lv2_PageShift     = 20,
    Heap_Lv3_PageShift     = 32,

    Heap_Lv1_TableSize     = 1 << (Heap_Lv2_PageShift - Heap_Lv1_PageShift),
    Heap_Lv2_TableSize     = 1 << (Heap_Lv3_PageShift - Heap_Lv2_PageShift),
    Heap_Lv2_PageSize      = 1 <<  Heap_Lv2_PageShift,   
    Heap_Lv2_PageMask      =       Heap_Lv2_PageSize-1,

    Heap_Root_TableSize    = Heap_Lv2_TableSize,
    Heap_Root_PageShift    = Heap_Lv2_PageShift,

#else

    // 64-bit tables support.
    //--------------------------------------------------------------------
    Heap_Lv1_PageShift     = 12,
    Heap_Lv2_PageShift     = 20,
    Heap_Lv3_PageShift     = 28,
    Heap_Lv4_PageShift     = 40,
    Heap_Lv5_PageShift     = 52,
    Heap_Lv6_PageShift     = 64,

    Heap_Lv1_TableSize     = 1 << (Heap_Lv2_PageShift - Heap_Lv1_PageShift),
    Heap_Lv2_TableSize     = 1 << (Heap_Lv3_PageShift - Heap_Lv2_PageShift),
    Heap_Lv3_TableSize     = 1 << (Heap_Lv4_PageShift - Heap_Lv3_PageShift),
    Heap_Lv4_TableSize     = 1 << (Heap_Lv5_PageShift - Heap_Lv4_PageShift),
    Heap_Lv5_TableSize     = 1 << (Heap_Lv6_PageShift - Heap_Lv5_PageShift),
  
    Heap_Root_TableSize    = Heap_Lv5_TableSize,
    Heap_Root_PageShift    = Heap_Lv5_PageShift,

#endif


    // System alignment size. Must be equal to Heap_Lv1_PageSize and 
    // not less than the minimal system page alignment
    //--------------------------------------------------------------------
    Heap_PageShift         = Heap_Lv1_PageShift,
    Heap_PageSize          = 1 << Heap_PageShift,
    Heap_PageMask          =      Heap_PageSize-1,

    // Segment types. Must be conformed with Heap_TinyBinSize.
    //--------------------------------------------------------------------
    Heap_SegmentTiny1      = 0,  // Tiny blocks segment of size 1*MinAlignSize
    Heap_SegmentTiny2      = 1,  // Tiny blocks segment of size 2*MinAlignSize
    Heap_SegmentTiny3      = 2,  // etc...
    Heap_SegmentTiny4      = 3,
    Heap_SegmentTiny5      = 4,
    Heap_SegmentTiny6      = 5,
    Heap_SegmentTiny7      = 6,
    Heap_SegmentTiny8      = 7,
    Heap_SegmentBookkeeper = 8,  // Reserved for the bookkeeper
    Heap_SegmentSystem     = 9,  // Direct system allocation
    Heap_SegmentBitSet     = 10, // BitSet allocator segment
    Heap_SegmentUndefined  = 16, // Undefined segment. Used in VisitMem() only.

    // Minimal possible alignment and size of allocations and empty 
    // blocks in the allocation engine.
    // Heap_MinSize value MUST NOT be less than size of 4 pointers.
    // Actual alignment can be less than Heap_MinSize, but it's impossible
    // to allocate memory blocks of less than Heap_MinSize.
    // The bookkeeper allocates memory with the Heap_MinSize alignment.
    //--------------------------------------------------------------------
#ifndef SF_64BIT_POINTERS
    Heap_MinShift          = 4,
#else
    Heap_MinShift          = 5,
#endif
    Heap_MinSize           = 1 << Heap_MinShift,
    Heap_MinMask           =      Heap_MinSize-1,

    // The number of tiny block sizes: 
    // 1*MinAlignSize...Heap_TinyBinSize*MinAlignSize. 
    // Currently must be 8.
    //--------------------------------------------------------------------
    Heap_TinyBinSize      = 8,


    Heap_DummyValue
};


// ***** MemVisitor
//
// Interface for visiting memory zones. See VisitMem(). It's a callback
// that allows you to see the internals. Mostly it's free zones, because
// the allocator in general cannot track the allocated (busy) zones. 
// Note the categories. The memory in different categories may overlap,
// for example, first there comes Cat_AllocBitSet for the whole segment size,
// then, Cat_AllocBitSetFree for every free zone. In general if the visitor
// just draws pixels in a bitmap, with colors that correspond to categories,
// you eventually will obtain a correct picture.
// Categories Cat_SysAlloc and Cat_SysAllocFree depend on the implementation
// of GSysAlloc. In particular, the call to "Visit" may do nothing, so,
// no Cat_SysAlloc and Cat_SysAllocFree events will occur at all. In your
// implementation of the visitor, you can also ignore these events.
//
// Function Visit():
// Parameters "address", "size", and "cat" are obvious.
// Parameter "seg" can be ignored, but it may help to collect and "compact"
// the segments when they are "far away" from each other. In other words,
// the "seg" is the same for all Cat_...Free events within one segment.
// And of course, it allows you to obtain more information, in particular,
// the heap, the visiting area belongs to.
// seg->pHeap is NULL for Cat_SysAlloc, Cat_SysAllocFree, Cat_Starter,
// Cat_Bookkeeper, and Cat_BookkeeperFree.
//
// Visiting flags are used in some of functions VisitMem, in particular,
// in HeapEngine::VisitMem and they determine how detailed the information
// should be. VF_Heap means to visit heap segments only, VF_HeapDetailed
// includes the full traversal of the free zones. The same is concerning
// VF_Bookkeeper and VF_BookkeeperDetailed.
//------------------------------------------------------------------------
class MemVisitor
{
public:
    enum Category
    {
        Cat_SysAlloc,       // System areas. Depend on the SysAlloc implementation.
        Cat_SysAllocFree,   // Free zones within the system areas.
        Cat_Starter,        // Starter segments.
        Cat_StarterFree,    // Free zones within the starter segments.
        Cat_Bookkeeper,     // Bookkeeper segments.
        Cat_BookkeeperFree, // Free zones within the bookkeeper segments.
        Cat_SystemDirect,   // Segments allocated directly from the system.
        Cat_AllocBitSet,    // Bit-set segments (payload only.)
        Cat_AllocBitSetFree,// Free zones within bit-set segments.
        Cat_AllocTiny,      // Tiny blocks.
        Cat_AllocTinyFree,  // Free zones in tiny segments.
        Cat_DebugInfo,      // Debug Info
        Cat_DebugInfoFree,
        Cat_DebugPagePool,  // Debug Info detailed categories
        Cat_DebugPageFree,
        Cat_DebugDataPool,
        Cat_DebugDataBusy,
        Cat_DebugDataFree,
        Cat_Other           // Other segments (DLMalloc, and so on.)
    };

    enum VisitingFlags
    {
        VF_SysAlloc           = 0x01,
        VF_Starter            = 0x02,
        VF_Bookkeeper         = 0x04,
        VF_BookkeeperFree     = 0x08,
        VF_BookkeeperDetailed = VF_Bookkeeper | VF_BookkeeperFree,
        VF_Heap               = 0x10,
        VF_HeapFree           = 0x20,
        VF_HeapDetailed       = VF_Heap | VF_HeapFree,
        VF_DebugInfo          = 0x40,
        VF_DebugInfoDetailed  = 0x80 | VF_DebugInfo,
        VF_All                = 0xFFFFFFFF
    };
    virtual ~MemVisitor() {}
    virtual void Visit(const HeapSegment* seg, UPInt address, UPInt size, Category cat) = 0;
};



//--------------------------------------------------------------------
class SegVisitor
{
public:
    enum Seg_Category
    {
        Seg_SysMem              = 1, // The order is important.
        Seg_PageMap             = 2,
        Seg_Bookkeeper          = 3,
        Seg_DebugInfo           = 4,
        Seg_Heap                = 5,
        Seg_UnusedMask          = 0x80,
        Seg_SysMemUnused        = Seg_UnusedMask | Seg_SysMem,
        Seg_PageMapUnused       = Seg_UnusedMask | Seg_PageMap,
        Seg_BookkeeperUnused    = Seg_UnusedMask | Seg_Bookkeeper,
        Seg_DebugInfoUnused     = Seg_UnusedMask | Seg_DebugInfo,
        Seg_HeapUnused          = Seg_UnusedMask | Seg_Heap
    };

    virtual ~SegVisitor() { }

    // Called for each segment in all heaps, plus for additional 
    // bookkeeping, page mapping and debug info segments. Heap
    // is NULL for all the bookkeeping segments. If Seg_SysAlloc 
    // category is present, all other categories overlap SysAlloc
    // segments. There can be some unused space in SysAlloc.
    virtual void Visit(unsigned cat, const MemoryHeap* heap, 
                       UPInt addr, UPInt size) = 0;
};


}} // Scaleform::Heap

#endif

