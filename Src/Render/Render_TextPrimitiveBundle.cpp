/**************************************************************************

Filename    :   Render_TextPrimitiveBundle.cpp
Content     :   TextPrimitiveBundle classes, SortKey and sorting support
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TextPrimitiveBundle.h"
#include "Render_TreeCacheText.h"
#include "Render_PrimitiveProcessor.h"
#include "Renderer2DImpl.h" // for GetHAL() inline
#include "Render_Queue.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


/* ***** Text Processing

Goals:
  1) Allow rendering of previously drawn in-cache items with minimal cost.
  2) Batch different TextFields together when possible, one layer at a time.
  3) Allow rendering more text (across different TextFields) that fits
     into FontCache (currently require one TextField to fit).

Assumptions:
  a) TextMeshProvider objects (associated with GlyphCache) can be Pinned or Unpinned.
     TextMeshProvider objects can't be evicted while Pinned, InFight or haven't past Fence
     based on mesh status.
  b) TextMeshProvider objects will notify us when evicted, in which case
     they are removed from Primitives and layers.

Algorithm used for Prepare/Emit:

  1) For all TextFields that have non-evicted batch that is a part of our
     primitive, mark batches as Pinned and add them to FontCache's processing list.
  2) Make a list (TFL) of all TextFields without batches.
  3) For all Primitives and their batches, shuffle any Virtual batches to the end of the
     list and combine them into one Virtual item. Index arrays get adjusted appropriately.
  4) For text fields in list (TFL), try to create a batch and pin each TextField.
      - If succeeded, remove item from list and add it to Primitives in batch end.
      - If failed, stop processing list.
  5) For all primitive layers and primitives, prepare PrimitiveBatch objects,
     and Emit them to HAL.
       - If mesh prepare didn't work for some batch, return NeedCache, Emit,
         and repeat similar to how we do this for regular batches.
       - NeedCache / Emit pair is also necessary if (TFL) list is not empty.
  6) If (TFL) list is not empty after rendering, clear all primitives and layers,
     and proceed to step 4, ensuring that all text fields are drawn.

*/


// ***** Text Prepare/Emit Buffers

class TextEmitBuffer;

class TextPrepareBuffer : public RQPrepareBuffer
{
    friend class TextEmitBuffer;
    friend class TextPrimitiveBundle;
public:
    typedef RenderQueueItem::QIPrepareResult QIPrepareResult;
    
    // StartPrimitive initializes PrimitivePrepareBuffer for the primitive.
    // After that is done, ProcessPrimitive is called one or more times
    // to prepare text batches for rendering.
    void            StartPrimitive(RenderQueueItem& item,
                                   TextPrimitiveBundle* bundle, 
                                   TreeCacheText* remainingTextFields,
                                   TextEmitBuffer* emitBuffer,
                                   HAL* hal);

    QIPrepareResult ProcessPrimitive(bool waitForCache);
    
private:
    // No constructor.
    TextPrepareBuffer()
    {
        SF_COMPILER_ASSERT(sizeof(TextPrepareBuffer) <= RQPrepareBuffer::BufferSize);
    }

    // Adds remaining TFs to layers if possible.
    void            addTextFieldLayers(bool startFlag);

    TextEmitBuffer* pEmitBuffer;
    HAL*            pHal;
    MeshCache*      pCache;

    // Current processing state, used for re-entrant processing.
    TextPrimitiveBundle* pBundle;    
    UPInt           ProcessingLayer;
    bool            LayersPinned;
    bool            LayersFinished;
    TreeCacheText*  pRemainingTextFields;
    // PPBuffer stores processing state for the Primitive at ProcessingLayer.
    PrimitivePrepareBuffer PPBuffer;
 };


class TextEmitBuffer : public RQEmitBuffer
{
    friend class Primitive;
    friend class TextPrepareBuffer;
public:

    enum MaskStatusType
    {
        Mask_NotInUse,
        Mask_Submitting,
        Mask_InUse
    };

    void    StartEmitting(void* item, TextPrimitiveBundle* bundle)
    {
        RQEmitBuffer::StartEmitting(item);
        pBundle = bundle;
        EmitLayer = 0;
        MaskStatus = Mask_NotInUse;
        PEBuffer.ClearItem();
    }
    void    EndEmitting()
    {
        RQEmitBuffer::ClearItem();
    }

    void    EmitPrimitive(TextPrepareBuffer* prepareBuffer,
                          HAL* hal);

private:
    // No constructors
    TextEmitBuffer()
    { SF_COMPILER_ASSERT(sizeof(TextEmitBuffer) <= RQEmitBuffer::BufferSize); }

