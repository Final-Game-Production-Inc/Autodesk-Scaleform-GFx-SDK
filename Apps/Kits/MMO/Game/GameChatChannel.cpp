/**************************************************************************

Filename    :   GameChatChannel.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameChatChannel.h"

#include "Kernel/SF_HeapNew.h"

GameMessageChannel::GameMessageChannel(Game::MessageType m, const char* name)
{
    Type = m;
    Name = name;
}

void    GameMessageChannel::AddSubscriber(GameChannelMessageSubscriber* sub)
{
    Subscribers.PushBack(sub);
}

void    GameMessageChannel::Broadcast(const GameChannelMessage& message)
{
    for (unsigned i = 0; i < Subscribers.GetSize(); i++)
    {
        Subscribers[i]->OnBroadcastedChannelMessage(this, message);
    }
}


//////////////////////////////////////////////////////////////////////////


GameMessageChannel*    GameMessageChannelFactory::CreateChannel(const char* id, Game::MessageType m, const char* name)
{
    GameMessageChannel* chan = SF_NEW GameMessageChannel(m, name);
    chan->ID = id;
    return chan;
}