/**************************************************************************

Filename    :   SF_Stats.cpp
Content     :   Statistics tracking and reporting APIs
Created     :   May 20, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef SF_ENABLE_STATS

#include "SF_Stats.h"
#include "SF_Atomic.h"
#include "SF_Memory.h"
#include "SF_Debug.h"

namespace Scaleform {

// For root, we allow StatGroup_Default.
SF_DECLARE_STAT_GROUP(StatGroup_Default, "General", StatGroup_Default)
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(Stat_Mem, "Memory", StatGroup_Default)
SF_DECLARE_MEMORY_STAT(Stat_Default_Mem, "General", Stat_Mem)

#ifndef SF_STATS_DECL_H
#define SF_STATS_DECL_H "GFx/GFx_StatsDecl.h"
#endif

#include SF_STATS_DECL_H

/*
    Notes On Statistics
    -------------------

    These notes discuss some of concerns that can be addressed by the statistics
    system in the future. It is good to understand these while making a general
    stat tracking and display system.

    Counter Types

    The interpretation of the counter can have a significant effect on how it
    is updated and displayed. There are several possible counter types:

     1. Collector Count - This counter is reset to 0 before statistics report
                          is generated and then incremented as all the items
                          are "collected" for inclusion. Absolute Min/Max is
                          not known with this counter and recording it is not
                          useful within the statistic itself, although average
                          min/max can still be computed externally (subjects
                          to precision limits based on the sampling rate).

                          In GFx, it's planned to use such counters for memory
                          stat reporting, DP/Triangle counts, etc.

     2. Running Count   - This counter keeps the real-time state continuously,
                          and is thus incremented and decremented as objects/
                          memory/resources are created and destroyed. Min/Max
                          can be tracked correctly if necessary. Reset of counter
                          to 0 does not make sense as it would make the rest
                          of the results incorrect, but Min/Max could be reset
                          based on the sampling rate.

                          Heap memory sizes can be tracked that was so that
                          real Min/Max is known.

     3. Absolute Count -  Increment only version of the Running Count, which
                          can not be reset. Since the value is always incremented
                          tracking Min/Max does not make sense.

                          Could be applied to loaded data, such as the number
                          of shapes or fonts loaded from the SWF file, or the
                          total time it took for a resource to load.


    Statistical Data Collection and Display

    As statistics data is collected at specified intervals, there is a number
    of data elements which can be usefully tracked and displayed for every
    statistic. These include:

      - Instantaneous (final) value.
      - Average
      - Sum
      - Minimum
      - Maximum

    The last four statistics could be tracked and displayed based on the different
    sampling windows; for example, we can display  Average and Maximum values
    over the period of one Frame, one Second, or program Lifetime. Unfortunately,
    not all of this information is useful in all cases and displaying all of the
    possible values may make the display busy and hard to interpret.

    Here are a few examples:

     a) Advance Ticks,   - Average, Min, Max make sense over one second. Sum does
        Advanced MCCount   not since a second has many frames.

     b) MovieDef Memory  - Instantaneous value is more interesting, since as its
                           exact as opposed to averages (it is at least enough).
                           Min/Max not useful.

        Load Time Ticks  - Min/Max/Avg meaningless, since this is an absolute
                           count. Only the final value is useful.
        
     c) MovieView Memory - Min, Max, Instantaneous Val all useful.

     d) Tessellate Count, - Sum may be more interesting then Avg since tessellations
        Triangles Gen      only happen in certain frames and Avg would be small,
                           while knowing the total # of shapes/triangles processed
                           is useful.
    
   
   So, ultimately while displaying all of the values may be ok but confusing. Allowing
   developers select which value is displayed in the "Avg" column per stat may be
   more useful {Average, Sum, Instance}.

*/



// ***** Supported Interfaces

StatInfo_InterfaceImpl<MemoryStat>    Stat_MemoryInterface;
StatInfo_InterfaceImpl<TimerStat>     Stat_TimerInterface;
StatInfo_InterfaceImpl<CounterStat>   Stat_CounterInterface;

StatInfo::StatInterface* Stats_InterfaceTable[Stat::Stat_TypeCount] =
{
    0,
    &Stat_MemoryInterface,
    &Stat_TimerInterface,
    &Stat_CounterInterface
};


// ***** Descriptor Registration

struct StatDescRegistry
{
    enum
    {   
        Desc_MemEntryCount   = Stat_EntryCount * 2,

        Desc_PageShift       = 3,
        Desc_PageSize        = 1 << Desc_PageShift,
        Desc_PageTableSize   = Stat_MaxId / Desc_PageSize,

        // Page table entries are set to this value if no memory or slot
        // is allocated to them. We rely on default zero-initialization 
        // to assign Unused values.
        Desc_IdUnused        = 0
    };

    unsigned    DescAllocOffset;  

    // Id page table. All the contained values are biased by one (+1) so
    // that a default value of '0' can represent the Unused value.
    UInt16      IdPageTable[Desc_PageTableSize];
    // Reserved memory for Ids.
    StatDesc*   DescMem[Desc_MemEntryCount];
  

    void        RegisterDesc(StatDesc* pdesc)
    {
        // Get offset of the page, and if page does not exit, allocate it.
        UInt16 pageOffset = IdPageTable[pdesc->GetId() >> Desc_PageShift];
        if (pageOffset == Desc_IdUnused)
        {            
            if (Desc_MemEntryCount < (DescAllocOffset + Desc_PageSize))
            {
                SF_DEBUG_WARNING1(1,
                    "StatDescRegistry out of reserved memory on statId %d", pdesc->GetId());
                return;
            }

            // Update the page index.
            pageOffset = (UInt16)(DescAllocOffset) + 1;
            IdPageTable[pdesc->GetId() >> Desc_PageShift] = pageOffset;

            // And mark all offsets as un-initialized.
            StatDesc** p = DescMem + DescAllocOffset;
            for(int i = 0; i <Desc_PageSize; i++, p++)
                *p = 0;
            DescAllocOffset += Desc_PageSize;
        }

        // Record the descriptor entry.
        DescMem[pageOffset - 1 + (pdesc->GetId() & (Desc_PageSize-1))] = pdesc;
    }

    StatDesc*  GetDescImpl(unsigned statId)
    {
        unsigned    pageOffset = IdPageTable[statId >> Desc_PageShift];
        StatDesc*  pdesc;

        // If this ASSERT gets hit, it means that StatId is not declared.
        if (pageOffset == Desc_IdUnused)
            return 0;       
        pdesc = DescMem[(pageOffset-1) + (statId & (Desc_PageSize-1))];

        SF_ASSERT(!pdesc || (pdesc->GetId() == statId));
        return pdesc;
    }

    StatDesc*  GetDesc(unsigned statId)
    {
        StatDesc* pdesc = GetDescImpl(statId);

        // Simple debugging logic to display a message with a previous name in case 
        // that the statId is use was not declared.
#ifdef SF_BUILD_DEBUG          
        if (!pdesc)
        {
            unsigned    id = statId;
            StatDesc*   pdescPrev;

            while (id > 0)
            {
                id--;
                pdescPrev = GetDescImpl(id);
                if (pdescPrev)
                {
                    SF_DEBUG_ERROR3(1, "StatId %d following after Id %d '%s' not registered.", 
                                     statId, id, pdescPrev->GetName());
                    break;
                }
            }

            SF_ASSERT(0);
        }

#endif
        return pdesc;
    }
};


// We expect registry values to be zero-initialized. This is important
// because registration routines can run in any order, so we can not
// rely on the default constructor being available before RegisterDesc is called.
StatDescRegistry StatDescRegistryInstance = { 0, {0}, {0} };


/*  Child Tree Initialization

  We use two step delayed initialization of the descriptor child tree
  to avoid problems with different translation units coming in unknown
  order. 
    1) During static initialization, we register descriptors and 
       combine them into a linked list.
    2) On first call to GetDesc, we traverse the list and convert
       it into the tree. We use AtomicOps to make this singleton
       initialization thread safe (in case GetDesc() is called from
       different threads simultaneously on the first call).
*/

// Descriptor pointers used during static initialization.
static StatDesc* Stats_pFirstDesc = 0;
static StatDesc* Stats_pLastDesc  = 0;
// Flags used to track atomic initialization.
static volatile   unsigned Stats_InitDone = 0;
static volatile   unsigned Stats_InitByUs = 0;


// Register descriptors so that they can be looked up by id.
void         StatDesc::RegisterDesc(StatDesc* pdesc)
{
    StatDescRegistryInstance.RegisterDesc(pdesc);

    // Link up all ids which can be children in a common list,
    // so that we can later initialize a descriptor tree.
    //if (pdesc->GroupId)
    {
        if (Stats_pLastDesc)
        {
            Stats_pLastDesc->pNextSibling = pdesc;
            Stats_pLastDesc               = pdesc;
        }
        else
        {
            Stats_pFirstDesc = pdesc;
            Stats_pLastDesc  = pdesc;
        }
    }   
}


