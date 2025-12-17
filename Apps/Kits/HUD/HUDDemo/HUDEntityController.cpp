/**************************************************************************

Filename    :   HUDEntityController.cpp
Content     :   HUD simulation entity controllers 
Created     :   December 9, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Math.h"

#include "HUDEntityController.h"
#include "HUDSimulation.h"



//////////////////////////////////////////////////////////////////////////

//
// Update logic for Capture Points.
// Handles updating all capture point entities and updating the player with 
// information if they are in range.  Also fires capture events for UI.
// 
void    SimCapturePointController::Update(float delta, HUDSimulation *psim, SimEntity *pent)
{
    SimCapturePointEntity* pcap = (SimCapturePointEntity*)pent;

    CapturedStateCheck.Elapse(delta);
    TeamScoreGenerator.Elapse(delta);

    if (CapturedStateCheck.IsDone())
    {
        float captureCoeff = 0.1f;
        float captureRange = 1.f;

		// Find players in range of the capture objective.  Update the CapturedState based on
		// the result for each team (# of team's players in range * the capture coefficient).
        Array<HUDSimulation::QueryResult> bluePlayers, redPlayers;
        psim->GetEntitiesInRadius(&bluePlayers, HUDSimulation::SIM_BluePlayer, 
            pcap->GetPosition(), captureRange, psim->GetPlayer());
        CapturedState += bluePlayers.GetSize() * captureCoeff;
        psim->GetEntitiesInRadius(&redPlayers, HUDSimulation::SIM_RedPlayer, 
            pcap->GetPosition(), captureRange, psim->GetPlayer());
        CapturedState -= redPlayers.GetSize() * captureCoeff;

		// Special case for user player (since the radial query only returns minimap entities - which 
	    // does not include user player)
        bool userPlayerFound = false;
        if (pent->DistanceLessThan(psim->GetPlayer(), captureRange))
        {
			psim->GetPlayer()->SetCapturingTarget(pcap);  // Notify the player he is capturing a flag.
            CapturedState += (psim->GetPlayer()->GetTeamID() == SimEntity::TEAM_Red ? -1.f : 1.f) * captureCoeff;
            userPlayerFound = true;
		} 

        // Check capture state limits
        CapturedState = Alg::Clamp(CapturedState, -1.f, 1.f);
		pcap->SetCapturedState(CapturedState);

        CapturedStateCheck.SetRemainingDuration(1.f);
		
		// Check which team controls the capture objective.
		// -1.0  thru -0.4 is Red
		// -0.39 thru 0.39 is Neutral
		//  0.39 thru 1.0  is Blue
        SimEntity::Team prevTeam = pcap->GetTeamID();
        if (CapturedState > -0.4f && CapturedState < 0.4f) pcap->SetTeamID(SimEntity::TEAM_Neutral);
        else if (CapturedState >= 0.4f) pcap->SetTeamID(SimEntity::TEAM_Blue);
        else if (CapturedState <= -0.4f) pcap->SetTeamID(SimEntity::TEAM_Red);

        if (pcap->GetTeamID() != prevTeam)
        {
			// If the objective's team has changed since the last update, notify the simulation.
            psim->CaptureStateChange(pcap);
            switch (pcap->GetTeamID())
            {
            case SimEntity::TEAM_Blue:
                {
                    for (unsigned i=0; i < bluePlayers.GetSize(); i++)
                        psim->CaptureObjective((SimPlayerEntity*)bluePlayers[i].pEntity, pcap);
                } break;
            case SimEntity::TEAM_Red:
                {
                    for (unsigned i=0; i < redPlayers.GetSize(); i++)
                        psim->CaptureObjective((SimPlayerEntity*)redPlayers[i].pEntity, pcap);
                } break;
			default:
				break;
            }

            // Special case for user player (since the radial query only returns minimap entities - which 
            // does not include user player)
            if (psim->GetPlayer()->GetTeamID() == pcap->GetTeamID() && userPlayerFound)
			{
                psim->CaptureObjective(psim->GetPlayer(), pcap);
			}
        }
    }

	// If the objective is controlled by one of the two teams, add one point to their score
	// every three seconds they retain control.
    if (pcap->GetTeamID() != SimEntity::TEAM_Neutral && TeamScoreGenerator.IsDone())
    {
        psim->AddToTeamScore(pcap->GetTeamID(), 1.f);
        TeamScoreGenerator.SetRemainingDuration(3.0f);
    }
}

//////////////////////////////////////////////////////////////////////////

//
// Update logic for Powerups.
// 
void    SimPowerupController::Update(float delta, HUDSimulation* psim, SimEntity* pent)
{
    SF_UNUSED(delta);

    SimPowerupEntity* ppu = (SimPowerupEntity*)pent;

    float powerupRange = 0.1f;
    SimPlayerEntity* closestPlayer = NULL;
    float closestDistSq = SF_MAX_FLOAT;

	// Check if user player is in range of the entity
    float dsq = pent->GetDistanceSquared(psim->GetPlayer()); 
    if (dsq < powerupRange)
    {
        closestPlayer = psim->GetPlayer();
        closestDistSq = dsq;
    }

	// Find players in range of the entity 
    Array<HUDSimulation::QueryResult> bluePlayers, redPlayers;
    psim->GetEntitiesInRadius(&bluePlayers, HUDSimulation::SIM_BluePlayer, 
        ppu->GetPosition(), powerupRange);
    for (unsigned i=0; i < bluePlayers.GetSize(); i++)
    {
        SimPlayerEntity* pplayer = (SimPlayerEntity*)bluePlayers[i].pEntity;
        dsq = pent->GetDistanceSquared(pplayer);
        if (dsq < closestDistSq)
        {
            closestPlayer = pplayer;
            closestDistSq = dsq;
        }
    }

    psim->GetEntitiesInRadius(&redPlayers, HUDSimulation::SIM_RedPlayer, 
        ppu->GetPosition(), powerupRange);    
    for (unsigned i=0; i < redPlayers.GetSize(); i++)
    {
        SimPlayerEntity* pplayer = (SimPlayerEntity*)redPlayers[i].pEntity;
        dsq = pent->GetDistanceSquared(pplayer);
        if (dsq < closestDistSq)
        {
            closestPlayer = pplayer;
            closestDistSq = dsq;
        }
    }

	// If a player is within range, apply the powerup to that player.
    if (closestPlayer)
        psim->ApplyPowerup(closestPlayer, ppu);
}

//////////////////////////////////////////////////////////////////////////

SimUserPlayerController::SimUserPlayerController()
{
    memset(PlayerState, 0, sizeof(bool) * sizeof(PlayerState));
}


//
// Logic for user player updates and player states.
//
void    SimUserPlayerController::Update(float delta, HUDSimulation *psim, SimEntity *pent)
{
    SF_UNUSED(psim);
    SimPlayerEntity* pplayer = (SimPlayerEntity*)pent;
    float moveDelta = delta; // Set movement speed to delta
    float turnDelta = delta * 50.f;  // Set turning speed to 100x delta
	if (PlayerState[PLAYER_Shooting])
		psim->PlayerFireWeapon(delta);
    if (PlayerState[PLAYER_MovingForward])
        pplayer->SetMovement(moveDelta);
    else if (PlayerState[PLAYER_MovingBackward])
        pplayer->SetMovement(-moveDelta);
    else
        pplayer->SetMovement(0);
    if (PlayerState[PLAYER_TurningLeft])
        pplayer->SetRotation(-turnDelta);
    else if (PlayerState[PLAYER_TurningRight])
        pplayer->SetRotation(turnDelta);
    else
        pplayer->SetRotation(0);
}

//////////////////////////////////////////////////////////////////////////


//
// Bot behavior: roam until an objective is located.
//
class SimPlayerRoamBehavior : public SimPlayerBehavior
{
public:
    SimPlayerRoamBehavior(const SimValueAction& move, const SimValueAction& turn) 
        : SimPlayerBehavior(move), TurnAction(turn) {}

    virtual void    Update(float elapsedTicks, SimPlayerEntity* pentity, HUDSimulation* psim)
    {
        SF_UNUSED(psim);

        MoveAction.Elapse(elapsedTicks);
        TurnAction.Elapse(elapsedTicks);

        pentity->SetMovement(MoveAction.GetValue() * elapsedTicks);
        pentity->SetRotation(TurnAction.GetValue() * elapsedTicks);
    }
    virtual bool    IsDone(SimPlayerEntity* pentity) const 
    { 
        SF_UNUSED(pentity);
        return (MoveAction.IsDone() && TurnAction.IsDone());
    }
    virtual const char*     GetType() const     { return "roam"; }

protected:
    SimValueAction      TurnAction;
};


//
// Bot behavior: Move toward some target location.
//
class SimPlayerGotoBehavior : public SimPlayerBehavior
{
public:
    SimPlayerGotoBehavior(const SimValueAction& move, float turnDelta, const PointF& target)
        : SimPlayerBehavior(move), TurnDelta(turnDelta), Target(target) {}

    virtual void    Update(float elapsedTicks, SimPlayerEntity* pentity, HUDSimulation* psim)
    {
        SF_UNUSED(psim);

        MoveAction.Elapse(elapsedTicks);

        if (MoveAction.GetValue() > 0.f && pentity->GetPosition().DistanceSquared(Target) < 4.f) // 2 squared
            MoveAction.SetRemainingDuration(0.f);

        pentity->SetMovement(MoveAction.GetValue() * elapsedTicks);
        float targetDir = (float)SF_RADTODEG(pentity->GetPosition().Angle(Target));
        float div = TurnDelta < 0.f ? pentity->GetDirection() - targetDir : targetDir - pentity->GetDirection();
        pentity->SetRotation(TurnDelta * elapsedTicks * div);
    }
    virtual bool    IsDone(SimPlayerEntity* pentity) const
    {
        SF_UNUSED(pentity);
        return MoveAction.IsDone();
    }
    virtual const char*     GetType() const     { return "goto"; }

protected:
    float       TurnDelta;
    PointF     Target;
};


//
// Bot behavior: Target, chase, and attack another player entity.
//
class SimPlayerAttackBehavior : public SimPlayerBehavior
{
public:
    SimPlayerAttackBehavior(const SimValueAction& move, float turnDelta, SimPlayerEntity* target, float detectRange)
        : SimPlayerBehavior(move), TurnDelta(turnDelta), pTarget(target), DetectableRange(detectRange) {}

    virtual void    Update(float elapsedTicks, SimPlayerEntity* pentity, HUDSimulation* psim)
    {
        SF_ASSERT(pTarget->GetTeamID() != pentity->GetTeamID());

        MoveAction.Elapse(elapsedTicks);    
			
		// Calculate firing range
        SimWeapon* pweapon = pentity->GetWeapon();

        float dist2 = pentity->GetPosition().DistanceSquared(pTarget->GetPosition());
        if (dist2 < (pweapon->GetFiringRange() * pweapon->GetFiringRange()))
        {
			// If we're within range, try to fire.

			// If the recoil duration hasn't elapsed, we'll have to wait.
            if (!RecoilAction.IsDone()) 
                RecoilAction.Elapse(elapsedTicks);
            // If we can fire, go ahead.
			else
            {
                psim->FireWeaponAt(pentity, pTarget);
                RecoilAction = SimValueAction(0.f, pweapon->GetRecoilDuration());
            }
            pentity->SetMovement(0.f); // Stop moving.
        }
        else 
        {
			// Keep moving toward the target until we're within firing range.
            pentity->SetMovement(MoveAction.GetValue() * elapsedTicks);
        }

		// Calculate the best path to the target, keeping them in range.
        float targetDir = (float)SF_RADTODEG(pentity->GetPosition().Angle(pTarget->GetPosition()));
        float div = TurnDelta < 0.f ? pentity->GetDirection() - targetDir : targetDir - pentity->GetDirection();
        if (Alg::Abs(div) < 5.f)
        {
            pentity->SetDirection((targetDir < 0) ? targetDir + 360.f : targetDir);
            pentity->SetRotation(0.f);
        }
        else
            pentity->SetRotation(TurnDelta * elapsedTicks * div);
    }
    virtual bool    IsDone(SimPlayerEntity* pentity) const
    {
        return MoveAction.IsDone() || !pentity->DistanceLessThan(pTarget, DetectableRange);
    }
    virtual const char*     GetType() const     { return "attack"; }

protected:
    float                   TurnDelta;
    SimPlayerEntity*        pTarget;
    float                   DetectableRange;
    SimValueAction          RecoilAction;
};

//
// Bot behavior: Change equipped weapon based on current situation.
//
void SimBotPlayerController::ChangeWeapon(SimPlayerEntity* pplayer)
{
    // If health is low, then go crazy!
    if (pplayer->GetHealth() < 0.3)
        pplayer->SetWeaponType(SimWeapon::WEAP_Grenade);

    // Else, pick a random weapon
    int weapChoose = rand() % 3;
    pplayer->SetWeaponType((SimWeapon::WeaponType)weapChoose);
}

//
//	SimBotPlayer update logic. Triggers avaiable bot behaviors.
//
void SimBotPlayerController::Update(float delta, HUDSimulation *psim, SimEntity *pent)
{
    SimPlayerEntity* pplayer = (SimPlayerEntity*)pent;
    float detectRange = 10.f;

    if (!pBehavior || pBehavior->IsDone(pplayer))
    {
        ClearBehaviors();

        // Create a new move action
        SimValueAction move(SF_RANDF(0.2f, 0.25f), SF_RANDF(25.f, 300.f));

        // Check for enemies (non-team members) in detectable range
        Array<HUDSimulation::QueryResult> othersInDetectRange;
        psim->GetEntitiesInRadius(&othersInDetectRange, 
            pplayer->GetTeamID() == SimEntity::TEAM_Red ? HUDSimulation::SIM_BluePlayer : HUDSimulation::SIM_RedPlayer,
            pplayer->GetPosition(),             
            detectRange);

        // Check for powerups in detectable range
        Array<HUDSimulation::QueryResult> powerupsInDetectRange;
        psim->GetEntitiesInRadius(&powerupsInDetectRange, HUDSimulation::SIM_Powerup,
            pplayer->GetPosition(),
            detectRange);

        int switchWeap = rand() % 10;
        if (!pplayer->GetWeapon()->HasAmmo() || switchWeap > 6)
            ChangeWeapon(pplayer);

        // If health is getting low and powerup in range, go get it
        if ((pplayer->GetHealth() < 0.7f || !pplayer->GetWeapon()->HasAmmo()) && powerupsInDetectRange.GetSize() > 0)
        {
            pBehavior = new SimPlayerGotoBehavior(move, SF_RANDF(-0.3f, 0.3f), 
                            powerupsInDetectRange[0].pEntity->GetPosition());
        }
        // Else if enemy in range, go attack it
        else if (pplayer->GetWeapon()->HasAmmo() && othersInDetectRange.GetSize())
        {
            SimPlayerEntity* target = (SimPlayerEntity*)othersInDetectRange[rand() % othersInDetectRange.GetSize()].pEntity;
            pBehavior = new SimPlayerAttackBehavior(move, 
                (rand() % 2 == 0 ? -1.f : 1.f) * SF_RANDF(0.3f, 0.6f), target, detectRange);
            bAttacking = true;
        }
        // Else pick between roaming and going after an objective
        else
        {
            int behavior = rand() % 10;            
            if (behavior < 3)
            {
                SimValueAction turn(SF_RANDF(-0.3f, 0.3f), SF_RANDF(20.0f, 100.0f));
                pBehavior = new SimPlayerRoamBehavior(move, turn);
            }
            else
            {
                const Array<SimEntity*>& capturePoints = psim->GetCapturePoints();                
                const PointF& target = capturePoints[rand() % capturePoints.GetSize()]->GetPosition();
                pBehavior = new SimPlayerGotoBehavior(move, SF_RANDF(-0.3f, 0.3f), target);
            }
        }
    }
    else
    {
        if (!bAttacking)
        {
            // Check for enemies (non-team members) in detectable range
            float playerDetectRange = 15.f;
            Array<HUDSimulation::QueryResult> othersInDetectRange;
            psim->GetEntitiesInRadius(&othersInDetectRange, 
                pplayer->GetTeamID() == SimEntity::TEAM_Red ? HUDSimulation::SIM_BluePlayer : HUDSimulation::SIM_RedPlayer,
                pplayer->GetPosition(),
                playerDetectRange);
            // Special case for user player
            if (pplayer->GetTeamID() == SimEntity::TEAM_Red && pplayer->DistanceLessThan(psim->GetPlayer(), playerDetectRange))
            {
                HUDSimulation::QueryResult res;
                res.pEntity = psim->GetPlayer();
                res.DistanceSquared = playerDetectRange * playerDetectRange;
                othersInDetectRange.PushBack(res);
            }

            if (othersInDetectRange.GetSize() > 0)
            {
                float moveVal = pBehavior->GetMovementValue();
                ClearBehaviors();
				if (pplayer->GetWeapon()->GetNumAmmoInClip() <= 4) pplayer->GetWeapon()->Reload(); 
                if (!pplayer->GetWeapon()->HasAmmo()) return;
                SimPlayerEntity* target = (SimPlayerEntity*)othersInDetectRange[rand() % othersInDetectRange.GetSize()].pEntity;
                SimValueAction move(moveVal, SF_RANDF(25.f, 300.f));
                pBehavior = new SimPlayerAttackBehavior(move, 
                    (rand() % 2 == 0 ? -1.f : 1.f) * SF_RANDF(0.3f, 0.6f), target, playerDetectRange);
                bAttacking = true;
                return;
            }
        }
        else if (pplayer->GetHealth() < 0.7f)
        {       
            // Check for powerups in detectable range
            Array<HUDSimulation::QueryResult> powerupsInDetectRange;
            psim->GetEntitiesInRadius(&powerupsInDetectRange, HUDSimulation::SIM_Powerup,
                pplayer->GetPosition(),
                detectRange);
            if (powerupsInDetectRange.GetSize() > 0)
            {
                float moveVal = pBehavior->GetMovementValue();
                ClearBehaviors();
                SimValueAction move(moveVal, SF_RANDF(25.f, 300.f));
                pBehavior = new SimPlayerGotoBehavior(move, SF_RANDF(-0.3f, 0.3f), 
                    powerupsInDetectRange[0].pEntity->GetPosition());
            }
        }
        pBehavior->Update(delta, pplayer, psim);
    }

    // Check for playfield bounds. SetRemainingDuration behavior if OOB    
    // PPS: May not be the ideal fallback.. removed for now
    //if (!psim->GetInboundsDimensions().Contains(pplayer->GetNextPhysicalPosition()))
    //        ClearBehaviors();
}

void    SimBotPlayerController::ClearBehaviors()
{
    if (pBehavior)
        delete pBehavior;
    pBehavior = NULL;
    bAttacking = false;
}

