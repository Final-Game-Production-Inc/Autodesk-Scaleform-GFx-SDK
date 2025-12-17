/**************************************************************************

Filename    :   GameConnection.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameConnection_H
#define INC_GameConnection_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_String.h"

#include "GameSimulation.h"
#include "GameEntity.h"
#include "GamePawn.h"
#include "GameEvent.h"

using namespace Scaleform;

class   GameServer;
class   GameClient;

//////////////////////////////////////////////////////////////////////////

class   GameConnection
{
public:
    virtual ~GameConnection() {}

    virtual void    SendEvent(GameServerEvent* event)   = 0;
    virtual GameServerEvent*    GetNextEvent()          = 0;
    virtual bool                HasEvents()             = 0;

    virtual void    SendRequest(GameClientRequest* request) = 0;
    virtual GameClientRequest*  GetNextRequest()            = 0;
    virtual bool                HasRequests()               = 0;
};

#endif  // INC_GameConnection_H
