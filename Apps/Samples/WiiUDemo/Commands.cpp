/**************************************************************************

Filename    :   Commands.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Commands.h"
#include "Player.h"
#include "Battle.h"

#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;


bool Command::IsPossible(Player* src, Player* target, Battle* battle)
{
    return !(src->IsKO() || src->GetSP() < ConsumedSP || !target || target->IsKO());
}

//////////////////////////////////////////////////////////////////////////

void CmdAttack::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
    result.PushBack(r);

    if (!miss)
        r->DeltaTargetHP = -target->ApplyDamage(src->GetPhysicalDamage() * critCoeff);
}

void CmdBladeFury::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
    result.PushBack(r);

    if (!miss)
        r->DeltaTargetHP = -target->ApplyDamage( (unsigned)(src->GetPhysicalDamage() * 2.5f * critCoeff));
}

void CmdProtect::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    // TODO
}

void CmdBlazingRain::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
    result.PushBack(r);

    if (!miss)
        r->DeltaTargetHP = -target->ApplyDamage( (unsigned)(src->GetMagicalDamage() * 2.5f * critCoeff));
}

void CmdSupremacy::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    // TODO
}

void CmdHeal::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
    result.PushBack(r);

    // Damage undead types
    if (target->IsUndeadType())
    {
        if (!miss)
            r->DeltaTargetHP = -target->ApplyDamage( (unsigned)(src->GetMagicalDamage() * 3.5f * critCoeff));
    }
    else
    {
        r->bMiss = false;   // Cannot miss a heal
        r->DeltaTargetHP = target->ApplyHeal( (unsigned)(src->GetMagicalDamage() * 3.0f * critCoeff));
        r->bHeal = true;
    }
}

void CmdShield::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
	// TODO
}

void CmdSonicLash::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
    result.PushBack(r);

    if (!miss)
        r->DeltaTargetHP = -target->ApplyDamage( (unsigned)(src->GetMagicalDamage() * 10.0f * critCoeff));
}

bool CmdVoid::IsPossible(Player* src, Player* target, Battle* battle)
{
    if (src->IsKO() || src->GetSP() < ConsumedSP || !target)
        return false;

    Party* otherParty = battle->GetOtherParty(src);
    for (unsigned i = 0; i < otherParty->GetNumMembers(); i++)
    {
        target = otherParty->GetMember(i);
        if (!target->IsKO()) return true;
    }

    return false;
}

void CmdVoid::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
    if (!IsPossible(src, target, battle)) return;
    src->ConsumeSP(ConsumedSP);

    float critCoeff = (rand() % 100 > 75) ? 2 : 1;
    bool miss = (rand() % 100 > 90) ? true : false;

    Party* otherParty = battle->GetOtherParty(src);
    for (unsigned i = 0; i < otherParty->GetNumMembers(); i++)
    {
        target = otherParty->GetMember(i);

        Ptr<BattleResult> r = *SF_NEW BattleResult(this, src, target, critCoeff > 1, miss, false);
        result.PushBack(r);

        if (!miss)
            r->DeltaTargetHP = -target->ApplyDamage( (unsigned)(src->GetMagicalDamage() * 3.0f * critCoeff));
    }
}

void CmdInfect::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
	// TODO
}

void CmdDrain::Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)
{
	// TODO
}
