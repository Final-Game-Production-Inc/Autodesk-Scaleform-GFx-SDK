/**************************************************************************

Filename    :   FxMinimapView.cpp
Content     :   Minimap view specific type implementations
Created     :   November 13, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_MsgFormat.h"

#include "FxMinimapView.h"


FxMinimapEntity::~FxMinimapEntity()
{
    if (pIconRef)
        pIconRef->ClearOwner();
}

//////////////////////////////////////////////////////////////////////////

FxMinimapIcon::~FxMinimapIcon() 
{
    if (pOwner)
        pOwner->SetIconRef(NULL);
}

void    FxMinimapIcon::RegisterOwner(FxMinimapEntity* powner)
{
    if (pOwner)
        ClearOwner();
    pOwner = powner;
    if (pOwner->GetIconRef())
        pOwner->GetIconRef()->ClearOwner();
    pOwner->SetIconRef(this);
}

void    FxMinimapIcon::ClearOwner()
{
    if (pOwner) pOwner->SetIconRef(NULL);
    pOwner = NULL;
}

//////////////////////////////////////////////////////////////////////////

//
// Templatized implementation of the icon cache factory interface.
// This is instantiated for each minimap icon type.
//
template <class T>
class MinimapIconFactoryBase : public FxMinimapIconCache::Factory
{
public:
    MinimapIconFactoryBase(Value canvas, String symbol, unsigned genCount)
        : FxMinimapIconCache::Factory(genCount), CanvasMC(canvas), SymbolName(symbol)
	{
		IconCount = 0;
	}

    void    GenerateBatch(List<FxMinimapIcon>* pbatch)
    {
		for (unsigned i = 0; i < GenCount; i++)
		{
			Value iconMC;
			if (GenerateIcon(&iconMC))
            {
			    T* picon = new T(iconMC, 0);
			    pbatch->PushBack(picon);
            }
		}
    }

	bool	GenerateIcon(Value* iconMC)
	{
		String instanceName;
		Format(instanceName, "{0}{1}", SymbolName, IconCount++);
		return CanvasMC.AttachMovie(iconMC, SymbolName, instanceName);
	}

    void    ReleaseBatch(List<FxMinimapIcon>& batch)
    {
        FxMinimapIcon* data = batch.GetFirst();
        while (!batch.IsNull(data))
        {
            FxMinimapIcon* next = batch.GetNext(data);
            delete data;
            data = next;
        }
    }

protected:    
    Value        CanvasMC;
	String			SymbolName;
	unsigned			IconCount;
};


//////////////////////////////////////////////////////////////////////////


//
// Icon resource definition for player icons. This definition drives
// both blue and red player icons because they have the same
// structure.
//
class PlayerIcon : public FxMinimapIcon
{
public:
    PlayerIcon(Value& mc, int stateData = 0)
        : FxMinimapIcon(mc, 0, stateData) {}

    virtual void    Update(FxMinimapView* pview, FxMinimapEntity* pentity, float distSquared)
    {
        SF_UNUSED(distSquared);
        bool hasIcon = (pentity->GetIconRef() != NULL);
        PointF pt = pentity->GetPhysicalPosition();
        bool showDir = pview->IsShowingPlayerDir();

        // If entity did not have an icon attached before, then wait for the
        // next update to set the state data. picon is most probably not initialized
        // (the movie needs to advance after creation) 
        if (hasIcon && (State != (int)showDir))
        {
            // state 0: no arrow, state 1: show arrow
            Value frame(double(showDir ? 2.0 : 1.0));
            MovieClip.Invoke("gotoAndStop", NULL, &frame, 1);
            State = (int)showDir;
        }

        pt = pview->GetIconTransform().Transform(pt);
        Value::DisplayInfo info;
        if (State)
        {
            info.SetRotation(pview->IsCompassLocked() ? (pentity->GetDirection() + 90) 
                : (pentity->GetDirection() - pview->GetPlayerDirection()));
        }
        info.SetPosition(pt.x, pt.y);
        MovieClip.SetDisplayInfo(info);

    }
};

typedef MinimapIconFactoryBase<PlayerIcon>    PlayerIconFactory;


//////////////////////////////////////////////////////////////////////////

//
// Icon resource definition for capture points (flags) icons.
//
class FlagIcon : public FxMinimapIcon
{
public:
    FlagIcon(Value& mc, int stateData = 0)
        : FxMinimapIcon(mc, 0, stateData) {}

    virtual void    Update(FxMinimapView* pview, FxMinimapEntity* pentity, float distSquared)
    {
        bool hasIcon = (pentity->GetIconRef() != NULL);
        PointF pt = pentity->GetPhysicalPosition();

        // Sticky ?
        if (distSquared > pview->GetPhysViewRadiusSquared())
        {
            PointF t = pt;
            t -= pview->GetPhysPlayerPosition();
            t *= (pview->GetPhysViewRadius() / t.Distance());
            t += pview->GetPhysPlayerPosition();
            pt = t;
        }

        // If entity did not have an icon attached before, then wait for the
        // next update to set the state data. picon is most probably not initialized
        // (the movie needs to advance after creation) 
        if (hasIcon && (IconData != pentity->GetIconData()))
        {
            Value frame(pentity->GetIconData() + double(1.0));
            MovieClip.Invoke("gotoAndStop", NULL, &frame, 1);
            IconData = pentity->GetIconData();
        }

        pt = pview->GetIconTransform().Transform(pt);
        Value::DisplayInfo info;
        info.SetPosition( pt.x, pt.y );        
        MovieClip.SetDisplayInfo(info);
    }
};

typedef MinimapIconFactoryBase<FlagIcon>    FlagIconFactory;


//////////////////////////////////////////////////////////////////////////

//
// Icon resource definition for objective icons.
//
class ObjectiveIcon : public FxMinimapIcon
{
public:
    ObjectiveIcon(Value& mc, int stateData = 0)
        : FxMinimapIcon(mc, 0, stateData) {}

    virtual void    Update(FxMinimapView* pview, FxMinimapEntity* pentity, float distSquared)
    {
        PointF pt = pentity->GetPhysicalPosition();

        // Sticky ?
        if (distSquared > pview->GetPhysViewRadiusSquared())
        {
            PointF t = pt;
            t -= pview->GetPhysPlayerPosition();
            t *= (pview->GetPhysViewRadius() / t.Distance());
            t += pview->GetPhysPlayerPosition();
            pt = t;
        }

        pt = pview->GetIconTransform().Transform(pt);
        Value::DisplayInfo info;
        info.SetPosition( pt.x, pt.y );
        MovieClip.SetDisplayInfo(info);
    }
};

typedef MinimapIconFactoryBase<ObjectiveIcon>    ObjectiveIconFactory;


//////////////////////////////////////////////////////////////////////////

//
// Icon resource definition for waypoint icons.
//
class WaypointIcon : public FxMinimapIcon
{
public:
    WaypointIcon(Value& mc, int stateData = 0)
        : FxMinimapIcon(mc, 0, stateData)
    {
        // Store a reference to the waypoint arrow for faster access
        mc.GetMember("waypoint_arrow_mc", &ArrowMC);
    }

    virtual void    Update(FxMinimapView* pview, FxMinimapEntity* pentity, float distSquared)
    {
        bool hasIcon = (pentity->GetIconRef() != NULL);
        PointF pt = pentity->GetPhysicalPosition();

        // Sticky ?
        bool bsticky = distSquared > pview->GetPhysViewRadiusSquared();
        if (bsticky)
        {
            PointF t = pt;
            t -= pview->GetPhysPlayerPosition();
            t *= (pview->GetPhysViewRadius() / t.Distance());
            t += pview->GetPhysPlayerPosition();
            pt = t;
        }

        // If entity did not have an icon attached before, then wait for the
        // next update to set the state data. picon is most probably not initialized
        // (the movie needs to advance after creation) 
        if (hasIcon && (State != (int)bsticky))
        {
            // state 0: no arrow, state 1: show arrow
            ArrowMC.GotoAndPlay(bsticky ? "out" : "in");
            State = ((int)bsticky);            
        }

        if (hasIcon && (IconData != pentity->GetIconData()))
        {           
            // Update the waypoint number if needed
            Value tf;
            MovieClip.GetMember("textField", &tf);
            char buf[4];
            StringDataPtr bufs(buf, sizeof(buf));
            Format(bufs, "{0}", pentity->GetIconData());
            tf.SetText(buf);
            IconData = pentity->GetIconData();
        }

        pt = pview->GetIconTransform().Transform(pt);
        Value::DisplayInfo info;
        info.SetPosition( pt.x, pt.y );
        MovieClip.SetDisplayInfo(info);

        if (bsticky)
        {
            // Rotate arrow if sticky
            double angleRad = pt.Angle(1, 0) - SF_MATH_PI_2;
            info.Clear();
            info.SetRotation(SF_RADTODEG(angleRad));
            ArrowMC.SetDisplayInfo(info);
        }
    }

private:
    Value    ArrowMC;
};

typedef MinimapIconFactoryBase<WaypointIcon>    WaypointIconFactory;


//////////////////////////////////////////////////////////////////////////


FxMinimapView::FxMinimapView(const Config& config)
    : bCompassLocked(false), bConvFactorsSet(false), bForceUpdate(true)
{
    ZoomFactors     = config.ZoomFactors;
    ZoomIndex       = config.InitialZoomIndex;
    NormZoomIndex   = config.CurrMapZoomIndex;
    ViewRadius      = config.ViewRadius;

	counter = 0;

    for (unsigned i=0; i < FxMinimapEntity::NUM_TYPES; i++)
        Filter[i] = true;

    // Initialize movie references
    MinimapMC = config.MinimapMC;
    SF_ASSERT(MinimapMC.IsDisplayObject());
    MinimapMC.GetMember("player", &PlayerIconMC);
    SF_ASSERT(PlayerIconMC.IsDisplayObject());
    MinimapMC.GetMember("map", &MapMC);
    SF_ASSERT(MapMC.IsDisplayObject());
    MinimapMC.GetMember("compass", &CompassMC);
    SF_ASSERT(CompassMC.IsDisplayObject());

    // The terrain map is loaded dynamically. We can add additional logic
    // to query its dimensions after load complete, but for ease the values
    // are hard coded.
    ViewDimensions = RectF(512.f, 512.f);

    for (unsigned i=0; i < FxMinimapEntity::NUM_TYPES; i++)
        IconCache[i] = NULL;
    // Create icon caches and register the icon generation methods
    // The method in actionscript is expected to take a single parameter (count), and return
    // an AS array of movieclips representing the newly created icons

	MinimapMC.GetMember("icons_player_red", &PlayerRedMC);
	SF_ASSERT(PlayerRedMC.IsDisplayObject());

	MinimapMC.GetMember("icons_player_blue", &PlayerBlueMC);
	SF_ASSERT(PlayerBlueMC.IsDisplayObject());

	MinimapMC.GetMember("icons_flag", &IconsFlagMC);
	SF_ASSERT(IconsFlagMC.IsDisplayObject());

	MinimapMC.GetMember("icons_diamond", &IconsDiamondMC);
	SF_ASSERT(IconsDiamondMC.IsDisplayObject());

	MinimapMC.GetMember("icons_waypoint", &IconsWaypointMC);
	SF_ASSERT(IconsWaypointMC.IsDisplayObject());

    IconCache[FxMinimapEntity::ID_BluePlayer] 
    = new FxMinimapIconCache(new PlayerIconFactory(PlayerBlueMC, "player_blue", 10));
    IconCache[FxMinimapEntity::ID_RedPlayer] 
    = new FxMinimapIconCache(new PlayerIconFactory(PlayerRedMC, "player_red", 10));
    IconCache[FxMinimapEntity::ID_Flag] 
    = new FxMinimapIconCache(new FlagIconFactory(IconsFlagMC, "flag", 10), true, 1.f, 0.2f);
    IconCache[FxMinimapEntity::ID_Objective] 
    = new FxMinimapIconCache(new ObjectiveIconFactory(IconsDiamondMC, "diamond", 10), true, 0.5f, 0.2f);
    IconCache[FxMinimapEntity::ID_Waypoint] 
    = new FxMinimapIconCache(new WaypointIconFactory(IconsWaypointMC, "waypoint", 10), true, 0.5f, 0.2f);
    bShowPlayerDir  = false;

    // Update the minimap controls
    //UpdateControls();
}


FxMinimapView::~FxMinimapView()
{   
    for (unsigned i=0; i < FxMinimapEntity::NUM_TYPES; i++)
    {
        delete IconCache[i];
    }

    RemoveIcons();
}

void FxMinimapView::RemoveIcons()
{
    RemoveIconsImpl(PlayerRedMC, "player_red");
    RemoveIconsImpl(PlayerBlueMC, "player_blue");
    RemoveIconsImpl(IconsFlagMC, "flag");
    RemoveIconsImpl(IconsDiamondMC, "diamond");
    RemoveIconsImpl(IconsWaypointMC, "waypoint");	
}

void FxMinimapView::RemoveIconsImpl(Value canvas, String symbol)
{
	Value temp;
	String instanceName;
	unsigned i = 0;
	Format(instanceName, "{0}{1}", symbol, i);
	while (canvas.HasMember(instanceName))
	{
		canvas.GetMember(instanceName, &temp);
		
        if (canvas.GetMovie()->GetAVMVersion() == 1) 
        {
            temp.Invoke("removeMovieClip");
        }
        else
        {
            canvas.Invoke("removeChild", NULL, &temp, 1);
        }
        
		Format(instanceName, "{0}{1}", symbol, ++i);
	}
}

void FxMinimapView::Accept(FxDelegateHandler::CallbackProcessor *cbreg)
{
    cbreg->Process("zoomMiniMapView", FxMinimapView::ZoomMiniMapView);
    cbreg->Process("filterFlags", FxMinimapView::FilterFlags);
    cbreg->Process("filterDiamonds", FxMinimapView::FilterObjectives);
    cbreg->Process("filterWaypoints", FxMinimapView::FilterWaypoints);
    cbreg->Process("filterBluePlayers", FxMinimapView::FilterFriendlies);
    cbreg->Process("filterRedPlayers", FxMinimapView::FilterEnemies);
    cbreg->Process("lockMiniMapCompass", FxMinimapView::LockMiniMapCompass);
}

void FxMinimapView::ZoomMiniMapView(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // zoom level shift (-1 or 1)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    int zoomLevelShift = (int)params[0].GetNumber();
    pthis->ChangeZoomLevel(zoomLevelShift);

    pthis->SetShowingPlayerDir(pthis->GetZoomIndex() >= 1);
}

void FxMinimapView::FilterFlags(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // activate or deactivate filter (bool)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->SetFilter(FxMinimapEntity::ID_Flag, params[0].GetBool());    
}

void FxMinimapView::FilterObjectives(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // activate or deactivate filter (bool)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->SetFilter(FxMinimapEntity::ID_Objective, params[0].GetBool());
}

void FxMinimapView::FilterWaypoints(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // activate or deactivate filter (bool)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->SetFilter(FxMinimapEntity::ID_Waypoint, params[0].GetBool());
}

void FxMinimapView::FilterEnemies(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // activate or deactivate filter (bool)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->SetFilter(FxMinimapEntity::ID_RedPlayer, params[0].GetBool());
}

void FxMinimapView::FilterFriendlies(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // activate or deactivate filter (bool)
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->SetFilter(FxMinimapEntity::ID_BluePlayer, params[0].GetBool());
}

void FxMinimapView::LockMiniMapCompass(const FxDelegateArgs& params)
{
    SF_ASSERT(params.GetArgCount()==1);   // lock?
    FxMinimapView* pthis = (FxMinimapView*)params.GetHandler();

    pthis->LockCompass(params[0].GetBool());
}


void FxMinimapView::ChangeZoomLevel(int dev)
{
    int newZoomIdx = ZoomIndex + dev;
    if (newZoomIdx < 0)
        newZoomIdx = 0;
    else if ((UPInt)newZoomIdx >= ZoomFactors.GetSize())
        newZoomIdx = (int)(ZoomFactors.GetSize() - 1);
    ZoomIndex = newZoomIdx;

    bForceUpdate = true;

    UpdateControls();
}

void FxMinimapView::LockCompass(bool lock)
{
    bCompassLocked = lock;
    bForceUpdate = true;
}

void FxMinimapView::UpdateControls()
{
    Value params[8];
    params[0].SetBoolean(IsMaxZoom());
    params[1].SetBoolean(IsMinZoom());
    params[2].SetBoolean(bCompassLocked);
    params[3].SetBoolean(Filter[FxMinimapEntity::ID_BluePlayer]);
    params[4].SetBoolean(Filter[FxMinimapEntity::ID_RedPlayer]);
    params[5].SetBoolean(Filter[FxMinimapEntity::ID_Flag]);
    params[6].SetBoolean(Filter[FxMinimapEntity::ID_Objective]);
    params[7].SetBoolean(Filter[FxMinimapEntity::ID_Waypoint]);
    MinimapMC.Invoke("updateControls", NULL, params, 8);
}

void FxMinimapView::UpdateView(FxMinimapEnvironment* penv, Movie* pmovie, FxMinimapView::Stats* pstats)
{    
    SF_UNUSED(pmovie);

    if (MinimapMC.IsUndefined()) return;

    if (!bConvFactorsSet)
    {
        // Compute transform from physical to view dimensions (aspect ratio is expected to be the same)
        const RectF& physicalBounds = penv->GetPhysicalMapBounds();
        Conv2PhysFactor = (physicalBounds.X2() - physicalBounds.X1()) / 
            (ViewDimensions.X2() - ViewDimensions.X1());
        InvConv2PhysFactor = 1 / Conv2PhysFactor;
        bConvFactorsSet = true;
    }

    // *** Setup view variables (transform matrix, etc.) for minimap icons

    // Get zoom factor
    float conv2ZoomFactor = ZoomFactors[ZoomIndex] / ZoomFactors[NormZoomIndex];

    // Map view center
    PointF currPlayerPhysPos = penv->GetPhysicalPlayerPosition();
    float currPlayerDirection = penv->GetPlayerDirection();
    bool viewChanged = false;
    if (currPlayerPhysPos != PlayerPhysPos) 
    {
        viewChanged = true;
        PlayerPhysPos = currPlayerPhysPos;
    }
    if (currPlayerDirection != PlayerDirection) 
    {
        viewChanged = true;
        PlayerDirection = currPlayerDirection;
    }
    if (bForceUpdate)
    {
        viewChanged = true;
        bForceUpdate = false;
    }

    if (viewChanged)
    {
        ViewCenterMat.SetIdentity();
        ViewCenterMat.AppendScaling(InvConv2PhysFactor * conv2ZoomFactor);
        ViewCenter = ViewCenterMat.Transform(PlayerPhysPos);

        PhysViewRadius = (ViewRadius * Conv2PhysFactor) / conv2ZoomFactor;
        PhysViewRadiusSquared = PhysViewRadius * PhysViewRadius;

        IconTransform.SetIdentity();
        if (bCompassLocked)
        {        
            IconTransform.AppendTranslation(-PlayerPhysPos.x, -PlayerPhysPos.y);
            IconTransform.Append(ViewCenterMat);
        }
        else
        {
            IconTransform.AppendTranslation(-PlayerPhysPos.x, -PlayerPhysPos.y);
            IconTransform.AppendRotation( (float)(-(PlayerDirection + 90) * SF_MATH_DEGTORAD) );
            IconTransform.Append(ViewCenterMat);
        }
    }

    // *** Get entities in view

    Array<QueryResult>     dispRedPlayers;
    Array<QueryResult>     dispBluePlayers;
    Array<QueryResult>     dispFlags;
    Array<QueryResult>     dispObjectives;
    Array<QueryResult>     dispWaypoints;

    if (Filter[FxMinimapEntity::ID_BluePlayer])
        penv->GetEntitiesInPhysicalRadius(&dispBluePlayers, FxMinimapEntity::ID_BluePlayer, currPlayerPhysPos, PhysViewRadius);
    if (Filter[FxMinimapEntity::ID_RedPlayer])
        penv->GetEntitiesInPhysicalRadius(&dispRedPlayers, FxMinimapEntity::ID_RedPlayer, currPlayerPhysPos, PhysViewRadius);
    if (Filter[FxMinimapEntity::ID_Flag])
        penv->GetEntitiesInPhysicalRadius(&dispFlags, FxMinimapEntity::ID_Flag, currPlayerPhysPos, PhysViewRadius);
    if (Filter[FxMinimapEntity::ID_Objective])
        penv->GetEntitiesInPhysicalRadius(&dispObjectives, FxMinimapEntity::ID_Objective, currPlayerPhysPos, PhysViewRadius);
    if (Filter[FxMinimapEntity::ID_Waypoint])
        penv->GetEntitiesInPhysicalRadius(&dispWaypoints, FxMinimapEntity::ID_Waypoint, currPlayerPhysPos, PhysViewRadius);


    // *** Start stopwatch    
    UInt64 currentTicks = Timer::GetTicks();
    if (pstats) pstats->UpdateTime = currentTicks;


    // *** Update non-icon entities

    Value::DisplayInfo info;

    if (viewChanged)
    {
        // Set player position
        // Compensate for the -90 deg shift to represent north
        info.SetRotation(bCompassLocked ? penv->GetPlayerDirection() + 90 : 0);
        PlayerIconMC.SetDisplayInfo(info);

        // Set compass position
        // Compensate for the -90 deg shift to represent north
        info.SetRotation(bCompassLocked ? 0 : -(penv->GetPlayerDirection() + 90));
        CompassMC.SetDisplayInfo(info);

        // Set terrain position
        Matrix2F terrainMat;
        terrainMat.AppendScaling(conv2ZoomFactor);
        if (bCompassLocked)
        {        
            terrainMat.AppendTranslation(-ViewCenter.x, -ViewCenter.y);        
        }
        else
        {
            terrainMat.AppendTranslation(-ViewCenter.x, -ViewCenter.y);
            terrainMat.AppendRotation( (float)(-(penv->GetPlayerDirection() + 90) * SF_MATH_DEGTORAD) );

            // Slower method ( nearly 100x slower - compared to the matrix math - , 
            // but still faster than having the logic in AS)
            /*
            Value mat, params[6];
            params[0] = mat2d.M_[0][0]; // a
            params[1] = mat2d.M_[1][0]; // b
            params[2] = mat2d.M_[0][1]; // c
            params[3] = mat2d.M_[1][1]; // d
            params[4] = mat2d.M_[0][2]; // tx
            params[5] = mat2d.M_[1][2]; // ty
            pmovie->CreateObject(&mat, "flash.geom.Matrix", params, 6);

            Value transform;
            MapMC.GetMember("transform", &transform);
            SF_ASSERT(transform.IsObject());
            transform.SetMember("matrix", mat);
            */
        }
        MapMC.SetDisplayMatrix(terrainMat);
    }

    // *** Update icon positions

    if (IconCache[FxMinimapEntity::ID_BluePlayer])
    {
        FxMinimapIconCache& bluePlayerIcons = *IconCache[FxMinimapEntity::ID_BluePlayer];
        bluePlayerIcons.BeginUpdate(currentTicks);
        if (Filter[FxMinimapEntity::ID_BluePlayer])
            bluePlayerIcons.Process(this, dispBluePlayers);
        bluePlayerIcons.EndUpdate();
    }

    if (IconCache[FxMinimapEntity::ID_RedPlayer])
    {
        FxMinimapIconCache& redPlayerIcons = *IconCache[FxMinimapEntity::ID_RedPlayer];
        redPlayerIcons.BeginUpdate(currentTicks);
        if (Filter[FxMinimapEntity::ID_RedPlayer])
            redPlayerIcons.Process(this, dispRedPlayers);
        redPlayerIcons.EndUpdate();
    }

    if (IconCache[FxMinimapEntity::ID_Flag])
    {
        FxMinimapIconCache& flagIcons = *IconCache[FxMinimapEntity::ID_Flag];
        flagIcons.BeginUpdate(currentTicks);
        if (Filter[FxMinimapEntity::ID_Flag])
            flagIcons.Process(this, dispFlags);
        flagIcons.EndUpdate();
    }

    if (IconCache[FxMinimapEntity::ID_Objective])
    {
        FxMinimapIconCache& objIcons = *IconCache[FxMinimapEntity::ID_Objective];
        objIcons.BeginUpdate(currentTicks);
        if (Filter[FxMinimapEntity::ID_Objective])
            objIcons.Process(this, dispObjectives);
        objIcons.EndUpdate();
    }

    if (IconCache[FxMinimapEntity::ID_Waypoint])
    {
        FxMinimapIconCache& wpIcons = *IconCache[FxMinimapEntity::ID_Waypoint];
        wpIcons.BeginUpdate(currentTicks);
        if (Filter[FxMinimapEntity::ID_Waypoint])
            wpIcons.Process(this, dispWaypoints);
        wpIcons.EndUpdate();
    }

    // *** Stop stopwatch and return elapsed time
    if (pstats) 
    {
        pstats->UpdateTime = (Timer::GetTicks() - pstats->UpdateTime);
        pstats->PushedObjects = (unsigned)(dispBluePlayers.GetSize() + dispRedPlayers.GetSize() + dispFlags.GetSize() + 
                                dispObjectives.GetSize() + dispWaypoints.GetSize());
    }
}


