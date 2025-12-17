/**************************************************************************

Filename    :   Renderer2DImpl.cpp
Content     :   Renderer2D Implementation
Created     :   September 3, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Renderer2DImpl.h"
#include "Render_GlyphCache.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MemItem.h"

extern unsigned BundlePatternFrameId;

// Set this #define to trace out timing of frames in renderer.
//#define RENDER_TRACE_FRAME_CHANGE_TIMING

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
    bool    Render2D_EntryChanges = false;
    double  Render2D_ChangeStartTime = 0;
    double  Render2D_ChangeEndTime = 0;
    double  Render2D_DrawEndTime = 0;
#endif 

namespace Scaleform { namespace Render {

const UInt8 Factors[4] = {0, 0x7F, 0xFF, 0xFF};

Renderer2DImpl::Renderer2DImpl(HAL* hal)
: Context::RenderNotify(hal->GetThreadCommandQueue()),
  pHal(hal), 
  MeshGen(Memory::GetHeapByAddress(getThis())), 
  StrokeGen(Memory::GetHeapByAddress(getThis())),
  FillManager(hal), MPool(Memory::GetHeapByAddress(getThis()))
{
    hal->AddNotify(this);

    // MeshKeyManager itself is allocated in Global heap because it is used
    // for shutdown thread synchronization. Its internal data does, however,
    // use our correct heap.
    MemoryHeap* rendererHeap = Memory::GetHeapByAddress(getThis());
    pMeshKeyManager = *SF_NEW MeshKeyManager(rendererHeap);
    pGlyphCache = *SF_HEAP_NEW(rendererHeap) GlyphCache(rendererHeap);
    pGlyphCache->Initialize(hal, &FillManager);
}


Renderer2DImpl::~Renderer2DImpl()
{
    ReleaseAllContextData();
    pMeshKeyManager->DestroyAllKeys();
    pHal->RemoveNotify(this);
}


MeshCacheConfig* Renderer2DImpl::GetMeshCacheConfig() const
{
    return &pHal->GetMeshCache();
}

bool Renderer2DImpl::BeginFrame()
{
    SF_AMP_SCOPE_RENDER_TIMER("Renderer2DImpl::BeginFrame", Amp_Profile_Level_Medium);
    pMeshKeyManager->ProcessKillList();
    if (pGlyphCache)
        pGlyphCache->OnBeginFrame();
    return pHal->BeginFrame();
}
void Renderer2DImpl::EndFrame()
{
    SF_AMP_SCOPE_RENDER_TIMER("Renderer2DImpl::EndFrame", Amp_Profile_Level_Medium);
    pHal->EndFrame();
    EndFrameContextNotify();
    if (pGlyphCache)
        pGlyphCache->OnEndFrame();

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
    if (Render2D_EntryChanges)
    {
        Render2D_EntryChanges = false;
        double frameTime = Timer::GetProfileSeconds() - Render2D_ChangeStartTime;
        double changesTime = Render2D_ChangeEndTime - Render2D_ChangeStartTime;
        double drawSubmit = Render2D_DrawEndTime - Render2D_ChangeEndTime;
        printf("Frame Time: %fs (changes %fs, drawSubmit: %fs)\n", frameTime, changesTime, drawSubmit);
    }
#endif
}

void Renderer2DImpl::FinishFrame()
{
    pHal->FinishFrame();
}

void Renderer2DImpl::BeginDisplay(Color backgroundColor, const Viewport& viewport)
{
    pHal->BeginDisplay(backgroundColor, viewport);
    VP = viewport;     
}
void Renderer2DImpl::EndDisplay()
{
    pHal->EndDisplay();
    MPool.MemorySqueeze(MatrixPool::Squeeze_Incremental);        
}    

void  Renderer2DImpl::SetGlyphCacheParam(const GlyphCacheParams& param)
{
    mGlyphCacheParam = param;
    if (pHal->IsInitialized())
    {
        pGlyphCache->Initialize(pHal, &FillManager);
    }
}


void Renderer2DImpl::Draw(TreeRoot *pnode)
{    
    SF_AMP_SCOPE_RENDER_TIMER_ID("Renderer2DImpl::Draw", Amp_Native_Function_Id_Draw);

    // If root node has a cached data structure, use it directly
    TreeCacheRoot*            prootCache = (TreeCacheRoot*)pnode->GetRenderData();
    const TreeRoot::NodeData* rootData   = pnode->GetDisplayData();   
    bool hasViewport                     = rootData->HasViewport();
    const Viewport& vp                   = hasViewport ? rootData->VP : VP;

    if ( hasViewport )
    {
        switch(vp.Flags & Viewport::View_Orientation_Mask)
        {
            default:
            {
                Render::Rect<int> viewRect( vp.Left, vp.Top, vp.Left + vp.Width, vp.Top + vp.Height );
                GetHAL()->SetFullViewRect(viewRect);
                break;
            }
            case Viewport::View_Orientation_L90:
            case Viewport::View_Orientation_R90:
            {
                Render::Rect<int> viewRect( vp.Left, vp.Top, vp.Left + vp.Height, vp.Top + vp.Width );
                GetHAL()->SetFullViewRect(viewRect);
                break;
            }
        }
    }

    RectF   viewCullRect;
    bool    viewValid;
    viewValid = vp.GetCullRectF(&viewCullRect);
  //  viewCullRect.SetRect(80,80, 720, 520);
    if (!vp.IsValid())
        return;

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
    if (!prootCache)
    {
        Render2D_EntryChanges = true;
        Render2D_ChangeStartTime = Timer::GetProfileSeconds();
    }
#endif

    // Create root cache
    if (!prootCache)
    {
        unsigned cnodeFlags = rootData->IsVisible() ? NF_Visible : 0;
        cnodeFlags |= TreeCacheNode::CalcEdgeAAMode(rootData);

        prootCache = SF_HEAP_AUTO_NEW(this) TreeCacheRoot(this, pHal, cnodeFlags, pnode);
        if (!prootCache)
            return;
        pnode->SetRenderData(prootCache);
        RenderRoots.PushBack(prootCache);
        
        prootCache->ViewValid = viewValid;
        prootCache->ViewCullRect = viewCullRect;

        // Proceed to build sub-tree
        prootCache->UpdateChildSubtree(pnode->GetDisplayData(), 1);
        prootCache->AddToUpdate(prootCache, Change_Matrix | Change_CxForm |
                                            Update_Pattern);
        prootCache->ChainUpdatesByDepth();
        prootCache->UpdateTreeData();

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
        Render2D_ChangeEndTime = Timer::GetProfileSeconds();
#endif
    }
    else
    {        
        if ((viewValid != prootCache->ViewValid) ||
            (viewCullRect != prootCache->ViewCullRect) )
        {
            prootCache->ViewValid = viewValid;
            prootCache->ViewCullRect = viewCullRect;
            prootCache->AddToUpdate(prootCache, Change_Matrix);
            prootCache->UpdateTreeData();
        }
    }

    if (viewValid)
        prootCache->Draw();

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
    if (Render2D_EntryChanges)
        Render2D_DrawEndTime = Timer::GetProfileSeconds();
#endif    
}


// Lifetime detection
void    Renderer2DImpl::EntryDestroy(Context::Entry* p)
{
    TreeCacheNode* pcache = (TreeCacheNode*)p->GetRenderData();
    if (pcache)
    {
        delete pcache;
        // Must clear pRenderData, as it is used to check for node validity
        // in RenderContext::FinishRendering.
        p->SetRenderData(0);
    }

}
void    Renderer2DImpl::EntryFlush(Context::Entry* p)
{
    EntryDestroy(p);
}

// Handle changes in display lists
// boolean "forceUpdateImages" param forces cache nodes to be updated (in order to get latest textures,
// for example).
void    Renderer2DImpl::EntryChanges(Context& context, Context::ChangeBuffer& cb, bool forceUpdateImages)
{
  //  CheckBatchConsistency();

    //SF_ASSERT(BundlePatternFrameId != 0xbe6);

    // Iterate through buffer and handle changes to the cached graph.    
    Context::ChangeBuffer::Page* pcbPage = cb.GetFirstPage();
    while(pcbPage)
    {
        unsigned iitem;
        for (iitem = 0; iitem < pcbPage->GetSize(); iitem++)
        {
            Context::EntryChange& change = pcbPage->GetItem(iitem);
            TreeNode*             pnode  = (TreeNode*)change.pNode;
            if (!pnode)
                continue;

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
            if (!Render2D_EntryChanges)
            {
                Render2D_EntryChanges = true;
                Render2D_ChangeStartTime = Timer::GetProfileSeconds();
            }
#endif            

            // If changed data doesn't have cached renderer data, we don't care about it.
            TreeCacheNode* pcache = (TreeCacheNode*)pnode->GetRenderData();
            if (!pcache)
                continue;
            
            if (change.ChangeBits & Change_HandleChanges_Mask)
            {
//printf("C");
                pcache->HandleChanges(change.ChangeBits);
                //CheckBatchConsistency();
            }

            TreeCacheRoot* root = pcache->pRoot;

            if (change.ChangeBits & (Change_AproxBounds|Change_Visible|Change_IsMask|Change_3D|
                                     Change_State_Effect_Mask))
            {
                unsigned updateFlags = 0;
                if (change.ChangeBits & Change_Visible)
                {
                    updateFlags |= Update_Pattern;
                    // Push visibility into cached bit.
                    pcache->ApplyFlags(NF_Visible, pcache->GetNodeData()->GetFlags() & NF_Visible);
                }

                // Handle mask changes, which can affect the tree.
                if (change.ChangeBits & Change_State_MaskNode)
                    pcache->updateMaskCache(pcache->GetNodeData(), pcache->Depth+1, false);

                if (change.ChangeBits & Change_AproxBounds)
                    updateFlags |= Update_PatternBounds;
                if (change.ChangeBits & Change_State_Effect_Mask)
                {
                    // UpdateFlags EffectMask update happens as a part of GetPattern
                    // call while rebuilding parent list.
                    pcache->UpdateFlags |= (change.ChangeBits & Change_State_Effect_Mask);
                    updateFlags |= Update_Pattern;
                }
                if (change.ChangeBits & Change_3D)
                    updateFlags |= Update_Pattern;

                if (root && updateFlags && pcache->GetParent())
                    root->AddToUpdate(pcache->GetParent(), updateFlags);
            }

            // Don't update matrices for items that don't have root.
            if (!root)
                continue;

            // Note: Since HandleChanges can re-organize the tree, it can also change
            // depth of nodes (by moving them up and/or down). This means that matrix
            // update may be added at the wrong depth index. This, however, is not
            // a problem because list ordering is just an optimization            
            //  - If a matrix node is moved down to lower depth, it would still
            //    just get processed earlier (worst case this gives us redundant 
            //    parent matrix chain multiplication).

            // Detect matrix/cxform changes, adding node to the update list.
            const unsigned changeCopyMask = Change_Matrix | Change_CxForm |
                                            Change_State_Scale9 | Change_3D;

            if (change.ChangeBits & changeCopyMask)
            {
                root->AddToUpdate(pcache, change.ChangeBits & changeCopyMask);
                
                if (change.ChangeBits & Change_Matrix)
                {
                    // Matrix change in mask means parent needs update (bounds changed)
                    // TBD: generalize for ancestors..
                    if (pcache->IsMaskNode())
                        root->AddToUpdate(pcache->pParent, Change_Matrix|Update_PatternBounds);
                }
            }
        }
        pcbPage = pcbPage->pNext;
    }

    if (forceUpdateImages)
        UpdateComplexMeshes();

    // Apply matrix changes.
    TreeCacheRoot* proot = (TreeCacheRoot*)RenderRoots.GetFirst();
    while(!RenderRoots.IsNull(proot))
    {
        //AB: if "forceUpdateImages" -> updateSortKey recursively for proot
        if (forceUpdateImages && proot->GetNode()->GetContext() == &context)
            proot->forceUpdateImages();

        proot->ChainUpdatesByDepth();
        // TBD: Check context pointer?

        proot->UpdateTreeData();
        proot = (TreeCacheRoot*)proot->pNext;
    }

#ifdef RENDER_TRACE_FRAME_CHANGE_TIMING
    if (Render2D_EntryChanges)
        Render2D_ChangeEndTime = Timer::GetProfileSeconds();
#endif
}

void Renderer2DImpl::ForceUpdateImages(Context* pcontext)
{
    UpdateComplexMeshes();

    // Apply matrix changes.
    TreeCacheRoot* proot = (TreeCacheRoot*)RenderRoots.GetFirst();
    for(; !RenderRoots.IsNull(proot); proot = (TreeCacheRoot*)proot->pNext)
    {
        if (pcontext && pcontext != proot->GetNode()->GetContext())
            continue;
        proot->forceUpdateImages();

        proot->ChainUpdatesByDepth();
        // TBD: Check context pointer?

        proot->UpdateTreeData();
    }
}

#ifdef SF_AMP_SERVER
void Renderer2DImpl::GetExternalMemory(MemItem* root)
{
    UInt32 nextId = root->GetMaxId();
    MemItem* externalItem = root->AddChild(++nextId, "External Memory");
    UInt32 externalTotalMemory = 0;

    // TO DO: implement external memory reporting
    // Loop through memory categories, and add them like this:
    // externalItem->AddChild(++nextId, "CategoryName", categoryMemory);
    // externalTotalMemory += categoryMemory;

    externalItem->SetValue(externalTotalMemory);
}
#endif

void Renderer2DImpl::OnHALEvent(HALNotifyType type)
{
    switch(type)
    {
    case HAL_Initialize:
    case HAL_RestoreAfterReset:
        pGlyphCache->Initialize(pHal, &FillManager);
        break;

    case HAL_Shutdown:
        ReleaseAllContextData();
        pMeshKeyManager->DestroyAllKeys();
        pGlyphCache->Destroy();
        break;

    case HAL_PrepareForReset:
        pGlyphCache->Destroy();
        break;

    case HAL_FinalPassBegin:
        break;
    }    
}

void Renderer2DImpl::AddComplexMeshToUpdateList(ComplexMesh::UpdateNode& meshNode)
{
    if (!meshNode.pPrev)
    {
        mComplexMeshUpdateList.PushBack(&meshNode);
    }
}

void Renderer2DImpl::UpdateComplexMeshes()
{
    // this lock is here to make sure the pDelegate of GetProvider() is not
    // nulified.
    Lock::Locker locker(GetMeshKeyManager()->getLock());
    ComplexMesh::UpdateNode* cur = mComplexMeshUpdateList.GetFirst();
    while (!mComplexMeshUpdateList.IsNull(cur))
    {
        cur->GetMesh()->updateFills();
        cur = cur->pNext;
    }
}

}} // Scaleform::Render

