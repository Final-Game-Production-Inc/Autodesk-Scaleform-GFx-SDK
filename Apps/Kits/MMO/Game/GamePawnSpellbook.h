/**************************************************************************

Filename    :   GamePawnSpellbook.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GamePawnSpellbook_H
#define INC_GamePawnSpellbook_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

#include "GameCommon.h"

class GamePlayerPawn;

//////////////////////////////////////////////////////////////////////////

class GamePawnAbilityCooldown
{
public:
    GamePawnAbilityCooldown() : Active(false), TicksLeft(0) {}
    void    Update(UInt32 ticksDelta);

    void    Clear() { Active = false; TicksLeft = 0; }

    UInt32  GetTicksLeft() const    { return TicksLeft; }
    bool    SetTicksLeft(UInt32 ticks);

    bool    IsActive() const        { return Active; }
    bool    IsDone() const          { return TicksLeft == 0; }

protected:
    bool    Active;
    UInt32  TicksLeft;
};

class GamePawnAbilityDesc : public RefCountBase<GamePawnAbilityDesc, Stat_Default_Mem>
{
public:
    GamePawnAbilityDesc(Game::AbilityID defnId, unsigned rank) : DefnID(defnId), Rank(rank) {}
    Game::AbilityID     GetDefnID() const     { return DefnID; }
    unsigned            GetRank() const     { return Rank; }

    GamePawnAbilityCooldown&    GetCooldown()   { return Cooldown; }

protected:
    Game::AbilityID             DefnID;
    unsigned                    Rank;
    GamePawnAbilityCooldown    Cooldown;
};

class GamePawnCastingInfo
{
public:
    GamePawnCastingInfo() : Active(false), TicksLeft(0) {}
    void    Update(UInt32 ticksDelta);

    void    Cast(GamePawnAbilityDesc* abilDesc);

    void    Clear()     { Active = false; TicksLeft = 0; Ability = NULL; }

    GamePawnAbilityDesc*    GetAbility() const  { return Ability; }

    bool    IsActive() const   { return Active; }
    bool    IsDone() const      { return TicksLeft == 0; }

protected:
    bool                        Active;
    Ptr<GamePawnAbilityDesc>    Ability;
    UInt32                      TicksLeft;
};

class GamePawnAbilityContainer : public RefCountBase<GamePawnAbilityContainer, Stat_Default_Mem>
{
public:
    GamePawnAbilityContainer(GamePlayerPawn* pawn);

    void        AddAbility(GamePawnAbilityDesc* abil);
    void        UseAbility(Game::AbilityID abilId);
    unsigned    GetNumAbilities() const     { return (unsigned) Abilities.GetSize(); }
    GamePawnAbilityDesc*    GetAbility(unsigned idx)    { return Abilities[idx]; }

    void        Update(UInt32 ticksDelta);

protected:
    GamePlayerPawn*                           Owner;
    Array< Ptr<GamePawnAbilityDesc> >   Abilities;
    GamePawnCastingInfo                 CastingInfo;

    void    addCooldown(GamePawnAbilityDesc* desc, UInt32 ticks);
    void    addGlobalCooldown(UInt32 ticks);
};

#endif  // INC_GamePawnSpellbook_H
