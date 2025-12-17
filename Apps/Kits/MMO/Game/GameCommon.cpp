/**************************************************************************

Filename    :   GameCommon.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_MsgFormat.h"
using namespace Scaleform;

#include "GameCommon.h"

namespace Game {

//////////////////////////////////////////////////////////////////////////

// NOTE: THE FOLLOWING TABLES SHOULD NOT BE TOUCHED/REORDERED

//////////////////////////////////////////////////////////////////////////
// ABILITIES
//////////////////////////////////////////////////////////////////////////

static IDStringsType    AbilityStrings[] =
{
    // Ability description params: 
    // {0} - Min heal/damage
    // {1} - Max heal/damage

    /*ID_ERR_Unknown*/                { "Unknown", "", "" },                                                          
    /*ID_ABIL_Attack*/                { "Attack", "Starts combat by attacking a target with a melee weapon.", "icons/abilities/icon_attack.png" },                            
    /*ID_ABIL_RangedAttack*/          { "Ranged Attack", "Shoots at an enemy, inflicting Physical damage.", "icons/abilities/icon_ranged_attack.png" },              
    /*ID_ABIL_Adrenaline*/            { "Adrenaline", "Increases your attack and casting speed by 20% for 15 sec.", "icons/abilities/icon_adrenaline.png" },                    
    /*ID_ABIL_Sprint*/                { "Sprint", "Increases your movement speed by 70% for 8 sec.", "icons/abilities/icon_sprint.png" },                            
    /*ID_ABIL_Float*/                 { "Float", "Allows the friendly party or raid target to levitate, floating a few feet above the ground.  While levitating, the target will fall at a reduced speed and travel over water.  Any damage will cancel the effect.  Lasts 10 min.", "icons/abilities/icon_float.png" },                              
    /*ID_ABIL_Portal*/                { "Portal", "Creates a portal, teleporting group members to a destination of your choosing.", "icons/abilities/icon_portal.png" },                            
    /*ID_ABIL_PaladinsSacrifice*/     { "Paladin's Sacrifice", "You scarifice yourself for a party or raid member, transfering 30% damage taken to the caster.  Lasts 12 sec.", "icons/abilities/icon_paladins_sacrifice.png" },   
    /*ID_ABIL_Berserk*/               { "Berserk", "Increases your Attack Power by 20% for 10 sec.", "icons/abilities/icon_berserk.png" },                          
    /*ID_ABIL_OffensiveBanner*/       { "Offensive Banner", "Gives {1} additional Attack Power and Spell Power to party and raid members within 40 yards.", "icons/abilities/icon_offensive_banner.png" },        
    /*ID_ABIL_DefensiveBanner*/       { "Defensive Banner", "Gives {1} additional armor to party and raid members within 40 yards.", "icons/abilities/icon_defensive_banner.png" },        
    /*ID_ABIL_Smite*/                 { "Smite", "Calls forth a bolt from heaven, causing {0} to {1} Holy damage.", "icons/abilities/icon_smite.png" },                              
    /*ID_ABIL_Raise*/                 { "Raise", "Brings a dead ally back to life with 35% of maximum health and mana.  Cannot be cast when in combat.", "icons/abilities/icon_raise.png" },                              
    /*ID_ABIL_Heal*/                  { "Heal", "Heals a friendly target for {0} to {1}.", "icons/abilities/icon_heal.png" },                                
    /*ID_ABIL_Faith*/                 { "Faith", "Places a Blessing on the friendly target, increasing Strength, Agility, Stamina, and Intellect by 5%, and all magical resistances by 10% for 1 hour.", "icons/abilities/icon_faith.png" },                              
    /*ID_ABIL_Sanctuary*/             { "Sanctuary", "Remove the party or raid member from harm, reducing their total threat by 2% every 1 sec for 10 sec.", "icons/abilities/icon_sanctuary.png" },                      
    /*ID_ABIL_DivineLight*/           { "Divine Light", "Reduces all damage taken by 20% for 10 sec.", "icons/abilities/icon_divine_light.png" },                
    /*ID_ABIL_Protect*/               { "Protect", "A targeted party or raid member is protected from all physical attacks for 10 sec, but during that time they cannot attack or use physical abilities.", "icons/abilities/icon_protect.png" },                          
    /*ID_ABIL_AngelsBlessing*/        { "Angel's Blessing", "Bless all party and raid members, increasing their Stamina by 584 for 1 hour.", "icons/abilities/icon_angels_blessing.png" },         
    /*ID_ABIL_SoulLance*/             { "Soul Lance", "Hurl a holy lance piercing the soul of the target, causing {0} to {1} Holy damage.", "icons/abilities/icon_soul_lance.png" },                    
    /*ID_ABIL_Cure*/                  { "Cure", "Removes 1 disease from the friendly target.", "icons/abilities/icon_cure.png" },                                
    /*ID_ABIL_Thunder*/               { "Thunder", "A charge bolt from the sky that causes {0} to {1} Nature damage.", "icons/abilities/icon_thunder.png" },                          
    /*ID_ABIL_FireBolt*/              { "Fire Bolt", "Hurls a fiery bolt that causes {0} to {1} Fire damage.", "icons/abilities/icon_fire_bolt.png" },                      
    /*ID_ABIL_Blizzard*/              { "Blizzard", "Ice shards pelt the targeted area doing {1} Frost damage over 8 sec to enemies in 20 yard radius.", "icons/abilities/icon_blizzard.png" },                        
    /*ID_ABIL_Freeze*/                { "Freeze", "Freeze the target in place causing {0} to {1} Frost damage. Lasts 4 sec.", "icons/abilities/icon_freeze.png" },                            
    /*ID_ABIL_Tsunami*/               { "Tsunami", "A wave of powerful water causing {0} to {1} Nature damage, knocking the target back 30 yards and slowing their movement by 50% for 10 sec.", "icons/abilities/icon_tsunami.png" },                          
    /*ID_ABIL_Tornado*/               { "Tornado", "A strong gust of air causing {0} to {1} Nature damage and knocking the target back 30 yards.", "icons/abilities/icon_tornado.png" },                          
    /*ID_ABIL_Quake*/                 { "Quake", "The earth crumbles and breaks at the targeted area dealing {1} Nature damage every 1 sec to enemies in 8 yard radius, with a 10% chance of knocking them down. Lasts 10 sec.", "icons/abilities/icon_quake.png" },                              
    /*ID_ABIL_QuantumField*/          { "Quantum Field", "Create a field of pure energy causing {0} to {1} Arcane damage and slowing the targets' casting by 50% for 10 sec.", "icons/abilities/icon_quantum_field.png" },              
    /*ID_ABIL_HailStorm*/             { "Hailstorm", "Hail pelts the targeted area doing {1} Frost damage over 8 sec to enemies in 20 yard radius.", "icons/abilities/icon_hail_storm.png" },                     
    /*ID_ABIL_Gravity*/               { "Gravity", "Pulls the target toward a singularity, causing {0} to {1} Arcane damage.", "icons/abilities/icon_gravity.png" },                          
    /*ID_ABIL_Leech*/                 { "Leech", "Bite the target, causing {0} Shadow damage and restoring {1} of the caster's total health over 30 sec.", "icons/abilities/icon_leech.png" },                              
    /*ID_ABIL_Fear*/                  { "Fear", "Strikes fear in the enemy, causing it to run in fear for up to 20 sec.  Damage caused may interrupt the effect.", "icons/abilities/icon_fear.png" },                                
    /*ID_ABIL_Curse*/                 { "Curse", "Curses the target for 5 min, reducing Arcane, Fire, Frost, Nature, and Shadow resistances by 183.  Only one curse can be active on any one target.", "icons/abilities/icon_curse.png" },                              
    /*ID_ABIL_Terror*/                { "Terror", "Causes the enemy target to run in terror for 3 sec and causes {1} Shadow damage.  The caster gains 200% of the damage caused in health.", "icons/abilities/icon_terror.png" },                            
    /*ID_ABIL_Weaken*/                { "Weaken", "Weakens the target, increasing Shadow damage by 8%.", "icons/abilities/icon_weaken.png" },                            
    /*ID_ABIL_SoulBlast*/             { "Soul Blast", "Sends a blast of unholy energy at the enemy, causing {0} to {1} Shadow damage.", "icons/abilities/icon_soul_blast.png" },                    
    /*ID_ABIL_Plague*/                { "Plague", "Afflicts the target with a disease that causes {1} Shadow damage over 18 sec.", "icons/abilities/icon_plague.png" },                            
    /*ID_ABIL_SoulDrain*/             { "Soul Drain", "Drain the soul of the target, causing {1} Shadow damage over 18 sec.", "icons/abilities/icon_soul_drain.png" },                    
    /*ID_ABIL_Infect*/                { "Infect", "Afflicts the target with a disease that causes {1} Shadow damage over 24 sec.", "icons/abilities/icon_infect.png" },                            
    /*ID_ABIL_Corruption*/            { "Corruption", "Corrupts the target, causing {1} Shadow damage over 18 sec.", "icons/abilities/icon_corruption.png" },                    
};

