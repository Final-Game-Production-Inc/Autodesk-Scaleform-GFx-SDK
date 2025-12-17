/**************************************************************************

Filename    :   GameEntity.cpp
Content     :   Sample Game Entity Base
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Incldues
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"
#include "GameEntity.h"
#include "GameEntitySlot.h"
#include "GameServer.h"


GameItemSlotContainer::~GameItemSlotContainer()
{
    for (unsigned i = 0; i < Slots.GetSize(); i++)
    {
        Slots[i]->RemoveEventListener(Owner);
        Slots[i]->Release();
    }
}

void    GameItemSlotContainer::AddSlot(GameItemSlot* slot)
{
    slot->AddRef();
    slot->AddEventListener(Owner);
    Slots.PushBack(slot);
}

GameItemSlot* GameItemSlotContainer::GetSlot(unsigned idx)
{
    return  Slots[idx];
}

unsigned GameItemSlotContainer::GetNumSlots() const 
{
    return (unsigned)Slots.GetSize();
}

bool        GameItemSlotContainer::AddItem(GameItem* entity)
{
    bool ok = false;
    int freeIdx = -1;
    for (unsigned i = 0; i < Slots.GetSize(); i++)
    {
        if (!Slots[i]->HasEntity()) 
        {
            freeIdx = i;
            break;
        }
    }
    if (freeIdx > -1)
    {
        ok = Slots[freeIdx]->SetEntity(entity);
        //SF_ASSERT(ok); // PPS: This can fail.. let it pass through as it may be expected behavior
                         //      Ex: Container with items being set in an inventory slot
    }
    return ok;
}

bool        GameItemSlotContainer::IsEmpty() const
{
    for (unsigned i = 0; i < Slots.GetSize(); i++)
    {
        if (Slots[i]->HasEntity()) return false;
    }
    return true;
}

bool        GameItemSlotContainer::HasEmptySlots() const
{
    for (unsigned i = 0; i < Slots.GetSize(); i++)
    {
        if (!Slots[i]->HasEntity()) return true;
    }
    return false;
}

GameItemSlot*        GameItemSlotContainer::GetFreeSlot()
{
    for (unsigned i = 0; i < Slots.GetSize(); i++)
    {
        if (!Slots[i]->HasEntity()) return Slots[i];
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

GameItem::~GameItem()
{
    SF_ASSERT(Manager);
    Manager->onEntityDestroy(this);
}

bool    GameItem::Consume(unsigned count)
{
    if (StackItemCount < count) return false;
    StackItemCount -= count;
    if (StackItemCount == 0)
    {
        GameEntityEvent ne(GameEntityEvent::ENTITY_Destroy, this);
        EventListenerGroup->NotifyEvent(ne);
    }
    else
    {
        GameEntityEvent ne(GameEntityEvent::ENTITY_StackChange, this);
        EventListenerGroup->NotifyEvent(ne);
    }
    return true;
}

void    GameItem::AddEventListener(GameEntityListener* l)
{
    if (!EventListenerGroup) 
        EventListenerGroup = *SF_NEW GameEntityListenerGroup();
    EventListenerGroup->AddListener(l);
}

void    GameItem::RemoveEventListener(GameEntityListener* l)
{
    if (EventListenerGroup) EventListenerGroup->RemoveListener(l);
}

GameItemSlotContainer*    GameItem::GetSlotContainer()
{
    if (!Slots) Slots = *SF_NEW GameItemSlotContainer(this);
    return Slots;
}

void    GameItem::OnSlotEvent(const GameEntitySlotEvent& e)
{
    switch (e.Type)
    {
    case GameEntitySlotEvent::ESLOT_SlotItemChanged:
        {
            GameEntityEvent ne(GameEntityEvent::ENTITY_SlotItemChanged, this, e.Slot->GetIndex(), e.OldEntity);
            EventListenerGroup->NotifyEvent(ne);
            break;
        }
    case GameEntitySlotEvent::ESLOT_SlotItemDropped:
        {
            GameEntityEvent ne(GameEntityEvent::ENTITY_SlotItemDropped, e.Entity);
            EventListenerGroup->NotifyEvent(ne);
            break;
        }
    default:
        {
            SF_ASSERT(0);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

static void    CreateEntityID(String* sink, const Game::EntityDefnID& defnId, unsigned idx)
{
    Format(*sink, "{0}:{1}#{2}", (unsigned)defnId.Class, defnId.ID, idx);
}

GameItem*   GameItemFactory::CreateInstance(GameServer* env, const Game::EntityDefnID& defnId, unsigned stackSz)
{
    Game::IDDescType* desc = Game::GetDescForID(defnId);
    SF_UNUSED(desc);
    SF_ASSERT(desc);

    // Reserve a new storage index
    unsigned newIDx = IdCounter++;

    // Generate a 'unique' id
    String newIDStr;
    CreateEntityID(&newIDStr, defnId, newIDx);

    // Create the instance
    GameItem*   newItem = SF_NEW GameItem();
    newItem->ID = newIDStr;
    newItem->Definition = defnId;
    newItem->StackItemCount = stackSz;
    
    // Setup entity
    env->SetupEntity(newItem);

    // Configure lifetime management members
    newItem->Manager = this;

    SF_DEBUG_MESSAGE2(1, "Created entity instance [%s]@%s\n", Game::GetStringsForID(defnId)->Name, newItem->GetInstanceID());
    return newItem;
}

void    GameItemFactory::onEntityDestroy(GameItem* item)
{
    SF_UNUSED(item);
    SF_DEBUG_MESSAGE2(1, "Destroyed entity instance [%s]@%s\n", Game::GetStringsForID(item->GetDefnID())->Name, item->GetInstanceID());
}
