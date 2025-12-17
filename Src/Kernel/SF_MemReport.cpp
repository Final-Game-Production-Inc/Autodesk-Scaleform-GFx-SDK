/**************************************************************************

Filename    :   SF_MemReport.cpp
Content     :   Forming string buffer with memory report.

Created     :   July 15, 2009
Authors     :   Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Memory.h"
#include "SF_String.h"
#include "SF_MsgFormat.h"
#include "SF_RefCount.h"
#include "SF_Alg.h"
#include "SF_Stats.h"
#include "SF_MemItem.h"

namespace Scaleform {


#ifdef SF_ENABLE_STATS

// This class creates the memory report
// It lives here so we don't have to pollute the public MemoryHeap header with internals
//
class StatsUpdate
{
public:
    StatsUpdate() : NextHandle(0) { }
    // Public interface
    // Called by MemoryHeap::MemReport
    void MemReport(MemItem* rootItem, MemoryHeap::MemReportType reportType);

private:
    // Heap visitor that adds all the child heaps to an array
    struct HolderVisitor : public MemoryHeap::HeapVisitor
    {
        Array<MemoryHeap*> Heaps;

        virtual void Visit(MemoryHeap* pParent, MemoryHeap *heap)
        { 
            SF_UNUSED(pParent);
            Heaps.PushBack(heap); 
        }
    };

    // Custom heap visitor class that keeps track of memory per StatID
    class SummaryStatIdVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        SummaryStatIdVisitor(bool debug) : Debug(debug) { }
        void ExcludeHeap(HeapId excludedHeap)
        {
            ExcludedHeaps.PushBack(excludedHeap);
        }
        virtual void Visit(MemoryHeap* parent, MemoryHeap *heap)
        {
            SF_UNUSED(parent);
            if (((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0) == Debug)
            {
                bool exclude = false;
                for (unsigned i = 0; i < ExcludedHeaps.GetSize(); ++i)
                {
                    if (heap->GetId() == static_cast<UPInt>(ExcludedHeaps[i]))
                    {
                        exclude = true;
                        break;
                    }
                }

                if (!exclude)
                {
                    StatBag bag;
                    heap->GetStats(&bag);
                    StatIdBag.Add(bag);

                    heap->VisitChildHeaps(this);
                }
            }
        }
        UPInt GetStatIdMemory(StatDesc::Iterator it) const
        {
            StatInfo statInfo;
            UPInt totalMemory = 0;

            if (StatIdBag.GetStat(&statInfo, it->GetId()))
            {
                Stat::StatValue statValue;
                statInfo.GetStat(&statValue, 0);
                totalMemory = statValue.IValue;
            }

            for (StatDesc::Iterator itChild = it->GetChildIterator(); !itChild.IsEnd(); ++itChild)
            {
                GetStatIdMemory(itChild);
            }

            return totalMemory;
        }
        void UpdateStatGroups()
        {
            StatIdBag.UpdateGroups();
        }
    private:
        StatBag         StatIdBag;
        bool            Debug;
        Array<HeapId>   ExcludedHeaps;
    };


    // Custom heap visitor class that keeps track of Movie, Mesh, and Font memory
    class SummaryMemoryVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        SummaryMemoryVisitor(bool debug) : 
          Debug(debug), 
          MovieViewMemory(0), MovieDataMemory(0),
          OtherMemory(0), VideoMemory(0) {}

        virtual void Visit(MemoryHeap* parent, MemoryHeap *heap)
        {
            SF_UNUSED(parent);
            if (((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0) == Debug)
            {
                switch (heap->GetId())
                {
                case HeapId_MovieView:
                    MovieViewMemory += heap->GetUsedSpace();
                    break;
                case HeapId_MovieData:
                    MovieDataMemory += heap->GetUsedSpace();
                    break;
                case HeapId_Video:
                    VideoMemory += heap->GetTotalUsedSpace();
                    break;
                default:
                    OtherMemory += heap->GetUsedSpace();
                    break;
                }
            }
            heap->VisitChildHeaps(this);
        }
        UPInt           GetMovieViewMemory() const { return MovieViewMemory; }
        UPInt           GetMovieDataMemory() const { return MovieDataMemory; }
        UPInt           GetVideoMemory() const { return VideoMemory; }
        UPInt           GetOtherMemory() const { return OtherMemory; }
    private:
        bool            Debug;
        UPInt           MovieViewMemory;
        UPInt           MovieDataMemory;
        UPInt           OtherMemory;
        UPInt           VideoMemory;
    };

    class HeapStatBagVisitor : public MemoryHeap::HeapVisitor
    {
    public:
        HeapStatBagVisitor(StatBag* statBag) : HeapStatBag(statBag) { }
        virtual void Visit(MemoryHeap*, MemoryHeap* heap)
        {
            if (heap->GetStats(HeapStatBag))
            {
                heap->VisitChildHeaps(this);
            }
        }

        const StatBag* GetStatBag() const { return HeapStatBag; }

    private:
        StatBag* HeapStatBag;
    };

    class HeapTreeCreator : public MemoryHeap::HeapVisitor
    {
    public:
        HeapTreeCreator(MemoryHeap* debugHeap, StatsUpdate* memReporter, UInt32* nextHandle) : 
            CurrentUsedSpaceParent(NULL),
            CurrentUnusedSpaceParent(NULL),
            DebugHeap(debugHeap),
            MemReporter(memReporter),
            NextHandle(nextHandle)
        {
            UsedSpaceRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            GlobalHeap = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            GlobalHeap->Name = "Global Heap";
            MovieViewRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            MovieViewRoot->Name = "Movie View Heaps";
            MovieDataRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            MovieDataRoot->Name = "Movie Data Heaps";
            VideoRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            VideoRoot->Name = "Video Heaps";
            OtherRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            OtherRoot->Name = "Other Heaps";
            UnusedSpaceRoot = *SF_HEAP_NEW(DebugHeap) MemItem((*NextHandle)++);
            UnusedSpaceRoot->Name = "Unused Space";
        }

        ~HeapTreeCreator()
        {
            for (UPInt i = 0; i < MovieViewRoot->Children.GetSize(); ++i)
            {
                MovieViewRoot->SetValue(MovieViewRoot->Value + MovieViewRoot->Children[i]->Value);
            }
            for (UPInt i = 0; i < MovieDataRoot->Children.GetSize(); ++i)
            {
                MovieDataRoot->SetValue(MovieDataRoot->Value + MovieDataRoot->Children[i]->Value);
            }
            for (UPInt i = 0; i < VideoRoot->Children.GetSize(); ++i)
            {
                VideoRoot->SetValue(VideoRoot->Value + VideoRoot->Children[i]->Value);
            }
            for (UPInt i = 0; i < OtherRoot->Children.GetSize(); ++i)
            {
                OtherRoot->SetValue(OtherRoot->Value + OtherRoot->Children[i]->Value);
            }
        }
        
        virtual void Visit(MemoryHeap* parent, MemoryHeap* heap)
        {
            if ((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0)
            {
                return;
            }

            if (parent == NULL)
            {
                CurrentUsedSpaceParent = UsedSpaceRoot;
                CurrentUnusedSpaceParent = UnusedSpaceRoot;

                heap->GetRootStats(&RootHeapStats);
            }

            MemItem* thisUsedItem = CurrentUsedSpaceParent->AddChild((*NextHandle)++, heap->GetName(), static_cast<UInt32>(heap->GetUsedSpace()));
            MemItem* thisUnusedItem = CurrentUnusedSpaceParent->AddChild((*NextHandle)++, heap->GetName(), static_cast<UInt32>(heap->GetFootprint()) - thisUsedItem->Value);
            MemItem* statIdRoot = thisUsedItem;

            switch (heap->GetId())
            {
            case HeapId_MovieView:
                MovieViewRoot->Children.PushBack(thisUsedItem);
                break;
            case HeapId_MovieData:
                MovieDataRoot->Children.PushBack(thisUsedItem);
                break;
            case HeapId_Video:
                VideoRoot->Children.PushBack(thisUsedItem);
                break;
            case HeapId_Global:
                GlobalHeap->SetValue(unsigned(GlobalHeap->Value + heap->GetUsedSpace()));
                statIdRoot = GlobalHeap;
                break;
            default:
                if (parent != NULL && parent->GetId() == HeapId_Global)
                {
                    OtherRoot->Children.PushBack(thisUsedItem);
                }
                break;
            }

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
            StatBag statBag;
            if (statIdRoot == thisUsedItem)
            {
                HeapStatBagVisitor visitor(&statBag);
                visitor.Visit(NULL, heap);
            }
            else
            {
                heap->GetStats(&statBag);
            }
            statBag.UpdateGroups();
            MemReporter->GetStatMemory(StatDesc::GetGroupIterator(Stat_Mem), &statBag, statIdRoot);

#endif
            MemItem* savedCurrentUsedSpaceParent = CurrentUsedSpaceParent;
            MemItem* savedUnusedFootprintParent = CurrentUnusedSpaceParent;
            CurrentUsedSpaceParent = thisUsedItem;
            CurrentUnusedSpaceParent = thisUnusedItem;
            heap->VisitChildHeaps(this);
            CurrentUsedSpaceParent = savedCurrentUsedSpaceParent;
            CurrentUnusedSpaceParent = savedUnusedFootprintParent;
            CurrentUsedSpaceParent->SetValue(CurrentUsedSpaceParent->Value + thisUsedItem->Value);
            CurrentUnusedSpaceParent->SetValue(CurrentUnusedSpaceParent->Value + thisUnusedItem->Value);
        }
        Ptr<MemItem>    UsedSpaceRoot;
        Ptr<MemItem>    GlobalHeap;
        Ptr<MemItem>    MovieViewRoot;
        Ptr<MemItem>    MovieDataRoot;
        Ptr<MemItem>    VideoRoot;
        Ptr<MemItem>    OtherRoot;
        Ptr<MemItem>    UnusedSpaceRoot;
        MemoryHeap::RootStats   RootHeapStats;           
    private:
        MemItem*        CurrentUsedSpaceParent;
        MemItem*        CurrentUnusedSpaceParent;
        MemoryHeap*     DebugHeap;
        StatsUpdate*    MemReporter;
        UInt32*         NextHandle;
    };

    // This struct holds the memory stats for a file
    // It is part of the heap visitor, FileVisitor, below
    struct FileStats
    {
        FileStats() : TotalMemory(0) { }
        StatBag         Bag;
        UPInt           TotalMemory;
    };

    // Heap visitor that traverses the tree and creates a map of 
    // file names to memory statistics
    // The total memory for each file is not updated by the visitor
    struct FileVisitor : public MemoryHeap::HeapVisitor
    {
        StringHash<FileStats> FileStatsMap;

        // recursively updates the file statistics by examining child heaps
        void UpdateMovieHeap(MemoryHeap* heap, StatBag* fileStatBag)
        {
            if ((heap->GetFlags() & MemoryHeap::Heap_UserDebug) == 0)
            {
                StatBag statBag;
                heap->GetStats(&statBag);
                *fileStatBag += statBag;

                HolderVisitor visitor;
                heap->VisitChildHeaps(&visitor);
                for (UPInt i = 0; i < visitor.Heaps.GetSize(); ++i)
                {
                    UpdateMovieHeap(visitor.Heaps[i], fileStatBag);
                }
            }
        }

        virtual void Visit(MemoryHeap* parent, MemoryHeap* heap)
        { 
            SF_UNUSED(parent);

            if ((heap->GetFlags() & MemoryHeap::Heap_UserDebug) != 0)
            {
                return;
            }

            switch (heap->GetId())
            {
            case HeapId_MovieDef:
            case HeapId_MovieData:
            case HeapId_MovieView:
                {
                    // Recover the file name
                    String heapName = heap->GetName();
                    UPInt heapNameLength = heapName.GetLength();
                    UPInt startIndex = 0;
                    UPInt endIndex = heapNameLength;
                    for (UPInt i = 0; i < heapNameLength; ++i)
                    {
                        UPInt iIndex = heapNameLength - i - 1;
                        if (heapName[iIndex] == '"')
                        {
                            if (endIndex == heapNameLength)
                            {
                                endIndex = iIndex;
                            }
                            else
                            {
                                startIndex = iIndex + 1;
                                break;
                            }
                        }
                    }
                    SF_ASSERT(startIndex < endIndex);
                    String filename = heapName.Substring(startIndex, endIndex);

                    // Add the file name to the map
                    StringHash<FileStats>::Iterator it = FileStatsMap.FindCaseInsensitive(filename);
                    if (it == FileStatsMap.End())
                    {
                        FileStatsMap.SetCaseInsensitive(filename, FileStats());
                        it = FileStatsMap.FindCaseInsensitive(filename);
                    }

                    // update the stats for this file
                    UpdateMovieHeap(heap, &(it->Second.Bag));
                }
                break;
            default:
                break;
            }
        }
    };

    UInt32 GetRoundUpKilobytes(UPInt bytes);
    UInt32 GetNearestKilobytes(UPInt bytes);

    void MemReportHeaps(MemoryHeap* pHeap, MemItem* rootItem, MemoryHeap::MemReportType reportType);
    void GetHeapMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem);
    void GetStatMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem);
    void MemReportFile(MemItem* rootItem, MemoryHeap::MemReportType reportType);
    void MemReportHeapsDetailed(MemItem* rootItem, MemoryHeap* heap);
    void GetFileMemory(StatDesc::Iterator it, FileStats& kFileStats, MemItem* rootItem, MemoryHeap::MemReportType reportType);

    UInt32 NextHandle;
};

void StatsUpdate::MemReport(MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    NextHandle = rootItem->GetMaxId() + 1;
    rootItem->ID = NextHandle++;
    rootItem->StartExpanded = true;

    switch (reportType)
    {
    case MemoryHeap::MemReportHeapDetailed:
        MemReportHeapsDetailed(rootItem, Memory::pGlobalHeap);
        break;

    case MemoryHeap::MemReportFileSummary:
        MemReportFile(rootItem, reportType);
        break;

    default:

        {
            MemoryHeap::RootStats rootStats;
            Memory::pGlobalHeap->GetRootStats(&rootStats);

            Format(rootItem->Name,
                "Memory {0:sep:,}K / {1:sep:,}K",
                GetNearestKilobytes(rootStats.SysMemUsedSpace - rootStats.UserDebugFootprint - 
                rootStats.DebugInfoFootprint),
                GetNearestKilobytes(rootStats.SysMemFootprint - rootStats.UserDebugFootprint - 
                rootStats.DebugInfoFootprint));

            if (reportType != MemoryHeap::MemReportBrief)
            {
                MemItem* sysSummaryItem = rootItem->AddChild(NextHandle++, "System Summary");
                sysSummaryItem->AddChild(NextHandle++, "System Memory FootPrint", static_cast<UInt32>(rootStats.SysMemFootprint));
                sysSummaryItem->AddChild(NextHandle++, "System Memory Used Space", static_cast<UInt32>(rootStats.SysMemUsedSpace));
                if (rootStats.PageMapFootprint > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Page Mapping Footprint", static_cast<UInt32>(rootStats.PageMapFootprint));
                }
                if (rootStats.PageMapUsedSpace > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Page Mapping UsedSpace", static_cast<UInt32>(rootStats.PageMapUsedSpace));
                }
                if (rootStats.BookkeepingFootprint > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Bookkeeping Footprint", static_cast<UInt32>(rootStats.BookkeepingFootprint));
                }
                if (rootStats.BookkeepingUsedSpace > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Bookkeeping Used Space", static_cast<UInt32>(rootStats.BookkeepingUsedSpace));
                }
                if (rootStats.DebugInfoFootprint > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Debug Info Footprint", static_cast<UInt32>(rootStats.DebugInfoFootprint));
                }
                if (rootStats.DebugInfoUsedSpace > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Debug Info Used Space", static_cast<UInt32>(rootStats.DebugInfoUsedSpace));
                }
                if (rootStats.UserDebugFootprint > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Debug Heaps Footprint", static_cast<UInt32>(rootStats.UserDebugFootprint));
                }
                if (rootStats.UserDebugUsedSpace > 0)
                {
                    sysSummaryItem->AddChild(NextHandle++, "Debug Heaps Used Space", static_cast<UInt32>(rootStats.UserDebugUsedSpace));
                }
            }

            MemItem* summaryItem;
            
            if (reportType == MemoryHeap::MemReportBrief)
            {
                summaryItem = rootItem;
            }
            else
            {
                summaryItem = rootItem->AddChild(NextHandle++, "Summary");
            }
            summaryItem->StartExpanded = true;

            SummaryStatIdVisitor statVisit(false);
            statVisit.Visit(NULL, Memory::GetGlobalHeap());
            statVisit.UpdateStatGroups();

            SummaryMemoryVisitor heapVisit(false);
            heapVisit.Visit(NULL, Memory::GetGlobalHeap());

            UPInt imageMemory = statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(Stat_Image_Mem));
            if (imageMemory > 0)
            {
                summaryItem->AddChild(NextHandle++, "Image", static_cast<UInt32>(imageMemory));
            }

#ifdef GFX_ENABLE_SOUND
            UPInt soundMemory = statVisit.GetStatIdMemory(StatDesc::GetGroupIterator(Stat_Sound_Mem));
            if (soundMemory > 0)
            {
                summaryItem->AddChild(NextHandle++, "Sound", static_cast<UInt32>(soundMemory)); 
            }
#endif

#ifdef GFX_ENABLE_VIDEO
            UPInt videoMemory = heapVisit.GetVideoMemory();
            if (videoMemory > 0)
            {
                summaryItem->AddChild(NextHandle++, "Video", static_cast<UInt32>(videoMemory));
            }
#endif
            summaryItem->AddChild(NextHandle++, "Movie View", static_cast<UInt32>(heapVisit.GetMovieViewMemory()));
            summaryItem->AddChild(NextHandle++, "Movie Data", static_cast<UInt32>(heapVisit.GetMovieDataMemory()));

            if (reportType != MemoryHeap::MemReportBrief)
            {
                MemReportHeaps(Memory::pGlobalHeap, rootItem, reportType);
            }
        }
        break;
    }
}



UInt32 StatsUpdate::GetRoundUpKilobytes(UPInt bytes)
{
    return (static_cast<UInt32>(bytes) + 1023) / 1024;
}

UInt32 StatsUpdate::GetNearestKilobytes(UPInt bytes)
{
    return (static_cast<UInt32>(bytes) + 512) / 1024;
}

void StatsUpdate::MemReportHeaps(MemoryHeap* pHeap, MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    if (((pHeap->GetFlags() & MemoryHeap::Heap_UserDebug) == 0) && pHeap->GetFootprint() > 0)
    {
        StatBag            statBag;
        StatInfo           statInfo;
        Stat::StatValue    statValue;

        pHeap->GetStats(&statBag);

        statBag.GetStat(&statInfo, StatHeap_TotalFootprint);
        statInfo.GetStat(&statValue, 0);

        String buffer;
        Format(buffer, "[Heap] {0}", pHeap->GetName());
        MemItem* heapItem = rootItem->AddChild(NextHandle++, buffer, static_cast<UInt32>(statValue.IValue));

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO
        if (reportType == MemoryHeap::MemReportFull)
        {
            StatDesc::Iterator it = StatDesc::GetGroupIterator(StatGroup_Core);
            if (reportType == MemoryHeap::MemReportFull)
            {
                it = StatDesc::GetGroupIterator(StatGroup_Default);
            }

            GetHeapMemory(it, &statBag, heapItem);

            statBag.UpdateGroups();
            StatInfo sumStat;
            if (statBag.GetStat(&sumStat, Stat_Mem))
            {
                heapItem->AddChild(NextHandle++, "Allocations Count", static_cast<UInt32>(sumStat.ToMemoryStat()->GetAllocCount()));
            }
        }
#endif
        HolderVisitor hv;
        pHeap->VisitChildHeaps(&hv);

        for (UPInt i = 0; i < hv.Heaps.GetSize(); ++i)
        {
            MemReportHeaps(hv.Heaps[i], heapItem, reportType);
        }
    }
}

// Invokes recursively memory statistic data belonging to the given heap.
void StatsUpdate::GetHeapMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem)
{
    const StatDesc* pdesc = *it;

    if (pdesc)
    {
        Ptr<MemItem> childItem;
        if (pdesc->GetGroupId() == 0)
        {
            if (pdesc->GetId() != 0)
            {
                if ((pdesc->GetType() == (UByte)Stat::Stat_Memory) ||
                    (pdesc->GetType() == (UByte)Stat::Stat_LogicalGroup))
                {
                    childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
                    childItem->Name = pdesc->GetName();
                    if (pdesc->GetId() != StatHeap_Summary)
                    {
                        childItem->StartExpanded = true;
                    }
                }
            }
        }

        if (!childItem)
        {
            childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
            childItem->Name = pdesc->GetName();
        }

        StatDesc::Iterator ichild = pdesc->GetChildIterator();  
        while (!ichild.IsEnd())
        {
            if ((pdesc->GetGroupId() == 0) && (pdesc->GetId() == 0))
            {
                GetHeapMemory(ichild, statBag, rootItem);
            }
            else if (childItem)
            {
                GetHeapMemory(ichild, statBag, childItem);
            }

            if (childItem)
            {
                StatInfo           statInfo;
                Stat::StatValue    statValue;

                if (statBag->GetStat(&statInfo, ichild.GetId()))
                {
                    statInfo.GetStat(&statValue, 0);
                    if (statValue.IValue > 0)
                    {
                        childItem->AddChild(NextHandle++, statInfo.GetName(), static_cast<UInt32>(statValue.IValue));
                    }
                }
            }
            ++ichild;
        }

        if (childItem && childItem->Children.GetSize() > 0)
        {
            rootItem->Children.PushBack(childItem);
        }
    }
}

// Invokes recursively memory statistic data belonging to the given heap.
void StatsUpdate::GetStatMemory(StatDesc::Iterator it, StatBag* statBag, MemItem* rootItem)
{
    const StatDesc* pdesc = *it;

    if (pdesc)
    {
        StatDesc::Iterator ichild = pdesc->GetChildIterator();  
        while (!ichild.IsEnd())
        {
            StatInfo            statInfo;
            Stat::StatValue     statValue;
            Ptr<MemItem> childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
            childItem->Name = ichild->GetName();

            if (statBag->GetStat(&statInfo, ichild.GetId()))
            {
                statInfo.GetStat(&statValue, 1);
                childItem->SetValue(static_cast<UInt32>(statValue.IValue));
            }

            if (statValue.IValue > 0)
            {
                rootItem->Children.PushBack(childItem);
            }

            GetStatMemory(ichild, statBag, childItem);

            ++ichild;
        }
    }
}

void StatsUpdate::MemReportFile(MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    FileVisitor visitor;
    Memory::pGlobalHeap->VisitChildHeaps(&visitor);

    StringHash<FileStats>& statsMap = visitor.FileStatsMap;
    StringHash<FileStats>::Iterator itFile;
    for (itFile = statsMap.Begin(); itFile != statsMap.End(); ++itFile)
    {
        String buffer;
        Format(buffer, "Movie File {0}", itFile->First);
        MemItem* childItem = rootItem->AddChild(NextHandle++, buffer);

        itFile->Second.Bag.UpdateGroups();
        GetFileMemory(StatDesc::GetGroupIterator(Stat_Mem), itFile->Second, childItem, reportType);
    }
}

void StatsUpdate::MemReportHeapsDetailed(MemItem* rootItem, MemoryHeap* heap)
{
    HeapTreeCreator heapTreeCreator(Memory::GetHeapByAddress(rootItem), this, &NextHandle);
    heap->LockAndVisit(&heapTreeCreator);

    MemItem* sysTotalItem = rootItem->AddChild(NextHandle++, "Scaleform Footprint", 
        static_cast<UInt32>(heapTreeCreator.RootHeapStats.SysMemFootprint));
    sysTotalItem->StartExpanded = true;
    MemItem* sysUsedItem = sysTotalItem->AddChild(NextHandle++, "Used Space", heapTreeCreator.UsedSpaceRoot->Value);
    sysUsedItem->StartExpanded = true;
    sysUsedItem->Children.PushBack(heapTreeCreator.GlobalHeap);
    sysUsedItem->Children.PushBack(heapTreeCreator.MovieDataRoot);
    sysUsedItem->Children.PushBack(heapTreeCreator.MovieViewRoot);
    sysUsedItem->Children.PushBack(heapTreeCreator.VideoRoot);
    sysUsedItem->Children.PushBack(heapTreeCreator.OtherRoot);

    sysTotalItem->AddChild(NextHandle++, "Debug Data", 
        static_cast<UInt32>(heapTreeCreator.RootHeapStats.DebugInfoFootprint + heapTreeCreator.RootHeapStats.UserDebugFootprint));
    UInt32 overhead = static_cast<UInt32>(heapTreeCreator.RootHeapStats.BookkeepingFootprint + heapTreeCreator.RootHeapStats.PageMapFootprint);
    if (overhead > 0)
    {
        sysTotalItem->AddChild(NextHandle++, "Heap Overhead", overhead);
    }
    sysTotalItem->Children.PushBack(heapTreeCreator.UnusedSpaceRoot);
}



// Recursively adds the memory per StatId for the given file
void StatsUpdate::GetFileMemory(StatDesc::Iterator it, FileStats& fileStats, MemItem* rootItem, MemoryHeap::MemReportType reportType)
{
    StatInfo           statInfo;
    Stat::StatValue    statValue;

    UPInt total = 0;
    if (fileStats.Bag.GetStat(&statInfo, it.GetId()))
    {
        statInfo.GetStat(&statValue, 0);

        fileStats.TotalMemory += statValue.IValue;
        total = statValue.IValue;
    }

    Ptr<MemItem> childItem = *SF_HEAP_AUTO_NEW(rootItem) MemItem(NextHandle++);
    for (StatDesc::Iterator itChild = it->GetChildIterator(); !itChild.IsEnd(); ++itChild)
    {

        GetFileMemory(itChild, fileStats, childItem, reportType);
    }

    if (total > 0)
    {
        childItem->Name = it->GetName();
        childItem->SetValue(static_cast<UInt32>(total));
        rootItem->Children.PushBack(childItem);
    }
}

//------------------------------------------------------------------------
// Collects memory statistic data from heaps and puts obtained data in string
// buffer. 
void MemoryHeap::MemReport(StringBuffer& buffer, MemReportType reportType, bool xmlFormat)
{   
    Ptr<MemItem> rootItem = *SF_NEW MemItem(0);
    StatsUpdate().MemReport(rootItem, reportType);
    if (xmlFormat)
    {
        for (UPInt i = 0; i < rootItem->Children.GetSize(); ++i)
        {
            rootItem->Children[i]->ToXml(&buffer);
        }
    }
    else
    {
        rootItem->ToString(&buffer);
    }
}

void MemoryHeap::MemReport(MemItem* rootItem, MemReportType reportType)
{
    StatsUpdate().MemReport(rootItem, reportType);
}

#else // SF_ENABLE_STATS

void MemoryHeap::MemReport(StringBuffer& buffer, MemReportType reportType, bool xmlFormat)
{
    SF_UNUSED3(buffer, reportType, xmlFormat);
}

void MemoryHeap::MemReport(MemItem* rootItem, MemReportType reportType)
{
    SF_UNUSED(rootItem);
    SF_UNUSED(reportType);
}


#endif // SF_ENABLE_STATS

} // Scaleform