const StatDesc*   StatDesc::GetDesc(unsigned id)
{
    if (!AtomicOps<unsigned>::Load_Acquire(&Stats_InitDone))
        InitChildTree();

    return StatDescRegistryInstance.GetDesc(id);
}


void    StatDesc::InitChildTree()
{
    if (AtomicOps<unsigned>::Load_Acquire(&Stats_InitDone))
        return;

    // Make sure we are the only thread who is performing
    // static tree initialization. 
    unsigned oldInitByUs;   
    do {
        oldInitByUs = Stats_InitByUs;

        if (oldInitByUs == 1)
        {
            // Spin if someone else started initialization.
            while (!AtomicOps<unsigned>::Load_Acquire(&Stats_InitDone))
                ;
            return;
        }
    } while (!AtomicOps<unsigned>::CompareAndSet_Sync(&Stats_InitByUs, oldInitByUs, 1));


    StatDesc* pdesc, *pnext;
    
    for(pdesc = Stats_pFirstDesc; pdesc != 0; pdesc = pnext)
    {
        pnext = pdesc->pNextSibling;
        pdesc->pNextSibling = 0;

        // Link up the parent. 
        StatDesc* pparentDesc = StatDescRegistryInstance.GetDesc(pdesc->GroupId);

        if (pparentDesc != pdesc)
        {
            if (!pparentDesc->pChild)
                pparentDesc->pChild = pdesc;
            else
            {
                // Connect us in the end so that iteration order is preserved.          
                // This is safe to do here since all pNextSibling pointers modified
                // here would have been traversed in the previous iteration of our
                // containing for loop.
                StatDesc* psibling = pparentDesc->pChild;
                while(psibling->pNextSibling)
                    psibling = psibling->pNextSibling;
                psibling->pNextSibling = pdesc;
            }
        }
        else
        {
            SF_ASSERT(pdesc->Id == pdesc->GroupId);
        }
        
    }

    Stats_pFirstDesc = 0;
    Stats_pLastDesc  = 0;

    AtomicOps<unsigned>::Store_Release(&Stats_InitDone, 1);
}



// ***** StatBag implementation

// Create a stat bag with specified number of entries.
StatBag::StatBag(MemoryHeap *pheap, unsigned memReserve)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();

    pMem    = (UByte*) SF_HEAP_ALLOC(pheap, memReserve, Stat_StatBag_Mem);
    MemSize = memReserve;
    Clear();
}

StatBag::StatBag(const StatBag& source)
{
    pMem    = (UByte*) SF_ALLOC(Stat_EntryCount * 16, Stat_StatBag_Mem);
    MemSize = Stat_EntryCount * 16;
    *this = source;
}

StatBag::~StatBag()
{
    SF_FREE(pMem);
}


// Clear out the bag, removing all states.
void    StatBag::Clear()
{
    MemAllocOffset = 0;
    for(int i = 0; i< StatBag_PageTableSize; i++)
        IdPageTable[i] = StatBag_IdUnused;
}

// Reset stat values, usually causing peaks to be recorded.
void    StatBag::Reset()
{
    for(int i = 0; i< StatBag_PageTableSize; i++)
    {
        unsigned pageOffset = IdPageTable[i];

        if (pageOffset != StatBag_IdUnused)
        {
            UInt16* ptable = (UInt16*) (pMem + (pageOffset * StatBag_MemGranularity));

            for (int j=0; j < StatBag_PageSize; j++)
            {
                if (ptable[j] != StatBag_IdUnused)
                {
                    unsigned       id    = j | (i << StatBag_PageShift);
                    StatInterface* psi   = GetInterface(id);
                    Stat*          pstat = (Stat*)(pMem + ptable[j] * StatBag_MemGranularity);
                    psi->Reset(pstat);
                }
            }
        }
    }  
}

StatInfo::StatInterface* StatBag::GetInterface(unsigned id)
{
    const StatDesc* pdesc = StatDesc::GetDesc(id);
    SF_ASSERT(pdesc->GetType() < Stat::Stat_TypeCount);
    return Stats_InterfaceTable[pdesc->GetType()];
}


// Add a statistic value of a certain id.
bool    StatBag::Add(unsigned statId, Stat* pstat)
{ 
    // Check that stat is registered.
    StatInterface* psi = GetInterface(statId);
    Stat*          pthisStat = GetStatRef(statId);
    
    if (!pthisStat)
    {
        // Try to add stat
        pthisStat = (Stat*)AllocStatData(statId, psi->GetStatDataSize());
        if (!pthisStat)
            return false;
        psi->Init(pthisStat);
    }
    
    psi->Add(pthisStat, pstat);
    return true;
}

