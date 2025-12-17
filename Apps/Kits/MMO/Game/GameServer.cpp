/**************************************************************************

Filename    :   GameServer.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <time.h>

#include "Kernel/SF_Debug.h"

#include "GameServer.h"
#include "GameClient.h"
#include "GamePawn.h"
#include "GameChatChannel.h"


GameServerConnectedClient::GameServerConnectedClient(GameConnection* conn, GameServer* server, GamePlayerPawn* pawn)
{
    Connection = conn;
    Server = server;
    PawnRef = pawn;
}

void    GameServerConnectedClient::Initialize()
{
    PawnRef->AddEventListener(this);
    broadcastPlayerAll();
    broadcastTargetAll();

    GameMessageChannel* SystemBuffer = Server->GetChannel("system");
    GameChannelMessage msg;
    msg.Message = "Connected!";
    PawnRef->OnBroadcastedChannelMessage(SystemBuffer, msg);
    msg.Message = "Welcome to the Scaleform MMO Kit! Please refrain from illegal activities. Maintenance may be scheduled in the next 15 minutes.";
    PawnRef->OnBroadcastedChannelMessage(SystemBuffer, msg);
}

void    GameServerConnectedClient::OnPawnTargetChange(GamePawn* pawn, GamePawn* oldt, GamePawn* newt)
{
    SF_UNUSED2(oldt, newt);

    if (pawn != PawnRef) return;
    broadcastTargetAll();
}

void    GameServerConnectedClient::OnPawnStatChange(GamePawn* pawn, Game::StatType s)
{
    GameServerEvent* e = NULL;
    if (pawn == PawnRef)
        e = GameServerEvent::CreatePlayerStatChangeEvent(s, pawn->GetStat(s));
    else
        e = GameServerEvent::CreatePlayerTargetStatChangeEvent(s, pawn->GetStat(s));
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnMoneyChange(GamePawn* pawn, Game::MoneyType m)
{
    if (pawn != PawnRef) return;
    GameServerEvent* e = GameServerEvent::CreatePlayerMoneyChangeEvent(m, pawn->GetMoney(m));
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnInventoryChange(GamePawn* pawn, Game::InventorySlotType inv)
{
    if (pawn != PawnRef) return;

    unsigned i = inv;
    GameItem* entity = PawnRef->GetBagContainer()->GetSlot(i)->GetEntity();
    GameServerEvent::EntityDesc ed;
    unsigned numSlots = 0;
    if (entity) 
    {
        createEntityDesc(&ed, entity);
        numSlots = entity->GetSlotContainer()->GetNumSlots();
    }
    GameServerEvent* e = GameServerEvent::CreatePlayerInventoryChangeEvent(inv, ed, numSlots);
    Connection->SendEvent(e);
    if (entity)
    {
        // Broadcast entity slots
        for (unsigned j = 0; j < numSlots; j++)
        {
            GameServerEvent::EntityDesc idesc;
            GameItem* ei = entity->GetSlotContainer()->GetSlot(j)->GetEntity();
            if (ei)
                createEntityDesc(&idesc, ei);
            GameServerEvent* ese = GameServerEvent::CreatePlayerBagItemChangeEvent(inv, idesc, j);
            Connection->SendEvent(ese);
        }
    }
}

void    GameServerConnectedClient::OnPawnBagItemChange(GamePawn* pawn, Game::InventorySlotType inv, unsigned slotIdx)
{
    if (pawn != PawnRef) return;

    unsigned i = inv, j = slotIdx;
    GameItem* entity = PawnRef->GetBagContainer()->GetSlot(i)->GetEntity();
    GameServerEvent::EntityDesc idesc;
    GameItem* ei = entity->GetSlotContainer()->GetSlot(j)->GetEntity();
    if (ei)
        createEntityDesc(&idesc, ei);
    GameServerEvent* e = GameServerEvent::CreatePlayerBagItemChangeEvent((Game::InventorySlotType)i, idesc, j);
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnEquipmentChange(GamePawn* pawn, Game::EquipmentSlotType eq)
{
    if (pawn != PawnRef) return;

    unsigned i = eq;
    GameItem* entity = PawnRef->GetEquipment()->GetSlot(i)->GetEntity();
    GameServerEvent::EntityDesc ed;
    if (entity) 
        createEntityDesc(&ed, entity);
    GameServerEvent* e = GameServerEvent::CreatePlayerEquipmentChangeEvent(eq, ed);
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnAbilityAdd(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityAddEvent(desc->GetDefnID(), desc->GetRank());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnAbilityRemove(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityRemoveEvent(desc->GetDefnID());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnCastStart(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityCastStartEvent(desc->GetDefnID());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnCastInterrupt(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityCastInterruptEvent(desc->GetDefnID());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnCastEnd(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityCastEndEvent(desc->GetDefnID());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnAbilityCooldownStart(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityCooldownStartEvent(desc->GetDefnID(), desc->GetCooldown().GetTicksLeft());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnAbilityCooldownEnd(GamePawn* pawn, GamePawnAbilityDesc* desc)
{
    if (pawn != PawnRef) return;

    GameServerEvent* e = GameServerEvent::CreatePlayerAbilityCooldownEndEvent(desc->GetDefnID());
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::OnPawnMessageReceived(GamePawn* pawn, Game::MessageType m, const char* chanId, const GameChannelMessage& msg)
{
    if (pawn != PawnRef) return;

    time_t  rawtime;
    time(&rawtime);

    GameServerEvent* e = GameServerEvent::CreateChannelMessageEvent(m, chanId, rawtime, msg);
    Connection->SendEvent(e);
}

void    GameServerConnectedClient::broadcastPlayerAll()
{
    broadcastInfoAll();
    broadcastStatAll();
    broadcastMoneyAll();
    broadcastInventoryAll();
    broadcastEquipmentAll();
    broadcastAbilitiesAll();
    broadcastSubscribedChannelsAll();
}

void    GameServerConnectedClient::broadcastTargetAll()
{
    broadcastInfoAll(true);
    broadcastStatAll(true);
}

void    GameServerConnectedClient::broadcastSubscribedChannelsAll()
{
    const Array<GameMessageChannel*> chans = PawnRef->GetSubscribedChannels();
    for (unsigned i = 0; i < chans.GetSize(); i++)
    {
        GameServerEvent* e = GameServerEvent::CreateChannelSubscribedEvent(chans[i]->GetType(), chans[i]->GetID(), chans[i]->GetName());
        Connection->SendEvent(e);
    }
}

void    GameServerConnectedClient::broadcastInfoAll(bool target /* = false */)
{
    if (target)
    {
        GamePawn* gpTarget = PawnRef->GetTarget();
        if (!gpTarget) return;
        for (unsigned i = 0; i < Game::NumInfoTypes; i++)
        {
            GameServerEvent* e = GameServerEvent::CreatePlayerTargetInfoChangeEvent((Game::InfoType)i, gpTarget->GetInfo((Game::InfoType)i));
            Connection->SendEvent(e);
        }
    }
    else
    {
        for (unsigned i = 0; i < Game::NumInfoTypes; i++)
        {
            GameServerEvent* e = GameServerEvent::CreatePlayerInfoChangeEvent((Game::InfoType)i, PawnRef->GetInfo((Game::InfoType)i));
            Connection->SendEvent(e);
        }
    }
}

