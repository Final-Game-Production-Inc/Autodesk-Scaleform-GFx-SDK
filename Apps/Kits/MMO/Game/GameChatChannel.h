/**************************************************************************

Filename    :   GameChatChannel.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameChatChannel_H
#define INC_GameChatChannel_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_String.h"
using namespace Scaleform;

#include "GameCommon.h"
#include "GameEvent.h"

class GameMessageChannel;

typedef     GameServerEvent::ChatMsgDesc        GameChannelMessage;

//////////////////////////////////////////////////////////////////////////

class GameChannelMessageSubscriber
{
public:
    virtual     ~GameChannelMessageSubscriber() {}

    virtual void    OnBroadcastedChannelMessage(GameMessageChannel* channel, const GameChannelMessage& message)  = 0;
};


//////////////////////////////////////////////////////////////////////////

class GameMessageChannel : public RefCountBase<GameMessageChannel, Stat_Default_Mem>
{
    friend class GameMessageChannelFactory;

public:
    Game::MessageType   GetType() const     { return Type; }
    const String&       GetID() const       { return ID;   }
    const String&       GetName() const     { return Name; }

    void    Broadcast(const GameChannelMessage& message);

    void    AddSubscriber(GameChannelMessageSubscriber* sub);

protected:
    String                          ID;
    Game::MessageType               Type;
    String                          Name;
    Array<GameChannelMessageSubscriber*>      Subscribers;

    GameMessageChannel(Game::MessageType m, const char* name);
    // Copying is prohibited
    GameMessageChannel(const GameMessageChannel&);
    const GameMessageChannel& operator = (const GameMessageChannel&);
};


//////////////////////////////////////////////////////////////////////////
//
// Entity that manages channel instances
//
class GameMessageChannelFactory
{
public:
    GameMessageChannel*    CreateChannel(const char* id, Game::MessageType m, const char* name);
};


#endif  // INC_GameChatChannel_H