bool    StatBag::AddMemoryStat(unsigned statId, const MemoryStat& stat)
{
    MemoryStat* pmemStat = (MemoryStat*)GetStatRef(statId);
    if (!pmemStat)
    {
        pmemStat = (MemoryStat*)AllocStatData(statId, sizeof(MemoryStat));
        if (!pmemStat)
            return false;
        pmemStat->Init();
    }
 
    pmemStat->Add((MemoryStat*)&stat);
    return true;
}

bool    StatBag::IncrementMemoryStat(unsigned statId, UPInt alloc, UPInt use)
{
    MemoryStat* pmemStat = (MemoryStat*)GetStatRef(statId);
    if (!pmemStat)
    {
        pmemStat = (MemoryStat*)AllocStatData(statId, sizeof(MemoryStat));
        if (!pmemStat)
            return false;
        pmemStat->Init();
    }

    pmemStat->Increment(alloc, use);
    return true;
}



bool    StatBag::SetMin(unsigned statId, Stat* pstat)
{
    // Check that stat is registered.
    StatInterface* psi = GetInterface(statId);
    Stat*          pthisStat = GetStatRef(statId);

    if (!pthisStat)
    {
        // Try to add stat
        pthisStat = (Stat*)AllocStatData(statId, psi->GetStatDataSize());
        if (!pthisStat)
            return false;
        psi->Init(pthisStat);
        psi->Add(pthisStat, pstat);
    }
    else
    {
        psi->SetMin(pthisStat, pstat);
    }

    return true;
}

bool    StatBag::SetMax(unsigned statId, Stat* pstat)
{
    StatInterface* psi = GetInterface(statId);
    Stat*          pthisStat = GetStatRef(statId);

    if (!pthisStat)
    {
        // Try to add stat
        pthisStat = (Stat*)AllocStatData(statId, psi->GetStatDataSize());
        if (!pthisStat)
            return false;
        psi->Init(pthisStat);
        psi->Add(pthisStat, pstat);
    }
    else
    {
        psi->SetMax(pthisStat, pstat);
    }

    return true;
}


// Add values of a different stat bag to ours.
void    StatBag::CombineStatBags(const StatBag& other,
                                 bool (StatBag::*combineFunc)(unsigned, Stat*))
{
    // Go through the items in the other bag, and add them one item at a time.
    for(int i = 0; i< StatBag_PageTableSize; i++)
    {
        unsigned pageOffset = other.IdPageTable[i];
        if (pageOffset != StatBag_IdUnused)
        {
            UInt16* ptable = (UInt16*) (other.pMem + (pageOffset * StatBag_MemGranularity));

            for (int j=0; j < StatBag_PageSize; j++)
            {
                if (ptable[j] != StatBag_IdUnused)
                {
                    unsigned id    = j | (i << StatBag_PageShift);
                    Stat*    pstat = (Stat*)(other.pMem + ptable[j] * StatBag_MemGranularity);
                    (this->*combineFunc)(id, pstat);
                }
            }
        }
    } 
}



void    StatBag::RecursiveGroupUpdate(StatDesc::Iterator it)
{
    const StatDesc* pdesc = *it;

    if (pdesc)
    {
        StatDesc::Iterator ichild = pdesc->GetChildIterator();     

        if (pdesc->IsAutoSumGroup())
        {
            // Add all children to us.
            while(!ichild.IsEnd())
            {
                RecursiveGroupUpdate(ichild);

                Stat* pstatData = GetStatRef(ichild.GetId());
                if (pstatData)              
                    Add(pdesc->GetId(), pstatData);
                ++ichild;
            }
        }
        else
        {
            while(!ichild.IsEnd())
            {
                RecursiveGroupUpdate(ichild);
                ++ichild;
            }
        }
    }
}

// Update all cumulative groups in the list.
void    StatBag::UpdateGroups()
{
    if (!AtomicOps<unsigned>::Load_Acquire(&Stats_InitDone))
        StatDesc::InitChildTree();

    // Traverse the StateDesc tree recursively and add the items.   
    RecursiveGroupUpdate(StatDesc::GetGroupIterator(StatGroup_Default));
}


Stat*  StatBag::GetStatRef(unsigned statId) const
{
    // GetStat size.
    if (statId >= Stat_MaxId)
    {
        SF_DEBUG_WARNING1(1,
            "StatBag::GetStat - statId value %d is out of supported range", statId);
        return 0;
    }

    unsigned pageOffset = IdPageTable[statId >> StatBag_PageShift];   
    if (pageOffset == StatBag_IdUnused)
        return 0;

    UInt16* ptable = (UInt16*) (pMem + (pageOffset * StatBag_MemGranularity));

    unsigned pageIndex = ptable[statId & (StatBag_PageSize-1)];
    if (pageIndex == StatBag_IdUnused)
        return 0;

    return (Stat*)(pMem + (pageIndex * StatBag_MemGranularity));
}