static AbilityDescType    AbilitiesDesc[] = 
{
    //                                  Lvl     Group               MinDH   MaxDH    MPct   MCons     Rng    Cast    Cool   GCool     IgGlob

    /*ID_ABIL_Unknown,             */ { 9999,   ABIL_Unknown,           0,      0,      0,      0,      0,      0,      0,      0,      false },

    // * ABILITIES
    // - General
    /*ID_ABIL_Attack,              */ { 1,      ABIL_General,           0,      0,      0,      0,      0,      0,      0,      0,      false },
    /*ID_ABIL_RangedAttack,        */ { 1,      ABIL_General,           0,      0,      0,      0,      0,      0,      0,      0,      false },
    /*ID_ABIL_Adrenaline,          */ { 1,      ABIL_General,           0,      0,      0,      0,      0,      0,  60000,   1000,      false },
    /*ID_ABIL_Sprint,              */ { 5,      ABIL_General,           0,      0,      0,      0,      0,      0, 600000,      0,      false },
    /*ID_ABIL_Float,               */ { 5,      ABIL_General,           0,      0,      0,      0,      0,      0,  60000,      0,      false },
    /*ID_ABIL_Portal,              */ { 10,     ABIL_General,           0,      0,     50,      0,      0,  15000,      0,   1000,      false },
    /*ID_ABIL_PaladinsSacrifice,   */ { 12,     ABIL_General,           0,      0,      0,      0,      0,      0, 600000,   1000,      true  },
    /*ID_ABIL_Berserk,             */ { 12,     ABIL_General,           0,      0,      0,      0,      0,      0,  60000,   1000,      false },
    /*ID_ABIL_OffensiveBanner,     */ { 15,     ABIL_General,           0,   4075,      0,      0,      0,      0, 600000,      0,      true  },
    /*ID_ABIL_DefensiveBanner,     */ { 30,     ABIL_General,           0,   4075,      0,      0,      0,      0, 600000,      0,      true  },

    // - RESTORATION
    /*ID_ABIL_Smite,               */ { 1,      ABIL_Restoration,    3405,   3850,     10,      0,     30,   1500,      0,   1000,      false },
    /*ID_ABIL_Raise,               */ { 5,      ABIL_Restoration,       0,      0,     40,      0,      0,   8000,      0,   1000,      false },
    /*ID_ABIL_Heal,                */ { 5,      ABIL_Restoration,    1584,   2045,      5,      0,      0,   2000,      0,   1000,      false },
    /*ID_ABIL_Faith,               */ { 10,     ABIL_Restoration,       0,      0,      8,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_Sanctuary,           */ { 12,     ABIL_Restoration,       0,      0,     10,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_DivineLight,         */ { 12,     ABIL_Restoration,       0,      0,     12,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_Protect,             */ { 15,     ABIL_Restoration,       0,      0,     12,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_AngelsBlessing,      */ { 15,     ABIL_Restoration,       0,      0,     10,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_SoulLance,           */ { 20,     ABIL_Restoration,    1800,   2310,      8,      0,      0,   1500,   4500,   1000,      false },
    /*ID_ABIL_Cure,                */ { 25,     ABIL_Restoration,       0,      0,      5,      0,      0,   1500,      0,   1000,      false },

    // - ELEMENTAL
    /*ID_ABIL_Thunder,             */ { 1,      ABIL_Elemental,      3775,   4200,      8,      0,      0,   2000,   6000,   1000,      false },
    /*ID_ABIL_FireBolt,            */ { 1,      ABIL_Elemental,      2600,   3145,      8,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_Blizzard,            */ { 1,      ABIL_Elemental,         0,  10200,     16,      0,     30,   3000,      0,   1000,      false },
    /*ID_ABIL_Freeze,              */ { 5,      ABIL_Elemental,       500,    850,      8,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_Tsunami,             */ { 7,      ABIL_Elemental,       800,   1250,     16,      0,     30,   2000,      0,   1000,      false },
    /*ID_ABIL_Tornado,             */ { 10,     ABIL_Elemental,       770,   1040,     16,      0,      0,   2000,      0,   1000,      false },
    /*ID_ABIL_Quake,               */ { 10,     ABIL_Elemental,         0,    325,     16,      0,      0,   2000,      0,   1000,      false },
    /*ID_ABIL_QuantumField,        */ { 10,     ABIL_Elemental,      1975,   2220,     28,      0,      0,  10000,      0,   1000,      false },
    /*ID_ABIL_HailStorm,           */ { 18,     ABIL_Elemental,         0,  11500,     16,      0,      0,   4000,      0,   1000,      false },
    /*ID_ABIL_Gravity,             */ { 22,     ABIL_Elemental,      4605,   4900,     12,      0,     25,   3000,      0,   1000,      false },

    // - LIFE BANE
    /*ID_ABIL_Leech,               */ { 1,      ABIL_LifeBane,        492,   2420,      8,      0,     20,   2000,      0,   1000,      false },
    /*ID_ABIL_Fear,                */ { 1,      ABIL_LifeBane,          0,      0,     12,      0,     10,      0,  30000,   1000,      false },
    /*ID_ABIL_Curse,               */ { 1,      ABIL_LifeBane,          0,      0,      5,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_Terror,              */ { 5,      ABIL_LifeBane,          0,    850,     16,      0,     10,      0,  90000,   1000,      false },
    /*ID_ABIL_Weaken,              */ { 10,     ABIL_LifeBane,          0,      0,     12,      0,      0,   1500,      0,   1000,      false },
    /*ID_ABIL_SoulBlast,           */ { 10,     ABIL_LifeBane,       2902,   3320,      5,      0,      0,   2500,      0,   1000,      false },
    /*ID_ABIL_Plague,              */ { 12,     ABIL_LifeBane,          0,    850,      5,      0,      0,      0,      0,   1000,      false },   
    /*ID_ABIL_SoulDrain,           */ { 16,     ABIL_LifeBane,          0,   2041,     10,      0,      0,   2000,      0,   1000,      false },
    /*ID_ABIL_Infect,              */ { 20,     ABIL_LifeBane,          0,   1204,      8,      0,      0,      0,      0,   1000,      false },
    /*ID_ABIL_Corruption,          */ { 25,     ABIL_LifeBane,          0,   1450,     18,      0,      0,   2000,      0,   1000,      false },
};

AbilityDescType*    GetAbilityDescForID(AbilityID abilId)
{
    return &AbilitiesDesc[abilId];
}

void    GetAbilitiesByGroup(Array<AbilityID>* abilIds, AbilityGroupType abilGroupId)
{
    abilIds->Clear();
    for (unsigned i = 0; i < NumAbilityIDs; i++)
    {
        if (AbilitiesDesc[i].GroupID == abilGroupId)
            abilIds->PushBack((AbilityID)i);
    }
}

const char*     GetAbilityGroupNameFromID(AbilityGroupType abilGroupId)
{
    switch (abilGroupId)
    {
    case Game::ABIL_General: return "General";
    case Game::ABIL_Elemental: return "Elemental";
    case Game::ABIL_LifeBane: return "Life Bane";
    case Game::ABIL_Restoration: return "Restoration";
    default: return "??";
    }
}

void    GetInstanceAbilDesc(AbilityDescType* desc, AbilityID abilId, const EntityInstanceStatMod& statMod)
{
    *desc = *GetAbilityDescForID(abilId);
    float minDHDelta = 0.f;
    float maxDHDelta = 0.f;

    float lvl = (float)statMod.GetPawnStat(STAT_Level);
    minDHDelta  = ( lvl * desc->MinDamageOrHeal * ((statMod.GetPawnStat(STAT_Intelligence) * 0.9f) + (statMod.GetPawnStat(STAT_Wisdom) * 0.1f)) ) * 0.001f;
    maxDHDelta  = ( lvl * desc->MaxDamageOrHeal * ((statMod.GetPawnStat(STAT_Intelligence) * 0.9f) + (statMod.GetPawnStat(STAT_Wisdom) * 0.1f)) ) * 0.001f;

    desc->MinDamageOrHeal += minDHDelta;
    desc->MaxDamageOrHeal += maxDHDelta;
}



//////////////////////////////////////////////////////////////////////////
// WEAPONS
//////////////////////////////////////////////////////////////////////////

static IDStringsType    WeaponStrings[] =
{
    /*ID_WEAP_Unknown,*/                { "Unknown", "", "" },          
    /*ID_WEAP_SwordOfReckoning*/        { "Sword of Reckoning", "", "icons/weapons/blade_of_reckoning.png" },           
    /*ID_WEAP_HammerOfRighteousness*/   { "Hammer of Righteousness", "Legendary weapon of Gurerris!", "icons/weapons/hammer_of_righteousness.png" }, 
    /*ID_WEAP_IronCrossbow*/            { "Iron Crossbow", "", "icons/weapons/iron_crossbow.png" },                     
};
static IDDescType      WeaponFlags[] =
{
    /*ID_WEAP_Unknown,*/                {   false,  0,  false,  false,  false,  0 },
    /*ID_WEAP_SwordOfReckoning*/        {   true,   0,  false,  false,  true,   WEAP_Melee1H },
    /*ID_WEAP_HammerOfRighteousness*/   {   true,   0,  false,  false,  true,   WEAP_Melee2H  },
    /*ID_WEAP_IronCrossbow*/            {   true,   0,  false,  false,  true,   WEAP_Ranged  },
};

static WeaponDescType   WeaponsDesc[] =
{
    //                                          Quality       Binding  LvlRq   Co  Si  Go         Class             MinDmg     MaxDmg  Spd(sec)    Buffs  (value, interval (ms), duration)
    //                                                                                                                                             Hp	          	MaxHP        	Mp           	MaxMp         	Str         	Dex         	Int         	End         	Wis       	  	Luck

    /*ID_WEAP_Unknown,              */  {   { QUAL_Poor,     BIND_None,   0,  { 12, 23, 34 } },   WEAPSUB_Unknown,    10.f,     100.f,    1.f,   { { 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_WEAP_SwordOfReckoning,     */  {   { QUAL_Rare, 	 BIND_None,  12,  { 12, 23, 34 } },   WEAPSUB_Sword,     563.f,    1047.f,   2.6f,   { { 0, 0, 0 }, 	{ 147, 0, 0 }, 	{ 0, 0, 0 }, 	{ 102, 0, 0 }, 	{ 98, 0, 0 }, 	{ 0, 0, 0 }, 	{ 54, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_WEAP_HammerOfRighteousness,*/  {   { QUAL_Legendary,BIND_None,  12,  { 12, 23, 34 } },   WEAPSUB_Mace,     1062.f,    1594.f,   3.3f,   { { 10, 1000, 0 }, { 420, 0, 0 }, 	{ 5, 1000, 0 }, { 256, 0, 0 }, 	{ 235, 0, 0 }, 	{ 0, 0, 0 }, 	{ 108, 0, 0 }, 	{ 25, 0, 0 }, 	{ 10, 0, 0 }, 	{ 35, 0, 0 } } },
    /*ID_WEAP_IronCrossbow,         */  {   { QUAL_Common,   BIND_None,   0,  { 12, 23, 34 } },   WEAPSUB_CrossBow,  121.f,     558.f,   2.9f,   { { 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
};

WeaponDescType*    GetWeaponDescForID(WeaponID weapId)
{
    return &WeaponsDesc[weapId];
}

void    GetInstanceWeaponDesc(WeaponDescType* desc, WeaponID weapId, const EntityInstanceStatMod& statMod)
{
    *desc = *GetWeaponDescForID(weapId);

    float dmgDelta = 0.f;
    float speedDelta = 0.f;

    switch (desc->SubGroup)
    {
    case WEAPSUB_ShortBow:
    case WEAPSUB_CrossBow:
    case WEAPSUB_LongBow:
        dmgDelta    = (statMod.GetPawnStat(STAT_Strength) * 0.2f + statMod.GetPawnStat(STAT_Dexterity) * 0.8f) / 100.f;
        speedDelta  = (statMod.GetPawnStat(STAT_Dexterity)) / 1000.f;
        break;

    case WEAPSUB_Dagger:
        dmgDelta    = (statMod.GetPawnStat(STAT_Strength) * 0.3f + statMod.GetPawnStat(STAT_Dexterity) * 0.7f) / 100.f;
        speedDelta  = (statMod.GetPawnStat(STAT_Dexterity)) / 500.f;
        break;

    case WEAPSUB_Mace:
    case WEAPSUB_Sword:
        dmgDelta    = (statMod.GetPawnStat(STAT_Strength) * 0.9f + statMod.GetPawnStat(STAT_Dexterity) * 0.1f) / 100.f;
        speedDelta  = (statMod.GetPawnStat(STAT_Dexterity)) / 2000.f;
        break;
    }

    desc->MinDamage += dmgDelta;
    desc->MaxDamage += dmgDelta;
    desc->SpeedInSeconds -= speedDelta;
}


//////////////////////////////////////////////////////////////////////////
// ARMOR
//////////////////////////////////////////////////////////////////////////

static IDStringsType    ArmorStrings[] = 
{
    /*ID_ARMO_Unknown,*/                { "Unknown", "", "" },
    /*ID_ARMO_BeltOfRedemption*/        { "Belt of Redemption", "", "icons/armor/belt_of_redemption.png" },
    /*ID_ARMO_BootsOfRedemption*/       { "Boots of Redemption", "", "icons/armor/boots_of_redemption.png" },
    /*ID_ARMO_BreastplateOfRedemption*/ { "Breastplate of Redemption", "", "icons/armor/breastplate_of_redemption.png" },
    /*ID_ARMO_CloakOfRedemption*/       { "Cloak of Redemption", "", "icons/armor/cloak_of_redemption.png" },
    /*ID_ARMO_GauntletsOfRedemption*/   { "Gauntlets of Redemption", "", "icons/armor/gauntlets_of_redemption.png" },
    /*ID_ARMO_HelmOfRedemption*/        { "Helm of Redemption", "", "icons/armor/helm_of_redemption.png" },
    /*ID_ARMO_LeggingsOfRedemption*/    { "Leggings of Redemption", "", "icons/armor/leggings_of_redemption.png" },
    /*ID_ARMO_ShouldersOfRedemption,*/  { "Shoulders of Redemption", "", "icons/armor/shoulders_of_redemption.png" },
    /*ID_ARMO_ShieldOfLight,*/          { "Shield of Light", "", "icons/armor/shield_of_light.png" },
    /*ID_ARMO_BandOfHope,*/             { "Band of Hope", "", "icons/armor/band_of_hope.png" },                         
    /*ID_ARMO_ShadowForestRing,*/       { "Shadow Forest Ring", "", "icons/armor/shadow_forest_ring.png" },             
    /*ID_ARMO_TearOfSacrifice,*/        { "Tears of Sacrifice", "", "icons/armor/tears_of_sacrifice.png" },   
    /*ID_ARMO_DarkAnkh,*/               { "Dark Ankh", "Use: Increases Shadow damage by 5% for 20 sec. 2 Min Cooldown", "icons/armor/dark_ankh.png" },                               
    /*ID_ARMO_GyroTimepiece,*/          { "Gyro Timepiece", "Use: Increases Haste rating by 10% for 20 sec. 2 Min Cooldown", "icons/armor/gyro_timepiece.png" },               
};
static IDDescType      ArmorFlags[] = 
{
    /*ID_ARMO_Unknown,*/                {   false,  0,  false,  false,  false,  0 },
    /*ID_ARMO_BeltOfRedemption*/        {   true,   0,  false,  false,  true,   ARMOR_Waist },
    /*ID_ARMO_BootsOfRedemption*/       {   true,   0,  false,  false,  true,   ARMOR_Feet },
    /*ID_ARMO_BreastplateOfRedemption*/ {   true,   0,  false,  false,  true,   ARMOR_Chest },
    /*ID_ARMO_CloakOfRedemption*/       {   true,   0,  false,  false,  true,   ARMOR_Back },
    /*ID_ARMO_GauntletsOfRedemption*/   {   true,   0,  false,  false,  true,   ARMOR_Hands },
    /*ID_ARMO_HelmOfRedemption*/        {   true,   0,  false,  false,  true,   ARMOR_Head },
    /*ID_ARMO_LeggingsOfRedemption*/    {   true,   0,  false,  false,  true,   ARMOR_Legs },
    /*ID_ARMO_ShouldersOfRedemption,*/  {   true,   0,  false,  false,  true,   ARMOR_Shoulders },
    /*ID_ARMO_ShieldOfLight,*/          {   true,   0,  false,  false,  true,   ARMOR_OffHand },
    /*ID_ARMO_BandOfHope,*/             {   true,   0,  false,  false,  true,   ARMOR_Finger },
    /*ID_ARMO_ShadowForestRing,*/       {   true,   0,  false,  false,  true,   ARMOR_Finger },
    /*ID_ARMO_TearOfSacrifice,*/        {   true,   0,  false,  false,  true,   ARMOR_Neck },
    /*ID_ARMO_DarkAnkh,*/               {   true,   0,  false,  false,  true,   ARMOR_Trinket },
    /*ID_ARMO_GyroTimepiece,*/          {   true,   0,  false,  false,  true,   ARMOR_Trinket },
};

static ArmorDescType   ArmorsDesc[] =
{
    //                                          Quality         Binding   LvlRq    Co  Si  Go         Class           Armor      Buffs  (value, interval (ms), duration)
    //                                                                                                                           Hp         	MaxHP        	Mp           	MaxMp         	Str         	Dex          	Int          	End          	Wis          	Luck

    /*ID_ARMO_Unknown,*/                {   { QUAL_Poor,      BIND_None,     0,  { 12, 23, 34 } },  ARMOMAT_Unknown,   25.f,   { { 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_BeltOfRedemption*/        {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    1927.f,   { { 0, 0, 0 }, 	{ 104, 0, 0 }, 	{ 0, 0, 0 }, 	{ 48, 0, 0 }, 	{ 67, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 4, 0, 0 } } },
    /*ID_ARMO_BootsOfRedemption*/       {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    2355.f,   { { 0, 0, 0 }, 	{ 152, 0, 0 }, 	{ 0, 0, 0 }, 	{ 65, 0, 0 }, 	{ 75, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 10, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_BreastplateOfRedemption*/ {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    3559.f,   { { 0, 0, 0 }, 	{ 246, 0, 0 }, 	{ 0, 0, 0 }, 	{ 142, 0, 0 }, 	{ 155, 0, 0 }, 	{ 0, 0, 0 }, 	{ 41, 0, 0 }, 	{ 18, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_CloakOfRedemption*/       {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_None,     625.f,   { { 0, 0, 0 }, 	{ 65, 0, 0 }, 	{ 0, 0, 0 }, 	{ 36, 0, 0 }, 	{ 45, 0, 0 }, 	{ 0, 0, 0 }, 	{ 22, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 5, 0, 0 } } },
    /*ID_ARMO_GauntletsOfRedemption*/   {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    2141.f,   { { 0, 0, 0 }, 	{ 145, 0, 0 }, 	{ 0, 0, 0 }, 	{ 50, 0, 0 }, 	{ 96, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_HelmOfRedemption*/        {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    2784.f,   { { 0, 0, 0 }, 	{ 173, 0, 0 }, 	{ 0, 0, 0 }, 	{ 81, 0, 0 }, 	{ 123, 0, 0 },	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 21, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_LeggingsOfRedemption*/    {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    2998.f,   { { 0, 0, 0 }, 	{ 202, 0, 0 }, 	{ 0, 0, 0 }, 	{ 103, 0, 0 }, 	{ 135, 0, 0 }, 	{ 0, 0, 0 }, 	{ 35, 0, 0 }, 	{ 10, 0, 0 }, 	{ 0, 0, 0 }, 	{ 7, 0, 0 } } },
    /*ID_ARMO_ShouldersOfRedemption,*/  {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Mail,    2669.f,   { { 0, 0, 0 }, 	{ 168, 0, 0 }, 	{ 0, 0, 0 }, 	{ 78, 0, 0 }, 	{ 102, 0, 0 }, 	{ 0, 0, 0 }, 	{ 24, 0, 0 }, 	{ 0, 0, 0 }, 	{ 12, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_ShieldOfLight,*/          {   { QUAL_Rare, 	  BIND_Pickup,  12,  { 12, 23, 34 } },  ARMOMAT_Shield,  9209.f,   { { 0, 0, 0 }, 	{ 175, 0, 0 }, 	{ 0, 0, 0 }, 	{ 76, 0, 0 }, 	{ 55, 0, 0 }, 	{ 0, 0, 0 }, 	{ 23, 0, 0 }, 	{ 12, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_BandOfHope,*/             {   { QUAL_Rare,      BIND_Pickup,   8,  { 12, 23, 34 } },  ARMOMAT_None,       0.f,   { { 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 32, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 33, 0, 0 }, 	{ 0, 0, 0 }, 	{ 18, 0, 0 }, 	{ 13, 0, 0 } } },
    /*ID_ARMO_ShadowForestRing,*/       {   { QUAL_Uncommon,  BIND_Equip,    5,  { 12, 23, 34 } },  ARMOMAT_None,       0.f,   { { 0, 0, 0 }, 	{ 42, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 28, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 11, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_TearOfSacrifice,*/        {   { QUAL_Uncommon,  BIND_Equip,    5,  { 12, 23, 34 } },  ARMOMAT_None,       0.f,   { { 0, 0, 0 }, 	{ 62, 0, 0 }, 	{ 0, 0, 0 }, 	{ 25, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 15, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 } } },
    /*ID_ARMO_DarkAnkh,*/               {   { QUAL_Epic,  	  BIND_Equip,   15,  { 12, 23, 34 } },  ARMOMAT_None,       0.f,   { { 0, 0, 0 },	{ 0, 0, 0 }, 	{ 5, 1000, 0 },	{ 40, 0, 0 }, 	{ 0, 0, 0 }, 	{ 0, 0, 0 }, 	{ 8, 0, 0 }, 	{ 0, 0, 0 }, 	{ 4, 0, 0 }, 	{ 4, 0, 0 } } },
    /*ID_ARMO_GyroTimepiece,*/          {   { QUAL_Uncommon,  BIND_Equip,    5,  { 12, 23, 34 } },  ARMOMAT_None,       0.f,   { { 0, 0, 0 }, 	{ 38, 0, 0 }, 	{ 0, 0, 0 }, 	{ 10, 0, 0 }, 	{ 22, 0, 0 }, 	{ 14, 0, 0 }, 	{ 0, 0, 0 }, 	{ 5, 0, 0 }, 	{ 0, 0, 0 }, 	{ 8, 0, 0 } } },
};

ArmorDescType*    GetArmorDescForID(ArmorID armoId)
{
    return &ArmorsDesc[armoId];
}

void    GetInstanceArmorDesc(ArmorDescType* desc, ArmorID weapId, const EntityInstanceStatMod& statMod)
{
    *desc = *GetArmorDescForID(weapId);
    if (desc->Armor == 0) return;

    float armorDelta = 0.f;

    switch (desc->Class)
    {
    case ARMOMAT_Cloth:
        armorDelta    = (statMod.GetPawnStat(STAT_Endurance) * 0.2f + statMod.GetPawnStat(STAT_Wisdom) * 0.8f) / 100.f;
        break;

    case ARMOMAT_Leather:
        armorDelta    = (statMod.GetPawnStat(STAT_Endurance) * 0.2f + statMod.GetPawnStat(STAT_Dexterity) * 0.8f) / 50.f;
        break;

    case ARMOMAT_Mail:
        armorDelta    = (statMod.GetPawnStat(STAT_Endurance) * 0.4f + statMod.GetPawnStat(STAT_Strength) * 0.6f) / 10.f;
        break;

    case ARMOMAT_Plate:
        armorDelta    = (statMod.GetPawnStat(STAT_Endurance) * 0.2f + statMod.GetPawnStat(STAT_Strength) * 0.8f) / 5.f;
        break;

    case ARMOMAT_Shield:
        armorDelta    = (statMod.GetPawnStat(STAT_Endurance) * 0.2f + statMod.GetPawnStat(STAT_Strength) * 0.8f);
        break;
    }

    desc->Armor += armorDelta;
}




//////////////////////////////////////////////////////////////////////////
// CONSUMABLES
//////////////////////////////////////////////////////////////////////////

static IDStringsType    ConsumableStrings[] = 
{
    /*ID_CONS_Unknown,*/            { "Unknown", "", "" },          
    /*ID_CONS_BasicHealthPotion*/   { "Basic Health Potion", "A small vial of rejuvenative syrum that heals small wounds.", "icons/consumables/icon_health_potion.png" },             
    /*ID_CONS_BasicManaPotion*/     { "Basic Mana Potion", "A small vial of elixir that provides a small amount of mana.", "icons/consumables/icon_mana_potion.png" },             
};
static IDDescType      ConsumableFlags[] = 
{
    /*ID_CONS_Unknown,*/            {   false,  0,  false,  false,  false,  0 },
    /*ID_CONS_BasicHealthPotion*/   {   true,   0,  true,  true,  false,   0 },
    /*ID_CONS_BasicManaPotion*/     {   true,   0,  true,  true,  false,   0 },
};

static ConsumableDescType   ConsumablesDesc[] =
{
    //                                          Quality      Binding   LvlRq   Co  Si  Go         Buffs  (value, interval (ms), duration)
    //                                                                                               Hp          MaxHP        Mp           MaxMp         Str         Dex          Int          End          Wis          Luck

    /* ID_CONS_Unknown,             */  {   { QUAL_Poor,     BIND_None,  0,  { 12, 23, 0 } },   { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }  },
    /* ID_CONS_BasicHealthPotion,   */  {   { QUAL_Common,   BIND_None,  0,  { 12, 23, 0 } },   { { 100, 1000, 10000 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }  },
    /* ID_CONS_BasicManaPotion,     */  {   { QUAL_Common,   BIND_None,  0,  { 12, 23, 0 } },   { { 0, 0, 0 }, { 0, 0, 0 }, { 100, 500, 7000 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }  },
};

ConsumableDescType*     GetConsumableDescForID(ConsumableID consId)
{
    return &ConsumablesDesc[consId];
}



//////////////////////////////////////////////////////////////////////////
// CONTAINERS
//////////////////////////////////////////////////////////////////////////

static IDStringsType    ContainerStrings[] = 
{
    /*ID_CONT_Unknown,*/            { "Unknown", "", "" },          
    /*ID_CONT_Backpack*/            { "Backpack", "Main player container.", "icons/other/icon_backpack.png" },             
    /*ID_CONT_LeatherPouch*/        { "Pouch", "Very small container.", "icons/other/icon_leather_pouch.png" },  
    /*ID_CONT_LinenBag*/            { "Linen Bag", "Small container.", "icons/other/icon_linen_bag.png" },             
};
static IDDescType      ContainerFlags[] = 
{
    /*ID_CONT_Unknown,*/            {   false,  0,  false,  false,  false,  0 },
    /*ID_CONT_Backpack*/            {   false, 24,  false,  false,  false,  0 },
    /*ID_CONT_LeatherPouch*/        {   true,   4,  false,  false,  false,  0 },
    /*ID_CONT_LinenBag*/            {   true,   8,  false,  false,  false,  0 },
};

static EntityDescType   ContainersDesc[] =
{
    //                                  Quality         Binding   LvlRq    Co  Si  Go 
    
    /*ID_CONT_Unknown*/             {   QUAL_Poor,     BIND_None,  0,   { 12, 23, 34 } },
    /*ID_CONT_Backpack*/            {   QUAL_Rare,     BIND_None,  0,   { 12, 23, 34 } },
    /*ID_CONT_LeatherPouch*/        {   QUAL_Poor,     BIND_Equip, 0,   { 12, 23, 34 } },
    /*ID_CONT_LinenBag*/            {   QUAL_Common,   BIND_Equip, 0,   { 12, 23, 34 } },
};

EntityDescType*     GetContainerDescForID(ContainerID contId)
{
    return &ContainersDesc[contId];
}



//////////////////////////////////////////////////////////////////////////
// QUEST ITEMS
//////////////////////////////////////////////////////////////////////////

static IDStringsType    QuestItemStrings[] = 
{
    /*ID_QUES_Unknown,*/            { "Unknown", "", "" },          
    /*ID_QUE_GnarlsPelt*/           { "Gnarl's Pelt", "The hide of Gnarl. It glistens only in moonlight. Doctor Jasper may find a need for it", "icons/quest/gnarls_pelt.png" },             
    /*ID_QUES_MysteriousBook*/      { "Mysterious Book", "A dusty, worn out book that feels ominous to the touch.", "icons/quest/mysterious_book.png" }, 
};
static IDDescType      QuestItemFlags[] = 
{
    /*ID_QUES_Unknown,*/            {   false,  0,  false,  false,  false,  0 },
    /*ID_QUE_GnarlsPelt*/           {   true,   0,  false,  false,  false,  0 },
    /*ID_QUES_MysteriousBook*/      {   true,   0,  false,  false,  false,  0 },
};

static EntityDescType   QuestItemssDesc[] =
{
    //                                  Quality         Binding   LvlRq   Co  Si  Go

    /*ID_QUES_Unknown,*/            {   QUAL_Poor,     BIND_None,  0,   { 12, 23, 34 } },
    /*ID_QUE_GnarlsPelt*/           {   QUAL_Uncommon, BIND_None,  0,   { 12, 23, 34 } },
    /*ID_QUES_MysteriousBook*/      {   QUAL_Rare,     BIND_None,  0,   { 12, 23, 34 } },
};

EntityDescType*     GetQuestItemDescForID(QuestItemID questId)
{
    return &QuestItemssDesc[questId];
}



//////////////////////////////////////////////////////////////////////////
// MISC ITEMS
//////////////////////////////////////////////////////////////////////////

static IDStringsType    MiscItemStrings[] =
{
    /*ID_OTHE_Unknown,*/            { "Unknown", "", "" },          
    /*ID_OTHE_WolfBone*/            { "Wolf Bone", "Unremarkable bone from a highland wolf.", "icons/other/wolf_bone.png" },             
    /*ID_OTHE_HeavyStone,*/         { "Heavy Stone", "Useless rock.", "icons/other/heavy_stone.png" }, 
    /*ID_OTHE_RaptorEgg,*/          { "Raptor Egg", "Best eaten scrambled.", "icons/other/raptor_egg.png" }, 
    /*ID_OTHE_Weeds,*/              { "Weeds", "Cows love this stuff.", "icons/other/weeds.png" }, 
};
static IDDescType      MiscItemFlags[] =
{
    /*ID_OTHE_Unknown,*/            {   false,  0,  false,  false,  false,  0 },
    /*ID_OTHE_WolfBone*/            {   true,   0,  true,   false,  false,  0 },
    /*ID_OTHE_HeavyStone,*/         {   true,   0,  true,   false,  false,  0 },
    /*ID_OTHE_RaptorEgg,*/          {   true,   0,  true,   false,  false,  0 },
    /*ID_OTHE_Weeds,*/              {   true,   0,  true,   false,  false,  0 },
};

static EntityDescType   MiscItemsDesc[] =
{
    //                                 Quality         Binding   LvlRq    Co  Si  Go

    /*ID_OTHE_Unknown,*/            {   QUAL_Poor,     BIND_None,  0,   { 12, 23, 34 } },
    /*ID_OTHE_WolfBone*/            {   QUAL_Common,   BIND_None,  0,   { 12, 23, 34 } },
    /*ID_OTHE_HeavyStone,*/         {   QUAL_Poor,     BIND_None,  0,   { 12, 23, 34 } },
    /*ID_OTHE_RaptorEgg,*/          {   QUAL_Common,   BIND_None,  0,   { 12, 23, 34 } },
    /*ID_OTHE_Weeds,*/              {   QUAL_Poor,     BIND_None,  0,   { 12, 23, 34 } },
};

EntityDescType*     GetMiscItemDescForID(MiscItemID miscId)
{
    return &MiscItemsDesc[miscId];
}



//////////////////////////////////////////////////////////////////////////
// ENTITY BASE
//////////////////////////////////////////////////////////////////////////

IDStringsType*  GetStringsForID(const EntityDefnID& id)
{
    switch (id.Class)
    {
    case ENTITY_WeaponItem:     return &WeaponStrings[id.ID];
    case ENTITY_ArmorItem:      return &ArmorStrings[id.ID];

    case ENTITY_ConsumableItem: return &ConsumableStrings[id.ID];
    case ENTITY_QuestItem:      return &QuestItemStrings[id.ID];
    case ENTITY_ContainerItem:  return &ContainerStrings[id.ID];
    case ENTITY_MiscItem:       return &MiscItemStrings[id.ID];

    case ENTITY_Ability:        return &AbilityStrings[id.ID];

    default:
        {
            SF_ASSERT(0);
            return NULL;
        }
    }
}

IDDescType*    GetDescForID(const EntityDefnID& id)
{
    switch (id.Class)
    {
    case ENTITY_WeaponItem:     return &WeaponFlags[id.ID];
    case ENTITY_ArmorItem:      return &ArmorFlags[id.ID];

    case ENTITY_ConsumableItem: return &ConsumableFlags[id.ID];
    case ENTITY_QuestItem:      return &QuestItemFlags[id.ID];
    case ENTITY_ContainerItem:  return &ContainerFlags[id.ID];
    case ENTITY_MiscItem:       return &MiscItemFlags[id.ID];

    case ENTITY_Ability:        return NULL;
    
    default:
        {
            SF_ASSERT(0);
            return NULL;
        }
    }
}

EntityInstanceStatMod::EntityInstanceStatMod()
{
    memset(PawnStats, 0, sizeof(unsigned) * NumStatTypes);
}

unsigned    EntityInstanceStatMod::GetPawnStat(StatType stat) const
{
    return PawnStats[stat];
}

void        EntityInstanceStatMod::SetPawnStat(StatType stat, unsigned val)
{
    PawnStats[stat] = val;
}



//////////////////////////////////////////////////////////////////////////
// SLOTS
//////////////////////////////////////////////////////////////////////////

bool    SlotSupportsEquipmentClass(EquipmentSlotType slot, const EntityDefnID& defnId)
{
    // Check slot type against entity class
    switch (slot)
    {
    case Game::EQS_Head:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Head);
        } 
        break;
    case Game::EQS_Neck:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Neck);
        } 
        break;
    case Game::EQS_Shoulders:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Shoulders);
        } 
        break;
    case Game::EQS_Chest:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Chest);
        } 
        break;
    case Game::EQS_Back:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Back);
        } 
        break;
    case Game::EQS_Waist:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Waist);
        } 
        break;
    case Game::EQS_Hands:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Hands);
        } 
        break;
    case Game::EQS_Legs:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Legs);
        } 
        break;
    case Game::EQS_Feet:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Feet);
        } 
        break;
    case Game::EQS_Ring0:
    case Game::EQS_Ring1:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Finger);
        } 
        break;
    case Game::EQS_Trinket0:
    case Game::EQS_Trinket1:
        {
            return (defnId.Class == Game::ENTITY_ArmorItem &&
                Game::GetDescForID(defnId)->SubClass == Game::ARMOR_Trinket);
        } 
        break;
    case Game::EQS_MainHand:
        {
            return (defnId.Class == Game::ENTITY_WeaponItem &&
                ( Game::GetDescForID(defnId)->SubClass == Game::WEAP_Melee1H ||
                Game::GetDescForID(defnId)->SubClass == Game::WEAP_Melee2H ));
        } 
        break;
    case Game::EQS_OffHand:
        {
            return (defnId.Class == Game::ENTITY_WeaponItem && Game::GetDescForID(defnId)->SubClass == Game::WEAP_Melee1H) ||
                (defnId.Class == Game::ENTITY_ArmorItem && Game::GetDescForID(defnId)->SubClass == Game::ARMOR_OffHand);
        } 
        break;
    case Game::EQS_Ranged:
        {
            return (defnId.Class == Game::ENTITY_WeaponItem &&
                Game::GetDescForID(defnId)->SubClass == Game::WEAP_Ranged);
        } 
        break;
    }

    SF_ASSERT(0);
    return false;
}


//////////////////////////////////////////////////////////////////////////
// FORMATTED STRINGS
//////////////////////////////////////////////////////////////////////////

const char*     getBindingTypeString(EntityBindType type)
{
    switch (type)
    {
    case BIND_Equip:    return "Binds when equipped";
    case BIND_Pickup:   return "Binds when picked up";
    default:            return "???";
    }
}

const char*     getWeaponTypeString(WeaponType type)
{
    switch (type)
    {
    case WEAP_Ranged:   return "Ranged";
    case WEAP_Melee1H:  return "One-Hand";
    case WEAP_Melee2H:  return "Two-Hand";
    default:            return "???";
    }
}

const char*     getWeaponTypeSubGroupString(WeaponTypeSubGroup type)
{
    switch (type)
    {
    case WEAPSUB_CrossBow:  return "Crossbow";
    case WEAPSUB_Dagger:    return "Dagger";
    case WEAPSUB_LongBow:   return "Longbow";
    case WEAPSUB_Mace:      return "Mace";
    case WEAPSUB_ShortBow:  return "Shortbow";
    case WEAPSUB_Sword:     return "Sword";
    default:                return "???";
    }
}

const char*     getEntityBuffTypeString(EntityBuffType type)
{
    switch (type)
    {
    case BUFF_Hp:           return "Health";
    case BUFF_MaxHp:        return "Max Health";
    case BUFF_Mp:           return "Mana";
    case BUFF_MaxMp:        return "Max Mana";
    case BUFF_Strength:     return "Strength";
    case BUFF_Dexterity:    return "Dexterity";
    case BUFF_Intelligence: return "Intelligence";
    case BUFF_Endurance:    return "Endurance";
    case BUFF_Wisdom:       return "Wisdom";
    case BUFF_Luck:         return "Luck";
    default:                return "???";
    }
}

const char*     getEntityBuffTypeStringLower(EntityBuffType type)
{
    switch (type)
    {
    case BUFF_Hp:           return "health";
    case BUFF_MaxHp:        return "max health";
    case BUFF_Mp:           return "mana";
    case BUFF_MaxMp:        return "max mana";
    case BUFF_Strength:     return "strength";
    case BUFF_Dexterity:    return "dexterity";
    case BUFF_Intelligence: return "intelligence";
    case BUFF_Endurance:    return "endurance";
    case BUFF_Wisdom:       return "wisdom";
    case BUFF_Luck:         return "luck";
    default:                return "???";
    }
}

const char*     getQualityTypeColorString(EntityQualityType type)
{
    switch (type)
    {
    case QUAL_Poor:         return "#808080";
    case QUAL_Common:       return "#FFFFFF";
    case QUAL_Uncommon:     return "#85DB02";
    case QUAL_Rare:         return "#0098FF";
    case QUAL_Epic:         return "#CC32FF";
    case QUAL_Legendary:    return "#FF6600";
    default:                return "#000000";
    }
}

const char*    getArmorLocationString(ArmorType type)
{
    switch (type)
    {
    case ARMOR_Head:        return "Head";
    case ARMOR_Shoulders:   return "Shoulders";
    case ARMOR_Chest:       return "Chest";
    case ARMOR_Back:        return "Back";
    case ARMOR_Waist:       return "Waist";
    case ARMOR_Hands:       return "Hands";
    case ARMOR_Legs:        return "Legs";
    case ARMOR_Feet:        return "Feet";
    case ARMOR_OffHand:     return "Off Hand";
    case ARMOR_Finger:      return "Finger";
    case ARMOR_Neck:        return "Neck";
    case ARMOR_Trinket:     return "Trinket";
    default:                return "???";
    }
}

const char*    getArmorMaterialString(ArmorClassType type)
{
    switch (type)
    {
    case ARMOMAT_None:      return "";
    case ARMOMAT_Cloth:     return "Cloth";
    case ARMOMAT_Leather:   return "Leather";
    case ARMOMAT_Mail:      return "Mail";
    case ARMOMAT_Plate:     return "Plate";
    case ARMOMAT_Shield:    return "Shield";
    default:                return "???";
    }
}

float           getTicksInSeconds(UInt32 ticks)
{
    return ticks / 1000.f;
}

void    generateQualityBaseHeader(StringBuffer* buffer, IDStringsType* strings, EntityQualityType qualType)
{
    String temp;

    Format(*buffer, "<b><font color='{0}'>{1}</font></b>\n", getQualityTypeColorString(qualType), ConvToUTF8(strings->Name).ToCStr());
    if (SFstrlen(ConvToUTF8(strings->Description).ToCStr()) > 0)
        Format(temp, "<i>{0}</i>\n\n", ConvToUTF8(strings->Description).ToCStr());
    else
        Format(temp, "\n");
    *buffer += temp;
}

void    generateSellPrice(StringBuffer* buffer, const EntityDescType& desc, unsigned stack)
{
    if ( (desc.SellPrice[MONEY_Copper] + desc.SellPrice[MONEY_Silver] > 0 + desc.SellPrice[MONEY_Gold]) == 0 ) return;
    if (stack == 0) return;

    String temp, copper, silver, gold;

    // We need to change sell price based on item stack sz
    unsigned    price[NumMoneyTypes];
    unsigned    carry = 0;
    for (unsigned i = 0; i < NumMoneyTypes; i++)
    {
        unsigned val = carry + desc.SellPrice[i] * stack;
        carry = val / 100;
        price[i] = val % 100;
    }
    price[NumMoneyTypes-1] += carry;

    if (price[MONEY_Copper] > 0)
        Format(copper, " {0}<img src='icon_copper' vspace='-4'/>", price[MONEY_Copper]);
    if (price[MONEY_Silver] > 0)
        Format(silver, " {0}<img src='icon_silver' vspace='-4'/>", price[MONEY_Silver]);
    if (price[MONEY_Gold] > 0)
        Format(gold, " {0}<img src='icon_gold' vspace='-4'/>", price[MONEY_Gold]);

    Format(temp, "Sell Price:{0}{1}{2}\n", gold, silver, copper);
    *buffer += temp;
}

void    generateMinLevel(StringBuffer* buffer, const EntityDescType& desc)
{
    String temp;
    if (desc.MinLevel > 0)
        Format(temp, "Requires Level {0}\n", desc.MinLevel);
    *buffer += temp;
}

void    generateUseBuffSummary(StringBuffer* buffer, EntityBuffDescType* buffs)
{
    String temp;
    bool first = true;
    for (unsigned i = 0; i < NumEntityBuffTypes; i++)
    {
        if (buffs[i].BuffValue > 0)
        {
            if (first)
            {
                Format(temp, "<font color='#33FF33'>Use: ");
                *buffer += temp;
                first = false;
            }
            Format(temp, "{0} {1} by {2}", buffs[i].BuffValue > 0 ? "Increases" : "Decreases", 
                getEntityBuffTypeStringLower((EntityBuffType)i), buffs[i].BuffValue);
            *buffer += temp;
            if (buffs[i].BuffInterval > 0)
            {
                Format(temp, " every {0:.2:g} seconds", buffs[i].BuffInterval / 1000.f);
                *buffer += temp;
            }
            if (buffs[i].BuffTicks > 0)
            {
                Format(temp, " for {0:.2:g} seconds", buffs[i].BuffTicks / 1000.f);
                *buffer += temp;
            }
            Format(temp, ". ");
            *buffer += temp;
        }
    }
    if (!first)
    {
        Format(temp, "</font>\n");
        *buffer += temp;
    }
}

void    generateBuffList(StringBuffer* buffer, EntityBuffDescType* buffs)
{
    String temp;
    for (unsigned i = 0; i < NumEntityBuffTypes; i++)
    {
        if (buffs[i].BuffValue > 0)
        {
            Format(temp, "<font color='#33FF33'>{0:+} {1}", buffs[i].BuffValue, getEntityBuffTypeString((EntityBuffType)i));
            *buffer += temp;
            if (buffs[i].BuffInterval > 0)
                Format(temp, " every {0:0.2:g} seconds</font>\n", getTicksInSeconds(buffs[i].BuffInterval));
            else
                Format(temp, "</font>\n");
            *buffer += temp;
        }
    }
}

void    generateAbilityDescription(StringBuffer* buffer, const char* descStr, const AbilityDescType& abilDesc)
{
    String temp, temp2;
    Format(temp, descStr, (unsigned)abilDesc.MinDamageOrHeal, (unsigned)abilDesc.MaxDamageOrHeal);
    Format(temp2, "<font color='#FFF380'>{0}</font>", temp);
    *buffer += temp2;
}

void    generateAbilityManaCostAndRange(StringBuffer* buffer, const AbilityDescType& abilDesc)
{
    if ( (abilDesc.ManaCostConstant + abilDesc.ManaCostPercent + abilDesc.Range) == 0 ) return;

    String temp;
    bool twoColmn = (abilDesc.ManaCostConstant > 0 || abilDesc.ManaCostPercent > 0) && abilDesc.Range > 0;

    if (twoColmn)
        *buffer += "<textformat leading='-12'><p>";

    if (abilDesc.ManaCostPercent > 0)
    {
        Format(temp, "{0}%% of base mana", abilDesc.ManaCostPercent);
        *buffer += temp;
    }
    else if (abilDesc.ManaCostConstant > 0)
    {
        Format(temp, "{0} mana", abilDesc.ManaCostConstant);
        *buffer += temp;
    }

    if (twoColmn)
        *buffer += "</p></textformat><p align='right'>\t\t";

    if (abilDesc.Range > 0)
    {
        Format(temp, "{0:.2:g} yd range", abilDesc.Range);
        *buffer += temp;
    }

    if (twoColmn)
        *buffer += "</p>";
    else
        *buffer += "\n";      
}

void    generateTimeFromTicks(String* buffer, UInt32 ticks)
{
    if (ticks >= 60000)
    {
        // minutes
        Format(*buffer, "{0:.2:g} min", ticks / 60000.f);
    }
    else if (ticks >= 1000)
    {
        // seconds
        Format(*buffer, "{0:.2:g} sec", ticks / 1000.f);
    }
    else
        Format(*buffer, "{0} ms", ticks);
}

void    generateCastTimeAndCooldown(StringBuffer* buffer, const AbilityDescType& abilDesc)
{
    if ( (abilDesc.CastTime + abilDesc.Cooldown) == 0) return;

    String temp;
    bool twoColmn = (abilDesc.Cooldown > 0);
    
    if (twoColmn)
        *buffer += "<textformat leading='-12'><p>";

    if (abilDesc.CastTime> 0)
    {
        String temp2;
        generateTimeFromTicks(&temp2, abilDesc.CastTime);
        Format(temp, "{0} cast", temp2);
        *buffer += temp;
    }
    else
    {
        Format(temp, "Instant");
        *buffer += temp;
    }

    if (twoColmn)
        *buffer += "</p></textformat><p align='right'>\t\t";

    if (abilDesc.Cooldown > 0)
    {
        String temp2;
        generateTimeFromTicks(&temp2, abilDesc.Cooldown);
        Format(temp, "{0} cooldown", temp2);
        *buffer += temp;
    }

    if (twoColmn)
        *buffer += "</p>";
    else
        *buffer += "\n";
}

void    GetFormattedDescriptionForID(StringBuffer* buffer, const EntityDefnID& id, 
                                     const EntityInstanceStatMod& params, unsigned stack)
{
    String temp;
    IDStringsType*  strings = Game::GetStringsForID(id);
    IDDescType*     desc    = Game::GetDescForID(id);

    switch (id.Class)
    {
    case ENTITY_WeaponItem:
        {
            WeaponDescType weapDesc;
            GetInstanceWeaponDesc(&weapDesc, (WeaponID)id.ID, params);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, weapDesc.EntityDesc.QualityType);
            // Binding
            if (weapDesc.EntityDesc.BindType != BIND_None)
            {
                Format(temp, "{0}\n", getBindingTypeString(weapDesc.EntityDesc.BindType));
                *buffer += temp;
            }
            // Weapon type and subtype group
            Format(temp, "<textformat leading='-12'><p>{0}</p></textformat><p align='right'>\t\t{1}</p>", 
                getWeaponTypeString((WeaponType)desc->SubClass), getWeaponTypeSubGroupString(weapDesc.SubGroup));
            *buffer += temp;
            // Min damage/max damage
            Format(temp, "<textformat leading='-12'><p>{0:.0} - {1:.0} Damage</p></textformat><p align='right'>\t\tSpeed {2:0.2}</p>", 
                weapDesc.MinDamage, weapDesc.MaxDamage, weapDesc.SpeedInSeconds);
            *buffer += temp;
            // DPS
            float dps = CalculateWeaponDPS(weapDesc);
            Format(temp, "({0:.1} damage per second)\n", dps);
            *buffer += temp;
            // Buffs
            generateBuffList(buffer, weapDesc.EquipBuffs);
            // Min level
            generateMinLevel(buffer, weapDesc.EntityDesc);
            // Sell price
            generateSellPrice(buffer, weapDesc.EntityDesc, stack);
            return;
        }
    case ENTITY_ArmorItem:
        {
            ArmorDescType armorDesc;
            GetInstanceArmorDesc(&armorDesc, (ArmorID)id.ID, params);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, armorDesc.EntityDesc.QualityType);
            // Binding
            if (armorDesc.EntityDesc.BindType != BIND_None)
            {
                Format(temp, "{0}\n", getBindingTypeString(armorDesc.EntityDesc.BindType));
                *buffer += temp;
            }
            // Armor location and material
            Format(temp, "<textformat leading='-12'><p>{0}</p></textformat><p align='right'>\t\t{1}</p>", 
                getArmorLocationString((ArmorType)desc->SubClass), getArmorMaterialString(armorDesc.Class));
            *buffer += temp;
            // Armor
            if (armorDesc.Armor > 0)
            {
                Format(temp, "{0:.0} Armor\n", armorDesc.Armor);
                *buffer += temp;
            }
            // Buffs
            generateBuffList(buffer, armorDesc.EquipBuffs);
            // Min level
            generateMinLevel(buffer, armorDesc.EntityDesc);
            // Sell price
            generateSellPrice(buffer, armorDesc.EntityDesc, stack);
            return;
        }
    case ENTITY_ConsumableItem:
        {
            ConsumableDescType* consDesc = GetConsumableDescForID((ConsumableID)id.ID);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, consDesc->EntityDesc.QualityType);
            // Min level
            generateMinLevel(buffer, consDesc->EntityDesc);
            // Use effects
            generateUseBuffSummary(buffer, consDesc->UseEffects);
            // Sell price
            generateSellPrice(buffer, consDesc->EntityDesc, stack);
            return;
        }
    case ENTITY_ContainerItem:
        {
            EntityDescType* contDesc = GetContainerDescForID((ContainerID)id.ID);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, contDesc->QualityType);
            // Binding
            if (contDesc->BindType != BIND_None)
            {
                Format(temp, "{0}\n", getBindingTypeString(contDesc->BindType));
                *buffer += temp;
            }
            // Slots
            Format(temp, "{0} Slot Bag\n", desc->Capacity);
            *buffer += temp;
            // Sell price
            generateSellPrice(buffer, *contDesc, stack);
            return;
        }
    case ENTITY_QuestItem:
        {
            EntityDescType* quesDesc = GetQuestItemDescForID((QuestItemID)id.ID);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, quesDesc->QualityType);
            // Quest Item
            Format(temp, "<font color='#FFF380'>Quest Item</font>");
            *buffer += temp;
            return;
        }
    case ENTITY_MiscItem:
        {
            EntityDescType* miscDesc = GetMiscItemDescForID((MiscItemID)id.ID);

            // Name and desc
            generateQualityBaseHeader(buffer, strings, miscDesc->QualityType);
            // Sell price
            generateSellPrice(buffer, *miscDesc, stack);
            return;
        }
    case ENTITY_Ability:
        {
            AbilityDescType abilDesc;
            GetInstanceAbilDesc(&abilDesc, (AbilityID)id.ID, params);

            // Name
            Format(temp, "<b>{0}</b>\n", ConvToUTF8(strings->Name).ToCStr());
            *buffer += temp;
            // Mana cost and range
            generateAbilityManaCostAndRange(buffer, abilDesc);
            // Cast time and cooldown
            generateCastTimeAndCooldown(buffer, abilDesc);
            // Description
            *buffer += "\n";
            generateAbilityDescription(buffer, ConvToUTF8(strings->Description).ToCStr(), abilDesc);
            return;
        }
    default:
        {
            break;
        }
    }

    SF_ASSERT(0);
}



//////////////////////////////////////////////////////////////////////////
// CALCULATIONS
//////////////////////////////////////////////////////////////////////////

float   CalculateWeaponDPS(const WeaponDescType& weapDesc)
{
    return ((weapDesc.MaxDamage - weapDesc.MinDamage) / 2.f) / weapDesc.SpeedInSeconds;
}

} // namespace Game
