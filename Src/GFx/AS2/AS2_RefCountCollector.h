/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_RefCountCollector.h
Content     :   Fast general purpose refcount-based garbage collector functions
Last rev    :   April 5, 2011
Created     :   November 25, 2008
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFxl_AS2_RefCountCollector_H
#define INC_SF_GFxl_AS2_RefCountCollector_H

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayPaged.h"

#ifdef SF_BUILD_DEBUG
//#define SF_GC_DO_TRACE
#endif

#ifdef SF_GC_DO_TRACE
#define SF_GC_DEBUG_PARAMS         int level
#define SF_GC_DEBUG_PARAMS_DEF     int level=0
#define SF_GC_DEBUG_PASS_PARAMS    level+1
#define SF_GC_TRACE(s)             Trace(level, s)
#define SF_GC_COND_TRACE(c,s)      do { if(c) Trace(level, s); } while(0)
#define SF_GC_PRINT(s)             printf("%s\n", s)
#define SF_GC_PRINTF(s,p1)         printf(s, p1)
#define SF_GC_PARAM_START_VAL      0
#else
#define SF_GC_DEBUG_PARAMS
#define SF_GC_DEBUG_PARAMS_DEF
#define SF_GC_DEBUG_PASS_PARAMS
#define SF_GC_TRACE(s)
#define SF_GC_COND_TRACE(c,s)
#define SF_GC_PRINT(s)
#define SF_GC_PRINTF(s,p1)
#define SF_GC_PARAM_START_VAL
#endif

namespace Scaleform {
namespace GFx {
namespace AS2 {

template <int Stat>
class RefCountCollector;

// This is a base class for all "refcount-collectables" classes.
// This class might be used instead of regular reference counter
// base class (RefCountBase) to provide functionality similar to
// garbage collection. It is still based on reference counters, however,
// it is possible to detect and free circular references. Use 
// RefCountCollector.Collect method to perform cycle references collection.
// Each instance of the "collectable" class may have so called "children".
// "Children" are strong pointers ("addref-ed", Ptr might be used) to
// another "collectable" objects. A "collectable" object may also contain
// strong or weak pointers on "non-collectable" objects.
//
// Destructors for "collectable" object should NEVER be called. The virtual
// method Finalize_GC will be invoked instead. However, the exact moment
// of this call is undefined.
// There is a limitation of what is possible to do inside the Finalize_GC.
// It is forbidden to touch any of "collectable" children. These pointers
// might be already invalid at the moment. Thus, no Release, no dtor calls
// for "collectables" from inside of the Finalize_GC (dtors are forbidden at all).
// For "non-collectables", such as regular non-refcounted objects, destructors
// should be called implicitly. For regular refcounted members Release functions
// should be called.
//
// Another limitation of Finalize_GC: it should not, directly or indirectly,
// cause invocation of Release for any "collectable" object, since "Release" 
// may cause adding new roots in root array and the system might be already in 
// collecting state. The only exception from this limitation is when it is guaranteed 
// that the refcount == 1 before Release is called. In this case, no roots will be
// added and the object will be destroyed instantly.
//
// To provide "children" to refcount collector it is necessary to add several
// methods in implementation of the "collectable" object. The first method is:
//
//     virtual void ExecuteForEachChild_GC(OperationGC operation) const;
//
// The implementation of this method is quite simple, for example:
//    void ASObject::ExecuteForEachChild_GC(OperationGC operation) const
//    {
//        ASRefCountBaseType::CallForEachChild<ASObject>(operation);
//    }
// Another method that is required to be defined is a template one, ForEachChild_GC:
//     template <class Functor> void ForEachChild_GC(SF_GC_DEBUG_PARAMS_DEF) const;
// Note, this method should be public or its class should be a friend to RefCountBaseGC<..>.
// The ForEachChild_GC should call static Functior::Call(RefCountBaseGC*) method for
// each "collectable child".
// This template method might be implemented inline, in the header file, or in CPP. 
// If the method is implemented in CPP then the source file should have the following
// statement right next to ForEachChild_GC implementation:
//      SF_GC_PREGEN_FOR_EACH_CHILD(ClassName)

template <int Stat = Stat_Default_Mem>
class RefCountBaseGC : public NewOverrideBase<Stat>
{
    friend class RefCountCollector<Stat>;
public:
    typedef class RefCountCollector<Stat> Collector;

