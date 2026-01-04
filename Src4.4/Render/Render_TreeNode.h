/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TreeNode.h
Content     :   Declarations of different rendering tree nodes.
Created     :   December 19, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TreeNode_H
#define INC_SF_Render_TreeNode_H

#include "Render_Context.h"

#include "Render_CxForm.h"
#include "Render_Matrix2x4.h"
#include "Render_Matrix3x4.h"
#include "Render_Containers.h"
#include "Render_Gradients.h"
#include "Render_Viewport.h"
#include "Render_States.h"

namespace Scaleform { namespace Render {

// A transformable node in the tree
class TreeContainer;

// Declarations needed for TreeNode::NodeData::updateCache.
class TreeCacheNode;
class TreeCacheContainer;
class HAL;


class TreeNode : public Context::Entry
{
public:

    class NodeData : public Context::EntryData
    {
    private:
        // This can be interpreted as either a 2D or 3D matrix (indicated by Is3D())
        // Use M3D(), M2D() accessors, (can't use unions since Matrix classes contain non-trivial ctors)
        Matrix3F    M34;          

    public:
        NodeData(Context::EntryData::EntryType type = ET_Base)
            : Context::EntryData(type, NF_Visible) { }
        // "Copy constructor" for cloning. States aren't copied easily, so leave them for later.
        NodeData(NonlocalCloneArg<NodeData> src)
            : Context::EntryData(NonlocalCloneArg<Context::EntryData>(*src.pC)),
              M34(src->M34), Cx(src->Cx),
              AproxLocalBounds(src->AproxLocalBounds), AproxParentBounds(src->AproxParentBounds) { }

        StateBag    States;
        Cxform      Cx;
        // Propagated LocalBounds approximation. This is not precise since it uses
        // cumulatively enclosed rectangles (not the precise matrix). Intended for
        // use with hit-testing, culling, masking and batch bounds checks.
        RectF    AproxLocalBounds; 
        RectF    AproxParentBounds;
        

