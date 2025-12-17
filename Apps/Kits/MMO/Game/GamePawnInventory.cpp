/**************************************************************************

Filename    :   GamePawnInventory.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GamePawnInventory.h"

#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;

#include "GamePawn.h"

//////////////////////////////////////////////////////////////////////////

GamePawnBagContainer::GamePawnBagContainer(GamePlayerPawn* pawn)
{
    Owner = pawn;
    BackpackSlot = *SF_NEW GameBackpackSlot(0);
    BackpackSlot->AddEventListener(this);
    for (unsigned i = 1; i < Game::NumInventorySlots; i++)
    {
        Slots[i-1] = *SF_NEW GameContainerSlot(i);
        Slots[i-1]->AddEventListener(this);
    }

    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        EventProxies[i] = *SF_NEW GamePawnBagContainer::BagSlotEventProxy(Owner, (Game::InventorySlotType)i);
    }
}

GameContainerSlot*    GamePawnBagContainer::GetSlot(unsigned idx)
{
    if (idx == 0) return BackpackSlot;
    return Slots[idx-1];
}

void    GamePawnBagContainer::OnSlotEvent(const GameEntitySlotEvent& e)
{
    unsigned slotIdx = e.Slot->GetIndex();

    switch (e.Type)
    {
    case GameEntitySlotEvent::ESLOT_SlotItemChanged:
        {
            if (e.OldEntity) e.OldEntity->RemoveEventListener(EventProxies[slotIdx]);
            if (e.Entity) e.Entity->AddEventListener(EventProxies[slotIdx]);
            Owner->notifyInventoryChange((Game::InventorySlotType)slotIdx);
            break;
        }
    default:
        {
            SF_ASSERT(0);
            break;
        }
    }
}

void    GamePawnBagContainer::AddItem(GameItem* entity)
{    
    // Try all bag slots
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameItem* bag = GetSlot(i)->GetEntity();
        if (bag)
        {
            if (bag->GetSlotContainer()->AddItem(entity)) return;
        }
    }
}

void    GamePawnBagContainer::BagSlotEventProxy::OnEntityEvent(const GameEntityEvent& e)
{  
    switch (e.Type)
    {
    case GameEntityEvent::ENTITY_SlotItemChanged:
        {
            Owner->notifyBagItemChange(InvSlot, e.SlotIndex);
            break;
        }
    case GameEntityEvent::ENTITY_SlotItemDropped:
        {
            // Try to put the item in backpack
            Owner->GetBagContainer()->AddItem(e.Entity);
            // Else, just let it go
            break;
        }
    default:
        {
            SF_ASSERT(0);
            break;
        }
    }
}