//////////////////////////////////////////////////////////////////////////


void FxMinimapIconCache::BeginUpdate(UInt64 currentTicks)
{
    // Tick fades
    if (bFadingEnabled)
    {
        if (bFirstUpdate)
            bFirstUpdate = false;
        else
        {            
            Value::DisplayInfo info;

            // Tick Fade In set
            float delta = (currentTicks - LastTicks) * FadeInFactor;
            FxMinimapIcon* picon = FadeInSet.GetFirst();
            while (!FadeInSet.IsNull(picon))
            {
                FxMinimapIcon* next = FadeInSet.GetNext(picon);
                picon->GetDisplayInfo(&info);
                GFx::Double alpha = info.GetAlpha();
                info.Clear();
                if (alpha != 100)
                {
                    info.SetAlpha(Alg::Min(GFx::Double(100.0), alpha + delta));
                    picon->SetDisplayInfo(info);
                }
                else
                {
                    picon->RemoveNode();
                    AddToActiveSet(picon);
                }
                picon = next;
            }

            // Tick Fade Out set
            delta = (currentTicks - LastTicks) * FadeOutFactor;
            picon = FadeOutSet.GetFirst();
            while (!FadeOutSet.IsNull(picon))
            {
                FxMinimapIcon* next = FadeOutSet.GetNext(picon);
                picon->GetDisplayInfo(&info);
                GFx::Double alpha = info.GetAlpha();
                info.Clear();
                if (alpha != 0)
                {
                    info.SetAlpha(Alg::Max(GFx::Double(0.0), alpha - delta));
                    picon->SetDisplayInfo(info);
                }
                else
                {
                    picon->RemoveNode();
                    AddToInactiveSet(picon);
                }
                picon = next;
            }
        }
        LastTicks = currentTicks;
    }

    // Move all active icons to the transition set (used to filter out entities not in view)
    FxMinimapIcon* picon = ActiveSet.GetFirst();
    while (!ActiveSet.IsNull(picon))
    {
        FxMinimapIcon* next = ActiveSet.GetNext(picon);
        picon->RemoveNode();
        TransitionSet.PushBack(picon);
        picon = next;
    }
    picon = FadeInSet.GetFirst();
    while (!FadeInSet.IsNull(picon))
    {
        FxMinimapIcon* next = FadeInSet.GetNext(picon);
        picon->RemoveNode();
        TransitionSet.PushBack(picon);
        picon = next;
    }
}

