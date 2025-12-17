/**************************************************************************

Filename    :   GameServer.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameServer_H
#define INC_GameServer_H

#include "Kernel/SF_Threads.h"

#include "GameSimulation.h"
#include "GameEntity.h"
#include "GamePawn.h"
#include "GameEvent.h"
#include "GameConnection.h"

using namespace Scaleform;

class GameServer;

//////////////////////////////////////////////////////////////////////////

class GameServerConnectedClient : public RefCountBase<GameServerConnectedClient, Stat_Default_Mem>, 
                                  public GamePawn::ListenerType
{
public:
    GameServerConnectedClient(GameConnection* conn, GameServer* server, GamePlayerPawn* pawn);

    void            Initialize();
    GamePlayerPawn* GetPawn() const     { return PawnRef; }

    virtual void    OnPawnTargetChange(GamePawn* pawn, GamePawn* oldt, GamePawn* newt);
    virtual void    OnPawnStatChange(GamePawn* pawn, Game::StatType s);
    virtual void    OnPawnMoneyChange(GamePawn* pawn, Game::MoneyType m);
    virtual void    OnPawnInventoryChange(GamePawn* pawn, Game::InventorySlotType inv);
    virtual void    OnPawnBagItemChange(GamePawn* pawn, Game::InventorySlotType inv, unsigned slotIdx);
    virtual void    OnPawnEquipmentChange(GamePawn* pawn, Game::EquipmentSlotType eq);
    virtual void    OnPawnAbilityAdd(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnAbilityRemove(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnCastStart(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnCastInterrupt(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnCastEnd(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnAbilityCooldownStart(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnAbilityCooldownEnd(GamePawn* pawn, GamePawnAbilityDesc* desc);
    virtual void    OnPawnMessageReceived(GamePawn* pawn, Game::MessageType m, const char* chanId, const GameChannelMessage& msg);

protected:
    GameConnection*     Connection;
    GameServer*         Server;
    GamePlayerPawn*     PawnRef;

    void            broadcastPlayerAll();
    void            broadcastTargetAll();

    void            broadcastInfoAll(bool target = false);
    void            broadcastStatAll(bool target = false);
    void            broadcastMoneyAll();
    void            broadcastInventoryAll();
    void            broadcastEquipmentAll();
    void            broadcastAbilitiesAll();
    void            broadcastSubscribedChannelsAll();

    void            createEntityDesc(GameServerEvent::EntityDesc* desc, GameItem* entity);
};


//////////////////////////////////////////////////////////////////////////
//
//
class GameServer : public Thread
{
public:
    GameServer(GameConnection* conn);

    virtual int     Run();

    void            SetupEntity(GameItem* entity);
    bool            UseEntity(GameItem* entity, GamePawn* player, GamePawn* target);

    // Create an entity instance by id. A unique entity ID is assigned to it for the duration of its lifetime.
    GameItem*     CreateEntityInstance(Game::EntityBaseClass cls, unsigned id, unsigned stackSz = 1);

    GameMessageChannel*    GetChannel(const char* name);

    GameGuild*      GetGuild(const char* name);

    void            SendWhisperToPlayer(const char* name, const GameChannelMessage& msg);

    void            ProcessClientRequest(GameClientRequest* request);

protected:
    GameConnection*                         Connection;
    Array< Ptr<GameServerConnectedClient> > ConnectedClients;

    Hash< String, Ptr<GameMessageChannel>, String::HashFunctor >   Channels;

    Array< Ptr<GamePlayerPawn> >  ActivePlayerPawns;
    Array< Ptr<GamePawn> >        ActiveMobPawns;
    GamePawnFactory         PawnFactory;

    GameSimulation          Simulation;

    GameItemFactory           EntityFactory;
    Array< Ptr<GameItem> >    Entities;

    GameMessageChannelFactory  ChannelFactory;

    Hash< String, Ptr<GameGuild>, String::HashFunctor >     Guilds;

    GamePlayerPawn* createRandomPlayerPawn(const char* name, unsigned lvl, GameAiBehavior* ai = NULL);
    GamePawn*       createRandomMobPawn(const char* name, unsigned lvl, bool isElite);

    GamePlayerPawn* getPlayerByName(const char* name);

    GameMessageChannel*    createChatChannel(const char* id, Game::MessageType m, const char* name);

    GameServerConnectedClient*   getClientConnection(unsigned clientId);
};


#endif  // INC_GameServer_H