    enum OperationGC
    {
        Operation_Release,
        Operation_MarkInCycle,
        Operation_ScanInUse
    };
protected:
    enum
    {
        Flag_Buffered = 0x80000000u,
        Mask_State    = 0x7,
        Shift_State   = 28,
        Flag_InList   = (1L<<27), // if set, pNext/pPrev are in use.
        Flag_DelayedRelease = (1L<<26),
        Mask_RefCount = 0x3FFFFFF
    };
    enum States
    {
        State_InUse     = 0,
        State_InCycle   = 1,
        State_Garbage   = 2,
        State_Root      = 3,
        State_Removed   = 4
    };
    union
    {
        Collector*      pRCC;
        RefCountBaseGC* pNext;
    };
    union
    {
        UInt32          RootIndex;
        RefCountBaseGC* pPrev;
    };
    // Ref counter takes bits 27-0 lower bytes.
    // The most significant bit (bit 31) is flag "buffered".
    // Bits 30-28 - color (7 values)
    UInt32              RefCount;

    SF_INLINE void Increment(SF_GC_DEBUG_PARAMS)
    {
        SF_ASSERT((RefCount & Mask_RefCount) < Mask_RefCount);
        ++RefCount;
        SF_GC_TRACE("Incremented ");
    }
    SF_INLINE unsigned Decrement(SF_GC_DEBUG_PARAMS)
    {
        SF_ASSERT((RefCount & Mask_RefCount) > 0);
        SF_GC_TRACE("Decrementing ");
        return ((--RefCount) & Mask_RefCount);
    }
    SF_INLINE unsigned GetRefCnt()
    {
        return (RefCount & Mask_RefCount);
    }

    void MarkInCycle(Collector* prcc SF_GC_DEBUG_PARAMS);

    struct ReleaseFunctor
    {
        SF_INLINE static void Call(Collector* prcc, RefCountBaseGC* pchild)
        {
            SF_ASSERT(pchild);
            //pchild->Release();
            
            SF_ASSERT(!pchild->IsRefCntZero());
            if (pchild->Decrement() == 0)
            {
                // if refcnt == 1 then we can't call Release since it will cause recursive calls
                // (for each child). Just add to a list.
                prcc->RemoveFromRoots(pchild);
                pchild->SetDelayedRelease();
                prcc->AddToList(pchild);
            }
            else
                pchild->ReleaseInternal();
        }
    };

    struct MarkInCycleFunctor
    {
        SF_INLINE static void Call(Collector* prcc, RefCountBaseGC* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Decrement();
            prcc->AddToList(pchild);
        }
    };
    struct ScanInUseFunctor
    {
        SF_INLINE static void Call(Collector* prcc, RefCountBaseGC* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Increment();
            if (!pchild->IsInState(State_InUse))
            {
                pchild->SetState(State_InUse);
                prcc->ReinsertToList(pchild); // to reinsert
            }
        }
    };

    // This template method is a kind of dispatcher function. It is called
    // from the virtual method ExecuteForEachChild_GC for every "collectable"
    // class that has "collectable" children. The "collectable" class should
    // also have a template method ForEachChild_GC.
    template <class ClassImpl>
    SF_INLINE void CallForEachChild(Collector* prcc, OperationGC op) const
    {
        switch(op)
        {
        case Operation_Release: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<ReleaseFunctor>(prcc); 
            break;
        case Operation_MarkInCycle: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<MarkInCycleFunctor>(prcc); 
            break;
        case Operation_ScanInUse: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<ScanInUseFunctor>(prcc); 
            break;
        default:;
        }
    }

    SF_INLINE bool IsRefCntZero() const { return (RefCount & Mask_RefCount) == 0; }

    SF_INLINE void SetBuffered(UInt32 rootIndex) 
    { 
        RefCount |= Flag_Buffered; RootIndex = rootIndex; 
    }
    SF_INLINE void ClearBuffered()    
    { 
        RefCount &= ~Flag_Buffered; 
        if (!IsInList()) 
            RootIndex = ~0u; 
    }
    SF_INLINE bool IsBuffered() const { return (RefCount & Flag_Buffered) != 0; }

    SF_INLINE void SetInList() 
    { 
        RefCount |= Flag_InList;
    }
    SF_INLINE void ClearInList(Collector* prcc) 
    { 
        RefCount &= ~Flag_InList; 
        pRCC      = prcc;
        ClearBuffered();
    }
    SF_INLINE bool IsInList() const { return (RefCount & Flag_InList) != 0; }

