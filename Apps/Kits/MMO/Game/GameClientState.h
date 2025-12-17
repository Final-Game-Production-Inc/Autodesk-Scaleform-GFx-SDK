/**************************************************************************

Filename    :   GameClientState.h
Content     :   Storage for the game state on the client side.
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameClientState_H
#define INC_GameClientState_H

//
// The client state shadows the server state for the connected player
// profile. The UI uses this client state to update its visuals.
//


// Includes
#include "Kernel/SF_String.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Hash.h"
#include "Render/Render_Color.h"
using namespace Scaleform;

#include "GameCommon.h"
#include "GameEvent.h"
#include "GameMessageBuffer.h"

// Typedefs
typedef GameServerEvent::EntityDesc     GameClientEntityRef;
typedef Game::EntitySlotDescriptor      GameClientSlotDesc;


//////////////////////////////////////////////////////////////////////////
// Represents an entity slot in the game
//
class GameClientSlotRef : public RefCountBase<GameClientSlotRef, Stat_Default_Mem>
{
public:
    GameClientSlotRef() {}
    GameClientSlotRef(const GameClientSlotDesc& desc, 
                      const GameClientEntityRef& data)
        : Desc(desc), Data(data) {}

    const GameClientSlotDesc&   GetDesc() const     { return Desc; }
    const GameClientEntityRef&  GetData() const     { return Data; }

protected:
    GameClientSlotDesc      Desc;
    GameClientEntityRef     Data;
};


//////////////////////////////////////////////////////////////////////////
// Represents a bag slot in the game. 
//
class GameClientBagSlotRef : public GameClientSlotRef
{
public:
    GameClientBagSlotRef(const GameClientSlotDesc& desc, 
                     const GameClientEntityRef& data, unsigned numSlots)
        : GameClientSlotRef(desc, data)
    {
        ItemSlots.Resize(numSlots);
    }
   
    unsigned                GetNumSlots() const
    { 
        return (unsigned) ItemSlots.GetSize(); 
    }
    GameClientSlotRef*      GetSlot(unsigned slotIdx);
    void                    SetSlot(const GameClientSlotDesc& desc, 
                                    const GameClientEntityRef& data);

    GameClientEntityRef const*  FindEntityByID(const char* id);

protected:
    Array<GameClientSlotRef>    ItemSlots;
};


//////////////////////////////////////////////////////////////////////////
// Represents an ability cooldown. A cooldown is associated with a single
// ability.
//
class GameClientAbilityCooldown
{
public:
    GameClientAbilityCooldown() 
        : AbilID(Game::ID_ABIL_Unknown), TicksLeft(0), TicksTotal(0) {}
    GameClientAbilityCooldown(Game::AbilityID abilId, UInt32 ticksTotal) 
        : AbilID(abilId), TicksLeft(ticksTotal), TicksTotal(ticksTotal) {}

    void                Update(UInt32 ticksDelta);

    Game::AbilityID     GetAbilityID() const    { return AbilID; }
    float               GetCompletionPercent() const  
    { 
        return (TicksTotal - TicksLeft) / (float)TicksTotal; 
    }

protected:
    Game::AbilityID     AbilID;
    UInt32              TicksLeft;
    UInt32              TicksTotal;
};


//////////////////////////////////////////////////////////////////////////
// Represents an ability cast. Only one cast can be active at a single
// moment.
//
class GameClientAbilityCast
{
public:
    GameClientAbilityCast() 
        : AbilID(Game::ID_ABIL_Unknown), TicksLeft(0), TicksTotal(0) {}
    GameClientAbilityCast(Game::AbilityID abilId, UInt32 ticksTotal) 
        : AbilID(abilId), TicksLeft(ticksTotal), TicksTotal(ticksTotal) {}

    void                Update(UInt32 ticksDelta);

    bool                IsCasting() const       { return TicksLeft != 0; }
    Game::AbilityID     GetAbilityID() const    { return AbilID; }
    float               GetCompletionPercent() const  
    { 
        return (TicksTotal - TicksLeft) / (float)TicksTotal; 
    }
   
    UInt32          GetTicksLeft() const    { return TicksLeft; }
    UInt32          GetTotalTicks() const   { return TicksTotal; }

protected:
    Game::AbilityID     AbilID;
    UInt32              TicksLeft;
    UInt32              TicksTotal;
};


//////////////////////////////////////////////////////////////////////////
// Represents an ability of the connected player. The rank is a property
// specific to the player.
//
struct GameClientAbility
{
    Game::AbilityID     AbilID;
    unsigned            Rank;

    GameClientAbility() 
        : AbilID(Game::ID_ABIL_Unknown), Rank(0) {}
    GameClientAbility(Game::AbilityID abilid, unsigned rank) 
        : AbilID(abilid), Rank(rank) {}
};


//////////////////////////////////////////////////////////////////////////
// Represents a list of abilities. The spell book uses multiple lists
// to store abilities for different ability groups.
//
class GameClientAbilitiesList : public RefCountBase<GameClientAbilitiesList, Stat_Default_Mem>
{
public:
    GameClientAbilitiesList(Game::AbilityGroupType a);

    void        Add(const GameClientAbility& abil);
    void        Remove(Game::AbilityID abilId);
    unsigned    GetNumAbilities() const    {  return (unsigned)Abilities.GetSize();   }

    const GameClientAbility&    GetAbility(unsigned idx) const;

protected:
    Array<GameClientAbility>    Abilities;
};


//////////////////////////////////////////////////////////////////////////
// Represents the connected player's spell book.
//
class GameClientSpellBook
{
public:
    GameClientSpellBook();

    bool        HasAbilityGroup(Game::AbilityGroupType a) const;
    const GameClientAbilitiesList* GetAbilityGroup(Game::AbilityGroupType a) const;

    void        AddAbility(const GameClientAbility& abil);
    void        RemoveAbility(unsigned abilId);

protected:
    Ptr<GameClientAbilitiesList>    PlayerAbilities[Game::NumAbilityGroups];
};


//////////////////////////////////////////////////////////////////////////
// Represents a single message received from the server. The base class
// includes storage for the message content and source.
//
struct GameClientLogMsg : public GameServerEvent::ChatMsgDesc
{
    time_t  Time;

    GameClientLogMsg(const GameServerEvent::ChatMsgDesc& desc, time_t time)
        : GameServerEvent::ChatMsgDesc(desc), Time(time) {}
    GameClientLogMsg(const String& msg, time_t time)
        : GameServerEvent::ChatMsgDesc(msg), Time(time) {}
    GameClientLogMsg(const String& id, const String& name, 
                     const String& msg, time_t time)
        : GameServerEvent::ChatMsgDesc(id, name, msg), Time(time) {}
};


//////////////////////////////////////////////////////////////////////////
// Represents a log buffer containing many messages of the same type.
//
class GameClientMsgBuffer : public RefCountBase<GameClientMsgBuffer, Stat_Default_Mem>
{
    // Storage for the messages. Uses the log msg storage as a base.
    struct Node : public NewOverrideBase<Stat_Default_Mem>, 
                  public ListNode<Node>,
                  public GameClientLogMsg
    {
        Node(const GameClientLogMsg& msg) : GameClientLogMsg(msg) {}
    };

public:
    enum {
        // Maximum number of messages stored in the buffer at any time.
        MaxMessageStorage = 500
    };

    GameClientMsgBuffer(Game::MessageType type, const String& name) 
        : Type(type), Name(name), Count(0) {}
    virtual ~GameClientMsgBuffer();

    Game::MessageType   GetType() const     { return Type; }
    const String&       GetName() const     { return Name; }

    void    AddMessage(const GameClientLogMsg& msg);

    // Iterator to access the messages
    class Iterator
    {
        friend class GameClientMsgBuffer;

    public:
        const GameClientLogMsg&    GetMessage() const  { return *CurrentNode; }

        void    operator++ ();
        bool    operator!= (const Iterator& iter) const;

    protected:
        List<Node>*     ListRef;
        Node*           CurrentNode;

        Iterator(List<Node>* list, Node* node) : ListRef(list), CurrentNode(node) {}
    };

    Iterator    Begin();
    Iterator    End();

    // Interface for log buffer listeners.
    class ChannelListener
    {
    public:
        virtual ~ChannelListener() {}
        virtual void    OnMessageAdded(GameClientMsgBuffer* pchan, 
                                       const GameClientLogMsg& msg) = 0;
    };
    void    AddListener(ChannelListener* l);

protected:
    Game::MessageType           Type;
    String                      Name;
    List<Node>                  Messages;
    unsigned                    Count;
    Array<ChannelListener*>     Listeners;
};


//////////////////////////////////////////////////////////////////////////
// Similar to GameClientMsgBuffer, but includes extra data to represent
// a broadcasting message channel in the server.
//
class GameClientBroadcastMsgBuffer : public GameClientMsgBuffer
{
public:
    GameClientBroadcastMsgBuffer(Game::MessageType m, const String& serverId, 
                                 const String& name, unsigned localId)
        : GameClientMsgBuffer(m, name), ServerID(serverId), LocalID(localId) {}

    // The real id is the id used by the server. The local id is used
    // in the client side, such as by the UI to refer to this buffer.
    const String&       GetServerId() const       { return ServerID; }
    unsigned            GetLocalId() const      { return LocalID; }

protected:
    String      ServerID;
    unsigned    LocalID;
};

class UIColorProvider
{
public:
    virtual const char*   GetMessageColor(Game::MessageType type) const         = 0;
    virtual const char*   GetMessageTimeColor(Game::MessageType type) const     = 0;
    virtual const char*   GetMessageSourceColor(Game::MessageType type) const   = 0;
    virtual ~UIColorProvider() {};
};


//////////////////////////////////////////////////////////////////////////
// Represents a client side log. It can include messages from multiple
// server channels (buffers) of multiple message types.
//
class GameClientStateMessageLog 
    : public RefCountBase<GameClientStateMessageLog, Stat_Default_Mem>,
      public GameClientMsgBuffer::ChannelListener
{
public:
    enum 
    {
        // Maximum char size of the log available to the UI
        LogSz = 16384,
    };

    // Descriptor for the output channel/buffer
    struct OutputChannelDesc
    {
        Game::MessageType       Type;
        unsigned                LocalID;
        String                  Target;
    };

    GameClientStateMessageLog(const char* name, UIColorProvider* cp) 
        : Name(name), ColorLookup(cp) {}

    const String&   GetName() const     { return Name; }
    // Retrieves the compiled buffer (max LogSz size). The result
    // is a concatenation of the many channels/buffers that comprise
    // this log.
    void            GetData(StringBuffer* buffer);

    void            AddBuffer(GameClientMsgBuffer* buffer);
    GameClientMsgBuffer*  GetBuffer(Game::MessageType m, unsigned localId = 0) const;

    const OutputChannelDesc&    GetOutputChannel() const    
    { 
        return OutputChannel; 
    }
    void            SetOutputChannel(const OutputChannelDesc& desc)
    {
        OutputChannel = desc;
    }

    // Implementation of the GameClientMsgBuffer::ChannelListener interface
    virtual void    OnMessageAdded(GameClientMsgBuffer* pchan, 
                                   const GameClientLogMsg& msg);

    // Interface for log listeners
    class LogListener
    {
    public:
        virtual ~LogListener() {}
        virtual void    OnMessageAdded(GameClientStateMessageLog* plog, 
                                       const String& formattedMsg) = 0;
    };
    void    AddListener(LogListener* l);

protected:
    String                          Name;
    Array< GameClientMsgBuffer* >   Channels;
    Array<LogListener*>             Listeners;
    Ptr< GameMessageBuffer<LogSz> > Buffer;

    OutputChannelDesc               OutputChannel;
    UIColorProvider*                ColorLookup;

    void    formatMessage(String& s, GameClientMsgBuffer* pchan, 
                          const GameClientLogMsg& msg);

    void    regenBuffer();
};


//////////////////////////////////////////////////////////////////////////
// Main storage for the game state on the client side.
//
class GameClientState : public RefCountBase<GameClientState, Stat_Default_Mem>
{
public:
    GameClientState(UIColorProvider* cp);

    GameClientEntityRef const*  FindEntityDataByID(const char* id);
    GameClientBagSlotRef*       FindBagByID(const char* id);
    
    unsigned    GetEntityCountForDefn(const Game::EntityDefnID& defn);

    bool        GetSlotDescriptorForEntity(GameClientSlotDesc* desc, const char* id);
    bool        GetSlotDescriptorForEntityDefn(GameClientSlotDesc* desc, const Game::EntityDefnID& defn);

    void        Update(UInt32 ticks);

    // *** Info
    //
    const char* GetPlayerInfo(Game::InfoType p) const               
    { 
        return PlayerInfo[p]; 
    }
    void        SetPlayerInfo(Game::InfoType p, const char* value)  
    { 
        PlayerInfo[p] = value; 
    }

    // *** Stats
    //
    unsigned    GetPlayerStat(Game::StatType p) const               
    { 
        return PlayerStats[p]; 
    }
    void        SetPlayerStat(Game::StatType p, unsigned value)     
    { 
        PlayerStats[p] = value; 
    }
    void        GetPlayerStatsForParams(Game::EntityInstanceStatMod* fparams);

    // *** Money
    //
    unsigned    GetPlayerMoney(Game::MoneyType p) const             
    { 
        return PlayerMoney[p]; 
    }
    void        SetPlayerMoney(Game::MoneyType p, unsigned value)   
    { 
        PlayerMoney[p] = value; 
    }

    // *** Inventory
    GameClientBagSlotRef*   GetPlayerInventoryItem(Game::InventorySlotType p);
    void    SetPlayerInventoryItem(Game::InventorySlotType p, 
                                   const GameClientEntityRef& value, 
                                   unsigned numSlots);
    GameClientSlotRef*  GetPlayerBagItem(Game::InventorySlotType p, unsigned slotIdx);
    void    SetPlayerBagItem(Game::InventorySlotType p, unsigned slotIdx, 
                             const GameClientEntityRef& value);

    // *** Equipment
    //
    GameClientSlotRef*      GetPlayerEquipmentItem(Game::EquipmentSlotType p);
    void    SetPlayerEquipmentItem(Game::EquipmentSlotType p, 
                                   const GameClientEntityRef& value);

    // *** Spellbook / Abilities
    //
    const GameClientSpellBook&   GetPlayerSpellBook() const    
    { 
        return PlayerSpellBook; 
    }
    bool    HasPlayerAbilityGroup(Game::AbilityGroupType g);
    void    AddPlayerAbility(const GameClientAbility& a);

    const Array<GameClientAbilityCooldown>&    GetPlayerAbilityCooldowns() const   
    { 
        return PlayerAbilityCooldowns; 
    }
    void    SetPlayerAbilityCooldown(Game::AbilityID abilId, UInt32 ticks);
    void    ClearPlayerAbilityCooldown(Game::AbilityID abilId);

    const GameClientAbilityCast&   GetPlayerAbilityCast() const    
    { 
        return PlayerAbilityCast; 
    }
    void    SetPlayerAbilityCast(Game::AbilityID abilId);
    void    ClearPlayerAbilityCast(Game::AbilityID abilId);
      
    // *** Target
    //
    const char* GetTargetInfo(Game::InfoType p) const           
    { 
        return TargetInfo[p]; 
    }
    void        SetTargetInfo(Game::InfoType p, const String& value)  
    { 
        TargetInfo[p] = value; 
    }
    unsigned    GetTargetStat(Game::StatType p) const           
    { 
        return TargetStats[p]; 
    }
    void        SetTargetStat(Game::StatType p, unsigned value) 
    { 
        TargetStats[p] = value; 
    }

    // *** Messages
    //
    void    SubscribeChatChannel(Game::MessageType m, const String& chanId, 
                                 const String& name);
    void    AddChatChannelMessage(Game::MessageType m, const String& chanId, 
                                  const GameClientLogMsg& msg);
    const String&    GetChatChannelIdFromLocalId(unsigned localId);
    bool    HasChatChannelForLocalId(unsigned localId);

    typedef     Array< Ptr<GameClientStateMessageLog> >   ChannelLogsType;
    const ChannelLogsType&      GetMessageLogs() const  
    { 
        return ChannelLogs; 
    }
    GameClientStateMessageLog*  GetMessageLog(const char* name);

    GameClientMsgBuffer*  GetSystemBuffer() const    
    { 
        return SystemBuffer; 
    }
    GameClientMsgBuffer*  GetErrorBuffer() const     
    { 
        return ErrorBuffer; 
    }

protected:
    String      PlayerInfo[Game::NumInfoTypes];
    unsigned    PlayerStats[Game::NumStatTypes];
    unsigned    PlayerMoney[Game::NumMoneyTypes];
    
    Ptr<GameClientBagSlotRef>           PlayerInventory[Game::NumInventorySlots];
    Ptr<GameClientSlotRef>              PlayerEquipment[Game::NumEquipmentSlotTypes];
    GameClientSpellBook                 PlayerSpellBook;
    Array<GameClientAbilityCooldown>    PlayerAbilityCooldowns;
    GameClientAbilityCast               PlayerAbilityCast;

    Ptr<GameClientMsgBuffer>    PartyBuffer;
    Ptr<GameClientMsgBuffer>    GuildBuffer;
    Ptr<GameClientMsgBuffer>    SystemBuffer;
    Ptr<GameClientMsgBuffer>    ErrorBuffer;
    Ptr<GameClientMsgBuffer>    CombatBuffer;
    Ptr<GameClientMsgBuffer>    WhisperBuffer;
    Ptr<GameClientMsgBuffer>    SayBuffer;
    Ptr<GameClientMsgBuffer>    YellBuffer;

    unsigned                    BroadcastChannelsLocalIdCount;
    Hash<unsigned, String>      BroadcastChannelsLocalToRealLookup;
    Hash<String, Ptr<GameClientBroadcastMsgBuffer>, String::HashFunctor> BroadcastChannels;

    ChannelLogsType             ChannelLogs;

    String      TargetInfo[Game::NumInfoTypes];
    unsigned    TargetStats[Game::NumStatTypes];

    UInt32      LastTicks;

    UIColorProvider*   ColorLookup;
};

#endif  // INC_GameClientState_H
