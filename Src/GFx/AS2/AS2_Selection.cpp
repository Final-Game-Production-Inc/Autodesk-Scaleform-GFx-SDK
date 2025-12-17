/**************************************************************************

Filename    :   AS2_Selection.cpp
Content     :   Implementation of Selection class
Created     :   February, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Selection.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_String.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/GFx_TextField.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_MovieRoot.h"

namespace Scaleform { namespace GFx { namespace AS2 {

Selection::Selection(Environment* penv)
: Object(penv)
{
    commonInit(penv);
}

void Selection::commonInit (Environment* penv)
{
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_Selection));
}

void Selection::BroadcastOnSetFocus(Environment* penv, 
                                    InteractiveObject* pOldFocus, 
                                    InteractiveObject* pNewFocus,
                                    unsigned controllerIdx)
{
    Value selectionCtorVal;
    if (penv->GetGC()->pGlobal->GetMemberRaw(penv->GetSC(), penv->GetBuiltin(ASBuiltin_Selection), &selectionCtorVal))
    {
        ObjectInterface* pselectionObj = selectionCtorVal.ToObject(penv);
        if (pselectionObj)
        {
            int nargs = 2;
            if (penv->CheckExtensions())
            {
                penv->Push(Number(controllerIdx));
                ++nargs;
            }
            if (pNewFocus)
                penv->Push(Value(pNewFocus));
            else
                penv->Push(Value::NULLTYPE);
            if (pOldFocus)
                penv->Push(Value(pOldFocus));
            else
                penv->Push(Value::NULLTYPE);
            AsBroadcaster::BroadcastMessage(penv, pselectionObj, penv->CreateConstString("onSetFocus"), nargs, penv->GetTopIndex());
            penv->Drop(nargs);
        }
    }
}

void Selection::DoTransferFocus(const FnCall& fn)
{
    MovieImpl* proot = fn.Env->GetMovieImpl();
    
    SF_ASSERT(proot && fn.NArgs == 3);
    SF_ASSERT(fn.Arg(0).IsCharacter() || fn.Arg(0).IsNull());
    GFx::FocusMovedType fmt = GFx::FocusMovedType(int(fn.Arg(1).ToNumber(fn.Env)));
    unsigned controllerIdx = fn.Arg(2).ToUInt32(fn.Env);

    proot->TransferFocus(fn.Arg(0).ToCharacter(fn.Env), controllerIdx, fmt);
}

void Selection::QueueSetFocus
    (Environment* penv, InteractiveObject* pNewFocus, unsigned controllerIdx, GFx::FocusMovedType fmt)
{
    SF_ASSERT(penv);

    ValueArray params;
    if (pNewFocus)
        params.PushBack(Value(pNewFocus));
    else
        params.PushBack(Value::NULLTYPE);
    params.PushBack(Value((int)fmt));
    params.PushBack(Number(controllerIdx));
    penv->GetAS2Root()->InsertEmptyAction()->SetAction(penv->GetAS2Root()->GetLevelMovie(0), 
        Selection::DoTransferFocus, &params);
}

#ifdef GFX_AS2_ENABLE_SELECTION
//////////////////////////////////////////
//
static void GFx_FuncStub(const FnCall& fn)
{
    fn.Result->SetUndefined();
}

static const NameFunction GAS_SelectionFunctionTable[] = 
{
    { "addListener",        &GFx_FuncStub },
    { "broadcastMessage",   &GFx_FuncStub },
    { "getBeginIndex",      &GFx_FuncStub },
    { "getCaretIndex",      &GFx_FuncStub },
    { "getEndIndex",        &GFx_FuncStub },
    { "getFocus",           &GFx_FuncStub },
    { "setFocus",           &GFx_FuncStub },
    { "setSelection",       &GFx_FuncStub },
    { "removeListener",     &GFx_FuncStub },
    { 0, 0 }
};

SelectionProto::SelectionProto(ASStringContext* psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<Selection>(psc, pprototype, constructor)
{
    InitFunctionMembers (psc, GAS_SelectionFunctionTable);
}

//////////////////
const NameFunction SelectionCtorFunction::StaticFunctionTable[] = 
{
    { "getBeginIndex",  &SelectionCtorFunction::GetBeginIndex },
    { "getCaretIndex",  &SelectionCtorFunction::GetCaretIndex },
    { "getEndIndex",    &SelectionCtorFunction::GetEndIndex },
    { "getFocus",       &SelectionCtorFunction::GetFocus },
    { "setFocus",       &SelectionCtorFunction::SetFocus },
    { "setSelection",   &SelectionCtorFunction::SetSelection },
    { 0, 0 }
};

SelectionCtorFunction::SelectionCtorFunction(ASStringContext *psc)
:   CFunctionObject(psc, GlobalCtor)
{
    AsBroadcaster::Initialize(psc, this);

    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void SelectionCtorFunction::GetFocus(const FnCall& fn)
{
    fn.Result->SetNull();
    if (!fn.Env)
        return;

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 1)
            controllerIdx = fn.Arg(0).ToUInt32(fn.Env);
    }

    Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(controllerIdx);
    if (focusedChar)
    {
        fn.Result->SetString(focusedChar->GetCharacterHandle()->GetNamePath());
    }
}

void SelectionCtorFunction::GetFocusArray(const FnCall& fn)
{
    fn.Result->SetNull();
    if (!fn.Env)
        return;

    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs < 1)
            return;

        Ptr<InteractiveObject> f = fn.Arg(0).ToCharacter(fn.Env);

        Ptr<ArrayObject> ao = *static_cast<ArrayObject*>
            (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
        SF_ASSERT(ao);
        ao->Reserve(GFX_MAX_KEYBOARD_SUPPORTED);
        for (unsigned i = 0; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i)
        {
            Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(i);
            if (focusedChar == f)
            {
                ao->PushBack(Value((int)i));
            }
        }
        fn.Result->SetAsObject(ao);
    }
}

void SelectionCtorFunction::GetFocusBitmask(const FnCall& fn)
{
    fn.Result->SetNull();
    if (!fn.Env)
        return;

    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs < 1)
            return;

        Ptr<InteractiveObject> f = fn.Arg(0).ToCharacter(fn.Env);

        unsigned bm = 0;
        for (unsigned i = 0, mask = 0x1; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i, mask <<= 1)
        {
            Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(i);
            if (focusedChar == f)
            {
                bm |= mask;
            }
        }
        fn.Result->SetNumber(bm & 0xFFFF);
    }
}

void SelectionCtorFunction::SetFocus(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.NArgs < 1 || !fn.Env)
        return;

    Ptr<InteractiveObject> newFocus;
    if (fn.Arg(0).IsString())
    {
        // resolve path
        Value val;
        bool retVal = fn.Env->FindVariable(Environment::GetVarParams(fn.Arg(0).ToString(fn.Env), &val));
        if (retVal && val.IsCharacter())
        {
            newFocus = val.ToCharacter(fn.Env);
        }
    }
    else
        newFocus = fn.Arg(0).ToCharacter(fn.Env);
    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 2)
            controllerIdx = fn.Arg(1).ToUInt32(fn.Env);
    }
    if (newFocus)
    {
        if (newFocus->IsFocusEnabled())
        {
            fn.Env->GetMovieImpl()->SetKeyboardFocusTo(newFocus, controllerIdx);
            fn.Result->SetBool(true);
        }
    }
    else
    {
        // just remove the focus
        fn.Env->GetMovieImpl()->SetKeyboardFocusTo(NULL, controllerIdx);
        fn.Result->SetBool(true);

    }
}

void SelectionCtorFunction::GetCaretIndex(const FnCall& fn)
{
    fn.Result->SetNumber(-1);
    if (!fn.Env)
        return;

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 1)
            controllerIdx = fn.Arg(0).ToUInt32(fn.Env);
    }
    Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(controllerIdx);
    if (focusedChar && focusedChar->GetType() == CharacterDef::TextField)
    {
        TextField* ptextField = static_cast<TextField*>(focusedChar.GetPtr());
        fn.Result->SetNumber((Number)ptextField->GetCaretIndex());
    }
}

void SelectionCtorFunction::SetSelection(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (!fn.Env)
        return;

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 3)
            controllerIdx = fn.Arg(2).ToUInt32(fn.Env);
    }
    Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(controllerIdx);
    if (focusedChar && focusedChar->GetType() == CharacterDef::TextField)
    {
        TextField* ptextField = static_cast<TextField*>(focusedChar.GetPtr());

        SPInt bi = 0, ei = SF_MAX_SPINT;

        if (fn.NArgs >= 2)
        {
            bi = fn.Arg(0).ToInt32(fn.Env);
            ei = fn.Arg(1).ToInt32(fn.Env);
        }

        ptextField->SetSelection(bi, ei);
    }
}

void SelectionCtorFunction::GetBeginIndex(const FnCall& fn)
{
    fn.Result->SetNumber(-1);
    if (!fn.Env)
        return;

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 3)
            controllerIdx = fn.Arg(2).ToUInt32(fn.Env);
    }
    Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(controllerIdx);
    if (focusedChar && focusedChar->GetType() == CharacterDef::TextField)
    {
        TextField* ptextField = static_cast<TextField*>(focusedChar.GetPtr());
        fn.Result->SetNumber((Number)ptextField->GetBeginIndex());
    }
}

void SelectionCtorFunction::GetEndIndex(const FnCall& fn)
{
    fn.Result->SetNumber(-1);
    if (!fn.Env)
        return;

    unsigned controllerIdx = 0;
    if (fn.Env->CheckExtensions())
    {
        if (fn.NArgs >= 3)
            controllerIdx = fn.Arg(2).ToUInt32(fn.Env);
    }
    Ptr<InteractiveObject> focusedChar = fn.Env->GetMovieImpl()->GetFocusedCharacter(controllerIdx);
    if (focusedChar && focusedChar->GetType() == CharacterDef::TextField)
    {
        TextField* ptextField = static_cast<TextField*>(focusedChar.GetPtr());
        fn.Result->SetNumber((Number)ptextField->GetEndIndex());
    }
}

void SelectionCtorFunction::CaptureFocus(const FnCall& fn)
{
    fn.Result->SetUndefined();

    bool capture;
    if (fn.NArgs >= 1)
        capture = fn.Arg(0).ToBool(fn.Env);
    else
        capture = true;
    MovieImpl* proot = fn.Env->GetMovieImpl();

    unsigned controllerIdx = 0;
    if (fn.NArgs >= 2)
        controllerIdx = fn.Arg(1).ToUInt32(fn.Env);

    Ptr<InteractiveObject> focusedChar = proot->GetFocusedCharacter(controllerIdx);
    if (!focusedChar)
    {
        // if no characters currently focused - simulate pressing of Tab
        proot->ActivateFocusCapture(controllerIdx);
        focusedChar = proot->GetFocusedCharacter(controllerIdx);
    }
    if (capture)
    {
        if (focusedChar && focusedChar->IsFocusEnabled())
        {
             proot->SetKeyboardFocusTo(focusedChar, controllerIdx);
        }
    }
    else
    {
        proot->HideFocusRect(controllerIdx);
    }
    if (focusedChar)
        fn.Result->SetAsCharacter(focusedChar);
}

// 1st parameter - strings "up", "down", "left", "right", "tab".
// 2nd parameter (optional) - the character to begin search from.
// 3rd parameter (optional) - bool, true to include only focusEnabled = true chars
// 4th parameter (optional) - keyboard index
void SelectionCtorFunction::MoveFocus(const FnCall& fn)
{
    fn.Result->SetUndefined();

    if (fn.NArgs == 0)
        return;

    unsigned controllerIdx = 0;
    if (fn.NArgs >= 4)
        controllerIdx = fn.Arg(3).ToUInt32(fn.Env);

    MovieImpl* proot = fn.Env->GetMovieImpl();
    Ptr<InteractiveObject> startChar;
    if (fn.NArgs >= 2 && !fn.Arg(1).IsUndefined() && !fn.Arg(1).IsNull())        
        startChar = fn.Arg(1).ToCharacter(fn.Env);
    else
        startChar = proot->GetFocusedCharacter(controllerIdx).GetPtr();

    bool includeFocusEnabled = (fn.NArgs >= 3) ? 
        fn.Arg(2).ToBool(fn.Env) : false;

    ASString directionStr = fn.Arg(0).ToString(fn.Env);
    short        keycode;
    KeyModifiers kmods;
    if (directionStr == "up")
        keycode = Key::Up;
    else if (directionStr == "down")
        keycode = Key::Down;
    else if (directionStr == "left")
        keycode = Key::Left;
    else if (directionStr == "right")
        keycode = Key::Right;
    else if (directionStr == "tab")
        keycode = Key::Tab;
    else if (directionStr == "shifttab")
    {
        keycode = Key::Tab;
        kmods.SetShiftPressed(true);
    }
    else
    {
        if (fn.Env->GetLog())
            fn.Env->GetLog()->LogWarning("moveFocus - invalid string id for key: '%s'",
            directionStr.ToCStr());
        return;
    }
    ProcessFocusKeyInfo focusInfo;
    InputEventsQueue::QueueEntry::KeyEntry ke;
    ke.KeyboardIndex            = (UInt8)controllerIdx;
    ke.Code                     = keycode;
    ke.KeysState                = kmods.States;
    proot->InitFocusKeyInfo(&focusInfo, ke, includeFocusEnabled);
    focusInfo.CurFocused        = startChar;
    focusInfo.ManualFocus       = true;
    proot->ProcessFocusKey(Event::KeyDown, ke, &focusInfo);
    proot->FinalizeProcessFocusKey(&focusInfo);
    fn.Result->SetAsCharacter(focusInfo.CurFocused);
}

// 1st parameter - strings "up", "down", "left", "right", "tab".
// 2nd parameter - a parent container (panel), or null
// 3rd           - bool, if true then loop
// 4th parameter (optional) - the character to begin search from.
// 5th parameter (optional) - bool, true to include only focusEnabled = true chars
// 6th parameter (optional) - keyboard index
void SelectionCtorFunction::FindFocus(const FnCall& fn)
{
    fn.Result->SetUndefined();

    if (fn.NArgs == 0)
        return;

    unsigned controllerIdx = 0;
    if (fn.NArgs >= 6)
        controllerIdx = fn.Arg(5).ToUInt32(fn.Env);

    MovieImpl* proot = fn.Env->GetMovieImpl();
    Ptr<InteractiveObject> startChar;
    if (fn.NArgs >= 4 && !fn.Arg(3).IsUndefined() && !fn.Arg(3).IsNull())        
        startChar = fn.Arg(3).ToCharacter(fn.Env);
    else
        startChar = proot->GetFocusedCharacter(controllerIdx).GetPtr();

    bool includeFocusEnabled = (fn.NArgs >= 5) ? 
        fn.Arg(4).ToBool(fn.Env) : false;

    ASString directionStr = fn.Arg(0).ToString(fn.Env);
    short keycode;
    KeyModifiers specKeysState;
    if (directionStr == "up")
        keycode = Key::Up;
    else if (directionStr == "down")
        keycode = Key::Down;
    else if (directionStr == "left")
        keycode = Key::Left;
    else if (directionStr == "right")
        keycode = Key::Right;
    else if (directionStr == "tab")
        keycode = Key::Tab;
    else if (directionStr == "shifttab")
    {
        keycode = Key::Tab;
        specKeysState.SetShiftPressed(true);
    }
    else
    {
        /* // PPS: Warning causes unnecessary spew in log with CLIK. CLIK's FocusHandler
           // passes all unhandled key events to findFocus.
        if (fn.Env->GetLog())
            fn.Env->GetLog()->LogWarning("findFocus - invalid string id for key: '%s'",
            directionStr.ToCStr());
        */
        return;
    }

    Ptr<InteractiveObject> panelChar;
    if (fn.NArgs >= 2)
    {
        Value context = fn.Arg(1);
        if (!context.IsNull() && !context.IsUndefined())
            panelChar = context.ToCharacter(fn.Env);
        else
        {
            // take a modal clip as panelChar, if the parameter is "null"
            panelChar = proot->GetModalClip(controllerIdx);
        }
    }

    bool looping = (fn.NArgs >= 3) ? fn.Arg(2).ToBool(fn.Env) : false;

    ProcessFocusKeyInfo focusInfo;
    InputEventsQueue::QueueEntry::KeyEntry ke;
    ke.KeyboardIndex            = (UInt8)controllerIdx;
    ke.Code                     = keycode;
    ke.KeysState                = specKeysState.States;
    FocusGroupDescr focusGroup(fn.Env->GetHeap()); 
    focusGroup.ModalClip        = (panelChar) ? panelChar->GetCharacterHandle() : NULL;
    focusGroup.LastFocused      = startChar;
    proot->InitFocusKeyInfo(&focusInfo, ke, includeFocusEnabled, &focusGroup);
    focusInfo.ManualFocus       = true;
    proot->ProcessFocusKey(Event::KeyDown, ke, &focusInfo);
    if (focusInfo.CurFocused && focusInfo.CurFocused != startChar)
        fn.Result->SetAsCharacter(focusInfo.CurFocused);
    else
    {
        if (!looping || focusGroup.TabableArray.GetSize() == 0)
            fn.Result->SetNull();
        else
        {
            if (keycode == Key::Tab)
            {
                if (!specKeysState.IsShiftPressed())
                    fn.Result->SetAsCharacter(focusGroup.TabableArray[0]);
                else
                    fn.Result->SetAsCharacter(focusGroup.TabableArray[focusGroup.TabableArray.GetSize()-1]);
            }
            else
            {
                float lastV = SF_MIN_FLOAT;
                UPInt index = 0;
                for (UPInt i = 0, n = focusGroup.TabableArray.GetSize(); i < n; ++i)
                {
                    Ptr<InteractiveObject> b = focusGroup.TabableArray[i];
                    if ((!focusInfo.InclFocusEnabled && !b->IsTabable()) ||
                        !b->IsFocusAllowed(proot, focusInfo.KeyboardIndex))
                    {
                        // If this is not for manual focus and not tabable - ignore.
                        continue;
                    }
                    DisplayObject::Matrix mb = b->GetLevelMatrix();
                    RectF bRect  = mb.EncloseTransform(b->GetFocusRect());
                    switch (keycode)
                    {
                    case Key::Down: 
                        // find top
                        if (lastV == SF_MIN_FLOAT || bRect.y1 < lastV)
                        {
                            lastV = bRect.y1;
                            index = i;
                        }
                        break; //
                    case Key::Up: 
                        // find bottom
                        if (lastV == SF_MIN_FLOAT || bRect.y2 > lastV)
                        {
                            lastV = bRect.y2;
                            index = i;
                        }
                        break; //
                    case Key::Left: 
                        // find most right
                        if (lastV == SF_MIN_FLOAT || bRect.x2 > lastV)
                        {
                            lastV = bRect.x2;
                            index = i;
                        }
                        break; //
                    case Key::Right: 
                        // find most left
                        if (lastV == SF_MIN_FLOAT || bRect.x1 < lastV)
                        {
                            lastV = bRect.x1;
                            index = i;
                        }
                        break; //
                    default: SF_ASSERT(0);
                    }
                }
                fn.Result->SetAsCharacter(focusGroup.TabableArray[index]);
            }
        }
    }
}

