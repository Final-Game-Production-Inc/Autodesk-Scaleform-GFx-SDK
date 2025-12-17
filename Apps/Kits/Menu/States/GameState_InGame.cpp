/**************************************************************************

Filename    :   GameState_InGame.cpp
Content     :   In-game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "GameState_InGame.h"
#include "../Game.h"
#include "GameState_Start.h"

//////////////////////////////////////////////////////////////////////////

bool    GameState_InGame::Initialize()
{
    Env->GetResourceMgr().GetMemoryFileCache()->AddSet(Data.FileCache);

    Env->GetResourceMgr().InitLoader(Loader);

    String uiFilename = Env->GetResourceMgr().BuildPath("InGame.swf");
    Ptr<GFx::MovieDef> ingameDef = *Env->GetResourceMgr().LoadMovieDef(&Loader, uiFilename);

    // Start displaying the main menu.swf
    InGameUI = *SF_HEAP_AUTO_NEW(this)    GameUISwfMovie();
    if (!InGameUI->Initialize(ingameDef, this)) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to create movie instance for In-game UI\n");
        return false;
    }
    SF_DEBUG_MESSAGE(1, "Pushing InGame UI\n");
    Env->GetUIMgr().PushUI(InGameUI);

    return true;
}


void    GameState_InGame::Update(GameUIAdvanceStats* pstats, UInt64 currTicks)
{
    SF_UNUSED(currTicks);

    InGameUI->Advance(pstats);
}

void    GameState_InGame::Cleanup()
{
    Env->GetResourceMgr().GetMemoryFileCache()->RemoveSet(Data.FileCache);

    // Clean up UI assets
    SF_DEBUG_MESSAGE(1, "Popping InGame UI\n");
    Env->GetUIMgr().PopUI();    // PPS: This is not ideal.. Need a mechanism to remove specific swfs instead of a stack?
}

void    GameState_InGame::OnUICallback(GameUISwfMovie* ui, const char* methodName, const GFx::Value* args, unsigned argCount)
{
    SF_UNUSED4(ui, methodName, args, argCount);
}


bool    GameState_InGame::OnKeyEvent(const GFx::KeyEvent& e)
{
    if (e.Type == GFx::Event::KeyUp && e.KeyCode == Key::Return)
    {
        Env->SetCurrentState(SF_NEW GameState_Start(Env));
        return true;
    }
    return InGameUI->OnKeyEvent(e);
}

bool    GameState_InGame::OnCharEvent(const GFx::CharEvent& e)
{
    return InGameUI->OnCharEvent(e);
}

bool    GameState_InGame::OnMouseEvent(const GFx::MouseEvent& e)
{
    return InGameUI->OnMouseEvent(e);
}

void    GameState_InGame::Pause(bool pause)
{
    InGameUI->Pause(pause);
}
