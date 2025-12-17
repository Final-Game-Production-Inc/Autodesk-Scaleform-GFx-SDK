/**************************************************************************

Filename    :   Render_MatrixPool.cpp
Content     :   Handle-indexed pool for different-sized matrices
                and color transforms.
Created     :   June 7, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_MatrixPool.h"
#include "Kernel/SF_HeapNew.h"

#ifdef  SF_RENDER_MATRIXPOOL_UNITTEST
#include "Kernel/SF_Random.h"
#include <stdio.h>
#endif

namespace Scaleform { namespace Render { namespace MatrixPoolImpl {

// ***** MatrixPool Page control constants
enum {
    Pool_HandlePageSize = 2048 - 16,
    Pool_HandlePageAlign= 2048,
    Pool_HandlePageMask = Pool_HandlePageAlign-1,

    // Data page
    Pool_DataPageSize   = 4096,
    Pool_DataPageAlign  = Pool_DataUnitAlign
};


// ***** HandlePage classes

// HandlePage is an allocation unit for handles.
// These pages are currently allocated aligned to alloc for easy UseCount
// tracking.
// Possible improvement:
//   - Remove alignment requirement; use one pointer bit of handle to access page header.

struct HandlePageBase : ListNode<HandlePageBase>
{   
    unsigned          UseCount; // Once this number reaches 0, page can be freed.
    EntryHandle*      pFreeList;
    EntryHandleTable* pTable;
};

struct HandlePage : HandlePageBase
{       
    enum {
        EntryCount = (Pool_HandlePageSize - sizeof(HandlePageBase)) / sizeof(EntryHandle)
    };
    EntryHandle Entries[EntryCount];

    void AddEntriesToFreeList();
};


// ***** DataPage

// Fixed-size DataPage holds allocated matrix data; these pages are allocated,
// freed and memory-squeezed as necessary. Allocation onlu requires a single
// unit alignment (currently 16 bytes).

// Possible improvement:
//  - Currently pages are freed in arbitrary order during memory squeeze. It
//    is possible to implement opposite order free within the current framework
//    by keeping an allocation order list and a free list, so that the
//    contents of the last allocated page are moved to the earlier free page.
//    Benefits need evaluation.

struct DataPage : ListNode<DataPage>
{
    MatrixPool* pPool;
    UInt16      FreeTail;  //
    UInt16      FreeMiddle;
    
    static inline UPInt  GetDataSize()
    { return Pool_DataPageSize - sizeof(DataPage); }

    inline UByte*       GetData()          { return (UByte*)(this + 1); }
    inline DataHeader*  GetFirstHeader()   { return (DataHeader*)GetData(); }
    inline DataHeader*  GetHeaderTailEnd() { return (DataHeader*)(GetData() + GetDataSize() - FreeTail); }
    inline DataHeader*  GetHeaderBufferEnd() { return (DataHeader*)(GetData() + GetDataSize()); }
    
    void Init(MatrixPool* pool)
    {
        pPool     = pool;
        FreeTail  = (UInt16)DataPage::GetDataSize();
        FreeMiddle= 0;
    }

    inline DataHeader* CreateDataHeader(void* mem, UPInt size,
                                        EntryHandle* handle = 0)
    {
        SF_ASSERT((size/Pool_DataUnitSize) < 255);
        
        DataHeader* p = (DataHeader*)mem;
        p->pHandle       = handle;
        p->RefCount      = 1;
        p->DataPageOffset= (SInt16)((UByte*)this - (UByte*)p);
        p->UnitSize      = 1 + (UByte)(size/Pool_DataUnitSize);
        p->Format        = 0;
        return p;
    }

    // Copy data to this destination page
    void CopyData(DataHeader* dest, DataHeader * source)
    {
        SF_ASSERT(source->pHandle);
        if (dest != source)
        {
            // TBD: Optimize with SIMD
            memmove(dest, source, source->UnitSize * Pool_DataUnitSize);
            dest->DataPageOffset = (SInt16)((UByte*)this - (UByte*)dest);
            dest->pHandle->pHeader = dest;
        }
    }
};


// ***** EntryHandle

inline HandlePage* EntryHandle::GetHandlePage()
{
    return (HandlePage*)(UPInt(this) & ~UPInt(Pool_HandlePageMask));
}

void EntryHandle::ReleaseHandle()
{    
    HandlePage* page = GetHandlePage();
    if (!page->pFreeList)
    {
        page->RemoveNode();
        page->pTable->PartiallyFreePages.PushFront(page);
    }
    pNext = page->pFreeList;
    page->pFreeList = this;

    if (--page->UseCount == 0)
        page->pTable->freeEntryPage(page);
}

void EntryHandle::ReallocToFormat(unsigned formatBits)
{
    GetHandlePage()->pTable->pPool->reallocMatrixData(this, formatBits);
}

void EntryHandle::ReallocAddElement(ElementIndex index)
{
    ReallocToFormat(pHeader->GetFormatBits() | (1 << index));
}


// ***** EntryHandleTable

EntryHandleTable::EntryHandleTable(MemoryHeap* pheap, MatrixPool* pool)
: pHeap(pheap), pPool(pool)
{ }

EntryHandleTable::~EntryHandleTable()
{
    // All of the matrix handles should've been freed when
    // MatrixPool is destroyed. If that is the case, these
    // pages should also be empty.
    SF_ASSERT(FullPages.IsEmpty());
    SF_ASSERT(PartiallyFreePages.IsEmpty());
}

void HandlePage::AddEntriesToFreeList()
{
    EntryHandle* p = 0;
    for (UPInt i = EntryCount; i != 0; i--)
    {
        Entries[i-1].pNext = p;
        p = &Entries[i-1];        
    }
    pFreeList = p;
}

// Allocates a page and adds its items to free list.
bool EntryHandleTable::allocEntryPage()
{
    HandlePage* ppage =
        (HandlePage*)SF_HEAP_MEMALIGN(pHeap, Pool_HandlePageSize,
                                      Pool_HandlePageAlign, StatRender_MatrixPool_Mem);
    if (!ppage) return false;

    ppage->pTable  = this;
    ppage->UseCount= 0;
    ppage->AddEntriesToFreeList();
    PartiallyFreePages.PushBack(ppage);
    return true;
}

void EntryHandleTable::freeEntryPage(HandlePage* ppage)
{    
    SF_ASSERT(ppage->UseCount == 0);    
    ppage->RemoveNode(); // Remove from Pages.    
    SF_HEAP_FREE(pHeap, ppage);
}

// Allocate a node
EntryHandle* EntryHandleTable::AllocEntry(DataHeader* header)
{
    // If no free space is available, allocate a new page.
    if (PartiallyFreePages.IsEmpty() && !allocEntryPage())
        return 0;

    HandlePage*  page = PartiallyFreePages.GetFirst();
    EntryHandle* p = page->pFreeList;
    page->pFreeList = p->pNext;
    page->UseCount++;

    // If page became full, move it to FullPages.
    if (p->pNext == 0)
    {
        page->RemoveNode();
        FullPages.PushBack(page);
    }
    p->pHeader = header;
    return p;
}


// ***** DataHeader

inline DataPage* DataHeader::GetDataPage()
{
    return (DataPage*)(((UByte*)this) + DataPageOffset);
}
inline DataHeader* DataHeader::GetNextHeader(unsigned unitSize)
{
    return (DataHeader*)((UByte*)this + Pool_DataUnitSize * unitSize);
}

void DataHeader::Release()
{
    SF_COMPILER_ASSERT(sizeof(DataHeader) <= Pool_DataUnitSize);

    SF_ASSERT(RefCount >= 1);
    if (--RefCount == 0)
    {
        DataPage* page = GetDataPage();
        UInt16 &  freeMiddle = page->FreeMiddle;
        freeMiddle = freeMiddle + (UInt16)(UnitSize * Pool_DataUnitSize);
        page->pPool->FreedSpace += UnitSize * Pool_DataUnitSize;
        pHandle->ReleaseHandle();
        pHandle = 0;
    }
}


// This lookup table calculates offset of the ElementIndex value within a format.
// For lookup, rows are formats (described by FormatBits), while columns are elements.
// Some columns may have 0s if that element doesn't not exit.
// The last column is always the full unit size of everything before matrix. 
HMatrixConstants::ElementsInfo
HMatrixConstants::MatrixElementSizeTable[HMatrixConstants::Has_TableSize] =
{
    {{ 0, 0, 0, 0, 0 }},   // No data
    {{ 0, 0, 0, 0, 2 }},   // Cx
    {{ 0, 0, 0, 0, 2 }},   // T0
    {{ 0, 2, 0, 0, 4 }},   // Cx, T0

    {{ 0, 0, 0, 0, 2 }},   // T1
    {{ 0, 0, 2, 0, 4 }},   // Cx, T1
    {{ 0, 0, 2, 0, 4 }},   // T0, T1
    {{ 0, 2, 4, 0, 6 }},   // Cx, T0, T1

    {{ 0, 0, 0, 0, Pool_UserDataUnits }},       // User
    {{ 0, 0, 0, 2, Pool_UserDataUnits + 2 }},   // Cx, User
    {{ 0, 0, 0, 2, Pool_UserDataUnits + 2 }},   // T0, User
    {{ 0, 2, 0, 4, Pool_UserDataUnits + 4 }},   // Cx, T0, User

    {{ 0, 0, 0, 2, Pool_UserDataUnits + 2 }},   // T1, User
    {{ 0, 0, 2, 4, Pool_UserDataUnits + 4 }},   // Cx, T1, User
    {{ 0, 0, 2, 4, Pool_UserDataUnits + 4 }},   // T0, T1, User
    {{ 0, 2, 4, 6, Pool_UserDataUnits + 6 }}    // Cx, T0, T1, User
};



//--------------------------------------------------------------------
// ***** HMatrix

// Default handle references an Identity matrix for convenience.
struct DefaultMatrixData
{
    union {
        DataHeader Data;
        UByte      DataBytes[Pool_DataUnitSize * 3];
    };
    DefaultMatrixData()
    {
        Data.pHandle  = &HMatrix::NullHandle;
        Data.Format   = 0; // Matrix only
        Data.UnitSize = 3;
        SF_COMPILER_ASSERT(sizeof(Matrix2F) <= Pool_DataUnitSize*2);
        *Data.GetMatrixPtr() = Matrix2F::Identity;
    }
};

DefaultMatrixData HMatrix_DefaultMatrixData;
EntryHandle       HMatrix::NullHandle = {{ &HMatrix_DefaultMatrixData.Data }};


void HMatrix::SetMatrix2D(const Matrix2F& m)
{
    SF_ASSERT(pHandle != &NullHandle);
    if (Has3D())
    {
        if (m == Matrix3F::Identity)
            return;
        pHandle->ReallocToFormat(pHandle->pHeader->Format & ~Has_3D);
    }
    *pHandle->GetMatrixPtr() = m;
}

void HMatrix::SetMatrix3D(const Matrix3F& m)
{
    SF_ASSERT(pHandle != &NullHandle);
    if (!(Has3D()))
    {
        if (m == Matrix3F::Identity)
            return;
        pHandle->ReallocToFormat(pHandle->pHeader->Format | Has_3D);
    }
    *pHandle->GetMatrix3DPtr() = m;
}

void HMatrix::SetCxform(const Cxform& m)
{
    SF_ASSERT(pHandle != &NullHandle);
    if (!pHandle->FormatHasElement(Element_Cxform))
    {
        if (m == Cxform::Identity)
            return;
        pHandle->ReallocAddElement(Element_Cxform);
    }
    *pHandle->GetCxformPtr() = m;
}

void HMatrix::SetTextureMatrix(const Matrix2F& m, unsigned index)
{
    SF_ASSERT(pHandle != &NullHandle);
    ElementIndex e = toTextureElementIndex(index);
    if (!pHandle->FormatHasElement(e))
    {
        if (m == Matrix2F::Identity)
            return;
        pHandle->ReallocAddElement(e);
    }
    *pHandle->GetTextureMatrixPtr(e) = m;
}

void HMatrix::SetUserData(const void* data, UPInt size)
{
    SF_ASSERT(pHandle != &NullHandle);
    SF_ASSERT(size <= Pool_UserDataSize);

    if (!pHandle->FormatHasElement(Element_UserData))
    {
        if (!data)
            return;
        pHandle->ReallocAddElement(Element_UserData);
    }

    void* dataDest = pHandle->GetUserDataPtr();
    if (data)
        memcpy(dataDest, data, size);
    else
        memset(dataDest, 0, size);
}


void HMatrix::operator = (const HMatrix& other)
{
    if (other.pHandle != &NullHandle)
        other.pHandle->AddRef();
    if (pHandle != &NullHandle)
        pHandle->Release();
    pHandle = other.pHandle;
}


//--------------------------------------------------------------------
// ***** MatrixPool

MatrixPool::MatrixPool(MemoryHeap* heap)
: pHeap(heap),
  AllocatedSpace(0), DataPageCount(0), FreedSpace(0),
  HandleTable(heap, getThis()),
  pAllocPage(0), pSqueezePage(0), pLastFreedPage(0)
{
}

MatrixPool::~MatrixPool()
{
    // Free all data pages.
    while(!DataPages.IsEmpty())
    {
        freeDataPage(DataPages.GetFirst());
    }

    if (pLastFreedPage)
        SF_FREE(pLastFreedPage);
}

// 2D version
EntryHandle* MatrixPool::createMatrixHelper(const Matrix2F& m, const Cxform& cx,
                                            unsigned formatBits)
{
    EntryHandle* h = allocMatrixData(formatBits);

    if (h)
    {
        *h->GetMatrixPtr() = m;
        if (formatBits & Has_Cxform)
            *h->GetCxformPtr() = cx;
        if (formatBits & Has_T0)
            h->GetTextureMatrixPtr(Element_T0)->SetIdentity();
        if (formatBits & Has_T1)
            h->GetTextureMatrixPtr(Element_T1)->SetIdentity();
        if (formatBits & Has_UserData)
            memset(h->GetUserDataPtr(), 0, Pool_UserDataSize);
    }
    else
    {
        h = &HMatrix::NullHandle;
    }
    return h;
}

// 3D version
EntryHandle* MatrixPool::createMatrixHelper(const Matrix3F& m, const Cxform& cx,
                                            unsigned formatBits)
{
    EntryHandle* h = allocMatrixData(formatBits);

    if (h)
    {
        *h->GetMatrix3DPtr() = m;
        if (formatBits & Has_Cxform)
            *h->GetCxformPtr() = cx;
        if (formatBits & Has_T0)
            h->GetTextureMatrixPtr(Element_T0)->SetIdentity();
        if (formatBits & Has_T1)
            h->GetTextureMatrixPtr(Element_T1)->SetIdentity();
        if (formatBits & Has_UserData)
            memset(h->GetUserDataPtr(), 0, Pool_UserDataSize);
    }
    else
    {
        h = &HMatrix::NullHandle;
    }
    return h;
}

HMatrix MatrixPool::CreateMatrix(unsigned formatBits)
{    
    return HMatrix(createMatrixHelper(Matrix2F::Identity, Cxform::Identity, formatBits));
}
HMatrix MatrixPool::CreateMatrix(const Matrix2F& m, unsigned formatBits)
{
    return HMatrix(createMatrixHelper(m, Cxform::Identity, formatBits));
}
HMatrix  MatrixPool::CreateMatrix(const Matrix3F& m, unsigned formatBits)
{
    return HMatrix(createMatrixHelper(m, Cxform::Identity, formatBits));
}
HMatrix MatrixPool::CreateMatrix(const Matrix2F& m, const Cxform& cx, unsigned formatBits)
{
    return HMatrix(createMatrixHelper(m, cx, formatBits));
}
HMatrix MatrixPool::CreateMatrix(const Matrix3F& m, const Cxform& cx, unsigned formatBits)
{
    return HMatrix(createMatrixHelper(m, cx, formatBits));
}

bool MatrixPool::MemorySqueeze(SqueezeType type)
{
    if (type != Squeeze_Force)
    {
        // Check thresholds that squeeze is needed.
        // Only squeeze if FreedSpace is ok.
        UPInt squeezeThreshold = (AllocatedSpace * 3)/32;
        if (FreedSpace < squeezeThreshold)
            return false;
    }

    // For incremental squeeze, try to resume from the last left-off location.
    if ((type == Squeeze_Incremental) && pSqueezePage)
    {
        // squeezeMemoryRange will adjust pSqueezePage, so we are done.
        if (squeezeMemoryRange(pSqueezePage, DataPages.GetFirst()->pPrev, type))
            return true;        
    }
    return squeezeMemoryRange(DataPages.GetFirst(), DataPages.GetFirst()->pPrev, type);
}


// Allocates data page adding it to the end of the list.
bool MatrixPool::allocDataPage()
{
    DataPage* page;
    
    if (!pLastFreedPage)
    {
        page = (DataPage*)SF_HEAP_MEMALIGN(pHeap, Pool_DataPageSize,
                                           Pool_DataPageAlign, StatRender_MatrixPool_Mem);
        if (!page) return false;
    }
    else
    {
        page = pLastFreedPage;
        pLastFreedPage = 0;
    }
    
    page->Init(this);
    DataPages.PushBack(page);
    AllocatedSpace += DataPage::GetDataSize();
    DataPageCount++;
    pAllocPage = page;
    return true;
}

void MatrixPool::freeDataPage(DataPage* page)
{
    page->RemoveNode();
    AllocatedSpace -= DataPage::GetDataSize();
    DataPageCount--;
    if (pLastFreedPage)
        SF_FREE(pLastFreedPage);
    pLastFreedPage = page;
}


bool MatrixPool::ensureAllocSpace(UPInt size)
{
    // If pAllocPage is not last, due to using memory squeeze hole, move to last
    // once the hole is exhausted.
    if ((pAllocPage != DataPages.GetLast()) && !DataPages.IsEmpty())
    {
        pAllocPage = DataPages.GetLast();
        if (pAllocPage->FreeTail >= size)
            return true;
    }

    // Try incremental squeeze, subject to internal checks.
    MemorySqueeze(Squeeze_Incremental);
    if (pAllocPage && (pAllocPage->FreeTail >= size))
        return true;

    return allocDataPage();
}

// Allocates data from last page
DataHeader* MatrixPool::allocData(UPInt size, EntryHandle* handle)
{
    SF_ASSERT((size & Pool_DataUnitMask) == 0);
    UPInt fullSize = size + Pool_DataUnitSize; 
        
    if (!pAllocPage || (pAllocPage->FreeTail < fullSize))
    {
        if (!ensureAllocSpace(fullSize))
            return 0;
        SF_ASSERT(pAllocPage->FreeTail >= fullSize);
    }

    // Allocate & initialize data header.
    DataPage*   lastPage = pAllocPage;    
    UByte*      mem = lastPage->GetData() + DataPage::GetDataSize() - lastPage->FreeTail;
    DataHeader* header = lastPage->CreateDataHeader(mem, size, handle);
    lastPage->FreeTail = lastPage->FreeTail - (UInt16)fullSize;
    return header; 
}


EntryHandle* MatrixPool::allocMatrixData(unsigned formatBits)
{    
    UPInt        size   = GetFormatDataSize(formatBits);
    EntryHandle* handle = HandleTable.AllocEntry();
    if (!handle) return 0;

    handle->pHeader = allocData(size, handle);
    if (!handle->pHeader)
    {
        handle->ReleaseHandle();
        return 0;
    }

    // TBD: Init content to identity? External.
    handle->pHeader->Format  = (UByte)formatBits;
    return handle;
}

bool MatrixPool::reallocMatrixData(EntryHandle* handle, unsigned formatBits)
{
    UPInt       size = GetFormatDataSize(formatBits);    
    DataHeader* newData = allocData(size, handle);
    if (!newData)
        return false;
        
    // Init new header.
    // We init oldData after allocData() call, which moves memory.
    DataHeader* oldData = handle->pHeader;
    newData->Format   = (UByte)formatBits;
    newData->RefCount = oldData->RefCount;

    // Copy old data.
    UByte           oldFormat = oldData->Format;
    if (formatBits & Has_3D)
    {
        if (oldData->Format  & Has_3D)
        {
            const Matrix3F* oldMatrix = oldData->GetMatrix3DPtr();
            *newData->GetMatrix3DPtr() = *oldMatrix;
        }
        else
        {
            const Matrix2F* oldMatrix = oldData->GetMatrixPtr();
            *newData->GetMatrix3DPtr() = *oldMatrix;
        }
    }
    else
    {
        const Matrix2F* oldMatrix = oldData->GetMatrixPtr();
        *newData->GetMatrixPtr() = *oldMatrix;
    }

    if (formatBits & Has_Cxform)
    {
        const Cxform* oldCxform = 
            (oldFormat & Has_Cxform) ? oldData->GetCxformPtr() : &Cxform::Identity;
        *newData->GetCxformPtr() = *oldCxform;
    }
    if (formatBits & Has_T0)
    {
        const Matrix2F* oldT0 = (oldFormat & Has_T0) ?
            oldData->GetTextureMatrixPtr(Element_T0) : &Matrix2F::Identity;
        *newData->GetTextureMatrixPtr(Element_T0) = *oldT0;
    }
    if (formatBits & Has_T1)
    {
        const Matrix2F* oldT1 = (oldFormat & Has_T1) ?
            oldData->GetTextureMatrixPtr(Element_T1) : &Matrix2F::Identity;
        *newData->GetTextureMatrixPtr(Element_T1) = *oldT1;
    }
    if (formatBits & Has_UserData)
    {
        void* dataDest = newData->GetUserDataPtr();
        if (oldFormat & Has_UserData)
            memcpy(dataDest, oldData->GetUserDataPtr(), Pool_UserDataSize);
        else
            memset(dataDest, 0, Pool_UserDataSize);
    }    

    // Old data is considered freed, mark it as such.
    if (oldData->pHandle != &HMatrix::NullHandle)
    {        
        UInt16 freedBytes = (UInt16)(oldData->UnitSize * Pool_DataUnitSize);
        
        oldData->pHandle  = 0;
        oldData->RefCount = 0;
        UInt16& freeMiddle = oldData->GetDataPage()->FreeMiddle;
        freeMiddle = freeMiddle + freedBytes; // avoid conversion warning.
        FreedSpace += freedBytes;
    }
    
    // Handle now points to new data.
    handle->pHeader = newData;
    return true;
}


bool MatrixPool::squeezeMemoryRange(DataPage* squeezeStart, DataPage* squeezeEnd,
                                    MatrixPool::SqueezeType type)
{
    bool freedData = false;

    enum {
        // 5% squeeze
        SquezePageThreshold        = (Pool_DataPageSize - sizeof(DataPage)) / 20,
        FreeTailTerminateThreshold = (Pool_DataPageSize - sizeof(DataPage)) / 4
    };

    // These will be re-assigned later on.
    pAllocPage   = 0;
    pSqueezePage = 0;
   
    // Destination is initialized on-demand in the loop.
    DataPage*   sourcePage = squeezeStart;    
    DataPage*   destPage = 0;
    DataHeader *destHeader = 0, *destBufferEnd = 0;

    while(sourcePage != squeezeEnd)
    {
        SF_ASSERT(!DataPages.IsNull(sourcePage));

        // Only include pages that needs squeezing.
        if ((sourcePage->FreeMiddle + sourcePage->FreeTail) <= SquezePageThreshold)
        {
            sourcePage = sourcePage->pNext;
            continue;
        }

        // Copy to destination page, advancing the later as needed.
        // We stop processing page once we hit sourceEnd.
        DataHeader* sourceHeader = sourcePage->GetFirstHeader();
        DataHeader* sourceEnd    = sourcePage->GetHeaderTailEnd();
        UInt16      middleFreed = sourcePage->FreeMiddle;

        if (!destPage)
        {
            destPage = sourcePage;
            destHeader = destPage->GetFirstHeader();
            destBufferEnd = destPage->GetHeaderBufferEnd();

            if (sourcePage->FreeMiddle == 0)
            {
                sourceHeader = sourceEnd;
                destHeader = sourceEnd;
            }
        }
        
        // Store advanceUnitSize since it can be overwritten by data move.
        unsigned advanceUnitSize = 0;
        for(; sourceHeader != sourceEnd;
              sourceHeader = sourceHeader->GetNextHeader(advanceUnitSize))
        {
            advanceUnitSize = sourceHeader->UnitSize;
            if (!sourceHeader->RefCount) // Skip empty space.
                continue;

            // Ensure that there should be enough space in destination.
            // If destPage isn't initialized, then it will begin at the same page.
            if ((destHeader->GetNextHeader(advanceUnitSize)) > destBufferEnd)
            {
                // This can't fail since source page is guaranteed to have space.
                SF_ASSERT(destPage != sourcePage); 

                // This destination page is done; record FreeTail.
                SF_ASSERT(destPage->FreeMiddle == 0); // Modified after source finish.
                destPage->FreeTail = (UInt16)((UByte*)destBufferEnd - (UByte*)destHeader);

                // Since source would've skipped any non-eligible pages in
                // between and freed empty ones, move directly to source.
                destPage = sourcePage;
                destHeader = destPage->GetFirstHeader();
                destBufferEnd = destPage->GetHeaderBufferEnd();
            }

            // Copy to destination & advance pointers.            
            destPage->CopyData(destHeader, sourceHeader);
            destHeader = destHeader->GetNextHeader(advanceUnitSize);
        }

        // Squeeze of a source page eliminates its middle space.
        FreedSpace -= sourcePage->FreeMiddle;
        sourcePage->FreeMiddle = 0;

        // We are done with source page; it may be empty or not.
        if ((destPage != sourcePage) ||
            (destHeader == destPage->GetFirstHeader()))
        {
            if (destPage == sourcePage)
                destPage = 0;
            // Page is fully empty as all data fit into earlier page.
            DataPage *oldPage = sourcePage;
            sourcePage = sourcePage->pNext;
            freeDataPage(oldPage);

            freedData = true;
            // If page freed, ok to terminate.
            if (type == Squeeze_Incremental)
                break;
            continue;
        }
        
        SF_ASSERT(destPage == sourcePage);

        SPInt freeSpaceInPage = (UByte*)destBufferEnd - (UByte*)destHeader;
        if (middleFreed && (freeSpaceInPage >= FreeTailTerminateThreshold))
        {
            freedData = true;
            if (type == Squeeze_Incremental)
                break;
        }
        
        sourcePage = sourcePage->pNext;
    }

    // If we were writing into destination page, update it now that we are done.
    if (destPage)
    {
        SF_ASSERT(destPage->FreeMiddle == 0);
        SPInt freeSpaceInPage = (UByte*)destBufferEnd - (UByte*)destHeader;
        destPage->FreeTail = (UInt16)freeSpaceInPage;

        // This can now be an allocation page.
        pAllocPage   = destPage;
        pSqueezePage = destPage;
    }
    else
    {
        pAllocPage = 0;
        if (!DataPages.IsNull(sourcePage))
            pSqueezePage = sourcePage;
    }

    return freedData;
}


//--------------------------------------------------------------------
// ***** Unit Test for MatrixPool

#ifdef  SF_RENDER_MATRIXPOOL_UNITTEST

void BasicTest(MatrixPool& pool)
{
    HMatrix hm0 = pool.CreateMatrix(HMatrix::Has_2D);

    Cxform cx;
    cx.M[1][1] = 3.0f;

    hm0.SetMatrix2D(Matrix2F::Translation(10.0f, 10.f));
    hm0.SetCxform(cx);

    HMatrix hm1 = pool.CreateMatrix(HMatrix::Has_2D|HMatrix::Has_T0);

    hm1.SetTextureMatrix(Matrix2F::Scaling(0.5f, 0.5f), 0);
    hm1.SetMatrix3D(Matrix3F::Scaling(0.5f, 0.5f, 1.f));

    HMatrix hm2 = pool.CreateMatrix(HMatrix::Has_Cxform|HMatrix::Has_T1);

    hm2.SetMatrix3D(Matrix3F::Translation(0,0,100.f));
    hm0.SetTextureMatrix(Matrix2F::Scaling(0.2f, 0.2f), 1);
    hm2.SetCxform(cx);

    HMatrix hm3 = pool.CreateMatrix(HMatrix::Has_Cxform|HMatrix::Has_T1|HMatrix::Has_3D);

    hm3.SetMatrix3D(Matrix3F::Translation(0,0,100.f));
    hm2.SetTextureMatrix(Matrix2F::Scaling(0.2f, 0.2f), 1);
    hm3.SetCxform(cx);

    Matrix2F m0 = hm0.GetMatrix2D();
    SF_ASSERT(m0.Ty() == 10.0f);
    cx = hm0.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);
    Matrix3F m2 = hm2.GetMatrix3D();
    SF_ASSERT(m2.Tz() == 100.f);
    cx = hm2.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);
    Matrix3F m3 = hm3.GetMatrix3D();
    SF_ASSERT(m3.Tz() == 100.f);
    cx = hm3.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);

    pool.MemorySqueeze(MatrixPool::Squeeze_Force);
    //pool.MemorySqueeze(MatrixPool::Squeeze_Force);

    m0 = hm0.GetMatrix2D();
    SF_ASSERT(m0.Ty() == 10.0f);
    cx = hm0.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);
    m0 = hm1.GetTextureMatrix(1);
    m0 = hm1.GetTextureMatrix(0);
    m2 = hm2.GetMatrix3D();
    SF_ASSERT(m2.Tz() == 100.f);
    cx = hm2.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);
    m3 = hm3.GetMatrix3D();
    SF_ASSERT(m3.Tz() == 100.f);
    cx = hm3.GetCxform();
    SF_ASSERT(cx.M[1][1] == 3.0f);
}

void LinearTest1(MatrixPool& pool)
{
    enum { MatrixCount = 2000 };
    HMatrix matrices[MatrixCount];
    UPInt   i;
    
    for(i = 0; i< MatrixCount; i++)
    {
        matrices[i] = pool.CreateMatrix();
        matrices[i].SetMatrix2D(Matrix2F::Translation((float)i, float(i+1)));
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix2D().Tx() == (float)i);
    }
    
    // Free every second matrix.
    for(i = 0; i< MatrixCount; i+=2)
    {
        matrices[i].Clear();
    }
    for(i = 0; i< MatrixCount; i++)
    {
        if (!matrices[i].IsNull())
        {
            SF_ASSERT(matrices[i].GetMatrix2D().Tx() == (float)i);
        }
    }

    for (i = 0; i<5; i++)
        pool.MemorySqueeze(MatrixPool::Squeeze_Incremental);
    
    // Assign every second matrix & verify
    for(i = 0; i< MatrixCount; i+=2)
    {
        matrices[i] = pool.CreateMatrix();
        matrices[i].SetMatrix2D(Matrix2F::Translation((float)i, float(i+1)));
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix2D().Tx() == (float)i);
    }

    // Convert to have color transforms; this increases size.
    for(i = 0; i< MatrixCount; i++)
    {
        Cxform cx;
        cx.M[0][0] = 100.0f * i;
        matrices[i].SetCxform(cx);
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix2D().Tx() == (float)i);
        SF_ASSERT(matrices[i].GetCxform().M[0][0] == 100.0f * i);
    }

    // Free rare matrices, then squeeze 
    // ... attempts to trigger page carry-over.
    for(i = 0; i< MatrixCount; i+=6)
    {
        matrices[i].Clear();
    }
    for (i = 0; i<5; i++)
        pool.MemorySqueeze(MatrixPool::Squeeze_Incremental);
    for(i = 0; i< MatrixCount; i++)
    {
        if (!matrices[i].IsNull())
        {
            SF_ASSERT(matrices[i].GetMatrix2D().Tx() == (float)i);
        }
    }   
}

// 3D Linear Test
void LinearTest2(MatrixPool& pool)
{
    enum { MatrixCount = 2000 };
    HMatrix matrices[MatrixCount];
    UPInt   i;

    for(i = 0; i< MatrixCount; i++)
    {
        matrices[i] = pool.CreateMatrix();
        matrices[i].SetMatrix3D(Matrix3F::Translation((float)i, float(i+1), 0));
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix3D().Tx() == (float)i);
    }

    // Free every second matrix.
    for(i = 0; i< MatrixCount; i+=2)
    {
        matrices[i].Clear();
    }
    for(i = 0; i< MatrixCount; i++)
    {
        if (!matrices[i].IsNull())
        {
            SF_ASSERT(matrices[i].GetMatrix3D().Tx() == (float)i);
        }
    }

    for (i = 0; i<5; i++)
        pool.MemorySqueeze(MatrixPool::Squeeze_Incremental);

    // Assign every second matrix & verify
    for(i = 0; i< MatrixCount; i+=2)
    {
        matrices[i] = pool.CreateMatrix();
        matrices[i].SetMatrix3D(Matrix3F::Translation((float)i, float(i+1), 0));
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix3D().Tx() == (float)i);
    }

    // Convert to have color transforms; this increases size.
    for(i = 0; i< MatrixCount; i++)
    {
        Cxform cx;
        cx.M[0][0] = 100.0f * i;
        matrices[i].SetCxform(cx);
    }
    for(i = 0; i< MatrixCount; i++)
    {
        SF_ASSERT(matrices[i].GetMatrix3D().Tx() == (float)i);
        SF_ASSERT(matrices[i].GetCxform().M[0][0] == 100.0f * i);
    }

    // Free rare matrices, then squeeze 
    // ... attempts to trigger page carry-over.
    for(i = 0; i< MatrixCount; i+=6)
    {
        matrices[i].Clear();
    }
    for (i = 0; i<5; i++)
        pool.MemorySqueeze(MatrixPool::Squeeze_Incremental);
    for(i = 0; i< MatrixCount; i++)
    {
        if (!matrices[i].IsNull())
        {
            SF_ASSERT(matrices[i].GetMatrix3D().Tx() == (float)i);
        }
    }   
}

using namespace Scaleform::Alg;

unsigned ScaledRandom(unsigned maxValue)
{
    unsigned r = Random::NextRandom() % 100;
    if (r > 90)
        return Random::NextRandom() % maxValue;
    if (r > 60)
        return Random::NextRandom() % (maxValue/2);
    return Random::NextRandom() % (maxValue/4);
}

void RandomTest_Verify(HMatrix* matrices, int* values, unsigned count)
{
    for (unsigned i = 0; i< count; i++)
    {
        if (!matrices[i].IsNull())
        {
            if (matrices[i].Has3D())
            {
                const Matrix3F& m = matrices[i].GetMatrix3D();
                SF_ASSERT(m.Tx() == values[i] + 1);      
                SF_UNUSED(m);
            }
            else
            {
                const Matrix2F& m = matrices[i].GetMatrix2D();
                SF_ASSERT(m.Sx()== i);
                SF_ASSERT(m.Sy() == values[i]);
                SF_ASSERT(m.Tx() == values[i] + 1);             
                SF_UNUSED(m);
            }
            if (matrices[i].HasCxform())
            {
                const Cxform& cx = matrices[i].GetCxform();
                SF_ASSERT(cx.M[0][0] == values[i] + 2);
                SF_ASSERT(cx.M[0][1] == values[i] + 2);
                SF_ASSERT(cx.M[0][2] == values[i] + 2);
                SF_ASSERT(cx.M[0][3] == values[i] + 2);
                SF_ASSERT(cx.M[1][0] == values[i] + 3);
                SF_ASSERT(cx.M[1][3] == values[i] + 3);
                SF_UNUSED(cx);
            }
            if (matrices[i].HasUserData())
            {
                const float* p = matrices[i].GetUserDataFloat();
                SF_ASSERT(p);
                SF_ASSERT(p[0] == values[i] + 8);
                SF_ASSERT(p[1] == values[i] + 9);
                SF_ASSERT(p[2] == values[i] + 10);
                SF_ASSERT(p[3] == values[i] + 11);
                SF_ASSERT(p[4] == values[i] + 12);
                SF_ASSERT(p[5] == values[i] + 13);
                SF_ASSERT(p[6] == values[i] + 14);
                SF_ASSERT(p[7] == values[i] + 15);
            }
        }
    }
    SF_UNUSED(values);
}


void RandomizedTest(MatrixPool &pool)
{
    enum {
        MatrixCount = 8000,
        Iterations  = 50000,
        RangeMax    = 500
    };

    enum Mode
    {
        Mode_RandomInit,
        Mode_RandomClear,
        Mode_RangeInit,
        Mode_RangeClear,
        Mode_SetCxform,
        Mode_SetT0,
        Mode_Set3D,
        Mode_SetUserData,
        Mode_Count
    };

    int     modeCounters[Mode_Count];

    HMatrix matrices[MatrixCount];
    int     values[MatrixCount];
    UPInt   i, j, k, start;

    memset(values, 0, sizeof(int) * MatrixCount);
    memset(modeCounters, 0, sizeof(int) * Mode_Count);

    for (unsigned iteration = 0; iteration < Iterations; iteration++)
    {
        Mode mode = (Mode)(Random::NextRandom() % Mode_Count);
        k = ScaledRandom(RangeMax);

       // SF_ASSERT(iteration != 16);

        switch(mode)
        {
        case Mode_RandomInit:
            for(i = 0; i < k; i++)
            {
                j = Random::NextRandom() % (MatrixCount - 1);
                int v = Random::NextRandom() % 16;
                matrices[j] = pool.CreateMatrix();
                Matrix2F m;
                m.Sx() = (float)j;
                m.Sy() = (float)v;
                m.Tx() = (float)v+1;
                matrices[j].SetMatrix2D(m);
                values[j] = v;
                
               // RandomTest_Verify(matrices, values, MatrixCount);
            }
            break;
        
        case Mode_RandomClear:
            for(i = 0; i < k; i++)
            {
                j = Random::NextRandom() % (MatrixCount - 1);
                matrices[j].Clear();
                values[j] = 0;
            }
            break;

        case Mode_RangeInit:
            start = Random::NextRandom() % (MatrixCount-k-1);
            for(i = start; i< (k+start); i++)
            {
                int v = Random::NextRandom() % 16;
                matrices[i] = pool.CreateMatrix();
                Matrix2F m;
                m.Sx() = (float)i;
                m.Sy() = (float)v;
                m.Tx() = (float)v+1;
                matrices[i].SetMatrix2D(m);
                values[i] = v;
            }
            break;

        case Mode_RangeClear:
            start = Random::NextRandom() % (MatrixCount-k-1);
            for(i = start; i< (k+start); i++)
            {
                matrices[i].Clear();
                values[i] = 0;          
            }
            break;

        case Mode_SetCxform:
            for(i = 0; i < k; i++)
            {
             //   SF_ASSERT((iteration != 252) || (i != 23));

                j = Random::NextRandom() % (MatrixCount - 1);
                if (!matrices[j].IsNull())
                {
                    Cxform cx;
                    cx.M[0][0] = cx.M[0][1] = cx.M[0][2] = cx.M[0][3] = (float)values[j] + 2;
                    cx.M[1][0] = cx.M[1][1] = cx.M[1][2] = cx.M[1][3] = (float)values[j] + 3;
                    matrices[j].SetCxform(cx);
                }
          //      RandomTest_Verify(matrices, values, MatrixCount);
            }
            break;

        case Mode_SetT0:
            for(i = 0; i < k; i++)
            {
                j = Random::NextRandom() % (MatrixCount - 1);
                if (!matrices[j].IsNull())
                {  
                    Matrix2F m;
                    m.Sx() = (float)(j+1);
                    m.Sy() = (float)values[j] + 88;
                    m.Tx() = (float)values[j] + 11;
                    matrices[j].SetTextureMatrix(m, 0);
                }                
            }
            break;

        case Mode_SetUserData:
            for(i = 0; i < k; i++)
            {
                j = Random::NextRandom() % (MatrixCount - 1);

                if (!matrices[j].IsNull())
                {
                    float v = (float) values[j];
                    float userFloats[8] =
                    { 
                        v + 8,  v + 9,  v + 10, v + 11,
                        v + 12, v + 13, v + 14, v + 15
                    };
                    matrices[j].SetUserDataFloat(userFloats, 8);
                }
                //      RandomTest_Verify(matrices, values, MatrixCount);
            }
            break;


        case Mode_Set3D:
            for(i = 0; i < k; i++)
            {
                j = Random::NextRandom() % (MatrixCount - 1);
                int v = Random::NextRandom() % 16;
                Matrix3F m;
                m.Tx() = (float)v+1;
                matrices[j] = pool.CreateMatrix(m);
                values[j] = v;
                // RandomTest_Verify(matrices, values, MatrixCount);
            }
            break;
        } // switch(mode)

        // verify
        RandomTest_Verify(matrices, values, MatrixCount);
        modeCounters[mode]++;
    }
    RandomTest_Verify(matrices, values, MatrixCount);
}


void UnitTest()
{
    MatrixPool pool(Memory::GetGlobalHeap());

    BasicTest(pool);
    LinearTest1(pool);
    LinearTest2(pool);
    RandomizedTest(pool);

    printf("MatrixPool::UnitTest() finished. AllocatedSpace: %d\n",
           pool.GetAllocatedSpace());
    
    pool.MemorySqueeze(MatrixPool::Squeeze_Force);    
}

#endif


}}} // Scaleform::Render::MatrixPool
