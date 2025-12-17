/**************************************************************************

Filename    :   FxHUDView.cpp
Content     :   HUD view specific type implementations
Created     :   December 13, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_MsgFormat.h"

#include "FxHUDView.h"

FxHUDLog::~FxHUDLog() 
{
	// Remove any remaining data and movieClip pointers.
    FxHUDMessage* data = Log.GetFirst();
    while (!Log.IsNull(data))
    {
        FxHUDMessage* next = Log.GetNext(data);
        delete data->GetMovieclip();
        Log.Remove(data);
        delete data;
        data = next;
    }

    FxHUDMessage::Movieclip* pmc = MessageMCs.GetFirst();
    while (!MessageMCs.IsNull(pmc))
    {
        FxHUDMessage::Movieclip* next = MessageMCs.GetNext(pmc);
        MessageMCs.Remove(pmc);
        delete pmc;
        pmc = next;
    }
}

//
// Process HUD events fired by the simulation and format info 
// appropriately for display in HTML.
//
void    FxHUDLog::Process(FxHUDEvent *pevent)
{
    String msg;
    switch (pevent->GetType())
    {
    case FxHUDEvent::EVT_Capture:
        {
            FxHUDCaptureEvent* e = (FxHUDCaptureEvent*)pevent;
            Format(msg, "{0} captured a flag!", e->IsUserPlayer() ? "You've" : (const char*)e->GetSrcName());
        } 
        break;
    case FxHUDEvent::EVT_Kill:
        {
            FxHUDKillEvent* e = (FxHUDKillEvent*)pevent;
            const char* pweapIcon = "";
            switch (e->GetWeaponType())
            {
                case FxHUDPlayer::WEAP_Pistol: pweapIcon = "pistol"; break;
                case FxHUDPlayer::WEAP_MachineGun: pweapIcon = "machine_gun"; break;
                case FxHUDPlayer::WEAP_Grenade: pweapIcon = "grenade"; break;
            }
            
            //msg = "TODO";
//*
            Format(msg, "<img src='rank{1}' width='20' height='20' align='baseline' vspace='-7'/>{0}  "
                          "<img src='{4}' align='baseline' vspace='-3'/> "
                          "<img src='rank{3}' width='20' height='20' align='baseline' vspace='-7'/>{2}", 
                e->GetSrcName(), e->GetSrcRank()+1, e->GetDestName(), e->GetDestRank()+1, pweapIcon);
//*/
        } 
        break;
    case FxHUDEvent::EVT_LevelUp:
        {
            FxHUDLevelUpEvent* e = (FxHUDLevelUpEvent*)pevent;
            
            //msg = "TODO";
//*
            Format(msg, "You are now a <img src='rank{1}'/> {0}!", e->GetNewRankName(), e->GetNewRank());
//*/
        } 
        break;
    case FxHUDEvent::EVT_Powerup:
        {
            msg = "You've picked up a powerup!";
        } 
        break;
    default:
        return;
    }

    FxHUDMessage::Movieclip* pmc = GetUnusedMessageMovieclip();
    FxHUDMessage* m = new FxHUDMessage(msg/*, 3*/, pmc); // 3 seconds
    Log.PushBack(m);
    NumMessages++;
}

//
// HUD Log update logic.
//
void    FxHUDLog::Update()
{
    SF_ASSERT(LogMC.IsDisplayObject());

    GFx::Double rowHeight = 30.f;

    // Tick the message lifetimes
    GFx::Double yoff = 0.0;
    FxHUDMessage* data = Log.GetFirst();
    while (!Log.IsNull(data))
    {
        FxHUDMessage* next = Log.GetNext(data);
        if (data->IsAlive())
        {
            // Layout mgmt and animation
            Value::DisplayInfo info;
            info.SetY(yoff);
            data->GetMovieclip()->GetRef().SetDisplayInfo(info);
        }
        data = next;
        yoff += rowHeight;
    }

    // Layout management and animation
    Value::DisplayInfo info;
    info.SetY(LogOriginalY - (NumMessages * rowHeight));
    LogMC.SetDisplayInfo(info);

    // Remove dead messages
    data = Log.GetFirst();
    while (!Log.IsNull(data))
    {
        FxHUDMessage* next = Log.GetNext(data);
        if (!data->IsAlive())
        {
            Log.Remove(data);
            NumMessages--;
            
            FxHUDMessage::Movieclip* pmc = data->GetMovieclip();
            pmc->SetVisible(false);
            MessageMCs.PushBack(pmc);

            delete data;
        }
        data = next;
    }
}

