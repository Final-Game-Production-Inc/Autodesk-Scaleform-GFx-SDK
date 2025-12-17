/**************************************************************************

Filename    :   GameState_MainMenu.cpp
Content     :   Main menu game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "GameState_MainMenu.h"
#include "../Game.h"
#include "GameState_Loading.h"

//////////////////////////////////////////////////////////////////////////

bool    GameState_MainMenu::Initialize()
{
    Env->GetResourceMgr().GetMemoryFileCache()->AddSet(Data.FileCache);

    Env->GetResourceMgr().InitLoader(Loader);

    String uiFilename = Env->GetResourceMgr().BuildPath("Manager.swf");
    Ptr<GFx::MovieDef> mainMenuDef = *Env->GetResourceMgr().LoadMovieDef(&Loader, uiFilename);

    // Start displaying the main menu.swf
    MainMenuUI = *SF_HEAP_AUTO_NEW(this)    GameUISwfMovie();
    if (!MainMenuUI->Initialize(mainMenuDef, this, true)) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to create movie instance for Main Menu UI\n");
        return false;
    }

#ifdef GFX_ENABLE_SOUND
    // Register the menu sound bank for CLIK sound events
    Env->GetResourceMgr().RegisterSoundEventBank(MainMenuUI, Data.SoundManager);
#endif 

    InstallWidgetCallbacks(MainMenuUI);
    SF_DEBUG_MESSAGE(1, "Pushing MainMenu UI\n");
    Env->GetUIMgr().PushUI(MainMenuUI);

    // Load the loading swf assets in the background (non-blocking task)
    GameDataLoader::LoaderDef defn;
    defn.Name = "LoadingScreenLoader";
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("Loading.swf"));
    LoadingScreenLoader = *SF_NEW GameDataLoader(defn);
    LoadingScreenLoader->Start();

    return true;
}

void    GameState_MainMenu::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);

    MainMenuUI->Advance(pstats);
}

void    GameState_MainMenu::Cleanup()
{
    Env->GetResourceMgr().GetMemoryFileCache()->RemoveSet(Data.FileCache);

    // Clean up UI assets
    SF_DEBUG_MESSAGE(1, "Popping MainMenu UI\n");
    Env->GetUIMgr().PopUI();    // PPS: This is not ideal.. Need a mechanism to remove specific swfs instead of a stack?

    if (!LoadingScreenLoader->IsDone())
        LoadingScreenLoader->Stop();
}

void    GameState_MainMenu::OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount)
{
    SF_UNUSED4(ui, methodName, args, argCount);
    // TODO
}

bool    GameState_MainMenu::OnKeyEvent(const GFx::KeyEvent& e)
{
    if (e.Type == GFx::Event::KeyUp && e.KeyCode == Key::Escape)
    {
        if (pManager != NULL)
        {
            pManager->ProcessEscapePress();
            return true;
        }
    }
    return MainMenuUI->OnKeyEvent(e);
}

bool    GameState_MainMenu::OnCharEvent(const GFx::CharEvent& e)
{
    return MainMenuUI->OnCharEvent(e);
}

bool    GameState_MainMenu::OnMouseEvent(const GFx::MouseEvent& e)
{
    return MainMenuUI->OnMouseEvent(e);
}

void    GameState_MainMenu::NextState()
{
    if (LoadingScreenLoader->IsDone())
    {
        GameStateData_Loading data;
        data.FileCache = LoadingScreenLoader->GetMemoryFileCache();

        Env->SetCurrentState(SF_NEW GameState_Loading(Env, data));
    }
}

void    GameState_MainMenu::Pause(bool pause)
{
    MainMenuUI->Pause(pause);
}

bool    GameState_MainMenu::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget)
{
    bool bHandled = false;
    // printf("\nGameState_MainMenu: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);
    String managerName = "Manager";
    if (MainMenuUI->GetSwf()->GetAVMVersion() == 2)
    {
        managerName = "root1";
    }

    if (!SFstrcmp(widgetName, managerName))
    {
        if (pManager == NULL)
        {
            pManager = new ManagerView(widget, this);
            pManager->SetMovieView(MainMenuUI->GetSwf());
            pManager->LoadViews();
            bHandled = true;
        }
    }
    else if (!SFstrcmp(widgetName, "MainMenu"))
    {
        if (pManager->GetMainMenuView() == NULL)
        {
            pManager->SetMainMenuView(new MainMenuView(widget, widgetName, pManager));
            String tempWidgetPath(widgetPath);
            // printf("Setting MainMenu's path as: %s", tempWidgetPath.ToCStr() );
            WidgetPathBindings.Set(tempWidgetPath, pManager->GetMainMenuView());
            pManager->PushView(pManager->GetMainMenuView());
            bHandled = true;
        }
    }

    else if (!SFstrcmp(widgetName, "StartGame"))
    {
        if (pManager->GetStartGameView() == NULL)
        {
            pManager->SetStartGameView(new StartGameView(widget, widgetName, pManager));
            String tempWidgetPath(widgetPath);
            WidgetPathBindings.Set(tempWidgetPath, pManager->GetStartGameView());
            bHandled = true;
        }
    }

    else if (!SFstrcmp(widgetName, "Settings"))
    {
        if (pManager->GetSettingsView() == NULL)
        {
            pManager->SetSettingsView(new SettingsView(widget, widgetName, pManager, Env));
            String tempWidgetPath(widgetPath);
            WidgetPathBindings.Set(tempWidgetPath, pManager->GetSettingsView());
            bHandled = true;
        }
    }
    else if (!SFstrcmp(widgetName, "ExitDialog"))
    {
        if (pManager->GetExitDialog() == NULL)
        {
            pManager->SetExitDialog(new MenuDialog(widget, widgetName, pManager));
            String tempWidgetPath(widgetPath);
            WidgetPathBindings.Set(tempWidgetPath, pManager->GetExitDialog());
            bHandled = true;
        }
    }
    return bHandled;
}

void    GameState_MainMenu::OnPostAdvance(GameUISwfMovie* ui)
{
    SF_UNUSED(ui);
    if (bWidgetsInitializedThisFrame)
    {
        PostWidgetInit();
        bWidgetsInitializedThisFrame = false;
    }
}

void    GameState_MainMenu::PostWidgetInit()
{
    if (!bMenuInitialized && pManager->GetMainMenuView() != NULL)
    {
        // Once the Main Menu has been initialized and it's been advanced 1 frame, set focus to the list.
        pManager->SetSelectionFocus( pManager->GetMainMenuView()->GetList() );
        bMenuInitialized = true;
    }
}