void    GameServerConnectedClient::broadcastStatAll(bool target /* = false */)
{
    if (target)
    {
        GamePawn* gpTarget = PawnRef->GetTarget();
        if (!gpTarget) return;
        for (unsigned i = 0; i < Game::NumStatTypes; i++)
        {
            GameServerEvent* e = GameServerEvent::CreatePlayerTargetStatChangeEvent((Game::StatType)i, gpTarget->GetStat((Game::StatType)i));
            Connection->SendEvent(e);
        }
    }
    else
    {
        for (unsigned i = 0; i < Game::NumStatTypes; i++)
        {
            GameServerEvent* e = GameServerEvent::CreatePlayerStatChangeEvent((Game::StatType)i, PawnRef->GetStat((Game::StatType)i));
            Connection->SendEvent(e);
        }
    }
}

void    GameServerConnectedClient::broadcastMoneyAll()
{
    for (unsigned i = 0; i < Game::NumMoneyTypes; i++)
    {
        GameServerEvent* e = GameServerEvent::CreatePlayerMoneyChangeEvent((Game::MoneyType)i, PawnRef->GetMoney((Game::MoneyType)i));
        Connection->SendEvent(e);
    }
}

void    GameServerConnectedClient::broadcastEquipmentAll()
{
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        GameItem* entity = PawnRef->GetEquipment()->GetSlot(i)->GetEntity();
        GameServerEvent::EntityDesc ed;
        if (entity) 
            createEntityDesc(&ed, entity);
        GameServerEvent* e = GameServerEvent::CreatePlayerEquipmentChangeEvent((Game::EquipmentSlotType)i, ed);
        Connection->SendEvent(e);
    }
}

