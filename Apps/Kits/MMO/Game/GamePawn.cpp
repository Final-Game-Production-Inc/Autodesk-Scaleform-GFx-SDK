/**************************************************************************

Filename    :   GamePawn.cpp
Content     :   
Created     :	2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include "Kernel/SF_Debug.h"
#include "../Game.h"
#include "GamePawn.h"



//////////////////////////////////////////////////////////////////////////

unsigned    GamePawn::GetXpToLevel(unsigned lvl)
{
    return (unsigned)pow(1.5f, (float)lvl);
}

unsigned    GamePawn::GetLevelFromXp(unsigned xp)
{
    int leftover = xp;
    unsigned level = 0;
    while (leftover > 0)
    {
        leftover = xp;
        level++;
        leftover -= GetXpToLevel(level);
    }
    return level;
}

const char*     GamePawn::GetInfo(Game::InfoType p)
{
    switch (p)
    {
    case Game::INFO_ID:    return ID;
    case Game::INFO_Name:  return Info.Name;
    case Game::INFO_Class: return Info.Class;
    case Game::INFO_Race:  return Info.Race;
    default:
        {
            SF_ASSERT(0);
        }
    }
    return NULL;
}

unsigned    GamePawn::GetStat(Game::StatType s)
{
    unsigned val = 0;

    switch (s)
    {
    case Game::STAT_Hp:           
        return CurrentHP;

    case Game::STAT_MaxHp:
        val = Level * (Info.Endurance * 125);// + CurrentBuff.HP;
        break;

    case Game::STAT_Mp:           
        return CurrentMP;

    case Game::STAT_MaxMp:
        val = Level * (Info.Wisdom * 200);// + CurrentBuff.MP;
        break;

    case Game::STAT_Strength:     
        val = Info.Strength * Level;// + CurrentBuff.Strength;
        break;

    case Game::STAT_Dexterity:
        val = Info.Dexterity * Level;// + CurrentBuff.Dexterity;
        break;

    case Game::STAT_Intelligence:
        val = Info.Intelligence * Level;// + CurrentBuff.Intelligence;
        break;

    case Game::STAT_Endurance:
        val = Info.Endurance * Level;// + CurrentBuff.Endurance;
        break;

    case Game::STAT_Wisdom:
        val = Info.Wisdom * Level;// + CurrentBuff.Wisdom;
        break;

    case Game::STAT_Luck:
        val = Info.Luck * Level;// + CurrentBuff.Luck;
        break;

    case Game::STAT_Level:        
        val = Level;
        break;

    case Game::STAT_Xp:
        val = XP;
        break;

    default: 
        {
            SF_ASSERT(0);
        }
    }

    return val;
}

unsigned GamePawn::GetMoney(Game::MoneyType m)
{
    unsigned val = 0;

    switch (m)
    {
    case Game::MONEY_Copper: 
        val = Money.GetAmount(Game::MONEY_Copper);
        break;

    case Game::MONEY_Silver:
        val = Money.GetAmount(Game::MONEY_Silver);
        break;

    case Game::MONEY_Gold:
        val = Money.GetAmount(Game::MONEY_Gold);
        break;

    default:
        {
            SF_ASSERT(0);
        }
    }

    return val;
}

void    GamePawn::SetTarget(GamePawn* target)
{
    GamePawn* oldt = Target;
    Target = target;
    notifyTargetChange(oldt, Target);
}

bool    GamePawn::AddHP(unsigned hp)
{ 
    unsigned maxhp = GetStat(Game::STAT_MaxHp);
    if (CurrentHP == maxhp) return false;
    unsigned newhp = CurrentHP += hp;
    if (newhp > maxhp) newhp = maxhp;
    CurrentHP = newhp;
    notifyStatChange(Game::STAT_Hp);
    return true;
}

bool    GamePawn::AddMP(unsigned mp)
{ 
    unsigned maxmp = GetStat(Game::STAT_MaxMp);
    if (CurrentMP == maxmp) return false;
    unsigned newmp = CurrentMP += mp;
    if (newmp > maxmp) newmp = maxmp;
    CurrentMP = newmp;
    notifyStatChange(Game::STAT_Mp);
    return true;
}


GamePawn::GamePawn(const BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai, Type type)
: mType(type), Info(baseInfo), LastTicks(0)
{ 
    XP = xp;
    Level = GamePawn::GetLevelFromXp(xp);

    CurrentHP = GetStat(Game::STAT_MaxHp);
    CurrentMP = GetStat(Game::STAT_MaxMp);

    Behavior = ai;

    SF_DEBUG_MESSAGE2(1, "Created pawn '%s' : level %d\n", Info.Name.ToCStr(), Level);
}

GamePawn::~GamePawn()
{
    SF_ASSERT(Manager);
    Manager->onPawnDestroy(this);
}

void    GamePawn::AddEventListener(ListenerType* l)
{
    Listeners.PushBack(l);
}

void    GamePawn::RemoveEventListener(ListenerType* l)
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

UInt32    GamePawn::Update(GameServer* env, UInt32 ticks)
{
    if (Behavior)
        Behavior->Process(env, ticks, this);

    if (LastTicks == 0) LastTicks = ticks;
    UInt32 ticksDelta = ticks - LastTicks;

    LastTicks = ticks;

    return ticksDelta;
}



//////////////////////////////////////////////////////////////////////////

GamePlayerPawn::GamePlayerPawn(const BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai)
: GamePawn(baseInfo, xp, ai, PAWN_Player), Guild(NULL)
{ 
    SpellBook = *SF_NEW GamePawnAbilityContainer(this);
    Equipment = *SF_NEW GamePawnEquipment(this);
    BagContainer = *SF_NEW GamePawnBagContainer(this);
}

void    GamePlayerPawn::HandleTargettedMessage(Game::MessageType type, const GameChannelMessage& message)
{
    if (Behavior != NULL && type == Game::MSG_Whisper)
        Behavior->OnWhisper(message);
    notifyMessageReceive(type, "", message);
}

void    GamePlayerPawn::OnBroadcastedChannelMessage(GameMessageChannel* channel, const GameChannelMessage& message)
{
    notifyMessageReceive(channel->GetType(), channel->GetID(), message);
}

void    GamePlayerPawn::SubscribeToChannel(GameMessageChannel* channel)
{
    channel->AddSubscriber(this);
    SubscribedChannels.PushBack(channel);
}

void    GamePlayerPawn::SwapSlots(const Game::EntitySlotDescriptor& src, const Game::EntitySlotDescriptor& dest)
{
    GameItemSlot* srcSlot = getSlotFromDescriptor(src);
    GameItemSlot* destSlot = getSlotFromDescriptor(dest);

    srcSlot->SwapWith(destSlot);
}

void    GamePlayerPawn::UseSlot(GameServer* env, const Game::EntitySlotDescriptor& slot)
{
    GameItemSlot* slotRef = getSlotFromDescriptor(slot);
    env->UseEntity(slotRef->GetEntity(), this, Target);
}

void    GamePlayerPawn::UseAbility(GameServer* env, Game::AbilityID abilId)
{
    SF_UNUSED(env);

    SpellBook->UseAbility(abilId);
}

GameItemSlot* GamePlayerPawn::getSlotFromDescriptor(const Game::EntitySlotDescriptor& desc)
{
    switch (desc.Type)
    {
    case Game::SLOT_Entity:
        {
            for (unsigned i = 0; i < Game::NumInventorySlots; i++)
            {
                GameItem* e = BagContainer->GetSlot(i)->GetEntity();
                if (e && desc.EntityID == e->GetInstanceID()) 
                    return e->GetSlotContainer()->GetSlot(desc.Index);
            }
            break;
        }
    case Game::SLOT_Equipment:
        {
            return Equipment->GetSlot(desc.Index);
            break;
        }
    case Game::SLOT_Inventory:
        {
            return BagContainer->GetSlot(desc.Index);
            break;
        }
    default:
        {
            SF_ASSERT(0);            
        }
    }
    return NULL;
}

UInt32    GamePlayerPawn::Update(GameServer* env, UInt32 ticks)
{
    UInt32 ticksDelta = GamePawn::Update(env, ticks);

    SpellBook->Update(ticksDelta);

    return ticksDelta;
}


//////////////////////////////////////////////////////////////////////////

void    GamePawn::notifyTargetChange(GamePawn* oldt, GamePawn* newt)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnTargetChange(this, oldt, newt);
}

void    GamePawn::notifyStatChange(Game::StatType s)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnStatChange(this, s);
}

void    GamePawn::notifyMoneyChange(Game::MoneyType m)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnMoneyChange(this, m);
}

void    GamePawn::notifyInventoryChange(Game::InventorySlotType inv)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnInventoryChange(this, inv);
}

void    GamePawn::notifyBagItemChange(Game::InventorySlotType inv, unsigned slotIdx)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnBagItemChange(this, inv, slotIdx);
}

void    GamePawn::notifyEquipmentChange(Game::EquipmentSlotType eq)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnEquipmentChange(this, eq);
}

void    GamePawn::notifyAbilityAdd(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnAbilityAdd(this, desc);
}

void    GamePawn::notifyAbilityCastStart(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnCastStart(this, desc);
}

void    GamePawn::notifyAbilityCastInterrupt(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnCastInterrupt(this, desc);
}

void    GamePawn::notifyAbilityCastEnd(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnCastEnd(this, desc);
}

void    GamePawn::notifyAbilityCooldownStart(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnAbilityCooldownStart(this, desc);
}

void    GamePawn::notifyAbilityCooldownEnd(GamePawnAbilityDesc* desc)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnAbilityCooldownEnd(this, desc);
}

void    GamePawn::notifyMessageReceive(Game::MessageType m, const char* chanId, const GameChannelMessage& msg)
{
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnPawnMessageReceived(this, m, chanId, msg);
}


//////////////////////////////////////////////////////////////////////////

static void     GetRandomString(String* sink, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
        sink->AppendChar((char)(rand()%10+48));
}

static void    CreatePawnID(String* sink, unsigned idx)
{
    String key, nameNoSpace;
    GetRandomString(&key, 3);
    Format(*sink, "pawn#{0}{1}", key, idx);
}

GamePawn*   GamePawnFactory::CreateInstance(GamePawn::Type type, const GamePawn::BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai)
{
    // Reserve a new storage index
    unsigned newIDx = IdCounter++;

    // Generate a 'unique' id
    String newIDStr;
    CreatePawnID(&newIDStr, newIDx);

    // Create the instance
    GamePawn*   newItem = NULL;
    switch (type) {
        case GamePawn::PAWN_Basic:      newItem = SF_NEW GamePawn(baseInfo, xp, ai); break;
        case GamePawn::PAWN_Player:   newItem = SF_NEW GamePlayerPawn(baseInfo, xp, ai); break;
    }
    SF_ASSERT(newItem);
    newItem->ID = newIDStr;

    // Configure lifetime management members
    newItem->Manager = this;

    SF_DEBUG_MESSAGE2(1, "Created pawn instance [%s]@%s\n", newItem->GetInfo(Game::INFO_Name), newItem->GetInfo(Game::INFO_ID));
    return newItem;
}

void    GamePawnFactory::onPawnDestroy(GamePawn* item)
{
    SF_UNUSED(item);
    SF_DEBUG_MESSAGE2(1, "Destroyed pawn instance [%s]@%s\n", item->GetInfo(Game::INFO_Name), item->GetInfo(Game::INFO_ID));
}