/**************************************************************************

Filename    :   Video_CriHeap.cpp
Content     :   Video custom heap
Created     :   June 4, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Types.h"
#include "Kernel/SF_HeapNew.h"
#include "Video/Video_CriHeap.h"

namespace Scaleform { namespace GFx { namespace Video {

//////////////////////////////////////////////////////////////////////////
//

static void  *customHeap_Alloc(CriHeap heap, Sint32 size, const Char8 *name, Sint32 align);
static Sint32 customHeap_Free (CriHeap heap, void *ptr);

// Function Table for CRI heap
criHeapVirtualFunctionTable custom_heap_vtbl =
{
    customHeap_Alloc,   // criHeap_AllocFix();
    customHeap_Alloc,   // criHeap_AllocTemporary();
    customHeap_Alloc,   // criHeap_AllocDynamic();
    customHeap_Free     // criHeap_Free();
};

// CRI heap object
struct CRICustomHeap {
    SF_MEMORY_REDEFINE_NEW(CRICustomHeap, Stat_Video_Mem)
    struct _criheap_vfunctiontable *vtbl;
    MemoryHeap                     *pHeap;
};

static void *customHeap_Alloc(CriHeap heap, Sint32 size, const Char8 *name, Sint32 align)
{
    SF_UNUSED(name);

    CRICustomHeap* h = (CRICustomHeap*)heap;
    void* ptr = SF_HEAP_MEMALIGN(h->pHeap, size, align, Stat_Video_Mem);
    SF_ASSERT(ptr);

    return ptr;
}

static Sint32 customHeap_Free(CriHeap, void *ptr)
{
    SF_FREE_ALIGN(ptr);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//

CriHeap criSmpCustomHeap_Create(MemoryHeap* pheap)
{
    MemoryHeap* pHeap = pheap->CreateHeap("_Cri_Video_Heap", 0, 64, 128*1024, 0);
    CRICustomHeap* h = SF_HEAP_NEW(pHeap) CRICustomHeap;
    h->vtbl = &custom_heap_vtbl;
    h->pHeap = pHeap;
    pHeap->ReleaseOnFree(h);
    return (CriHeap)h;
}

void criSmpCustomHeap_Destroy(CriHeap heap)
{
    CRICustomHeap* h = (CRICustomHeap*)heap;
    delete h;
}

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO
