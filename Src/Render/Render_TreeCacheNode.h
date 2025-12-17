/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TreeCacheNode.h
Content     :   Cached version of tree node classes
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TreeCacheNode_H
#define INC_SF_Render_TreeCacheNode_H

#include "Render_Containers.h"
#include "Render_TreeNode.h"
#include "Render_Bundle.h"
#include "Render_CacheEffect.h"
#include "Render_HAL.h"
#include "Render_Scale9Grid.h"

#include "Kernel/SF_SIMD.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

// ***** Forward declarations
// Cache Tree
class TreeCacheNode;
class TreeCacheRoot;
class TreeCacheContainer;
class TreeCacheMeshBase; // For pattern match


//--------------------------------------------------------------------
// ***** TreeCacheNode Classes

// These classes represent renderer-side cached objects associated with each TreeNodeData.
// The TreeCacheNode tree is built on the first call to Renderer2D::Draw, with its nodes
// having one-to-one correspondence to those of the original TreeNodeData tree. The TreeNodeData
// tree can, however, be modified independently (in a different thread) with changes tracked
// by the RenderContext class. After such modification the trees become mismatched and
// the change list must be applied to the TreeCacheNode objects before the next frame
// can be properly rendered. The changes are applied by calling HandleChanges() on each
// modified node, as reported by the change list.
//
// To enable primitive batching, each TreeCacheNode maintains a list of BundleLayer
// layers. This list contains all of the batches produced by the traversal of child
// TreeNodes, grouped together by their display order. Each BundleLayer contains
// one or more shapes, obtained from one or more child TreeNodes. The Layers list
// maintained by the top-level TreeCacheRoot object contains the complete set
// of RenderBatches describing the entire rendering sub-tree.
//
// As changes take place within the rendering tree, with TreeCacheNode objects being
// modified, removed, inserted or rearranged, the Layers maintained within TreeCacheNode
// objects must also be updated. Modifications to render batches may involve insertion
// and removal of child nodes, as well as splitting or merging of existing batches.
// Modifications made within the child Layers list must also be propagated into the parent
// Layers list. This batch propagation is handled by the LayerSet_InsertChildLayers and
// LayerSet_RemoveChildLayers functions, which are called on a parent whenever a change
// in a child Layers list takes place.


enum TransformFlags
{
    TF_None             = 0,
    TF_Matrix           = Change_Matrix,
    TF_Cxform           = Change_CxForm,
    TF_NeedCull         = 0x00000010,   // TBD: Verify that doesn't match above.
    TF_CullCxform       = 0x00000020,   // Cxforms are only culled for leaf shapes.
    TF_Has2D            = 0x00000040,
    TF_Has3D            = 0x00000080,
    TF_ParentCacheable  = 0x00000100,   // A parent of this node contains a cacheable primitive (filter or cached blend).
    TF_ForceRecache     = 0x00000200,   // The update transform pass should force re-caching results (eg. filters)
    TF_ParentLayerBlend = 0x00000400,   // A parent of this ndoe contains a BlendMode == Layer.

    // Combined flags for readability in debugger.
    TF_Matrix_Cxform   = TF_Matrix | TF_Cxform,
    TF_Matrix_NeedCull = TF_Matrix | TF_NeedCull,
    TF_Cxform_NeedCull = TF_Cxform | TF_NeedCull,
    TF_Matrix_Cxform_NeedCull = TF_Matrix_Cxform | TF_NeedCull,

    // Above + CullCxform
    TF_NeedCull_CullCxform = TF_NeedCull | TF_CullCxform,
    TF_Matrix_Cxform_CullCxform   = TF_Matrix | TF_Cxform | TF_CullCxform,
    TF_Matrix_NeedCull_CullCxform = TF_Matrix | TF_NeedCull | TF_CullCxform,
    TF_Cxform_NeedCull_CullCxform = TF_Cxform | TF_NeedCull | TF_CullCxform,
    TF_Matrix_Cxform_NeedCull_CullCxform = TF_Matrix_Cxform | TF_NeedCull | TF_CullCxform
};

//--------------------------------------------------------------------
// Arguments for TreeCacheNode::UpdateTransform
struct TransformArgs
{
public:
    RectF               CullRect;       // Rectangle to compare for culling. May not be identical to the viewport, due to masking.
private:    
    Matrix3F            Mat3D;          // use accessor to get 3D matrix, since it may have to be combined with 2D mat
    const ViewMatrix3DState* viewState;
    const ProjectionMatrix3DState* projState;
    mutable Matrix4F    ViewProj;       // cached view and projection
    mutable bool        bRecomputeViewProj;
public:
    Matrix2F            Mat;
    Cxform              Cx;

    // Matrix and culling info only.
    TransformArgs(const TransformArgs& copy)
        : CullRect(copy.CullRect), 
        Mat3D(Matrix3F::NoInit),
        ViewProj(Matrix4F::NoInit),
        Mat(Matrix2F::NoInit), Cx(Cxform::NoInit){  CopyViewProj(copy);  }

    // Matrix, culling, 2D xform and Cxform.
    TransformArgs(const TransformArgs& copy, const Matrix2F& mat)
        : CullRect(copy.CullRect), Mat3D(Matrix3F::NoInit),
        ViewProj(Matrix4F::NoInit), Mat(mat),Cx(copy.Cx) { CopyViewProj(copy); }

    // Matrix, culling, 3D xform and Cxform. Note: 2D matrix gets set to identity, it may be used in some calculations.
    TransformArgs(const TransformArgs& copy, const Matrix3F& mat )
        : CullRect(copy.CullRect), Mat3D(mat), 
        ViewProj(Matrix4F::NoInit),
        Mat(Matrix2F::Identity), Cx(copy.Cx) {  CopyViewProj(copy);  }

    // Full constructor (use only at TreeRoot).
    TransformArgs(const RectF & cullRect, const Matrix2F& mat2D, const Matrix3F& mat3D, const Cxform& cx)
        : CullRect(cullRect), Mat3D(mat3D),
        viewState(NULL), projState(NULL), ViewProj(Matrix4F::NoInit), bRecomputeViewProj(false),
        Mat(mat2D), Cx(cx) { }

    void SetMatrix3D(const Matrix3F& m) { Mat3D = m; }
    void SetMatrix3D(const Matrix3F& m1, const Matrix3F& m2) { Mat3D.MultiplyMatrix(m1, m2); }
    void AppendMatrix3D(const Matrix3F& m) { Mat3D.Append(m); }
    void PrependMatrix3D(const Matrix3F& m) { Mat3D.Prepend(m); }
    void GetMatrix3D(TransformFlags flags, Matrix3F *m) const
    { 
        if (flags & TF_Has3D)
        {
            if (flags & TF_Has2D) 
                m->MultiplyMatrix(Mat3D, Mat);
            else
                *m = Mat3D;
        }
        else
            *m = Mat;
    }
    const Matrix3F &GetMat3DDirect() const { return Mat3D; }    // only use in specific circumstances when 2D portion can be ignored
    const Matrix4F &GetViewProj() const
    {
        if (bRecomputeViewProj)
        {
            if ( viewState && projState )
                ViewProj = Matrix4F(Matrix4F(projState->GetProjectionMatrix3D()->M), Matrix3F(viewState->GetViewMatrix3D()->M));
            else
                ViewProj.SetIdentity();
            bRecomputeViewProj = false;
        }

        return ViewProj;
    }

    void SetViewProj(const ViewMatrix3DState* vsIn, const ProjectionMatrix3DState* psIn)
    {
        if (vsIn)
        {
            viewState = vsIn;
            bRecomputeViewProj = true;
        }

        if (psIn)
        {
            projState = psIn;
            bRecomputeViewProj = true;
        }
    }

    // apply view and proj states from NodeData if they are there, else get them from the TransformArgs passed in
    void SetViewProj(const TreeNode::NodeData* data, const TransformArgs* t)
    {
        // copy states from passed in args
        if (t)
            CopyViewProj(*t);

        // now copy from NodeData if provided
        if (data)
        {
            const ViewMatrix3DState* vs = data->HasViewMatrix3D() ? data->GetState<ViewMatrix3DState>() : NULL;
            const ProjectionMatrix3DState* ps = data->HasProjectionMatrix3D() ? data->GetState<ProjectionMatrix3DState>() : NULL;
            SetViewProj(vs, ps);
        }
    }

private:
    TransformArgs& operator=(const TransformArgs& ) { return *this; }; // Get rid of warning.

