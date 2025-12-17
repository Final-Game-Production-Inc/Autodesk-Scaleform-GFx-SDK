/**************************************************************************

Filename    :   Game.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Game_H
#define INC_Game_H

#include "Game/GameServer.h"
#include "Game/GameClient.h"

//////////////////////////////////////////////////////////////////////////
//
//
class MMOGame
{
public:
    MMOGame();
    virtual ~MMOGame();

    // Initialization and cleanup routines; called by FxPlayerApp 
    void    Initialize(GFx::Movie* pmovie, GameUIRuntimeAtlasManager* pimagecreator);

    void    Shutdown();

    // Called by FxPlayerApp to tick game states and UIs
    void    Update();

protected:
    Ptr<GameServer>         TheServer;
    Ptr<GameClient>         TheClient;
    
    GameConnection*         Connection;

    Array<GameClientRequest*>     RequestQueue;
    Lock                    RequestQueueLock;
    Array<GameServerEvent*>       EventQueue;
    Lock                    EventQueueLock;
};

#endif  // INC_Game_H
