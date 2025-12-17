/**************************************************************************

Filename    :   GameState_Start.cpp
Content     :   Bootup game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_HeapNew.h"
#include "GameState_Start.h"
#include "GameState_Splash.h"
#include "../Game.h"

bool    GameState_Start::Initialize()
{ 
    Env->SetCurrentState(SF_HEAP_NEW(Env->GetMainMenuHeap()) GameState_Splash(Env));
    return true;
}
