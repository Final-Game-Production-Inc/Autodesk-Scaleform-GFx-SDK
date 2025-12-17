/**************************************************************************

Filename    :   Render_TreeMesh.h
Content     :   Defines TreeMesh - TreeNode that displayes triangles.
Created     :   December 19, 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TreeMesh_H
#define INC_SF_Render_TreeMesh_H

#include "Render_TreeNode.h"
#include "Render_Primitive.h"


namespace Scaleform { namespace Render {


class TreeMesh : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
        typedef ContextData_ImplMixin<NodeData, TreeNode::NodeData> BaseClass;
    public:
        NodeData() : BaseClass(ET_Mesh)
        { }
        NodeData(MeshProvider* provider)
            : BaseClass(ET_Mesh), pShape(provider)
        { }
        NodeData(NonlocalCloneArg<NodeData> src)
            :  BaseClass(NonlocalCloneArg<BaseClass>(*src.pC)), pShape(src->pShape)
        { }

        ~NodeData()
        {
        }
        Ptr<MeshProvider> pShape;

        virtual bool            PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;
        virtual TreeNode*       CloneCreate(Context* context) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    inline MeshProvider* GetShape() const 
    { 
        return GetReadOnlyData()->pShape; 
    }

    inline void       SetShape(MeshProvider* pshape) 
    { 
        GetWritableData(Change_Shape)->pShape = pshape;
        AddToPropagate();
    }
};


}} // Scaleform::Render

#endif
