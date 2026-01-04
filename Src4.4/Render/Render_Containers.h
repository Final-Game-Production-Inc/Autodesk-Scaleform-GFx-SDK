/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Containers.h
Content     :   Containers for tessellator, stroker and so on.
Created     :   2005-2007
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Containers_H
#define INC_SF_Render_Containers_H

#include "Kernel/SF_Allocator.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
class LinearHeap
{
    enum
    {
        MinAlignMask    = sizeof(UPInt)-1,
        PagePoolSize    = 64
    };

    struct PageType
    {
        UByte* pStart;
        UByte* pEnd;
        UByte* pFree;
    };

public:
    LinearHeap(MemoryHeap* heap, UPInt granularity=8192) :
        pHeap(heap),
        Granularity(granularity),
        pPagePool(0),
        pLastPage(0),
        MaxPages(0)
    {}

    ~LinearHeap()
    {
        ClearAndRelease();
    }

    void Clear()
    {
        for (UPInt i = 0; i < MaxPages; ++i)
        {
            pPagePool[i].pFree = pPagePool[i].pStart;
        }
        pLastPage = pPagePool;
    }

    void ClearAndRelease()
    {
        if (MaxPages)
        {
            PageType* page = pPagePool + MaxPages - 1;
            while (MaxPages--)
            {
                if (page->pStart)
                {
                    pHeap->Free(page->pStart);
                }
                --page;
            }
            pHeap->Free(pPagePool);
        }
        pPagePool = pLastPage = 0;
        MaxPages  = 0;
    }

    UPInt GetFootprint() const
    {
        UPInt fp = 0;
        for (UPInt i = 0; i < MaxPages; ++i)
        {
            fp += pPagePool[i].pEnd - pPagePool[i].pStart;
        }
        return UPInt(fp + MaxPages * sizeof(PageType));
    }

    void* Alloc(UPInt size)
    {
        size = (size + MinAlignMask) & ~MinAlignMask;
        if (pLastPage)
        {
            void* ret = allocFromLastPage(size);
            if (ret) return ret;
            ++pLastPage;
        }
        else
        {
            pLastPage = pPagePool = (PageType*)pHeap->Alloc(PagePoolSize * sizeof(PageType));
            memset(pPagePool, 0, PagePoolSize * sizeof(PageType));
            MaxPages = PagePoolSize;
        }
        if (pLastPage - pPagePool >= SPInt(MaxPages))
        {
            UPInt np = pLastPage - pPagePool;
            PageType* newPool = (PageType*)pHeap->Alloc(MaxPages * 2 * sizeof(PageType));
            SF_ASSERT(newPool);
            memcpy(newPool, pPagePool, MaxPages * sizeof(PageType));
            memset(newPool + MaxPages, 0, MaxPages * sizeof(PageType));
            pHeap->Free(pPagePool);
            pPagePool = newPool;
            MaxPages *= 2;
            pLastPage = pPagePool + np;
        }
        void* p = allocFromLastPage(size);
        SF_ASSERT(p);
        return p;
    }

    MemoryHeap* GetMemoryHeap() { return pHeap; }

private:
    void* allocFromLastPage(UPInt size)
    {
        SF_ASSERT((size & 3) == 0);
        if (pLastPage->pEnd - pLastPage->pFree >= SPInt(size))
        {
            pLastPage->pFree += size;
            return pLastPage->pFree - size;
        }
        if (pLastPage->pFree == pLastPage->pStart) // Empty page
        {
            if (pLastPage->pEnd - pLastPage->pFree < SPInt(size))
            {
                allocPage(size);
            }
            pLastPage->pFree += size;
            return pLastPage->pStart;
        }
        return 0;
    }

    void allocPage(UPInt size)
    {
        if (pLastPage->pStart)
            pHeap->Free(pLastPage->pStart);

        size = (size + Granularity - 1) / Granularity * Granularity;

        pLastPage->pStart = pLastPage->pFree = (UByte*)pHeap->Alloc(size);
        pLastPage->pEnd   = pLastPage->pStart + size;
        SF_ASSERT(pLastPage->pStart);
    }

    MemoryHeap*     pHeap;
    UPInt           Granularity;
    PageType*       pPagePool;
    PageType*       pLastPage;
    UPInt           MaxPages;
};


