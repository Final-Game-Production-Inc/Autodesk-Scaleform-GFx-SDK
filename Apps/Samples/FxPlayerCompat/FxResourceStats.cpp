/**************************************************************************

Filename    :   FxResourceStats.cpp
Content     :   Resource statistics tracking
Created     :   October 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxResourceStats.h"
#include "FxHeapStatItem.h"

#include "GFx_Kernel.h"
#include "FxReportCollector.h"
#include "Kernel/SF_MsgFormat.h"

using namespace Scaleform;
using namespace Render;
using namespace GFx;

// ***** FxResourceRootStatItem


FxResourceRootStatItem::FxResourceRootStatItem(
    FxGlobalStatTracker* pstatTracker,
    ResourceLib *presourceLib, Array<Ptr<Movie> > *pmovies)
: BaseClass(NULL)
, pStatTracker(pstatTracker)
, pResourceLib(presourceLib)
{
    SF_UNUSED(pmovies);

    AddChildArrayList(&MovieDefItems);

    pSharedResourceItem = *SF_HEAP_AUTO_NEW(this) FxResourceFolderStatItem(this, "Shared Resources");
    AddChild(pSharedResourceItem);

  //  AddChildArrayList(&ImageResourceItems);    
}



typedef   FxArrayMergeAccessor<FxResourceRootStatItem, FxImageResourceStatItem,
                               Ptr<ImageResource> >  ImageResourceMergeAccessor;

void    FxResourceRootStatItem::Update(const FxStatUpdate& update)
{

    // First, obtain the full list of resources from the Resource lib.
    Array<Ptr<Resource> > resources;
    pResourceLib->GetResourceArray(&resources);


    // GFxMovieDataDef is not exposed publicly; this is a problem
    // if there is not Impl. We may need an exposed base class
    // that provides the following:
    //   - Name, Heap, Stats (counters, etc)
    //   - A way to check that DefImpl belongs to us.
    Array<Ptr<Resource> > Defs;
    Array<Ptr<MovieDef> > DefImpls;
    Array<Ptr<ImageResource> > ImageResources;

    UPInt i;

    for (i=0; i< resources.GetSize(); i++)
    {
        Resource::ResourceType rtype = resources[i]->GetResourceType();

        switch (rtype)
        {
        case Resource::RT_MovieDataDef:
            Defs.PushBack(resources[i]);
            break;
        case Resource::RT_MovieDef:
            DefImpls.PushBack((MovieDef*)resources[i].GetPtr());
            break;
        case Resource::RT_Image:
            ImageResources.PushBack((ImageResource*)resources[i].GetPtr());
            break;
        default:
            break;
        }
    }


    FxMovieDataStatItem::MovieDataUpdate newUpdate(update, &DefImpls);
    FxArrayMergeAccessor<FxResourceRootStatItem,
                         FxMovieDataStatItem, Ptr<Resource> > a(this, Defs);
    Fx_MergeItems(MovieDefItems, a, newUpdate);


    /*
    FxArrayMergeAccessor<FxResourceRootStatItem,
                         FxImageResourceStatItem, Ptr<ImageResource> > ar(this, ImageResources);
   // ImageResourceMergeAccessor ar(this, ImageResources);
    Fx_MergeItems(ImageResourceItems, ar, update);
    */

    pSharedResourceItem->Update(update, &ImageResources);

}

void FxResourceRootStatItem::Accept( Visitor& v )
{
    for (UPInt i = 0; i < MovieDefItems.GetSize(); ++i)
    {
        MovieDefItems[i]->Accept(v);
    }

    v.Visit_FxResourceRootStatItem(*this);
}

void FxResourceRootStatItem::GetText( String* pstring )
{
    *pstring = "";
}

void FxResourceRootStatItem::GetName( String* pstring )
{
    *pstring = "Objects";
}

unsigned FxResourceRootStatItem::GetFlags() const
{
    return 0;
}

FxStatItem::StatType FxResourceRootStatItem::GetType() const
{
    return ST_ResourceRoot;
}

// ***** FxMovieDataStatItem


FxMovieDataStatItem::FxMovieDataStatItem(FxStatItem* pparent, ItemCreateType* pmovieData)
  : BaseClass(pparent, pmovieData)
{

    ResourceReport* presReport = pmovieData->GetResourceReport();    
    NameKey = presReport->GetResourceName();


    FxStat_HeapDesc* pglobalHeapDesc = GetStatTracker()->GetGlobalHeapDesc();
    MemoryHeap*     pmovieDataHeap  = presReport->GetResourceHeap();

    // For now MovieData heaps are always global. In the future we can implement
    // some type of {child -> parent} traversal to identify the descriptor path
    // and create the correct one.
    SF_ASSERT(pmovieDataHeap);
    SF_ASSERT(pglobalHeapDesc);
    SF_ASSERT((pmovieDataHeap->GetParentHeap() == pglobalHeapDesc->GetHeapPtr()));

    // Create root heap stat item.
    Ptr<FxStat_HeapDesc> pheapDesc = *pglobalHeapDesc->CreateChildHeapDesc(pmovieDataHeap);
    pHeapItem = *SF_HEAP_AUTO_NEW(this) FxHeapStatItem(this, pheapDesc, false);


    AddChild(pHeapItem);
    AddChildArrayList(&MovieDefItems);
}

