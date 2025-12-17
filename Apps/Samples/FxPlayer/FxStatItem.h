/**************************************************************************

Filename    :   FxStatItem.h
Content     :   Statistics reporting base item for FxPlayer 
Created     :   September 23, 2008
Authors     :   Michael Antonov, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxStatItem_H
#define INC_FxStatItem_H

#include "GFx.h"
#include "GFx_Kernel.h"

// ***** Classes declared

namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;

class FxStatItem;
class FxStatUpdate;
class FxStatsTreeUpdate;
class FxStatItemChildArray;

//template<class Base>
//class FxStatItem_ChildSupport ;
//template<class ItemBase, class ICreateType>
//struct FxStatItem_ExpireBase;

// Forward declarations
class FxGlobalStatTracker;




// ***** FxStatItem 

// Contains shared data that items can use during their update,
// such as the previous value of stat bags.


// Tree update interface.
// Functions of this interface get called to inform up of
// insertion and removal of items in the tree.
//class FxStatsTreeUpdate
//{
//public:
//    virtual void    Insert(FxStatItem* pitem/*, UPInt childIndex, UPInt count*/) = 0;
//    virtual void    Remove(FxStatItem* pitem/*, UPInt childIndex, UPInt count*/) = 0;
//};


// ***** Stat Item Base

// This is the base item in a tree; it can have children that are expanded
// based on the tree control's needs.
// 
 
/* *** Stat Item States

Depending on the way the item is displayed, it can be in several possible
states:

 Shown      - Indicates that the item is part of an expanded branch of the tree
              and is not explicitly hidden.
 Hidden     - Items marked as hidden are not displayed as a part of the tree 
              until the later is refreshed, the newly introduced items are
              usually marked as such.
 Dead       - An item is dead if it no longer represents a live object in memory.
              Dead item will usually exist until it (a) expires and (b) the tree 
              is refreshed. History is tracked for dead items until expiration.
              Dead items can be revived if they are matched for a new item.
 Expired    - Once an item dies, expire counter is decremented for it. Once this
              counter reaches 0, the item becomes 'Expired'. Expired items will
              be removed from the tree if they are not shown.
 
 Possible sate combinations:
  { Hidden }
  { Hidden, Dead }
  { Hidden, Dead, Expired } - As soon as this is hit, the item is removed.
  { Shown }
  { Shown, Dead }
  { Shown, Dead, Expired }  - Item will be removed on next refresh.


  *** Item Creation

  All of the stats for the tree come from one root item. When this item is
  created, it usually initializes only a minimal set of internal child states,
  the full tree is not necessarily captured. To complete initialization users
  should call Update(Update_Init).

  Once the the item is thus initialized, further updating is done by the
  following methods:
    Update(Update_Frame)
    Update(Update_Tick)
    Refresh() ? - Inserts/Removes items based on Hidden/Dead state.
  
*/


// Update structure used for 


class FxStatUpdate
{
public:
    enum Type
    {
        Update_Init,    // Tree initialization is taking place.
        Update_Tick,    // History tick and complete update.
        Update_Frame    // Partial update for a single frame, no visual tree changes.
    };

public:
    FxStatUpdate(Type type, FxGlobalStatTracker* pdata, FxStatsTreeUpdate* ptreeUpdate)
        : UpdateType(type), pTracker(pdata), pTreeUpdate(ptreeUpdate)
    { }
    FxStatUpdate(const FxStatUpdate& src)
        : UpdateType(src.UpdateType), pTracker(src.pTracker), pTreeUpdate(src.pTreeUpdate)
    { }

public:
    Type GetType() const    { return UpdateType; }
    bool IsInit() const     { return GetType() == Update_Init; }
    bool IsTick() const     { return GetType() == Update_Tick; }
    bool IsFrame() const    { return GetType() == Update_Frame; }    

private:
    Type                  UpdateType;
    FxGlobalStatTracker*  pTracker; // Not used at the moment.
    FxStatsTreeUpdate*    pTreeUpdate; // Not used at the moment.
};