    SF_INLINE void SetDelayedRelease() 
    { 
        RefCount |= Flag_DelayedRelease;
    }
    SF_INLINE void ClearDelayedRelease() 
    { 
        RefCount &= ~Flag_DelayedRelease; 
    }
    SF_INLINE bool IsDelayedRelease() const { return (RefCount & Flag_DelayedRelease) != 0; }

    SF_INLINE void SetState(States c)  
    { 
        RefCount = (RefCount & ~(Mask_State << Shift_State)) | 
                   (((UInt32)c & Mask_State) << Shift_State); 
    }
    SF_INLINE States GetState() const  { return (States)((RefCount >> Shift_State) & Mask_State); }
    SF_INLINE bool IsInState(States c) const { return GetState() == c; }

    SF_INLINE void Free_GC(SF_GC_DEBUG_PARAMS)
    {
        SF_GC_TRACE("Freeing ");
        Finalize_GC();
        SF_FREE(this);
    }

    // Virtual methods, which should be overloaded by the inherited classes.
    virtual void ExecuteForEachChild_GC(Collector*, OperationGC operation) const { SF_UNUSED(operation); }
    virtual void Finalize_GC() = 0;

    void RemoveFromList()
    {
        SF_ASSERT(IsInList());
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
        RefCount &= ~Flag_InList; 
        pRCC = NULL;
        ClearBuffered();
    }
    void ReleaseInternal(SF_GC_DEBUG_PARAMS_DEF);
public:
    RefCountBaseGC(Collector* prcc) : pRCC(prcc), RefCount(1) { /*SF_ASSERT(pRCC);*/ }

    void AddRef() 
    { 
        SF_ASSERT(pRCC);
        Increment(SF_GC_PARAM_START_VAL);
        SetState(State_InUse);
    }
    void Release(SF_GC_DEBUG_PARAMS_DEF);

    SF_INLINE Collector* GetCollector() { SF_ASSERT(!IsInList()); return pRCC; }
 
protected:
    virtual ~RefCountBaseGC() // virtual to avoid gcc -Wall warnings
    {
        // destructor should never be invoked; Finalize_GC should be 
        // invoked instead.
        SF_ASSERT(!pRCC);
    }
private:
    // copying is prohibited
    RefCountBaseGC(const RefCountBaseGC&) { }
    RefCountBaseGC& operator=(const RefCountBaseGC&) { return *this; }

#ifdef SF_GC_DO_TRACE
    void Trace(int level, const char* text)
    {
        char s[1024] = {0};
        for (int i = 0; i < level; ++i)
            SFstrcat(s, sizeof(s), "  ");
        printf ("%s%s %p, state %d, refcnt = %d\n", s, text, this, GetState(), 
            (RefCount & RefCountBaseGC<Stat>::Mask_RefCount));
    }
#endif // SF_GC_DO_TRACE
};

// This macro should be used if it is necessary to pre-generate ForEachChild_GC
// template methods. Usually, this is necessary if it is impossible to put 
// inlined implementation of ForEachChild_GC into the header file. In this case,
// the header might contain only template declaration:
//
//        template <class Functor> void ForEachChild_GC(SF_GC_DEBUG_PARAMS_DEF) const;
// and the implementation might be placed into the CPP file. Right next to the 
// implementation this macro should be placed in the CPP file to pre-generate
// all necessary variants of the template.
#define SF_GC_PREGEN_FOR_EACH_CHILD(Class) \
template void Class::ForEachChild_GC<ASRefCountBaseType::ReleaseFunctor>(Collector* prcc) const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::MarkInCycleFunctor>(Collector* prcc) const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::ScanInUseFunctor>(Collector* prcc) const;

// This class performs "refcount collection" (other words - garbage collection).
// See comments above how to organize "collectable" classes. The collector
// may allocate memory only during the "Release" calls for possible roots.
// No allocations are done during the Collect call.
template <int Stat = Stat_Default_Mem>
class RefCountCollector : public RefCountBase<RefCountCollector<Stat>, Stat>
{
    friend class RefCountBaseGC<Stat>;

    ArrayPagedLH_POD<RefCountBaseGC<Stat> *, 10, 5>     Roots;
    UPInt                                               FirstFreeRootIndex;