void    GameServerConnectedClient::broadcastInventoryAll()
{
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameItem* entity = PawnRef->GetBagContainer()->GetSlot(i)->GetEntity();
        GameServerEvent::EntityDesc desc;
        unsigned numSlots = 0;
        if (entity) 
        {
            createEntityDesc(&desc, entity);
            numSlots = entity->GetSlotContainer()->GetNumSlots();
        }        
        GameServerEvent* e = GameServerEvent::CreatePlayerInventoryChangeEvent((Game::InventorySlotType)i, desc, numSlots);
        Connection->SendEvent(e);
        if (e)
        {
            // Broadcast entity slots
            for (unsigned j = 0; j < numSlots; j++)
            {
                GameServerEvent::EntityDesc idesc;
                GameItem* ei = entity->GetSlotContainer()->GetSlot(j)->GetEntity();
                if (ei)
                    createEntityDesc(&idesc, ei);
                GameServerEvent* ese = GameServerEvent::CreatePlayerBagItemChangeEvent((Game::InventorySlotType)i, idesc, j);
                Connection->SendEvent(ese);
            }
        }
    }
}

void    GameServerConnectedClient::broadcastAbilitiesAll()
{
    GamePawnAbilityContainer* abilc = PawnRef->GetSpellBook();
    for (unsigned i = 0; i < abilc->GetNumAbilities(); i++)
    {
        GamePawnAbilityDesc* abil = abilc->GetAbility(i);;
        GameServerEvent* e = GameServerEvent::CreatePlayerAbilityAddEvent(abil->GetDefnID(), abil->GetRank());
        Connection->SendEvent(e);
    }
}

void    GameServerConnectedClient::createEntityDesc(GameServerEvent::EntityDesc* desc, GameItem* entity)
{
    *desc = GameServerEvent::EntityDesc(entity->GetInstanceID(), entity->GetDefnID(), entity->GetStackCount());
}



//////////////////////////////////////////////////////////////////////////

#define ADD_ABILITY(id, rank)   Ptr<GamePawnAbilityDesc> abil_##id = *SF_NEW GamePawnAbilityDesc(Game::id, rank); thePlayer->GetSpellBook()->AddAbility(abil_##id);
#define ADD_EQUIPMENT(cls, id, slot) e = *CreateEntityInstance(cls, id); thePlayer->GetEquipment()->GetSlot(slot)->SetEntity(e);