//
// Request a movieClip for displaying new HUD Log message.
//
FxHUDMessage::Movieclip* FxHUDLog::GetUnusedMessageMovieclip()
{
    if (MessageMCs.IsEmpty())
    {
        // Request a new line in the SWF
        Value logline;
		LogMC.AttachMovie(&logline, "LogMessage", "logMessage"+NumMessages);
        FxHUDMessage::Movieclip* pmc = new FxHUDMessage::Movieclip(logline);
        MessageMCs.PushBack(pmc);  // Add new Message MC to list of unused message movieClips.
    }
    FxHUDMessage::Movieclip* pmc = MessageMCs.GetFirst();  // Use an unused Message movieClip.
    pmc->SetVisible(true);
    MessageMCs.Remove(pmc);
    return pmc;
}

//////////////////////////////////////////////////////////////////////////
FxHUDView::FxHUDView(const Config& config)
    : Paused(false), bFirstUpdate(true), bForceUpdate(true), ReticleHeat(0.f), 
      ReticleFiringOffset(0.f)
{
    // Initialize movie references
    HUDMC = config.HUDMC;
    SF_ASSERT(HUDMC.IsDisplayObject());

	Value billboardContainerMC;
	HUDMC.GetMember("billboard_container", &billboardContainerMC);
    FriendlyBillboards.Configure(billboardContainerMC, "billboard_friendly", "friendly");
    EnemyBillboards.Configure(billboardContainerMC, "billboard_enemy", "enemy");

	// Register log movieClip reference for HUD Log.
    Value log;
    HUDMC.GetMember("log", &log);
    Log.SetLogMovieclip(log);

	// Cache core movieClips so we don't have to retrieve them every update.
	// TeamStatsMC
    HUDMC.GetMember("teamStats", &TeamStatsMC);
    SF_ASSERT(TeamStatsMC.IsDisplayObject());

	TeamStatsMC.GetMember("scoreRed", &ScoreRedMC);
	TeamStatsMC.GetMember("teamRed", &TeamRedMC);
	TeamStatsMC.GetMember("scoreBlue", &ScoreBlueMC);
	TeamStatsMC.GetMember("teamBlue", &TeamBlueMC);
	TeamStatsMC.GetMember("redWinning", &RedWinningMC);
	TeamStatsMC.GetMember("blueWinning", &BlueWinningMC);

	TeamStatsMC.GetMember("roundTime", &RoundTimeMC);
	SF_ASSERT(RoundTimeMC.IsDisplayObject());

	// PlayerStatsMC
	HUDMC.GetMember("playerStats", &PlayerStatsMC);
    SF_ASSERT(PlayerStatsMC.IsDisplayObject());

	PlayerStatsMC.GetMember("healthN", &HealthNMC);
	PlayerStatsMC.GetMember("health", &HealthMC);
	PlayerStatsMC.GetMember("ammoN", &AmmoNMC);
	PlayerStatsMC.GetMember("clipN", &ClipNMC);
	PlayerStatsMC.GetMember("ammo", &AmmoMC);
	AmmoMC.GetMember("ammoBG", &AmmoBGMC);
	PlayerStatsMC.GetMember("grenade", &GrenadeMC);
	PlayerStatsMC.GetMember("weapon", &WeaponMC);
	PlayerStatsMC.GetMember("reload", &ReloadMC);

	// DirHitMC
	HUDMC.GetMember("dirHit", &DirHitMC);
	SF_ASSERT(DirHitMC.IsDisplayObject());
	
	DirHitMC.GetMember("t", &DirMC_T);
	DirHitMC.GetMember("tl", &DirMC_TL);
	DirHitMC.GetMember("l", &DirMC_L);
	DirHitMC.GetMember("bl", &DirMC_BL);
	DirHitMC.GetMember("b", &DirMC_B);
	DirHitMC.GetMember("br", &DirMC_BR);
	DirHitMC.GetMember("r", &DirMC_R);
	DirHitMC.GetMember("tr", &DirMC_TR);

	Value expMC;
	HUDMC.GetMember("expBar", &expMC);
	expMC.GetMember("exp", &ExpBarMC);
	HUDMC.GetMember("rank", &RankMC);

	Value reticuleContainerMC;
	HUDMC.GetMember("reticule", &reticuleContainerMC);
	reticuleContainerMC.GetMember("left", &ReticuleMC_Left);
	reticuleContainerMC.GetMember("right", &ReticuleMC_Right);
	reticuleContainerMC.GetMember("bottom", &ReticuleMC_Bottom);
	reticuleContainerMC.GetMember("top", &ReticuleMC_Top);

	HUDMC.GetMember("flagMC", &FlagMC);
	FlagMC.GetMember("flag", &FlagBarMC);
	FlagBarMC.GetMember("arrow", &FlagArrowMC);

	HUDMC.GetMember("popup", &PopupMC);
	PopupMC.GetMember("popupNumber", &PopupNumberMC);
	PopupNumberMC.GetMember("textField", &PTextFieldMC);

	HUDMC.GetMember("centerTextMC", &CenterTextMC);
	CenterTextMC.GetMember("centerText", &CenterText);
	CenterText.GetMember("textField", &TextFieldMC);

	WinningText = "Winning";
	LosingText  = "Losing";
	RespawnText = "You were killed! Respawning...";
	MaxScore = 500;
	MessageCount = 0;

	// Hide the unused HUD elements
	Value flagBlue;
	HUDMC.GetMember("flagBlue", &flagBlue);
	SetVisible(&flagBlue, false);

	Value flagRed;
	HUDMC.GetMember("flagRed", &flagRed);
	SetVisible(&flagRed, false);

	Value redName;
	TeamStatsMC.GetMember("redName", &redName);
	SetVisible(&redName, false);

	bReloadMCVisible = false;
	bFlagMCVisible = false;
}

