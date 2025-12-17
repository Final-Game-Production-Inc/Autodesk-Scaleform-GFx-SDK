/**************************************************************************

Filename    :   GameGuild.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameGuild.h"
#include "GamePawn.h"

#include "Kernel/SF_Debug.h"
using namespace Scaleform;

//////////////////////////////////////////////////////////////////////////

GameGuild::GameGuild(const char* name, GameMessageChannel* channel) 
: Name(name), GuildBuffer(channel)
{
    SF_ASSERT(channel);
}

void    GameGuild::EnrollPawn(GamePlayerPawn* pawn)
{
    pawn->SetGuild(this);
    SF_DEBUG_MESSAGE2(1, "Added %s to guild %s..", pawn->GetInfo(Game::INFO_Name), this->GetName().ToCStr());
    Members.PushBack(pawn);
    GuildBuffer->AddSubscriber(pawn);
}

void    GameGuild::Broadcast(const GameChannelMessage& message)
{
    GuildBuffer->Broadcast(message);
}