//-----------------------------------------------------------------------
template<class T, unsigned PageSh, unsigned PtrPoolInc> class ArrayPaged
{
public:
    enum PageScale_e
    {   
        PageShift = PageSh,
        PageSize  = 1 << PageShift,
        PageMask  = PageSize - 1
    };

    typedef T ValueType;

    ArrayPaged(LinearHeap* heap) : 
        pHeap(heap),
        Size(0),
        NumPages(0),
        MaxPages(0),
        Pages(0)
    {}

    void Clear() 
    { 
        Size = 0;
    }

    void ClearAndRelease() 
    {
        Size = NumPages = MaxPages = 0; Pages = 0; 
    }

    void PushBack(const T& val)
    {
        *dataPtr() = val;
        ++Size;
    }

    void PopBack()
    {
        if(Size) --Size;
    }

    void CutAt(UPInt size)
    {
        if(size < Size) Size = size;
    }

    UPInt GetSize() const 
    { 
        return Size; 
    }

    const T& operator [] (UPInt i) const
    {
        return Pages[i >> PageShift][i & PageMask];
    }

    T& operator [] (UPInt i)
    {
        return Pages[i >> PageShift][i & PageMask];
    }

    const T& Back() const
    {
        return (*this)[Size - 1];
    }

    T& Back()
    {
        return (*this)[Size - 1];
    }

private:
    // Copying is prohibited
    ArrayPaged(const ArrayPaged<T,PageSh,PtrPoolInc>& v);
    const ArrayPaged<T,PageSh,PtrPoolInc>& operator = (const ArrayPaged<T,PageSh,PtrPoolInc>& v);

    void allocPage(UPInt nb)
    {
        if(nb >= MaxPages) 
        {
            if(Pages)
            {
                T** newPages = (T**)pHeap->Alloc((MaxPages * 2) * sizeof(T*));
                memcpy(newPages, Pages, NumPages * sizeof(T*));
                Pages = newPages;
                MaxPages *= 2;
            }
            else
            {
                Pages = (T**)pHeap->Alloc((MaxPages = PtrPoolInc) * sizeof(T*));
            }
        }
        Pages[nb] = (T*)pHeap->Alloc(PageSize * sizeof(T));
        SF_ASSERT(Pages[nb]);
        NumPages++;
    }

    T* dataPtr()
    {
        UPInt nb = Size >> PageShift;
        if(nb >= NumPages)
        {
            allocPage(nb);
        }
        SF_ASSERT(Pages[nb]);
        return Pages[nb] + (Size & PageMask);
    }

    LinearHeap* pHeap;
    UPInt       Size;
    UPInt       NumPages;
    UPInt       MaxPages;
    T**         Pages;
};



//-----------------------------------------------------------------------
template<class T, unsigned PageSh, unsigned PtrPoolInc> class ArrayJagged
{
    struct ArrayType
    {
        UPInt   Size;
        UPInt   NumPages;
        UPInt   MaxPages;
        T**     Pages;
    };

public:
    enum PageScale_e
    {
        ArrayInc  = 16,

        PageShift = PageSh,
        PageSize  = 1 << PageShift,
        PageMask  = PageSize - 1
    };

    typedef T ValueType;

    class ArrayAdaptor
    {
    public:
        typedef T ValueType;

        ArrayAdaptor(UPInt size, T** pages) : Size(size), Pages(pages) {}
        UPInt GetSize() const { return Size; }
        const T& operator [] (UPInt i) const
        {
            return Pages[i >> PageShift][i & PageMask];
        }
        T& operator [] (UPInt i)
        {
            return Pages[i >> PageShift][i & PageMask];
        }
    private:
        UPInt   Size;
        T**     Pages;
    };

    ArrayJagged(LinearHeap* heap) : 
        pHeap(heap),
        Arrays(0),
        NumArrays(0),
        MaxArrays(0)
    {}

    void ClearAndRelease() 
    {
        Arrays = 0; NumArrays = MaxArrays = 0;
    }

    void AddArray()
    {
        if(NumArrays >= MaxArrays)
        {
            if(NumArrays)
            {
                ArrayType* newArrays = (ArrayType*)pHeap->Alloc((MaxArrays * 2) * sizeof(ArrayType));
                memcpy(newArrays, Arrays, NumArrays * sizeof(ArrayType));
                Arrays = newArrays;
                MaxArrays *= 2;
            }
            else
            {
                Arrays = (ArrayType*)pHeap->Alloc((MaxArrays = ArrayInc) * sizeof(ArrayType));
            }
        }
        memset(&Arrays[NumArrays], 0, sizeof(ArrayType));
        ++NumArrays;
    }

    ArrayAdaptor GetArrayAdaptor(UPInt i)
    {
        return ArrayAdaptor(Arrays[i].Size, Arrays[i].Pages);
    }

    UPInt GetArrayCount() const 
    { 
        return NumArrays; 
    }

    UPInt GetSize(UPInt i) const 
    { 
        return Arrays[i].Size; 
    }

    void PushBack(UPInt i, const T& val)
    {
        *dataPtr(i) = val;
        ++Arrays[i].Size;
    }

    void PopBack(UPInt i)
    {
        if(Arrays[i].Size) --Arrays[i].Size;
    }

    void CutAt(UPInt i, UPInt size)
    {
        if(size < Arrays[i].Size) Arrays[i].Size = size;
    }

    const T& At(UPInt i, UPInt j) const
    {
        return Arrays[i].Pages[j >> PageShift][j & PageMask];
    }

    T& At(UPInt i, UPInt j)
    {
        return Arrays[i].Pages[j >> PageShift][j & PageMask];
    }

    const T& Back(UPInt i) const
    {
        return At(i, Arrays[i].Size - 1);
    }

    T& Back(UPInt i)
    {
        return At(i, Arrays[i].Size - 1);
    }

    UPInt GetPageCount(UPInt i) const
    {
        return Arrays[i].NumPages;
    }

    const T* GetPage(UPInt i, UPInt j) const
    {
        return Arrays[i].Pages[j];
    }

    bool Split(UPInt i, UPInt at)
    {
        if (at >= GetSize(i)) return false;
        UPInt oldSize      = at & ~PageMask;
        UPInt newSize      = GetSize(i) - oldSize;
        UPInt oldNumPages  = at >> PageShift;
        UPInt newNumPages  = Arrays[i].NumPages - oldNumPages;
        UPInt newMaxPages  = Arrays[i].MaxPages - oldNumPages;
        T**   newPages     = Arrays[i].Pages    + oldNumPages;
        AddArray();
        Arrays[i].NumPages = oldNumPages;
        Arrays[i].MaxPages = oldNumPages;
        Arrays[i].Size     = oldSize;
        i = NumArrays - 1;
        Arrays[i].NumPages = newNumPages;
        Arrays[i].MaxPages = newMaxPages;
        Arrays[i].Size     = newSize;
        Arrays[i].Pages    = newPages;
        return true;
    }

private:
    // Copying is prohibited
    ArrayJagged(const ArrayJagged<T,PageSh,PtrPoolInc>& v);
    const ArrayJagged<T,PageSh,PtrPoolInc>& operator = (const ArrayJagged<T,PageSh,PtrPoolInc>& v);

    void allocPage(ArrayType* a, UPInt nb)
    {
        if(nb >= a->MaxPages)
        {
            if(a->Pages)
            {
                T** newPages = (T**)pHeap->Alloc((a->MaxPages * 2) * sizeof(T*));
                memcpy(newPages, a->Pages, a->NumPages * sizeof(T*));
                a->Pages = newPages;
                a->MaxPages *= 2;
            }
            else
            {
                a->Pages = (T**)pHeap->Alloc((a->MaxPages = PtrPoolInc) * sizeof(T*));
            }
        }
        a->Pages[nb] = (T*)pHeap->Alloc(PageSize * sizeof(T));
        a->NumPages++;
    }

    T* dataPtr(UPInt i)
    {
        ArrayType* a = &Arrays[i];
        UPInt nb = a->Size >> PageShift;
        if(nb >= a->NumPages)
        {
            allocPage(a, nb);
        }
        return a->Pages[nb] + (a->Size & PageMask);
    }

    LinearHeap* pHeap;
    ArrayType*  Arrays;
    UPInt       NumArrays;
    UPInt       MaxArrays;
};




