/**************************************************************************

Filename    :   Game.cpp
Content     :   Sample 'Game' for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Renderer2D.h"
#include "Kernel/SF_HeapNew.h"
#include "Game.h"
#include "States/GameState_MainMenu.h"

#include "States/GameState_Start.h"

Game::Game() : bRunning(false), LastMouseX(0.f), LastMouseY(0.f), VideoEnabled(false), VideosAvailable(false)
{
    // Check whether GFxVideo is enabled so we know whether to play intro / loading movies.
#ifdef GFX_ENABLE_VIDEO
    VideoEnabled = true;
#endif

    // Check whether the .USM files exist for playback and simulated loading.
    FileStat fileStats;
#ifdef SF_OS_XBOX360
    if (SysFile::GetFileStat(&fileStats, ResourceMgr.BuildPath("movies\\loading_movie.usm")) &&
        SysFile::GetFileStat(&fileStats, ResourceMgr.BuildPath("movies\\intro_movie.usm"))) 
#else // SF_OS_XBOX360
    if (SysFile::GetFileStat(&fileStats, ResourceMgr.BuildPath("movies/loading_movie.usm")) &&
        SysFile::GetFileStat(&fileStats, ResourceMgr.BuildPath("movies/intro_movie.usm"))) 
#endif // !SF_OS_XBOX360

    {
        VideosAvailable = true;
    }

    const char* argsTexture[] = {"Low", "Medium", "High"};
    Ptr<GameSettings_ValueList> textureDetailSetting = *new GameSettings_ValueList(0, "TEXTURE DETAIL", "Adjusts the texture detail for the application.", argsTexture, 3, 2);
    Settings.PushBack(textureDetailSetting);

    Ptr<GameSettings_Boolean> AASetting = *new GameSettings_Boolean(1, "ANTI-ALIASING", "Toggles the anti-aliasing.", true);
    Settings.PushBack(AASetting);

    Ptr<GameSettings_NumericRange> numSetting = *new GameSettings_NumericRange(2, "SOUND VOLUME", "Changes the volume level for the application.", 5, 0, 10, 1);
    Settings.PushBack(numSetting);

    const char* argsResolution[] = {"640x480", "800x600", "1280x720"};
    Ptr<GameSettings_ValueList> valueListSetting = *new GameSettings_ValueList(3, "VIDEO RESOLUTION", "Changes the application's resolution.", argsResolution, 3, 2);
    Settings.PushBack(valueListSetting);

    Ptr<GameSettings_NumericRange> gammaCorrection = *new GameSettings_NumericRange(4, "GAMMA CORRECTION", "Adjusts the gamma correction for the application.", 8, 0, 10, 1);
    Settings.PushBack(gammaCorrection);

    Ptr<GameSettings_Boolean> subtitlesSetting = *new GameSettings_Boolean(5, "SUBTITLES", "Toggles English subtitles.", false);
    Settings.PushBack(subtitlesSetting);
}


#ifdef GFX_ENABLE_SOUND
bool    Game::Initialize(FxRenderThread* prenderer, Sound::SoundRenderer* psndrenderer)
#else
bool    Game::Initialize(FxRenderThread* prenderer)
#endif // GFX_ENABLE_SOUND
{
#ifdef GFX_ENABLE_SOUND
    ResourceMgr.Initialize(prenderer, psndrenderer);
#else
    ResourceMgr.Initialize(prenderer);
#endif // GFX_ENABLE_SOUND
    UIMgr.Initialize(&ResourceMgr);

    SystemState = *SF_NEW GameState_System(this);
    if (!(SystemState->Initialize()))
        return false;

    SetCurrentState(SF_NEW GameState_Start(this));

    bRunning = true;
    return true;
}


void    Game::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);
    UIMgr.SetViewport(Viewport);

    SystemState->Update(pstats, currTicks);

    if (HasCurrentState())
        CurrentState->Update(pstats, currTicks);

    /*
    bool clear = (TrashedStates.GetSize() > 0);
    GFC_DEBUG_MESSAGE(clear, ">> Clearing TrashedStates");
    if (clear) {
        for (UPInt i=0; i < TrashedStates.GetSize(); i++)
        {
            GFC_DEBUG_MESSAGE1(1, "\tState: '%s'", TrashedStates[i]->GetName().ToCStr());
        }
    }
    */
    TrashedStates.Clear();
    // SF_DEBUG_MESSAGE(clear, "<< Done clearing TrashedStates\n");
   // TrashedStates.Clear();

}

void    Game::Shutdown()
{
    SystemState->Cleanup();
    SystemState = NULL;

    CurrentState->Cleanup();
    CurrentState = NULL;

    TrashedStates.Clear();

    UIMgr.Shutdown();
    // Shutdown resource mgr last
    ResourceMgr.Shutdown();    
}

void    Game::OnKeyEvent(const GFx::KeyEvent& e)
{
    if (!bRunning) return;

    // Intercept special keys to simulate 'system' events
    if (e.Type == GFx::Event::KeyUp)
    {
        // ** Simulate a network error
        if (e.KeyCode == Key::Pause)
        {
            OnNetworkError();
        }
    }

    if (SystemState->OnKeyEvent(e)) return;

    if (!HasCurrentState()) return;

    //if (!CurrentState->OnKeyEvent(e))
    //    UIMgr.OnKeyEvent(e);
    CurrentState->OnKeyEvent(e);
}

void    Game::OnCharEvent(const GFx::CharEvent& e)
{
    if (!bRunning) return;

    if (SystemState->OnCharEvent(e)) return;

    if (!HasCurrentState()) return;

    CurrentState->OnCharEvent(e);
}

void    Game::OnMouseEvent(const GFx::MouseEvent& e)
{
    if (!bRunning) return;

    // Keep track of mouse position
    // When a state is added; we update the mouse position for the state immediately
    LastMouseX = e.x; LastMouseY = e.y;

    if (SystemState->OnMouseEvent(e)) return;

    if (!HasCurrentState()) return;

    CurrentState->OnMouseEvent(e);
}

void    Game::UpdateViewSize(Render::Size<unsigned> size)
{
    Viewport = size;
}

void    Game::OnNetworkError()
{
    if (!bRunning) return;

    // On a network error, we show an error message/dialog in the system UI
    SystemState->ShowError();

    // Update the new state with the current mouse cursor position
    SystemState->OnMouseEvent(GFx::MouseEvent(GFx::Event::MouseMove, 0, LastMouseX, LastMouseY));
}
