/**************************************************************************

Filename    :   GameEvent.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_HeapNew.h"

#include "GameEvent.h"


GameClientRequest*    GameClientRequest::CreatePlayerLoginRequest(const char* playerName)
{
    GameClientRequest* r = SF_NEW GameClientRequest(GR_PlayerLogin);
    r->SValue = playerName;
    return r;
}

GameClientRequest*    GameClientRequest::CreatePlayerSlotSwapRequest(unsigned clientId, const Game::EntitySlotDescriptor& src, const Game::EntitySlotDescriptor& dest)
{
    GameClientRequest* r = SF_NEW GameClientRequest(GR_PlayerSlotSwap);
    r->ClientID = clientId;
    r->SrcSlot = src;
    r->DestSlot = dest;
    return r;
}

GameClientRequest*    GameClientRequest::CreatePlayerAbilityUseRequest(unsigned clientId, unsigned abilId)
{
    GameClientRequest* r = SF_NEW GameClientRequest(GR_PlayerAbilityUse);
    r->ClientID = clientId;
    r->UIValue = abilId;
    return r;
}

GameClientRequest*    GameClientRequest::CreatePlayerSlotUseRequest(unsigned clientId, const Game::EntitySlotDescriptor& slot)
{
    GameClientRequest* r = SF_NEW GameClientRequest(GR_PlayerEntitySlotUse);
    r->ClientID = clientId;
    r->SrcSlot = slot;
    return r;
}

GameClientRequest*  GameClientRequest::CreatePlayerSendMessage(unsigned clientId, Game::MessageType type, 
                                                               const char* chanId, const char* msg, 
                                                               const char* target)
{
    GameClientRequest* r = SF_NEW GameClientRequest(GR_PlayerSendMessage);
    r->ClientID = clientId;
    r->MsgType = type;
    r->SValue = chanId;
    r->Target = target;
    r->Message = msg;
    return r;
}

//////////////////////////////////////////////////////////////////////////


GameServerEvent*    GameServerEvent::CreateLoginSuccessEvent(unsigned clientId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_LoginSuccess);
    e->UIValue32 = clientId;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerInfoChangeEvent(Game::InfoType p, const char* value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerInfoChange);
    e->Info = p;
    e->SValue = value;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerStatChangeEvent(Game::StatType s, unsigned value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerStatChange);
    e->Stat = s;
    e->UIValue32 = value;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerMoneyChangeEvent(Game::MoneyType s, unsigned value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerMoneyChange);
    e->Money = s;
    e->UIValue32 = value;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerInventoryChangeEvent(Game::InventorySlotType s, const EntityDesc& value, unsigned numSlots)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerInventoryChange);
    e->InventorySlot = s;
    e->Entity = value;
    e->UIValue32 = numSlots;
    return e;
}


GameServerEvent*    GameServerEvent::CreatePlayerBagItemChangeEvent(Game::InventorySlotType s, const EntityDesc& value, unsigned bagSlot)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerBagItemChange);
    e->InventorySlot = s;
    e->Entity = value;
    e->UIValue32 = bagSlot;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerEquipmentChangeEvent(Game::EquipmentSlotType s, const EntityDesc& value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerEqiupmentChange);
    e->EquipmentSlot = s;
    e->Entity = value;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityAddEvent(Game::AbilityID abilId, unsigned rank)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityAdd);
    e->AbilID = abilId;
    e->UIValue32 = rank;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityRemoveEvent(Game::AbilityID abilId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityRemove);
    e->AbilID = abilId;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityCastStartEvent(Game::AbilityID abilId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityCastStart);
    e->AbilID = abilId;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityCastInterruptEvent(Game::AbilityID abilId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityCastInterrupt);
    e->AbilID = abilId;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityCastEndEvent(Game::AbilityID abilId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityCastEnd);
    e->AbilID = abilId;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityCooldownStartEvent(Game::AbilityID abilId, UInt32 ticksLeft)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityCooldownStart);
    e->AbilID = abilId;
    e->UIValue32 = ticksLeft;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerAbilityCooldownEndEvent(Game::AbilityID abilId)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerAbilityCooldownEnd);
    e->AbilID = abilId;
    return e;
}

GameServerEvent*    GameServerEvent::CreateChannelSubscribedEvent(Game::MessageType m, const String& chanId, const String& chanName)
{
    GameServerEvent* e  = SF_NEW GameServerEvent(GE_ChannelSubscribed);
    e->MessageType  = m;
    e->SValue     = chanId;
    e->SValue2       = chanName;
    return e;
}

GameServerEvent*    GameServerEvent::CreateChannelMessageEvent(Game::MessageType m, const String& chanId, 
                                                               time_t t, const ChatMsgDesc& msg)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_ChannelMessage);
    e->MessageType  = m;
    e->SValue    = chanId;
    e->Time         = t;
    e->ChatMsg      = msg;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerTargetInfoChangeEvent(Game::InfoType s, const char* value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerTargetInfoChange);
    e->Info = s;
    e->SValue = value;
    return e;
}

GameServerEvent*    GameServerEvent::CreatePlayerTargetStatChangeEvent(Game::StatType s, unsigned value)
{
    GameServerEvent* e = SF_NEW GameServerEvent(GE_PlayerTargetStatChange);
    e->Stat = s;
    e->UIValue32 = value;
    return e;
}

/////////////////////////////////////////////////////////////////////////

void    GameEventListenerSet::Remove(GameEventListener* l)
{
    int remidx = -1;
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
    {
        if (Listeners[i] == l)
        {
            remidx = i;
            break;
        }
    }
    if (remidx >= 0)
        Listeners.RemoveAt(remidx);
}

void    GameEventListenerSet::BroadcastEvent(GameServerEvent* e)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
    {
        Listeners[i]->OnGameEvent(e);
    }
}
