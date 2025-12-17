/**************************************************************************

Filename    :   GameEntity.h
Content     :   Sample Game Entity Base
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameEntity_H
#define INC_GameEntity_H

// Includes
#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
#include "GameEntityEvent.h"

// Namespaces
using namespace Scaleform;

// Forward declarations
class GameItemSlot;
class GameItemFactory;
class GameServer;
class GamePawn;


//////////////////////////////////////////////////////////////////////////
// Slot container for entities. This allows entities to hold other entities,
// such as in the case for container items.
//
class GameItemSlotContainer : public RefCountBase<GameItemSlotContainer, Stat_Default_Mem>
{
public:
    GameItemSlotContainer(GameItem* owner) : Owner(owner) {}
    virtual     ~GameItemSlotContainer();

    void                AddSlot(GameItemSlot* slot);
    GameItemSlot*     GetSlot(unsigned idx);
    GameItemSlot*     GetFreeSlot();
    unsigned            GetNumSlots() const;

    bool        AddItem(GameItem* entity);
    bool        IsEmpty() const;
    bool        HasEmptySlots() const;

protected:
    Array< GameItemSlot* >    Slots;
    GameItem*                 Owner;
};

//////////////////////////////////////////////////////////////////////////
// An entity instance or a stack of entity instances of the same type.
//
class GameItem : public RefCountBase<GameItem, Stat_Default_Mem>, public GameEntitySlotListener
{
    friend class GameItemFactory;

public:
    virtual ~GameItem();

    const char*                 GetInstanceID() const   { return ID; }
    const Game::EntityDefnID&   GetDefnID() const       { return Definition; }

    GameItemSlotContainer*    GetSlotContainer();

    bool            IsStack() const             { return StackItemCount > 1; }
    unsigned        GetStackCount() const       { return StackItemCount; }
    void            SetStackCount(unsigned s)   { StackItemCount = s; }

    bool            Consume(unsigned count);

    bool            IsEnabled() const        { return Enabled; }
    void            SetEnabled(bool enabled);

    // Splits a specific number of items in this stack into a new item/stack
    GameItem*     CreateSplit(unsigned splitSz);

    void            AddEventListener(GameEntityListener* l);
    void            RemoveEventListener(GameEntityListener* l);

    virtual void    OnSlotEvent(const GameEntitySlotEvent& e);

protected:
    String              ID;
    Game::EntityDefnID  Definition;
    unsigned            StackItemCount;
    bool                Enabled;

    Ptr<GameItemSlotContainer>    Slots;

    Ptr<GameEntityListenerGroup>  EventListenerGroup;
    GameItemFactory*      Manager;

    // GameEntities are created via the GameEntityFactory
    GameItem() : Enabled(false), Manager(NULL) {}
    // Copying is prohibited
    GameItem(const GameItem&);
    const GameItem& operator = (const GameItem&);
};


//////////////////////////////////////////////////////////////////////////
// Entity that manages item instances
//
class GameItemFactory
{
    friend class GameItem;

public:
    GameItemFactory() :IdCounter(1) {}

    GameItem*             CreateInstance(GameServer* env, const Game::EntityDefnID& defnId, unsigned stackSz = 1);

protected:
    void                    onEntityDestroy(GameItem* item);

    unsigned                IdCounter;
};


#endif  // INC_GameEntity_H
