/**************************************************************************

Filename    :   GamePawn.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GamePawn_H
#define INC_GamePawn_H

#include <math.h>

#include "Kernel/SF_Hash.h"
#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;

#include "Render/Render_Types2D.h"
using namespace Render;

#include "GameEntitySlot.h"
#include "GameChatChannel.h"
#include "GameAiBehavior.h"

#include "GameGuild.h"
#include "GamePawnInventory.h"
#include "GamePawnEquipment.h"
#include "GamePawnSpellbook.h"

class GameServer;
class GamePawnFactory;


class GamePawn : public RefCountBase<GamePawn, Stat_Default_Mem>
{
    friend class GamePawnFactory;

public: 
    enum Type
    {
        PAWN_Basic,
        PAWN_Player,
    };

    // Pawn event listener interface
    class ListenerType
    {
    public:
        virtual void    OnPawnTargetChange(GamePawn* pawn, GamePawn* oldt, GamePawn* newt) = 0 ;
        virtual void    OnPawnStatChange(GamePawn* pawn, Game::StatType s) = 0;
        virtual void    OnPawnMoneyChange(GamePawn* pawn, Game::MoneyType m) = 0;
        virtual void    OnPawnInventoryChange(GamePawn* pawn, Game::InventorySlotType inv) = 0;
        virtual void    OnPawnBagItemChange(GamePawn* pawn, Game::InventorySlotType inv, unsigned slotIdx) = 0;
        virtual void    OnPawnEquipmentChange(GamePawn* pawn, Game::EquipmentSlotType eq) = 0;
        virtual void    OnPawnAbilityAdd(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnAbilityRemove(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnCastStart(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnCastInterrupt(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnCastEnd(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnAbilityCooldownStart(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnAbilityCooldownEnd(GamePawn* pawn, GamePawnAbilityDesc* desc) = 0;
        virtual void    OnPawnMessageReceived(GamePawn* pawn, Game::MessageType m, const char* chanId, const GameChannelMessage& msg) = 0;

        virtual ~ListenerType() {};
    };

    virtual ~GamePawn();

    void            AddEventListener(ListenerType* l);
    void            RemoveEventListener(ListenerType* l);

    Type                    GetType() const     { return mType; }

    virtual UInt32            Update(GameServer* env, UInt32 ticks);

    // Accessors for entity state
    virtual const PointF&   GetPosition() const             { return Position; }
    virtual void            SetPosition(const PointF& p)    { Position = p;   }
    virtual PointF          GetNextPosition() const         { return Position; }
    virtual float           GetDirection() const            { return 0.f; }   // Degrees

    // Check if another entity is in a certain range
    float                   GetDistanceSquared(const GamePawn* pm) const
    {
        return Position.DistanceSquared(pm->GetPosition());
    }
    bool                    DistanceLessThan(const GamePawn* pm, float dist) const
    {
        float distSq = dist * dist;
        return Position.DistanceSquared(pm->GetPosition()) < distSq;
    }


    // Base info/stats of pawn
    struct BaseInfo
    {
        String          Name, Class, Race;
        unsigned        Strength, Dexterity, Intelligence, Endurance, Wisdom, Luck;

        BaseInfo() : Strength(0), Dexterity(0), Intelligence(0), Endurance(0),
            Wisdom(0), Luck(0) {}
    };

    // Info/stat Accessors   
    const char*     GetInfo(Game::InfoType p);
    unsigned        GetStat(Game::StatType s);
    unsigned        GetMoney(Game::MoneyType m);

    GamePawn*       GetTarget() const               { return Target; }
    void            SetTarget(GamePawn* target);

    GameMoneyContainer*         GetMoneyContainer() { return &Money; }

    // Stat modifiers
    bool            AddHP(unsigned hp);
    bool            AddMP(unsigned mp);

    static unsigned     GetXpToLevel(unsigned lvl);
    static unsigned     GetLevelFromXp(unsigned xp);

protected:
    Type                    mType;

    PointF                 Position;

    Ptr<GameAiBehavior>         Behavior;

    UInt32          LastTicks;

    String          ID;
    BaseInfo        Info;

    unsigned        Level;
    unsigned        XP;

    unsigned        CurrentHP;
    unsigned        CurrentMP;

    Ptr<GamePawn>   Target;

    GameMoneyContainer              Money;

    GamePawnFactory*            Manager;

    Array<ListenerType*>        Listeners;

    void            notifyTargetChange(GamePawn* oldt, GamePawn* newt);
    void            notifyStatChange(Game::StatType s);
    void            notifyMoneyChange(Game::MoneyType m);
    void            notifyInventoryChange(Game::InventorySlotType inv);
    void            notifyBagItemChange(Game::InventorySlotType inv, unsigned slotIdx);
    void            notifyEquipmentChange(Game::EquipmentSlotType eq);
    void            notifyAbilityAdd(GamePawnAbilityDesc* desc);
    void            notifyAbilityRemove(GamePawnAbilityDesc* desc);
    void            notifyAbilityCastStart(GamePawnAbilityDesc* desc);
    void            notifyAbilityCastInterrupt(GamePawnAbilityDesc* desc);
    void            notifyAbilityCastEnd(GamePawnAbilityDesc* desc);
    void            notifyAbilityCooldownStart(GamePawnAbilityDesc* desc);
    void            notifyAbilityCooldownEnd(GamePawnAbilityDesc* desc);
    void            notifyMessageReceive(Game::MessageType m, const char* id, const GameChannelMessage& msg);

    GamePawn(const BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai, Type type = PAWN_Basic);
    // Copying is prohibited
    GamePawn(const GamePawn&);
    const GamePawn& operator = (const GamePawn&);
};



//////////////////////////////////////////////////////////////////////////
//
// An active entity in the game (PC or NPC or MoB)
//
class GamePlayerPawn : public GamePawn,
                       public GameChannelMessageSubscriber
{
    friend class GamePawnFactory;
    friend class GamePawnEquipment;
    friend class GamePawnAbilityContainer;
    friend class GamePawnBagContainer;
    friend class GamePawnBagContainer::BagSlotEventProxy;

public:
    // Pawn container accessors
    GamePawnEquipment*          GetEquipment()      { return Equipment; }
    GamePawnBagContainer*       GetBagContainer()   { return BagContainer; }
    GamePawnAbilityContainer*   GetSpellBook()      { return SpellBook; }

    GameGuild*      GetGuild() const            { return Guild; }
    void            SetGuild(GameGuild* guild)  { Guild = guild; }
   
    void            SwapSlots(const Game::EntitySlotDescriptor& src, const Game::EntitySlotDescriptor& dest);
    void            UseSlot(GameServer* env, const Game::EntitySlotDescriptor& slot);
    void            UseAbility(GameServer* env, Game::AbilityID abilId);

    virtual UInt32            Update(GameServer* env, UInt32 ticks);

    void            HandleTargettedMessage(Game::MessageType type, const GameChannelMessage& message);
    // Chat subscriber impl
    virtual void    OnBroadcastedChannelMessage(GameMessageChannel* channel, const GameChannelMessage& message);

    void            SubscribeToChannel(GameMessageChannel* channel);
    const Array<GameMessageChannel*>&  GetSubscribedChannels() const   { return SubscribedChannels; }

protected:
    Ptr<GamePawnEquipment>          Equipment;
    Ptr<GamePawnBagContainer>       BagContainer;
    Ptr<GamePawnAbilityContainer>   SpellBook;

    Ptr<GamePawnAbilityDesc>        CastingAbility;

    Array<GameMessageChannel*>  SubscribedChannels;

    GameGuild*      Guild;

    GameItemSlot* getSlotFromDescriptor(const Game::EntitySlotDescriptor& desc);

    GamePlayerPawn(const BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai);
    // Copying is prohibited
    GamePlayerPawn(const GamePlayerPawn&);
    const GamePlayerPawn& operator = (const GamePlayerPawn&);
};


//////////////////////////////////////////////////////////////////////////
//
// Entity that manages item instances
//
class GamePawnFactory
{
    friend class GamePawn;

public:
    GamePawnFactory() : IdCounter(1) {}

    GamePawn*             CreateInstance(GamePawn::Type type, const GamePawn::BaseInfo& baseInfo, unsigned xp, GameAiBehavior* ai = NULL);

protected:
    void                    onPawnDestroy(GamePawn* pawn);

    unsigned                IdCounter;
};

#endif  // INC_GamePawn_H
