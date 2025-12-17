/**************************************************************************

Filename    :   Render_TreeCacheNode.cpp
Content     :   Cached version of tree node classes
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_TreeCacheNode.h"
#include "Render/Renderer2DImpl.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Render_Twips.h"

bool Check_Address = false;
unsigned BundlePatternFrameId = 0;
unsigned BuildPatternCount = 0;

namespace SFR = Scaleform::Render;

// For crash debugging
volatile const SFR::TreeCacheNode  *TCN_This = 0;
volatile const SFR::TreeNode::NodeData *TCN_ThisData = 0;
volatile const SFR::TreeNode* TCN_ChildNode = 0;
volatile const SFR::TreeCacheNode *TCN_Child = 0;
volatile const SFR::TreeCacheNode *TCN_Parent = 0;
volatile Scaleform::UPInt TCN_Index = 0;



namespace Scaleform { namespace Render {

// ***** TreeCacheNode

TreeCacheNode::TreeCacheNode(TreeNode* pnode, Renderer2DImpl* prenderer2D, unsigned flags)
 : pRoot(0),  pNode(pnode), pRenderer2D(prenderer2D),
   pParent(0), pMask(0),
   Depth(0), Flags((UInt16)flags), UpdateFlags(Change_State_Effect_Mask),
   pNextUpdate(0)
{   
    SF_ASSERT((flags & ~NF_CacheNode_Mask) == 0);
    // Clear nodes to mark that Parent is not a container.
    pPrev = pNext = 0;
}

TreeCacheNode::~TreeCacheNode()
{
    // Destroyed nodes need to remove themselves from parent,
    // in case HandleChanges is called on it.    

    if (pPrev)
        RemoveNode();
    else if (pParent)
    {
        SF_ASSERT(pParent->GetMask() == this);
        pParent->ClearMask();
    }
    
    if (pMask)
    {
        // We may still have pMask if it was moved a new invisible subtree,
        // for example. Don't leave it dangling.
        SF_ASSERT(pMask->pParent == this);
        pMask->RemoveFromParent();
    }
}

// Helper Renderer2DImpl accessor to avoid including that header
HAL* TreeCacheNode::GetHAL() const
{
    return pRenderer2D->GetHAL();
}
PrimitiveFillManager& TreeCacheNode::GetPrimitiveFillManager() const
{
    return pRenderer2D->GetPrimitiveFillManager();
}
MatrixPool& TreeCacheNode::GetMatrixPool() const
{
    return pRenderer2D->GetMatrixPool();
}


void TreeCacheNode::UpdateInsertIntoParent(TreeCacheNode* pparent,
                                           TreeCacheNode* pinsert,
                                           const TreeNode::NodeData* nodeData,
                                           UInt16 depth)
{
    TreeCacheNode* oldParent = GetParent();
    if ((oldParent != pparent) ||
        (pinsert && pparent->GetMask() == this) || // Adding to container what was its mask earlier.
        (!pinsert && pparent->GetMask() != this))  // Setting as mask what was a child earlier.
    {
        if (oldParent)
        {
            RemoveFromParent();
            if (oldParent->pRoot && oldParent->IsPatternChainValid())
                oldParent->pRoot->AddToUpdate(oldParent, Update_Pattern);
        }

        // Insert into us.
        if (pinsert)
            pinsert->InsertNodeAfter(this);
        // nodeData can be null for Shape Layer.
        if (nodeData)
        {
            if (nodeData->IsMaskNode())
            {
                // If parent had a different mask, remove it.
                if (pparent->GetMask() && (pparent->GetMask() != this))
                    pparent->GetMask()->RemoveFromParent();
                SF_ASSERT(pinsert == 0);
                SF_ASSERT(pparent->GetNodeData()->GetState<MaskNodeState>()
                                 ->GetNode()->GetRenderData() == this);
                pparent->SetMask(this);
                SetFlags(GetFlags()|NF_MaskNode);
                propagateMaskFlag(NF_PartOfMask);
            }

            // Old version. Probably incorrect
            //if (nodeData->GetState<Scale9State>())
            //    propagateScale9Flag(NF_PartOfScale9);

            // New version. Looks correct
            unsigned s9flag = pparent->GetFlags() & NF_PartOfScale9;
            if (nodeData->GetState<Scale9State>())
                s9flag |= NF_PartOfScale9;
            propagateScale9Flag(s9flag);

            propagateEdgeAA((EdgeAAMode)(pparent->GetFlags() & NF_EdgeAA_Mask));
        }
        SetParent(pparent);
    }

    // Verify pParent insertion and Depth.
    // - Depth is needed even if there is no pRoot since is allows us to
    //   propagate bounds, keeping them properly cached.
    Depth = depth;
    pRoot = pparent->pRoot;

    // Call UpdateChildSubtree to build child layers.
    // This is a virtual function that will do nothing for leaf nodes.
    UpdateChildSubtree(nodeData, depth + 1);
}

// Removed node from a cached parent child list, also updating the Primitive.
// Used by HandleChanges.
void TreeCacheNode::RemoveFromParent()
{
    if (pPrev)
    {
        SF_ASSERT(pParent);
        RemoveNode();
    }
    else if (pParent)
    {
        SF_ASSERT(pParent->GetMask() == this);
        pParent->ClearMask();
        SetFlags(GetFlags() & ~NF_MaskNode);
    }
    // Clear pPrev, pNext to mark no parent container.
    pNext   = pPrev = 0;
    pParent = 0;
    Depth   = 0;

    // Clear NF_PartOfMask flag in subtree.
    if (GetFlags() & NF_PartOfMask)
        propagateMaskFlag(0);
    if (GetFlags() & NF_PartOfScale9)
        propagateScale9Flag(0); // propagate will check flag presence.
    
    // This nulls out pRoot/CachedChildPattern in us and subtree, which effectively
    // disables matrix, cxform and pattern updates.
    HandleRemoveNode();
}

// Removes this node by clearing out pRoot in the subtree.
void TreeCacheNode::HandleRemoveNode()
{
    pRoot = 0;
    if (HasMask())
        GetMask()->HandleRemoveNode();
}


void TreeCacheNode::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{   
    // NF_PartOfMask flag update is moved into UpdateChildSubtree because it
    // includes custom change-detection logic in TreeCacheShapeLayer.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());
    if (data->GetState<Scale9State>())
        newFlags |= NF_PartOfScale9;
    if (GetParent())
        newFlags |= (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9));
    if (data->Is3D())
        newFlags |= NF_3D;
    SetFlags(newFlags);

    updateMaskCache(data, depth);
}

void TreeCacheNode::updateMaskCache(const TreeNode::NodeData* data, UInt16 depth,
                                    bool updateSubtree)
{
    TCN_This = this;
    TCN_ThisData = data;

    // Mask can be added, removed, or replaced.
    if (data->HasMask())
    {
        TreeNode* maskNode = data->GetState<MaskNodeState>()->GetNode();

        TCN_ChildNode = maskNode;
        
        if (updateSubtree)
        {
            // Regular update for pRoot, Depth, etc called from UpdateChildSubtree
            maskNode->GetDisplayData()->updateCache(this, 0, maskNode, depth);
        }
        else
        {
            // Mask state-change triggered update.
            TreeCacheNode* newMask = (TreeCacheNode*)maskNode->GetRenderData();

            if ((newMask != GetMask()) || !GetMask())
            {   
                // Update should remove non-matching pMask in this.
                newMask = maskNode->GetDisplayData()->updateCache(this, 0, maskNode, depth);
                SF_UNUSED(newMask);
                SF_ASSERT((GetMask() == newMask) && (GetFlags()&NF_HasMask));
                SF_ASSERT(newMask->GetFlags() & NF_MaskNode);
                goto update_on_mask_change;
            }
        }
    }
    else if (HasMask())
    {   // Mask removed.
        GetMask()->RemoveFromParent();
        SF_ASSERT(!GetMask());

    update_on_mask_change:
        if (pRoot && IsPatternChainValid())
            pRoot->AddToUpdate(this, Update_Pattern);
    }
}

void TreeCacheNode::updateMaskTransform(TransformArgs& t,
                                        TransformFlags flags)
{
    if (HasMask())
    {
        const TreeNode::NodeData* data = GetMask()->GetNodeData();
        if ( !(flags & TF_Has3D) )
            t.Mat.Prepend(data->M2D());
        else
            t.PrependMatrix3D(data->M3D());
        GetMask()->UpdateTransform(data, t, flags);
    }
}

void TreeCacheNode::updateEffectChain(BundleEntryRange* contentChain)
{
    if (HasMask())
    {
        BundleEntryRange maskPattern(Range_Empty);
        GetMask()->GetPatternChain(&maskPattern, 0);
        Effects.UpdateBundleChain(contentChain, &maskPattern);
    }
    else
    {
        Effects.UpdateBundleChain(contentChain, 0);
    }
}


void TreeCacheNode::propagateMaskFlag(unsigned partOfMask)
{    
    ApplyFlags(NF_PartOfMask, partOfMask);
}
void TreeCacheNode::propagateScale9Flag(unsigned partOfScale9)
{
    if (GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    ApplyFlags(NF_PartOfScale9, partOfScale9);
}
void TreeCacheNode::propagateEdgeAA(EdgeAAMode parentEdgeAA)
{
    EdgeAAMode edgeAA = TreeCacheNode::CalcEdgeAAMode(GetNodeData(), parentEdgeAA);
    ApplyFlags(NF_EdgeAA_Mask, edgeAA);
}
void TreeCacheNode::propagate3DFlag(unsigned parent3D)
{
    // ApplyFlags must only be called on the node directly being set/unset as 3D.
    SF_UNUSED(parent3D);
}

RectF TransformBounds3D( const Matrix4F& viewproj, const Viewport & vp, const Matrix3F& viewMatrix, const RectF & inputBounds, bool orient = false )
{
    // Transform into clip space.
    RectF clipBounds;
    Matrix4F wvp(viewproj, viewMatrix);
    wvp.EncloseTransformHomogeneous(&clipBounds, inputBounds);
    switch(vp.Flags & Viewport::View_Orientation_Mask)
    {
        default: 
            break;
        case Viewport::View_Orientation_L90:
        case Viewport::View_Orientation_R90:
            if ( orient )
                return Viewport::ScaleToViewport(0, 0, vp.Height, vp.Width, clipBounds);
            break;
    }
    return Viewport::ScaleToViewport(0, 0, vp.Width, vp.Height, clipBounds);
}

// Updates culling for this class and returns new needCull flag.
TransformFlags TreeCacheNode::updateCulling(
    const TreeNode::NodeData* data, const TransformArgs& t,
    RectF* cullRect, TransformFlags flags)
{
    enum CullReason
    {
        NotCulled,
        MaskCulled,
        AlphaCulled,
        TransformCulled,
        TransformCulled_NearFar,
        TransformCulled_3D
    };

    CullReason reason = NotCulled;
    bool culled = false;
    bool is3d = (flags & TF_Has3D) != 0;
    if (flags & TF_NeedCull)
    {
        RectF bounds;

        Matrix3F        m3(Matrix3F::NoInit);

        bool blendNeedsM3 = (data->GetState<BlendState>() != 0 && 
            BlendState::IsTargetAllocationNeededForBlendMode(data->GetState<BlendState>()->GetBlendMode()));

        if ( HasMask() || data->HasFilter() || blendNeedsM3)
        {
            t.GetMatrix3D(flags, &m3);
        }

        // Update Mask Matrix (in effect) + do mask culling.
        if (HasMask())
        {
            Matrix2F        maskAreaMatrix;
            MaskEffect*     maskEffect = Effects.GetEffectOfType<MaskEffect, State_MaskNode>();
            MaskEffectState mes;

            // Update MaskEffect state, including its matrix.
            if (maskEffect)
            {
                mes = calcMaskBounds(&bounds, &maskAreaMatrix, m3, t.GetViewProj());
                if (maskEffect->UpdateMatrix(mes, maskAreaMatrix))
                {
                    UpdateFlags |= Change_State_MaskNode;
                    addParentToDepthPatternUpdate();
                }
            }
            else
            {
                mes = calcMaskBounds(&bounds, &maskAreaMatrix, m3, t.GetViewProj());
            }

            // Adjust cullRect.
            // TODO: This does not work properly with orientation. Disable mask culling for oriented screens.
            if (!is3d && pRoot && ((pRoot->GetNodeData()->VP.Flags & Viewport::View_Orientation_Mask) == 0))
            {
                if (mes != MES_NoMask)
                {
                    if (mes == MES_Culled)
                    {
                        reason = MaskCulled;
                        culled = true;
                    }
                    else
                    {   
                        // If view cullRect became empty, we are culled.
                        if ( !is3d )
                        {
                            t.Mat.EncloseTransform(&bounds, bounds);
                        }
                        else
                        {
                            // View and project matrices always come from the root (for now).
                            TransformBounds3D(t.GetViewProj(), pRoot->GetNodeData()->VP, m3, bounds );
                        }

                        if ( !cullRect->IntersectRect(cullRect, bounds))
                        {
                            culled = true;
                            reason = MaskCulled;
                        }
                    }
                }
            }
        }

        // Update cacheable object here - doing it later would cause some redundant calculations.
        updateFilterCache(data, t, flags, cullRect, m3, is3d);
        updateBlendCache(data, t, flags, cullRect, m3, is3d);

        // Check for 'alpha' culling - ie. if an object's alpha is approximately zero.
        if ((flags & TF_CullCxform) && !culled )
        {
            // Cull object if its Alpha is perceivably at 0.
            // This is only done for leaf shapes, i.e. the ones that have TF_CullCxform
            // bit passed in an argument. We can't do this for containers since child
            // cxforms can be increased again through additive channel.
            if ((fabs(t.Cx.M[0][3]) < 0.001f) && (fabs(t.Cx.M[1][3]) < (1.0f/255.0f)) &&
                !(GetFlags() & NF_PartOfMask))
            {
                culled = true;
                reason = AlphaCulled;
            }
        }

        // 'Regular' out of screen view culling.
        if (!culled && pRoot)
        {
            if ( !is3d )
            {
                t.Mat.EncloseTransform(&bounds, data->AproxLocalBounds);
                if ( !cullRect->Intersects(bounds) )
                {
                    culled = true;
                    reason = TransformCulled;
                }
            }
            else
            {
                // View and project matrices always come from the root (for now).
                Matrix3F m3f(Matrix3F::NoInit);
                t.GetMatrix3D(flags, &m3f);

                RectF viewportBounds = TransformBounds3D(t.GetViewProj(), pRoot->GetNodeData()->VP, m3f, data->AproxLocalBounds, true);
                if ( !cullRect->Intersects(viewportBounds))
                {
                    // Check whether it is in the viewspace or not.
                    reason = TransformCulled_3D;
                    culled = true;
                }
            }
        }
    }

    // Temporarily disable culling when the frame is oriented other than default.
    if ( pRoot && pRoot->GetNodeData()->VP.Flags & Viewport::View_Orientation_Mask )
        culled = false;

    // Temporarily disable 3D culling. In certain circumstances, the culling may give false-positives on culled
    // objects, causing them to disappear when they shouldn't. This is due to an approximation of the 3D culling,
    // which is not always valid.
    if (culled && reason == TransformCulled_3D)
        culled = false;

    // If this was a 2D node, but it has a 3D child, don't cull it, as this may not be accurate.
    if (culled && reason == TransformCulled && data->GetFlags() & NF_3DChild)
        culled = false;

    // Never set culled flag on the root; you should always have the stage color.
    if (culled && (this != pRoot))
    {
        if (!(GetFlags() & NF_Culled))
        {
            SetFlags(GetFlags()|NF_Culled);
            addParentToDepthPatternUpdate();
        }

        // Remove NeedCull flag for children, since the container was culled.
        flags = (TransformFlags)(flags & ~TF_NeedCull);
    }
    else if (GetFlags() & NF_Culled)
    {
        SetFlags(GetFlags()&~NF_Culled);
        addParentToDepthPatternUpdate();
    }

    if (HasMask())
    {
        if ( !(flags & TF_Has3D) )
        {
            TransformArgs args(t, t.Mat);
            updateMaskTransform(args, flags);
        }
        else
        {
            Matrix3F m3(Matrix3F::NoInit);
            t.GetMatrix3D(flags, &m3);
            TransformArgs args(t, m3);
            args.Mat = Matrix2F::Identity;
            updateMaskTransform(args, flags);
        }
    }

    return flags;
}


// Calculates bounds of the mask in local coordinates. Return false if
// no adjustment is necessary.
bool TreeCacheNode::calcChildMaskBounds(RectF *bounds, TreeCacheNode* child)
{
    SF_ASSERT(child->HasMask() && (child->pParent == this));
               
    TreeCacheNode*            maskNodeCache= child->pMask;
    const TreeNode::NodeData* maskNodeData = maskNodeCache->GetNodeData();    
    if (maskNodeData->AproxParentBounds.IsEmpty())
        return false;

    // Figure out if there is there any 3D in the tree (in which case we need to project the bounds into clip space).
    bool has3d = maskNodeData->Is3D();
    TreeCacheNode * pparent = this;
    Matrix4F viewproj;
    while ( pparent && !has3d )
    {
        has3d |= pparent->GetNodeData()->Is3D();
        pparent = pparent->GetParent();
    }

    if ( has3d )
    {
        SF_ASSERT( pRoot->GetNodeData()->HasViewport() );
        const Viewport & vp = pRoot->GetNodeData()->VP;
        RectF fullVP(-1,-1,1,1);
        switch(vp.Flags & Viewport::View_Orientation_Mask)
        {
            default:
                *bounds = Viewport::ScaleToViewport(vp.Left, vp.Top, vp.Width, vp.Height, fullVP);
                break;
            case Viewport::View_Orientation_L90:
            case Viewport::View_Orientation_R90:
                *bounds = Viewport::ScaleToViewport(vp.Left, vp.Top, vp.Height, vp.Width, fullVP);
                break;
        }
        return true;
    }

    // TBD: When is TreeCacheNode created for masks?
    //  --> Attached: at parent attachment (to pRoot)
    //  --> Non-Attached: at time of pRoot initialization.
    // Conclusion: all masks should have a root.

    const TreeNode::NodeData* childData = child->GetNodeData();

    // Transform mask into our coordinate system. Transform logic has
    // optimized special cases for direct child/sibling masks.
    Matrix m(maskNodeData->M2D());
    if (maskNodeCache->GetParent() != this)
    {        
        if (maskNodeCache->GetParent() == child)
            m.Append(childData->M2D());
        else
        {   // TBD: Not necessary if Mask is always a child of parent.
            maskNodeCache->CalcViewMatrix(&m);
            Matrix m2, m3;
            CalcViewMatrix(&m3);
            m2.SetInverse(m3);
            m.Append(m3);
        }
    }

    *bounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);
    return true;
}

// Assume the input RectF is in twips, and expand the bounds of the rectangle to include an extra pixel on each side.
void SnapRectToPixels(RectF& rect, float numPixels = 0.5f)
{
    rect.x1 = floorf(rect.x1 - PixelsToTwips(numPixels));
    rect.y1 = floorf(rect.y1 - PixelsToTwips(numPixels));
    rect.x2 = ceilf(rect.x2 + PixelsToTwips(numPixels));
    rect.y2 = ceilf(rect.y2 + PixelsToTwips(numPixels));
}

MaskEffectState TreeCacheNode::calcMaskBounds(RectF* maskBounds,
                                              Matrix2F* boundAreaMatrix,
                                              const Matrix3F& viewMatrix,
                                              const Matrix4F& viewProjMatrix)
{
    SF_ASSERT(HasMask());
    // TBD: Assumes mask parent is this.
    SF_ASSERT(GetMask()->GetParent() == this);

    float pixelSnap = 8.0f;
    const TreeNode::NodeData* data = GetNodeData();    
    const TreeNode::NodeData* maskNodeData = GetMask()->GetNodeData();
    if (!pRoot)
        return MES_NoMask;

    // Figure out if there is there any 3D in the tree (in which case we need to project the bounds into clip space).
    bool has3d = maskNodeData->Is3D();
    TreeCacheNode * pparent = this;
    while ( pparent && !has3d )
    {
        has3d |= pparent->GetNodeData()->Is3D();
        pparent = pparent->GetParent();
    }

    // 2D case.
    if ( !has3d )
    {
        Matrix2F viewMatrix2D;
        viewMatrix2D.SetMatrix(viewMatrix);

        // TBD: When is TreeCacheNode created for masks?
        //  --> Attached: at parent attachment (to pRoot)
        //  --> Non-Attached: at time of pRoot initialization.
        // Conclusion: all masks should have a root.    
        
        // Transform mask into our coordinate system. 
        // TBD: Transform logic has optimized special cases for direct child/sibling masks.
        Matrix2F m(maskNodeData->M2D());
        *maskBounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);

        //  Determine intersection; cull all content if there is no common area.
        RectF intersect, boundsUnion;
        if (!maskBounds->IntersectRect(&intersect, data->AproxLocalBounds))
            return MES_Culled;

        maskBounds->UnionRect(&boundsUnion, data->AproxLocalBounds);

        // Expand to enclose pixel (could be significant with FSAA).
        SnapRectToPixels(boundsUnion, pixelSnap);

        // Combined mask; we'll clear the whole local area.
        boundAreaMatrix->SetMatrix(boundsUnion.Width(), 0.0f, boundsUnion.x1,
            0.0f, boundsUnion.Height(), boundsUnion.y1);
        boundAreaMatrix->Append(viewMatrix2D);
        return MES_Combinable;
    }
    else
    {
        // 3D case.
        RectF contentBounds;
        Matrix4F maskVP;
        Matrix3F m;
        GetMask()->CalcViewMatrix(&m,&maskVP);

        // View and project matrices always come from the root (for now).
        const Viewport & vp = pRoot->GetNodeData()->VP;
        *maskBounds = TransformBounds3D( maskVP, vp, m, maskNodeData->AproxLocalBounds );
        contentBounds = TransformBounds3D( viewProjMatrix, vp, viewMatrix, data->AproxLocalBounds );

        // If the mask and content don't intersect, cull everything.
        RectF intersect;
        if (!maskBounds->IntersectRect(&intersect, contentBounds))
            return MES_Culled;

        // Make the clear rectangle the total local area.
        *maskBounds = maskBounds->Union(contentBounds);

        // Expand to enclose pixel (could be significant with FSAA).
        RectF viewClipRect = *maskBounds;
        SnapRectToPixels(viewClipRect, pixelSnap);

        // Save the results.
        boundAreaMatrix->SetMatrix(viewClipRect.Width(), 0.0f, viewClipRect.x1, 0.0f, viewClipRect.Height(), viewClipRect.y1);
        return MES_Combinable;
    }
}

TreeCacheNode::CacheableBoundResult TreeCacheNode::calcCacheableBounds(
    RectF* returnBounds, Matrix2F* areaMatrix, const Matrix3F& viewMatrix, 
    const Matrix4F& viewProjMatrix, RectF* cullRect)
{
    const TreeNode::NodeData* data = GetNodeData();    
    if (!pRoot)
        return CacheableBoundResult_CompletelyClipped;

    // Figure out if there is there any 3D in the tree (in which case we need to project the bounds into clip space).
    bool has3d = data->Is3D();
    TreeCacheNode * pparent = this;
    while ( pparent && !has3d )
    {
        has3d |= pparent->GetNodeData()->Is3D();
        pparent = pparent->GetParent();
    }

    RectF cullRectTemp, reasonableCullRect;
    RectF viewClipRect;

    // 2D case.
    if ( !has3d )
    {
        // Transform local bounds into view space.
        Matrix2F viewMatrix2D;
        viewMatrix2D.SetMatrix(viewMatrix);
        *returnBounds = data->AproxLocalBounds;

        // Make sure the area is not outside of the culling rect. If it is, we could allocate
        // a much larger render target than is required to draw the cacheable.
        viewMatrix2D.EncloseTransform(&viewClipRect, *returnBounds);
    }
    else
    {
        // View and project matrices always come from the root (for now).
        const Viewport & vp = pRoot->GetNodeData()->VP;
        viewClipRect = TransformBounds3D( viewProjMatrix, vp, viewMatrix, data->AproxLocalBounds );
    }

    // If there is no cullRect, use the entire screen at a minimum. This will guard
    // against gigantic cacheables that are mostly offscreen.
    const Viewport & vp = pRoot->GetNodeData()->VP;
    vp.GetCullRectF(&cullRectTemp);
    if ( !cullRect )
        cullRect = &cullRectTemp;
    RectF orientCullRect = *cullRect;
    switch ( vp.Flags & Viewport::View_Orientation_Mask )
    {
        case Viewport::View_Orientation_L90:
        case Viewport::View_Orientation_R90:
            orientCullRect = cullRectTemp;
            Alg::Swap(orientCullRect.x1, orientCullRect.y1);
            Alg::Swap(orientCullRect.x2, orientCullRect.y2);
    }

    // Make sure it's within a 'reasonable' limitation within the window. For example, with shadow
    // filter's offset or many large blur passes, you could create an extremely large offset which 
    // would be mostly offscreen, and we don't want the renderer allocating space for this.
    RectF expandedCullRect = orientCullRect;
    data->expandByFilterBounds(&expandedCullRect, false);   // TBD: could be made more generic, but if this cacheable is not a filter, just returns the input.
    reasonableCullRect = orientCullRect;
    reasonableCullRect.Expand(32.0f, 32.0f);
    expandedCullRect.IntersectRect(&expandedCullRect, reasonableCullRect );

    bool fullyVisible = true;
    RectF tempRect = viewClipRect;
    if (!reasonableCullRect.Contains(viewClipRect))
    {
        fullyVisible = false;
        if (!reasonableCullRect.IntersectRect(&tempRect, viewClipRect))
            return CacheableBoundResult_CompletelyClipped;
    }

    // Calculate the clip rect from the tempRect, which should be only slightly larger than the cullRect.
    viewClipRect.x1 = floorf(tempRect.x1);
    viewClipRect.y1 = floorf(tempRect.y1);
    viewClipRect.x2 = ceilf(tempRect.x2);
    viewClipRect.y2 = ceilf(tempRect.y2);

    // Save the results.
    *returnBounds = viewClipRect;
    areaMatrix->SetMatrix(viewClipRect.Width(), 0.0f, viewClipRect.x1, 0.0f, viewClipRect.Height(), viewClipRect.y1);
    return fullyVisible ? CacheableBoundResult_FullyVisible : CacheableBoundResult_PartiallyClipped;
}

void TreeCacheNode::UpdateBundlePattern(unsigned)
{
    // Mask update not possible locally because chain isn't known.
}


void TreeCacheNode::CalcViewMatrix(Matrix* pviewMatrix) const
{
    // CalcViewMatrix shouldn't be called on TreeCacheNodes that don't have
    // associated TreeNodes (such as TreeShapeCacheLayer).
    SF_ASSERT(pNode);

    const TreeCacheNode* p = this;
    *pviewMatrix = GetNodeData()->M2D();

    while(p->GetParent())
    {
        p = p->GetParent();
        pviewMatrix->Append(p->GetNodeData()->M2D());
    }
}

void TreeCacheNode::CalcViewMatrix(Matrix3F* pviewMatrix, Matrix4F *pviewProj) const
{
    // CalcViewMatrix shouldn't be called on TreeCacheNodes that don't have
    // associated TreeNodes (such as TreeShapeCacheLayer).
    SF_ASSERT(pNode);

    const TreeNode::NodeData* data = GetNode() ? GetNodeData() : NULL;

    // check if node have view and projection.  If not, get from closes ancestor
    const ViewMatrix3DState* viewState = data && data->HasViewMatrix3D() ? data->GetState<ViewMatrix3DState>() : NULL;
    const ProjectionMatrix3DState* projState = data && data->HasProjectionMatrix3D() ? data->GetState<ProjectionMatrix3DState>() : NULL;

    const TreeCacheNode* p = this;
    *pviewMatrix = data->M3D();

    while(p->GetParent())
    {
        p = p->GetParent();
        pviewMatrix->Append(p->GetNodeData()->M3D());

        if (!viewState && p->GetNodeData()->HasViewMatrix3D())
            viewState = p->GetNodeData()->GetState<ViewMatrix3DState>();
        if (!projState && p->GetNodeData()->HasProjectionMatrix3D())
            projState = p->GetNodeData()->GetState<ProjectionMatrix3DState>();
    }

    if ( viewState && projState )
        *pviewProj = Matrix4F(Matrix4F(projState->GetProjectionMatrix3D()->M), Matrix3F(viewState->GetViewMatrix3D()->M));
    else
    {
        // This assert is flash specific - with renderer only, there does not necessarily need
        // to be a viewproj anywhere in the tree.
        // SF_ASSERT(false);
        *pviewProj = Matrix4F::Identity;
    }
}

bool TreeCacheNode::CalcFilterFlag() const
{
    const TreeCacheNode* p = this;
    while(p->GetParent())
    {
        const TreeCacheNode::NodeData* nodeData = p->GetNodeData();
        if (nodeData && nodeData->HasFilter())
            return true;
        p = p->GetParent();
    }
    return false;
}

void TreeCacheNode::CalcCxform(Cxform& dest) const
{
    dest = GetNodeData()->Cx;
    const TreeCacheNode* p = GetParent();
    while(p)
    {
        if (p->GetNodeData()->HasFilter())
            break;
        dest.Prepend(p->GetNodeData()->Cx);
        p = p->GetParent();
    }
}

//
// Find first ancestor with view and projection matrices.
// Brute force method - avoid if possible (prefer TransformArgs.GetViewProj())
//
Matrix4F TreeCacheNode::GetViewProj() const 
{
    const TreeNode::NodeData* data = GetNode() ? GetNodeData() : NULL;

    // check if node have view and projection.  If not, get from closes ancestor
    const ViewMatrix3DState* viewState = data && data->HasViewMatrix3D() ? data->GetState<ViewMatrix3DState>() : NULL;
    const ProjectionMatrix3DState* projState = data && data->HasProjectionMatrix3D() ? data->GetState<ProjectionMatrix3DState>() : NULL;

    const TreeCacheNode* p = this;

    while(p->GetParent() && !(viewState && projState))
    {
        p = p->GetParent();

        if (!viewState && p->GetNodeData()->HasViewMatrix3D())
            viewState = p->GetNodeData()->GetState<ViewMatrix3DState>();
        if (!projState && p->GetNodeData()->HasProjectionMatrix3D())
            projState = p->GetNodeData()->GetState<ProjectionMatrix3DState>();
    }

    if ( viewState && projState )
        return Matrix4F(Matrix4F(projState->GetProjectionMatrix3D()->M), Matrix3F(viewState->GetViewMatrix3D()->M));

    SF_ASSERT(false);
    return Matrix4F::Identity;
}

void TreeCacheNode::updateFilterCache( const TreeNode::NodeData* data, const TransformArgs &t, 
                                      TransformFlags flags, RectF* cullRect, const Matrix3F& m3, bool is3d )
{
    if (data->HasFilter() && (flags & (TF_Matrix|TF_Cxform|TF_ForceRecache)))
    {
        Matrix2F m;
        RectF filterBounds;
        FilterEffect*  filterEffect = Effects.GetEffectOfType<FilterEffect, State_Filter>();
        if ( filterEffect )
        {
            CacheableBoundResult result = calcCacheableBounds(&filterBounds, &m, m3, t.GetViewProj(), cullRect );
            if (result != CacheableBoundResult_CompletelyClipped)
            {
                // Assume we can cache across the transformation.
                bool requiresUpdate = false;

                // Cxform for this node is not accumulated, but is required.
                if (flags&TF_Cxform)
                {
                    Cxform cx = t.Cx;
                    filterEffect->UpdateCxform(cx);                        
                }

                if (flags&TF_Matrix)
                {
                    // Force filter-uncaching when a 3D or partially clipped object has a matrix change. 
                    // We do not decompose transformation differences across 3D matrices, or detect clipping changes,
                    // so it's undetermined whether the filter could actually cache.
                    requiresUpdate = filterEffect->UpdateMatrix(m, t.Mat, is3d || result == CacheableBoundResult_PartiallyClipped);
                }


                // Check to see whether the update should be forced.
                requiresUpdate |= (flags&TF_ForceRecache) != 0;

                // For now, also update the filter's bundle, causing it to be re-filtered.
                // We will be able to handle simple matrix changes without filtering.
                if (requiresUpdate)
                    filterEffect->Update(data->GetState<FilterState>());
            }
        }
    }
}

void TreeCacheNode::updateBlendCache( const TreeNode::NodeData* data, const TransformArgs &t, TransformFlags flags, RectF* cullRect, const Matrix3F& m3, bool)
{
    if (data->States.GetState(State_BlendMode) != 0 && (flags & (TF_Matrix|TF_Cxform)))
    {
        Matrix2F m;
        RectF blendBounds;
        BlendModeEffect*  blendEffect = Effects.GetEffectOfType<BlendModeEffect, State_BlendMode>();
        if ( blendEffect )
        {
            if (flags&TF_Matrix)
            {
                // TBD: we may want to support changing the bounds of the blend operation. For now, assume that the blend
                // happens over pixel centers on top of the parent target.
                CacheableBoundResult result = calcCacheableBounds(&blendBounds, &m, m3, t.GetViewProj(), cullRect );
                if (result != CacheableBoundResult_CompletelyClipped)
                {
                    blendBounds = data->AproxLocalBounds;
                    blendEffect->UpdateMatrix(m);
                }
            }
            blendEffect->UpdateCxform(t.Cx);
        }
    }
}


// ***** Cached Nodes Handling 

TreeCacheContainer::~TreeCacheContainer()
{
    // Set all child items, if still alive, to have null pParent pointer.
    TreeCacheNode* p = Children.GetFirst();
    while(!Children.IsNull(p))
    {
        SF_ASSERT(p->GetParent() == this);
        TreeCacheNode* next = p->pNext;
        p->SetParent(0);
        p->pPrev = p->pNext = 0;
		// Clear out the pRoot pointers in sub-tree to avoid invalid access.
        if (p->pRoot)
            p->HandleRemoveNode();
        p = next;
    }
}



//------------------------------------------------------------------------
// UpdateTransform - updates and propagates Matrix and Cxform values
// down the tree. Also performs culling & mask updates.
// For nested layer, these will receive matrix and node from TreeCacheShape.
void TreeCacheContainer::UpdateTransform(const TreeNode::NodeData* pbaseData,
                                         const TransformArgs& t,
                                         TransformFlags flags)
{    
    TransformArgs args(t); // Assign nothing.
    flags = updateCulling(pbaseData, t, &args.CullRect, flags);

    SortParentBounds = pbaseData->AproxParentBounds;
    SetFlags(GetFlags()&~NF_ExpandedBounds);

    // Traverse child list and not pbaseData->Children array because the
    // later may have nodes with no cache (in case we are in destroying phase).
    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {        
        const TreeNode::NodeData* pchildData = child->GetNodeData();
        TransformFlags            childFlags = (TransformFlags)
                                    (flags | (child->UpdateFlags & (Change_CxForm|Change_Matrix)));

        child->UpdateFlags &= ~(Change_Matrix|Change_CxForm); 

        // Concat transforms:
        // if child mat is 2D then append it to cur 2D mat, set has2D flag (start or continue a 2D sequence)
        // if child mat is 3D then,
        //      if cur has2D then append child 3D to cur 2D (converted to 3D), clear has2D
        //      else append child 3D to cur 3D
        //      set has3D
        if (!pchildData->Is3D())
        {
            // child is 2D
            if (flags & TF_Has2D)
                args.Mat.SetToAppend(pchildData->M2D(), t.Mat);         // child append t or  (t * child)
            else
                args.Mat = pchildData->M2D();
            childFlags = (TransformFlags)(childFlags | TF_Has2D);
            if (flags & TF_Has3D)
            {
                args.SetMatrix3D(t.GetMat3DDirect());       // carry along the 3D transform if necessary
            }
        }
        else
        {   // child is 3D            
            if (flags & TF_Has2D)
            {
                Matrix3F m3(Matrix3F::NoInit);
                t.GetMatrix3D(flags, &m3);
                args.SetMatrix3D(m3, pchildData->M3D());
                childFlags = (TransformFlags)(childFlags & ~TF_Has2D);  // clear 2D flag since 2D has been appended to 3D
            }
            else	
                if (flags & TF_Has3D)
                {
                    Matrix3F m3(Matrix3F::NoInit);
                    t.GetMatrix3D(flags, &m3);

                    args.SetMatrix3D(m3, pchildData->M3D());           // append child 3D to cur 3D, continue a 3D sequence		
                }
            childFlags = (TransformFlags)(childFlags | TF_Has3D);
        }

        args.SetViewProj(pchildData, &t);

        // Do not accumulate a Cxform from a parent that has a filter. It is applied to
        // the filtered results, not the child objects within the filter.
        bool accumulateCxform = true;
        if ( pbaseData->HasFilter() )
        {
            const FilterState* filterState = pbaseData->GetState<FilterState>();
            if ( filterState )
            {
                const FilterSet * filterSet = filterState->GetFilters();
                if ( filterSet && filterSet->IsContributing() )
                    accumulateCxform = false;
            }
            childFlags = (TransformFlags)(childFlags | TF_ParentCacheable);
        }

        // Do not accumulate a Cxform from a parent that has a cacheable blend mode.
        const BlendState* blendState = pbaseData->GetState<BlendState>();
        if ( blendState && BlendState::IsTargetAllocationNeededForBlendMode(blendState->GetBlendMode()))
        {
            accumulateCxform = false;
            childFlags = (TransformFlags)(childFlags | TF_ParentCacheable);

            // If we encounter a BlendMode which requires a target, but has no parent with a Layer BlendMode,
            // then enable the fullscreen blend target for the tree root. TBD: currently, once the fullscreen
            // target is enabled, it will never be disabled, even if all required blends are removed.
            if (blendState->GetBlendMode() == Blend_Layer)
                childFlags = (TransformFlags)(childFlags | TF_ParentLayerBlend);
            else if (((flags & TF_ParentLayerBlend) == 0) && pRoot)
                pRoot->EnableBlendTarget = true;
        }

        if ( accumulateCxform )
            args.Cx.SetToAppend(pchildData->Cx, t.Cx);
        else
            args.Cx = pchildData->Cx;

        child->UpdateTransform(pchildData, args, childFlags);
        child = child->pNext;
    }
}


//------------------------------------------------------------------------
// 
void    TreeCacheContainer::HandleChanges(unsigned changeBits)
{
    SF_ASSERT(pNode);
    if (changeBits & Change_EdgeAAMode)
        propagateEdgeAA(CalcEdgeAAMode(GetNodeData(), GetParent()));

    if (!(changeBits & (Change_ChildInsert|Change_ChildRemove)))
        return;

    // Go through child data array and verify that it matches our list.
    // Remove/add items as necessary.
    bool            change = false;
    const NodeData* pdata = GetNodeData();
    UPInt           childCount = pdata->Children.GetSize();
    unsigned        index;
    TreeCacheNode*  pcache1 = Children.GetFirst();    

    for(index = 0; index < childCount; index++)
    {
        TreeNode*      pnode0  = pdata->Children.GetAt(index);
        TreeCacheNode* pcache0 = (TreeCacheNode*)pnode0->GetRenderData();

        // If item matches, move on.
        if (pcache0 == pcache1)
        {
            pcache1 = pcache1->pNext;
            continue;
        }

        // If item doesn't have us as parent or isn't a part of our Children
        // list, as in (pPrev == 0) for masks, re-root it.
        if (!pcache0 || (pcache0->GetParent() != this) || !pcache0->pPrev)
        {
            if (pcache0 && pcache0->GetParent())
            {
                TreeCacheNode* oldParent = pcache0->GetParent();
                // This branch was removed elsewhere in the tree and re-rooted to us!!
                //  handle that remove...
                pcache0->RemoveFromParent();
                if (oldParent->pRoot)
                    oldParent->pRoot->AddToUpdate(oldParent, Update_Pattern);
            }
            
            // Insertion; insert content ... it can either be a sprite or a shape!
            TreeCacheNode* pinserted = 
                pnode0->GetDisplayData()->updateCache(this, pcache1->pPrev, pnode0, Depth + 1);

            if (pinserted && pinserted->pRoot)
                pinserted->pRoot->AddToUpdate(pinserted, Change_Matrix | 
                                                         Change_CxForm);
            change = true;

            // It is possible (although highly rare) that insert operation will
            // remove our next child, adding it somewhere in the subtree.
            // If this happens, re-run the loop.
            if (!Children.IsNull(pcache1) && (pcache1->GetParent() != this))
            {   // Reset.
                pcache1 = Children.GetFirst();
                index   = (unsigned)-1;
            }

            // no pcache1 advance
            continue;
        }

        // If we are here, (pcache0->pParent == this).
        // If means that either that
        //   (a) the pcache1 item was removed from the list/relocated or
        //   (b) the pcache0 item was moved here from a different location in our list.
        SF_ASSERT(!Children.IsNull(pcache1));
        
        // Just remove the current and future items until we hit pcache0 or reach
        // the end of the cached item list.
        do {
            TreeCacheNode* pcache1Next = pcache1->pNext;
            SF_ASSERT(pcache1->pParent == this);
            pcache1->RemoveFromParent();
            pcache1 = pcache1Next;

        } while((pcache1 != pcache0) && !Children.IsNull(pcache1));
        
        change = true;
               
        // Don't increment index, since we need to loop around to insert this index item
        index--;
    }

    // Remove remaining items, if any.
    while (!Children.IsNull(pcache1))
    {
        TreeCacheNode* pcache1Next = pcache1->pNext;
        SF_ASSERT(pcache1->pParent == this);
        pcache1->RemoveFromParent();
        pcache1 = pcache1Next;
        change = true;
    }

    if (change && IsPatternChainValid())
    {
        SF_ASSERT(pRoot); // implied not null by CachedChildPattern validity.
        pRoot->AddToUpdate(this, Update_Pattern);
    }
}


// Removes this node by clearing out pRoot, so that node matrix propagation doesn't happen
void TreeCacheContainer::HandleRemoveNode()
{
    TreeCacheNode::HandleRemoveNode();
    //pRoot = 0;
    CachedChildPattern.Clear(Range_Invalid);

    TreeCacheNode* p = Children.GetFirst();
    while (!Children.IsNull(p))
    {
        p->HandleRemoveNode();
        p = p->pNext;
    }
}


// TBD: UpdateChildSubtree processing may be somewhat incorrect if a tree is being added that
// had several modifications done to it in the same frame. If that is the case, HandleChanges
// will rearrange the subtree leafs again...
void TreeCacheContainer::UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth)
{
    TreeCacheNode::UpdateChildSubtree(pdata, depth);
    // Insertion pointer for new nodes as we go.
    TreeCacheNode*        pnewInsert = Children.GetFirst()->pPrev;
    const TreeNodeArray & children = static_cast<const NodeData*>(pdata)->Children;

    for(UPInt i=0; i<children.GetSize(); i++)
    {
        TreeNode* pchildNode = children[i];
        pnewInsert = 
            pchildNode->GetDisplayData()->updateCache(this, pnewInsert, pchildNode, depth);
    }
}

void TreeCacheContainer::propagateMaskFlag(unsigned partOfMask)
{
    ApplyFlags(NF_PartOfMask, partOfMask);    

    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        // State of a branch under NF_MaskNode flag can't change,
        // so don't go there.
        if (!(child->IsMaskNode()))
            child->propagateMaskFlag(partOfMask);
        child = child->pNext;
    }
}

void TreeCacheContainer::propagateScale9Flag(unsigned partOfScale9)
{
    if (GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    // No need to recurse down if the flag is already set correctly.
    if ((GetFlags() & NF_PartOfScale9) == partOfScale9)
        return;

    ApplyFlags(NF_PartOfScale9, partOfScale9);

    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        child->propagateScale9Flag(partOfScale9);
        child = child->pNext;
    }
}

void TreeCacheContainer::propagateEdgeAA(EdgeAAMode parentEdgeAA)
{
    EdgeAAMode edgeAA = TreeCacheNode::CalcEdgeAAMode(GetNodeData(), parentEdgeAA);
    if ((GetFlags() & NF_EdgeAA_Mask) == (unsigned)edgeAA)
        return;
    ApplyFlags(NF_EdgeAA_Mask, edgeAA);

    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        child->propagateEdgeAA(edgeAA);
        child = child->pNext;
    }
}

void TreeCacheContainer::propagate3DFlag(unsigned parent3D)
{
    unsigned is3D = GetFlags() & NF_3D;

    // If we are 3D, and this node is already 3D, we can stop propagation.
    if ( parent3D == NF_3D && is3D )
        return;
    parent3D |= is3D;
    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        child->propagate3DFlag(parent3D);
        child = child->pNext;
    }
}


bool FixedSizeArrayRectF::Intersects(const RectF& bounds)
{
    if (!bounds.IsEmpty())
    {
        for (UPInt i=0; i<GetSize(); i++)
        {
            RectF& r = operator[](i);
            bool noIntersection = (r.x1 >= bounds.x2) || (bounds.x1 >= r.x2) ||
                                  (r.y1 >= bounds.y2) || (bounds.y1 >= r.y2);
            if (!noIntersection)
                return true;
        }
    }
    return false;
}    

#if defined(SF_ENABLE_SIMD)
// This implementation is roughly +50% faster on X86 then stock Intersects.
// For frogger.swf (Ctrl+G), this improves framerate 895 -> 960fps.

void FixedSizeArrayRect2F::PushBack(const RectF& r)
{
    using namespace Scaleform::SIMD;
    if (HalfRect)
    {
        Rect2F& t = pData[Size-1];
        Vector4f r0 = IS::LoadAligned(&r.x1);
        t.r0 = IS::Shuffle<0, 1, 0, 1>(t.r0, r0);
        t.r1 = IS::Shuffle<0, 1, 2, 3>(t.r1, r0);
        HalfRect = false;
    }
    else
    {
        // Allow for fast growth; ok since its temp store.
        if (Size == Reserve)
        {
            grow(Size * 2);
        }
        Rect2F& t = pData[Size];
        Size++;

        Vector4f r0 = IS::LoadAligned(&r.x1);
        t.r0 = IS::Shuffle<0, 1, 2, 3>(r0, t.r0);
        t.r1 = IS::Shuffle<2, 3, 2, 3>(r0, t.r1);
        HalfRect = true;
    }
}

bool FixedSizeArrayRect2F::Intersects(const RectF& bounds)
{
    using namespace Scaleform::SIMD;
    if (!bounds.IsEmpty())
    {
        static Vector4f c0000 = IS::Constant<0,0,0,0>();
        static Vector4f c1100 = IS::Constant<0xFFFFFFFF,0xFFFFFFFF,0,0>();
        static Vector4f c0011 = IS::Constant<0,0,0xFFFFFFFF,0xFFFFFFFF>();

        Vector4f bound_y2x2y1x1 = IS::LoadAligned(&bounds.x1);
        Vector4f b_y1x1y1x1 = IS::Shuffle<0,1,0,1>(bound_y2x2y1x1, bound_y2x2y1x1);
        Vector4f b_y2x2y2x2 = IS::Shuffle<2,3,2,3>(bound_y2x2y1x1, bound_y2x2y1x1);

        UPInt i = 0;
        UPInt count = GetSize();
        if (HalfRect)
            count--;

        for (; i<count; i++)
        {
            Rect2F* pr = &operator[](i);
            Vector4f r1_y1x1_r0_y1x1 = pr->r0;
            Vector4f r1_y2x2_r0_y2x2 = pr->r1;

            Vector4f compare0 = IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2);
            Vector4f compare1 = IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2);
            Vector4f compare2 = IS::Or(compare0, compare1);

            if ( IS::CompareEQ_All( IS::And(compare2, c1100), c0000) || 
                 IS::CompareEQ_All( IS::And(compare2, c0011), c0000) )
            {
                return true;
            }
        }

        if (HalfRect)
        {
            Rect2F* pr = &operator[](i);
            Vector4f r1_y1x1_r0_y1x1 = pr->r0;
            Vector4f r1_y2x2_r0_y2x2 = pr->r1;

            Vector4f compare0 = IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2);
            Vector4f compare1 = IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2);
            Vector4f compare2 = IS::Or(compare0, compare1);
            if ( IS::CompareEQ_All( IS::And(compare2, c1100), c0000) )
                return true;
        }
    }
    return false;
}

void FixedSizeArrayRect2F::Clear()
{
    FixedSizeArray<Rect2F>::Clear();
    HalfRect = false;
}
#endif // SF_ENABLE_SIMD

void TreeCacheContainer::BuildChildPattern(BundleEntryRange* pattern, unsigned flags)
{
    FixedSizeArrayRectFImpl prevRects;
    
    SF_ASSERT(pRoot);
    unsigned childIndex = 0;

    pattern->Clear(Range_Empty);
    BundleEntryRangeMatcher prevPattern(Range_Empty);
    bool             expandBounds = false;
    RectF            expandedBounds;

    // Check if this node has a user-data object which enables/disables batching (matching)
    const UserDataState*   userData = GetNodeData()->GetState<UserDataState>();
    if (userData)
    {
        UserDataState::Data* data = userData->GetUserData();
        flags |= ((data->Flags & UserDataState::Data::Data_Batching) && data->BatchingDisabled) ? Pattern_DisableMatch : 0;
    }
    bool             disableMatch = (flags & Pattern_DisableMatch) == Pattern_DisableMatch;

    TreeCacheNode* child = Children.GetFirst();
    for (; !Children.IsNull(child); child = child->pNext, childIndex++)
    {        
        // Get existing chain
        BundleEntryRange childPattern(Range_Empty);
        SF_ASSERT(child->Depth != 0);
        if (!child->GetPatternChain(&childPattern, flags))
            continue;
        //childPattern.VerifyValidChildDepth(chainIndex);

        // Limit on match length.
        if ( childPattern.GetLength() > Bundle_MergeLengthLimit)
        {
            pattern->Append(prevPattern);
            pattern->Append(childPattern);
            prevPattern.Clear(Range_Empty);
            prevRects.Clear();
            continue;
        }

        RectF bounds;
        bool  match = false;

        if (child->pNode)
        {
            bounds = child->SortParentBounds;
            //const TreeCacheNode::NodeData* data = child->GetNodeData();
            //bounds = data->AproxParentBounds;
            // data->M2D().EncloseTransform(data->AproxLocalBounds);
        }

        // Expand bounds to accommodate mask.
        if (child->GetFlags() & (NF_ExpandedBounds|NF_HasMask))
        {
            if (child->HasMask())
            {
                // Expand by mask.
                RectF maskBounds;
                if (calcChildMaskBounds(&maskBounds, child))
                {
                    if (expandBounds)
                        expandedBounds = maskBounds;
                    else
                    {
                        expandBounds = true;
                        expandedBounds.UnionRect(&expandedBounds, maskBounds);
                    }
                    if (!bounds.IsEmpty())
                        bounds.UnionRect(&bounds, maskBounds);
                    else
                        bounds = maskBounds;
                }
            }

            if (expandBounds)
                expandedBounds = bounds;
            else
            {
                expandBounds = true;
                expandedBounds.UnionRect(&expandedBounds, bounds);
            }
        }

        if (!prevPattern.IsEmpty() && !disableMatch)
        {
            // Don't do overlap test if we're 3D - it isn't accurate.
            bool child3D = (child->GetFlags() & NF_3D) != 0;
            bool matchAllowed = !child3D && !prevRects.Intersects(bounds);

            // See if new pattern is sub-pattern of the old one
            if (matchAllowed)
            {
                match = prevPattern.Match(childPattern);
                // TBD: Chain on match?
            }
            else if (!child3D)
            {
                // If pattern is one-unit long and key is identical, overlaps are ok
                if ((prevPattern.GetLength() == 1) && (childPattern.GetLength() == 1) &&
                    (prevPattern.GetFirstKey().MatchSingleItemOverlap(childPattern.GetFirstKey())))
                    match = true;
            }
        }

        if (match)
        {
            // On match, concatenate our chains to those of child.
            prevPattern.MergeMatchingChains(childPattern, Depth);
        }
        else
        {
            // If no match took place
            pattern->Append(prevPattern);
            prevPattern = childPattern;
            prevRects.Clear();
        }

        if (!bounds.IsEmpty())
            prevRects.PushBack(bounds);
    }

    pattern->Append(prevPattern);


    // Expand our bounds to account for child expansion (masks), if necessary.
    // This will ensure that we are properly sorted.
    if (expandBounds)
    {
        SF_ASSERT(pNode);
        const TreeNode::NodeData* data = pNode->GetDisplayData();
        if (!data->AproxLocalBounds.IsEmpty())
            expandedBounds.UnionRect(&expandedBounds, data->AproxLocalBounds);
        data->M2D().EncloseTransform(&expandedBounds, expandedBounds);

        if (expandedBounds != SortParentBounds)
        {
            SortParentBounds = expandedBounds;
            SetFlags(GetFlags()|NF_ExpandedBounds);
        }
    }

    BuildPatternCount++;
}


// Make sure that all items are valid
void VerifyPattern(BundleEntryRange& pattern)
{
    unsigned       count = 0;
    BundleIterator ibundle = pattern.GetIterator();
    
    while(ibundle)
    {   
        // Alias chain, stripping merged nodes that we originally
        // connected higher up in the tree.
        BundleEntry*   chain = ibundle;
        TreeCacheNode* pcacheNode = ibundle->pSourceNode;

        SF_ASSERT(pcacheNode->pRoot);
        SF_UNUSED(pcacheNode);
        if (!pcacheNode->pRoot)
            printf("Null pRoot in %p - %p node\n", ibundle.GetEntry(), pcacheNode);

        unsigned chainCount = 0;
        while(chain->pChain)
        {   
            SF_ASSERT(chain->pSourceNode->pRoot);
            if (!chain->pSourceNode->pRoot)
                printf("Null pRoot in %p - %p node - at chain loop %d\n",
                       ibundle.GetEntry(), ibundle->pSourceNode, chainCount);
            chain = chain->pChain;
            chainCount++;
        }

        // Check validity of possible advance.
        if (ibundle.GetEntry() != pattern.GetLast())
        {
            if (!ibundle->HasNextPattern())
            {
                printf("Frame %d: Null NextPattern after %p, count = %d\n",
                       BundlePatternFrameId, ibundle.GetEntry(), count);
                TreeCacheNode* parent = pcacheNode->GetParent();
                printf("SourceNode = %p, parent = %p\n", pcacheNode, parent);
                while(parent)
                {
                    printf("%p Depth = %d\n", parent, parent->Depth);
                    //printf("%p Updated at frame %d, Depth = %d\n",
                    //          parent, parent->BuildFrameId, parent->Depth);
                    parent = parent->GetParent();
                }
            }
        }

        ibundle++;
        count++;
    }
}


// Verified/Rebuilds the bundle pattern for this container,
// adding parent to update list if necessary.

void TreeCacheContainer::UpdateBundlePattern(unsigned flags)
{
    // Not that invalid patterns are processed later above in top-down
    // fashion, so skip them if they were added by mistake.
    if (!IsPatternChainValid())
        return;
    SF_ASSERT(pRoot);

    // Build an alias list pattern
    BundleEntryRange pattern(Range_Invalid);
    BuildChildPattern(&pattern, flags);

    if (!Effects.IsEmpty() && !pattern.IsEmpty())
    {   
        // We always rebuild this chain, even if pattern and CachedChildPattern values match.
        // The later can't be relied upon because child item could've been re-rooted deeper
        // in subtree, which would yield same pattern but with broken pNextPattern node.
        BundleEntryRange chain(pattern);        
        updateEffectChain(&chain);
    }

    // See if parent update is necessary. It is needed if:
    //  1. First/Last pointer doesn't match.
    //  2. Pattern old/new Length <= limit and pattern structure has changed.
    //  3. Pattern old/new Length <= limit and at least one of its tail chain nodes don't match.

    // Invisible nodes don't require parent update notification, since they
    // don't contribute bundles. Parent notification takes place externally
    // when visibility changes.
    if (IsDrawn())
    {
        if ((CachedChildPattern.GetLength() <= Bundle_MergeLengthLimit) ||
            (pattern.GetLength() <= Bundle_MergeLengthLimit) ||
            (CachedChildPattern.GetFirst() != pattern.GetFirst()) ||
            (CachedChildPattern.GetLast() != pattern.GetLast()) )
        {  
            addParentToDepthPatternUpdate();
        }
        else
        {
            
            if (CachedChildPattern.GetLength() != pattern.GetLength())
            {
                if (GetParent() && GetParent()->IsPatternChainValid() &&
                    !(GetParent()->UpdateFlags & Update_Pattern_Mask))
                {
                    // Propagate length change; this will very rarely happen.
                    addParentToDepthPatternUpdate();

              //      printf("Node %p at d=%d, Length %d -> %d\n", this, Depth,
              //             CachedChildPattern.GetLength(), pattern.GetLength());
                }
            }
        }
    }

    CachedChildPattern = pattern;
}


bool TreeCacheContainer::GetPatternChain(BundleEntryRange* range, unsigned flags)
{
    // Invisible and/or culled objects don't contribute bundles to parent.
    if (!IsDrawn())
    {
        range->Clear(Range_Empty);
        return false;
    }
    
    bool     oldChainValid     = CachedChildPattern.IsValid();
    unsigned updateEffectsMask = UpdateFlags & Change_State_Effect_Mask;
    
    if (oldChainValid && !updateEffectsMask)
    {
        if (!CachedChildPattern.IsEmpty())
        {
            if (!Effects.GetRange(range))
                *range = CachedChildPattern;
            range->StripChainsByDepth(Depth);
        }
    }
    else
    {
        if (oldChainValid)
            CachedChildPattern.StripChainsByDepth(Depth);
        else
            BuildChildPattern(&CachedChildPattern, flags);
        *range = CachedChildPattern;

        // Update the effects.
        if (updateEffectsMask)
        {
            Effects.UpdateEffects(this, updateEffectsMask);
            UpdateFlags &= ~Change_State_Effect_Mask;
        }
        if (!CachedChildPattern.IsEmpty())
            updateEffectChain(range);
    }

    return range->GetLength() != 0;
}

bool TreeCacheContainer::IsPatternChainValid() const
{
    return CachedChildPattern.IsValid();
}

void TreeCacheContainer::forceUpdateImages()
{
    TreeCacheNode* child = Children.GetFirst();
    for (; !Children.IsNull(child); child = child->pNext)
    {        
        child->forceUpdateImages();
    }
}


//------------------------------------------------------------------------
// ***** TreeCacheRoot Implementation

void TreeCacheRoot::ChainUpdatesByDepth()
{
    TreeCacheNode *p0, *p = pUpdateList;
    pUpdateList = 0;

    while (p)
    {
        p0 = p->pNextUpdate;
        DepthUpdates.Link(p->Depth, &p->pNextUpdate, p);
        p = p0;
    }

    DepthUpdatesChained = true;
}


void TreeCacheRoot::AddToUpdate( TreeCacheNode *pnode, unsigned flags )
{
    SF_ASSERT(flags && pnode->pNode && (DepthUpdatesChained == false));
    if (!(pnode->UpdateFlags & Update_InList))
    {
        pnode->pNextUpdate = pUpdateList;
        pUpdateList = pnode;
        flags |= Update_InList;
    }
    pnode->UpdateFlags |= flags;
}

void TreeCacheRoot::AddToDepthUpdate( TreeCacheNode *pnode, unsigned flags )
{
    SF_ASSERT(flags && pnode->pNode);
    if (!(pnode->UpdateFlags & Update_InList))
    {
        if (DepthUpdatesChained)
        {
            DepthUpdates.Link(pnode->Depth, &pnode->pNextUpdate, pnode);
        }
        else
        {
            pnode->pNextUpdate = pUpdateList;
            pUpdateList = pnode;
        }

        flags |= Update_InList;
    }
    pnode->UpdateFlags |= flags;
}

void TreeCacheRoot::HandleChanges(unsigned changeBits)
{
    if ( changeBits & Change_Viewport && GetNodeData())
    {
        ViewValid = GetNodeData()->VP.GetCullRectF(&ViewCullRect, false);
    }
    TreeCacheContainer::HandleChanges(changeBits & ~Change_Viewport);
}

/*
// Simple test logic to count the number TreeContainer objects in the scene,
// used when approximating benefits of TreeNodeArray.
static unsigned CountainerCount = 0;
static unsigned ContainerUseCounts[8];

void CountContainerUse_Recursive(const TreeContainer::NodeData* n)
{
    UPInt size = n->Children.GetSize();
    if (size < 7)
        ContainerUseCounts[size] ++;
    else
        ContainerUseCounts[7]++;
    CountainerCount++;

    for (UPInt j = 0; j< size; j++)
    {
        const TreeNode::NodeData* cn = n->Children[j]->GetDisplayData();
        if (cn->GetType() == TreeNode::NodeData::ET_Container)        
            CountContainerUse_Recursive((const TreeContainer::NodeData*)cn);
    }
}

void CountContainerUse(const TreeContainer::NodeData* n)
{
    CountainerCount = 0;
    for (unsigned i = 0; i< 8; i++)
        ContainerUseCounts[i] = 0;
    CountContainerUse_Recursive(n);
}
*/

