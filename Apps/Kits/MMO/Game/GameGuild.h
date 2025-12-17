/**************************************************************************

Filename    :   GameGuild.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameGuild_H
#define INC_GameGuild_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Kernel/SF_Array.h"
using namespace Scaleform;

#include "GameChatChannel.h"

class GamePlayerPawn;


//////////////////////////////////////////////////////////////////////////
//
//
class GameGuild : public RefCountBase<GameGuild, Stat_Default_Mem>
{
public:
    GameGuild(const char* name, GameMessageChannel* channel);

    const String&   GetName() const     { return Name; }

    void            EnrollPawn(GamePlayerPawn* pawn);
    void            Broadcast(const GameChannelMessage& message);

protected:
    String                  Name;
    GameMessageChannel*     GuildBuffer;
    Array<GamePlayerPawn*>  Members;
};


#endif  // INC_GameGuild_H
