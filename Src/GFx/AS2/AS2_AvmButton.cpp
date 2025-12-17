/**************************************************************************

Filename    :   AS2_AvmButton.cpp
Content     :   Implementation of AS2-dependent part of Button character 
                and its AS2 Button class.
Created     :   Dec, 2009
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_AvmButton.h"
#include "GFx/AS2/AS2_AvmSprite.h"

//#include "Render/Render_Renderer.h"

#include "GFx/AS2/AS2_Action.h"
#include "GFx/Audio/GFx_Sound.h" 
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/AS2/AS2_RectangleObject.h"
//#include "GFx/GFx_Loader.h"
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


namespace Scaleform { namespace GFx { namespace AS2 {

AvmButton::AvmButton(Button* pbutton)
: AvmCharacter(pbutton)
{
    //ASButtonObj = *SF_HEAP_AUTO_NEW(this) ButtonObject(GetGC(), this);
    // let the base class know what's going on
    //pProto = ASButtonObj->Get__proto__();
    pProto = GetGC()->GetActualPrototype(GetASEnvironment(), ASBuiltin_Button);
}

ButtonDef* AvmButton::GetButtonDef() const
{
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::Button);
    return static_cast<Button*>(pDispObj)->GetButtonDef();
}

bool AvmButton::OnMouseEvent(const EventId& event)
{
    bool handlerFound = false;

    if (event.RollOverCnt == 0) // RollOverCnt is not zero only when multiple mice used
        // and only for onRoll/Drag/Over/Out events
    {
        // @@ eh, should just be a lookup table.
        int c = 0, kc = 0;
        if (event.Id == EventId::Event_RollOver) c |= (ButtonAction::IDLE_TO_OVER_UP);
        else if (event.Id == EventId::Event_RollOut) c |= (ButtonAction::OVER_UP_TO_IDLE);
        else if (event.Id == EventId::Event_Press) c |= (ButtonAction::OVER_UP_TO_OVER_DOWN);
        else if (event.Id == EventId::Event_Release) c |= (ButtonAction::OVER_DOWN_TO_OVER_UP);
        else if (event.Id == EventId::Event_DragOut) c |= (ButtonAction::OVER_DOWN_TO_OUT_DOWN);
        else if (event.Id == EventId::Event_DragOver) c |= (ButtonAction::OUT_DOWN_TO_OVER_DOWN);
        else if (event.Id == EventId::Event_ReleaseOutside) c |= (ButtonAction::OUT_DOWN_TO_IDLE);
        else if (event.Id == EventId::Event_KeyPress) 
        {
            // convert keycode/ascii to button's keycode
            kc = event.ConvertToButtonKeyCode();
        }
        //IDLE_TO_OVER_DOWN = 1 << 7,
        //OVER_DOWN_TO_IDLE = 1 << 8,

        InteractiveObject* pparent = GetParent();                
        if (pparent)
        {
            Sprite* pparentSprite = pparent->CharToSprite();                
            if (pparentSprite)
            {
                ButtonDef* pdef = GetButtonDef();
                // Add appropriate actions to the GFxMovieSub's execute list...
                for (UPInt i = 0, n = pdef->GetButtonActionsCount(); i < n; i++)
                {
                    const ButtonAction* action = static_cast<const ButtonAction*>(pdef->GetButtonAction(i));
                    if (((action->Conditions & (~0xFE00)) & c) ||     //!AB??
                        (kc > 0 && ((action->Conditions >> 9) & 0x7F) == kc))
                    {
                        // Matching action
                        AvmSprite* avmParentSpr = GFx::AS2::ToAvmSprite(pparentSprite);
                        ASStringContext* psc = avmParentSpr->GetASEnvironment()->GetSC();
                        const UPInt an = action->Actions.GetSize();
                        for (UPInt j = 0; j < an; ++j)
                        {
                            if (!action->Actions[j]->IsNull())
                            {
                                Ptr<ActionBuffer> pbuff = 
                                    *SF_HEAP_NEW(psc->GetHeap()) ActionBuffer(psc, action->Actions[j]);
                                avmParentSpr->AddActionBuffer(pbuff);
                            }
                        }
                        if (an > 0) handlerFound = true;
                    }
                }
            }
        }
    }
    // Call conventional attached method.
    // Check for member function, it is called after onClipEvent(). 
    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    Environment *penv = GetASEnvironment();
    if (penv)
    {
        // penv can be NULL, if the character has already been removed
        // from the display list by earlier handlers (for example, by
        // "Mouse" class listeners, or by "on(...)" handler).
        ASString methodName(EventId_GetFunctionName(penv->GetStringManager(), event));

        if (methodName.GetSize() > 0)
        {
            Value method;
            if (GetMemberRaw(penv->GetSC(), methodName, &method))
            {
                handlerFound = true;
                // do actual dispatch
                MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction();
                if (pe) pe->SetAction(pDispObj, event);
            }
        }
    }
    return handlerFound;
}

void AvmButton::ConstructCharacter(InteractiveObject* pscriptCh, const ButtonRecord& rec)
{

    //MovieImpl* proot = GetMovieImpl();
    //SF_ASSERT(proot);
    if (pscriptCh->IsSprite())
    {
        GlobalContext* gctxt = this->GetGC();
        SF_ASSERT(gctxt);

        FunctionRef ctorFunc;    
        const String* psymbolName = pscriptCh->GetResourceMovieDef()->GetNameOfExportedResource(rec.CharacterId);

        if (psymbolName)
        {
            ASString symbolName = GetASEnvironment()->CreateString(*psymbolName);

            if (gctxt->FindRegisteredClass(GetASEnvironment()->GetSC(), symbolName, &ctorFunc))
            {
                GFx::AS2::ToAvmCharacter(pscriptCh)->SetProtoToPrototypeOf(ctorFunc.GetObjectPtr());

                // schedule "construct" event, if any
                //if (hasConstructEvent) //!AB, fire onConstruct always, since it is impossible
                {                        // to check whether onConstruct method exists or not
                    MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction(MovieRoot::AP_Construct);
                    if (pe) pe->SetAction(pscriptCh, EventId::Event_Construct);
                }

                // now, schedule constructor invocation
                MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, ctorFunc);
            }
            else
            {
                ValueArray params;
                params.PushBack(Value(symbolName)); //[0]
                //params.PushBack(Value(hasConstructEvent)); //[1]

                MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction(MovieRoot::AP_Construct);
                if (pe) pe->SetAction(pscriptCh, AvmSprite::FindClassAndInitializeClassInstance, &params);
            }
        }
    }
}

bool AvmButton::OnButtonKeyEvent(const EventId& id, int* )
{
    // Check for member function.   
    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.
    Environment *penv = GetASEnvironment();
    ASString       methodName(EventId_GetFunctionName(penv->GetStringManager(), id));
    MovieImpl*   proot = GetMovieImpl();

    if (methodName.GetSize() > 0)
    {           
        Value    method;
        if ((id.Id == EventId::Event_KeyDown || id.Id == EventId::Event_KeyUp) &&
            GetMemberRaw(penv->GetSC(), methodName, &method)) 
        {
            // onKeyDown/onKeyUp are available only in Flash 6 and later
            // (don't mess with onClipEvent (keyDown/keyUp)!)
            if (penv->GetVersion() >= 6 && proot->IsKeyboardFocused(pDispObj, id.KeyboardIndex))
            {
                // also, onKeyDown/onKeyUp should be invoked only if focus
                // is enabled and set to this button

                // do actual dispatch
                MovieRoot::ActionEntry* pe = GetAS2Root()->InsertEmptyAction();
                if (pe) pe->SetAction(pDispObj, id);
            }
        }
    }
    return true;
}

unsigned AvmButton::GetCursorType() const 
{ 
    const Environment* penv = GetASEnvironment();
    Value val;

    // Theoretically, penv could be NULL if parent is null (means,
    // object is already disconnected).
    if (penv && pDispObj->IsEnabledFlagSet())
    {
        if (ASButtonObj)
            const_cast<AvmButton*>(this)->GetMemberRaw
            (penv->GetSC(), penv->GetBuiltin(ASBuiltin_useHandCursor), &val);
        else if (pProto)
            pProto->GetMemberRaw
            (penv->GetSC(), penv->GetBuiltin(ASBuiltin_useHandCursor), &val);
        if (val.ToBool(penv))
            return MouseCursorEvent::HAND;
    }
    return AvmCharacter::GetCursorType();
}

ButtonObject* AvmButton::GetButtonASObject()          
{ 
    if (!ASButtonObj)
    {
        ASButtonObj = *SF_HEAP_AUTO_NEW(this) ButtonObject(GetGC(), GetButton());
    }
    return ASButtonObj;
}
Object*  AvmButton::GetASObject() 
{ 
    return GetButtonASObject(); 
}
Object*  AvmButton::GetASObject() const 
{ 
    return const_cast<AvmButton*>(this)->GetButtonASObject(); 
}

bool    AvmButton::GetMember(Environment* penv, const ASString& name, Value* pval)
{
    if (name.IsStandardMember())                    
        if (GetStandardMember(GetStandardMemberConstant(name), pval, 0))
            return true;        

    if (ASButtonObj)
    {
        return ASButtonObj->GetMember(penv, name, pval);
    }
    else 
    {
        // Handle the __proto__ property here, since we are going to 
        // zero out it temporarily (see comments below).
        if (penv && name == penv->GetBuiltin(ASBuiltin___proto__))
        {
            Object* proto = Get__proto__();
            pval->SetAsObject(proto);
            return true;
        }

        // Now we can search in the __proto__
        Object* proto = Get__proto__();
        if (proto)
        {
            // ASMovieClipObj is not created yet; use __proto__ instead
            if (proto->GetMember(penv, name, pval))    
            {
                return true;
            }
        }
    }
    // Looks like _global is accessible from any character
    if (penv && (name == penv->GetBuiltin(ASBuiltin__global)))
    {
        pval->SetAsObject(penv->GetGC()->pGlobal);
        return true;
    }
    return false;
}

void AvmButton::CreateCharacters()
{
    RecreateCharacters(ButtonRecord::MouseUp);
}

void AvmButton::SwitchState(ButtonRecord::MouseState mouseState)
{
    RecreateCharacters(mouseState);
}

Button::CharToRec AvmButton::FindCharacterAndRemove(const ButtonRecord& rec)
{
    Button* button  = GetButton();
    for (unsigned i = 0; i < Button::StatesCount; ++i)
    {
        if (i != Button::Hit)
        {
            Button::ButtonState curState = (Button::ButtonState)i;
            Button::StateCharArray& 
                chars = button->GetCharacters(curState).Characters;
            for (UPInt j = 0, n = chars.GetSize(); j < n; ++j)
            {
                if (chars[j].Record == &rec)
                {
                    Button::CharToRec chr = chars[j];
                    chars.RemoveAt(j);
                    return chr;
                }
            }
        }
    }
    return Button::CharToRec();
}

void AvmButton::RecreateCharacters(ButtonRecord::MouseState mouseState)
{
    ButtonDef* pdef = GetButtonDef();
    Button* button  = GetButton();

    Button::ButtonState bs = button->GetButtonState(mouseState);

    // clear render tree from all button states
    for (unsigned i = 0; i < Button::StatesCount; ++i)
    {
        if (i != unsigned(bs) && i != Button::Hit)
        {
            button->ClearRenderTreeForState((Button::ButtonState)i);
        }
    }
    // Restart our relevant characters
    for (unsigned i = 0; i < Button::StatesCount; ++i)
    {
        if (i != unsigned(bs) && i != Button::Hit)
        {
            continue;
        }

        // check if we need to create chars for the state "i".

        Button::ButtonState curState = (Button::ButtonState)i;
        Button::StateCharArray& chars = 
            button->GetCharacters(curState).Characters;
        Ptr<Render::TreeContainer> pContainer = button->GetStateRenderContainer(curState);
        if (chars.GetSize() == 0)
        {
            if (!pContainer)
                pContainer = button->CreateStateRenderContainer(curState);
            for (UPInt j = 0, n = pdef->GetRecordsCount(); j < n; ++j)
            {
                const ButtonRecord& rec = pdef->GetRecordAt(j);

                if ((i != Button::Hit && rec.MatchMouseState(mouseState)) || 
                    (i == Button::Hit && rec.IsHitTest()))
                {
                    const Matrix2F&  mat = rec.ButtonMatrix;
                    const Cxform&    cx  = rec.ButtonCxform;

                    // check if we have existing char in other states
                    Button::CharToRec chr;
                    if (i != Button::Hit)
                        chr = FindCharacterAndRemove(rec);
                    if (chr.Char)
                    {
                        // reuse existing instance
                        chars.PushBack(chr);
                        // insert render tree node
                        SF_ASSERT(button->GetRenderContainer());
                        pContainer->Add(chr.Char->GetRenderNode());

                        if (rec.pFilters)
                            chr.Char->SetFilters(rec.pFilters);

                        chr.Char->SetMatrix(mat);
                        chr.Char->SetCxform(cx);
                        chr.Char->SetBlendMode(rec.BlendMode);
                    }
                    else
                    {
                        // create character

                        // Resolve the DisplayObjectBase id.
                        CharacterCreateInfo ccinfo = GetDefImpl()->GetCharacterCreateInfo(rec.CharacterId);
                        SF_ASSERT(ccinfo.pCharDef && ccinfo.pBindDefImpl);

                        if (ccinfo.pCharDef)
                        {
                            Ptr<DisplayObjectBase> ch = *GetAS2Root()->GetASSupport()->CreateCharacterInstance
                                (GetMovieImpl(), ccinfo, GetButton(), rec.CharacterId);
                            chars.PushBack(Button::CharToRec(ch, &rec));

                            // insert render tree node
                            SF_ASSERT(button->GetRenderContainer());
                            pContainer->Add(ch->GetRenderNode());

                            if (rec.pFilters)
                                ch->SetFilters(rec.pFilters);


                            ch->SetMatrix(mat);
                            ch->SetCxform(cx);
                            ch->SetBlendMode(rec.BlendMode);

                            ch->SetScale9GridExists(false);
                            const InteractiveObject* parent = ch->GetParent();
                            while (parent)
                            {
                                if (parent->HasScale9Grid())
                                {
                                    ch->SetScale9GridExists(true);
                                    ch->PropagateScale9GridExists();
                                    break;
                                }
                                parent = parent->GetParent();
                            }

                            InteractiveObject* pscriptCh   = ch->CharToInteractiveObject();
                            if (pscriptCh)
                            {
                                ConstructCharacter(pscriptCh, rec);

                                pscriptCh->AddToPlayList();
                                pscriptCh->ModifyOptimizedPlayList();

                                ch->OnEventLoad();
                            }
                        }            
                    }
                }
            }
        }
        if (curState != Button::Hit && !pContainer->GetParent())
            button->GetRenderContainer()->Add(pContainer); // only one node should be added!
    }
    // unload remaining characters from other states
    for (unsigned i = 0; i < Button::StatesCount; ++i)
    {
        if (i != unsigned(bs) && i != Button::Hit)
        {
            // unload
            button->UnloadCharactersForState((Button::ButtonState)i);
        }
    }
    SF_ASSERT(button->GetRenderContainer()->GetSize() <= 1);
}

// InteractiveObject override to indicate which standard members are handled for us.
UInt32  AvmButton::GetStandardMemberBitMask() const
{
    // MovieClip lets base handle all members it supports.
    return  UInt32(
        M_BitMask_PhysicalMembers |         
        M_BitMask_CommonMembers |
        (1u << M_target) |
        (1u << M_url) |
        (1u << M_parent) |
        (1u << M_blendMode) |
        (1u << M_cacheAsBitmap) |
        (1u << M_filters) |
        (1u << M_focusrect) |
        (1u << M_enabled) |
        (1u << M_trackAsMenu) |                                  
        (1u << M_tabEnabled) |
        (1u << M_tabIndex) |
        (1u << M_useHandCursor) |
        (1u << M_quality) |
        (1u << M_highquality) |
        (1u << M_soundbuftime) |
        (1u << M_xmouse) |
        (1u << M_ymouse)                                                                 
        );
    // MA Verified: _lockroot does not exist/carry over from buttons, so don't include it.
    // If a movie is loaded into button, local _lockroot state is lost.
}

bool AvmButton::GetStandardMember(StandardMember member, Value* pval, bool opcodeFlag) const
{
    if (AvmCharacter::GetStandardMember (member, pval, opcodeFlag))
        return true;

    // Handle MovieClip specific "standard" members.
    switch(member)
    {
    case M_scale9Grid:
        if (GetASEnvironment()->GetVersion() >= 8)
        {
            if (GetButton()->HasScale9Grid())
            {
                Environment* penv = const_cast<Environment*>(GetASEnvironment());
                const RectF s9g = GetButton()->GetScale9Grid();

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> rectObj = *SF_HEAP_NEW(penv->GetHeap()) RectangleObject(penv);
                ASRect gr(TwipsToPixels(s9g.x1), 
                    TwipsToPixels(s9g.y1), 
                    TwipsToPixels(s9g.x2), 
                    TwipsToPixels(s9g.y2)); 
                rectObj->SetProperties(penv, gr);
#else
                Ptr<Object> rectObj = *SF_HEAP_NEW(penv->GetHeap()) Object(penv);
                ASStringContext *psc = penv->GetSC();
                rectObj->SetConstMemberRaw(psc, "x", TwipsToPixels(s9g.x1));
                rectObj->SetConstMemberRaw(psc, "y", TwipsToPixels(s9g.y1));
                rectObj->SetConstMemberRaw(psc, "width", TwipsToPixels(s9g.x2));
                rectObj->SetConstMemberRaw(psc, "height", TwipsToPixels(s9g.y2));
#endif

                pval->SetAsObject(rectObj);
            }
            else
            {
                pval->SetUndefined();
            }
            return true;
        }
        break;

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(GetButton()->IsHitTestDisableFlagSet());
            return 1;
        }
        break;

    default:
        break;
    }
    return false;
}

bool AvmButton::SetStandardMember(StandardMember member, const Value& origVal, bool opcodeFlag)
{   
    Value val(origVal);
    Environment* penv = GetASEnvironment();
    if (member > M_BuiltInProperty_End)
    {
        // Check, if there are watch points set. Invoke, if any.
        if (penv && GetButtonASObject() && ASButtonObj->HasWatchpoint()) // have set any watchpoints?
        {
            Value newVal;
            if (ASButtonObj->InvokeWatchpoint(penv, 
                penv->CreateConstString(AvmCharacter::MemberTable[member].pName), val, &newVal))
            {
                val = newVal;
            }
        }
    }
    if (AvmCharacter::SetStandardMember (member, val, opcodeFlag))
        return true;

    // Handle MovieClip specific "standard" members.
    switch(member)
    {   
    case M_scale9Grid:
        if (GetASEnvironment()->GetVersion() >= 8)
        {
            Environment* penvGrid = GetASEnvironment();
            Object* pobj = val.ToObject(penvGrid);

#ifdef GFX_AS2_ENABLE_RECTANGLE
            if (pobj && pobj->GetObjectType() == Object_Rectangle)
            {
                RectangleObject* prect = (RectangleObject*)pobj;
                ASRect gr;
                prect->GetProperties(penvGrid, gr);
                RectF sg;
                sg.x1 = PixelsToTwips(float(gr.x1));
                sg.y1 = PixelsToTwips(float(gr.y1));
                sg.SetWidth(PixelsToTwips(float(gr.Width())));
                sg.SetHeight(PixelsToTwips(float(gr.Height())));
                GetButton()->SetScale9Grid(sg);
            }
#else
            if (pobj)
            {
                ASStringContext *psc = penvGrid->GetSC();
                Value params[4];
                pobj->GetConstMemberRaw(psc, "x", &params[0]);
                pobj->GetConstMemberRaw(psc, "y", &params[1]);
                pobj->GetConstMemberRaw(psc, "width", &params[2]);
                pobj->GetConstMemberRaw(psc, "height", &params[3]);
                RectF sg;
                sg.x1 = PixelsToTwips(float(params[0].ToNumber(penvGrid)));
                sg.y1 = PixelsToTwips(float(params[1].ToNumber(penvGrid)));
                sg.SetWidth(PixelsToTwips(float(params[2].ToNumber(penvGrid))));
                sg.SetHeight(PixelsToTwips(float(params[3].ToNumber(penvGrid))));
                GetButton()->SetScale9Grid(sg);
            }
#endif
            else
                GetButton()->SetScale9Grid(RectF(0));
            return true;
        }
        break;

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            GetButton()->SetHitTestDisableFlag(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;
        // No other custom properties to set for now.
    default:
        break;
    }
    return false;
}


void ButtonObject::commonInit ()
{
}

static const NameFunction GAS_ButtonFunctionTable[] = 
{
    { "getDepth",       &AvmCharacter::CharacterGetDepth },

    { 0, 0 }
};

ButtonProto::ButtonProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
    Prototype<ButtonObject>(psc, prototype, constructor)
{
    InitFunctionMembers(psc, GAS_ButtonFunctionTable);
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_useHandCursor), Value(true), 
        PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

void ButtonCtorFunction::GlobalCtor(const FnCall& fn) 
{
    SF_UNUSED(fn);
    //fn.Result->SetAsObject(Ptr<Object>(*SF_HEAP_NEW(fn.Env->GetHeap()) ButtonObject()));
}

Object* ButtonCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) ButtonObject(penv);
}

FunctionRef ButtonCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) ButtonCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ButtonProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Button, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Button), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2