void TreeCacheRoot::UpdateTreeData()
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheRoot::UpdateTreeData", Amp_Profile_Level_Medium);

    //CountContainerUse(GetNodeData());

    // And update all Transforms (Matrices, Cxforms) as necessary.
    // Data is propagated top-down, so we visit low-depth indices first.    
    unsigned depthUsed = DepthUpdates.GetDepthUsed();
    unsigned i;
    for (i = 0; i < depthUsed; i++)
    {
        TreeCacheNode* pnodeCache = DepthUpdates.Get(i);
        while(pnodeCache)
        {
            unsigned transformChangeFlags =
                pnodeCache->UpdateFlags & (Change_Matrix|Change_CxForm);
            
            if (pnodeCache->UpdateFlags & Change_State_Scale9)
            {
                TreeCacheNode* p = pnodeCache->GetParent();
                // propagateScale9Flag will check nodes own Scale9 state.
                pnodeCache->propagateScale9Flag(p ? (p->GetFlags() & NF_PartOfScale9) : 0);
                pnodeCache->UpdateFlags &= ~Change_State_Scale9;
                transformChangeFlags |= Change_Matrix;
            }

            if (pnodeCache->UpdateFlags & Change_3D)
            {
                pnodeCache->ApplyFlags(NF_3D, pnodeCache->GetNodeData()->Is3D() ? NF_3D : 0 );
                pnodeCache->propagate3DFlag(0);
                pnodeCache->UpdateFlags &= ~Change_3D;
                transformChangeFlags |= Change_Matrix;
            }

            // If there are no transform changes, walk up the tree and make sure that no cacheables exist.
            // If they do, they will need to be recalculated, because presumably something changed to update the
            // node's pattern.
            if ( !transformChangeFlags )
            {
                TreeCacheNode* p = pnodeCache;
                while ( p )
                {
                    const TreeNode::NodeData* data = p->GetNodeData();
                    // bool filterContributes = false;
                    if ( data->HasFilter() )
                    {
                        const FilterState* filterState = data->GetState<FilterState>();
                        const FilterSet* filters =  filterState ? filterState->GetFilters() : 0;
                        if ( filters && filters->IsContributing() )
                        {
                            // Indicate movement, which will cause a tree rebuild
                            transformChangeFlags |= TF_ForceRecache;
                            // filterContributes = true;
                            break;
                        }
                    }

                    const BlendState* blendState = data->GetState<BlendState>();
                    if (blendState && BlendState::IsTargetAllocationNeededForBlendMode(blendState->GetBlendMode()))
                    {
                        // Just indicate something that will cause a tree rebuild
                        transformChangeFlags |= TF_ForceRecache;
                        break;
                    }
                    p = p->GetParent();
                }
            }

            // Update transform.
            if (transformChangeFlags)
            {
                SF_ASSERT(pnodeCache->pNode);
                
                // Compute cumulative ancestor Matrix/Cxform.
                const TreeNode::NodeData* pdata = pnodeCache->GetNodeData();
                TransformFlags flags = (TransformFlags)
                    (transformChangeFlags | (ViewValid ? TF_NeedCull : 0));
                bool is3d = pdata->Is3D();
                bool anyTransformChanged = transformChangeFlags != 0;

                TransformArgs  args(ViewCullRect,
                    is3d ? Matrix2F::Identity : pdata->M2D(),
                    is3d ? pdata->M3D() : Matrix3F::Identity, 
                    pdata->Cx);
                args.SetViewProj(pdata, NULL);
                flags = (TransformFlags)(flags | (is3d ? TF_Has3D : TF_Has2D));

                const ViewMatrix3DState* viewState = pdata && pdata->HasViewMatrix3D() ? pdata->GetState<ViewMatrix3DState>() : NULL;
                const ProjectionMatrix3DState* projState = pdata && pdata->HasProjectionMatrix3D() ? pdata->GetState<ProjectionMatrix3DState>() : NULL;

                // We need to compute both Matrix and Cxform even if one of the flags
                // is not set in case one of child objects does need that attribute
                // updated as well.
                TreeCacheNode* p = pnodeCache->GetParent();
                while(p)
                {
                    if (!p->IsDrawn())
                        flags = (TransformFlags)(flags & ~TF_NeedCull);
                    const TreeNode::NodeData* data = p->GetNodeData();

                    if (flags & TF_Has3D)
                        // accumulate in 3D
                        args.AppendMatrix3D(data->Is3D() ? data->M3D() : data->M2D());
                    else
                    {   // 2D case
                        if (data->Is3D())
                        {   // switch to 3D accumulation
                            args.SetMatrix3D(args.Mat);
                            args.AppendMatrix3D(data->M3D());
                            args.Mat.SetIdentity();
                            flags = (TransformFlags)(flags & ~TF_Has2D);
                            flags = (TransformFlags)(flags | TF_Has3D);
                        }
                        else
                            args.Mat.Append(data->M2D());
                    }

                    // Record whether any transform has changed.
                    anyTransformChanged |= ((p->UpdateFlags & (Change_Matrix|Change_CxForm)) != 0);

                    // Record whether any parent has a (contributing) filter.
                    // bool filterContributes = false;
                    if ( data->HasFilter() )
                    {
                        const FilterState* filterState = data->GetState<FilterState>();
                        const FilterSet* filters =  filterState ? filterState->GetFilters() : 0;
                        if ( filters && filters->IsContributing() )
                        {               
                            // If anywhere up the tree has changed a transform, then the filters will need to be redone.
                            if ( anyTransformChanged )
                            {
                                p->UpdateFlags |= Update_Pattern|Change_State_Filter;
                                p->addParentToDepthPatternUpdate();
                            }
                            flags = (TransformFlags)(flags | TF_ParentCacheable);
                            // filterContributes = true;
                        }
                    }

                    // Look for cacheable blend states, and if they exist, update parents.
                    const BlendState* blendState = data->GetState<BlendState>();
                    if (blendState)
                    {
                        if (BlendState::IsTargetAllocationNeededForBlendMode(blendState->GetBlendMode()))
                        {
                            if (anyTransformChanged)
                            {
                                p->UpdateFlags |= Update_Pattern|Change_State_BlendMode;
                                p->addParentToDepthPatternUpdate();
                            }
                            flags = (TransformFlags)(flags | TF_ParentCacheable);

                            if (blendState->GetBlendMode() == Blend_Layer)
                                flags = (TransformFlags)(flags | TF_ParentLayerBlend);
                        }
                    }

                    // Do not accumulate a Cxform from a parent that has a contributing filter. 
                    // It is applied to the filtered results, not the child objects within the filter.
                    if ((flags&TF_ParentCacheable) == 0)
                        args.Cx.Prepend(data->Cx);

                    // check for first ancestor with view / perspective
                    if (!viewState && data->HasViewMatrix3D())
                        viewState = data->GetState<ViewMatrix3DState>();
                    if (!projState && data->HasProjectionMatrix3D())
                        projState = data->GetState<ProjectionMatrix3DState>();

                    p = p->GetParent();
                }

                if (viewState || projState)
                    args.SetViewProj(viewState, projState);
                pnodeCache->UpdateTransform(pdata, args, flags);
                pnodeCache->UpdateFlags &= ~(Change_Matrix|Change_CxForm);
            }

            pnodeCache = pnodeCache->pNextUpdate;
        }
    }

    // Update patterns - this needs to be done from top to bottom.
    // Refresh depthUsed since higher depth updates could've been queued up
    // due to culling.
    depthUsed = DepthUpdates.GetDepthUsed();
    for (i = depthUsed; i != 0; i--)
    {
        TreeCacheNode* pnodeCache = DepthUpdates.Get(i-1);
        while(pnodeCache)
        {
            if (pnodeCache->UpdateFlags & Update_Pattern_Mask)
            {
                // Rebuild/verify pattern for this node.
                pnodeCache->UpdateBundlePattern(0);
                pnodeCache->UpdateFlags &= ~Update_Pattern_Mask;
            }

            pnodeCache->UpdateFlags &= ~Update_InList;
            pnodeCache = pnodeCache->pNextUpdate;
        }
    }

    // Debug only.
    // VerifyPattern(CachedChildPattern);
        
    ClearDepthUpdates();

    // Update all root bundle chains.
    BundleIterator ibundles = CachedChildPattern.GetIterator();
    while(ibundles)
    {
        ibundles->UpdateBundleEntry(this, pRenderer2D, ibundles);
        ibundles++;
    }


    BundlePatternFrameId++;
    //if ((BundlePatternFrameId & 0xFFF) == 0)
    //    printf("Frame %x\n", BundlePatternFrameId);
}

