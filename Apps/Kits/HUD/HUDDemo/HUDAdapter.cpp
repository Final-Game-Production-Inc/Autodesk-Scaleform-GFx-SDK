/**************************************************************************

Filename    :   HUDAdapter.cpp
Content     :   Adapter implementation to further decouple Simulation and 
HUD/Minimap views.
Created     :   December 22, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "HUDAdapter.h"


//
// Adapter for SimEntity that exposes the FxMinimapEntity interface
//
class HUDEntityAdapter : public NewOverrideBase<Stat_Default_Mem>, public FxMinimapEntity
{
public:
    HUDEntityAdapter(SimEntity* pent) : pEntity(pent) {}

    // FxMinimapEntity implementation
    virtual const PointF&  GetPhysicalPosition() const { return pEntity->GetPosition(); }
    virtual float           GetDirection() const        { return pEntity->GetDirection(); }

private:
    SimEntity*          pEntity;
};


//////////////////////////////////////////////////////////////////////////


HUDEnvironmentAdapter::~HUDEnvironmentAdapter()
{
    ClearAllEntities();
}

//
// Mutator for HUDEnvironment Adapter's HUDSimulation.
//
void HUDEnvironmentAdapter::SetSimulation(HUDSimulation* psim)
{
    pSimulation = psim;
    pSimulation->AddEventListener(this);
}

//
// Query method to retrieve sets of entities of a specific type in 
// the view range.
//
void HUDEnvironmentAdapter::GetEntitiesInPhysicalRadius(Array<QueryResult>* result, 
                                                            FxMinimapEntity::IconType type,
                                                            const PointF& physCenter,  
                                                            float physicalRadius) const
{
    HUDSimulation::SimType  simType = HUDSimulation::SIM_Unknown;
    float newRadius = physicalRadius;
	// Translate "type" parameter from FxMinimapEntity type to HUDSimulation type.
    switch (type)
    {
    case FxMinimapEntity::ID_BluePlayer:  simType = HUDSimulation::SIM_BluePlayer; break;
    case FxMinimapEntity::ID_RedPlayer:   simType = HUDSimulation::SIM_RedPlayer; break;
    case FxMinimapEntity::ID_Flag:        
        simType = HUDSimulation::SIM_CapturePoint; 
        newRadius = physicalRadius * 4; 
        break;
    case FxMinimapEntity::ID_Objective:   
        simType = HUDSimulation::SIM_Powerup; 
        newRadius = physicalRadius * 2;
        break;
    default: 
		return;
    }

    Array<HUDSimulation::QueryResult>  entitiesInView;
    pSimulation->GetEntitiesInRadius(&entitiesInView, simType, physCenter, newRadius, pSimulation->GetPlayer());

    for (unsigned i=0; i < entitiesInView.GetSize(); i++)
    {
        QueryResult r;
        SimEntity* mment = entitiesInView[i].pEntity;
        HUDEntityAdapter* pia = NULL;
        HUDEntityAdapter** ppia = EntityAdapters.Get(mment);
        if (!ppia) 
        {
			// If no adapter exists for this entity, create one and add it to EntityAdapaters list.
            pia = new HUDEntityAdapter(mment);
            switch (simType)
            {
            case HUDSimulation::SIM_CapturePoint:
                {
					// If the entity is a Capture Point, set the Adapter's IconData to the entity's Team ID.
                    pia->SetIconData( ((SimCapturePointEntity*)mment)->GetTeamID() );
                } 
                break;
			default:
				break;
            }
            EntityAdapters.Add(mment, pia);
        }

		// If an adapter already exists for this entity, use it.
        else
            pia = *ppia;

		// Add the adapter + data to the query's result.
        r.pEntity = pia;
        r.DistanceSquared = entitiesInView[i].DistanceSquared;
        result->PushBack(r);
    }
}

//
// Deletes all Player and Entity Adapters and clears lists.
//
void    HUDEnvironmentAdapter::ClearAllEntities()
{
    for (EntityHash::Iterator iter=EntityAdapters.Begin(); iter != EntityAdapters.End(); ++iter )
    {
        delete (*iter).Second;
    }
    EntityAdapters.Clear();
    for (PlayerHash::Iterator iter=PlayerAdapters.Begin(); iter != PlayerAdapters.End(); ++iter)
    {
        delete (*iter).Second;
    }
    PlayerAdapters.Clear();
}

//
// Process and/or forward HUDSimulation events.
//
void HUDEnvironmentAdapter::OnSimulationEvent(const HUDSimulation::Event& evt)
{
    switch(evt.EventType)
    {
    case HUDSimulation::Event::EVT_RemoveEntity: 
        {
			// Remove entity adapter and its references.
			// Primarily used when number of players/capture points/powerups changes.
            HUDEntityAdapter** ppia = EntityAdapters.Get(evt.pSource);
            if (ppia)
            {
                HUDEntityAdapter* pia = *ppia;
                if (pia->GetIconRef())
                    pia->GetIconRef()->ClearOwner();
                pia->SetIconRef(NULL);
                delete pia;
                EntityAdapters.Remove(evt.pSource);
            }
        }
        break;
    case HUDSimulation::Event::EVT_Damage:
        {
            SimPlayerEntity* pdest = (SimPlayerEntity*)evt.pDest;
            FxHUDDamageEvent e(pdest->ComputeRelativeDirection(evt.pSource), evt.Data);
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_Kill:
        {
            SimPlayerEntity* psrc = (SimPlayerEntity*)evt.pSource;
            SimPlayerEntity* pdest = (SimPlayerEntity*)evt.pDest;
            FxHUDKillEvent e(psrc->GetName(), psrc->GetRank(), 
                pdest->GetName(), pdest->GetRank(), 
                (FxHUDPlayer::Weapon)psrc->GetWeaponType());
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_Powerup:
        {
            FxHUDPowerupEvent e;
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_LevelUp:
        {
            SimPlayerEntity* psrc = (SimPlayerEntity*)evt.pSource;
            FxHUDLevelUpEvent e(psrc->GetRank(), 
                pSimulation->GetRankString((SimPlayerEntity::Rank)psrc->GetRank()));
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_PlayerCapture:
        {
            SimPlayerEntity* psrc = (SimPlayerEntity*)evt.pSource;
            FxHUDCaptureEvent e(psrc->GetName(), 
                psrc->GetTeamID() == SimEntity::TEAM_Blue, 
                evt.pSource == pSimulation->GetPlayer());
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_CaptureStateChange:
        {
            SimCapturePointEntity* psrc = (SimCapturePointEntity*)evt.pSource;
            HUDEntityAdapter** ppia = EntityAdapters.Get(psrc);
            if (ppia)
                (*ppia)->SetIconData( psrc->GetTeamID() );
        }
        break;
    case HUDSimulation::Event::EVT_KillStreak:
        {
            SimPlayerEntity* psrc = (SimPlayerEntity*)evt.pSource;
            FxHUDKillStreakEvent e(psrc->GetName(), psrc->GetKillStreak());
            FireEvent(&e);
        }
        break;
    case HUDSimulation::Event::EVT_RoundOver:
        {
            // TODO: Unimplemented.
        }
        break;
    case HUDSimulation::Event::EVT_PlayerFire:
        {
            SimPlayerEntity* psrc = (SimPlayerEntity*)evt.pSource;
            HUDPlayerAdapter pa;
            pa.SetPlayerEntity(psrc);
            FxHUDPlayerFireEvent e(pa.GetWeaponType());
            FireEvent(&e);
        }
        break;
	default: 
		break;
    }
}

//
// Query method to retrieve set of friendlies in view frustum for display using billboards.
//
void HUDEnvironmentAdapter::GetFriendliesInViewFrustum(Array<BillboardQueryResult>* presult) const
{
    Array<HUDSimulation::FrustumResult> fr;
    pSimulation->GetEntitiesInViewFrustum(&fr, 
		((HUDSimulation::SimType)pSimulation->GetPlayer()->GetTeamID() == HUDSimulation::SIM_BluePlayer) ? 
        HUDSimulation::SIM_BluePlayer : HUDSimulation::SIM_RedPlayer, 5.f);
    float xextent = 1280.f/2.f;
    float fov = pSimulation->GetFOVAngle();
    for (unsigned i=0; i < fr.GetSize(); i++)
    {
        SimPlayerEntity* ps = (SimPlayerEntity*)fr[i].pEntity;
        HUDPlayerAdapter* pha = NULL;
        HUDPlayerAdapter** ppha = PlayerAdapters.Get(ps);
        if (!ppha)
        {
            pha = new HUDPlayerAdapter();
            pha->SetPlayerEntity(ps);
            PlayerAdapters.Add(ps, pha);
        }
        else
            pha = (*ppha);
        BillboardQueryResult qr;
        qr.pPlayer = pha;
        qr.X = (fr[i].Angle / fov) * xextent;
        qr.Y = (-100 + fr[i].DistanceSquared);
        qr.Scale = ((25 - fr[i].DistanceSquared) /*/ 25*/ * 0.04f) * 50 + 50;
        qr.bShowLabel = true;
        presult->PushBack(qr);
    }
}

