/**************************************************************************

Filename    :   GFx_Button.cpp
Content     :   Implementation of Button character
Created     :   Dec, 2009
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
// 
////#include "Render/Render_Renderer.h"

#include "GFx/GFx_Button.h"
#include "GFx/Audio/GFx_Sound.h" 
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_SoundTagsReader.h"

#include "GFx/GFx_LoadProcess.h"
//#include "GFx/GFx_DisplayContext.h"

#include "Render/Render_TreeLog.h"

/*

Observations about button & mouse behavior

Entities that receive mouse events: only buttons and sprites, AFAIK

When the mouse button goes down, it becomes "captured" by whatever
element is topmost, directly below the mouse at that moment.  While
the mouse is captured, no other entity receives mouse events,
regardless of how the mouse or other elements move.

The mouse remains captured until the mouse button goes up.  The mouse
remains captured even if the element that captured it is removed from
the display list.

If the mouse isn't above a button or sprite when the mouse button goes
down, then the mouse is captured by the Background (i.E. mouse events
just don't get sent, until the mouse button goes up again).

MA: The only exception to this is Menu mode button, which will get
dragOver/dragOut and release even if the inital click was outside
of the button (unless it was in a NON-menu button initally, in
which case that button would capture the mouse).

Mouse events:

+------------------+---------------+-------------------------------------+
| Event            | Mouse Button  | description                         |
=========================================================================
| onRollOver       |     up        | sent to topmost entity when mouse   |
|                  |               | cursor initially goes over it       |
+------------------+---------------+-------------------------------------+
| onRollOut        |     up        | when mouse leaves entity, after     |
|                  |               | onRollOver                          |
+------------------+---------------+-------------------------------------+
| onPress          |  up -> down   | sent to topmost entity when mouse   |
|                  |               | button goes down.  onRollOver       |
|                  |               | always precedes onPress.  Initiates |
|                  |               | mouse capture.                      |
+------------------+---------------+-------------------------------------+
| onRelease        |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while over the element           |
+------------------+---------------+-------------------------------------+
| onDragOut        |     down      | sent to active entity if mouse      |
|                  |               | is no longer over the entity        |
+------------------+---------------+-------------------------------------+
| onReleaseOutside |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while not over the entity.       |
|                  |               | onDragOut always precedes           |
|                  |               | onReleaseOutside                    |
+------------------+---------------+-------------------------------------+
| onDragOver       |     down      | sent to active entity if mouse is   |
|                  |               | dragged back over it after          |
|                  |               | onDragOut                           |
+------------------+---------------+-------------------------------------+

There is always one active entity at any given Time (considering NULL to
be an active entity, representing the background, and other objects that
don't receive mouse events).

When the mouse button is up, the active entity is the topmost element
directly under the mouse pointer.

When the mouse button is down, the active entity remains whatever it
was when the button last went down.

The active entity is the only object that receives mouse events.

!!! The "trackAsMenu" property alters this behavior!  If trackAsMenu
is set on the active entity, then onReleaseOutside is filtered out,
and onDragOver from another entity is Allowed (from the background, or
another trackAsMenu entity). !!!

*/

namespace Scaleform { namespace GFx {
//
Button::Button(ButtonDef* def, 
               MovieDefImpl *pbindingDefImpl,
               ASMovieRootBase* pasRoot, 
               InteractiveObject* parent, 
               ResourceId id)
: 
    InteractiveObject(pbindingDefImpl, pasRoot, parent, id),
    pDef(def),
    LastMouseFlags(IDLE),
    mMouseFlags(IDLE),
    MouseState(ButtonRecord::MouseUp)
{
    SF_ASSERT(pDef);

    SetScale9Grid(def->GetScale9Grid());
    SetTrackAsMenuFlag(pDef->Menu);

    //RecordCharacter.Resize(pDef->ButtonRecords.GetSize());
}

Button::~Button()
{
}

void Button::SetDirtyFlag()
{
    //GetMovieImpl()->SetDirtyFlag();
}

void Button::SetScale9Grid(const RectF& rect)
{
    bool propagate = rect != GetScale9Grid();
    DisplayObject::SetScale9Grid(rect);
    SetScale9GridExists(!rect.IsEmpty());
    if (propagate)
        PropagateScale9GridExists();
}

void Button::PropagateScale9GridExists()
{
    bool actualGrid = HasScale9Grid();
    // Stop cleaning up scale9Grid if actual one exists in the node
    if (!DoesScale9GridExist() && actualGrid)
        return; 

    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            DisplayObjectBase* _ch = chars[i].Char;
            if (!_ch)
                continue;
            InteractiveObject* ch = _ch->CharToInteractiveObject();
            if (ch)
            {
                ch->SetScale9GridExists(DoesScale9GridExist() || actualGrid);
                ch->PropagateScale9GridExists();
            }
        }   
    }
}

