/**************************************************************************

Filename    :   Player.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Player.h"

#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;

Player::Player(unsigned id, const char* name, unsigned level)
: Name(name), ID(id), Level(level), SelectedCommandIdx(0), TargetID(0)
{
    Commands[CMD_Attack] = *SF_NEW CmdAttack(false);
    Commands[CMD_Defend] = *SF_NEW CmdDefend(false);
}


//////////////////////////////////////////////////////////////////////////

Swordsman::Swordsman(unsigned id, const char* name, unsigned level)
: Player(id, name, level) 
{
    Commands[CMD_Special1] = *SF_NEW CmdBladeFury(false);
    Commands[CMD_Special2] = *SF_NEW CmdProtect(true);
}

unsigned        Swordsman::GetMaxHP() const
{
    return 804;
}

unsigned        Swordsman::GetMaxSP() const
{
    return 150;
}

unsigned        Swordsman::GetPhysicalDamage() const
{
    return 94;
}

unsigned        Swordsman::GetMagicalDamage() const
{
    return 10;
}

//////////////////////////////////////////////////////////////////////////

Mage::Mage(unsigned id, const char* name, unsigned level)
: Player(id, name, level) 
{
    Commands[CMD_Special1] = *SF_NEW CmdBlazingRain(false);
    Commands[CMD_Special2] = *SF_NEW CmdSupremacy(true);
}

unsigned        Mage::GetMaxHP() const
{
    return 652;
}

unsigned        Mage::GetMaxSP() const
{
    return 589;
}

unsigned        Mage::GetPhysicalDamage() const
{
    return 46;
}

unsigned        Mage::GetMagicalDamage() const
{
    return 110;
}


//////////////////////////////////////////////////////////////////////////

Priest::Priest(unsigned id, const char* name, unsigned level)
: Player(id, name, level) 
{
    Commands[CMD_Special1] = *SF_NEW CmdHeal(false);
    Commands[CMD_Special2] = *SF_NEW CmdShield(true);
}

unsigned        Priest::GetMaxHP() const
{
    return 471;
}

unsigned        Priest::GetMaxSP() const
{
    return 496;
}

unsigned        Priest::GetPhysicalDamage() const
{
    return 62;
}

unsigned        Priest::GetMagicalDamage() const
{
    return 90;
}


//////////////////////////////////////////////////////////////////////////

AlmeidaVargas::AlmeidaVargas(unsigned id)
: Player(id, "Almeida Vargas", 10) 
{
    Commands[CMD_Special1] = *SF_NEW CmdSonicLash(true);
    Commands[CMD_Special2] = *SF_NEW CmdVoid(true);
}

unsigned        AlmeidaVargas::GetMaxHP() const
{
    return 1760;
}

unsigned        AlmeidaVargas::GetMaxSP() const
{
    return 1178;
}

unsigned        AlmeidaVargas::GetPhysicalDamage() const
{
    return 209;
}

unsigned        AlmeidaVargas::GetMagicalDamage() const
{
    return 140;
}


//////////////////////////////////////////////////////////////////////////

Undead::Undead(unsigned id, const char* name, unsigned level)
: Player(id, name, level) 
{
    Commands[CMD_Special1] = *SF_NEW CmdInfect(true);
    Commands[CMD_Special2] = *SF_NEW CmdDrain(true);
}

unsigned        Undead::GetMaxHP() const
{
    return 820;
}

unsigned        Undead::GetMaxSP() const
{
    return 190;
}

unsigned        Undead::GetPhysicalDamage() const
{
    return 84;
}

unsigned        Undead::GetMagicalDamage() const
{
    return 19;
}