//
// Query method to retrieve set of enemies in view frustum for display using billboards.
//
void HUDEnvironmentAdapter::GetEnemiesInViewFrustum(Array<BillboardQueryResult>* presult) const
{
    Array<HUDSimulation::FrustumResult> fr;
    pSimulation->GetEntitiesInViewFrustum(&fr, 
        ((HUDSimulation::SimType)pSimulation->GetPlayer()->GetTeamID() == HUDSimulation::SIM_BluePlayer) ? 
        HUDSimulation::SIM_RedPlayer : HUDSimulation::SIM_BluePlayer, 5.f);
    float xextent = 1280.f/2.f;
    float fov = pSimulation->GetFOVAngle();
    for (unsigned i=0; i < fr.GetSize(); i++)
    {
		BillboardQueryResult qr;
        SimPlayerEntity* ps = (SimPlayerEntity*)fr[i].pEntity;
        HUDPlayerAdapter* pha = NULL;
        HUDPlayerAdapter** ppha = PlayerAdapters.Get(ps);
        if (!ppha)
        {
			// If no adapter exists for this entity, create one and add it to PlayerAdapaters list.
            pha = new HUDPlayerAdapter();
            pha->SetPlayerEntity(ps);
            PlayerAdapters.Add(ps, pha);
        }

		// If an adapter already exists for this entity, use it.
        else
            pha = (*ppha);

		// Add the adapter + data to the query's result.
        qr.pPlayer = pha;
        qr.X = (fr[i].Angle / fov) * xextent;
        qr.Y = (-100 + fr[i].DistanceSquared);
        qr.Scale = ((25 - fr[i].DistanceSquared) /*/ 25*/ * 0.04f) * 50 + 50;
        qr.bShowLabel =  fr[i].Angle > -5.f && fr[i].Angle < 5.f;
        presult->PushBack(qr);
    }
}