void Button::PropagateNoAdvanceGlobalFlag()
{
    bool actualValue = IsNoAdvanceGlobalFlagSet();
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return;

    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            DisplayObjectBase* _ch = chars[i].Char;
            if (!_ch)
                continue;
            InteractiveObject* ch = _ch->CharToInteractiveObject();
            if (ch)
            {
                ch->SetNoAdvanceGlobalFlag(IsNoAdvanceGlobalFlagSet() || actualValue);
                ch->PropagateNoAdvanceGlobalFlag();
                ch->ModifyOptimizedPlayList();
            }
        }   
    }
}

void Button::PropagateNoAdvanceLocalFlag()
{
    bool actualValue = IsNoAdvanceLocalFlagSet();
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return;

    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            DisplayObjectBase* _ch = chars[i].Char;
            if (!_ch)
                continue;
            InteractiveObject* ch = _ch->CharToInteractiveObject();
            if (ch)
            {
                ch->SetNoAdvanceLocalFlag(IsNoAdvanceLocalFlagSet() || actualValue);
                ch->PropagateNoAdvanceLocalFlag();
                ch->ModifyOptimizedPlayList();
            }
        }   
    }
}

void Button::SetVisible(bool visible)            
{ 
    SetVisibleFlag(visible); 
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return;
    bool noAdvGlob = !visible && proot->IsNoInvisibleAdvanceFlagSet();
    if (noAdvGlob != IsNoAdvanceGlobalFlagSet())
    {
        SetNoAdvanceGlobalFlag(noAdvGlob);
        ModifyOptimizedPlayListLocal<Button>();
        InteractiveObject* pparent = GetParent();
        if (pparent && !pparent->IsNoAdvanceGlobalFlagSet())
            PropagateNoAdvanceGlobalFlag();
    }
    SetDirtyFlag(); 
}

void Button::Restart()
{
    LastMouseFlags = IDLE;
    mMouseFlags = IDLE;
    MouseState = ButtonRecord::MouseUp;
    RollOverCnt = 0;
    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            if (chars[i].Char)
                chars[i].Char->Restart();
        }
    }
    SetDirtyFlag();
}

// int Button::GetRecordIndex(ButtonRecord::MouseState mouseState) const
// {
//     for (unsigned i = 0; i < pDef->ButtonRecords.GetSize(); i++)
//     {
//         ButtonRecord& rec = pDef->ButtonRecords[i];
//         if (!RecordCharacter[i])                
//             continue;               
//         if (rec.MatchMouseState(mouseState))
//         {
//             return i;
//         }
//     }
//     // Not found
//     return -1;
// }

void Button::OnEventLoad()
{
    InteractiveObject::OnEventLoad();
    CreateCharacters();
}

void Button::OnEventUnload()
{
    SetUnloading();
    UnloadCharactersForState(Up); 
    UnloadCharactersForState(Over); 
    UnloadCharactersForState(Down); 
    UnloadCharactersForState(Hit); 
    InteractiveObject::OnEventUnload();
}

bool Button::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{
    if (IsHitTestDisableFlagSet())
        return false;

    if ((hitTestMask & HitTest_IgnoreInvisible) && !GetVisible())
        return false;

    if (!DoesScale9GridExist())
    {
        if (!GetBounds(Matrix2F()).Contains(pt))
            return false;
        else if (!(hitTestMask & HitTest_TestShape))
            return true;
    }

    const StateCharArray& chars = States[Hit].Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase* ch = chars[i].Char;
        if (!ch)
            continue;

        if ((hitTestMask & HitTest_IgnoreInvisible) && !ch->GetVisible())
            continue;

        Matrix2F   m = ch->GetMatrix();
        Render::PointF             p = m.TransformByInverse(pt);   

        if (ch->PointTestLocal (p, hitTestMask))
            return true;
    }   
    return false;
}