GameServer::GameServer(GameConnection* conn) : Connection(conn)
{
    // ** Setup the environment

    // Create channels
    Ptr<GameMessageChannel> chan;
    chan = *createChatChannel("system",         Game::MSG_System,  "System");
    chan = *createChatChannel("world",          Game::MSG_Other,   "World");
    chan = *createChatChannel("level:10-19",    Game::MSG_Other,   "Level 10-19");
    chan = *createChatChannel("trade",          Game::MSG_Other,   "Trade");
    chan = *createChatChannel("combat",         Game::MSG_Combat,  "Combat");

    // Create guilds
    Ptr<GameGuild> g;
    chan = *createChatChannel("guild:CrazyTrain", Game::MSG_Guild,   "CrazyTrain");
    g = *SF_NEW GameGuild("CrazyTrain", chan);    Guilds.Add(g->GetName(), g);
    chan = *createChatChannel("guild:TribesRulez", Game::MSG_Guild,   "TribesRulez");
    g = *SF_NEW GameGuild("TribesRulez", chan);   Guilds.Add(g->GetName(), g);

    // Create players    
    Ptr<GamePlayerPawn> p;
    Ptr<GameAiBehavior> ai;
    
    ai= *SF_NEW GameAiBehaviorChatterBox();
    p = *createRandomPlayerPawn("Zelias", 8, ai);
    g->EnrollPawn(p);
    
    ai = *SF_NEW GameAiBehaviorChatterBox();
    p = *createRandomPlayerPawn("Prometheus", 50, ai);
    g->EnrollPawn(p);
    
    ai = *SF_NEW GameAiBehaviorChatterBox();
    p = *createRandomPlayerPawn("Gregorius", 5, ai);

    ai = *SF_NEW GameAiBehaviorChatterBox();
    p = *createRandomPlayerPawn("BigDog", 29, ai);

    ai = *SF_NEW GameAiBehaviorChatterBox();
    p = *createRandomPlayerPawn("Scaleform", 4, ai);
    g->EnrollPawn(p);

    Ptr<GamePawn> m0 = *createRandomMobPawn("Greater Wraith", 18, true);
    Ptr<GamePawn> m1 = *createRandomMobPawn("Skeleton Guard", 17, false);

    // Set PC and target
    p = *createRandomPlayerPawn("Terranin", 15);
    g->EnrollPawn(p);
    GamePlayerPawn* thePlayer = p;
    thePlayer->SetTarget(m0);

    // Load PC's inventory
    SF_DEBUG_MESSAGE1(1, "Setting up inventory for '%s'\n", thePlayer->GetInfo(Game::INFO_Name));
    Ptr<GameItem> e;

    // - Create more containers
    GamePawnBagContainer* bagContainer = thePlayer->GetBagContainer();
    Ptr<GameItem> bagEnt1 = *CreateEntityInstance(Game::ENTITY_ContainerItem, Game::ID_CONT_LinenBag);
    bagContainer->GetSlot(1)->SetEntity(bagEnt1);
    Ptr<GameItem> bagEnt2 = *CreateEntityInstance(Game::ENTITY_ContainerItem, Game::ID_CONT_LinenBag);
    bagContainer->GetSlot(2)->SetEntity(bagEnt2);
    Ptr<GameItem> bagEnt3 = *CreateEntityInstance(Game::ENTITY_ContainerItem, Game::ID_CONT_LeatherPouch);
    bagContainer->GetSlot(3)->SetEntity(bagEnt3);
    // - Fill containers with some items
    GameItemSlotContainer* bag0 = bagContainer->GetSlot(0)->GetEntity()->GetSlotContainer();
    GameItemSlotContainer* bag1 = bagContainer->GetSlot(1)->GetEntity()->GetSlotContainer();
    GameItemSlotContainer* bag2 = bagContainer->GetSlot(2)->GetEntity()->GetSlotContainer();
    Ptr<GameItem> bagEnt, hp1, hp2;
    bagEnt = *CreateEntityInstance(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicHealthPotion, 5);    bag0->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicManaPotion, 8);      bag0->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_QuestItem,      Game::ID_QUES_MysteriousBook);          bag0->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicHealthPotion, 3);    bag0->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_MiscItem,       Game::ID_OTHE_HeavyStone, 4);           bag1->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_MiscItem,       Game::ID_OTHE_RaptorEgg);               bag1->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_MiscItem,       Game::ID_OTHE_Weeds, 66);               bag1->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_MiscItem,       Game::ID_OTHE_WolfBone, 18);            bag1->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_QuestItem,      Game::ID_QUES_GnarlsPelt);              bag1->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_WeaponItem,     Game::ID_WEAP_HammerOfRighteousness);   bag2->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_ArmorItem,      Game::ID_ARMO_BandOfHope);              bag2->AddItem(bagEnt);
    bagEnt = *CreateEntityInstance(Game::ENTITY_ArmorItem,      Game::ID_ARMO_GyroTimepiece);           bag2->AddItem(bagEnt);

    // Load PC's Equipment
    ADD_EQUIPMENT(Game::ENTITY_WeaponItem,      Game::ID_WEAP_SwordOfReckoning,         Game::EQS_MainHand);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_ShieldOfLight,            Game::EQS_OffHand);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_BeltOfRedemption,         Game::EQS_Waist);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_BootsOfRedemption,        Game::EQS_Feet);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_BreastplateOfRedemption,  Game::EQS_Chest);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_CloakOfRedemption,        Game::EQS_Back);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_GauntletsOfRedemption,    Game::EQS_Hands);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_HelmOfRedemption,         Game::EQS_Head);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_LeggingsOfRedemption,     Game::EQS_Legs);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_ShouldersOfRedemption,    Game::EQS_Shoulders);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_DarkAnkh,                 Game::EQS_Trinket0);
    ADD_EQUIPMENT(Game::ENTITY_WeaponItem,      Game::ID_WEAP_IronCrossbow,             Game::EQS_Ranged);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_ShadowForestRing,         Game::EQS_Ring1);
    ADD_EQUIPMENT(Game::ENTITY_ArmorItem,       Game::ID_ARMO_TearOfSacrifice,          Game::EQS_Neck);

    // Load PC's Abilities
    ADD_ABILITY(ID_ABIL_Attack,             1);
    ADD_ABILITY(ID_ABIL_RangedAttack,       1);    
    ADD_ABILITY(ID_ABIL_Adrenaline,         3);
    ADD_ABILITY(ID_ABIL_Portal,             2);
    ADD_ABILITY(ID_ABIL_Sprint,             1);
    ADD_ABILITY(ID_ABIL_Float,              1);
    ADD_ABILITY(ID_ABIL_PaladinsSacrifice,  2);
    ADD_ABILITY(ID_ABIL_Berserk,            3);
    ADD_ABILITY(ID_ABIL_OffensiveBanner,    1);
    ADD_ABILITY(ID_ABIL_Smite,              3);
    ADD_ABILITY(ID_ABIL_Raise,              2);
    ADD_ABILITY(ID_ABIL_Heal,               2);
    ADD_ABILITY(ID_ABIL_DivineLight,        2);
    ADD_ABILITY(ID_ABIL_AngelsBlessing,     1);
    ADD_ABILITY(ID_ABIL_Sanctuary,          1);
    ADD_ABILITY(ID_ABIL_Faith,              1);
    ADD_ABILITY(ID_ABIL_Protect,            1);
    ADD_ABILITY(ID_ABIL_Thunder,            2);
    ADD_ABILITY(ID_ABIL_FireBolt,           3);
    ADD_ABILITY(ID_ABIL_Blizzard,           1);
    ADD_ABILITY(ID_ABIL_Freeze,             2);
    ADD_ABILITY(ID_ABIL_Tsunami,            1);
    ADD_ABILITY(ID_ABIL_Tornado,            2);
    ADD_ABILITY(ID_ABIL_Quake,              2);
    ADD_ABILITY(ID_ABIL_QuantumField,       1);
    ADD_ABILITY(ID_ABIL_Leech,              2);
    ADD_ABILITY(ID_ABIL_Fear,               2);
    ADD_ABILITY(ID_ABIL_Curse,              1);
    ADD_ABILITY(ID_ABIL_Terror,             1);
    ADD_ABILITY(ID_ABIL_Weaken,             2);
    ADD_ABILITY(ID_ABIL_SoulBlast,          3);
    ADD_ABILITY(ID_ABIL_Plague,             1);
}

