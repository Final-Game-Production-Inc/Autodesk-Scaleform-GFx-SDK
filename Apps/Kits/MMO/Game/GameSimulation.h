/**************************************************************************

Filename    :   GameSimulation.h
Content     :   Sample Game Simulation for MMO Kit
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameSimulation_H
#define INC_GameSimulation_H

// Includes
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
using namespace Scaleform;

#include "GameChatChannel.h"

// Forward declarations
class GameServer;
class GamePawn;


//////////////////////////////////////////////////////////////////////////
//
// The simulation manager
//
class GameSimulation
{
public:
    GameSimulation() : LastHeartBeatTicks(0) {}
    ~GameSimulation();

    void        Update(GameServer* env, UInt32 ticks);

    void        AddPawn(GamePawn* actor);

    void        BroadcastVicinityMessage(const GameChannelMessage& msg, bool yell);

protected:
    struct  PawnRef : public NewOverrideBase<Stat_Default_Mem>, public ListNode<PawnRef>
    {
        GamePawn*   Pawn;
        PawnRef(GamePawn* pawn) : Pawn(pawn) {}
    };

    List<PawnRef>       PawnRefList;
    UInt32              LastHeartBeatTicks;
};


#endif  // INC_GameSimulation_H
