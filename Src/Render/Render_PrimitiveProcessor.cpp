/**************************************************************************

Filename    :   Render_PrimitiveProcessor.cpp
Content     :   Render queue processing for Primitive items and batches
Created     :   April 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_PrimitiveProcessor.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// ***** Primitive Queue handling implementation

Primitive::QIPrepareResult
Primitive::Prepare(RenderQueueItem& item, RenderQueueProcessor& qp, bool waitForCache)
{
    if ( qp.GetQueuePrepareFilter() != RenderQueueProcessor::QPF_All )
        return RenderQueueItem::QIP_Done;
    return prepare(&item, (PrimitivePrepareBuffer*)qp.GetPrepareItemBuffer(),
                         (PrimitiveEmitBuffer*)qp.GetEmitItemBuffer(),
                   qp.GetHAL(), qp.GetMeshCache(), waitForCache);
}

void Primitive::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    if ( qp.GetQueueEmitFilter() != RenderQueueProcessor::QPF_All )
        return;

    PrimitivePrepareBuffer* prepareBuffer = (PrimitivePrepareBuffer*)qp.GetPrepareItemBuffer();
    PrimitiveEmitBuffer*    emitBuffer    = (PrimitiveEmitBuffer*)qp.GetEmitItemBuffer();

    emitToHAL(&item, prepareBuffer, emitBuffer, qp.GetHAL());
}


Primitive::QIPrepareResult
Primitive::prepare(void* item,
                   PrimitivePrepareBuffer* prepareBuffer,
                   PrimitiveEmitBuffer* emitBuffer,
                   HAL* hal, MeshCache* cache, bool waitForCache)
{
    if (!prepareBuffer->IsProcessing(item))
        prepareBuffer->StartPrimitive(item, this, emitBuffer, hal, cache);

    return prepareBuffer->ProcessPrimitive(waitForCache);
}

void Primitive::emitToHAL(void* item,
                          PrimitivePrepareBuffer* prepareBuffer, PrimitiveEmitBuffer* emitBuffer,
                          HAL* hal)
{
    PrimitiveBatch* drawStart, *drawEnd;

    if (!emitBuffer->IsEmitting(item))
    {
        drawStart = Batches.GetFirst();
        emitBuffer->StartEmitting(item, drawStart);
    }
    else
    {
        drawStart = emitBuffer->pDraw;
    }

    if (!prepareBuffer->IsProcessing(item))
    {
        drawEnd = Batches.GetLast()->GetNext();
    }
    else
    {
        drawEnd = prepareBuffer->pPrepare;
        // Although we assign pDraw here, pPrepare and pDraw may be adjusted inside
        // of batchConvertStep if items are inserted before it that need to be processed.
        emitBuffer->pDraw = drawEnd;
    }

    UpdateMeshIndicies();
    hal->DrawProcessedPrimitive(this, drawStart, drawEnd);
}



//------------------------------------------------------------------------
// ***** PrimitivePrepareBuffer

void PrimitivePrepareBuffer::StartPrimitive(void* item,
                                            Primitive* p, PrimitiveEmitBuffer* emitBuffer,
                                            HAL* hal, MeshCache* cache)
{
    RQPrepareBuffer::StartProcessing(item);

    pEmitBuffer       = emitBuffer;
    pHal              = hal;
    pCache            = cache;
    pPrimitive        = p;
    MaximumBatchCount = 0;

    
    pSourceVFormat = p->GetVertexFormat();

    if (pSourceVFormat)
    {
        pHal->MapVertexFormat(
            p->GetFill()->GetType(), pSourceVFormat,
            &pSingleVFormat, &pBatchVFormat, &pInstancedVFormat, MeshCacheItem::Mesh_Regular);
        // MA: Oct 21, 2010 - Single and Instanced formats should be the same
        // if we are to make a decision about uploading large meshes early.
        // Needs research if this is possible for all back-ends.
        SF_ASSERT(pInstancedVFormat == NULL || *pSingleVFormat == *pInstancedVFormat);

        MaximumBatchCount = pHal->GetMaximumBatchCount(p);
    }
    else
    {
        SF_ASSERT(p->GetMeshCount() == 0);
        pSingleVFormat = pBatchVFormat = pInstancedVFormat = 0;
    }    

    State      = PS_Loop;
    pConvert   = p->Batches.GetFirst();
    pPrepare   = pPrepareTail = pConvert;
    Converting = false;
}


// ProcessPrimitive() goes through a list of batches in the primitive
// and prepares them for rendering. This is a two-step process:
//
//   1) Convert - All of the virtual primitives are converted to non-virtual ones
//      calling batchConvertBegin/Step functions. 
//   2) Prepare - All non-cached items are brought into cache by calling
//      Cache.PreparePrimitive. Cached batches are also moved to InFlight
//      list to avoid being swapped out before rendering.
//
// An important feature of this function is that it is re-entrant for the same
// primitive. This is necessary to allow cache swapping to take place within
// a primitive, which requires the function to exit (with PR_NeedCache) so that
// the batches processed so far can be Drawn, enabling cache reuse.

PrimitivePrepareBuffer::QIPrepareResult
PrimitivePrepareBuffer::ProcessPrimitive(bool waitForCache)
{  
    List<PrimitiveBatch>& batches = getPrimitive()->Batches;

    while(!batches.IsNull(pPrepare))
    {
        while (pPrepare != pPrepareTail)
        {
            SF_ASSERT(!pPrepare->IsVirtual());

            if (!pPrepare->GetCacheItem())
            {
                MeshCacheItem::MeshContent mc(pPrepare);

                // Check cache on a first call for the primitive (State == PS_Loop).
                if (pPrepare->Type != PrimitiveBatch::DP_Failed)
                {
                    if ((State == PS_Loop) && pCache->GetHashPrimitive(pPrepare, mc))
                    {
                        // Primitive match found.
                    }
                    else if (!pCache->PreparePrimitive(pPrepare, mc, waitForCache))
                    {
                        State = PS_NeedCache;
                        return RenderQueueItem::QIP_NeedCache;
                    }
                }

                State = PS_Loop;

                // (Needed ?) Clear waitForCache once at least one primitive has been
                // processed (it will allow us to return quickly, Draw, and spin).
                waitForCache = false;
            }

            // Move pBatchMesh closer in mesh cache to InFlight slot, to ensure
            // that it doesn't get swapped out.
            pCache->MoveToCacheListFront(MCL_InFlight, pPrepare->GetCacheItem());
            pPrepare = pPrepare->pNext;
        }

        if (!Converting)
        {
            while(!batches.IsNull(pConvert) && !pConvert->IsVirtual())
            {
                // Since PrepareTail is not actually touched by Prepare, this
                // ensures that the previous item before the virtual batch is
                // available for processing.
                pPrepareTail = pConvert;
                pConvert     = pConvert->pNext;
            }

            // If we reached the end, we are done - so finish prepare.
            if (batches.IsNull(pConvert))
            {
                 pPrepareTail = pConvert;
                 continue;
            }
            
            if (!batchConvertBegin())
                continue;
        }

        // Do Conversion Step
        // Side effects:
        //   * 1 - 2 items may be inserted before pConvert
        //   * if (pPrepareTail != pConvert) before the call, previous item may be
        //     removed from the list; pPrepareTail may be advanced.
        //   * pConvert may be removed and advanced.
        //   * pConvert->pNext may be consumed as well.
        //
        batchConvertStep();
    }

    return RenderQueueItem::QIP_Done;
}


bool PrimitivePrepareBuffer::batchConvertBegin()
{
    // Virtual chunk can't have a mesh.
    SF_ASSERT(!pConvert->GetCacheItem());
    pConvert->pPrimitive->UpdateMeshIndicies();
    if (pConvert->MeshCount == 0)
        return false;
    
    Converting = true;
    return true;
}


// batchConvertStep converts the current virtual BatchPrimitive item into
// one or more non-virtual fixed-size batches. During conversion, it considers
// the MeshCache thresholds and limits to determine the size of the batch;
// it also attempts to accumulate consecutive meshes of the same type into
// special Instanced batches.
//
// During processing, this algorithm should try to merge virtual batch data
// with neighbours if possible, so as to avoid Insert/Remove operations from
// fragmenting the batch. Merge conditions:
// 
//  Previous:
//    a) Merge with previous batch if all data fits (Done).
//    b) TBD: Merge with previoused Instanced batch if we start with the same
//            item types and they can fit into it.
//    c) TBD: Merge with previous if we are generating multi-batch data and
//            previous block is under the completeness threshold,
//            (ex., under 2/3 full in all categories) and can hold some data.
//  Next:
//    a) Merge with next batch if last generated batch can fit into it.
//    b) TBD: Merge with next Instanced batch if possible.

void PrimitivePrepareBuffer::batchConvertStep()
{

    // 1. Identify the range of index items for the batch.
    unsigned                  totalVerticesSize = 0,
                              totalIndexCount   = 0;
    Mesh*                     pprevMesh         = 0;
    unsigned                  tailRepeatCount   = 0;
    bool                      largeTailMesh     = false;
    bool                      meshTooBigFail    = false;
    PrimitiveBatch::BatchType tailMeshType      = PrimitiveBatch::DP_Batch;
    const MeshCacheParams&    params = pCache->GetParams();

    // Instancing threshold applied to this iteration to tail items,
    // enabling their separation. It can be switched to 1 to force a separate
    // batch to be generated for a single item.
    unsigned                  instancingThreshold = params.InstancingThreshold;
    unsigned                  batchVertexSize = pBatchVFormat ? pBatchVFormat->Size : 0;
    bool                      instancingSupported = pInstancedVFormat ? true : false;

    Primitive* primitive = pConvert->pPrimitive;
    Primitive::MeshEntry* convertMeshes = &primitive->Meshes[pConvert->MeshIndex];

    unsigned meshIndex;
    unsigned originalConvertMeshCount = pConvert->MeshCount;

    for (meshIndex = 0; meshIndex < originalConvertMeshCount; meshIndex++)
    {
        Mesh *pmesh = convertMeshes[meshIndex].pMesh;

        // Detect long consecutive same-item runs so that we can generate separate
        // instanced batches for them.
        if (pmesh != pprevMesh)
        {
            if (instancingSupported && (tailRepeatCount >= instancingThreshold))
                break;
            tailRepeatCount = 1;
        }
        else
        {
            if (instancingSupported)
            {
                // If we already reached maximum number of items, done. This check
                // is necessary to handle MaxBatchInstances == 1.
                if (tailRepeatCount == MaximumBatchCount)
                    break;
                tailRepeatCount++;
                if (tailRepeatCount >= MaximumBatchCount)
                {
                    meshIndex++;
                    break;
                }
            }
            else
            {
                tailRepeatCount++;
            }
        }

        if (!pmesh->IndexCount)
        {
            // Mesh has not been tessellated. Tessellate it.
            //  - there is a chance that the mesh will end up being large,
            //    in which case it will need to be independent

            MeshCache::MeshResult mr = 
                pCache->GenerateMesh(pmesh, pSourceVFormat,
                                     pSingleVFormat, pBatchVFormat, false);

            if (!mr.Succeded() && (mr != MeshCache::MeshResult::Fail_LargeMesh_NeedCache))
            {
                // This shouldn't happen as long as our
                // (MeshStagingBuffer >= max batch + biggest mesh allowed).
                SF_ASSERT(mr != MeshCache::MeshResult::Fail_Staging_NoBuffer);
                // TBD: Should we record this flag in mesh as well for future reference?
                
                if (mr == MeshCache::MeshResult::Fail_LargeMesh_TooBig)
                    meshTooBigFail = true;
                // Fail_LargeMesh_ThisFrame doesn't set Failed DP type, as we want to
                // retry in the future frames.
                if (mr != MeshCache::MeshResult::Fail_LargeMesh_ThisFrame)
                {
                    tailMeshType = PrimitiveBatch::DP_Failed;
                }
                else
                {
                    SF_ASSERT(pmesh->LargeMesh);
                }
            }
        }

        if ((pmesh->LargeMesh) || (tailMeshType == PrimitiveBatch::DP_Failed) || !pBatchVFormat)
        {
            // This mesh should get an independent chunk.
            // - Emit previous chunk before the batch, then the single item itself.
            // - We change the instancing threshold to allow repeating of this large mesh.
            instancingThreshold = 1;
            largeTailMesh       = true;
            meshIndex++;
            break;
        }

        if (((totalIndexCount + pmesh->IndexCount) > params.MaxIndicesInBatch) ||
            ((totalVerticesSize + pmesh->VertexCount * batchVertexSize) > params.MaxVerticesSizeInBatch) ||
            (meshIndex >= MaximumBatchCount))
        {
            // We exceeded batching threshold, break.
            // At least one item should fit into the chunk according to the above criteria,
            // as NoBatchVerticesSizeThreshold should be <= MaxVerticesSizeInBatch.
            SF_ASSERT(meshIndex > 0);

            // If we break out here, the last meshIndex isn't considered, so make
            // and adjustment to avoid tail item being counted twice.
            if (pmesh == pprevMesh)
                tailRepeatCount--;
            break;
        }

        totalIndexCount   += pmesh->IndexCount;
        totalVerticesSize += pmesh->VertexCount * batchVertexSize;
        pprevMesh = pmesh;
    }


    // Try to see if we can extend the tailRepeatCount to run further with instancing.
    SF_ASSERT((meshIndex>0) && (meshIndex <= pConvert->MeshCount));
    Mesh *   pmesh           = convertMeshes[meshIndex-1].pMesh;
    unsigned repeatMeshIndex = meshIndex;

    if (instancingSupported)
    {
        while((repeatMeshIndex < pConvert->MeshCount) &&
            (convertMeshes[repeatMeshIndex].pMesh == pmesh) &&
            (tailRepeatCount < MaximumBatchCount))
        {
            tailRepeatCount++;
            repeatMeshIndex++;
        }
    }
    else
    {
        if (tailRepeatCount > 1)
            tailRepeatCount = 0;
    }

    // Well create a separate batch for tail either if it exceeded instancing threshold,
    // or if the whole batch is built out of the same mesh.
    if ((tailRepeatCount >= instancingThreshold) ||
        ((repeatMeshIndex == tailRepeatCount) && (tailRepeatCount != 1)) )
    {
        meshIndex = repeatMeshIndex;
    }
    else
    {
        tailRepeatCount = 0;
    }


    // 2. Generate batches in the list for this sub-set.

    // At this point we have a piece of a primitive that can generate one or two 
    // physical batches. The range [FirstMeshIndex, meshIndex) contains either:
    //  [Batch] 
    //  [Batch][repeated instance items]
    //  [Batch][single item]
    //  [repeated instance items]
    //  [single item]    
    unsigned frontMeshCount = meshIndex - tailRepeatCount;

    if (frontMeshCount)
    {
        PrimitiveBatch* presult;

        if ((meshIndex == pConvert->MeshCount) && !tailRepeatCount)
        {
            // If this is the last item, reuse chunk
            pConvert->Type = PrimitiveBatch::DP_Batch;
            SF_ASSERT(frontMeshCount == pConvert->MeshCount);

            // Complete fit, see if we can combine with the previous item.
            if (pPrepareTail != pConvert)
            {
                PrimitiveBatch* pprev = pConvert->GetPrev();
                // Prev must be:
                //   - not instanced, unless it can be un-instanced to form a batch.
                //   - not failed.
                //   - not single item over threshold (large mesh).
                //   - have enough space for us (vertices, indices, batch items)
                attemptMergeBatches(pprev, pConvert,
                                    pprev, pConvert, totalVerticesSize, totalIndexCount);
            }

            // See if we can combine with the next item.
            if (!getPrimitive()->Batches.IsNull(pConvert->GetNext()))
                attemptMergeBatches(pConvert, pConvert->GetNext(),
                                    pConvert->GetNext(), pConvert,
                                    totalVerticesSize, totalIndexCount);
            // NOTE: pConvert changes after successful MergeBatches!

            presult = pConvert;
        }
        else
        {
            // Create a new chunk with this data.
            PrimitiveBatch* pnewChunk = 
                PrimitiveBatch::Create(pConvert->pPrimitive, PrimitiveBatch::DP_Batch,
                                       pConvert->MeshIndex, frontMeshCount);
            presult = pnewChunk;

            // Insert the new chunk before the current one in primitives list.
            pConvert->InsertNodeBefore(pnewChunk);
            pConvert->MeshIndex += frontMeshCount;
            pConvert->MeshCount -= frontMeshCount;

            // if (pDraw == pConvert)
            //     pDraw = pnewChunk;
            patchEmitDrawStartIfEq(pConvert, pnewChunk);
            if (pPrepare == pConvert)
                pPrepare = pnewChunk;
            // Checking pPrepareTail is not necessary because we assign it below.
        }

        if (presult->MeshCount == 1)
        {
            presult->Type    = PrimitiveBatch::DP_Single;
            presult->pFormat = pSingleVFormat;
        }
        else
        {
            SF_ASSERT(pBatchVFormat != 0);
            presult->pFormat = pBatchVFormat;
        }

        pPrepareTail = pConvert;
    }

    if (tailRepeatCount)
    {
        // Create a new chunk with this data.
        if ((tailRepeatCount > 1) && (tailMeshType != PrimitiveBatch::DP_Failed))
            tailMeshType = PrimitiveBatch::DP_Instanced;
        else if (!meshTooBigFail)
            tailMeshType = PrimitiveBatch::DP_Single;

        // TBD: pConvert->MeshIndex -> could be invalid ??
        PrimitiveBatch* pnewChunk = 
            PrimitiveBatch::Create(pConvert->pPrimitive, tailMeshType,
                                   pConvert->MeshIndex, tailRepeatCount);
        pnewChunk->LargeMesh = largeTailMesh;

        if (tailMeshType == PrimitiveBatch::DP_Instanced)
            pnewChunk->pFormat = pInstancedVFormat;
        else if ((tailMeshType == PrimitiveBatch::DP_Single) ||
                 (tailMeshType == PrimitiveBatch::DP_Failed))
            pnewChunk->pFormat = pSingleVFormat;
        
        pConvert->InsertNodeBefore(pnewChunk);
        pConvert->MeshIndex += tailRepeatCount;
        pConvert->MeshCount -= tailRepeatCount;

        //if (pDraw == pConvert)
        //    pDraw = pnewChunk;
        patchEmitDrawStartIfEq(pConvert, pnewChunk);
        if (pPrepare == pConvert)
            pPrepare = pnewChunk;
        // Checking pPrepareTail is not necessary because we assign it below.

        // If the batch ended here, free it (TBD: we could've also reused it).
        if (pConvert->MeshCount == 0)
        {
            pConvert->RemoveAndFree();
            pConvert = pnewChunk;
        }
        pPrepareTail = pnewChunk;
    }

    // Advance to next mesh, which will be processed in future steps.
    // If we reached the end, conversion is finished.
    if (meshIndex >= originalConvertMeshCount)
        Converting = false;

    primitive->VerifyMeshIndices();
}


// Helper function to merge batches around pConvert (next or previous), used in batchConvertStep.
bool PrimitivePrepareBuffer::attemptMergeBatches(
            PrimitiveBatch *pfirst, PrimitiveBatch *psecond,
            PrimitiveBatch *pother, PrimitiveBatch *pknown,
            unsigned &knownVerticesSize, unsigned &knownIndexCount)
{
    if (pother->LargeMesh || !pBatchVFormat ||
        ((pother->Type != PrimitiveBatch::DP_Batch) && (pother->Type != PrimitiveBatch::DP_Single)))
        return false;

    const MeshCacheParams& params = pCache->GetParams();
    unsigned totalMeshCount = pfirst->MeshCount + psecond->MeshCount;   
    if (totalMeshCount > MaximumBatchCount)
        return false;
        
    // Compute mesh space.
    unsigned otherVertexCount, otherIndexCount;
    pother->CalcMeshSizes(&otherVertexCount, &otherIndexCount);

    if (((knownVerticesSize + otherVertexCount * pBatchVFormat->Size) > params.MaxVerticesSizeInBatch) ||
        ((knownIndexCount + otherIndexCount) > params.MaxIndicesInBatch))
        return false;
    
    // Combine with the other item.
    pknown->MeshCount = totalMeshCount;
    pknown->MeshIndex = pfirst->MeshIndex;  // First item must have smaller MeshIndex.
    pknown->Type = PrimitiveBatch::DP_Batch;
    pknown->ClearCacheItem();
    
    pother->RemoveAndFree();

    // Check these pointers since they might've reached old pPrepareTail.
    //if (pDraw == pfirst)
    //    pDraw = pknown;
    patchEmitDrawStartIfEq(pfirst, pknown);
    if (pPrepare == pfirst)
        pPrepare = pknown;
    pConvert = pknown;

    // Add in case Next combine takes place after this.
    knownVerticesSize += otherVertexCount * pBatchVFormat->Size;
    knownIndexCount   += otherIndexCount;

    // Force conversion to be done, since FirstMeshIndex check in the bottom
    // will now be incorrect.
    Converting = false;
    return false;
}

void PrimitivePrepareBuffer::patchEmitDrawStartIfEq(PrimitiveBatch *check, PrimitiveBatch *drawStart)
{
    if (pEmitBuffer->IsEmitting(GetItem()))
    {
        if (pEmitBuffer->pDraw == check)
            pEmitBuffer->pDraw = drawStart;
    }
}


}} // Scaleform::Render