FxHUDView::~FxHUDView() 
{
	//
}

//
// FxDelegateHandler implementation.
//
void FxHUDView::Accept(FxDelegateHandler::CallbackProcessor *cbreg)
{
    SF_UNUSED(cbreg);
}

//
// Register Minimap MC and initialize minimap object.
//
void FxHUDView::RegisterMinimap(FxMinimapView::Config& config)
{
	MinimapMC.SetUndefined();
    MinimapMC = config.MinimapMC;
	pMinimapView = NULL;
    pMinimapView = *new FxMinimapView(config);
    pMinimapView->SetShowingPlayerDir(true);
}

//
// HUD view update logic.
//
void FxHUDView::UpdateView(FxHUDEnvironment* penv, Movie* pmovie, FxHUDView::Stats* pstats)
{    
    SF_UNUSED(pmovie);
	SF_UNUSED(pstats);

    if (HUDMC.IsUndefined()) return;

	if (!penv->IsRunning()) return;

    // Update and show blip data
    if ( pMinimapView )
        pMinimapView->UpdateView(penv, pmovie, &(pstats->MinimapStats));

    UInt64 ticks = Timer::GetTicks();

    if (bFirstUpdate)
    {
        LastTicks = ticks;
        bFirstUpdate = false;
		FullRoundTime = (unsigned)penv->GetSecondsLeftInRound();
		ResetState();
    }

	Array<FxHUDEnvironment::BillboardQueryResult>    friendlies, enemies;   
	penv->GetFriendliesInViewFrustum(&friendlies);
	penv->GetEnemiesInViewFrustum(&enemies);

    // *** Start stopwatch    
    UInt64 currentTicks = Timer::GetTicks();
    if (pstats) pstats->UpdateTime = currentTicks;
	
    Log.Update();  // Update the HUD Log

	// Update HUD View elements
    UpdateTeamStats(penv);
    UpdatePlayerEXP(penv);
    UpdatePlayerStats(penv);
    UpdateEvents(penv);
    UpdateTutorial(penv);

	// Update billboards
    UpdateBillboards(friendlies, enemies);

	LastTicks = ticks;

	UpdateState(penv);

    // *** Stop stopwatch and return elapsed time
    if (pstats) 
        pstats->UpdateTime = (Timer::GetTicks() - pstats->UpdateTime);
}

