/**************************************************************************

Filename    :   GameEvent.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameEvent_H
#define INC_GameEvent_H

#include "Kernel/SF_String.h"
using namespace Scaleform;

#include "GameCommon.h"

class GameEventListener;
class GameEventListenerSet;

struct GameClientRequest : public NewOverrideBase<Stat_Default_Mem>
{
public:
    enum RequestType
    {
        GR_PlayerLogin,             // Called to log-in and request all player data

        GR_PlayerSlotSwap,          // Called to notify swapping of item slots

        GR_PlayerAbilityUse,        // Called to notify ability use
        GR_PlayerEntitySlotUse,     // Called to notify entity use

        GR_PlayerSendMessage,       // Called to send a message to the server
    };

    RequestType     Type;

    Game::EntitySlotDescriptor    SrcSlot;
    Game::EntitySlotDescriptor    DestSlot;

    unsigned            ClientID;
    Game::MessageType   MsgType;
    unsigned            UIValue;
    String              SValue;
    String              Target;
    String              Message;

    static GameClientRequest*  CreatePlayerLoginRequest(const char* playerName);
    static GameClientRequest*  CreatePlayerSlotSwapRequest(unsigned clientId, const Game::EntitySlotDescriptor& src, const Game::EntitySlotDescriptor& dest);
    static GameClientRequest*  CreatePlayerAbilityUseRequest(unsigned clientId, unsigned abilId);
    static GameClientRequest*  CreatePlayerSlotUseRequest(unsigned clientId, const Game::EntitySlotDescriptor& slot);
    static GameClientRequest*  CreatePlayerSendMessage(unsigned clientId, Game::MessageType type, const char* chanId, const char* msg, const char* target);

protected:
    GameClientRequest(RequestType type) : Type(type) {}
    // Copying is prohibited
    GameClientRequest(const GameClientRequest&);
    const GameClientRequest& operator = (const GameClientRequest&);
};

//////////////////////////////////////////////////////////////////////////

struct GameServerEvent : public NewOverrideBase<Stat_Default_Mem>
{
public:
    enum EventType
    {
        GE_LoginSuccess,
        GE_PlayerInfoChange,    
        GE_PlayerStatChange,    // Fired when a player's stat changes
        GE_PlayerMoneyChange,  // Fired when a player's money amounts changes
        GE_PlayerInventoryChange,   
        GE_PlayerBagItemChange,
        GE_PlayerEqiupmentChange,
        GE_PlayerAbilityAdd,
        GE_PlayerAbilityRemove,
        GE_PlayerAbilityCastStart,
        GE_PlayerAbilityCastInterrupt,
        GE_PlayerAbilityCastEnd,
        GE_PlayerAbilityCooldownStart,
        GE_PlayerAbilityCooldownEnd,
        
        GE_ChannelSubscribed,
        GE_ChannelMessage,

        GE_PlayerTargetInfoChange,        // Fired when a player's target changes
        GE_PlayerTargetStatChange,    // Fired when a player target's stat changes
    };

    struct EntityDesc
    {
        bool                IsNull;
        String              EntityID;
        Game::EntityDefnID  DefnID;
        unsigned            StackCount;

        EntityDesc() : IsNull(true) {}
        EntityDesc(const char* entId, const Game::EntityDefnID& defnId, unsigned stack) 
            : IsNull(false), EntityID(entId), DefnID(defnId), StackCount(stack) {}

        bool    operator == (const EntityDesc& other) const 
        {
            return (IsNull == other.IsNull) && (EntityID == other.EntityID) &&
                   (DefnID == other.DefnID);
        }
    };

    struct ChatMsgDesc
    {
        String              SourceId;
        String              SourceName;
        String              Message;

        ChatMsgDesc() {}
        ChatMsgDesc(const String& msg) : Message(msg) {}
        ChatMsgDesc(const String& id, const String& name, const String& msg) :
            SourceId(id), SourceName(name), Message(msg) {}
    };

    EventType               Type;

    union 
    {
        Game::InfoType              Info;
        Game::StatType              Stat;
        Game::MoneyType             Money;
        Game::InventorySlotType     InventorySlot;
        Game::EquipmentSlotType     EquipmentSlot;
        Game::AbilityID             AbilID;
        Game::MessageType  MessageType;
    };

    UInt32                  UIValue32;
    String                  SValue;
    String                  SValue2;
    EntityDesc              Entity;
    time_t                  Time;
    ChatMsgDesc             ChatMsg;

    static GameServerEvent* CreateLoginSuccessEvent(unsigned clientId);
    static GameServerEvent* CreatePlayerInfoChangeEvent(Game::InfoType s, const char* value);
    static GameServerEvent* CreatePlayerStatChangeEvent(Game::StatType s, unsigned value);
    static GameServerEvent* CreatePlayerMoneyChangeEvent(Game::MoneyType s, unsigned value);
    static GameServerEvent* CreatePlayerInventoryChangeEvent(Game::InventorySlotType s, const EntityDesc& value, unsigned numSlots);
    static GameServerEvent* CreatePlayerBagItemChangeEvent(Game::InventorySlotType s, const EntityDesc& value, unsigned bagSlot);
    static GameServerEvent* CreatePlayerEquipmentChangeEvent(Game::EquipmentSlotType s, const EntityDesc& value);   
    static GameServerEvent* CreatePlayerAbilityAddEvent(Game::AbilityID abilId, unsigned rank);
    static GameServerEvent* CreatePlayerAbilityRemoveEvent(Game::AbilityID abilId);
    static GameServerEvent* CreatePlayerAbilityCastStartEvent(Game::AbilityID abilId);
    static GameServerEvent* CreatePlayerAbilityCastInterruptEvent(Game::AbilityID abilId);
    static GameServerEvent* CreatePlayerAbilityCastEndEvent(Game::AbilityID abilId);
    static GameServerEvent* CreatePlayerAbilityCooldownStartEvent(Game::AbilityID abilId, UInt32 ticksLeft);
    static GameServerEvent* CreatePlayerAbilityCooldownEndEvent(Game::AbilityID abilId);
    
    static GameServerEvent* CreateChannelSubscribedEvent(Game::MessageType m, const String& chanId, const String& chanName);
    static GameServerEvent* CreateChannelMessageEvent(Game::MessageType m, const String& chanId, 
                                                        time_t t, const ChatMsgDesc& msg);
    
    static GameServerEvent* CreatePlayerTargetInfoChangeEvent(Game::InfoType s, const char* value);
    static GameServerEvent* CreatePlayerTargetStatChangeEvent(Game::StatType s, unsigned value);

protected:
    GameServerEvent(EventType type) : Type(type) {}
    // Copying is prohibited
    GameServerEvent(const GameServerEvent&);
    const GameServerEvent& operator = (const GameServerEvent&);
};

//////////////////////////////////////////////////////////////////////////

class GameEventListener
{
public:
    virtual void    OnGameEvent(GameServerEvent* event)   = 0;
    virtual ~GameEventListener() {};
};

class GameEventListenerSet
{
public:
    void                Add(GameEventListener* l)       { Listeners.PushBack(l); }
    void                Remove(GameEventListener* l);

    void                BroadcastEvent(GameServerEvent* e);

protected:
    Array<GameEventListener*>   Listeners;
};

#endif  // INC_GameEvent_H
