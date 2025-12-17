/**************************************************************************

Filename    :   Battle.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Battle.h"

#include "Kernel/SF_Debug.h"


bool    Party::IsAllKO() const
{
    for (unsigned i = 0; i < Members.GetSize(); i++)
    {
        if (!Members[i]->IsKO()) return false;
    }
    return true;
}


Battle::Battle(Party* friendly, Party* enemy) 
: FriendlyParty(friendly), EnemyParty(enemy), State(STATE_Active),
  RoundCount(0)
{
	// Make round selection and AI behavior 'random'
    srand((unsigned)time(NULL));

    // Setup initial targets for all players
    SF_ASSERT(FriendlyParty->GetNumMembers() > 0);
    SF_ASSERT(EnemyParty->GetNumMembers() > 0);
    Player* target0 = FriendlyParty->GetMember(0);
    Player* target1 = EnemyParty->GetMember(0);

    for (unsigned i = 0; i < FriendlyParty->GetNumMembers(); i++)
    {
        Player* member = FriendlyParty->GetMember(i);
        member->SetTargetID(target1->GetId());
    }
    for (unsigned i = 0; i < EnemyParty->GetNumMembers(); i++)
    {
        Player* member = EnemyParty->GetMember(i);
        member->SetTargetID(target0->GetId());
    }
}

Player*     Battle::FindPlayer(unsigned id)
{
    for (unsigned i = 0; i < FriendlyParty->GetNumMembers(); i++)
    {
        Player* member = FriendlyParty->GetMember(i);
        if (member->GetId() == id)
            return member; 
    }
    for (unsigned i = 0; i < EnemyParty->GetNumMembers(); i++)
    {
        Player* member = EnemyParty->GetMember(i);
        if (member->GetId() == id)
            return member; 
    }
    return NULL;
}

bool    Battle::IsInFriendlyParty(Player* p) const
{
    for (unsigned i = 0; i < FriendlyParty->GetNumMembers(); i++)
    {
        if (p == FriendlyParty->GetMember(i))
            return true;
    }
    return false;
}

Party*  Battle::GetOtherParty(Player* p)
{
    if (IsInFriendlyParty(p))
        return EnemyParty;
    else
        return FriendlyParty;
}

void    Battle::Execute()
{
    RoundCount++;

    LastResults.Clear();
    ExecOrder.Clear();
    LastResultIdx = 0;
    ActiveCommandIdx = 0;

    if (State != STATE_Active) return;

    unsigned numFriends = FriendlyParty->GetNumMembers();
    unsigned numEnemies = EnemyParty->GetNumMembers();

    // Create execution order (roll virtual dice / random_shuffle)
    ExecOrder.Clear();
    for (unsigned i = 0; i < FriendlyParty->GetNumMembers(); i++)
    {
        if (!FriendlyParty->GetMember(i)->IsKO())
            ExecOrder.PushBack(FriendlyParty->GetMember(i));
    }
    for (unsigned i = 0; i < EnemyParty->GetNumMembers(); i++)
    {
        if (!EnemyParty->GetMember(i)->IsKO())
            ExecOrder.PushBack(EnemyParty->GetMember(i));
    }
    for (unsigned i = ExecOrder.GetSize() - 1; i > 0; --i)
    {
        // Swap
        unsigned d = rand() % (i+1);
        Player* p = ExecOrder[i];
        ExecOrder[i] = ExecOrder[d];
        ExecOrder[d] = p;
    }

    // Pick random commands and targets for enemies (only non-KO'd friendlies)
    Array<Player*> validFriendlies;
    for (unsigned i = 0; i < FriendlyParty->GetNumMembers(); i++)
    {
        if (!FriendlyParty->GetMember(i)->IsKO())
            validFriendlies.PushBack(FriendlyParty->GetMember(i));
    }
    for (unsigned i = 0; i < EnemyParty->GetNumMembers(); i++)
    {
        Player* e = EnemyParty->GetMember(i);
        // Target
        e->SetTargetID(validFriendlies[rand() % validFriendlies.GetSize()]->GetId());
        // Command
		// TODO: PPS: Attack only for now
//        e->SetSelectedCommandIdx(rand() % NumCommandSlots);
    }

    CheckNextCommand();
}

void    Battle::CheckEndgame()
{
    // Check for endgame
    if (FriendlyParty->IsAllKO())
        State = STATE_EnemyWin;
    else if (EnemyParty->IsAllKO())
        State = STATE_FriendlyWin;
}

bool    Battle::GetActiveCommand(BattleCommand* pcmd)
{
    if (ActiveCommandIdx >= ExecOrder.GetSize())
        return false;

    pcmd->pSrc = ExecOrder[ActiveCommandIdx];
    pcmd->pTarget = FindPlayer(pcmd->pSrc->GetTargetID());
    pcmd->pCmd = pcmd->pSrc->GetCommand((CommandSlot)pcmd->pSrc->GetSelectedCommandIdx());
}

void    Battle::NextCommand()
{
    ActiveCommandIdx++;
    CheckNextCommand();
}

void    Battle::CheckNextCommand()
{
    while (ActiveCommandIdx < ExecOrder.GetSize())
    {
        Player* psrc = ExecOrder[ActiveCommandIdx];
        Player* ptarget = FindPlayer(psrc->GetTargetID());

        if (psrc->GetCommand((CommandSlot)psrc->GetSelectedCommandIdx())->IsPossible(psrc, ptarget, this))
            return;
        
        ActiveCommandIdx++;
    }
}

void    Battle::ExecuteActiveCommand()
{
    LastResultIdx = 0;
    LastResults.Clear();
    Player* psrc = ExecOrder[ActiveCommandIdx];
    Player* ptarget = FindPlayer(psrc->GetTargetID());
    psrc->GetCommand((CommandSlot)psrc->GetSelectedCommandIdx())->Execute(LastResults, psrc, ptarget, this);
}

BattleResult*   Battle::GetCurrentResult()
{
    if (LastResultIdx >= LastResults.GetSize())
        return NULL;

    return LastResults[LastResultIdx];
}

void    Battle::NextResult()
{
    LastResultIdx++;
}
