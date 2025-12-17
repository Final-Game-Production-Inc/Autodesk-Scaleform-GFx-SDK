/**************************************************************************

Filename    :   HeapPT_PageTable.cpp
Content     :   Allocator headers describing allocator data structures.
            :   
Created     :   July 14, 2008
Authors     :   Michael Antonov, Boris Rayskiy, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HeapPT_PageTable.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
UPInt PageTableMem[(sizeof(PageTable) + sizeof(UPInt) - 1) / sizeof(UPInt)];
PageTable* GlobalPageTable = 0;

PageTable::PageTable() : pStarter(0) 
{
    SF_HEAP_ASSERT(GlobalPageTable == 0);
    GlobalPageTable = this;
}

void PageTable::Init()
{
    ::new(PageTableMem) PageTable;
}


// Maps an address range by allocating all of the headers for it.
// Input:
//      mem             - start address of the heap segment.
//      size            - Heap size in bytes.
// Returns true if headers of Lv2 are successfully mapped, false
// if allocation failed.
//------------------------------------------------------------------------
bool PageTable::MapRange(void* mem, UPInt size)
{
    UPInt address = (UPInt)mem;

#ifndef  SF_64BIT_POINTERS

     SF_HEAP_ASSERT(((address + size) >> Heap_Lv2_PageShift) < Heap_Lv2_TableSize);
   
    // Do mapping of addresses in two steps:
    //  1. Allocate the node tables needed, roll back if
    //     there was a memory error.
    //  2. Assign segment pointers into all memory range nodes.
    
    UPInt startIndex = address >> Heap_Root_PageShift;    
    // We subtract 1 from end index and do inclusive iteration to make sure the last
    // address is mapped correctly if it didn't end on the border of the root node.
    UPInt endIndex   = (address + size - 1) >> Heap_Root_PageShift;    

    // Iterate over root pages AddRef/Allocate nested tables as necessary.
    for (UPInt i = startIndex; i <= endIndex; i++)
    {
        if (!RootTable[i].AddRef(pStarter))
        {
            // If failed, roll back and release mapped pages.
            for(; i > startIndex; --i)
                RootTable[i-1].Release(pStarter);
            return false;
        }
    }

#else // SF_64BIT_POINTERS
    
    UPInt startIndex5 = address >> Heap_Lv5_PageShift;
    UPInt endIndex5   = (address + size - 1) >> Heap_Lv5_PageShift;

    for (UPInt i5 = startIndex5; i5 <= endIndex5; ++i5)
    {
        HeapHeader5* table5 = RootTable + i5;

        if(!table5->AddRef(pStarter))
        {
            UPInt badSize = (i5 - startIndex5) << Heap_Lv5_PageShift;
            UnmapRange(mem, badSize);
            return false;
        }

        UPInt i4 = 0;
        UPInt endIndex4 = Heap_Lv4_TableSize - 1;

        if (i5 == startIndex5)
            i4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
        if (i5 == endIndex5)
            endIndex4 = ((address + size - 1) & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;

        UPInt startIndex4 = i4;

        for (; i4 <= endIndex4; ++i4)
        {
            HeapHeader4* table4 = table5->GetTable(i4);

            if (!table4->AddRef(pStarter))
            {
                UPInt badSize = ((i5 - startIndex5) << Heap_Lv5_PageShift) + 
                                ((i4 - startIndex4) << Heap_Lv4_PageShift);
                UnmapRange(mem, badSize);
                return false;
            }
            
            UPInt i3 = 0;
            UPInt endIndex3 = Heap_Lv3_TableSize - 1;

            if (i4 == startIndex4)
                i3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
            if (i4 == endIndex4)
                endIndex3 = ((address + size - 1) & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;

            UPInt startIndex3 = i3;

            for (; i3 <= endIndex3; ++i3)
            {
                HeapHeader3* table3 = table4->GetTable(i3);

                if (!table3->AddRef(pStarter))
                {
                    UPInt badSize = ((i5 - startIndex5) << Heap_Lv5_PageShift) + 
                                    ((i4 - startIndex4) << Heap_Lv4_PageShift) +
                                    ((i3 - startIndex3) << Heap_Lv3_PageShift);
                    UnmapRange(mem, badSize);
                    return false;
                }

                UPInt i2 = 0;
                UPInt endIndex2 = Heap_Lv2_TableSize - 1;

                if (i3 == startIndex3)
                    i2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
                if (i3 == endIndex3)
                    endIndex2 = ((address + size - 1) & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;

                UPInt startIndex2 = i2;

                for (; i2 <= endIndex2; ++i2)
                {
                    HeapHeader2* table2 = table3->GetTable(i2);

                    if (!table2->AddRef(pStarter))
                    {
                        UPInt badSize = ((i5 - startIndex5) << Heap_Lv5_PageShift) + 
                                        ((i4 - startIndex4) << Heap_Lv4_PageShift) +
                                        ((i3 - startIndex3) << Heap_Lv3_PageShift) +
                                        ((i2 - startIndex2) << Heap_Lv2_PageShift);
                        UnmapRange(mem, badSize);
                        return false;
                    }
                }
            }
        }
    }

#endif // SF_64BIT_POINTERS 

    return true;

}


// Unmaps a memory range mapped by MapRange. 
//------------------------------------------------------------------------
void PageTable::UnmapRange(void* mem, UPInt size)
{
    UPInt address = (UPInt)mem;

#ifndef SF_64BIT_POINTERS

    SF_HEAP_ASSERT(((address + size) >> Heap_Lv2_PageShift) < Heap_Lv2_TableSize);
     
    UPInt startIndex = address >> Heap_Root_PageShift;
    UPInt endIndex   = (address + size - 1) >> Heap_Root_PageShift;

    // Iterate over root pages AddRef/Allocate nested tables as necessary.
    for (UPInt i = startIndex; i <= endIndex; i++)
        RootTable[i].Release(pStarter);

#else // SF_64BIT_POINTERS

    SF_HEAP_ASSERT(address != 0);
    SF_HEAP_ASSERT(((address + size) >> Heap_Lv5_PageShift) < Heap_Lv5_TableSize);

    UPInt startIndex5 = address >> Heap_Lv5_PageShift;
    UPInt endIndex5   = (address + size - 1) >> Heap_Lv5_PageShift;

    for (UPInt i5 = startIndex5; i5 <= endIndex5; ++i5)
    {
        HeapHeaderRoot* table5 = RootTable + i5;

        UPInt i4 = 0;
        UPInt endIndex4 = Heap_Lv4_TableSize - 1;

        if (i5 == startIndex5)
            i4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
        if (i5 == endIndex5)
            endIndex4 = ((address + size - 1) & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;

        UPInt startIndex4 = i4;

        for (; i4 <= endIndex4; ++i4)
        {
            HeapHeader4* table4 = table5->GetTable(i4);

            UPInt i3 = 0;
            UPInt endIndex3 = Heap_Lv3_TableSize - 1;

            if (i4 == startIndex4)
                i3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
            if (i4 == endIndex4)
                endIndex3 = ((address + size - 1) & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;

            UPInt startIndex3 = i3;

            for (; i3 <= endIndex3; ++i3)
            {
                HeapHeader3* table3 = table4->GetTable(i3);

                UPInt i2 = 0;
                UPInt endIndex2 = Heap_Lv2_TableSize - 1;

                if (i3 == startIndex3)
                    i2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
                if (i3 == endIndex3)
                    endIndex2 = ((address + size - 1) & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;

                for (; i2 <= endIndex2; ++i2)
                {
                    HeapHeader2* table2 = table3->GetTable(i2);

                    table2->Release(pStarter);
                }
                table3->Release(pStarter);
            }
            table4->Release(pStarter);
        }
        table5->Release(pStarter);
    }

#endif // SF_64BIT_POINTERS

}


// Remaps memory range from old size of segment to new one.
bool PageTable::RemapRange(void* mem, UPInt newSize, UPInt oldSize)
{
    SF_HEAP_ASSERT(oldSize);
    SF_HEAP_ASSERT(newSize);

    if (newSize == oldSize)
        return true;

#ifndef SF_64BIT_POINTERS

    UPInt address = (UPInt)mem;

    // Grow.
    if (newSize > oldSize)
    {
        UPInt startIndex = (address + oldSize - 1) >> Heap_Root_PageShift;
        UPInt endIndex   = (address + newSize - 1) >> Heap_Root_PageShift;

        for (UPInt i = startIndex + 1; i <= endIndex; i++)
        {
            if (!RootTable[i].AddRef(pStarter))
            {
                for(--i; i > startIndex; --i)
                    RootTable[i].Release(pStarter);
                return false;
            }
        }
        SetSegmentInRange(UPInt(mem)+oldSize, newSize-oldSize, GetSegment(UPInt(mem)));
    }
    // Shrink.
    else
    {
        UPInt startIndex = (address + newSize - 1) >> Heap_Root_PageShift;
        UPInt endIndex   = (address + oldSize - 1) >> Heap_Root_PageShift;;

        for (UPInt i = startIndex + 1; i <= endIndex; i++)
            RootTable[i].Release(pStarter);
    }
    return true;

#else // SF_64BIT_POINTERS

    HeapSegment* seg = GetSegment(UPInt(mem));
    bool  ret  = false;
    UPInt size = oldSize;
    UnmapRange(mem, oldSize);
    if (MapRange(mem, newSize))
    {
        size = newSize;
        ret  = true;
    }
    else
    {
        bool ok = MapRange(mem, oldSize);
        SF_UNUSED(ok);
        SF_HEAP_ASSERT(ok); // Must succeed!
    }
    SetSegmentInRange(UPInt(mem), size, seg);
    return ret;

#endif // SF_64BIT_POINTERS
}


// Helper function used to assign a range of 'seg' pointers.
//------------------------------------------------------------------------
void PageTable::SetSegmentInRange(UPInt address, UPInt size, HeapSegment* seg)
{
    SF_HEAP_ASSERT(seg != NULL);
    
#ifndef SF_64BIT_POINTERS

    UPInt startIndex = address >> Heap_Root_PageShift;
    UPInt endIndex   = (address + size - 1) >> Heap_Root_PageShift;

    // Set segment pointers within the papped node descriptors.
    for (UPInt i = startIndex; i <= endIndex; i++)
    {        
        UPInt j    = 0; 
        UPInt jend = Heap_Lv1_TableSize-1; 

        if (i == startIndex)
            j = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
        if (i == endIndex)
            jend = ((address+size-1) & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;


        HeapHeader1* table = RootTable[i].GetTable();

        while(j <= jend)
        {
            table[j++].pSegment = seg;
        }
    }

#else //  SF_64BIT_POINTERS

    // DBG Working but slow.
    //for (UPInt a = address; a < (address + size); a += 4096)
    //{
    //
    //    UPInt i5 = a >> Heap_Lv5_PageShift;
    //    UPInt i4 = (a & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;
    //    UPInt i3 = (a & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;
    //    UPInt i2 = (a & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;
    //    UPInt i1 = (a & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
    //
    //    HeapHeaderRoot*  rootHeader = RootTable + i5;
    //
    //    rootHeader->GetTable(i4)->GetTable(i3)->GetTable(i2)->GetTable(i1)->pSegment = seg;
    //}


    UPInt lastAddr = address + size - 1;

    UPInt startIndex5 = address >> Heap_Lv5_PageShift;
    UPInt endIndex5   = lastAddr >> Heap_Lv5_PageShift;

    for (UPInt i5 = startIndex5; i5 <= endIndex5; ++i5)
    {
        const HeapHeader5* table5 = RootTable + i5;

        UPInt startIndex4   = 0;
        UPInt endIndex4     = Heap_Lv4_TableSize - 1;

        if (i5 == startIndex5)
            startIndex4 = (address & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;

        if (i5 == endIndex5)
            endIndex4   = (lastAddr & Heap_Lv5_PageMask) >> Heap_Lv4_PageShift;

        for (UPInt i4 = startIndex4; i4 <= endIndex4; ++i4)
        {
            const HeapHeader4* table4 = table5->GetTable(i4);

            UPInt startIndex3   = 0;
            UPInt endIndex3     = Heap_Lv3_TableSize - 1;

            if ((i4 == startIndex4) && (i5 == startIndex5))
                startIndex3 = (address & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;

            if ((i4 == endIndex4) && (i5 == endIndex5))
                endIndex3   = (lastAddr & Heap_Lv4_PageMask) >> Heap_Lv3_PageShift;

            for (UPInt i3 = startIndex3; i3 <= endIndex3; ++i3)
            {
                const HeapHeader3* table3 = table4->GetTable(i3);

                UPInt startIndex2   = 0;
                UPInt endIndex2     = Heap_Lv2_TableSize - 1;

                if ((i3 == startIndex3) && (i4 == startIndex4) && (i5 == startIndex5))
                    startIndex2 = (address & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;

                if ((i3 == endIndex3) && (i4 == endIndex4) && (i5 == endIndex5))
                    endIndex2   = (lastAddr & Heap_Lv3_PageMask) >> Heap_Lv2_PageShift;

                for (UPInt i2 = startIndex2; i2 <= endIndex2; ++i2)
                {
                    const HeapHeader2* table2 = table3->GetTable(i2);

                    UPInt startIndex1   = 0;
                    UPInt endIndex1     = Heap_Lv1_TableSize - 1;

                    if ((i2 == startIndex2) && (i3 == startIndex3) && (i4 == startIndex4) && (i5 == startIndex5))
                        startIndex1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
                    if ((i2 == endIndex2) && (i3 == endIndex3) && (i4 == endIndex4) && (i5 == endIndex5))
                        endIndex1   = (lastAddr & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;

                    for (UPInt i1 = startIndex1; i1 <= endIndex1; ++i1)
                    {
                        HeapHeader1* table1 = table2->GetTable(i1);
                        table1->pSegment = seg;
                    }
                }
            }
        }
    }

#endif

}




//------------------------------------------------------------------------
#if defined(SF_MEMORY_ENABLE_DEBUG_INFO) && !defined(SF_64BIT_POINTERS)

//------------------------------------------------------------------------
void PageTable::SetDebugNode(UPInt address, DebugNode* node)
{
    UPInt            rootIndex  = address >> Heap_Root_PageShift;
    HeapHeaderRoot*  rootHeader = RootTable + rootIndex;

    if (rootHeader->HasTable())
    {
        UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
        rootHeader->GetTable()[i1].pDebugNode = node;
    }
}



//------------------------------------------------------------------------
DebugNode* PageTable::GetDebugNode(UPInt address) const
{
    UPInt                  rootIndex  = address >> Heap_Root_PageShift;
    const HeapHeaderRoot*  rootHeader = RootTable + rootIndex;

    SF_HEAP_ASSERT(rootHeader->HasTable());
    UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;
    return rootHeader->GetTable()[i1].pDebugNode;
}



//------------------------------------------------------------------------
DebugNode* PageTable::FindDebugNode(UPInt address) const
{
    UPInt                 rootIndex  = address >> Heap_Root_PageShift;
    const HeapHeaderRoot* rootHeader = RootTable + rootIndex;
    SF_HEAP_ASSERT(rootHeader->HasTable());
    UPInt i1 = (address & Heap_Lv2_PageMask) >> Heap_Lv1_PageShift;

    const HeapHeader1* table = rootHeader->GetTable();

    if (table[i1].pDebugNode)
        return table[i1].pDebugNode;

    const HeapSegment* thisSeg = table[i1].pSegment;

    for(;;)
    {
        for(; i1; --i1)
        {
            UPInt i = i1-1;

            if (table[i].pSegment != thisSeg)
                return 0;

            if (table[i].pDebugNode)
                return table[i].pDebugNode;
        }

        if (rootIndex == 0)
            break;

        table = RootTable[--rootIndex].GetTable();
        if (table == 0)
            break;

        i1 = Heap_Lv1_TableSize;
    }
    return 0;
}

#endif //SF_MEMORY_ENABLE_DEBUG_INFO

}} // Scaleform::Heap

