/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Constants.h
Content     :   Constants and flags used by TreeNode/TreeCacheNode.
Created     :   July 10, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Constants_H
#define INC_SF_Render_Constants_H

namespace Scaleform { namespace Render {


//--------------------------------------------------------------------
// ***** EdgeAAMode

// EdgeAAMode specified EdgeAA state as configured per TreeNode, as can
// be applied by TreeNode::SetEdgeAAMode() or GFx::Movie::SetEdgeAAMode.
// By default Inherit mode should be used by TreeNode, being changed based
// on this function call.

// EdgeAAMode flag values match parts of TreeNodeFlags.

enum EdgeAAMode
{
    EdgeAA_Inherit = 0x0000,    // Take EdgeAA setting from parent; On by default.
    EdgeAA_On      = 0x0004,    // Use EdgeAA for this node and its children, unless disabled.
    EdgeAA_Off     = 0x0008,    // Do not use EdgeAA for this node or its children.
    EdgeAA_Disable = 0x000C     // Disable EdgeAA for this node and subtree, overriding On settings.
};


//--------------------------------------------------------------------
// ***** TreeNodeFlags

// Node flags are stored in TreeNode::NodeData and in TreeCacheNode. In case of cache,
// flags are copied over from NodeData during change processing.
// There are some flags that are present only in TreeCacheNode.

enum TreeNodeFlags
{
    // *** Flags for Both Tree and Cache Nodes.
    NF_Visible              = 0x0001,
    NF_HasMask              = 0x0010,
    NF_MaskNode             = 0x0020,
    NF_3D                   = 0x0200,       // indicates that the node has a 3D transform
    NF_HasFilter            = 0x0400,       // indicates that the node has filter applied
    NF_HasViewMatrix3D      = 0x0800,       // indicates that the node has a ViewMatrix set
    NF_HasProjectionMatrix3D= 0x1000,       // indicates that the node has a ProjMatrix set
    NF_HasOrigScale9Parent  = 0x2000,       // indicates that the node has a original mask's parent set
    NF_3DChild              = 0x4000,       // indicates that the node has a child which has a 3D transform


    // EdgeAA flags are stored TreeNode, with cache values propagated through
    // TreeCacheNodes. The Inherit flag should be a part of TreeNode but NOT TreeCacheNode,
    // as it should be replaced due to propagation.
    NF_EdgeAA_Inherit   = EdgeAA_Inherit,
    NF_EdgeAA_On        = EdgeAA_On,
    NF_EdgeAA_Off       = EdgeAA_Off,
    NF_EdgeAA_Disable   = EdgeAA_Disable,
    NF_EdgeAA_Mask      = 0x000C,

    // *** Cache Node Only
    NF_Culled           = 0x0002,
    
    // PartOfMask is maintained in nodes to allow creation of proper SortKey for
    // shapes and custom mask-specific fills. It is set for MaskNode an all of
    // its children and initialized as follows:
    //  1. On TreeCacheNode create, so as to ensure correct SortKey initialization.
    //  2. In RemoveFromParent, recursively through propagateMaskFlag.
    //  3. In UpdateInsertIntoParent, recursively through propagateMaskFlag.
    NF_PartOfMask       = 0x0040, // NF_MaskNode << 1.
    // This flag is set when cache node or one of its ancestors has Scale9 attached.
    // Propagated the same way as NF_PartOfMask, but also on Change_State_Scale9 change.
    NF_PartOfScale9     = 0x0080,

    NF_PartOf3D         = 0x0100,
    NF_PartOfFilter     = 0x0200,

    // This flag is set if SortParentBounds have been expanded,
    // typically to accommodate a mask.
    NF_ExpandedBounds   = 0x0400,