void FxMinimapIconCache::EndUpdate()
{
    // At this point, all icons in the transition set are attached to 
    // entities not in view. If fading is required, move it to the fade out
    // set. Else, release it to inactive.
    FxMinimapIcon* picon = TransitionSet.GetFirst();
    if (bFadingEnabled)
    {
        while (!TransitionSet.IsNull(picon))
        {
            FxMinimapIcon* next = TransitionSet.GetNext(picon);
            picon->RemoveNode();
            AddToFadeOutSet(picon);
            picon = next;
        }
    }
    else
    {
        while (!TransitionSet.IsNull(picon))
        {
            FxMinimapIcon* next = TransitionSet.GetNext(picon);
            picon->RemoveNode();
            AddToInactiveSet(picon);
            picon = next;
        }
    }
}

FxMinimapIcon* FxMinimapIconCache::GetNewIcon()
{
    // Make sure there are cached icons; if not, generate some
    if (InactiveSet.IsEmpty())
    {
        // Generate a set of icons
        List<FxMinimapIcon> batch;
        pIconFactory->GenerateBatch(&batch);
        FxMinimapIcon* picon = batch.GetFirst();
        while (!batch.IsNull(picon))
        {
            FxMinimapIcon* next = batch.GetNext(picon);
            picon->RemoveNode();
            AddToInactiveSet(picon);
            picon = next;
        }
    }
    FxMinimapIcon* picon = InactiveSet.GetFirst();
    if (picon)
    {
        picon->RemoveNode();

        if (bFadingEnabled)
            AddToFadeInSet(picon);
        else
            AddToActiveSet(picon);
    }

    return picon;
}