        // updateCache - Internal function used for node implementation. Needs to handle 3 cases:
        //  1. Renderer cache node is not created. Create the proper node and process/cache subtree,
        //     adding items into DP.
        //  2. Render cache node created, but has no parent. This means cached node has been removed and
        //     is now being re-added. pParent, pRoot and other variables need to be initialized, DPs inserted.
        //  3. Render node is created and is part of parent. In this case, we are verifying a child node
        //     of a subtree after an ancestor (ex, parent) was re-added. We don't need to do insert/remove,
        //     but we do need to update pRoot/Depth and insert DPs.
        virtual TreeCacheNode*  updateCache(HAL* hal, TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const = 0;

        // expandByFilterBounds - Helper function to expand the local bounds based on a node's FilterSet.
        bool expandByFilterBounds(RectF* bounds, bool boundsEmpty) const;
        static void expandByFilterBounds( const Filter* filter, RectF* bounds );

        // This function compares the AproxLocalBounds with the input, and if they are different, saves the 
        // input in an OrigNodeBoundsState object in this object.
        void updateOriginalBoundState(const RectF& bounds);

        virtual void ReleaseNodes() const;

        void  SetVisible(bool visible)
        {
            Flags = (UInt16)((Flags & ~NF_Visible) | (visible ? NF_Visible : 0));
        }
        bool  IsVisible() const     { return (Flags & NF_Visible) != 0; }

        bool  IsMaskNode() const    { return (Flags & NF_MaskNode) != 0; }
        bool  HasMask() const       { return (Flags & NF_HasMask) != 0; }
        bool  HasFilter() const     { return (Flags & NF_HasFilter) != 0; }
        bool  HasProjectionMatrix3D() const     { return (Flags & NF_HasProjectionMatrix3D) != 0; }
        bool  HasViewMatrix3D() const           { return (Flags & NF_HasViewMatrix3D) != 0; }
        bool  Is3D() const          { return (Flags & NF_3D) != 0; }
        void  Clear3D()             { Flags &= ~NF_3D; }
        unsigned GetFlags() const   { return Flags; }

        EdgeAAMode GetEdgeAAMode() const          { return (EdgeAAMode)(GetFlags() & NF_EdgeAA_Mask); }
        void       SetEdgeAAMode(EdgeAAMode mode) { Flags = (UInt16)((Flags & (~NF_EdgeAA_Mask)) | mode); }

        template<class S>
        const S* GetState() const   { return States.GetState<S>(); }

        TreeNode* GetMaskOwner() const
        {
            const Internal_MaskOwnerState* ownerState = GetState<Internal_MaskOwnerState>();
            return ownerState ? ownerState->GetMaskOwner() : 0;
        }       

        // Matrix 2D/3D accessors
        const Matrix2F &M2D() const { return *(Matrix2F *)(void*)&M34; }
        Matrix2F &M2D()             { return *(Matrix2F *)(void*)&M34; }

        const Matrix3F &M3D() const { return M34; }
        Matrix3F &M3D()             { return M34; }        

        // Don't clear NF_3D flag here, we may be updating the 2D portion only
        void SetMatrix(const Matrix2F &mat)     { *(Matrix2F *)(void*)&M34 = mat; }  

        void SetMatrix3D(const Matrix3F &mat)   { M34 = mat; Flags |= NF_3D; }

        // copies all matrices, cxform, visibility from src to 'this' node.
        virtual void CopyGeomData(TreeNode* destNode, const TreeNode& srcNode);

        // Clone TreeNode, potentially in new context.
        virtual TreeNode* CloneCreate(Context* context) const;
        virtual bool      CloneInit(TreeNode* node, Context* context) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    // copies all matrices, cxform, visibility from src to 'this' node.
    void CopyGeomData(const TreeNode& src);

    // Clone TreeNode and subtree, potentially in new context.
    // Used by DrawableImage to capture tree state. Shared data (such as Shapes and
    // images) will be AddRefed.
    TreeNode* Clone(Context* context) const;

    bool HasChanges(unsigned cb) const 
    { 
        if (GetChange())
            return (GetChange()->ChangeBits & cb) != 0;
        return false;
    }

    void            SetMatrix(const Matrix2F& m); 
    const Matrix2F&   M2D() const
    {
        return GetReadOnlyData()->M2D();
    }

    void            SetMatrix3D(const Matrix3F& m); 
    const Matrix3F& M3D() const
    {
        return GetReadOnlyData()->M3D();
    }

    void SetViewMatrix3D(const Matrix3F& m);
    void ClearViewMatrix3D();
    bool GetViewMatrix3D(Matrix3F *mat) const
    {        
        const ViewMatrix3DState* state = GetState<ViewMatrix3DState>();
        if (!state)
            return false;
        *mat = Matrix3F(state->GetViewMatrix3D()->M);
        return true;
    }

    void SetProjectionMatrix3D(const Matrix4F& m);
    void ClearProjectionMatrix3D();
    bool GetProjectionMatrix3D(Matrix4F *mat) const
    {        
        const ProjectionMatrix3DState* state = GetState<ProjectionMatrix3DState>();
        if (!state)
            return false;

        *mat = Matrix4F(state->GetProjectionMatrix3D()->M);
        return true;
    }

    // Calculates view matrix of this tree node; i.e. the matrix that would
    // transform local coordinates to the coordinates of the root tree node, which
    // typically matches viewport.
    void            CalcViewMatrix(Matrix2F *m) const;


    void            SetCxform(const Cxform& cx)
    {
        NodeData* pdata = GetWritableData(Change_CxForm);
        pdata->Cx = cx;
    }
    const Cxform&   GetCxform() const
    {        
        return GetReadOnlyData()->Cx;
    }

    void            SetVisible(bool visible);
    bool            IsVisible() const
    {
        return GetReadOnlyData()->IsVisible();
    }
    unsigned        GetFlags() const
    {
        return GetReadOnlyData()->GetFlags();
    }

    void            SetEdgeAAMode(EdgeAAMode edgeAA);
    EdgeAAMode      GetEdgeAAMode() const
    {
        return (EdgeAAMode)(GetFlags() & NF_EdgeAA_Mask);
    }

    // General state access.
    const State* GetState(StateType state) const;

    template<class S>
    const S*     GetState() const
    {
        return static_cast<const S*>(GetState(S::GetType_Static()));
    }

    void        SetScale9Grid(const RectF& rect);
    RectF       GetScale9Grid() const
    {        
        const Scale9State* state = GetState<Scale9State>();
        return state ? state->GetRect() : RectF(0);
    }
    bool        HasScale9Grid() const { return GetState<Scale9State>() != NULL; }
    
    void        SetBlendMode(BlendMode mode);
    BlendMode   GetBlendMode() const
    {
        const BlendState* state = GetState<BlendState>();
        return state ? state->GetBlendMode() : Blend_None;
    }

    void        SetMaskNode(TreeNode* node);
    TreeNode*   GetMaskNode() const
    {
        const MaskNodeState* state = GetState<MaskNodeState>();
        return state ? state->GetNode() : 0;
    }

    void              SetFilters(const FilterSet* filters);
    const FilterSet*  GetFilters() const
    {
        const FilterState* state = GetState<FilterState>();
        return state ? state->GetFilters() : 0;
    }

    void              SetOrigScale9Parent(TreeNode* origParent);
    TreeNode*         GetOrigScale9Parent() const
    {
        const OrigScale9ParentState* state = GetState<OrigScale9ParentState>();
        return state ? state->GetNode() : 0;
    }

    // Treat this TreeNode as 2D only
    void        Clear3D();
    // Determine if this TreeNode has a 3D transform
    bool        Is3D() const        { return GetReadOnlyData()->Is3D(); }
    // Determine if this TreeNode is itself a mask node (root of mask applied elsewhere).
    bool        IsMaskNode() const  { return GetReadOnlyData()->IsMaskNode(); }
    // Determine if this node has a mask applied (call GetMaskNode for actual node).
    bool        HasMask() const     { return GetReadOnlyData()->HasMask(); }
    // Determine if this node has any filters applied (call Filters for actual filters).
    bool        HasFilter() const   { return GetReadOnlyData()->HasFilter(); }
    // Determine if this node has a 3D projection
    bool        HasProjectionMatrix3D() const   { return GetReadOnlyData()->HasProjectionMatrix3D(); }
    // Determine if this node has a 3D view matrix
    bool        HasViewMatrix3D() const   { return GetReadOnlyData()->HasViewMatrix3D(); }
    bool        IsPartOfMask()
    {
        TreeNode* node = this;
        while(node)
        {
            if (node->IsMaskNode())
                return true;
            node = node->GetParent();
        }
        return false;
    }

    inline TreeNode*  GetParent() const;

    const RectF& GetAproxLocalBounds() const
    {
        return GetReadOnlyData()->AproxLocalBounds;
    }
    const RectF& GetAproxParentBounds() const
    {
        return GetReadOnlyData()->AproxParentBounds;
    }

    void SetRendererString(const char* str);
    const char* GetRendererString() const;
    void SetRendererFloat(float f);
    float GetRendererFloat() const;
    void DisableBatching(bool b);
    bool IsBatchingDisabled() const;

    // Internal CalcViewMatrix helper.
    void        appendAncestorMatrices(Matrix2F *m) const;

protected:

    friend class NodeData;
    bool        setThisAsMaskOwner(NodeData* thisData, TreeNode* node);
    static bool removeThisAsMaskOwner(const NodeData* thisData);

    // Helpers for sprite, etc
    static void AddRef_Multiple(TreeNode** pnodes, UPInt count)
    {
        while(count--) (*(pnodes++))->AddRef();
    }
    static void Release_Multiple(TreeNode** pnodes, UPInt count)
    {
        while(count--) (*(pnodes++))->Release();
    }
};



//--------------------------------------------------------------------
// ***** TreeNodeArray 

// TreeNodeArray is a custom array class used for TreeNode* storage
// in TreeContainer. This array is thread/Capture aware and utilizes
// copy-on-write to enable thread safety after Capture. User must call
// TreeNodeArray::SignalFrameCapture() at the end of frame to force
// copy-on-write and disable in-place modification.
//
// Special features:
//  - TreeNodeArray uses 2 local fixed-size slots without allocation
//    to reduce memory/allocation overhead when only holding 1 or 2 items.
//  - Array uses in-place resizable capacity to enable fast insert/remove
//    before capture. After capture or re-assignement takes place,
//    copy-on-write becomes necessary before further modifications.
//    Capacity is only tracked while direct modification is permitted.

class TreeNodeArray
{
public:

    enum {
        FixedStoreSize = 2, // Can't be changed without source modification.
        FirstNodeSize  = 2,
        RoundMultiple  = 4, // Must be power-of two. 
        RoundMask      = RoundMultiple-1
    };

    struct ArrayData
    {
        volatile int    RefCount;
        UPInt           Size;
        TreeNode*       pNodes[FirstNodeSize];

        void AddRef()
        {
            AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
        }
        void Release()
        {
            if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
                SF_FREE(this);
        }
    };

    union  {
        TreeNode*  pNodes[FixedStoreSize];
        UPInt      pData[FixedStoreSize]; 
    };

    static bool       hasArrayDataBit(UPInt data)  { return (data & 1) != 0; }
    static ArrayData* toArrayData(UPInt data)      { return (ArrayData*)(data & ~(UPInt)1); }

    bool        hasArrayData() const  { return hasArrayDataBit(pData[0]); }
    ArrayData*  getArrayData() const  { return toArrayData(pData[0]); }
    UPInt       getCapacity() const   { return pData[1]; }

    void        setArrayData(ArrayData* data, UPInt capacity = 0)
    {
        pData[0] = ((UPInt)data) | 1;
        pData[1] = capacity;
    }

    void       setCapacity(UPInt capacity) { pData[1] = capacity; }