    // EmittingLayer is the first layer we need to render.
    // PrimitiveEmitBuffer tracks Primitive rendering state in it.
    TextPrimitiveBundle* pBundle;
    UPInt               EmitLayer;
    MaskStatusType      MaskStatus;
    PrimitiveEmitBuffer PEBuffer;
};


//------------------------------------------------------------------------

void TextPrepareBuffer::StartPrimitive(RenderQueueItem& item,
                                       TextPrimitiveBundle* bundle,
                                       TreeCacheText* remainingTextFields,
                                       TextEmitBuffer* emitBuffer,
                                       HAL* hal)
{
    RQPrepareBuffer::StartProcessing(&item);

    pEmitBuffer = emitBuffer;
    pHal        = hal;
    pCache      = &hal->GetMeshCache();

    pBundle = bundle;
    pRemainingTextFields = remainingTextFields;
    ProcessingLayer = 0;
    LayersFinished = false;
    LayersPinned = false;
    PPBuffer.ClearItem();

    // 4. If we couldn't create glyph batch wrapper, done.
    addTextFieldLayers(true);
}


void TextPrepareBuffer::addTextFieldLayers(bool startFlag)
{
    TreeCacheText* textFieldsList = pRemainingTextFields;

    // Adds remaining TFs to layers if possible.
    while(pRemainingTextFields)
    {
        TreeCacheText* next = pRemainingTextFields->pNextNoBatch;

        // Make sure that CreateGlyphBatch doesn't swap out our
        // existing batches.
        if (!LayersPinned)
        {
            pBundle->pinLayerBatches();
            LayersPinned = true;
        }

        TextMeshProvider* tm = pRemainingTextFields->GetMeshProvider();
        if (!tm) tm = pRemainingTextFields->CreateMeshProvider();
        if (!tm) break;
        tm->AddToInUseList();

        if (pBundle->addAndPinBatchLayers(pRemainingTextFields, tm))
        {   // Mark as 'Added'.
            tm->SetBundle(pBundle, &pRemainingTextFields->SorterShapeNode);
        }
        
        pRemainingTextFields->pNextNoBatch = 0;
        pRemainingTextFields = next;
    }

    if (!startFlag && (textFieldsList == pRemainingTextFields))
    {
        // Clear pRemainingTextFields to avoid infinite loop if we still
        // can't prepare text.
        SF_DEBUG_WARNING(1,
            "Render::TextPrepareBuffer failed - TextField glyphs don't fit in cache");
        pRemainingTextFields = 0;
    }
}

TextPrepareBuffer::QIPrepareResult
TextPrepareBuffer::ProcessPrimitive(bool waitForCache)
{
    if (LayersFinished)
    {
        SF_ASSERT(pRemainingTextFields);
        SF_ASSERT(!LayersPinned);
        PPBuffer.StartProcessing(0);
        // Clear list and add these TextFields.
        pBundle->clearBatchLayers();
        addTextFieldLayers(false); // TBD: pass waitForCache?
        LayersFinished = false;
        ProcessingLayer = 0;
    }

    // Prepare all layers. This loop is reentrant in case we have to
    // return QIP_NeedCache and start over later.
    while (ProcessingLayer < pBundle->Layers.GetSize())
    {
        TextLayerPrimitive* p = pBundle->Layers[ProcessingLayer];
        QIPrepareResult prepareResult =
            p->prepare(p, &PPBuffer, &pEmitBuffer->PEBuffer, pHal, pCache, waitForCache);
        if (prepareResult == RenderQueueItem::QIP_NeedCache)
            return RenderQueueItem::QIP_NeedCache;
        ProcessingLayer++;
    }
    
    // Once processing of this batch group is finishes, it can be unpinned.
    if (LayersPinned)
    {
        pBundle->unpinLayerBatches();
        LayersPinned = false;
    }

    // First time, flush remaining layers and finish their rendering by
    // returning QIP_NeedCache, which will cause Emit top be called.
    if (pRemainingTextFields)
    {       
        LayersFinished = true;
        return RenderQueueItem::QIP_NeedCache;
    }
   
    return RenderQueueItem::QIP_Done;
}