void    FxMinimapIconCache::AddToActiveSet(FxMinimapIcon *picon)
{
    Value::DisplayInfo info(true);
    picon->SetDisplayInfo(info);
    ActiveSet.PushBack(picon);
}

void    FxMinimapIconCache::AddToFadeInSet(FxMinimapIcon *picon)
{
    Value::DisplayInfo info(true);
    info.SetAlpha(0.0);
    picon->SetDisplayInfo(info);
    FadeInSet.PushBack(picon);
}

void    FxMinimapIconCache::AddToFadeOutSet(FxMinimapIcon *picon)
{
    // The icon is already if this method is called.
    FadeOutSet.PushBack(picon);
}

void    FxMinimapIconCache::AddToInactiveSet(FxMinimapIcon *picon)
{
    Value::DisplayInfo info(false);
    picon->SetDisplayInfo(info);
    InactiveSet.PushBack(picon);
    picon->ClearOwner();
}

void    FxMinimapIconCache::Process(FxMinimapView* pview, const Array<QueryResult>& entities)
{
    for (unsigned i=0; i < entities.GetSize(); i++)
    {
        FxMinimapEntity* pent = entities[i].pEntity;
        FxMinimapIcon* picon = pent->GetIconRef();
        bool hasIcon = (picon != NULL);
        if (!hasIcon)
            // Get an unused icon from the cache. If the cache is empty, then create more icon Movieclips.
            picon = GetNewIcon();
        else
        {
            // If entity already has an icon attached, then it's in the transition set
            // Move it back to active set (there's no need to change display info)
            picon->RemoveNode();
            if (bFadingEnabled)
                FadeInSet.PushBack(picon);
            else
                ActiveSet.PushBack(picon);
        }
        if (picon)
        {
            picon->Update(pview, pent, entities[i].DistanceSquared);
            if (!hasIcon)
                picon->RegisterOwner(pent);
        }
    }
}
