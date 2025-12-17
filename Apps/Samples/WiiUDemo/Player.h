/**************************************************************************

Filename    :   Player.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Player_H
#define INC_Player_H

#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

#include "Commands.h"


//
// Definition for a player 
//
class Player : public RefCountBase<Player, Stat_Default_Mem>
{
public:
    Player(unsigned id, const char* name, unsigned level);
    void            Initialize() 
    {
        HP = GetMaxHP();
        SP = GetMaxSP();
    }

    unsigned        GetId() const           { return ID; }
    const char*     GetName() const         { return Name.ToCStr(); }
    unsigned        GetLevel() const        { return Level; }
    unsigned        GetHP() const           { return HP; }
    unsigned        GetSP() const           { return SP; }
    bool            IsKO() const            { return HP == 0; }

    virtual bool            IsUndeadType() const    { return false; }

    unsigned        GetTargetID() const         { return TargetID; }
    void            SetTargetID(unsigned id)    { TargetID = id; }

    Command*        GetCommand(CommandSlot slot)            { return Commands[slot]; }
    unsigned        GetSelectedCommandIdx() const           { return SelectedCommandIdx; }
    void            SetSelectedCommandIdx(unsigned idx)     { SelectedCommandIdx = idx; }

    unsigned            ApplyDamage(unsigned dmg)
    {
        // Halve damage when defending
        if (SelectedCommandIdx == CMD_Defend)
            dmg /= 2;
        unsigned ret = 0;
        if (HP < dmg) 
        {
            ret = HP;
            HP = 0;
        }
        else 
        {
            ret = dmg;
            HP -= dmg;
        }
        return ret;
    }

    unsigned            ApplyHeal(unsigned hp)
    {
        unsigned ret = 0;
        if ((HP + hp) > GetMaxHP())
        {
            ret = GetMaxHP() - HP;
            HP = GetMaxHP();
        }
        else
        {
            ret = hp;
            HP += hp;
        }
        return ret;
    }

    void                ConsumeSP(unsigned sp)
    {
        if (SP < sp) return;
        SP -= sp;
    }

    virtual const char*     GetClass() const        = 0;
    virtual unsigned        GetMaxHP() const    = 0;
    virtual unsigned        GetMaxSP() const    = 0;
    virtual unsigned        GetPhysicalDamage() const   = 0;
    virtual unsigned        GetMagicalDamage() const   = 0;

protected:
    String          Name;
    unsigned        ID;
    unsigned        Level;
    unsigned        HP;
    unsigned        SP;

    unsigned        TargetID;

    Ptr<Command>    Commands[NumCommandSlots];
    unsigned        SelectedCommandIdx;
};



//////////////////////////////////////////////////////////////////////////
// Definition for different player classes
//

class Swordsman : public Player
{
public:
    Swordsman(unsigned id, const char* name, unsigned level);

    virtual const char*     GetClass() const    { return "swordsman"; }
    virtual unsigned        GetMaxHP() const;
    virtual unsigned        GetMaxSP() const;
    virtual unsigned        GetPhysicalDamage() const;
    virtual unsigned        GetMagicalDamage() const;
};

class Mage : public Player
{
public:
    Mage(unsigned id, const char* name, unsigned level);

    virtual const char*     GetClass() const    { return "mage"; }
    virtual unsigned        GetMaxHP() const;
    virtual unsigned        GetMaxSP() const;
    virtual unsigned        GetPhysicalDamage() const;
    virtual unsigned        GetMagicalDamage() const;
};

class Priest : public Player
{
public:
    Priest(unsigned id, const char* name, unsigned level);

    virtual const char*     GetClass() const    { return "priest"; }
    virtual unsigned        GetMaxHP() const;
    virtual unsigned        GetMaxSP() const;
    virtual unsigned        GetPhysicalDamage() const;
    virtual unsigned        GetMagicalDamage() const;
};


class AlmeidaVargas : public Player
{
public:
    AlmeidaVargas(unsigned id);

    virtual const char*     GetClass() const    { return "almeida_vargas"; }
    virtual unsigned        GetMaxHP() const;
    virtual unsigned        GetMaxSP() const;
    virtual unsigned        GetPhysicalDamage() const;
    virtual unsigned        GetMagicalDamage() const;
};

class Undead : public Player
{
public:
    Undead(unsigned id, const char* name, unsigned level);

    virtual const char*     GetClass() const    { return "undead"; }
    virtual unsigned        GetMaxHP() const;
    virtual unsigned        GetMaxSP() const;
    virtual unsigned        GetPhysicalDamage() const;
    virtual unsigned        GetMagicalDamage() const;

    virtual bool            IsUndeadType() const    { return true; }
};

#endif
