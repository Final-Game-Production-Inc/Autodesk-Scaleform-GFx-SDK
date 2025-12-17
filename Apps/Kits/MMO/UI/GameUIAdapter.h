/**************************************************************************

Filename    :   GameUIAdapter.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameUIAdapter_H
#define INC_GameUIAdapter_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_MsgFormat.h"

#include "GFx/GFx_Player.h"

#include "../Game/GameCommon.h"
#include "../Game/GameEvent.h"
#include "../Game/GameClientState.h"

#include "GameUIRuntimeAtlas.h"
#include "GameUIActionBar.h"

#include "../Apps/Samples/DataBinding/CLIKDataBinding.h"
#include "Platform/Platform_RenderThread.h"

using namespace Scaleform;

class GameClient;
class GameClientState;

class GameUIEquipmentSlots
{
public:
    void    AddSlot(Game::EquipmentSlotType slot, GFx::Value mc);
    void    RemoveSlot(Game::EquipmentSlotType slot, GFx::Value mc);
    
    void    HighlightAllForEntityClass(const Game::EntityDefnID& defId);
    void    UnhighlightAll();

protected:
    ArrayCPP<GFx::Value>    Slots[Game::NumEquipmentSlotTypes];
    ArrayCPP<GFx::Value>    HighlightedSlots;
};

//////////////////////////////////////////////////////////////////////////

class GameUIAbilitySlotMC : public RefCountBase<GameUIAbilitySlotMC, Stat_Default_Mem>
{
public:
    GameUIAbilitySlotMC(const GFx::Value& mc) : MC(mc) {}
    GFx::Value& GetMC()     { return MC; }
protected:
    GFx::Value    MC;
};

class GameUIAbilitySlots
{
public:
    void    AddSlot(Game::AbilityID abilId, GFx::Value mc);
    void    RemoveSlot(Game::AbilityID abilId, GFx::Value mc);

    void    UpdateSlots(UInt32 ticks, GameClientState* s);
    void    NotifyCooldownEnd(Game::AbilityID abilId);

protected:
    typedef     Array< Ptr<GameUIAbilitySlotMC> >   MovieClipArray;
    Hash< unsigned, MovieClipArray >      Slots;
};


//////////////////////////////////////////////////////////////////////////

class GameUICastBar : public RefCountBase<GameUICastBar, Stat_Default_Mem>
{
public:
    GameUICastBar(const GFx::Value& cb);

    void    UpdateCastBar(UInt32 ticks, GameClientState* s);
    void    NotifyCastInterrupt(unsigned abilId);

protected:
    GFx::Value  Ref;
    GFx::Value  AbilityNameRef;
    GFx::Value  CastTimeRef;

    GFx::Value  BarRef0, BarRef1;
    GFx::Value  ShineRef0, ShineRef1;
    GFx::Value  MaskRef0, MaskRef1;
    GFx::Value  FillRef0, FillRef1;

    GFx::Double FillStartX;
    GFx::Double FillEndX;
    float       LastPct;

    enum ModeType
    {
        MODE_Inactive,
        MODE_Active,
        MODE_FadeOutHold,
        MODE_FadeOut,
    } Mode;
    UInt32      StartFadeTicks;

    void    setPercent(float pct, UInt32 leftTicks);
    void    setPercentBar(float pct, GFx::Value& shineref, GFx::Value& maskref);
};


//////////////////////////////////////////////////////////////////////////

class GameUIAdapter : public RefCountBase<GameUIAdapter, Stat_Default_Mem>,
                      public GameClientStateMessageLog::LogListener
{
public:
    GameUIAdapter(GameClient* client, GFx::Movie* pmovie);
    
    void    Initialize(GameUIRuntimeAtlasManager* pimagecreator);

    CLIKDataBindingAdapter*     GetDataBindings() const     { return DataBindingAdapter; }
    GameUIActionBarManager&     GetActionBarManager()       { return ActionBarManager; }

    void    Update(UInt32 ticks);
    
    // Data change notification interface
    void    HandlePlayerInfoChange(Game::InfoType p);
    void    HandlePlayerStatChange(Game::StatType p);
    void    HandlePlayerMoneyChange(Game::MoneyType p);
    void    HandlePlayerInventoryItemAdd(Game::InventorySlotType p);
    void    HandlePlayerInventoryItemRemove(Game::InventorySlotType p);
    void    HandlePlayerBagItemChange(Game::InventorySlotType p, unsigned bagSlot);
    void    HandlePlayerEquipmentChange(Game::EquipmentSlotType p);
    void    HandlePlayerAbilityGroupAdd(Game::AbilityGroupType g);
    void    HandlePlayerAbilityAdd(const GameClientAbility& a);
    void    HandlePlayerAbilityCastInterrupt(Game::AbilityID abilId);
    void    HandlePlayerAbilityCooldownEnd(Game::AbilityID abilId);

    void    HandleTargetInfoChange(Game::InfoType p);
    void    HandleTargetStatChange(Game::StatType p);

    // GameDataModel AS3 class handlers
    typedef     const GFx::FunctionHandler::Params          ParamsType;
    //
    static void     onNotifySlotDragStart(GameUIAdapter* pthis, ParamsType& params);
    static void     onNotifySlotDragEnd(GameUIAdapter* pthis, ParamsType& params);
    static void     onNotifySlotUse(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnGetBagSlotBindings(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnGetSpellbookGroups(GameUIAdapter* pthis, ParamsType& params);
    static void     OnGetSpellsByGroup(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnGetSlotTooltip(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnRegisterCastBar(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnGetMessageLogs(GameUIAdapter* pthis, ParamsType& params);
    static void     OnRegisterMessageLogListener(GameUIAdapter* pthis, ParamsType& params);
    static void     OnUnregisterMessageLogListener(GameUIAdapter* pthis, ParamsType& params);
    static void     OnGetMessageLogData(GameUIAdapter* pthis, ParamsType& params);
    //
    static void     OnSubmitLogMessage(GameUIAdapter* pthis, ParamsType& params);
    static void     OnGetSubmitChannelInfo(GameUIAdapter* pthis, ParamsType& params);

    void    RegisterEquipmentSlotMC(Game::EquipmentSlotType slot, GFx::Value obj);
    void    UnregisterEquipmentSlotMC(Game::EquipmentSlotType slot, GFx::Value obj);
    void    RegisterAbilitySlotMC(Game::AbilityID abilId, GFx::Value obj);
    void    UnregisterAbilitySlotMC(Game::AbilityID abilId, GFx::Value obj);

    void    UpdateLinksToItem(const GameClientEntityRef& ent);

    virtual void    OnMessageAdded(GameClientStateMessageLog* plog, const String& formattedMsg);

protected:
    GameClient*         Client;
    GFx::Movie*         TheUI;

    Ptr<GameUIRuntimeAtlasManager>  IconAtlasManager;
    Ptr<CLIKDataBindingAdapter>     DataBindingAdapter;

    GameUIEquipmentSlots    EquipmentSlots;
    GameUIAbilitySlots      AbilitySlots;
    Ptr<GameUICastBar>      CastBar;

    GameUIActionBarManager ActionBarManager;

    ArrayCPP< GFx::Value >   LogHandlers;

    void            registerBoundDataWrappers();
    void            addAbilityToActionBar(unsigned barIdx, unsigned slotIdx, Game::AbilityID abilId);
    void            addItemToActionBar(unsigned barIdx, unsigned slotIdx, const GameClientEntityRef& ent);
    void            addItemDefnToActionBar(unsigned barIdx, unsigned slotIdx, const Game::EntityDefnID& defn);
};

#endif  // INC_GameUIAdapter_H