// Return the topmost entity that the given point covers.  NULL if none.
// I.E. check against ourself.
DisplayObjectBase::TopMostResult 
Button::GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr) 
{
    pdescr->pResult = NULL;
    if (!GetVisible())
        return TopMost_FoundNothing; 

    if (pdescr->pIgnoreMC == this)
        return TopMost_FoundNothing;

    if (!IsFocusAllowed(GetMovieImpl(), pdescr->ControllerIdx)) 
        return TopMost_FoundNothing;

    Render::PointF p;
    if (!TransformPointToLocalAndCheckBounds(&p, pt))
        return TopMost_FoundNothing;

    const StateCharArray& chars = States[Hit].Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase* pcharacter = chars[i].Char;

        if (pcharacter)
        {
            Render::PointF subp;
            pcharacter->TransformPointToLocal(&subp, p);

            if (pcharacter->PointTestLocal (subp, HitTest_TestShape))
            {
                pdescr->pResult = this;
                return TopMost_Found;
            }
        }
    }
    pdescr->LocalPt = p;
    return TopMost_Continue;
}

// Return a single character bounds
RectF  Button::GetBoundsOfRecord(const Matrix &transform, DisplayObjectBase* pch) const
{
    // Custom based on state.
    RectF  bounds(0);
    Matrix  m;

    if (pch)
    {
        m = transform;
        m *= pch->GetMatrix();
        bounds = pch->GetBounds(m);
    }
    
    return bounds;
}

// Return a single character "pure rectangle" bounds (not considering the stroke)
RectF  Button::GetRectBounds(const Matrix &transform, DisplayObjectBase* pch) const
{
    // Custom based on state.
    RectF  bounds(0);
    Matrix  m;

    if (pch)
    {
        m = transform;
        m *= pch->GetMatrix();
        bounds = pch->GetRectBounds(m);
    }
    
    return bounds;
}


// Get bounds. This is used,
// among other things, to calculate button width & height.
RectF  Button::GetBounds(const Matrix &transform) const
{
    // Custom based on state.
    RectF  bounds(0);
    RectF  tempRect;
    bool   boundsInit = 0;

    const StateCharArray& chars = GetCharacters(MouseState).Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase*  pch = chars[i].Char;

        tempRect = GetBoundsOfRecord(transform, pch);
        if (!tempRect.IsNull())
        {
            if (!boundsInit)
            {
                bounds = tempRect;
                boundsInit = 1;
            }
            else
            {
                bounds.Union(tempRect);
            }
        }
    }
    return bounds;
}

// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF Button::GetRectBounds(const Matrix &transform) const
{
    // Custom based on state.
    RectF  bounds(0);
    RectF  tempRect;
    bool    boundsInit = 0;

    const StateCharArray& chars = GetCharacters(MouseState).Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase*  pch = chars[i].Char;
        // TO DO: Clarify this, whether or not we should match the state.
        // If yes, it may change the scale9grid configuration and the appearance.
        //ButtonRecord& rec = pDef->ButtonRecords[i];
        //if (rec.MatchMouseState(MouseState)) 
        {
            tempRect = GetRectBounds(transform, pch);
            if (!tempRect.IsNull())
            {
                if (!boundsInit)
                {
                    bounds = tempRect;
                    boundsInit = 1;
                }
                else
                {
                    bounds.Union(tempRect);
                }
            }
        }
    }
    return bounds;
}

// returns the local boundaries of whole state
RectF  Button::GetBoundsOfState(const Matrix &transform, ButtonState state) const
{
    // Custom based on state.
    RectF  bounds(0);
    RectF  tempRect;

    const StateCharArray& chars = States[state].Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase*  pch = chars[i].Char;

        tempRect = GetBoundsOfRecord(transform, pch);
        if (!tempRect.IsNull())
        {
            if (bounds.IsNull())
                bounds = tempRect;
            else
                bounds.Union(tempRect);
        }
    }
    
    return bounds;
}

