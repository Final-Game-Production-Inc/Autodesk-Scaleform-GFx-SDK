/**************************************************************************

Filename    :   GFx_Shape.cpp
Content     :   Shape character definition with paths and edges
Created     :
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_File.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_Shape.h"

#include <float.h>
#include <stdlib.h>


namespace Scaleform { namespace GFx {

PathAllocator::PathAllocator(unsigned pageSize) : 
    pFirstPage(0), pLastPage(0), FreeBytes(0), DefaultPageSize((UInt16)pageSize)
{
}

PathAllocator::~PathAllocator()
{
    Clear();
}

void   PathAllocator::Clear()
{
    for(Page* pcurPage = pFirstPage; pcurPage; )
    {
        Page* pnextPage = pcurPage->pNext;
        SF_FREE(pcurPage);
        pcurPage = pnextPage;
    }

     pFirstPage = 0;
     pLastPage  = 0;
     FreeBytes  = 0;
}

// edgesDataSize - size of geometrical data including the optional edge count, edge infos and vertices.
// Can be 0 if new shape.
// pathSize - 0 - new shape, 1 - 8-bit, 2 - 16-bit (aligned), 4 - 32-bit (aligned) 
// edgeSize - 2 - 16-bit, 4 - 32-bit
UByte* PathAllocator::AllocPath(unsigned edgesDataSize, unsigned pathSize, unsigned edgeSize)
{
    unsigned freeBytes = FreeBytes;
    unsigned size = 1 + pathSize*3 + edgesDataSize;
    unsigned sizeForCurrentPage = size;

    if (edgesDataSize > 0)
    {
        // calculate the actual size of path taking into account both alignments (for path and for edges)
        unsigned delta = 0;
        if (pLastPage)
        {
            UByte* ptr = pLastPage->GetBufferPtr(freeBytes) + 1; // +1 is for first byte (bit flags)

            // The first delta represents alignment for path
            delta = (unsigned)(((UPInt)ptr) & (pathSize - 1));
            delta = ((delta + pathSize - 1) & (~(pathSize - 1))) - delta; // aligned delta

            // The second delta (delta2) represents the alignment for edges, taking into account
            // the previous alignment.
            unsigned delta2 = (unsigned)((UPInt)ptr + delta + pathSize*3) & (edgeSize - 1);
            delta2 = ((delta2 + edgeSize - 1) & (~(edgeSize - 1))) - delta2;

            delta += delta2;
        }
        sizeForCurrentPage += delta;
        if (!pLastPage || FreeBytes < sizeForCurrentPage)
        {
            // new page will be allocated, calculate new delta size
            delta = (1 + (pathSize - 1)) & (~(pathSize - 1));
            delta = ((delta + pathSize*3 + (edgeSize - 1)) & (~(edgeSize - 1))) + edgesDataSize - size;
        }
        size += delta;
    }
    UByte* ptr = AllocMemoryBlock(sizeForCurrentPage, size);
    return ptr;
}

UByte* PathAllocator::AllocRawPath(UInt32 sizeInBytes)
{
    return AllocMemoryBlock(sizeInBytes, sizeInBytes);
}

UByte* PathAllocator::AllocMemoryBlock(UInt32 sizeForCurrentPage, UInt32 sizeInNewPage)
{
    unsigned size = sizeInNewPage;
    unsigned freeBytes = FreeBytes;
    if (pLastPage == NULL || FreeBytes < sizeForCurrentPage)
    {
        unsigned pageSize = DefaultPageSize;
        Page* pnewPage;
        if (size > pageSize)
        {
            pageSize = size;
        }
        freeBytes           = pageSize;
        pnewPage            = (Page*)SF_HEAP_AUTO_ALLOC(this, sizeof(Page) + pageSize);
                                                      // Was: StatMD_ShapeData_Mem);
        if (!pnewPage)
            return NULL;
#ifdef SF_BUILD_DEBUG
        memset(pnewPage, 0xba, sizeof(Page) + pageSize);
#endif
        pnewPage->pNext     = NULL;
        pnewPage->PageSize  = pageSize;
        if (pLastPage)
        {
            pLastPage->pNext    = pnewPage;
            // correct page size
            pLastPage->PageSize = pLastPage->PageSize - FreeBytes;
        }
        pLastPage = pnewPage;
        if (!pFirstPage)
            pFirstPage = pnewPage;
    }
    else
    {
        size = sizeForCurrentPage;
    }

    UByte* ptr  = pLastPage->GetBufferPtr(freeBytes);
#ifdef SF_BUILD_DEBUG
    memset(ptr, 0xcc, size);
#endif
    FreeBytes   = (UInt16)(freeBytes - size);

    return ptr;
}

bool   PathAllocator::ReallocLastBlock(UByte* ptr, UInt32 oldSize, UInt32 newSize)
{
    SF_ASSERT(newSize <= oldSize);
    if (newSize < oldSize && pLastPage && IsInPage(pLastPage, ptr))
    {
        UPInt off = ptr - pLastPage->GetBufferPtr();
        if (pLastPage->PageSize - (off + oldSize) == FreeBytes)
        {
            UPInt newFreeBytes = pLastPage->PageSize - (off + newSize);
            if (newFreeBytes < 65536)
                FreeBytes = UInt16(newFreeBytes);
        }
    }
    return false;
}

void PathAllocator::SetDefaultPageSize(unsigned dps)
{
    if (pFirstPage == NULL)
    {
        SF_ASSERT(dps < 65536);
        DefaultPageSize = (UInt16)dps;
    }
}

//////////////////////////////////////////////////////////////////////////
Ptr<Render::TreeNode> ShapeBaseCharacterDef::CreateTreeShape(Render::Context& context, 
                                                             MovieDefImpl* defImpl) const
{
    Ptr<Render::TreeShape> tshp = *context.CreateEntry<Render::TreeShape>();
    if (NeedsResolving())
    {
        // the shape mesh provider may require to be resolved (for example, images in fill styles).
        Ptr<Render::ShapeMeshProvider> shMeshProv = defImpl->pBindData->GetShapeMeshProvider(pShapeMeshProvider);
        // if shape mesh provider is not found in BindData then we need to resolve it and then cache it there
        if (!shMeshProv)
        {
            SF_ASSERT(defImpl);
            shMeshProv = BindResourcesInStyles(defImpl->GetResourceBinding());
            defImpl->pBindData->AddShapeMeshProvider(pShapeMeshProvider, shMeshProv);
        }
        SF_ASSERT(shMeshProv);
        tshp->SetShape(shMeshProv);
    }
    else // no resolving is necessary: no images to bind.
        tshp->SetShape(pShapeMeshProvider);
    return tshp.GetPtr();
}


}} // namespace Scaleform::GFx

