/**************************************************************************

Filename    :   HeapMH_AllocBitSet2.cpp
Content     :   Bit-set based allocator, 2 bits per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "../SF_Alg.h"
#include "HeapMH_Root.h"
#include "HeapMH_AllocBitSet2.h"

namespace Scaleform { namespace HeapMH {

using namespace Heap;

//------------------------------------------------------------------------
AllocBitSet2MH::AllocBitSet2MH()
{
}


//------------------------------------------------------------------------
void AllocBitSet2MH::InitPage(PageMH* page, UInt32 index)
{
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(page->Start), &headers);
    memset(headers.BitSet, 0x55, PageMH::BitSetBytes); // set "all busy"
    if (headers.Header1)
    {
        headers.Header1->Magic = (UInt16)MagicHeader::MagicValue;
        headers.Header1->UseCount = 0;
        headers.Header1->Index = index;
        headers.Header1->DebugHeader = 0;
    }
    if (headers.Header2)
    {
        headers.Header2->Magic = (UInt16)MagicHeader::MagicValue;
        headers.Header2->UseCount = 0;
        headers.Header2->Index = index;
        headers.Header2->DebugHeader = 0;
    }
    UByte* start1 = 0;
    UByte* end1   = 0;
    UByte* start2 = 0;
    UByte* end2   = 0;
    if (headers.Header1)
    {
        start1 = headers.AlignedStart;
        end1   = (UByte*)headers.Header1;
    }
    if (headers.Header2)
    {
        start2 = ((UByte*)headers.Header2) + sizeof(MagicHeader);
        end2   = headers.AlignedEnd;
    }
    if ((UByte*)headers.BitSet < headers.Bound)
    {
        SF_ASSERT(end1);
        end1 -= PageMH::BitSetBytes;
    }
    else
    {
        SF_ASSERT(start2);
        start2 += PageMH::BitSetBytes;
    }
    if (start1)
    {
        BinNodeMH::MakeNode(start1, end1-start1, page);
        Bin.Push(start1);
        BitSet2::MarkFree(headers.BitSet, (start1-headers.AlignedStart) >> PageMH::UnitShift, 
                                          (end1-start1) >> PageMH::UnitShift);
    }
    if (start2)
    {
        BinNodeMH::MakeNode(start2, end2-start2, page);
        Bin.Push(start2);
        BitSet2::MarkFree(headers.BitSet, (start2-headers.AlignedStart) >> PageMH::UnitShift, 
                                          (end2-start2) >> PageMH::UnitShift);
    }
}


//------------------------------------------------------------------------
void AllocBitSet2MH::ReleasePage(UByte* start)
{
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(start), &headers);
    UByte* start1 = 0;
    UByte* start2 = 0;
    if (headers.Header1)
    {
        SF_ASSERT(headers.Header1->UseCount == 0);
        start1 = headers.AlignedStart;
        Bin.Pull(start1);
    }
    if (headers.Header2)
    {
        SF_ASSERT(headers.Header2->UseCount == 0);
        start2 = ((UByte*)headers.Header2) + sizeof(MagicHeader);
        if ((UByte*)headers.BitSet > headers.Bound)
        {
            start2 += PageMH::BitSetBytes;
        }
        Bin.Pull(start2);
    }
}



//------------------------------------------------------------------------
void* AllocBitSet2MH::Alloc(UPInt bytes, MagicHeadersInfo* headers)
{
    // Size must be greater or equal than UnitSize.
    // The conditions must be verified externally.
    SF_ASSERT(bytes >= PageMH::UnitSize && 
             (bytes &  PageMH::UnitMask) == 0 &&
              bytes <= PageMH::MaxBytes);

    UPInt      blocks = bytes >> PageMH::UnitShift;
    BinNodeMH* node   = Bin.PullBest(blocks);

    UByte*     best   = 0;
    if (node)
    {
        PageMH* page = node->GetPage();
#ifdef SF_64BIT_POINTERS
        if (page == 0)
            page = GlobalRootMH->ResolveAddress(UPInt(node));
#endif
        SF_ASSERT(page);
        best = (UByte*)node;
        GetMagicHeaders(UPInt(page->Start), headers);
        headers->Page = page;
        UPInt tailBytes = node->GetBytes() - bytes;
        if (tailBytes)
        {
            BinNodeMH::MakeNode(best + bytes, tailBytes, page);
            Bin.Push(best + bytes);
            BitSet2::MarkFree(headers->BitSet, 
                             (best + bytes - headers->AlignedStart) >> PageMH::UnitShift, 
                              tailBytes >> PageMH::UnitShift);
        }
        BitSet2::MarkBusy(headers->BitSet, 
                         (best - headers->AlignedStart) >> PageMH::UnitShift, 
                          bytes >> PageMH::UnitShift);
    }
    return best;
}

//------------------------------------------------------------------------
void AllocBitSet2MH::Free(PageMH* page, void* ptr, 
                          MagicHeadersInfo* headers,
                          UPInt* oldSize)
{
    GetMagicHeaders(UPInt(page->Start), headers);
    headers->Page = page;
    bool  left, right, tail;
    UPInt blocks, bytes, start;

    start  = ((UByte*)ptr - headers->AlignedStart) >> PageMH::UnitShift;
    blocks = BitSet2::GetBlockSize(headers->BitSet, start);
    SF_ASSERT(blocks);

    bytes  = *oldSize = blocks << PageMH::UnitShift;

    BitSet2::MarkFree(headers->BitSet, start, blocks);
    tail  = (UByte*)ptr + bytes < headers->AlignedEnd;
    left  = start && BitSet2::GetValue(headers->BitSet, start - 1     ) == 0;
    right = tail  && BitSet2::GetValue(headers->BitSet, start + blocks) == 0;
    if (left | right)
    {
        Bin.Merge((UByte*)ptr, bytes, left, right, page);
    }
    else
    {
        BinNodeMH::MakeNode((UByte*)ptr, bytes, page);
        Bin.Push((UByte*)ptr);
    }
}


//------------------------------------------------------------------------
UPInt AllocBitSet2MH::GetUsableSize(const PageMH* page, const void* ptr) const
{
    MagicHeadersInfo headers;
    GetMagicHeaders(UPInt(page->Start), &headers);
    UPInt start, blocks;
    start  = ((const UByte*)ptr - headers.AlignedStart) >> PageMH::UnitShift;
    blocks = BitSet2::GetBlockSize(headers.BitSet, start);
    SF_ASSERT(blocks);
    return blocks << PageMH::UnitShift;
}

//------------------------------------------------------------------------
void* AllocBitSet2MH::Alloc(UPInt bytes, UPInt alignSize, 
                            MagicHeadersInfo* headers)
{
    // Size must be greater or equal than UnitSize.
    // The conditions must be verified externally.
    SF_ASSERT(bytes >= PageMH::UnitSize && 
             (bytes &  PageMH::UnitMask) == 0 &&
              bytes <= PageMH::MaxBytes &&
              alignSize >= PageMH::UnitSize);

    UPInt      blocks    = bytes >> PageMH::UnitShift;
    UPInt      alignMask = alignSize-1;
    BinNodeMH* node      = Bin.PullBest(blocks, alignMask);
    UByte*     best      = 0;
    if (node)
    {
        PageMH* page = node->GetPage();
#ifdef SF_64BIT_POINTERS
        if (page == 0)
            page = GlobalRootMH->ResolveAddress(UPInt(node));
#endif
        SF_ASSERT(page);
        best = (UByte*)node;
        UByte* aligned = ListBinMH::GetAlignedPtr(best, alignMask);

        GetMagicHeaders(UPInt(page->Start), headers);
        headers->Page = page;
        UPInt headBytes = aligned - best;
        UPInt tailBytes = (best + node->GetBytes()) - (aligned + bytes);
        if (headBytes)
        {
            BinNodeMH::MakeNode(best, headBytes, page);
            Bin.Push(best);
            BitSet2::MarkFree(headers->BitSet, 
                             (best - headers->AlignedStart) >> PageMH::UnitShift, 
                              headBytes >> PageMH::UnitShift);
        }
        if (tailBytes)
        {
            BinNodeMH::MakeNode(aligned + bytes, tailBytes, page);
            Bin.Push(aligned + bytes);
            BitSet2::MarkFree(headers->BitSet, 
                             (aligned + bytes - headers->AlignedStart) >> PageMH::UnitShift, 
                              tailBytes >> PageMH::UnitShift);
        }
        BitSet2::MarkBusy(headers->BitSet, 
                         (aligned - headers->AlignedStart) >> PageMH::UnitShift, 
                          bytes >> PageMH::UnitShift);
        return aligned;
    }
    return 0;
}

//------------------------------------------------------------------------
void* AllocBitSet2MH::ReallocInPlace(PageMH* page, void* oldPtr, 
                                     UPInt newSize, UPInt* oldSize, 
                                     MagicHeadersInfo* headers)
{
    SF_ASSERT(oldPtr && newSize && 
              newSize >= PageMH::UnitSize &&
             (newSize &  PageMH::UnitMask) == 0);

    GetMagicHeaders(UPInt(page->Start), headers);
    headers->Page = page;

    UPInt    newBytes = newSize;
    UByte*   base     = headers->AlignedStart;
    UByte*   end      = headers->AlignedEnd;
    if ((UByte*)oldPtr < headers->Bound)
    {
        end = headers->Bound - sizeof(MagicHeader);
        if ((UByte*)headers->BitSet < headers->Bound)
            end -= PageMH::BitSetBytes;
    }

    UInt32*  bitSet   = headers->BitSet;
    UPInt    start    = ((UByte*)oldPtr - base) >> PageMH::UnitShift;
    UPInt    blocks   = BitSet2::GetBlockSize(bitSet, start);
    UPInt    oldBytes = blocks << PageMH::UnitShift;
    UByte*   nextAdj;
    BinNodeMH* nextNode;
    UPInt    nextBytes;
    
    *oldSize = oldBytes;

    if (newBytes > oldBytes)                // Grow
    {
        nextAdj = (UByte*)oldPtr + oldBytes;
        if (nextAdj < end && BitSet2::GetValue(bitSet, start + blocks) == 0)
        {
            nextNode = (BinNodeMH*)nextAdj;
            nextBytes = nextNode->GetBytes();

            SF_ASSERT(nextBytes);
            if (newBytes <= oldBytes + nextBytes)
            {
                Bin.Pull(nextAdj);
                UPInt tailBytes = oldBytes + nextBytes - newBytes;

                if (tailBytes)
                {
                    UByte* tailBlock = (UByte*)oldPtr + newBytes;
                    BinNodeMH::MakeNode(tailBlock, tailBytes, page);
                    Bin.Push(tailBlock);
                    BitSet2::MarkFree(bitSet, (tailBlock - base) >> PageMH::UnitShift, 
                                               tailBytes >> PageMH::UnitShift);
                }
                BitSet2::MarkBusy(bitSet, start, newBytes >> PageMH::UnitShift);
                return oldPtr;
            }
        }
        return 0;   // Can't grow
    }

    if (newBytes < oldBytes)                // Shrink
    {
        UPInt thisTail = oldBytes - newBytes;
        UPInt nextTail = 0;

        nextAdj = (UByte*)oldPtr + oldBytes;
        if (nextAdj < end && BitSet2::GetValue(bitSet, start + blocks) == 0)
        {
            nextNode = (BinNodeMH*)nextAdj;
            nextTail = nextNode->GetBytes();
            SF_ASSERT(nextTail);
        }
        if (thisTail + nextTail)
        {
            if (nextTail)
            {
                Bin.Pull(nextAdj);
            }
            nextAdj   = (UByte*)oldPtr + newBytes;
            thisTail += nextTail;

            BinNodeMH::MakeNode(nextAdj, thisTail, page);
            Bin.Push(nextAdj);
            BitSet2::MarkBusy(bitSet, start, newBytes >> PageMH::UnitShift);
            BitSet2::MarkFree(bitSet, (nextAdj - base) >> PageMH::UnitShift, 
                                      thisTail >> PageMH::UnitShift);
        }
    }
    return oldPtr;
}


}} // Scaleform::HeapMH