    // We round up all sizes up to multiples of 4, plus the two head-slots.
    UPInt       roundUpSize(UPInt size)
    {
        SF_ASSERT(size >= FirstNodeSize);
        return FirstNodeSize + ((size - FirstNodeSize + RoundMask) & ~((UPInt)RoundMask));
    }
    
    // Grow policy is to increase size by 50% as necessary.
    UPInt       calcIncreasedCapacity(UPInt oldSize)
    { return roundUpSize(oldSize + (oldSize >> 1)); }

    // Shrink policy is to only resize down when under 50% utilization.
    UPInt       calcRemoveCapacity(UPInt oldCapacity, UPInt newSize)
    {   
        if ((oldCapacity/2 < newSize) || (oldCapacity <= (FirstNodeSize + RoundMultiple*2)))
            return oldCapacity;
        return roundUpSize(newSize);
    }

    ArrayData*  allocByCapacity(UPInt capacity, UPInt size);

public:

    TreeNodeArray()
    {
        pNodes[0] = pNodes[1] = 0;
    }

    TreeNodeArray(const TreeNodeArray& src);

    ~TreeNodeArray()
    {
        if (hasArrayData())
            getArrayData()->Release();
    }

    void operator = (const TreeNodeArray& src);


    UPInt GetSize() const
    {
        if (!pNodes[0])
            return 0;
        if (hasArrayData())
            return getArrayData()->Size;
        return pNodes[1] ? 2 : 1;
    }

