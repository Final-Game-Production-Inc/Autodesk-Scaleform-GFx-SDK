/**************************************************************************

Filename    :   GameEntitySlot.cpp
Content     :   Sample Game Entity Slot Types
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "GameEntitySlot.h"
#include "GameEntity.h"


//////////////////////////////////////////////////////////////////////////

bool    GameItemSlot::SetEntity(GameItem* newEntity, bool silent)
{
    bool success = false;

    // TODO: Handle entity merging here

    success = SupportsEntity(newEntity);
    if (success) 
    {
        Ptr<GameItem> oldEntity = EntityRef;
         if (EntityRef)
             EntityRef->RemoveEventListener(this);
        EntityRef = newEntity;
        if (EntityRef)
            EntityRef->AddEventListener(this);
        
        if (!silent)
        {            
            EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, newEntity, oldEntity));
        }
    }

    return success;
}

void    GameItemSlot::SwapWith(GameItemSlot* other)
{
    if (other == this) return;

    Ptr<GameItem> s = GetEntity();
    Ptr<GameItem> d = other->GetEntity();

    // See if we can merge
    if (d && mergeWith(s, d))
    {
        SetEntity(NULL, true);

        EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, GetEntity(), s));
        other->EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, other, other->GetEntity(), d));
        return;
    }

    // Check if a fallback slot is necessary (if acceptance rule check fails)
    GameItemSlot* fallbackSlot = other->CheckFallbackSlot(this, s);
    if (fallbackSlot)
    {
        Ptr<GameItem> f = fallbackSlot->GetEntity();
        SF_ASSERT(!f);
        if (fallbackSlot->SetEntity(s, true))
        {
            SetEntity(NULL, true);
            EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, GetEntity(), s));
            fallbackSlot->EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, fallbackSlot, fallbackSlot->GetEntity(), f));
        }
    }
    else
    {
        if (other->SetEntity(s, true))
        {
            if (!SetEntity(d, true))
            {
                SetEntity(NULL, true);

                // Drop the other item
                other->EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemDropped, other, d));
            }

            EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, GetEntity(), s));
            other->EventListenerGroup->NotifyEvent(GameEntitySlotEvent(GameEntitySlotEvent::ESLOT_SlotItemChanged, other, other->GetEntity(), d));
        }
    }
}

bool    GameItemSlot::mergeWith(GameItem* s, GameItem* d)
{
    // Can type be stacked?
    Game::IDDescType* sdesc = Game::GetDescForID(s->GetDefnID());
    if (!sdesc->Stackable) return false;

    // Types must be the same
    if (s->GetDefnID() != d->GetDefnID()) return false;

    // Perform merge
    d->SetStackCount(s->GetStackCount() + d->GetStackCount());

    // Reduce other count to 0
    s->SetStackCount(0);

    return true;
}

void    GameItemSlot::OnEntityEvent(const GameEntityEvent& e)
{
    switch (e.Type)
    {
        case GameEntityEvent::ENTITY_StackChange:
        {
            GameEntitySlotEvent ese(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, EntityRef, EntityRef);
            EventListenerGroup->NotifyEvent(ese);
            break;
        }
        case GameEntityEvent::ENTITY_Destroy:
        {
            Ptr<GameItem> ent = GetEntity();
            SetEntity(NULL, true);
            GameEntitySlotEvent ese(GameEntitySlotEvent::ESLOT_SlotItemChanged, this, EntityRef, ent);
            EventListenerGroup->NotifyEvent(ese);
            break;
        }
        default:
        {
            //SF_ASSERT(0);
            break;
        }
    }
}

void    GameItemSlot::AddEventListener(GameEntitySlotListener* l)
{
    if (!EventListenerGroup) 
        EventListenerGroup = *SF_NEW GameEntitySlotListenerGroup();
    EventListenerGroup->AddListener(l);
}

void    GameItemSlot::RemoveEventListener(GameEntitySlotListener* l)
{
    if (EventListenerGroup) EventListenerGroup->RemoveListener(l);
}


//////////////////////////////////////////////////////////////////////////

bool    GameInventorySlot::SupportsEntity(GameItem* entity)
{
    if (!entity) return true;
    if (ContainerID == entity->GetInstanceID()) return false;   // Can't put container in itself
    GameItemSlotContainer* slotContainer = entity->GetSlotContainer();
    if (slotContainer && !slotContainer->IsEmpty()) return false;   // Can't put a container with items in it
    return Game::GetDescForID(entity->GetDefnID())->CanPawnHold;
}


//////////////////////////////////////////////////////////////////////////

bool    GameContainerSlot::SupportsEntity(GameItem* entity)
{
    if (!entity) return true;
    return entity->GetDefnID().Class == Game::ENTITY_ContainerItem &&
        Game::GetDescForID(entity->GetDefnID())->CanPawnHold;
}

GameItemSlot* GameContainerSlot::CheckFallbackSlot(GameItemSlot* src, GameItem* e)
{
    if (src->GetType() == Game::SLOT_Inventory && SupportsEntity(e)) return NULL;
    // Couldn't set entity into slot; try as an entity slot item?
    if (EntityRef && EntityRef->GetSlotContainer()->HasEmptySlots() && Game::GetDescForID(e->GetDefnID())->CanPawnHold)
    {
        return EntityRef->GetSlotContainer()->GetFreeSlot();
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

bool    GameBackpackSlot::SupportsEntity(GameItem* entity)
{
    if (!entity) return true;
    return entity->GetDefnID().Class == Game::ENTITY_ContainerItem &&
            entity->GetDefnID().ID == Game::ID_CONT_Backpack;
}


//////////////////////////////////////////////////////////////////////////

bool    GameEquipmentSlot::SupportsEntity(GameItem* entity)
{
    if (!entity) return true;
    return Game::SlotSupportsEquipmentClass(Type, entity->GetDefnID());
}
