/**************************************************************************

Filename    :   GameState_Start.h
Content     :   Bootup game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameStart_Start_H
#define INC_GameStart_Start_H

// Includes
#include "GameState.h"

// ********************************************************************
// This bootup state simply loads the splash state
//
class GameState_Start : public GameState
{
public:
    GameState_Start(Game* env) : GameState(env, "start") {}

    virtual bool    Initialize();

protected:

};

#endif  // INC_GameStart_Start_H
