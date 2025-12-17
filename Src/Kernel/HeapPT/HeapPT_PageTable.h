/**************************************************************************

Filename    :   HeapPT_PageTable.h
Content     :   Allocator page table for mapping the address space.
            :   
Created     :   July 14, 2008
Authors     :   Michael Antonov, Boris Rayskiy, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapPT_PageTable_H
#define INC_SF_Kernel_HeapPT_PageTable_H

#include "../SF_HeapTypes.h"
#include "HeapPT_Starter.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

#ifdef SF_64BIT_POINTERS

#define Heap_Lv1_PageSize (UPInt(1) << Heap_Lv1_PageShift)
#define Heap_Lv2_PageSize (UPInt(1) << Heap_Lv2_PageShift)
#define Heap_Lv3_PageSize (UPInt(1) << Heap_Lv3_PageShift)
#define Heap_Lv4_PageSize (UPInt(1) << Heap_Lv4_PageShift)
#define Heap_Lv5_PageSize (UPInt(1) << Heap_Lv5_PageShift)

#define Heap_Lv1_PageMask ((UPInt(1) << Heap_Lv1_PageShift) - 1)
#define Heap_Lv2_PageMask ((UPInt(1) << Heap_Lv2_PageShift) - 1)
#define Heap_Lv3_PageMask ((UPInt(1) << Heap_Lv3_PageShift) - 1)
#define Heap_Lv4_PageMask ((UPInt(1) << Heap_Lv4_PageShift) - 1)
#define Heap_Lv5_PageMask ((UPInt(1) << Heap_Lv5_PageShift) - 1)

#endif


//------------------------------------------------------------------------
template<class T, UPInt N> class HeapHeader
{
    T*      pTable;
    UPInt   RefCount;

public:

    HeapHeader() : pTable(0), RefCount(0) {}

    T*      GetTable() const            { return pTable;                    }
    T*      GetTable(UPInt i) const     { return &pTable[i];                }
    //T*      GetTableSafe(UPInt i) const { return pTable ? &pTable[i] : 0;   } // DBG


#ifndef SF_64BIT_POINTERS
     
    bool    HasTable() const    { return pTable != 0; }

#else

    SF_INLINE bool  HasTable(UPInt address) const;

#endif

    bool    AddRef(Starter* starter)
    {
        if (!pTable)
        {
            // For potential ref counting in the pointer: We must ensure
            // the alignment provides enough capacity for ref counting.
            //-------------------
            pTable = (T*)starter->Alloc(sizeof(T) * N, sizeof(void*) * N);
            if (!pTable)
                return false;

            // We must initialize the table to 0 since that is the correct value
            // of headers that indexing relies on.
            memset(pTable, 0, sizeof(T) * N);
        }
        RefCount++;
        return true;
    }

    void    Release(Starter* starter)
    {
        if (--RefCount == 0)
        {
            starter->Free((UByte*)pTable, sizeof(T) * N, sizeof(void*) * N);
            pTable = 0;
        }
    }
};


#ifndef SF_64BIT_POINTERS

//------------------------------------------------------------------------
struct DebugNode;
struct HeapHeader1
{
    HeapSegment*    pSegment;
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
    DebugNode*      pDebugNode;
#endif
};

// Allocator header of level 2.
// Describes 1M memory block, using table containing 256 Lv1 headers.
//------------------------------------------------------------------------
typedef HeapHeader<HeapHeader1, Heap_Lv1_TableSize> HeapHeader2;
typedef HeapHeader2 HeapHeaderRoot;

#else // SF_64BIT_POINTERS

struct HeapHeader1
{
    HeapSegment*    pSegment;
};

// Allocator header of level 2-5. For details see Types.h
//________________________________________________________________________
typedef HeapHeader<HeapHeader1, Heap_Lv1_TableSize> HeapHeader2;
typedef HeapHeader<HeapHeader2, Heap_Lv2_TableSize> HeapHeader3;
typedef HeapHeader<HeapHeader3, Heap_Lv3_TableSize> HeapHeader4;
typedef HeapHeader<HeapHeader4, Heap_Lv4_TableSize> HeapHeader5;

typedef HeapHeader5 HeapHeaderRoot;


template<class T, UPInt N>
inline bool HeapHeader<T,N>::HasTable(UPInt address) const    
{
        UPInt i4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
        UPInt i3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
        UPInt i2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
        //UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift; // DBG
        
        HeapHeader4* table4 = this->GetTable();
        if (table4)
        {
            HeapHeader3* table3 = table4[i4].GetTable();
            if (table3)
            {
                HeapHeader2* table2 = table3[i3].GetTable();
                if (table2)
                {
                    HeapHeader1* table1 = table2[i2].GetTable();
                    if (table1)
                    {
                        return true;
                    }
                }
            }
        }
        return false; 
}

#endif // SF_64BIT_POINTERS


// ***** PageTable
//
// Root Page Table.
// Root interface implementing a page table that spans an address space,
// implemented differently for 32 and 64-bit system. The interface supports
// mapping and un-mapping address ranges, such that after a memory range is
// mapped, a Header class is allocated that corresponds to every BlockSize.
//
// Internally the headers for mapped ranges are managed with reference counts.
//------------------------------------------------------------------------
class PageTable
{
public:
    PageTable();

    // Init creates a single global instance in static memory 
    // and assigns GlobalPageTable.
    static void Init();

    // Set/Get the bootstrapping starter allocator.
    void        SetStarter(Starter* s)   { pStarter = s; }
    Starter*    GetStarter()             { return pStarter;  }

    // Looks up Header1 address based on the global page table.
    // DOESN'T DO ERROR CHECKS - these are not necessary within internal addressing
    // in allocator (only needed for externally passed addresses).
    SF_INLINE HeapSegment* GetSegment(UPInt address) const
    {
        
#ifndef SF_64BIT_POINTERS

        UPInt                  rootIndex  = address >> Heap_Root_PageShift;
        const HeapHeaderRoot*  rootHeader = RootTable + rootIndex;
        UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
        return rootHeader->GetTable()[i1].pSegment;

#else

        UPInt i5 = address >> Heap_Lv5_PageShift;
        UPInt i4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
        UPInt i3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
        UPInt i2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
        UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;

        const HeapHeaderRoot*  rootHeader = RootTable + i5;

        return rootHeader->
            GetTable(i4)->
            GetTable(i3)->
            GetTable(i2)->
            GetTable(i1)->pSegment;   
#endif

    }


    // Obtains a pointer to header with page table validity check.
    SF_INLINE HeapSegment*  GetSegmentSafe(UPInt address) const
    {

#ifndef SF_64BIT_POINTERS

        UPInt rootIndex = address >> Heap_Root_PageShift;
        UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
        const HeapHeaderRoot* rootHeader = RootTable + rootIndex;

        return rootHeader->HasTable() ? (rootHeader->GetTable()[i1].pSegment) : 0;

#else

        UPInt i5 = address >> Heap_Lv5_PageShift;
        UPInt i4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
        UPInt i3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
        UPInt i2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
        UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;

        const HeapHeaderRoot* table5 = RootTable + i5;
        if (table5)
        {
            const HeapHeader4* table4 = table5->GetTable(i4);
            if (table4)
            {
                const HeapHeader3* table3 = table4->GetTable(i3);
                if (table3)
                {
                    const HeapHeader2* table2 = table3->GetTable(i2);
                    if (table2)
                    {
                        const HeapHeader1* table1 = table2->GetTable(i1);
                        if (table1)
                        {
                            return table1->pSegment;
                        }
                    }
                }
            }
        }

        return NULL;

#endif

    }

    // Maps an address range by allocating all of the headers for it.
    // Input:
    //      address         - start address of the heap segment.
    //      size            - Heap size in bytes.
    // Returns true if headers of Lv2 are successfully mapped, false
    // if allocation failed.
    bool    MapRange(void* address, UPInt size);
    
    // Unmaps a memory range mapped by MapRange. 
    void    UnmapRange(void* address, UPInt size);

    // Remaps a memory range mapped by MapRange. 
    bool    RemapRange(void* address, UPInt newSize, UPInt oldSize);

    // Helper function used to assign a range of segment pointers,
    // used for mapping and external nested heap assignments.
    void    SetSegmentInRange(UPInt address, UPInt size, HeapSegment* seg);

#if defined(SF_MEMORY_ENABLE_DEBUG_INFO) && !defined(SF_64BIT_POINTERS)
    void        SetDebugNode(UPInt address, DebugNode* page);
    DebugNode*  GetDebugNode(UPInt address) const;
    DebugNode*  FindDebugNode(UPInt address) const;
#endif

    void    VisitMem(MemVisitor* visitor) 
    {
        pStarter->VisitMem(visitor); 
    }

private:
    // Bootstrapping starter allocator used for pages needed for 
    // Map/Unmap implementation.
    Starter*   pStarter;

    // Root page table. We could make it static or allocate it in the future;
    // however, we can't grab it from page allocator as the later one has fixed
    // size while this table contains the remaining number of entries.
    HeapHeaderRoot RootTable[Heap_Root_TableSize];
};

extern PageTable* GlobalPageTable;

}} // Scaleform::Heap

#endif
