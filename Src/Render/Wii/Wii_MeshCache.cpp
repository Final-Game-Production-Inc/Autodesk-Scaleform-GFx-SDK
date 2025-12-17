/**************************************************************************

Filename    :   Wii_MeshCache.cpp
Content     :   Wii Mesh Cache implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Wii/Wii_MeshCache.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"


namespace Scaleform { namespace Render { namespace Wii {


// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params)
    : SimpleMeshCache(pheap, params, 0)
{
}

MeshCache::~MeshCache()
{
    Reset(); 
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool    MeshCache::Initialize(MemoryManager* pmem, RenderSync* psync)
{
    pRenderSync = psync;
    pMemManager = pmem;

    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;

    if (!allocateReserve())
        return false;

    return true;
}

void    MeshCache::Reset()
{
    releaseAllBuffers();
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    MeshCacheParams params(argParams), oldParams(Params);
    adjustMeshCacheParams(&params);

    CacheList.EvictAll();
    Params = params;

    if (Params.StagingBufferSize != params.StagingBufferSize)
    {
        if (!StagingBuffer.Initialize(pHeap, params.StagingBufferSize))
        {
            if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
            {
                SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore StagingBuffer after fail");
            }
            return false;
        }
    }

    if ((Params.MemReserve != oldParams.MemReserve) ||
        (Params.MemGranularity != oldParams.MemGranularity))
    {
        releaseAllBuffers();

        // Allocate new reserve. If not possible, restore previous one and fail.
        if (params.MemReserve && !allocateReserve())
        {
            SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore Reserve after fail");
        }
    }
    return true;
}

void MeshCache::adjustMeshCacheParams(MeshCacheParams* p)
{
    p->MaxBatchInstances    = Alg::Clamp<unsigned>(p->MaxBatchInstances, 1, SF_RENDER_WII_INSTANCE_MATRICES);
    p->VBLockEvictSizeLimit = Alg::Clamp<UPInt>(p->VBLockEvictSizeLimit, 0, p->VBLockEvictSizeLimit = 1024 * 256);
    p->InstancingThreshold  = 1<<30;
}

SimpleMeshBuffer*  MeshCache::createHWBuffer(UPInt size, AllocType atype, unsigned arena)
{
    MeshBuffer* pbuffer = SF_HEAP_NEW(pHeap) MeshBuffer(size, atype, arena);
    if (!pbuffer)
        return 0;

    pbuffer->pData = (UByte*)pMemManager->Alloc(pbuffer->Size, 32, Memory_Wii_MEM1, arena);
    if (!pbuffer->pData)
    {
        delete pbuffer;
        return false;
    }

    return pbuffer;
}

void        MeshCache::destroyHWBuffer(SimpleMeshBuffer* pbuffer)
{
    pMemManager->Free(pbuffer->pData, pbuffer->Size);
    delete pbuffer;
}

MeshCache::AllocResult
MeshCache::AllocCacheItem(Render::MeshCacheItem** pdata,
                          UByte** pvertexDataStart, IndexType** pindexDataStart,
                          MeshCacheItem::MeshType meshType,
                          MeshCacheItem::MeshBaseContent &mc,
                          UPInt vertexBufferSize,
                          unsigned vertexCount, unsigned indexCount,
                          bool waitForCache,
                          const VertexFormat* pDestFormat)
{
    UPInt   allocAddress;
    UPInt   allocSize;
    UPInt   indexSize;

    if (meshType == MeshCacheItem::Mesh_Regular)
        indexSize = 256 + ((const SysVertexFormat*)pDestFormat->pSysFormat.GetPtr())->IndexSize * indexCount;
    else
        indexSize = indexCount * sizeof(IndexType);
    allocSize = vertexBufferSize + indexSize;

    // ensure start of all buffers are aligned, since SimpleMeshCache doesn't align.
    if (allocSize & 31)
        allocSize = (allocSize + 31) & ~31;

    if (!allocBuffer(&allocAddress, allocSize, waitForCache))
        return Alloc_Fail;

    MeshBuffer* pbuffer  = (MeshBuffer*)findBuffer(allocAddress);
    UPInt       ibOffset = allocAddress - (UPInt)pbuffer->pData;
    UPInt       vbOffset = ibOffset + indexSize;

    // Create new MeshCacheItem; add it to hash.
    if (meshType == MeshCacheItem::Mesh_Regular)
        *pdata = MeshCacheItem::Create(meshType, &CacheList, mc, pbuffer, allocAddress, allocSize,
                                       vbOffset, vertexCount, indexCount,
                                       ibOffset, indexSize, pDestFormat);
    else
        *pdata = MeshCacheItem::Create(meshType, &CacheList,
                                       mc, pbuffer, allocAddress, allocSize, vbOffset, vertexCount, ibOffset, indexCount, pDestFormat);

    *pvertexDataStart = (UByte*)allocAddress + indexSize;
    *pindexDataStart  = (IndexType*)(allocAddress);
    return Alloc_Success;
}

void MeshCache::PostUpdateMesh(Render::MeshCacheItem * pcacheItem)
{
    Wii::MeshCacheItem* pmesh = (MeshCacheItem*)pcacheItem;

    if (pmesh->Type == MeshCacheItem::Mesh_Regular)
    {
        const SysVertexFormat* pwiivf = (const SysVertexFormat*)pmesh->pFormat->pSysFormat.GetPtr();

        UByte*   pIndexData = ((UByte*)pmesh->pBuffer->pData) + pmesh->DListOffset + pmesh->PrimOffset;
        unsigned indexCount = *((UInt16*) (pIndexData - 2));
        unsigned dlSize = pmesh->PrimOffset + indexCount * pwiivf->IndexSize;
        if (dlSize & 31)
        {
            unsigned dlSizePad = (dlSize + 31) & ~31;
            memset(pIndexData + indexCount * pwiivf->IndexSize, 0, dlSizePad - dlSize);
            dlSize = dlSizePad;
        }
        pmesh->DListSize = dlSize;
    }

    DCStoreRange((UByte*)pmesh->AllocAddress, pmesh->AllocSize);
    GXInvalidateVtxCache();
}

extern const UInt32 Factors[];

static UByte* DLSetArray(UByte* pdlist, GXAttr attr, const void* pVerts, unsigned stride)
{
    *((UInt16*)pdlist)++ = 0x0897 + attr;
    *((const void**)pdlist)++ = pVerts;
    *((UInt16*)pdlist)++ = 0x08a7 + attr;
    *((UInt32*)pdlist)++ = stride;
    return pdlist;
}

static UByte* DLSetArrays(UByte* pdlist, const VertexFormat* pvf, UByte* pVerts)
{
    GXAttr factor = GX_VA_CLR0;

    for (const VertexElement* pve = pvf->pElements; pve->Attribute != VET_None; pve++)
    {
        switch (pve->Attribute & (VET_Usage_Mask|VET_Index_Mask))
        {
        case VET_Pos:      pdlist = DLSetArray(pdlist, GX_VA_POS, pVerts + pve->Offset, pvf->Size); break;
        case VET_Color:    pdlist = DLSetArray(pdlist, GX_VA_CLR0, pVerts + pve->Offset, pvf->Size); factor = GX_VA_CLR1; break;
        case VET_TexCoord: pdlist = DLSetArray(pdlist, GX_VA_TEX0, pVerts + pve->Offset, pvf->Size); break;
        case VET_Color | VET_Index1:
        case VET_Color | VET_Index2:
                           pdlist = DLSetArray(pdlist, factor, Factors, 4); break;
        default:
            SF_ASSERT(0);
        }
    }
    return pdlist;
}

static UByte* DLSetVertexDesc(UByte* pdlist, const VertexFormat* pvf)
{
    UInt32 vcd0 = 0, vcd1 = 0, colors = 0, texs = 0;

    for (const VertexElement* pve = pvf->pElements; pve->Attribute != VET_None; pve++)
    {
        GXAttrType mode;
        
        switch (pve->Attribute & VET_CompType_Mask)
        {
        case VET_I8:  mode = GX_INDEX8; break;
        default:      mode = GX_INDEX16; break;
        }

        switch (pve->Attribute & VET_Usage_Mask)
        {
        case VET_Instance:
            vcd0 |= (1 << (1 + (pve->Attribute & VET_Components_Mask))) - 1;
            break;

        case VET_Pos:
            vcd0 |= mode << 9;
            break;

        case VET_Color:
            SF_ASSERT(colors <= 1);
            vcd0 |= mode << (13 + colors * 2);
            colors++;
            break;

        case VET_TexCoord:
            SF_ASSERT(texs <= 7);
            vcd1 |= mode << (texs * 2);
            texs++;
        }
    }

    *((UInt16*)pdlist)++ = 0x0850;
    *((UInt32*)pdlist)++ = vcd0;
    *((UInt16*)pdlist)++ = 0x0860;
    *((UInt32*)pdlist)++ = vcd1;
    *pdlist++ = 0x10;
    *((UInt32*)pdlist)++ = 0x00001008;
    *((UInt32*)pdlist)++ = colors | (texs << 4);

    return pdlist;
}

static UByte* DLSetCurrentMtx(UByte* pdlist, UInt32 basematrix, unsigned ntextmat)
{
    UInt32    mtx0 = basematrix;

    if (ntextmat)
        mtx0 |= ((basematrix+3) << 6) + ((basematrix+5) << 12);
    else
        mtx0 |= (60 << 6) + (60 << 12);

    *((UInt16*)pdlist)++ = 0x0830;
    *((UInt32*)pdlist)++ = mtx0;
    *((UInt16*)pdlist)++ = 0x0840;
    *((UInt32*)pdlist)++ = 0;
    *pdlist++ = 0x10;
    *((UInt32*)pdlist)++ = 0x00011018;
    *((UInt32*)pdlist)++ = mtx0;
    *((UInt32*)pdlist)++ = 0;

    return pdlist;
}

static void ConvertMeshVertices(const VertexFormat& sourceFormat, const void* psource,
                                const VertexFormat& destFormat, void* pdest, void* pdestExtra,
                                unsigned count)
{
    const VertexElement* pdestElement;
    const VertexElement* psourceElement;

    const SysVertexFormat* pwiivf = (const SysVertexFormat*)destFormat.pSysFormat.GetPtr();
    unsigned extraOffset = 0;

    for (pdestElement = destFormat.pElements; pdestElement->Attribute != VET_None; pdestElement++)
    {
        unsigned destUsage = pdestElement->Attribute & (VET_Usage_Mask|VET_Index_Mask);

        if ((pdestElement->Attribute & VET_CompType_Mask) == VET_I8)
        {
            if (pdestExtra)
            {
                int factor1offset = -1, factor2offset = -1;

                UByte* pdestPtr = ((UByte*)pdestExtra) + extraOffset;
                UByte* psourcePtr = ((UByte*)psource);
                UByte* psourceEnd = psourcePtr + sourceFormat.Size * count;

                for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
                    if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | VET_Index1))
                        factor1offset = psourceElement->Offset;
                    else if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | VET_Index2))
                        factor2offset = psourceElement->Offset;

                if (factor2offset >= 0 && factor1offset >= 0)
                {
                    while(psourcePtr < psourceEnd)
                    {
                        UByte f1 = psourcePtr[factor1offset], f2 = psourcePtr[factor2offset];
                        UByte factors = (psourcePtr[factor2offset] >> 7) | ((psourcePtr[factor1offset] >> 6) & 0x2);
                        *pdestPtr = factors;
                        pdestPtr += pwiivf->ExtraSize;
                        psourcePtr += sourceFormat.Size;
                    }
                }
                else if (factor1offset >= 0)
                {
                    while(psourcePtr < psourceEnd)
                    {
                        UByte f1 = psourcePtr[factor1offset];
                        UByte factors = (psourcePtr[factor1offset] >> 6) | 1;
                        *pdestPtr = factors;
                        pdestPtr += pwiivf->ExtraSize;
                        psourcePtr += sourceFormat.Size;
                    }
                }
                else if (factor2offset >= 0)
                {
                    while(psourcePtr < psourceEnd)
                    {
                        UByte f2 = psourcePtr[factor2offset];
                        UByte factors = (psourcePtr[factor2offset] >> 7) | 2;
                        *pdestPtr = factors;
                        pdestPtr += pwiivf->ExtraSize;
                        psourcePtr += sourceFormat.Size;
                    }
                }
                extraOffset++;
            }

            continue;
        }

        // Find the matching usage in the source declaration element.
        for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
            if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == destUsage)
                break;

        if (psourceElement->Attribute == pdestElement->Attribute)
        {
            UByte* pdestPtr, *psourcePtr;
            pdestPtr = ((UByte*)pdest) + pdestElement->Offset;
            psourcePtr = ((UByte*)psource)+ psourceElement->Offset;

            CopyVertexElements(psourcePtr, sourceFormat.Size,
                               pdestPtr, destFormat.Size, psourceElement->Size(), count);
        }
        else if (psourceElement->Attribute == VET_ColorARGB8)
        {
            UByte* pdestPtr, *psourcePtr;
            pdestPtr = ((UByte*)pdest) + pdestElement->Offset;
            psourcePtr = ((UByte*)psource)+ psourceElement->Offset;
            UByte* psourceEnd = psourcePtr + sourceFormat.Size * count;

            while(psourcePtr < psourceEnd)
            {
                UInt32* ps = (UInt32*)psourcePtr;
                UInt32* pd = (UInt32*)pdestPtr;
                *pd = (*ps >> 24) | ((*ps & 0xffffff) << 8);
                pdestPtr   += destFormat.Size;
                psourcePtr += sourceFormat.Size;
            }
        }
        else
            SF_ASSERT(0);
    }
}

static void ConvertMeshIndices(const VertexFormat& sourceFormat, const void* psource, const void* psourceIndex, const void* psourceExtra,
                               const VertexFormat& destFormat, void* pdestIndex, int indexAdj,
                               unsigned indexCount, unsigned indexStride, int BatchIndex)
{
    const VertexElement* pdestElement;
    const VertexElement* psourceElement;

    const SysVertexFormat* pwiivf = (const SysVertexFormat*)destFormat.pSysFormat.GetPtr();

    unsigned indexOffset = pwiivf->NumMatrices;

    for (pdestElement = destFormat.pElements; pdestElement->Attribute != VET_None; pdestElement++)
    {
        unsigned destUsage = pdestElement->Attribute & (VET_Usage_Mask|VET_Index_Mask);

        if (destUsage == VET_Instance)
        {
            UByte* pdestPtr = (UByte*)pdestIndex;

            for (unsigned j = 0; j < indexCount; j++, pdestPtr += pwiivf->IndexSize)
            {
                switch (pwiivf->NumMatrices)
                {
                case 3: pdestPtr[2] = BatchIndex;
                case 2: pdestPtr[1] = BatchIndex;
                case 1: pdestPtr[0] = BatchIndex; break;
                default: SF_ASSERT(0);
                }
            }
            continue;
        }

        if ((pdestElement->Attribute & VET_CompType_Mask) == VET_I8)
        {
            // Find the matching usage in the source declaration element.
            for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
                if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == destUsage)
                    break;

            UByte* pdestPtr = ((UByte*)pdestIndex) + pdestElement->Offset;

            if (psourceExtra)
            {
                for (unsigned j = 0; j < indexCount; j++)
                {
                    UByte* psourcePtr = ((UByte*)psourceExtra) + *((UInt16*)((UByte*)psourceIndex + j * indexStride));

                    *pdestPtr = *psourcePtr;
                    pdestPtr += pwiivf->IndexSize;
                }
                indexOffset++;
                continue;
            }
            else if (indexStride != 2)
            {
                for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
                    if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | VET_Index1))
                        break;

                SF_ASSERT((psourceElement->Attribute & VET_CompType_Mask) == VET_I8);

                UByte* psourcePtr = ((UByte*)psourceIndex) + psourceElement->Offset;
                for (unsigned j = 0; j < indexCount; j++)
                {
                    UByte factors = psourcePtr[j*indexStride];
                    *pdestPtr = factors;
                    pdestPtr += pwiivf->IndexSize;
                }
                indexOffset++;
                continue;
            }
            else if (pdestElement->Attribute == VET_Color | VET_Index1 | VET_I8 | 2)
            {
                int factor1offset = -1, factor2offset = -1, srcStride = sourceFormat.Size;
                UByte* psourcePtr1 = (UByte*)psource;

                for (psourceElement = sourceFormat.pElements; psourceElement->Attribute != VET_None; psourceElement++)
                    if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | VET_Index1))
                        factor1offset = psourceElement->Offset;
                    else if ((psourceElement->Attribute & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | VET_Index2))
                        factor2offset = psourceElement->Offset;

                SF_ASSERT(factor1offset >= 0);
                SF_ASSERT(factor2offset >= 0);

                for (unsigned j = 0; j < indexCount; j++)
                {
                    UByte* psourcePtr = psourcePtr1 + *((UInt16*)((UByte*)psourceIndex + j * indexStride)) * srcStride;

                    UByte f1 = psourcePtr[factor1offset], f2 = psourcePtr[factor2offset];
                    UByte factors = (psourcePtr[factor2offset] >> 7) | ((psourcePtr[factor1offset] >> 6) & 0x2);
                    *pdestPtr = factors;
                    pdestPtr += pwiivf->IndexSize;
                }
                indexOffset++;
                continue;
            }
            else
            {
                for (unsigned j = 0; j < indexCount; j++)
                {
                    UByte* psourcePtr = ((UByte*)psource) + *((UInt16*)((UByte*)psourceIndex + j * indexStride)) * sourceFormat.Size;

                    UByte factors = (psourcePtr[psourceElement->Offset] >> 7);
                    *pdestPtr = factors;
                    pdestPtr += pwiivf->IndexSize;
                }
                indexOffset++;
                continue;
            }
        }

        // Indices
        UByte* pdestPtr = (UByte*)pdestIndex;
        for (unsigned j = 0; j < indexCount; j++, pdestPtr += pwiivf->IndexSize)
        {
            IndexType index = *((UInt16*)((UByte*)psourceIndex + j * indexStride));
            *((UInt16*) (pdestPtr+indexOffset)) = index + indexAdj;
        }

        indexOffset += 2;
    }
}

static void ConvertMesh(const VertexFormat& sourceFormat, const void* psource, const void* psourceIndex,
                        const VertexFormat& destFormat, void* pdest, void* pdestIndex, int indexAdj,
                        unsigned count, unsigned indexCount, unsigned indexStride, int BatchIndex)
{
    ConvertMeshVertices(sourceFormat, psource, destFormat, pdest, 0, count);
    ConvertMeshIndices(sourceFormat, psource, psourceIndex, 0, destFormat, pdestIndex, indexAdj, indexCount, indexStride, BatchIndex);
}

// Generates meshes and uploads them to buffers.
// Returns 'false' if there is not enough space in Cache, so Unlock and
// flush needs to take place.
//   - Pass 'false' for firstCall is PreparePrimitive was already called once
//     with this data and failed.
bool MeshCache::PreparePrimitive(PrimitiveBatch* pbatch,
                                 MeshCacheItem::MeshContent &mc,
                                 bool waitForCache)
{
    SF_AMP_SCOPE_RENDER_TIMER(__FUNCTION__, Amp_Profile_Level_Medium);

    Primitive* prim = pbatch->GetPrimitive();

    if (mc.IsLargeMesh())
    {
        SF_ASSERT(mc.GetMeshCount() == 1);
        MeshResult mr = GenerateMesh(mc[0], prim->GetVertexFormat(),
                                     pbatch->pFormat, 0, waitForCache);

        if (mr.Succeded())
            pbatch->SetCacheItem(mc[0]->CacheItems[0]);
        // Return 'false' if we just need more cache, to flush and retry.
        if (mr == MeshResult::Fail_LargeMesh_NeedCache)
            return false;
        return true;
    }

    // NOTE: We always know that meshes in one batch fit into Mesh Staging Cache.
    unsigned totalVertexCount, totalIndexCount;
    pbatch->CalcMeshSizes(&totalVertexCount, &totalIndexCount);

    const VertexFormat* pbatchvf = pbatch->pFormat;
    const SysVertexFormat* pwiivf = (const SysVertexFormat*)pbatchvf->pSysFormat.GetPtr();

    bool      usesTexMatrix    = 1;

    for (const VertexElement* pdestElement = pbatchvf->pElements; pdestElement->Attribute != VET_None; pdestElement++)
        if ((pdestElement->Attribute & VET_Usage_Mask) == VET_TexCoord)
        {
            usesTexMatrix = 0;
            break;
        }

    // First, try to allocate the buffer space while swapping out any stale data.
    // Note that AllocBuffer takes place before mesh-gen to ensure that packing code
    // does not rely on MeshCacheItem items that may get evicted during allocation.

    UPInt   dlistSize = 256 * mc.GetMeshCount() + pwiivf->IndexSize * totalIndexCount;
    UPInt   vertexByteSize = totalVertexCount * pbatch->pFormat->Size;

    dlistSize = (dlistSize + 31) & ~31;

    UPInt  allocSize = pbatchvf->Size * totalVertexCount + dlistSize;
    allocSize = (allocSize + 31) & ~31;

    UPInt   allocAddress;

    if (!allocBuffer(&allocAddress, allocSize, waitForCache))
        return false;

    MeshCacheItem* batchData = 0;

    MeshBuffer* pbuffer  = (MeshBuffer*)findBuffer(allocAddress);
    UPInt       dlOffset = allocAddress - (UPInt)pbuffer->pData;
    UPInt       vbOffset = dlOffset + dlistSize;

    batchData = MeshCacheItem::Create(MeshCacheItem::Mesh_Regular, &CacheList,
                                      mc, pbuffer, allocAddress, allocSize, vbOffset, totalVertexCount, totalIndexCount,
                                      dlOffset, dlistSize, pbatch->pFormat);

    if (!batchData)
    {
        return false;
    }

    pbatch->SetCacheItem(batchData);

    // Prepare and Pin mesh data with the StagingBuffer.
    StagingBufferPrep   meshPrep(this, mc, pbatch->GetPrimitive()->GetVertexFormat(), true, batchData);

    // Copy meshes into the Vertex/Index buffers.
    // All the meshes have been pinned, so we can
    // go through them and copy them into buffers.

    UByte*    pdlistStart      = (UByte*)allocAddress;
    UByte*    pvertexDataStart = pdlistStart + dlistSize;
    UByte*    pdlist           = pdlistStart;

    UByte*    pstagingBuffer  = StagingBuffer.GetBuffer();

    bool      isBatched = (mc.GetMeshCount() > 1);
    UByte     vfn = pwiivf->vfn >= 0 ? pwiivf->vfn : 7;

    pdlist = DLSetVertexDesc(pdlist, pbatchvf);
    if (!isBatched)
        pdlist = DLSetCurrentMtx(pdlist, 0, usesTexMatrix);
    pdlist = DLSetArrays(pdlist, pbatchvf, pvertexDataStart);

    *pdlist++ = GX_DRAW_TRIANGLES | vfn;
    *((UInt16*)pdlist)++ = totalIndexCount;

    batchData->PrimOffset = pdlist - pdlistStart;

    unsigned  i;
    unsigned  indexStart = 0;

    for(i = 0; i< mc.GetMeshCount(); i++)
    {
        Mesh* pmesh = mc[i];        
        //SF_ASSERT(pmesh->Data.GetVertexFormat() == pvf);

        // Convert vertices and initialize them to the running index
        // within this primitive.
        UByte   instance = (UByte)i;
        void*   convertArgArray[1] = { &instance };

        // GDebug::Message(GDebug::Message_Note, "[i = %d] Mesh %p starts at Vertex %d", i, pmesh, indexStart);

        // If mesh is in staging buffer, initialize it from there.
        if (pmesh->StagingBufferSize != 0)
        {
            ConvertMesh(*prim->GetVertexFormat(), pstagingBuffer+pmesh->StagingBufferOffset, (IndexType*)(pstagingBuffer+pmesh->StagingBufferIndexOffset),
                        *pbatch->pFormat, pvertexDataStart, pdlist, indexStart,
                        pmesh->VertexCount, pmesh->IndexCount, 2, isBatched ? instance*3 : -1);

            pdlist += pmesh->IndexCount * pwiivf->IndexSize;
        }
        else
        {
            // If mesh is not in a staging buffer, find the batch that has our mesh and location in it,
            // so vertex and index data can be copied.
            unsigned        prevVertexCount, prevIndexCount;
            MeshCacheItem*  psourceBatchMesh = (MeshCacheItem*)
                MeshCacheItem::FindMeshSourceBatch(pmesh, &prevVertexCount, &prevIndexCount,
                                                   pbatch->GetCacheItem());

            //     GDebug::Message(GDebug::Message_Note, "[i = %d of %d] Reusing mesh %p in BatchMesh %p from BatchMesh %p index %d",
            //                     i, meshContent.MeshCount, pmesh, pbatch->GetBatchMesh(), psourceBatchMesh, imesh);


            // Now, copy the old mesh data into the new location.
            const VertexFormat * psrcFmt = psourceBatchMesh->GetVertexFormat();
            const SysVertexFormat * psrcWiiFmt = (const SysVertexFormat*)psrcFmt->pSysFormat.GetPtr();

            UByte* psourceBufferStart = (UByte*)(psourceBatchMesh->GetBuffer())->pData;
            UByte* pvertexSource      = psourceBufferStart + psourceBatchMesh->GetVertexOffset() +
                                        prevVertexCount * psrcFmt->Size;
            UByte* pindexSource       = psourceBufferStart + psourceBatchMesh->DListOffset + psourceBatchMesh->PrimOffset +
                                        psrcWiiFmt->IndexSize * prevIndexCount + psrcWiiFmt->NumMatrices;

            {
                ConvertMesh(*psrcFmt, pvertexSource, pindexSource,
                            *pbatch->pFormat, pvertexDataStart, pdlist, indexStart-prevVertexCount,
                            pmesh->VertexCount, pmesh->IndexCount, psrcWiiFmt->IndexSize, isBatched ? instance*3 : -1);

                pdlist += pmesh->IndexCount * pwiivf->IndexSize;
            }
        }

        DCStoreRange(pvertexDataStart, pmesh->VertexCount * pbatchvf->Size);

        pvertexDataStart += pmesh->VertexCount * pbatchvf->Size;
        indexStart       += pmesh->VertexCount;
    }   

    unsigned dlSize = pdlist - pdlistStart;
    if (dlSize & 31)
    {
        dlSize = (dlSize + 31) & ~31;
        memset(pdlist, 0, dlSize - (pdlist - pdlistStart));
    }
    batchData->DListSize = dlSize;
    DCStoreRange(pdlistStart, dlSize);
    SF_ASSERT(pdlist < pdlistStart + dlistSize);

    // We invalidate GPU cache after we've written data to it, so that stale content
    // does not accidentally get used when rendering.
    GXInvalidateVtxCache();

    // ~StagingBufferPrep will Unpin meshes in the staging buffer.             
    return true;
}

bool    MeshCache::SetLargeMeshVertices(Render::MeshCacheItem* pcacheItem,
                                        const VertexFormat* pSourceFormat, UPInt vertexOffset, UByte* pvertices, unsigned vertexCount,
                                        const VertexFormat* pDestFormat, UByte* pdestStart)
{
    const SysVertexFormat* pwiivf = ((const SysVertexFormat*)pDestFormat->pSysFormat.GetPtr());
    MeshCacheItem*         pmesh = (MeshCacheItem*)pcacheItem;
    UByte*                 pdest = pdestStart + vertexOffset * pDestFormat->Size;

    SF_ASSERT(pdestStart == (UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset);

    if (!pcacheItem->pMeshes[0]->StagingBufferSize && pwiivf->ExtraSize)
        StagingBuffer.AllocateMesh((Mesh*)pmesh->pMeshes[0], pmesh->VertexCount, pwiivf->ExtraSize, 0);

    ConvertMeshVertices(*pSourceFormat, pvertices, *pDestFormat, pdest,
                        StagingBuffer.GetBuffer()+pmesh->pMeshes[0]->StagingBufferOffset + vertexOffset * pwiivf->ExtraSize, vertexCount);
    return true;
}

bool    MeshCache::SetLargeMeshIndices(Render::MeshCacheItem* pcacheItem,
                                       const VertexFormat* pSourceFormat, UPInt indexOffset, const UInt16* pindices, unsigned indexCount,
                                       const VertexFormat* pDestFormat, UByte* pdestDList)
{
    MeshCacheItem*         pmesh = (MeshCacheItem*)pcacheItem;
    UByte*                 pvertexDataStart = (UByte*)pmesh->pBuffer->pData + pmesh->VertexOffset;
    const SysVertexFormat* pwiivf = ((const SysVertexFormat*)pDestFormat->pSysFormat.GetPtr());

    if (indexOffset == 0)
    {
        UByte*    pdlistStart      = pdestDList;
        UByte*    pdlist           = pdlistStart;
        bool      usesTexMatrix    = 1;

        SF_ASSERT(pdestDList == (UByte*)pmesh->pBuffer->pData + pmesh->DListOffset);

        for (const VertexElement* pdestElement = pDestFormat->pElements; pdestElement->Attribute != VET_None; pdestElement++)
            if ((pdestElement->Attribute & VET_Usage_Mask) == VET_TexCoord)
            {
                usesTexMatrix = 0;
                break;
            }

        UByte     vfn = pwiivf->vfn >= 0 ? pwiivf->vfn : 7;

        pdlist = DLSetVertexDesc(pdlist, pDestFormat);
        pdlist = DLSetCurrentMtx(pdlist, 0, usesTexMatrix);
        pdlist = DLSetArrays(pdlist, pDestFormat, pvertexDataStart);

        *pdlist++ = GX_DRAW_TRIANGLES | vfn;
        *((UInt16*)pdlist)++ = 0;

        pmesh->PrimOffset = pdlist - pdlistStart;
    }

    UByte*   pdestIndex = (UByte*)pmesh->pBuffer->pData + pmesh->DListOffset + pmesh->PrimOffset;
    ConvertMeshIndices(*pSourceFormat, pvertexDataStart, pindices, StagingBuffer.GetBuffer()+pmesh->pMeshes[0]->StagingBufferOffset,
                       *pDestFormat, pdestIndex + pwiivf->IndexSize * indexOffset, 0, indexCount, 2, -1);

    ((UInt16*)((UByte*)pmesh->pBuffer->pData + pmesh->DListOffset + pmesh->PrimOffset))[-1] += indexCount;
    return true;
}

// **** RenderSync_DrawSync

bool RenderSync_DrawSync::Init = 0;
unsigned RenderSync_DrawSync::Count = 0;
UInt32 RenderSync_DrawSync::WaitOn = 0;
UInt32 RenderSync_DrawSync::LastV, RenderSync_DrawSync::LastW;
OSThreadQueue RenderSync_DrawSync::WaitQ;

}}}; // namespace Scaleform::Render::Wii