// Stores cashed values of Graph parameters
//class GraphInfo : public RefCountBase<GraphInfo, Stat_DebugTracker_Mem>
//{
//public:
//    Double GetScale() const { return Scale; }
//    unsigned   GetGraphCount() const { return GraphCount; }
//
//    void   SetScale (Double scale) { Scale = scale; }
//    void   SetGraphCount (unsigned graphCount) { GraphCount = graphCount; }
//
//private:
//    //GraphFlags Flags;
//    Double Scale;
//    unsigned   GraphCount;
//};

///////////////////////////////////////////////////////////////////////////////
// Forward declaration for Visitor.
class FxHeapStatItem;
class FxHeapElementStatItem;
class FxResourceRootStatItem;
class FxMovieDataStatItem;
class FxMovieDefStatItem;
class FxHeapSummaryRootStatItem;
class FxHeapSummaryElementStatItem;

///////////////////////////////////////////////////////////////////////////////
class FxStatItem : public RefCountBase<FxStatItem, Stat_DebugTracker_Mem>
{
public:

    enum StatType
    {
        ST_Unknown,
        ST_ResourceRoot,

        ST_Counts,
        ST_Ticks,
        ST_Memory
    };

    // Different states
    enum StatFlags
    {
        SF_Shown     = 0x0001,      // Item is currently shown in tree,
                                    // i.e. is a part of an expanded branch.
        SF_Hidden    = 0x0002,      // Item is not rendered as a part of the tree.
        SF_Dead      = 0x0004,      // Item is no longer alive (it can still be shown).
       
        SF_Expanded  = 0x0010,      // This particular item itself is expanded.
        SF_Changed   = 0x0020,      // This item is removed from the tree, or has added or removed 
        
        SF_Graphable = 0x0100,      // Item can be graphed.
    };

    class Visitor
    {
    public:
        virtual ~Visitor() {}

    public:
        virtual void Visit_FxStatItem(FxStatItem&) {}
        virtual void Visit_FxHeapElementStatItem(FxHeapElementStatItem&) {}
        virtual void Visit_FxHeapStatItem(FxHeapStatItem&) {}
        virtual void Visit_FxResourceRootStatItem(FxResourceRootStatItem&) {}
        virtual void Visit_FxMovieDataStatItem(FxMovieDataStatItem&) {}
        virtual void Visit_FxMovieDefStatItem(FxMovieDefStatItem&) {}
        virtual void Visit_FxHeapSummaryRootStatItem(FxHeapSummaryRootStatItem&) {}
        virtual void Visit_FxHeapSummaryElementStatItem(FxHeapSummaryElementStatItem&) {}
    };

protected:
    //
    //Ptr<GraphInfo> pGraphInfo; //NULL, if item is not currently displayed

public:    
    // Explicitly pass NULL for pparent if you want this item to be a root node.
    FxStatItem (FxStatItem* pparent, unsigned flags = 0) : pParent(pparent), Flags(flags) {}
    virtual ~FxStatItem() {}
      
    // Item type & general information.
    virtual StatType    GetType() const     { return ST_Unknown; }
    // Parent can be null for the stat item.
    FxStatItem*         GetParent() const   { return pParent; }

    // Access stat tracker; this function must be overriden in the root.
    virtual FxGlobalStatTracker* GetStatTracker() const
    {
        SF_ASSERT(pParent); return pParent->GetStatTracker();
    }

    // Flag accessors.
    inline  unsigned    GetFlags() const    { return Flags; }
    void                SetFlags(unsigned flags) { Flags = flags; }
    
    inline bool         IsShown() const     { return (GetFlags() & SF_Shown) != 0; }
    inline bool         IsHidden() const    { return (GetFlags() & SF_Hidden) != 0; }
    inline bool         IsDead() const      { return (GetFlags() & SF_Dead) != 0; }
    inline bool         IsExpanded() const  { return (GetFlags() & SF_Expanded) != 0; }
    inline bool         IsChanged() const   { return (GetFlags() & SF_Changed) != 0; }

    inline void         SetShown(bool v)    { SetFlags((GetFlags()&~SF_Shown) | (v ? SF_Shown : 0) ); }
    inline void         SetHidden(bool v)   { SetFlags((GetFlags()&~SF_Hidden) | (v ? SF_Hidden : 0) ); }
    inline void         SetDead(bool v)     { SetFlags((GetFlags()&~SF_Dead) | (v ? SF_Dead : 0) ); }
    inline void         SetExpanded(bool v) { SetFlags((GetFlags()&~SF_Expanded) | (v ? SF_Expanded : 0) );  }
    inline void         SetChanged(bool v)  { SetFlags((GetFlags()&~SF_Changed) | (v ? SF_Changed : 0) );  }

    // Expired state is tracked by the user so it is reported by a virtual function.
    virtual bool        IsExpired() const   { return IsDead(); }
 

    virtual unsigned    GetIndexInParent() const
    {
        if (!pParent)
            return 0;
        unsigned i = 0;
        while (this != pParent->GetChild(i++));
        return i-1;
    }


public:
    // *** Name & Text

    // Name is displayed to the left of the tree control. It can be
    // followed by an additional Text string which     
    virtual void        GetName(String* pstring) { *pstring = Name; }

    // Text string is used to report additional information next to the name.
    // Usually this is information that can change with time, such as a number of 
    // resources or child item memory used. In addition, Text supports column
    // tags, so that display columns can be accumulated.
    //
    //  Columns:
    //   - A column will be generated if Text starts with '|' character. Successive
    //     uses of '|' will delimit content of subsequent columns. Additional format
    //     characters:
    //         "|>"   - Generates right aligned column.
    //         "|<"   - Generates left aligned column.
    //         "|12<" - Generates left align column of minimum 12 characters.
    virtual void        GetText(String* pstring) { *pstring = Text; }     

    // Access to child items, if any. These should only need
    // to be accessed if the tree is expanded.
    virtual  UPInt       GetChildCount() const         { return 0; }
    virtual  FxStatItem* GetChild(UPInt index) const    { SF_UNUSED(index); return 0; }

    

    // Updates all of the items, obtaining the new value and history.
    // Stats can also add/remove children. The tree control gets
    // notified about item changes through the ptreeUpdate interfaces,
    // which can help it update its columns.
    virtual  void       Update(const FxStatUpdate& update)
    {
        SF_UNUSED(update);
    }


public:   
    // *** Graph Interface

    bool                IsGraphable() const { return (GetFlags() & SF_Graphable) == 0; }

    // If an item is graphable we can obtain the following:
    //  - Graph scale and label types
    //  - Number of graphs and their type
    //  - Legend
    //  - Actual graph data

    //virtual Double  GetScale() const { return pGraphInfo ? pGraphInfo->GetScale() : 0; }

    //virtual UPInt   GetGraphCount() const { return pGraphInfo ? pGraphInfo->GetGraphCount() : 0; }
    //virtual UPInt   GetPointsCount() const { return 0; }

    virtual void   GetLegend(Array<String>* /*plegend*/) const {}

    // Points for all subgraphs for the index in History
    virtual void   GetGraphData(UPInt /*index*/, Array<Double>* /*pdata*/) const {} 

    // Last points (for all subgraphs)
    //void   GetGraphDataLast(Array<Double> *pdata) const 
    //{
    //    if (GetPointsCount() > 0)
    //        GetGraphData(GetPointsCount() - 1, pdata);
    //} 


    // These functions are called from parent GetGraphData* to get data from children
    //virtual Double GetItemGraphData(UPInt index) const {SF_UNUSED(index); return 0;} 
    //virtual Double GetItemGraphDataLast() {return GetPointsCount() > 0 ? GetItemGraphData(GetPointsCount() - 1) : 0;}

    //virtual void    OnInitGraph() {} 
    //virtual void    OnStopGraph() { pGraphInfo = NULL; }

    virtual void    Accept(Visitor& v);

protected:
    FxStatItem& SetName(const String& name) { Name = name; return *this; }
    FxStatItem& SetText(const String& text) { Text = text; return *this; }

private:
    FxStatItem* pParent;
    unsigned    Flags;
    SF::StringLH   Name, Text;
};





enum { FxStat_HistorySize = 10 };

// ***** FxHistoryRingBuffer

// This is a circular array used for history. The array initializes with
// the number of max elements it can hold and it will accept as many items
// as have been specified. Further calls to Add() will overwrite some of
// the older items in history.

template<class ValueType>
class FxHistoryRingBuffer
{ 
public: 

    FxHistoryRingBuffer(UPInt maxSize)
        : MaxSize(maxSize), 
        Start(0),
	CurrentSize(0),
        Values((int)maxSize) 
    { 
    } 