//
// Store the state of the the simulation. Used to avoid updating
// HUD elements unnecessarily. 
//
void FxHUDView::UpdateState(FxHUDEnvironment *penv)
{
	FxHUDPlayer* pplayer = penv->GetHUDPlayer();

	State.ScoreBlue = unsigned(penv->GetHUDBlueTeamScore());
	State.ScoreRed = unsigned(penv->GetHUDRedTeamScore());
	State.AmmoClipSize = pplayer->GetMaxAmmoInClip();
	State.AmmoInClip = pplayer->GetNumAmmoInClip();
	State.AmmoClips = pplayer->GetNumClips();
	State.Grenades = pplayer->GetNumGrenades();
	State.Health = pplayer->GetHealth();
	State.Exp = pplayer->GetLevelXP();
	State.Rank = unsigned(pplayer->GetRank());
	State.WeaponType = pplayer->GetWeaponType();
}

//
// Reset the state data for the simulation to ensure all elements 
// are updated during the next update.
//
void FxHUDView::ResetState()
{
	State.ScoreBlue = unsigned(-1);
	State.ScoreRed = unsigned(-1);
	State.AmmoClipSize = unsigned(-1);
	State.AmmoInClip = unsigned(-1);
	State.AmmoClips = unsigned(-1);
	State.Grenades = unsigned(-1);
	State.Health = -1;
	State.Exp = -1;
	State.Rank = unsigned(-1);
	State.Sec = unsigned(-1);
}

//
// Logic to update the TeamStats movieClip
//
void FxHUDView::UpdateTeamStats(FxHUDEnvironment *penv)
{
	// Retrieve the scores from the simulation.
	unsigned scoreBlue = unsigned(penv->GetHUDBlueTeamScore());
	unsigned scoreRed = unsigned(penv->GetHUDRedTeamScore());

	String text;
	Value tf;
	Value::DisplayInfo info;

	// We won't update any element who has not changed since the last update
	// to avoid unnecessary updates.  To do so, we compare the previous simulation State
	// to the latest simulation data.  If the two are different, update the UI element in the HUD View.
	if (State.ScoreRed != scoreRed)
	{
		Format(text, "{0}", scoreRed);
		ScoreRedMC.SetText(text); // Update the red team score text.

		info.SetScale((scoreRed / (double)MaxScore) * 100, 100); 
		TeamRedMC.SetDisplayInfo(info); // Update and scale the red team score bar movieClip.
	}

	if (State.ScoreBlue != scoreBlue)
	{
		Format(text, "{0}", scoreBlue);
		ScoreBlueMC.SetText(text); // Update the blue team score text.

		info.SetScale((scoreBlue / (double)MaxScore) * 100, 100);  
		TeamBlueMC.SetDisplayInfo(info); // Update and scale the blue team score bar movieClip.
	}

	if (State.ScoreRed != scoreRed || State.ScoreBlue != scoreBlue)
	{
		// Update the "Winning" and "Losing" text fields for both teams if the score has changed.
		if (scoreBlue > scoreRed) 
		{
			RedWinningMC.SetText(LosingText);
			BlueWinningMC.SetText(WinningText);
		} 
		else 
		{
			RedWinningMC.SetText(WinningText);
			BlueWinningMC.SetText(LosingText);
		}
	}

	// Update the roundtime clock
	float secondsLeft = penv->GetSecondsLeftInRound();
	unsigned sec = ((unsigned)secondsLeft % 60);
	if (sec != State.Sec)
	{
		unsigned min = ((unsigned)secondsLeft / 60);
		String timeLeft;
		Format(timeLeft, "{0}:{1:0.2}", min, sec);
		RoundTimeMC.SetText(timeLeft);
		State.Sec = sec;
	}
}

//
// Update the player's EXP elements.
//
void FxHUDView::UpdatePlayerEXP(FxHUDEnvironment *penv)
{
	FxHUDPlayer* pplayer = penv->GetHUDPlayer();
	if (pplayer->GetLevelXP() != State.Exp)
	{
		Value::DisplayInfo info;
		// Update the rank icon.  Each rank icon is on a different frame of the rank movieClip.
		RankMC.GotoAndStop(unsigned(pplayer->GetRank() + 1));
		info.SetScale(pplayer->GetLevelXP() * 100, 100);  // Scale the EXP bar movieClip.
		ExpBarMC.SetDisplayInfo(info);
	}
}

