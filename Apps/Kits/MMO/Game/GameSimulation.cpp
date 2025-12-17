/**************************************************************************

Filename    :   GameSimulation.cpp
Content     :   Sample Game Simulation for MMO Kit
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include "Kernel/SF_HeapNew.h"
#include "GameSimulation.h"
#include "GameServer.h"


GameSimulation::~GameSimulation()
{
    PawnRef* data = PawnRefList.GetFirst();
    while (!PawnRefList.IsNull(data))
    {
        PawnRef* next = PawnRefList.GetNext(data);
        PawnRefList.Remove(data);
        delete data;
        data = next;
    }
}

void    GameSimulation::AddPawn(GamePawn* pawn)
{
    // Add an actor to the active list
    PawnRef* newPawn = SF_NEW PawnRef(pawn);
    PawnRefList.PushBack(newPawn);
}

void    GameSimulation::BroadcastVicinityMessage(const GameChannelMessage& msg, bool yell)
{
    // TODO: Zones?
    // For now broadcast to all entities
    PawnRef* data = PawnRefList.GetFirst();
    while (!PawnRefList.IsNull(data))
    {
        if (data->Pawn->GetType() == GamePawn::PAWN_Player)
        {
            GamePlayerPawn* player = (GamePlayerPawn*)data->Pawn;
            player->HandleTargettedMessage(yell ? Game::MSG_Yell : Game::MSG_Say, msg);
        }
        data = PawnRefList.GetNext(data);
    }    
}

void    GameSimulation::Update(GameServer* env, UInt32 ticks)
{    
    // Broadcast a heartbeat system message every 5 minutes
    if (ticks - LastHeartBeatTicks > 300000)
    {
        GameChannelMessage  msg;
        msg.Message = "This is the Scaleform MMO Kit. We hope you enjoy it!";
        env->GetChannel("system")->Broadcast(msg);

        LastHeartBeatTicks = ticks;
    }

    // Update active actors
    PawnRef* data = PawnRefList.GetFirst();
    while (!PawnRefList.IsNull(data))
    {
        data->Pawn->Update(env, ticks);
        data = PawnRefList.GetNext(data);
    }    
}
