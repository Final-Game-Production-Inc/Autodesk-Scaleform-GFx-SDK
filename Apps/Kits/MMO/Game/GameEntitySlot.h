/**************************************************************************

Filename    :   GameEntitySlot.h
Content     :   Sample Game Entity Slot Types
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameEntitySlot_H
#define INC_GameEntitySlot_H

// Includes
#include "Kernel/SF_String.h"
#include "GameEntity.h"

// Namespaces
using namespace Scaleform;


//////////////////////////////////////////////////////////////////////////
//
// A slot for entity storage
//
class GameItemSlot : public RefCountBase<GameItemSlot, Stat_Default_Mem>, public GameEntityListener
{
public:
    GameItemSlot(Game::SlotType type, unsigned idx) : Type(type), Index(idx) {}

    Game::SlotType  GetType() const     { return Type; }
    unsigned        GetIndex() const    { return Index; }

    bool            HasEntity() const   { return EntityRef.GetPtr() != NULL; }
    GameItem*     GetEntity() const   { return EntityRef; }
    
    virtual GameItemSlot* CheckFallbackSlot(GameItemSlot* src, GameItem* e) { SF_UNUSED2(src, e); return false; }
    bool            SetEntity(GameItem* newEntity, bool silent = false);

    void            SwapWith(GameItemSlot* other);  

    virtual bool    SupportsEntity(GameItem* entity) = 0;

    virtual void    OnEntityEvent(const GameEntityEvent& e);
    void            AddEventListener(GameEntitySlotListener* l);
    void            RemoveEventListener(GameEntitySlotListener* l);

protected:
    Game::SlotType          Type;
    Ptr<GameItem>         EntityRef;
    unsigned                Index;

    Ptr<GameEntitySlotListenerGroup>  EventListenerGroup;

    bool            mergeWith(GameItem* s, GameItem* d);
};


//////////////////////////////////////////////////////////////////////////

//
// *** Implementations of entity slots

// - For inventory entities
//
class GameInventorySlot : public GameItemSlot
{
public:
    GameInventorySlot(const char* containerID, unsigned index) : GameItemSlot(Game::SLOT_Entity, index), ContainerID(containerID) {}

    virtual bool    SupportsEntity(GameItem* entity);

protected:
    String     ContainerID;
};
// - For equipment
//
class GameEquipmentSlot : public GameItemSlot
{
public:
    GameEquipmentSlot(Game::EquipmentSlotType type, unsigned index) : GameItemSlot(Game::SLOT_Equipment, index), Type(type) {}

    virtual bool    SupportsEntity(GameItem* entity);

protected:
    Game::EquipmentSlotType    Type;
};
//
// - For containers (bags)
//
class GameContainerSlot : public GameItemSlot
{
public:
    GameContainerSlot(unsigned index) : GameItemSlot(Game::SLOT_Inventory, index) {}

    virtual GameItemSlot* CheckFallbackSlot(GameItemSlot* src, GameItem* e);

    virtual bool    SupportsEntity(GameItem* entity);
};
class GameBackpackSlot : public GameContainerSlot
{
public:
    GameBackpackSlot(unsigned index) : GameContainerSlot(index) {}

    virtual bool    SupportsEntity(GameItem* entity);
};

#endif  // INC_GameEntitySlot_H