    // conditional copy of view and projection
    void CopyViewProj(const TransformArgs& copy)
    {
            viewState = copy.viewState;
            projState = copy.projState;
            bRecomputeViewProj = copy.bRecomputeViewProj;
            ViewProj = copy.ViewProj;
    }

};

//--------------------------------------------------------------------

class TreeCacheNode : public ListNode<TreeCacheNode>,                      
                      public NewOverrideBase<StatRender_TreeCache_Mem>
{
public:
    TreeCacheRoot*          pRoot;    
    TreeNode*               pNode;
    Renderer2DImpl*         pRenderer2D;
    TreeCacheNode*          pParent;
    CacheEffectChain        Effects;
private:
    TreeCacheNode*          pMask;
public:
    UInt16                  Depth;
private:
    UInt16                  Flags;
public:
    unsigned                UpdateFlags;   // Set when the entry is valid
    TreeCacheNode*          pNextUpdate;
    // Our cached bounds in parent coordinates; obtained from pNode->AproxParentBounds,
    // but potentially enlarged due to masks.    
    RectF                   SortParentBounds;        

public:    

    TreeCacheNode(TreeNode* pnode, Renderer2DImpl* prenderer2D, unsigned flags);
    virtual ~TreeCacheNode();

    unsigned    GetFlags() const { return Flags; }
    void        SetFlags(unsigned flags)
    {
        SF_ASSERT((flags & ~NF_CacheNode_Mask) == 0);
        Flags = (UInt16) flags;
    }
    inline void  ApplyFlags(unsigned clearMask, unsigned setMask)
    {
        SF_ASSERT((setMask & ~NF_CacheNode_Mask) == 0);
        Flags = (UInt16) ((Flags & ~clearMask) | setMask);
    }
    
    inline bool IsMaskNode() const { return (Flags & NF_MaskNode) != 0; }
    inline bool IsDrawn() const
    {
        return (Flags & (NF_Visible|NF_Culled)) == NF_Visible || (Flags&NF_PartOfMask);
    }

    inline bool HasMask() const    { return pMask != 0; }
    TreeCacheNode* GetMask() const { return pMask; }
    
    void        SetMask(TreeCacheNode* pmask)
    {
        SF_ASSERT(pmask);
        pMask = pmask;
        SetFlags(GetFlags()|NF_HasMask);
    }
    void        ClearMask()
    {
        pMask = 0;
        SetFlags(GetFlags()&~NF_HasMask);
    }

    static EdgeAAMode CalcEdgeAAMode(const TreeNode::NodeData* node, TreeCacheNode* parent)
    {
        return CalcEdgeAAMode(node,
            (EdgeAAMode) (parent ? (parent->GetFlags() & NF_EdgeAA_Mask) : NF_EdgeAA_On));
    }
    static EdgeAAMode CalcEdgeAAMode(const TreeNode::NodeData* node, EdgeAAMode parentMode = EdgeAA_On)
    {
        if (parentMode == EdgeAA_Disable)
            return EdgeAA_Disable;
        EdgeAAMode nodeMode = node->GetEdgeAAMode();
        return (nodeMode == EdgeAA_Inherit) ? parentMode : nodeMode;
    }

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeNode::NodeData NodeData;
    inline TreeNode*       GetNode() const     { return pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }

    void                SetParent(TreeCacheNode* p)  { pParent = p; }    
    TreeCacheNode*      GetParent() const            { return pParent; }


    // Helper Renderer2DImpl accessor to avoid including that header
    HAL*                  GetHAL() const;
    PrimitiveFillManager& GetPrimitiveFillManager() const;
    MatrixPool&           GetMatrixPool() const;



    // *** TreeCacheNode Virtual API for nodes.

    // Handles change bits for the node type.
    virtual void    HandleChanges(unsigned changeBits) = 0;
    
    // Updates this node and subtree, inserting it into parent if it's not already there.
    // 'pinsert' can be null is we are only establishing ownership.
    // Update pRoot, Depth, pParent values in all children.
    void            UpdateInsertIntoParent(TreeCacheNode* pparent,
                                           TreeCacheNode* pinsert,
                                           const TreeNode::NodeData* nodeData,
                                           UInt16 depth);
    virtual void    UpdateChildSubtree(const TreeNode::NodeData*, UInt16 depth);
    // Removed node from a cached parent child list, also updating the Primitive.
    // Used by HandleChanges.
    void            RemoveFromParent();
    // Removes this node by clearing out pRoot in the subtree.
    virtual void    HandleRemoveNode();


    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags) = 0;

    void            updateMaskCache(const TreeNode::NodeData* data, UInt16 depth,
                                    bool updateSubtree = true);

    void            updateMaskTransform(TransformArgs& args, TransformFlags flags);
    void            updateEffectChain(BundleEntryRange* contentChain);

    inline void     addParentToDepthPatternUpdate();

    virtual void    propagateMaskFlag(unsigned partOfMask);
    virtual void    propagateScale9Flag(unsigned partOfScale9);
    virtual void    propagateEdgeAA(EdgeAAMode parentEdgeAA);
    virtual void    propagate3DFlag(unsigned parent3D);

    // Updates culling for this class, modifies argument cullRect, and
    // potentially removes the TF_NeedCull flag if everything was culled.
    TransformFlags updateCulling(const TreeNode::NodeData* data, const TransformArgs& t,
                                 RectF* cullRect, TransformFlags flags);

    void updateFilterCache( const TreeNode::NodeData* data, const TransformArgs &t, 
        TransformFlags flags, RectF* cullRect, const Matrix3F& m3, bool is3d );
    void updateBlendCache( const TreeNode::NodeData* data, const TransformArgs &t, 
        TransformFlags flags, RectF* cullRect, const Matrix3F& m3, bool is3d );

    // Calculates bounds of the mask in local coordinates. Return false if no
    // adjustment is necessary (mask is invisible, has no content, etc).
    bool            calcChildMaskBounds(RectF *bounds, TreeCacheNode *child);

    // Calculate the local mask bounds + bounding area matrix to use in mask rendering;
    // reports masking state to use based on oldState.
    MaskEffectState calcMaskBounds(RectF* maskBounds, Matrix2F* boundAreaMatrix,
                                   const Matrix3F& viewMatrix, const Matrix4F& viewProjMatrix);

    // Calculate the local mask bounds + bounding area matrix to use in filter rendering;
    enum CacheableBoundResult
    {
        CacheableBoundResult_CompletelyClipped,
        CacheableBoundResult_PartiallyClipped,
        CacheableBoundResult_FullyVisible
    };
    CacheableBoundResult calcCacheableBounds(RectF* returnBounds, Matrix2F* boundAreaMatrix, 
                                       const Matrix3F& viewMatrix, const Matrix4F& viewProjMatrix, 
                                       RectF* cullRect = 0);

    /*    
    void            CalcScale9GridParameters(RectF* s9gRect, Matrix2F* shapeMtx) const;
    */

    void            CalcViewMatrix(Matrix2F* pviewMatrix) const;
    void            CalcViewMatrix(Matrix3F* pviewMatrix, Matrix4F *pviewProj) const;
    void            CalcCxform(Cxform& dest) const;
    bool            CalcFilterFlag() const;

    Matrix4F        GetViewProj() const;

    TreeCacheNode*  GetScale9GridNode()
    { return 0; // pScale9Bounds.GetPtr() ? pScale9Bounds->pOwner : 0;
    }


    virtual void    UpdateBundlePattern(unsigned flags);

    // Obtains a full BundleEntry pattern chain for the subtree indexed at list.
    // This list includes visibility and mask related entries for this node; it is
    // different from child-only cached list.
    virtual bool    GetPatternChain(BundleEntryRange* range, unsigned flags)
    {
        SF_UNUSED(flags);
        range->Clear(Range_Empty);
        return false;
    }
    // Returns true if the full pattern chain of this node is valid; used to determine
    // the need for parent update queuing.
    virtual bool    IsPatternChainValid() const
    {
        return true;
    }

    virtual void    forceUpdateImages() {}
};