//------------------------------------------------------------------------
template<class T> class ArrayUnsafe
{
public:
    typedef T ValueType;

    ArrayUnsafe(LinearHeap* heap) : 
        pHeap(heap), Size(0), Array(0) 
    {}

    void ClearAndRelease()
    { 
        Size  = 0;
        Array = 0;
    }

    // Allocate n elements. All data is lost, 
    // but elements can be accessed in range 0...size-1. 
    void Resize(UPInt size)
    {
        if(size > Size)
        {
            T* newArray = (T*)pHeap->Alloc(size * sizeof(T));
            memset(newArray, 0, size * sizeof(T));
            if (Array && Size)
            {
                memcpy(newArray, Array, Size * sizeof(T));
            }
            Array = newArray;
        }
        Size = size;
    }

    void Zero()
    {
        memset(Array, 0, sizeof(T) * Size);
    }

    UPInt GetSize() const 
    { 
        return Size; 
    }

    const T& operator [] (UPInt i) const { return Array[i]; }
          T& operator [] (UPInt i)       { return Array[i]; }
private:
    // Copying is prohibited
    ArrayUnsafe(const ArrayUnsafe<T>& v);
    const ArrayUnsafe<T>& operator = (const ArrayUnsafe<T>& v);

    LinearHeap* pHeap;
    UPInt       Size;
    T*          Array;
};


//------------------------------------------------------------------------
template<class T> class ArrayMatrix
{
public:
    typedef T ValueType;

    ArrayMatrix(LinearHeap* heap) : 
        pHeap(heap), Size(0), Array(0) 
    {}

    void ClearAndRelease()
    { 
        Size  = 0;
        Array = 0;
    }

    void Resize(UPInt size)
    {
        Size = size;
        Array = (T*)pHeap->Alloc(size * size * sizeof(T));
        memset(Array, 0xFF, size * size * sizeof(T));
    }

    const T& At(UPInt i, UPInt j) const { return Array[i * Size + j]; }
          T& At(UPInt i, UPInt j)       { return Array[i * Size + j]; }
    T   ValueAt(UPInt i, UPInt j) const { return Array[i * Size + j]; }

private:
    LinearHeap* pHeap;
    UPInt       Size;
    T*          Array;
};


// PagedItemBuffer allocates items of class C consecutively in memory,
// freeing th whole list in its destructor. Allows items to be iterated
// in order.
// Used for ContextChangeNode(s).

template<class C, int ItemsPerPage>
class PagedItemBuffer
{
public:
    struct Page
    {
        Page*    pNext;
        unsigned Count;
        C        Items[ItemsPerPage];

        Page*    GetNextPage() const   { return pNext; }
        unsigned GetSize() const       { return Count; }
        C&       GetItem(unsigned index)   { return Items[index]; }
    };

private:
    Page*   pPages;
    Page*   pLast;

    void    ensureCountAvailable(unsigned count)
    {
        SF_ASSERT(count <= ItemsPerPage);
        if (!pLast)
        {
            pLast = pPages= (Page*)SF_HEAP_AUTO_ALLOC(this, sizeof(Page));
            SF_ASSERT(pLast);
            pLast->pNext = 0;
            pLast->Count = 0;
        }
        else if ((pLast->Count + count) > ItemsPerPage)
        {    
            Page* plast = pLast;
            pLast = (Page*)SF_HEAP_AUTO_ALLOC(this, sizeof(Page));
            plast->pNext = pLast;
            pLast->pNext = 0;
            pLast->Count = 0;
        }
    }