// focus rect for buttons is calculated as follows:
// 1) if "hit" state exists - boundary rect of "hit" shape
// 2) if "down" state exists  - boundary rect of "down" shape
// 3) if "over" state exists  - boundary rect of "over" shape
// 4) otherwise - boundary rect of "up" shape
RectF Button::GetFocusRect() const 
{
    Matrix2F m;
    RectF tempRect;
    if (!(tempRect = GetBoundsOfState(m, Hit)).IsNull())
        return tempRect;
    else if (!(tempRect = GetBoundsOfState(m, Down)).IsNull())
        return tempRect;
    else if (!(tempRect = GetBoundsOfState(m, Over)).IsNull())
        return tempRect;
    else if (!(tempRect = GetBoundsOfState(m, Up)).IsNull())
        return tempRect;
    return GetBounds(m); // shouldn't reach this point, actually
}

// invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed)
void Button::OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt)
{
    if (fmt == GFx_FocusMovedByKeyboard)
        OnMouseEvent(EventId(EventId::Event_RollOver, Key::Tab, 0, 0, UInt8(controllerIdx)));
}

// invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
bool Button::OnLosingKeyboardFocus(InteractiveObject*, unsigned controllerIdx, FocusMovedType) 
{
    MovieImpl* proot = GetMovieImpl();
    if (proot->IsFocusRectShown(controllerIdx))
        OnMouseEvent(EventId(EventId::Event_RollOut, Key::Tab, 0, 0, UInt8(controllerIdx)));
    return true;
}

void Button::PropagateMouseEvent(const EventId& evt)
{
    if (evt.Id == EventId::Event_MouseMove)
    {
        // Implement mouse-drag.
        InteractiveObject::DoMouseDrag(evt.MouseIndex);
    }

    InteractiveObject::PropagateMouseEvent(evt);
}

/* returns true, if event fired */
bool Button::OnMouseEvent(const EventId& evt)
{
    if (IsUnloading() || IsUnloaded())
        return false;
    bool handlerFound = false;

    //ButtonRecord::MouseState prevMouseState = MouseState;
    if (evt.RollOverCnt == 0) // RollOverCnt is not zero only when multiple mice used
                                // and only for onRoll/Drag/Over/Out events
    {
        GFx::MouseState& ms = *GetMovieImpl()->GetMouseState(evt.MouseIndex);
        SF_ASSERT(&ms);

        // Set our mouse State (so we know how to render).
        switch (evt.Id)
        {
        case EventId::Event_RollOut:
        case EventId::Event_ReleaseOutside:
            MouseState = ButtonRecord::MouseUp;
            break;

        case EventId::Event_MouseUp:
        case EventId::Event_Release:
            MouseState = ButtonRecord::MouseOver;
            break;
        case EventId::Event_RollOver:
            if (GetTrackAsMenu() && (ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left))
                MouseState = ButtonRecord::MouseDown;
            else if (!(ms.GetButtonsState() & GFx::MouseState::MouseButton_Left))
                MouseState = ButtonRecord::MouseOver;
            else // prevent button from changing state if left mouse button is pressed (like in Flash)
                return false;
            break;
        case EventId::Event_DragOut:
            if (GetTrackAsMenu())
                MouseState = ButtonRecord::MouseUp;
            else
                MouseState = ButtonRecord::MouseOver;
            break;

        case EventId::Event_MouseDown:
        case EventId::Event_Press:
        case EventId::Event_DragOver:
            MouseState = ButtonRecord::MouseDown;
            break;

        case EventId::Event_KeyPress:
            break; 
        default:
            //SF_ASSERT(0); // missed a case?
            break;
        };

#ifdef GFX_ENABLE_SOUND
        // Button transition sounds.
        if (pDef->pSound != NULL)
        {
            int bi; // button sound Array index [0..3]
            switch (evt.Id)
            {
            case EventId::Event_RollOut:
                bi = 0;
                break;
            case EventId::Event_RollOver:
                bi = 1;
                break;
            case EventId::Event_Press:
                bi = 2;
                break;
            case EventId::Event_Release:
                bi = 3;
                break;
            default:
                bi = -1;
                break;
            }
            pDef->pSound->Play(this, bi);
        }
#endif // SF_NO_SOUND

        // recreate the characters of the new state and release ones for previous state.
        SwitchState();
    }

    // do not fire the AS event if disableFocusRollOver == true and the RollOver/Out
    // events were produced by a keyboard.
    if (GetMovieImpl()->IsDisableFocusRolloverEvent() && 
        (evt.Id == EventId::Event_RollOver || evt.Id == EventId::Event_RollOut) && 
        evt.KeyCode != Key::None)
        return handlerFound;

    if (HasAvmObject())
        handlerFound = GetAvmButton()->OnMouseEvent(evt);
    return handlerFound;
}

