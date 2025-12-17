/**************************************************************************

Filename    :   GameEntityEvent.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameEntityEvent_H
#define INC_GameEntityEvent_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

#include "GameCommon.h"

class GameItem;
class GameItemSlot;
class GameItemSlotContainer;

//////////////////////////////////////////////////////////////////////////
//
//
class GameEntityEvent
{
public:
    enum EventType
    {
        ENTITY_StackChange, // StackCount changes
        ENTITY_Destroy,     // StackCount goes to 0
        ENTITY_SlotItemChanged,
        ENTITY_SlotItemDropped,
    };

    EventType           Type;
    GameItem          *Entity, *OldEntity;
    unsigned            SlotIndex;

    GameEntityEvent(EventType type, GameItem* entity, unsigned slotIdx = 0, GameItem* oldEntity = NULL) 
        : Type(type), Entity(entity), SlotIndex(slotIdx), OldEntity(oldEntity) {}
};

class GameEntityListener
{
public:

    virtual ~GameEntityListener() {}

    virtual void    OnEntityEvent(const GameEntityEvent& e)     = 0;
};

class GameEntityListenerGroup : public RefCountBase<GameEntityListenerGroup, Stat_Default_Mem>
{
public:
    void    AddListener(GameEntityListener* l);
    void    RemoveListener(GameEntityListener* l);
    void    NotifyEvent(const GameEntityEvent& e);
protected:
    Array<GameEntityListener*>   Listeners;
};


//////////////////////////////////////////////////////////////////////////

class GameEntitySlotEvent
{
public:
    enum EventType
    {
        ESLOT_SlotItemChanged,  // A new item was added to the slot successfully
        ESLOT_SlotItemDropped,  // A new item was added to the slot, but the operation 
                                // could not be completed successfully
    };

    EventType           Type;
    GameItemSlot      *Slot;
    GameItem          *Entity, *OldEntity;

    GameEntitySlotEvent(EventType type, GameItemSlot* slot, GameItem* entity, GameItem* oldEntity = NULL) 
        : Type(type), Slot(slot), Entity(entity), OldEntity(oldEntity) {}
};

class GameEntitySlotListener
{
public:
    virtual ~GameEntitySlotListener() {}

    virtual void    OnSlotEvent(const GameEntitySlotEvent& e)     = 0;
};

class GameEntitySlotListenerGroup : public RefCountBase<GameEntitySlotListenerGroup, Stat_Default_Mem>
{
public:
    void    AddListener(GameEntitySlotListener* l);
    void    RemoveListener(GameEntitySlotListener* l);
    void    NotifyEvent(const GameEntitySlotEvent& e);
protected:
    Array<GameEntitySlotListener*>   Listeners;
};


#endif  // INC_GameEntityEvent_H
