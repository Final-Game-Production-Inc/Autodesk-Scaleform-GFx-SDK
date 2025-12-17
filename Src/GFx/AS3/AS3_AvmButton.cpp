/**************************************************************************

Filename    :   AS3_AvmButton.cpp
Content     :   Implementation of AS3-dependent part of Button character 
and its AS3 Button class.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_AvmButton.h"
#include "GFx/AS3/AS3_AvmSprite.h"

//#include "Render/Render_Renderer.h"

#include "GFx/Audio/GFx_Sound.h" 
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_Sprite.h"
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

namespace Scaleform {
namespace GFx {
namespace AS3 {

AvmButton::AvmButton(Button* pbutton)
    : AvmInteractiveObj(pbutton)
{
    pbutton->SetTabEnabledFlag(true);
}

ButtonDef* AvmButton::GetButtonDef() const
{
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::Button);
    return static_cast<Button*>(pDispObj)->GetButtonDef();
}

bool AvmButton::OnMouseEvent(const EventId& evt)
{
    if (evt.Id == EventId::Event_Press)
    {
        if (HasAS3Obj())
        {
            //simulate Click event
            EventId e(evt);
            e.Id = EventId::Event_Click;
            GetAS3Obj()->Dispatch(e, pDispObj);
        }
        return true;
    }
    return OnEvent(evt);
}

bool AvmButton::OnButtonKeyEvent(const EventId& , int* )
{
    return true;
}

unsigned AvmButton::GetCursorType() const 
{ 
    // Theoretically, penv could be NULL if parent is null (means,
    // object is already disconnected).
    if (GetButton()->IsEnabledFlagSet())
    {
        return MouseCursorEvent::HAND;
    }
    return AvmInteractiveObj::GetCursorType();
}

void AvmButton::CreateCharacters()
{
    // AS3 creates all characters at once at the loading time
    Button* button  = GetButton();
    ButtonDef* pdef = GetButtonDef();
    Button::ButtonState currentState = button->GetCurrentState();
    for (UPInt i = 0, n = pdef->GetRecordsCount(); i < n; i++)
    {
        const ButtonRecord& rec = pdef->GetRecordAt(i);

        UInt8 flags = rec.GetFlags();

        for (UInt8 mask = 1; mask & ButtonRecord::Mask_All; mask <<= 1)
        {
            if (!(mask & flags))
                continue;

            Button::ButtonState curState = Button::Up;
            Button::StateCharArray* chars = NULL;
            if (mask & ButtonRecord::Mask_Up)
                curState = Button::Up;
            else if (mask & ButtonRecord::Mask_Down)
                curState = Button::Down;
            else if (mask & ButtonRecord::Mask_Over)
                curState = Button::Over;
            else if (mask & ButtonRecord::Mask_HitTest)
                curState = Button::Hit;
            else
                SF_ASSERT(0);

            chars = &button->GetCharacters(curState).Characters;
            Ptr<Render::TreeContainer> pcontainer = button->GetStateRenderContainer(curState);
            if (!pcontainer)
                pcontainer = button->CreateStateRenderContainer(curState);

            Ptr<Sprite> emptyParent;
            if (chars->GetSize() == 0)
            {
                // create a parent
                CharacterCreateInfo ccinfo = GetDefImpl()->
                    GetCharacterCreateInfo(ResourceId(CharacterDef::CharId_EmptyMovieClip));
                SF_ASSERT(ccinfo.pCharDef && ccinfo.pBindDefImpl);

                Ptr<DisplayObjectBase>  ch = *GetAS3Root()->GetASSupport()->CreateCharacterInstance
                    (GetMovieImpl(), ccinfo, NULL, rec.CharacterId);

                SF_ASSERT(ch);

                emptyParent = ch->CharToSprite();
                SF_ASSERT(emptyParent);

                chars->PushBack(Button::CharToRec(emptyParent, &rec));
                GetAS3Root()->AddScriptableMovieClip(emptyParent);
                //emptyParent->AddToPlayList();
                //emptyParent->ModifyOptimizedPlayList();

                pcontainer->Add(ch->GetRenderNode());
            }
            else
            {
                // get existing parent
                emptyParent = (*chars)[0].Char->CharToSprite();
                SF_ASSERT(chars->GetSize() == 1);
            }
            SF_ASSERT(emptyParent);

            // Create a character and add it as a child of "emptyParent"
            CharacterCreateInfo ccinfo = GetDefImpl()->GetCharacterCreateInfo(rec.CharacterId);
            SF_ASSERT(ccinfo.pCharDef && ccinfo.pBindDefImpl);

            if (ccinfo.pCharDef)
            {
                const Matrix2F&  mat = rec.ButtonMatrix;
                const Cxform&    cx  = rec.ButtonCxform;

                CharPosInfo pos;
                pos.ColorTransform  = cx;
                pos.Matrix_1        = mat;
                pos.CharacterId     = rec.CharacterId;
                pos.BlendMode       = (UInt8)rec.BlendMode;
                pos.Depth           = rec.Depth;
                pos.Flags           = CharPosInfoFlags::Flags_HasCxform | 
                    CharPosInfoFlags::Flags_HasMatrix | 
                    CharPosInfoFlags::Flags_HasCharacterId |
                    CharPosInfoFlags::Flags_HasBlendMode | 
                    CharPosInfoFlags::Flags_HasDepth;

                DisplayObjectBase* newCh = emptyParent->AddDisplayObject(pos,
                    GetAS3Root()->GetStringManager()->CreateEmptyString(), NULL, NULL,
                    0, DisplayList::Flags_PlaceObject);

                if (newCh && rec.pFilters)
                    newCh->SetFilters(rec.pFilters);

                if (button->DoesScale9GridExist())
                {
                    emptyParent->SetScale9GridExists(true);
                    emptyParent->PropagateScale9GridExists();
                }

            }
            
            // for scale9 hittesting it is necessary that hit chars have parent
            if (curState == Button::Hit)
                emptyParent->SetParent(button);

            if (currentState == curState && !pcontainer->GetParent())
                button->GetRenderContainer()->Add(pcontainer); // only one node should be added!
        }
    }
    // !AB: Actually, button->GetRenderContainer()->GetSize() can be 0, for example, when 
    // only HitTest record is presented.
    SF_ASSERT(button->GetRenderContainer()->GetSize() <= 1);
}

void AvmButton::SwitchState(ButtonRecord::MouseState mouseState)
{
    Button* button  = GetButton();
    if (button->IsUnloaded() || button->IsUnloading())
        return;

    Button::ButtonState bs = button->GetButtonState(mouseState);
    SwitchStateIntl(bs);
}

void AvmButton::SwitchStateIntl(Button::ButtonState bs)
{
    Button* button  = GetButton();
    if (button->IsUnloaded() || button->IsUnloading())
        return;
    
    SF_ASSERT(bs != Button::Hit); // can't switch to hit state!

    if (button->GetRenderContainer()->GetSize() > 0)
    {
        SF_ASSERT(button->GetRenderContainer()->GetSize() == 1);

        // remove from render tree
        button->GetRenderContainer()->Remove(0, 1);
    }

    // first, detach all inactive states
    for (unsigned i = 0; i < Button::StatesCount - 1; ++i)
    {
        const Button::StateCharArray& chars = 
            button->GetCharacters((Button::ButtonState)i).Characters;
        if (chars.GetSize() > 0)
        {
            SF_ASSERT(chars.GetSize() == 1);

            DisplayObject* character = chars[0].Char->CharToScriptableObject();
            DisplayObjContainer* sprite = character->CharToDisplayObjContainer();

            SF_ASSERT(character);

            if (i != unsigned(bs))
            {
                if (character->GetParent())
                {
                    // remove from its current parent.
                    if (ToAvmDisplayObj(character->GetParent())->DetachChild(character))
                    {
                        // should be no treenode parent after RemoveChild
                        SF_ASSERT(!character->GetRenderNode()->GetParent());
                    }
                    character->SetParent(0);
                    // re-insert into playlist
                    if (sprite)
                        ToAvmInteractiveObj(sprite)->MoveBranchInPlayList();
                }
            }
        }
    }

    const Button::StateCharArray& chars = button->GetCharacters(bs).Characters;
    if (chars.GetSize() > 0)
    {
        SF_ASSERT(chars.GetSize() == 1);

        DisplayObject* character = chars[0].Char->CharToScriptableObject();
        DisplayObjContainer* sprite = character->CharToDisplayObjContainer();

        SF_ASSERT(character);

        Ptr<Render::TreeContainer> pcontainer = button->GetStateRenderContainer(bs);
        SF_ASSERT(pcontainer);

        // if parent or treenode parent mismatched - reinsert
        if (character->GetParent() && (character->GetParent() != button || 
            character->GetRenderNode()->GetParent() != pcontainer))
        {
            // remove from its current parent.
            ToAvmDisplayObj(character->GetParent())->DetachChild(character);
            // should be no treenode parent after RemoveChild
            SF_ASSERT(!character->GetRenderNode()->GetParent());
        }
        // make sure render treenode is a child of the container.
        if (!character->GetRenderNode()->GetParent())
            pcontainer->Add(character->GetRenderNode());
        if (!character->GetParent())
        {
            character->SetParent(button);
            // re-insert into playlist
            if (sprite)
            {
                ToAvmInteractiveObj(sprite)->MoveBranchInPlayList();

                // Restart all parentSprite's children sprites.
                const DisplayList& dl = sprite->GetDisplayList();
                for (UPInt i = 0; i < dl.GetCount(); ++i)
                {
                    // Can't use Restart here, since Flash doesn't execute initialization logic again.
                    // Seems like "gotoAndPlay(1)" is the way to go.
                    DisplayObjectBase* dobj = dl.GetDisplayObject(i);
                    if (dobj->IsSprite())
                    {
                        Sprite* spr = dobj->CharToSprite_Unsafe();
                        spr->GotoFrame(0); // 1st frame (0-based)
                        spr->SetPlayState(State_Playing);
                    }
                }
            }
        }
        SF_ASSERT(!pcontainer->GetParent());
        button->GetRenderContainer()->Add(pcontainer); // only one node should be added!
    }
}

bool AvmButton::DetachChild(DisplayObjectBase* child)
{
    bool rv = false;
    Button* button  = GetButton();
    for (unsigned i = 0; i < Button::StatesCount - 1; ++i)
    {
        Ptr<Render::TreeContainer> pcontainer = button->GetStateRenderContainer((Button::ButtonState)i);
        const Button::StateCharArray& chars = 
            button->GetCharacters((Button::ButtonState)i).Characters;
        if (chars.GetSize() > 0)
        {
            for(UPInt j = 0, n = chars.GetSize(); j < n; ++j)
            {
                if (chars[j].Char == child)
                {
                    child->SetParent(NULL);
                    if (child->GetRenderNode())
                    {
                        Render::TreeContainer* treePcontainer = static_cast<Render::TreeContainer*>
                            (child->GetRenderNode()->GetParent());
                        if (treePcontainer)
                        {
                            for(UPInt k = 0, kn = treePcontainer->GetSize(); k < kn; ++k)
                            {
                                if (treePcontainer->GetAt(k) == child->GetRenderNode())
                                {
                                    treePcontainer->Remove(k, 1);
                                    break;
                                }
                            }
                        }
                    }
                    rv = true;
                }
            }
        }
    }
    return rv;
}

void AvmButton::SetStateObject(Button::ButtonState state, DisplayObject* ch)
{
    Button* button  = GetButton();

    if (state != Button::Hit)
    {
        Ptr<Render::TreeContainer> pcontainer = button->GetStateRenderContainer(state);
        if (!pcontainer)
            pcontainer = button->CreateStateRenderContainer(state);
        else
        {
            // cleanup all existing child TreeNodes in pcontainer
            pcontainer->Remove(0, pcontainer->GetSize());
        }
        SF_ASSERT(pcontainer);

        if (ch->GetParent() && (ch->GetParent() != button || 
            ch->GetRenderNode()->GetParent() != pcontainer))
        {
            // need to remove the child from its current location
            if (ToAvmDisplayObj(ch->GetParent())->DetachChild(ch))
            {
                // should be no treenode parent after RemoveChild
                SF_ASSERT(!ch->GetRenderNode()->GetParent());
                ch->SetParent(button);
            }
        }
        // make sure render treenode is a child of the container.
        if (!ch->GetRenderNode()->GetParent())
            pcontainer->Add(ch->GetRenderNode());
    }

    Button::StateCharArray& chars = button->GetCharacters(state).Characters;
    if (ch)
    {
        chars.Resize(1);
        chars[0].Char = ch;
    }
    else
        chars.Resize(0);

    if (button->GetCurrentState() == state)
        SwitchStateIntl(state);
}

DisplayObject* AvmButton::GetUpStateObject() const
{
    Button* button  = GetButton();
    const Button::StateCharArray& chars = button->GetCharacters(Button::Up).Characters;
    if (chars.GetSize() > 0)
        return static_cast<DisplayObject*>(chars[0].Char.GetPtr());
    return NULL;
}

void AvmButton::SetUpStateObject(DisplayObject* ch)
{
    SetStateObject(Button::Up, ch);
}

DisplayObject*          AvmButton::GetOverStateObject() const
{
    Button* button  = GetButton();
    const Button::StateCharArray& chars = button->GetCharacters(Button::Over).Characters;
    if (chars.GetSize() > 0)
        return static_cast<DisplayObject*>(chars[0].Char.GetPtr());
    return NULL;
}

void                    AvmButton::SetOverStateObject(DisplayObject* ch)
{
    SetStateObject(Button::Over, ch);
}

DisplayObject*          AvmButton::GetDownStateObject() const
{
    Button* button  = GetButton();
    const Button::StateCharArray& chars = button->GetCharacters(Button::Down).Characters;
    if (chars.GetSize() > 0)
        return static_cast<DisplayObject*>(chars[0].Char.GetPtr());
    return NULL;
}

void                    AvmButton::SetDownStateObject(DisplayObject* ch)
{
    SetStateObject(Button::Down, ch);
}

DisplayObject*          AvmButton::GetHitStateObject() const
{
    Button* button  = GetButton();
    const Button::StateCharArray& chars = button->GetCharacters(Button::Hit).Characters;
    if (chars.GetSize() > 0)
        return static_cast<DisplayObject*>(chars[0].Char.GetPtr());
    return NULL;
}

void                    AvmButton::SetHitStateObject(DisplayObject* ch)
{
    SetStateObject(Button::Hit, ch);
}

}}} // SF::GFx::AS3