    void    freePages(bool keepLast)
    {
        Page* p = pPages;
        Page* pold = 0;

        while(p)
        {
            for (unsigned i=0; i< p->Count; i++)
                p->Items[i].C::~C();
            if (pold)
                SF_FREE(pold);
            pold = p;
            p = p->pNext;
        }
        
        if (pold)
        {
            if (keepLast)
            {
                pold->Count = 0;
            }
            else
            {
                SF_FREE(pold);
                pold = 0;
            }
        }

        pLast = pPages = pold;
    }

public:

    PagedItemBuffer()
        : pPages(0), pLast(0)
    {  }
    
    ~PagedItemBuffer()
    {
        freePages(false);
    }    

    C*  AddItem()
    {
        ensureCountAvailable(1);
        C* pitem = &pLast->Items[pLast->Count];
        Construct<C>(pitem);
        pLast->Count++;
        return pitem;
    }

    template<class A>
    C*  AddItem(const A& arg)
    {
        ensureCountAvailable(1);        
        C* pitem = &pLast->Items[pLast->Count];
        ConstructAlt<C>(pitem, arg);
        pLast->Count++;
        return pitem;
    }

    // Adds multiple consecutive items returning a pointer to first one
    C*  AddItems(unsigned count)
    {
        ensureCountAvailable(count);
        C* pitems = &pLast->Items[pLast->Count];
        for (unsigned i = 0; i< count; i++)
            Construct<C>(pitems + i);
        pLast->Count+= count;
        return pitems;
    }

    C*  AddItems(C* source, unsigned count)
    {
        ensureCountAvailable(count);
        C* pitems = &pLast->Items[pLast->Count];
        for (unsigned i = 0; i< count; i++)
            ConstructAlt<C>(pitems + i, source[i]);
        pLast->Count+= count;
        return pitems;
    }

    void Clear(bool keepLast = false)
    {
        freePages(keepLast);
    }

    bool IsEmpty() const
    {
        return !pPages || (pPages->Count == 0);
    }

    // Obtain page for faster external iteration
    Page*   GetFirstPage() const { return pPages; }

  
    class Iterator
    {
        Page*       pPage;
        SPInt       Index;

    public:
        Iterator() : pPage(0), Index(-1) {}
        Iterator(Page* ppage, unsigned idx = 0) : pPage(ppage), Index(idx) {}

        bool operator==(const Iterator& it) const { return pPage == it.pPage && Index == it.Index; }
        bool operator!=(const Iterator& it) const { return pPage != it.pPage || Index != it.Index; }

        Iterator& operator++()
        {
            SF_ASSERT(pPage);
            if (Index > pPage->Count)
            {
                Index++;
                if ((Index == pPage->Count) && pPage->pNext)
                {
                    Index = 0;
                    pPage = pPage->pNext;
                }
            }
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator it(*this);
            operator++();
            return it;
        }

        C& operator*() const    { SF_ASSERT(pPage); return  pPage->Items[Index]; }
        C* operator->() const   { SF_ASSERT(pPage); return &pPage->Items[Index]; }
        C* GetPtr() const       { SF_ASSERT(pPage); return &pPage->Items[Index]; }

        bool IsFinished() const
        {
            return !pPage || (Index == pPage->Count);
        }
    };

    Iterator    GetFirst() const
    {
        return Iterator(pPages);
    }

};


// ***** DefragAllocEntry

// DefragAllocEntry is a base implementation for the entry type that
// can be contained by the PagedDefragAlloc. To support the allocator's
// Defrag() operation, the node maintains two bits describing the object
// live state, as well as a pointer to the next free item.

// Instead of inheritance, this class can be re-implemented in the Entry
// by only maintaining 2 bits dedicated to lifetime management when Alive,
// as the next pointer is used only after the Entry's destructor has been
// called.

template<class C>
class DefragAllocEntry
{
    UPInt   Alloc_NextFree;
public:
    // Alive / Not Alive. Used when de-fragmenting to know if
    // a certain address was freed.

    DefragAllocEntry() : Alloc_NextFree(0) { }

    // Sets the next free node pointer and marks the object as Destroyed,
    // i.e. not Alive. Called after destructor to internal PagedDefragAlloc
    // lifetime management; never called for live objects.
    void    Alloc_SetNextFreeNode(C* p)
    {
        Alloc_NextFree = ((UPInt)p) | 1;
    }
    // Returns the next free node pointer stored by Alloc_SetNextFreeNode.
    C*      Alloc_GetNextFreeNode()
    {
        C* p = (C*)(Alloc_NextFree & (UPInt)~3);
        Alloc_NextFree = 0;
        return p;
    }

    // IsAlice returns 1 when the object is alive (its destructor wasn't called).
    bool    Alloc_IsAlive()               { return Alloc_NextFree == 0; }
    // Alloc_IsDestroyedAndFree returns true if Alloc_SetDestroyedAndFree
    // was called on the object.
    bool    Alloc_IsDestroyedAndFree()    { return (Alloc_NextFree & 2) != 0; }    
    void    Alloc_SetDestroyedAndFree()   { Alloc_NextFree |= 2; }