GamePlayerPawn*     GameServer::createRandomPlayerPawn(const char* name, unsigned lvl, GameAiBehavior* ai)
{
    // TODO: Use player racial and class stats for base
    GamePawn::BaseInfo info;
    info.Name = name;
    info.Class = "Paladin";
    info.Race = "Human";
    info.Strength = rand() % 10 + 5;
    info.Dexterity = rand() % 10 + 5;
    info.Intelligence = rand() % 10 + 5;
    info.Endurance = rand() % 10 + 5;
    info.Wisdom = rand() % 10 + 5;
    info.Luck = rand() % 10 + 5;

    GamePlayerPawn* player = (GamePlayerPawn*)PawnFactory.CreateInstance(GamePawn::PAWN_Player, info, GamePawn::GetXpToLevel(lvl), ai);

    // Subscribe to channels
    player->SubscribeToChannel(GetChannel("system"));
    player->SubscribeToChannel(GetChannel("world"));
    player->SubscribeToChannel(GetChannel("level:10-19"));
    player->SubscribeToChannel(GetChannel("trade"));
    player->SubscribeToChannel(GetChannel("combat"));

    // Add player to simulation
    Simulation.AddPawn(player);

    // Keep track of the pawns
    ActivePlayerPawns.PushBack(player);

    // Add a backpack to the player
    Ptr<GameItem> backpack = *CreateEntityInstance(Game::ENTITY_ContainerItem, Game::ID_CONT_Backpack);
    GamePawnBagContainer* bc = player->GetBagContainer();
    GameContainerSlot* slot = bc->GetSlot(0);
    slot->SetEntity(backpack);
 
     // Give some cash to the player
    GameMoneyContainer* mc = player->GetMoneyContainer();
    mc->AddAmount(Game::MONEY_Copper, rand() % 100);
    mc->AddAmount(Game::MONEY_Silver, rand() % 100);
    mc->AddAmount(Game::MONEY_Gold, rand() % 500);

    return player;
}