void    FxMovieDataStatItem::MarkAsDead()
{
    BaseClass::MarkAsDead();
    pHeapItem->MarkAsDead();
}

void    FxMovieDataStatItem::Update(const MovieDataUpdate& update)
{

    pHeapItem->Update(update);

    if (!IsDead())
    {
    //    PostUpdate(update.GetType());
    //    return;

        ResourceReport* presReport = GetItemCreateData()->GetResourceReport();

        String name = "MovieData \"";
        name += presReport->GetResourceName() + "\"";

        SetName(name);

        // Assign correct Text based on the report,
        // that is add memory use, etc.
        SetText("");
    }

    
    // This is stat bag with any custom MovieDef Stats.
 //   StatBag statBag;
 //   presReport->GetStats(&statBag);


    // Make a list of all our Defs
    Array<Ptr<MovieDef> > ourDefImpls;
    
    UPInt i;
    for (i = 0; i < update.pDefImpls->GetSize(); i++)
    {
        if ((*update.pDefImpls)[i]->GetMovieDataResource() == GetItemCreateData())
            ourDefImpls.PushBack((*update.pDefImpls)[i]);
    }

    // Merge changes

    FxArrayMergeAccessor<FxMovieDataStatItem, FxMovieDefStatItem,
                         Ptr<MovieDef> > a(this, ourDefImpls);
    Fx_MergeItems(MovieDefItems, a, update);

    PostUpdate(update.GetType());

}

void FxMovieDataStatItem::Accept(Visitor& v)
{
    for (UPInt i = 0; i < MovieDefItems.GetSize(); ++i)
    {
        MovieDefItems[i]->Accept(v);
    }

    v.Visit_FxMovieDataStatItem(*this);
}



// ***** FxMovieDataStatItem


FxMovieDefStatItem::FxMovieDefStatItem(FxMovieDataStatItem* pparent, ItemCreateType* pmovieDef)
: BaseClass(pparent, pmovieDef)
{
    // Create Heap object
    // Find HeapDesc for this item
    //    -> may be tricky with a chain of Desc creations?
    
    FxStat_HeapDesc* pglobalHeapDesc = GetStatTracker()->GetGlobalHeapDesc();
    MemoryHeap*     pmovieDefHeap  = pmovieDef->GetBindDataHeap();

    // Create Heap stat item.
    SF_ASSERT((pmovieDefHeap->GetParentHeap() == pglobalHeapDesc->GetHeapPtr())); 
    Ptr<FxStat_HeapDesc> pheapDesc = *pglobalHeapDesc->CreateChildHeapDesc(pmovieDefHeap);
    pHeapItem = *SF_HEAP_AUTO_NEW(this) FxHeapStatItem(this, pheapDesc, false);

  //  pHeapItem = *SF_HEAP_AUTO_NEW(this) FxHeapStatItem(this, pheap);

    DataURL = pmovieDef->GetFileURL();
    SetName(String("MovieDef \"", GetShortFilename(DataURL.ToCStr()), "\"" ));

    AddChild(pHeapItem);
}

void    FxMovieDefStatItem::MarkAsDead()
{
    BaseClass::MarkAsDead();
    pHeapItem->MarkAsDead();
}

void    FxMovieDefStatItem::Update(const FxStatUpdate& update)
{
    // Refresh the heap?
    if (pHeapItem)
        pHeapItem->Update(update);

    PostUpdate(update.GetType());
}

void FxMovieDefStatItem::Accept( Visitor& v )
{
    v.Visit_FxMovieDefStatItem(*this);
}


// ***** Image Resource reporting

FxImageResourceStatItem::FxImageResourceStatItem(FxResourceFolderStatItem* pparent, ItemCreateType* pres)
: BaseClass(pparent, pres)
{
    ResourceURL = pres->GetFileURL();
    String name = "Image Resource";

    if (!ResourceURL.IsEmpty())
        name += String(" \"", GetShortFilename(ResourceURL.ToCStr()), "\"");
    if (pres->GetResourceUse() == Resource::Use_Gradient)
        name += " [Gradient]";

    SetName(name);

    ImageInfoBase* pbase = pres->GetImageInfo();
    if (pbase)
    {
        Format(InfoText, "(%{0} x {1})", pbase->GetWidth(), pbase->GetHeight());
        SetText(InfoText);
    }
    else
        SetText("(not available)");
}