//--------------------------------------------------------------------
// TreeCacheContainer contains a renderer-side cached data associated with TreeContainer,
// including a list of Children. 

class TreeCacheContainer : public TreeCacheNode
{
public:
    List<TreeCacheNode> Children;

    // A range of items.    
    BundleEntryRange CachedChildPattern;

    // Technically type of 'pnode' should be TreeContainer, but we leave it
    // as node since this class is also used as base for TreeCacheShape.
    TreeCacheContainer(TreeNode* pnode, Renderer2DImpl* prenderer2D, unsigned flags)
    : TreeCacheNode(pnode, prenderer2D, flags),
      CachedChildPattern(Range_Invalid)
    { }
    ~TreeCacheContainer();

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeContainer::NodeData NodeData;
    inline TreeContainer*  GetNode() const { return (TreeContainer*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }

    // TreeCacheNode API.
    virtual void    HandleChanges(unsigned changeBits);
    virtual void    HandleRemoveNode();
    virtual void    UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth);

    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);
    
    virtual void    propagateMaskFlag(unsigned partOfMask);
    virtual void    propagateScale9Flag(unsigned partOfScale9);
    virtual void    propagateEdgeAA(EdgeAAMode parentEdgeAA);
    virtual void    propagate3DFlag(unsigned parent3D);

    enum
    {        
        Pattern_DisableMatch = 0x1, // Causes matching to be completely disabled when building patterns.
    };
    void            BuildChildPattern(BundleEntryRange* pattern, unsigned flags);
    virtual void    UpdateBundlePattern(unsigned flags);
    
    virtual bool    GetPatternChain(BundleEntryRange* range, unsigned flags);
    virtual bool    IsPatternChainValid() const;    

    virtual void    forceUpdateImages();
};


