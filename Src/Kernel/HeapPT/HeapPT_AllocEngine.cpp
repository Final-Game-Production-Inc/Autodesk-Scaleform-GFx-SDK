/**************************************************************************

Filename    :   HeapPT_AllocEngine.cpp
Content     :   The main allocation engine
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../SF_Alg.h"
#include "HeapPT_AllocEngine.h"
#include "HeapPT_Root.h"
#include "HeapPT_MemoryHeap.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

//------------------------------------------------------------------------
void AllocEngine::compilerAsserts()
{
    // Must be at least 4 pointers
    SF_COMPILER_ASSERT(Heap_MinSize >= 4*sizeof(void*));
}


//------------------------------------------------------------------------
AllocEngine::AllocEngine(SysAllocPaged* sysAlloc,
                         MemoryHeapPT*  heap,
                         unsigned       allocFlags, 
                         UPInt          minAlignSize,
                         UPInt          granularity, 
                         UPInt          reserve,
                         UPInt          threshold,
                         UPInt       limit) :
    pHeap(heap),
    pSysAlloc(sysAlloc),
    pBookkeeper(GlobalRoot->GetBookkeeper()),
    MinAlignShift(Alg::UpperBit(minAlignSize)),
    MinAlignMask((UPInt(1) << MinAlignShift) - 1),
    Allocator(MinAlignShift),
    AllowTinyBlocks((allocFlags & Heap_AllowTinyBlocks) != 0),
    AllowDynaSize((allocFlags & Heap_AllowDynaSize) != 0),
    Valid(false),
    HasRealloc(false),
    SysGranularity(Heap_PageSize),
    Granularity((granularity + Heap_PageMask) & ~UPInt(Heap_PageMask)),
    Reserve((reserve + Granularity-1) / Granularity * Granularity),
    Threshold(threshold),
    SysDirectThreshold(0),
    Footprint(0),
    TinyFreeSpace(0),
    SysDirectSpace(0),
    pCachedBSeg(0),
    pCachedTSeg(0),
    Limit(limit),
    pLimHandler(0)
{
    // Must be locked by Root::Lock externally
    SF_HEAP_ASSERT(minAlignSize >= sizeof(void*) &&
                 ((minAlignSize-1) & minAlignSize) == 0); // Must be pow2

    SysAllocPaged::Info i;
    memset(&i, 0, sizeof(SysAllocPaged::Info));
    pSysAlloc->GetInfo(&i);

    HasRealloc = i.HasRealloc;

    if (i.Granularity < Heap_PageSize)
        i.Granularity = Heap_PageSize;
    SF_HEAP_ASSERT((i.Granularity & Heap_PageMask) == 0);

    SysGranularity     = i.Granularity;
    SysDirectThreshold = i.SysDirectThreshold;

    Granularity = (Granularity + i.Granularity-1) / i.Granularity * i.Granularity;

    if (i.MaxHeapGranularity)
    {
        i.MaxHeapGranularity = (i.MaxHeapGranularity + Heap_PageMask) & ~Heap_PageMask;
        if (Granularity > i.MaxHeapGranularity)
        {
            Granularity     = i.MaxHeapGranularity;
            AllowTinyBlocks = false;
            AllowDynaSize   = false;
          //Reserve         = 0; // Questionable
        }
    }

    // Restrict the minimal threshold. If direct system requests are too small
    // compared with the granularity there will be significant memory losses.
    // Empirically it's good enough to have at least 32x system granularity.
    // If threshold is 0 all the allocation will be redirected to the system
    // consuming a huge amount of memory. May be useful only for debugging.
    //------------------
    if (Threshold < 32*SysGranularity && Threshold)
        Threshold = 32*SysGranularity;

    if (SysDirectThreshold)
    {
        if (Threshold > SysDirectThreshold)
            Threshold = SysDirectThreshold;

        if (Threshold < Heap_PageSize && Threshold)
            Threshold = Heap_PageSize;

        if (Granularity > SysDirectThreshold)
        {
            Granularity = SysDirectThreshold;
            Granularity = (Granularity + Heap_PageMask) & ~Heap_PageMask;
        }

        if (SysGranularity > SysDirectThreshold)
        {
            SysGranularity = SysDirectThreshold;
            SysGranularity = (SysGranularity + Heap_PageMask) & ~Heap_PageMask;
        }
        if (Reserve > SysDirectThreshold)
            Reserve = SysDirectThreshold;
        AllowDynaSize = false;
    }

#ifdef SF_MEMORY_FORCE_SYSALLOC
    SysGranularity  = Heap_PageSize;
    Granularity     = Heap_PageSize;
    Threshold       = 0;//GHeap_PageSize;
    Reserve         = 0;
    AllowTinyBlocks = false;
#endif

    bool t;
    Valid = Reserve ? allocSegmentBitSet(Reserve, minAlignSize, Granularity, &t) != 0 : true;
}


//------------------------------------------------------------------------
AllocEngine::~AllocEngine()
{
    FreeAll();
}


// Function FreeAll() is used when the heap is destroyed.
// After calling this function the heap remains usable, but it loses 
// all the allocations. 
//------------------------------------------------------------------------
void AllocEngine::FreeAll()
{
    // Already locked in Root::DestroyHeap()
    HeapSegment* seg;
    while(!SegmentList.IsNull(seg = SegmentList.GetFirst()))
    {
        freeSegment(seg);
    }
    Allocator.Reset();
}


//------------------------------------------------------------------------
UPInt AllocEngine::SetLimit(UPInt lim)
{
    UPInt segSize = calcDynaSize();
    return Limit = (lim + segSize-1) / segSize * segSize;
}

//------------------------------------------------------------------------
HeapSegment* AllocEngine::allocSegment(unsigned  segType, UPInt dataSize, 
                                       UPInt alignSize, UPInt bookkeepingSize,
                                       bool *limHandlerOK)
{
    // Already locked by the caller

    if (Limit && Footprint + dataSize > Limit && pLimHandler)
    {
        LockSafe::TmpUnlocker unlocker(GlobalRoot->GetLock());
        ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        *limHandlerOK = 
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + dataSize - Limit);
        --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
    }

    *limHandlerOK = false;

    UPInt totalSegSize = 
        (sizeof(HeapSegment) + bookkeepingSize + Heap_MinMask) & ~UPInt(Heap_MinMask);

    UByte* segMem = (UByte*)pBookkeeper->Alloc(totalSegSize);
    HeapSegment* seg = (HeapSegment*)segMem;
    if (seg)
    {
        seg->SelfSize   = totalSegSize;
        seg->SegType    = UPIntHalf(segType);
        seg->Alignment  = Alg::UpperBit(UInt32(alignSize));
        seg->UseCount   = 0;
        seg->pHeap      = pHeap;
        seg->DataSize   = dataSize;
        seg->pData      = 0;
        if (dataSize)
        {
            // Allocate with the alignment of at least Heap_PageSize.
            // However, the original alignment must be stored as is.
            //---------------
            if (alignSize < Heap_PageSize)
                alignSize = Heap_PageSize;

            seg->pData = (UByte*)pSysAlloc->Alloc(dataSize, alignSize);
            if (seg->pData == 0)
            {
                pBookkeeper->Free(seg, totalSegSize);
                return 0;
            }
            if (!GlobalPageTable->MapRange(seg->pData, seg->DataSize))
            {
                pSysAlloc->Free(seg->pData, dataSize, alignSize);
                pBookkeeper->Free(seg, totalSegSize);
                return 0;
            }
            GlobalPageTable->SetSegmentInRange(UPInt(seg->pData), seg->DataSize, seg);
        }
        SegmentList.PushFront(seg);
        Footprint += seg->DataSize;
        *limHandlerOK = true;
        return seg;
    }
    return 0;
}


//------------------------------------------------------------------------
HeapSegment* AllocEngine::allocSegmentNoGranulator(UPInt dataSize, 
                                                   UPInt alignSize, 
                                                   bool* limHandlerOK)
{
    if (Limit && Footprint + dataSize > Limit && pLimHandler)
    {
        ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
        *limHandlerOK = 
            ((MemoryHeap::LimitHandler*)pLimHandler)->
                OnExceedLimit(pHeap, Footprint + dataSize - Limit);
        --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
    }

    *limHandlerOK = false;

    LockSafe::Locker rl(GlobalRoot->GetLock()); // Lock is necessary here!

    UByte* segMem = (UByte*)pBookkeeper->Alloc(sizeof(HeapSegment));
    HeapSegment* seg = (HeapSegment*)segMem;
    if (seg)
    {
        seg->SelfSize   = sizeof(HeapSegment);
        seg->SegType    = UPIntHalf(Heap_SegmentSystem);
        seg->Alignment  = 0;
        seg->UseCount   = 0;
        seg->pHeap      = pHeap;
        seg->DataSize   = 0;
        seg->pData      = 0;
        if (dataSize)
        {
            UPInt actualSize, actualAlign;
            seg->pData = (UByte*)pSysAlloc->AllocSysDirect(dataSize, alignSize, &actualSize, &actualAlign);
            if (seg->pData == 0)
            {
                pBookkeeper->Free(seg, seg->SelfSize);
                return 0;
            }

            UPInt pageAlign = (alignSize > Heap_PageSize) ? alignSize : Heap_PageSize;
            UPInt headBytes = ((UPInt(seg->pData) + pageAlign-1) & ~(pageAlign-1)) - UPInt(seg->pData);
            seg->Alignment  = Alg::UpperBit(UInt32(actualAlign));
            seg->UseCount   = 0x80000000 | headBytes;
            seg->DataSize   = actualSize - headBytes;
            seg->pData     += headBytes;

            if (!GlobalPageTable->MapRange(seg->pData, seg->DataSize))
            {
                pSysAlloc->FreeSysDirect(seg->pData    - headBytes,
                                         seg->DataSize + headBytes,
                                         UPInt(1) << seg->Alignment);

                pSysAlloc->FreeSysDirect(seg->pData, dataSize, alignSize);
                pBookkeeper->Free(seg, seg->SelfSize);
                return 0;
            }
            GlobalPageTable->SetSegmentInRange(UPInt(seg->pData), seg->DataSize, seg);
        }
        SegmentList.PushFront(seg);
        Footprint += seg->DataSize + (seg->UseCount & 0x7FFFFFFF);
        *limHandlerOK = true;
        return seg;
    }
    return 0;
}


//------------------------------------------------------------------------
HeapSegment* AllocEngine::allocSegmentLocked(unsigned segType, UPInt dataSize, 
                                             UPInt alignSize, UPInt bookkeepingSize,
                                             bool *limHandlerOK)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());
    return allocSegment(segType, dataSize, alignSize, bookkeepingSize, limHandlerOK);
}

//------------------------------------------------------------------------
HeapSegment* AllocEngine::allocSegmentBitSet(UPInt size, UPInt alignSize, 
                                             UPInt granularity,
                                             bool *limHandlerOK)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());

    UPInt alignment = (alignSize > MinAlignMask+1) ? alignSize : MinAlignMask+1;
    UPInt extraTail = (alignment > Heap_PageSize) ? alignment : 0;
    UPInt segSize   = (size + extraTail + alignment-1) & ~(alignment-1);
          segSize   = (segSize + granularity-1) / granularity * granularity;

    HeapSegment* seg = 
        allocSegment(Heap_SegmentBitSet, 
                     segSize, 
                     Heap_PageSize,
                     Allocator.GetBitSetBytes(segSize),
                     limHandlerOK);
    if (seg)
    {
        Allocator.InitSegment(seg);
    }
    return seg;
}


//------------------------------------------------------------------------
void AllocEngine::freeSegment(HeapSegment* seg)
{
    // Already locked by the caller

    UPInt freeingSize = seg->DataSize;
    if (pLimHandler)
    {
        ((MemoryHeap::LimitHandler*)pLimHandler)->OnFreeSegment(pHeap, freeingSize);
    }
    Footprint -= freeingSize;
    GlobalPageTable->UnmapRange(seg->pData, freeingSize);
    UPInt alignSize = UPInt(1) << seg->Alignment;
    if (seg->UseCount & 0x80000000)
    {
        Footprint -= (seg->UseCount & 0x7FFFFFFF);
        pSysAlloc->FreeSysDirect(seg->pData  - (seg->UseCount & 0x7FFFFFFF),
                                 freeingSize + (seg->UseCount & 0x7FFFFFFF),
                                 alignSize);
    }
    else
    {
        pSysAlloc->Free(seg->pData, freeingSize, 
                        (alignSize > Heap_PageSize) ? 
                         alignSize : Heap_PageSize);
    }
    SegmentListType::Remove(seg);
    pBookkeeper->Free(seg, seg->SelfSize);
}

//------------------------------------------------------------------------
void AllocEngine::freeSegmentLocked(HeapSegment* seg)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());
    freeSegment(seg);
}


//------------------------------------------------------------------------
void AllocEngine::freeSegmentBitSet(HeapSegment* seg)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());

    if (pCachedBSeg && pCachedBSeg != seg && pCachedBSeg->UseCount == 0)
    {
        Allocator.ReleaseSegment(pCachedBSeg);
        freeSegment(pCachedBSeg);
    }
    pCachedBSeg = 0;

#ifndef SF_MEMORY_FORCE_SYSALLOC
    // Option 1.
    if (GetUsedSpace() && seg->DataSize <= 4*Granularity)
    {
        pCachedBSeg = seg;
    }
    else
#endif
    {
        if (Footprint >= Reserve + seg->DataSize)
        {
            Allocator.ReleaseSegment(seg);
            freeSegment(seg);
        }
    }
}

//------------------------------------------------------------------------
void* AllocEngine::allocSysDirect(UPInt dataSize, UPInt alignSize)
{
    bool limHandlerOK = false;
    HeapSegment* seg;

    if (SysDirectThreshold && dataSize >= SysDirectThreshold)
    {
        do
        {
            seg = allocSegmentNoGranulator(dataSize, alignSize, &limHandlerOK);
            if (seg)
            {
                SysDirectSpace += seg->DataSize;
                return seg->pData;
            }
        }
        while(limHandlerOK);
    }

    // If allocSegmentNoGranulator() didn't work, try regular allocation.
    dataSize = (dataSize + (alignSize-1)) & ~(alignSize-1);
    dataSize = (dataSize + SysGranularity-1) / SysGranularity * SysGranularity;
    limHandlerOK = false;
    do
    {
        seg = allocSegmentLocked(Heap_SegmentSystem, dataSize, alignSize, 
                                 0, &limHandlerOK);
        if (seg)
        {
            SysDirectSpace += seg->DataSize;
            return seg->pData;
        }
    }
    while(limHandlerOK);
    return 0;
}


//------------------------------------------------------------------------
UPInt AllocEngine::calcDynaSize() const
{
#ifdef SF_MEMORY_FORCE_SYSALLOC
    return Granularity;
#else
    if (AllowDynaSize)
    {
        // Dynamic granularity works as follows:
        // 1. It takes a 1/16th of the totally allocated size and rounds it up 
        //    to the nearest granularity value.
        // 2. If not a power-of-two it rounds the size down to the nearest 
        //    power-of-two.
        // 3. Sets the size in range of [1...16] * Granularity
        UPInt dynaSize = (((GetUsedSpace() + 16) / 16) + Granularity-1) / Granularity * Granularity;

        dynaSize = UPInt(1) << Alg::UpperBit(dynaSize);

        if (dynaSize < Granularity)
            dynaSize = Granularity;

        if (dynaSize > 4*Granularity)
            dynaSize = 4*Granularity;

        return dynaSize;
    }
    return Granularity;
#endif
}

//------------------------------------------------------------------------
HeapSegment* AllocEngine::allocSegmentBitSet(UPInt size, UPInt alignSize, bool* limHandlerOK)
{
    HeapSegment* seg = allocSegmentBitSet(size, alignSize, calcDynaSize(), limHandlerOK);
    if (seg)
    {
        return seg;
    }
    return *limHandlerOK ? 
        allocSegmentBitSet(size, alignSize, Granularity, limHandlerOK) : 0;
}


//------------------------------------------------------------------------
void* AllocEngine::allocBitSet(UPInt size)
{
    void* ptr;
    HeapSegment* seg;
    bool limHandlerOK = false;
    do
    {
        ptr = Allocator.Alloc(size, &seg);
        if (ptr)
        {
            ++seg->UseCount;
            return ptr;
        }
        allocSegmentBitSet(size, MinAlignMask+1, &limHandlerOK);
    }
    while(limHandlerOK);
    return 0;
}


//------------------------------------------------------------------------
void* AllocEngine::allocBitSet(UPInt size, UPInt alignSize)
{
    void* ptr;
    HeapSegment* seg;
    bool limHandlerOK = false;
    do
    {
        ptr = Allocator.Alloc(size, alignSize, &seg);
        if (ptr)
        {
            ++seg->UseCount;
            return ptr;
        }
        allocSegmentBitSet(size, alignSize, &limHandlerOK);
    }
    while(limHandlerOK);
    return 0;
}


//------------------------------------------------------------------------
void* AllocEngine::allocSegmentTiny(unsigned sizeIdx)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());

    bool limHandlerOK;
    UPInt blockSize = (sizeIdx + 1) << MinAlignShift;

    // The segment must be able to keep at least 4 tiny blocks,
    // aligned to Heap_PageSize.
    //--------------------------
    UPInt segSize;
    segSize = (blockSize * 4 > Heap_PageSize) ? blockSize * 4 : Heap_PageSize;
    segSize = (segSize + Heap_PageMask) & ~UPInt(Heap_PageMask);

    HeapSegment* seg = allocSegment(sizeIdx, segSize, Heap_PageSize, 0, &limHandlerOK);
    if (seg)
    {
        UPInt numBlocks = seg->DataSize / blockSize;
        UByte* ptr = seg->pData;
        TinyListType& lst = TinyBlocks[sizeIdx];
        UPInt i;
        for (i = 0; i < numBlocks; ++i, ptr += blockSize)
        {
            ((TinyBlock*)ptr)->pSegment = seg;
            lst.PushBack((TinyBlock*)ptr);
        }
        TinyFreeSpace += seg->DataSize;
        return seg->pData;
    }
    return 0;
}



//------------------------------------------------------------------------
void AllocEngine::releaseSegmentTiny(HeapSegment* seg)
{
    UPInt i;
    UPInt blockSize = (seg->SegType + 1) << MinAlignShift;
    UPInt numBlocks = seg->DataSize / blockSize;
    UByte* ptr = seg->pData;
    for (i = 0; i < numBlocks; ++i, ptr += blockSize)
    {
        TinyListType::Remove((TinyBlock*)ptr);
    }
    TinyFreeSpace -= seg->DataSize;
    freeSegment(seg);
}


//------------------------------------------------------------------------
void AllocEngine::freeSegmentTiny(HeapSegment* seg)
{
    LockSafe::Locker rl(GlobalRoot->GetLock());

    if (pCachedTSeg && pCachedTSeg != seg && pCachedTSeg->UseCount == 0)
    {
        releaseSegmentTiny(pCachedTSeg);
    }
    pCachedTSeg = 0;

    if (GetUsedSpace())
    {
        pCachedTSeg = seg;
    }
    else
    {
        if (Footprint >= Reserve + seg->DataSize)
        {
            releaseSegmentTiny(seg);
        }
    }
}


//------------------------------------------------------------------------
inline void* AllocEngine::allocTiny(unsigned sizeIdx)
{
    TinyBlock* ptr = TinyBlocks[sizeIdx].GetFirst();
    if (TinyBlocks[sizeIdx].IsNull(ptr))
    {
        ptr = (TinyBlock*)allocSegmentTiny(sizeIdx);
        if (ptr == 0)
        {
            return 0;
        }
    }
    TinyListType::Remove(ptr);
    ++ptr->pSegment->UseCount;
    TinyFreeSpace -= (sizeIdx + 1) << MinAlignShift;
    return ptr;
}


//------------------------------------------------------------------------
inline void AllocEngine::freeTiny(HeapSegment* seg, TinyBlock* ptr)
{
    ptr->pSegment = seg;
    TinyBlocks[seg->SegType].PushFront(ptr);
    TinyFreeSpace += (seg->SegType + 1) << MinAlignShift;
    if (--seg->UseCount == 0)
    {
        freeSegmentTiny(seg);
    }
}

//------------------------------------------------------------------------
inline void* 
AllocEngine::reallocGeneral(HeapSegment* seg, void* oldPtr, 
                            UPInt oldSize, UPInt newSize,
                            UPInt alignShift)
{
    void* newPtr = Alloc(newSize, UPInt(1) << alignShift);
    if (newPtr)
    {
        memcpy(newPtr, oldPtr, (oldSize < newSize) ? oldSize : newSize);
        Free(seg, oldPtr);
    }
    return newPtr;
}




//------------------------------------------------------------------------
void* AllocEngine::reallocSysDirect(HeapSegment* seg, void* oldPtr, UPInt newSize)
{
    if (seg->UseCount & 0x80000000)
    {
        // Not-null UseCound means we have a system direct allocation, without
        // the granulator. In this case just use general realloc.
        //--------------------
        return reallocGeneral(seg, oldPtr, seg->DataSize, newSize, seg->Alignment);
    }

    UPInt alignSize = UPInt(1) << seg->Alignment;

    newSize = (newSize + (alignSize-1)) & ~(alignSize-1);
    newSize = (newSize + SysGranularity-1) / SysGranularity * SysGranularity;
    UPInt oldSize = seg->DataSize;
    if (newSize == oldSize)
    {
        return seg->pData; // May happen often
    }

    SF_HEAP_ASSERT((UByte*)oldPtr == seg->pData);
    if (newSize < seg->DataSize && 2*newSize < Threshold)
    {
        // When shrinking, the size can become too small to continue
        // with the direct system allocations. Switch to general realloc.
        //--------------------
        return reallocGeneral(seg, oldPtr, seg->DataSize, newSize, seg->Alignment);
    }

    bool ok = false;
    SF_UNUSED(ok);

    if (newSize > oldSize)
    {
        if (Limit && Footprint + newSize - oldSize > Limit && pLimHandler)
        {
            ++((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;
            bool limHandlerOK = 
                ((MemoryHeap::LimitHandler*)pLimHandler)->
                    OnExceedLimit(pHeap, Footprint + newSize - oldSize - Limit);
            --((MemoryHeap::LimitHandler*)pLimHandler)->AllocCount;

            if (!limHandlerOK || Footprint + newSize - oldSize > Limit)
                return reallocGeneral(seg, oldPtr, oldSize, newSize, seg->Alignment);
        }
    }

    {
        LockSafe::Locker rl(GlobalRoot->GetLock());
        if (HasRealloc &&
            pSysAlloc->ReallocInPlace(seg->pData, oldSize, newSize, alignSize))
        {
            // Remap
            //--------------------
            if (newSize > oldSize)
            {
                // Grow
                //--------------------
                if (!GlobalPageTable->RemapRange(seg->pData, newSize, oldSize))
                {
                    // Could not map, revert to the old state (shrink back)
                    //--------------------
                    ok = pSysAlloc->ReallocInPlace(seg->pData, newSize, oldSize, alignSize);
                    SF_HEAP_ASSERT(ok);
                    return NULL;
                }
            }
            else
            {
                // Shrink
                //--------------------
                ok = GlobalPageTable->RemapRange(seg->pData, newSize, oldSize);
                SF_HEAP_ASSERT(ok); 
            }

            Footprint      -= oldSize;
            Footprint      += newSize;
            SysDirectSpace -= oldSize;
            SysDirectSpace += newSize;

            seg->DataSize = newSize;
            return seg->pData;
        }
    }

    // Already locked by Root::Lock
    //--------------------
    return reallocGeneral(seg, oldPtr, oldSize, newSize, seg->Alignment);
}





//------------------------------------------------------------------------
void* AllocEngine::Alloc(UPInt size)
{
    // The malloc() policy is to allocate the smallest possible 
    // block if size=0, or, in general, less than AC_MinSize.
    //------------------------
    if (size < Heap_MinSize)
        size = Heap_MinSize;

    size = (size + MinAlignMask) & ~MinAlignMask;

    if (AllowTinyBlocks && size <= (UPInt(Heap_TinyBinSize) << MinAlignShift))
    {
        void* ptr;

        // The following empirical logic is necessary to correctly handle 
        // the situation with exceeding the footprint limit. It isn't necessary 
        // to make the allocation namely in the tiny segment. However, if the limit 
        // is exceeded after the first attempt it makes sense to try again 
        // considering that the application algorithm could free some memory.
        // If the second attempt fails just do a regular allocation.
        //-----------------
        ptr = allocTiny(unsigned((size - 1) >> MinAlignShift));
        if (ptr) return ptr;

        ptr = allocTiny(unsigned((size - 1) >> MinAlignShift));
        if (ptr) return ptr;
    }

    if (size < Threshold)
    {
        return allocBitSet(size);
    }

    return allocSysDirect(size, Heap_PageSize);
}


//------------------------------------------------------------------------
static const unsigned TinyPow2AllocType[Heap_TinyBinSize] =
{
//  For 16-byte MinAlign:
//  16->16  32->32  48->64  64->64  etc->128
      0,      1,      3,      3,    7, 7, 7, 7
};


// Alloc with the given alignment
//------------------------------------------------------------------------
void* AllocEngine::Alloc(UPInt size, UPInt alignSize)
{
    SF_HEAP_ASSERT(((alignSize-1) & alignSize) == 0); // Must be a power of two

    if ((alignSize-1) <= MinAlignMask)
    {
        return Alloc(size); // Redirect to regular alloc 
    }

    // The malloc() policy is to allocate the smallest possible 
    // block if size=0, or, in general, less than AC_MinSize.
    //------------------------
    if (size < Heap_MinSize)
        size = Heap_MinSize;

    // alignSize cannot be less than MinAlignSize
    //------------------------
    if (alignSize < MinAlignMask+1)
        alignSize = MinAlignMask+1;

    // The malloc() policy is to allocate the smallest possible 
    // block if size=0, or, in general, if less than alignSize.
    //------------------------
    if (size < alignSize)
        size = alignSize;

    size = (size + alignSize-1) & ~(alignSize-1);

    if (AllowTinyBlocks && size <= (UPInt(Heap_TinyBinSize) << MinAlignShift))
    {
        void* ptr;

        // The following empirical logic is necessary to correctly handle 
        // the situation with exceeding the footprint limit. It isn't necessary 
        // to make the allocation namely in the tiny segment. However, if the limit 
        // is exceeded after the first attempt it makes sense to try again 
        // considering that the application algorithm could free some memory.
        // If the second attempt fails just do a regular allocation.
        //-----------------
        ptr = allocTiny(TinyPow2AllocType[(size - 1) >> MinAlignShift]);
        if (ptr) return ptr;

        ptr = allocTiny(TinyPow2AllocType[(size - 1) >> MinAlignShift]);
        if (ptr) return ptr;
    }

    if (size < Threshold)
    {
        return allocBitSet(size, alignSize);
    }

    return allocSysDirect(size, alignSize);
}


//------------------------------------------------------------------------
void* AllocEngine::Realloc(HeapSegment* seg, void* oldPtr, UPInt newSize)
{
    void* newPtr  = 0;
    UPInt oldSize = 0;
    SF_HEAP_ASSERT(seg != 0);

    // The realloc() policy is to shrink to the smallest possible 
    // block if size=0, or, in general, less than AC_MinSize.
    //------------------------
    if (newSize < Heap_MinSize)
        newSize = Heap_MinSize;

    newSize = (newSize + MinAlignMask) & ~MinAlignMask;

    if (seg->SegType <= Heap_SegmentTiny8)
    {
        oldSize = (seg->SegType + 1) << MinAlignShift;
        if (newSize <= oldSize)
        {
            return oldPtr;      // Less or equal size.
        }
        return reallocGeneral(seg, oldPtr, oldSize, newSize, 
                              MinAlignShift + TinyPow2AllocType[seg->SegType]);
    }

    if (seg->SegType == Heap_SegmentBitSet)
    {
        newPtr = Allocator.ReallocInPlace(seg, oldPtr, newSize, &oldSize);
        if (newPtr)
        {
            return newPtr;
        }
        return reallocGeneral(seg, oldPtr, oldSize, newSize, 
                              Allocator.GetAlignShift(seg, oldPtr, oldSize));
    }

    SF_HEAP_ASSERT(seg->SegType == Heap_SegmentSystem);
    return reallocSysDirect(seg, oldPtr, newSize);
}


//------------------------------------------------------------------------
void AllocEngine::Free(HeapSegment* seg, void* ptr)
{
    SF_HEAP_ASSERT(seg != 0);
    if (seg->SegType <= Heap_SegmentTiny8)
    {
        freeTiny(seg, (TinyBlock*)ptr);
        return;
    }

    if (seg->SegType == Heap_SegmentBitSet)
    {
        Allocator.Free(seg, ptr);
        if (--seg->UseCount == 0)
        {
            freeSegmentBitSet(seg);
        }
        return;
    }

    SF_HEAP_ASSERT(seg->SegType == Heap_SegmentSystem);
    SysDirectSpace -= seg->DataSize;
    freeSegmentLocked(seg);
}


//------------------------------------------------------------------------
void* AllocEngine::Realloc(void* oldPtr, UPInt newSize)
{
    return Realloc(GlobalPageTable->GetSegment(UPInt(oldPtr)), oldPtr, newSize);
}


//------------------------------------------------------------------------
void AllocEngine::Free(void* ptr)
{
    Free(GlobalPageTable->GetSegment(UPInt(ptr)), ptr);
}


//------------------------------------------------------------------------
UPInt AllocEngine::GetUsableSize(HeapSegment* seg, const void* ptr)
{
    SF_HEAP_ASSERT(seg != 0);
    if (seg->SegType <= Heap_SegmentTiny8)
    {
        return UPInt(seg->SegType + 1) << MinAlignShift;
    }

    if (seg->SegType == Heap_SegmentBitSet)
    {
        return Allocator.GetUsableSize(seg, ptr);
    }

    SF_HEAP_ASSERT(seg->SegType == Heap_SegmentSystem);
    return seg->DataSize;
}


//------------------------------------------------------------------------
UPInt AllocEngine::GetUsableSize(const void* ptr)
{
    return GetUsableSize(GlobalPageTable->GetSegment(UPInt(ptr)), ptr);
}


//------------------------------------------------------------------------
UPInt AllocEngine::GetFootprint() const
{
    return Footprint;
}

//------------------------------------------------------------------------
UPInt AllocEngine::GetUsedSpace() const
{
    return Footprint - Allocator.GetTotalFreeSpace() - TinyFreeSpace;
}

//------------------------------------------------------------------------
void AllocEngine::ReleaseCachedMem()
{
    // Already locked by the caller with Root::Lock.

    if (pCachedBSeg && pCachedBSeg->UseCount == 0)
    {
        Allocator.ReleaseSegment(pCachedBSeg);
        freeSegment(pCachedBSeg);
    }
    if (pCachedTSeg && pCachedTSeg->UseCount == 0)
    {
        freeSegmentTiny(pCachedTSeg);
    }
    pCachedBSeg = 0;
    pCachedTSeg = 0;
}


//------------------------------------------------------------------------
void AllocEngine::VisitMem(MemVisitor* visitor, unsigned flags) const
{
    if (flags & MemVisitor::VF_SysAlloc)
        pSysAlloc->VisitMem(visitor);

    if (flags & MemVisitor::VF_Starter)
        GlobalPageTable->VisitMem(visitor);

    if (flags & MemVisitor::VF_Bookkeeper)
        pBookkeeper->VisitMem(visitor, flags);

    if (flags & MemVisitor::VF_Heap)
    {
        const HeapSegment* seg = SegmentList.GetFirst();
        while(!SegmentList.IsNull(seg))
        {
            switch(seg->SegType)
            {
            case Heap_SegmentTiny1:
            case Heap_SegmentTiny2:
            case Heap_SegmentTiny3:
            case Heap_SegmentTiny4:
            case Heap_SegmentTiny5:
            case Heap_SegmentTiny6:
            case Heap_SegmentTiny7:
            case Heap_SegmentTiny8:
                visitor->Visit(seg, UPInt(seg->pData), seg->DataSize, 
                               MemVisitor::Cat_AllocTiny);
                {
                    UPInt tinySize = (seg->SegType + 1) << MinAlignShift;
                    UPInt tailSize = seg->DataSize % tinySize;
                    if (tailSize)
                    {
                        // Visit the tail in the tiny block that always remains free
                        UPInt tail = UPInt(seg->pData) + seg->DataSize / tinySize * tinySize;
                        visitor->Visit(seg, tail, tailSize, MemVisitor::Cat_AllocTinyFree);
                    }
                }
                break;

            case Heap_SegmentBookkeeper: // Reserved for the bookkeeper
                SF_HEAP_ASSERT(0);       // This event must never happen
                break;

            case Heap_SegmentSystem:
                visitor->Visit(seg, UPInt(seg->pData), seg->DataSize, 
                               MemVisitor::Cat_SystemDirect);
                break;

            case Heap_SegmentBitSet:
                visitor->Visit(seg, UPInt(seg->pData), seg->DataSize, 
                               MemVisitor::Cat_AllocBitSet);
                break;
            }
            seg = seg->pNext;
        }

        if (flags & MemVisitor::VF_HeapFree)
        {
            Allocator.VisitMem(visitor, MemVisitor::Cat_AllocBitSetFree);

            // Visit free tiny blocks
            UPInt i;
            for (i = 0; i < Heap_TinyBinSize; ++i)
            {
                const TinyListType& lst = TinyBlocks[i];
                const TinyBlock* block = lst.GetFirst();
                while(!lst.IsNull(block))
                {
                    visitor->Visit(block->pSegment,
                                   UPInt(block), (i+1) << MinAlignShift, 
                                   MemVisitor::Cat_AllocTinyFree);
                    block = block->pNext;
                }
            }
        }
    }
}


//------------------------------------------------------------------------
void AllocEngine::VisitSegments(SegVisitor* visitor) const
{
    const HeapSegment* seg = SegmentList.GetFirst();
    while(!SegmentList.IsNull(seg))
    {
        visitor->Visit(SegVisitor::Seg_Heap, seg->pHeap, 
                       UPInt(seg->pData), 
                       (seg->DataSize + Heap_PageMask) & ~Heap_PageMask);
        seg = seg->pNext;
    }
    Allocator.VisitUnused(visitor, SegVisitor::Seg_HeapUnused);
}


//------------------------------------------------------------------------
void AllocEngine::GetHeapOtherStats(HeapOtherStats* otherStats) const
{
    otherStats->Segments = 0;
    otherStats->Bookkeeping = 0;
    otherStats->DynamicGranularity = calcDynaSize();
    otherStats->SysDirectSpace = SysDirectSpace;
    const HeapSegment* seg = SegmentList.GetFirst();
    while(!SegmentList.IsNull(seg))
    {
        otherStats->Segments++;
        otherStats->Bookkeeping += seg->SelfSize;
        seg = seg->pNext;
    }

}

}} // Scaleform::Heap
