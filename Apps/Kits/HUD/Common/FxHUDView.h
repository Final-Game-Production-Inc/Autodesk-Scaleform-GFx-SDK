/**************************************************************************

Filename    :   FxHUDView.h
Content     :   HUD View types
Created     :   December 13, 2009
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxHUDInterface_H
#define INC_FxHUDInterface_H

#include "Kernel/SF_List.h"
#include "GFx/GFx_Player.h"

#include "FxGameDelegate.h"
#include "FxMinimapView.h"
#include "FxHUDKit.h"


namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;


//
// The following classes are used by the HUD view. 
// They represent the HUD view itself (the movieclip representing the HUD),
// the HUD event log, the HUD event messages and the billboard cache.
//

//////////////////////////////////////////////////////////////////////////


//
// Message class used by the FxHUDLog.
//
class FxHUDMessage : public NewOverrideBase<Stat_Default_Mem>, public ListNode<FxHUDMessage>
{
public:
    class Movieclip : public NewOverrideBase<Stat_Default_Mem>, public ListNode<Movieclip>
    {
    public:
        Movieclip(Value mc) : MC(mc) {}
        Value&   GetRef() { return MC; }
        void        SetVisible(bool flag) 
        {
            Value::DisplayInfo info(flag);
            MC.SetDisplayInfo(info);
        }
    private:
        Value    MC;
    };

    FxHUDMessage(const String& msg, Movieclip* pm) 
        : pMovieclip(pm)
    {
        Value tf;
        pm->GetRef().GetMember("message", &MessageMC);
        MessageMC.GetMember("textField", &tf);
        tf.SetTextHTML(msg.ToCStr());
        pm->GetRef().GotoAndPlay("show");

#ifdef SF_BUILD_DEBUG
        Message = msg;
#endif
    }

#ifdef SF_BUILD_DEBUG    
    const String&  GetMessage() const      { return Message; }
#endif
    Movieclip*      GetMovieclip() const        { return pMovieclip; }
    
    bool        IsAlive() const     
    { 
        Value::DisplayInfo info;
        MessageMC.GetDisplayInfo(&info);
        return (info.GetAlpha() > 0);
    }

private:
#ifdef SF_BUILD_DEBUG
    String     Message;
#endif
    Movieclip*  pMovieclip;
    Value    MessageMC;
};


class FxHUDLog
{
public:
    FxHUDLog() : NumMessages(0) {}
    ~FxHUDLog();

    void    SetLogMovieclip(Value& mc)   
    { 
        LogMC = mc; 
        Value::DisplayInfo info;
        LogMC.GetDisplayInfo(&info);
        LogOriginalY = info.GetY();
    }

    void    Process(FxHUDEvent* pevent);
    void    Update();

private:
    List<FxHUDMessage>              Log;    
    unsigned                        NumMessages;

    Value                           LogMC;
    GFx::Double                     LogOriginalY;
    List<FxHUDMessage::Movieclip>   MessageMCs;

    FxHUDMessage::Movieclip*        GetUnusedMessageMovieclip();
};


//////////////////////////////////////////////////////


class FxHUDView : public FxDelegateHandler, public FxHUDEventListener
{
public:
    struct Config
    {
        // HUD movieclip
        Value           HUDMC;
		Movie*			pMovie;
    };

    // Stat collection structure used for testing purposes
    struct Stats
    {
        UInt64                  UpdateTime;
        FxMinimapView::Stats    MinimapStats;

        Stats() : UpdateTime(0) {}
    };

	struct ViewState
	{
		unsigned					ScoreBlue;
		unsigned					ScoreRed;

		unsigned					AmmoInClip;
		unsigned					AmmoClips;
		unsigned					AmmoClipSize;
		unsigned					Grenades;
		FxHUDPlayer::Weapon		    WeaponType;

		float					    Health;
		float					    Exp;
		unsigned					Rank;
		unsigned					Sec;
	};

    FxHUDView(const Config& config);
    virtual ~FxHUDView();

    // FxDelegateHandler implementation
    virtual void        Accept(FxDelegateHandler::CallbackProcessor *cbreg);
        
    void                RegisterMinimap(FxMinimapView::Config& config);
    //void			    ToggleMinimapControls(bool toggle);

    FxMinimapView*      GetMinimapView() const      { return pMinimapView; }
	void				ToggleMinimapLock();

    // Methods to update the view
    void                UpdateControls();
    void                UpdateView(FxHUDEnvironment* penv, Movie* pmovie, Stats* pstats = NULL);
    
	virtual void		OnEvent(FxHUDEvent *pevent);

	void				Stop()	{ Paused = true; }
	void				Start() { Paused = false; }

protected:
    Value            HUDMC;			// HUD movieClip
    Value            MinimapMC;		// Minimap movieClip

	// Stored movieClip (Value) references used by update logic
    Value            TeamStatsMC;	// Team stats movieClip
	Value			PlayerStatsMC;	// Player stats movieClip
	Value			DirHitMC;		// Direction hit indicator container movieClip
	Value			RoundTimeMC;	// Roundtime movieClip

	Value			ScoreRedMC, TeamRedMC, ScoreBlueMC, TeamBlueMC, RedWinningMC, BlueWinningMC;	// Within TeamStatsMC

	Value			RankMC, ExpBarMC;	// Within HUDMC

	Value			HealthNMC, HealthMC, AmmoNMC, ClipNMC;	// Within PlayerStatsMC
	Value			AmmoMC, AmmoBGMC, GrenadeMC, ReloadMC, WeaponMC;

	Value			FlagMC, FlagBarMC, FlagArrowMC;	// Within HUDMC

	Value			PopupNumberMC, PopupMC, PTextFieldMC;	// Within HUDMC
	Value			CenterTextMC, CenterText, TextFieldMC;

	// Directional hit indicator movieClips within DirHitMC.
	Value			DirMC_T, DirMC_TL, DirMC_L, DirMC_BL, DirMC_B, DirMC_BR, DirMC_R, DirMC_TR;

	// Reticule movieClips within HUDMC.
	Value			ReticuleMC_Top, ReticuleMC_Bottom, ReticuleMC_Right, ReticuleMC_Left;

	// HUD Minimap object
    FxMinimapView::Config   MinimapViewConfig;
    Ptr<FxMinimapView>     pMinimapView;

	// HUD Log object
    FxHUDLog			Log;

	// Used for the animation logic
    UInt64              LastTicks;
	bool				Paused;

	ViewState			State;

    // Flags to be used by tutorial, currently unimplemented.
    bool				bWhatObjective;
    bool				bWhatPowerups;
    bool				bHowShoot;
    bool				bHowReload;
    bool				bHowChangeWeapon;
	
	// Core HUD View update logic.
	void				UpdateState(FxHUDEnvironment *penv);
	void				ResetState();

	// Update logic divided by movieClip / type.
    void                UpdateTeamStats(FxHUDEnvironment *penv);
	void				UpdatePlayerEXP(FxHUDEnvironment *penv);
	void				UpdatePlayerStats(FxHUDEnvironment *penv);
	void				UpdateEvents(FxHUDEnvironment *penv);
	void				UpdateTutorial(FxHUDEnvironment *penv);

	// Returns the movie clip frame with the respective weapon icon.
    unsigned				GetWeaponFrame(FxHUDPlayer::Weapon w);

	// Helper method for changing movieClips visibility.
	void				SetVisible(Value* mc, bool visible);

    bool                bFirstUpdate;
	bool				bReloadMCVisible;
	bool				bFlagMCVisible;

    bool                bForceUpdate;

	float				ReticleHeat;
	float				ReticleFiringOffset;

    const char*			WinningText;
	const char*			LosingText;
	String				RespawnText; 

	FxMinimapView::Config	origMMConfig;

	unsigned				MaxScore;
	unsigned				MessageCount;
	unsigned				FullRoundTime; // Used for tutorial updates.

    //////////////////////////////////////////////////////////////////////////
	struct BillboardMC : public NewOverrideBase<Stat_Default_Mem>, public ListNode<BillboardMC>
	{
		FxHUDPlayer*    pOwner;
		Value        Billboard;
		Value        Textfield;
	};
 
    class BillboardCache
    {
    public:
		BillboardCache() : BillboardCount(0) {}
        ~BillboardCache();
        void                Configure(Value& canvas, const char* linkage, const char* prefix)
        {
            CanvasMC = canvas;
			SymbolLinkage = linkage;
            BillboardPrefix = prefix;
			BillboardCount = 0;
        }
        void                BeginProcessing();
		void			    CreateBillboards(Value* billboards, unsigned count);
        bool                GetUnusedBillboardMovieclip(FxHUDPlayer* pplayer, BillboardMC** ppbb);
        void                EndProcessing();

    private:
        List<BillboardMC>                  UnusedBillboards;
        List<BillboardMC>                  UsedBillboards;
        List<BillboardMC>                  TransientBillboards;
        Hash<FxHUDPlayer*, BillboardMC*>   BillboardOwners;
		
        Value        CanvasMC;
        String         BillboardPrefix;
		String			SymbolLinkage;
		unsigned			BillboardCount;
    };    

    BillboardCache      FriendlyBillboards;
    BillboardCache      EnemyBillboards;

    void                UpdateBillboards(const Array<FxHUDEnvironment::BillboardQueryResult>& friendlies,
										 const Array<FxHUDEnvironment::BillboardQueryResult>& enemies);    

    //////////////////////////////////////////////////////////////////////////
};

#endif  // INC_FxHUDInterface_H