//
// Update the PlayerStats movieClip.
//
void FxHUDView::UpdatePlayerStats(FxHUDEnvironment *penv)
{
    FxHUDPlayer* pplayer = penv->GetHUDPlayer();

	// Load latest player info.
	unsigned ammoInClip = pplayer->GetNumAmmoInClip();
	unsigned ammoClips	= pplayer->GetNumClips();
	unsigned clipSize	= pplayer->GetMaxAmmoInClip();
	unsigned grenades	= pplayer->GetNumGrenades();

	unsigned ammoFrames = 51;
	unsigned grenadeFrames = 5;

	String text;
	
	if(pplayer->GetHealth() != State.Health)
	{
		unsigned health		= unsigned(pplayer->GetHealth() * 100);
		Format(text, "{0}", health);
		HealthNMC.SetText(text); // Update the health text field.

		Value::DisplayInfo info;
		info.SetScale(double(health), 100);  
		HealthMC.SetDisplayInfo(info); // Update and scale the health bar.
	}

	if(ammoInClip != State.AmmoInClip)
	{
		Format(text, "{0}", ammoInClip);
		AmmoNMC.SetText(text); // Update the ammo text field.

		// The ammo is divided into two parts: the white bullet icons (AmmoMC)
		// and their respective backgrounds (AmmoBGMC). 
		//
		// For update the number of bullets currently in the clip, we will use
		// GotoAndStop with AmmoMC and select the frame with the proper number of 
		// white bullet icons.  There are 51 frames in AmmoMC (defined above in ammoFrames)
		// and the first frame displays every bullet icon. To display X number of bullets, 
		// we subtract X from the total number of frames in AmmoMC (51) and GotoAndStop on 
		// the result.
		AmmoMC.GotoAndStop(ammoFrames - ammoInClip); 
	}

	if(ammoClips != State.AmmoClips)
	{
		Format(text, "x{0}", ammoClips);
		ClipNMC.SetText(text); // Update the remaining ammo clips text field.
	}

	if(grenades != State.Grenades)
	{
		// Grenades are setup similar to Ammo.  The first frame for the Grenades movieClip
		// shows 4 white grenade icons.  The second frame shows 3.  To display X number of grenades,
		// we subtract X from the total number of frames in GrenadeMC (5) and GotoAndStop on
		// the result.
		// Note: GrenadeMC actually contains 10 frames but 6-10 are unused by this implementation.
		GrenadeMC.GotoAndStop(grenadeFrames - grenades); 
	}

	if (pplayer->GetWeaponType() != State.WeaponType)
	{
		// Change the weapon icon if the player has changed weapons.
		unsigned weaponFrame = GetWeaponFrame(pplayer->GetWeaponType());
		WeaponMC.GotoAndStop(weaponFrame);

		// Update the ammo background icons based on clipSize.
		if (clipSize != State.AmmoClipSize)
			AmmoBGMC.GotoAndStop(ammoFrames - clipSize);
	}

	// If the ammo in clip is less than 6 bullets, play the "Reload" indicator movieClip.
	if (ammoInClip <= 5)
	{
		// ReloadMC will play until we GotoAndStop on Frame 1, so no need to start the animation
		// more than once.
		if (!bReloadMCVisible)
		{ 
			bReloadMCVisible = true;
			ReloadMC.GotoAndPlay("on"); // Will cycle back to "on" frame when it reaches the end and play again.
		}
	}
	// If the reload indicator is displayed but there are more than six bullets
	// hide it and stop the animation by playing frame 1 of the movieClip.
	else if (bReloadMCVisible) 
	{
		ReloadMC.GotoAndStop("1"); // Frame 1 contains stop();
		bReloadMCVisible = false;
	}
}

