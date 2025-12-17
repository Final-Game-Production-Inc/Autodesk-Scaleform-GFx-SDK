/**************************************************************************

Filename    :   Render_TreeCacheText.h
Content     :   TreeText Cache implementation header
Created     :   2009-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_Render_TreeCacheText_H
#define SF_Render_TreeCacheText_H

#include "Render_TreeText.h"
#include "Render_TreeCacheNode.h"
#include "Render_TextMeshProvider.h"

namespace Scaleform { namespace Render {

class TextMeshProvider;

//------------------------------------------------------------------------
// ***** TreeCacheText

class TreeCacheText : public TreeCacheMeshBase
{
friend class TreeCacheShape;
public:
    // Construct TreeCacheShapeLayer.
    //  pnode may be null if we are a child under 
    TreeCacheText(TreeText* pnode, Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheText();
    

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeText::NodeData NodeData;
    inline TreeText*       GetNode() const { return (TreeText*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }


    void            UpdateCache(TreeCacheContainer* pparent, UInt16 depth);

    virtual void    UpdateChildSubtree(const TreeNode::NodeData*, UInt16 depth);
    
    // For nested layer, these will receive matrix and node from TreeCacheShape.
    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);
    virtual void    HandleChanges(unsigned changeBits);

    virtual void    UpdateDistanceFieldUniforms();

    virtual void    propagateMaskFlag(unsigned partOfMask);

    //virtual RectF   CalcScale9GridBounds(const Matrix2F &m);
    //virtual void    UpdateScale9GridShapeMatrix();

    // Assigns an inserts a mesh into a specified location.
    virtual MeshBase* GetMesh() { SF_ASSERT(0); return 0; }
    virtual void      RemoveMesh() {}

    // TBD: Used by TextPrimitiveBundle instead of GetMesh()..
    TextMeshProvider* GetMeshProvider();
    TextMeshProvider* CreateMeshProvider();

    inline Bundle* GetBundle() const
    {
        return SorterShapeNode.pBundle.GetPtr();
    }

    // Used by TextPrimitiveBundle.
    TreeCacheText*   pNextNoBatch;

protected:
    virtual void propagateEdgeAA(EdgeAAMode parentEdgeAA);
    virtual void propagate3DFlag(unsigned parent3D);

    void getMatrix4F(Matrix4F* m4, const Matrix4F &viewProj) const;
    unsigned calcMeshGenFlags() const;
    virtual void forceUpdateImages();

    TextMeshProvider TMProvider;
};


}} // Scaleform::Render

#endif