void TreeCacheRoot::Draw()
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheRoot::Draw", Amp_Profile_Level_Medium);

    if (!IsDrawn())
        return;

    // Enable the fullscene blend target, if a child target blend is detected. TBD: if BeginDisplay
    // is called externally from Draw, this flag may be getting set too late. This is abnormal use-case,
    // but still possible.
    pRenderer2D->GetHAL()->SetFullSceneTargetBlend(EnableBlendTarget);

    const TreeRoot::NodeData* data = GetNodeData();
    if ( data->HasViewport() )
        pRenderer2D->GetHAL()->BeginDisplay(data->BGColor, data->VP);

    pRenderer2D->GetHAL()->DrawBundleEntries(CachedChildPattern.GetIterator(), pRenderer2D);

    if (data->HasViewport())
    {
        SF_AMP_SCOPE_RENDER_TIMER_END_DISPLAY("HAL::EndDisplay", Amp_Profile_Level_High);
        pRenderer2D->GetHAL()->EndDisplay();
    }
}

//------------------------------------------------------------------------
// ***** TreeCacheMeshBase implementation

TreeCacheMeshBase::TreeCacheMeshBase(TreeNode* pnode, const SortKey& key,
                                     Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheNode(pnode, prenderer2D, flags),
    SorterShapeNode(GetThis(), key)
{    
}

TreeCacheMeshBase::~TreeCacheMeshBase()
{    
}

// updates computed final HMatrix
void TreeCacheMeshBase::ComputeFinalMatrix(const TransformArgs& t, TransformFlags flags)
{
    // Initialize HW Matrix/Cxform.
    if (M.IsNull())
    {   // Don't allocate Cxform data initially if not used.
        unsigned matrixFormatBits = HMatrix::Has_2D;
        if (!(t.Cx == Cxform::Identity))
            matrixFormatBits |= HMatrix::Has_Cxform;

        if (flags & TF_Has3D)
        {
            Matrix3F m3(Matrix3F::NoInit);
            t.GetMatrix3D(flags, &m3);

            matrixFormatBits |= HMatrix::Has_3D;
            M = GetMatrixPool().CreateMatrix(m3, t.Cx, matrixFormatBits);
        }
        else
            M = GetMatrixPool().CreateMatrix(t.Mat, t.Cx, matrixFormatBits);
    }
    else
    {
        if (flags & TF_Has3D)
        {
            Matrix3F m3(Matrix3F::NoInit);
            t.GetMatrix3D(flags, &m3);
            M.SetMatrix3D(m3);
        }
        else
            M.SetMatrix2D(t.Mat);
        if (flags & TF_Cxform)
            M.SetCxform(t.Cx);
    }
}

// TBD: Handle shape pointer change
void TreeCacheMeshBase::HandleChanges(unsigned changeBits)
{
    SF_UNUSED(changeBits);
}

bool TreeCacheMeshBase::GetPatternChain(BundleEntryRange* range, unsigned)
{    
    if (!IsDrawn())
    {
        range->Clear(Range_Empty);
        return false;
    }

    range->SetToEntry(&SorterShapeNode);
    SorterShapeNode.Removed = false;

    // We request chain when rebuilding a bundle; so mark it for verify.
    if (Effects.IsEmpty() && !(UpdateFlags & Change_State_Effect_Mask))
    {
        // Optimized "strip chains" version for once item.
        if (SorterShapeNode.pBundle)
            SorterShapeNode.pBundle->SetNeedsUpdate();
        // Note: We do NOT clear pNextPattern on purpose!
        SorterShapeNode.pChain = 0;
        SorterShapeNode.ChainHeight = 0;
    }
    else
    {
        if (UpdateFlags & Change_State_Effect_Mask)
        {
            Effects.UpdateEffects(this, UpdateFlags & Change_State_Effect_Mask);
            updateEffectChain(range);
            UpdateFlags &= ~Change_State_Effect_Mask;
        }
        else
        {
            Effects.GetRange(range);
        }
        // Always clear all chains + notify bundles, since this is
        // a leaf node shape.
        // TBD: May not be true for mask that may have mergeable chain entrys (?)
        //range->StripChains();
        range->StripChainsByDepth(Depth);
    }
  
    return true;
}

void TreeCacheMeshBase::UpdateBundlePattern(unsigned)
{   
    if (HasMask() && !Effects.IsEmpty())
    {
        BundleEntryRange chain(Range_Empty);
        chain.SetToEntry(&SorterShapeNode);
        updateEffectChain(&chain);
    }
}

}} // Scaleform::Render