//
// Flag capture indicators and reticule behavior.
//
void FxHUDView::UpdateEvents(FxHUDEnvironment *penv)
{
	Value::DisplayInfo info;

	if (penv->IsHUDPlayerCapturingObjective())
    {
		if(!bFlagMCVisible)
		{
			SetVisible(&FlagMC, true); // Set the Flag MovieClip's visibility to true.
			FlagMC.GotoAndPlay("on"); // Play the fade-in animation once.
			bFlagMCVisible = true;
		}
		
		// Shift the x-coordinate of the Flag Arrow to show the capture state of the flag
		// the player is currently capturing.
		info.SetX(penv->GetHUDCaptureObjectiveState() * 177);
		FlagArrowMC.SetDisplayInfo(info);
		info.Clear();
	}
	else if (bFlagMCVisible & !penv->IsHUDPlayerCapturingObjective())
	{
		// If the flag indicator is still visible and the player is
		// no longer capturing an objective, play the fade-out animation
		FlagMC.GotoAndPlay("off");
		bFlagMCVisible = false;
	}

	// Shift the XY-coordinates of the reticule elements based on the
	// firing duration and rate of fire.
	if (ReticleHeat > 1 || ReticleFiringOffset > 0){
		if (ReticleHeat > 1.0f) 
			ReticleHeat -= .02f;

		ReticleFiringOffset -= .02f;
		if (ReticleFiringOffset < 0) ReticleFiringOffset = 0;

		info.SetY((-6) - (ReticleFiringOffset + ReticleHeat));
		ReticuleMC_Top.SetDisplayInfo(info);
		info.Clear();

		info.SetX((6) + (ReticleFiringOffset + ReticleHeat));
		ReticuleMC_Right.SetDisplayInfo(info);
		info.Clear();

		info.SetX((-6) - (ReticleFiringOffset + ReticleHeat));
		ReticuleMC_Left.SetDisplayInfo(info);
		info.Clear();

		info.SetY((6) + (ReticleFiringOffset + ReticleHeat));
		ReticuleMC_Bottom.SetDisplayInfo(info);
		info.Clear();
	}
}

//
// FxHUDEvent handler for HUD view.
//
void FxHUDView::OnEvent( FxHUDEvent* pevent )
{ 
	switch ( pevent->GetType() ) 
	{
		// If the Damage Event (Player was hit) is fired, show the appropriate directional hit indicator.
		// We can use GotoAndPlay("on") to play the fade-in animation.  It will fade out on its own
		// after ~1 second (this animation is setup on the Flash timeline).
		case FxHUDEvent::EVT_Damage:
			{
				FxHUDDamageEvent* peventDamage = (FxHUDDamageEvent*)pevent;
				float dir = peventDamage->GetDirection();
				if (dir > 0)
				{
					if (dir > 90)
					{
						if (dir > 135) 
							DirMC_B.GotoAndPlay("on");
						else 
							DirMC_BR.GotoAndPlay("on");		
					}
					else
					{
						if (dir > 45)
							DirMC_TR.GotoAndPlay("on");
						else
							DirMC_T.GotoAndPlay("on");
					}
				}
				else 
				{
					if (dir < -90)
					{
						if (dir < -135) 
							DirMC_B.GotoAndPlay("on");
						else 
							DirMC_BL.GotoAndPlay("on");		
					}
					else
					{
						if (dir < -45)
							DirMC_L.GotoAndPlay("on");
						else
							DirMC_TL.GotoAndPlay("on");
					}
				}
			}
			break;
		
		// If a KillStreak event is fired, display the kill streak pop-up. 
		case FxHUDEvent::EVT_KillStreak:
			{
				FxHUDKillStreakEvent* peventKS = (FxHUDKillStreakEvent*)pevent;
				
				String text;
				Format(text, "{0}", peventKS->GetSrcKillStreak()); // Format the number of kills
				PTextFieldMC.SetText(text); // Set the text field of the PopUp.

				// Play the fade-in animation, it will fade on its own after ~4 sec.
				PopupMC.GotoAndPlay("on"); 
        	}
			break;
		
		// If the HUD player is killed, display the "Respawning..." text.
		case FxHUDEvent::EVT_Kill:
			{
				FxHUDKillEvent* peventKill = (FxHUDKillEvent*)pevent;
				if(SFstrcmp(peventKill->GetDestName(), "east77") == 0) 
				{
					TextFieldMC.SetText(RespawnText); // Set the text field of the Center Text.
					CenterTextMC.GotoAndPlay("on"); // Play the fade-in animation, it will fade on its own after ~3 sec.
				}
			}
			break;

		// If the player is firing his/her weapon, update the reticule's heat.  
		// The reticule elements are updated in FxHUDView::UpdateEvents() based on
		// the ReticleHeat and the ReticleFiringOffset.
		case FxHUDEvent::EVT_PlayerFire:
			{	
				if (ReticleHeat < 8)
					ReticleHeat += 2.5f;

				ReticleFiringOffset = 2;
			}
		default:
			break;
	}

	//Pass the event to the log for display.
	Log.Process(pevent);
}