    // User must implement:
    //void    Alloc_MoveTo(C* pnewAddr);
};


// ***** PagedDefragAlloc

// PagedDefragAlloc is a paged Fragmentation-Free allocator for objects
// of class C. Every object of class C must be movable by implementing
// the DefragAllocEntry interface, including the Alloc_MoveTo() operation.
//
// The allocator allocates memory in multiples of PageSize and
// frees it strictly in the order opposite to the allocation. To avoid
// fragmentation, the allocator supports the Defrag() operation, which
// moves items from the tail of the allocated pages into the recently
// destroyed spots. This movement means that no memory is ever wasted
// within the allocated pages, and that tail pages are released back to
// the user as soon as possible.
//
// Use the allocator by calling the Create() function, which creates 
// and object C by calling its constructor. Call Destroy(pC) on the created
// object to destroy it and release it memory. After Destroy, the freed
// up memory is not available for immediate reuse until Defrag is called;
// it should be called as soon as possible after Destroy, once it is known
// that movement of any objects of type C is safe.


template<class C, int PageSize = 1024, int statId = Stat_Default_Mem>
class PagedDefragAlloc
{
    enum {
        ItemsPerPage = (PageSize - sizeof(UPInt)) / sizeof(C)
    };

    // Page data structure, contains items and pPrev. pPrev points to
    // an earlier allocated page that must've been completely filled up.
    struct Page
    {
        C     Items[ItemsPerPage];
        Page* pPrev; // Comes after array for better C type alignment support.
    };

    // Pointer to the last page from which we grow.
    Page*    pLastPage;
    unsigned LastPageItemCount;
    // List of freed nodes pending de-fragmentation.
    C*       pFreeNodes;

    bool    AllocPage()
    {
        Page* ppage = (Page*)SF_HEAP_AUTO_ALLOC_ID(this, sizeof(Page), statId);
        if (ppage)
        {
            ppage->pPrev      = pLastPage;
            pLastPage         = ppage;
            LastPageItemCount = 0;
            return true;
        }
        return false;
    }

    void    FreePage(Page* p)
    {
        SF_FREE(p);
    }


public:

    PagedDefragAlloc()
    {
        pLastPage         = 0;
        LastPageItemCount = ItemsPerPage;
        pFreeNodes        = 0;
    }

    ~PagedDefragAlloc()
    {
        // Destroys all non-free list items implicitly.
        while(pLastPage)
        {
            Page* ppage = pLastPage;
            for (UPInt i = LastPageItemCount; i > 0; i--)
            {
                if (ppage->Items[i-1].Alloc_IsAlive())
                    ppage->Items[i-1].C::~C();
            }
            pLastPage         = ppage->pPrev;
            LastPageItemCount = ItemsPerPage;
            FreePage(ppage);
        }
    }


    // Create an object of type C by calling its constructor.
    C*  Create()
    {
        if ((LastPageItemCount == ItemsPerPage) && !AllocPage())
            return 0;
        void *p = &pLastPage->Items[LastPageItemCount++];
        Construct<C>(p);
        return (C*)p;
    }

    template<class S>
    C*  Create(const S& source)
    {
        if ((LastPageItemCount == ItemsPerPage) && !AllocPage())
            return 0;
        void *p = &pLastPage->Items[LastPageItemCount++];
        ConstructAlt<C, S>(p, source);
        return (C*)p;
    }

    template<class S1, class S2>
    C*  Create(const S1& s1, const S2& s2)
    {
        if ((LastPageItemCount == ItemsPerPage) && !AllocPage())
            return 0;
        void *p = &pLastPage->Items[LastPageItemCount++];
        ConstructAlt<C, S1, S2>(p, s1, s2);
        return (C*)p;
    }

    // Destroy an object of type C. Memory is not actually reclaimed
    // until Defrag is called, as doing so would invalidate all pointers to C.
    void    Destroy(C* p)
    {
        p->~C();
        p->Alloc_SetNextFreeNode(pFreeNodes);
        pFreeNodes = p;
    }


    // De-fragments the list by moving non-freed entries at the tail in the freed
    // location and then releasing any leftover pages.
    // 
    // WARNING! Defrag() will invalidate ANY temporary user pointers to C types
    // stored in this list. This means that this function is best run after an
    // algorithm was removing the items completes. All left-over valid pointers 
    // are expected to be properly adjusted by C::MoveTo().

    void    Defrag()
    {
        if (pFreeNodes)
            DefragImpl();
    }

private:

    void    DefragImpl()
    {
        C* p = pFreeNodes;

        Page* pfreePages = 0;
        Page* ppage;

        // Ok to start with do() since the first check is done by Defrag().
        do {
            SF_ASSERT(pLastPage);

            if (!p->Alloc_IsDestroyedAndFree())
            {
                // Get top item
                C* ptopItem = 0;

                while(1)
                {
                    if (LastPageItemCount == 0)
                    {
                        // Free the last page.
                        ppage       = pLastPage;
                        pLastPage   = ppage->pPrev;
                        ppage->pPrev= pfreePages;
                        pfreePages  = ppage;
                        LastPageItemCount = ItemsPerPage;
                    }

                    LastPageItemCount--;
                    ptopItem = &pLastPage->Items[LastPageItemCount];

                    if (ptopItem->Alloc_IsAlive() || (p == ptopItem))
                        break;

                    ptopItem->Alloc_SetDestroyedAndFree();
                }
                
                if (ptopItem != p)
                    ptopItem->Alloc_MoveTo(p);
            }

            p = p->Alloc_GetNextFreeNode();
        } while(p);

        // Free Any free pages
        while (pfreePages)
        {
            ppage      = pfreePages;
            pfreePages = pfreePages->pPrev;
            FreePage(ppage);
        }

        pFreeNodes = 0;
    }

};



