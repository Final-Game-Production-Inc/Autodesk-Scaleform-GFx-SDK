/**************************************************************************

Filename    :   GamePawnInventory.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GamePawnInventory_H
#define INC_GamePawnInventory_H

#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

#include "GameEntityEvent.h"
#include "GameEntitySlot.h"

class GamePlayerPawn;


struct GameMoneyContainer
{
public:
    GameMoneyContainer()
    {
        for (unsigned i = 0; i < Game::NumMoneyTypes; i++)
            Amount[i] = 0;
    }
    void        AddAmount(Game::MoneyType type, int value)        { Amount[type] += value; }
    void        RemoveAmount(Game::MoneyType type, int value)     { if (HasAmount(type, value)) Amount[type] -= value; }
    bool        HasAmount(Game::MoneyType type, int value) const  { return (Amount[type] - value) >= 0; }
    int         GetAmount(Game::MoneyType type) const             { return Amount[type]; }
protected:
    int         Amount[Game::NumMoneyTypes];
};


//////////////////////////////////////////////////////////////////////////

class GamePawnBagContainer : public RefCountBase<GamePawnBagContainer, Stat_Default_Mem>, 
                             public GameEntitySlotListener
{
    friend class GamePlayerPawn;

public:
    GamePawnBagContainer(GamePlayerPawn* pawn);
    GameContainerSlot*  GetSlot(unsigned idx);

    void            AddItem(GameItem* entity);
    virtual void    OnSlotEvent(const GameEntitySlotEvent& e);

protected:
    class BagSlotEventProxy : public RefCountBase<BagSlotEventProxy, Stat_Default_Mem>, public GameEntityListener
    {
        friend class GamePlayerPawn;
    public:
        BagSlotEventProxy(GamePlayerPawn* owner, Game::InventorySlotType invSlot) : Owner(owner), InvSlot(invSlot) {}
        virtual void    OnEntityEvent(const GameEntityEvent& e);
    protected:
        GamePlayerPawn*   Owner;
        Game::InventorySlotType    InvSlot;
    };

    Ptr<GameBackpackSlot>       BackpackSlot;
    Ptr<GameContainerSlot>      Slots[Game::NumInventorySlots];
    Ptr<BagSlotEventProxy>      EventProxies[Game::NumInventorySlots];
    GamePlayerPawn*             Owner;
};

#endif  // INC_GamePawnInventory_H
