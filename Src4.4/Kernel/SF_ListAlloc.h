/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_ListAlloc.h
Content     :   A simple and fast allocator for "recyclable" elements.
Created     :   2008
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_ListAlloc_H
#define INC_SF_Kernel_ListAlloc_H

#include "SF_Allocator.h"

namespace Scaleform {

// ***** ListAllocBase
//
// A simple and fast allocator for "recyclable" elements. It allocates
// pages of elements of "PageSize" and never performs reallocations, 
// maintaining minimal memory fragmentation. The allocator also keeps
// a linked list of freed elements. 
//
// The allocator is mostly used together with List and List2.
//
// Function Alloc() first checks the freeMem list, and if not empty returns 
// the first freeMem element. If the freeMem list is empty the function 
// allocates elements in pages, creating new pages as needed. 
//
// Function Free(T* element) recycles the element adding it into the 
// freeMem list.
//
// The constructors and destructors are called in Alloc() and Free()
// respectively. Constructors and destructors are not called in the 
// "POD" modification of the allocator.
// 
// Function ClearAndRelease() and the allocator's destructor DO NOT call 
// elements' destructors! They just freeMem the allocated pages. 
//
// The use policy is that if you Alloc() the element you must Free() it.
// The reason of such a behavior is that it's impossible to determine 
// whether or not the element in the page was destroyed, so that, 
// ClearAndRelease cannot call the elements' destructors, otherwise 
// there will be "double destruction".
//------------------------------------------------------------------------
template<class T, int PageSize, class Allocator> class ListAllocBase
{
public:
    typedef T                                       ValueType;
    typedef Allocator                               AllocatorType;
    typedef ListAllocBase<T, PageSize, Allocator>   SelfType;

private:
    struct PageType
    {
        ValueType   Data[PageSize];
        PageType*   pNext;
    };

    struct NodeType
    {
        NodeType*   pNext;
    };
protected:
    ListAllocBase(void* p) :
        FirstPage(0),
        LastPage(0),
        NumElementsInPage(PageSize),
        FirstEmptySlot(0),
        pHeapOrPtr(p)
    {
        SF_COMPILER_ASSERT(sizeof(T) >= sizeof(T*));
    }

public:
    ListAllocBase() :
        FirstPage(0),
        LastPage(0),
        NumElementsInPage(PageSize),
        FirstEmptySlot(0)
    {
        SF_COMPILER_ASSERT(sizeof(T) >= sizeof(T*));
        pHeapOrPtr = this;
    }

    ~ListAllocBase()
    {
        freeMem();
    }

    void ClearAndRelease()
    {
        freeMem();
        FirstPage           = 0;
        LastPage            = 0;
        NumElementsInPage   = PageSize;
        FirstEmptySlot      = 0;
    }

    UPInt GetNumBytes() const
    {
        UPInt numBytes = 0;
        const PageType* page = FirstPage;
        while (page)
        {
            numBytes += sizeof(PageType);
            page = page->pNext;
        }
        return numBytes;
    }

    T* Alloc()
    {
        T* ret = allocate();
        Allocator::Construct(ret);
        return ret;
    }

    T* Alloc(const T& v)
    {
        T* ret = allocate();
        Allocator::Construct(ret, v);
        return ret;
    }

    template<class S>
    T* AllocAlt(const S& v)
    {
        ValueType* ret = allocate();
        Allocator::ConstructAlt(ret, v);
        return ret;
    }

    void Free(T* element)
    {
        Allocator::Destruct(element);
        ((NodeType*)element)->pNext = FirstEmptySlot;
        FirstEmptySlot = (NodeType*)element;
    }

private:
    // Copying is prohibited
    ListAllocBase(const SelfType&);
    const SelfType& operator = (SelfType&);

    inline T* allocate()
    {
        if (FirstEmptySlot)
        {
            T* ret = (T*)FirstEmptySlot;
            FirstEmptySlot = FirstEmptySlot->pNext;
            return ret;
        }
        if (NumElementsInPage < PageSize)
        {   
            return &LastPage->Data[NumElementsInPage++];
        }

        PageType* next = (PageType*)Allocator::Alloc(pHeapOrPtr, sizeof(PageType), __FILE__, __LINE__);
        next->pNext = 0;
        if (LastPage)
            LastPage->pNext = next;
        else
            FirstPage = next;
        LastPage = next;
        NumElementsInPage = 1;
        return &LastPage->Data[0];
    }

    inline void freeMem()
    {
        PageType* page = FirstPage;
        while (page)
        {
            PageType* next = page ->pNext;
            Allocator::Free(page);
            page = next;
        }
    }

    PageType*   FirstPage;
    PageType*   LastPage;
    unsigned    NumElementsInPage;
    NodeType*   FirstEmptySlot;
    void*       pHeapOrPtr;
};



// ***** ListAllocPOD
//
// Paged list allocator for objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// Global heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAllocPOD : 
    public ListAllocBase<T, PageSize, AllocatorGH_POD<T, SID> >
{
public:
    typedef ListAllocPOD<T, PageSize, SID>     SelfType;
    typedef T                                  ValueType;
    typedef AllocatorGH_POD<T, SID>            AllocatorType;
};


// ***** ListAlloc
//
// Paged list allocator array for objects that require 
// explicit construction/destruction. 
// Global heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAlloc : 
    public ListAllocBase<T, PageSize, AllocatorGH<T, SID> >
{
public:
    typedef ListAlloc<T, PageSize, SID>         SelfType;
    typedef T                                   ValueType;
    typedef AllocatorGH<T, SID>                 AllocatorType;
};


// ***** ListAllocLH_POD
//
// Paged list allocator for objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// Local heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAllocLH_POD : 
    public ListAllocBase<T, PageSize, AllocatorLH_POD<T, SID> >
{
public:
    typedef ListAllocLH_POD<T, PageSize, SID>   SelfType;
    typedef T                                   ValueType;
    typedef AllocatorLH_POD<T, SID>             AllocatorType;
};


// ***** ListAllocLH
//
// Paged list allocator for objects that require 
// explicit construction/destruction. 
// Local heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAllocLH : 
    public ListAllocBase<T, PageSize, AllocatorLH<T, SID> >
{
public:
    typedef ListAllocLH<T, PageSize, SID>       SelfType;
    typedef T                                   ValueType;
    typedef AllocatorLH<T, SID>                 AllocatorType;
};

// ***** ListAllocDH_POD
//
// Paged list allocator for objects that DOES NOT require  
// construction/destruction. Constructors and destructors are not called! 
// The specified heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAllocDH_POD : 
    public ListAllocBase<T, PageSize, AllocatorDH_POD<T, SID> >
{
public:
    typedef ListAllocDH_POD<T, PageSize, SID>   SelfType;
    typedef T                                   ValueType;
    typedef AllocatorDH_POD<T, SID>             AllocatorType;

    ListAllocDH_POD(void* pHeap)
     : ListAllocBase<T, PageSize, AllocatorDH_POD<T, SID> >(pHeap) {}
};


// ***** ListAllocDH
//
// Paged list allocator for objects that require 
// explicit construction/destruction. 
// The specified heap is in use.
//------------------------------------------------------------------------
template<class T, int PageSize=127, int SID=Stat_Default_Mem>
class ListAllocDH : 
    public ListAllocBase<T, PageSize, AllocatorDH<T, SID> >
{
public:
    typedef ListAllocDH<T, PageSize, SID>       SelfType;
    typedef T                                   ValueType;
    typedef AllocatorDH<T, SID>                 AllocatorType;

    ListAllocDH(void* pHeap)
     : ListAllocBase<T, PageSize, AllocatorDH<T, SID> >(pHeap) {}
};

} // Scaleform

#endif