// 1st param - modal clip
// 2nd param - controller id
void SelectionCtorFunction::SetModalClip(const FnCall& fn)
{
    fn.Result->SetUndefined();

    if (fn.NArgs < 1)
        return;

    MovieImpl* proot = fn.Env->GetMovieImpl();
    Ptr<InteractiveObject> pchar = fn.Arg(0).ToCharacter(fn.Env);
    unsigned controllerIdx = (fn.NArgs >= 2) ? fn.Arg(1).ToUInt32(fn.Env) : 0;
    if (pchar && pchar->IsSprite())
    {
        Sprite* pspr = pchar->CharToSprite_Unsafe();
        proot->SetModalClip(pspr, controllerIdx);
    }
    else
        proot->SetModalClip(NULL, controllerIdx);
}

// 1st param - controller id
// ret - modal clip
void SelectionCtorFunction::GetModalClip(const FnCall& fn)
{
    fn.Result->SetUndefined();

    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned controllerIdx = (fn.NArgs >= 1) ? fn.Arg(0).ToUInt32(fn.Env) : 0;
    Sprite* pmodalClip = proot->GetModalClip(controllerIdx);
    fn.Result->SetAsCharacter(pmodalClip);
}

// 1st param - an id of logical controller (focus group index)
// returns a bit-mask where each bit represents a physical controller, 
// associated with the specified focus group.
void SelectionCtorFunction::GetControllerMaskByFocusGroup(const FnCall& fn)
{
    fn.Result->SetUndefined();

    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned focusGroupIdx = (fn.NArgs >= 1) ? fn.Arg(0).ToUInt32(fn.Env) : 0;
    fn.Result->SetNumber((int)proot->GetControllerMaskByFocusGroup(focusGroupIdx));
}

