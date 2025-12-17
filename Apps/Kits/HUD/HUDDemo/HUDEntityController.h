/**************************************************************************

Filename    :   HUDEntityController.h
Content     :   HUD simulation entity controllers 
Created     :   December 9, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_HUDEntityController_H
#define INC_HUDEntityController_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Memory.h"

namespace SF = Scaleform;
using namespace Scaleform;

class SimEntity;
class SimPlayerEntity;
class HUDSimulation;

//
// The following types are primarily controllers for entities, primarily update
// logic and AI behaviors. Controllers are passed an entity which they then update
// appropriately.  User player state processing is also defined and managed.
//

//
// A standard action that reports a non-zero value for a duration
//
class SimValueAction
{
public:
    SimValueAction() : Value(0.f), DurationTicks(0.f) {}
    SimValueAction(float v, float dticks) : Value(v), DurationTicks(dticks) {}
    float   GetValue() const                        { return Value; }
    void    SetValue(float val)                     { Value = val; }
    float   GetRemainingDuration() const            { return DurationTicks; }
    void    SetRemainingDuration(float duration)    { DurationTicks = duration; }
    void    Elapse(float ticks)    
    { 
        if (DurationTicks == 0) return;
        DurationTicks -= ticks; 
        if (DurationTicks < 0.f)
        {
            Value = 0.f;
            DurationTicks = 0.f;
        }
    }
    bool    IsDone() const { return DurationTicks == 0; }

private:
    float   Value;
    float   DurationTicks;
};

//
// Base player behavior interface.
//
class SimPlayerBehavior : public NewOverrideBase<Stat_Default_Mem>
{
public:
    SimPlayerBehavior(const SimValueAction& move) : MoveAction(move) {}
    virtual ~SimPlayerBehavior() {}

    virtual void    Update(float elapsedTicks, SimPlayerEntity* pentity, HUDSimulation* psim) = 0;
    virtual bool    IsDone(SimPlayerEntity* pentity) const = 0;

    virtual const char*    GetType() const     { return "unknown"; }
    virtual float   GetMovementValue() const { return MoveAction.GetValue(); }

protected:
    SimValueAction          MoveAction;
};


//
// Base entity controller interface.
//
class SimEntityController : public NewOverrideBase<Stat_Default_Mem>
{
public:
    virtual ~SimEntityController() {}

    virtual void    Update(float delta, HUDSimulation* psim, SimEntity* pent) = 0;
};


//
// CapturePoint controller.
//
class SimCapturePointController : public SimEntityController
{
public:
    SimCapturePointController() : CapturedState(0.f) {}
    virtual ~SimCapturePointController() {}

    virtual void    Update(float delta, HUDSimulation* psim, SimEntity* pent);

protected:
    float                   CapturedState;
    SimValueAction          CapturedStateCheck;
    SimValueAction          TeamScoreGenerator;
};


//
// Powerup controller.
//
class SimPowerupController : public SimEntityController
{
public:    
    virtual ~SimPowerupController() {}

    virtual void    Update(float delta, HUDSimulation* psim, SimEntity* pent);
};


//
// Player controller.
//
class SimPlayerController : public SimEntityController 
{
public:
    virtual ~SimPlayerController() {}
};


//
// User Player controller.
//
class SimUserPlayerController : public SimPlayerController
{
protected:
    // State enumeration values used to denote user
    // actions corresponding to movement commands.
    enum UserPlayerStateID
    {
        PLAYER_MovingForward    = 0,
        PLAYER_MovingBackward,
        PLAYER_TurningLeft,
        PLAYER_TurningRight,
        PLAYER_Shooting,

        PLAYER_StateCount,
    };

public:
    SimUserPlayerController();
    virtual ~SimUserPlayerController() {}

    // State management methods to control player movement.
    // Ex: If PlayerState[PLAYER_MovingForward] is true, then the player 
    // will continue to move forward every update until it is set to false.
    void            PlayerMoveForward(bool flag)    { PlayerState[PLAYER_MovingForward] = flag; }
    void            PlayerMoveBackward(bool flag)   { PlayerState[PLAYER_MovingBackward] = flag; }
    void            PlayerTurnLeft(bool flag)       { PlayerState[PLAYER_TurningLeft] = flag; }
    void            PlayerTurnRight(bool flag)      { PlayerState[PLAYER_TurningRight] = flag; }
    void            PlayerShooting(bool flag)       { PlayerState[PLAYER_Shooting] = flag; }

    virtual void    Update(float delta, HUDSimulation* psim, SimEntity* pent);

protected:
    bool    PlayerState[PLAYER_StateCount];
};

//
// Bot Player controller.
//
class SimBotPlayerController : public SimPlayerController
{
public:
    SimBotPlayerController() : pBehavior(NULL), bAttacking(false) {}
    virtual ~SimBotPlayerController()
    {
        if (pBehavior)
            delete pBehavior;
    }

    void    Update(float delta, HUDSimulation* psim, SimEntity* pent);

protected:
    SimPlayerBehavior*      pBehavior;
    bool                    bAttacking;

    void    ChangeWeapon(SimPlayerEntity* pplayer);
    void    ClearBehaviors();
};


#endif  //  INC_HUDEntityController_H
