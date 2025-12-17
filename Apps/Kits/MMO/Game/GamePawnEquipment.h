/**************************************************************************

Filename    :   GamePawnEquipment.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GamePawnEquipment_H
#define INC_GamePawnEquipment_H

#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

#include "GameEntitySlot.h"

class GamePlayerPawn;


//////////////////////////////////////////////////////////////////////////

class GamePawnEquipment : public RefCountBase<GamePawnEquipment, Stat_Default_Mem>, public GameEntitySlotListener
{
public:
    GamePawnEquipment(GamePlayerPawn* pawn);
    GameEquipmentSlot*      GetSlot(unsigned idx);

    virtual void    OnSlotEvent(const GameEntitySlotEvent& e);

protected:
    Ptr<GameEquipmentSlot>  Slots[Game::NumEquipmentSlotTypes];
    GamePlayerPawn*               Owner;

    void            dropEquipment(Game::EquipmentSlotType slot);
};


#endif  // INC_GamePawnEquipment_H