    // Clears the history, resetting size to 0.
    void    Clear() 
    { 
        Start       = 0;        
        CurrentSize = 0; 
    }

    void    PushBack(const ValueType& v)    
    {
        // If size hasn't reached the maximum, we increment it and insert items
        // based on it. If the size has overflowed, we overwrite the first Start
        // element, adjusting the list.

        if (CurrentSize < MaxSize)
        {
            Values[CurrentSize] = v;
            CurrentSize++;
        }
        else
        {
            Values[Start] = v;
            Start++;
            if (Start == MaxSize)
                Start = 0;
        }
    }

    // Indexes items from the start of the history (the oldest item is at index 0). 
    // We can access up to the GetSize() number of items.
    const ValueType&    operator[] (UPInt index) const 
    { 
        SF_ASSERT(index < CurrentSize); 
        if (Start + index < MaxSize) 
            return Values[Start + index]; 
        else 
            return Values[Start + index - MaxSize]; 
    } 

    const ValueType&    Back() const
    {
        SF_ASSERT(CurrentSize > 0);
        return (*this)[CurrentSize-1];
    }

    UPInt               GetSize() const  { return CurrentSize; } 

private:     
    UPInt                   MaxSize;  
    UPInt                   Start, CurrentSize;

    ArrayLH<ValueType>     Values; 
};




// A stat item while Expiration support.
//

template<class ItemBase, class ICreateType>
class FxStatItem_ExpireBase : public ItemBase
{
public:
    typedef ICreateType ItemCreateType;

public:
    FxStatItem_ExpireBase(FxStatItem* pparent, ItemCreateType* pdata)
        : ItemBase(pparent)
    {
        pData_Unsafe  = pdata;
        ExpireCounter = FxStat_HistorySize;
    }

public:
    // Expired state is tracked by the user so it is reported by a virtual function.
    virtual bool    IsExpired() const           { return true; }
    //virtual bool    IsExpired() const             { return ExpireCounter == 0; }
    virtual void    SetExpireText(bool expired) { this->SetText(expired ? "(Unloaded)" : ""); }

    virtual void    MarkAsDead()
    {
        this->SetDead(true);
        pData_Unsafe = 0;
        SetExpireText(true);
    }
    void            Revive(ItemCreateType* pdata)
    {
        this->SetDead(false);
        pData_Unsafe = pdata;
        SetExpireText(false);
        ExpireCounter = FxStat_HistorySize;
    }

    void            PostUpdate(FxStatUpdate::Type type)
    {
        if (type == FxStatUpdate::Update_Tick && !pData_Unsafe && (ExpireCounter > 0))
            ExpireCounter--;
    }

    // Default implementation of merge insertion functions.
    // It is best to override those per item, as exact pointer compare is bad for dead items.
    bool            Merge_Equals(const ItemCreateType* const pother) const        { return pData_Unsafe == pother; }
    bool            Merge_PartialEquals(const ItemCreateType* const pother) const { return false; }
    bool            Merge_ComesBefore(const ItemCreateType* const pother) const   { return pData_Unsafe < pother; }

    ItemCreateType* GetItemCreateData() const { return pData_Unsafe; }

private:
    ItemCreateType* pData_Unsafe;
    UPInt           ExpireCounter;
};


// *** General Purpose Item Child Array

// This helper class fakes a single linear Child Array by concatenating a set of
// child items and typed item sub-arrays into one list. It is used to avoid having to
// implement custom GetChild logic in every item.
//
// TBD: Indexing is currently slow; it can be optimized by introducing a last-access
// cached index. If this is done, we would also need invalidate logic.


class FxStatItemChildArray
{
    struct  ArrayWrapper : public RefCountBase<FxStatItem, Stat_DebugTracker_Mem>
    {
        virtual UPInt       GetSize() const       = 0;
        virtual FxStatItem* At(UPInt index) const = 0;
    };

    template<class ListClass>
    class ArrayWrapperImpl : public ArrayWrapper
    {
    public:
        ArrayWrapperImpl(ListClass* plist) : pList(plist) { SF_ASSERT(pList); }

    public:
        virtual UPInt       GetSize() const        { return pList->GetSize(); }
        virtual FxStatItem* At(UPInt index) const  { return (*pList)[index]; }

