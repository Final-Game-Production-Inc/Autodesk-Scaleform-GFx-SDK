/**************************************************************************

Filename    :   GameState_Loading.cpp
Content     :   Loading screen game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MsgFormat.h"
#include "GameState_Loading.h"
#include "../Game.h"
#include "GameState_InGame.h"

//////////////////////////////////////////////////////////////////////////

bool    GameState_Loading::Initialize()
{
    Env->GetResourceMgr().GetMemoryFileCache()->AddSet(Data.FileCache);

    Env->GetResourceMgr().InitLoader(Loader);

    String uiFilename = Env->GetResourceMgr().BuildPath("Loading.swf");
    Ptr<GFx::MovieDef> loadingDef = *Env->GetResourceMgr().LoadMovieDef(&Loader, uiFilename);

    // Start displaying the loading.swf
    LoadingUI = *SF_HEAP_AUTO_NEW(this)    GameUISwfMovie();
    if (!LoadingUI->Initialize(loadingDef, this)) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to create movie instance for Loading UI\n");
        return false;
    }
    SF_DEBUG_MESSAGE(1, "Pushing Loading UI\n");
    Env->GetUIMgr().PushUI(LoadingUI);

    // Load the main menu swf assets in the background (non-blocking task)
    // - We'll also load the movies files multiple times to simulate an actual game data load
    // - Read chunk size is lowered to let the loading screen be more 'realistic'
    GameDataLoader::LoaderDef defn;
    defn.Name = "InGameScreenLoader";
#if defined(SF_OS_XBOX360)
    defn.DataChunkSz = 65536;
#elif defined(SF_OS_PS3)
    defn.DataChunkSz = 4096;
#elif defined(SF_OS_WIIU)
    defn.DataChunkSz = 65536;
#else
    defn.DataChunkSz = 128;
#endif
    defn.DelayInSeconds = 3;
    defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("InGame.swf"));

    if (Env->AreVideosAvailable())
    {
#if defined(SF_OS_WIN32) || defined(SF_OS_MAC) || defined(SF_OS_LINUX)
        // On non-consoles, load the loading_movie as well to increase the amount of data to load.
        defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("movies/loading_movie.usm"));
#endif
        defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("movies/intro_movie.usm"));
    }
    else 
    {
    // If the movie's aren't on disk, load Loading.swf multiple times for the simulated load.
#if defined(SF_OS_WIIU)
        for (unsigned i = 0; i < 5; i++)
#else
		for (unsigned i = 0; i < 50; i++)
#endif
            defn.Filenames.PushBack(Env->GetResourceMgr().BuildPath("Loading.swf"));
    }

    InGameScreenLoader = *SF_NEW GameDataLoader(defn);
    InGameScreenLoader->Start();

    return true;
}


void    GameState_Loading::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);

    if (InGameScreenLoader->IsDone())
    {
        GameStateData_InGame data;
        data.FileCache = InGameScreenLoader->GetMemoryFileCache();

        Env->SetCurrentState(SF_NEW GameState_InGame(Env, data));
        return;
    }

    if (!LoadingUI->IsValid() || ProgressBarMask.IsUndefined()) return;
    
    unsigned progress = (unsigned)(InGameScreenLoader->GetProgress() * 100);
    GFx::Value::DisplayInfo di;
    di.SetXScale(progress);
    ProgressBarMask.SetDisplayInfo(di);
    char buff[16];
    Format(buff, "{0}", progress);
    ProgressBarText.SetText(buff);

    LoadingUI->Advance(pstats);
}

void    GameState_Loading::Cleanup()
{
    Env->GetResourceMgr().GetMemoryFileCache()->RemoveSet(Data.FileCache);

    // Clean up UI assets
    SF_DEBUG_MESSAGE(1, "Popping Loading UI\n");
    Env->GetUIMgr().PopUI();    // PPS: This is not ideal.. Need a mechanism to remove specific swfs instead of a stack?

    if (!InGameScreenLoader->IsDone())
        InGameScreenLoader->Stop();
}

void    GameState_Loading::OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount)
{
    SF_UNUSED2(ui, argCount);

    if (!SFstrcmp("registerProgressBar", methodName))
    {
        SF_ASSERT(argCount == 2);
        ProgressBarMask = args[0];
        ProgressBarText = args[1];
    }

    if (!SFstrcmp(methodName, "isVideoEnabled")) 
    {
        GFx::Value bIsVideoEnabled;
        GFx::Value retVal = args[0];
        SF_ASSERT(!retVal.IsUndefined());
        // Need to check if the videos are available to cover the case where the videos are not installed.
        bIsVideoEnabled.SetBoolean( Env->IsVideoEnabled() && Env->AreVideosAvailable() );
        retVal.SetMember(methodName, bIsVideoEnabled);
    }
}

void    GameState_Loading::Pause(bool pause)
{
    InGameScreenLoader->Pause(pause);
    LoadingUI->Pause(pause);
}

void    GameState_Loading::OnIoLock()
{
    if (!InGameScreenLoader->IsDone())
        InGameScreenLoader->EnableIO(false);
}

void    GameState_Loading::OnIoUnlock()
{
    if (!InGameScreenLoader->IsDone())
        InGameScreenLoader->EnableIO(true);
}