// 1st param - controller idx
// 2nd param - focus group idx
// ret - bool, true, if succ
void SelectionCtorFunction::SetControllerFocusGroup(const FnCall& fn)
{
    fn.Result->SetUndefined();

    if (fn.NArgs < 2)
        return;

    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned controllerIdx = fn.Arg(0).ToUInt32(fn.Env);
    unsigned groupIdx      = fn.Arg(1).ToUInt32(fn.Env);
    fn.Result->SetBool(proot->SetControllerFocusGroup(controllerIdx, groupIdx));
}

// 1st param - controller id
// ret - focus group idx
void SelectionCtorFunction::GetControllerFocusGroup(const FnCall& fn)
{
    fn.Result->SetUndefined();

    MovieImpl* proot = fn.Env->GetMovieImpl();
    unsigned controllerIdx = (fn.NArgs >= 1) ? fn.Arg(0).ToUInt32(fn.Env) : 0;
    fn.Result->SetNumber(proot->GetControllerFocusGroup(controllerIdx));
}


bool SelectionCtorFunction::GetMember(Environment* penv, const ASString& name, Value* pval)
{
    if (penv->CheckExtensions())
    {
        MovieImpl* proot = penv->GetMovieImpl();
        // Extensions are always case-insensitive
        if (name == "captureFocus") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::CaptureFocus);
            return true;
        }
        else if (name == "disableFocusAutoRelease")
        {
            if (proot->IsDisableFocusAutoReleaseSet())
                pval->SetBool(proot->IsDisableFocusAutoRelease());
            else
                pval->SetUndefined();
            return true;
        }
        else if (name == "alwaysEnableArrowKeys")
        {
            if (proot->IsAlwaysEnableFocusArrowKeysSet())
                pval->SetBool(proot->IsAlwaysEnableFocusArrowKeys());
            else
                pval->SetUndefined();
            return true;
        }
        else if (name == "alwaysEnableKeyboardPress")
        {
            if (proot->IsAlwaysEnableKeyboardPressSet())
                pval->SetBool(proot->IsAlwaysEnableKeyboardPress());
            else
                pval->SetUndefined();
            return true;
        }
        else if (name == "disableFocusRolloverEvent")
        {
            if (proot->IsDisableFocusRolloverEventSet())
                pval->SetBool(proot->IsDisableFocusRolloverEvent());
            else
                pval->SetUndefined();
            return true;
        }
        else if (name == "disableFocusKeys")
        {
            if (proot->IsDisableFocusKeysSet())
                pval->SetBool(proot->IsDisableFocusKeys());
            else
                pval->SetUndefined();
            return true;
        }
        else if (name == "modalClip")
        {
            Sprite* pmodalClip = proot->GetModalClip(0);
            pval->SetAsCharacter(pmodalClip);
            return true;
        }
        else if (name == "moveFocus") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::MoveFocus);
            return true;
        }
        else if (name == "findFocus") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::FindFocus);
            return true;
        }
        else if (name == "setModalClip") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::SetModalClip);
            return true;
        }
        else if (name == "getModalClip") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::GetModalClip);
            return true;
        }
        else if (name == "setControllerFocusGroup") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::SetControllerFocusGroup);
            return true;
        }
        else if (name == "getControllerFocusGroup") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::GetControllerFocusGroup);
            return true;
        }
        else if (name == "getFocusBitmask") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::GetFocusBitmask);
            return true;
        }
        else if (name == "numFocusGroups") 
        {
            *pval = Value(int(proot->GetFocusGroupCount()));
            return true;
        }
        else if (name == "getControllerMaskByFocusGroup")
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::GetControllerMaskByFocusGroup);
            return true;
        }
        else if (name == "getFocusArray") 
        {
            *pval = Value(penv->GetSC(), SelectionCtorFunction::GetFocusArray);
            return true;
        }
    }
    return FunctionObject::GetMember(penv, name, pval);
}