//--------------------------------------------------------------------

class TreeCacheRoot : public TreeCacheContainer
{
public:
    // Cached primitives and matrices.
    HAL*            pHAL;    
    RectF           ViewCullRect;
    TreeCacheNode*  pUpdateList;
    
    bool            ViewValid;
    bool            EnableBlendTarget;
    bool            DepthUpdatesChained;    

    DepthUpdateArrayPOD<TreeCacheNode*> DepthUpdates;

    TreeCacheRoot(Renderer2DImpl* prenderer2D, HAL* phal, unsigned flags, TreeRoot* pnode)
        : TreeCacheContainer(pnode, prenderer2D, flags),
          pHAL(phal), pUpdateList(0), ViewValid(false), EnableBlendTarget(false), DepthUpdatesChained(false), 
          DepthUpdates(Memory::GetHeapByAddress(GetThis()), 0)
          
    {
        pRoot = GetThis();
        // Consiter root pattern to be empty
        // (not Invalid, default for container).
        CachedChildPattern.Clear(Range_Empty);
    }
    ~TreeCacheRoot()
    {
        // Remove from renderer root list
        if (pPrev)
        {
            RemoveNode();
            pPrev = pNext = 0;
        }
    }

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeRoot::NodeData NodeData;        
    inline TreeRoot*       GetNode() const { return (TreeRoot*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }


    inline TreeCacheRoot* GetThis() { return this; }



    // Chains the results of AddToUpdate calls into DepthUpdates array.
    void    ChainUpdatesByDepth();

    void    ClearDepthUpdates()
    {
        DepthUpdates.Clear();
        DepthUpdatesChained = false;
    }    

    void AddToUpdate(TreeCacheNode *pnode, unsigned flags);

    // Adds node directly to depth update array. Can also be used after
    // ChainUpdatesByDepth was called for original updates.
    // Intended to tree update rebuilding.
    void AddToDepthUpdate(TreeCacheNode *pnode, unsigned flags);

    virtual void HandleChanges(unsigned changeBits);

    void    UpdateTreeData();
    // Draws all the primitives in the root.
    void    Draw();

};

// TreeCacheNode Inline dependent on root.
inline void  TreeCacheNode::addParentToDepthPatternUpdate()
{
    if (pRoot && GetParent())
        pRoot->AddToDepthUpdate(GetParent(), Update_Pattern);
}


//--------------------------------------------------------------------
class DrawableBundle;

class MeshKey;
extern VertexFormat MeshVertex1Format;

class TreeCacheMeshBase : public TreeCacheNode
{
protected:
    // Updates computed final matrix, in UpdateTransform calls
    virtual void ComputeFinalMatrix(const TransformArgs& t, TransformFlags flags);

public:
    // Our node in the parent batch
    BundleEntry SorterShapeNode;

