/**************************************************************************

PublicHeader:   None
Filename    :   SF_List.h
Content     :   Template implementation for doubly-connected linked List
Created     :   2008
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_List_H
#define INC_SF_Kernel_List_H

#include "SF_Types.h"

namespace Scaleform {

// ***** ListNode
//
// Base class for the elements of the intrusive linked list.
// To store elements in the List do:
//
// struct MyData : ListNode<MyData>
// {
//     . . .
// };
//------------------------------------------------------------------------
template<class T>
struct ListNode
{
    union {
        T*    pPrev;
        void* pVoidPrev;
    };
    union {
        T*    pNext;
        void* pVoidNext;
    };

    void    RemoveNode()
    {
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
        pPrev = reinterpret_cast<T*>(~UPInt(0));
        pNext = reinterpret_cast<T*>(~UPInt(0));
    }

    // Removes us from the list and inserts pnew there instead.
    void    ReplaceNodeWith(T* pnew)
    {
        pPrev->pNext = pnew;
        pNext->pPrev = pnew;
        pnew->pPrev = pPrev;
        pnew->pNext = pNext;
        pPrev = reinterpret_cast<T*>(~UPInt(0));
        pNext = reinterpret_cast<T*>(~UPInt(0));
    }
       
    // Inserts the argument linked list node after us in the list.
    void    InsertNodeAfter(T* p)
    {
        p->pPrev          = pNext->pPrev; // this
        p->pNext          = pNext;
        pNext->pPrev      = p;
        pNext             = p;
    }
    // Inserts the argument linked list node before us in the list.
    void    InsertNodeBefore(T* p)
    {
        p->pNext          = pNext->pPrev; // this
        p->pPrev          = pPrev;
        pPrev->pNext      = p;
        pPrev             = p;
    }

    void    Alloc_MoveTo(ListNode<T>* pdest)
    {
        pdest->pNext = pNext;
        pdest->pPrev = pPrev;
        pPrev->pNext = (T*)pdest;
        pNext->pPrev = (T*)pdest;
    }
};



// ***** List
//
// Doubly linked intrusive list. 
// The data type must be derived from ListNode.
// 
// Adding:   PushFront(), PushBack().
// Removing: Remove() - the element must be in the list!
// Moving:   BringToFront(), SendToBack() - the element must be in the list!
//
// Iterating:
//    MyData* data = MyList.GetFirst();
//    while (!MyList.IsNull(data))
//    {
//        . . .
//        data = MyList.GetNext(data);
//    }
//
// Removing:
//    MyData* data = MyList.GetFirst();
//    while (!MyList.IsNull(data))
//    {
//        MyData* next = MyList.GetNext(data);
//        if (ToBeRemoved(data))
//             MyList.Remove(data);
//        data = next;
//    }
//
//------------------------------------------------------------------------
// List<> represents a doubly-linked list of T, where each T must derive
// from ListNode<B>. B specifies the base class that was directly
// derived from ListNode, and is only necessary if there is an intermediate
// inheritance chain.

template<class T, class B = T> class List
{
public:
    typedef T ValueType;

    List()
    {
        Root.pNext = Root.pPrev = (ValueType*)&Root;
    }

    void Clear()
    {
        Root.pNext = Root.pPrev = (ValueType*)&Root;
    }

    const ValueType* GetFirst() const { return (const ValueType*)Root.pNext; }
    const ValueType* GetLast () const { return (const ValueType*)Root.pPrev; }
          ValueType* GetFirst()       { return (ValueType*)Root.pNext; }
          ValueType* GetLast ()       { return (ValueType*)Root.pPrev; }

    // Determine if list is empty (i.e.) points to itself.
    // Go through void* access to avoid issues with strict-aliasing optimizing out the
    // access after RemoveNode(), etc.
    bool IsEmpty()                   const { return Root.pVoidNext == (const T*)(const B*)&Root; }
    bool IsFirst(const ValueType* p) const { return p == Root.pNext; }
    bool IsLast (const ValueType* p) const { return p == Root.pPrev; }
    bool IsNull (const ValueType* p) const { return p == (const T*)(const B*)&Root; }

    inline static const ValueType* GetPrev(const ValueType* p) { return (const ValueType*)p->pPrev; }
    inline static const ValueType* GetNext(const ValueType* p) { return (const ValueType*)p->pNext; }
    inline static       ValueType* GetPrev(      ValueType* p) { return (ValueType*)p->pPrev; }
    inline static       ValueType* GetNext(      ValueType* p) { return (ValueType*)p->pNext; }

    void PushFront(ValueType* p)
    {
        p->pNext          =  Root.pNext;
        p->pPrev          = (ValueType*)&Root;
        Root.pNext->pPrev =  p;
        Root.pNext        =  p;
    }

    void PushBack(ValueType* p)
    {
        p->pPrev          =  Root.pPrev;
        p->pNext          = (ValueType*)&Root;
        Root.pPrev->pNext =  p;
        Root.pPrev        =  p;
    }

    static void Remove(ValueType* p)
    {
        p->pPrev->pNext = p->pNext;
        p->pNext->pPrev = p->pPrev;
    }

    void BringToFront(ValueType* p)
    {
        Remove(p);
        PushFront(p);
    }

    void SendToBack(ValueType* p)
    {
        Remove(p);
        PushBack(p);
    }

    // Appends the contents of the argument list to the front of this list;
    // items are removed from the argument list.
    void PushListToFront(List<T>& src)
    {
        if (!src.IsEmpty())
        {
            ValueType* pfirst = src.GetFirst();
            ValueType* plast  = src.GetLast();
            src.Clear();
            plast->pNext   = Root.pNext;
            pfirst->pPrev  = (ValueType*)&Root;
            Root.pNext->pPrev = plast;
            Root.pNext        = pfirst;
        }
    }

    void PushListToBack(List<T>& src)
    {
        if (!src.IsEmpty())
        {
            ValueType* pfirst = src.GetFirst();
            ValueType* plast  = src.GetLast();
            src.Clear();
            plast->pNext   = (ValueType*)&Root;
            pfirst->pPrev  = Root.pPrev;
            Root.pPrev->pNext = pfirst;
            Root.pPrev        = plast;
        }
    }

    // Removes all source list items after (and including) the 'pfirst' node from the 
    // source list and adds them to out list.
    void    PushFollowingListItemsToFront(List<T>& src, ValueType *pfirst)
    {
        if (pfirst != &src.Root)
        {
            ValueType *plast = src.Root.pPrev;

            // Remove list remainder from source.
            pfirst->pPrev->pNext = (ValueType*)&src.Root;
            src.Root.pPrev      = pfirst->pPrev;
            // Add the rest of the items to list.
            plast->pNext      = Root.pNext;
            pfirst->pPrev     = (ValueType*)&Root;
            Root.pNext->pPrev = plast;
            Root.pNext        = pfirst;
        }
    }

    // Removes all source list items up to but NOT including the 'pend' node from the 
    // source list and adds them to out list.
    void    PushPrecedingListItemsToFront(List<T>& src, ValueType *ptail)
    {
        if (src.GetFirst() != ptail)
        {
            ValueType *pfirst = src.Root.pNext;
            ValueType *plast  = ptail->pPrev;

            // Remove list remainder from source.
            ptail->pPrev      = (ValueType*)&src.Root;
            src.Root.pNext    = ptail;            

            // Add the rest of the items to list.
            plast->pNext      = Root.pNext;
            pfirst->pPrev     = (ValueType*)&Root;
            Root.pNext->pPrev = plast;
            Root.pNext        = pfirst;
        }
    }


    // Removes a range of source list items starting at 'pfirst' and up to, but not including 'pend',
    // and adds them to out list. Note that source items MUST already be in the list.
    void    PushListItemsToFront(ValueType *pfirst, ValueType *pend)
    {
        if (pfirst != pend)
        {
            ValueType *plast = pend->pPrev;

            // Remove list remainder from source.
            pfirst->pPrev->pNext = pend;
            pend->pPrev          = pfirst->pPrev;
            // Add the rest of the items to list.
            plast->pNext      = Root.pNext;
            pfirst->pPrev     = (ValueType*)&Root;
            Root.pNext->pPrev = plast;
            Root.pNext        = pfirst;
        }
    }


    void    Alloc_MoveTo(List<T>* pdest)
    {
        if (IsEmpty())
            pdest->Clear();
        else
        {
            pdest->Root.pNext = Root.pNext;
            pdest->Root.pPrev = Root.pPrev;

            Root.pNext->pPrev = (ValueType*)&pdest->Root;
            Root.pPrev->pNext = (ValueType*)&pdest->Root;
        }        
    }


private:
    // Copying is prohibited
    List(const List<T>&);
    const List<T>& operator = (const List<T>&);

    ListNode<B> Root;
};


// ***** List2
//
// Doubly linked intrusive list with data accessor.
// Used when the same data must be in two or more different linked lists. 
// List2, unlike List keeps the whole ValueType as the root element 
// (while List keeps only ListNode<T>).
// 
//  struct MyData
//  {
//      MyData* pPrev1;
//      MyData* pNext1;
//      MyData* pPrev2;
//      MyData* pNext2;
//      . . .
//  };
//
//  struct MyData_Accessor1
//  {
//      static void SetPrev(MyData* self, MyData* what)  { self->pPrev1 = what; }
//      static void SetNext(MyData* self, MyData* what)  { self->pNext1 = what; }
//      static const MyData* GetPrev(const MyData* self) { return self->pPrev1; }
//      static const MyData* GetNext(const MyData* self) { return self->pNext1; }
//      static       MyData* GetPrev(MyData* self)       { return self->pPrev1; }
//      static       MyData* GetNext(MyData* self)       { return self->pNext1; }
//  };
//
//  struct MyData_Accessor2
//  {
//      static void SetPrev(MyData* self, MyData* what)  { self->pPrev2 = what; }
//      static void SetNext(MyData* self, MyData* what)  { self->pNext2 = what; }
//      static const MyData* GetPrev(const MyData* self) { return self->pPrev2; }
//      static const MyData* GetNext(const MyData* self) { return self->pNext2; }
//      static       MyData* GetPrev(MyData* self)       { return self->pPrev2; }
//      static       MyData* GetNext(MyData* self)       { return self->pNext2; }
//  };
//
//  List2<MyData, MyData_Accessor1> list1;
//  List2<MyData, MyData_Accessor2> list2;
//
//------------------------------------------------------------------------
template<class T, class Accessor> class List2
{
public:
    typedef T ValueType;

    inline static void SetPrev(ValueType* self, ValueType* what)  { Accessor::SetPrev(self, what);  }
    inline static void SetNext(ValueType* self, ValueType* what)  { Accessor::SetNext(self, what);  }
    inline static const ValueType* GetPrev(const ValueType* self) { return Accessor::GetPrev(self); }
    inline static const ValueType* GetNext(const ValueType* self) { return Accessor::GetNext(self); }
    inline static       ValueType* GetPrev(ValueType* self)       { return Accessor::GetPrev(self); }
    inline static       ValueType* GetNext(ValueType* self)       { return Accessor::GetNext(self); }

    List2()
    {
        SetPrev(&Root, &Root);
        SetNext(&Root, &Root);
    }

    void Clear()
    {
        SetPrev(&Root, &Root);
        SetNext(&Root, &Root);
    }

    const ValueType* GetFirst() const { return GetNext(&Root); }
    const ValueType* GetLast () const { return GetPrev(&Root); }
          ValueType* GetFirst()       { return GetNext(&Root); }
          ValueType* GetLast ()       { return GetPrev(&Root); }

    bool IsEmpty()                   const { return GetNext(&Root) == &Root; }
    bool IsFirst(const ValueType* p) const { return p == GetNext(&Root); }
    bool IsLast (const ValueType* p) const { return p == GetPrev(&Root); }
    bool IsNull (const ValueType* p) const { return p == &Root; }

    void PushFront(ValueType* p)
    {
        SetNext(p, GetNext(&Root));
        SetPrev(p, &Root);
        SetPrev(GetNext(&Root), p);
        SetNext(&Root, p);
    }

    void PushBack(ValueType* p)
    {
        SetPrev(p, GetPrev(&Root));
        SetNext(p, &Root);
        SetNext(GetPrev(&Root), p);
        SetPrev(&Root, p);
    }

    void InsertBefore(ValueType* existing, ValueType* newOne)
    {
        ValueType* prev = GetPrev(existing);
        SetNext(newOne,   existing);
        SetPrev(newOne,   prev);
        SetNext(prev,     newOne);
        SetPrev(existing, newOne);
    }

    void InsertAfter(ValueType* existing, ValueType* newOne)
    {
        ValueType* next = GetNext(existing);
        SetPrev(newOne,   existing);
        SetNext(newOne,   next);
        SetPrev(next,     newOne);
        SetNext(existing, newOne);
    }

    static void Remove(ValueType* p)
    {
        SetNext(GetPrev(p), GetNext(p));
        SetPrev(GetNext(p), GetPrev(p));
    }

    void BringToFront(ValueType* p)
    {
        Remove(p);
        PushFront(p);
    }

    void SendToBack(ValueType* p)
    {
        Remove(p);
        PushBack(p);
    }

    // Appends the contents of the argument list to the front of this list;
    // items are removed from the argument list.
    void PushListToFront(List2<T,Accessor>& src)
    {
        if (!src.IsEmpty())
        {
            ValueType* pfirst = src.GetFirst();
            ValueType* plast  = src.GetLast();
            src.Clear();
            SetNext(plast, GetNext(&Root));
            SetPrev(pfirst, &Root);
            SetPrev(GetNext(&Root), plast);
            SetNext(&Root, pfirst);
        }
    }

private:
    // Copying is prohibited
    List2(const List2<T,Accessor>&);
    const List2<T,Accessor>& operator = (const List2<T,Accessor>&);

    ValueType Root;
};


// ***** FreeListElements
//
// Remove all elements in the list and free them in the allocator
//------------------------------------------------------------------------
template<class List, class Allocator>
void FreeListElements(List& list, Allocator& allocator)
{
    typename List::ValueType* self = list.GetFirst();
    while(!list.IsNull(self))
    {
        typename List::ValueType* next = list.GetNext(self);
        allocator.Free(self);
        self = next;
    }
    list.Clear();
}

} // Scaleform

#endif