bool SelectionCtorFunction::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    if (penv->CheckExtensions())
    {
        MovieImpl* proot = penv->GetMovieImpl();
        if (name == "disableFocusAutoRelease")
        {
            proot->SetDisableFocusAutoRelease(val.ToBool(penv));
            return true;
        }
        else if (name == "alwaysEnableArrowKeys")
        {
            proot->SetAlwaysEnableFocusArrowKeys(val.ToBool(penv));
            return true;
        }
        else if (name == "alwaysEnableKeyboardPress")
        {
            proot->SetAlwaysEnableKeyboardPress(val.ToBool(penv));
            return true;
        }
        else if (name == "disableFocusRolloverEvent")
        {
            proot->SetDisableFocusRolloverEvent(val.ToBool(penv));
            return true;
        }
        else if (name == "disableFocusKeys")
        {
            proot->SetDisableFocusKeys(val.ToBool(penv));
            return true;
        }
        else if (name == "modalClip")
        {
            InteractiveObject* pchar = val.ToCharacter(penv);
            if (pchar && pchar->GetType() == CharacterDef::Sprite)
            {
                Sprite* pspr = static_cast<Sprite*>(pchar);
                proot->SetModalClip(pspr, 0);
            }
            else
                proot->SetModalClip(NULL, 0);
            return true;
        }
    }
    return FunctionObject::SetMember(penv, name, val, flags);
}

void SelectionCtorFunction::GlobalCtor(const FnCall& fn)
{
    fn.Result->SetNull();
}

FunctionRef SelectionCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) SelectionCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) SelectionProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Selection, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Selection), Value(ctor));
    return ctor;
}
#endif //#ifdef GFX_AS2_ENABLE_SELECTION
}}} // SF::GFx::AS2
