/**************************************************************************

Filename    :   AS2_AvmButton.h
Content     :   Implementation of AS2-dependent part of Button character 
                and its AS2 Button class.
Created     :   Dec, 2009
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_AS2_AVMBUTTON_H
#define INC_SF_GFX_AS2_AVMBUTTON_H

#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_Tags.h"
#include "GFx/GFx_ButtonDef.h"
#include "GFx/GFx_Button.h"
#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { 

// ***** External Classes
class MouseState;
class ButtonRecord;
class ButtonDef;
class GFxSoundSampleImpl;
class LoadProcess;

namespace AS2 {

// ***** Declared Classes
class ButtonAction;
class ButtonObject;
class ButtonProto;

// This class represents AS2-dependent part of GFx::Button 
// character class.
class AvmButton : public AvmCharacter, public AvmButtonBase
{
    Ptr<ButtonObject> ASButtonObj;
public:
    AvmButton(Button* pbutton);

    //*** AvmDisplayObjBase
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmCharacter*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return this; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }
    virtual bool                        OnEvent(const EventId& id) 
    { 
        return AvmCharacter::OnEvent(id); 
    }
    virtual void                        OnEventLoad()
    {
        AvmCharacter::OnEventLoad();
    }
    virtual void                        OnEventUnload()
    {
        AvmCharacter::OnEventUnload();
    }
    //virtual void                        OnRemoved() {}
    virtual bool                        OnUnloading(bool)
    {
        pDispObj->RemoveFromPlayList();
        return true;
    }
    virtual bool                        HasEventHandler(const EventId& ) const
    {
        return false; //?
    }

    //*** AvmInteractiveObjBase
    virtual bool            ActsAsButton() const { return true; }
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmCharacter::GetAbsolutePath(ppath);
    }
    virtual unsigned        GetCursorType() const;
    virtual void            CloneInternalData(const InteractiveObject* src)
    {
        AvmCharacter::CloneInternalData(src);
    }
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmCharacter::CopyPhysicalProperties(poldChar);
    }
    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmCharacter::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned /*controllerIdx*/, FocusMovedType) {}
    // Special event handler; key down and up
    // See also PropagateKeyEvent, KeyMask
    virtual bool            OnKeyEvent(const EventId&, int*) { return false; }
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject*, unsigned , FocusMovedType = GFx_FocusMovedByKeyboard) { return true; }

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool            IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const  
    { 
        if (fmt == GFx_FocusMovedByMouse)
            return AvmCharacter::IsFocusEnabled(fmt); 
        else
            return true;
    }
    virtual bool            IsTabable() const       { return AvmCharacter::IsTabable(); }

    //*** AvmButtonBase
    virtual void            CreateCharacters();
    virtual bool            OnMouseEvent(const EventId& event);
    virtual bool            OnButtonKeyEvent(const EventId& id, int* pkeyMask); 
    virtual void            SwitchState(ButtonRecord::MouseState);

    //*** AvmCharacter
    virtual Object*         GetASObject();
    virtual Object*         GetASObject() const;

    virtual UInt32          GetStandardMemberBitMask() const;
    virtual bool            GetMember(Environment* penv, const ASString& name, Value* pval);
    virtual bool            GetStandardMember(StandardMember member, Value* pval, bool opcodeFlag) const;
    virtual bool            SetStandardMember(StandardMember member, const Value& origVal, bool opcodeFlag);
    virtual ObjectType      GetObjectType() const
    {
        return Object_Button;
    }

private:
    ButtonObject*           GetButtonASObject();
    ButtonDef*              GetButtonDef() const;
    Button*                 GetButton() const { return static_cast<Button*>(pDispObj); }

    void                    RecreateCharacters(ButtonRecord::MouseState);
    void                    ConstructCharacter(InteractiveObject*, const ButtonRecord& rec);

    Button::CharToRec       FindCharacterAndRemove(const ButtonRecord& rec);
};


// ActionScript objects. Should be separated later.

class ButtonObject : public Object
{
    friend class ButtonProto;
    friend class ButtonCtorFunction;

    WeakPtr<InteractiveObject> pButton;   // weak ref on button obj

    void commonInit ();
#ifdef GFX_AS_ENABLE_GC
protected:
    void Finalize_GC()
    {
        pButton.~WeakPtr<InteractiveObject>();
        Object::Finalize_GC();
    }
#endif // SF_NO_GC
protected:

    ButtonObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
        commonInit();
    }
public:
    ButtonObject(GlobalContext* gCtxt, InteractiveObject* pbutton) 
        : Object(gCtxt->GetGC()), pButton(pbutton)
    {
        commonInit ();
        GFx::AS2::AvmCharacter* avmCh = GFx::AS2::ToAvmCharacter(pbutton);
        ASStringContext* psc = avmCh->GetASEnvironment()->GetSC();
        //Set__proto__ (psc, gCtxt->GetPrototype(ASBuiltin_Button));
        Set__proto__ (psc, avmCh->Get__proto__());
    }
    ButtonObject(Environment *penv) : Object(penv)
    { 
        commonInit ();
        Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_Button));
    }

    virtual ObjectType      GetObjectType() const   { return Object_ButtonASObject; }

    virtual InteractiveObject* GetASCharacter() { return Ptr<InteractiveObject>(pButton).GetPtr(); }
    virtual const InteractiveObject* GetASCharacter() const { return Ptr<InteractiveObject>(pButton).GetPtr(); }
};

class ButtonProto : public Prototype<ButtonObject>
{
public:
    ButtonProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);
};

class ButtonCtorFunction : public CFunctionObject
{
public:
    ButtonCtorFunction(ASStringContext *psc) : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_AS2_AVMBUTTON_H