//------------------------------------------------------------------------

// Array used to track singly-linked lists of tree node pointers indexed
// by depth; used for efficient matrix/bounds propagation.

template<class N>
class DepthUpdateArrayPOD
{
    enum { MaxDepthReserve = 32 };

    N*          pDepth;
    unsigned    DepthUsed;
    unsigned    DepthAvailable;
    MemoryHeap* pHeap;
    N           NullValue;
    N           ArrayReserve[MaxDepthReserve];

    bool grow(unsigned depth)
    {
        unsigned newSize = (depth + MaxDepthReserve-1) & ~(MaxDepthReserve-1);
        N* p = (N*)SF_HEAP_ALLOC(pHeap, newSize * sizeof(N), Stat_Default_Mem);
        if (p)
        {
            memcpy(p, pDepth, DepthUsed * sizeof(N));
            unsigned clearCount = newSize - DepthUsed;
            for (unsigned i = 0; i < clearCount; i++)
                p[DepthUsed + i] = NullValue;

            if (pDepth != ArrayReserve)
                SF_FREE(pDepth);
            pDepth = p;
            DepthAvailable = depth;
            return true;
        }
        return false;
    }

public:
    // We allow a different nullValue to end the lists other then 0;
    // this is needed for bounds propagation that used link pointer
    // as a marker of item being "already added".
    DepthUpdateArrayPOD(MemoryHeap* heap, N nullValue)
        : pDepth(&ArrayReserve[0]), DepthUsed(0),
        DepthAvailable(MaxDepthReserve), pHeap(heap), NullValue(nullValue)
    {   
        for (unsigned i = 0; i < MaxDepthReserve; i++)
            ArrayReserve[i] = NullValue;
    }
    ~DepthUpdateArrayPOD()
    {
        if (pDepth != ArrayReserve)
            SF_FREE(pDepth);
    }


    unsigned GetDepthUsed() const { return DepthUsed; }

    // Obtains item at depth
    N    Get(unsigned index) const
    {
        SF_ASSERT(index < DepthUsed);
        return pDepth[index];
    }

    void Set(unsigned index, N node)
    {
        if ((index < DepthAvailable) || grow(index+1))
        {
            pDepth[index] = node;
            if (DepthUsed < (index + 1))
                DepthUsed = index + 1;
        }
    }

    void Link(unsigned index, N* pnext, N val)
    {
        if ((index < DepthAvailable) || grow(index+1))
        {
            *pnext = pDepth[index];
            pDepth[index] = val;
            if (DepthUsed < (index + 1))
                DepthUsed = index + 1;
        }
    }

    void Clear()
    {
        for (unsigned i = 0; i < DepthUsed; i++)
            pDepth[i] = NullValue;
        DepthUsed = 0;
    }
};


//------------------------------------------------------------------------

// ArrayReseveLH_Mov is a simple array container class used in renderer data structures.
// This class has two key features:
//  - It uses internal fixed-size storage to avoid allocation operation for
//    a typical case where array contains only one of few elements.
//  - Elements must be movable.

template<class C, int InternalSize = 2>
class ArrayReserveLH_Mov
{
    UPInt   Size;

    struct  AllocData
    {
        C*      pData;
        UPInt   Reserve;
    };

    union {
        AllocData AD;
        UByte     Raw[InternalSize * sizeof(C)];
    };

    static UPInt   GetAllocSize(UPInt size)  { return (size + 3) & ~3; }

    C* allocInsertCopy(UPInt index, C* source, UPInt size, UPInt allocSize)
    {
        C* pdata = (C*) SF_HEAP_AUTO_ALLOC(this, allocSize * sizeof(C));
        if (pdata)
        {
            if (index)
                memcpy(pdata, source, index * sizeof(C));
            if (index < size)
                memcpy(pdata + index + 1, source + index, (size-index) * sizeof(C));
        }
        return pdata;
    }

    void insertMemorySpot(UPInt index, C* source, UPInt size)
    {
        if (index < size)
            memmove(source + (index + 1),
                    source + index, (size-index) * sizeof(C));
    }

    C* insertSpot(UPInt index)
    {
        SF_ASSERT(index <= Size);
        C* result;
        
        if (Size < InternalSize)
        {
            insertMemorySpot(index, (C*)Raw, Size);
            result = ((C*)Raw) + index;
        }
        else if (Size == InternalSize)
        {
            UPInt allocSize = GetAllocSize(Size+1);
            C*    pdata     = allocInsertCopy(index, (C*)Raw, Size, allocSize);
            if (!pdata)
                return 0;
            result = pdata + index;
            AD.pData   = pdata;
            AD.Reserve = allocSize;
        }
        else
        {
            // Reuse free slot if possible
            if (Size < AD.Reserve)
            {
                insertMemorySpot(index, AD.pData, Size);
                result = AD.pData+index;
            }
            else
            {
                // Grow (TBD: Optimize with Realloc)
                UPInt allocSize = GetAllocSize(Size + 1);
                C*    pdata     = allocInsertCopy(index, AD.pData, Size, allocSize);
                if (!pdata)
                    return 0;
                result = pdata + index;
                SF_FREE(AD.pData);
                AD.pData   = pdata;
                AD.Reserve = allocSize;
            }
        }

        Size++;
        return result;
    }

public:

