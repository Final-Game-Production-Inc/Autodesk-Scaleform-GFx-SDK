/**************************************************************************

Filename    :   WiiUDemo.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "WiiUDemo.h"

// Scaleform Includes
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_Std.h"

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Render.h"
#include "Render/Render_ImageFiles.h"
#include "Render/Render_TessGen.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/ImageFiles/TGA_ImageFile.h"
#include "Render/ImageFiles/GTX_ImageFile.h"

#include "GFx/AS3/AS3_Global.h"

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;
using GFx::Event;

SF_PLATFORM_SYSTEM_APP(WiiUDemo, Scaleform::GFx::System, WiiUDemo);

WiiUDemo* WiiUDemo::pApp = 0;

WiiUDemo::WiiUDemo()
{
    pApp = this;
#if defined(GFX_ENABLE_SOUND)
    pSoundSystem = 0;
#endif
    ExitTimeout = 0;

    AccumClockTicks = 0;
    PrevTicks       = 0;
    LastFPSUpdatedTicks  = 0;
    LastStatUpdatedTicks = 0;
        
    LastFPS         = 0.0f;
    AccumFPS        = 0.0f;
    FrameCounter    = 0;
    AccumFPSSecondCount = 0;
    LastAdvanceTicks= 0;
}

class FxPlayerFSCommandHandler : public FSCommandHandler
{
public:
    virtual void Callback(Movie* pmovie, const char* pcommand, const char* parg)
    {
        if (WiiUDemo::pApp)
            WiiUDemo::pApp->FsCommand(pmovie, pcommand, parg);
    }
};

void WiiUDemo::FsCommand(Movie* pmovie, const char* pcommand, const char* parg)
{
}

void WiiUDemo::InitArgDescriptions(Args* args)
{
    BaseClass::InitArgDescriptions(args);
    ArgDesc options []=
    {
        //      {"","--------------spacer example------------------\n","",FxCmdOption::Spacer,""},
        {"t",   "ExitTimeout",         Args::FloatOption, "0.0",
        "<sec>    Timeout and exit after the specified number of seconds."},
        {"","",Args::ArgEnd,"",""},
    };
    args->AddDesriptions(options);
}

bool WiiUDemo::OnArgs(const Platform::Args& args, Platform::Args::ParseResult parseResult)
{
    ExitTimeout =     args.GetFloat("ExitTimeout");
    return true;   
}

bool WiiUDemo::LoadMovie(int window, const char *FileName)
{
    if (mLoader.GetMovieInfo(FileName, &mMovieInfo))
    {
        Ptr<MovieDef> pMovieDef = *mLoader.CreateMovie(FileName, Loader::LoadAll|Loader::LoadWaitFrame1);
        if (pMovieDef)
        {
            Ptr<Movie> pMovie = *pMovieDef->CreateInstance(false);
            if (pMovie)
            {
                // implicitly disable action error reporting
                //Ptr<ActionControl> pactionControl = *new ActionControl();
                //pactionControl->SetActionErrorSuppress(true);
                //pMovie->SetActionControl(pactionControl);

                pMovie->SetViewport(Windows[window].mViewport);
                pMovie->SetMouseCursorCount(window ? 1 : 4);

                pRenderThread->AddDisplayHandle(pMovie->GetDisplayHandle(), Platform::RenderThread::DHCAT_Normal, 
                                                false, 0, Windows[window].pWindow);

                // Set interval for app to call AdvanceAndDisplay
                SetFrameTime(1.0f / pMovieDef->GetFrameRate());

                Windows[window].pMovieDef = pMovieDef;
                Windows[window].pMovie = pMovie;
                if (pHudDataProvider)
                {
                    pHudDataProvider->SetMovie(window, pMovie);
                }
                return true;
            }
        }
    }

	if (pHud)
	{
		pHud->ClearMsgText();
		pHud->Update();
	}

    SF_DEBUG_WARNING1(1, "Failed to load %s", FileName);
    return false;
}

void WiiUDemo::Window::UpdateViewport(const Platform::ViewConfig& vc)
{
    mViewport = GFx::Viewport(vc.ViewSize.Width, vc.ViewSize.Height, 0,0, vc.ViewSize.Width, vc.ViewSize.Height, vc.Orientation);
}

class XiHandler : public ExternalInterface
{
public:
    XiHandler(WiiUDemo* app) : TheApp(app) {}

    virtual void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount)
    {
        if (!SFstrcmp(methodName, "selectMap"))
        {
            SF_DEBUG_ASSERT1(argCount == 1, "Arg count is not == 1. It is %d.\n", argCount);
            SF_DEBUG_ASSERT(args[0].IsString(), "Arg[0] is not a string\n");
            WorldLocation* loc = TheApp->CassienWorld.GetLocation(args[0].GetString());
            SF_DEBUG_ASSERT1(loc, "Loc was not found (tried name '%s').\n", args[0].GetString());
            printf("Selecting loc %s\n", args[0].GetString());
            Value args[3];
            args[0].SetString(loc->Name.ToCStr());
            args[1].SetString(loc->Description.ToCStr());
            args[2].SetString(WorldLocation::ToString(loc->Type));
            TheApp->LocationView.Invoke("setLocation", NULL, args, 3);
        }        
        else if (!SFstrcmp(methodName, "getFriendlies"))
        {
            Value arr;
            pmovieView->CreateArray(&arr);
            pmovieView->SetExternalInterfaceRetVal(arr);
            
            Party* friendlies = TheApp->CassienBattle->GetFriendlies();
            SF_DEBUG_ASSERT(friendlies, "Friendlies invalid\n");
            for (unsigned i = 0; i < friendlies->GetNumMembers(); i++)
            {
                Value obj;
                pmovieView->CreateObject(&obj);
                Player* member = friendlies->GetMember(i);
                SF_DEBUG_ASSERT(member, "Friendly player is invalid\n");

                obj.SetMember("id", member->GetId());
                obj.SetMember("type", member->GetClass());

                arr.PushBack(obj);
            }
        }
        else if (!SFstrcmp(methodName, "getEnemies"))
        {
            Value arr;
            pmovieView->CreateArray(&arr);
            pmovieView->SetExternalInterfaceRetVal(arr);

            Party* enemies = TheApp->CassienBattle->GetEnemies();
            SF_DEBUG_ASSERT(enemies, "Enemies invalid\n");
            for (unsigned i = 0; i < enemies->GetNumMembers(); i++)
            {
                Value obj;
                pmovieView->CreateObject(&obj);
                Player* member = enemies->GetMember(i);
                SF_DEBUG_ASSERT(member, "Enemy player is invalid\n");

                obj.SetMember("id", member->GetId());
                obj.SetMember("type", member->GetClass());

                arr.PushBack(obj);
            }
        }
        else if (!SFstrcmp(methodName, "getInfo"))
        {
            Value obj;
            pmovieView->CreateObject(&obj);
            pmovieView->SetExternalInterfaceRetVal(obj);

            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            obj.SetMember("id", (Double)player->GetId());
            obj.SetMember("name", player->GetName());
            obj.SetMember("type", player->GetClass());
            obj.SetMember("level", (Double)player->GetLevel());
            obj.SetMember("hp", (Double)player->GetHP());
            obj.SetMember("maxhp", (Double)player->GetMaxHP());
            obj.SetMember("sp", (Double)player->GetSP());
            obj.SetMember("maxsp", (Double)player->GetMaxSP());
        }
        else if (!SFstrcmp(methodName, "getActions"))
        {
            SF_DEBUG_MESSAGE(1, "getActions\n");

            Value arr;
            pmovieView->CreateArray(&arr);
            pmovieView->SetExternalInterfaceRetVal(arr);

            SF_DEBUG_ASSERT(argCount == 1, "setTarget argCount != 1\n");
            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            for (unsigned i = 0; i < NumCommandSlots; i++)
            {
                Value obj;
                pmovieView->CreateObject(&obj);
                //SF_DEBUG_ASSERT2(player->GetCommand((CommandSlot)i), "Command %d for %s is invalid!\n", i, player->GetName());
                if (player->GetCommand((CommandSlot)i) != NULL)
                {
                    Command* cmd = player->GetCommand((CommandSlot)i);
                    obj.SetMember("name", cmd->GetName());
                    obj.SetMember("disabled", cmd->IsDisabled());
                }
                arr.PushBack(obj);
            }
        }
        else if (!SFstrcmp(methodName, "getSelectedAction"))
        {
            SF_DEBUG_MESSAGE(1, "getSelectedActions\n");

            SF_DEBUG_ASSERT(argCount == 1, "setTarget argCount != 1\n");
            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            pmovieView->SetExternalInterfaceRetVal(player->GetSelectedCommandIdx());
        }
        else if (!SFstrcmp(methodName, "setSelectedAction"))
        {
            SF_DEBUG_MESSAGE(1, "setSelectedAction\n");

            SF_DEBUG_ASSERT(argCount == 2, "setTarget argCount != 2\n");
            pmovieView->SetExternalInterfaceRetVal(false);
            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            player->SetSelectedCommandIdx(args[1].GetNumber());
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "getTarget"))
        {
            SF_DEBUG_MESSAGE(1, "getTarget\n");

            SF_DEBUG_ASSERT(argCount == 1, "getTarget argCount != 1\n");
            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            Player* target = TheApp->CassienBattle->FindPlayer(player->GetTargetID());
            if (!target) return;
            pmovieView->SetExternalInterfaceRetVal((Double)target->GetId());
        }
        else if (!SFstrcmp(methodName, "setTarget"))
        {
            SF_DEBUG_MESSAGE(1, "setTarget\n");

            SF_DEBUG_ASSERT(argCount == 2, "setTarget argCount != 2\n");
            pmovieView->SetExternalInterfaceRetVal(false);
            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            if (!player) return;
            Player* newtarget = TheApp->CassienBattle->FindPlayer(args[1].GetNumber());
            player->SetTargetID(newtarget->GetId());
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "selectPlayer"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            printf("selecting player\n");

            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            Value id = args[0];
            TheApp->BattleScreen.Invoke("selectHero", NULL, &id, 1);

            Player* target = TheApp->CassienBattle->FindPlayer(player->GetTargetID());
            id = target->GetId();
            if (!TheApp->CassienBattle->IsInFriendlyParty(target)) {
                TheApp->BattleScreen.Invoke("selectEnemyTarget", NULL, &id, 1);
            } else {
                TheApp->BattleScreen.Invoke("selectFriendTarget", NULL, &id, 1);
            }

            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "selectTarget"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            printf("selecting target\n");

            Player* player = TheApp->CassienBattle->FindPlayer(args[0].GetNumber());
            Value id = args[0];
            if (!TheApp->CassienBattle->IsInFriendlyParty(player)) {
                TheApp->BattleScreen.Invoke("selectEnemyTarget", NULL, &id, 1);
            } else {
                TheApp->BattleScreen.Invoke("selectFriendTarget", NULL, &id, 1);
            }

            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "executeRound"))
        {
            SF_DEBUG_MESSAGE(1, "executeRound\n");

            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(TheApp->CassienBattle);
            TheApp->CassienBattle->Execute();
            pmovieView->SetExternalInterfaceRetVal(true);

            // TODO: Delay if TVScreen is running an animation?
            Value arg;
            arg.SetNumber(TheApp->CassienBattle->GetRoundCount());
            TheApp->BattleUI.Invoke("processBattleRoundStart", NULL, &arg, 1);
            TheApp->BattleScreen.Invoke("startBattleRound");
        }
        else if (!SFstrcmp(methodName, "forceBattleOver"))
        {
            SF_DEBUG_MESSAGE(1, "battleOver\n");

            Value args[2];
            args[0].SetBoolean(TheApp->CassienBattle->GetCurrentState() == Battle::STATE_FriendlyWin);
            args[1].SetBoolean(TheApp->CassienBattle->GetCurrentState() == Battle::STATE_EnemyWin);
            TheApp->BattleUI.Invoke("processBattleEnd", NULL, args, 2);
            printf("FORCE closing battle screen..\n");
            TheApp->TvScreenMC.Invoke("closeBattleScreen");
            printf("\tdone\n");
        }
        else if (!SFstrcmp(methodName, "roundOver"))
        {
            SF_DEBUG_MESSAGE(1, "roundOver\n");

            TheApp->CassienBattle->CheckEndgame();

            if (TheApp->CassienBattle->GetCurrentState() == Battle::STATE_Active)
                TheApp->BattleUI.Invoke("processBattleRoundEnd");
            else
            {
                Value args[2];
                args[0].SetBoolean(TheApp->CassienBattle->GetCurrentState() == Battle::STATE_FriendlyWin);
                args[1].SetBoolean(TheApp->CassienBattle->GetCurrentState() == Battle::STATE_EnemyWin);
                TheApp->BattleUI.Invoke("processBattleEnd", NULL, args, 2);
                printf("closing battle screen..\n");
                TheApp->TvScreenMC.Invoke("closeBattleScreen");
                printf("\tdone\n");
            }
        }
        else if (!SFstrcmp(methodName, "attackStart"))
        {
            SF_DEBUG_MESSAGE(1, "attackStart\n");

            TheApp->BattleUI.Invoke("processBattleRoundCast");
        }
        else if (!SFstrcmp(methodName, "attackOver"))
        {
            SF_DEBUG_MESSAGE(1, "attackOver\n");

            SF_DEBUG_MESSAGE(1, "--next command\n");
            TheApp->CassienBattle->NextCommand();
        }
        else if (!SFstrcmp(methodName, "hitOver"))
        {
            SF_DEBUG_MESSAGE(1, "hitOver\n");

            TheApp->BattleUI.Invoke("processBattleRoundHit");

            SF_DEBUG_MESSAGE(1, "--next result\n");
            TheApp->CassienBattle->NextResult();
        }
        else if (!SFstrcmp(methodName, "getNextBattleCommand"))
        {
            SF_DEBUG_MESSAGE(1, "getNextBattleCommand\n");
            BattleCommand cmd;
            bool ok = TheApp->CassienBattle->GetActiveCommand(&cmd);
            Value arg;
            
            SF_DEBUG_MESSAGE(!ok, "NO COMMANDS LEFT\n");
            if (ok)
            {
                pmovieView->CreateObject(&arg);
                arg.SetMember("srcId", cmd.pSrc->GetId());
                arg.SetMember("srcIsFriendly", TheApp->CassienBattle->IsInFriendlyParty(cmd.pSrc));
                arg.SetMember("targetId", cmd.pTarget->GetId());
                arg.SetMember("targetIsFriendly", TheApp->CassienBattle->IsInFriendlyParty(cmd.pTarget));
                arg.SetMember("command", cmd.pCmd->GetName());
                arg.SetMember("deltaSP", -((Double)cmd.pCmd->GetConsumedSP()));
            }

            pmovieView->SetExternalInterfaceRetVal(arg);
        }
        else if (!SFstrcmp(methodName, "executeBattleCommand"))
        {
            SF_DEBUG_MESSAGE(1, "executeBattleCommand\n");
            TheApp->CassienBattle->ExecuteActiveCommand();            
        }
        else if (!SFstrcmp(methodName, "getNextBattleResult"))
        {
            SF_DEBUG_MESSAGE(1, "getNextBattleResult\n");

            Value robj;

            BattleResult* result = TheApp->CassienBattle->GetCurrentResult();
            SF_DEBUG_MESSAGE(!result, "NO RESULTS LEFT\n");
            if (result)
            {
                printf("#### %s -> %s\n", result->pSrc->GetName(), result->pTarget->GetName());
                pmovieView->CreateObject(&robj);
                robj.SetMember("srcId", result->pSrc->GetId());
                robj.SetMember("srcIsFriendly", TheApp->CassienBattle->IsInFriendlyParty(result->pSrc));
                robj.SetMember("targetId", result->pTarget->GetId());
                robj.SetMember("targetIsFriendly", TheApp->CassienBattle->IsInFriendlyParty(result->pTarget));
                robj.SetMember("deltaHP", (Double)result->DeltaTargetHP);
                robj.SetMember("isKO", result->pTarget->IsKO());
                robj.SetMember("isCrit", result->bCrit);
                robj.SetMember("isMiss", result->bMiss);
                robj.SetMember("isHeal", result->bHeal);
            }

            pmovieView->SetExternalInterfaceRetVal(robj);
        }
        else if (!SFstrcmp(methodName, "startBattle"))
        {           
            pmovieView->SetExternalInterfaceRetVal(false);
            printf("starting battle\n");
            Ptr<Player> h1 = *SF_NEW Swordsman(1, "Garrett Banner", 9);
            h1->Initialize();
            Ptr<Player> h2 = *SF_NEW Mage(2, "Calias Elden", 8);
            h2->Initialize();
            Ptr<Player> h3 = *SF_NEW Priest(3, "Mia Kessel", 7);
            h3->Initialize();
            Ptr<Player> e1 = *SF_NEW AlmeidaVargas(4);
            e1->Initialize();
            Ptr<Player> e2 = *SF_NEW Undead(5, "Ghoul A", 6);
            e2->Initialize();
            Ptr<Player> e3 = *SF_NEW Undead(6, "Ghoul B", 6);
            e3->Initialize();
            Ptr<Party> hp = *SF_NEW Party();
            hp->AddMember(h1);
            hp->AddMember(h2);
            hp->AddMember(h3);
            Ptr<Party> ep = *SF_NEW Party();
            ep->AddMember(e1);
            ep->AddMember(e2);
            ep->AddMember(e3);
            TheApp->CassienBattle = *SF_NEW Battle(hp, ep);
            pmovieView->SetExternalInterfaceRetVal(true);

            printf("opening battle screen..\n");
            TheApp->TvScreenMC.Invoke("openBattleScreen");
            printf("\tdone\n");
        }
        else if (!SFstrcmp(methodName, "registerDrcScreen"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(argCount == 1);
            SF_ASSERT(args[0].IsDisplayObject());
            TheApp->DrcScreenMC = args[0];
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "registerBattleUI"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(argCount == 1);
            SF_ASSERT(args[0].IsDisplayObject());
            TheApp->BattleUI = args[0];
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "registerTvScreen"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(argCount == 1);
            SF_ASSERT(args[0].IsDisplayObject());
            TheApp->TvScreenMC = args[0];
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "registerLocationView"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(argCount == 1);
            SF_ASSERT(args[0].IsDisplayObject());
            TheApp->LocationView = args[0];
            pmovieView->SetExternalInterfaceRetVal(true);
        }
        else if (!SFstrcmp(methodName, "registerBattleScreen"))
        {
            pmovieView->SetExternalInterfaceRetVal(false);
            SF_ASSERT(argCount == 1);
            SF_ASSERT(args[0].IsDisplayObject());
            TheApp->BattleScreen = args[0];
            pmovieView->SetExternalInterfaceRetVal(true);
        }
    }

protected:
    WiiUDemo*    TheApp;
};

bool WiiUDemo::OnInit(Platform::ViewConfig& config)
{
    CassienWorld.AddLocation(WorldLocation("Rockville", "A sleepy town and home to our heroes.", MAP_Town, false, true));
    CassienWorld.AddLocation(WorldLocation("Potomac Keep", "Run down fort, crawling with bandits.", MAP_Castle, false, true));
    CassienWorld.AddLocation(WorldLocation("Forest Glen", "A pristine forest with many friendly creatures.", MAP_Forest, false, true));
    CassienWorld.AddLocation(WorldLocation("Friendship Heights", "Farmland surround this charming town.", MAP_Town, false, true));
    CassienWorld.AddLocation(WorldLocation("Tacoma Park", "Well maintained garden of the Kyllian nobility.", MAP_Forest, false, true));
    CassienWorld.AddLocation(WorldLocation("Strathmore Castle", "The Duke of Strathmore resides here.", MAP_Castle, false, true));
    CassienWorld.AddLocation(WorldLocation("Laurel", "The locals are not too friendly.", MAP_Town, false, true));
    CassienWorld.AddLocation(WorldLocation("Eastern Shore", "Many fossils litter the beachfront.", MAP_Ruins, false, true));
    CassienWorld.AddLocation(WorldLocation("Deep Creek", "Excellent fishing spot, if you can handle the fish.", MAP_Forest, false, false));
    CassienWorld.AddLocation(WorldLocation("Skyline Pass", "Falcons perch ominously on top of the rocks.", MAP_Pass, true, false));
    CassienWorld.AddLocation(WorldLocation("Merriweather", "Entertainment capital of the land.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Wolf Trap Farm", "Abandoned farm, now haunted by evil pumpkins.", MAP_Ruins, true, false));
    CassienWorld.AddLocation(WorldLocation("Great Falls", "Beautiful waterfall, but what's behind it?", MAP_Forest, true, false));
    CassienWorld.AddLocation(WorldLocation("Tudor Hall", "The Earl of Tudor is friendly, but only during the day.", MAP_Castle, true, false));
    CassienWorld.AddLocation(WorldLocation("Arlington", "Many merchant vessels litter the horizon.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Rose Hill Manor", "The Duchess is looking for suitors. Are you up for it?", MAP_Castle, true, false));
    CassienWorld.AddLocation(WorldLocation("Cabin John Forest", "A great wolf reigns over this forest.", MAP_Forest, true, false));
    CassienWorld.AddLocation(WorldLocation("Annapolis", "Home of the Kyllion military academy.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Wheaton", "Shanty town of immigrants and day laborers.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Glen Echo Cemetery", "Rumors of a skeleton army circle this place.", MAP_Ruins, true, false));
    CassienWorld.AddLocation(WorldLocation("Black Hill Forest", "Iron miners love this place.", MAP_Forest, true, false));
    CassienWorld.AddLocation(WorldLocation("Seneca Creek", "Watch out for flash flooding!", MAP_Forest, true, false));
    CassienWorld.AddLocation(WorldLocation("Kensington", "The antiques industry thrives here.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Greenbrier Meadow", "Flowers galore in every imaginable color.", MAP_Forest, true, false));
    CassienWorld.AddLocation(WorldLocation("Catoctin Canyon", "Falling rocks can be hazardous.", MAP_Pass, true, false));
    CassienWorld.AddLocation(WorldLocation("Sugarloaf Mountain", "Many dwarves call this place home.", MAP_Pass, true, false));
    CassienWorld.AddLocation(WorldLocation("Aberdeen Harbor", "Why is the water green?", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Walker Mill", "Sleepy harbor town that survived the last great war.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Fairland", "Home to the Great Cathedral, now occupied by the Kyllion Empire.", MAP_Town, true, false));
    CassienWorld.AddLocation(WorldLocation("Patuxent Ruins", "Rumor has it that these ruins hold a weapon of great power.", MAP_Ruins, true, false));
    CassienWorld.AddLocation(WorldLocation("Gambrill Estate", "Abandoned home of a very powerful wizard.", MAP_Ruins, true, false));
    CassienWorld.AddLocation(WorldLocation("Fort Washington", "Powerful stronghold of the Kyllion Empire.", MAP_Castle, true, false));

    ButtonDown = TouchDown = 0;
    Paused = 0;
    EdgeAA = 1;
    MeasurePerformance = 0;
    FastForward = 0;
    Wireframe = 0;
    //ProfileMode = FxRenderThread::Profile_None;
    Width = config.ViewSize.Width;
    Height = config.ViewSize.Height;

    Ptr<ThreadedTaskManager> pTaskManager = *new ThreadedTaskManager;
    mLoader.SetTaskManager(pTaskManager);

    Ptr<FSCommandHandler> pcommandHandler = *new FxPlayerFSCommandHandler;
    mLoader.SetFSCommandHandler(pcommandHandler);

    mLoader.SetExternalInterface(this);

    SF::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new GFx::ImageFileHandlerRegistry();
#ifdef SF_ENABLE_LIBJPEG
    pimgReg->AddHandler(&SF::Render::JPEG::FileReader::Instance);
#endif
#ifdef SF_ENABLE_LIBPNG
    pimgReg->AddHandler(&SF::Render::PNG::FileReader::Instance);
#endif
    pimgReg->AddHandler(&SF::Render::TGA::FileReader::Instance);
	pimgReg->AddHandler(&SF::Render::GTX::FileReader::Instance);
    mLoader.SetImageFileHandlerRegistry(pimgReg);

#if defined(GFX_ENABLE_SOUND)
    SoundVolume = 1.0;
    IsMute = false;
    InitializeSound();
    Sound::SoundRenderer* psoundRenderer = GetSoundRenderer();
    if (psoundRenderer)
    {
        Ptr<Audio> paudioState = *new Audio(psoundRenderer);
        mLoader.SetAudio(paudioState);
        psoundRenderer->SetMasterVolume(SoundVolume);
        psoundRenderer->Mute(IsMute);
    }
#endif // GFX_ENABLE_SOUND

#ifdef GFX_AS2_SUPPORT	
    Ptr<ASSupport> pAS2Support = *new AS2Support();
    mLoader.SetAS2Support(pAS2Support);
#endif
#ifdef GFX_AS3_SUPPORT	
    Ptr<ASSupport> pAS3Support = *new AS3Support();
    mLoader.SetAS3Support(pAS3Support);
#endif

    OnInitHelper(config, "WiiUDemo");

    Windows[0].pWindow = GetDeviceWindow();
    Windows[0].UpdateViewport(config);

    //pHudWindow = 
    Windows[1].pWindow = pDevice->CreateGraphicsWindow(1, Platform::ViewConfig());
    Platform::ViewConfig hwconfig;
    Windows[1].pWindow->GetViewConfig(&hwconfig);
    Windows[1].UpdateViewport(hwconfig);

    GlyphCacheParams fontCacheConfig;
    fontCacheConfig.TextureWidth   = 1024;
    fontCacheConfig.TextureHeight  = 1024;
    fontCacheConfig.NumTextures = 1;
    //Temporarily changing value from 32 too 100 as Vector Fonts aren't supported yet
    //If the value is kept at 32, then large fonts will be blurry
    fontCacheConfig.MaxSlotHeight  = 100;
    //fontCacheConfig.SlotPadding    = 2;

    pRenderThread->SetFontCacheConfig(fontCacheConfig);

    SF::Ptr<GFx::ImageCreator> pimageCreator = *SF_NEW GFx::ImageCreator(pRenderThread->GetTextureManager());
    mLoader.SetImageCreator(pimageCreator);

    // Load the movies.
    if (!LoadMovie(0, "/vol/content/WiiUDemo/TVScreen.swf"))
        return 0;
    if (!LoadMovie(1, "/vol/content/WiiUDemo/DRCScreen.swf"))
        return 0;

    SF::Ptr<XiHandler> ptxi = *SF_NEW XiHandler(this);
    Windows[0].pMovie->SetExternalInterface(ptxi);
    Windows[1].pMovie->SetExternalInterface(ptxi);

    // Store initial timing, so that we can determine
    // how much to advance Flash playback.
    LastLoggedFps = MovieLastTime = Timer::GetTicks()/1000;
    FrameCount = -1;

    pHudHeap = Memory::GetGlobalHeap()->CreateHeap("HUD", MemoryHeap::Heap_UserDebug);

    pHudDataProvider = 
        *SF_HEAP_NEW(pHudHeap) FxHUDDataProvider(Windows[0].pMovie, Windows[1].pMovie, pRenderThread, this, IsConsole());

    //if (!GetArgs().GetBool("NoHud") && pHudDataProvider) 
    {
        pHud = *FxHUD::CreateHUD(pHudDataProvider);
        UpdateHUDViewport();
    }

    return 1;
}

void WiiUDemo::OnShutdown()
{
    Windows[0].pMovie = 0;
    Windows[0].pMovieDef = 0;
    Windows[1].pMovie = 0;
    Windows[1].pMovieDef = 0;

#if defined(GFX_ENABLE_SOUND)
    ShutdownSound();
#endif

    pHud.Clear();
    pHudDataProvider.Clear();
    pHudHeap->Release();


    BaseClass::OnShutdown();
}

// Advances GFx animation and draws the scene.
void WiiUDemo::OnUpdateFrame(bool needDraw)
{
    // Update timing.
    UInt64 timer = Timer::GetTicks();
    UInt64 delta = timer - PrevTicks;
    PrevTicks = timer;
    AccumClockTicks += delta;

    // Check to see if exit timeout was hit.
    if ((ExitTimeout > 0.0f) && (AccumClockTicks >= (ExitTimeout * 1000.0f)))
    {
        Shutdown();
        return;
    }

    // Advance time and display the movie.
    if (!Paused)
    {
        float deltaf = delta / 1000000.f;
        UInt64 startAdvanceTicks = Timer::GetProfileTicks();
        Windows[0].pMovie->Advance(deltaf, 0);
        Windows[1].pMovie->Advance(deltaf, 0);
        LastAdvanceTicks = Timer::GetProfileTicks() - startAdvanceTicks;
    }


    pRenderThread->WaitForOutstandingDrawFrame();
    pRenderThread->DrawFrame();

    FrameCounter++;    
    if ((LastFPSUpdatedTicks + 1000000) <= AccumClockTicks)
    {
        LastFPS = 1000000.f * FrameCounter / (float)(AccumClockTicks - LastFPSUpdatedTicks);
        AccumFPS += LastFPS;
        LastFPSUpdatedTicks = AccumClockTicks;
        AccumFPSSecondCount++;
        FrameCounter = 0;
        //if (pRenderThread)
       //     LastDisplayFrames = pRenderThread->GetFrames();
        //UpdateFpsDisplay();
    }

    if (pHud)
    {
        if ((LastStatUpdatedTicks + 1000000/5) <= AccumClockTicks)
        {
            LastStatUpdatedTicks = AccumClockTicks;
            pHud->Update();
            pHudDataProvider->UpdateRendererStats();
            float fps = AccumFPSSecondCount > 0 ? AccumFPS / AccumFPSSecondCount : 0.0f;
            pHudDataProvider->UpdatePerformanceStats(LastAdvanceTicks, pRenderThread->GetLastDisplayTicks(), fps);
        }

    }
}

void WiiUDemo::OnMouseButton(unsigned inputSource, unsigned button, bool downFlag, const Point<int> &pos, KeyModifiers modifiers)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;

    MouseEvent event(downFlag ? GFx::Event::MouseDown : GFx::Event::MouseUp, button, pos.x, pos.y, 0.0f, mouseIndex);
    Windows[windowIndex].pMovie->HandleEvent(event);
}

void WiiUDemo::OnMouseMove(unsigned inputSource, const Point<int> &pos, KeyModifiers modifiers)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;

    MouseEvent event(GFx::Event::MouseMove, 0, pos.x, pos.y, 0.0f, mouseIndex);
    Windows[windowIndex].pMovie->HandleEvent(event);
}

void WiiUDemo::OnMouseWheel(unsigned inputSource, float zdelta,
                                  const Point<int>& pos, KeyModifiers mods)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned mouseIndex = inputSource & Platform::InputController_Mask;

    MouseEvent event(GFx::Event::MouseWheel, 0, pos.x, pos.y, zdelta, mouseIndex);
    Windows[windowIndex].pMovie->HandleEvent(event);
}

void WiiUDemo::OnKey(unsigned inputSource, KeyCode key, unsigned wcharCode, bool down, KeyModifiers modifiers)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned kbIndex = inputSource & Platform::InputController_Mask;

    KeyEvent event(down ? GFx::Event::KeyDown : GFx::Event::KeyUp, key, wcharCode > 128 ? 0 : wcharCode, wcharCode, modifiers, kbIndex);
    Windows[windowIndex].pMovie->HandleEvent(event);
}

void WiiUDemo::OnPad(unsigned controllerIdx, PadKeyCode keyCode, bool downFlag)
{
    switch(keyCode)
    {
    case SF::Pad_Left:
        OnKey(controllerIdx, Key::Left, 0, downFlag, 0);
        return;
    case SF::Pad_Right:
        OnKey(controllerIdx, Key::Right, 0, downFlag, 0);
        return;
    case SF::Pad_Up:
        OnKey(controllerIdx, Key::Up, 0, downFlag, 0);
        return;
    case SF::Pad_Down:
        OnKey(controllerIdx, Key::Down, 0, downFlag, 0);
        return;

    case SF::Pad_L1:
        OnKey(controllerIdx, Key::PageDown, 0, downFlag, 0);
        return;
    case SF::Pad_R1:
        OnKey(controllerIdx, Key::PageUp, 0, downFlag, 0);
        return;

    case SF::Pad_A:
        OnKey(controllerIdx, Key::Return, 0, downFlag, 0);
        return;
    case SF::Pad_B:
        OnKey(controllerIdx, Key::Escape, 0, downFlag, 0);
        return;
    case SF::Pad_X:
        OnKey(controllerIdx, Key::X, 0, downFlag, 0);
        return;
    case SF::Pad_Y:
        OnKey(controllerIdx, Key::Y, 0, downFlag, 0);
        return;

    case SF::Pad_Plus:
        //OnKey(controllerIdx, Key::KP_Add, 0, downFlag, 0);
        if (pHud && !downFlag)
            pHud->ToggleTab(FxHUD::Summary);
        return;
    case SF::Pad_Minus:
        OnKey(controllerIdx, Key::KP_Subtract, 0, downFlag, 0);
        return;
    case SF::Pad_Start:
        OnKey(controllerIdx, Key::KP_Enter, 0, downFlag, 0);
        return;
    case SF::Pad_Select:
        OnKey(controllerIdx, Key::Space, 0, downFlag, 0);
        return;

    default:
        break;
    }
}

void WiiUDemo::OnPadStick(unsigned inputSource, PadKeyCode padCode, float xpos, float ypos)
{
    unsigned windowIndex = inputSource >> Platform::InputWindow_Shift;
    unsigned controllerIdx = inputSource & Platform::InputController_Mask;

    GamePadAnalogEvent event(padCode, xpos, ypos, controllerIdx);
    if (Windows[windowIndex].pMovie)
        Windows[windowIndex].pMovie->HandleEvent(event);
}

void WiiUDemo::Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount)
{

}

void WiiUDemo::UpdateHUDViewport()
{
    if (pHud)
    {
        Render::Size<unsigned> viewSize;//GetViewSize();        
        viewSize.Width = 1280;
        viewSize.Height = 720;
        Render::Viewport hudViewport(viewSize.Width, viewSize.Height,
            int(viewSize.Width * GetSafeArea().Width),
            int(viewSize.Height * GetSafeArea().Height),
            int(viewSize.Width - 2 * viewSize.Width * GetSafeArea().Width),
            int(viewSize.Height - 2 * viewSize.Height * GetSafeArea().Height));

        pHud->SetViewport(hudViewport);
    }
}

void WiiUDemo::GetExtraInfo(FxHUDExtraInfo* pinfo) 
{ 
    pinfo->LastFPS = LastFPS;
    pinfo->TessTriangles = 0; //TessTriangles;
//    pinfo->Zoom = Zoom;   
    pinfo->Paused = Paused;
    pinfo->AppRes = GetViewSize();
//    switch (AAMode) 
//    {
//    case AAMode_None:
//        pinfo->AAMode = "None";
//        break;
//    case AAMode_EdgeAA:
//        // Display a custom message if edge AA is #ifdef-ed out.
//#ifndef SF_NO_FXPLAYER_EDGEAA
//        pinfo->AAMode = "Edge AA";
//        {
//            // Report if EdgeAA is not supported by renderer.
//            //@TODO
////             Renderer::RenderCaps caps;
////             caps.CapBits = 0;
////             if (GetRenderer())
////                 GetRenderer()->GetRenderCaps(&caps);
////             if (!(caps.CapBits & Renderer::Cap_FillGouraud))
////                 pinfo->AAMode = "Edge AA (Not Supported)";
////             else if (!(caps.CapBits & Renderer::Cap_FillGouraudTex))
////                 pinfo->AAMode = "Edge AA (Limited)";
//        }
//#else
//        pinfo->AAMode = "Edge AA [#disabled]";
//#endif
//        break;
//    case AAMode_FSAA:
//        if (ViewConfigHasFlag(Platform::View_FSAA))
//             pinfo->AAMode = "HW FSAA";
//        else
//             pinfo->AAMode = "HW FSAA (Not Supported)";
//        break;
//    }

//    pinfo->CurvePixelError = CurvePixelError;
    

    // Display a custom message if stroke is #ifdef-ed out.
//#ifndef SF_NO_FXPLAYER_STROKER
//    pinfo->StrokeType = "Correct";
//    if (stroke == RenderConfig::RF_StrokeNormal)
//        pinfo->StrokeType = "Normal";
//#else
//    pinfo->StrokeType = "Correct [#disabled]";
//    if (stroke == RenderConfig::RF_StrokeNormal)
//        pinfo->StrokeType = "Normal [#disabled]";
//#endif

    //pinfo->FontConfig = (FontConfigIndex == -1) ? "Default"
    //    : FontConfigs[FontConfigIndex]->ConfigName.ToCStr();
    pinfo->RasCount = pRenderThread->GetGlyphRasterizationCount();
}

void WiiUDemo::GetHelpStr(String* phelpStr)
{
    phelpStr = NULL;
}


#if defined(GFX_ENABLE_SOUND)
void WiiUDemo::InitializeSound()
{
#if defined(GFX_SOUND_FMOD)
    if (!pSoundSystem)
        pSoundSystem = new FxSoundFMOD;
    if (!pSoundSystem->IsInitialized())
    {
        if (!pSoundSystem->Initialize())
            ShutdownSound();
    }
#endif
}

void WiiUDemo::ShutdownSound()
{
#if defined(GFX_SOUND_FMOD)
    if (pSoundSystem && pSoundSystem->IsInitialized())
        pSoundSystem->Finalize();
    delete pSoundSystem;
    pSoundSystem = 0;
#endif
}

Sound::SoundRenderer* WiiUDemo::GetSoundRenderer()
{
#if defined(GFX_SOUND_FMOD)
    return pSoundSystem && pSoundSystem->IsInitialized() ?
        pSoundSystem->GetSoundRenderer() :
    0;
#else
    return 0;
#endif
}

#endif // GFX_ENABLE_SOUND

