/**************************************************************************

Filename    :   HUDAdapter.h
Content     :   Adapter implementation to further decouple Simulation and 
				HUD/Minimap views.
Created     :   December 22, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_HUDAdapter_H
#define INC_HUDAdapter_H

#include "Kernel/SF_Hash.h"
#include "HUDSimulation.h"
#include "../Common/FxHUDKit.h"
#include "../Common/FxMinimapView.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;

// Forward declaration
class   HUDEntityAdapter;

//
// Adapter implementation to further decouple Simulation and HUD/Minimap views.
// Classes below expose interfaces and translate them for HUD and Simulation 
// communication. Only necessary accessors are implemented here.
//

//
//  Adapter for Player Entity that exposes FxHUDPlayer interface.
//
class HUDPlayerAdapter : public NewOverrideBase<Stat_Default_Mem>, public FxHUDPlayer
{
public:
    HUDPlayerAdapter() : pPlayer(NULL) {}
    void    SetPlayerEntity(SimPlayerEntity* pplayer)   { pPlayer = pplayer; }

    virtual const String&  GetName() const         { return pPlayer->GetName(); }
    virtual float           GetHealth() const       { return pPlayer->GetHealth(); }
    virtual unsigned            GetRank() const         { return pPlayer->GetRank(); }
    virtual float           GetLevelXP() const      { return pPlayer->GetLevelXP(); }

    virtual Weapon          GetWeaponType() const       
    {
        switch (pPlayer->GetWeaponType())
        {
        case SimWeapon::WEAP_Pistol:        return FxHUDPlayer::WEAP_Pistol;
        case SimWeapon::WEAP_MachineGun:    return FxHUDPlayer::WEAP_MachineGun;
        case SimWeapon::WEAP_Grenade:       return FxHUDPlayer::WEAP_Grenade;
		default: break;
        }
        SF_ASSERT(0); // Unknown weapon
        return FxHUDPlayer::WEAP_Pistol;
    }
    virtual unsigned            GetNumAmmoInClip() const    { return pPlayer->GetNumAmmoInClip(); }
    virtual unsigned            GetNumClips() const         { return pPlayer->GetNumClips(); }
    virtual unsigned            GetMaxAmmoInClip() const    { return pPlayer->GetMaxAmmoInClip(); }
    virtual unsigned			GetNumGrenades() const      { return pPlayer->GetNumGrenades(); }

private:
    SimPlayerEntity*    pPlayer;
};

class HUDItemAdapter : public NewOverrideBase<Stat_Default_Mem>, public FxHUDItem
{
public:
	HUDItemAdapter() : pItem(NULL) {}
	void    SetItem(SimItem* pitem)   { pItem = pitem; }

	virtual String		GetName() const		{ return pItem->GetName(); }
	virtual bool		IsUsable() const	{ return pItem->IsUsable(); }
	virtual SimItem::Type GetItemType() const	{ return pItem->GetItemType(); }

private:
	SimItem*    pItem;
};

//
// Adapter for HUDSimulation that exposes the FxHUDEnvironment and FxMinimapEnvironment interfaces
//
class HUDEnvironmentAdapter : public FxHUDEnvironment, 
                              public HUDSimulation::EventListener
{
    typedef     Hash<SimEntity*, HUDEntityAdapter*>        EntityHash;
    typedef     Hash<SimPlayerEntity*, HUDPlayerAdapter*>  PlayerHash;

public:
    HUDEnvironmentAdapter() : pSimulation(NULL) {}
    virtual ~HUDEnvironmentAdapter();

    virtual FxHUDPlayer*    GetHUDPlayer() const            
    { 
        HUDPlayer.SetPlayerEntity(pSimulation->GetPlayer());
        return &HUDPlayer; 
    }

	virtual bool			IsRunning() const	{ return pSimulation->IsRunning(); }

    virtual bool            IsHUDPlayerCapturingObjective() const   { return pSimulation->GetPlayer()->GetCapturingTarget() != NULL; }
    virtual float           GetHUDCaptureObjectiveState() const     { return pSimulation->GetPlayer()->GetCapturingTarget()->GetCapturedState(); }

    virtual float           GetHUDBlueTeamScore() const             { return pSimulation->GetBlueTeamScore(); }
    virtual float           GetHUDRedTeamScore() const              { return pSimulation->GetRedTeamScore(); }

    virtual float           GetSecondsLeftInRound() const           { return pSimulation->GetSecondsLeftInRound(); }

    // Query method to retrieve the set of players in a certain circular 
    // view range - based on radial distance from the player position.
    virtual void            GetFriendliesInViewFrustum(Array<BillboardQueryResult>* presult) const;
    virtual void            GetEnemiesInViewFrustum(Array<BillboardQueryResult>* presult) const;

    // Query method to retrieve sets of entities of a specific type in 
    // the view range.
    virtual void            GetEntitiesInPhysicalRadius(Array<FxMinimapEnvironment::QueryResult>* result, 
                                                        FxMinimapEntity::IconType type,
                                                        const PointF& physCenter,  
                                                        float physicalRadius) const;

    // Accessors for the physical environment properties
    virtual const RectF&   GetPhysicalMapBounds() const      { return pSimulation->GetMapBounds(); }
    virtual const PointF&  GetPhysicalPlayerPosition() const { return pSimulation->GetPlayer()->GetPosition(); }
    virtual float           GetPlayerDirection() const        { return pSimulation->GetPlayer()->GetDirection(); }

    virtual void            OnSimulationEvent(const HUDSimulation::Event& evt);
    void                    SetSimulation(HUDSimulation* psim);

private:
    HUDSimulation*              pSimulation;

    mutable HUDPlayerAdapter    HUDPlayer;
    mutable EntityHash          EntityAdapters;
    mutable PlayerHash          PlayerAdapters;

    void                    ClearAllEntities();
};

#endif  // INC_HUDAdapter_H