    class Root : public RefCountBaseGC<Stat>
    {
    public:
        Root() : RefCountBaseGC<Stat>(0) {}
        virtual void Finalize_GC() {}
    }                                                   ListRoot;
    RefCountBaseGC<Stat>*                               pLastPtr;

    enum
    {
        Flags_Collecting,
        Flags_AddingRoot
    };
    UInt8 Flags;

protected:
    void AddRoot(RefCountBaseGC<Stat>* root);

public: // shouldn't be public but some compilers (such as ps3) complain
    void AddToList(RefCountBaseGC<Stat>* proot);
    void ReinsertToList(RefCountBaseGC<Stat>* proot);
    void RemoveFromRoots(RefCountBaseGC<Stat>* root);
public:
    struct Stats
    {
        Ptr<AmpStats> AdvanceStats;
        unsigned RootsNumber;
        unsigned RootsFreedTotal;
        void ResetStats() { RootsNumber = RootsFreedTotal = 0; }
        Stats(AmpStats* advanceStats) 
            : AdvanceStats(advanceStats) { ResetStats(); } 
    };
public:
    RefCountCollector():FirstFreeRootIndex(SF_MAX_UPINT), Flags(0) { pLastPtr = &ListRoot; }
    ~RefCountCollector() { Collect(); }

    // Perform collection. Returns 'true' if collection process was 
    // executed.
    bool Collect(Stats* pstat = NULL);

    // Returns number of roots; might be used to determine necessity
    // of call to Collect.
    UPInt GetRootsCount() const { return Roots.GetSize(); }

    // Shrinks the array for roots to a minimal possible size
    void ShrinkRoots();

