/**************************************************************************

Filename    :   GameClient.cpp
Content     :   Sample representation of an MMO game client
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include "GameClient.h"
#include "GameConnection.h"


GameClient::GameClient(GFx::Movie* pmovie, GameConnection* conn)
: Connection(conn), ClientID(0)
{
    pMovie = pmovie;

    State = *SF_NEW GameClientState(this);

    // Install a custom external interface handler that will be used to register
    // function handlers to the AS3 GameDataModel interface
    UIAdapter = *SF_NEW GameUIAdapter(this, pMovie);
}


//////////////////////////////////////////////////////////////////////////
// Called by the main game application to initialize the UI.
//
void    GameClient::Initialize(GameUIRuntimeAtlasManager* pimagecreator)
{
    UIAdapter->Initialize(pimagecreator);
}


//////////////////////////////////////////////////////////////////////////
// Called by the main game application to update the client state and 
// client UI.
//
void    GameClient::Update()
{
    // Check for pending events
    GameServerEvent* e = Connection->GetNextEvent();
    while (e)
    {
        processServerEvent(e);
        delete e;
        e = Connection->GetNextEvent();
    }

    UInt32 ticks = Timer::GetTicksMs();
    State->Update(ticks);
    UIAdapter->Update(ticks);
}


//////////////////////////////////////////////////////////////////////////
// This method performs a simulated login of a player. The connection
// is now tied to that player in the server side.
//
void    GameClient::Connect(const char* playerName)
{
    GameClientRequest* r = GameClientRequest::CreatePlayerLoginRequest(playerName);
    Connection->SendRequest(r);
}



//////////////////////////////////////////////////////////////////////////
// Returns the appropriate default color for message types.
//
const char*     GameClient::GetMessageColor(Game::MessageType type) const
{
    switch (type)
    {
    case Game::MSG_Say:
        return "#eeeeee";
    case Game::MSG_Yell:
        return "#ee2222";
    case Game::MSG_Error:
        return "#e52f4d";
    case Game::MSG_Whisper:
        return "#45e5cb";
    case Game::MSG_System:
        return "#ffff66";
    case Game::MSG_Guild:
        return "#66cc33";
    default:
        return "#cfcfc6";
    }
}


//////////////////////////////////////////////////////////////////////////
// Returns the appropriate color for message sources (i.e.: player names)
//
const char*     GameClient::GetMessageSourceColor(Game::MessageType type) const 
{
    return GetMessageColor(type);
}


//////////////////////////////////////////////////////////////////////////
// Returns the appropriate color for message timestamps
//
const char*     GameClient::GetMessageTimeColor(Game::MessageType type) const
{
    SF_UNUSED(type);
    return "#8c8c87";
}


//////////////////////////////////////////////////////////////////////////
// Called to handle server events from the Update() method.
//
void    GameClient::processServerEvent(GameServerEvent* event)
{
    switch (event->Type)
    {
    case GameServerEvent::GE_LoginSuccess:
        {
            ClientID = event->UIValue32;
            break;
        }
    case GameServerEvent::GE_PlayerInfoChange:
        {
            State->SetPlayerInfo(event->Info, event->SValue);
            UIAdapter->HandlePlayerInfoChange(event->Info);
            break;
        }
    case GameServerEvent::GE_PlayerStatChange:
        {
            State->SetPlayerStat(event->Stat, event->UIValue32);
            UIAdapter->HandlePlayerStatChange(event->Stat);
            break;
        }
    case GameServerEvent::GE_PlayerMoneyChange:
        {
            State->SetPlayerMoney(event->Money, event->UIValue32);
            UIAdapter->HandlePlayerMoneyChange(event->Money);
            break;
        }
    case GameServerEvent::GE_PlayerTargetInfoChange:
        {
            State->SetTargetInfo(event->Info, event->SValue);
            UIAdapter->HandleTargetInfoChange(event->Info);
            break;
        }
    case GameServerEvent::GE_PlayerTargetStatChange:
        {
            State->SetTargetStat(event->Stat, event->UIValue32);
            UIAdapter->HandleTargetStatChange(event->Stat);
            break;
        }
    case GameServerEvent::GE_PlayerInventoryChange:
        {
            UIAdapter->HandlePlayerInventoryItemRemove(event->InventorySlot);
            State->SetPlayerInventoryItem(event->InventorySlot, event->Entity, event->UIValue32);
            UIAdapter->HandlePlayerInventoryItemAdd(event->InventorySlot);
            break;
        }
    case GameServerEvent::GE_PlayerBagItemChange:
        {
            // Get a reference to the old entity in the slot before it is overwritten
            GameClientSlotRef* slotRef = State->GetPlayerBagItem(event->InventorySlot, event->UIValue32);
            GameClientEntityRef oldEnt;
            if (!slotRef->GetData().IsNull && event->Entity.IsNull)
                oldEnt = slotRef->GetData();

            State->SetPlayerBagItem(event->InventorySlot, event->UIValue32, event->Entity);
            UIAdapter->HandlePlayerBagItemChange(event->InventorySlot, event->UIValue32);
            
            // Special case: Update any links pointing to both the old item and the new one
            if (!oldEnt.IsNull)
                UIAdapter->UpdateLinksToItem(oldEnt);
            if (!event->Entity.IsNull)
                UIAdapter->UpdateLinksToItem(event->Entity);

            break;
        }
    case GameServerEvent::GE_PlayerEqiupmentChange:
        {
            State->SetPlayerEquipmentItem(event->EquipmentSlot, event->Entity);
            UIAdapter->HandlePlayerEquipmentChange(event->EquipmentSlot);
            break;
        }
    case GameServerEvent::GE_PlayerAbilityAdd:
        {
            GameClientAbility a(event->AbilID, event->UIValue32);
            Game::AbilityGroupType grpId = Game::GetAbilityDescForID(event->AbilID)->GroupID;
            bool hasGroup = State->HasPlayerAbilityGroup(grpId);
            State->AddPlayerAbility(a);

            if (!hasGroup)
                UIAdapter->HandlePlayerAbilityGroupAdd(grpId);
            else
                UIAdapter->HandlePlayerAbilityAdd(a);

            break;
        }
    case GameServerEvent::GE_PlayerAbilityCooldownStart:
        {
            State->SetPlayerAbilityCooldown(event->AbilID, event->UIValue32);
            break;
        }
    case GameServerEvent::GE_PlayerAbilityCooldownEnd:
        {
            State->ClearPlayerAbilityCooldown(event->AbilID);
            UIAdapter->HandlePlayerAbilityCooldownEnd(event->AbilID);
            break;
        }
    case GameServerEvent::GE_PlayerAbilityCastStart:
        {
            State->SetPlayerAbilityCast(event->AbilID);
            break;
        }
    case GameServerEvent::GE_PlayerAbilityCastInterrupt:
        {
            State->ClearPlayerAbilityCast(event->AbilID);
            UIAdapter->HandlePlayerAbilityCastInterrupt(event->AbilID);
            break;
        }
    case GameServerEvent::GE_PlayerAbilityCastEnd:
        {
            State->ClearPlayerAbilityCast(event->AbilID);
            break;
        }
    case GameServerEvent::GE_ChannelSubscribed:
        {
            State->SubscribeChatChannel(event->MessageType, event->SValue, event->SValue2);
            break;
        }
    case GameServerEvent::GE_ChannelMessage:
        {
            GameClientLogMsg msg(event->ChatMsg, event->Time);
            State->AddChatChannelMessage(event->MessageType, event->SValue, msg);
            break;
        }
    default:
        {
            SF_ASSERT(0);
        }
    }
}
