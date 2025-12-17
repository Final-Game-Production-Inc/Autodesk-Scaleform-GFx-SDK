/**************************************************************************

Filename    :   AS3_ValueStack.cpp
Content     :   
Created     :   Mar, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_ValueStack.h"
#include "Kernel/SF_Allocator.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

ValueStack::ValueStack() 
: VSBase(NULL SF_DEBUG_ARG(0))
, pCurrentPage(NULL)
, pReserved(NULL)
{
    SF_DEBUG_CODE(NumOfReservedElem = 0;)
    pCurrentPage = NewPage(DefPageSize);
    pCurrentPage->pNext = NULL;
    pCurrentPage->pPrev = NULL;
    SF_DEBUG_CODE(pCurrentPage->pFirst = NULL;)
    pCurrentPage->pCurrent = NULL;

    pCurrent = pCurrentPage->Values;
    pStack = pCurrent;
    --pCurrent;

    // We need this reserved element at the bottom of the stack to store result
    // of function call moved from released page. Each CallFrame reserves one
    // extra element on its stack for this purpose. But it can happen that
    // function that returns result is called on originally empty stack and
    // it requires allocation/release of a new page (because this function
    // requires bigger stack size than default page size). In this case we
    // need this reserved element.
    Reserve(1);
}

ValueStack::~ValueStack()
{
    // Take care of pReserved.
    while (pReserved)
    {
        Page* p = pReserved;
        pReserved = p->pNext;
        SF_FREE(p);
    }

    // Take care of pCurrentPage.
    while (pCurrentPage)
    {
        // Clean up the stack.
        while (pCurrent >= pCurrentPage->Values)
            PopBack();

        Page* p = pCurrentPage;

        pCurrentPage = pCurrentPage->pPrev;
        if (pCurrentPage)
        {
            // Restore stored pCurrent and pFirst.
            pCurrent = pCurrentPage->pCurrent;
            pStack = pCurrentPage->pFirst;
        }

        SF_FREE(p);
    }
}

void ValueStack::Reserve(UInt16 n)
{
    // If it will fit into the current page.
    if (pCurrent + n < pCurrentPage->Values + pCurrentPage->PageSize)
    {
        // It will fit.
        // We do not move pCurrent, we move only pFirst.
        pStack = pCurrent + 1;
    }
    else
    {
        // It won't fit.
        Page* p = NewPage(n);

        // Add new page to a list.
        p->pNext = NULL;
        p->pPrev = pCurrentPage;
        pCurrentPage->pNext = p;

        // Store pCurrent and pFirst.
        pCurrentPage->pCurrent = pCurrent;
        SF_DEBUG_CODE( pCurrentPage->pFirst = pStack; )

        pCurrentPage = p;

        // New pCurrent and pFirst.
        pStack = pCurrentPage->Values;
        pCurrent = pCurrentPage->Values;
        --pCurrent;
    }

#ifdef SF_BUILD_DEBUG
    NumOfReservedElem = n;
    SF_ASSERT((pStack - pCurrentPage->Values) + NumOfReservedElem <= pCurrentPage->PageSize);
#endif

    ++pCurrentPage->ReservationNum;
}

void ValueStack::ReleaseReserved(ValueType* first SF_DEBUG_ARG(UInt16 prevReservNum))
{
    --pCurrentPage->ReservationNum;
    SF_DEBUG_CODE(NumOfReservedElem = prevReservNum;)

    if (pCurrentPage->ReservationNum == 0 && pCurrentPage->pPrev)
    {
        // We can release this page.
        Page* p = pCurrentPage;
        ValueType* pCurr = pCurrent; 
        pCurrentPage = p->pPrev;

        pCurrentPage->pNext = NULL;

        // Restore stored pCurrent and pFirst.
        pCurrent = pCurrentPage->pCurrent;
        pStack = pCurrentPage->pFirst;

        // Add it to the list of reserved pages.
        p->pNext = pReserved;
        SF_DEBUG_CODE( p->pFirst = 0; )
        pReserved = p;

        // Released page may have data, which should be moved into previous page.
        for (ValueType* data = p->Values; data <= pCurr; ++data)
            PickPushBack(*data);
    }
    else
    {
        pStack = first;
    }
}

void ValueStack::PopReserved(ValueType* current, bool retVal)
{
    // "current" can be on a different page, so we want to limit PopBack() to the
    // current page.

    ValueType* firstOnPage = pCurrentPage->Values;
    ValueType* pageEnd = pCurrentPage->Values + pCurrentPage->PageSize;
    const bool onCurrPage = current >= firstOnPage && current < pageEnd;

    if (onCurrPage)
    {
        if (retVal && (pCurrent > current + retVal))
            Alg::Swap(*pCurrent, *current);

        // Pop all values except of return value.
        current += retVal;
        while (pCurrent > current)
            PopBack();
    }
    else
    {
        if (retVal && (pCurrent >= firstOnPage + retVal))
            Alg::Swap(*pCurrent, *firstOnPage);

        // Pop all values on current page (except of return value).
        firstOnPage += retVal;
        while (pCurrent >= firstOnPage)
            PopBack();
    }
}

ValueStack::Page* ValueStack::AllocPage(UInt16 pageSize)
{
    pageSize = Alg::Max<UInt16>(pageSize, DefPageSize);
    Page* p = (Page*) SF_HEAP_AUTO_ALLOC(this, sizeof(Page) + (pageSize - 1) * sizeof(ValueType));
    p->PageSize = pageSize;
    p->ReservationNum = 0;

    return p;
}

ValueStack::Page* ValueStack::NewPage(UInt16 pageSize)
{
    Page* p = pReserved;

    while (p) 
    {
        if (p->PageSize >= pageSize)
            break;

        // Move forward.
        p = p->pNext;
    }

    if (p)
    {
        // We found a ready to use page.
        if (p == pReserved)
        {
            // This is a head of a list.
            pReserved = p->pNext;
            if (pReserved)
                pReserved->pPrev = NULL;

            // Remove from a list.
            p->pNext = NULL;
            //p->pPrev = NULL;
        }
        else
        {
            // Remove from a list.
            p->pPrev->pNext = p->pNext;
            if (p->pNext)
                p->pNext->pPrev = p->pPrev;

            p->pNext = NULL;
            p->pPrev = NULL;
        }
    }
    else
        // There is nothing appropriate. Allocate a new page.
        p = AllocPage(pageSize);

    return p;
}

///////////////////////////////////////////////////////////////////////////////
ValueRegisterFile::ValueRegisterFile() 
: VRFBase(NULL, 0)
, MaxReservedPageSize(0)
, MaxAllocatedPageSize(0)
, pCurrentPage(NULL)
, pReserved(NULL)
{
    pCurrentPage = NewPage(0);
    pCurrentPage->pNext = NULL;
    pCurrentPage->pPrev = NULL;

    pRF = pCurrentPage->Values;
}

ValueRegisterFile::~ValueRegisterFile()
{
    // Take care of pReserved.
    while (pReserved)
    {
        Page* p = pReserved;
        pReserved = p->pNext;
        SF_FREE(p);
    }

    // Destruct Array elements.
    // ReservedNum is a total number of reserved elements.
    for (UInt16 i = 0; i < ReservedNum; ++i)
        Scaleform::Destruct(pRF + i);

    // Take care of pCurrentPage.
    if (pCurrentPage)
    {
        // Stack should be empty at this point.
        SF_ASSERT(pCurrentPage->pPrev == NULL);
        SF_FREE(pCurrentPage);
    }
}

void ValueRegisterFile::Reserve(UInt16 n)
{
    UInt16 currSize = 0;

    // If it will fit into the current page.
    if (ReservedNum + n <= pCurrentPage->PageSize)
    {
        // It will fit.

        currSize = GetSize();

        // Shift pCurrent.
        pRF = pCurrentPage->Values + ReservedNum;
        // msvc80 generates a warning in case of ReservedNum += n;
		ReservedNum = ReservedNum + n;
    }
    else
    {
        // It won't fit.
        Page* p = NewPage(n);

        // Add new page to a list.
        p->pNext = NULL;
        p->pPrev = pCurrentPage;
        pCurrentPage->pNext = p;
        pCurrentPage->ReservedNum = ReservedNum;
        pCurrentPage->pCurrent = pRF;
        pCurrentPage = p;

        pRF = pCurrentPage->Values;
        ReservedNum = n;
    }

    // Store current size.
    pCurrentPage->PushSize(currSize);

    // Initialize with default values, so, we can read and write to
    // and from these locations.
    for (UInt32 i = 0; i < n; ++i)
        Construct<ValueType>(pRF + i);
}

void ValueRegisterFile::ReleaseReserved(UInt16 n)
{
    // Destruct Array elements.
    for (UInt16 i = 0; i < n; ++i)
        Scaleform::Destruct<ValueType>(pRF + i);

    // msvc80 generates a warning in case of ReservedNum -= n;
	ReservedNum = ReservedNum - n;
    pRF -= pCurrentPage->PopSize();

    if (ReservedNum == 0 && pCurrentPage->pPrev)
    {
        // We can release whole page.

        MaxReservedPageSize = Alg::Max(pCurrentPage->PageSize, MaxReservedPageSize);

        Page* p = pCurrentPage->pPrev;
        pCurrentPage->pPrev = NULL;
        
        pCurrentPage->pNext = pReserved;
        pReserved = pCurrentPage;

        pCurrentPage = p;
        pCurrentPage->pNext = NULL;

        // Restore stored info.
        ReservedNum = pCurrentPage->ReservedNum;
        pRF = pCurrentPage->pCurrent;
    }
}

ValueRegisterFile::Page* ValueRegisterFile::AllocPage(UInt16 pageSize)
{
    if (pageSize > MaxAllocatedPageSize)
        MaxAllocatedPageSize = static_cast<UInt16>((pageSize + DefPageSize) / DefPageSize * DefPageSize);
    else
        // Allocate MaxAllocatedPageSize or DefPageSize if it is bigger.
        MaxAllocatedPageSize = Alg::Max<UInt16>(MaxAllocatedPageSize, DefPageSize);

    pageSize = MaxAllocatedPageSize;
    const UPInt memSize = sizeof(Page) + ((pageSize - 1) * sizeof(ValueType)) + (pageSize * sizeof(UInt16));
    Page* p = (Page*) SF_HEAP_AUTO_ALLOC(this, memSize);

    p->PageSize = pageSize;
    p->ReservedNum = 0;
    p->CurrPos = 0;
    p->pCurrent = NULL;

    return p;
}

ValueRegisterFile::Page* ValueRegisterFile::NewPage(UInt16 pageSize)
{
    Page* p = NULL;

    if (pageSize <= MaxReservedPageSize)
    {
        // There is an already allocated page of appropriate size.

        p = pReserved;
        while (p) 
        {
            if (p->PageSize >= pageSize)
                break;

            // Move forward.
            p = p->pNext;
        }
    }

    if (p)
    {
        // We found a ready to use page.
        if (p == pReserved)
        {
            // This is a head of a list.
            pReserved = p->pNext;
            if (pReserved)
                pReserved->pPrev = NULL;

            // Remove from a list.
            p->pNext = NULL;
            //p->pPrev = NULL;
        }
        else
        {
            // Remove from a list.
            p->pPrev->pNext = p->pNext;
            if (p->pNext)
                p->pNext->pPrev = p->pPrev;

            p->pNext = NULL;
            p->pPrev = NULL;
        }
    }
    else
        // There is nothing appropriate. Allocate a new page.
        p = AllocPage(pageSize);

    SF_ASSERT(p->CurrPos == 0);

    return p;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

