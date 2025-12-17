/**************************************************************************

Filename    :   HeapMH_AllocEngine.h
Content     :   The main allocation engine
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_HeapMH_AllocEngine_H
#define INC_SF_Kernel_HeapMH_AllocEngine_H

#include "../SF_Atomic.h"
#include "HeapMH_Root.h"
#include "HeapMH_AllocBitSet2.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
class AllocEngineMH
{
    typedef List<PageMH> PageListType;
public:
    AllocEngineMH(SysAlloc*     sysAlloc,
                  MemoryHeapMH* heap,
                  UPInt         minAlignSize=16,
                  UPInt         limit=0);
    ~AllocEngineMH();

    bool IsValid() const { return true; }

    void FreeAll();

    UPInt SetLimit(UPInt lim)            { return Limit = lim; }
    void  SetLimitHandler(void* handler) { pLimHandler = handler; }

    void*   Alloc(UPInt size, PageInfoMH* info, bool globalLocked);
    void*   Alloc(UPInt size, UPInt alignSize, PageInfoMH* info, bool globalLocked);

    void*   ReallocInPage(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool globalLocked);
    void*   ReallocInNode(NodeMH* node, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool globalLocked);
    void*   ReallocGeneral(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo, bool globalLocked);
    void*   Realloc(void* oldPtr, UPInt newSize, bool globalLocked);
    void    Free(PageMH* page, void* ptr, bool globalLocked);
    void    Free(NodeMH* node, void* ptr, bool globalLocked);
    void    Free(void* ptr, bool globalLocked);

    void    GetPageInfo(PageMH* page, PageInfoMH* info) const;
    void    GetPageInfo(NodeMH* node, PageInfoMH* info) const;
    void    GetPageInfoWithSize(PageMH* page, const void* ptr, PageInfoMH* info) const;
    void    GetPageInfoWithSize(NodeMH* node, const void* ptr, PageInfoMH* info) const;

    UPInt   GetFootprint() const { return Footprint; }
    UPInt   GetUsedSpace() const { return UsedSpace; }

    UPInt   GetUsableSize(void* ptr);

private:
    PageMH* allocPageLocked(bool* limHandlerOK);
    PageMH* allocPageUnlocked(bool* limHandlerOK);
    void*   allocDirect(UPInt size, UPInt alignSize, bool* limHandlerOK, PageInfoMH* info);
    void    freePage(PageMH* page, bool globalLocked);

    void*   allocFromPage(UPInt size, PageInfoMH* info, bool globalLocked);
    void*   allocFromPage(UPInt size, UPInt alignSize, PageInfoMH* info, bool globalLocked);

    void*   reallocInNodeNoLock(NodeMH* node, void* oldPtr, UPInt newSize, PageInfoMH* newInfo);

    SysAlloc*       pSysAlloc;
    MemoryHeapMH*   pHeap;
    UPInt           MinAlignSize;
    AllocBitSet2MH  Allocator;
    PageListType    Pages;
    UPInt           Footprint;
    UPInt           UsedSpace;
    UPInt           Limit;
    void*           pLimHandler;
    UPInt           UseCount;
};

}} // Scaleform::HeapMH

#endif