    // Masks
    NF_TreeNode_Mask    = NF_Visible | NF_HasMask | NF_MaskNode | NF_3D | NF_EdgeAA_Mask | NF_HasFilter |
                          NF_HasViewMatrix3D | NF_HasProjectionMatrix3D | NF_HasOrigScale9Parent | NF_3DChild,
    NF_CacheNode_Mask   = NF_TreeNode_Mask | 
                          NF_Culled | NF_PartOfMask | NF_PartOfScale9 | NF_ExpandedBounds | NF_3D |
                          NF_HasOrigScale9Parent | NF_PartOf3D | NF_PartOfFilter
};

// Helper function: maps NF_MaskNode flag to NF_PartOfMask.
inline unsigned NF_MaskNode_MapTo_PartOfMask(unsigned flags)
{    
    return (flags & NF_MaskNode) << 1;
}



//--------------------------------------------------------------------
// ***** TreeChangeConstants

// These flag constants are used for Change and Update tracking within
// Context::EntryChange, TreeNode, and TreeCacheNode objects.
//
// Constants are separated into the following groups:
//  1. Change_Context - Used by Render::Context for change tracking.
//  2. Change_*       - Flags describing internal change to TreeNode,
//                      copied into TreeCacheNode::UpdateFlags as needed.
//  3. Change_State_* - Flags describing a change to TreeNode-attached
//                      states, copied to UpdateFlags and potentially
//                      used to update CacheEffect chain of the node.
//  4. Update_*       - Flags used internally for TreeCacheNode update;
//                      these exist as a part of UpdateFlags only.

enum TreeChangeConstants
{
    // Used by Context::EntryChange
    Change_Context_NewNode  = 0x80000000,
    Change_Context_Mask     = Change_Context_NewNode,

    // Change flags reserved for the render node. Having these flags
    // in the base allows change lists to be processed more efficiently
    Change_Matrix           = 0x00000001,
    Change_CxForm           = 0x00000002,
    Change_Visible          = 0x00000004,
    Change_AproxBounds      = 0x00000008,
    Change_MorphRatio       = 0x00000010,
    Change_EdgeAAMode       = 0x00000020,
    Change_IsMask           = 0x00000080,

    // Container.
    Change_ChildInsert      = 0x00000100,
    Change_ChildRemove      = 0x00000200,
    Change_Shape            = 0x00000400,
    Change_TextLayout       = Change_Shape, // Alias
    Change_Viewport         = 0x00001000,
    Change_3D               = 0x00002000,
    Change_Node_Mask        = 0x0000FFFF,

    // TreeNode State flags; set when state is modified in a StateBag.    
    Change_State_Scale9             = 0x00010000,
    Change_State_BlendMode          = 0x00020000,
    Change_State_MaskNode           = 0x00040000,
    Change_State_ViewMatrix3D       = 0x00080000,
    Change_State_ProjectionMatrix3D = 0x00100000,
    Change_State_Filter             = 0x00200000,
    Change_State_UserData           = 0x00400000,
    Change_State_Mask               = 0x00FF0000,

    // Mask that combines all states that may require an effect update
    // for the tree node.
    Change_State_Effect_Mask= Change_State_BlendMode | Change_State_MaskNode | Change_State_Filter | 
                              Change_State_ViewMatrix3D | Change_State_ProjectionMatrix3D | Change_State_UserData,
    
    // Change mask copied to UpdateFlags.
    //Change_Mask             = Change_Node_Mask | Change_State_Mask,
    // Change for which HandleChnages is called.
    Change_HandleChanges_Mask = Change_ChildInsert | Change_ChildRemove | Change_EdgeAAMode |
                                Change_Shape | Change_TextLayout | Change_MorphRatio | Change_3D | Change_Viewport,

    // Flags indicating that pattern needs to be Verified/Rebuilt.
    Update_Pattern          = 0x01000000,  // Pattern change due to child Add/Remove, culling, etc.
    Update_PatternBounds    = 0x02000000,  // Pattern needs to be verified due to child bounds change.
    Update_Pattern_Mask     = 0x03000000,

    // Marks that the entry is in update list (either linked list or Depth list chain).
    // It's ok for Update_ bits to use same bits as Change_Context.
    Update_InList           = 0x80000000
};


}} // Scaleform::Render

#endif
