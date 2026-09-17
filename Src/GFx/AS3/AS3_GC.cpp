/**************************************************************************

Filename    :   AS3_GC.cpp
Content     :   
Created     :   Jan, 2010
Last rev    :   Sep 5, 2012
Authors     :   Artem Bolgar, Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_GC.h"

namespace Scaleform { namespace GFx { namespace AS3
{

// after 5th run of GC generations will be upgraded
#define SF_GC_DEFAULT_RUNS_TO_UPG_GENS  5 

// Young objs are collected each 5th run of GC 
#define SF_GC_DEFAULT_RUNS_TO_COLLECT_YOUNG  5

// Young objs are collected each 5th run of GC 
#define SF_GC_DEFAULT_RUNS_TO_COLLECT_OLD  10

// Max root count
#define SF_GC_DEFAULT_MAX_ROOT_COUNT 1000

// max num of roots before collection, off by default
#define SF_GC_DEFAULT_MAX_FRAMES_BTW_COLL 0

///////////////////////////////////////////////////////////////////////////
//
ASRefCountCollector::ASRefCountCollector()
{
    FrameCnt            = 0;
    PeakRootCount       = 0;
    LastRootCount       = 0;
    LastCollectedRoots  = 0;
    LastPeakRootCount   = 0;
    TotalFramesCount    = 0;
    LastCollectionFrameNum = 0;
    CollectionScheduledFlags = 0;
    SuspendCnt           = 0;
    
    RunsCnt              = 0;
    RunsToUpgradeGen     = 0;
    RunsToCollectYoung   = 0;
    RunsToCollectOld     = 0;

    SetParams(~0u, ~0u);
}

void ASRefCountCollector::SetParams(unsigned frameBetweenCollections, unsigned maxRootCount,
                                    unsigned runsToUpgradeGen, unsigned runsToCollectYoung,
                                    unsigned runsToCollectOld)
{
    // max num of roots before collection
    if (frameBetweenCollections != ~0u)
        MaxFramesBetweenCollections = frameBetweenCollections;
    else
        MaxFramesBetweenCollections = SF_GC_DEFAULT_MAX_FRAMES_BTW_COLL;
    
    // force collection for every N-frames
    if (maxRootCount != ~0u)
        PresetMaxRootCount  = MaxRootCount = maxRootCount;
    else
        PresetMaxRootCount  = MaxRootCount = SF_GC_DEFAULT_MAX_ROOT_COUNT;

    if (runsToUpgradeGen != ~0u)
        RunsToUpgradeGen   = runsToUpgradeGen;
    else 
        RunsToUpgradeGen   = SF_GC_DEFAULT_RUNS_TO_UPG_GENS;
    
    if (runsToCollectYoung != ~0u)
        RunsToCollectYoung = runsToCollectYoung;
    else
        RunsToCollectYoung = SF_GC_DEFAULT_RUNS_TO_COLLECT_YOUNG;

    if (runsToCollectOld != ~0u)
        RunsToCollectOld   = runsToCollectOld;
    else
        RunsToCollectOld   = SF_GC_DEFAULT_RUNS_TO_COLLECT_OLD;
}

//////////////////////////////////////////////////////////////////////////
// ASRefCountCollection
//
#ifdef SF_BUILD_DEBUG
//#define SF_TRACE_COLLECTIONS
#endif

unsigned ASRefCountCollector::CheckGenerations(bool* upgradeGen)
{
    SF_ASSERT(upgradeGen);

    unsigned gen = RefCountBaseGC<Mem_Stat>::Gen_NewBorn;
    *upgradeGen = false;
    if (RunsCnt > 0)
    {
        if (RunsCnt % RunsToCollectOld == 0)
            gen = RefCountBaseGC<Mem_Stat>::Gen_Old;
        else if (RunsCnt % RunsToCollectYoung == 0)
            gen = RefCountBaseGC<Mem_Stat>::Gen_Young;

        if (RunsCnt % RunsToUpgradeGen == 0)
            *upgradeGen = true;
    }
    return gen;
}

void ASRefCountCollector::AdvanceFrame
(unsigned* movieFrameCnt, unsigned* movieLastCollectFrame, AmpStats* ampStats)
{
    // Is this the first advance since a collection by a different MovieView?
    if (*movieLastCollectFrame != LastCollectionFrameNum)
    {
        *movieLastCollectFrame = LastCollectionFrameNum;
        *movieFrameCnt = 1;
        return;
    }

    // Make sure we don't collect multiple times per frame
    // in the case where several views share the same GC
    if (*movieFrameCnt < FrameCnt)
    {
        // the calling MovieView is advancing the GC during a frame where
        // the GC has already been advanced more times by different MovieView
        ++(*movieFrameCnt);
        return;
    }

    // Determine if we need to upgrade generations.
    bool upgradeGen = false;
    unsigned gen = CheckGenerations(&upgradeGen);

    unsigned curRootCount = (unsigned)GetRootsCount(gen);
    
    ++TotalFramesCount;
    ++FrameCnt;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);

    // Collection occurs if:
    // 1) if number of root exceeds currently set MaxRootCount;
    // 2) if MaxFramesBetweenCollections is set to value higher than 0 and the
    //    frame counter (FrameCnt) exceeds this value, and number of roots
    //    exceeds PresetMaxRootCount.
    if (!IsSuspended() && ((PresetMaxRootCount != 0 && curRootCount > MaxRootCount) || 
        (MaxFramesBetweenCollections != 0 && 
        FrameCnt >= MaxFramesBetweenCollections && 
        curRootCount > PresetMaxRootCount)))
    {
        ASRefCountCollector::Stats stats(ampStats);
        Collect(gen, upgradeGen, &stats);
        ++RunsCnt;

#ifdef SF_TRACE_COLLECTIONS        
        printf("Collect! Total roots %d, Roots Processed %d, MaxRoots %d, Peak %d, Iterated %d, "
            "Freed %d, Gens %d, frames between %d, upgradeGen '%s'\n", 
            curRootCount, stats.RootsNumber, MaxRootCount, PeakRootCount, stats.ObjectsIteratedNumber, 
            stats.ObjectsFreedTotal, stats.GensNumber, (TotalFramesCount - LastCollectionFrameNum), 
            (upgradeGen)?"true":"false");
#endif

        // If number of roots exceeds the preset max root count then we need to reset the PeakRootCount
        // in order to decrease currently set MaxRootCount.
        if (stats.RootsFreedTotal > PresetMaxRootCount)
        {
            PeakRootCount = curRootCount; // reset peak count
            MaxRootCount = PresetMaxRootCount;
        }

        // MaxRootCount has been updated every collection event
        //MaxRootCount = Alg::Max(PresetMaxRootCount, PeakRootCount - stats.RootsFreedTotal);
        if (curRootCount > stats.RootsFreedTotal)  // avoid unsigned underflow
        {
            MaxRootCount = Alg::Max(MaxRootCount, curRootCount - stats.RootsFreedTotal);
        }

        if (PeakRootCount < (unsigned)(MaxRootCount * 0.7))
            MaxRootCount = (unsigned)(MaxRootCount * 0.7);

#ifdef SF_TRACE_COLLECTIONS        
        SF_ASSERT((int)MaxRootCount >= 0);
        printf("new maxroots %d\n", MaxRootCount);
#endif

        LastCollectionFrameNum = TotalFramesCount;

        FrameCnt          = 0;
        LastPeakRootCount = PeakRootCount;
        LastCollectedRoots= stats.RootsFreedTotal;
    }
    LastRootCount = curRootCount;
    *movieFrameCnt = FrameCnt;
    *movieLastCollectFrame = LastCollectionFrameNum;
}

void ASRefCountCollector::ForceCollect(AmpStats* ampStats, unsigned collectFlags)
{
    if (IsSuspended())
        return;

    bool upgradeGen = false;
    // Determine if we need to upgrade generations.
    unsigned gen = CheckGenerations(&upgradeGen);

    if (collectFlags & (Collect_Shutdown | Collect_Emergency))
    {
        gen = RefCountBaseGC<Mem_Stat>::Gen_Max;
        Flags |= Flags_ForcedCleanup;
    }
    else 
    {
        upgradeGen = false; // do not upgrade gens for non-emergency user-forced collections.
        if (collectFlags & Collect_Full)
            gen = RefCountBaseGC<Mem_Stat>::Gen_Old;
        else if (collectFlags & Collect_Medium)
            gen = RefCountBaseGC<Mem_Stat>::Gen_Young;
        else if (collectFlags & Collect_Quick)
            gen = RefCountBaseGC<Mem_Stat>::Gen_NewBorn;
    }
    
    unsigned curRootCount = (unsigned)GetRootsCount(gen);

    // we do not upgrade generations for force collection (should we?)
    ASRefCountCollector::Stats stats(ampStats);
    Collect(gen, upgradeGen, &stats);
    
    if (collectFlags & (Collect_Shutdown | Collect_Emergency))
        ++RunsCnt;

#ifdef SF_TRACE_COLLECTIONS        
    printf("Forced collect! Total roots %d, Roots Processed %d, MaxRoots %d, Peak %d, Iterated %d, "
           "Freed %d, Gens %d, flags 0x%X, upgradeGen '%s'\n", 
        curRootCount, stats.RootsNumber, MaxRootCount, PeakRootCount, stats.ObjectsIteratedNumber, 
        stats.ObjectsFreedTotal, stats.GensNumber, collectFlags, (upgradeGen)?"true":"false");
#endif
    FrameCnt  = 0;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);
    LastRootCount = curRootCount;

    if (DoesRequireAdditionalCollect())
    {
        collectFlags &= ~(Collect_Shutdown | Collect_Emergency);
        ScheduleCollect(collectFlags);
    }
}

void ASRefCountCollector::ForceEmergencyCollect(AmpStats* ampStats)
{
    if (IsSuspended())
    {
        // GC is suspended. At least, schedule collect here, so it will be called 
        // when GC is re-enabled.
        ScheduleCollect(Collect_Emergency);
        return;
    }
    ForceCollect(ampStats, Collect_Emergency);

    // Reset peak and max root counters.
    PeakRootCount = 0;
    MaxRootCount  = PresetMaxRootCount;
}


///////////////////////////////////////////////////////////////////////////////
void ForEachChild_GC_Internal(RefCountCollector<Mem_Stat>* prcc, 
                              const Value& v, RefCountBaseGC<Mem_Stat>::GcOp op
                              SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    switch(v.GetKind())
    {
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
    case Value::kThunkFunction: // = 12
    case Value::kNamespace:
        // We do not use GetObject here because we handle kNamespace.
        if (v.GetGASRefCountBase())
            (*op)(prcc, (const AS3::RefCountBaseGC<Mem_Stat>**)v.AsGASRefCountBasePtrPtr() SF_DEBUG_ARG(owner));

        break;
    case Value::kThunkClosure:
    case Value::kVTableIndClosure:
        if (v.GetClosure())
            (*op)(prcc, (const AS3::RefCountBaseGC<Mem_Stat>**)v.AsClosurePtrPtr() SF_DEBUG_ARG(owner));

        break;
    default:
        SF_ASSERT(false);
        break;
    }
}

void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, 
                     const ValueArray& v, RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    for(UPInt i = 0; i < v.GetSize(); ++i)
        AS3::ForEachChild_GC(prcc, v[i], op SF_DEBUG_ARG(owner));
}

void ForEachChild_GC(RefCountCollector<Mem_Stat>* prcc, 
                     const ValueArrayDH& v, RefCountBaseGC<Mem_Stat>::GcOp op
                     SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner))
{
    for(UPInt i = 0; i < v.GetSize(); ++i)
        AS3::ForEachChild_GC(prcc, v[i], op SF_DEBUG_ARG(owner));
}


///////////////////////////////////////////////////////////////////////////
GASRefCountBase::~GASRefCountBase()
{
    if (HasWeakRef())
    {
        // This code shouldn't be called when this object is in a kill list.
        RefCountCollector<Mem_Stat>* rcc = GetCollector();
        WeakProxy* wp = NULL;

        // We should always have WeakProxy here.
        if (rcc->WProxyHash.Get(this, &wp) && wp)
        {
            wp->NotifyObjectDied();
            wp->Release();
            rcc->WProxyHash.Remove(this);
        }
    }
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {
