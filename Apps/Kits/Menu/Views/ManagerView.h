/**************************************************************************

Filename    :   ManagerView.h
Content     :  C++ interface for the manager that loads all views in
                the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_ManagerView_H
#define INC_ManagerView_H

// Includes
#include "UIView.h"
#include "MenuDialog.h"
#include "MenuView.h"

#include "../States/GameStateWidgetSupport.h"

#include "MainMenuView.h"
#include "StartGameView.h"
#include "SettingsView.h"

// Forward declaration.
class GameState_MainMenu;

// ********************************************************************
class ManagerView
{
public:
    ManagerView(GFx::Value pval, GameState_MainMenu* pgameState) : Val(pval), pGameState(pgameState),
        pMainMenuView(NULL), pStartGameView(NULL), pSettingsView(NULL), pExitDialog(NULL)
    {

        ViewInfo mainMenuViewInfo;
        mainMenuViewInfo.SWFName = "MainMenu.swf";
        mainMenuViewInfo.ViewName = "MainMenu";
        ViewData.PushBack(mainMenuViewInfo);

        ViewInfo settingsViewInfo;
        settingsViewInfo.SWFName = "Settings.swf";
        settingsViewInfo.ViewName = "Settings";
        ViewData.PushBack(settingsViewInfo);

        ViewInfo startGameViewInfo;
        startGameViewInfo.SWFName = "StartGame.swf";
        startGameViewInfo.ViewName = "StartGame";
        ViewData.PushBack(startGameViewInfo);

        ViewInfo exitDialogInfo;
        exitDialogInfo.SWFName = "OptionDialog.swf";
        exitDialogInfo.ViewName = "ExitDialog";
        ViewData.PushBack(exitDialogInfo);
    }

    virtual ~ManagerView() 
    {
        if (pMainMenuView != NULL)
        {
            delete(pMainMenuView);
            pMainMenuView = NULL;
        }
        if (pStartGameView != NULL)
        {
            delete(pStartGameView);
            pStartGameView = NULL;
        }
        if (pSettingsView != NULL)
        {
            delete(pSettingsView);
            pSettingsView = NULL;
        }
        if (pExitDialog != NULL)
        {
            delete(pExitDialog);
            pExitDialog = NULL;
        }
    }

    struct ViewInfo {
        String     ViewName;
        String     SWFName;
    };

    /** Retrieves a reference to the actual GFx::Value for this view. */
    const GFx::Value&           GetValue()   const   { return Val; }
    const GameState_MainMenu*   GetGameState()  const   { return pGameState; }
    Ptr<GFx::Movie>             GetMovieView()  const    { return pMovieView; }
    void                        SetMovieView(GFx::Movie* pmovieView) { pMovieView = pmovieView; }

    void                    LaunchGame();
    void                    ExitGame();

    void                    ProcessEscapePress();

    void                    LoadViews();
    void                    ConfigureView(UIView* InView, const char* viewName);
    void                    PushView(MenuView* TargetView);
    void                    PushDialog(MenuDialog* targetDialog);
    MenuDialog*             SpawnDialog(const char* viewName);
    void                    PushViewByName(const char* viewName);
    void                    PopView();

    void                    AddViewToLoadedViews(String viewName, UIView* view);
    void                    SetSelectionFocus(GFxClikWidget& targetMovieClip);

    MainMenuView*           GetMainMenuView() const     { return pMainMenuView; }
    void                    SetMainMenuView(MainMenuView* pmainMenuView)
    {
        pMainMenuView = pmainMenuView;
        ConfigureView(pMainMenuView, "MainMenu");
    }

    StartGameView*          GetStartGameView() const    { return pStartGameView; }
    void                    SetStartGameView(StartGameView* pstartGameView)
    {
        pStartGameView = pstartGameView;
        ConfigureView(pstartGameView, "StartGame");
    }

    SettingsView*           GetSettingsView() const     { return pSettingsView; }
    void                    SetSettingsView(SettingsView* psettingsView)
    {
        pSettingsView = psettingsView;
        ConfigureView(pSettingsView, "Settings");
    }

    MenuDialog*             GetExitDialog() const       { return pExitDialog; }
    void                    SetExitDialog(MenuDialog* pexitDialog)
    {
        pExitDialog = pexitDialog;
        ConfigureView(pExitDialog, "ExitDialog");
    }

protected:
    // Holds a reference to the the view in the SWF (MovieClip for example)
    GFx::Value                Val;

    GameState_MainMenu*     pGameState;

    MainMenuView*           pMainMenuView;
    StartGameView*          pStartGameView;
    SettingsView*           pSettingsView;
    MenuDialog*             pExitDialog;

    Array<ViewInfo>        ViewData;
    Array<UIView*>         ViewStack;
    Ptr<GFx::Movie>      pMovieView;

    Hash<String,UIView*, String::HashFunctor>     LoadedViews;

private:
    void                    LoadView(const ViewInfo viewInfo);

};

#endif  // INC_ManagerView_H
