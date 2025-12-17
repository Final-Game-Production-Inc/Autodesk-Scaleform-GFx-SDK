/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TreeShape.h
Content     :   TreeShape represents a Flash shape node in a tree.
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_Render_TreeShape_H
#define SF_Render_TreeShape_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_List.h"
#include "Render_TreeNode.h"
#include "Render_ShapeMeshProvider.h"

namespace Scaleform { namespace Render {


class TreeShape : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {      
        typedef ContextData_ImplMixin<NodeData, TreeNode::NodeData> BaseClass;
    public:
        NodeData() : BaseClass(ET_Shape), MorphRatio(0)
        { }
        NodeData(ShapeMeshProvider* meshProvider)
            :  BaseClass(ET_Shape), pMeshProvider(meshProvider), MorphRatio(0)
        { }
        NodeData(NonlocalCloneArg<NodeData> src)
            :  BaseClass(NonlocalCloneArg<BaseClass>(*src.pC)),
               pMeshProvider(src->pMeshProvider), MorphRatio(src->MorphRatio)
        { }
        ~NodeData()
        {
        }
        Ptr<ShapeMeshProvider> pMeshProvider;
        float                  MorphRatio;

        virtual bool            PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;

        // Clone TreeNode, potentially in new context.
        virtual TreeNode* CloneCreate(Context* context) const;
        //virtual bool      CloneInit(TreeNode* node, Context* context) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    ShapeMeshProvider* GetShape() const 
    { 
        return GetReadOnlyData()->pMeshProvider;
    }

    void SetShape(ShapeMeshProvider* pshape) 
    { 
        GetWritableData(Change_Shape)->pMeshProvider = pshape;
        AddToPropagate();
    }

    float GetMorphRatio() const
    {
        return GetReadOnlyData()->MorphRatio;
    }

    void SetMorphRatio(float ratio)
    {
        GetWritableData(Change_MorphRatio)->MorphRatio = ratio;
    }


};


}} // Scaleform::Render

#endif
