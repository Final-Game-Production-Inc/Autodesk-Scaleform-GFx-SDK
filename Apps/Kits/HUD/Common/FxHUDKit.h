/**************************************************************************

Filename    :   FxHUDKit.h
Content     :   HUD kit specific types
Created     :   November 13, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxHUDKit_H
#define INC_FxHUDKit_H

#include "FxGameDelegate.h"
#include "FxMinimap.h"

//
//
//
class FxHUDItem
{
public:
	enum Item
	{
		ITEM_MachineGun,
		ITEM_Pistol,
		ITEM_Armor,
		ITEM_Grenade,
		ITEM_Medkit,
		ITEM_Moltov
	};

	virtual ~FxHUDItem();

	virtual String		GetName() const			= 0;
	virtual unsigned		GetIcon() const			= 0;
	virtual bool		IsEquipped() const		= 0;
	virtual bool		IsUsable() const		= 0;
};

// 
// 
//
class FxHUDPlayer
{
public:
    virtual ~FxHUDPlayer() {}

    enum Weapon
    {
        WEAP_Pistol,
        WEAP_MachineGun,
        WEAP_Grenade,
    };

    virtual const String&  GetName() const = 0;
    virtual float           GetHealth() const = 0;
    virtual unsigned            GetRank() const = 0;
    virtual float           GetLevelXP() const = 0;

    virtual Weapon          GetWeaponType() const = 0;
    virtual unsigned            GetNumAmmoInClip() const = 0;
    virtual unsigned            GetNumClips() const = 0;
    virtual unsigned            GetMaxAmmoInClip() const = 0;
	virtual unsigned			GetNumGrenades() const = 0;
};

//////////////////////////////////////////////////////////////////////////

//
// Events are fired from the simulation and caught by the HUD and Minimap views.
// They are processed to display/hide/animate elements of the user interface. 
//
class FxHUDEvent
{
public:
    enum Type
    {
        EVT_Damage,				// Player takes damage
        EVT_Kill,				// PlayerEntity is killed
        EVT_Powerup,			// Player picks up powerup
        EVT_LevelUp,			// Player's rank increases
        EVT_Capture,			// Capture entity is captured
		EVT_KillStreak,			// Player adds a kill to a ongoing kill streak
		EVT_RoundOver,			// Round time expired or max score reached
		EVT_PlayerFire			// Player fires weapon
    };

    FxHUDEvent(Type evt) : EventType(evt) {}
    Type    GetType() const         { return EventType; }

protected:
    Type    EventType;
};

//
// Event class definitions.
//
class FxHUDDamageEvent : public FxHUDEvent
{
public:
    FxHUDDamageEvent(float direction, float damage) : FxHUDEvent(EVT_Damage),
        Direction(direction), Damage(damage) {}

    float       GetDirection() const    { return Direction; }
    float       GetDamage() const       { return Damage; }

protected:
    float       Direction;
    float       Damage;
};

class FxHUDKillEvent : public FxHUDEvent
{
public:
    FxHUDKillEvent(const char* psrcName, unsigned srcRank, const char* pdestName, unsigned destRank, FxHUDPlayer::Weapon weap) 
        : FxHUDEvent(EVT_Kill), SrcName(psrcName), DestName(pdestName), SrcRank(srcRank), DestRank(destRank), WeaponType(weap) {}

    const String&          GetSrcName() const      { return SrcName; }
    const String&          GetDestName() const     { return DestName; }
    unsigned                    GetSrcRank() const      { return SrcRank; }
    unsigned                    GetDestRank() const     { return DestRank; }
    FxHUDPlayer::Weapon     GetWeaponType() const   { return WeaponType; }

protected:
    String                 SrcName;
    String                 DestName;
    unsigned                    SrcRank;
    unsigned                    DestRank;
    FxHUDPlayer::Weapon     WeaponType;
};

class FxHUDPowerupEvent : public FxHUDEvent
{
public:
    FxHUDPowerupEvent() : FxHUDEvent(EVT_Powerup) {}
};

class FxHUDPlayerFireEvent : public FxHUDEvent
{
public:
	FxHUDPlayerFireEvent(FxHUDPlayer::Weapon weap) : FxHUDEvent(EVT_PlayerFire), WeaponType(weap) {}

	FxHUDPlayer::Weapon     GetWeaponType() const   { return WeaponType; }

protected:
	FxHUDPlayer::Weapon     WeaponType;
};

class FxHUDLevelUpEvent : public FxHUDEvent
{
public:
    FxHUDLevelUpEvent(unsigned newRank, const char* newRankName) 
        : FxHUDEvent(EVT_LevelUp), NewRank(newRank), NewRankName(newRankName) {}

    const String&      GetNewRankName() const      { return NewRankName; }
    unsigned                GetNewRank() const          { return NewRank; }

protected:
	unsigned            NewRank;
    String         NewRankName;
};

class FxHUDKillStreakEvent : public FxHUDEvent
{
public:
    FxHUDKillStreakEvent(const char* psrcName, const unsigned ks) : FxHUDEvent(EVT_KillStreak), SrcName(psrcName), SrcKillStreak(ks) {}

    const String&          GetSrcName() const      { return SrcName; }
	unsigned                    GetSrcKillStreak() const		{ return SrcKillStreak; }

protected:
    String                 SrcName;
    unsigned                    SrcKillStreak;
};

class FxHUDCaptureEvent : public FxHUDEvent
{
public:
    FxHUDCaptureEvent(const char* psrcName, bool isblueTeam, bool isuser) 
        : FxHUDEvent(EVT_Capture), SrcName(psrcName), bBlueTeam(isblueTeam), bUserPlayer(isuser) {}

    const String&      GetSrcName() const      { return SrcName; }
    bool                IsBlueTeam() const      { return bBlueTeam; }
    bool                IsUserPlayer() const    { return bUserPlayer; }

protected:
    String         SrcName;
    bool            bBlueTeam;
    bool            bUserPlayer;
};

//////////////////////////////////////////////////////////////////////////

//
// FxHUDEvent listener interface.
//
class FxHUDEventListener
{
public:
	virtual ~FxHUDEventListener() {};
    virtual void    OnEvent(FxHUDEvent* pevent) = 0;
};

//////////////////////////////////////////////////////////////////////////

//
// FxHUDEnvironment interface.
//
class FxHUDEnvironment : public FxMinimapEnvironment
{
public:
    // Query result for GetPlayersInViewFrustum method.
    struct BillboardQueryResult
    {
        FxHUDPlayer*        pPlayer;
        // Distance squared is expected; avoids unnecessary sqrt()
        float               X;
        float               Y;
        float               Scale;
        bool                bShowLabel;
    };

    virtual ~FxHUDEnvironment() {}

	virtual bool     IsRunning() const = 0;

	// Accessor for retrieving the HUD Player
    virtual FxHUDPlayer*    GetHUDPlayer() const = 0;

    virtual bool            IsHUDPlayerCapturingObjective() const = 0;
    virtual float           GetHUDCaptureObjectiveState() const = 0;

	// Accessors for simulation data
    virtual float           GetHUDBlueTeamScore() const = 0;
    virtual float           GetHUDRedTeamScore() const = 0;

    virtual float           GetSecondsLeftInRound() const = 0;

    // Query methods to retrieve the set of players in a certain circular 
    // view range - based on radial distance from the player position.
    virtual void            GetFriendliesInViewFrustum(Array<BillboardQueryResult>* result) const = 0;
    virtual void            GetEnemiesInViewFrustum(Array<BillboardQueryResult>* result) const = 0;

	// Register new FxHUDEvent listener
    void                    RegisterHUDEventListener(FxHUDEventListener* plistener)
    {
        HUDEventListeners.PushBack(plistener);
    }

	// Pass FxHUDEvent to listeners for processing
    void                    FireEvent(FxHUDEvent* pevent)
    {
        for (unsigned i=0; i < HUDEventListeners.GetSize(); i++)
            HUDEventListeners[i]->OnEvent(pevent);
    }

protected:
    Array<FxHUDEventListener*>     HUDEventListeners;
};


#endif  // INC_FxHUDKit_H