GamePawn*   GameServer::createRandomMobPawn(const char* name, unsigned lvl, bool isElite)
{
    SF_UNUSED(isElite);

    GamePawn::BaseInfo info;
    info.Name = name;
    info.Class = "Monster";
    info.Race = "Monster";
    info.Strength = rand() % 10 + 5;
    info.Dexterity = rand() % 10 + 5;
    info.Intelligence = rand() % 10 + 5;
    info.Endurance = rand() % 10 + 5;
    info.Wisdom = rand() % 10 + 5;
    info.Luck = rand() % 10 + 5;

    GamePawn* mob = PawnFactory.CreateInstance(GamePawn::PAWN_Basic, info, GamePawn::GetXpToLevel(lvl), NULL);

    // Add mob to simulation
    Simulation.AddPawn(mob);

    // Keep track of the pawns
    ActiveMobPawns.PushBack(mob);

    return mob;
}

GameItem*   GameServer::CreateEntityInstance(Game::EntityBaseClass cls, unsigned id, unsigned stackSz)
{
    Game::EntityDefnID defnId;
    defnId.Class = cls;
    defnId.ID = id;
    GameItem* e = EntityFactory.CreateInstance(this, defnId, stackSz);
    Entities.PushBack(e);
    return e;
}

GameMessageChannel*    GameServer::createChatChannel(const char* id, Game::MessageType m, const char* name)
{
    GameMessageChannel* chan = ChannelFactory.CreateChannel(id, m, name);
    Channels.Add(id, chan);
    return chan;
}

void    GameServer::SetupEntity(GameItem* entity)
{
    // We only need to setup containers for now
    if (entity->GetDefnID().Class == Game::ENTITY_ContainerItem)
    {
        Game::IDDescType* desc = Game::GetDescForID(entity->GetDefnID());
        if (desc->Capacity == 0) return;
        GameItemSlotContainer* sc = entity->GetSlotContainer();
        for (unsigned i = 0; i < desc->Capacity; i++)
        {
            Ptr<GameInventorySlot> slot = *SF_NEW GameInventorySlot(entity->GetInstanceID(), i);
            sc->AddSlot(slot);
        }
    }
}

