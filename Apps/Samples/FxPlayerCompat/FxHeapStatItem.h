/**************************************************************************

Filename    :   FxHeapStatItem.h
Content     :   Memory heap statistics tracking items
Created     :   September 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxHeapStatItem_H
#define INC_FxHeapStatItem_H

#include "FxStatItem.h"
#include "FxPlayerStats.h"

// ***** Classes Declared
class FxHeapStatItem;
class FxHeapElementStatItem;
class FxMemoryRootStatItem;

class FxStat_HeapDesc;
class FxStat_HeapEntryDesc;

class FxHeapSummaryRootStatItem;
class FxHeapSummaryElementStatItem;

///////////////////////////////////////////////////////////////////////////////
class MemStatistic
{
public:
    MemStatistic() : AllocN(0), Allocated(0), Used(0), PeakUsed(0), PeakCurrent(0) {}
    MemStatistic(const MemoryStat& ms);
    MemStatistic(UPInt v1, UPInt v2, UPInt v3, UPInt v4);

public:
    void operator /=(UPInt v);
    MemStatistic& operator  /(UPInt v);

    MemStatistic& operator  =(const MemoryStat& ms);

    void          operator +=(UPInt v);

    void          operator +=(const MemStatistic& other);

    MemStatistic& operator  =(const FxStat_HeapDesc& d);
    void          operator +=(const FxStat_HeapDesc& d);
    MemStatistic& SetWithDebud(const FxStat_HeapDesc& d);

    MemStatistic& operator  =(const FxStat_HeapEntryDesc& d);
    void          operator +=(const FxStat_HeapEntryDesc& d);

    void Init();

    UPInt   GetAllocN() const { return AllocN; }
    UPInt   GetAllocated() const { return Allocated; }
    UPInt   GetUsed() const { return Used; }
    UPInt   GetPeakUsed() const { return PeakUsed; }

public:
    UPInt   AllocN;
    UPInt   Allocated;
    UPInt   Used;
    UPInt   PeakUsed;

private:
    UPInt   PeakCurrent;
};

///////////////////////////////////////////////////////////////////////////////
// ***** FxStat_HeapDesc and FxStat_HeapEntryDesc

// FxStat_HeapEntryDesc structures are owned by FxHeapElementStatItem and are
// shared through a hash tables in FxStat_HeapDesc. This allows multiple HeapItems displaying
// one heap to use a shared history and minimize updates.
// If all referencing FxHeapElementStatItems are released the item will automatically
// be removed from the parent heap descriptor; however, this typically should not
// happen until the heap dies or the item expires.


class FxStat_HeapEntryDesc : public RefCountBase<FxStat_HeapEntryDesc, Stat_DebugTracker_Mem>
{
    friend class FxHeapElementStatItem;

public:
    typedef FxHistoryRingBuffer<MemStatistic> HistoryType;
        
    bool                    IsExpired() const           { return true; }
    //bool                    IsExpired() const         { return ExpireCounter == 0; }
    const FxStat_HeapDesc&  GetHeapDesc() const         { return *pHeapDesc; }
    unsigned                GetId() const               { return Id; }
    const HistoryType&      GetHistory() const          { return History; }

    unsigned    GetExpireCounter() const { return ExpireCounter; }
    void    SetExpireCounter(unsigned value) { ExpireCounter = value; }
    void    DecExpireCounter()
    {
        if (GetExpireCounter() > 0)
            --ExpireCounter;
    }

    void    AddHistory(const MemStatistic& item) { History.PushBack(item); }

    UPInt   GetAllocCount() const { return AllocCount; }
    void    SetAllocCount(UPInt value) { AllocCount = value; }

    UPInt   GetFilePeak() const      { return FilePeak; }
    void    SetFilePeak(UPInt value) { FilePeak = value; }


public:
    FxStat_HeapEntryDesc(FxStat_HeapDesc* pheapDesc, unsigned id);
    // Released entry removes itself from parent hash-table.
    ~FxStat_HeapEntryDesc();

protected:
    FxStat_HeapDesc&    GetHeapDesc() { return *pHeapDesc; }

private:
    FxStat_HeapDesc*    pHeapDesc;
    unsigned            Id;
    unsigned            ExpireCounter;

    HistoryType         History;

    // Extra last frame stats; recorded in addition to history.
    UPInt               AllocCount;
    UPInt               FilePeak;
};


///////////////////////////////////////////////////////////////////////////////
class FxStat_HeapDesc : public RefCountBase<FxStat_HeapDesc, Stat_DebugTracker_Mem>
{
public:
    typedef ArrayLH<Ptr<MemoryHeap> > HeapCacheType;

public:
    FxStat_HeapDesc(FxStat_HeapDesc* pparent, MemoryHeap* pheap);
    ~FxStat_HeapDesc();

public:

    MemoryHeap*            GetHeapPtr() const      { return pHeap_Unsafe; }
    const SF::StringLH&        GetName() const         { return HeapName; }
    unsigned                GetFlags() const        { return HeapFlags; }
    const HeapCacheType&    GetHeapCache() const    { return TempUpdateHeaps; }

    const MemStatistic& GetMemLocal() const    { return MemLocal; }
    // Including children heaps.
    const MemStatistic& GetMemChildren() const { return MemChildren; }
    // Including children heaps and debug objects.
    const MemStatistic& GetMemDebugChildren() const { return MemDebugChildren; }

    UPInt                   GetAllocCount() const        { return AllocCount; }

    const StatBag&         GetSums() const                     { return Sums; }

    unsigned        GetExpireMax() const    { return FxStat_HistorySize; }
    bool            IsExpired() const       { return true; }
    bool            IsDead() const          { return GetHeapPtr() == 0; }

    void            MarkAsDead() { pHeap_Unsafe = 0; }
    void            Revive(MemoryHeap* pheap);



public:
    // ***** Update Logic

    // Heap updating is split up into three stages:
    //   1. UpdateCollectStats   - Should be called to refresh/collect frame stats
    //                             before child elements are updated.
    //   2. UpdateChildTotals    - Should be called after children have been updated.
    //   3. FinalizeUpdate       - Called externally to finalize update, recursive.

    void        UpdateCollectStats(const FxStatUpdate& update);
    void        UpdateChildTotals(const FxStatUpdate& update);
    void        FinalizeUpdate(const FxStatUpdate& update);

public:
    // *** Entry Creation/Update APIs.

    // Creates an entry for the id, matching one if it already exists.
    FxStat_HeapEntryDesc*   CreateEntry(unsigned id);

    // Updates history of an entry by recording Stat values for it; not that a
    // values of 0 are valid although they cause the Expire counter to be decremented.    
    // If 0 is recorded,
    // history expiration counter starts being decremented. Returns true if the
    // entry has not expired, false if it has (and should be removed).
    bool                    UpdateEntry(FxStat_HeapEntryDesc* pentry);
    bool                    UpdateEntry(FxHeapSummaryElementStatItem* pentry) const;

    // Helper method called from ~FxStat_HeapEntryDesc (on user release).
    void                    RemoveEntry(unsigned id) { Entries.Remove(id); }
    // Helper to remove child heap.
    void                    RemoveChildHeap(FxStat_HeapDesc* );

    
    // Creates a child heap. This function will also detect heap matches across
    // tree views, but only if there is an exact match. In-exact matches are
    // identified directly by tree insertion code in FxHeapStatItem::Update.
    FxStat_HeapDesc*        CreateChildHeapDesc(MemoryHeap* pheap);

private:
    FxStat_HeapDesc* GetThis() { return this; }

private:
    // Lockable heaps can be queried directly (after they are AddRefed),
    // since their stat access is locked and safe.
    // - Non-lockable heap stats can only be populated through a thread
    //   local callback.
    bool        Lockable; //


    // Data from this frame.
    // Due to asynchronous nature of query, frame count can be different.
    StatBag         Sums, Peaks;
    unsigned        Frames;

    SF::StringLH    HeapName;
    unsigned        HeapFlags;

    MemoryHeap*     pHeap_Unsafe;

    // History for different stat ids.
    HashLH<unsigned, FxStat_HeapEntryDesc*> Entries;

    // A list of all child heap descriptors; these are searched exhaustively 
    // for name + pointer match. Thins is easier to implement then hash
    // table since pointer value can change if a different heap takes place
    // of existing one with the same name.
    ArrayLH<FxStat_HeapDesc*> ChildHeaps;

    // Parent descriptor, if not null.
    FxStat_HeapDesc*        pParent;

    unsigned                ExpireCounter;

    // Current memory use, obtained from Stats.
    MemStatistic MemLocal;
    MemStatistic MemChildren;
    MemStatistic MemDebugChildren;

    // Current allocation number, obtained from Stats.
    UPInt       AllocCount;


    // Temporaries used during Update.
    // UpdateHeapRefs is cached until the end of Update call to make sure that 
    // all stat displays are synchronized. It is only used if Updated flag is
    // not set.
    bool            Updated;
    HeapCacheType   TempUpdateHeaps;
};



///////////////////////////////////////////////////////////////////////////////
// FxHeapStatItem represents a root item describing a heap.
// Heap items will contain a list of HeapElementItems, describing the
// number and size of allocations within each stat category, and a list
// of their nested child heap items.
//

// *** Heap report:
//  Heap (48K)
//    Pages: 12 x 4K
//    Free: 894 Bytes
//  - Allocations  [Alloc#] ....
//    - Memory


class FxHeapStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
public:
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;
    typedef MemoryHeap ItemCreateType;

public:
    
    // Construct root heap item; usually only called once for the global heap.
    FxHeapStatItem(FxStatItem* pparent, FxStat_HeapDesc *pheapDesc,
                   const char* pname, bool showDebugHeaps = false);
    // Construct a child heap item, nested in a parent heap.
    FxHeapStatItem(FxHeapStatItem* pparent, ItemCreateType *pheapDesc);
    virtual ~FxHeapStatItem();


public:
    const FxStat_HeapDesc&
                    GetHeapDesc() const { return *pHeapDesc; }

    // Expired state is tracked by the user so it is reported by a virtual function.
    virtual bool    IsExpired() const           { return pHeapDesc->IsExpired(); }
    virtual void    SetExpireText(bool expired) { SetText(expired ? "(Unloaded)" : ""); }

    void            MarkAsDead()
    {
        SetDead(true);
        pHeapDesc->MarkAsDead();
        SetExpireText(true);
    }
    void            Revive(ItemCreateType* pdata)
    {
        SetDead(false);
        pHeapDesc->Revive(pdata);        
        SetExpireText(false);
    }


    // Merge implementation, which considers URL as sort basis.   
    bool            Merge_Equals(const ItemCreateType* const pother) const
    {
        return (pHeapDesc->GetHeapPtr() == pother) && (pHeapDesc->GetName() == pother->GetName());
    }
    bool            Merge_PartialEquals(const ItemCreateType* const pother) const
    {
        return (pHeapDesc->GetHeapPtr() == 0) && (pHeapDesc->GetName() == pother->GetName());
    }
    bool            Merge_ComesBefore(const ItemCreateType* const pother) const
    {
        return pHeapDesc->GetName() < pother->GetName();
    }

    virtual void    Update(const FxStatUpdate& update);

    virtual void Accept(Visitor& v);

private:
    bool                        ShowDebugHeaps;

    Ptr<FxStat_HeapDesc>       pHeapDesc;

    // List contains:
    //   - Heap
    //   - Implementations
    //   - Resources

    // There can be multiple of these in the future.
    Ptr<FxMessageStatItem>         pHeapInfoItem0;    
    Ptr<FxHeapSummaryRootStatItem> pHeapElements0;

    Ptr<FxMessageStatItem>         pHeapInfoItem1;    
    Ptr<FxHeapElementStatItem>     pHeapElements1;    
    ArrayLH<Ptr<FxHeapStatItem> > ChildHeapItems;
};



///////////////////////////////////////////////////////////////////////////////
// Reports sub-category of heap allocations
// Each FxHeapElementStatItem represents a single memory StatId in a heap.

class FxHeapElementStatItem : public FxStatItem
{
public:
    FxHeapElementStatItem(FxStatItem* pparent, unsigned groupId,
                          FxStat_HeapEntryDesc* pentry );
 
public:
    bool    IsExpired() const       { return pHeapEntry->IsExpired(); }
    unsigned    GetHeapEntryId() const  { return pHeapEntry->GetId(); }
    unsigned    GetStatId() const       { return GroupId; }

    const FxStat_HeapEntryDesc&
            GetHeapEntryDesc() const    { return *pHeapEntry; }


    virtual UPInt       GetChildCount() const;
    virtual FxStatItem* GetChild(UPInt index) const;

    virtual void        Update(const FxStatUpdate& update);

    //Graph functionality
    //virtual UPInt    GetPointsCount();     
    //virtual void     GetGraphData(UPInt index, Array<Double> *pdata) const;
    //virtual Double   GetItemGraphData(UPInt index) const;

    //virtual void     OnInitGraph();
    //static  Double   ScaleCalc(Double maxValue); //Calculate scale for maximum graph point

    virtual void Accept(Visitor& v);

private:
    FxHeapElementStatItem& operator =(const FxHeapElementStatItem&);

private:
    // Child items, if any.
    ArrayLH<Ptr<FxHeapElementStatItem> >  Children;
    const unsigned                          GroupId;

    Ptr<FxStat_HeapEntryDesc>  pHeapEntry;
};

///////////////////////////////////////////////////////////////////////////////
class FxSummaryStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
public:
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

    class GSummaryVisitor : public FxStatItem::Visitor
    {
    public:
        virtual ~GSummaryVisitor() {}

    public:
        const MemStatistic& GetImageStat() const       { return ImageStat; }
        const MemStatistic& GetSoundStat() const       { return SoundStat; }
        const MemStatistic& GetVideoStat() const       { return VideoStat; }
        const MemStatistic& GetMeshCacheStat() const   { return MeshCacheStat; }
        const MemStatistic& GetFontStat() const        { return FontStat; }
        const MemStatistic& GetMovieDataStat() const   { return MovieDataStat; }
        const MemStatistic& GetMovieViewStat() const   { return MovieViewStat; }

        GSummaryVisitor& Init();

    public:
        virtual void Visit_FxHeapElementStatItem(FxHeapElementStatItem&);
        virtual void Visit_FxHeapSummaryElementStatItem(FxHeapSummaryElementStatItem&);

    private:
        MemStatistic ImageStat;
        MemStatistic SoundStat;
        MemStatistic VideoStat;
        MemStatistic MeshCacheStat;
        MemStatistic FontStat;
        MemStatistic MovieDataStat; // MovieData + MovieDef
        MemStatistic MovieViewStat;
    };

public:

    // Construct root heap item; usually only called once for the global heap.
    FxSummaryStatItem(
        FxStatItem* pparent, 
        const char* pname
        );
    ~FxSummaryStatItem();

public:
    virtual  void   Update(const FxStatUpdate& update);

    GSummaryVisitor& GetVisitor() { return Visitor; }

private:
    typedef const MemStatistic& (GSummaryVisitor::*StatFuncType)() const;

public: // for Wii compiler 4.3 145
    class StatItem : public FxStatItem
    {
    public:
        StatItem(FxStatItem* pparent, const String& name, GSummaryVisitor& v, StatFuncType func);

    public:
        virtual void Update(const FxStatUpdate&);

    private:
        StatItem& operator =(const StatItem&);

    private:
        const String       Name;
        GSummaryVisitor*    pVisitor;
        StatFuncType        Function;
    };

private:
    Ptr<FxMessageStatItem>     pHeapInfoItem1;    
    GSummaryVisitor             Visitor;
    ArrayLH<Ptr<StatItem> >   ChildItems;
};


///////////////////////////////////////////////////////////////////////////////
// Reports System Summary (System Memory Footprint etc.)
class FxSystemSummaryStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;
       
    class StatInfoProvider
    {
        MemoryHeap::RootStats Stats;
    public:
        StatInfoProvider () { Update(); }
        void  Update () { Memory::pGlobalHeap->GetRootStats(&Stats); }

        UPInt GetSysMemFootprint() const      { return Stats.SysMemFootprint; }
        UPInt GetSysMemUsedSpace() const      { return Stats.SysMemUsedSpace; }
        UPInt GetBookkeepingFootprint() const { return Stats.BookkeepingFootprint + Stats.PageMapFootprint; }
        UPInt GetBookkeepingUsedSpace() const { return Stats.BookkeepingUsedSpace + Stats.PageMapUsedSpace; }
        UPInt GetDebugInfoFootprint() const   { return Stats.DebugInfoFootprint; }
        UPInt GetDebugInfoUsedSpace() const   { return Stats.DebugInfoUsedSpace; }
        UPInt GetUserDebugFootprint() const   { return Stats.UserDebugFootprint; }
        UPInt GetUserDebugUsedSpace() const   { return Stats.UserDebugUsedSpace; }
    };
    typedef UPInt (StatInfoProvider::*StatFuncType)() const;

public:

    FxSystemSummaryStatItem(
        FxStatItem* pparent, 
        const char* pname
        );
    ~FxSystemSummaryStatItem() {}

    virtual  void   Update(const FxStatUpdate& update);

    class StatItem : public FxStatItem
    {
    public:
        StatItem(FxStatItem* pparent, const String& name, const StatInfoProvider& statInfo, StatFuncType func);
        virtual void Update(const FxStatUpdate&);
        UPInt GetPeakStat() const {return Peak;}
        UPInt GetCurrentStat() const {return Current;}
    private:
        StatFuncType        GetStat;
        const StatInfoProvider*   pStatInfo;
        UPInt Current;
        UPInt Peak;
    };

private:
    StatInfoProvider StatInfo;
    ArrayLH<Ptr<StatItem> >   ChildItems;
    Ptr<FxMessageStatItem>     pHeapInfoItem;  
};


///////////////////////////////////////////////////////////////////////////////
//class FxHeapSummaryRootStatItem;

class FxHeapSummaryElementStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;
    typedef FxHeapSummaryRootStatItem ParentClass;

public:
    FxHeapSummaryElementStatItem(
        FxStatItem* pparent,
        ParentClass& parent,
        unsigned statId
        );

public:
    virtual void Update(const FxStatUpdate& update);

    const ParentClass& GetParent() const { return Parent; }
    ParentClass& GetParent() { return Parent; }

    inline const FxStat_HeapEntryDesc& GetHeapEntryDesc() const;

    unsigned        GetStatId() const { return StatId; }

    void            SetCounter(UPInt v) { Counter = v; }
    UPInt           GetCounter() const { return Counter; }

    void            SetPeakCounter(UPInt v) { PeakCounter = v; }
    UPInt           GetPeakCounter() const { return PeakCounter; }

    virtual void    Accept(Visitor& v);

private:
    FxHeapSummaryElementStatItem& operator =(const FxHeapSummaryElementStatItem&);

private:
    ParentClass&    Parent;
    const unsigned  StatId;
    UPInt           Counter;
    UPInt           PeakCounter;
};

class FxHeapSummaryRootStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:
    FxHeapSummaryRootStatItem(
        FxStatItem* pparent, 
        unsigned groupId,
        FxStat_HeapEntryDesc* pentry
        );

public:
    virtual void    Update(const FxStatUpdate& update);

    const FxStat_HeapEntryDesc& GetHeapEntryDesc() const { return *pHeapEntry; }

    UPInt GetTotal() const { return Total; }
    void  SetTotal(UPInt v) { Total = v; }

    virtual void    Accept(Visitor& v);

private:
    FxHeapSummaryRootStatItem* GetThis() { return this; }

private:
    FxHeapSummaryRootStatItem& operator =(const FxHeapSummaryRootStatItem& other);

private:
    typedef ArrayLH<Ptr<FxHeapSummaryElementStatItem> > ChildrenType;

    const unsigned              GroupId;
    unsigned                    Frames;
    ChildrenType                Children;
    UPInt                       Total;

    Ptr<FxStat_HeapEntryDesc>  pHeapEntry;
};

inline
const FxStat_HeapEntryDesc& 
FxHeapSummaryElementStatItem::GetHeapEntryDesc() const 
{ 
    return GetParent().GetHeapEntryDesc(); 
}

///////////////////////////////////////////////////////////////////////////////
// Represents the memory root which has the following items
//
//  - Summary
//  - Heaps
//  - Video Memory
//  - Debug

class FxMemoryRootStatItem : public FxStatItem_ChildSupport<FxStatItem>
{
    typedef FxStatItem_ChildSupport<FxStatItem> BaseClass;

public:
    FxMemoryRootStatItem(FxGlobalStatTracker* pstatTracker);

public:
    virtual FxGlobalStatTracker* GetStatTracker() const { return pStatTracker; }

    virtual void    GetName(String* pstring);
    //virtual void    GetText(String* pstring);

    virtual void    Update(const FxStatUpdate& update);
    virtual void    Accept(Visitor& v);

    bool IsEnabled() const { return Enabled; }

private:
    FxGlobalStatTracker*        pStatTracker;
    bool                        Enabled;

    // Stat items
    Ptr<FxHeapStatItem>        pHeaps;
    //Ptr<FxHeapStatItem>        pDebugHeaps;
    Ptr<FxSummaryStatItem> pSummary;
    Ptr<FxSystemSummaryStatItem> pSystemSummary;
};



#endif // INC_FxHeapStatItem_H
