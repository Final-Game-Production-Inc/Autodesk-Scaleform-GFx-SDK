/**************************************************************************

PublicHeader:   Render
Filename    :   Render_StateBag.h
Content     :   Declaration for render State and StateBag classes.
                States are attached to render tree nodes, where they
                are stored in a bag.
Created     :   June 22, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_StateBag_H
#define INC_SF_Render_StateBag_H

#include "Kernel/SF_RefCount.h"

namespace Scaleform { namespace Render {

class State;
class StateBag;

//--------------------------------------------------------------------
// ***** StateType Constants

// StateType describes states that can be attached to a TreeNode, where
// it is stored as a part of StateBag. Each state is typically represented
// by a custom class derived from State.
enum StateType
{
    State_None,
    State_BlendMode,
    State_Scale9,
    State_Filter,
    State_MaskNode,
    State_ViewMatrix3D,
    State_ProjectionMatrix3D,
    State_UserData,
    State_OrigScale9Parent,
    // Internal states should not be manipulated directly.
    State_Internal_MaskOwner, 
    State_OrigNodeBounds,
    State_Type_Count
};


//--------------------------------------------------------------------
// StateData is an internal class implementing state and/or StateBag
// as single data item.

class StateData
{
    friend class State;
    friend class StateBag;    
    // private destructor - this shouldn't be called directly.
    ~StateData() { }

public:

    // Interface implements lifetime management for a State of a given
    // type. Only one instance of this class is created for each registered
    // state type.
    class Interface
    {
        StateType   Type;
        inline Interface* getThis() { return this; }
    public:
        Interface(StateType type);
        virtual ~Interface() {}

        inline StateType GetType() const { return Type; }

        // Obtains StateInterface singleton for the specified state type.
        static Interface* FromStateType(StateType type);

        // RefBehaviour describes action expected from AddRef/Release,
        // and is needed for proper TreeNode support in StateBag, where nodes need
        // custom ref-counting for proper Advance/Display thread interaction.
        // If behavior indicates that no action is to take place for this
        // interface StatType, data argument shouldn't be touched.
        enum RefBehaviour
        {
            Ref_All,                // AddRef/Release unconditionally.
            Ref_NoTreeNode,         // AddRef/Release only if this is NOT TreeNode.
            Ref_ReleaseTreeNodeOnly // Release only if this is TreeNode.
        };
        virtual void     AddRef(void*, RefBehaviour) = 0;
        virtual void     Release(void*, RefBehaviour) = 0;
    };

    // Interface implementation for data stored as a value (no ref-counting needed).
    class Interface_Value : public Interface
    {
    public:
        Interface_Value(StateType type) : Interface(type) { }

        virtual void AddRef(void*, RefBehaviour) { }
        virtual void Release(void*, RefBehaviour) { }
    };

    // Interface implementation for data stored as RefCountImpl-derived class;
    // default thread-safe ref-counting is used.
    class Interface_RefCountImpl : public Interface
    {
    public:
        Interface_RefCountImpl(StateType type) : Interface(type) { }

        virtual void AddRef(void* data, RefBehaviour)
        {
            ((RefCountImpl*)data)->AddRef();
        }
        virtual void Release(void* data, RefBehaviour b)
        {
            if (b != Ref_ReleaseTreeNodeOnly)
                ((RefCountImpl*)data)->Release();
        }
    };

    // Defined for force linking to Render_States.cpp.
    static void InitDefaultStates_ForceLink();

protected:
    // NO AddRef - these are external!
    StateData(Interface* i = 0, void* data = 0)
    {
        pInterface = i;
        pData      = data;
    }
    StateData(const StateData &src)
    {
        pInterface = src.pInterface;
        pData      = src.pData;
    }

    struct ArrayData
    {
        volatile int RefCount;

        inline void   AddRef();
        inline void   Release(UPInt count);
        inline State* GetStates();
    };

    // Pointer to the interface, if any. Lower bit of this 
    // state may define container size.
    union {
        Interface* pInterface;
        UPInt      ArraySize;
    };
    union {
        void*      pData;
        UPInt      DataValue;
        ArrayData* pArray;
    };

    Interface* getInterface() const        { return (Interface*) (ArraySize & ~(UPInt)1); }
    Interface* getInterface_Unsafe() const { SF_ASSERT(!(ArraySize & 1)); return pInterface; }

    void       initSingleItem(Interface* i, void* data) { ArraySize = ((UPInt)i) | 1; pData = data; }
    void       initArrayItem(Interface* i, void* data) { pInterface = i; pData = data; }
    void       initArray(ArrayData* arr, UPInt size) { ArraySize = size << 1; pArray = arr; }

    bool       isEmpty() const       { return ArraySize == 0; }
    bool       isSingleItem() const  { return (ArraySize & 1) != 0; }
    UPInt      getArraySize() const  { SF_ASSERT(!(ArraySize & 1)); return ArraySize >> 1; }

    void       addRefBag_NotEmpty() const;
    void       destroyBag_NotEmpty();
    void       assignBag(const StateData& src);
};



//--------------------------------------------------------------------
// ***** State
//--------------------------------------------------------------------

// Single state data item, implemented as an {Interface, UPInt} pair,
// with the later part being interpreted based on an interface.

class State : public StateData
{
public:
    Interface* GetInterface() const { return getInterface(); }
    void*      GetData() const      { return pData; }

    StateType  GetType() const 
    {
        Interface* i = getInterface();
        return i ? i->GetType() : State_None;
    }

    enum NoAddRefType { NoAddRef };

    State(Interface* i, void* data, NoAddRefType) : StateData(i, data) { }

    State(Interface* i, void* data) : StateData(i, data)
    {
        if (i) i->AddRef(data, Interface::Ref_All);
    }
    
    State(const State& other) : StateData(other)
    {
        if (getInterface_Unsafe())
            getInterface_Unsafe()->AddRef(pData, Interface::Ref_All);
    }

    ~State()
    {
        if (getInterface_Unsafe())
            getInterface_Unsafe()->Release(pData, Interface::Ref_All);
    }
};



//--------------------------------------------------------------------
// ***** StateBag
//--------------------------------------------------------------------

// StateBag is a collection of states stored within TreeNode::NodeData.

class StateBag : private StateData
{
public:
    StateBag() : StateData() { }

    StateBag(const StateBag& src);
    ~StateBag()
    {
        if (!isEmpty())
            destroyBag_NotEmpty();
    }

    void operator = (const StateBag& src)
    {   // Optimize empty assignment, as it is common.
        if ((ArraySize|src.ArraySize) != 0)
            assignBag(src);
    }
    
    // Obtained state is only valid until you modify array.
    const State* GetState(StateType type) const;
    void         SetStateVoid(Interface* i, void* data);
    void         SetState(Interface* i, RefCountImpl* data) { SetStateVoid(i, data); }
    void         SetState(const State& s) { SetStateVoid(s.GetInterface(), s.GetData()); }
    bool         RemoveState(StateType type);
    
    bool         IsEmpty() const { return isEmpty(); }
    UPInt        GetSize() const { return isSingleItem() ? 1 : getArraySize(); }
    const State& GetAt(UPInt index) const;
    const State& operator [] (UPInt index) const { return GetAt(index); }

    template<class S>
    const S*     GetState() const
    {
        return static_cast<const S*>(GetState(S::GetType_Static()));
    }

    // Needed for TreeNode::NodeData data implementation,
    // which has custom lifetime cleanup protocol.
    void         ReleaseNodes() const;

protected:
    ArrayData*  allocData(State* pnodes, UPInt count, UPInt extra = 0);
    ArrayData*  allocData2(State* source1, UPInt count1, State* source2, UPInt count2);
    static void copyArrayAddRef(State* d, State* s, UPInt count);

    State*       toState()       { return (State*)(StateData*)this; }
    const State* toState() const { return (const State*)(const StateData*)this; }
};

}} // Scaleform::Render

#endif