    ArrayReserveLH_Mov() : Size(0)
    { }
    ArrayReserveLH_Mov(const C& val) : Size(1)
    {
        ConstructAlt<C>(Raw, val);
    }

    ~ArrayReserveLH_Mov()
    {
        DestructArray<C>(GetBuffer(), Size);
        if (Size > InternalSize)
            SF_FREE(AD.pData);
    }

    bool    Add(const C& val)
    {
        C* p = insertSpot(Size);
        if (p)
        {
            ConstructAlt<C>(p, val);
            return true;
        }
        return false;
    }

    bool    InsertAt(UPInt index, const C& val)
    {
        C* p = insertSpot(index);
        if (p)
        {
            ConstructAlt<C>(p, val);
            return true;
        }
        return false;
    }

    // Add pointer to set only if it isn't here.
    bool    Add_NoDuplicate(const C& val)
    {
        C* pbuffer = GetBuffer();
        for (UPInt i=0; i< Size; i++)
            if (pbuffer[i] == val)
                return false;
        return Add(val);
    }

    void    RemoveAt(UPInt index)
    {
        C* pbuff = GetBuffer();
        Destruct<C>(pbuff + index);
        memmove(pbuff + index, pbuff + index + 1, sizeof(C) * (Size-index-1));
        Size--;

        if (Size == InternalSize)
        {
            C* pdata = AD.pData;
            memcpy(Raw, pdata, Size * sizeof(C));
            SF_FREE(pdata);
        }
    }

    // Remove succeeds quietly when item in not found. THis is important
    // for uses together with Add_NoDuplicate.
    void    Remove(const C& val)
    {
        C* pbuffer = GetBuffer();

        for (UPInt i=0; i< Size; i++)
        {
            if (pbuffer[i] == val)
            {
                RemoveAt(i);
                break;
            }
        }
    }

    void Clear()
    {
        DestructArray<C>(GetBuffer(), Size);
        if (Size > InternalSize)
            SF_FREE(AD.pData);
        Size = 0;
    }

    UPInt    GetSize() const     { return Size; }
    const C* GetBuffer() const   { return (Size > InternalSize) ? AD.pData : (const C*)Raw; }
    C*       GetBuffer()         { return (Size > InternalSize) ? AD.pData : (C*)Raw; }

    const C& operator []  (UPInt index) const { return GetBuffer()[index]; }
    C& operator []        (UPInt index)       { return GetBuffer()[index]; }
};


class BitSet
{
public:
    BitSet(MemoryHeap* heap) :
        Size(32),
        Local(0),
        pData(&Local),
        pHeap(heap)
    {}

    ~BitSet()
    {
        if (pData != &Local)
            pHeap->Free(pData);
    }

    void Clear()
    {
        if (pData != &Local)
            pHeap->Free(pData);
        pData = &Local;
        Size = 32;
        Local = 0;
    }

    unsigned GetSize() const
    {
        return Size;
    }

    void Set(unsigned bit)
    {
        if (bit >= Size)
            resize(bit+1);
        pData[bit >> 5] |= 1 << (bit & 31);
    }

    void Clr(unsigned bit)
    {
        if (bit >= Size)
            resize(bit+1);
        pData[bit >> 5] &= ~(1 << (bit & 31));
    }

    // The return value is UInt32 only to potentially reduce one 
    // jmp operation. Typically it should be used as bool.
    UInt32 operator [] (unsigned bit) const
    {
        return (bit < Size) ? (pData[bit >> 5] & (1 << (bit & 31))) : 0;
    }

    // Navigate to the next set bit, that is, skip zero bits.
    // If the "start" bit is one the function returns "start".
    unsigned SkipZeros(unsigned start) const
    {
        unsigned word = start >> 5;
        unsigned end  = (Size + 31) >> 5;
        unsigned bit  = start & 31;
        while(word < end)
        {
            UInt32 bits = pData[word] >> bit;
            if (bits)
            {
                return word * 32 + bit + Alg::LowerBit(bits);
            }
            bit = 0;
            ++word;
        }
        return ~0U;
    }

private:
    void resize(unsigned newSize)
    {
        unsigned oldWords = (Size    + 31) >> 5;
        unsigned newWords = (newSize + 31) >> 5;

        if (newWords < oldWords*2)
            newWords = oldWords*2; // Resize by the factor of 2

        if (newWords > oldWords)
        {
            if (pData == &Local) 
            {
                pData = (UInt32*)pHeap->Alloc(newWords * sizeof(UInt32));
                *pData = Local;
            }
            else
            {
                pData = (UInt32*)pHeap->Realloc(pData, newWords * sizeof(UInt32));
            }
            memset(pData + oldWords, 0, (newWords - oldWords) * sizeof(UInt32));
            Size = newWords * 32;
        }
    }