void TextEmitBuffer::EmitPrimitive(TextPrepareBuffer* prepareBuffer,
                                   HAL* hal)
{
    // Emit from the last emitted location and up to the last
    // processed location. This code is reentrant for the same layer.
    UPInt endLayer;
    bool  layerProcessingFinished = true;

    if (!prepareBuffer->IsProcessing(GetItem()))
    {
        endLayer = pBundle->Layers.GetSize();
    }
    else
    {
        endLayer = prepareBuffer->ProcessingLayer;
        if (endLayer < pBundle->Layers.GetSize())
        {
            endLayer++;
            layerProcessingFinished = false;
        }
    }

    for(;EmitLayer < endLayer; EmitLayer++)
    {
        TextLayerPrimitive* p = pBundle->Layers[EmitLayer];

        // Apply/change mask state as necessary.
        if (p->Type >= TextLayer_Mask)
        {
            if (p->Type == TextLayer_Mask)
            {
                SF_ASSERT(pBundle->pMaskPrimitive);
                if ((MaskStatus == Mask_NotInUse) && pBundle->pMaskPrimitive)
                {
                    hal->PushMask_BeginSubmit(pBundle->pMaskPrimitive);
                    MaskStatus = Mask_Submitting;
                }
            }
            else if (MaskStatus == Mask_Submitting)
            {
                hal->EndMaskSubmit();
                MaskStatus = Mask_InUse;
            }
        }
        
        p->emitToHAL(p, &prepareBuffer->PPBuffer, &PEBuffer, hal);
    }

    // If we reach the end, mark as non-emitting.
    // This is helpful in case Process has remaining TextFields.
    if (layerProcessingFinished)
    {
        if (MaskStatus != Mask_NotInUse)
        {
            SF_ASSERT(MaskStatus != Mask_Submitting);
            hal->PopMask();
            MaskStatus = Mask_NotInUse;
        }
        EndEmitting();
    }
    else
    {
        // Make sure that the last layer is re-visited if its processing
        // wasn't finished originally.
        EmitLayer--;
    }
}


//--------------------------------------------------------------------
// ***** TextLayerPrimitive

bool TextLayerPrimitive::AddEntry(BundleEntry* e, Mesh* pmesh, const HMatrix& m)
{    
    if (Insert(GetMeshCount(), pmesh, m))
    {
        Entries.PushBack(e);
        return true;
    }
    return false;
}

bool TextLayerPrimitive::RemoveEntry(BundleEntry* e)
{
    UPInt index = 0;
    for (;index < Entries.GetSize(); index++)
    {
        // It is possible to have several copies of the same entry if the same
        // image is used multiple times in one textfield. Remove them all.
        if (Entries[index] == e)
        {
            Entries.RemoveAt(index);
            Remove((unsigned)index, 1);
            index--;
        }
    }
    return false;
}

void TextLayerPrimitive::ShuffleAndMergeVirtualBatches()
{
    // 1. Move any "finalized" non-virtual batches to front.
    // 2. Group any Virtual batches formed in the end into one.
    //     -- TBD: Won't primitive processing do this step anyway?
    //

}
void TextLayerPrimitive::PinEntryBatches()
{    
    for (UPInt index = 0; index < Entries.GetSize(); index++)
    {        
        TreeCacheText* text = (TreeCacheText*)Entries[index]->pSourceNode;
        TextMeshProvider* tm = text->GetMeshProvider();
        SF_ASSERT(tm);
        tm->Pin();
    }
}
void TextLayerPrimitive::UnpinEntryBatches()
{
    for (UPInt index = 0; index < Entries.GetSize(); index++)
    {        
        TreeCacheText* text = (TreeCacheText*)Entries[index]->pSourceNode;
        TextMeshProvider* tm = text->GetMeshProvider();
        SF_ASSERT(tm);
        tm->Unpin();
    }
}



//--------------------------------------------------------------------
// ***** Text Primitive Bundle

TextPrimitiveBundle::TextPrimitiveBundle() : Bundle(0)
{
}
TextPrimitiveBundle::~TextPrimitiveBundle()
{
}

void TextPrimitiveBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    
    // Nothing else to do; Layers are updated lafter during prepare.
    TreeCacheText* source = (TreeCacheText*)entry->pSourceNode;
    SF_ASSERT(!source->GetMeshProvider() ||
              !source->GetMeshProvider()->GetBundle());
    SF_UNUSED(source);
}

void TextPrimitiveBundle::RemoveEntries(UPInt index, UPInt count)
{
    // Go through Primitive bundles and remove this TF data from batches.
    for (UPInt i = index; i< (index+count); i++)
    {
        TreeCacheText* t = (TreeCacheText*)Entries[i]->pSourceNode;
        TextMeshProvider* tm = t->GetMeshProvider();
        if (tm && tm->GetBundle())
        {
            removeEntryFromLayers(&t->SorterShapeNode);
            tm->ClearBundle();
        }
    }

    Bundle::RemoveEntries(index, count);
}

