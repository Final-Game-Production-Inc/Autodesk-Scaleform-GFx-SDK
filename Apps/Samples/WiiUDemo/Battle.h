/**************************************************************************

Filename    :   Battle.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Battle_H
#define INC_Battle_H

#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
using namespace Scaleform;

#include "Player.h"


//
// A battle party consisting of players
//
class Party : public RefCountBase<Party, Stat_Default_Mem>
{
public:
    Party() {}
    void    AddMember(Player* p)
    {
        Members.PushBack(p);
    }

    Player*     GetMember(UPInt idx) const      { return Members[idx]; }
    UPInt       GetNumMembers() const           { return Members.GetSize(); }

    bool        IsAllKO() const;

protected:
    Array< Ptr<Player> >    Members;
};


//
// A battle instance with two opposing parties
//
class Battle : public RefCountBase<Battle, Stat_Default_Mem>
{
public:
    enum StateType
    {
        STATE_Active,
        STATE_FriendlyWin,
        STATE_EnemyWin,
    };

    Battle(Party* friendly, Party* enemy);

    StateType   GetCurrentState() const     { return State; }

    unsigned    GetRoundCount() const   { return RoundCount; }

    void        CheckEndgame();

    void        Execute();

    Party*      GetFriendlies() const   { return FriendlyParty; }
    Party*      GetEnemies() const      { return EnemyParty; }
    Party*      GetOtherParty(Player* p);

    Player*     FindPlayer(unsigned id);
    bool        IsInFriendlyParty(Player* p) const;

    BattleResult*  GetCurrentResult();	// If NULL, then no more results exist
    void            NextResult();		// This moves internal iterator to next result

    bool    GetActiveCommand(BattleCommand* pcmd);	// If NULL, then no more commands
													// exist in this round
    void    ExecuteActiveCommand();
    void    NextCommand();				// This moves internal iterator to next command 
										// in current round

protected:
    Ptr<Party>      FriendlyParty;
    Ptr<Party>      EnemyParty;
    StateType       State;

    unsigned        RoundCount;

    Array<Player*>  ExecOrder;
    unsigned        ActiveCommandIdx;

    BattleResultArr    LastResults;
    unsigned           LastResultIdx;

    void            CheckNextCommand();
};

#endif
