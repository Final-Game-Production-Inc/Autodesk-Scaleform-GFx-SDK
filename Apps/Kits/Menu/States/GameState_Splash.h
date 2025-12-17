/**************************************************************************

Filename    :   GameState_Splash.h
Content     :   Splash game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_Splash_H
#define INC_GameState_Splash_H

#include "GameState.h"
#include "../GameUIManager.h"
#include "GFx/GFx_Loader.h"
#include "../GameResourceManager.h"


// ********************************************************************
// This game state is resposible for displaying and managing the splash 
// assets on load
//
class GameState_Splash : public GameState, public GameUIListener
{
public:
    GameState_Splash(Game* env) : GameState(env, "splash") {}

    virtual bool    Initialize();
    virtual void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);
    virtual void    Cleanup();

    virtual void    Pause(bool pause);
    virtual bool    OnKeyEvent(const GFx::KeyEvent& e);
    virtual bool    OnMouseEvent(const GFx::MouseEvent& e);
    virtual bool    OnCharEvent(const GFx::CharEvent& e);

    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount);

protected:
    Ptr<GameUISwfMovie>       SplashUI;
    GFx::Loader               Loader;

    Ptr<GameDataLoader>    MainMenuScreenLoader;
};

#endif  // INC_GameState_Splash_H
