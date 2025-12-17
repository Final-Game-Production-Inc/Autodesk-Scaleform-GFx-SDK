/**************************************************************************

Filename    :   AS3_AvmTextField.h
Content     :   Implementation of AS3 part of TextField character
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_AS3_AvmTextField_H
#define INC_SF_GFx_AS3_AvmTextField_H

#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "GFx/GFx_TextFieldDef.h"
#include "GFx/GFx_TextField.h"
#include "GFx/Text/Text_StyleSheet.h"
#include "Obj/Text/AS3_Obj_Text_StyleSheet.h"

class TextFieldObject;

namespace Scaleform {
namespace GFx {

namespace Text { using namespace Render::Text; }

namespace AS3 {

class VM;
class Value;

// This class represents AS2-dependent part of GFx::TextField 
// character class.
class AvmTextField : public AvmInteractiveObj, public AvmTextFieldBase
{
#ifdef GFX_ENABLE_CSS
public:
    struct CSSHolder : public TextField::CSSHolderBase
    {
        SPtr<Instances::fl_text::StyleSheet>   pASStyleSheet; // an AS object, pASStyleSheet->CSS is the data

        virtual bool HasASStyleSheet() const
        {
            return pASStyleSheet.GetPtr() != NULL;
        }
        virtual const Text::StyleManager* GetTextStyleManager() const
        {
            if (pASStyleSheet)
                return pASStyleSheet->GetTextStyleManager();
            return NULL;
        }
    };
    CSSHolder* GetCSSData() const 
    {
        return static_cast<CSSHolder*>(GetTextField()->GetCSSData());
    }
#endif //GFX_ENABLE_CSS
public:
    AvmTextField(TextField* pbutton);

    //*** AvmDisplayObjBase
    virtual const char* GetDefaultASClassName() const { return "flash.text.TextField"; }
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmInteractiveObj*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return this; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }
    virtual bool                        OnEvent(const EventId& id)
    {
        return AvmDisplayObj::OnEvent(id);
    }
    virtual void                        OnEventLoad();
    virtual void                        OnEventUnload()
    {
        AvmInteractiveObj::OnEventUnload();
    }
//     virtual void                        OnRemoved()
//     {
//         AvmInteractiveObj::OnRemoved();
//     }
    virtual bool                        OnUnloading(bool)
    {
        return true;
    }
    virtual bool                        HasEventHandler(const EventId& id) const
    {
        return AvmDisplayObj::HasEventHandler(id);
    }

    //*** AvmInteractiveObjBase implementation
    virtual bool            ActsAsButton() const { return false; }
    virtual void            CloneInternalData(const InteractiveObject* src)
    {
        AvmInteractiveObj::CloneInternalData(src);
    }
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmInteractiveObj::CopyPhysicalProperties(poldChar);
    }
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmInteractiveObj::GetAbsolutePath(ppath);
    }
    virtual unsigned        GetCursorType() const { return 0; }

    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned /*controllerIdx*/, FocusMovedType) {}
    // Special event handler; key down and up
    // See also PropagateKeyEvent, KeyMask
    virtual bool            OnKeyEvent(const EventId& e, int*) 
    {
        if (HasAS3Obj())
            GetAS3Obj()->Dispatch(e, GetTextField());
        return false; 
    }
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject*, unsigned , FocusMovedType = GFx_FocusMovedByKeyboard) { return true; }

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool            IsFocusEnabled(FocusMovedType fmt = GFx_FocusMovedByKeyboard) const  
    { 
        // Flash allows to set focus on read-only non-selectable text fields.
        if (fmt == GFx_FocusMovedByKeyboard) 
            return !GetTextField()->IsReadOnly();
        return true; // AS and Mouse may focus dynamic text field always.
    }
    virtual bool            IsTabable() const;

    // *** AvmTextFieldBase implementation
    virtual bool            HasStyleSheet() const;
    virtual void            NotifyChanged();
    virtual void            OnLinkEventEx(TextField::LinkEvent event, unsigned pos, unsigned controllerIndex);
    virtual bool            OnMouseEvent(const EventId& event);
    // returns false, if default action should be prevented
    virtual bool            OnCharEvent(wchar_t wcharCode, unsigned controllerIdx);
    virtual void            OnScroll();
    virtual bool            OnEditorInsertingText(UPInt pos, UPInt len, const wchar_t* wstr, unsigned controllerIdx = 0);
    virtual bool            OnEditorRemovingText(UPInt pos, UPInt len, unsigned controllerIdx = 0);
    virtual bool            UpdateTextFromVariable() { return false; }
    virtual void            UpdateVariable() {}

    // *** domestic
    const Text::StyleManager* GetStyleSheet() const;

    SF_INLINE TextField*      GetTextField() const
    {
        SF_ASSERT(pDispObj); 
        SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
        return static_cast<TextField*>(pDispObj);
    }
    TextFieldDef*           GetTextFieldDef() const;
    void                    SetDirtyFlag() { GetTextField()->SetDirtyFlag(); }
    void                    UpdateAutosizeSettings();

    void                    ProceedImageSubstitution(VM& vm, int idx, const Value& ve);
#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::TextField"; }
#endif
};

SF_INLINE AvmTextField* ToAvmTextField(InteractiveObject* po)
{
    SF_ASSERT(po->IsAVM2() && po->GetType() == CharacterDef::TextField);
    return static_cast<AvmTextField*>(static_cast<AvmInteractiveObj*>(po->GetAvmIntObj()));
}


}}} // SF::GFx::AS3

#endif //INC_SF_GFx_AS3_AvmTextField_H
