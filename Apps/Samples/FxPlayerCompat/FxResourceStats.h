/**************************************************************************

Filename    :   FxResourceStats.h
Content     :   Resource statistics tracking
Created     :   September 23, 2008
Authors     :   Dmitry Polenur, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxResourceStats_H
#define INC_FxResourceStats_H

#include "FxStatItem.h"

#include "GFx.h"
#include "GFx_Kernel.h"

#include "FxPlayerStats.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;

class FxResourceRootStatItem;
class FxMovieDataStatItem;
class FxMovieDefStatItem;
class FxMessageStatItem;


// External declarations
class FxStat_HeapDesc;
class FxMemoryRootStatItem;

class FxHeapStatItem;
class FxHeapElementStatItem;




/* ***** Desired Resource Tree Structure

 Root <invisible>
  - MovieDef "file1.swf" (896K)
    + Heap
    + Impl1
    + Impl2
    + Resources (Unshared)
  + MovieDef "file2.swf" (346K)
  + Shared Resources
    + Image "img1.tga" (512x512)
    + Gradient Image 1 (1x256)
    + System Font "Arial"
  - MovieView 1 "file1.swf"
    + Heap
    + Counters
  + Font Cache
  + Renderer


  Note that we may want a separate heap view structure,
  which will display the global heap detal.
*/



///////////////////////////////////////////////////////////////////////////////
class FxMovieDataStatItem : public FxStatItem_ExpireBase<FxStatItem_ChildSupport<FxStatItem>, Resource>
{
    typedef FxStatItem_ExpireBase<FxStatItem_ChildSupport<FxStatItem>, Resource> BaseClass;

public:
    FxMovieDataStatItem(FxStatItem* pparent, ItemCreateType* pmovieData);

    virtual void    MarkAsDead();

    // Merge implementation, which considers Name.   
    // These three methods below are the only three methods that require non-const argument.
    bool            Merge_Equals(ItemCreateType* const pother) const
    {
        return BaseClass::Merge_Equals(pother) &&
               (NameKey == pother->GetResourceReport()->GetResourceName());
    }
    bool            Merge_PartialEquals(ItemCreateType* const pother) const
    {
        return BaseClass::Merge_Equals(0) &&
               (NameKey == pother->GetResourceReport()->GetResourceName());
    }
    bool            Merge_ComesBefore(ItemCreateType* const pother) const
    {
        return NameKey < pother->GetResourceReport()->GetResourceName();
    }

   
    struct MovieDataUpdate : public FxStatUpdate
    {
        Array<Ptr<MovieDef> >* pDefImpls;
        MovieDataUpdate(const FxStatUpdate& update, Array<Ptr<MovieDef> > *pdefImpls)
            : FxStatUpdate(update), pDefImpls(pdefImpls)
        { }
    };

    // We pass defImpl array so that the right ones can be added from the same request.
    virtual  void       Update(const MovieDataUpdate& update);

    virtual void Accept(Visitor& v);

private:
    // List contains:
    //   - Heap
    //   - Implementations
    //   - Resources

    Ptr<FxHeapStatItem>                pHeapItem;
    ArrayLH<Ptr<FxMovieDefStatItem> > MovieDefItems;

    // Resources belonging to this DataDef.
    //Array<Ptr<FxResources> > MovieDefItems;
    SF::StringLH NameKey;
};



///////////////////////////////////////////////////////////////////////////////
class FxMovieDefStatItem : public FxStatItem_ExpireBase<FxStatItem_ChildSupport<FxStatItem>, MovieDef>
{
    typedef FxStatItem_ExpireBase<FxStatItem_ChildSupport<FxStatItem>, MovieDef> BaseClass;

public:
    FxMovieDefStatItem(FxMovieDataStatItem* pparent, ItemCreateType* pmovieDef);

public:
    virtual void    MarkAsDead();

    // Merge implementation, which considers URL as sort basis.   
    bool            Merge_Equals(const ItemCreateType* const pother) const
    {
        return BaseClass::Merge_Equals(pother) &&
            (DataURL == pother->GetFileURL());
    }
    bool            Merge_PartialEquals(const ItemCreateType* const pother) const
    {
        return BaseClass::Merge_Equals(0) &&
            (DataURL == pother->GetFileURL());
    }
    bool            Merge_ComesBefore(const ItemCreateType* const pother) const
    {
        return DataURL < pother->GetFileURL();
    }

    // We pass defImpl array so that the right ones can be added from the same request.
    virtual  void       Update(const FxStatUpdate& update);

    virtual void Accept(Visitor& v);

private:
    // List contains:
    //   - Heap
    //   - Counters, Load Timings?
    Ptr<FxHeapStatItem>    pHeapItem; 

    SF::StringLH               DataURL;
};


///////////////////////////////////////////////////////////////////////////////
class FxResourceFolderStatItem;

