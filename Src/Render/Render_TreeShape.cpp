/**************************************************************************

Filename    :   Render_TreeShape.cpp
Content     :   TreeShape represents a Flash shape node in a tree.
Created     :   2009-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TreeShape.h"
#include "Render_TreeCacheNode.h"

#include "Render_PrimitiveBundle.h" // For GetBundle inline
#include "Renderer2DImpl.h"

namespace Scaleform { namespace Render {

class TreeCacheShape;
class TreeCacheShapeLayer;


//------------------------------------------------------------------------
// ***** TreeCacheShape

// TreeCacheShape contains cached TreeShape data built out of
// child TreeCacheShapeLayer objects.
//   - It is only created if MeshProvider numLayers > 1;
//     otherwise TreeCacheShapeLayer is used directly for efficiency.

class TreeCacheShape : public TreeCacheContainer
{
public:
    TreeCacheShape(TreeShape* pnode, Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheShape();

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeShape::NodeData NodeData;
    inline TreeShape*      GetNode() const { return (TreeShape*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }

    inline ShapeMeshProvider* GetMeshProvider() const { return GetNodeData()->pMeshProvider; }

    inline float GetMorphRatio() const
    { 
        return GetNodeData()->MorphRatio;
    }

    // TreeCacheNode API.
    virtual void    HandleChanges(unsigned changeBits);
    //  virtual void    HandleRemoveNode();
    virtual void    UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth);

    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);
};


//------------------------------------------------------------------------
// ***** TreeCacheShapeLayer

// TreeCacheShapeLayer is a single Layer of a shape, which may be a shape,
// complex shape or a stroke.
//  - For LayerCount > 1, TreeCacheShapeLayers are child items under TreeCacheShape.
//  - For LayerCount == 1, TreeCacheShapeLayer lives directly in the rendering tree.

class TreeCacheShapeLayer : public TreeCacheMeshBase
{
friend class TreeCacheShape;
public:
    // Construct TreeCacheShapeLayer.
    //  pnode may be null if we are a child under 
    TreeCacheShapeLayer(TreeShape* pnode, const SortKey& key, unsigned drawLayer,
                        Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheShapeLayer();
    
    bool    IsComplexShape() const { return ComplexShape; }
    bool    HasParentShapeCache() const { return pNode == 0; }

    const TreeShape::NodeData* GetShapeNodeData() const
    { 
        TreeNode* node = HasParentShapeCache() ? GetParent()->pNode : pNode;
        return (const TreeShape::NodeData*)node->GetDisplayData();
    }

    ShapeMeshProvider* GetMeshProvider() const
    { 
        return GetShapeNodeData()->pMeshProvider;
    }

    inline float GetMorphRatio() const
    { 
        return GetShapeNodeData()->MorphRatio;
    }

    // Creates a new TreeCacheShapeLayer.
    //  - shapeNode may be null if we are creating leaf node under TreeCacheShape parent.
    static TreeCacheNode* Create(TreeCacheNode* pparent,
                                 ShapeMeshProvider* provider, unsigned drawLayer,
                                 unsigned flags, TreeShape* shapeNode, float morphRatio);

    static SortKey        CreateSortKey(TreeCacheNode* refNode,
                                        ShapeMeshProvider* provider, unsigned drawLayer,
                                        unsigned flags, Ptr<Image>* gradientImage, 
                                        float morphRatio);

    unsigned  calcMeshGenFlags() const
    {
        unsigned meshGenFlags = 0;
        if (GetFlags() & NF_PartOfMask)
            meshGenFlags |= Mesh_Mask;
        else if ((GetFlags() & NF_EdgeAA_Mask) == NF_EdgeAA_On)        
            meshGenFlags |= Mesh_EdgeAA;        
        if (GetFlags() & NF_PartOfScale9)
            meshGenFlags |= Mesh_Scale9;
        return meshGenFlags;
    }

    void            getShapeMatrixFrom3D(const TreeShape::NodeData* nd, Matrix2F* m_, const Matrix4F& viewProj) const;

    // Re-assigned SortKey; called due to flag change (Mask/NoMask).
    void            updateSortKey();

    // Optimized conditional version of updateSortKey() that does the job only when there's a gradient morph.
    void            updateSortKeyGradientMorph();
    
    void            UpdateCache(TreeCacheContainer* pparent, UInt16 depth);
    
    virtual void    HandleChanges(unsigned changeBits);
    virtual void    UpdateChildSubtree(const TreeNode::NodeData*, UInt16 depth);
    
    // For nested layer, these will receive matrix and node from TreeCacheShape.
    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);

    virtual void    propagateMaskFlag(unsigned partOfMask);
    virtual void    propagateScale9Flag(unsigned partOfScale9);
    virtual void    propagateEdgeAA(EdgeAAMode parentEdgeAA);
    virtual void    propagate3DFlag(unsigned parent3D);

    // Assigns an inserts a mesh into a specified location.    
    virtual MeshBase* GetMesh();
    virtual void    RemoveMesh();

    inline Bundle* GetBundle() const
    {
        return SorterShapeNode.pBundle.GetPtr();
    }
    virtual void    forceUpdateImages();

private:

    unsigned   getLayer() const { return Layer; }

    // Computes mesh key value for matrix.
    unsigned   calcMeshKey(const ShapeMeshProvider* pmeshProvider, 
                           const Matrix& m, unsigned drawLayer, float* keyData,
                           Scale9GridData* s9g, float morphRatio) const;

    // Updates pMeshKey and its mesh to work for viewMatrix. New MeshKey is
    // created if it didn't exists already. Returns true if update took place.
    // Fills in *meshKeyFlags (mandatory).
    bool       updateMeshKey(Renderer2DImpl* r2D, ShapeMeshProvider* provider, float morphRatio,
                             const Matrix& viewMatrix, unsigned meshGenFlags, 
                             unsigned *meshKeyFlags = 0);

    void       updateTexture0Matrix();

    Ptr<MeshKey>    pMeshKey;
    Ptr<Image>      pGradient;
    bool            ComplexShape;
    unsigned        Layer;
};



//------------------------------------------------------------------------
// ***** TreeShape::NodeData Implementation

bool TreeShape::NodeData::PropagateUp(Entry* entry) const
{        
    RectF    bounds, parentBounds, orgBounds;

    // TO DO: Force Update bounds when the shape changes!!!
    if (AproxLocalBounds.IsEmpty())
    {
        if (pMeshProvider)
        {
            // TO DO: Consider correct computation of stroke bounds that requires 
            // StrokeGenerator and ToleranceParam.
            bounds = pMeshProvider->GetIdentityBounds();
        }
    }
    else
    {
        bounds = AproxLocalBounds;
    }

    if (!bounds.IsEmpty())
    {
        // Must apply any filters to the bounds before it is transformed into parent space.
        orgBounds = bounds;
        expandByFilterBounds(&bounds, false );
        if ( !Is3D() )
            parentBounds = M2D().EncloseTransform(bounds);
        else
            parentBounds = M3D().EncloseTransform(bounds);
    }
    
    if ((bounds != AproxLocalBounds) || (parentBounds != bounds))
    {
        TreeShape* pc = (TreeShape*)entry;
        NodeData* d = pc->GetWritableData(Change_AproxBounds);
        d->AproxLocalBounds = bounds;
        d->AproxParentBounds = parentBounds;
        d->updateOriginalBoundState(orgBounds);
        return IsVisible();
    }
    return false;   
}

TreeCacheNode* TreeShape::NodeData::updateCache(TreeCacheNode* pparent,
                                                TreeCacheNode* pinsert,
                                                TreeNode* pnode, UInt16 depth) const
{
    // Create & insert node if it doesn't already exist.
    TreeShape*          pshape = (TreeShape*)pnode;
    TreeCacheNode*      pcache = (TreeCacheNode*)pnode->GetRenderData();
    if (!pcache)
    {
        ShapeMeshProvider*  meshProvider = pshape->GetDisplayData()->pMeshProvider;
        unsigned            numLayers = meshProvider->GetLayerCount();
        
        // Pass NF_MaskNode|NF_PartOfMask to create proper SortKey; it is
        // also adjusted in Insert.
        unsigned cnodeFlags = (Flags & (NF_Visible|NF_MaskNode)) |
                              NF_MaskNode_MapTo_PartOfMask(Flags) |
                              TreeCacheNode::CalcEdgeAAMode(this, pparent);
        cnodeFlags |= (pparent->GetFlags() & (NF_PartOfMask|NF_PartOfScale9));
        cnodeFlags |= (Flags & (NF_3D));
        if (GetState<Scale9State>())
            cnodeFlags |= NF_PartOfScale9;

        // For numLayers == 1, create TreeCacheShapeLayer directly.
        // If numLayers != 1, create TreeCacheShape with child layers.
        // numLayers == 0 is handled as an empty TreeCacheShape container.
        if (numLayers != 1)
            pcache = SF_HEAP_AUTO_NEW(pparent)
                     TreeCacheShape(pshape, pparent->pRenderer2D, cnodeFlags);
        else
            pcache = TreeCacheShapeLayer::Create(pparent, meshProvider, 0, cnodeFlags, pshape, MorphRatio);
        
        if (!pcache) return 0;
        pnode->SetRenderData(pcache);
    }

    pcache->UpdateInsertIntoParent(pparent, pinsert, this, depth);    
    return pcache;
}

// Clone TreeNode, potentially in new context.
TreeNode* TreeShape::NodeData::CloneCreate(Context* context) const
{
    return context->CreateEntry<TreeShape>(NonlocalCloneArg<TreeShape::NodeData>(*this));
}
// Unnecessary, since copy constructor is good enough
//bool      TreeShape::NodeData::CloneInit(TreeNode* node, Context* context) const
//{
//}



//------------------------------------------------------------------------
// ***** TreeCacheShape Implementation

TreeCacheShape::TreeCacheShape(TreeShape* pnode, Renderer2DImpl* prenderer2D, unsigned flags)
: TreeCacheContainer(pnode, prenderer2D, flags)
{
}

TreeCacheShape::~TreeCacheShape()
{        
    // Since cache Layers have no nodes, delete them explicitly.
    while (!Children.IsEmpty())
    {
        TreeCacheNode* p = Children.GetFirst();
        p->RemoveNode();
        p->pPrev = 0;
        p->SetParent(0);
        delete p;
    }
}

//------------------------------------------------------------------------
void TreeCacheShape::UpdateTransform(const TreeNode::NodeData* shapeData,
                                     const TransformArgs& t, TransformFlags flags)
{
    TransformArgs args(t, t.Mat);
    if (flags & TF_Has3D)
        args.SetMatrix3D(t.GetMat3DDirect());

    updateCulling(shapeData, t, &args.CullRect, (TransformFlags) (flags | TF_CullCxform));

    SortParentBounds = shapeData->AproxParentBounds;
    SetFlags(GetFlags()&~NF_ExpandedBounds);

    // Child shape layers don't need individual culling because group
    // culling was just done above.
    flags = (TransformFlags) (flags & ~TF_NeedCull);

    TreeCacheNode* pnode = Children.GetFirst();
    while (!Children.IsNull(pnode))
    {
        // Avoid virtual call since we know child node type.
        TreeCacheShapeLayer* p = (TreeCacheShapeLayer*)pnode;
        p->TreeCacheShapeLayer::UpdateTransform(shapeData, args, flags);
        // TreeCacheShapeLayer doesn't have NeedUpdate flag.
        pnode = pnode->pNext;
    }
}


//------------------------------------------------------------------------
void TreeCacheShape::HandleChanges(unsigned changeBits)
{
    if (changeBits & Change_EdgeAAMode)
        propagateEdgeAA(CalcEdgeAAMode(GetNodeData(), GetParent()));

    if ((changeBits & Change_MorphRatio) && pRoot)
    {
        // Force a call to UpdateTransform, which will detect ratio change in updateMesh.
        pRoot->AddToUpdate(this, Change_Matrix);

        TreeCacheNode* pnode = Children.GetFirst();
        while (!Children.IsNull(pnode))
        {
            // Avoid virtual call since we know child node type.
            TreeCacheShapeLayer* p = (TreeCacheShapeLayer*)pnode;
            p->TreeCacheShapeLayer::updateSortKeyGradientMorph();
            // TreeCacheShapeLayer doesn't have NeedUpdate flag.
            pnode = pnode->pNext;
        }

    }

    // TO DO: Recreate Layers due to SetShape()

    // Since the structure of the shape can change (1 layer vs. more layers),
    // this object may need to be destroyed & re-created.
    // Potentially implement as:
    //   - Remove from parent.
    //   - Update parent to insert new item here.
    //   - Delete this (and return immediately).
    //  Renderer2DImpl::EntryChanges would need updating to handle deleted pcache;
    //  perhaps this function can return a replacement? TBD.

    // TreeCacheShapeLayer would need a similar implementation as well.
}


//------------------------------------------------------------------------
void TreeCacheShape::UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth)
{
    TreeCacheNode::UpdateChildSubtree(pdata, depth);

    // Insertion pointer for new nodes as we go.
    ShapeMeshProvider* meshProvider = static_cast<const NodeData*>(pdata)->pMeshProvider;
    unsigned layerCount = meshProvider->GetLayerCount();
    unsigned drawLayer  = 0;

    if (Children.IsEmpty())
    {
        TreeCacheNode*  pnewInsert = Children.GetFirst()->pPrev;
    
        for(drawLayer = 0; drawLayer < layerCount; ++drawLayer)
        {
            unsigned flags = (GetFlags() & NF_EdgeAA_Mask) | NF_Visible;
            TreeCacheNode* cache =
                TreeCacheShapeLayer::Create(this, meshProvider, drawLayer, flags, 0, GetMorphRatio());
            if (cache)
            {                
                cache->UpdateInsertIntoParent(this, pnewInsert, 0, depth);
                pnewInsert = cache;
            }
        }
    }
    else
    {
        TreeCacheNode*  p = Children.GetFirst();
        while(!Children.IsNull(p))
        {
            ((TreeCacheShapeLayer*)p)->UpdateCache(this, depth);
            p = p->pNext;
        }
    }
}


//------------------------------------------------------------------------
// ***** TreeCacheShapeLayer implementation

TreeCacheShapeLayer::TreeCacheShapeLayer(TreeShape* node, const SortKey& key, unsigned drawLayer,
                                         Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheMeshBase(node, key, prenderer2D, flags),
    ComplexShape(key.GetType() == SortKey_MeshProvider), Layer(drawLayer)
{
}

TreeCacheShapeLayer::~TreeCacheShapeLayer()
{
}


//------------------------------------------------------------------------
TreeCacheNode* TreeCacheShapeLayer::Create(TreeCacheNode* pparent,
                                           ShapeMeshProvider* provider, unsigned drawLayer,
                                           unsigned flags, TreeShape* shapeNode, float morphRatio)
{   
    Ptr<Image> gradient(0);
    SortKey    key(CreateSortKey(pparent, provider, drawLayer, flags, &gradient, morphRatio));
    
    TreeCacheShapeLayer* pcache = 
        SF_HEAP_AUTO_NEW(pparent)
        TreeCacheShapeLayer(shapeNode, key, drawLayer, pparent->pRenderer2D, flags);
    if (!pcache)
        return 0;

    pcache->pGradient = gradient;
    return pcache;
}

SortKey TreeCacheShapeLayer::CreateSortKey(TreeCacheNode* refNode,
                                           ShapeMeshProvider* provider, unsigned drawLayer,
                                           unsigned flags, Ptr<Image>* gradientImage,
                                           float morphRatio)
{
    unsigned meshGenFlags = 0;
    if ((flags & NF_EdgeAA_Mask) == NF_EdgeAA_On)
        meshGenFlags |= Mesh_EdgeAA;
    if (flags & NF_PartOfMask)
        meshGenFlags = Mesh_Mask; // Mask replaces EdgeAA
    if (flags & NF_PartOfScale9)
        meshGenFlags |= Mesh_Scale9;

    TreeCacheNode* pancNode = refNode;
    bool is3D = (flags & NF_3D) ? true : false;
    do {
        is3D |= (pancNode->GetFlags() & NF_3D) ? true : false;
        pancNode = pancNode->GetParent();
    } while ( pancNode && !is3D);

    if (provider->GetFillCount(drawLayer, meshGenFlags) > 1)
    {
        // ComplexMesh key, based on MeshProvider.
        return SortKey(provider, is3D);
    }

    // Simple Mesh key, based on PrimitiveFill.
    TextureManager*       mng = refNode->GetHAL()->GetTextureManager();
    PrimitiveFillManager& fillManager = refNode->GetPrimitiveFillManager();
    FillData   fillData;    
    provider->GetFillData(&fillData, drawLayer, 0, meshGenFlags);
    Ptr<PrimitiveFill> fill = *fillManager.CreateFill(fillData, gradientImage, mng, morphRatio);
    return SortKey(fill.GetPtr(), is3D );
}


void TreeCacheShapeLayer::updateSortKey()
{
    // Sort key changes in response to Flags; currently to enable/disable mask.
    SorterShapeNode.ClearBundle(); // Removes from Bundle::Entries array, if any.

    SorterShapeNode.Key = CreateSortKey(this, GetMeshProvider(), getLayer(),
                                        GetFlags(), &pGradient, GetMorphRatio());
    ComplexShape = (SorterShapeNode.Key.GetType() == SortKey_MeshProvider);
    
    pMeshKey.Clear();

    if (pRoot && GetParent())
        pRoot->AddToDepthUpdate(GetParent(), Update_Pattern|Change_Matrix);    
}

void TreeCacheShapeLayer::updateSortKeyGradientMorph()
{
    if (GetMeshProvider()->HasGradientMorph())
        updateSortKey();
}

void TreeCacheShapeLayer::updateTexture0Matrix() 
{
    // TO DO: Revise GetFillMatrix for morphing.
    if (!ComplexShape && SorterShapeNode.pBundle)
    {            
        PrimitiveFill* fill = 
            ((PrimitiveBundle*)SorterShapeNode.pBundle.GetPtr())->Prim.GetFill();

        Texture* texture = fill->GetTexture(0);
        if (texture)
        {
            Matrix tm;
            GetMeshProvider()->GetFillMatrix(pMeshKey->pMesh, &tm, getLayer(), 0, calcMeshGenFlags());

            SF_ASSERT(!M.IsNull());
            //if (M.IsNull())
            //    M = GetMatrixPool().CreateMatrix(m);
            M.SetTextureMatrix(tm, 0);
        }
    }
}

void TreeCacheShapeLayer::forceUpdateImages() 
{
    // Create a sort key, like in updateSortKey, but AddToUpdate only
    // if it is different from the current one.
    // Sort key changes in response to Flags; currently to enable/disable mask.
    ShapeMeshProvider* provider = GetMeshProvider();
    SortKey sk = CreateSortKey(this, provider, getLayer(),
                               GetFlags(), &pGradient, GetMorphRatio());
    if (sk != SorterShapeNode.Key)
    {
        // Sort key changes in response to Flags; currently to enable/disable mask.
        SorterShapeNode.ClearBundle(); // Removes from Bundle::Entries array, if any.

        SorterShapeNode.Key = sk;
        ComplexShape = (SorterShapeNode.Key.GetType() == SortKey_MeshProvider);

        pMeshKey.Clear();

        if (pRoot && GetParent())
            pRoot->AddToUpdate(GetParent(), Update_Pattern|Change_Matrix|Change_Shape);    
    }
    else if (pMeshKey)
    {
        updateTexture0Matrix();
    }
}

void TreeCacheShapeLayer::UpdateCache(TreeCacheContainer* pparent, UInt16 depth)
{  
    SF_ASSERT(pParent == pparent);
    Depth = depth;
    pRoot = pparent->pRoot;
}

void TreeCacheShapeLayer::HandleChanges(unsigned changeBits)
{
    if (changeBits & Change_EdgeAAMode)
    {
        // THis change bit can come in only if ShapeLayer owns its own node. 
        SF_ASSERT(GetNode());
        propagateEdgeAA(CalcEdgeAAMode(GetNodeData(), GetParent()));
    }


    if ((changeBits & Change_MorphRatio) && pRoot)
    {
        // Force a call to UpdateTransform, which will detect ratio change in updateMesh.
        pRoot->AddToUpdate(this, Change_Matrix);
        updateSortKeyGradientMorph();
    }
}

void TreeCacheShapeLayer::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{
    // We override UpdateChildSubtree so that sortKey can be updated if necessary.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9)) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());    
    if (data && data->GetState<Scale9State>())
        newFlags |= NF_PartOfScale9;
    if (data && data->Is3D())
        newFlags |= NF_3D;

    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        updateSortKey();
    }
    if (data)
        updateMaskCache(data, depth);
}

// Returns mesh key flags.
//------------------------------------------------------------------------
unsigned TreeCacheShapeLayer::calcMeshKey(const ShapeMeshProvider* pmeshProvider, 
                                          const Matrix& m, unsigned drawLayer, float* keyData, 
                                          Scale9GridData* s9g, float morphRatio) const
{
    unsigned flags = MeshKey::KF_Fill;

    // TO DO: Try to optimize, calculate it in TreeCacheShape.
    if (GetFlags() & NF_PartOfScale9)
    {
        const TreeCacheNode* thisNode = this;
        if (HasParentShapeCache())
            thisNode = GetParent();

        Matrix2F shapeMtx;
        while(thisNode)
        {
            const NodeData* nd = thisNode->GetNodeData();

            const OrigScale9ParentState* orig = nd->GetState<OrigScale9ParentState>();
            const TreeNode* nd2 = orig ? orig->GetNode() : 0;
            const TreeCacheNode* ndc = nd2 ? nd2->GetRenderDataClean() : 0;

            const Scale9State* s9 = nd->GetState<Scale9State>();
            if (s9)
            {
                s9g->S9Rect    = s9->GetRect();
                s9g->Bounds    = nd->AproxLocalBounds;
                s9g->ShapeMtx  = shapeMtx;
                s9g->Scale9Mtx = nd->M2D();
                s9g->ViewMtx   = m;

                // Scale9 expects that the bounds do not include the filter size in local bounds.
                // The original bounds should be stored as a state on the node, if they were modified by a filter.
                const OrigNodeBoundsState* boundsState = nd->GetState<OrigNodeBoundsState>();
                if (boundsState)
                    s9g->Bounds = *boundsState->GetBounds();
                else
                    s9g->Bounds = nd->AproxLocalBounds;

                flags |= MeshKey::KF_Scale9Grid;
                break;
            }
            shapeMtx.Append(nd->M2D());
            thisNode = ndc ? ndc : thisNode->GetParent();
        }
    }

    bool validKey = true;

    if (flags & MeshKey::KF_Scale9Grid)
    {
        s9g->MakeMeshKey(keyData);
    }
    else
    {
        validKey = MeshKey::CalcMatrixKey(m, keyData, 0);
        unsigned strokeStyle = pmeshProvider->GetLayerStroke(drawLayer);
        if (strokeStyle)
        {
            flags = MeshKey::KF_Stroke;

            StrokeStyleType ss;
            pmeshProvider->GetStrokeStyle(strokeStyle, &ss, 0);
            if (ss.Flags & StrokeFlag_StrokeHinting)
                flags = MeshKey::KF_StrokeHinted;

            if ((ss.Flags & StrokeScaling_Mask) == StrokeScaling_Normal)
                flags |= MeshKey::KF_NormalStroke;
        }
    }

    if (!validKey) 
        flags |= MeshKey::KF_Degenerate;

    if (GetMeshProvider()->HasGradientMorph())
        flags |= MeshKey::KF_GradientMorph;

    keyData[MeshKey::GetKeySize(flags) - 1] = morphRatio;
    return flags;
}

// Updates pMeshKey and its mesh to work for viewMatrix. New MeshKey is
// created if it didn't exists already. Returns true if update took place.
//------------------------------------------------------------------------
bool TreeCacheShapeLayer::updateMeshKey(Renderer2DImpl* r2D, ShapeMeshProvider* provider,
                                        float morphRatio, const Matrix& viewMatrix, 
                                        unsigned meshGenFlags, unsigned *meshKeyFlags)
{
    SF_SIMD_ALIGN( float keyData[MeshKey::MaxKeySize] );

    const ToleranceParams& cfg = r2D->GetToleranceParams();
    unsigned drawLayer = getLayer();
    unsigned f2 = 0;
    if (meshGenFlags & Mesh_EdgeAA) f2 |= MeshKey::KF_EdgeAA;
    if (meshGenFlags & Mesh_Mask)   f2 |= MeshKey::KF_Mask;    

    Scale9GridData s9g;

    unsigned flags = calcMeshKey(provider, viewMatrix, drawLayer, keyData, &s9g, morphRatio) | f2;
    if (meshKeyFlags)
        *meshKeyFlags = flags;
    
    MeshKeyManager* mkmanager = r2D->GetMeshKeyManager();
    MeshKey*        newKey;


    if (pMeshKey)
    {
        if (pMeshKey->Match(drawLayer, flags, keyData, cfg))
            return false;
        newKey = mkmanager->CreateMatchingKey(pMeshKey->GetKeySet(), drawLayer, flags, keyData, cfg);
    }
    else
    {
        newKey = mkmanager->CreateMatchingKey(provider, drawLayer, flags, keyData, cfg);
    }

    if (!newKey)
        return false;
    SF_ASSERT(newKey != pMeshKey);

    if (!newKey->pMesh)
    {
        Ptr<MeshBase> pmesh;
        if (ComplexShape)
        {
            newKey->pMesh = *SF_HEAP_AUTO_NEW(this)
                ComplexMesh(r2D, newKey->GetKeySet(), &r2D->GetPrimitiveFillManager(),
                            viewMatrix, morphRatio, drawLayer, meshGenFlags);
        } // ...don't remove curlies
        else
        {
            newKey->pMesh = *SF_HEAP_AUTO_NEW(this)
                Mesh(r2D, newKey->GetKeySet(), viewMatrix, morphRatio, drawLayer, meshGenFlags);
        }

        if (!newKey->pMesh)
        {
            newKey->Release();
            return false;
        }

        if (flags & MeshKey::KF_Scale9Grid)
        {
            Ptr<Scale9GridData> p = *SF_HEAP_AUTO_NEW(this) Scale9GridData(s9g);
            newKey->pMesh->SetScale9Grid(p);
        }
    }

//if (pMeshKey)
//    printf("*** %u\n", pMeshKey->UseCount);

    pMeshKey = *newKey;
    return true;
}


//------------------------------------------------------------------------
// Compute 2D matrix for tessellation
void TreeCacheShapeLayer::getShapeMatrixFrom3D(const TreeShape::NodeData* nd, Matrix2F* mat, const Matrix4F& viewProj) const
{
    SF_ASSERT(pRoot);
    SF_ASSERT(M.Has3D());
    RectF bounds = nd->pMeshProvider->GetIdentityBounds();

    const ToleranceParams& cfg = pRenderer2D->GetToleranceParams();
    Matrix4F m(viewProj, M.GetMatrix3D());
    const Viewport & vp = pRoot->GetNodeData()->VP;

    SF_SIMD_ALIGN(float coord[8]);
    m.TransformHomogeneousAndScaleCorners(bounds, (float)vp.Width, (float)vp.Height, coord);
    mat->SetRectToParl(bounds.x1, bounds.y1, bounds.x2, bounds.y2, coord);

    if (fabsf(mat->GetDeterminant()) < cfg.MinDet3D) // TO DO: Move constants to ToleranceParams
       *mat = Matrix2F::Scaling(mat->GetScale());

    unsigned strokeStyle = nd->pMeshProvider->GetLayerStroke(getLayer());
    if (strokeStyle)
    {
        float s = mat->GetScale();
        if (s < cfg.MinScale3D) // TO DO: Move constants to ToleranceParams
            s = cfg.MinScale3D;
        *mat = Matrix2F::Scaling(s);
    }
}



//------------------------------------------------------------------------
// Updates computed final matrix
void TreeCacheShapeLayer::UpdateTransform(const TreeNode::NodeData* nodeData,
                                          const TransformArgs& t,
                                          TransformFlags flags)
{
    RectF cullRect(t.CullRect);
    updateCulling(nodeData, t, &cullRect, (TransformFlags) (flags | TF_CullCxform));
    SortParentBounds = nodeData->AproxParentBounds;
    SetFlags(GetFlags() & ~NF_ExpandedBounds );

    // updates computed final HMatrix
    ComputeFinalMatrix(t, flags);

    // If we were updating Cxform only, skip the rest of the logic, which
    // depends only on matrix. (TBD: Handle Cxform alpha change).
    if (!(flags & TF_Matrix))
        return;

    const TreeShape::NodeData* shapeNodeData = (const TreeShape::NodeData*)nodeData;    
    if (!pMeshKey)
        return;

    unsigned meshKeyFlags = 0;
    bool     updateMesh = false;


    Matrix2F mat(Matrix2F::NoInit);
    if ((flags & TF_Has3D) && pRoot)
        getShapeMatrixFrom3D(shapeNodeData, &mat, t.GetViewProj());
    else
        mat = t.Mat;

    if (updateMeshKey(pRenderer2D, shapeNodeData->pMeshProvider, shapeNodeData->MorphRatio,
                      mat, calcMeshGenFlags(), &meshKeyFlags))
    {
        updateMesh = true;

        if (shapeNodeData->MorphRatio != 0)
        {
            updateTexture0Matrix();
        }
    }

    if (M.Has3D() == 0 && (meshKeyFlags & MeshKey::KF_KeyTypeMask) == MeshKey::KF_StrokeHinted)
    {
        Matrix m2 = t.Mat;
        m2.Tx() = floorf(m2.Tx() + ((m2.Tx() < 0) ? -0.5f : +0.5f));
        m2.Ty() = floorf(m2.Ty() + ((m2.Ty() < 0) ? -0.5f : +0.5f));
        M.SetMatrix2D(m2);
    }

    Bundle* bundle = GetBundle();
    if (!bundle)
        return;

    if (updateMesh)
        bundle->UpdateMesh(&SorterShapeNode);
}


// Returns an a mesh, typically before insertion into a specified location by Bundle.
//------------------------------------------------------------------------
MeshBase* TreeCacheShapeLayer::GetMesh()
{    
//    SF_ASSERT(pRoot);
    if (pRoot && (!pMeshKey || !pMeshKey->pMesh))
    {
        Matrix2F mat(Matrix2F::NoInit);
        unsigned meshGenFlags = calcMeshGenFlags();

        SF_ASSERT(!M.IsNull());

        if (M.Has3D())
        {
            Matrix4F viewProj = GetViewProj();
            getShapeMatrixFrom3D(GetShapeNodeData(), &mat, viewProj);
        }
        else
            mat = M.GetMatrix2D();

        //(HasParentShapeCache() ? GetParent() : (TreeCacheNode*)this)->CalcViewMatrix(&m);
        updateMeshKey(pRenderer2D, GetMeshProvider(), GetMorphRatio(), mat, meshGenFlags);

        // Simple shapes only!
        if (!ComplexShape && SorterShapeNode.pBundle)
        {            
            PrimitiveFill* fill = 
                ((PrimitiveBundle*)SorterShapeNode.pBundle.GetPtr())->Prim.GetFill();

            Texture* texture = fill->GetTexture(0);
            if (texture)
            {
                Matrix tm;
                GetMeshProvider()->GetFillMatrix(pMeshKey->pMesh, &tm, getLayer(), 0, meshGenFlags);

                //if (M.IsNull())
                //    M = GetMatrixPool().CreateMatrix(mat);
                M.SetTextureMatrix(tm, 0);
            }
        }
    }

    return pMeshKey ? pMeshKey->pMesh.GetPtr() : 0;
}

//------------------------------------------------------------------------
void TreeCacheShapeLayer::RemoveMesh()
{    
    pMeshKey = 0;
    // Removal of mesh from primitive can be done by group externally.
}


void TreeCacheShapeLayer::propagateMaskFlag(unsigned partOfMask)
{        
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) | partOfMask;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        updateSortKey(); // Mask can change SortKey; add to update...
    }
}

void TreeCacheShapeLayer::propagateScale9Flag(unsigned partOfScale9)
{    
    if (GetNode() && GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    unsigned newFlags = (GetFlags() & ~NF_PartOfScale9) | partOfScale9;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        // Scale9 can change SortKey, since its passed to GetFillData()
        updateSortKey(); 
    }
}

void TreeCacheShapeLayer::propagateEdgeAA(EdgeAAMode parentEdgeAA)
{
    EdgeAAMode edgeAA = parentEdgeAA;
    if (GetNode())
        edgeAA = TreeCacheNode::CalcEdgeAAMode(GetNodeData(), parentEdgeAA);
    if ((GetFlags() & NF_EdgeAA_Mask) != (unsigned)edgeAA)
    {        
        ApplyFlags(NF_EdgeAA_Mask, edgeAA);
        updateSortKey();
    }
}

void TreeCacheShapeLayer::propagate3DFlag(unsigned parent3D)
{
    SF_UNUSED(parent3D);
    updateSortKey(); // Changing 3D state always affects SortKey impl.
}



}} // Scaleform::Render