    private:
        ListClass* pList;
    };

    class Element
    {
    public:
        enum EType
        {
            ET_None,
            ET_Item,
            ET_ArrayList
        }  Type;

    public:
        Element() : Type(ET_None), pItem(0) { }
        Element(FxStatItem *pitem) : Type(ET_Item), pItem(pitem) { }
        Element(ArrayWrapper *parray) : Type(ET_ArrayList), pArray(parray) { }

        Element(const Element& src) : Type(src.Type), pItem(src.pItem)
        { if (Type == ET_ArrayList) pArray->AddRef(); }
        ~Element()
        { if (Type == ET_ArrayList) pArray->Release(); }

    public:
        void operator = (const Element& src)
        {
            if (src.Type == ET_ArrayList) pArray->AddRef();
            if (Type == ET_ArrayList)     pArray->Release();
            Type  = src.Type;
            pItem = src.pItem;
        }

        UPInt GetSize() const
        {
            if (Type == ET_Item)            return 1;
            else if (Type == ET_ArrayList)  return pArray->GetSize();
            return 0;
        }

        bool IsItem() const      { return (Type == ET_Item); }
        bool IsArrayList() const { return (Type == ET_ArrayList); }

        FxStatItem*         GetItem() const { SF_ASSERT(IsItem()); return pItem; }
        const ArrayWrapper& GetArray() const { SF_ASSERT(IsArrayList()); return *pArray; }

    private:
        union {
            FxStatItem*     pItem;
            ArrayWrapper*   pArray;
        };
    };

    typedef ArrayLH<Element> ElementsType;

public:
    FxStatItemChildArray() { }
    ~FxStatItemChildArray() { }

public:
    // Add items to array.
    void        AddChild(FxStatItem *pitem)
    {
        Elements.PushBack(Element(pitem));        
    }

    template<class ArrayClass>
    void        AddChildArrayList(ArrayClass* parray)
    {
        ArrayWrapper* pwrapper = SF_HEAP_AUTO_NEW(this) ArrayWrapperImpl<ArrayClass>(parray);
        Elements.PushBack(Element(pwrapper));
    }

    // Array Access implementation - in source file.
    UPInt       GetChildCount() const;
    FxStatItem* GetChild(UPInt index) const;

private:
    ElementsType Elements;
};


// FxStatItem base class wrapper that implements child array support.

template<class Base>
class FxStatItem_ChildSupport : public Base
{
public:
    // FxStatItem_ChildSupport() : Base() { }
    FxStatItem_ChildSupport(FxStatItem* pitem) : Base(pitem) { }    

public:
    virtual  UPInt       GetChildCount() const       { return ChildRefs.GetChildCount(); }
    virtual  FxStatItem* GetChild(UPInt index) const { return ChildRefs.GetChild(index); }

    // Add functionality (delegated).
    void        AddChild(FxStatItem *pitem)           { ChildRefs.AddChild(pitem); }

    template<class ArrayClass>
    void        AddChildArrayList(ArrayClass* parray) { ChildRefs.AddChildArrayList(parray); }    

protected:
    FxStatItemChildArray ChildRefs;
};




// A simple tree item that displays a line of text and has not childen.

class FxMessageStatItem : public FxStatItem
{ 
public:
    FxMessageStatItem(FxStatItem* pparent, const String& name, const String& text)
        : FxStatItem(pparent)
    {
        SetMessage(name, text);
    }

public:
    // Modify message based on potentially new status
    void    SetMessage(const String& name, const String& text)
    {
        SetName(name); 
        SetText(text);
    }
};




// ***** General-Purpose Item list Merge Logic

template<class ParentItem, class Item, class Element>
class FxArrayMergeAccessor
{
public:
    typedef Array<Element> ArrayType;

public:
    FxArrayMergeAccessor(ParentItem *pparent, ArrayType& src)
        : pThisItem(pparent), pSource(&src)
    { }

public:
    // Source array manipulation.
    UPInt           GetSize() const     { return pSource->GetSize(); }
    void            RemoveAt(UPInt i)   { pSource->RemoveAt(i); }
    Element&        At(UPInt i)         { return (*pSource)[i]; }


    // *** Methods used by Fx_MergeItems