class FxImageResourceStatItem : public FxStatItem_ExpireBase<FxStatItem, ImageResource>
{
    typedef FxStatItem_ExpireBase<FxStatItem, ImageResource> BaseClass;
    
public:
    typedef ImageResource ItemCreateType;

public:
    FxImageResourceStatItem(FxResourceFolderStatItem* pparent, ItemCreateType* pres);

public:
    virtual void    SetExpireText(bool expired);

    // Merge implementation, which considers URL as sort basis.   
    bool            Merge_Equals(const ItemCreateType* const pother) const
    {
        const char *potherURL = pother->GetFileURL();
        return BaseClass::Merge_Equals(pother) && (ResourceURL == (potherURL ? potherURL : ""));
    }
    bool            Merge_PartialEquals(const ItemCreateType* const pother) const
    {
        const char *potherURL = pother->GetFileURL();
        return BaseClass::Merge_Equals(0) && (ResourceURL == (potherURL ? potherURL : ""));
    }
    bool            Merge_ComesBefore(const ItemCreateType* const pother) const
    {
        const char *potherURL = pother->GetFileURL();
        return ResourceURL < (potherURL ? potherURL : "");
    }

    // We pass defImpl array so that the right ones can be added from the same request.
    virtual  void   Update(const FxStatUpdate& update);

private:
    SF::StringLH ResourceURL;
    SF::StringLH InfoText;
};


///////////////////////////////////////////////////////////////////////////////
// A simple folder used to hold resources; can not expire.

class FxResourceFolderStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:    
    FxResourceFolderStatItem(FxStatItem* pparent, const char* pname);

public:    
    // Update the list of resources.
    void            Update(const FxStatUpdate& update,
                           Array<Ptr<ImageResource> >* presources);

    virtual void Accept(Visitor& v);

private:
    ArrayLH<Ptr<FxImageResourceStatItem> > ResourceItems;
};


///////////////////////////////////////////////////////////////////////////////
class FxResourceRootStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:
    FxResourceRootStatItem(FxGlobalStatTracker* pstatTracker,
        ResourceLib *presourceLib,
        Array<Ptr<Movie> > *pmovies);

    virtual FxGlobalStatTracker* GetStatTracker() const { return pStatTracker; }
        
public:
    // Item type & general information
    virtual StatType    GetType() const;
    virtual unsigned    GetFlags() const;

    virtual void        GetName(String* pstring);
    virtual void        GetText(String* pstring);

    // Updates all of the items, obtaining the new value and history.
    // Stats can also add/remove children. The tree control gets
    // notified about item changes through the ptreeUpdate interfaces,
    // which can help it update its columns.
    virtual  void       Update(const FxStatUpdate& update);
   
    virtual void Accept(Visitor& v);

private:
    FxGlobalStatTracker*    pStatTracker;
    Ptr<ResourceLib>    pResourceLib;

    // Items forming a list.
    ArrayLH<Ptr<FxMovieDataStatItem> >     MovieDefItems;
    //ArrayLH<Ptr<FxImageResourceStatItem> > ImageResourceItems;
    Ptr<FxResourceFolderStatItem>           pSharedResourceItem;
};



///////////////////////////////////////////////////////////////////////////////
// ***** Global Stat Tracker

// Maintains object, memory and performance statistic info trees,
// allowing them to be updated and displayed.


class FxGlobalStatTracker : public RefCountBase<FxGlobalStatTracker, Stat_DebugTracker_Mem>
{

public:
    FxGlobalStatTracker(Movie* pmovie, ResourceLib *presourceLib, MemoryHeap* ptrackerHeap);
    ~FxGlobalStatTracker();

public:
    FxStat_HeapDesc* GetGlobalHeapDesc() const { return pGlobalHeapDesc; }

    // Accessors for branches of tree control.
    FxStatItem*     GetObjectStats() const { return pObjectStats.GetPtr(); }
    FxStatItem*     GetMemoryStats() const;
    FxStatItem*     GetPerformanceStats() const;

    // Traverses heaps from global root, updating them.
    void UpdateMemoryStats(const FxStatUpdate& update);
    void UpdatePerformanceStats(const FxStatUpdate& update);
    void GetReport(String *reportStr, bool reportAll = false);

    // Temporarily ...
    void SetMovie(Movie* pmovie);

private:
    // Reports an individual item
    static void    ReportItems(FxStatItem* pitem, const String& indent);

private:
    MemoryHeap*                    pTrackerHeap;
    Ptr<FxStat_HeapDesc>           pGlobalHeapDesc;
    // Lock to guard asynchronous update from.
    Lock                           HeapUpdateLock;   
    // Different types of tracked statistics.    
    Ptr<FxResourceRootStatItem>    pObjectStats;
    Ptr<FxMemoryRootStatItem>      pMemoryStats;
    Ptr<FxPerformanceRootStatItem> pPerformanceStats;
};




#endif // INC_SF_GFX_ResourceStats_H
