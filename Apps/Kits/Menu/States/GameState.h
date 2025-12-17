/**************************************************************************

Filename    :   GameState.h
Content     :   Base game state for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameState_H
#define INC_GameState_H

// Includes
#include "Kernel/SF_String.h"
#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Event.h"

using namespace Scaleform;

// Forward declarations
class Game;
struct GameUIAdvanceStats;

// ********************************************************************
// Base game state
//
class GameState : public RefCountBase<GameState, Stat_Default_Mem>
{
public:
    GameState(Game* env, const char* name) : Env(env), Name(name) {}
    virtual ~GameState() {}

    // Called when the game state is activated (made current)
    virtual bool        Initialize() = 0;
    // Called when the game state is ticked
    virtual void        Update(GameUIAdvanceStats* pstats, UInt64 currTicks)    { SF_UNUSED2(pstats, currTicks); }
    // Called when the game state is destroyed
    virtual void        Cleanup()                   {}

    // Pause/unpause the game state and attached resources
    virtual void        Pause(bool pause)                     { SF_UNUSED(pause); }

    // User events to be handled by game state
    virtual bool        OnMouseEvent(const GFx::MouseEvent& e)    { SF_UNUSED(e); return false; }
    virtual bool        OnKeyEvent(const GFx::KeyEvent& e)        { SF_UNUSED(e); return false; }
    virtual bool        OnCharEvent(const GFx::CharEvent& e)      { SF_UNUSED(e); return false; }

    // IO access notification interface
    virtual void        OnIoLock()          {}
    virtual void        OnIoUnlock()        {}

    // Retrieve the state's name
    const String&       GetName() const     { return Name; }
    Game*               GetGame() const     { return Env; }

protected:
    Game*       Env;

    String     Name;
};


#endif // INC_GameState_H