UByte*  StatBag::AllocStatData(unsigned statId, UPInt size)
{
    SF_ASSERT(statId != 0);

    // Round up the size.
    size = (size + StatBag_MemGranularity - 1) & ~(StatBag_MemGranularity-1);

    UInt16 pageOffset = IdPageTable[statId >> StatBag_PageShift];   
    if (pageOffset == StatBag_IdUnused)
    {
        // If page does not exit, allocate it.
        UPInt   tableSize = ((StatBag_PageSize * sizeof(UInt16)) + StatBag_MemGranularity - 1) &
                            ~(StatBag_MemGranularity-1);

        if (MemSize < (MemAllocOffset + tableSize))
        {
            SF_DEBUG_WARNING1(1, "StatBag out of reserved memory on adding statId %d", statId);
            return 0;
        }

        // Update the page index.
        pageOffset = (UInt16)(MemAllocOffset / StatBag_MemGranularity);
        IdPageTable[statId >> StatBag_PageShift] = pageOffset;
        
        // And mark all offsets as un-initialized.
        UInt16* p = (UInt16*) (pMem + MemAllocOffset);
        for (int i = 0; i < StatBag_PageSize; i++, p++)
            *p = (UInt16)StatBag_IdUnused;

        MemAllocOffset += tableSize;        
    }

    UInt16*  ptable = (UInt16*) (pMem + (pageOffset * StatBag_MemGranularity));
    unsigned pageIndex = ptable[statId & (StatBag_PageSize-1)];   
    SF_ASSERT(pageIndex == StatBag_IdUnused);
    SF_UNUSED(pageIndex);

    // Make sure there is enough memory.
    if (MemSize < (MemAllocOffset + size))
    {
        SF_DEBUG_WARNING1(1, "StatBag out of reserved memory on adding statId %d", statId);
        return 0;
    }

    ptable[statId & (StatBag_PageSize-1)] = (UInt16)(MemAllocOffset / StatBag_MemGranularity);
    UByte *pdata = pMem + MemAllocOffset;
    MemAllocOffset += size;
    return pdata;
}


// Does this accumulate data or not?
bool    StatBag::GetStat(StatInfo *pstat, unsigned statId) const
{
    Stat* pstatData = GetStatRef(statId);
    if (!pstatData)
        return false;

    // Get type.
    *pstat = StatInfo(statId, GetInterface(statId), pstatData);
    return true;
}




// *** StatBag::Iterator Logic

StatBag::Iterator::Iterator(StatBag* pbag, unsigned id, unsigned groupId)
    : Id(id), GroupId(groupId), pBag(pbag)
{
    // Potentially advance Iterator once so that we point to
    // a valid item or end.   
    if (pbag)
    {
        AdvanceTillValid();
    }
    else
    {
        Id = StatBag_EndId;
    }
}

// Advance to the next valid item or fail.
bool StatBag::Iterator::AdvanceTillValid()
{
    if (!AtomicOps<unsigned>::Load_Acquire(&Stats_InitDone))
        StatDesc::InitChildTree();

    // Advance forward until we found a matching id or hit the end.
    while (Id < StatBag_EndId)
    {   
        // If this page is empty skip it.
        if (pBag->IdPageTable[Id >> StatBag_PageShift] == StatBag_IdUnused)
        {
            Id = (Id + StatBag_PageSize) & ~(StatBag_PageSize-1);
        }
        else
        {
            if (pBag->GetStat(&Result, Id))
            {
                const StatDesc* pdesc = StatDescRegistryInstance.GetDesc(Id);                 

                SF_ASSERT(pdesc);
                if ((GroupId == Stat_MaxId) ||
                    (pdesc->GetGroupId() == GroupId))
                {
                    // Found id, we are done.
                    return true;
                }
            }
            
            Id++;
        }
    }
    return false;
}


// Obtains an Iterator for the specified stat group. Default implementation
// will return all of the stats in the bag.
StatBag::Iterator    StatBag::GetIterator(unsigned groupId)
{
    return Iterator (this, 0, groupId);
}




// Wait for stats to be ready; useful if stat update
// request is queued up for update in a separate thread.
/*
void    StatBag::WaitForData()
{
}
*/

} // Scaleform

#endif // SF_ENABLE_STATS
