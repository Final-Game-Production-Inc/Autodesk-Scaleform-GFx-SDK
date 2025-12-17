/**************************************************************************

Filename    :   GameState_MainMenu.h
Content     :   Loading screen game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_MainMenu_H
#define INC_GameState_MainMenu_H

// Includes
#include "GameStateWidgetSupport.h"
#include "../GameUIManager.h"
#include "GFx/GFx_Loader.h"
#include "../GameResourceManager.h"

#include "../Views/ManagerView.h"
#include "../Views/StartGameView.h"
#include "../Views/SettingsView.h"
#include "../Views/MenuDialog.h"

// ********************************************************************
// Data loaded for the main menu
// - Used by the splash game state's resource loader
//
struct GameStateData_MainMenu
{
    Ptr<GameMemoryFileCache>    FileCache;
    Ptr<GameSoundManager>       SoundManager;
};


// ********************************************************************
// This game state manages the main menu and all interaction with it
//
class GameState_MainMenu : public GameStateWidgetSupport, public GameUIListener
{
public:
   GameState_MainMenu(Game* env, const GameStateData_MainMenu& data) 
       : GameStateWidgetSupport(env, "main_menu"), Data(data), pManager(NULL), bMenuInitialized(false) {}
   ~GameState_MainMenu() 
    {
        if (pManager != NULL)
        {
            delete(pManager);
            pManager = NULL;
        }
    }

    virtual bool    Initialize();
    virtual void    Update(GameUIAdvanceStats* pstats, UInt64 currTicks);
    virtual void    Cleanup();

    virtual void    Pause(bool pause);
    virtual bool    OnKeyEvent(const GFx::KeyEvent& e);
    virtual bool    OnMouseEvent(const GFx::MouseEvent& e);
    virtual bool    OnCharEvent(const GFx::CharEvent& e);

    virtual void    OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount);

    virtual bool    OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);
    virtual void    OnPostAdvance(GameUISwfMovie* ui);

    void            NextState();

protected:
    void            PostWidgetInit();

    Ptr<GameUISwfMovie>    MainMenuUI;
    GameStateData_MainMenu  Data;
    GFx::Loader             Loader;

    ManagerView*            pManager;

    Ptr<GameDataLoader>     LoadingScreenLoader;

    bool                    bMenuInitialized;
};

#endif  // INC_GameState_MainMenu_H
