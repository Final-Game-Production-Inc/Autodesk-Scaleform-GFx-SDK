/**************************************************************************

Filename    :   GameClient.h
Content     :   Sample representation of an MMO game client
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameClient_H
#define INC_GameClient_H

// Includes
#include "GFx/GFx_Player.h"
#include "GameClientState.h"
#include "../UI/GameUIAdapter.h"


// Forward declarations
class   GameConnection;
struct  GameServerEvent;


//////////////////////////////////////////////////////////////////////////
// Sample representation of an MMO game client.
//
class GameClient : public RefCountBase<GameClient, Stat_Default_Mem>,
                   public UIColorProvider
{
public:
    GameClient(GFx::Movie* pmovie, GameConnection* conn);

    void        Initialize(GameUIRuntimeAtlasManager* pimagecreator);
    void        Connect(const char* playerName);

    void        Update();

    unsigned                GetClientID() const         { return ClientID;   }
    GFx::Movie*             GetMovie() const            { return pMovie;     }
    GameClientState*        GetState()                  { return State;     }
    GameUIAdapter*          GetUIAdapter() const        { return UIAdapter;  }
    GameConnection*         GetConnection() const       { return Connection; }

    // Implementation of GameClientStateHtmlColorProvider interface
    virtual const char*     GetMessageColor(Game::MessageType type) const;
    virtual const char*     GetMessageTimeColor(Game::MessageType type) const;
    virtual const char*     GetMessageSourceColor(Game::MessageType type) const;

protected:
    Ptr<GFx::Movie>         pMovie;
    GameConnection*         Connection;

    unsigned                ClientID;
    Ptr<GameClientState>    State;
    Ptr<GameUIAdapter>      UIAdapter;

    void        processServerEvent(GameServerEvent* event);
};


#endif  // INC_GameClient_H