void TextPrimitiveBundle::UpdateMesh(BundleEntry* entry)
{    
    // For now, just throw away meshes for update.
    UPInt index;
    if (findEntryIndex(&index, entry))
    {
        TreeCacheText*    t = (TreeCacheText*)entry->pSourceNode;
        TextMeshProvider* tm = t->GetMeshProvider();
        if (tm && tm->GetBundle())
        {
            removeEntryFromLayers(&t->SorterShapeNode);
            tm->ClearBundle();
        }
    }
}


RenderQueueItem::QIPrepareResult
TextPrimitiveBundle::Prepare(RenderQueueItem& item,
                             RenderQueueProcessor& qp, bool waitForCache)
{
    if ( qp.GetQueuePrepareFilter() != RenderQueueProcessor::QPF_All )
        return RenderQueueItem::QIP_Done;

    TextPrepareBuffer* prepareBuffer = (TextPrepareBuffer*)qp.GetPrepareItemBuffer();
    if (!prepareBuffer->IsProcessing(&item))
        startPrimitive(item, prepareBuffer, qp);
    return prepareBuffer->ProcessPrimitive(waitForCache);
}

void TextPrimitiveBundle::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    if ( qp.GetQueueEmitFilter() != RenderQueueProcessor::QPF_All )
        return;
    TextPrepareBuffer* prepareBuffer = (TextPrepareBuffer*)qp.GetPrepareItemBuffer();
    TextEmitBuffer*    emitBuffer    = (TextEmitBuffer*)qp.GetEmitItemBuffer();
  
    // Emit from the last emitted location and up to the last
    // processed location. This code is reentrant for the same layer.
    if (!emitBuffer->IsEmitting(&item))
         emitBuffer->StartEmitting(&item, this);
    emitBuffer->EmitPrimitive(prepareBuffer, qp.GetHAL());
}


void TextPrimitiveBundle::startPrimitive(RenderQueueItem& item,
                                         TextPrepareBuffer* prepareBuffer,
                                         RenderQueueProcessor& qp)
{

    // Go through TextFields and layers making sure that:
    //  - All TFs have contributed to the specified layer (i.e. their content is in FontCache
    //    and pinned(?)), or has no layer content.
    //  - That all their meshes are committed to cache.

    // 1 and 2.
    UPInt i;
    TreeCacheText* remainingTextFields = 0;

    for (i = 0; i < Entries.GetSize(); i++)
    {
        TreeCacheText* textNode = (TreeCacheText*)Entries[i]->pSourceNode;
        TextMeshProvider* tm = textNode->GetMeshProvider();
        if (tm && tm->GetBundle() == this)
        {
            tm->AddToInUseList();
        }
        else
        {
            // TextField without batch.
            textNode->pNextNoBatch = remainingTextFields;
            remainingTextFields = textNode;
        }
    }

    // 3. Shuffle virtual batches.
   // for (i = 0; i != Layers.GetSize(); i++)
   //     Layers[i]->ShuffleAndMergeVirtualBatches();

    // TBD: Unpin BatchWrappers?
    prepareBuffer->StartPrimitive(item, this, remainingTextFields,
                                  (TextEmitBuffer*)qp.GetEmitItemBuffer(), qp.GetHAL());
}


