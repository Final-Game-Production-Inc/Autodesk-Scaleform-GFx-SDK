/**************************************************************************

Filename    :   GameClientState.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include <time.h>

#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_Debug.h"
using namespace Scaleform;

#include "GameClientState.h"


//////////////////////////////////////////////////////////////////////////
// Returns a reference to a specific slot in the bag by its index.
//
GameClientSlotRef*  GameClientBagSlotRef::GetSlot(unsigned slotIdx)
{
    SF_ASSERT(slotIdx < ItemSlots.GetSize());
    if (slotIdx > ItemSlots.GetSize()) return NULL;
    return &ItemSlots[slotIdx];
}


//////////////////////////////////////////////////////////////////////////
// Sets a reference to a specific slot in the bag. The index information
// is in the slot descriptor.
//
void    GameClientBagSlotRef::SetSlot(const GameClientSlotDesc& desc, 
                                  const GameClientEntityRef& data)
{
    ItemSlots[desc.Index] = GameClientSlotRef(desc, data);
}


//////////////////////////////////////////////////////////////////////////
// Find an entity in the bag by its id.
//
GameClientEntityRef const*  GameClientBagSlotRef::FindEntityByID(const char* id)
{
    // Check Bag Slots
    for (unsigned i = 0; i < ItemSlots.GetSize(); i++)
    {
        if (ItemSlots[i].GetData().EntityID == id)
            return &(ItemSlots[i].GetData());
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////

GameClientAbilitiesList::GameClientAbilitiesList(Game::AbilityGroupType a)
{
    Array<Game::AbilityID>    grpAbilIDs;
    Game::GetAbilitiesByGroup(&grpAbilIDs, a);
    for (unsigned i = 0; i < grpAbilIDs.GetSize(); i++)
    {
        GameClientAbility abil(grpAbilIDs[i], 0);
        Abilities.PushBack(abil);
    }
}

void    GameClientAbilitiesList::Add(const GameClientAbility& abil)
{
    for (unsigned i = 0; i < Abilities.GetSize(); i++)
    {
        if (Abilities[i].AbilID == abil.AbilID)
        {
            Abilities[i].Rank = abil.Rank;
            return;
        }
    }
    SF_ASSERT(0);   // Not found in group?
}

void    GameClientAbilitiesList::Remove(Game::AbilityID abilId)
{
    for (unsigned i = 0; i < Abilities.GetSize(); i++)
    {
        if (Abilities[i].AbilID == abilId)
        {
            Abilities[i].Rank = 0;
            return;
        }
    }
    SF_ASSERT(0);   // Not found in group?
}

const GameClientAbility&   GameClientAbilitiesList::GetAbility(unsigned idx) const
{
    return Abilities[idx];
}

//////////////////////////////////////////////////////////////////////////

void    GameClientAbilityCooldown::Update(UInt32 ticksDelta)
{
    if (TicksLeft == 0) return;
    if (TicksLeft < ticksDelta)
        TicksLeft = 0;
    else
        TicksLeft -= ticksDelta;
}

void    GameClientAbilityCast::Update(UInt32 ticksDelta)
{
    if (TicksLeft == 0) return;
    if (TicksLeft < ticksDelta)
        TicksLeft = 0;
    else
        TicksLeft -= ticksDelta;
}

GameClientSpellBook::GameClientSpellBook()
{
}

bool    GameClientSpellBook::HasAbilityGroup(Game::AbilityGroupType a) const
{
    return PlayerAbilities[a] != NULL;
}

const GameClientAbilitiesList*     GameClientSpellBook::GetAbilityGroup(Game::AbilityGroupType a) const
{
    return PlayerAbilities[a];
}

void    GameClientSpellBook::AddAbility(const GameClientAbility &abil)
{
    Game::AbilityGroupType  grpId = Game::GetAbilityDescForID(abil.AbilID)->GroupID;
    GameClientAbilitiesList* list = PlayerAbilities[grpId];
    if (!list) 
    {
        Ptr<GameClientAbilitiesList> abils = *SF_NEW GameClientAbilitiesList(grpId);
        PlayerAbilities[grpId] = abils;
        list = abils;
    }
    list->Add(abil);
}

//////////////////////////////////////////////////////////////////////////


GameClientState::GameClientState(UIColorProvider* cp) 
: LastTicks(0), BroadcastChannelsLocalIdCount(1), ColorLookup(cp)
{
    // Create client channels for specific message types
    WhisperBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Whisper, "Whisper");
    PartyBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Party, "Party");
    GuildBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Guild, "Guild");
    SystemBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_System, "System");
    ErrorBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Error, "Error");
    CombatBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Combat, "Combat");
    SayBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Say, "Say");
    YellBuffer = *SF_NEW GameClientMsgBuffer(Game::MSG_Yell, "Yell");

    // Create default client channel logs
    Ptr<GameClientStateMessageLog> log;

    log = *SF_NEW GameClientStateMessageLog("General", ColorLookup);
    log->AddBuffer(SayBuffer);
    log->AddBuffer(YellBuffer);
    log->AddBuffer(WhisperBuffer);
    log->AddBuffer(PartyBuffer);
    log->AddBuffer(GuildBuffer);
    log->AddBuffer(SystemBuffer);
    log->AddBuffer(ErrorBuffer);
    ChannelLogs.PushBack(log);

    log = *SF_NEW GameClientStateMessageLog("Guild", ColorLookup);
    log->AddBuffer(GuildBuffer);
    log->AddBuffer(WhisperBuffer);
    log->AddBuffer(PartyBuffer);
    log->AddBuffer(SystemBuffer);
    log->AddBuffer(ErrorBuffer);
    ChannelLogs.PushBack(log);

    log = *SF_NEW GameClientStateMessageLog("Combat", ColorLookup);
    log->AddBuffer(CombatBuffer);
    ChannelLogs.PushBack(log);

    log = *SF_NEW GameClientStateMessageLog("Whisper", ColorLookup);
    log->AddBuffer(WhisperBuffer);
    ChannelLogs.PushBack(log);
}

GameClientEntityRef const*  GameClientState::FindEntityDataByID(const char* id)
{
    // PPS: This should use an intelligent hash that knows about entity lifetimes
    //      The following is a hack

    // Check Inventory Slots
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameClientBagSlotRef* bag = PlayerInventory[i];
        if (!bag) continue;
        if (bag->GetData().EntityID == id) return &(bag->GetData());
        GameClientEntityRef const* data = bag->FindEntityByID(id);
        if (data) return data;
    }

    // Check Equipment Slots
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        GameClientSlotRef* slot = PlayerEquipment[i];
        if (slot && slot->GetData().EntityID == id) return &(slot->GetData());
    }

    return NULL;
}

GameClientBagSlotRef*     GameClientState::FindBagByID(const char* id)
{
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameClientBagSlotRef* bag = PlayerInventory[i];
        if (!bag) continue;
        if (bag->GetData().EntityID == id) return bag;
    }
    return NULL;
}

unsigned    GameClientState::GetEntityCountForDefn(const Game::EntityDefnID& defn)
{
    // Accumulate the count for items matching the entity defn in player inventory/equipment
    unsigned count = 0;

    // Check Inventory Slots
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameClientBagSlotRef* bag = PlayerInventory[i];
        if (!bag) continue;
        if (bag->GetData().DefnID == defn)
        {
            count += bag->GetData().StackCount;
        }
        for (unsigned j = 0; j < bag->GetNumSlots(); j++)
        {
            GameClientSlotRef* slot = bag->GetSlot(j);
            if (slot->GetData().DefnID == defn)
            {
                count += slot->GetData().StackCount;
            }
        }
    }

    // Check Equipment Slots
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        GameClientSlotRef* slot = PlayerEquipment[i];
        if (slot && slot->GetData().DefnID == defn) 
        {
            count += slot->GetData().StackCount;
        }
    }

    return count;
}

bool    GameClientState::GetSlotDescriptorForEntity(Game::EntitySlotDescriptor* desc, const char* id)
{
    // PPS: This should use an intelligent hash that knows about entity lifetimes
    //      The following is a hack

    // Check Inventory Slots
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameClientBagSlotRef* bag = PlayerInventory[i];
        if (!bag) continue;
        if (bag->GetData().EntityID == id)
        {
            desc->Type = Game::SLOT_Inventory;
            desc->Index = i;
            return true;
        }
        for (unsigned j = 0; j < bag->GetNumSlots(); j++)
        {
            GameClientSlotRef* slot = bag->GetSlot(j);
            if (slot->GetData().EntityID == id)
            {
                desc->Type = Game::SLOT_Entity;
                desc->EntityID = bag->GetData().EntityID;
                desc->Index = j;
                return true;
            }
        }
    }

    // Check Equipment Slots
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        GameClientSlotRef* slot = PlayerEquipment[i];
        if (slot && slot->GetData().EntityID == id) 
        {
            desc->Type = Game::SLOT_Equipment;
            desc->Index = i;
            return true;
        }
    }

    return false;
}

bool    GameClientState::GetSlotDescriptorForEntityDefn(GameClientSlotDesc* desc, const Game::EntityDefnID& defn)
{
    // Find the first item matching the entity defn in player inventory/equipment

    Game::IDDescType* ed = Game::GetDescForID(defn);
    SF_ASSERT(ed);

    // If wearable, check equipment slots
    if (ed->Wearable)
    {
        for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
        {
            GameClientSlotRef* slot = PlayerEquipment[i];
            if (slot && slot->GetData().DefnID == defn) 
            {
                desc->Type = Game::SLOT_Equipment;
                desc->Index = i;
                return true;
            }
        }
    }

    // Check Inventory Slots
    for (unsigned i = 0; i < Game::NumInventorySlots; i++)
    {
        GameClientBagSlotRef* bag = PlayerInventory[i];
        if (!bag) continue;
        if (bag->GetData().DefnID == defn)
        {
            desc->Type = Game::SLOT_Inventory;
            desc->Index = i;
            return true;
        }
        for (unsigned j = 0; j < bag->GetNumSlots(); j++)
        {
            GameClientSlotRef* slot = bag->GetSlot(j);
            if (slot->GetData().DefnID == defn)
            {
                desc->Type = Game::SLOT_Entity;
                desc->EntityID = bag->GetData().EntityID;
                desc->Index = j;
                return true;
            }
        }
    }
    
    return false;
}

void    GameClientState::Update(UInt32 ticks)
{
    if (LastTicks == 0) LastTicks = ticks;
    UInt32 ticksDelta = ticks - LastTicks;

    for (unsigned i = 0; i < PlayerAbilityCooldowns.GetSize(); i++)
    {
        GameClientAbilityCooldown& ac = PlayerAbilityCooldowns[i];
        ac.Update(ticksDelta);
    }
    if (PlayerAbilityCast.IsCasting())
    {
        PlayerAbilityCast.Update(ticksDelta);
    }

    LastTicks = ticks;
}

GameClientBagSlotRef*         GameClientState::GetPlayerInventoryItem(Game::InventorySlotType p)
{
    return PlayerInventory[p];
}

GameClientSlotRef*        GameClientState::GetPlayerEquipmentItem(Game::EquipmentSlotType p)
{
    return PlayerEquipment[p];
}

void    GameClientState::GetPlayerStatsForParams(Game::EntityInstanceStatMod* fparams)
{
    for (unsigned i = 0; i < Game::NumStatTypes; i++)
        fparams->SetPawnStat((Game::StatType)i, PlayerStats[i]);
}

GameClientSlotRef*  GameClientState::GetPlayerBagItem(Game::InventorySlotType p, unsigned slotIdx)
{
    return PlayerInventory[p]->GetSlot(slotIdx);
}

void    GameClientState::SetPlayerInventoryItem(Game::InventorySlotType p, const GameClientEntityRef& value, unsigned numSlots)
{
    Game::EntitySlotDescriptor desc(Game::SLOT_Inventory, p);
    Ptr<GameClientBagSlotRef> bag = *SF_NEW GameClientBagSlotRef(desc, value, numSlots);
    PlayerInventory[p] = bag;
}

void    GameClientState::SetPlayerBagItem(Game::InventorySlotType p, unsigned slotIdx, const GameClientEntityRef& value)
{
    SF_ASSERT(!PlayerInventory[p]->GetData().IsNull);
    Game::EntitySlotDescriptor desc(PlayerInventory[p]->GetData().EntityID, slotIdx);
    PlayerInventory[p]->SetSlot(desc, value);
}

void    GameClientState::SetPlayerEquipmentItem(Game::EquipmentSlotType p, const GameClientEntityRef& value)
{
    Game::EntitySlotDescriptor desc(Game::SLOT_Equipment, p);
    Ptr<GameClientSlotRef> eq = *SF_NEW GameClientSlotRef(desc, value);
    PlayerEquipment[p] = eq;
}


//////////////////////////////////////////////////////////////////////////

bool    GameClientMsgBuffer::Iterator::operator != (const GameClientMsgBuffer::Iterator& iter) const
{
    return CurrentNode != iter.CurrentNode;
}

void    GameClientMsgBuffer::Iterator::operator ++ ()
{
    CurrentNode = CurrentNode->pNext;
}

GameClientMsgBuffer::~GameClientMsgBuffer()
{
   Node* data = Messages.GetFirst();
   while (!Messages.IsNull(data))
   {
       Node* next = Messages.GetNext(data);
       Messages.Remove(data);
       delete data;
       data = next;
   }
}

GameClientMsgBuffer::Iterator GameClientMsgBuffer::Begin()
{
    Iterator iter(&Messages, Messages.GetFirst());
    return iter;
}

GameClientMsgBuffer::Iterator GameClientMsgBuffer::End()
{
    Iterator iter(&Messages, Messages.GetLast()->pNext);
    return iter;
}

void    GameClientMsgBuffer::AddMessage(const GameClientLogMsg& msg)
{
    if (Count == MaxMessageStorage)
    {
        // Remove last message
        Node* node = Messages.GetFirst();
        Messages.Remove(node);
        delete node;
        Count--;
    }
    Node* node = SF_NEW Node(msg);
    Messages.PushBack(node);
    Count++;

    // Notify all listeners
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
        Listeners[i]->OnMessageAdded(this, msg);
}

void    GameClientMsgBuffer::AddListener(ChannelListener* l)
{
    Listeners.PushBack(l);
}

//////////////////////////////////////////////////////////////////////////

void    GameClientStateMessageLog::GetData(StringBuffer* buffer)
{
    buffer->AppendString(Buffer->GetPartA());
    buffer->AppendString(Buffer->GetPartB());
}

void    GameClientStateMessageLog::regenBuffer()
{
    if (!Buffer) return;

    // We manually stitch the channels using timestamp
    Array<GameClientMsgBuffer::Iterator> iterators;
    for (unsigned i = 0; i < Channels.GetSize(); i++)
        iterators.PushBack(Channels[i]->Begin());

    int minIdx = -1;
    do
    {
        minIdx = -1;
        time_t minT = 0;
        for (unsigned i = 0; i < iterators.GetSize(); i++)
        {
            bool check = false;
            if (iterators[i] != Channels[i]->End())
            {
                if (minIdx == -1)
                {
                    check = true;
                }
                else
                {
                    if (iterators[i].GetMessage().Time < minT)
                    {
                        check = true;
                    }
                }
            }
            if (check)
            {
                minT = iterators[i].GetMessage().Time;
                minIdx = i;
            }
        }

        if (minIdx != -1)
        {
            String temp;
            formatMessage(temp, Channels[minIdx], iterators[minIdx].GetMessage());
            ++iterators[minIdx];
            Buffer->Append(temp);
        }

    } while(minIdx != -1);

    Buffer->Compile();
}

void    GameClientStateMessageLog::AddBuffer(GameClientMsgBuffer* channel)
{
    channel->AddListener(this);
    Channels.PushBack(channel);

    if (Channels.GetSize() == 1)
    {
        OutputChannel.Type = Game::MSG_Say;
        // Special cases
        switch (channel->GetType())
        {
        case Game::MSG_Other:
            {
                OutputChannel.Type = channel->GetType();
                OutputChannel.LocalID = ((GameClientBroadcastMsgBuffer*)channel)->GetLocalId();
                break;
            }
        case Game::MSG_Guild:
        case Game::MSG_Party:
            {
                OutputChannel.Type = channel->GetType();
                break;
            }
        }
    }

    Buffer = *SF_NEW GameMessageBuffer<LogSz>();
    regenBuffer();
}

GameClientMsgBuffer*  GameClientStateMessageLog::GetBuffer(Game::MessageType m, unsigned localId) const
{
    for (unsigned i = 0; i < Channels.GetSize(); i++)
    {
        GameClientMsgBuffer* chan = Channels[i];
        if (chan->GetType() == Game::MSG_Other)
        {
            GameClientBroadcastMsgBuffer* bc = (GameClientBroadcastMsgBuffer*)chan;
            if (bc->GetLocalId() == localId)
                return chan;
        }
        else if (chan->GetType() == m)
        {
            return chan;
        }
    }
    return NULL;
}

void    GameClientStateMessageLog::OnMessageAdded(GameClientMsgBuffer* pchan, 
                                                  const GameClientLogMsg& msg)
{
    SF_UNUSED(pchan);

    String temp;
    formatMessage(temp, pchan, msg);

    Buffer->Append(temp).Compile();

    // Notify log listeners
    for (unsigned i = 0; i < Listeners.GetSize(); i++)
    {
        Listeners[i]->OnMessageAdded(this, temp);
    }
}

void    GameClientStateMessageLog::AddListener(GameClientStateMessageLog::LogListener* l)
{
    Listeners.PushBack(l);
}

void    GameClientStateMessageLog::formatMessage(String& s, GameClientMsgBuffer* pchan, 
                                                 const GameClientLogMsg& msg)
{
    tm* timeinfo;
#ifdef SF_OS_WIN32
    tm timeinfo_s;
    localtime_s(&timeinfo_s, &msg.Time);
    timeinfo = &timeinfo_s;
#else
    timeinfo = localtime(&msg.Time);
#endif
    int hour = timeinfo->tm_hour;
    int min = timeinfo->tm_min;
    int sec = timeinfo->tm_sec;

    String time;
    Format(time, "<font color='{0}'>[{1:02:d}:{2:02:d}:{3:02:d}]</font> ", 
        ColorLookup->GetMessageTimeColor(pchan->GetType()),
        hour, min, sec);

    String channel;
    if (pchan->GetType() == Game::MSG_Other)
    {
        GameClientBroadcastMsgBuffer* bc = (GameClientBroadcastMsgBuffer*)pchan;
        Format(channel, "[{0}. {1}] ", bc->GetLocalId(), bc->GetName());
    }

    String source;
    if (!msg.SourceId.IsEmpty())
    {
        Format(source, "[{0}] ", msg.SourceName);
    }

    Format(s, "<font color='{0}'>{1}{2}{3}: {4}</font>\n", 
            ColorLookup->GetMessageColor(pchan->GetType()),
            time, channel, source, msg.Message);
}


//////////////////////////////////////////////////////////////////////////

bool    GameClientState::HasPlayerAbilityGroup(Game::AbilityGroupType g)
{
    return PlayerSpellBook.HasAbilityGroup(g);
}

void    GameClientState::AddPlayerAbility(const GameClientAbility& a)
{
    PlayerSpellBook.AddAbility(a);
}

void    GameClientState::SetPlayerAbilityCooldown(Game::AbilityID abilId, UInt32 ticks)
{
    GameClientAbilityCooldown ac(abilId, ticks);
    PlayerAbilityCooldowns.PushBack(ac);
}

void    GameClientState::ClearPlayerAbilityCooldown(Game::AbilityID abilId)
{
    int idx = -1;
    for (unsigned i = 0; i < PlayerAbilityCooldowns.GetSize(); i++)
    {
        if (PlayerAbilityCooldowns[i].GetAbilityID() == abilId)
        {
            idx = i;
            break;
        }
    }
    if (idx >= 0)
        PlayerAbilityCooldowns.RemoveAt(idx);
}

void    GameClientState::SetPlayerAbilityCast(Game::AbilityID abilId)
{
    unsigned ticks = Game::GetAbilityDescForID(abilId)->CastTime;
    PlayerAbilityCast = GameClientAbilityCast(abilId, ticks);
}

void    GameClientState::ClearPlayerAbilityCast(Game::AbilityID abilId)
{
    SF_UNUSED(abilId);

    PlayerAbilityCast = GameClientAbilityCast();
}

void    GameClientState::SubscribeChatChannel(Game::MessageType m, const String& chanId, const String& name)
{
    if (m == Game::MSG_Other)
    {
        Ptr<GameClientBroadcastMsgBuffer> channel 
            = *SF_NEW GameClientBroadcastMsgBuffer(m, chanId, name, BroadcastChannelsLocalIdCount++);
        BroadcastChannels.Add(chanId, channel);
        BroadcastChannelsLocalToRealLookup.Add(channel->GetLocalId(), channel->GetServerId());

        // Always add new channels to the general log
        GameClientStateMessageLog* log = GetMessageLog("General");
        log->AddBuffer(channel);
    }
}

void    GameClientState::AddChatChannelMessage(Game::MessageType m, const String& chanId,
                                               const GameClientLogMsg& msg)
{
    switch (m)
    {
    case Game::MSG_System:  SystemBuffer->AddMessage(msg); break;
    case Game::MSG_Error:   ErrorBuffer->AddMessage(msg); break;
    case Game::MSG_Combat:  CombatBuffer->AddMessage(msg); break;
    case Game::MSG_Guild:   GuildBuffer->AddMessage(msg); break;
    case Game::MSG_Party:   PartyBuffer->AddMessage(msg); break;
    case Game::MSG_Whisper: WhisperBuffer->AddMessage(msg); break;
    case Game::MSG_Say:     SayBuffer->AddMessage(msg); break;
    case Game::MSG_Yell:    YellBuffer->AddMessage(msg); break;
    default:
        {
            Ptr<GameClientBroadcastMsgBuffer>* pchan = BroadcastChannels.Get(chanId);
            SF_ASSERT(pchan);
            (*pchan)->AddMessage(msg);
        }
    }

    SF_DEBUG_MESSAGE3(1, "MSG:%d[chan:%s] : %s\n", m, chanId.ToCStr(), msg.Message.ToCStr());
}

const String&   GameClientState::GetChatChannelIdFromLocalId(unsigned localId)
{
    String* p = BroadcastChannelsLocalToRealLookup.Get(localId);
    SF_ASSERT(p);   // Use the check below
    return *p;
}

bool    GameClientState::HasChatChannelForLocalId(unsigned localId)
{
    String* p = BroadcastChannelsLocalToRealLookup.Get(localId);
    return p != NULL;
}

GameClientStateMessageLog*  GameClientState::GetMessageLog(const char* name)
{
    for (unsigned i = 0; i < ChannelLogs.GetSize(); i++)
    {
        if (ChannelLogs[i]->GetName() == name)
            return ChannelLogs[i];
    }
    return NULL;
}