    // Pool matrix handle used for us in bundles; used to initialize
    // and update matrices.
    HMatrix            M;
    
   // inline DrawableBundle* GetBundle() const;

    inline TreeCacheMeshBase*  GetThis() { return this; }

    TreeCacheMeshBase(TreeNode* pnode, const SortKey& key,
                      Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheMeshBase();

    const Matrix2F&   GetMatrixSafe()
    {
        if (!pNode) return Matrix2F::Identity;
        return GetNodeData()->M2D();
    }
    

    // Assigns an inserts a mesh into a specified location.
    virtual MeshBase*   GetMesh() = 0;    
    virtual void        RemoveMesh() = 0;

    // Removes this node by clearing out pRoot in the subtree.
    virtual void    HandleRemoveNode()
    {
        TreeCacheNode::HandleRemoveNode();
        //pRoot = 0;
        SorterShapeNode.Removed = true;        
    }

    // TreeCacheNode Virtual API.
    virtual void    HandleChanges(unsigned changeBits);
    virtual bool    GetPatternChain(BundleEntryRange* range, unsigned flags);
    virtual void    UpdateBundlePattern(unsigned flags);

};

//---------------------------------------------------------------------------------------
template<class C>
class FixedSizeArray
{
protected:
    enum { DefSize = 32 };
    UByte DataReserve[(sizeof(C))*DefSize + 16];
    UPInt Size;
    UPInt Reserve;
    C*    pData;

    bool grow(UPInt reserve)
    {        
        UPInt newReserve = (reserve + DefSize-1) & ~(DefSize-1);        
        C* p = (C*)SF_MEMALIGN(newReserve * sizeof(C), 16, StatRender_TreeCache_Mem);
        if (p)
        {
            memcpy(p, pData, Size * sizeof(C));
            if (pData != (C*)((reinterpret_cast<size_t>(DataReserve)+15) & ~15))
                SF_FREE(pData);
            pData = p;
            Reserve = newReserve;
            return true;
        }
        return false;
    }

public:
    FixedSizeArray(): Size(0), Reserve(DefSize)
    {
        pData = (C*)((reinterpret_cast<size_t>(DataReserve) + 15) & ~15);
    }
    ~FixedSizeArray()
    {
        Clear();
        if (pData != (C*)((reinterpret_cast<size_t>(DataReserve)+15) & ~15))
            SF_FREE_ALIGN(pData);
    }

    UPInt GetSize() const { return Size; }

    C& operator [] (UPInt index)             { return pData[index]; }
    const C& operator [] (UPInt index) const { return pData[index]; }

    void PushBack(const C& val)
    {
        // Allow for fast growth; ok since its temp store.
        if (Size == Reserve)
            grow(Size * 2);
        Scaleform::Construct<C>(pData + Size, val);
        Size++;
    }

    void Clear()
    {
        for (UPInt i = 0; i<Size; i++)
            (operator [] (Size-1)).~C();
        Size = 0;
    }
};

class FixedSizeArrayRectF : public FixedSizeArray<RectF>
{
public:
    // Return 'true' if we intersect with at least one
    // rectangle in array.
    bool Intersects(const RectF& bounds);
};

#if defined(SF_ENABLE_SIMD)
// This implementation is roughly +50% faster on X86 then stock Intersects.
// For frogger.swf (Ctrl+G), this improves framerate 895 -> 960fps.
struct Rect2F
{
    SIMD::Vector4f r0;
    SIMD::Vector4f r1;
};

class FixedSizeArrayRect2F : private FixedSizeArray<Rect2F>
{
    bool HalfRect;
public:
    FixedSizeArrayRect2F() : FixedSizeArray<Rect2F>(), HalfRect(false) { }
    void PushBack(const RectF& r);
    bool Intersects(const RectF& bounds);
    void Clear();
};

typedef FixedSizeArrayRect2F FixedSizeArrayRectFImpl;
#else
typedef FixedSizeArrayRectF FixedSizeArrayRectFImpl;
#endif // SF_ENABLE_SIMD


}} // Scaleform::Render

#endif

