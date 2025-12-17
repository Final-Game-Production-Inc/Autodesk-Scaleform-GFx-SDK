/**************************************************************************

Filename    :   GameUIAdapter.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

#include "GameUIAdapter.h"
#include "../Game.h"


//
// GameDataModel AS3 class method map
//

typedef void (*GameUIAdapterCallback)(GameUIAdapter* pthis,  GameUIAdapter::ParamsType& params);

struct GameDataModelNamedFunction
{
    const char*             Name;
    GameUIAdapterCallback   Function;
};

const GameDataModelNamedFunction GameDataModelStaticFunctionTable[] = 
{
    // Item slots
    { "notifySlotDragStart",        &GameUIAdapter::onNotifySlotDragStart },
    { "notifySlotDragEnd",          &GameUIAdapter::onNotifySlotDragEnd },
    { "notifySlotUse",              &GameUIAdapter::onNotifySlotUse },

    // Inventory
    { "getBagSlotBindings",         &GameUIAdapter::OnGetBagSlotBindings },

    // Spellbook
    { "getSpellbookGroups",         &GameUIAdapter::OnGetSpellbookGroups },
    { "getSpellsByGroup",           &GameUIAdapter::OnGetSpellsByGroup },

    // Tooltip
    { "getSlotTooltip",             &GameUIAdapter::OnGetSlotTooltip },
    
    // Castbar
    { "registerCastBar",            &GameUIAdapter::OnRegisterCastBar },

    // Message logs
    { "getMessageLogs",                 &GameUIAdapter::OnGetMessageLogs },
    { "registerMessageLogListener",     &GameUIAdapter::OnRegisterMessageLogListener },
    { "unregisterMessageLogListener",   &GameUIAdapter::OnUnregisterMessageLogListener },
    { "getMessageLogData",              &GameUIAdapter::OnGetMessageLogData },

    // Message input
    { "submitLogMessage",               &GameUIAdapter::OnSubmitLogMessage },
    { "getSubmitChannelInfo",               &GameUIAdapter::OnGetSubmitChannelInfo },

    { 0, 0 }
};

//
// Registrar and delegator for GameDataModel AS3 class callbacks
//

class GameUIAdapterCallbackHandler : public GFx::FunctionHandler
{
public:
    GameUIAdapterCallbackHandler(GameUIAdapter* pthis) : pThis(pthis) {}

    virtual void    Call(const GFx::FunctionHandler::Params& params)
    {
        ((GameUIAdapterCallback)params.pUserData)(pThis, params);
    }

protected:
    GameUIAdapter*  pThis;
};

class GameUIAdapterDelegate : public GFx::ExternalInterface
{
public:
    GameUIAdapterDelegate(GameUIAdapter* pthis, GFx::ExternalInterface* pxiChain) 
      : pThis(pthis), pExtIfcChain(pxiChain) {}

    virtual void Callback(GFx::Movie* pmovie, const char* methodName, const GFx::Value* args, unsigned argCount)
    {
        // Register the adapter class methods in the VM
        if (!SFstrcmp("gameDataModelInit", methodName))
        {
            SF_DEBUG_MESSAGE(1, "Registering GameDataModel handlers\n");

            // First param is expected to be the class object
            SF_UNUSED(argCount);
            SF_ASSERT(argCount == 1 && args[0].IsObject());
            GFx::Value classObj = args[0];

            Ptr<GameUIAdapterCallbackHandler>   handler = *SF_NEW GameUIAdapterCallbackHandler(pThis);

            for(unsigned i = 0; GameDataModelStaticFunctionTable[i].Name; i++)
            {
                GFx::Value func;
                pmovie->CreateFunction(&func, handler, (void *) GameDataModelStaticFunctionTable[i].Function);
                classObj.SetMember(GameDataModelStaticFunctionTable[i].Name, func);

                GFx::Value test;
                classObj.GetMember(GameDataModelStaticFunctionTable[i].Name, &test);
                SF_ASSERT(test.IsObject());
            }
        }
		    else if (!SFstrcmp("isShippingBuild", methodName))
		    {
			    SF_ASSERT(argCount == 1 && args[0].IsObject());
			    GFx::Value returnValue = args[0];
			    GFx::Value isShippingBuild;
    #if defined (NDEBUG)
			    isShippingBuild.SetBoolean(true);
    #else
			    isShippingBuild.SetBoolean(false);
    #endif // NDEBUG
			    returnValue.SetMember("isShipping", isShippingBuild);
		    }
        else if(pExtIfcChain.GetPtr()!=NULL)
        {
          pExtIfcChain->Callback(pmovie, methodName, args, argCount);
        }
        else
        {
            SF_DEBUG_WARNING1(1, "Unknown method '%s' called on GameUIAdapterDelegate!\n", methodName);
        }
    }

protected:

    Ptr<GFx::ExternalInterface>     pExtIfcChain;
    GameUIAdapter*      pThis;
};


//////////////////////////////////////////////////////////////////////////

void    GameUIEquipmentSlots::AddSlot(Game::EquipmentSlotType slot, GFx::Value mc)
{
    Slots[slot].PushBack(mc);
}

void    GameUIEquipmentSlots::RemoveSlot(Game::EquipmentSlotType slot, GFx::Value mc)
{
    int idx = -1;
    for (unsigned i = 0; i < Slots[slot].GetSize(); i++)
    {
        if (mc == Slots[slot][i])
        {
            idx = i;
            break;
        }
    }
    if (idx >= 0)
        Slots[slot].RemoveAt(idx);
}

void    GameUIEquipmentSlots::HighlightAllForEntityClass(const Game::EntityDefnID& defnId)
{
    for (unsigned i = 0; i < Game::NumEquipmentSlotTypes; i++)
    {
        if (Game::SlotSupportsEquipmentClass((Game::EquipmentSlotType)i, defnId))
        {
            for (unsigned j = 0; j < Slots[i].GetSize(); j++)
            {
                GFx::Value& mc = Slots[i][j];
                HighlightedSlots.PushBack(mc);
                mc.Invoke("highlight");
            }
        }
    }
}

void    GameUIEquipmentSlots::UnhighlightAll()
{
    for (unsigned i = 0; i < HighlightedSlots.GetSize(); i++)
    {
        GFx::Value& mc = HighlightedSlots[i];
        mc.Invoke("unhighlight");
    }
    HighlightedSlots.Clear();
}

//////////////////////////////////////////////////////////////////////////

void    GameUIAbilitySlots::AddSlot(Game::AbilityID abilId, GFx::Value mc)
{
    MovieClipArray* arr = Slots.Get(abilId);
    if (!arr)
    {
        Slots.Add(abilId, MovieClipArray());
        arr = Slots.Get(abilId);
    }
    Ptr<GameUIAbilitySlotMC> slotMc = *SF_NEW GameUIAbilitySlotMC(mc);
    arr->PushBack(slotMc);
}

void    GameUIAbilitySlots::RemoveSlot(Game::AbilityID abilId, GFx::Value mc)
{
    MovieClipArray* arr = Slots.Get(abilId);
    if (!arr) return;

    int idx = -1;
    for (unsigned i = 0; i < arr->GetSize(); i++)
    {
        if (arr->At(i)->GetMC() == mc)
        {
            idx = i;
            break;
        }
    }
    if (idx >= 0)
        arr->RemoveAt(idx);
}

void    GameUIAbilitySlots::UpdateSlots(UInt32 ticks, GameClientState* s)
{
    SF_UNUSED(ticks);

    const Array<GameClientAbilityCooldown>& cds = s->GetPlayerAbilityCooldowns();
    for (unsigned i = 0; i < cds.GetSize(); i++)
    {
        unsigned id = cds[i].GetAbilityID();

        MovieClipArray* arr = Slots.Get(id);
        if (arr)
        {
            for (unsigned j = 0; j < arr->GetSize(); j++)
            {
                GameUIAbilitySlotMC* mc = arr->At(j);
                GFx::Value pct;
                pct.SetNumber(cds[i].GetCompletionPercent());
                mc->GetMC().Invoke("updateCooldown", NULL, &pct, 1);
            }
        }
    }
}

void    GameUIAbilitySlots::NotifyCooldownEnd(Game::AbilityID abilId)
{
    MovieClipArray* arr = Slots.Get(abilId);
    if (arr)
    {
        for (unsigned j = 0; j < arr->GetSize(); j++)
        {
            GameUIAbilitySlotMC* mc = arr->At(j);
            mc->GetMC().Invoke("endCooldown");
        }
    }
}

//////////////////////////////////////////////////////////////////////////

GameUICastBar::GameUICastBar(const GFx::Value& ref) : Mode(MODE_Inactive)
{
    Ref = ref;
    // Store references to sub elements
    Ref.GetMember("abilityName", &AbilityNameRef);
    Ref.GetMember("castTime", &CastTimeRef);
    Ref.GetMember("castbar0", &BarRef0);
    Ref.GetMember("castbar1", &BarRef1);

    BarRef0.GetMember("shine", &ShineRef0);
    BarRef0.GetMember("mask", &MaskRef0);
    BarRef0.GetMember("fill", &FillRef0);
    BarRef1.GetMember("shine", &ShineRef1);
    BarRef1.GetMember("mask", &MaskRef1);
    BarRef1.GetMember("fill", &FillRef1);

    // Hide the cast bar
    GFx::Value::DisplayInfo info;
    info.SetVisible(false);
    BarRef1.SetDisplayInfo(info);
    Ref.SetDisplayInfo(info);

    // Get fill extents
    GFx::Value fillX, fillWidth;
    FillRef0.GetMember("x", &fillX);
    FillRef0.GetMember("width", &fillWidth);
    FillStartX = fillX.GetNumber();
    FillEndX = fillWidth.GetNumber() + FillStartX;
}

void    GameUICastBar::UpdateCastBar(UInt32 ticks, GameClientState* s)
{
    if (Ref.IsUndefined()) return;

    const GameClientAbilityCast& cast = s->GetPlayerAbilityCast();
    if (cast.IsCasting() && Mode != MODE_Active)
    {        
        LastPct = 0;
        Game::EntityDefnID id;
        id.Class = Game::ENTITY_Ability;
        id.ID = cast.GetAbilityID();
        AbilityNameRef.SetText(Game::GetStringsForID(id)->Name);
        setPercent(0, cast.GetTicksLeft());

        GFx::Value::DisplayInfo info;        
        info.SetVisible(true);
        info.SetAlpha(100);
        Ref.SetDisplayInfo(info);

        ShineRef0.GotoAndPlay("casting");
        FillRef0.GotoAndPlay("casting");

        Mode = MODE_Active;
        return;
    }

    // Process animations if necessary
    switch (Mode)
    {
    case MODE_FadeOutHold: 
        if (ticks - StartFadeTicks > 500)
        {
            StartFadeTicks = ticks;
            Mode = MODE_FadeOut;
        }
        break;
    case MODE_FadeOut:
        GFx::Value::DisplayInfo info;
        float fadePct = (ticks - StartFadeTicks) / 250.f;
        if (fadePct > 1.f)
        {
            Mode = MODE_Inactive;
            info.SetVisible(false);
        }
        else 
        {
            info.SetAlpha((1 - fadePct) * 100);
        }
        Ref.SetDisplayInfo(info);
        break;
    }
    // Update cast bar if necessary
    if (cast.IsCasting())
    {
        float pct = (cast.GetTotalTicks() - cast.GetTicksLeft()) / (float)cast.GetTotalTicks();
        setPercent(pct, cast.GetTicksLeft());
        LastPct = pct;
    }
    else
    {
        if (Mode == MODE_Active)
        {
            StartFadeTicks = Timer::GetTicksMs();
            ShineRef0.GotoAndPlay("completed");
            FillRef0.GotoAndPlay("completed");
            Mode = MODE_FadeOutHold;
        }
    }
}

void    GameUICastBar::NotifyCastInterrupt(unsigned abilId)
{
    SF_UNUSED(abilId);

    // Show second bar fading
    GFx::Value::DisplayInfo info;
    info.SetVisible(true);
    BarRef1.SetDisplayInfo(info);
    setPercentBar(LastPct, ShineRef1, MaskRef1);
    ShineRef1.GotoAndPlay("interrupted");
    FillRef1.GotoAndPlay("interrupted"); 
    
    info.SetXScale((0 + 0.01) * 100);
    MaskRef0.SetDisplayInfo(info);

    Mode = MODE_FadeOutHold;
    StartFadeTicks = Timer::GetTicksMs();
}

void    GameUICastBar::setPercent(float pct, UInt32 leftTicks)
{
    setPercentBar(pct, ShineRef0, MaskRef0);
    // Cast time
    float ct = leftTicks / 1000.f;
    char buf[16];
    Format(StringDataPtr(buf, 16), "{0:.1}", ct);
    CastTimeRef.SetText(buf);
}
void    GameUICastBar::setPercentBar(float pct, GFx::Value& shineref, GFx::Value& maskref)
{
    GFx::Value::DisplayInfo info;
    // Mask
    info.SetXScale((pct + 0.01) * 100);
    maskref.SetDisplayInfo(info);
    // Shine
    Double sx = (FillEndX - FillStartX) * pct + FillStartX;
    info.Clear();
    info.SetX(sx);
    shineref.SetDisplayInfo(info);
}


//////////////////////////////////////////////////////////////////////////

GameUIAdapter::GameUIAdapter(GameClient* client, GFx::Movie* pmovie) : Client(client), TheUI(pmovie)
{
    // We chain our ExternalInterface handler with the data binding adapter
    Ptr<GameUIAdapterDelegate>  gameUICallbackHandler = *SF_NEW GameUIAdapterDelegate(this, pmovie->GetExternalInterface());
    DataBindingAdapter = *SF_NEW CLIKDataBindingAdapter(TheUI, gameUICallbackHandler);
}

void    GameUIAdapter::Update(UInt32 ticks)
{
    AbilitySlots.UpdateSlots(ticks, Client->GetState());
    if (CastBar) CastBar->UpdateCastBar(ticks, Client->GetState());
}

void    GameUIAdapter::RegisterEquipmentSlotMC(Game::EquipmentSlotType slot, GFx::Value obj)
{
    EquipmentSlots.AddSlot(slot, obj);
}

void    GameUIAdapter::UnregisterEquipmentSlotMC(Game::EquipmentSlotType slot, GFx::Value obj)
{
    EquipmentSlots.RemoveSlot(slot, obj);
}

void    GameUIAdapter::RegisterAbilitySlotMC(Game::AbilityID abilId, GFx::Value obj)
{
    AbilitySlots.AddSlot(abilId, obj);
}

void    GameUIAdapter::UnregisterAbilitySlotMC(Game::AbilityID abilId, GFx::Value obj)
{
    AbilitySlots.RemoveSlot(abilId, obj);
}


//////////////////////////////////////////////////////////////////////////

void    GameUIAdapter::OnMessageAdded(GameClientStateMessageLog* plog, const String& formattedMsg)
{
    GFx::Value args[2];
    args[0].SetString(plog->GetName());
    args[1].SetString(formattedMsg.ToCStr());
    for (unsigned i = 0; i < LogHandlers.GetSize(); i++)
    {
        LogHandlers[i].InvokeSelf(NULL, args, 2);
    }
}
