/**************************************************************************

Filename    :   GameUIActionBar.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameUIActionBar_H
#define INC_GameUIActionBar_H

#include "../Game/GameCommon.h"
#include "../Game/GameClientState.h"

//////////////////////////////////////////////////////////////////////////

class GameUISlotLink
{
public:
    enum LinkType
    {
        LINK_Empty,
        LINK_Ability,
        LINK_Entity,
        LINK_EntityDefn,
    };

    GameUISlotLink() : Type(LINK_Empty) {}
    GameUISlotLink(Game::AbilityID abilId) : Type(LINK_Ability), AbilityID(abilId) {}
    GameUISlotLink(const GameClientEntityRef& entData) : Type(LINK_Entity), EntityData(entData) {}
    GameUISlotLink(const Game::EntityDefnID& entId) : Type(LINK_EntityDefn), DefnID(entId) {}

    void    Clear()     { Type = LINK_Empty; }
    bool    IsEmpty() const     { return Type == LINK_Empty; }

    LinkType                    GetType() const         { return Type; }
    Game::AbilityID             GetAbilityID() const    { return AbilityID; }
    const GameClientEntityRef&  GetEntityData() const   { return EntityData; }
    const Game::EntityDefnID&   GetDefnID() const       { return DefnID; }

protected:
    LinkType                    Type;
    Game::AbilityID             AbilityID;
    GameClientEntityRef         EntityData;
    Game::EntityDefnID          DefnID;
};

class GameUIActionBar : public RefCountBase<GameUIActionBar, Stat_Default_Mem>
{
public:
    GameUIActionBar(unsigned numSlots);

    unsigned    GetNumSlots() const                 { return (unsigned)Links.GetSize(); }
    const GameUISlotLink&   GetSlot(unsigned idx)   { return Links[idx]; }
    void        SetSlot(unsigned idx, const GameUISlotLink& link)
    {
        Links[idx] = link;
    }
    void    ClearSlot(unsigned idx)
    {
        Links[idx].Clear();
    }

protected:
    Array<GameUISlotLink>   Links;
};

class GameUIActionBarManager
{
public:
    GameUIActionBarManager();

    void    AddBar(GameUIActionBar* bar)   { Bars.PushBack(bar); }

    unsigned    GetNumBars() const  { return (unsigned)Bars.GetSize(); }
    GameUIActionBar*    GetBar(unsigned idx) const  { return Bars[idx]; }

protected:
    Array< Ptr<GameUIActionBar> >  Bars;
};

#endif  // INC_GameUIActionBar_H