void Button::UnloadCharactersForState(ButtonState state)
{
    // unload character
    StateCharArray& chars = States[state].Characters;
    for (UPInt i = 0; i < chars.GetSize(); ++i)
    {
        DisplayObjectBase* ch = chars[i].Char;

        InteractiveObject* pscriptCh = ch->CharToInteractiveObject();
        if (pscriptCh)
        {
            bool mayRemove = ch->OnUnloading();
            if (mayRemove)
            {
                // We can remove the object instantly, so, do it now.
                ch->OnUnload();
            }
        }
        chars[i].Char = NULL;
    }
    chars.Clear();
    ClearRenderTreeForState(state);
}

void Button::ClearRenderTreeForState(ButtonState state)
{
    if (States[state].pRenNode)
    {
        States[state].pRenNode->Remove(0, States[state].pRenNode->GetSize());
        // detach the render node
        if (States[state].pRenNode->GetParent())
        {
            GetRenderContainer()->Remove(0, 1);
        }
    }
}

void Button::RemoveDisplayObject(DisplayObjectBase* chToRemove)
{
    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            DisplayObjectBase* _ch = chars[i].Char;
            if (!_ch)
                continue;
            InteractiveObject* ch = _ch->CharToInteractiveObject();
            if (ch == chToRemove)
            {
                ch->OnEventUnload();
                chars[i].Char = NULL;
            }
        }   
    }
}

void Button::CreateCharacters()
{
    if (HasAvmObject())
        GetAvmButton()->CreateCharacters();
}

void Button::SwitchState()
{
    if (HasAvmObject())
        GetAvmButton()->SwitchState(MouseState);
}

// Set state changed flags
void Button::SetStateChangeFlags(UInt8 flags)
{
    InteractiveObject::SetStateChangeFlags(flags);
    for (UPInt s = 0; s < StatesCount; ++s)
    {
        StateCharArray& chars = States[s].Characters;
        for (UPInt i = 0; i < chars.GetSize(); ++i)
        {
            DisplayObjectBase* ch = chars[i].Char;
            if (ch)
                ch->SetStateChangeFlags(flags);
        }   
    }
}

bool Button::OnKeyEvent(const EventId& id, int* pkeyMask) 
{
    if (HasAvmObject())
        GetAvmButton()->OnButtonKeyEvent(id, pkeyMask);

    if (id.Id == EventId::Event_KeyDown)
    {
        // covert Event_KeyDown to Event_KeyPress
        SF_ASSERT (pkeyMask != 0);

        // check if keyPress already was handled then do not handle it again
        if (!((*pkeyMask) & InteractiveObject::KeyMask_KeyPress))
        {
            // If ascii code is not specified try to figure it out...
            UByte ascii = id.AsciiCode;
            if (!id.AsciiCode)
            {
                if (id.WcharCode && id.WcharCode >= 32 && id.WcharCode < 128)
                    ascii = (UByte)id.WcharCode;
                else if (id.KeyCode >= 32)
                {
                    // attempt to figure it out from a keycode. It might be not accurate
                    // if CapsLock state is not set correctly in KeyboardState.
                    
                    ascii = id.ConvertKeyCodeToAscii();
                }
            }
            if (OnMouseEvent(EventId (EventId::Event_KeyPress, id.KeyCode, ascii)))
            {
                *pkeyMask |= InteractiveObject::KeyMask_KeyPress;
            }
        }

        MovieImpl* proot = GetMovieImpl();
        if (proot->IsKeyboardFocused(this, id.KeyboardIndex) && (id.KeyCode == Key::Return || id.KeyCode == Key::Space))
        {
            if (IsFocusRectEnabled() || proot->IsAlwaysEnableKeyboardPress())
            {
                // if focused and enter - simulate on(press)/onPress and on(release)/onRelease
                OnMouseEvent(EventId(EventId::Event_Press, Key::Return, 0, 0, id.KeyboardIndex));

                Ptr<InteractiveObject> thisHolder = this;
                proot->Advance(0, 0); //??AB, allow on(press) to be executed 
                // completely before on(release). Otherwise, these events may affect each other
                // (see focusKB_test.swf). Need further investigations.

                OnMouseEvent(EventId(EventId::Event_Release, Key::Return, 0, 0, id.KeyboardIndex));
            }
        }
    }
    return true;
}

