/**************************************************************************

Filename    :   AS2_AvmTextField.h
Content     :   Implementation of AS2 part of TextField character
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_AS2_AVMTEXTFIELD_H
#define INC_SF_GFX_AS2_AVMTEXTFIELD_H

#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/AS2/AS2_StyleSheet.h"
#include "GFx/GFx_TextFieldDef.h"
#include "GFx/GFx_TextField.h"

namespace Scaleform {
namespace GFx {
namespace AS2 {

class TextFieldObject;

// This class represents AS2-dependent part of GFx::TextField 
// character class.
class AvmTextField : public AvmCharacter, public AvmTextFieldBase
{
#ifdef GFX_ENABLE_CSS
    struct CSSHolder : public TextField::CSSHolderBase
    {
        Ptr<StyleSheetObject>   pASStyleSheet; // an AS object, pASStyleSheet->CSS is the data

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
#endif //SF_NO_CSS_SUPPORT
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    void ProceedImageSubstitution(const FnCall& fn, int idx, const Value* pve);
#endif

public:
    AvmTextField(TextField* pbutton);

    //*** AvmDisplayObjBase
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmCharacter*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return this; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }
    virtual bool                        OnEvent(const EventId& id) 
    { 
        return AvmCharacter::OnEvent(id); 
    }
    virtual void                        OnEventLoad();
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
        return false; //? AvmCharacter::HasEventHandler(id);
    }

    //*** AvmInteractiveObjBase implementation
    virtual bool            ActsAsButton() const { return false; }
    virtual void            CloneInternalData(const InteractiveObject* src)
    {
        AvmCharacter::CloneInternalData(src);
    }
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmCharacter::CopyPhysicalProperties(poldChar);
    }
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmCharacter::GetAbsolutePath(ppath);
    }
    virtual unsigned        GetCursorType() const { return 0; }

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
            return !GetTextField()->IsReadOnly() || GetTextField()->IsSelectable(); 
        return true; 
    }
    virtual bool            IsTabable() const;

    // *** AvmTextFieldBase implementation
    virtual bool            HasStyleSheet() const;
    virtual void            NotifyChanged();
    virtual bool            OnMouseEvent(const EventId& event);
    // returns false, if default action should be prevented
    virtual bool            OnCharEvent(wchar_t wcharCode, unsigned controllerIdx);
    virtual void            OnScroll();
    virtual bool            OnEditorInsertingText(UPInt, UPInt, const wchar_t*, unsigned  = 0) { return true; }
    virtual bool            OnEditorRemovingText(UPInt, UPInt, unsigned  = 0) { return true; }
    virtual bool            UpdateTextFromVariable() ;
    virtual void            UpdateVariable();

    // *** AvmCharacter / GASObjectInterface implementation
    virtual Object*         GetASObject ();
    virtual Object*         GetASObject () const; 
    virtual bool            GetMember(Environment* penv, const ASString& name, Value* pval);
    virtual ObjectType      GetObjectType() const { return Object_TextField; }
    virtual UInt32          GetStandardMemberBitMask() const;
    // For ActionScript toString.
    virtual const char*     GetTextValue(Environment* =0) const
    {       
        return "";//NULL;//pDocument->GetText()
    }
    virtual bool            SetMember(Environment* penv, const ASString& name, const Value& origVal, const PropFlags& flags);
    //virtual bool            SetStandardMember(StandardMember member, const Value& val, bool opcodeFlag);

    // *** domestic
    const Text::StyleManager* GetStyleSheet() const;

    SF_INLINE TextField*      GetTextField() const
    {
        SF_ASSERT(pDispObj); 
        SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
        return static_cast<TextField*>(pDispObj);
    }
    TextFieldDef*           GetTextFieldDef() const;
    TextFieldObject*        GetTextFieldASObject();  
    void                    SetDirtyFlag() { GetTextField()->SetDirtyFlag(); }
    void                    UpdateAutosizeSettings();

public:

    static void SetTextFormat(const FnCall& fn);
    static void SetNewTextFormat(const FnCall& fn);
    static void GetTextFormat(const FnCall& fn);
    static void GetNewTextFormat(const FnCall& fn);
    static void ReplaceSel(const FnCall& fn);
    static void ReplaceText(const FnCall& fn);
    static void RemoveTextField(const FnCall& fn);

#ifndef SF_NO_TEXTFIELD_EXTENSIONS
    // extensions
    static void GetCharBoundaries(const FnCall& fn);
    static void GetCharIndexAtPoint(const FnCall& fn);
    static void GetExactCharBoundaries(const FnCall& fn);
    static void GetLineIndexAtPoint(const FnCall& fn);
    static void GetLineIndexOfChar(const FnCall& fn);
    static void GetLineOffset(const FnCall& fn);
    static void GetLineLength(const FnCall& fn);
    static void GetLineMetrics(const FnCall& fn);
    static void GetLineText(const FnCall& fn);
    static void GetFirstCharInParagraph(const FnCall& fn);
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    static void SetImageSubstitutions(const FnCall& fn);
    static void UpdateImageSubstitution(const FnCall& fn);
#endif // SF_NO_FXPLAYER_AS_BITMAPDATA
    static void AppendText(const FnCall& fn);
    static void AppendHtml(const FnCall& fn);
    static void CopyToClipboard(const FnCall& fn);
    static void CutToClipboard(const FnCall& fn);
    static void PasteFromClipboard(const FnCall& fn);
#endif //SF_NO_TEXTFIELD_EXTENSIONS

    static void BroadcastMessage(const FnCall& fn);

protected:
    ASString                VariableName;
    Value                   VariableVal;
    Ptr<TextFieldObject>    ASTextFieldObj;
};

SF_INLINE AvmTextField* ToAvmTextField(InteractiveObject* po)
{
    SF_ASSERT(po->IsAVM1() && po->GetType() == CharacterDef::TextField);
    return static_cast<AvmTextField*>(static_cast<AvmCharacter*>(po->GetAvmObjImpl()));
}

// ActionScript objects.

class TextFieldObject : public Object
{
    friend class TextFieldProto;
    friend class TextFieldCtorFunction;

    WeakPtr<InteractiveObject> pTextField;    // weak ref on textfield obj
#ifdef GFX_AS_ENABLE_GC
protected:
    void Finalize_GC()
    {
        pTextField.~WeakPtr<InteractiveObject>();
        delete pIMECompositionStringStyles;
        Object::Finalize_GC();
    }
#endif // SF_NO_GC
protected:

    Text::IMEStyle* pIMECompositionStringStyles;

    TextFieldObject(ASStringContext *psc, Object* pprototype);
    TextFieldObject(Environment* penv);
public:
    TextFieldObject(GlobalContext* gCtxt, InteractiveObject* ptextfield);
    ~TextFieldObject();

    virtual ObjectType      GetObjectType() const   { return Object_TextFieldASObject; }

    virtual InteractiveObject*       GetASCharacter() { return Ptr<InteractiveObject>(pTextField); }
    virtual const InteractiveObject* GetASCharacter() const { return Ptr<InteractiveObject>(pTextField); }

    Text::IMEStyle*         GetIMECompositionStringStyles();
    void                    SetIMECompositionStringStyles(const Text::IMEStyle& imeStyles);
};

class TextFieldProto : public Prototype<TextFieldObject>
{
    static Text::HighlightInfo ParseStyle(const FnCall& fn, unsigned paramIndex, const Text::HighlightInfo& initialHInfo);
    static void                 MakeStyle(const FnCall& fn, const Text::HighlightInfo& hinfo);
public:
    TextFieldProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    static void SetIMECompositionStringStyle(const FnCall& fn);
    static void GetIMECompositionStringStyle(const FnCall& fn);
};

class TextFieldCtorFunction : public CFunctionObject
{
public:
    TextFieldCtorFunction(ASStringContext *);

    virtual Object* CreateNewObject(Environment* penv) const;

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);

    static void GetFontList(const FnCall& fn);
};


}}} // SF::GFx::AS2

#endif //INC_SF_GFX_AS2_AVMTEXTFIELD_H
