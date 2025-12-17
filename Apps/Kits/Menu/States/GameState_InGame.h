/**************************************************************************

Filename    :   GameState_InGame.h
Content     :   In-game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_InGame_H
#define INC_GameState_InGame_H

// Includes
#include "GameState.h"
#include "../GameUIManager.h"
#include "GFx/GFx_Loader.h"
#include "../GameResourceManager.h"


// ********************************************************************
// Data loaded for in-game
// - Used by the loading screen game state's resource loader
//
struct GameStateData_InGame
{
    Ptr<GameMemoryFileCache>    FileCache;
};


// ********************************************************************
// This game manages all in-game assets and interaction
//
class GameState_InGame : public GameState, public GameUIListener
{
public:
    GameState_InGame(Game* env, const GameStateData_InGame& data) 
        : GameState(env, "in_game"), Data(data) {}

    virtual bool    Initialize();
    virtual void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);
    virtual void    Cleanup();

    virtual void    Pause(bool pause);
    virtual bool    OnKeyEvent(const GFx::KeyEvent& e);
    virtual bool    OnMouseEvent(const GFx::MouseEvent& e);
    virtual bool    OnCharEvent(const GFx::CharEvent& e);

    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount);

protected:
    Ptr<GameUISwfMovie>         InGameUI;
    GameStateData_InGame        Data;
    GFx::Loader                 Loader;
    GFx::Value                  ButtonTextField;
};

#endif  // INC_GameState_InGame_H
