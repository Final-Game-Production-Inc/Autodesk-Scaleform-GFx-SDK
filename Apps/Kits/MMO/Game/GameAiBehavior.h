/**************************************************************************

Filename    :   GameAiBehavior.h
Content     :   
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameAiBehavior_H
#define INC_GameAiBehavior_H

// Includes
#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

// Forward declarations
class GameServer;
class GamePawn;


//////////////////////////////////////////////////////////////////////////
//
// The behavior of an actor. Contains AI blocks that are processed depending on internal rules.
//
class GameAiBehavior : public RefCountBase<GameAiBehavior, Stat_Default_Mem>
{
public:
    virtual void    Process(GameServer* env, UInt32 ticks, GamePawn* pawn)    = 0;

    virtual void    OnWhisper(GameChannelMessage msg)   = 0;
};



//////////////////////////////////////////////////////////////////////////
//
//
class GameAiBehaviorChatterBox : public GameAiBehavior
{
public:
    GameAiBehaviorChatterBox();

    virtual void    Process(GameServer* env, UInt32 ticks, GamePawn* pawn);

    virtual void    OnWhisper(GameChannelMessage msg);

protected:
    UInt32      LastChatterTicks;
    UInt32      Interval;

    unsigned    Mode;
};

#endif  // INC_GameAiBehavior_H
