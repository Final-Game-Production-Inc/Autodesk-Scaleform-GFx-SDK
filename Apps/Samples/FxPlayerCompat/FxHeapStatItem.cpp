/**************************************************************************

Filename    :   FxHeapStatItem.cpp
Content     :   Memory heap statistics tracking items
Created     :   October 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxHeapStatItem.h"
#include "FxResourceStats.h"

#include "GFx_Kernel.h"
#include "Kernel/SF_MsgFormat.h"
////////////////////////////////////////////////////////////////////////////////
// ***** MemStatistic

MemStatistic::MemStatistic( UPInt v1, UPInt v2, UPInt v3, UPInt v4 ) 
: AllocN(v1), Allocated(v2), Used(v3), PeakUsed(v4), PeakCurrent(v4)
{
}

MemStatistic::MemStatistic(const MemoryStat& ms)
: AllocN(ms.GetAllocCount())
, Allocated(ms.GetAllocated())
, Used(ms.GetUsed())
, PeakUsed(ms.GetUsed())
, PeakCurrent(ms.GetUsed())
{
}

void MemStatistic::operator /=(UPInt v)
{
    if (v)
    {
        AllocN /= v;
        Allocated /= v;
        Used /= v;
        // PeakUsed = ???;
        PeakCurrent /= v;
    } else
        Init();
}

MemStatistic& MemStatistic::operator /(UPInt v)
{
    if (v)
    {
        AllocN /= v;
        Allocated /= v;
        Used /= v;
        PeakUsed /= v; // ???;
        PeakCurrent /= v;
    } else
        Init();

    return *this;
}

MemStatistic& 
MemStatistic::operator =(const MemoryStat& ms)
{
    AllocN = ms.GetAllocCount();
    Allocated = ms.GetAllocated();
    Used = ms.GetUsed();
    PeakCurrent = ms.GetUsed();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);

    return *this;
}

void MemStatistic::operator +=( UPInt v )
{
    //AllocN += 0;
    Allocated += v;
    Used += v;
    PeakCurrent += v;
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);
}

void MemStatistic::operator +=(const MemStatistic& other)
{
    AllocN += other.GetAllocN();
    Allocated += other.GetAllocated();
    Used += other.GetUsed();
    PeakCurrent += other.GetPeakUsed();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);
}

MemStatistic& 
MemStatistic::operator =( const FxStat_HeapDesc& d )
{
    AllocN = d.GetAllocCount();
    Allocated = d.GetMemLocal().GetAllocated() + d.GetMemChildren().GetAllocated();
    Used = d.GetMemLocal().GetUsed() + d.GetMemChildren().GetUsed();
    PeakCurrent = d.GetMemLocal().GetPeakUsed() + d.GetMemChildren().GetPeakUsed();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);

    return *this;
}

MemStatistic& 
MemStatistic::SetWithDebud(const FxStat_HeapDesc& d)
{
    AllocN = d.GetAllocCount();
    Allocated = d.GetMemLocal().GetAllocated() + d.GetMemChildren().GetAllocated() + d.GetMemDebugChildren().GetAllocated();
    Used = d.GetMemLocal().GetUsed() + d.GetMemChildren().GetUsed() + d.GetMemDebugChildren().GetUsed();
    PeakCurrent = d.GetMemLocal().GetPeakUsed() + d.GetMemChildren().GetPeakUsed() + d.GetMemDebugChildren().GetPeakUsed();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);

    return *this;
}

void MemStatistic::operator +=( const FxStat_HeapDesc& d )
{
    AllocN += d.GetAllocCount();
    Allocated += d.GetMemLocal().GetAllocated() + d.GetMemChildren().GetAllocated();
    Used += d.GetMemLocal().GetUsed() + d.GetMemChildren().GetUsed();
    PeakCurrent += d.GetMemLocal().GetPeakUsed() + d.GetMemChildren().GetPeakUsed();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);
}

MemStatistic& 
MemStatistic::operator=( const FxStat_HeapEntryDesc& d )
{
    const FxStat_HeapEntryDesc::HistoryType&  history = d.GetHistory();

    AllocN = d.GetAllocCount();
    Allocated = history.GetSize() ? history.Back().Allocated : 0;
    Used = history.GetSize() ? history.Back().GetUsed() : 0;
    PeakUsed = d.GetFilePeak();

    return *this;
}

void MemStatistic::operator+=( const FxStat_HeapEntryDesc& d )
{
    const FxStat_HeapEntryDesc::HistoryType&  history = d.GetHistory();

    AllocN += d.GetAllocCount();
    Allocated += history.GetSize() ? history.Back().Allocated : 0;
    Used += history.GetSize() ? history.Back().GetUsed() : 0;
    PeakCurrent += d.GetFilePeak();
    PeakUsed = Alg::Max(PeakUsed, PeakCurrent);
}

void MemStatistic::Init()
{
    AllocN = 0;
    Allocated = 0;
    Used = 0;
    // We keep PeakUsed unchanged.
    PeakCurrent = 0;
}

////////////////////////////////////////////////////////////////////////////////
// ***** Heap Support classes - FxStat_HeapDesc and FxStat_HeapEntryDesc


FxStat_HeapDesc::FxStat_HeapDesc(FxStat_HeapDesc* pparent, MemoryHeap* pheap)
    : Sums(Memory::GetHeapByAddress(GetThis())), Peaks(Memory::GetHeapByAddress(GetThis())),
      pHeap_Unsafe(pheap), pParent(pparent)
{
    HeapName = pheap->GetName();
    HeapFlags= pheap->GetFlags();
    
    // Non Thread Safe heaps must be queried separately, since they can not be traversed
    // by different thread. However, we allow this for now since FxPlayer only calls
    // Advance on its main thread, which is the same as the HUD.
 //   Lockable = pheap->IsThreadSafe();
    Lockable = true;
    Frames   = 0;
    ExpireCounter = GetExpireMax();

    AllocCount       = 0;

    Updated = false;

    if (pParent)
        pParent->ChildHeaps.PushBack(this);

}

FxStat_HeapDesc::~FxStat_HeapDesc()
{
    if (pParent)
        pParent->RemoveChildHeap(this);
    SF_ASSERT(ChildHeaps.GetSize() == 0);
}


void FxStat_HeapDesc::Revive(MemoryHeap* pheap)
{
    SF_ASSERT(pHeap_Unsafe == 0);
    pHeap_Unsafe  = pheap;
    ExpireCounter = GetExpireMax();
}


void FxStat_HeapDesc::UpdateCollectStats(const FxStatUpdate& update)
{    
    if (!Updated)
    {
        // Collect and AddRef all of the child heaps, so that we can further
        // iterate through them to collect stats. This is done only once per Update,
        // even if there are multiple tree items describing the heap.
        struct HVisitor : MemoryHeap::HeapVisitor
        {
            ArrayLH<Ptr<MemoryHeap> > *pHeaps;
            HVisitor(ArrayLH<Ptr<MemoryHeap> > *pheaps) : pHeaps(pheaps) { }

            virtual void Visit(MemoryHeap*, MemoryHeap *pchildHeap)
            { pHeaps->PushBack(pchildHeap); }
        };

        HVisitor    visitor(&TempUpdateHeaps);    
        if (pHeap_Unsafe)
            pHeap_Unsafe->VisitChildHeaps(&visitor);  
        

        // Accumulate stats or refresh their groups, depending on
        // the type of update this is.

        if (update.IsFrame())
        {
            // Accumulate counters.
            if (Lockable && !IsDead())
            {
                StatBag stats(Memory::GetHeapByAddress(this));
                pHeap_Unsafe->GetStats(&stats);

                Sums += stats;
                Peaks.SetMax(stats);
                Frames++;            
            }
        }
        else
        {
            Sums.UpdateGroups();
            Peaks.UpdateGroups();

            StatInfo sumStat;
            if (Sums.GetStat(&sumStat, Stat_Mem))
            {
                MemoryStat* psum  = sumStat.ToMemoryStat();

                MemLocal = MemStatistic(*psum) / Frames;
                AllocCount = Frames ? (psum->GetAllocCount()/Frames) : 0;
            }
            else
            {
                MemLocal.Init();
                AllocCount = 0;
            }

            StatInfo peakStat;
            if (Peaks.GetStat(&peakStat, Stat_Mem))
            {
                MemoryStat* ppeak  = peakStat.ToMemoryStat();

                MemLocal.PeakUsed = Alg::Max(MemLocal.PeakUsed, ppeak->GetUsed());
            }
        }
    }

    // NOTE: 'Updated' flag is set later in UpdateChildTotals.       
}


void FxStat_HeapDesc::UpdateChildTotals(const FxStatUpdate& update)
{  
    if (!Updated && update.IsTick())
    {     
        // Calculate total memory use across child heaps. Since UpdateChild totals must be
        // called after children have been updated, child MemUseCumulative values are ready.

        MemChildren.Init();
        MemDebugChildren.Init();

        unsigned i;
        for (i=0; i< ChildHeaps.GetSize(); i++)
        {
            FxStat_HeapDesc* pch = ChildHeaps[i];
            MemStatistic childMem;

            childMem.SetWithDebud(*pch);

            if (pch->HeapFlags & MemoryHeap::Heap_UserDebug)            
                MemDebugChildren += childMem;
            else
                MemChildren += childMem;

            AllocCount += pch->AllocCount;
        }

        if (!pHeap_Unsafe && (ExpireCounter > 0))
            ExpireCounter--;
    }

    Updated = true;
}


void FxStat_HeapDesc::FinalizeUpdate(const FxStatUpdate& update)
{
    // Clear Update flag for next Update and release heap references.
    Updated = false;
    TempUpdateHeaps.Clear();

    // Reset statistics. This needs to be done after all traversals
    // since multiple Items can access the Stats update.
    if (update.IsTick())
    {
        Sums.Clear();
        Peaks.Clear();
        Frames = 0;
    }
    
    for (unsigned i=0; i< ChildHeaps.GetSize(); i++)
        ChildHeaps[i]->FinalizeUpdate(update);
}




// Helper to remove child heap.
void FxStat_HeapDesc::RemoveChildHeap(FxStat_HeapDesc* pdesc)
{
    SF_ASSERT(pdesc->pParent == this);

    unsigned i;
    for (i=0; i<ChildHeaps.GetSize(); i++)
    {
        if (ChildHeaps[i] == pdesc)
        {
            ChildHeaps.RemoveAt(i);
            break;
        }
    }    
}

FxStat_HeapDesc* FxStat_HeapDesc::CreateChildHeapDesc(MemoryHeap* pheap)
{
    unsigned i;
    for (i=0; i<ChildHeaps.GetSize(); i++)
    {
        if ((ChildHeaps[i]->pHeap_Unsafe == pheap) &&
            (ChildHeaps[i]->HeapName == pheap->GetName()))
        {
            ChildHeaps[i]->AddRef();
            return ChildHeaps[i];
        }
    }
    
    return SF_HEAP_AUTO_NEW(this) FxStat_HeapDesc(this, pheap);    
}

// Creates an entry for the id, matching one if it already exists.
FxStat_HeapEntryDesc* FxStat_HeapDesc::CreateEntry(unsigned id)
{    
    FxStat_HeapEntryDesc* p = 0;
    if (Entries.Get(id, &p))
        p->AddRef();
    else
    {   
        p = SF_HEAP_AUTO_NEW(this) FxStat_HeapEntryDesc(this, id);
        Entries.Set(id, p);        
    }
    
    return p;
}


// Updates history of an entry by recording Stat values or 0. If 0 is recorded,
// history expiration counter starts being decremented. Returns true if the
// entry has not expired, false if it has (and should be removed).
bool FxStat_HeapDesc::UpdateEntry(FxStat_HeapEntryDesc* pentry)
{
    SF_ASSERT(pentry);
    SF_ASSERT(& const_cast<const FxStat_HeapEntryDesc*>(pentry)->GetHeapDesc() == this);

    StatInfo sumStat;
    StatInfo peakStat;

    unsigned  id    = pentry->GetId();    
    bool      hasId = Sums.GetStat(&sumStat, id);

    MemStatistic item;

    AllocCount = 0;

    if (hasId)
    {
        // Add stat to history, reset expire counter.
        Peaks.GetStat(&peakStat, id);

        MemoryStat* psum  = sumStat.ToMemoryStat();
        MemoryStat* ppeak = peakStat.ToMemoryStat();

        item.Allocated  = Frames ? (psum->GetAllocated()/Frames) : 0;
        item.Used       = Frames ? (psum->GetUsed()/Frames) : 0;
        item.PeakUsed   = ppeak->GetUsed();

        AllocCount = Frames ? (psum->GetAllocCount()/Frames) : 0;
    }

    // Update history.    
    pentry->AddHistory(item);
    pentry->SetAllocCount(AllocCount);
    pentry->SetFilePeak(Alg::Max(item.PeakUsed, pentry->GetFilePeak()));

    if ((item.Allocated == 0) && (item.Used == 0) && (item.PeakUsed == 0))
        pentry->DecExpireCounter();
    else
        pentry->SetExpireCounter(GetExpireMax());

    return (pentry->GetExpireCounter() != 0);
}

bool FxStat_HeapDesc::UpdateEntry(FxHeapSummaryElementStatItem* pentry) const
{
    SF_ASSERT(pentry);

    StatInfo sumStat;
    StatInfo peakStat;

    unsigned  id    = pentry->GetStatId();    
    bool      hasId = Sums.GetStat(&sumStat, id);

    if (hasId)
    {
        // Add stat to history, reset expire counter.
        Peaks.GetStat(&peakStat, id);

        CounterStat* psum  = sumStat.ToCounterStat();
        CounterStat* ppeak = peakStat.ToCounterStat();

        pentry->SetCounter(Frames ? (psum->GetCount()/Frames) : 0);
        pentry->SetPeakCounter(Alg::Max(pentry->GetPeakCounter(), ppeak->GetCount()));
    }

    return true;
}


////////////////////////////////////////////////////////////////////////////////
// ***** FxStat_HeapEntryDesc

FxStat_HeapEntryDesc::FxStat_HeapEntryDesc(FxStat_HeapDesc* pheapDesc, unsigned id)
: pHeapDesc(pheapDesc), Id(id),
  ExpireCounter(pHeapDesc->GetExpireMax()),
  History(pHeapDesc->GetExpireMax()),
  AllocCount(0), FilePeak(0)
{

}

// Released entry removes itself from parent hash-table.
FxStat_HeapEntryDesc::~FxStat_HeapEntryDesc()
{
    pHeapDesc->RemoveEntry(Id);
}



////////////////////////////////////////////////////////////////////////////////
// ***** FxHeapStatItem


FxHeapStatItem::FxHeapStatItem(FxStatItem* pparent, FxStat_HeapDesc *pheapDesc,
                               const char* pname, bool showDebugHeaps)
: BaseClass(pparent), ShowDebugHeaps(showDebugHeaps), pHeapDesc(pheapDesc)
{

    // Don't populate children yet - this will be done in the first
    // call to Update(Update_Init).

    // TBD: Calling UpdateEntry to create an element here would seem incorrect
    //      as it would start it off in Expiring state with a history tick.
    //     - No history tick is needed here?


//    Name = pHeapDesc->HeapName;    
    SetName(String("[Heap] ", pname));

    //pHeapInfoItem0 = 
    //    *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|>  Count");
    //AddChild(pHeapInfoItem0);
    
    // Heap summary ...
    {
        Ptr<FxStat_HeapEntryDesc> pentry = *pHeapDesc->CreateEntry(StatHeap_Summary);
        pHeapElements0 = *SF_HEAP_AUTO_NEW(this) FxHeapSummaryRootStatItem(this, StatHeap_Summary, pentry);
#ifndef SF_BUILD_DEBUG
        // Expand heap summary for the global heap ...
        pHeapElements0->SetExpanded(true);
#endif
        AddChild(pHeapElements0);    
    }

    // When we first get the item we don't know the average or peak;
    // we can just get a single value.
    //  - The graph can display [No] data <empty history>.
    //  - The item can display the current snapshot.
    //  -  Does that mean that the first history item is special?

    // The heap can already be in the list (if it has been tracked)
    // or not.

#ifdef SF_BUILD_DEBUG
    pHeapInfoItem1 = 
        *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|>  Alloc#|>  Requested|>  Consumed|>  Peak");
    AddChild(pHeapInfoItem1);
    
    if (!showDebugHeaps)
    {
        // Add an element item.
        Ptr<FxStat_HeapEntryDesc> pentry = *pHeapDesc->CreateEntry(Stat_Mem);
        pHeapElements1 = *SF_HEAP_AUTO_NEW(this) FxHeapElementStatItem(this, Stat_Mem, pentry);
        pHeapElements1->SetExpanded(true);
        AddChild(pHeapElements1);    
    }    
#endif

    AddChildArrayList(&ChildHeapItems);
}


FxHeapStatItem::FxHeapStatItem(FxHeapStatItem* pparent, ItemCreateType *pheap)
: BaseClass(pparent), pHeapDesc(0)
{
    SF_ASSERT(pparent);
    pHeapDesc = *pparent->pHeapDesc->CreateChildHeapDesc(pheap);
    SF_ASSERT(pHeapDesc);
    SetName(String("[Heap] ", pHeapDesc->GetName().ToCStr())); 
    
    ShowDebugHeaps = pparent->ShowDebugHeaps;

    // Heap summary ...
    {
        Ptr<FxStat_HeapEntryDesc> pentry = *pHeapDesc->CreateEntry(StatHeap_Summary);
        //pHeapElements0 = *SF_HEAP_AUTO_NEW(this) FxHeapElementStatItem(this, GStatHeap_Summary, pentry);
        pHeapElements0 = *SF_HEAP_AUTO_NEW(this) FxHeapSummaryRootStatItem(this, StatHeap_Summary, pentry);
#ifndef SF_BUILD_DEBUG
        // Expand heap summary for a child heap ...
        pHeapElements0->SetExpanded(true);
#endif
        AddChild(pHeapElements0);    
    }

#ifdef SF_BUILD_DEBUG
    // Add an element item.
    {
        Ptr<FxStat_HeapEntryDesc> pentry = *pHeapDesc->CreateEntry(Stat_Mem);
        pHeapElements1 = *SF_HEAP_AUTO_NEW(this) FxHeapElementStatItem(this, Stat_Mem, pentry);
        pHeapElements1->SetExpanded(true);
        AddChild(pHeapElements1);
    }
#endif

    AddChildArrayList(&ChildHeapItems);
}


FxHeapStatItem::~FxHeapStatItem()
{    
#ifdef SF_BUILD_DEBUG
    pHeapElements1 = 0;
#endif
    pHeapElements0 = 0;

    // Must clear out child items before descriptor dies.
    ChildHeapItems.Clear();
}


void FxHeapStatItem::Update(const FxStatUpdate& update)
{
    SF_ASSERT(pHeapDesc);

    // First, update our history as necessary, then child items.
    pHeapDesc->UpdateCollectStats(update);

    if (pHeapElements0)
        pHeapElements0->Update(update);

    if (pHeapElements1)
        pHeapElements1->Update(update);
   

    // Copy child heaps array captured by UpdateCollectStats;
    // this is necessary because Fx_MergeItems modified is.
    // No need to AddRef to heap since pHeapDesc->TempUpdateHeaps does so.    
    Array<MemoryHeap*> Heaps;

    // For a dead heap, we'll have an empty list of children.
    for (UPInt iheap = 0; iheap < pHeapDesc->GetHeapCache().GetSize(); iheap++)
    {
        MemoryHeap *pheap = pHeapDesc->GetHeapCache()[iheap];
        bool debugHeap = (pheap->GetFlags() & MemoryHeap::Heap_UserDebug) ? true : false;
                        
        if (debugHeap == ShowDebugHeaps)
            Heaps.PushBack(pHeapDesc->GetHeapCache()[iheap]);
    }
    

    // Go through child items. Several cases are possible:
    //
    //  A. Item is found matching (name) and pointer
    //       - Update stats/history; there is a match.
    //       - THIS item should not be dead.    
    //  B. A dead item is found matching by name.
    //       - Bring the item back to life, update Stat.
    //  C. An item is not found.
    //       - Create a new entry (sort-by name), Update.
    //  D. An item exists in a list which is no longer in the heap.    
    //       - Need to be set as 'dead', and stat/expire counters
    //         updated. If it expires and not shown, remove.

    //  Is this possible: Same pointer, different name?
    //    - Means that heap was destroyed and re-created in the same
    //      place. We will simply treat it as a new item.


    FxArrayMergeAccessor<FxHeapStatItem, FxHeapStatItem,
                         MemoryHeap* > a(this, Heaps);
    Fx_MergeItems(ChildHeapItems, a, update);


    pHeapDesc->UpdateChildTotals(update);

    // Update text to include total memory occupied by heap.
    if (update.IsTick())
    {
        SF::StringDH text(Memory::GetHeapByAddress(this));

        Format(text, " - {0:sep:,}K",
            (pHeapElements0->GetTotal() + 1023) / 1024
            );

        SetText(text);
    }
}

void FxHeapStatItem::Accept(Visitor& v)
{
    for (UPInt i = 0; i < ChildHeapItems.GetSize(); ++i)
        ChildHeapItems[i]->Accept(v);

    if (pHeapElements0)
        pHeapElements0->Accept(v);

    if (pHeapElements1)
        pHeapElements1->Accept(v);

    v.Visit_FxHeapStatItem(*this);
}


////////////////////////////////////////////////////////////////////////////////
// ***** FxHeapElementStatItem

FxHeapElementStatItem::FxHeapElementStatItem(FxStatItem* pparent, unsigned groupId, FxStat_HeapEntryDesc* pentry)
: FxStatItem(pparent), GroupId(groupId), pHeapEntry(pentry)
{
    SetFlags(SF_Graphable);

    // We may not need to do this loop if we implement it in update. 
    //
    // The only difference this should have is with respect to Hidden attribute.
    //   - New child branch items should not be hidden before they are expanded.
    //   - Hidden applies only to newly added child items while
    //     sub-tree is displayed.
    //     - But, for 'removal' logic "Not Shown" attribute may be more useful,
    //       which indicates that the item is not a part of the tree.

    SetHidden(false);
 
    // We need to obtain the Stat from the bag to
    // implement text rendering.

    SetName(SF::StringDH(Memory::GetHeapByAddress(this), StatDesc::GetDesc(pentry->GetId())->GetName()));

}


UPInt FxHeapElementStatItem::GetChildCount() const
{
    return Children.GetSize();
}
FxStatItem* FxHeapElementStatItem::GetChild(UPInt index) const
{
    return Children[index];
}
  

void FxHeapElementStatItem::Update(const FxStatUpdate& update)
{
    // Heap items only handle HistoryTicks. Individual frame updates
    // are filtered out on the heap level.
    if (update.IsFrame())
        return;

    // Update the HeapDesc.
    SF_ASSERT(pHeapEntry);
    FxStat_HeapDesc& heapDesc  = pHeapEntry->GetHeapDesc();
    heapDesc.UpdateEntry(pHeapEntry);


    // Text should be formed out of:
    //  - Alloc count
    //  - 

    const FxStat_HeapEntryDesc::HistoryType& history = GetHeapEntryDesc().GetHistory();

    String text;
    Format(text, "|8>  {0:sep:,}|11>  {1:sep:,}|11>  {2:sep:,}|11>  {3:sep:,}",
        pHeapEntry->GetAllocCount(),
        (history.GetSize() ? history.Back().Allocated : 0),
        (history.GetSize() ? history.Back().Used : 0),
        pHeapEntry->GetFilePeak()
        );
    SetText(text);


    // Traverse the list of stats and merge in any into our list;
    // these should still be hidden.    
    // Any items no longer there would get a 0 (decrement expire).
    //  - Any item completely expired can be removed if it's not shown.

    StatDesc::Iterator istats = StatDesc::GetDesc(GetHeapEntryId())->GetChildIterator();
    //unsigned            childIndex = 0;    

    while(!istats.IsEnd())
    {
        const StatDesc* pchildDesc = *istats;

        StatInfo sinfo;

        // TBD: Optimize this with external childIndex; should be possible
        //      due to sorted id order. Note that child check conditionals would
        //      need to be changed below.
        unsigned ichild;
        for (ichild = 0; ichild < Children.GetSize(); ichild++)
            if (Children[ichild]->GetHeapEntryId() == pchildDesc->GetId())
                break;

        // If item is in the StatBag
        if (heapDesc.GetSums().GetStat(&sinfo, pchildDesc->GetId()))
        {                       
            // If the item isn't in the list, add it.
            if (ichild == Children.GetSize())
            {               
                Ptr<FxStat_HeapEntryDesc> pentry = *heapDesc.CreateEntry(pchildDesc->GetId());

                // Create an item for it.
                Ptr<FxHeapElementStatItem> pitem = 
                    *SF_HEAP_AUTO_NEW(this) FxHeapElementStatItem(this, pchildDesc->GetId(), pentry);
                //pitem->SetHidden(update.IsInit() ? false : true);

                if (pitem->GetParent()->IsShown() && pitem->GetParent()->IsExpanded())
                    pitem->SetHidden(true);
                else
                    pitem->SetHidden(false);
                Children.InsertAt(ichild, pitem);
                //if (IsExpanded() && update.pTreeUpdate)
                //    update.pTreeUpdate->Insert(pitem);
                FxStatItem* pparent = pitem->GetParent();
                if(pparent && pparent->IsShown() && pparent->IsExpanded())
                    pparent->SetChanged(true);
            }

            // Record statistic history point.
            Children[ichild]->Update(update);

        }
        else
        {            
            // If item is in the list, we still need to update it. For stat items
            // this will add zeros to history and ultimately expire the item,
            // causing it to be removed from the list.
            
            
            if (ichild != Children.GetSize())
            {
                // Calling update will decrement Expire counter internally.
                FxHeapElementStatItem* pchild = Children[ichild];
                pchild->SetDead(true);
                pchild->Update(update);

                //SF_ASSERT(pentry == pchild->pHeapEntry);
                if ( pchild->IsExpired())
                {
                    // Removal will cause HeapEntry to die
                    // if it isn't referenced elsewhere.
                    //if (IsExpanded() && update.pTreeUpdate)
                    //    update.pTreeUpdate->Remove(pchild);

                    if (pchild->IsShown())
                        pchild->SetChanged(true);
                    else
                        Children.RemoveAt(ichild);
                }
            }
        }  
        ++ istats;
    }
}

//UPInt FxHeapElementStatItem::GetPointsCount()
//{
//    return pHeapEntry->GetHistory().GetSize();
//}

//void FxHeapElementStatItem::GetGraphData(UPInt index, Array<Double> *pdata) const
//{
//    pdata->Clear();
//    const FxStat_HeapEntryDesc::HistoryType& history = pHeapEntry->GetHistory();
//
//    pdata->PushBack(history.GetSize() > index ? static_cast<Double>(history[index].PeakUsed) : 0.0);
//    pdata->PushBack(history.GetSize() > index ? static_cast<Double>(history[index].Used) : 0.0);
//
//    for (unsigned i = 0; i < GetChildCount(); i++)
//    {
//        FxStatItem* pchild = GetChild(i);
//        if(pchild->IsGraphable() && pchild->GetType() == ST_Memory)
//        {
//            pdata->PushBack(pchild->GetItemGraphData(index));
//        }
//    }  
//}

//Double FxHeapElementStatItem::GetItemGraphData( UPInt index ) const
//{
//    SF_ASSERT(pHeapEntry);
//    return static_cast<Double>(pHeapEntry->GetHistory()[index].Used);
//}

//void FxHeapElementStatItem::OnInitGraph()
//{
//    SF_ASSERT(!pGraphInfo);
//    pGraphInfo = *SF_HEAP_AUTO_NEW(this) GraphInfo();
//    Double maxValue = 0;
//    unsigned   graphCount = 0;
//    const FxStat_HeapEntryDesc::HistoryType& history = pHeapEntry->GetHistory();
//    for (unsigned i = 0; i < history.GetSize(); i++)
//        if (history[i].PeakUsed > maxValue)
//            maxValue = static_cast<Double>(history[i].PeakUsed);
//
//    for (unsigned i = 0; i < GetChildCount(); i++)
//    {
//        FxStatItem* pchild = GetChild(i);
//        if(pchild->IsGraphable() && pchild->GetType() == ST_Memory)
//           ++graphCount; 
//    } 
//
//    pGraphInfo->SetScale(ScaleCalc(maxValue));
//    pGraphInfo->SetGraphCount(graphCount + 2); //number of graphable children + Peak and Average
//}

//Double FxHeapElementStatItem::ScaleCalc( Double maxValue )
//{
//    const Double minScale = 16384;
//    Double scale = 0;
//     
//    if (maxValue < minScale)
//        scale = minScale;
//    else
//    {
//        int lowPowerOf2 =  1 << (int)floor(log(maxValue)/log(2.0));
//        if (maxValue < 1.5 * lowPowerOf2)
//            scale = 1.5 * lowPowerOf2;
//        else
//            scale = 2 * lowPowerOf2;
//    }
//
//    return scale;
//}

void FxHeapElementStatItem::Accept( Visitor& v )
{
    // Visit children.
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->Accept(v);
    }

    v.Visit_FxHeapElementStatItem(*this);
}

///////////////////////////////////////////////////////////////////////////////
// ***** FxSummaryStatItem::GSummaryVisitor

void FxSummaryStatItem::GSummaryVisitor::Visit_FxHeapElementStatItem(FxHeapElementStatItem& obj)
{
    // Each FxHeapElementStatItem represents a single memory StatId in a heap.
    switch (obj.GetStatId())
    {
    case Stat_Image_Mem:
        if (obj.GetHeapEntryDesc().GetHeapDesc().GetName() == "_Font_Cache")
            return;

        ImageStat += obj.GetHeapEntryDesc();
        break;
#ifdef GFX_ENABLE_SOUND
    case Stat_Sound_Mem:
        SoundStat += obj.GetHeapEntryDesc();
        break;
#endif
#ifdef GFX_ENABLE_VIDEO
    case Stat_Video_Mem:
        VideoStat += obj.GetHeapEntryDesc();
        break;
#endif
    default:
        break;
    }
}

void FxSummaryStatItem::GSummaryVisitor::Visit_FxHeapSummaryElementStatItem(FxHeapSummaryElementStatItem& obj)
{
    if (obj.GetStatId() != StatHeap_LocalFootprint)
        return;

    const SF::StringLH& name = obj.GetParent().GetHeapEntryDesc().GetHeapDesc().GetName();

    if (name.GetSize() < 2)
        return;

    switch (name[0])
    {
    case '_':
        switch (name[1])
        {
        case 'F':
            if (name == "_Font_Cache")
                FontStat += obj.GetCounter();
            break;
        case 'M':
            if (name == "_Mesh_Cache")
                MeshCacheStat += obj.GetCounter();
            break;
        default:
            break;
        }
        break;
    case 'M':
        if (SFstrncmp(name.ToCStr(), "MovieData", 9) == 0 ||
            SFstrncmp(name.ToCStr(), "MovieDef", 8) == 0
            )
        {
            MovieDataStat += obj.GetCounter();
        } else if (SFstrncmp(name.ToCStr(), "MovieView", 9) == 0)
        {
            MovieViewStat += obj.GetCounter();
        }
        break;
    default:
        break;
    }
}


FxSummaryStatItem::GSummaryVisitor& FxSummaryStatItem::GSummaryVisitor::Init()
{
    ImageStat.Init();
    SoundStat.Init();
    VideoStat.Init();
    MeshCacheStat.Init();
    FontStat.Init();
    MovieDataStat.Init();
    MovieViewStat.Init();

    return *this;
}


///////////////////////////////////////////////////////////////////////////////
// ***** FxSummaryStatItem::StatItem

FxSummaryStatItem::StatItem::StatItem( 
    FxStatItem* pparent, 
    const String& name,
    FxSummaryStatItem::GSummaryVisitor& v, 
    FxSummaryStatItem::StatFuncType func 
    ) 
: FxStatItem(pparent), Name(name), pVisitor(&v), Function(func)
{
    SetName(Name);
}


void FxSummaryStatItem::StatItem::Update(const FxStatUpdate& update)
{
    // Update string once a second.
    if (update.IsTick())
    {
        const MemStatistic& stst = (pVisitor->*Function)();

        SF::StringBuffer text;
        //Format(text, "|8>|11>  {0:sep:,}K|11>  {1:sep:,}K|11>  {2:sep:,}K",
        Format(text, "|8>|11>|11>  {0:sep:,}K|11>  {1:sep:,}K",
            //(stst.Allocated + 1023) / 1024,
            (stst.Used + 1023) / 1024,
            (stst.PeakUsed + 1023) / 1024
            );
        SetText(text);
    }
}


///////////////////////////////////////////////////////////////////////////////
// ***** FxSummaryStatItem

FxSummaryStatItem::FxSummaryStatItem(
    FxStatItem* pparent, 
    const char* pname
    )
    : BaseClass(pparent)
{
    SetExpanded(true);
    SetName(pname);

    pHeapInfoItem1 = 
        //*SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|>  Alloc#|>  Requested|>   PeakUsed");
        //*SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|8>|11>  Requested|>  Consumed|>   Peak");
        *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|8>|11>|11>  Consumed|>   Peak");
    AddChild(pHeapInfoItem1);

    // Add an element item.
    Ptr<StatItem> statItem;

#ifdef SF_BUILD_DEBUG
    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Image", Visitor, &GSummaryVisitor::GetImageStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

#ifdef GFX_ENABLE_SOUND
    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Sound", Visitor, &GSummaryVisitor::GetSoundStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);
#endif

#ifdef GFX_ENABLE_VIDEO
    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Video", Visitor, &GSummaryVisitor::GetVideoStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);
#endif
#endif // SF_BUILD_DEBUG

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Movie View", Visitor, &GSummaryVisitor::GetMovieViewStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Movie Data", Visitor, &GSummaryVisitor::GetMovieDataStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Mesh Cache", Visitor, &GSummaryVisitor::GetMeshCacheStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Font Cache", Visitor, &GSummaryVisitor::GetFontStat);
    ChildItems.PushBack(statItem);
    AddChild(statItem);
}

FxSummaryStatItem::~FxSummaryStatItem()
{
}

void FxSummaryStatItem::Update(const FxStatUpdate& update)
{
    for (UPInt i = 0; i < ChildItems.GetSize(); ++i)
    {
        ChildItems[i]->Update(update);
    }
}

///////////////////////////////////////////////////////////////////////////////
// ***** FxSummaryStatItem
FxSystemSummaryStatItem::FxSystemSummaryStatItem( FxStatItem* pparent, const char* pname ) : BaseClass(pparent)
{
    SetName(pname);

    pHeapInfoItem = 
        *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|8>|11>|11>  Current|>   Peak");
    AddChild(pHeapInfoItem);

    Ptr<StatItem> statItem;
    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "System Memory Footprint",
        StatInfo, &StatInfoProvider::GetSysMemFootprint);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "System Memory Used Space",
        StatInfo, &StatInfoProvider::GetSysMemUsedSpace);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Bookkeeping Footprint",
        StatInfo, &StatInfoProvider::GetBookkeepingFootprint);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Bookkeeping Used Space",
        StatInfo, &StatInfoProvider::GetBookkeepingUsedSpace);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Debug Info Footprint",
        StatInfo, &StatInfoProvider::GetDebugInfoFootprint);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "Debug Info Used Space",
        StatInfo, &StatInfoProvider::GetDebugInfoUsedSpace);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "HUD Footprint",
        StatInfo, &StatInfoProvider::GetUserDebugFootprint);
    ChildItems.PushBack(statItem);
    AddChild(statItem);

    statItem = *SF_HEAP_AUTO_NEW(this) StatItem(this, "HUD Used Space",
        StatInfo, &StatInfoProvider::GetUserDebugUsedSpace);
    ChildItems.PushBack(statItem);
    AddChild(statItem);
}

void FxSystemSummaryStatItem::Update( const FxStatUpdate& update )
{
    StatInfo.Update();
    for (UPInt i = 0; i < GetChildCount(); i++)
        GetChild(i)->Update(update);

}

FxSystemSummaryStatItem::StatItem::StatItem( FxStatItem* pparent, const String& name, const StatInfoProvider& statInfo, StatFuncType func )
: FxStatItem(pparent), GetStat(func), pStatInfo(&statInfo),  Current(0), Peak(0)
{
    SF_ASSERT(pStatInfo);
    SetName(name);
}

void FxSystemSummaryStatItem::StatItem::Update( const FxStatUpdate& update)
{
    Current = (pStatInfo->*GetStat)();
    if (Current > Peak)
        Peak = Current;

    if (update.IsTick())
    {
        SF::StringBuffer text;
        Format(text, "|8>|11>|11>  {0:sep:,}K|11>  {1:sep:,}",
            (Current + 1023) / 1024,
            (Peak + 1023) / 1024
            );
        SetText(text);
    }

}

///////////////////////////////////////////////////////////////////////////////
FxHeapSummaryRootStatItem::FxHeapSummaryRootStatItem(
    FxStatItem* pparent, 
    unsigned groupId,
    FxStat_HeapEntryDesc* pentry
    ) 
: BaseClass(pparent)
, GroupId(groupId)
, Frames(0)
, Total(0)
, pHeapEntry(pentry)
{
    SetName(StatDesc::GetDesc(GroupId)->GetName());

    //SetExpanded(true);

    //pHeapInfoItem = 
    //    *SF_HEAP_AUTO_NEW(this) FxMessageStatItem(this, "", "|>  Average|>  Peak");
    //AddChild(pHeapInfoItem);
}

void FxHeapSummaryRootStatItem::Update( const FxStatUpdate& update)
{
    switch (update.GetType())
    {
    case FxStatUpdate::Update_Init:
        // Create children.
        {
            StatDesc::Iterator iter = StatDesc::GetGroupIterator(GroupId)->GetChildIterator();
            while (!iter.IsEnd())
            {                    
                Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxHeapSummaryElementStatItem(this, *this, iter.GetId()));
                AddChild(Children.Back());

                ++iter;
            }
        }
        break;
    case FxStatUpdate::Update_Tick:
        SetTotal(0);

        // Update all children.
        for (unsigned i = 0; i < Children.GetSize(); ++i)
            Children[i]->Update(update);

        break;
    case FxStatUpdate::Update_Frame:
        // Hack for a logical error.
        // FxStatUpdate::Update_Init is not called for newly created heaps.
        if (Children.GetSize() == 0)
        {
            StatDesc::Iterator iter = StatDesc::GetGroupIterator(GroupId)->GetChildIterator();
            while (!iter.IsEnd())
            {                    
                Children.PushBack(*SF_HEAP_AUTO_NEW(this) FxHeapSummaryElementStatItem(this, *this, iter.GetId()));
                AddChild(Children.Back());

                ++iter;
            }
        }
        break;
    }
}

void FxHeapSummaryRootStatItem::Accept(Visitor& v)
{
    // Visit children.
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->Accept(v);
    }

    v.Visit_FxHeapSummaryRootStatItem(*this);
}

///////////////////////////////////////////////////////////////////////////////
FxHeapSummaryElementStatItem::FxHeapSummaryElementStatItem( 
    FxStatItem* pparent,
    ParentClass& parent, 
    unsigned statId
    ) 
: BaseClass(pparent)
, Parent(parent)
, StatId(statId)
, Counter(0)
, PeakCounter(0)
{
    SetName(StatDesc::GetDesc(StatId)->GetName());
    SetExpanded(true);
}

void FxHeapSummaryElementStatItem::Update( const FxStatUpdate& update )
{
    if (update.IsTick())
    {
        const FxStat_HeapDesc& heapDesc = GetHeapEntryDesc().GetHeapDesc();
        heapDesc.UpdateEntry(this);

        if (StatId == StatHeap_TotalFootprint)
            GetParent().SetTotal(Counter);

        String text;
        Format(text, "|8>|11>|11>  {0:sep:,}|11>  {1:sep:,}",
            Counter,
            PeakCounter
            );
        SetText(text);
    }
}

void FxHeapSummaryElementStatItem::Accept(Visitor& v)
{
    v.Visit_FxHeapSummaryElementStatItem(*this);
}

///////////////////////////////////////////////////////////////////////////////
// ***** MemoryRootStatItem

FxMemoryRootStatItem::FxMemoryRootStatItem(FxGlobalStatTracker* pstatTracker)
: BaseClass(NULL) // It is a parent node.
, pStatTracker(pstatTracker)
, Enabled(true)
{
    SetExpanded(true);
    // Check whether memory statistic is available or not.
    {
        MemoryHeap* globalHeap = Memory::GetGlobalHeap();

        StatBag tmpStats(globalHeap);
        globalHeap->GetStats(&tmpStats);
        Enabled = true;
    }

    if (IsEnabled())
    {
        FxStat_HeapDesc* pheapDesc = pstatTracker->GetGlobalHeapDesc();
    
        pSystemSummary = *SF_HEAP_AUTO_NEW(this) FxSystemSummaryStatItem(this, "System Summary");
        AddChild(pSystemSummary);

        pSummary = *SF_HEAP_AUTO_NEW(this) FxSummaryStatItem(this, "Summary");
        AddChild(pSummary);

        pHeaps = *SF_HEAP_AUTO_NEW(this) FxHeapStatItem(this, pheapDesc, pheapDesc->GetName().ToCStr(), false);
        AddChild(pHeaps);

        //pDebugHeaps = *SF_HEAP_AUTO_NEW(this) FxHeapStatItem(this, pheapDesc, "Debug Heaps", true);
        //AddChild(pDebugHeaps);
    }
}

void FxMemoryRootStatItem::Update(const FxStatUpdate& update)
{
    if (IsEnabled())
    {
        pHeaps->Update(update);
        //pDebugHeaps->Update(update);

        pHeaps->Accept(pSummary->GetVisitor().Init());
        pSummary->Update(update);
        pSystemSummary->Update(update);
        
        MemoryHeap::RootStats stats;
        Memory::pGlobalHeap->GetRootStats(&stats);
        SF::StringBuffer text;
        Format(text,
            "  {0:sep:,}K / {1:sep:,}K",
            (stats.SysMemUsedSpace - stats.DebugInfoFootprint - stats.UserDebugFootprint + 1023) / 1024,
            (stats.SysMemFootprint - stats.DebugInfoFootprint - stats.UserDebugFootprint + 1023) / 1024
            );
        SetText(text);
    }
}

// Virtual method.
void FxMemoryRootStatItem::Accept( Visitor& v )
{
    if (IsEnabled())
        pHeaps->Accept(v);
}

// Virtual method.
//void FxMemoryRootStatItem::GetText( String* pstring )
//{
//    if (pstring)
//        pstring->Clear();
//}

// Virtual method.
void FxMemoryRootStatItem::GetName( String* pstring )
{
    if (IsEnabled())
        *pstring = "Memory";
    else
        *pstring = "Memory stats not supported in this build";
}