    unsigned    Size;
    UInt32      Local;
    UInt32*     pData;
    MemoryHeap* pHeap;
};


//------------------------------------------------------------------------
template<class T> class Array2D
{
public:
    typedef T ValueType;

    Array2D(MemoryHeap* heap = 0) : 
        pHeap(heap ? heap : Memory::GetGlobalHeap()), Rows(0), Cols(0), Data(0) 
    {}

    ~Array2D()
    {
        ClearAndRelease();
    }

    void ClearAndRelease()
    { 
        pHeap->Free(Data);
        Rows = 0;
        Cols = 0;
        Data = 0;
    }

    void Resize(UPInt rows, UPInt cols)
    {
        // All data is lost!
        pHeap->Free(Data);
        Rows = rows;
        Cols = cols;
        Data = (T*)pHeap->Alloc(Rows * Cols * sizeof(T));
    }

    void Zero()
    {
        memset(Data, 0, Rows * Cols * sizeof(T));
    }

    const T& At(UPInt i, UPInt j) const { return Data[i * Cols + j]; }
          T& At(UPInt i, UPInt j)       { return Data[i * Cols + j]; }
    T   ValueAt(UPInt i, UPInt j) const { return Data[i * Cols + j]; }

private:
    MemoryHeap* pHeap;
    UPInt       Rows;
    UPInt       Cols;
    T*          Data;
};


//------------------------------------------------------------------------

// StrideArray is a simple container wrapper used pass arrays
// of objects separated by stride that may be different from object size.
template<class C>
class StrideArray
{
    C*      pData;
    UPInt   Size;        // Number of items
    UPInt   StrideSize;  // Byte stride between elements
public:

    StrideArray(C* data, UPInt size, UPInt strideSize = sizeof(C))
        : pData(data), Size(size), StrideSize(strideSize)
    { }
    // Constructor indexing StrideArray sub-range.
    StrideArray(const StrideArray& other, UPInt index, UPInt size)
        : Size(size), StrideSize(other.StrideSize)
    { 
        SF_ASSERT((index < other.GetSize()) && ((index+size) <= other.GetSize()));
        pData = (C*)(((UByte*)other.pData) + index * StrideSize);        
    }

    C*      GetDataStart() const { return pData; }
    UPInt   GetSize() const { return Size; }
    UPInt   GetStride() const { return StrideSize; }

    C& operator[] (UPInt index)
    {
        SF_ASSERT(index < Size);
        return *(C*)(((UByte*)pData) + index * StrideSize);
    }
    const C& operator[] (UPInt index) const
    {
        return *(const C*)(((const UByte*)pData) + index * StrideSize);
    }
};


// **** MultiKeyCollection 

// MultiKeyCollection is a grow-only collection that stores items
// of type T based on variable-length key, where every element
// of key is KeyElement.  Added items are destroyed in destructor.

template<class KeyElement, class T,
int KeyPageCount = 8, int ValuePageCount = 8>    
class MultiKeyCollection
{
    struct ValueItem
    {
        KeyElement* pKey;
        unsigned    KeyCount;
        T           Value;
    };

    typedef PagedItemBuffer<KeyElement, KeyPageCount>  KeyBufferType; 
    typedef PagedItemBuffer<ValueItem, ValuePageCount> ValueBufferType;

    KeyBufferType   KeyBuffer;
    ValueBufferType ValueBuffer;

public:
    MultiKeyCollection() { }
    //~PagedItemBuffer destructors clean up resources.
    ~MultiKeyCollection() { }

    // Find item with matching key elements, return 0 if not found.
    T*    Find(KeyElement* keys, unsigned count = 1)
    {
        typename ValueBufferType::Page* pvp = ValueBuffer.GetFirstPage();
        while (pvp)
        {
            for (unsigned i = 0; i < pvp->GetSize(); i++)
            {
                ValueItem& item = pvp->GetItem(i);
                if (item.KeyCount == count)
                {
                    unsigned j;
                    for (j = 0; j< count; j++)
                        if (!(item.pKey[j] == keys[j]))
                            break;
                    if (j == count)
                        return &item.Value;
                }
            }
            pvp = pvp->GetNextPage();
        }
        return 0;
    }

    // Adds new item based on specified key elements.
    T*    Add(KeyElement* keys, unsigned count = 1)
    {
        KeyElement*        newKeys = KeyBuffer.AddItems(keys, count);
        ValueItem*  p = ValueBuffer.AddItem();
        if (!newKeys || !p)
            return 0;
        p->pKey     = newKeys;
        p->KeyCount = count;
        return &p->Value;
    }

    // Adds new item based on specified key elements. Also reports the
    // address of allocated
    T*    Add(KeyElement** ppnewKeys, KeyElement* keys, unsigned count = 1)
    {
        *ppnewKeys = KeyBuffer.AddItems(keys, count);
        ValueItem*  p = ValueBuffer.AddItem();
        if (!*ppnewKeys || !p)
            return 0;
        p->pKey     = *ppnewKeys;
        p->KeyCount = count;
        return &p->Value;
    }

    void Clear()
    {
        KeyBuffer.Clear();
        ValueBuffer.Clear();
    }
};

}} // Scaleform::Render

#endif