bool TextPrimitiveBundle::addAndPinBatchLayers(TreeCacheText* textCache, TextMeshProvider* tm)
{
    // Add TextField to primitive:

    // for each layer, find its proper location.
    //   - If layer not found, allocate primitive and insert.
    //   Add to tail batch and record index of TF + HMatrix.
    // - mark TextMeshProvider as being a part of a primitive.

    BundleEntry* entry = &textCache->SorterShapeNode;
    unsigned tfLayerCount = tm->GetLayerCount();    
    unsigned tfLayerIndex;
    UPInt    layerIndex;

    for (tfLayerIndex = 0, layerIndex = 0; tfLayerIndex < tfLayerCount; tfLayerIndex ++)
    {
        const TextMeshLayer&  layer = tm->GetLayer(tfLayerIndex);
        TextLayerPrimitive*   primitive = 0;

        while(layerIndex < Layers.GetSize())
        {
            TextLayerPrimitive* p = Layers[layerIndex];
            if (p->Type >= layer.Type)
            {
                // Exact match, add to this primitive.
                // This logic allows several TextField layers to merged into one bundle layer,
                // which is done on purpose to allow batching of shape glyphs.
                if ((p->Type == layer.Type) &&
                    (p->GetFill() == layer.pFill.GetPtr()))
                {
                    primitive = p;
                    break;
                }

                if ((p->Type > layer.Type) ||
                    (p->GetFill() > layer.pFill.GetPtr()))
                {   // Found insertion location, keep primitive null.
                    break;
                }
            }
            layerIndex++;
        }

        if (!primitive)
        {
            primitive = SF_HEAP_AUTO_NEW(this)
                TextLayerPrimitive(textCache->GetHAL(), layer.Type, layer.pFill);
            if (!primitive)
                return false; // TBD
            Layers.InsertAt(layerIndex, primitive);
            primitive->Release();
        }

        // Add this TF to primitive
        primitive->AddEntry(entry, layer.pMesh, layer.M);
        tm->Pin();
    }

    if (tm->HasMask())
    {
        // Create mask primitive and add entries to it.
        if (!pMaskPrimitive)
        {            
            pMaskPrimitive = *SF_HEAP_AUTO_NEW(this) MaskPrimitive();
        }
        if (pMaskPrimitive)
        {
            pMaskPrimitive->Insert(pMaskPrimitive->GetMaskCount(),
                                   tm->UpdateMaskClearBounds(textCache->M));
        }
    }

    return true;
}

void TextPrimitiveBundle::removeEntryFromLayers(BundleEntry* e)
{    
    for (UPInt i = 0; i< Layers.GetSize(); i++)
    {
        Layers[i]->RemoveEntry(e);
        if (Layers[i]->GetMeshCount() == 0)
        {
            Layers.RemoveAt(i);
            i--;
        }
    }

    // If we had a mask, remove its entry.
    if (pMaskPrimitive)
    {
        TreeCacheText* textCache =(TreeCacheText*)e->pSourceNode;
        TextMeshProvider* tm = textCache->GetMeshProvider();
        if (tm && tm->HasMask())
        {
            HMatrix        removeMatrix = tm->GetMaskClearBounds();
            UPInt          maskCount = pMaskPrimitive->GetMaskCount();
            const HMatrix* matrices = pMaskPrimitive->GetMaskAreaMatrices();
            
            // An exhaustive search is easier then tracking indices here,
            // especially since many text fields may not have a mask.
            for (UPInt i = 0; i < maskCount; i++)
            {
                if (matrices[i] == removeMatrix)
                {
                    pMaskPrimitive->Remove(i, 1);
                    break;
                }
            }
        }

        if (pMaskPrimitive->GetMaskCount() == 0)
            pMaskPrimitive.Clear();
    }
}

void TextPrimitiveBundle::clearBatchLayers()
{
    // Mark all TextMeshProvider's as not having layer entry.
    for (UPInt i = 0; i< Entries.GetSize(); i++)
    {
        TreeCacheText*     t = (TreeCacheText*)Entries[i]->pSourceNode;
        TextMeshProvider* tm = t->GetMeshProvider();
        if (tm)
        {
            tm->ClearBundle();
        }
    }

    Layers.Clear();
    pMaskPrimitive.Clear();
}


//--------------------------------------------------------------------
// ***** SortKey for TextComplexPrimitive implementation


class SKI_TextPrimitive : public SortKeyInterface
{
public: 
    SKI_TextPrimitive()
        : SortKeyInterface(SortKey_Text, SKF_MatchNoOverlap)
    { }

    static SKI_TextPrimitive Instance;
    static SKI_TextPrimitive Instance3D;

    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl*,
                                   const BundleIterator&)
    {
        if (!p->pBundle)
        {
            Ptr<TextPrimitiveBundle> bundle = 
                *SF_HEAP_AUTO_NEW(tr) TextPrimitiveBundle();
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_TextPrimitive::DrawBundleEntry", Amp_Profile_Level_High);

        if (p->pBundle)
        {
            TextPrimitiveBundle* bundle = (TextPrimitiveBundle*)p->pBundle.GetPtr();
            bundle->Draw(r->GetHAL());
        }
    }
};

SKI_TextPrimitive SKI_TextPrimitive::Instance;
SKI_TextPrimitive SKI_TextPrimitive::Instance3D;

// SortKeyTextType is just a constructor tag, as all TextFields are together
// when not overlapping.
SortKey::SortKey(SortKeyTextType, bool is3D)
{
    if ( !is3D )
        initKey(&SKI_TextPrimitive::Instance, 0);
    else
        initKey(&SKI_TextPrimitive::Instance3D, 0);
}


}} // Scaleform::Render