//
// Simplistic tutorial system based on the elapsed time.
// Displays basic instructions in the CenterText movieClip (CenterTextMC).
//
void FxHUDView::UpdateTutorial(FxHUDEnvironment *penv)
{
	float elapsedTimeMS = (float)FullRoundTime - penv->GetSecondsLeftInRound();
	bool bShowMessage = false;
	if (elapsedTimeMS < 100.f){
		String text;
		if (MessageCount == 0 && elapsedTimeMS <= 3.f)
		{
			text = "Capture the objectives!";
			MessageCount++;
			bShowMessage = true;
		}
		else if (MessageCount == 1 && elapsedTimeMS > 15.f)
		{ 
			text = "Press SPACE to fire your weapon.";
			MessageCount++;
			bShowMessage = true;
		}
		else if (MessageCount == 2 && elapsedTimeMS > 30.f)
		{ 
			text = "Press R to reload.";
			MessageCount++;
			bShowMessage = true;
		}
		else if (MessageCount == 3 && elapsedTimeMS > 45.f)
		{ 
			text = "Use the number keys, [1] and [2], to change your weapon.";
			MessageCount++;
			bShowMessage = true;
		}
		else if (MessageCount == 4 && elapsedTimeMS > 60.f)
		{ 
			text = "Press G to throw a grenade.";
			MessageCount++;
			bShowMessage = true;
		}
		else if (MessageCount == 5 && elapsedTimeMS > 75.f)
		{
			text = "Collect power-ups to recover health and ammo.";
			MessageCount++;
			bShowMessage = true;
		}

		if (bShowMessage)
		{
			TextFieldMC.SetText(text); // Update the text with the appropriate text.
			CenterTextMC.GotoAndPlay("on5"); // Play the animation which displays text for 5 seconds then fades out.
		}
	}	
}

//
// Helper method for changing movieClips visibility.
//
void FxHUDView::SetVisible(Value* mc, bool visible)
{
	if (mc->IsDisplayObject())
	{
		Value::DisplayInfo info;
		info.SetVisible(visible);
		mc->SetDisplayInfo(info);
	}
}

void FxHUDView::ToggleMinimapLock()
{
    if (pMinimapView)
    {
    	pMinimapView->LockCompass(!pMinimapView->IsCompassLocked());
    }
}

//
// Returns the movie clip frame with the respective weapon icon.
//
unsigned FxHUDView::GetWeaponFrame(FxHUDPlayer::Weapon w)
{
	switch(w) 
	{
		case (FxHUDPlayer::WEAP_Pistol):
			{
				return 13;
			}
		break;
		case (FxHUDPlayer::WEAP_MachineGun):
			{
				return 12;
			}
		break;
		case (FxHUDPlayer::WEAP_Grenade):
		default:
			return 12;
		break;
	}
}

void FxHUDView::UpdateBillboards(const Array<FxHUDEnvironment::BillboardQueryResult>& friendlies,
								 const Array<FxHUDEnvironment::BillboardQueryResult>& enemies)
{
    Value::DisplayInfo info(true);

    // Process friendlies
    // Friendly billboards will always show names
    
    BillboardMC* pbb = NULL;

    FriendlyBillboards.BeginProcessing();
    for (unsigned i=0; i < friendlies.GetSize(); i++)
    {
         info.Clear();
         if (FriendlyBillboards.GetUnusedBillboardMovieclip(friendlies[i].pPlayer, &pbb))
         {   
             info.SetVisible(true);
             pbb->Billboard.GotoAndPlay("show");
             // Load player info
             String title;
             
//             title = "TODO";
//*
             Format(title, "<img src='rank{0}' width='20' height='20' align='baseline' vspace='-7'/> {1}", 
                 friendlies[i].pPlayer->GetRank()+1, friendlies[i].pPlayer->GetName());
//*/
             pbb->Textfield.SetTextHTML(title);
         }
         info.SetX(friendlies[i].X);
         info.SetY(friendlies[i].Y);
         info.SetScale(friendlies[i].Scale, friendlies[i].Scale);
         pbb->Billboard.SetDisplayInfo(info);
    }
    FriendlyBillboards.EndProcessing();

    // Process enemies
    // Enemy billboards will show up between a certain direction range: (-5, 5) degrees

    EnemyBillboards.BeginProcessing();
    for (unsigned i=0; i < enemies.GetSize(); i++)
    {
        info.Clear();
        if (EnemyBillboards.GetUnusedBillboardMovieclip(enemies[i].pPlayer, &pbb))
        {               
            pbb->Billboard.GotoAndPlay("show");                       
            String title;

//            title = "TODO";
//*
            Format(title, "<img src='rank{0}' width='20' height='20' align='baseline' vspace='-7'/> {1}", 
                enemies[i].pPlayer->GetRank()+1, enemies[i].pPlayer->GetName());
//*/
            pbb->Textfield.SetTextHTML(title);            
        }
        // Load player info
        info.SetVisible(enemies[i].bShowLabel);
        pbb->Textfield.SetDisplayInfo(info);
        info.Clear();
        info.SetVisible(true);
        info.SetX(enemies[i].X);
        info.SetY(enemies[i].Y);
        info.SetScale(enemies[i].Scale, enemies[i].Scale);
        pbb->Billboard.SetDisplayInfo(info);

    }
    EnemyBillboards.EndProcessing();
}

