/**************************************************************************

Filename    :   GameUIDataBindings.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameUIAdapter.h"
#include "../Game/GameClient.h"
#include "../Game/GameCommon.h"
#include "../Game/GameConnection.h"
#include "../../Src/Kernel/SF_Debug.h"

#include <time.h>

//////////////////////////////////////////////////////////////////////////

static void     CreateUISlotDataForAbility(GFx::Value* retVal, GFx::Movie* movie, unsigned abilId)
{
    String tooltipStr;
    movie->CreateObject(retVal);
    Game::EntityDefnID id;
    id.Class = Game::ENTITY_Ability;
    id.ID = abilId;
    retVal->SetMember("name", Game::GetStringsForID(id)->Name);
    retVal->SetMember("iconPath", Game::GetStringsForID(id)->IconPath);
}

static void    CreateUISlotDataForEntity(GFx::Value* retVal, GFx::Movie* movie, const GameClientEntityRef& data)
{
    if (data.IsNull)
    {
        retVal->SetNull();
    }
    else
    {
        movie->CreateObject(retVal);
        retVal->SetMember("id", data.EntityID.ToCStr());
        retVal->SetMember("name", Game::GetStringsForID(data.DefnID)->Name);
        retVal->SetMember("count", (float)data.StackCount);
        retVal->SetMember("iconPath", Game::GetStringsForID(data.DefnID)->IconPath);
    }
}

static void     CreateUISlotDataForEntityDefn(GFx::Value* retVal, GFx::Movie* movie, 
                                              const Game::EntityDefnID& defn, unsigned count)
{
    movie->CreateObject(retVal);
    retVal->SetMember("name", Game::GetStringsForID(defn)->Name);   
    retVal->SetMember("count", (float)count);    
    retVal->SetMember("iconPath", Game::GetStringsForID(defn)->IconPath);
}

//////////////////////////////////////////////////////////////////////////

enum DW_Type
{
    DWT_Base        = 0,
    DWT_SlotBase    = 1,
    DWT_EntitySlotBase = 2,
    DWT_AbilitySlot = 3,
    DWT_ActionSlot  = 4,
};

class DW_Base : public CLIKDataWrapper
{
public:
    DW_Base(GameClient* c) : Client(c) {}
    virtual ~DW_Base() {}
    virtual DW_Type     GetType() const     { return DWT_Base; }
protected:
    GameClient*   Client;
};

static bool     IsSlotWrapper(DW_Base* dw)
{
    return (dw->GetType() >= DWT_SlotBase);
}

class DW_SlotBaseW : public DW_Base
{
public:
    DW_SlotBaseW(GameClient* c, unsigned idx) : DW_Base(c), SlotIdx(idx) {}
    unsigned            GetIndex() const    { return SlotIdx; }
    virtual DW_Type     GetType() const     { return DWT_SlotBase; }

    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)     = 0;
    virtual bool    GetTooltip(StringBuffer* s)                               = 0;
    virtual void    DropSlotItem(DW_SlotBaseW* src)                     = 0;
    virtual void    UseSlot()                                           = 0;

protected:
    unsigned    SlotIdx;
};

class DW_EntitySlotBase : public DW_SlotBaseW
{
public:
    DW_EntitySlotBase(GameClient* c, unsigned idx) : DW_SlotBaseW(c, idx) {}
    virtual DW_Type         GetType() const     { return DWT_EntitySlotBase; }

    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        GameClientSlotRef* slot = GetSlot();
        if (slot)
            CreateUISlotDataForEntity(value, pmovie, slot->GetData());
    }
    virtual bool    GetTooltip(StringBuffer* s)
    {
        GameClientSlotRef* slot = GetSlot();
        if (slot)
        {
            Game::EntityInstanceStatMod fparams;
            Client->GetState()->GetPlayerStatsForParams(&fparams);
            Game::GetFormattedDescriptionForID(s, slot->GetData().DefnID, fparams, slot->GetData().StackCount);
            return true;
        }
        return false;
    }
    virtual void    DropSlotItem(DW_SlotBaseW* src)
    {
        if (src->GetType() != DWT_EntitySlotBase) return;
        DW_EntitySlotBase* esb = (DW_EntitySlotBase*)src;
        GameClientSlotRef* srcSlot = esb->GetSlot();
        GameClientSlotRef* destSlot = GetSlot();
        if (srcSlot && destSlot)
        {
            GameClientRequest* r = GameClientRequest::CreatePlayerSlotSwapRequest(Client->GetClientID(), esb->GetSlot()->GetDesc(), GetSlot()->GetDesc());
            Client->GetConnection()->SendRequest(r);
        }
    }
    virtual void    UseSlot()
    {
        GameClientSlotRef* slot = GetSlot();
        if (slot)
        {
            GameClientRequest* r = GameClientRequest::CreatePlayerSlotUseRequest(Client->GetClientID(), GetSlot()->GetDesc());
            Client->GetConnection()->SendRequest(r);
        }
    }
    virtual GameClientSlotRef* GetSlot() const             = 0;
};

//////////////////////////////////////////////////////////////////////////

class DW_PlayerInfo : public DW_Base
{
public:
    DW_PlayerInfo(GameClient* c, Game::InfoType s) : DW_Base(c), Info(s) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetString(Client->GetState()->GetPlayerInfo(Info));
    }
protected:
    Game::InfoType  Info;
};

class DW_PlayerStat : public DW_Base
{
public:
    DW_PlayerStat(GameClient* c, Game::StatType s) : DW_Base(c), Stat(s) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetNumber(Client->GetState()->GetPlayerStat(Stat));
    }
protected:
    Game::StatType    Stat;
};

class DW_PlayerMoney : public DW_Base 
{
public:
    DW_PlayerMoney(GameClient* c, Game::MoneyType s) : DW_Base(c), Money(s) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetNumber(Client->GetState()->GetPlayerMoney(Money));
    }
protected:
    Game::MoneyType     Money;
};

//////////////////////////////////////////////////////////////////////////

class DW_TargetActive : public DW_Base
{
public:
    DW_TargetActive(GameClient* c) : DW_Base(c) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetBoolean(SFstrlen(Client->GetState()->GetTargetInfo(Game::INFO_ID)) > 0);
    }
};

class DW_TargetInfo : public DW_PlayerInfo
{
public:
    DW_TargetInfo(GameClient* c, Game::InfoType s) : DW_PlayerInfo(c, s) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetString(Client->GetState()->GetTargetInfo(Info));
    }
};

class DW_TargetStat : public DW_PlayerStat
{
public:
    DW_TargetStat(GameClient* c, Game::StatType s) : DW_PlayerStat(c, s) {}
    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        SF_UNUSED(pmovie);
        value->SetNumber(Client->GetState()->GetTargetStat(Stat));
    }
};

//////////////////////////////////////////////////////////////////////////

class DW_EquipmentSlot : public DW_EntitySlotBase
{
    // Returns descriptor for an equipment slot item
    // - {object}   : Equipment item entity data
public:
    DW_EquipmentSlot(GameClient* c, unsigned slotIdx) : DW_EntitySlotBase(c, slotIdx) {}
    virtual GameClientSlotRef* GetSlot() const
    {
        return Client->GetState()->GetPlayerEquipmentItem((Game::EquipmentSlotType)SlotIdx);
    }
    virtual void    OnAddHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        SF_ASSERT(desc.BoundObj.IsObject());        
        Client->GetUIAdapter()->RegisterEquipmentSlotMC((Game::EquipmentSlotType)SlotIdx, desc.BoundObj);
        SF_DEBUG_MESSAGE1(1, "Adding slot reference for %s\n", binding);
    }
    virtual void    OnRemoveHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        SF_ASSERT(desc.BoundObj.IsObject());
        Client->GetUIAdapter()->UnregisterEquipmentSlotMC((Game::EquipmentSlotType)SlotIdx, desc.BoundObj);
        SF_DEBUG_MESSAGE1(1, "Removing slot reference for %s\n", binding);
    }
    static void     GetBindingID(String* s, unsigned slotIdx)
    {
        Format(*s, "equipment.slot{0}", slotIdx);
    }
};

//////////////////////////////////////////////////////////////////////////

class DW_AbilitySlot : public DW_SlotBaseW
{
    // Returns descriptor for an ability slot
    // - {object}   : Ability data
public:
    DW_AbilitySlot(GameClient* c, unsigned abilId) : DW_SlotBaseW(c, abilId) {}
    virtual DW_Type         GetType() const     { return DWT_AbilitySlot; }

    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        CreateUISlotDataForAbility(value, pmovie, SlotIdx);
    }
    virtual bool    GetTooltip(StringBuffer* s)
    {
        Game::EntityDefnID id;
        id.Class = Game::ENTITY_Ability;
        id.ID = SlotIdx;
        Game::EntityInstanceStatMod fparams;
        Client->GetState()->GetPlayerStatsForParams(&fparams);
        Game::GetFormattedDescriptionForID(s, id, fparams, 0);
        return true;
    }
    virtual void    DropSlotItem(DW_SlotBaseW* src)
    {
        SF_UNUSED(src);
        // Do nothing. Cannot drop into this slot.
    }
    virtual void    UseSlot()
    {
        GameClientRequest* r = GameClientRequest::CreatePlayerAbilityUseRequest(Client->GetClientID(), SlotIdx);
        Client->GetConnection()->SendRequest(r);
    }
    virtual void    OnAddHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        SF_ASSERT(desc.BoundObj.IsObject());        
        Client->GetUIAdapter()->RegisterAbilitySlotMC((Game::AbilityID)SlotIdx, desc.BoundObj);
        SF_DEBUG_MESSAGE1(1, "Adding slot reference for %s\n", binding);
    }
    virtual void    OnRemoveHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        SF_ASSERT(desc.BoundObj.IsObject());
        Client->GetUIAdapter()->UnregisterAbilitySlotMC((Game::AbilityID)SlotIdx, desc.BoundObj);
        SF_DEBUG_MESSAGE1(1, "Removing slot reference for %s\n", binding);
    }
    static void     GetBindingID(String* s, unsigned abilId)
    {
        Format(*s, "ability.slot{0}", abilId);
    }
};

//////////////////////////////////////////////////////////////////////////

class DW_ActionSlot : public DW_SlotBaseW
{
    // Returns descriptor for the item linked by the action slot
    // - {object}   : Ability or entity data
public:
    DW_ActionSlot(GameClient* c, unsigned barIdx, unsigned slotIdx) : DW_SlotBaseW(c, slotIdx), BarIdx(barIdx) 
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        if (destBar)
        {
            const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());
            if (destLink.GetType() == GameUISlotLink::LINK_Ability) 
                removeAbilitySlotMCs(destLink.GetAbilityID());
        }
    }
    virtual DW_Type         GetType() const     { return DWT_ActionSlot; }

    virtual void    GetValue(GFx::Movie* pmovie, GFx::Value* value)
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());
        
        switch (destLink.GetType())
        {
        case GameUISlotLink::LINK_Ability:
            {
                CreateUISlotDataForAbility(value, pmovie, destLink.GetAbilityID());
                break;
            }
        case GameUISlotLink::LINK_Entity:
            {
                const GameClientEntityRef& data = destLink.GetEntityData();
                CreateUISlotDataForEntity(value, pmovie, data);
                break;
            }
        case GameUISlotLink::LINK_EntityDefn:
            {
                unsigned count = Client->GetState()->GetEntityCountForDefn(destLink.GetDefnID());
                CreateUISlotDataForEntityDefn(value, pmovie, destLink.GetDefnID(), count);
                break;
            }
        case GameUISlotLink::LINK_Empty:
            {
                value->SetNull();
                break;
            }
        }
    }
    virtual bool    GetTooltip(StringBuffer* s)
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());

        switch (destLink.GetType())
        {
        case GameUISlotLink::LINK_Ability:
            {
                Game::EntityDefnID id;
                id.Class = Game::ENTITY_Ability;
                id.ID = destLink.GetAbilityID();
                Game::EntityInstanceStatMod fparams;
                Client->GetState()->GetPlayerStatsForParams(&fparams);
                Game::GetFormattedDescriptionForID(s, id, fparams, 0);
                return true;
            }
        case GameUISlotLink::LINK_Entity:
            {
                Game::EntityInstanceStatMod fparams;
                Client->GetState()->GetPlayerStatsForParams(&fparams);
                Game::GetFormattedDescriptionForID(s, destLink.GetEntityData().DefnID, 
                    fparams, destLink.GetEntityData().StackCount);
                return true;
            }
        case GameUISlotLink::LINK_EntityDefn:
            {
                Game::EntityInstanceStatMod fparams;
                Client->GetState()->GetPlayerStatsForParams(&fparams);
                Game::GetFormattedDescriptionForID(s, destLink.GetDefnID(), 
                    fparams, 0);
                return true;
            }
        }
        return false;
    }
    virtual void    DropSlotItem(DW_SlotBaseW* src)
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());

        if (destLink.GetType() == GameUISlotLink::LINK_Ability) 
            removeAbilitySlotMCs(destLink.GetAbilityID());

         if (src->GetType() == DWT_EntitySlotBase)
         {
             DW_EntitySlotBase* srcSLot = (DW_EntitySlotBase*)src;
             Game::IDDescType* desc = Game::GetDescForID(srcSLot->GetSlot()->GetData().DefnID);
             SF_ASSERT(desc);
             // If the item is a stackable type, then we create a link for the whole type
            if (desc->Stackable)
            {
                GameUISlotLink newLink(srcSLot->GetSlot()->GetData().DefnID);
                destBar->SetSlot(GetIndex(), newLink);
            }
            // Else it will be a direct link to a specific item
            else
            {
                GameUISlotLink newLink(srcSLot->GetSlot()->GetData());
                destBar->SetSlot(GetIndex(), newLink);
            }
         }
         else if (src->GetType() == DWT_AbilitySlot)
         {
             DW_AbilitySlot* srcSLot = (DW_AbilitySlot*)src;
             GameUISlotLink newLink((Game::AbilityID)srcSLot->GetIndex());
             destBar->SetSlot(GetIndex(), newLink);            
         }
         else if (src->GetType() == DWT_ActionSlot)
         {
             DW_ActionSlot* srcSlot = (DW_ActionSlot*)src;             
             GameUIActionBar* srcBar = bmgr.GetBar(srcSlot->GetBarIdx());
             GameUISlotLink srcLink = srcBar->GetSlot(srcSlot->GetIndex());
             SF_ASSERT(!srcLink.IsEmpty());

             if (srcLink.GetType() == GameUISlotLink::LINK_Ability)
                 srcSlot->removeAbilitySlotMCs(srcLink.GetAbilityID());
             //
            // Swap slots
            srcBar->SetSlot(srcSlot->GetIndex(), destLink);
            destBar->SetSlot(GetIndex(), srcLink);
            //
            srcLink = srcBar->GetSlot(srcSlot->GetIndex());
            if (srcLink.GetType() == GameUISlotLink::LINK_Ability)
                srcSlot->addAbilitySlotMCs(srcLink.GetAbilityID());
         }

         if (destLink.GetType() == GameUISlotLink::LINK_Ability) 
             addAbilitySlotMCs(destLink.GetAbilityID());
    }
    void    DeleteSlotItem()
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());

        if (destLink.GetType() == GameUISlotLink::LINK_Ability) 
            removeAbilitySlotMCs(destLink.GetAbilityID());

        SF_ASSERT(!destLink.IsEmpty());
        destBar->ClearSlot(GetIndex());
    }
    virtual void    UseSlot()
    {
        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());

        if (destLink.IsEmpty()) return;
        switch (destLink.GetType())
        {
        case GameUISlotLink::LINK_Ability:
            {
                GameClientRequest* r = GameClientRequest::CreatePlayerAbilityUseRequest(Client->GetClientID(), destLink.GetAbilityID());
                Client->GetConnection()->SendRequest(r);
                break;
            }
        case GameUISlotLink::LINK_Entity:
            {                
                Game::EntitySlotDescriptor slotDesc;
                bool found = Client->GetState()->GetSlotDescriptorForEntity(&slotDesc, destLink.GetEntityData().EntityID);
                SF_UNUSED(found);
                SF_ASSERT(found);
                GameClientRequest* r = GameClientRequest::CreatePlayerSlotUseRequest(Client->GetClientID(), slotDesc);
                Client->GetConnection()->SendRequest(r);
                break;
            }
        case GameUISlotLink::LINK_EntityDefn:
            {
                Game::EntitySlotDescriptor slotDesc;
                bool found = Client->GetState()->GetSlotDescriptorForEntityDefn(&slotDesc, destLink.GetDefnID());
                if (!found) return;
                GameClientRequest* r = GameClientRequest::CreatePlayerSlotUseRequest(Client->GetClientID(), slotDesc);
                Client->GetConnection()->SendRequest(r);
                break;
            }
        }
    }
    virtual void    OnAddHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        SlotMCs.PushBack(desc.BoundObj);

        GameUIActionBarManager& bmgr = Client->GetUIAdapter()->GetActionBarManager();
        GameUIActionBar* destBar = bmgr.GetBar(GetBarIdx());
        const GameUISlotLink& destLink = destBar->GetSlot(GetIndex());
        if (destLink.GetType() == GameUISlotLink::LINK_Ability) 
            Client->GetUIAdapter()->RegisterAbilitySlotMC((Game::AbilityID)destLink.GetAbilityID(), desc.BoundObj);
    }
    virtual void    OnRemoveHandler(const char* binding, const CLIKDataBindingHandlerDesc& desc)
    {
        SF_UNUSED(binding);
        int idx = -1;
        for (unsigned i = 0; i < SlotMCs.GetSize(); i++)
        {
            if (SlotMCs[i] == desc.BoundObj)
            {
                idx = i;
                break;
            }
        }
        if (idx >= 0) SlotMCs.RemoveAt(idx);
    }
    static void     GetBindingID(String* s, unsigned barIdx, unsigned slotIdx)
    {
        Format(*s, "actionbar{0}.slot{1}", barIdx, slotIdx);
    }
    unsigned    GetBarIdx() const   { return BarIdx; }
protected:
    unsigned            BarIdx;
    ArrayCPP<GFx::Value>    SlotMCs;

    void    addAbilitySlotMCs(unsigned abilId)
    {
        for (unsigned i = 0; i < SlotMCs.GetSize(); i++)
            Client->GetUIAdapter()->RegisterAbilitySlotMC((Game::AbilityID)abilId, SlotMCs[i]);
    }
    void    removeAbilitySlotMCs(unsigned abilId)
    {
        for (unsigned i = 0; i < SlotMCs.GetSize(); i++)
            Client->GetUIAdapter()->UnregisterAbilitySlotMC((Game::AbilityID)abilId, SlotMCs[i]);
    }
};

//////////////////////////////////////////////////////////////////////////

class DW_BagItemSlot : public DW_EntitySlotBase
{
    // Returns descriptor for a bag slot item
    // - {object}   : Bag item entity data
public:
    DW_BagItemSlot(GameClient* c, GameClientBagSlotRef* bag, unsigned slotIdx) : DW_EntitySlotBase(c, slotIdx), Bag(bag) {}
    virtual GameClientSlotRef* GetSlot() const
    {
        return Bag->GetSlot(SlotIdx);
    }
    static void     GetSlotBindingID(String* s, GameClientBagSlotRef* bag, unsigned slotIdx)
    {
        Format(*s, "bag_{0}.slot{1}", bag->GetData().EntityID, slotIdx);
    }
protected:
    Ptr<GameClientBagSlotRef>    Bag;
};

class DW_BagSlot : public DW_EntitySlotBase
{
    // Returns descriptor for a bag slot
    // - {object}   : Bag entity data
public:
    DW_BagSlot(GameClient* c, unsigned idx) : DW_EntitySlotBase(c, idx) {}
    virtual GameClientSlotRef* GetSlot() const
    {
        return Client->GetState()->GetPlayerInventoryItem((Game::InventorySlotType)SlotIdx);
    }
    static void     GetBindingID(String* s, unsigned slotIdx)
    {
        Format(*s, "bags.slot{0}", slotIdx);
    }
};

void    GameUIAdapter::OnGetBagSlotBindings(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Bag entity ID
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* bagId = params.pArgs[0].GetString();
    GameClientBagSlotRef* bag = pthis->Client->GetState()->FindBagByID(bagId);
    if (!bag)
    {
        printf("Cannot get slot bindings for bag:%s. Unknown entity.\n", bagId);
        return;
    }
    //
    // Returns descriptor for a bag container
    // - {array}
    //   - Binding ID for the bag's item slot
    GFx::Movie*  movie = params.pMovie;
    movie->CreateArray(params.pRetVal);
    for (unsigned i = 0; i < bag->GetNumSlots(); i++)
    {
        GFx::Value slotBinding;
        String s;
       DW_BagItemSlot::GetSlotBindingID(&s, bag, i);
        movie->CreateString(&slotBinding, s.ToCStr());
        params.pRetVal->PushBack(slotBinding);
   }
}

void    GameUIAdapter::OnGetSpellbookGroups(GameUIAdapter* pthis, ParamsType& params)
{
    // Returns array of spellbook group descriptors
    // - {array}
    //   - String for name
    //   - Number for Group id
    GFx::Movie* movie = params.pMovie;
    movie->CreateArray(params.pRetVal);
    const GameClientSpellBook& spellbook = pthis->Client->GetState()->GetPlayerSpellBook();
    for (unsigned i = 0; i < Game::NumAbilityGroups; i++)
    {
        Game::AbilityGroupType grpId = (Game::AbilityGroupType)i;
        if (spellbook.HasAbilityGroup(grpId))
        {
            GFx::Value desc;
            movie->CreateObject(&desc);
            desc.SetMember("label", Game::GetAbilityGroupNameFromID(grpId));
            desc.SetMember("id", (float)grpId);
            params.pRetVal->PushBack(desc);
        }
    }
}

void    GameUIAdapter::OnGetSpellsByGroup(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Bag entity ID
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsNumber());
    unsigned groupId = (unsigned)params.pArgs[0].GetNumber();

    // Returns array of spells in spell group
    // - {array}
    //   - binding
    //   - disabled
    //   - name
    //   - description
    //   - message
    GFx::Movie* movie = params.pMovie;
    movie->CreateArray(params.pRetVal);
    const GameClientSpellBook& spellbook = pthis->Client->GetState()->GetPlayerSpellBook();
    const GameClientAbilitiesList* abilGroup = spellbook.GetAbilityGroup((Game::AbilityGroupType)groupId);
    for (unsigned i = 0; i < abilGroup->GetNumAbilities(); i++)
    {
        const GameClientAbility& abil = abilGroup->GetAbility(i);
        Game::EntityDefnID id;
        id.Class = Game::ENTITY_Ability;
        id.ID = abil.AbilID;

        GFx::Value desc;
        movie->CreateObject(&desc);
        String s;
        DW_AbilitySlot::GetBindingID(&s, abil.AbilID);
        desc.SetMember("binding", s.ToCStr());
        desc.SetMember("disabled", abil.Rank == 0);
        desc.SetMember("name", Game::GetStringsForID(id)->Name);
        desc.SetMember("description", Game::GetStringsForID(id)->Description);
        String msg;
        if (abil.Rank == 0)
            Format(msg, "Unlocked at level {0}", Game::GetAbilityDescForID(abil.AbilID)->MinLevel);
        else
            Format(msg, "Rank {0}", abil.Rank);
        desc.SetMember("message", msg.ToCStr());
        params.pRetVal->PushBack(desc);
    }
}

//////////////////////////////////////////////////////////////////////////

void    GameUIAdapter::onNotifySlotDragStart(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Binding ID for slot
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* bindingId = params.pArgs[0].GetString();
    
    CLIKDataBindingAdapter* const dba = pthis->DataBindingAdapter;

    DW_SlotBaseW* sW = (DW_SlotBaseW*)dba->GetDataWrapper(bindingId);
    if (!sW || !IsSlotWrapper(sW))
    {
        printf("Slot drag? Binding %s does not represent a slot element.\n", bindingId);
        return;
    }

    if (sW->GetType() == DWT_EntitySlotBase)
    {
        SF_ASSERT(((DW_EntitySlotBase*)sW)->GetSlot());
        const GameClientEntityRef& srcData = ((DW_EntitySlotBase*)sW)->GetSlot()->GetData();
        // We have the entity class here
        // Check it against all registered slot handlers
        pthis->EquipmentSlots.HighlightAllForEntityClass(srcData.DefnID);
    }
}

void    GameUIAdapter::onNotifySlotDragEnd(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Binding ID for source slot
    // arg1: Binding ID for destination slot or NULL
    SF_ASSERT(params.ArgCount == 2);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* srcId = params.pArgs[0].GetString();
    const char* destId = params.pArgs[1].IsString() ? params.pArgs[1].GetString() : NULL;

    CLIKDataBindingAdapter* const dba = pthis->DataBindingAdapter;
    
    DW_SlotBaseW* sW = (DW_SlotBaseW*)dba->GetDataWrapper(srcId);
    if (!sW || !IsSlotWrapper(sW))
    {
        printf("Cannot swap slots. Source binding %s does not represent a slot element.\n", srcId);
        return;
    }
    DW_SlotBaseW* dW = (DW_SlotBaseW*)dba->GetDataWrapper(destId);
    if (dW)
    {
        if (!IsSlotWrapper(dW))
        {
            printf("Cannot swap slots. Destination binding %s does not represent a slot element.\n", destId);
            return;
        }
        dW->DropSlotItem(sW);
    }
    // Special case: Action slot items can be destroyed by dropping on something other than a slot
    else
    {
        if (sW->GetType() == DWT_ActionSlot)
        {
            DW_ActionSlot* actionSlot = (DW_ActionSlot*)sW;
            actionSlot->DeleteSlotItem();
        }
    }

    dba->NotifyDataChange(srcId);
    if (dW)
        dba->NotifyDataChange(destId);

    pthis->EquipmentSlots.UnhighlightAll();
}

void    GameUIAdapter::onNotifySlotUse(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Binding ID for slot
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* bindingId = params.pArgs[0].GetString();

    CLIKDataBindingAdapter* const dba = pthis->DataBindingAdapter;

    DW_SlotBaseW* sW = (DW_SlotBaseW*)dba->GetDataWrapper(bindingId);
    if (!sW || !IsSlotWrapper(sW))
    {
        printf("Cannot use slot. Binding %s does not represent a slot element.\n", bindingId);
        return;
    }

    sW->UseSlot();
}

void    GameUIAdapter::OnGetSlotTooltip(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Binding ID
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* bindingId = params.pArgs[0].GetString();

    CLIKDataBindingAdapter* const dba = pthis->DataBindingAdapter;

    DW_SlotBaseW* sW = (DW_SlotBaseW*)dba->GetDataWrapper(bindingId);
    if (!sW || !IsSlotWrapper(sW))
    {
        printf("Cannot return tooltip. Binding %s does not represent a slot element.\n", bindingId);
        return;
    }

    StringBuffer s;
    if (sW->GetTooltip(&s))
        params.pMovie->CreateString(params.pRetVal, s);
    else
        params.pRetVal->SetNull();
}

void    GameUIAdapter::OnRegisterCastBar(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: Castbar movieclip
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsDisplayObject());
    
    pthis->CastBar = *SF_NEW GameUICastBar(params.pArgs[0]);
}

void    GameUIAdapter::OnGetMessageLogs(GameUIAdapter* pthis, ParamsType& params)
{
    SF_UNUSED(pthis);

    // Returns array of log ids
    // - {array}
    GFx::Movie* movie = params.pMovie;
    movie->CreateArray(params.pRetVal);
    GameClientState* state = pthis->Client->GetState();
    const GameClientState::ChannelLogsType& logs = state->GetMessageLogs();
    for (unsigned i = 0; i < logs.GetSize(); i++)
    {
        GameClientStateMessageLog* log = logs[i];

        GFx::Value desc;
        movie->CreateObject(&desc);
        desc.SetMember("id", log->GetName().ToCStr());
        desc.SetMember("label", log->GetName().ToCStr());
        params.pRetVal->PushBack(desc);
    }
}

void    GameUIAdapter::OnRegisterMessageLogListener(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: handler (Closure)
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsClosure());
    GFx::Value& handler = params.pArgs[0];

    pthis->LogHandlers.PushBack(handler);
}

void    GameUIAdapter::OnUnregisterMessageLogListener(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: handler (Closure)
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsClosure());
    GFx::Value& handler = params.pArgs[0];

    int idx = -1;
    for (unsigned i = 0; i < pthis->LogHandlers.GetSize(); i++)
    {
        if (pthis->LogHandlers[i] == handler)
        {
            idx = i;
            break;
        }
    }
    if (idx != -1)
        pthis->LogHandlers.RemoveAt(idx);
}

void    GameUIAdapter::OnGetMessageLogData(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: logId (String)
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* logId = params.pArgs[0].GetString();

    // Returns string representing log data
    GFx::Movie* movie = params.pMovie;
    GameClientState* state = pthis->Client->GetState();
    GameClientStateMessageLog* log = state->GetMessageLog(logId);
    SF_ASSERT(log);
    StringBuffer buffer;
    log->GetData(&buffer);
    movie->CreateString(params.pRetVal, buffer.ToCStr());
}

void    GameUIAdapter::OnSubmitLogMessage(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: logId (String)
    // arg1: msg (String)
    SF_ASSERT(params.ArgCount == 2);
    SF_ASSERT(params.pArgs[0].IsString());
    SF_ASSERT(params.pArgs[1].IsString());
    const char* logId = params.pArgs[0].GetString();
    String msg = params.pArgs[1].GetString();
    String realId = 0;
    time_t  rawtime;

    GameClientStateMessageLog* log = pthis->Client->GetState()->GetMessageLog(logId);
    SF_ASSERT(log);

    StringDataPtr cmd(msg.ToCStr() + 1, 0);
    const char* realMsg = msg.ToCStr();

    if (msg.GetSize() == 0)
        return;

    // Command?
    if (*realMsg == '/')
    {
        realMsg++;

        // Extract command
        unsigned cmdLen = 0;
        while (*realMsg != ' ' && *realMsg != 0)
        {
            cmdLen++;
            realMsg++;
        }
        if (*realMsg != 0) realMsg++;

        cmd = StringDataPtr(msg.ToCStr() + 1, cmdLen);
        if (cmdLen == 0)
        {
            GameClientMsgBuffer* pchan = pthis->Client->GetState()->GetErrorBuffer();
            time(&rawtime);
            pchan->AddMessage(GameClientLogMsg("Unknown command!", rawtime));
            return;
        }
    }
    
    // TODO: Do something with commands
    //       Implement this using a hash of command processors. for now hardcoded
    GameClientStateMessageLog::OutputChannelDesc odesc;
    if (cmd.GetSize() > 0)
    {
        String command(cmd.ToCStr(), cmd.GetSize());
        if (command == "g")
        {
            odesc.Type = Game::MSG_Guild;
            log->SetOutputChannel(odesc);
        }
        else if (command == "p")
        {
            odesc.Type = Game::MSG_Party;
            log->SetOutputChannel(odesc);
        }
        else if (command == "s")
        {
            odesc.Type = Game::MSG_Say;
            log->SetOutputChannel(odesc);
        }
        else if (command == "y")
        {
            odesc.Type = Game::MSG_Yell;
            log->SetOutputChannel(odesc);
        }
        else if (command == "t")
        {           
            const char* tgs = realMsg;

            // Next token is the target
            unsigned targetLen = 0;
            while (*realMsg != ' ' && *realMsg != 0)
            {
                targetLen++;
                realMsg++;
            }
            if (*realMsg != 0) realMsg++;

            if (targetLen > 0)
            {
                odesc.Type = Game::MSG_Whisper;
                odesc.Target.AssignString(tgs, targetLen);
                log->SetOutputChannel(odesc);
            }
            else
            {
                GameClientMsgBuffer* pchan = pthis->Client->GetState()->GetErrorBuffer();
                time(&rawtime);
                pchan->AddMessage(GameClientLogMsg("Target not specified!", rawtime));
                return;
            }
        }
        else 
        {
            int localId = atoi(command.ToCStr());
            if (localId > 0)
            {               
                if (pthis->Client->GetState()->HasChatChannelForLocalId(localId))
                {
                    odesc.Type = Game::MSG_Other;
                    odesc.LocalID = localId;
                    log->SetOutputChannel(odesc);
                }
                else
                {
                    GameClientMsgBuffer* pchan = pthis->Client->GetState()->GetErrorBuffer();
                    time(&rawtime);
                    pchan->AddMessage(GameClientLogMsg("Channel does not exist!", rawtime));
                    return;
                }
            }
            else
            {
                GameClientMsgBuffer* pchan = pthis->Client->GetState()->GetErrorBuffer();
                time(&rawtime);
                pchan->AddMessage(GameClientLogMsg("Unknown command!", rawtime));
                return;
            }
        }
    }

    // No message?
    if (realMsg >= (msg.ToCStr() + msg.GetSize()))
        return;

    if (log->GetOutputChannel().Type == Game::MSG_Other)
    {
        realId = pthis->Client->GetState()->GetChatChannelIdFromLocalId(log->GetOutputChannel().LocalID);
    }
    
    GameClientRequest* r = GameClientRequest::CreatePlayerSendMessage(pthis->Client->GetClientID(), 
                                log->GetOutputChannel().Type, realId, realMsg, log->GetOutputChannel().Target);
    pthis->Client->GetConnection()->SendRequest(r);
}

void    GameUIAdapter::OnGetSubmitChannelInfo(GameUIAdapter* pthis, ParamsType& params)
{
    // arg0: logId (String)
    SF_ASSERT(params.ArgCount == 1);
    SF_ASSERT(params.pArgs[0].IsString());
    const char* logId = params.pArgs[0].GetString();

    GameClientStateMessageLog* log = pthis->Client->GetState()->GetMessageLog(logId);
    SF_ASSERT(log);

    GFx::Movie* movie = params.pMovie;
    movie->CreateObject(params.pRetVal);

    String name = "";
    switch (log->GetOutputChannel().Type)
    {
    case Game::MSG_Say:       name = "Say";     break;
    case Game::MSG_Yell:      name = "Yell";    break;
    case Game::MSG_Guild:     name = "Guild";   break;
    case Game::MSG_Party:     name = "Party";   break;
    case Game::MSG_Whisper:   name = log->GetOutputChannel().Target;  break;
    case Game::MSG_Other:     
        {
            GameClientMsgBuffer* chan = log->GetBuffer(Game::MSG_Other, log->GetOutputChannel().LocalID);
            Format(name, "[{0}. {1}]", log->GetOutputChannel().LocalID, chan->GetName());
        }
        break;
    }
    params.pRetVal->SetMember("name", name.ToCStr());
    params.pRetVal->SetMember("color", pthis->Client->GetMessageColor(log->GetOutputChannel().Type));
}


//////////////////////////////////////////////////////////////////////////

void    GameUIAdapter::Initialize(GameUIRuntimeAtlasManager* pimagecreator)
{
    IconAtlasManager = *pimagecreator;
    TheUI->SetImageCreator(IconAtlasManager);

    // Listen to chat log changes
    GameClientState* state = Client->GetState();
    const GameClientState::ChannelLogsType& logs = state->GetMessageLogs();
    for (unsigned i = 0; i < logs.GetSize(); i++)
    {
        GameClientStateMessageLog* log = logs[i];
        log->AddListener(this);
    }

    // Register the bound data
    registerBoundDataWrappers();

    // Let's add some stuff to the action slots. This data should come from a UI
    // settings registry/db.
    //
    addAbilityToActionBar(0, 0, Game::ID_ABIL_Attack);
    addAbilityToActionBar(0, 1, Game::ID_ABIL_RangedAttack);
    addAbilityToActionBar(0, 2, Game::ID_ABIL_Thunder);
    addAbilityToActionBar(0, 3, Game::ID_ABIL_Heal);
    addAbilityToActionBar(0, 4, Game::ID_ABIL_Adrenaline);
    addAbilityToActionBar(0, 5, Game::ID_ABIL_Plague);
    addAbilityToActionBar(0, 6, Game::ID_ABIL_FireBolt);
    addAbilityToActionBar(0, 7, Game::ID_ABIL_Blizzard);
    addAbilityToActionBar(0, 8, Game::ID_ABIL_Freeze);
    addAbilityToActionBar(0, 9, Game::ID_ABIL_Tsunami);
    addAbilityToActionBar(0, 10, Game::ID_ABIL_Tornado);
    addAbilityToActionBar(0, 11, Game::ID_ABIL_Quake);
    //
    addAbilityToActionBar(1, 0, Game::ID_ABIL_QuantumField);
    addAbilityToActionBar(1, 1, Game::ID_ABIL_DivineLight);
    addAbilityToActionBar(1, 2, Game::ID_ABIL_AngelsBlessing);
    addAbilityToActionBar(1, 3, Game::ID_ABIL_Protect);
    addAbilityToActionBar(1, 4, Game::ID_ABIL_Curse);
    addAbilityToActionBar(1, 10, Game::ID_ABIL_Portal);
    addAbilityToActionBar(1, 11, Game::ID_ABIL_Sprint);
    //
    addAbilityToActionBar(2, 0, Game::ID_ABIL_PaladinsSacrifice);
    addAbilityToActionBar(2, 1, Game::ID_ABIL_OffensiveBanner);
    addAbilityToActionBar(2, 2, Game::ID_ABIL_Leech);
    addAbilityToActionBar(2, 3, Game::ID_ABIL_Berserk);
    addAbilityToActionBar(2, 4, Game::ID_ABIL_Faith);
    //
    Game::EntityDefnID defnId(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicHealthPotion);
    addItemDefnToActionBar(1, 6, Game::EntityDefnID(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicHealthPotion));
    addItemDefnToActionBar(1, 7, Game::EntityDefnID(Game::ENTITY_ConsumableItem, Game::ID_CONS_BasicManaPotion));
}

void    GameUIAdapter::addAbilityToActionBar(unsigned barIdx, unsigned slotIdx, Game::AbilityID abilId)
{
    GameUIActionBar* bar = ActionBarManager.GetBar(barIdx);
    bar->SetSlot(slotIdx, GameUISlotLink(abilId));
    String s;
    DW_ActionSlot::GetBindingID(&s, barIdx, slotIdx);
    DataBindingAdapter->NotifyDataChange(s);
}

void    GameUIAdapter::addItemToActionBar(unsigned barIdx, unsigned slotIdx, const GameClientEntityRef& ent)
{
    GameUIActionBar* bar = ActionBarManager.GetBar(barIdx);
    bar->SetSlot(slotIdx, GameUISlotLink(ent));
    String s;
    DW_ActionSlot::GetBindingID(&s, barIdx, slotIdx);
    DataBindingAdapter->NotifyDataChange(s);
}

void    GameUIAdapter::addItemDefnToActionBar(unsigned barIdx, unsigned slotIdx, const Game::EntityDefnID& defn)
{
    GameUIActionBar* bar = ActionBarManager.GetBar(barIdx);
    bar->SetSlot(slotIdx, GameUISlotLink(defn));
    String s;
    DW_ActionSlot::GetBindingID(&s, barIdx, slotIdx);
    DataBindingAdapter->NotifyDataChange(s);
}

void    GameUIAdapter::registerBoundDataWrappers()
{
    // Install bindings
    DataBindingAdapter->RegisterBoundData("player.name",    *SF_NEW DW_PlayerInfo(Client, Game::INFO_Name));
    DataBindingAdapter->RegisterBoundData("player.class",    *SF_NEW DW_PlayerInfo(Client, Game::INFO_Class));
    DataBindingAdapter->RegisterBoundData("player.race",    *SF_NEW DW_PlayerInfo(Client, Game::INFO_Race));

    DataBindingAdapter->RegisterBoundData("player.level",   *SF_NEW DW_PlayerStat(Client, Game::STAT_Level));
    DataBindingAdapter->RegisterBoundData("player.hp",      *SF_NEW DW_PlayerStat(Client, Game::STAT_Hp));
    DataBindingAdapter->RegisterBoundData("player.maxhp",   *SF_NEW DW_PlayerStat(Client, Game::STAT_MaxHp));
    DataBindingAdapter->RegisterBoundData("player.mp",      *SF_NEW DW_PlayerStat(Client, Game::STAT_Mp));
    DataBindingAdapter->RegisterBoundData("player.maxmp",   *SF_NEW DW_PlayerStat(Client, Game::STAT_MaxMp));

    DataBindingAdapter->RegisterBoundData("player.gold",    *SF_NEW DW_PlayerMoney(Client, Game::MONEY_Gold));
    DataBindingAdapter->RegisterBoundData("player.silver",  *SF_NEW DW_PlayerMoney(Client, Game::MONEY_Silver));
    DataBindingAdapter->RegisterBoundData("player.copper",  *SF_NEW DW_PlayerMoney(Client, Game::MONEY_Copper));

    // Register bag slots
    for (int i = 0; i < Game::NumInventorySlots; i++)
    {
        String s;
        DW_BagSlot::GetBindingID(&s, i);
        DataBindingAdapter->RegisterBoundData(s.ToCStr(),   *SF_NEW DW_BagSlot(Client, i));
    }

    // Register equipment slots
    for (int i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        String s;
        DW_EquipmentSlot::GetBindingID(&s, i);
        DataBindingAdapter->RegisterBoundData(s.ToCStr(),   *SF_NEW DW_EquipmentSlot(Client, i));
    }

    // Register ability slots
    Array<Game::AbilityID> abilIds;
    for (int i = 0; i < Game::NumAbilityGroups; i++)
    {        
        Game::GetAbilitiesByGroup(&abilIds, (Game::AbilityGroupType)i);
        for (unsigned j = 0; j < abilIds.GetSize(); j++)
        {
            String s;
            DW_AbilitySlot::GetBindingID(&s, abilIds[j]);
            DataBindingAdapter->RegisterBoundData(s.ToCStr(),   *SF_NEW DW_AbilitySlot(Client, abilIds[j]));
        }
    }

    DataBindingAdapter->RegisterBoundData("target.active",  *SF_NEW DW_TargetActive(Client));
    DataBindingAdapter->RegisterBoundData("target.name",    *SF_NEW DW_TargetInfo(Client, Game::INFO_Name));
    DataBindingAdapter->RegisterBoundData("target.level",   *SF_NEW DW_TargetStat(Client, Game::STAT_Level));
    DataBindingAdapter->RegisterBoundData("target.hp",      *SF_NEW DW_TargetStat(Client, Game::STAT_Hp));
    DataBindingAdapter->RegisterBoundData("target.maxhp",   *SF_NEW DW_TargetStat(Client, Game::STAT_MaxHp));
    DataBindingAdapter->RegisterBoundData("target.mp",      *SF_NEW DW_TargetStat(Client, Game::STAT_Mp));
    DataBindingAdapter->RegisterBoundData("target.maxmp",   *SF_NEW DW_TargetStat(Client, Game::STAT_MaxMp));

    // Register action bar slots
    for (unsigned i = 0; i < ActionBarManager.GetNumBars(); i++)
    {
        GameUIActionBar* bar = ActionBarManager.GetBar(i);
        for (unsigned j = 0; j < bar->GetNumSlots(); j++)
        {
            String s;
            DW_ActionSlot::GetBindingID(&s, i, j);
            DataBindingAdapter->RegisterBoundData(s.ToCStr(),   *SF_NEW DW_ActionSlot(Client, i, j));
        }
    }
}

void    GameUIAdapter::HandlePlayerInfoChange(Game::InfoType p)
{
    switch (p)
    {
    case Game::INFO_ID: DataBindingAdapter->NotifyDataChange("player.id"); break;
    case Game::INFO_Name: DataBindingAdapter->NotifyDataChange("player.name"); break;
    case Game::INFO_Race: DataBindingAdapter->NotifyDataChange("player.race"); break;
    case Game::INFO_Class: DataBindingAdapter->NotifyDataChange("player.class"); break;
    }
}

void    GameUIAdapter::HandlePlayerStatChange(Game::StatType p)
{
    switch (p)
    {
    case Game::STAT_Hp: DataBindingAdapter->NotifyDataChange("player.hp"); break;
    case Game::STAT_MaxHp: DataBindingAdapter->NotifyDataChange("player.maxhp"); break;
    case Game::STAT_Mp: DataBindingAdapter->NotifyDataChange("player.mp"); break;
    case Game::STAT_MaxMp: DataBindingAdapter->NotifyDataChange("player.maxmp"); break;
    case Game::STAT_Strength: DataBindingAdapter->NotifyDataChange("player.str"); break;
    case Game::STAT_Dexterity: DataBindingAdapter->NotifyDataChange("player.dex"); break;
    case Game::STAT_Intelligence: DataBindingAdapter->NotifyDataChange("player.int"); break;
    case Game::STAT_Endurance: DataBindingAdapter->NotifyDataChange("player.end"); break;
    case Game::STAT_Wisdom: DataBindingAdapter->NotifyDataChange("player.wis"); break;
    case Game::STAT_Luck: DataBindingAdapter->NotifyDataChange("player.luck"); break;
    case Game::STAT_Level: DataBindingAdapter->NotifyDataChange("player.level"); break;
    case Game::STAT_Xp: DataBindingAdapter->NotifyDataChange("player.xp"); break;  
    }
}

void    GameUIAdapter::HandlePlayerMoneyChange(Game::MoneyType p)
{
    switch (p)
    {
    case Game::MONEY_Copper: DataBindingAdapter->NotifyDataChange("player.copper"); break;
    case Game::MONEY_Silver: DataBindingAdapter->NotifyDataChange("player.silver"); break;
    case Game::MONEY_Gold: DataBindingAdapter->NotifyDataChange("player.gold"); break;
    }
}

void    GameUIAdapter::HandlePlayerInventoryItemAdd(Game::InventorySlotType p)
{
    // Add bag item slot bindings
    GameClientBagSlotRef* bag = Client->GetState()->GetPlayerInventoryItem(p);
    if (bag)
    {
        for (unsigned i = 0; i < bag->GetNumSlots(); i++)
        {
            String s;
            DW_BagItemSlot::GetSlotBindingID(&s, bag, i);
            DataBindingAdapter->RegisterBoundData(s, *SF_NEW DW_BagItemSlot(Client, bag, i));
        }
    }

    String s;
    DW_BagSlot::GetBindingID(&s, p);
    DataBindingAdapter->NotifyDataChange(s);
}

void    GameUIAdapter::HandlePlayerInventoryItemRemove(Game::InventorySlotType p)
{
    // Remove bag item slot bindings
    // Add bag item slot bindings
    GameClientBagSlotRef* bag = Client->GetState()->GetPlayerInventoryItem(p);
    if (bag)
    {
        for (unsigned i = 0; i < bag->GetNumSlots(); i++)
        {
            String s;
            DW_BagItemSlot::GetSlotBindingID(&s, bag, i);
            DataBindingAdapter->UnregisterBoundData(s);
        }
    }

    // This is immediately followed by an Add event, so no need to notify data change twice
}

void    GameUIAdapter::HandlePlayerBagItemChange(Game::InventorySlotType p, unsigned bagSlot)
{
    String s;
    GameClientBagSlotRef* bag = Client->GetState()->GetPlayerInventoryItem(p);
    SF_ASSERT(bag);
    DW_BagItemSlot::GetSlotBindingID(&s, bag, bagSlot);
    DataBindingAdapter->NotifyDataChange(s);
}

void    GameUIAdapter::HandlePlayerEquipmentChange(Game::EquipmentSlotType p)
{
    String s;
    GameClientSlotRef* slot = Client->GetState()->GetPlayerEquipmentItem(p);
    if (slot)
    {
        DW_EquipmentSlot::GetBindingID(&s, p);
        DataBindingAdapter->NotifyDataChange(s);
    }
}

void    GameUIAdapter::HandlePlayerAbilityGroupAdd(Game::AbilityGroupType g)
{
    SF_UNUSED(g);
    // Do something with this?
}

void    GameUIAdapter::HandlePlayerAbilityAdd(const GameClientAbility& a)
{
    SF_UNUSED(a);
    // Do something with this?
}

void    GameUIAdapter::HandlePlayerAbilityCastInterrupt(Game::AbilityID abilId)
{
    CastBar->NotifyCastInterrupt(abilId);
}

void    GameUIAdapter::HandlePlayerAbilityCooldownEnd(Game::AbilityID abilId)
{
    AbilitySlots.NotifyCooldownEnd(abilId);
}

void    GameUIAdapter::HandleTargetInfoChange(Game::InfoType p)
{
    switch (p)
    {
    case Game::INFO_ID: DataBindingAdapter->NotifyDataChange("target.active"); break;
    case Game::INFO_Name: DataBindingAdapter->NotifyDataChange("target.name"); break;
    case Game::INFO_Race: DataBindingAdapter->NotifyDataChange("target.race"); break;
    case Game::INFO_Class: DataBindingAdapter->NotifyDataChange("target.class"); break;
    }
}

void    GameUIAdapter::HandleTargetStatChange(Game::StatType p)
{
    switch (p)
    {
    case Game::STAT_Hp: DataBindingAdapter->NotifyDataChange("target.hp"); break;
    case Game::STAT_MaxHp: DataBindingAdapter->NotifyDataChange("target.maxhp"); break;
    case Game::STAT_Mp: DataBindingAdapter->NotifyDataChange("target.mp"); break;
    case Game::STAT_MaxMp: DataBindingAdapter->NotifyDataChange("target.maxmp"); break;
    case Game::STAT_Strength: DataBindingAdapter->NotifyDataChange("target.str"); break;
    case Game::STAT_Dexterity: DataBindingAdapter->NotifyDataChange("target.dex"); break;
    case Game::STAT_Intelligence: DataBindingAdapter->NotifyDataChange("target.int"); break;
    case Game::STAT_Endurance: DataBindingAdapter->NotifyDataChange("target.end"); break;
    case Game::STAT_Wisdom: DataBindingAdapter->NotifyDataChange("target.wis"); break;
    case Game::STAT_Luck: DataBindingAdapter->NotifyDataChange("target.luck"); break;
    case Game::STAT_Level: DataBindingAdapter->NotifyDataChange("target.level"); break;
    case Game::STAT_Xp: DataBindingAdapter->NotifyDataChange("target.xp"); break;  
    }
}

void    GameUIAdapter::UpdateLinksToItem(const GameClientEntityRef& ent)
{
    for (unsigned i = 0; i < ActionBarManager.GetNumBars(); i++)
    {
        GameUIActionBar* bar = ActionBarManager.GetBar(i);
        for (unsigned j = 0; j < bar->GetNumSlots(); j++)
        {
            const GameUISlotLink& slot = bar->GetSlot(j);
            if (slot.GetType() == GameUISlotLink::LINK_Entity)
            {
                if (slot.GetEntityData() == ent)
                {
                    // Notify UI
                    String s;
                    DW_ActionSlot::GetBindingID(&s, i, j);
                    DataBindingAdapter->NotifyDataChange(s);
                }
            }
            else if (slot.GetType() == GameUISlotLink::LINK_EntityDefn)
            {
                if (slot.GetDefnID() == ent.DefnID)
                {
                    // Notify UI
                    String s;
                    DW_ActionSlot::GetBindingID(&s, i, j);
                    DataBindingAdapter->NotifyDataChange(s);
                }
            }
        }
    }
}