void    FxImageResourceStatItem::SetExpireText(bool expired)
{
    SetText(expired ? String("Unloaded") : InfoText);
}

void    FxImageResourceStatItem::Update(const FxStatUpdate& update)
{
     PostUpdate(update.GetType());

    // Refresh the heap?
  //  if (pHeapItem)
   //     pHeapItem->Update(update);


}





// ***** Image Resource reporting

FxResourceFolderStatItem::FxResourceFolderStatItem(FxStatItem* pparent, const char* pname)
: BaseClass(pparent)
{    
    SetName(pname);
    AddChildArrayList(&ResourceItems);
}

void    FxResourceFolderStatItem::Update(const FxStatUpdate& update,
                                         Array<Ptr<ImageResource> >* presources)
{
    FxArrayMergeAccessor<FxResourceFolderStatItem,
                         FxImageResourceStatItem, Ptr<ImageResource> > ar(this, *presources);
    // ImageResourceMergeAccessor ar(this, ImageResources);
    Fx_MergeItems(ResourceItems, ar, update);

    if (update.IsTick())
    {
        String text;

        Format(text, "(%d Resources)", ResourceItems.GetSize());
        SetText(text);
    }
}

void FxResourceFolderStatItem::Accept( Visitor& v )
{
    for (UPInt i = 0; i < ResourceItems.GetSize(); ++i)
    {
        ResourceItems[i]->Accept(v);
    }
}

// ***** FxGlobalStatTracker


FxGlobalStatTracker::FxGlobalStatTracker(
    Movie* pmovie, 
    ResourceLib *presourceLib, 
    MemoryHeap *pheap
    )
: pTrackerHeap(pheap)
{
    // Store all tracker data in a dedicated heap.
    pGlobalHeapDesc = *SF_HEAP_NEW(pTrackerHeap) FxStat_HeapDesc(0, Memory::GetGlobalHeap());

    FxStatUpdate update(FxStatUpdate::Update_Init, this, 0);

    // Temporarily pass pmovie ...
    pMemoryStats = *SF_HEAP_NEW(pTrackerHeap) FxMemoryRootStatItem(this);
    pMemoryStats->Update(update);

    pObjectStats = *SF_HEAP_NEW(pTrackerHeap) FxResourceRootStatItem(this, presourceLib, 0);
    pObjectStats->Update(update);

    pPerformanceStats = *SF_HEAP_AUTO_NEW(this) FxPerformanceRootStatItem(this, pmovie);
    pPerformanceStats->Update(update);

    pGlobalHeapDesc->FinalizeUpdate(update);
}

FxGlobalStatTracker::~FxGlobalStatTracker()
{
    pObjectStats = 0;
    pMemoryStats = 0;

    pGlobalHeapDesc = 0;
}


// Accessor moved to source due to FxMemoryRootStatItem dependency
FxStatItem*     FxGlobalStatTracker::GetMemoryStats() const
{
    return pMemoryStats.GetPtr();
}

FxStatItem* FxGlobalStatTracker::GetPerformanceStats() const
{
    return pPerformanceStats.GetPtr();
}

void FxGlobalStatTracker::UpdateMemoryStats( const FxStatUpdate& update )
{
    pMemoryStats->Update(update);
    // Needs to be fixed. Commented this call out because it causes a crash on PS3
    //pObjectStats->Update(update);
    pGlobalHeapDesc->FinalizeUpdate(update);
}

void FxGlobalStatTracker::UpdatePerformanceStats( const FxStatUpdate& update )
{
     pPerformanceStats->Update(update);   
}


void FxGlobalStatTracker::GetReport( String *reportStr, bool reportAll /*= false*/ )
{
    class FxStringReportOutput : public FxReportOutput
    {
        public:
        String ReportStr;
        virtual void Clear(){ReportStr = "";}
        virtual void AddLine(const char* pname, const char* ptext)
        {
            ReportStr += String(pname, ptext, "\n");
        }
    } report;
    FxReportCollector rc(&report);

    //printf("\n\n");
    rc.ReportItems(pMemoryStats, reportAll);
    //ReportItems(pHeapStats, String());
    *reportStr += report.ReportStr;
    //printf("\n\n");
    rc.ReportItems(pPerformanceStats, reportAll);
    *reportStr += report.ReportStr;

    //ReportItems(pObjectStats, String());
}

void FxGlobalStatTracker::SetMovie( Movie* pmovie )
{
    pPerformanceStats->SetMovie(pmovie);
}

