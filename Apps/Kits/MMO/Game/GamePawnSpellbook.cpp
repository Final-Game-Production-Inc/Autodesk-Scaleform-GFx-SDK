/**************************************************************************

Filename    :   GamePawnSpellbook.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GamePawnSpellbook.h"
#include "GamePawn.h"

//////////////////////////////////////////////////////////////////////////

bool    GamePawnAbilityCooldown::SetTicksLeft(UInt32 ticks)
{
    Active = true;
    if (TicksLeft > ticks) return false;
    TicksLeft = ticks;
    return true;
}

void    GamePawnAbilityCooldown::Update(UInt32 ticksDelta)
{
    if (!Active) return;
    if (TicksLeft < ticksDelta)
        TicksLeft = 0;
    else
        TicksLeft -= ticksDelta;
}

//////////////////////////////////////////////////////////////////////////

void    GamePawnCastingInfo::Cast(GamePawnAbilityDesc* abilDesc)
{
    Active = true;
    Ability = abilDesc;
    TicksLeft = Game::GetAbilityDescForID(abilDesc->GetDefnID())->CastTime;

    // Insta cast?
    if (TicksLeft == 0)
        Clear();
}

void    GamePawnCastingInfo::Update(UInt32 ticksDelta)
{
    if (!Active) return;
    if (TicksLeft < ticksDelta)
        TicksLeft = 0;
    else
        TicksLeft -= ticksDelta;
}

//////////////////////////////////////////////////////////////////////////

GamePawnAbilityContainer::GamePawnAbilityContainer(GamePlayerPawn* pawn)
{
    Owner = pawn;
}

void    GamePawnAbilityContainer::AddAbility(GamePawnAbilityDesc* abil)
{
    SF_ASSERT(abil);

    Abilities.PushBack(abil);
    Owner->notifyAbilityAdd(abil);
}

void    GamePawnAbilityContainer::UseAbility(Game::AbilityID abilId)
{
    GamePawnAbilityDesc* desc = NULL;
    for (unsigned i = 0; i < Abilities.GetSize(); i++)
    {
        if (Abilities[i]->GetDefnID() == abilId)
        {
            desc = Abilities[i];
            break;
        }
    }
    if (!desc) 
    {
        Game::EntityDefnID defnId;
        defnId.Class = Game::ENTITY_Ability;
        defnId.ID = abilId;
        printf("%s doesn't have ability '%s'!\n", Owner->GetInfo(Game::INFO_Name), Game::GetStringsForID(defnId)->Name);
        return;
    }  
    if (desc->GetCooldown().IsActive()) return;

    if (CastingInfo.IsActive())
    {
        Owner->notifyAbilityCastInterrupt(CastingInfo.GetAbility());
    }
    CastingInfo.Cast(desc);
    if (CastingInfo.IsActive())
    {
        Owner->notifyAbilityCastStart(desc);
    }

    // Has cooldown?
    Game::AbilityDescType* abilDesc = Game::GetAbilityDescForID(abilId);
    if (abilDesc->Cooldown > 0)
    {
        addCooldown(desc, abilDesc->Cooldown);
    }
    // Has global cooldown?
    if (abilDesc->GlobalCooldown > 0)
    {
        addGlobalCooldown(abilDesc->GlobalCooldown);
    }
}

void    GamePawnAbilityContainer::addCooldown(GamePawnAbilityDesc* desc, UInt32 ticks)
{
    if (desc->GetCooldown().SetTicksLeft(ticks))
        Owner->notifyAbilityCooldownStart(desc);
}

void    GamePawnAbilityContainer::addGlobalCooldown(UInt32 ticks)
{
    for (unsigned i = 0; i < Abilities.GetSize(); i++)
    {
        GamePawnAbilityDesc* desc = Abilities[i];
        if (!Game::GetAbilityDescForID(desc->GetDefnID())->IgnoresGlobalCooldown)
        {
            if (desc->GetCooldown().SetTicksLeft(ticks))
                Owner->notifyAbilityCooldownStart(desc);
        }
    }    
}

void    GamePawnAbilityContainer::Update(UInt32 ticksDelta)
{
    for (unsigned i = 0; i < Abilities.GetSize(); i++)
    {
        GamePawnAbilityDesc* desc = Abilities[i];
        desc->GetCooldown().Update(ticksDelta);
        if (desc->GetCooldown().IsActive() && desc->GetCooldown().GetTicksLeft() == 0)
        {
            desc->GetCooldown().Clear();
            Owner->notifyAbilityCooldownEnd(desc);
        }
    }    

    if (CastingInfo.IsActive())
    {
        CastingInfo.Update(ticksDelta);
        if (CastingInfo.IsDone())
        {
            Owner->notifyAbilityCastEnd(CastingInfo.GetAbility());
            CastingInfo.Clear();
        }
    }
}
