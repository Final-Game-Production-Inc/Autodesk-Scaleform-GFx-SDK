/**************************************************************************

Filename    :   HUDSimulation.h
Content     :   HUD simulation types
Created     :   December 4, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_HUDSimluation_H
#define INC_HUDSimluation_H

#include "Render/Render_Types2D.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_String.h"

#include "HUDEntityController.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;

//
// The following types implement a demo environment to drive the 
// Game UI. This environment contains players from two teams in 
// a capture and hold game type. The NPCs are driven by action blocks 
// to produce primitive behavior. The user is also able to move around
// and interact in the simulation space.
//


// Helper to retrieve a random float in a certain range
float SF_GetRandomfloat(float min, float max);
#define SF_RANDF(min, max)     SF_GetRandomfloat(min, max)

// Time (from Platform GetTicks()) is in micro-seconds
#define TIME_UNITS 1000000.f

// Forward declaration
class SimMinimapIconAdapter;
class SimEnvironmentAdapter;
class HUDSimulation;


//
// The static game entity for this demo. Capture points (flags), 
// objectives and waypoints are of this type.
//
class SimEntity : public NewOverrideBase<Stat_Default_Mem>
{
public: 
    enum Team
    {
        TEAM_Neutral = 0,
        TEAM_Blue,
        TEAM_Red,

        TEAM_Count,
    };

    SimEntity(SimEntityController* pcont);
    virtual ~SimEntity();

    // Accessors for entity state
    virtual const PointF&  GetPosition() const             { return Position; }
    virtual void            SetPosition(const PointF& p)   {   Position = p;   }
    virtual PointF         GetNextPosition() const         { return Position; }
    virtual float           GetDirection() const            { return 0.f; }   // Degrees

    // Method to update the entity state.
    void            Update(float delta, HUDSimulation* psim);

    // Check if another entity is in a certain range
    float                   GetDistanceSquared(const SimEntity* pentity) const
    {
        return Position.DistanceSquared(pentity->GetPosition());
    }
    bool                    DistanceLessThan(const SimEntity* pentity, float dist) const
    {
        float distSq = dist * dist;
        return Position.DistanceSquared(pentity->GetPosition()) < distSq;
    }

protected:
    SimEntityController*    pController;

    PointF                 Position;
};

class SimItem : public NewOverrideBase<Stat_Default_Mem>
{
public:
	enum Type
	{
		ITEM_MachineGun,
		ITEM_Pistol,
		ITEM_Armor,
		ITEM_Grenade,
		ITEM_Medkit,
		ITEM_Molotov,

		NUM_TYPES,
	};

	SimItem(Type item) : ItemType(item) {} 
	virtual ~SimItem() {}

	Type				GetItemType() const		{ return ItemType; }
	virtual String		GetName() const			= 0;
	virtual bool		IsUsable() const		= 0;

protected:
	Type    ItemType;
private:

};

//
// Base weapon entity definition for the demo. Game currently supports pistol
// machine gun, and grenade weapons.  Ammo for a weapon is either loaded in the
// gun or in the player's remaining ammo.
//
class SimWeapon : public SimItem
{
public:
    enum WeaponType
    {
        WEAP_Pistol,
        WEAP_MachineGun,
        WEAP_Grenade,

        NUM_TYPES,
    };

	SimWeapon(Type itemType, WeaponType weapType) : SimItem(itemType), LoadedAmmo(0), RemainingAmmo(0), WeapType(weapType) {}
    virtual ~SimWeapon() {}

	// Weapon ammo accessors, mutators.
    bool    HasAmmo() const             { return (LoadedAmmo + RemainingAmmo) > 0; }
	unsigned	GetTotalAmmo()				{ return LoadedAmmo + RemainingAmmo; }
    void    AddAmmo(unsigned a)             { RemainingAmmo = Alg::Min(GetMaxClips() * GetMaxAmmoInClip(), RemainingAmmo + a); }
    void    DecrementAmmo()             { if (LoadedAmmo > 0) --LoadedAmmo; }
    void    SetMaxAmmo()                { RemainingAmmo = GetMaxClips() * GetMaxAmmoInClip(); }
	bool	Reload()					
	{ 
		if (RemainingAmmo > 0) 
		{
			unsigned MissingAmmo = (GetMaxAmmoInClip() - LoadedAmmo);
			if (RemainingAmmo <= MissingAmmo)	// If we can't fully reload the clip, load all RemainingAmmo.
			{		
				LoadedAmmo += RemainingAmmo; 
				RemainingAmmo = 0;
				return true;
			}
			else	// If we can properly reload, refill LoadedAmmo and decrement RemainingAmmo.				
			{								
				LoadedAmmo += MissingAmmo;
				RemainingAmmo -= MissingAmmo;
				return true;
			}
		}
		else
			return false;
	}		

	// Return the number of clips of ammo remaining for a weapon. Returns 1 if there is
	// less than a full clip of ammo but the player still has remaining ammo that can be
	// loaded.
    unsigned    GetNumClips() const  
	{ 
		if (RemainingAmmo / GetMaxAmmoInClip() > 0) 
			return RemainingAmmo / GetMaxAmmoInClip();
		else
			if (RemainingAmmo >= 1) return 1;
			else return 0;
	}

    unsigned    GetNumAmmoInClip() const    { return LoadedAmmo; }

	// Virtual methods to be overriden by weapon classes.
    virtual float   GetAOERange() const         = 0;	// Radius of weapon splash damage.
    virtual float   GetFiringRange() const      = 0;    // Maximum distance from source to target for hit.
    virtual float   GetDamageCoeff() const      = 0;	// Damage coefficient of weapon hit
    virtual float   GetRecoilDuration() const   = 0;    // Time player must wait before firing weapon again.
    virtual unsigned    GetMaxClips() const         = 0;	// Maximum number of clips player can hold for weapon.
    virtual unsigned    GetMaxAmmoInClip() const    = 0;	// Weapon's clip size

	bool	IsUsable() const	{ return false; }

private:
	unsigned	LoadedAmmo;
	unsigned	RemainingAmmo;
	WeaponType	WeapType;
};



//
// Base capture point entity for the demo.  Capture points always belong to a team. 
// Their captured state designates which team they currently belong to.
// In the game, players entities can capture these entities by staying within range
// of a capture point entity until they have shifted the capture state in their
// team's favor.
//
class SimCapturePointEntity : public SimEntity
{
public:
    SimCapturePointEntity(SimCapturePointController* pcont, SimEntity::Team teamid)
        : SimEntity(pcont), TeamID(teamid) {}

	// Accessor, mutator for the capture point's controlling team.
    SimEntity::Team     GetTeamID() const                   { return TeamID; }
    void                SetTeamID(SimEntity::Team teamid)   { TeamID = teamid; }

	// Accessor, mutator for captured state.
	void				SetCapturedState(float capState) { CapturedState = capState; }
	float				GetCapturedState()				 { return CapturedState; }

protected:
    SimEntity::Team     TeamID;
	float				CapturedState;
};


//
// Base powerup entity for the demo.  Powerups supply a health and ammo boost.
// The amount of health and the type of ammo received are generated
// randomly when the powerup is (re)spawned.
//
class SimPowerupEntity : public SimEntity
{
public:
    SimPowerupEntity(SimPowerupController* pcont)
        : SimEntity(pcont)
    {
        GenerateRandomPowerup();
    }

    void            Respawn(const PointF& respawnPoint)
    {
        SimEntity::SetPosition(respawnPoint);
        GenerateRandomPowerup();
    }

	// Accessors for powerup bonuses.
    float                   GetHealthBoost() const      { return HealthBoost; }
    SimWeapon::WeaponType	GetWeaponBoostType() const  { return WeaponType; }
    unsigned                    GetAmmoBoost() const        { return AmmoBoost; }

protected:
    float                   HealthBoost;
    SimWeapon::WeaponType			WeaponType;
    unsigned                    AmmoBoost;

    void					GenerateRandomPowerup();
};



//
// A dynamic/movable player entity for this demo. The user is of
// this type, as well as the bots.
//
class SimPlayerEntity: public SimEntity
{
public:
	
	static const int INVENTORY_SIZE = 28;
	static const int WEAPON_SLOTS = 4;
	static const int ITEM_SLOTS = INVENTORY_SIZE - WEAPON_SLOTS;
	static const int WEAPON_SLOT_1 = 24;
	static const int WEAPON_SLOT_2 = 25;
	static const int WEAPON_SLOT_3 = 26;
	static const int WEAPON_SLOT_4 = 27;

	// All players are of some rank, 1-18 (where 18 is the highest rank).
    enum Rank
    {
        RANK_Private,
        RANK_Pvt_1st_Class,
        RANK_Corporal,
        RANK_Sergeant,
        RANK_Staff_Sgt,
        RANK_Sgt_1st_Class,
        RANK_Master_Sgt,
        RANK_Sgt_Major,
        RANK_2nd_Lieutenant,
        RANK_1st_Lieutenant,
        RANK_Captain,
        RANK_Major,
        RANK_Lt_Colonel,
        RANK_Colonel,
        RANK_Brig_General,
        RANK_Major_General,
        RANK_Lt_General,
        RANK_General,

        NUM_RANKS,
    };

    SimPlayerEntity(SimPlayerController* pcont, const char* pname, SimEntity::Team team);
    virtual ~SimPlayerEntity();

    SimPlayerController*    GetPlayerController() const   { return (SimPlayerController*)pController; }
    void                    SetPlayerController(SimPlayerController* pcont)
    {
        if (pController) delete pController;
        pController = pcont;
    }

    // Accessors for player properties
    const String&          GetName() const             { return Name; }
    void                    SetName(const char* pname)  { Name = pname; }
    SimEntity::Team         GetTeamID() const			{ return TeamID; }    

    float                   ComputeRelativeDirection(SimEntity* pent); // Returns degrees

    // Accessors for player location and direction
    void            SetPosition(const PointF& p)
    {
        // Set Initial health and ammo counts
        Respawn(p);
    }
    float                   GetDirection() const;   // Degrees; override from SimEntity
    void                    SetDirection(float a);  // Degrees

    // Special method that returns the player position after the next update
    virtual PointF         GetNextPosition() const;

    // Health accessors
    float                   GetHealth() const       { return Health; }
    bool                    IsAlive() const         { return Health > 0.f; }
    bool                    IsDead() const          { return Health <= 0.f; }
    void                    AddHealthBoost(float h) { Health = Alg::Min(1.f, Health + h); }
    void                    ApplyDamage(float dmg)  { Health = Alg::Max(0.f, Health - dmg); }		

    void                    Respawn(const PointF& respawnPoint);

    // Weapon, ammo accessors, mutators.
    SimWeapon*              GetWeapon() const                       { return Weapons[CurrentWeapon]; }
	bool					ChangeWeapon(SimWeapon::WeaponType w);
	bool					ChangeWeaponByIndex(unsigned index);
    SimWeapon::WeaponType   GetWeaponType() const                   { return CurrentWeapon; }
    void                    SetWeaponType(SimWeapon::WeaponType w)        { CurrentWeapon = w; }
    void                    AddAmmo(SimWeapon::WeaponType w, unsigned count)  { Weapons[w]->AddAmmo(count); }
    unsigned                    GetNumAmmoInClip() const                { return GetWeapon()->GetNumAmmoInClip(); }
    unsigned                    GetNumClips() const                     { return GetWeapon()->GetNumClips(); }
    unsigned                    GetMaxAmmoInClip() const                { return GetWeapon()->GetMaxAmmoInClip(); }
	unsigned				    GetNumGrenades() const					{ return Weapons[SimWeapon::WEAP_Grenade]->GetTotalAmmo(); }

	// Inventory accessors, mutators.
	SimItem*    			GetInventoryItem(unsigned index) const	{ return Inventory[index]; }
	void					ClearInventory();
	bool					StoreItem(SimItem* item);
	unsigned					HasInventorySpace();
	bool					SwapInventorySlot();

	bool					CycleWeapons();

    // Kill count accessors
    unsigned                    GetKillStreak() const   { return KillStreak; }
    void                    IncrementKillStreak()   { KillStreak++; }
    void                    ResetKillStreak()       { KillStreak = 0; }

    // Accessors for movement and rotation deltas. If the deltas 
    // != 0, then this entities position and/or movement will change
    // over time.
    void                    SetMovement(float dm)           { ActiveMovement = dm; }
    void                    SetRotation(float da)           { ActiveRotation = da; }
    void                    SetMovementMultiplier(float m)  { MovementMultiplier = m; }

    // Method to update the entity state
    virtual void            Update(float delta, HUDSimulation* psim);

    // Rank and experience points accessors
    bool                    AddToXP(float xp);  // Returns true if level up
    unsigned                    GetRank() const     { return CurrentRank; }
    float                   GetLevelXP() const  { return LevelXP; }

	SimValueAction          RecoilAction;

	// CapturePointEntity accessors, mutators.  Changed when in range of a capture point.
	SimCapturePointEntity*	GetCapturingTarget() const { return CapturingTarget; }
	void					SetCapturingTarget(SimCapturePointEntity* pcap) { CapturingTarget = pcap; } 

protected:
    String                 Name;
    SimEntity::Team         TeamID;
    PointF                 Direction;  // Normal towards direction
    float                   Health;
    unsigned                    KillStreak;

	SimItem* 				Inventory [INVENTORY_SIZE];

	unsigned					CurrentWeaponSlot;
    SimWeapon::WeaponType   CurrentWeapon;
    SimWeapon*              Weapons[SimWeapon::NUM_TYPES];
    
    float                   LevelXP;
    Rank                    CurrentRank;

    float                   ActiveMovement;
    float                   ActiveRotation;

    float                   MovementMultiplier;
	
	SimCapturePointEntity*	CapturingTarget;
};



//
// The custom environment for this demo
//
class HUDSimulation
{
public:
    enum SimType
    {
        SIM_Unknown,
        SIM_BluePlayer,
        SIM_RedPlayer,        
        SIM_CapturePoint,
        SIM_Powerup,
    };

    // Simulation runtime settings
    struct Settings
    {
        unsigned            NumBluePlayers;
        unsigned            NumRedPlayers;
        unsigned            NumCapturePoints;
        unsigned            NumPowerups;
    };

    // Simulation locations
    struct Locations
    {
        Array<PointF> BlueSpawnPoints;
        Array<PointF> RedSpawnPoints;
    };

    // Simulation configuration properties
    struct Config
    {
        RectF          PhysicalMapDimensions;
        // Inbounds is typically smaller than the PhysicalMapDimensions
        RectF          InboundsDimensions; 
        Settings        InitialSettings;
        Locations       InitialLocations;
    };

    struct QueryResult
    {
        SimEntity*      pEntity;
        float           DistanceSquared;
    };

    struct FrustumResult
    {
        SimEntity*      pEntity;
        float           DistanceSquared;
        float           Angle;
    };

    HUDSimulation(const Config& config);
    virtual ~HUDSimulation();

    // Accessors and control methods for the simulation properties
    inline bool     IsRunning() const       { return bEnabled; }
    inline void     Start()                 { bEnabled = true; }
    inline void     Stop()                  { bEnabled = false; }
    void            Restart();	

    // Method to tick the simulation (ticks are expected to be in micro seconds)
    void            Update(UInt64 ticks);
	UInt64			GetLastTicks() const	{ return LastTicks; }
	
	virtual float   GetSecondsLeftInRound() const	{ return SecondsLeftInRound; }
   
    // Query method to retrieve sets of entities of a specific type in 
    // the view range.
    void            GetEntitiesInRadius(Array<QueryResult>* presult, SimType type,
                                        const PointF& physCenter, float physicalRadius,
                                        SimEntity* pignore = NULL);

    void            GetEntitiesInViewFrustum(Array<FrustumResult>* presult, SimType type, float radius);

    // Accessors for the physical environment properties
    SimPlayerEntity*        GetPlayer() const               { return pPlayer; }
    SimPlayerController*    GetPlayerController() const     { return pPlayerController; }
    const RectF&           GetMapBounds() const			{ return MapDimensions; }
    const PointF&          GetPlayerPosition() const		{ return pPlayer->GetPosition(); }
    float                   GetPlayerDirection() const		{ return pPlayer->GetDirection(); }

    float                   GetBlueTeamScore() const		{ return GetTeamScore(SimEntity::TEAM_Blue); }
    float                   GetRedTeamScore() const			{ return GetTeamScore(SimEntity::TEAM_Red); }

    const RectF&               GetInboundsDimensions() const   { return InboundsDimensions; }
    const Array<SimEntity*>&   GetCapturePoints() const        { return CapturePointEntities; }

    // Settings access
    const Settings& GetRuntimeSettings() const			{ return RuntimeSettings; }
    void            SetNumBluePlayers(unsigned newVal);
    void            SetNumRedPlayers(unsigned newVal);
    void            SetNumCapturePoints(unsigned newVal);
    void            SetNumPowerups(unsigned newVal);

    void            CaptureStateChange(SimCapturePointEntity* psrc);

	// Simulation events
    void            CaptureObjective(SimPlayerEntity* psrc, SimCapturePointEntity* pdest);
    void            ApplyPowerup(SimPlayerEntity* pplayer, SimPowerupEntity* ppup);
    bool            FireWeaponAt(SimPlayerEntity* psrc, SimPlayerEntity* pdest);
	void			GiveItem(SimPlayerEntity* pplayer, SimItem* pitem);
	SimItem*		GenerateRandomItem();

    // Team score access
	float			GetScoreToWin() const			{ return ScoreToWin; }
    void            AddToTeamScore(unsigned teamID, float val);
    float           GetTeamScore(unsigned teamID) const;
	void			ResetTeamScores();
    void            SwitchUserController();

    // User control interface
    void            PlayerMovingForward(bool flag);
    void            PlayerMovingBackward(bool flag);
    void            PlayerTurningLeft(bool flag);
    void            PlayerTurningRight(bool flag);
	void			PlayerShooting(bool flag);
	void			PlayerFireWeapon(float delta);
	void			PlayerThrowGrenade();
	void			PlayerReloadWeapon();

    float           GetFOVAngle() const     { return 45.f; }

    const char*     GetRankString(SimPlayerEntity::Rank rank) const;

    
    ///// Event listener interface

    struct Event
    {
    public:
        enum Type
        {
            EVT_Damage,					// Player takes damage
            EVT_Kill,					// PlayerEntity is killed
            EVT_Powerup,				// Player picks up powerup
            EVT_LevelUp,				// Player's rank increases
            EVT_PlayerCapture,			// Player captures CaptureEntity
            EVT_CaptureStateChange,		// CaptureEntity's CaptureState changes
            EVT_KillStreak,				// Player adds a kill to a ongoing kill streak
            EVT_RoundOver,				// Round time expired or max score reached
            EVT_PlayerFire,				// Player fires weapon
            EVT_AddEntity,				// An entity is added
            EVT_RemoveEntity,			// An entity is removed
        };
        Type                EventType;
        SimEntity*          pSource;
        SimEntity*          pDest;
        float               Data;
        Event(Type type, SimEntity* psrc = NULL, SimEntity* pdest = NULL, float data = 0.f)
            : EventType(type), pSource(psrc), pDest(pdest), Data(data) {}
    };

    class   EventListener
    {
    public:
		virtual ~EventListener() {} 
        virtual void    OnSimulationEvent(const Event& evt) = 0;
    };
    Array<EventListener*>     EventListeners;

    void    AddEventListener(EventListener* pevl)  { EventListeners.PushBack(pevl); }


private:
    // Physical map dimensions
    RectF          MapDimensions;
    // Valid area for dynamic entities. This is smaller than the map dimensions.
    RectF          InboundsDimensions;
    // Runtime settings
    Settings        RuntimeSettings;
    // Simulation locations
    Locations       RuntimeLocations;

    SimPlayerEntity*            pPlayer;
    SimPlayerController*        pPlayerController;

    // Entity storage
    Array<SimEntity*>      BluePlayerEntities;
    Array<SimEntity*>      RedPlayerEntities;
    Array<SimEntity*>      CapturePointEntities;
    Array<SimEntity*>      PowerupEntities;

    // Capture and Hold data
    float           TeamScore[SimEntity::TEAM_Count];   // For team0 and team1
	float			ScoreToWin;

    // Simulation flags
    UInt64          LastTicks;
    float           SecondsLeftInRound;
    bool            bFirstUpdate;
    bool            bEnabled;

	bool			bPaused;
    bool            bUserBotControlled;

    void            Respawn(SimPlayerEntity* pplayer);
    PointF         GetRandomInboundsPoint() const;
    
	// Fire weapon logic for a weapon that only hits one target (pistol, machine gun).
    bool            FireNoAOEWeaponAt(SimPlayerEntity* psrc, SimPlayerEntity* pdest);

	// Fire weapon logic for a weapon that can hit multiple targets in a radius (grenade).
    bool            FireAOEWeaponAt(SimPlayerEntity* psrc, const PointF& dest);

	// Pass Simulation events on to all registered listeners.
    void            FireEvent(const Event& type)
    {
        for (unsigned i=0; i < EventListeners.GetSize(); i++)
            EventListeners[i]->OnSimulationEvent(type);
    }
};



#endif  // INC_HUDSimluation_H
