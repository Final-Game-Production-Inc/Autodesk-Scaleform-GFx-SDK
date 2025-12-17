/**************************************************************************

Filename    :   GameState_Splash.cpp
Content     :   Splash game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "GameState_Splash.h"
#include "../Game.h"
#include "GameState_MainMenu.h"
#include "GameState_Start.h"

#ifdef SF_OS_WIN32
#include <tchar.h>
#endif

//////////////////////////////////////////////////////////////////////////

class MainMenuScreenDataLoader : public GameDataLoader
{
public:
#ifdef GFX_ENABLE_SOUND
    MainMenuScreenDataLoader(const LoaderDef& defn, GameSoundManager* psndMgr)
        : GameDataLoader(defn), pSoundMgr(psndMgr) 
#else
    MainMenuScreenDataLoader(const LoaderDef& defn)
        : GameDataLoader(defn)
#endif
    {
        // TEMP BLOCKING LOAD
#ifdef GFX_ENABLE_SOUND
        pSoundMgr->LoadSound("defaultviewPushed", "sounds/menuPushViewSound.wav");
        pSoundMgr->LoadSound("defaultviewPopped", "sounds/menuPopViewSound.wav");
#endif
    }
    virtual ~MainMenuScreenDataLoader() {}

    GameSoundManager*       GetSoundManager() const     { return pSoundMgr; }

protected:
    Ptr<GameSoundManager>      pSoundMgr;
};

//////////////////////////////////////////////////////////////////////////

bool    GameState_Splash::Initialize()
{
    // Load and start displaying the splash.swf (blocking load)
    String uiFilename = Env->GetResourceMgr().BuildPath("Splash.swf");
    Ptr<SysFile> pf = *new SysFile(uiFilename);
    int len = pf->GetLength();
    if (len <= 0)
    {
        printf("Failed loading file info for '%s'\nPlease check your working directory and try again.\n", uiFilename.ToCStr());
#ifdef SF_OS_WIN32
        char temp[128];
        sprintf_s(temp, ("Failed loading file info for '%s'\nPlease check your working directory and try again.\n"), uiFilename.ToCStr());
        MessageBoxA(NULL, temp, _T("MenuKit Error"), MB_OK);
#endif
        return false;
    }

    Ptr<GameMemoryFile> pmem = *Env->GetResourceMgr().GetMemoryFileCache()->CreateFile(uiFilename, len);
    pf->Read(pmem->GetData(), len);

    Env->GetResourceMgr().InitLoader(Loader);
    Ptr<GFx::MovieDef> movieDef = *Env->GetResourceMgr().LoadMovieDef(&Loader, uiFilename);

    SplashUI = *SF_HEAP_AUTO_NEW(this) GameUISwfMovie();
    if (!SplashUI->Initialize(movieDef, this)) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to create movie instance\n");
    }
    SF_DEBUG_MESSAGE(1, "Pushing Splash UI\n");
    Env->GetUIMgr().PushUI(SplashUI);

    // Load the main menu swf assets in the background (non-blocking task)
    GameDataLoader::LoaderDef defn;
    defn.Name = "MainMenuScreenLoader";
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("images/difficulty_easy.png"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("images/difficulty_medium.png"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("images/difficulty_hard.png"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("Localization/gfxfontlib.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("MenuAssets.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("MainMenu.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("OptionDialog.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("Settings.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("StartGame.swf"));
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("Manager.swf"));
#ifdef GFX_ENABLE_SOUND
    Ptr<GameSoundManager> psndMgr = *SF_NEW GameSoundManager(&Env->GetResourceMgr(), Env->GetResourceMgr().BuildPath(""));
    MainMenuScreenLoader = *SF_NEW MainMenuScreenDataLoader(defn, psndMgr);
#else
    MainMenuScreenLoader = *SF_NEW MainMenuScreenDataLoader(defn);
#endif
    MainMenuScreenLoader->Start();

    return true;
}

void    GameState_Splash::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);

    if (!SplashUI) return;

    SplashUI->Advance(pstats);
}

void    GameState_Splash::Cleanup()
{
    if (!SplashUI) return;

    Env->GetResourceMgr().GetMemoryFileCache()->Remove(Env->GetResourceMgr().BuildPath("Splash.swf"));

    // Clean up UI assets
    SF_DEBUG_MESSAGE(1, "Popping Splash UI\n");
    Env->GetUIMgr().PopUI();    // PPS: This is not ideal.. Need a mechanism to remove specific swfs instead of a stack?

    if (!MainMenuScreenLoader->IsDone())
        MainMenuScreenLoader->Stop();
}

void    GameState_Splash::OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount)
{
    SF_UNUSED3(ui, args, argCount);

    if (!SFstrcmp(methodName, "splashDone") && MainMenuScreenLoader->IsDone())
    {
        MainMenuScreenDataLoader* ploader = (MainMenuScreenDataLoader*)MainMenuScreenLoader.GetPtr();
        GameStateData_MainMenu data;
        data.FileCache = ploader->GetMemoryFileCache();
#ifdef GFX_ENABLE_SOUND
        data.SoundManager = ploader->GetSoundManager();
#endif

        Env->SetCurrentState(SF_NEW GameState_MainMenu(Env, data));
    }

    if (!SFstrcmp(methodName, "isVideoEnabled")) 
    {
        GFx::Value bIsVideoEnabled;
        GFx::Value retVal = args[0];
        SF_ASSERT(!retVal.IsUndefined());
        bIsVideoEnabled.SetBoolean( Env->IsVideoEnabled() && Env->AreVideosAvailable() );
        retVal.SetMember(methodName, bIsVideoEnabled);
    }
}


bool    GameState_Splash::OnKeyEvent(const GFx::KeyEvent& e)
{
    if (!SplashUI) return false;

    if (e.Type == GFx::Event::KeyUp && e.KeyCode == Key::Escape)
    {
        GFx::Value root;
        SplashUI->GetSwf()->GetVariable(&root, "_root");
        root.Invoke("play");    // Play the next transition
        return true;
    }
    return false;
}

bool    GameState_Splash::OnCharEvent(const GFx::CharEvent& e)
{
    if (!SplashUI) return false;

    return SplashUI->OnCharEvent(e);
}

bool    GameState_Splash::OnMouseEvent(const GFx::MouseEvent& e)
{
    if (!SplashUI) return false;

    return SplashUI->OnMouseEvent(e);
}

void    GameState_Splash::Pause(bool pause)
{
    SplashUI->Pause(pause);
}
