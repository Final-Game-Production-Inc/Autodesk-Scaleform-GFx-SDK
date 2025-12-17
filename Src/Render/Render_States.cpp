/**************************************************************************

Filename    :   Render_States.cpp
Content     :   Implementation of different render states used for render
                tree node configuration.
Created     :   June 24, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_States.h"
#include "Render_TreeNode.h"
#include "Render/Render_Filters.h"

namespace Scaleform { namespace Render {


State::Interface_Value StateNone_InterfaceImpl(State_None);

void StateData::InitDefaultStates_ForceLink()
{
}

//--------------------------------------------------------------------

// ***** BlendState
BlendState::Interface BlendState::InterfaceImpl(State_BlendMode);

bool BlendState::IsTargetAllocationNeededForBlendMode(BlendMode mode)
{
    switch(mode)
    {
        default:
            return false;

        // These require sampling the target, or cannot be achieved through regular blending operations (at least, on most platforms)
#if !defined(SF_RENDER_DARKEN_LIGHTEN_OLD_BEHAVIOR)
        case Blend_Lighten:
        case Blend_Darken:
#endif
        case Blend_Layer:
        case Blend_Difference:
        case Blend_Overlay:
        case Blend_HardLight:
            return true;


    }
}

// ***** Scale9State
Scale9State::Interface Scale9State::InterfaceImpl(State_Scale9);

// ***** ViewMatrix3DState
ViewMatrix3DState::Interface ViewMatrix3DState::InterfaceImpl(State_ViewMatrix3D);

// ***** ProjectionMatrix3DState
ProjectionMatrix3DState::Interface ProjectionMatrix3DState::InterfaceImpl(State_ProjectionMatrix3D);

// ***** UserDataState
UserDataState::Interface UserDataState::InterfaceImpl(State_UserData);

// ***** OrigScale9Parent
void OrigScale9ParentState::Interface::AddRef(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->AddRef();
}
void OrigScale9ParentState::Interface::Release(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->Release();
}

OrigScale9ParentState::Interface OrigScale9ParentState::InterfaceImpl;

// ***** MaskNodeState
MaskNodeState::Interface MaskNodeState::InterfaceImpl;

void MaskNodeState::Interface::AddRef(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->AddRef();
}
void MaskNodeState::Interface::Release(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->Release();
}

// ***** Internal_MaskOwnerState
Internal_MaskOwnerState::Interface Internal_MaskOwnerState::InterfaceImpl(State_Internal_MaskOwner);


// ***** FilterState
FilterState::Interface FilterState::InterfaceImpl(State_Filter);

// ***** OrigNodeBoundsState
OrigNodeBoundsState::Interface OrigNodeBoundsState::InterfaceImpl(State_OrigNodeBounds);

FilterState::FilterState( FilterSet* filters ) : State(&InterfaceImpl, (void*)filters)
{
    filters->Freeze();
}

const FilterSet* FilterState::GetFilters() const
{
    return (const FilterSet*)GetData();
}

Scaleform::UPInt FilterState::GetFilterCount() const
{
    return GetFilters()->GetFilterCount();
}

const Filter* FilterState::GetFilter( UPInt index ) const
{
    return GetFilters()->GetFilter(index);
}

}} // Scaleform::Render