FxHUDView::BillboardCache::~BillboardCache()
{   
	// Remove any remaining data and movieClip pointers.
    BillboardMC* pbb = UnusedBillboards.GetFirst();
    while (!UnusedBillboards.IsNull(pbb))
    {
        BillboardMC* next = UnusedBillboards.GetNext(pbb);
        UnusedBillboards.Remove(pbb);
        delete pbb;
        pbb = next;
    }

    pbb = UsedBillboards.GetFirst();
    while (!UsedBillboards.IsNull(pbb))
    {
        BillboardMC* next = UsedBillboards.GetNext(pbb);
        UsedBillboards.Remove(pbb);
        delete pbb;
        pbb = next;
    }

    SF_ASSERT(TransientBillboards.IsEmpty());
}

void FxHUDView::BillboardCache::BeginProcessing()
{
    // Put used billboards in transient set
    BillboardMC* data = UsedBillboards.GetFirst();
    while (!UsedBillboards.IsNull(data))
    {
        BillboardMC* next = UsedBillboards.GetNext(data);
        UsedBillboards.Remove(data);
        TransientBillboards.PushBack(data);
        data = next;
    }
}

void FxHUDView::BillboardCache::EndProcessing()
{
    // Check transient set for stragglers.
    // These billboards are unused.
    Value::DisplayInfo info(false);
    BillboardMC* data = TransientBillboards.GetFirst();
    while (!TransientBillboards.IsNull(data))
    {
        BillboardMC* next = TransientBillboards.GetNext(data);
        TransientBillboards.Remove(data);
        data->Billboard.SetDisplayInfo(info);
        UnusedBillboards.PushBack(data);
        BillboardOwners.Remove(data->pOwner);
        data = next;
    }
}

bool FxHUDView::BillboardCache::GetUnusedBillboardMovieclip(FxHUDPlayer* pplayer, FxHUDView::BillboardMC** ppbb)
{
    if (CanvasMC.IsUndefined()) return false;

    // Return billboard already owned by player
    BillboardMC** ppowned = BillboardOwners.Get(pplayer);
    if (!ppowned)
    {
        // If no billboard is owned, then get one from the unused list
        // If no unused billboards, create some in the swf
        if (UnusedBillboards.IsEmpty())
        {
            Value::DisplayInfo info(false);
			
			Value billboard, temp;
			String instanceName;
			Format(instanceName, "{0}{1}", BillboardPrefix, BillboardCount++);
			CanvasMC.AttachMovie(&billboard, SymbolLinkage, instanceName);

            BillboardMC* pbb = new BillboardMC();
            pbb->Billboard = billboard;

            billboard.GetMember("label", &temp);
            temp.GetMember("textField", &temp);
            pbb->Textfield = temp;

            pbb->Billboard.SetDisplayInfo(info);
            UnusedBillboards.PushBack(pbb);
           
        }
        BillboardMC *pbb = UnusedBillboards.GetLast();
        UnusedBillboards.Remove(pbb);
        UsedBillboards.PushBack(pbb);

        pbb->pOwner = pplayer;
        BillboardOwners.Add(pplayer, pbb);

        *ppbb = pbb;
        return true;
    }
    else 
    {
        *ppbb = *ppowned;
        TransientBillboards.Remove(*ppbb);
        UsedBillboards.PushBack(*ppbb);
    }

    return false;
}