bool    GameServer::UseEntity(GameItem* entity, GamePawn* pawn, GamePawn* target)
{
    // TODO: Incorporate target somewhere. Maybe desc type should have a bool for self or target?
    SF_UNUSED(target);

    Game::IDDescType* d = Game::GetDescForID(entity->GetDefnID());
    if (d->Usable)
    {
        switch (entity->GetDefnID().Class)
        {
        case Game::ENTITY_ConsumableItem:
            {
                Game::ConsumableDescType* consDesc = Game::GetConsumableDescForID((Game::ConsumableID)entity->GetDefnID().ID);
                SF_UNUSED(consDesc);
                
                // TODO: For now just consume 
                entity->Consume(1);

                return true;
            }
        }    
    }

    if (pawn->GetType() == GamePawn::PAWN_Player) 
    {
        GameChannelMessage errMsg;
        Game::IDStringsType* s = Game::GetStringsForID(entity->GetDefnID());
        Format(errMsg.Message, "Cannot use {0}!", ConvToUTF8(s->Name).ToCStr());
        ((GamePlayerPawn*)pawn)->HandleTargettedMessage(Game::MSG_Error, errMsg);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

int     GameServer::Run()
{
    while (!GetExitFlag())
    {
        // Check for pending requests
        GameClientRequest* r = Connection->GetNextRequest();
        while (r) 
        {
            ProcessClientRequest(r);
            delete r;
            r = Connection->GetNextRequest();
        }

        // Tick the simulation
        UInt32 ticks = Timer::GetTicksMs();
        Simulation.Update(this, ticks);

        // Yield
        Thread::MSleep(1);
    }

    return 0;
}

void    GameServer::SendWhisperToPlayer(const char* name, const GameChannelMessage& msg)
{
    GamePlayerPawn* targetPlayer = getPlayerByName(name);
    GamePlayerPawn* srcPlayer = getPlayerByName(msg.SourceName);
    SF_ASSERT(srcPlayer);
    if (!targetPlayer)
    {
        GameChannelMessage errMsg;
        Format(errMsg.Message, "Cannot find player %s!", name);
        srcPlayer->HandleTargettedMessage(Game::MSG_Error, errMsg);
        return;
    }
    else
    {
        targetPlayer->HandleTargettedMessage(Game::MSG_Whisper, msg);
        if (targetPlayer != srcPlayer)
            srcPlayer->HandleTargettedMessage(Game::MSG_Whisper, msg);
    }
}

void    GameServer::ProcessClientRequest(GameClientRequest* request)
{
    switch (request->Type)
    {
    case GameClientRequest::GR_PlayerLogin:
        {
            GamePlayerPawn* player = getPlayerByName(request->SValue);
            SF_ASSERT(player);
            unsigned clientId = (unsigned)ConnectedClients.GetSize() + 1;
            Ptr<GameServerConnectedClient> clientConn = *SF_NEW GameServerConnectedClient(Connection, this, player);
            ConnectedClients.PushBack(clientConn);
            GameServerEvent* e = GameServerEvent::CreateLoginSuccessEvent(clientId);
            Connection->SendEvent(e);
            clientConn->Initialize();
        } break;
    case GameClientRequest::GR_PlayerSlotSwap:
        {
            GameServerConnectedClient* clientConn = getClientConnection(request->ClientID);
            if (clientConn) clientConn->GetPawn()->SwapSlots(request->SrcSlot, request->DestSlot);
        } break;
    case GameClientRequest::GR_PlayerAbilityUse:
        {
            GameServerConnectedClient* clientConn = getClientConnection(request->ClientID);
            if (clientConn) clientConn->GetPawn()->UseAbility(this, (Game::AbilityID)request->UIValue);
            break;
        }
    case GameClientRequest::GR_PlayerEntitySlotUse:
        {
            GameServerConnectedClient* clientConn = getClientConnection(request->ClientID);
            GamePlayerPawn* player = clientConn->GetPawn();
            player->UseSlot(this, request->SrcSlot);
            break;
        }
    case GameClientRequest::GR_PlayerSendMessage:
        {
            GameServerConnectedClient* clientConn = getClientConnection(request->ClientID);
            GamePlayerPawn* player = clientConn->GetPawn();
            SF_ASSERT(player);
            if (clientConn)
            {
                GameChannelMessage msg;
                msg.SourceId = player->GetInfo(Game::INFO_ID);
                msg.SourceName = player->GetInfo(Game::INFO_Name);
                msg.Message = request->Message;

                switch (request->MsgType)
                {
                case Game::MSG_Guild:
                    {
                        GameGuild* guild = player->GetGuild();
                        if (guild)
                            guild->Broadcast(msg);
                        else 
                        {
                            GameChannelMessage errMsg;
                            errMsg.Message = "You are not in a guild!";
                            player->HandleTargettedMessage(Game::MSG_Error, errMsg);
                        }
                    }
                    break;

                case Game::MSG_Party:
                    {
                        // TODO: No party support yet
                        GameChannelMessage errMsg;
                        errMsg.Message = "You are not in a party!";
                        player->HandleTargettedMessage(Game::MSG_Error, errMsg);
                    }
                    break;

                case Game::MSG_Say:
                    {
                        Simulation.BroadcastVicinityMessage(msg, false);
                    }
                    break;

                case Game::MSG_Yell:
                    {
                        Simulation.BroadcastVicinityMessage(msg, true);
                    }
                    break;

                case Game::MSG_Whisper:
                    {
                        if (request->Target.IsEmpty())
                        {
                            GameChannelMessage errMsg;
                            errMsg.Message = "No target specified!";
                            player->HandleTargettedMessage(Game::MSG_Error, errMsg);
                        }
                        else
                        {
                            GamePlayerPawn* target = getPlayerByName(request->Target);
                            if (!target)
                            {
                                GameChannelMessage errMsg;
                                Format(errMsg.Message, "Player {0} not found!", request->Target);
                                player->HandleTargettedMessage(Game::MSG_Error, errMsg);
                            }
                            else
                            {
                                target->HandleTargettedMessage(Game::MSG_Whisper, msg);
                                if (target != player)
                                    player->HandleTargettedMessage(Game::MSG_Whisper, msg);
                            }
                        }
                    }
                    break;

                case Game::MSG_Other:
                    {
                        const String& chanId = request->SValue;
                        GameMessageChannel* channel = GetChannel(chanId);
                        if (!channel)
                        {
                            GameChannelMessage errMsg;
                            errMsg.Message = "No such channel exists!";
                            player->HandleTargettedMessage(Game::MSG_Error, errMsg);    
                        }
                        else
                            channel->Broadcast(msg);
                    }
                    break;

                default:
                    // Client should never send these messages!
                    SF_ASSERT(0);
                }
            }
            break;
        }
    default:
        {
            SF_ASSERT(0);
        }
    }
}


//////////////////////////////////////////////////////////////////////////

GamePlayerPawn*   GameServer::getPlayerByName(const char* name)
{
    for (unsigned i = 0; i < ActivePlayerPawns.GetSize(); i++)
    {
        if (!SFstricmp(ActivePlayerPawns[i]->GetInfo(Game::INFO_Name), name))
            return ActivePlayerPawns[i];
    }
    return NULL;
}

GameServerConnectedClient*   GameServer::getClientConnection(unsigned clientId)
{
    if (clientId <= 0 || clientId > ConnectedClients.GetSize()) return NULL;
    return ConnectedClients[clientId - 1];
}


GameMessageChannel*    GameServer::GetChannel(const char* name)
{
    Ptr<GameMessageChannel>* p = Channels.Get(name);
    if (p)
        return *p;
    return NULL;
}