    bool IsCollecting() const { return (Flags & Flags_Collecting) != 0; }
    bool IsAddingRoot() const { return (Flags & Flags_AddingRoot) != 0; }
private:
    RefCountCollector(const RefCountCollector&) {}
    RefCountCollector& operator=(const RefCountCollector&) { return *this; }
};

/////////////////////////////////////////////////////
// Implementations
//
template <int Stat>
void RefCountBaseGC<Stat>::Release(SF_GC_DEBUG_PARAMS)
{
    if (IsRefCntZero())
        return;
    SF_ASSERT(!IsDelayedRelease());

    Decrement(SF_GC_DEBUG_PASS_PARAMS);
    ReleaseInternal(SF_GC_DEBUG_PASS_PARAMS);
}

template <int Stat>
void RefCountBaseGC<Stat>::ReleaseInternal(SF_GC_DEBUG_PARAMS)
{
    if (IsRefCntZero())
    {
        SF_GC_TRACE("Releasing ");

        // Release all children. We can't perform recursive calls here.
        if (IsInList())
        {
            // It is already in list, just set flag "DelayedRelease". 
            // pRCC is invalid here.
            // This may happen when Release is called from Collect.
            SetDelayedRelease();
            return; // finish for now
        }
        else
        {
            // pRCC is valid here.

            SF_ASSERT(!pRCC->IsCollecting());
            // Check if the list is already in use...
            if (!pRCC->ListRoot.IsInList())
            {
                // ... no: we need to iterate through the list until all children are released
                pRCC->pLastPtr = &pRCC->ListRoot;
                pRCC->ListRoot.pNext = pRCC->ListRoot.pPrev = &pRCC->ListRoot;
                pRCC->ListRoot.SetInList();

                // add children to the list with flag DelayedRelease
                ExecuteForEachChild_GC(pRCC, Operation_Release); 

                while(pRCC->ListRoot.pNext != &pRCC->ListRoot)
                {
                    RefCountBaseGC<Stat>* cur = pRCC->ListRoot.pNext;
                    cur->RemoveFromList();
                    cur->ClearInList(pRCC);
                    SF_ASSERT(cur->IsDelayedRelease());
                    cur->ClearDelayedRelease();
                    pRCC->pLastPtr = pRCC->ListRoot.pPrev;
                    cur->ReleaseInternal(SF_GC_DEBUG_PASS_PARAMS);
                }

                pRCC->ListRoot.ClearInList(NULL);
            }
            else
            {
                // ... yes: just add children to the list with flag DelayedRelease
                ExecuteForEachChild_GC(pRCC, Operation_Release); 
            }
        }
        
//        ExecuteForEachChild_GC(NULL, Operation_Release); // pRCC is not required for Operation_Release

        SetState(State_InUse);

        if (!IsBuffered())
        {
            if (IsInList())
                RemoveFromList();
            Free_GC(SF_GC_DEBUG_PASS_PARAMS);
        }
        else
        {
            if (!IsInList())
            {
                SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
                pRCC->RemoveFromRoots(this);
            }
            else
                RemoveFromList();
            Free_GC(SF_GC_DEBUG_PASS_PARAMS);
        }
    }
    else
    {
        SF_ASSERT(!IsInState(State_Removed));
        // Possible root
        if (!IsInState(State_Root))
        {
            //SF_ASSERT(!pRCC->IsCollecting());
            // Add to the array of roots, if not buffered yet
            SF_GC_COND_TRACE(!IsBuffered(), "Adding possible root");
            SetState(State_Root);
            // If the obj is in list this means it is being released during the collection.
            // In this case, just mark it as root and it will be added into "Roots" later, by 
            // Collect.
            if (!IsInList() && !IsBuffered())
            {
                pRCC->AddRoot(this);
            }
        }
    }
}

template <int Stat>
void RefCountBaseGC<Stat>::MarkInCycle(Collector* prcc SF_GC_DEBUG_PARAMS)
{
    if (!IsInState(State_InCycle))
    {
        SF_GC_TRACE("Marking 'incycle' ");
        SF_ASSERT(prcc);
        SetState(State_InCycle);

        ExecuteForEachChild_GC(prcc, Operation_MarkInCycle);
    }
}

template <int Stat>
void RefCountCollector<Stat>::ReinsertToList(RefCountBaseGC<Stat>* pchild)
{
    if (pchild->IsInList())
    {
        // first, remove
        pchild->pPrev->pNext = pchild->pNext;
        pchild->pNext->pPrev = pchild->pPrev;
        SF_ASSERT(pLastPtr != pchild);

        SF_ASSERT(pLastPtr);
        pchild->pPrev          = pLastPtr->pNext->pPrev; // this
        pchild->pNext          = pLastPtr->pNext;
        pLastPtr->pNext->pPrev = pchild;
        pLastPtr->pNext        = pchild;  
    }
}

template <int Stat>
void RefCountCollector<Stat>::AddToList(RefCountBaseGC<Stat>* pchild)
{
    if (!pchild->IsInList())
    {
        SF_ASSERT(pLastPtr);
        pchild->pPrev          = pLastPtr->pNext->pPrev; // this
        pchild->pNext          = pLastPtr->pNext;
        pLastPtr->pNext->pPrev = pchild;
        pLastPtr->pNext        = pchild;  
        pLastPtr  = pchild;
        pchild->SetInList();
    }
}

template <int Stat>
void RefCountCollector<Stat>::AddRoot(RefCountBaseGC<Stat>* root)
{
    SF_ASSERT(!root->IsDelayedRelease());
    // check if we have free roots; if so - reuse first of it.
    if (FirstFreeRootIndex != SF_MAX_UPINT)
    {
        root->SetBuffered((UInt32)FirstFreeRootIndex);

        // update the FirstFreeRootIndex
        union
        {
            RefCountBaseGC<Stat>*   Ptr;
            UPInt                   UPtrValue;
            SPInt                   SPtrValue;
        } u;
        u.Ptr = Roots[FirstFreeRootIndex];
        SF_ASSERT(u.UPtrValue & 1);
        u.SPtrValue >>= 1; // keep the most significant bit, needed to get 
        // 0xFFFFFFFF if shifting the 0xFFFFFFFF (~0u).
        Roots[FirstFreeRootIndex] = root;
        FirstFreeRootIndex = u.UPtrValue;
    }
    else
    {
        // no free roots, just append.
        root->SetBuffered((UInt32)Roots.GetSize()); // save root index for 
        // fast access from Release
        // Append to roots
        Flags |= Flags_AddingRoot;
        if (!Roots.PushBackSafe(root))
        {
            // can't allocate memory for roots.
            Flags &= ~Flags_AddingRoot;

            // call Collect since Collect was disabled during the PushBack...
            bool collected = Collect();

            // try to PushBack again
            Flags |= Flags_AddingRoot;
            if (!collected || !Roots.PushBackSafe(root))
            {
                // well, still no luck... Clear "buffered", set state "InUse".
                root->ClearBuffered();
                root->SetState(RefCountBaseGC<Stat>::State_InUse);
            }
            Flags &= ~Flags_AddingRoot;
        }
        else
            Flags &= ~Flags_AddingRoot;
    }
}

template <int Stat>
void RefCountCollector<Stat>::RemoveFromRoots(RefCountBaseGC<Stat>* root)
{
    if (root->IsBuffered() && !root->IsInList())
    {
        SF_GC_TRACE("bufferred, find it roots and free.");

        // We are going to free a buffered root. We already have stored the index of 
        // element in Roots array, lets make sure it is valid by the following ASSERT...
        SF_ASSERT(Roots[root->RootIndex] == root);

        if (root->RootIndex + 1 != Roots.GetSize())
        {
            // Now, need to make kind of "free-list". pRCC stores the index of the first
            // free root (not necessary to be physically first). This index points to
            // the element in Roots which is free. This element contains an index of the next 
            // free root, encoded by the following way: bits 31-1 contain the index, bit 0 
            // is set to 1 as an indicator that this is an index and not a pointer.
            union
            {
                RefCountBaseGC<Stat>*   Ptr;
                UPInt                   PtrValue;
            } u;

            // lowest bit (bit 0) set to 1 means this is an index rather than a real ptr.
            // Since all ptrs suppose to be aligned on boundary of 4, it is safe to
            // do so.
            u.PtrValue = (FirstFreeRootIndex << 1) | 1;

            Roots[root->RootIndex]   = u.Ptr;
            FirstFreeRootIndex = root->RootIndex;
        }
        else
        {
            // no need in adding root to free list since this is the last one, just
            // resize.
            Roots.Resize(root->RootIndex); 
        }
        root->ClearBuffered();
    }
}

template <int Stat>
bool RefCountCollector<Stat>::Collect(Stats* pstat)
{
    // It is forbidden to call collection if the collector is already
    // collecting or if new root is being added. So, do nothing in 
    // this case.
    if (IsCollecting() || IsAddingRoot() || Roots.GetSize() == 0)
    {
        // already collecting - do nothing.
        if (pstat)
        {
            memset(pstat, 0, sizeof(*pstat));
        }
        return false;
    }

    AmpStats* ampStats = (pstat != NULL) ? pstat->AdvanceStats : NULL;
    SF_AMP_SCOPE_TIMER_ID(ampStats, "GC::Collect", Amp_Native_Function_Id_GcCollect);

    SF_GC_PRINT("++++++++ Starting collecting");
    UPInt  initialNRoots     = 0;
    UPInt  totalKillListSize = 0;
    
    // In most cases this loop will make only one iteration per call.
    // But in some cases, processing of kill list may produce new roots
    // to collect. This may happen, for example, when collectible object (A)
    // contains a strong ptr to non-collectible one (B), and that non-collectible
    // object has a Ptr to another collectible object (C) with refcnt > 1.
    // When Finalize_GC for A is called, it releases the ptr on B and this cause
    // invocation of Release for C. Release for C will add the pointer to Roots,
    // since refcnt is not 1. In this case the second pass will be required.
    do 
    {
        Flags |= Flags_Collecting;
        
        UPInt i, nroots = Roots.GetSize();
        initialNRoots += (unsigned)nroots;

        pLastPtr = &ListRoot;
        ListRoot.pNext = ListRoot.pPrev = &ListRoot;
        ListRoot.SetInList();

        // Mark roots stage.
        // For each root from Roots array:
        //   1) if not marked as root - skip (clear "buffered" flag);
        //   2) otherwise, add the object in a doubly-linked list, pFirstPtr is the pointer to the first node,
        //      pLastPtr is the pointer to the last node (or, to the node where a new node should be inserted).
        //   3) simultaneously, mark the root as "in cycle" and add all of its children to the list 
        //      decrementing their refcnt; if a child is already in the list then just decrement refcnt
        //      and mark it as "in cycle".
        //   4) repeat steps 1-4 until end of the list.
        for (i = 0; i < nroots; ++i)
        {
            union
            {
                RefCountBaseGC<Stat>*   Ptr;
                UPInt                   PtrValue;
            } u;
            u.Ptr = Roots[i];
            if (u.PtrValue & 1) 
                continue; // free root, skip
            RefCountBaseGC<Stat>* proot = u.Ptr;

            RefCountBaseGC<Stat>* cur = proot;
            if (cur->IsInState(RefCountBaseGC<Stat>::State_Root))
            {
                AddToList(cur);
                while(cur != &ListRoot)
                {
                    SF_ASSERT(!cur->IsDelayedRelease());
                    cur->MarkInCycle(this SF_GC_PARAM_START_VAL);
                    cur = cur->pNext;
                }
            }
            else
            {
                cur->ClearBuffered();
                if (cur->IsInState(RefCountBaseGC<Stat>::State_InUse) && cur->IsRefCntZero())
                {
                    // shouldn't get here, since Release frees immediately now
                    SF_ASSERT(0);
                }
            }
        }

        FirstFreeRootIndex = SF_MAX_UPINT;
        // cleanup roots, we don't need them anymore
        Roots.Resize(0);

        // Scan objects in the list and determine if they are garbage or not. If not - restore refcnt.
        // Each object might be visited several times, as well as incrementing may occur as many times
        // as many references to the particular object exists. The very same object might be marked 
        // as "garbage" first but later it might be re-marked as "in use" if a reference to is found.
        // The algorithm in general is as follows:
        // For each object in the list do:
        // 1) if refcnt zero - mark the object as "garbage"
        // 2) otherwise, mark the current object as "in use and for each child do
        //      a) increment refcnt
        //      b) if the child is not marked as "in use" yet then mark it and re-insert after the 
        //         current object thus it will be revisited later (to scan its children too).
        // 3) proceed to the next object in the list; this might be an object recently re-inserted at
        //    step 2b. If this is happening then this object is already marked as "in use" and refcnt > 0,
        //    thus its children will be visited as well (at step 2 of next iteration).
        RefCountBaseGC<Stat>* cur = ListRoot.pNext;
        while(cur != &ListRoot)
        {
            if ((cur->RefCount & RefCountBaseGC<Stat>::Mask_RefCount) > 0)
            {
                cur->SetState(RefCountBaseGC<Stat>::State_InUse);
                pLastPtr = cur; // to perform reinserting after cur
                cur->ExecuteForEachChild_GC(this, RefCountBaseGC<Stat>::Operation_ScanInUse);
            }
            else
            {
                cur->SetState(RefCountBaseGC<Stat>::State_Garbage);
                SF_GC_TRACE("Marking garbage ");
            }

            cur = cur->pNext;
        }

        // process the list, clean it up, free garbage.
        cur = ListRoot.pNext;
        while(cur != &ListRoot)
        {
            RefCountBaseGC<Stat>* next = cur->pNext;
            if (cur->IsInState(RefCountBaseGC<Stat>::State_Garbage))
            {
                // This Free may cause adding DelayedRelease nodes in the list
                // so will need to check for them
                cur->Free_GC(SF_GC_PARAM_START_VAL);
                ++totalKillListSize;
            }
            else 
            {
                cur->ClearInList(this);
                if (cur->IsDelayedRelease())
                {
                    // if DelayedRelease then Decrement was already called.
                    // Just clear the flag and call ReleaseInternal.
                    cur->ClearDelayedRelease();
                    cur->ReleaseInternal();
                }
                else
                {
                    if (cur->IsInState(RefCountBaseGC<Stat>::State_Root))
                        // delayed root addition, see Release
                        AddRoot(cur);
                    else
                        cur->ClearBuffered();
                }
            }

            cur = next;
        }
        pLastPtr = &ListRoot;
        ListRoot.ClearInList(NULL);
        
        Flags &= ~Flags_Collecting;
        FirstFreeRootIndex = SF_MAX_UPINT;
    } while (Roots.GetSize() > 0);
    
    if (pstat)
    {
        // save stats. Note totalKillListSize might be greater than initialNRoots,
        // since it might free some non-root elements as well. So, just correct RootsFreedTotal
        // to avoid negative difference between RootsNumber and RootsFreedTotal.
        pstat->RootsNumber              = (unsigned)initialNRoots;
        pstat->RootsFreedTotal          = (unsigned)Alg::PMin(initialNRoots, totalKillListSize);
        if (ampStats)
        {
            ampStats->AddGcRoots(pstat->RootsNumber);
            ampStats->AddGcFreedRoots(pstat->RootsFreedTotal);
        }
    }
    SF_GC_PRINT("-------- Finished collecting\n");
    return true;
}

template <int Stat>
void RefCountCollector<Stat>::ShrinkRoots()
{
    if (Roots.GetSize() == 0)
        Roots.ClearAndRelease();
}

}}} // Scaleform::GFx::AS2

#endif
