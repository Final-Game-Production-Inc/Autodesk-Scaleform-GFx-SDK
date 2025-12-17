/**************************************************************************

Filename    :   Render_StateBag.cpp
Content     :   Implementation for render State and StateBag classes.
                States are attached to render tree nodes, where they
                are stored in a bag.
Created     :   June 22, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_StateBag.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// Force linking of Render_States.
struct Render_States_ForceLinkClass
{
    Render_States_ForceLinkClass()
    { StateData::InitDefaultStates_ForceLink(); }
};
Render_States_ForceLinkClass Render_States_ForceLinkClassInstance;


// ***** StateData

static State::Interface* StateType_Interfaces[State_Type_Count] = { 0 };

StateData::Interface* StateData::Interface::FromStateType(StateType type)
{
    SF_ASSERT(type < State_Type_Count);
    return StateType_Interfaces[type];
}

StateData::Interface::Interface(StateType type) : Type(type)
{
    SF_ASSERT((type < State_Type_Count) && (StateType_Interfaces[type]==0));
    StateType_Interfaces[type] = getThis();
}


inline void StateData::ArrayData::AddRef()
{
    AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
}
inline void StateData::ArrayData::Release(UPInt count)
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
    {
        // Release ref-count of items.
        State* states = GetStates();
        for(;count; count--, states++)        
            states->getInterface_Unsafe()->Release(states->pData, Interface::Ref_NoTreeNode);
        SF_FREE(this);
    }
}
inline State* StateData::ArrayData::GetStates()
{
    return (State*)(this + 1);
}


// StateBag methods - in StateData due its generality.

void StateData::addRefBag_NotEmpty() const
{
    SF_ASSERT(ArraySize != 0);
    if (isSingleItem())
        getInterface()->AddRef(pData, Interface::Ref_NoTreeNode);
    else
        pArray->AddRef();
}

void StateData::destroyBag_NotEmpty()
{
    SF_ASSERT(ArraySize != 0);
    if (isSingleItem())
        getInterface()->Release(pData, Interface::Ref_NoTreeNode);
    else
        pArray->Release(getArraySize());

    // NULL out the interface/data (or the Array and ArraySize). It is possible that
    // either of the above will cause the data to be destroyed, however, we will still have
    // a dangling pointer to them.
    pInterface = 0;
    pData = 0;
}

void StateData::assignBag(const StateData& src)
{
    if (!src.isEmpty())
        src.addRefBag_NotEmpty();
    if (!isEmpty())
        destroyBag_NotEmpty();
    pInterface = src.pInterface;
    pData      = src.pData; 
}



//--------------------------------------------------------------------
// ***** StateBag implementation

const State* StateBag::GetState(StateType type) const
{
     if (isEmpty())
         return 0;
     // Map state type to interface pointer, so that we can search for it.
     State::Interface* pi = Interface::FromStateType(type);
     if (isSingleItem())
         return (getInterface() == pi) ? toState() : (const State*)0;
    
     UPInt  count = getArraySize();
     State* states = pArray->GetStates();
     
     // Search for state in the array.
     for (UPInt i = 0; i < count; i++)
         if (states[i].pInterface == pi)
             return &states[i];
     return 0;
}


void StateBag::SetStateVoid(Interface* pi, void* data)
{
    if (isEmpty())
    {
        initSingleItem(pi, data);
        pi->AddRef(data, Interface::Ref_All);
        return;
    }

    if (isSingleItem())
    {
        if (getInterface() == pi)
        {
            pi->AddRef(data, Interface::Ref_All);
            pi->Release(pData, Interface::Ref_All);
            pData = data;
            return;
        }
        
        // Must resize to array.
        ArrayData* arr = allocData(0, 0, 2);
        if (!arr) return;
        // Transfer ref-count from our body to array.
        arr->GetStates()[0].initArrayItem(getInterface(), pData);
        arr->GetStates()[1].initArrayItem(pi, data);
        pi->AddRef(data, Interface::Ref_All);
        initArray(arr, 2);
        return;
    }

    // Array resize.
    UPInt  count = getArraySize();
    State* states = pArray->GetStates();

    for (UPInt i = 0; i < count; i++)
    {
        if (states[i].pInterface == pi)
        {
            pi->AddRef(data, Interface::Ref_All);
            pi->Release(states[i].pData, Interface::Ref_All);
            states[i].pData = data;
            return;
        }
    }

    ArrayData* arr = allocData(states, count, 1);
    if (!arr) return;

    arr->GetStates()[count].initArrayItem(pi, data);
    pi->AddRef(data, Interface::Ref_All);
    pArray->Release(count);
    initArray(arr, count+1);
}


bool StateBag::RemoveState(StateType type)
{
    if (isEmpty())
        return false;
    
    Interface* pi = Interface::FromStateType(type);

    if (isSingleItem())
    {
        if (getInterface() == pi)
        {            
            pi->Release(pData, Interface::Ref_All);
            pInterface = 0;
            pData = 0;
            return true;
        }
        return false;
    }

    // First, find the item to remove.
    UPInt  count = getArraySize();
    State* states = pArray->GetStates();
    UPInt  i;

    for (i = 0; i < count; i++)    
        if (states[i].pInterface == pi)
            break;
    if (i == count)
        return false;

    ArrayData* a = pArray;
    State*     us = a->GetStates() + i;
    
    if (count == 2)
    {
        // Turn into single item.
        State* other = a->GetStates() + (i ^ 1);
        initSingleItem(other->getInterface_Unsafe(), other->GetData());
        other->getInterface_Unsafe()->AddRef(other->GetData(), Interface::Ref_NoTreeNode);
    }
    else
    {
        // Resize array (this copies all but the removed item).
        ArrayData* newArray = allocData2(a->GetStates(), i,
                                         a->GetStates()+i+1, count-i-1);
        if (!newArray) return false;
        initArray(newArray, count-1);
    }

    // Since Array will release all but our TreeNode, release the tree node.
    us->getInterface_Unsafe()->Release(us->GetData(), Interface::Ref_ReleaseTreeNodeOnly);
    a->Release(count);
    return true;
}

const State& StateBag::GetAt(UPInt index) const
{
    SF_ASSERT(!isEmpty());

    if (isSingleItem())
    {
        SF_ASSERT(index == 0);
        return *toState();
    }

    SF_ASSERT(index < getArraySize());
    return pArray->GetStates()[index];
}

// Needed for TreeNode::NodeData data implementation,
// which has custom lifetime cleanup protocol.
void StateBag::ReleaseNodes() const
{
    if (isEmpty())
        return;

    if (isSingleItem())
    {
        getInterface()->Release(pData, Interface::Ref_ReleaseTreeNodeOnly);
    }
    else
    {        
        UPInt  count = getArraySize();
        State* state = pArray->GetStates();
        
        for (; count; count--, state++) 
        {
            state->getInterface_Unsafe()->
                Release(state->pData, Interface::Ref_ReleaseTreeNodeOnly);
        }
    }
}


StateBag::ArrayData* StateBag::allocData(State* source, UPInt count, UPInt extra)
{    
    UPInt       size = sizeof(ArrayData) + sizeof(State) * (count + extra);
    ArrayData*  data = (ArrayData*)SF_HEAP_AUTO_ALLOC(this, size);
    if (!data) return 0;

    // Must be AddRefed!
    data->RefCount = 1;
    copyArrayAddRef(data->GetStates(), source, count);
    return data;
}

StateBag::ArrayData* StateBag::allocData2(State* source1, UPInt count1,
                                          State* source2, UPInt count2)
{
    ArrayData*  data = allocData(source1, count1, count2);
    if (!data) return 0;
    copyArrayAddRef(data->GetStates() + count1, source2, count2);
    return data;
}

void StateBag::copyArrayAddRef(State* d, State* s, UPInt count)
{
    for (; count; count--, s++, d++)
    {
        Interface *pi = s->getInterface_Unsafe();
        d->initArrayItem(pi, s->pData);
        pi->AddRef(s->pData, Interface::Ref_NoTreeNode);
    }
}

StateBag::StateBag( const StateBag& src ) : StateData(0, 0)
{
    if (!src.isEmpty())
    {
        if (src.isSingleItem())
        {
            src.getInterface()->AddRef(src.pData, Interface::Ref_NoTreeNode);
            pInterface = src.pInterface;
            pData      = src.pData;
        }
        else
        {
            // NOTE: We cannot simply AddRef to the src's ArrayData here, as it may not be 'snapshot' safe.
            // There are several situations which will lead states being destroyed as they are accessed,
            // such as: D.CopyDisplaying(C), A.ChangeState, C.Merge(A), (while D is still being displayed).
            pArray = allocData(src.pArray->GetStates(), src.getArraySize());
            ArraySize = src.ArraySize;
        }
    }
}


}} // Scaleform::Render