    // Mark items as dead/alive.
    void            MarkAsDead(Item *pitem)      { pitem->MarkAsDead(); }    
    void            Revive(Item *pitem, UPInt i) { pitem->Revive(At(i)); }
    // Comparison - for insertion.
    bool            Equals(Item* pitem, UPInt i)        { return pitem->Merge_Equals(At(i)); }
    bool            PartialEquals(Item* pitem, UPInt i) { return pitem->Merge_PartialEquals(At(i)); }
    bool            ComesBefore(Item* pitem, UPInt i)   { return pitem->Merge_ComesBefore(At(i)); }

    Item*     Create(UPInt i)
    { Item* pitem = SF_HEAP_AUTO_NEW(pThisItem) Item(pThisItem, At(i)); return pitem; }    

private:
    ParentItem*     pThisItem;
    ArrayType*      pSource;
};



template<class Item, class SourceAccessor, class UpdateClass>
void    Fx_MergeItems(ArrayLH<Ptr<Item> >& a, SourceAccessor& s,
                      UpdateClass& update)
{
    // At this point we need to do a merge, but we can't do so with a basic
    // algorithm since the ChildHeaps list is not strictly sorted (it can
    // have several entries Dead with a valid name but no heap pointer).

    // We separate the problem as follows:
    //  1. First run through the existing list and see what items are no longer
    //     in the heaps list. Update and mark these as dead.
    //        - Remove any items which expired.
    //  2. Run through the new Heaps and try to find exact match.
    //        - Update those items.
    //  3. Run through the new heaps and see if there is a dead match by name.
    //        - Revive and update those.
    //  4. Any remaining items are new with no available 'dead' sports. Insert them.

    UPInt i, j;

    // Steps (1) and (2): Take care of dead and exactly matching items.
    for (i=0; i< a.GetSize(); i++)
    {
        Item* pchildItem = a[i];
        bool  found      = false;

        for (j=0; j< s.GetSize(); j++)
        {
            if (s.Equals(pchildItem, j))
            {
                // Exact match, update and remove.
                pchildItem->Update(update);
                s.RemoveAt(j);
                found = true;
                break;
            }
        }

        // If not found, the item is dead or a death candidate.
        if (!found)
        {
            s.MarkAsDead(pchildItem);
            pchildItem->Update(update);

            if (pchildItem->IsExpired() )
            {
                if (pchildItem->IsShown())
                    pchildItem->SetChanged(true);
                else
                {

                //if (pchildItem->GetParent()->IsExpanded() && update.pTreeUpdate)
                //    update.pTreeUpdate->Remove(pchildItem);
                a.RemoveAt(i);
                i--;
                }
                continue;
            }
        }
    }

    // Steps (3) and (4).
    // Run through items and try to do a dead match by name.

    for (j=0; j< s.GetSize(); j++)
    {
        //   const char*  pheapName   = visitor.Heaps[j]->GetName();
        //   MemoryHeap* pheap       = visitor.Heaps[j];
        UPInt        insertPoint = 0;

        for (i=0; i< a.GetSize(); i++)
        {
            Item* pchildItem = a[i];

            if (s.PartialEquals(pchildItem, j))
            {
                // Revive dead item.
                s.Revive(pchildItem, j);
                //pchildItem->SetDead(false);
                //pchildItem->pHeapDesc->Revive(pheap);
                pchildItem->Update(update);
                break;
            }
            else
            {   
                // Sort items by heap name.
                if (s.ComesBefore(pchildItem, j))
                    insertPoint = i;
            }
        }

        // If not found, insert new item
        if (i == a.GetSize())
        {   
            Ptr<Item> pitem = *s.Create(j);
            a.InsertAt(insertPoint, pitem);
            pitem->Update(update);
            FxStatItem* pparent = pitem->GetParent();
            if (pparent && pparent->IsShown() && pparent->IsExpanded())
                pitem->SetHidden(true);
            else
                pitem->SetHidden(false);

            if(pparent && pparent->IsShown() && pparent->IsExpanded())
                pparent->SetChanged(true);

            //if (pitem->GetParent()->IsExpanded() && update.pTreeUpdate)
            //{
            //     update.pTreeUpdate->Insert(pitem);
            //}
        }
    }   
}


#endif // INC_FxStatItem_H
