/**************************************************************************

Filename    :   GameCommon.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameCommon_H
#define INC_GameCommon_H

#include "Kernel/SF_String.h"
#include "Kernel/SF_Array.h"
using namespace Scaleform;

namespace Game {


//////////////////////////////////////////////////////////////////////////

enum InfoType
{
    INFO_ID,
    INFO_Name,
    INFO_Class,
    INFO_Race,

    NumInfoTypes,
};

enum StatType
{
    STAT_Hp,
    STAT_MaxHp,
    STAT_Mp,
    STAT_MaxMp,
    
    STAT_Strength,
    STAT_Dexterity,
    STAT_Intelligence,
    STAT_Endurance,
    STAT_Wisdom,
    STAT_Luck,
    
    STAT_Level,
    STAT_Xp,

    NumStatTypes,
};

enum MoneyType
{
    MONEY_Copper    = 0,
    MONEY_Silver,
    MONEY_Gold,

    NumMoneyTypes,
};

enum MessageType
{
    MSG_System,
    MSG_Error,
    MSG_Combat,
    MSG_Guild,
    MSG_Party,
    MSG_Whisper,
    MSG_Say,
    MSG_Yell,
    MSG_Other,
};


//////////////////////////////////////////////////////////////////////////
// ABILITIES
//////////////////////////////////////////////////////////////////////////

enum AbilityGroupType
{
    ABIL_Unknown,
    ABIL_General,
    ABIL_Elemental,
    ABIL_LifeBane,
    ABIL_Restoration,

    NumAbilityGroups,
};

struct AbilityDescType
{
    unsigned                MinLevel;
    Game::AbilityGroupType  GroupID;
    float                   MinDamageOrHeal;
    float                   MaxDamageOrHeal;
    unsigned                ManaCostPercent;
    unsigned                ManaCostConstant;
    float                   Range;
    UInt32                  CastTime;
    UInt32                  Cooldown;
    UInt32                  GlobalCooldown;
    bool                    IgnoresGlobalCooldown;
};

enum AbilityID
{
    ID_ABIL_Unknown,

    // * ABILITIES
    // - General
    ID_ABIL_Attack,
    ID_ABIL_RangedAttack,
    ID_ABIL_Adrenaline,
    ID_ABIL_Sprint,
    ID_ABIL_Float,
    ID_ABIL_Portal,
    ID_ABIL_PaladinsSacrifice,
    ID_ABIL_Berserk,
    ID_ABIL_OffensiveBanner,
    ID_ABIL_DefensiveBanner,

    // - RESTORATION
    ID_ABIL_Smite,
    ID_ABIL_Raise,
    ID_ABIL_Heal,
    ID_ABIL_Faith,
    ID_ABIL_Sanctuary,
    ID_ABIL_DivineLight,
    ID_ABIL_Protect,
    ID_ABIL_AngelsBlessing,
    ID_ABIL_SoulLance,
    ID_ABIL_Cure,

    // - ELEMENTAL
    ID_ABIL_Thunder,
    ID_ABIL_FireBolt,
    ID_ABIL_Blizzard,
    ID_ABIL_Freeze,
    ID_ABIL_Tsunami,
    ID_ABIL_Tornado,
    ID_ABIL_Quake,
    ID_ABIL_QuantumField,
    ID_ABIL_HailStorm,
    ID_ABIL_Gravity,

    // - LIFE BANE
    ID_ABIL_Leech,
    ID_ABIL_Fear,
    ID_ABIL_Curse,
    ID_ABIL_Terror,
    ID_ABIL_Weaken,
    ID_ABIL_SoulBlast,
    ID_ABIL_Plague,
    ID_ABIL_SoulDrain,
    ID_ABIL_Infect,
    ID_ABIL_Corruption,

    NumAbilityIDs,
};

AbilityDescType*    GetAbilityDescForID(AbilityID abilId);
void                GetAbilitiesByGroup(Array<AbilityID>* abilIds, AbilityGroupType abilGroupId);
const char*         GetAbilityGroupNameFromID(AbilityGroupType abilGroupId);



//////////////////////////////////////////////////////////////////////////
// ENTITY BASE
//////////////////////////////////////////////////////////////////////////

enum EntityBaseClass
{
    ENTITY_Unknown,

    ENTITY_WeaponItem,
    ENTITY_ArmorItem,

    ENTITY_ConsumableItem,
    ENTITY_QuestItem,
    ENTITY_ContainerItem,
    ENTITY_MiscItem,

    ENTITY_Ability,
};

struct  EntityDefnID
{
    EntityBaseClass     Class;
    unsigned            ID;

    EntityDefnID() : Class(ENTITY_Unknown), ID(0) {} 
    EntityDefnID(EntityBaseClass cls, unsigned id) : Class(cls), ID(id) {}
    bool    operator==(const EntityDefnID& e) const { return Class == e.Class && ID == e.ID; }
    bool    operator!=(const EntityDefnID& e) const { return Class != e.Class || ID != e.ID; }
};

struct IDStringsType
{
    const char*     Name;
    const char*     Description;
    const char*     IconPath;
};

struct IDDescType
{
    bool            CanPawnHold;
    unsigned        Capacity;
    bool            Stackable;
    bool            Usable;
    bool            Wearable;
    unsigned        SubClass;
};

IDStringsType*  GetStringsForID(const EntityDefnID& id);
IDDescType*     GetDescForID(const EntityDefnID& id);

enum    EntityBindType
{
    BIND_None,
    BIND_Pickup,
    BIND_Equip,
};

enum    EntityBuffType
{
    BUFF_Hp,
    BUFF_MaxHp,
    BUFF_Mp,
    BUFF_MaxMp,
    BUFF_Strength,
    BUFF_Dexterity,
    BUFF_Intelligence,
    BUFF_Endurance,
    BUFF_Wisdom,
    BUFF_Luck,

    NumEntityBuffTypes,
};

struct  EntityBuffDescType
{
    int         BuffValue;      // Modifier
    unsigned    BuffInterval;   // 0 means applied once at start
    unsigned    BuffTicks;      // Duration; 0 is constant effect
};

enum    EntityQualityType
{
    QUAL_Poor,
    QUAL_Common,
    QUAL_Uncommon,
    QUAL_Rare,
    QUAL_Epic,
    QUAL_Legendary,
};

struct EntityDescType
{
    EntityQualityType   QualityType;
    EntityBindType      BindType;
    unsigned            MinLevel;   // 0 means no level req
    unsigned            SellPrice[NumMoneyTypes];
};

class EntityInstanceStatMod
{
public:
    EntityInstanceStatMod();

    unsigned    GetPawnStat(StatType stat) const;
    void        SetPawnStat(StatType stat, unsigned val);

protected:
    unsigned    PawnStats[NumStatTypes];
};



//////////////////////////////////////////////////////////////////////////
// WEAPONS
//////////////////////////////////////////////////////////////////////////

enum    WeaponType
{
    WEAP_Melee1H,
    WEAP_Melee2H,
    WEAP_Ranged,

    NumWeaponTypes,
};

enum    WeaponTypeSubGroup
{
    WEAPSUB_Unknown,
    WEAPSUB_Sword,
    WEAPSUB_Mace,
    WEAPSUB_Dagger,
    WEAPSUB_LongBow,
    WEAPSUB_ShortBow,
    WEAPSUB_CrossBow,
};

struct WeaponDescType
{
    EntityDescType              EntityDesc;
    WeaponTypeSubGroup          SubGroup;
    float                       MinDamage;
    float                       MaxDamage;
    float                       SpeedInSeconds;
    EntityBuffDescType  EquipBuffs[NumEntityBuffTypes];
};

enum WeaponID 
{
    ID_WEAP_Unknown,
    ID_WEAP_SwordOfReckoning,
    ID_WEAP_HammerOfRighteousness,
    ID_WEAP_IronCrossbow,
};

WeaponDescType* GetWeaponDescForID(WeaponID weapId);
void            GetInstanceWeaponDesc(WeaponDescType* desc, WeaponID weapId, const EntityInstanceStatMod& statMod);



//////////////////////////////////////////////////////////////////////////
// ARMOR
//////////////////////////////////////////////////////////////////////////

enum ArmorType
{
    ARMOR_Head,
    ARMOR_Shoulders,
    ARMOR_Chest,
    ARMOR_Back,
    ARMOR_Waist,
    ARMOR_Hands,
    ARMOR_Legs,
    ARMOR_Feet,
    ARMOR_OffHand,
    ARMOR_Finger,
    ARMOR_Neck,
    ARMOR_Trinket,

    NumArmorTypes,
};

enum ArmorClassType
{
    ARMOMAT_Unknown,
    ARMOMAT_None,
    ARMOMAT_Cloth,
    ARMOMAT_Leather,
    ARMOMAT_Mail,
    ARMOMAT_Plate,
    ARMOMAT_Shield,
};

struct ArmorDescType
{
    EntityDescType          EntityDesc;
    ArmorClassType          Class;
    float                   Armor;
    EntityBuffDescType      EquipBuffs[NumEntityBuffTypes];
};

enum ArmorID
{
    ID_ARMO_Unknown,
    // * ARMOR
    ID_ARMO_BeltOfRedemption,
    ID_ARMO_BootsOfRedemption,
    ID_ARMO_BreastplateOfRedemption,
    ID_ARMO_CloakOfRedemption,
    ID_ARMO_GauntletsOfRedemption,
    ID_ARMO_HelmOfRedemption,
    ID_ARMO_LeggingsOfRedemption,
    ID_ARMO_ShouldersOfRedemption,
    // * SHIELDS
    ID_ARMO_ShieldOfLight,
    // * ARMOLLERY
    ID_ARMO_BandOfHope,
    ID_ARMO_ShadowForestRing,
    ID_ARMO_TearOfSacrifice,
    // * ARMOKETS
    ID_ARMO_DarkAnkh,
    ID_ARMO_GyroTimepiece,
};

ArmorDescType*  GetArmorDescForID(ArmorID armorId);
void            GetInstanceArmorDesc(ArmorDescType* desc, ArmorID armoId, const EntityInstanceStatMod& statMod);



//////////////////////////////////////////////////////////////////////////
// CONSUMABLES
//////////////////////////////////////////////////////////////////////////

enum ConsumableID
{
    ID_CONS_Unknown,

    // * CONSUMABLE
    ID_CONS_BasicHealthPotion,
    ID_CONS_BasicManaPotion,
};

struct ConsumableDescType
{
    EntityDescType              EntityDesc;
    EntityBuffDescType          UseEffects[NumEntityBuffTypes];
};

ConsumableDescType* GetConsumableDescForID(ConsumableID consId);



//////////////////////////////////////////////////////////////////////////
// CONTAINERS
//////////////////////////////////////////////////////////////////////////

enum ContainerID
{
    ID_CONT_Unknown,

    // * CONTAINERS
    ID_CONT_Backpack,
    ID_CONT_LeatherPouch,
    ID_CONT_LinenBag,
};

EntityDescType* GetContainerDescForID(ContainerID contId);



//////////////////////////////////////////////////////////////////////////
// QUEST ITEMS
//////////////////////////////////////////////////////////////////////////

enum QuestItemID
{
    ID_QUES_Unknown,

    // * QUEST
    ID_QUES_GnarlsPelt,
    ID_QUES_MysteriousBook,
};

EntityDescType* GetQuestItemDescForID(QuestItemID quesId);


//////////////////////////////////////////////////////////////////////////
// MISC ITEMS
//////////////////////////////////////////////////////////////////////////

enum MiscItemID
{
    ID_OTHE_Unknown,

    // * OTHER
    ID_OTHE_WolfBone,
    ID_OTHE_HeavyStone,
    ID_OTHE_RaptorEgg,
    ID_OTHE_Weeds,
};

EntityDescType* GetMiscItemDescForID(MiscItemID miscId);


//////////////////////////////////////////////////////////////////////////
// SLOTS
//////////////////////////////////////////////////////////////////////////

enum SlotType
{
    SLOT_Unknown,
    SLOT_Entity,
    SLOT_Equipment,
    SLOT_Inventory,

    NumSlotTypes,
};

enum EquipmentSlotType
{
    EQS_Head = 0,
    EQS_Shoulders,
    EQS_Chest,
    EQS_Back,
    EQS_Hands,
    EQS_Waist,
    EQS_Legs,
    EQS_Feet,
    EQS_MainHand,
    EQS_OffHand,
    EQS_Ranged,
    EQS_Neck,
    EQS_Ring0,
    EQS_Ring1,
    EQS_Trinket0,
    EQS_Trinket1,

    NumEquipmentSlotTypes,
};

enum InventorySlotType
{
    INV_Slot0,
    INV_Slot1,
    INV_Slot2,
    INV_Slot3,
    INV_Slot4,

    NumInventorySlots,
};

struct EntitySlotDescriptor
{
    SlotType    Type;
    unsigned    Index;
    String      EntityID;
    
    EntitySlotDescriptor() : Type(SLOT_Unknown) {}
    EntitySlotDescriptor(const char* entId, unsigned idx) : Type(SLOT_Entity), EntityID(entId), Index(idx) {}
    EntitySlotDescriptor(SlotType type, unsigned idx) : Type(type), Index(idx) {}
};


bool    SlotSupportsEquipmentClass(EquipmentSlotType slot, const EntityDefnID& defnId);



//////////////////////////////////////////////////////////////////////////
// FORMATTED STRINGS
//////////////////////////////////////////////////////////////////////////

void    GetFormattedDescriptionForID(StringBuffer* buffer, const EntityDefnID& id, 
                                     const EntityInstanceStatMod& params, unsigned stack);



//////////////////////////////////////////////////////////////////////////
// CALCULATIONS
//////////////////////////////////////////////////////////////////////////

float   CalculateWeaponDPS(const WeaponDescType& weapDesc);

}   // namespace Game


//
// Helper class for wchar_t* support in Format
//
class ConvToUTF8
{
public:
    enum { MaxStackSz = 1024 };

    ConvToUTF8(const char* str) : pStr(str), Managed(false) {}
    ConvToUTF8(const wchar_t* str) : Managed(false)
    {
        UPInt len = SFwcslen(str);
        UPInt sz = UTF8Util::GetEncodeStringSize(str, len);
        char* buff = pBuff;
        if (sz > MaxStackSz - 1)
        {
            buff = new char[sz];
            Managed = true;
        }
        UTF8Util::EncodeString(buff, str, sz);
        pStr = buff;
    }
    virtual ~ConvToUTF8()
    {
        if (Managed) delete[] pStr;
    }

    const char* ToCStr() const   { return pStr; }

protected:
    const char*     pStr;
    char            pBuff[MaxStackSz];
    bool            Managed;
};

#endif  // INC_GameCommon_H
