/**************************************************************************

PublicHeader:   Render
Filename    :   Render_States.h
Content     :   Declarations of different render states used for render
                tree node configuration.
Created     :   June 24, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_States_H
#define INC_SF_Render_States_H

#include "Render_StateBag.h"
#include "Render_Scale9Grid.h"
#include "Render_Matrix3x4.h"
#include "Render_Matrix4x4.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render {

class FilterSet;
class Filter;

//--------------------------------------------------------------------
// ***** BlendState

// Blend modes supported by Flash.
enum BlendMode
{
    Blend_None          = 0,    // Same as Normal, but means that BlendMode was not set.
    Blend_Normal        = 1,
    Blend_Layer         = 2,
    Blend_Multiply      = 3,
    Blend_Screen        = 4,
    Blend_Lighten       = 5,
    Blend_Darken        = 6,
    Blend_Difference    = 7,
    Blend_Add           = 8,
    Blend_Subtract      = 9,
    Blend_Invert        = 10,
    Blend_Alpha         = 11,
    Blend_Erase         = 12,
    Blend_Overlay       = 13,
    Blend_HardLight     = 14,

    // These do not have equivalents in flash, but are used internally in GFx.
    Blend_Overwrite,          // No blending occurs; a straight blit (RGB).
    Blend_OverwriteAll,       // No blending occurs; a straight blit (RGBA - must use sourceAc=true).
    Blend_FullAdditive,       // Adds both colors (non-multiplied) and alphas.
    Blend_FilterBlend,        // Used when blending filter passes together.
    Blend_Ignore,             // Used to completely ignore the incoming fragment (alpha/erase layer errors)

    Blend_Count,
    Blend_Invalid,          // Used as a special value to end BlendPrimitives, not an actual blend mode (not counted in Blend_Count).
};

class BlendState : public State
{
public:    
    typedef State::Interface_Value Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_BlendMode; }

    BlendState(BlendMode mode) : State(&InterfaceImpl, (void*)mode, NoAddRef) { }

    BlendMode GetBlendMode() const    { return (BlendMode)DataValue; }

    // Returns whether the allocation of a render target is needed when the given blend mode is pushed on the blend stack.
    static bool IsTargetAllocationNeededForBlendMode(BlendMode mode);
};


//--------------------------------------------------------------------
// ***** Scale9State

class Scale9State : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_Scale9; }

    Scale9State(Scale9GridRect* sgrect) : State(&InterfaceImpl, (void*)sgrect)
    { SF_ASSERT(sgrect);}

    RectF GetRect() const
    { 
        Scale9GridRect* sgrect = (Scale9GridRect*)GetData();
        return sgrect->Scale9;
    }
};

//--------------------------------------------------------------------
// ***** MaskNodeState

class TreeNode;

class MaskNodeState : public State
{
public:
    class Interface : public State::Interface
    {
    public:
        Interface() : State::Interface(State_MaskNode) { }
        virtual void AddRef(void* data, RefBehaviour b);
        virtual void Release(void* data, RefBehaviour b);
    };
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_MaskNode; }

    MaskNodeState(TreeNode* node) : State(&InterfaceImpl, (void*)node)
    { SF_ASSERT(node);}

    TreeNode* GetNode() const { return (TreeNode*)pData; }
};

//--------------------------------------------------------------------
// ***** ViewMatrix3DState

class ViewMatrix3DState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_ViewMatrix3D; }

    ViewMatrix3DState(const Matrix3FRef* mat3D) : State(&InterfaceImpl, (RefCountBase<Matrix3F, Stat_Default_Mem>*)mat3D)
    { SF_ASSERT(mat3D);}

    const Matrix3FRef *GetViewMatrix3D() const    { return (Matrix3FRef*)(RefCountBase<Matrix3F, Stat_Default_Mem>*)GetData(); }
};

//--------------------------------------------------------------------
// ***** ProjectionMatrix3DState

class ProjectionMatrix3DState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_ProjectionMatrix3D; }

    ProjectionMatrix3DState(const Matrix4FRef* mat3D) : State(&InterfaceImpl, (RefCountBase<Matrix4F, Stat_Default_Mem>*)mat3D)
    { SF_ASSERT(mat3D);}

    const Matrix4FRef *GetProjectionMatrix3D() const    { return (Matrix4FRef*)(RefCountBase<Matrix4F, Stat_Default_Mem>*)GetData(); }
};

//--------------------------------------------------------------------
// ***** UserDataState
class UserDataState : public State
{
public:
    struct Data : RefCountBase<Data, Stat_Default_Mem>
    {
        enum DataFlags
        {
            Data_String     = 0x01,
            Data_Float      = 0x02,
            Data_Batching   = 0x04
        };
        StringLH RendererString;
        float    RendererFloat;
        bool     BatchingDisabled;
        unsigned Flags;

        Data() : RendererFloat(0), BatchingDisabled(false), Flags(0) {}
    };
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_UserData; }

    UserDataState(const Data* data) : State(&InterfaceImpl, (RefCountBase<Data, Stat_Default_Mem>*)data)
    { SF_ASSERT(data);}

    Data *GetUserData() const    { return (Data*)(RefCountBase<Data, Stat_Default_Mem>*)GetData(); }
};

// Original Mask's parent
class OrigScale9ParentState : public State
{
public:
    class Interface : public State::Interface
    {
    public:
        Interface() : State::Interface(State_OrigScale9Parent) { }
        virtual void AddRef(void* data, RefBehaviour b);
        virtual void Release(void* data, RefBehaviour b);
    };
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_OrigScale9Parent; }

    OrigScale9ParentState(const TreeNode* op) : State(&InterfaceImpl, (void*)op)
    { SF_ASSERT(op);}

    TreeNode *GetNode() const    { return (TreeNode*)pData; }
};

// MaskOwner - This stores a pointer to mask owner TreeNode, without AddRef.
class Internal_MaskOwnerState : public State
{
public:    
    typedef State::Interface_Value Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_Internal_MaskOwner; }

    Internal_MaskOwnerState(TreeNode* owner) : State(&InterfaceImpl, (void*)owner, NoAddRef) { }

    TreeNode* GetMaskOwner() const  { return (TreeNode*)DataValue; }
};


//--------------------------------------------------------------------
// ***** FilterState

// FilterState describes a set of filters that can be associated with Render::TreeNode. 
class FilterState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_Filter; }

    FilterState(FilterSet* filters);

    const FilterSet* GetFilters() const;

    UPInt         GetFilterCount() const;
    const Filter* GetFilter(UPInt index) const;
};

//--------------------------------------------------------------------
// ***** OrigNodeBoundsState
// OrigNodeBoundsState holds the original bounds of a node, before it is modified by filters, etc.
class OrigNodeBoundsState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_OrigNodeBounds; }

    OrigNodeBoundsState(const RectFRef* bounds) : State(&InterfaceImpl, (void*)bounds)
    { SF_DEBUG_ASSERT(bounds, "Cannot use NULL bounds in OrigNodeBoundsState"); }

    const RectFRef* GetBounds() const
    { return (const RectFRef*)GetData(); }
};

}} // Scaleform::Render

#endif