void Button::AdvanceFrame(bool nextFrame, float /*framePos*/)
{
    if (nextFrame)
    {   
        if (HasAvmObject())
        {
            // Post OnEnterFrame event, so that it arrives before other
            // frame actions generated by frame tags.
            GetAvmButton()->OnEvent(EventId::Event_EnterFrame);
        }
    }    
}

// Returns 0 if nothing to do
// 1 - if need to add to optimized play list
// -1 - if need to remove from optimized play list
int Button::CheckAdvanceStatus(bool playingNow)
{
    int rv = 0;
    // First of all, check if advance is disabled at all. Advance is disabled
    // * if noAdvance extension property is set;
    // * if sprite is invisible and _global.noInvisibleAdvance extension property is set;
    // * if sprite is unloading or unloaded from timeline.
    bool advanceDisabled = (IsAdvanceDisabled() || !CanAdvanceChar());

    // Check if movie is playable.
    //bool advancable = (!advanceDisabled && GetPlayState() == State_Playing);
    bool advancable = !advanceDisabled;

    if (advancable) 
    {
        // if it is already playing and advancable - do nothing (return 0).
        // otherwise, return 1 (play).
        rv = !playingNow; //(playingNow) ? 0 : 1
#ifdef SF_TRACE_ADVANCE
        if (rv)
        {
            printf("+++ Advance: Starting to play: %s, state %s\n", GetCharacterHandle()->GetNamePath().ToCStr(),
                (GetPlayState() == State_Playing) ? "playing" : "stopped");
        }
#endif
    }
    else
    {
        if (playingNow)
        {
            // check, if the onEnterFrame exists
            if (!advanceDisabled)
            {
                if (!HasEventHandler(EventId::Event_EnterFrame))
                {
                    rv = -1;
#ifdef SF_TRACE_ADVANCE
                    printf("+++ Advance: Stopping play (no onEnterFrame): %s, state %s\n", 
                        GetCharacterHandle()->GetNamePath().ToCStr(),
                        (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
                }
            }
            else
            {
                rv = -1;
#ifdef SF_TRACE_ADVANCE
                printf("+++ Advance: Stopping play (adv disabled): %s, state %s\n", 
                    GetCharacterHandle()->GetNamePath().ToCStr(),
                    (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
            }
        }
        else
        {
            if (!advanceDisabled)
            {
                if (HasEventHandler(EventId::Event_EnterFrame))
                {
                    rv = 1;
#ifdef SF_TRACE_ADVANCE
                    printf("+++ Advance: Starting to play (onEnterFrame): %s, state %s\n", 
                        GetCharacterHandle()->GetNamePath().ToCStr(),
                        (GetPlayState() == State_Playing) ? "playing" : "stopped");
#endif
                }
            }
        }
    }
    return rv;
}

Button::ButtonState      Button::GetButtonState(ButtonRecord::MouseState mouseState) 
{
    Button::ButtonState i = Up;
    switch(mouseState)
    {
    case ButtonRecord::MouseUp:   i = Up; break;
    case ButtonRecord::MouseDown: i = Down; break;
    case ButtonRecord::MouseOver: i = Over; break;
    default: SF_ASSERT(0);
    }
    return i;
}

Ptr<Render::TreeContainer> Button::CreateStateRenderContainer(ButtonState buttonState)
{
    States[buttonState].pRenNode = *GetRenderContext().CreateEntry<Render::TreeContainer>();
    return States[buttonState].pRenNode;
}

}} //namespace Scaleform::GFx