    TreeNode* GetAt(UPInt index) const
    {
        TreeNode*const* nodes;

        // Empty array can't be indexed.
        SF_ASSERT(pNodes[0] != 0);

        if (hasArrayData())
        {
            ArrayData* data = getArrayData();
            SF_ASSERT(index < data->Size);
            nodes = data->pNodes;
        }
        else
        {
            SF_ASSERT(index < FixedStoreSize);
            nodes = pNodes;
        }
        return nodes[index];
    }   

    TreeNode* operator [] (UPInt index) const
    {        
        return GetAt(index);
    }

    TreeNode*const* GetMultipleAt(UPInt index) const;

    bool        Insert(UPInt index, TreeNode* pnode);
    bool        Remove(UPInt index, UPInt count = 1);

    // Must be called during Render::Context::Capture.
    void        SignalFrameCapture()
    {
        if (hasArrayData())
            setCapacity(0);
    }
};



//--------------------------------------------------------------------
// ***** TreeContainer


class TreeContainer : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
        typedef ContextData_ImplMixin<NodeData, TreeNode::NodeData> BaseClass;
    protected:
        NodeData(Context::EntryData::EntryType type)
            :  BaseClass(type) { }
    public:
        NodeData() : BaseClass(ET_Container) { }
        NodeData(NonlocalCloneArg<NodeData> src)
            : BaseClass(NonlocalCloneArg<BaseClass>(*src.pC)) { }
        
        TreeNodeArray Children;


        // Needed for Entry implementation.
        virtual void        ReleaseNodes() const;
        virtual bool        PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(HAL* hal, TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;

        virtual TreeNode* CloneCreate(Context* context) const;
        virtual bool      CloneInit(TreeNode* node, Context* context) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)
   
    bool        Insert(UPInt index, TreeNode* pnode);
    void        Remove(UPInt index, UPInt count);
    bool        Add(TreeNode* node) { return Insert(GetSize(), node); }

    // Child access
    TreeNode* GetAt(UPInt index) const
    {     
        return GetReadOnlyData()->Children.GetAt(index);
    }
    UPInt       GetSize() const
    {        
        return GetReadOnlyData()->Children.GetSize();
    }
   
};

inline TreeNode* TreeNode::GetParent() const
{
    return static_cast<TreeNode*>(Context::Entry::GetParent());
}


//--------------------------------------------------------------------

// Special root node for rendering.
//
// TreeRoot can optionally apply it own Viewport and clear it with background color.
// Viewport is only applied if it is valid (off by default); background is only
// cleared if its alpha channel is not 0.

class TreeRoot : public TreeContainer
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeContainer::NodeData>
    {
    public:
        NodeData() :  ContextData_ImplMixin<NodeData, TreeContainer::NodeData>(ET_Root), BGColor(0) { }
        
        // Viewport and background color for it.
        Viewport VP;
        Color    BGColor;

        Color   GetBackgroundColor() const { return BGColor; }
        bool    HasViewport() const        { return VP.IsValid(); }

    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    // Returns 'true' of TreeRoot has a valid viewport that is used.

    bool HasViewport() const
    {
        return GetReadOnlyData()->HasViewport();
    }
    const Viewport& GetViewport() const
    {
        return GetReadOnlyData()->VP;
    }
    Color           GetBackgroundColor() const
    {
        return GetReadOnlyData()->GetBackgroundColor();
    }

    // Sets a viewport to use for rendering; if this is set, BeginDisplay will be
    // called with these values.
    void SetViewport(const Viewport& vp);

    void SetBackgroundColor(const Color& color)
    {
        if (GetReadOnlyData()->BGColor != color)
        {
            NodeData* data = GetWritableData(Change_Viewport);
            data->BGColor = color;
        }
    }
};


// Convenience typedef for DisplayHandle for TreeRoot.
typedef DisplayHandle<TreeRoot> TreeRootDisplayHandle;


}} // Scaleform::Render

#endif
