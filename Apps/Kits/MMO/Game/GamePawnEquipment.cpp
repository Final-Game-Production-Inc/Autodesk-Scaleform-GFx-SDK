/**************************************************************************

Filename    :   GamePawnEquipment.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GamePawnEquipment.h"

#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;

#include "GamePawn.h"

//////////////////////////////////////////////////////////////////////////

GamePawnEquipment::GamePawnEquipment(GamePlayerPawn* pawn)
{
    Owner = pawn;
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        Slots[i] = *SF_NEW GameEquipmentSlot((Game::EquipmentSlotType)i, i);
        Slots[i]->AddEventListener(this);
    }
}

GameEquipmentSlot*      GamePawnEquipment::GetSlot(unsigned idx)
{
    return Slots[idx];
}


void    GamePawnEquipment::OnSlotEvent(const GameEntitySlotEvent& e)
{
    unsigned slotIdx = e.Slot->GetIndex();

    switch (e.Type)
    {
    case GameEntitySlotEvent::ESLOT_SlotItemChanged:
        {
            Owner->notifyEquipmentChange((Game::EquipmentSlotType)slotIdx);
            // Check consistency
            if (GetSlot(slotIdx)->GetEntity())
            {
                // Should the off hand item be dropped?
                if (slotIdx == Game::EQS_MainHand)
                {
                    GameItem* entity = GetSlot(Game::EQS_MainHand)->GetEntity();
                    if ( entity && (Game::GetDescForID(entity->GetDefnID())->SubClass == Game::WEAP_Melee2H) )
                    {
                        dropEquipment(Game::EQS_OffHand);
                    }
                }
                // Should the main hand be dropped?
                if (slotIdx == Game::EQS_OffHand) 
                {
                    GameItem* entity = GetSlot(Game::EQS_MainHand)->GetEntity();
                    if ( entity && (Game::GetDescForID(entity->GetDefnID())->SubClass == Game::WEAP_Melee2H) )
                    {
                        dropEquipment(Game::EQS_MainHand);
                    }
                }
            }
            break;
        }
    case GameEntitySlotEvent::ESLOT_SlotItemDropped:
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

void    GamePawnEquipment::dropEquipment(Game::EquipmentSlotType slot)
{
    Ptr<GameItem> ent = GetSlot(slot)->GetEntity();
    GetSlot(slot)->SetEntity(NULL);
    // Try to put the item in backpack
    Owner->GetBagContainer()->AddItem(ent);
    // Else, just let it go..
}
