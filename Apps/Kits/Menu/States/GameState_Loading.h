/**************************************************************************

Filename    :   GameState_Loading.h
Content     :   Main menu game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_Loading_H
#define INC_GameState_Loading_H

// Includes
#include "GameState.h"
#include "../GameUIManager.h"
#include "GFx/GFx_Loader.h"
#include "../GameResourceManager.h"


// ********************************************************************
// Data loaded for the loading screen assets
// - Used by the main menu game state's resource loader
//
struct GameStateData_Loading
{
    Ptr<GameMemoryFileCache>    FileCache;
};


// ********************************************************************
// This game state manages the loading screen assets
//
class GameState_Loading : public GameState, public GameUIListener
{
public:
   GameState_Loading(Game* env, const GameStateData_Loading& data) 
       : GameState(env, "loading"), Data(data) {}

    virtual bool    Initialize();
    virtual void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);
    virtual void    Cleanup();

    virtual void    Pause(bool pause);

    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount);

    virtual void    OnIoLock();
    virtual void    OnIoUnlock();

protected:
    Ptr<GameUISwfMovie>     LoadingUI;
    GameStateData_Loading   Data;
    GFx::Loader             Loader;

    Ptr<GameDataLoader>     InGameScreenLoader;

    GFx::Value              ProgressBarMask;
    GFx::Value              ProgressBarText;
};

#endif  // INC_GameState_Loading_H
