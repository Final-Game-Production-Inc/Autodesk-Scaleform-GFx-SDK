/**************************************************************************

Filename    :   HUDSimulation.cpp
Content     :   HUD demo simulation type implementations
Created     :   December 4, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Math.h"
#include "Kernel/SF_MsgFormat.h"

#include "HUDSimulation.h"

float SF_GetRandomfloat(float min, float max)
{
    return min + ( ((float)rand() / RAND_MAX) * (max - min) );
}


//////////////////////////////////////////////////////////////////////////

SimEntity::SimEntity(SimEntityController* pcont)
: pController(pcont)
{
    
}

SimEntity::~SimEntity()
{
    if (pController)
        delete pController;
}

void    SimEntity::Update(float delta, HUDSimulation *psim)
{
    if (pController)
        pController->Update(delta, psim, this);
}

//////////////////////////////////////////////////////////////////////////

//
// Pistol definition.
//
class SimPistolWeapon : public SimWeapon
{
public:
	SimPistolWeapon() : SimWeapon(SimItem::ITEM_Pistol, SimWeapon::WEAP_Pistol) {}
	String			GetName() const				{ return "Pistol"; }

    virtual float   GetAOERange() const         { return 0.f; }
    virtual float   GetFiringRange() const      { return 4.f; }
    virtual float   GetDamageCoeff() const      { return 0.1f; }
    virtual float   GetRecoilDuration() const   { return 0.5f; }
    virtual unsigned    GetMaxClips() const         { return 6; }
    virtual unsigned    GetMaxAmmoInClip() const    { return 12; }
};

//
// Machine gun definition.
//
class SimMachineGunWeapon : public SimWeapon
{
public:
	SimMachineGunWeapon() : SimWeapon(SimItem::ITEM_MachineGun, SimWeapon::WEAP_MachineGun) {}
	String			GetName() const				{ return "MachineGun"; }

    virtual float   GetAOERange() const         { return 0.f; }
    virtual float   GetFiringRange() const      { return 4.f; }
    virtual float   GetDamageCoeff() const      { return 0.05f; }
    virtual float   GetRecoilDuration() const   { return 0.15f; }
    virtual unsigned    GetMaxClips() const         { return 5; }
    virtual unsigned    GetMaxAmmoInClip() const    { return 50; }
};

//
// Grenade definition.
//
class SimGrenadeWeapon : public SimWeapon
{
public:

	SimGrenadeWeapon() : SimWeapon(SimItem::ITEM_Grenade, SimWeapon::WEAP_Grenade) {}
	String			GetName() const				{ return "Grenade"; }

    virtual float   GetAOERange() const         { return 2.5f; }
    virtual float   GetFiringRange() const      { return 4.f; }
    virtual float   GetDamageCoeff() const      { return 0.8f; }
    virtual float   GetRecoilDuration() const   { return 1.f; }
    virtual unsigned    GetMaxClips() const         { return 4; }
    virtual unsigned    GetMaxAmmoInClip() const    { return 1; }
};

class SimMolotovWeapon : public SimWeapon
{
public:

	SimMolotovWeapon() : SimWeapon(SimItem::ITEM_Molotov, SimWeapon::WEAP_Grenade) {}
	String			GetName() const				{ return "Molotov"; }

	virtual float   GetAOERange() const         { return 2.5f; }
	virtual float   GetFiringRange() const      { return 4.f; }
	virtual float   GetDamageCoeff() const      { return 0.8f; }
	virtual float   GetRecoilDuration() const   { return 1.f; }
	virtual unsigned    GetMaxClips() const         { return 4; }
	virtual unsigned    GetMaxAmmoInClip() const    { return 1; }
};

//////////////////////////////////////////////////////////////////////////


class SimMedkitItem : public SimItem
{
public:
	SimMedkitItem() : SimItem(SimItem::ITEM_Medkit) {}

	String				GetName() const			{ return "Medkit"; }
	bool				IsUsable() const		{ return true; }
}; 

class SimArmorItem : public SimItem
{
public:
	SimArmorItem() : SimItem(SimItem::ITEM_Armor) {}

	String				GetName() const			{ return "Armor"; }
	bool				IsUsable() const		{ return false; }
}; 

//////////////////////////////////////////////////////////////////////////

SimPlayerEntity::SimPlayerEntity(SimPlayerController* pcont, const char* pname, 
                                 SimEntity::Team team)
: SimEntity(pcont), Name(pname), TeamID(team), KillStreak(0), LevelXP(0.f), CurrentRank(RANK_Private), ActiveMovement(0), 
  ActiveRotation(0), MovementMultiplier(1.f)
{
    // Set random direction
    Direction.SetPoint(SF_RANDF(-(float)SF_MATH_PI, (float)SF_MATH_PI), 
        SF_RANDF(-(float)SF_MATH_PI, (float)SF_MATH_PI)); 

	// Supply the player with weapons.
    Weapons[SimWeapon::WEAP_Pistol] = new SimPistolWeapon();
    Weapons[SimWeapon::WEAP_MachineGun] = new SimMachineGunWeapon();
    Weapons[SimWeapon::WEAP_Grenade] = new SimGrenadeWeapon();

	for (unsigned i=0; i < (unsigned)INVENTORY_SIZE; i++)
		Inventory[i] = NULL;
}

SimPlayerEntity::~SimPlayerEntity()
{
    for (unsigned i=0; i < (unsigned)SimWeapon::NUM_TYPES; i++)
        delete Weapons[i];

	//TODO: Is this necessary?
	for (unsigned i=0; i < (unsigned)INVENTORY_SIZE; i++)
	{
		if (Inventory[i] != NULL)
			delete Inventory[i];
	}
}

PointF SimPlayerEntity::GetNextPosition() const
{ 
    float am = ActiveMovement * MovementMultiplier;
    PointF p = Position;
    p.x += (Direction.x * am);
    p.y += (Direction.y * am);
    return p;
}

float SimPlayerEntity::GetDirection() const
{
    return (float)SF_RADTODEG( atan2(Direction.y, Direction.x) );
}

void SimPlayerEntity::SetDirection(float a)
{
    float x = 1.f, y = 0.f; // 0 angle
    float rad = (float)SF_DEGTORAD(a);
    float cosa = (float)cos(rad), sina = (float)sin(rad);
    Direction.x = x * cosa - y * sina;
    Direction.y = x * sina + y * cosa;  
}

void SimPlayerEntity::ClearInventory()
{
	for (unsigned i=0; i < (unsigned)INVENTORY_SIZE; i++) 
	{
		if (Inventory[i] != NULL)
		{
			delete Inventory[i];
			Inventory[i] = NULL;
		}
	}
}

//
// Player respawn logic.
//
void SimPlayerEntity::Respawn(const PointF& respawnPoint)
{
	// Player always respawns at the respawn point.
    SimEntity::SetPosition(respawnPoint);
    Health = 1.f;   // Normalized

    CurrentWeapon = SimWeapon::WEAP_MachineGun;		// Start the player with the machine gun selected.
	for (unsigned i=0; i < (unsigned)SimWeapon::NUM_TYPES; i++){	// Replenish their ammo and reload their weapons.
        Weapons[i]->SetMaxAmmo();
		Weapons[i]->Reload();
	}

	ClearInventory();

	Inventory[WEAPON_SLOT_1] = new SimPistolWeapon();
	Inventory[WEAPON_SLOT_2] = new SimMachineGunWeapon();

    SetCapturingTarget( NULL ); 
}

//
// Increment player XP.
//
bool SimPlayerEntity::AddToXP(float xp)
{
	if (CurrentRank < NUM_RANKS-1)
	{
		LevelXP += xp;  // Takes 10*rank xp to level up
		float diff = LevelXP - 1.f;
		if (diff > 0.f)
		{
			 // Level up
            CurrentRank = (Rank)(CurrentRank + 1);
            LevelXP = diff;
            return true;
        }
    }
    return false;
}

//
// Change the weapon the player has equipped.
//
bool SimPlayerEntity::ChangeWeapon(SimWeapon::WeaponType w)
{
	if (w != CurrentWeapon && Weapons[w] != NULL)
	{
		SetWeaponType(w);
		return true;
	}
	return false;
}

//
//	Cycles through the player's available weapons.
//
bool SimPlayerEntity::CycleWeapons()
{
	//If the player has no weapons equipped, return false.
	//If a weapon is available in the next weapon slot, use that.
	//If a weapon is not available in the next weapon slot, use the next available weapon in a loop.
	//If no weapon other weapon is available, return false and do not change weapons.
	SimWeapon::WeaponType targWeapon;
	if ( GetWeaponType() == SimWeapon::WEAP_MachineGun )
		targWeapon = SimWeapon::WEAP_Pistol;
	else 
		targWeapon = SimWeapon::WEAP_MachineGun;

	if ( Weapons[targWeapon] != NULL )
	{
		ChangeWeapon(targWeapon);
		return true;
	}
	else
		return false;
}

bool SimPlayerEntity::ChangeWeaponByIndex(unsigned index)
{
	if (index > 3)
		return false;

	unsigned invIndex = index + ITEM_SLOTS;
	if (Inventory[invIndex] == NULL)
		return false;
	else
	{
		SimWeapon* targetWeapon = (SimWeapon*)Inventory[invIndex];
		switch (targetWeapon->GetItemType())
		{
		case SimItem::ITEM_MachineGun:
			{
				ChangeWeapon( SimWeapon::WEAP_MachineGun );
			}
			break;

		case SimItem::ITEM_Pistol:
			{
				ChangeWeapon( SimWeapon::WEAP_Pistol );
			}
			break;

		default:
			return false;
		break;
		}
	}
	
	return true;
}

bool SimPlayerEntity::StoreItem(SimItem* item)
{
	unsigned freeIndex = HasInventorySpace();
	if (freeIndex != (unsigned)-1)
	{
		Inventory[freeIndex] = item;
		return true;
	}
	return false;
}

unsigned SimPlayerEntity::HasInventorySpace()
{
	for (unsigned i = 0; i < INVENTORY_SIZE - WEAPON_SLOTS; i++) 
	{
		if (Inventory[i] == NULL)
			return i;
	}
	return unsigned(-1);
}

//
// Compute relative direction from this entity to the target entity.
//
float SimPlayerEntity::ComputeRelativeDirection(SimEntity* pent)
{
    PointF entRel = pent->GetPosition();
    float a = (float)SF_RADTODEG( atan2( entRel.y - Position.y, entRel.x - Position.x ) );
    return a - GetDirection();
}

//
// PlayerEntity update logic. Handles player states, movement, and tracks
// the capturing target.
//
void SimPlayerEntity::Update(float delta, HUDSimulation* psim)
{
    SimEntity::Update(delta, psim);

    if (ActiveRotation != 0)
    {
        float x = Direction.x, y = Direction.y;
        float rad = (float)SF_DEGTORAD(ActiveRotation);
        float cosa = (float)cos(rad), sina = (float)sin(rad);
        Direction.x = x * cosa - y * sina;
        Direction.y = x * sina + y * cosa;  
    }
    if (ActiveMovement != 0)
    {
        float am = ActiveMovement * MovementMultiplier;
        float oldx = Position.x, oldy = Position.y;
        Position.x += (Direction.x * am);
        Position.y += (Direction.y * am);
        if (!psim->GetInboundsDimensions().Contains(Position))
        {            
            Position.x = oldx;
            Position.y = oldy;
        }
		if ( psim->GetPlayer()->GetCapturingTarget() != NULL ) 
		{
			if (!psim->GetPlayer()->GetCapturingTarget()->DistanceLessThan(psim->GetPlayer(), 1.0f))
			{
				psim->GetPlayer()->SetCapturingTarget(NULL);
			}
		}
    }
}

//////////////////////////////////////////////////////////////////////////

//
// Generate a random health and ammo boost for a powerup.
// These boosts will be applied to the first player who comes within range.
//
void SimPowerupEntity::GenerateRandomPowerup()
{
    HealthBoost = ((rand() % 80) + 20) / 100.f;
    WeaponType = (SimWeapon::WeaponType)(rand() % SimWeapon::NUM_TYPES);
    AmmoBoost = 0;

    switch (WeaponType)
    {
    case SimWeapon::WEAP_Pistol:
        {
            AmmoBoost = rand() % 100;
        } break;
    case SimWeapon::WEAP_MachineGun:
        {
            AmmoBoost = rand() % 200;
        } break;
    case SimWeapon::WEAP_Grenade:
        {
            AmmoBoost = rand() % 4;
        } break;
	default:
		break;
    }
}


//////////////////////////////////////////////////////////////////////////

//
// Setup for HUD Simulation.
//
HUDSimulation::HUDSimulation(const Config& config)
 : bFirstUpdate(true), bEnabled(false)
{
    MapDimensions = config.PhysicalMapDimensions;
    InboundsDimensions = config.InboundsDimensions;

    RuntimeLocations.BlueSpawnPoints = config.InitialLocations.BlueSpawnPoints;
    RuntimeLocations.RedSpawnPoints = config.InitialLocations.RedSpawnPoints;

    for (unsigned i=0; i < SimEntity::TEAM_Count; i++)
        TeamScore[i] = 0.f;

    srand(31337);

    RuntimeSettings = config.InitialSettings;
    Restart();

    pPlayerController = new SimUserPlayerController();
    pPlayer->SetPlayerController(pPlayerController);
    bUserBotControlled = false;

	ScoreToWin = 500.f;
}

HUDSimulation::~HUDSimulation()
{
    for (UPInt i=0; i < BluePlayerEntities.GetSize(); i++) 
        delete BluePlayerEntities[i];
    for (UPInt i=0; i < RedPlayerEntities.GetSize(); i++) 
        delete RedPlayerEntities[i];
    for (UPInt i=0; i < CapturePointEntities.GetSize(); i++) 
        delete CapturePointEntities[i];
    for (UPInt i=0; i < PowerupEntities.GetSize(); i++) 
        delete PowerupEntities[i];
}

//
// Restart the round by resetting the round time and team scores.
//
void HUDSimulation::Restart()
{
    // Generate entities
    SetNumBluePlayers(RuntimeSettings.NumBluePlayers);
    SetNumRedPlayers(RuntimeSettings.NumRedPlayers);
    SetNumCapturePoints(RuntimeSettings.NumCapturePoints);
    SetNumPowerups(RuntimeSettings.NumPowerups);

    // Assign one of the blue team players as the user player
    pPlayer = (SimPlayerEntity*)BluePlayerEntities[0];
    pPlayer->SetName("east77");
    pPlayer->SetDirection(45); 

	for (unsigned i=0; i < CapturePointEntities.GetSize(); i++)
	{
		SimCapturePointEntity* flag = (SimCapturePointEntity*)CapturePointEntities[i];
		flag->SetCapturedState(0);
		flag->SetTeamID(SimEntity::TEAM_Neutral);
	}

    SecondsLeftInRound = 15 * 60;  // 15 minutes
	ResetTeamScores();
}

//
// Simulation update logic.  Call update logic for all valid entities.  Restart the game
// if round over conditions have been met.
// 
void HUDSimulation::Update(UInt64 ticks)
{
	if (!IsRunning()) return;

    if (bFirstUpdate)
    {
        LastTicks = ticks;
        bFirstUpdate = false;
    }
    UInt64 ticksDiff = (ticks - LastTicks);
    float delta = ticksDiff * (1.0f/TIME_UNITS); // Normalized to 1s
    LastTicks = ticks;
    
	// Track the seconds left in the round.  Restart if the round time has elapsed.
    SecondsLeftInRound -= delta;
    if (SecondsLeftInRound < 0 || (GetBlueTeamScore() >= ScoreToWin) || (GetRedTeamScore() >= ScoreToWin))         
    {
        Restart();
        return;
	}

    // Tick all players
	float playerDelta = delta * 2.f;
    for (unsigned i=0; i < BluePlayerEntities.GetSize(); i++)
    {
        SimPlayerEntity* pplayer = (SimPlayerEntity*)BluePlayerEntities[i];
        if (pplayer->GetHealth() == 0.f) Respawn(pplayer);
        pplayer->Update(playerDelta, this);
    }
    for (unsigned i=0; i < RedPlayerEntities.GetSize(); i++)
    {
        SimPlayerEntity* pplayer = (SimPlayerEntity*)RedPlayerEntities[i];
        if (pplayer->GetHealth() == 0.f) Respawn(pplayer);
        pplayer->Update(playerDelta, this);
    }

    // Process capture points
    for (unsigned i=0; i < CapturePointEntities.GetSize(); i++)
    {
        SimCapturePointEntity* flag = (SimCapturePointEntity*)CapturePointEntities[i];
        flag->Update(delta, this);
    }

    // Process powerups
    for (unsigned i=0; i < PowerupEntities.GetSize(); i++)
    {
        SimPowerupEntity* pup = (SimPowerupEntity*)PowerupEntities[i];
        pup->Update(delta, this);
    }

	// Elapse player's recoil duration.
	pPlayer->RecoilAction.Elapse( delta );
}

//
// Query method to retrieve the set of entities in a certain circular 
// range of some xy point - based on radial distance from that center position.
//
void HUDSimulation::GetEntitiesInRadius(Array<HUDSimulation::QueryResult>* presult, 
                                             HUDSimulation::SimType  type,
                                             const PointF& physCenter, float physicalRadius,
                                             SimEntity* pignore)
{
    presult->Clear();

    // Get the source array
    Array<SimEntity*>*  psrc = NULL;
    switch (type)
    {
    case SIM_BluePlayer:    psrc = &BluePlayerEntities; break;
    case SIM_RedPlayer:     psrc = &RedPlayerEntities; break;
    case SIM_CapturePoint:  psrc = &CapturePointEntities; break;
    case SIM_Powerup:       psrc = &PowerupEntities; break;
	default:
		break;
    }
    if (!psrc) return;

    // Prune by radius (implement a faster algorithm for this.. works for now)
    float radius2 = physicalRadius * physicalRadius;
    for (UPInt i=0; i < psrc->GetSize(); i++) 
    {
        SimEntity* data = (*psrc)[i];
        if (pignore == data) continue;
        float dx = physCenter.x - data->GetPosition().x;
        float dy = physCenter.y - data->GetPosition().y;
        float dist2 = (dx*dx + dy*dy);
        if (  dist2 <= radius2 )
        {
            HUDSimulation::QueryResult result;
            result.pEntity = data;
            result.DistanceSquared = dist2;
            presult->PushBack(result);
        }
    }
}

//
// Query methods to retrieve the set of players in a certain circular 
// view range - based on radial distance from the player position.
//
void    HUDSimulation::GetEntitiesInViewFrustum(Array<FrustumResult> *presult, 
                                                HUDSimulation::SimType type,
                                                float radius)
{
    presult->Clear();

    // Get the source array
    Array<SimEntity*>*  psrc = NULL;
    switch (type)
    {
    case SIM_BluePlayer:    psrc = &BluePlayerEntities; break;
    case SIM_RedPlayer:     psrc = &RedPlayerEntities; break;
    case SIM_CapturePoint:  psrc = &CapturePointEntities; break;
    case SIM_Powerup:       psrc = &PowerupEntities; break;
	default: break;
    }
    if (!psrc) return;

    // Prune by radius (implement a faster algorithm for this.. works for now)
    float radius2 = radius * radius;
    for (UPInt i=0; i < psrc->GetSize(); i++) 
    {
        SimEntity* data = (*psrc)[i];
        float dx = GetPlayerPosition().x - data->GetPosition().x;
        float dy = GetPlayerPosition().y - data->GetPosition().y;
        float dist2 = (dx*dx + dy*dy);
        float a = pPlayer->ComputeRelativeDirection(data);
        float mina = -GetFOVAngle(), maxa = GetFOVAngle();
        if (  (a > mina && a < maxa) && dist2 <= radius2 && pPlayer != data )
        {
            FrustumResult fr;
            fr.pEntity = data;
            fr.Angle = a;
            fr.DistanceSquared = dist2;
            presult->PushBack(fr);
        }
    }    
}

//
// Sets the number of blue players in the simulation.
//
void    HUDSimulation::SetNumBluePlayers(unsigned newVal)
{
    // Remove extra entities
    while (BluePlayerEntities.GetSize() > newVal)
    {
        // Remove icon reference before deletion
        SimEntity* pentity = BluePlayerEntities[BluePlayerEntities.GetSize()-1];
        BluePlayerEntities.PopBack();
        FireEvent(Event(Event::EVT_RemoveEntity, pentity));
        delete pentity;
    }

    // Create extra entities
    while (BluePlayerEntities.GetSize() < newVal)
    {
        String botname;
        Format(botname, "bluebot{0}", BluePlayerEntities.GetSize()+1);
        SimEntity* pent = new SimPlayerEntity(new SimBotPlayerController(),
            botname.ToCStr(),
            SimEntity::TEAM_Blue);
        pent->SetPosition(RuntimeLocations.BlueSpawnPoints[rand() % RuntimeLocations.BlueSpawnPoints.GetSize()]);
        BluePlayerEntities.PushBack(pent);
        FireEvent(Event(Event::EVT_AddEntity, pent));
    }

    RuntimeSettings.NumBluePlayers = newVal;
}


//
// Sets the number of red players in the simulation.
//
void    HUDSimulation::SetNumRedPlayers(unsigned newVal)
{
    // Remove extra entities
    while (RedPlayerEntities.GetSize() > newVal)
    {
        // Remove icon reference before deletion
        SimEntity* pentity = RedPlayerEntities[RedPlayerEntities.GetSize()-1];
        RedPlayerEntities.PopBack();
        FireEvent(Event(Event::EVT_RemoveEntity, pentity));
        delete pentity;
    }

    // Create extra entities
    while (RedPlayerEntities.GetSize() < newVal)
    {
        String botname;
        Format(botname, "redbot{0}", RedPlayerEntities.GetSize()+1);
        SimEntity* pent = new SimPlayerEntity(new SimBotPlayerController(),
            botname.ToCStr(),
            SimEntity::TEAM_Red);
        pent->SetPosition(RuntimeLocations.RedSpawnPoints[rand() % RuntimeLocations.RedSpawnPoints.GetSize()]);
        RedPlayerEntities.PushBack(pent);
        FireEvent(Event(Event::EVT_AddEntity, pent));
    }

    RuntimeSettings.NumRedPlayers = newVal;
}


//
// Sets the number of capture points in the simulation.
//
void    HUDSimulation::SetNumCapturePoints(unsigned newVal)
{
    // Remove extra entities
    while (CapturePointEntities.GetSize() > newVal)
    {
        // Remove icon reference before deletion
        SimEntity* pentity = CapturePointEntities[CapturePointEntities.GetSize()-1];
        CapturePointEntities.PopBack();
        FireEvent(Event(Event::EVT_RemoveEntity, pentity));
        delete pentity;
    }

    // Create extra entities
    while (CapturePointEntities.GetSize() < newVal)
    {
        SimEntity* pent = new SimCapturePointEntity(new SimCapturePointController(),
            SimEntity::TEAM_Neutral);
        pent->SetPosition(PointF(SF_RANDF(InboundsDimensions.X1(), InboundsDimensions.X2()), 
            SF_RANDF(InboundsDimensions.Y1(), InboundsDimensions.Y2())));
        CapturePointEntities.PushBack(pent);
        FireEvent(Event(Event::EVT_AddEntity, pent));
    }

    RuntimeSettings.NumCapturePoints = newVal;
}

//
// Sets the number of powerups in the simulation.
//
void    HUDSimulation::SetNumPowerups(unsigned newVal)
{
    // Remove extra entities
    while (PowerupEntities.GetSize() > newVal)
    {
        // Remove icon reference before deletion
        SimEntity* pentity = PowerupEntities[PowerupEntities.GetSize()-1];
        PowerupEntities.PopBack();
        FireEvent(Event(Event::EVT_RemoveEntity, pentity));
        delete pentity;
    }

    // Create extra entities
    while (PowerupEntities.GetSize() < newVal)
    {
        SimEntity* pent =  new SimPowerupEntity(new SimPowerupController());
        pent->SetPosition(GetRandomInboundsPoint());
        PowerupEntities.PushBack(pent);
        FireEvent(Event(Event::EVT_AddEntity, pent));
    }

    RuntimeSettings.NumPowerups = newVal;
}

//
// Respawn a player at an appropriate spawn point.
//
void    HUDSimulation::Respawn(SimPlayerEntity* pplayer)
{
    if (pplayer->GetTeamID() == SimEntity::TEAM_Blue)
        pplayer->Respawn(RuntimeLocations.BlueSpawnPoints[rand() % RuntimeLocations.BlueSpawnPoints.GetSize()]);
    else
        pplayer->Respawn(RuntimeLocations.RedSpawnPoints[rand() % RuntimeLocations.RedSpawnPoints.GetSize()]);  
}

PointF HUDSimulation::GetRandomInboundsPoint() const
{
    return PointF(SF_RANDF(InboundsDimensions.X1(), InboundsDimensions.X2()), 
        SF_RANDF(InboundsDimensions.Y1(), InboundsDimensions.Y2()));
}

//
// Fire weapon logic for a weapon that only hits one target (pistol, machine gun).
// Returns true if the target player was killed.
//
bool    HUDSimulation::FireNoAOEWeaponAt(SimPlayerEntity *psrc, SimPlayerEntity *pdest)
{
    bool destWasAlive = pdest->IsAlive();

    float dmg = psrc->GetWeapon()->GetDamageCoeff();
    pdest->ApplyDamage(dmg);

	// Print a message to the log if the player scores a hit since we have no other indicator.
	if (psrc == GetPlayer())
	{
		printf("\nHit! - [%s: %f hp]", pdest->GetName().ToCStr(), pdest->GetHealth());
	}

	// Fire an Damage event if the player takes a hit, will ultimately show the UI hit indicator 
    if (pdest == GetPlayer())
    {
        FireEvent(Event(Event::EVT_Damage, psrc, pdest, dmg));
    }

    if (destWasAlive && pdest->IsDead())
    {
        AddToTeamScore(psrc->GetTeamID(), 1.f);		// If there was a kill, increment the team score.
        psrc->IncrementKillStreak();				// Increment the source's kill streak counter.
        pdest->ResetKillStreak();					// Reset the destination's kill streak counter.
        bool levelup = psrc->AddToXP(0.2f / (psrc->GetRank() + 1));		// Award the source player XP.

        FireEvent(Event(Event::EVT_Kill, psrc, pdest));

		if (psrc == GetPlayer() && (psrc->GetKillStreak() >= 3))
		{
			// If the player is on a kill streak > 3, fire an event to all listeners.
            FireEvent(Event(Event::EVT_KillStreak, psrc));
		}

        if (levelup && psrc == GetPlayer())
        {
			// If the player's level increased, fire an event to all listeners.
            FireEvent(Event(Event::EVT_LevelUp, psrc));
        }

        return true;
    }
    return false;
}

//
// Fire weapon logic for a weapon that can hit multiple targets in a radius (grenade). Used when
// the AOE Radius of the weapon fired is > 0.
//
// Queries for all enemies within radius (size defined by the weapon type) of destination and passes
// information on to FireNoAOEWeaponAt().  Returns true if at least one player entity was killed.
//
bool    HUDSimulation::FireAOEWeaponAt(SimPlayerEntity* psrc, const PointF& dest)
{
    // Get all enemy players in aoe range
    Array<HUDSimulation::QueryResult> enemies;
    GetEntitiesInRadius(&enemies, 
        (psrc->GetTeamID() == SimEntity::TEAM_Blue) ? HUDSimulation::SIM_RedPlayer : HUDSimulation::SIM_BluePlayer,
        dest, psrc->GetWeapon()->GetAOERange());
    unsigned kcount = 0;
    for (unsigned i=0; i < enemies.GetSize(); i++)
    {
        SimPlayerEntity* pdest = (SimPlayerEntity*)enemies[i].pEntity;
        if (FireNoAOEWeaponAt(psrc, pdest)) kcount++;
    }
    return kcount > 0;
}

//
// Fire an event when a capture entity's capture state changes.
//
void    HUDSimulation::CaptureStateChange(SimCapturePointEntity* psrc)
{
    FireEvent(Event(Event::EVT_CaptureStateChange, psrc));
}

//
// When a CaptureEntity is captured, increment the player who captured the objective's
// exp and fire appropriate events.
//
void    HUDSimulation::CaptureObjective(SimPlayerEntity* psrc, SimCapturePointEntity* pdest)
{
    SF_UNUSED(pdest);
    bool levelup = psrc->AddToXP(0.3f);

    FireEvent(Event(Event::EVT_PlayerCapture, psrc));

    if (levelup && psrc == GetPlayer())
    {
        FireEvent(Event(Event::EVT_LevelUp, psrc));
    }
}

//
// Apply a powerup to a player entity and fire appropriate events.
//
void    HUDSimulation::ApplyPowerup(SimPlayerEntity* pplayer, SimPowerupEntity* ppup)
{
    pplayer->AddHealthBoost(ppup->GetHealthBoost());
    pplayer->AddAmmo(ppup->GetWeaponBoostType(), ppup->GetAmmoBoost());
    ppup->Respawn(GetRandomInboundsPoint());

    if (pplayer == GetPlayer())
    {
        FireEvent(Event(Event::EVT_Powerup, pplayer)); // Fire a powerup event.
		GiveItem(pplayer, GenerateRandomItem());
    }
}

SimItem*	HUDSimulation::GenerateRandomItem()
{
	unsigned ItemType = (SimItem::Type)( ((rand() % 4) + 2) );

	SimItem* pitem = NULL;
	switch (ItemType)
	{
	case SimItem::ITEM_Armor:
		{
			pitem = new SimArmorItem();
		} break;
	case SimItem::ITEM_Grenade:
		{
			pitem = new SimGrenadeWeapon();
		} break;
	case SimItem::ITEM_Medkit:
		{
			pitem = new SimMedkitItem();
		} break;
	case SimItem::ITEM_Molotov:
		{
			pitem = new SimMolotovWeapon();
		} break;
	default:
		break;
	}
	return pitem;
}

void	HUDSimulation::GiveItem(SimPlayerEntity* pplayer, SimItem* pitem)
{
	if (pplayer == GetPlayer() && pitem != NULL)
	{
		if (!pplayer->StoreItem(pitem))
			pitem = NULL;
		
		//else
		//Fire InventoryFull event.
	}
}

//
// Base FireWeapon logic.  Forward call on to more specific FireAOEWeaponAt and FireNOAOEWeaponAt
// based on the weapon type.  Prevent firing if the player has no ammo for the weapon.  Returns
// true if the FireWeapon call killed the target player entity.
//
bool    HUDSimulation::FireWeaponAt(SimPlayerEntity* psrc, SimPlayerEntity* pdest)
{
    SimWeapon* pweap = psrc->GetWeapon();
    if (!pweap->HasAmmo()) return false;
    pweap->DecrementAmmo();
    if (pweap->GetAOERange() > 0)
        return FireAOEWeaponAt(psrc, pdest->GetPosition());
    else
        return FireNoAOEWeaponAt(psrc, pdest);
}


void	HUDSimulation::ResetTeamScores()
{
	for(unsigned i = 0; i < SimEntity::TEAM_Count; i++)
		TeamScore[i] = 0;
}

//
// Team score access
//
void    HUDSimulation::AddToTeamScore(unsigned teamID, float val)
{
    SF_ASSERT(teamID < sizeof(TeamScore));
    TeamScore[teamID] += val;
}


float    HUDSimulation::GetTeamScore(unsigned teamID) const  
{ 
    SF_ASSERT(teamID < sizeof(TeamScore)); 
    return TeamScore[teamID]; 
}

//
// Toggle user/AI control over user player.
//
void    HUDSimulation::SwitchUserController()
{
    bUserBotControlled = !bUserBotControlled;
    if (bUserBotControlled)
        pPlayer->SetPlayerController(new SimBotPlayerController());
    else
        pPlayer->SetPlayerController(new SimUserPlayerController());            
    pPlayerController = pPlayer->GetPlayerController();
}

//
// User control interface
//
void    HUDSimulation::PlayerMovingForward(bool flag)  
{ 
    if (!bUserBotControlled) 
        ((SimUserPlayerController*)pPlayerController)->PlayerMoveForward(flag); 
}
void    HUDSimulation::PlayerMovingBackward(bool flag)  
{ 
    if (!bUserBotControlled) 
        ((SimUserPlayerController*)pPlayerController)->PlayerMoveBackward(flag); 
}
void    HUDSimulation::PlayerTurningLeft(bool flag)  
{ 
    if (!bUserBotControlled) 
        ((SimUserPlayerController*)pPlayerController)->PlayerTurnLeft(flag); 
}
void    HUDSimulation::PlayerTurningRight(bool flag)  
{ 
    if (!bUserBotControlled) 
        ((SimUserPlayerController*)pPlayerController)->PlayerTurnRight(flag); 
}

void	HUDSimulation::PlayerShooting(bool flag)
{
	if (!bUserBotControlled)
		((SimUserPlayerController*)pPlayerController)->PlayerShooting(flag); 
}

void    HUDSimulation::PlayerFireWeapon( float delta )
{
	SF_UNUSED( delta );
	if (pPlayer->RecoilAction.IsDone() && pPlayer->GetWeapon()->GetNumAmmoInClip())
	{
		// Locate players within firing range of weapon and FOV.
        HUDSimulation::SimType enemyTeam = pPlayer->GetTeamID() == SimEntity::TEAM_Blue ? 
            HUDSimulation::SIM_RedPlayer : HUDSimulation::SIM_BluePlayer; 
		SimWeapon* pweapon = pPlayer->GetWeapon();
		float range = pweapon->GetFiringRange();
		Array<HUDSimulation::QueryResult> enemiesInRange;
		GetEntitiesInRadius(&enemiesInRange, 
                            enemyTeam,
						    pPlayer->GetPosition(), 						     
						    range);
		bool bFiredWeapon = false;
		for (unsigned i = 0; i < enemiesInRange.GetSize() && !bFiredWeapon; i++) {
			float relDirection = pPlayer->ComputeRelativeDirection((SimEntity*)enemiesInRange[i].pEntity);
			//Player can only hit one person at a time.
			if (relDirection < 25 && relDirection > -25){
					FireWeaponAt(pPlayer, (SimPlayerEntity*)enemiesInRange[i].pEntity);		
					bFiredWeapon = true;
			}
		}
		//If we weren't able to find a player, we missed.  Decrement ammo.
		if (!bFiredWeapon){
			pweapon->DecrementAmmo();
		}
		pPlayer->RecoilAction = SimValueAction(0.f, pweapon->GetRecoilDuration());
        FireEvent(Event(Event::EVT_PlayerFire, pPlayer)); // Fire a PlayerFire event.  Will affect UI reticule.
	}
}

//
// Control for user player throwing grenade.  Unique in that a player never 
// has to switch to their grenade weapon.  Instead, they can throw grenades with other
// weapons equipped. 
//
void HUDSimulation::PlayerThrowGrenade() 
{
    SimWeapon::WeaponType lastWeapon = pPlayer->GetWeaponType();
	pPlayer->ChangeWeapon(SimWeapon::WEAP_Grenade);
    PlayerFireWeapon( 0 );
	pPlayer->GetWeapon()->Reload();
	pPlayer->ChangeWeapon((SimWeapon::WeaponType)lastWeapon);
}

//
// Control for reloading user player's weapon.
//
void HUDSimulation::PlayerReloadWeapon()
{
	if (!pPlayer->GetWeapon()->Reload())
	{
		//Fire insufficient ammo event to display "Out of Ammo" indicator.  Currently unimplemented.
	}
}

//
// Translator for Rank into String format.
//
const char*     HUDSimulation::GetRankString(SimPlayerEntity::Rank rank) const 
{
    switch (rank)
    {
    case SimPlayerEntity::RANK_Private:          { return "Private"; } break;
    case SimPlayerEntity::RANK_Pvt_1st_Class:    { return "Private 1st Class"; } break;
    case SimPlayerEntity::RANK_Corporal:         { return "Corporal"; } break;
    case SimPlayerEntity::RANK_Sergeant:         { return "Sergeant"; } break;
    case SimPlayerEntity::RANK_Staff_Sgt:        { return "Staff Sergeant"; } break;
    case SimPlayerEntity::RANK_Sgt_1st_Class:    { return "Sergeant 1st Class"; } break;
    case SimPlayerEntity::RANK_Master_Sgt:       { return "Master Sergeant"; } break;
    case SimPlayerEntity::RANK_Sgt_Major:        { return "Sgt. Major"; } break;
    case SimPlayerEntity::RANK_2nd_Lieutenant:   { return "2nd Lieutenant"; } break;
    case SimPlayerEntity::RANK_1st_Lieutenant:   { return "1st Lieutenant"; } break;
    case SimPlayerEntity::RANK_Captain:          { return "Captain"; } break;
    case SimPlayerEntity::RANK_Major:            { return "Major"; } break;
    case SimPlayerEntity::RANK_Lt_Colonel:       { return "Lt. Colonel"; } break;
    case SimPlayerEntity::RANK_Colonel:          { return "Colonel"; } break;
    case SimPlayerEntity::RANK_Brig_General:     { return "Brigadier General"; } break;
    case SimPlayerEntity::RANK_Major_General:    { return "Major General"; } break;
    case SimPlayerEntity::RANK_Lt_General:       { return "Lt. General"; } break;
    case SimPlayerEntity::RANK_General:          { return "General"; } break;
	default:									 break;
    }
    return "UNKNOWN";
}



