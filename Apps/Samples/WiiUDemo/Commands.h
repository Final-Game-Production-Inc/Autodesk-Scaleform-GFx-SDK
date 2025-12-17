/**************************************************************************

Filename    :   Commands.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Commands_H
#define INC_Commands_H

#include "Kernel/SF_String.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_RefCount.h"
using namespace Scaleform;

class Player;
class Battle;
class Command;

//
// Predefined command slots for players
//
enum CommandSlot
{
    CMD_Attack,
    CMD_Special1,
    CMD_Special2,
    CMD_Defend,

    NumCommandSlots
};


//
// A command issued in battle
//
struct BattleCommand : public RefCountBase<BattleCommand, Stat_Default_Mem>
{
    Command     *pCmd;
    Player      *pSrc, *pTarget;

    BattleCommand() : pCmd(0), pSrc(0), pTarget(0) {}
    BattleCommand(Command* cmd, Player* src, Player* target) 
        : pCmd(cmd), pSrc(src), pTarget(target) {}
};

//
// A result generated from a command issued in battle
//
struct BattleResult : public RefCountBase<BattleResult, Stat_Default_Mem>
{
    Command     *pCmd;
    Player      *pSrc, *pTarget;
    int         DeltaTargetHP;
    bool        bCrit, bMiss, bHeal;

    BattleResult(Command* cmd, Player* src, Player* target, bool crit, bool miss, bool bheal) 
        : pCmd(cmd), pSrc(src), pTarget(target), DeltaTargetHP(0), bCrit(crit), bMiss(miss), bHeal(bheal) {}
};
typedef Array<Ptr<BattleResult> >      BattleResultArr;


//////////////////////////////////////////////////////////////////////////


//
// Base battle command
//
class Command : public RefCountBase<Command, Stat_Default_Mem>
{
public:
    Command(unsigned consumedSP, bool disabled) : ConsumedSP(consumedSP), Disabled(disabled) {}

    virtual const char*     GetName() const     = 0;
    virtual unsigned        GetConsumedSP() const   { return ConsumedSP; }

    bool            IsDisabled() const  { return Disabled; }

    virtual bool    IsPossible(Player* src, Player* target, Battle* battle);
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle)  = 0;

protected:
    unsigned    ConsumedSP;
    bool        Disabled;
};



//////////////////////////////////////////////////////////////////////////
// Player command definitions
//

class CmdAttack : public Command
{
public:
    CmdAttack(bool disabled) : Command(0, disabled) {}
    virtual const char*     GetName() const     { return "Attack"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdDefend : public Command
{
public:
    CmdDefend(bool disabled) : Command(0, disabled) {}
    virtual const char*     GetName() const     { return "Defend"; }
    virtual bool    IsPossible(Player* src, Player* target, Battle* battle) { return false; }   // This is not an 'action'
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle) {}
};

class CmdBladeFury : public Command
{
public:
    CmdBladeFury(bool disabled) : Command(35, disabled) {}
    virtual const char*     GetName() const     { return "Blade Fury"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdProtect : public Command
{
public:
    CmdProtect(bool disabled) : Command(10, disabled) {}
    virtual const char*     GetName() const     { return "Protect"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdBlazingRain : public Command
{
public:
    CmdBlazingRain(bool disabled) : Command(50, disabled) {}
    virtual const char*     GetName() const     { return "Blazing Rain"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdSupremacy : public Command
{
public:
    CmdSupremacy(bool disabled) : Command(40, disabled) {}
    virtual const char*     GetName() const     { return "Supremacy"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdHeal : public Command
{
public:
    CmdHeal(bool disabled) : Command(40, disabled) {}
    virtual const char*     GetName() const     { return "Heal"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdShield : public Command
{
public:
    CmdShield(bool disabled) : Command(50, disabled) {}
    virtual const char*     GetName() const     { return "Shield"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdSonicLash : public Command
{
public:
    CmdSonicLash(bool disabled) : Command(30, disabled) {}
    virtual const char*     GetName() const     { return "Sonic Lash"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdVoid : public Command  // AOE
{
public:
    CmdVoid(bool disabled) : Command(70, disabled) {}
    virtual const char*     GetName() const     { return "Void"; }
    virtual bool    IsPossible(Player* src, Player* target, Battle* battle);
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdInfect : public Command
{
public:
    CmdInfect(bool disabled) : Command(40, disabled) {}
    virtual const char*     GetName() const     { return "Infect"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};

class CmdDrain : public Command
{
public:
    CmdDrain(bool disabled) : Command(50, disabled) {}
    virtual const char*     GetName() const     { return "Drain"; }
    virtual void    Execute(BattleResultArr& result, Player* src, Player* target, Battle* battle);
};



#endif
