/**************************************************************************

Filename    :   AS2_AvmTextField.cpp
Content     :   Implementation of AS2 part of TextField character
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS2/AS2_AvmTextField.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_TextFormat.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "GFx/AS2/AS2_PointObject.h"
#include "GFx/AS2/AS2_BitmapData.h"
#include "GFx/AS2/AS2_BitmapFilter.h"
#include "GFx/Text/Text_StyleSheet.h"

namespace Scaleform {
namespace GFx {
namespace AS2 {

AvmTextField::AvmTextField(TextField* ptf) 
:
    AvmCharacter(ptf),
    VariableName(ptf->GetStringManager()->CreateString(ptf->GetTextFieldDef()->VariableName))
{
    if (!VariableName.IsEmpty())
        ptf->SetForceAdvance();
//    MovieImpl* proot = ptf->GetMovieImpl();
    // initialize built-in members by default values
    //ASTextFieldObj = *SF_HEAP_NEW(proot->GetMovieHeap()) TextFieldObject(GetGC(), this);
    // let the base class know what's going on
    //pProto = ASTextFieldObj->Get__proto__();
    pProto = GetGC()->GetActualPrototype(GetASEnvironment(), ASBuiltin_TextField);
    //pProto = GetGC()->GetPrototype(ASBuiltin_TextField);

    // add itself as a listener, the Flash behavior
    AsBroadcaster::InitializeInstance(ToAvmTextField(ptf)->GetASEnvironment()->GetSC(), this);
    AsBroadcaster::AddListener(ToAvmTextField(ptf)->GetASEnvironment(), this, this);

}

Object*    AvmTextField::GetASObject () 
{ 
    return GetTextFieldASObject(); 
}

Object*    AvmTextField::GetASObject () const 
{ 
    return const_cast<AvmTextField*>(this)->GetTextFieldASObject(); 
}

TextFieldDef* AvmTextField::GetTextFieldDef() const
{
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    return static_cast<TextField*>(pDispObj)->GetTextFieldDef();
}

TextFieldObject* AvmTextField::GetTextFieldASObject()          
{ 
    if (!ASTextFieldObj)
    {
        ASTextFieldObj = *SF_HEAP_AUTO_NEW(this) TextFieldObject(GetGC(), pDispObj);
    }
    return ASTextFieldObj;
}

bool AvmTextField::HasStyleSheet() const 
{ 
#ifdef GFX_ENABLE_CSS
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    const TextField::CSSHolderBase* pcss = GetTextField()->GetCSSData();
    return pcss && static_cast<const CSSHolder*>(pcss)->pASStyleSheet.GetPtr() != NULL; 
#else
    return false;
#endif
}

const Text::StyleManager* AvmTextField::GetStyleSheet() const 
{ 
#ifdef GFX_ENABLE_CSS
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    const TextField::CSSHolderBase* pcss = GetTextField()->GetCSSData();
    return (HasStyleSheet()) ? static_cast<const CSSHolder*>(pcss)->pASStyleSheet->GetTextStyleManager() : NULL; 
#else
    return NULL;
#endif
}

bool AvmTextField::OnMouseEvent(const EventId& event)
{ 
    if (event.Id == EventId::Event_Release)
    {
        TextField* ptf       = GetTextField();
        // if url is clicked - execute the action
        if (ptf->IsHtml() && ptf->MayHaveUrl())
        {
            Render::PointF p;
            if (ptf->IsUrlUnderMouseCursor(event.MouseIndex, &p))
            {
                UPInt pos = ptf->GetCharIndexAtPoint(p.x, p.y);
                if (!Alg::IsMax(pos))
                {
                    const Text::TextFormat* ptextFmt;
                    if (ptf->GetTextAndParagraphFormat(&ptextFmt, NULL, pos))
                    {
                        if (ptextFmt->IsUrlSet())
                        {
                            const char* url = ptextFmt->GetUrl();

                            // url should represent link in asfunction protocol:
                            // "asfunction:funcName, parametersString"
                            // NOTE: parametersString is not parsed.
                            MovieImpl* proot = GetMovieImpl();
                            if (proot && String::CompareNoCase(url, "asfunction:", sizeof("asfunction:") - 1) == 0)
                            {
                                url += sizeof("asfunction:") - 1;
                                // now get the function's name
                                const char* pnameEnd = strchr(url, ',');
                                String   funcName;
                                GFx::Value param;
                                unsigned paramCnt = 0;
                                if (pnameEnd)
                                {
                                    funcName.AppendString(url, pnameEnd - url);
                                    ++pnameEnd;
                                    paramCnt = 1;
                                    param.SetString(pnameEnd);
                                }
                                else
                                    funcName = url;
                                GFx::Value result;
                                Ptr<InteractiveObject> par = GetParent();
                                Sprite* parSpr = NULL;
                                if (par)
                                {
                                    parSpr = par->CharToSprite();
                                }
                                if (parSpr)
                                    ToAS2Root(proot)->Invoke(parSpr, funcName, &result, &param, paramCnt);
                                else
                                    ToAS2Root(proot)->Invoke(funcName, &result, &param, paramCnt);
                            }
                            //printf ("%s\n", url);
                        }
                    }
                }
            }
        }

        return true;
    }
    return false; 
}

bool AvmTextField::OnCharEvent(wchar_t wcharCode, unsigned controllerIdx)
{
    SF_UNUSED2(wcharCode, controllerIdx);
    SF_ASSERT(wcharCode);
    return true;
//     // cursor positioning is available for selectable readonly textfields
//     MovieRoot* proot = GetAS2Root();
//     if (proot && HasEditorKit() && (!IsReadOnly() || IsSelectable()))
//     {
//         // need to queue up key processing. Flash doesn't handle 
//         // keys from keyevents, it executes them afterward.
//         ValueArray params;
//         params.PushBack(Value(int(2)));
//         params.PushBack(Value(int(wcharCode)));
//         proot->InsertEmptyAction()->SetAction(this, 
//             AvmTextField::KeyProcessing, &params);
//     }
}

void AvmTextField::OnEventLoad()
{
    // finalize the initialization. We need to initialize text here rather than in ctor
    // since the name of instance is not set yet in the ctor and setting text might cause
    // GFx::Translator to be invoked, which uses the instance name.
    TextField* ptf = GetTextField();

    if (GetTextFieldDef()->DefaultText.GetLength() > 0)
    {
        bool varExists = false;
        // check, does variable exist or not. If it doesn't exist, we
        // will need to call UpdateVariable in order to create this variable.
        if (!VariableName.IsEmpty())
        {
            if (!VariableName.IsEmpty())
                ptf->SetForceAdvance();
            Environment* penv = GetASEnvironment();
            if (penv)
            {
                Value val;
                varExists = penv->GetVariable(VariableName, &val);
            }
        }
        if (!varExists)
        {
            ptf->SetTextValue(GetTextFieldDef()->DefaultText.ToCStr(), ptf->IsHtml());
            UpdateVariable();
        }
    }
    else
    {
        // This assigns both Text and HtmlText vars.
        ptf->SetTextValue("", ptf->IsHtml(), false);
    }

    // update text, if variable is used
    UpdateTextFromVariable();
}


bool AvmTextField::UpdateTextFromVariable() 
{
    if (!VariableName.IsEmpty()) 
    {   
        TextField* ptf = GetTextField();
        ptf->SetForceAdvance();
        // Variable name can be a path, so use GetVariable().
        Environment* penv = GetASEnvironment();
        if (penv)
        {
            Value val;
            if (penv->GetVariable(VariableName, &val)) 
            {        
                if (!val.IsEqual(penv, VariableVal))
                {
                    VariableVal = val;
                    ASString str = val.ToString(penv);
                    ptf->SetTextValue(str.ToCStr(), false, false);
                    return true;
                }
            }
            else
            {
                // if variable name is specified but no variable found (were deleted, for example)
                // then just set empty string
                ptf->SetTextValue("", false, false);
            }
        }
        return false; // can't update the text for some reasons
    }
    return true; // do not need update anything, so, status of operation is OK.
}

// InteractiveObject override to indicate which standard members are handled for us.
UInt32  AvmTextField::GetStandardMemberBitMask() const
{       
    return  M_BitMask_PhysicalMembers |
        M_BitMask_CommonMembers |                               
        (1 << M_target) |
        (1 << M_url) |
        (1 << M_filters) |          
        (1 << M_tabEnabled) |
        (1 << M_tabIndex) |         
        (1 << M_quality) |
        (1 << M_highquality) |
        (1 << M_soundbuftime) |
        (1 << M_xmouse) |
        (1 << M_ymouse)
        ;
    // MA Verified: _lockroot does not exist/carry over from text fields, so don't include it.
    // If a movie is loaded into TextField, local _lockroot state is lost.
}

void AvmTextField::UpdateAutosizeSettings()
{
    TextField* ptf          = GetTextField();
    const bool  autoSizeByX = (ptf->IsAutoSize() && !ptf->IsWordWrap());
    const bool  autoSizeByY = (ptf->IsAutoSize());
    if (autoSizeByX)
        ptf->SetAutoSizeX();
    else
        ptf->ClearAutoSizeX();
    if (autoSizeByY)
        ptf->SetAutoSizeY();
    else
        ptf->ClearAutoSizeY();
    ptf->SetNeedUpdateGeomData();
}

bool AvmTextField::SetMember(Environment* penv, const ASString& name, const Value& origVal, const PropFlags& flags)
{
    StandardMember member = GetStandardMemberConstant(name);

    // Check, if there are watch points set. Invoke, if any. Do not invoke
    // watch for built-in properties.
    Value val(origVal);
    if (member < M_BuiltInProperty_Begin || member > M_BuiltInProperty_End)
    {
        if (penv && GetTextFieldASObject() && ASTextFieldObj->HasWatchpoint()) // have set any watchpoints?
        {
            Value        newVal;
            if (ASTextFieldObj->InvokeWatchpoint(penv, name, val, &newVal))
            {
                val = newVal;
            }
        }
    }

    if (SetStandardMember(member, origVal, false))
        return true;

    TextField* ptf          = GetTextField();
    SF_ASSERT(ptf);
    // Handle TextField-specific "standard" members.
    switch(member)
    {
    case M_html:
        ptf->SetHtml(val.ToBool(penv));
        return true; 

    case M_htmlText:
        {
            ASString str(val.ToStringVersioned(penv, GetVersion()));
            ptf->SetTextValue(str.ToCStr(), ptf->IsHtml());
            return true;
        }

    case M_text:
        {
            ptf->ResetBlink(1);
            ASString str(val.ToStringVersioned(penv, GetVersion()));
            ptf->SetTextValue(str.ToCStr(), false);
            return true;
        }

    case M_textColor:
        {
            // The arg is 0xRRGGBB format.
            UInt32  rgb = val.ToUInt32(penv);
            ptf->SetTextColor(rgb);
            return true;
        }

    case M_autoSize:
        {
            ASString asStr(val.ToString(penv));
            String str = (val.IsBoolean()) ? 
                ((val.ToBool(penv)) ? "left":"none") : asStr.ToCStr();

            Text::DocView::ViewAlignment oldAlignment = ptf->GetAlignment();
            bool oldAutoSize = ptf->IsAutoSize();
            if (str == "none")
            {
                ptf->ClearAutoSize();
                ptf->SetAlignment(Text::DocView::Align_Left);
            }
            else 
            {
                ptf->SetAutoSize();
                if (str == "left")
                    ptf->SetAlignment(Text::DocView::Align_Left);
                else if (str == "right")
                    ptf->SetAlignment(Text::DocView::Align_Right);
                else if (str == "center")
                    ptf->SetAlignment(Text::DocView::Align_Center);
            }
            if ((oldAlignment != ptf->GetAlignment()) || (oldAutoSize != ptf->IsAutoSize()))
                UpdateAutosizeSettings();
            SetDirtyFlag();
            return true;
        }

    case M_wordWrap:
        {
            bool oldWordWrap = ptf->IsWordWrap();
            bool wordWrap = val.ToBool(penv);
            if (wordWrap != oldWordWrap)
            {
                if (wordWrap)
                    ptf->SetWordWrap();
                else
                    ptf->ClearWordWrap();
                UpdateAutosizeSettings();
            }
            SetDirtyFlag();
            return true;
        }

    case M_multiline:
        {
            bool oldML = ptf->IsMultiline();
            bool newMultiline = val.ToBool(penv);
            if (oldML != newMultiline)
            {
                if (newMultiline)
                    ptf->SetMultiline();
                else
                    ptf->ClearMultiline();
                UpdateAutosizeSettings();
            }
            SetDirtyFlag();
            return true;
        }

    case M_variable:
        {
            //if (val.IsNull() || val.IsUndefined())
            //    VariableName = "";
            //!AB: even if we set variable name pointing to invalid or non-exisiting variable
            // we need to save it and try to update text using it. If we can't get a value 
            // (variable doesn't exist), then text shouldn't be changed.
            {                        
                VariableName = val.ToString(penv);
                UpdateTextFromVariable();
                ptf->SetForceAdvance();
                ptf->ModifyOptimizedPlayListLocal<TextField>();
            }
            return true;
        }

    case M_selectable:
        {
            ptf->SetSelectable(val.ToBool(penv));
            return true;
        }

    case M_embedFonts:
        {
            if (val.ToBool(penv))
                ptf->ClearUseDeviceFont();
            else
                ptf->SetUseDeviceFont();
            ptf->ForceCompleteReformat();
            SetDirtyFlag();
            return true;
        }

    case M_password:
        {
            bool pswd = ptf->IsPassword();
            bool password = val.ToBool(penv);
            if (pswd != password)
            {
                ptf->SetPassword(password);
                if (password)
                    ptf->SetPasswordMode();
                else
                    ptf->ClearPasswordMode();
                ptf->ForceCompleteReformat();
            }
            SetDirtyFlag();
            return true;
        }

    case M_hscroll:
        {
            int scrollVal = (int)val.ToInt32(penv);
            if (scrollVal < 0)
                scrollVal = 0;
            ptf->SetHScrollOffset((Double)scrollVal);    
            SetDirtyFlag();
            return true;
        }

    case M_scroll:
        {
            int scrollVal = (int)val.ToInt32(penv);
            if (scrollVal < 1)
                scrollVal = 1;
            ptf->SetVScrollOffset((unsigned)(scrollVal - 1));    
            SetDirtyFlag();
            return true;
        }
    case M_border:
        {
            ptf->SetBorder(val.ToBool(penv));
            return true;
        }
    case M_background:
        {
            ptf->SetBackground(val.ToBool(penv));
            return true;
        }
    case M_backgroundColor:
        {
            Number bc = val.ToNumber(penv);
            if (!NumberUtil::IsNaN(bc))
            {
                UInt32 c = val.ToUInt32(penv);
                ptf->SetBackgroundColor(c);
            }
            return true;
        }
    case M_borderColor:
        {
            Number bc = val.ToNumber(penv);
            if (!NumberUtil::IsNaN(bc))
            {
                UInt32 c = val.ToUInt32(penv);
                ptf->SetBorderColor(c);
            }
            return true;
        }
    case M_type:
        {
            ASString typeStr = val.ToString(penv);
            if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "dynamic"))
            {
                if (ptf->HasEditorKit())
                    ptf->GetEditorKit()->SetReadOnly();
            }
            else if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "input") && !HasStyleSheet())
            {
                Text::EditorKit* peditor = ptf->CreateEditorKit();
                peditor->ClearReadOnly();
            }
            // need to reformat because scrolling might be changed
            ptf->ForceReformat();
            return true;
        }
    case M_maxChars:
        {
            Number ml = val.ToNumber(penv);
            if (!NumberUtil::IsNaN(ml) && ml >= 0)
            {
                ptf->SetMaxLength((UPInt)val.ToUInt32(penv));
            }
            return true;
        }
    case M_condenseWhite:
        {
            ptf->SetCondenseWhite(val.ToBool(penv));
            return true;
        }
    case M_antiAliasType:
        {
            ASString typeStr = val.ToString(penv);
            if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "normal"))
            {
                ptf->ClearAAForReadability();
            }
            else if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "advanced"))
            {
                ptf->SetAAForReadability();                    
            }
            SetDirtyFlag();
            return true;
        }

    case M_mouseWheelEnabled:
        {
            ptf->SetMouseWheelEnabled(val.ToBool(penv));
            return true;
        }
    case M_styleSheet:
        {
#ifdef GFX_ENABLE_CSS
            Object* pobj = val.ToObject(penv);
            if (pobj && pobj->GetObjectType() == Object::Object_StyleSheet)
            {
                if (!ptf->GetCSSData())
                    ptf->SetCSSData(new CSSHolder);
                GetCSSData()->pASStyleSheet = static_cast<StyleSheetObject*>(pobj);

                // make the text field read-only
                if (ptf->HasEditorKit())
                {
                    ptf->GetEditorKit()->SetReadOnly();
                }

                SetDirtyFlag();

                if (GetCSSData()->pASStyleSheet->CSS.IsLoading())
                {
                    // if style sheet is in Loading state - force the TextField::AdvanceFrame call
                    // until loading is finished.
                    ptf->ModifyOptimizedPlayListLocal<TextField>();
                }
            }
            else if (GetCSSData())
                GetCSSData()->pASStyleSheet = NULL;
            ptf->CollectUrlZones();
            ptf->UpdateUrlStyles();
            ptf->SetNeedUpdateLayoutFlag();
#endif //SF_NO_CSS_SUPPORT
            return true;
        }

        // extension
    case M_shadowStyle:
        if (!penv->CheckExtensions())
            break;

        {
            ASString   styleStr = val.ToString(penv);
            const char *pstr = styleStr.ToCStr();

            ptf->SetShadowStyle(pstr);
            return true;
        }

    case M_shadowColor:
        if (!penv->CheckExtensions())
            break;
        {
            ptf->SetShadowColor(val.ToUInt32(penv));
            return true;
        }

    case M_hitTestDisable:
        if (penv->CheckExtensions())
        {
            ptf->SetHitTestDisableFlag(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;

    case M_noTranslate:
        if (penv->CheckExtensions())
        {
            ptf->SetNoTranslate(val.ToBool(GetASEnvironment()));
            ptf->ForceCompleteReformat(); 
            return 1;
        }
        break;

    case M_autoFit:
        if (penv->CheckExtensions())
        {
            if (val.ToBool(penv))
                ptf->SetAutoFit();
            else
                ptf->ClearAutoFit();
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurX:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurX((float)PixelsToTwips(val.ToNumber(penv)));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurY:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurY((float)PixelsToTwips(val.ToNumber(penv)));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurStrength:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurStrength((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_outline:
        if (penv->CheckExtensions())
        {
            ptf->SetOutline((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_fauxBold:
        if (penv->CheckExtensions())
        {
            ptf->SetFauxBold(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_fauxItalic:
        if (penv->CheckExtensions())
        {
            ptf->SetFauxItalic(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowAlpha:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowAlpha((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowAngle:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowAngle(((float)val.ToNumber(penv)) * (3.1415926f/180.f));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowBlurX:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowBlurX((float)PixelsToTwips(val.ToNumber(penv)));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowBlurY:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowBlurY((float)PixelsToTwips(val.ToNumber(penv)));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowDistance:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowDistance(PixelsToTwips((float)val.ToNumber(penv)));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowHideObject:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowHideObject(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowKnockOut:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowKnockOut(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowQuality:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowQuality((unsigned)val.ToUInt32(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowStrength:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowStrength((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowOutline:
        // Not implemented
        //if (GetASEnvironment()->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_verticalAutoSize:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
            {
                ptf->ClearAutoSizeY();
                ptf->SetVAlignment(Text::DocView::VAlign_None);
            }
            else 
            {
                ptf->SetAutoSizeY();
                if (str == "top")
                    ptf->SetVAlignment(Text::DocView::VAlign_Top);
                else if (str == "bottom")
                    ptf->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (str == "center")
                    ptf->SetVAlignment(Text::DocView::VAlign_Center);
            }
            ptf->SetNeedUpdateGeomData();
            SetDirtyFlag();
            return true;
        }
        break;

    case M_fontScaleFactor:
        if (penv->CheckExtensions())
        {
            Number factor = val.ToNumber(penv);
            if (factor > 0 && factor < 1000)
                ptf->SetFontScaleFactor((float)factor);
            SetDirtyFlag();
        }
        break;

    case M_verticalAlign:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
                ptf->SetVAlignment(Text::DocView::VAlign_None);
            else 
            {
                if (str == "top")
                    ptf->SetVAlignment(Text::DocView::VAlign_Top);
                else if (str == "bottom")
                    ptf->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (str == "center")
                    ptf->SetVAlignment(Text::DocView::VAlign_Center);
            }
            SetDirtyFlag();
            return true;
        }
        break;

    case M_textAutoSize:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
                ptf->SetTextAutoSize(Text::DocView::TAS_None);
            else 
            {
                if (str == "shrink")
                    ptf->SetTextAutoSize(Text::DocView::TAS_Shrink);
                else if (str == "fit")
                    ptf->SetTextAutoSize(Text::DocView::TAS_Fit);
            }
            SetDirtyFlag();
            return true;
        }
        break;

    case M_useRichTextClipboard:
        if (penv->CheckExtensions())
        {
            ptf->SetUseRichClipboard(val.ToBool(GetASEnvironment()));
            if (ptf->HasEditorKit())
            {
                if (ptf->DoesUseRichClipboard())
                    ptf->GetEditorKit()->SetUseRichClipboard();
                else
                    ptf->GetEditorKit()->ClearUseRichClipboard();
            }
            return 1;
        }
        break;

    case M_alwaysShowSelection:
        if (penv->CheckExtensions())
        {
            ptf->SetAlwaysShowSelection(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;

    case M_selectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetActiveSelectionTextColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_selectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetActiveSelectionBackgroundColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_inactiveSelectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetInactiveSelectionTextColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_inactiveSelectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetInactiveSelectionBackgroundColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_noAutoSelection:
        if (penv->CheckExtensions())
        {
            ptf->SetNoAutoSelection(val.ToBool(penv));
            return true;
        }
        break;
    case M_disableIME:
        if (penv->CheckExtensions())
        {
            ptf->SetIMEDisabledFlag(val.ToBool(penv));
            return true;
        }
        break;
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    case M_bidirectionalTextEnabled:
        if (penv->CheckExtensions())
        {
            ptf->EnableBidirectionalText(val.ToBool(penv));
            return true;
        }
        break;
#endif
    case M_filters:
        {
#ifdef GFX_AS2_ENABLE_FILTERS
            Object* pobj = val.ToObject(penv);
            if (pobj && pobj->InstanceOf(penv, penv->GetPrototype(ASBuiltin_Array)))
            {
                TextFilter textfilter;
                bool filterdirty = false;
                ArrayObject* parrobj = static_cast<ArrayObject*>(pobj);
                for (int i=0; i < parrobj->GetSize(); ++i)
                {
                    Value* arrval = parrobj->GetElementPtr(i);
                    if (arrval)
                    {
                        Object* pavobj = arrval->ToObject(penv);
                        if (pavobj && pavobj->InstanceOf(penv, penv->GetPrototype(ASBuiltin_BitmapFilter)))
                        {
                            BitmapFilterObject* pfilterobj = static_cast<BitmapFilterObject*>(pavobj);
                            textfilter.LoadFilterDesc(pfilterobj->GetFilter());
                            filterdirty = true;
                        }
                    }
                }
                if (filterdirty)
                {
                    ptf->SetTextFilters(textfilter);
                    SetDirtyFlag();
                    ptf->SetAcceptAnimMoves(false);
                }
            }
#endif  // SF_NO_FXPLAYER_AS_FILTERS
            return true;
        }
        break;
    case M_restrict:
        {
            if (val.IsNull() || val.IsUndefined())
                ptf->ClearRestrict();
            else
                ptf->SetRestrict(val.ToString(penv));
            break;
        }
    default:
        break;
    }
    return AvmCharacter::SetMember(penv, name, val, flags);
}

bool AvmTextField::GetMember(Environment* penv, const ASString& name, Value* pval)
{
    StandardMember member = GetStandardMemberConstant(name);
    TextField* ptf          = GetTextField();
    SF_ASSERT(ptf);

    // Handle TextField-specific "standard" members.
    // We put this before GetStandardMember so that we can handle _width
    // and _height in a custom way.
    switch(member)
    {
    case M_html:
        {               
            pval->SetBool(ptf->IsHtml());
            return true;
        }

    case M_htmlText:
        {
            pval->SetString(ptf->GetText(true));
            return true;
        }

    case M_text:
        {                   
            pval->SetString(ptf->GetText(false));
            return true;
        }

    case M_textColor:
        {
            // Return color in 0xRRGGBB format
            pval->SetUInt(ptf->GetTextColor32());
            return true;
        }

    case M_textWidth:           
        {
            // Return the width, in pixels, of the text as laid out.
            // (I.E. the actual text content, not our defined
            // bounding box.)
            //
            // In local coords.  Verified against Macromedia Flash.
            pval->SetNumber((Number)ptf->GetTextWidth());
            return true;
        }

    case M_textHeight:              
        {
            // Return the height, in pixels, of the text as laid out.
            // (I.E. the actual text content, not our defined
            // bounding box.)
            //
            // In local coords.  Verified against Macromedia Flash.
            pval->SetNumber((Number)ptf->GetTextHeight());
            return true;
        }

    case M_length:
        {
            pval->SetNumber((Number)ptf->GetTextLength());
            return true;
        }

    case M_autoSize:
        {
            if (!ptf->IsAutoSize())
                pval->SetString(penv->CreateConstString("none"));
            else
            {
                switch(ptf->GetAlignment())
                {
                case Text::DocView::Align_Left:
                    pval->SetString(penv->CreateConstString("left")); 
                    break;
                case Text::DocView::Align_Right:
                    pval->SetString(penv->CreateConstString("right")); 
                    break;
                case Text::DocView::Align_Center:
                    pval->SetString(penv->CreateConstString("center")); 
                    break;
                default: pval->SetUndefined();
                }
            }
            return true;
        }
    case M_wordWrap:
        {
            pval->SetBool(ptf->IsWordWrap());
            return true;
        }
    case M_multiline:
        {
            pval->SetBool(ptf->IsMultiline());
            return true;
        }
    case M_border:
        {
            pval->SetBool(ptf->HasBorder());
            return true;
        }
    case M_variable:
        {
            if (!VariableName.IsEmpty())
                pval->SetString(VariableName);
            else
                pval->SetNull();
            return true;
        }
    case M_selectable:
        {
            pval->SetBool(ptf->IsSelectable());
            return true;
        }
    case M_embedFonts:
        {
            pval->SetBool(!ptf->DoesUseDeviceFont());
            return true;
        }
    case M_password:
        {
            pval->SetBool(ptf->IsPassword());
            return true;
        }

    case M_shadowStyle:
        {
            if (!penv->CheckExtensions())
                break;

            pval->SetString(ptf->GetShadowStyle());
            return true;
        }
    case M_shadowColor:
        if (penv->CheckExtensions())
        {
            pval->SetInt(ptf->GetShadowColor32());
            return true;
        }
        break;

    case M_hscroll:
        {
            pval->SetNumber((Number)((unsigned)ptf->GetHScrollOffset()));    
            return 1;
        }
    case M_scroll:
        {
            pval->SetNumber((Number)(ptf->GetVScrollOffset() + 1));    
            return 1;
        }
    case M_maxscroll:
        {
            pval->SetNumber((Number)(ptf->GetMaxVScroll() + 1));    
            return 1;
        }
    case M_maxhscroll:
        {
            pval->SetNumber((Number)((unsigned)ptf->GetMaxHScroll()));    
            return 1;
        }
    case M_background:
        {
            pval->SetBool(ptf->HasBackground());
            return 1;
        }
    case M_backgroundColor:
        {
            pval->SetNumber(Number(ptf->GetBackgroundColor24()));
            return 1;
        }
    case M_borderColor:
        {
            pval->SetNumber(Number(ptf->GetBorderColor24()));
            return 1;
        }
    case M_bottomScroll:
        {
            pval->SetNumber((Number)(ptf->GetBottomVScroll() + 1));
            return 1;
        }
    case M_type:
        {
            pval->SetString(ptf->IsReadOnly() ? 
                penv->CreateConstString("dynamic") : penv->CreateConstString("input")); 
            return 1;
        }
    case M_maxChars:
        {
            if (!ptf->HasMaxLength())
                pval->SetNull();
            else
                pval->SetNumber(Number(ptf->GetMaxLength()));
            return 1;
        }
    case M_condenseWhite:
        {
            pval->SetBool(ptf->IsCondenseWhite());
            return 1;
        }

    case M_antiAliasType:
        {
            pval->SetString(ptf->IsAAForReadability() ? 
                penv->CreateConstString("advanced") : penv->CreateConstString("normal"));
            return 1;
        }
    case M_mouseWheelEnabled:
        {
            pval->SetBool(ptf->IsMouseWheelEnabled());
            return true;
        }
    case M_styleSheet:
        {
            pval->SetUndefined();
#ifdef GFX_ENABLE_CSS
            if (GetCSSData() && GetCSSData()->pASStyleSheet.GetPtr())
                pval->SetAsObject(GetCSSData()->pASStyleSheet);
#endif //SF_NO_CSS_SUPPORT
            return true;
        }

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(ptf->IsHitTestDisableFlagSet());
            return 1;
        }
        break;

    case M_noTranslate:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(ptf->IsNoTranslate());
            return 1;
        }
        break;

    case M_caretIndex:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetCursorPos()));
            return 1;
        }
        break;
    case M_selectionBeginIndex:
        if (penv->CheckExtensions())
        {
            if (ptf->IsSelectable() && ptf->HasEditorKit())
                pval->SetNumber(Number(ptf->GetBeginIndex()));
            else
                pval->SetNumber(-1);
            return 1;
        }
        break;
    case M_selectionEndIndex:
        if (penv->CheckExtensions())
        {
            if (ptf->IsSelectable() && ptf->HasEditorKit())
                pval->SetNumber(Number(ptf->GetEndIndex()));
            else
                pval->SetNumber(-1);
            return 1;
        }
        break;
    case M_autoFit:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsAutoFit());
            return true;
        }
        break;

    case M_blurX:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(TwipsToPixels(Number(ptf->GetBlurX())));
            return true;
        }
        break;

    case M_blurY:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(TwipsToPixels(Number(ptf->GetBlurY())));
            return true;
        }
        break;

    case M_blurStrength:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetBlurStrength()));
            return true;
        }
        break;

    case M_outline:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetOutline()));
            return true;
        }
        break;

    case M_fauxBold:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->GetFauxBold());
            return true;
        }
        break;

    case M_fauxItalic:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->GetFauxItalic());
            return true;
        }
        break;


    case M_shadowAlpha:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowAlpha()));
            return true;
        }
        break;

    case M_shadowAngle:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowAngle()*(180.f/3.1415926f)));
            return true;
        }
        break;

    case M_shadowBlurX:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(TwipsToPixels(Number(ptf->GetShadowBlurX())));
            return true;
        }
        break;

    case M_shadowBlurY:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(TwipsToPixels(Number(ptf->GetShadowBlurY())));
            return true;
        }
        break;

    case M_shadowDistance:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(TwipsToPixels(Number(ptf->GetShadowDistance())));
            return true;
        }
        break;

    case M_shadowHideObject:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsShadowHiddenObject());
            return true;
        }
        break;

    case M_shadowKnockOut:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsShadowKnockOut());
            return true;
        }
        break;

    case M_shadowQuality:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowQuality()));
            return true;
        }
        break;

    case M_shadowStrength:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowStrength()));
            return true;
        }
        break;

    case M_shadowOutline:
        // Not implemented
        //if (penv->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_numLines:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetLinesCount()));
            return true;
        }
        break;

    case M_verticalAutoSize:
        if (penv->CheckExtensions())
        {
            if (!ptf->IsAutoSizeY())
                pval->SetString(penv->CreateConstString("none"));
            else
            {
                switch(ptf->GetVAlignment())
                {
                case Text::DocView::VAlign_None:
                    pval->SetString(penv->CreateConstString("none")); 
                    break;
                case Text::DocView::VAlign_Top:
                    pval->SetString(penv->CreateConstString("top")); 
                    break;
                case Text::DocView::VAlign_Bottom:
                    pval->SetString(penv->CreateConstString("bottom")); 
                    break;
                case Text::DocView::VAlign_Center:
                    pval->SetString(penv->CreateConstString("center")); 
                    break;
                default: pval->SetUndefined();
                }
            }
            return true;
        }
        break;

    case M_fontScaleFactor:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(ptf->GetFontScaleFactor());
        }
        break;

    case M_verticalAlign:
        if (penv->CheckExtensions())
        {
            switch(ptf->GetVAlignment())
            {
            case Text::DocView::VAlign_None:
                pval->SetString(penv->CreateConstString("none")); 
                break;
            case Text::DocView::VAlign_Top:
                pval->SetString(penv->CreateConstString("top")); 
                break;
            case Text::DocView::VAlign_Bottom:
                pval->SetString(penv->CreateConstString("bottom")); 
                break;
            case Text::DocView::VAlign_Center:
                pval->SetString(penv->CreateConstString("center")); 
                break;
            default: pval->SetUndefined();
            }
            return true;
        }
        break;

    case M_textAutoSize:
        if (penv->CheckExtensions())
        {
            switch(ptf->GetTextAutoSize())
            {
            case Text::DocView::TAS_Shrink:
                pval->SetString(penv->CreateConstString("shrink")); 
                break;
            case Text::DocView::TAS_Fit:
                pval->SetString(penv->CreateConstString("fit")); 
                break;
            default:
                pval->SetString(penv->CreateConstString("none")); 
                break;
            }
        }
        break;

    case M_useRichTextClipboard:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->DoesUseRichClipboard());
        }
        break;

    case M_alwaysShowSelection:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->DoesAlwaysShowSelection());
        }
        break;

    case M_selectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetActiveSelectionTextColor()));
            return true;
        }
        break;
    case M_selectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetActiveSelectionBackgroundColor()));
            return true;
        }
        break;
    case M_inactiveSelectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetInactiveSelectionTextColor()));
            return true;
        }
        break;
    case M_inactiveSelectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetInactiveSelectionBackgroundColor()));
            return true;
        }
        break;
    case M_noAutoSelection:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsNoAutoSelection());
            return true;
        }
        break;
    case M_disableIME:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsIMEDisabledFlag());
            return true;
        }
        break;
    case M_filters:
        {
            pval->SetUndefined();
#ifdef GFX_AS2_ENABLE_FILTERS
            // Not implemented because TextFilter does not distinguish
            // between drop shadow and glow filters
            Log* plog = penv->GetLog();
            if (plog)
                plog->LogWarning("Retrieval of the TextField.filters property is not implemented.");
#endif  // SF_NO_FXPLAYER_AS_FILTERS
            return true;
        }
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    case M_bidirectionalTextEnabled:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsBidirectionalTextEnabled());
            return true;
        }
        break;
#endif
    default: 
        if (GetStandardMember(member, pval, 0))
            return true;
        break;
    }

    if (ASTextFieldObj)
    {
        return ASTextFieldObj->GetMember(penv, name, pval);
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
    // looks like _global is accessable from any character
    if (name == "_global" && penv)
    {
        pval->SetAsObject(penv->GetGC()->pGlobal);
        return true;
    }
    return false;
}

bool    AvmTextField::IsTabable() const
{
    Object* pproto = Get__proto__();
    if (pproto)
    {
        // check prototype for tabEnabled
        Value val;
        const Environment* penv = GetASEnvironment();
        if (pproto->GetMemberRaw(penv->GetSC(), penv->CreateConstString("tabEnabled"), &val))
        {
            if (!val.IsUndefined())
                return val.ToBool(penv);
        }
    }
    return !GetTextField()->IsReadOnly();
}

void AvmTextField::UpdateVariable()
{
    if (!VariableName.IsEmpty())
    {
        Environment* penv = GetASEnvironment();
        if (penv)
        {
            TextField* ptf          = GetTextField();
            SF_ASSERT(ptf);
            VariableVal.SetString(ptf->GetText(false));
            penv->SetVariable(VariableName, VariableVal);
        }
    }
}

void AvmTextField::NotifyChanged()
{
    Environment* penv = GetASEnvironment();

    int nargs = 1;
    if (penv->CheckExtensions() && GetTextField()->GetFocusedControllerIdx() != ~0u)
    {
        penv->Push(int(GetTextField()->GetFocusedControllerIdx()));
        ++nargs;
    }
    penv->Push(Value(GetTextField()));
    AsBroadcaster::BroadcastMessage(penv, this, penv->CreateConstString("onChanged"), nargs, penv->GetTopIndex());
    penv->Drop(nargs);
}

void AvmTextField::BroadcastMessage(const FnCall& fn)
{
    ASString eventName(fn.Arg(0).ToString(fn.Env));
    AsBroadcaster::BroadcastMessage(fn.Env, fn.ThisPtr, eventName, fn.NArgs - 1, fn.Env->GetTopIndex() - 1);
}

void AvmTextField::OnScroll()
{
    TextField* ptextField = GetTextField();

    Environment* penv    = GetASEnvironment();
    MovieRoot::ActionEntry ae;
    ae.SetAction(ptextField, BroadcastMessage, NULL);
    if (!GetAS2Root()->HasActionEntry(ae))
    {
        ValueArray params;
        params.PushBack(Value(penv->CreateConstString("onScroller")));
        params.PushBack(Value(ptextField));
        GetAS2Root()->InsertEmptyAction()->SetAction(ptextField, BroadcastMessage, &params);
    }
}

#ifdef GFX_AS2_ENABLE_BITMAPDATA
void AvmTextField::ProceedImageSubstitution(const FnCall& fn, int idx, const Value* pve)
{
    if (pve && pve->IsObject())
    {
        TextField* ptextField = GetTextField();
        Object* peobj = pve->ToObject(fn.Env);
        SF_ASSERT(peobj);
        Value val;
        Text::DocView::ImageSubstitutor* pimgSubst = ptextField->CreateImageSubstitutor();
        if (!pimgSubst)
            return;
        Text::DocView::ImageSubstitutor::Element isElem;
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "subString", &val))
        {
            ASString str = val.ToString(fn.Env);
            UPInt wstrLen = str.GetLength();

            if (wstrLen > 15)
            {
                ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - length of subString should not exceed 15 characters",
                    GetName().ToCStr(), idx);
                return;
            }
            UTF8Util::DecodeStringSafe(isElem.SubString, 20, str.ToCStr(), str.GetSize() + 1);
            isElem.SubStringLen = (UByte)wstrLen;
        }
        else
        {
            // subString is mandatory!
            ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - subString should be specified",
                GetName().ToCStr(), idx);
            return;
        }
        float screenWidth = 0, screenHeight = 0;
        float origWidth = 0, origHeight = 0;
        float baseLineX = 0, baseLineY = 0;
        const char* idStr = NULL;
        Ptr<Render::Image> img;
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "image", &val))
        {
            Object* piobj = val.ToObject(fn.Env);
            if (piobj && piobj->GetObjectType() == Object_BitmapData)
            {
                BitmapData* pbmpData = static_cast<BitmapData*>(piobj);
                ImageResource* pimgRes = pbmpData->GetImage();
                Ptr<MovieDefImpl> md = pDispObj->GetResourceMovieDef();
                if (pimgRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
                    img = static_cast<Render::Image*>(pimgRes->GetImage());
                else
                {
                    if (!md->GetImageCreator())
                        LogDebugMessage(Log_Warning, "ImageCreator is null in ProceedImageSubstitution");
                    else
                    {
                        ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(this));
                        img = *md->GetImageCreator()->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimgRes->GetImage()));
                    }
                }
                if (img)
                {
                    Render::ImageRect dimr = img->GetRect();
                    origWidth  = (float)dimr.Width();
                    origHeight = (float)dimr.Height();
                    screenWidth  = PixelsToTwips(origWidth);
                    screenHeight = PixelsToTwips(origHeight);
                    if (origWidth == 0 || origHeight == 0)
                    {
                        ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - image has one zero dimension",
                            GetName().ToCStr(), idx);
                        return;
                    }
                }
            }
        }
        if (!img)
        {
            ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - 'image' is not specified or not a BitmapData",
                GetName().ToCStr(), idx);
            return;
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "width", &val))
        {
            Number v = val.ToNumber(fn.Env);
            screenWidth = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "height", &val))
        {
            Number v = val.ToNumber(fn.Env);
            screenHeight = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "baseLineX", &val))
        {
            Number v = val.ToNumber(fn.Env);
            baseLineX = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "baseLineY", &val))
        {
            Number v = val.ToNumber(fn.Env);
            baseLineY = (float)PixelsToTwips(v);
        }
        else
        {
            // if baseLineY is not specified, then use (origHeight - 1) pts instead
            baseLineY = origHeight - PixelsToTwips(1.0f);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "id", &val))
        {
            idStr = val.ToString(fn.Env).ToCStr();
        }

        isElem.pImageDesc = *SF_HEAP_NEW(fn.Env->GetHeap()) Text::ImageDesc;
        SF_ASSERT(isElem.pImageDesc);
        isElem.pImageDesc->pImage       = img;
        isElem.pImageDesc->BaseLineX    = TwipsToPixels(baseLineX);
        isElem.pImageDesc->BaseLineY    = TwipsToPixels(baseLineY);
        isElem.pImageDesc->ScreenWidth  = screenWidth;
        isElem.pImageDesc->ScreenHeight = screenHeight;
        if (idStr)
            ptextField->AddIdImageDescAssoc(idStr, isElem.pImageDesc);
        isElem.pImageDesc->Matrix.AppendTranslation(-isElem.pImageDesc->BaseLineX, -isElem.pImageDesc->BaseLineY);
        isElem.pImageDesc->Matrix.AppendScaling(isElem.pImageDesc->ScreenWidth/origWidth, 
                                                isElem.pImageDesc->ScreenHeight/origHeight);

        pimgSubst->AddImageDesc(isElem);
        ptextField->ForceCompleteReformat();
        SetDirtyFlag();
    }
}


void AvmTextField::SetImageSubstitutions(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        if (fn.NArgs >= 1)
        {
            if (fn.Arg(0).IsNull())
            {
                // clear all substitutions
                pthis->ClearIdImageDescAssoc();
                pthis->ClearImageSubstitutor();
                pthis->ForceCompleteReformat();
                pthis->SetDirtyFlag();
            }
            else
            {
                Object* pobj = fn.Arg(0).ToObject(fn.Env);
                if (pobj)
                {
                    // if array is specified as a parameter, proceed it; otherwise
                    // if an object is specified - proceed it as single element.
                    if (pobj->GetObjectType() == Object_Array)
                    {
                        ArrayObject* parr = static_cast<ArrayObject*>(pobj);
                        for (int i = 0, n = parr->GetSize(); i < n; ++i)
                        {
                            Value* pve = parr->GetElementPtr(i);
                            ToAvmTextField(pthis)->ProceedImageSubstitution(fn, i, pve);
                        }
                    }
                    else
                    {
                        const Value& ve = fn.Arg(0);
                        ToAvmTextField(pthis)->ProceedImageSubstitution(fn, 0, &ve);
                    }
                }
                else
                {
                    pthis->LogScriptWarning("%s.setImageSubstitutions() failed: parameter should be either 'null', object or array",
                        pthis->GetName().ToCStr());
                }
            }
        }
    }
}

void AvmTextField::UpdateImageSubstitution(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        if (fn.NArgs >= 1)
        {
            ASString idStr = fn.Arg(0).ToString(fn.Env);
            StringHashLH<Ptr<Text::ImageDesc> >* pimageDescAssoc = pthis->GetImageDescAssoc();
            if (pimageDescAssoc)
            {
                Ptr<Text::ImageDesc>* ppimgDesc = pimageDescAssoc->Get(idStr.ToCStr());
                if (ppimgDesc)
                {
                    Text::ImageDesc* pimageDesc = ppimgDesc->GetPtr();
                    SF_ASSERT(pimageDesc);
                    if (fn.NArgs >= 2)
                    {
                        if (fn.Arg(1).IsNull() || fn.Arg(1).IsUndefined())
                        {
                            // if null or undefined - remove the substitution and reformat
                            Text::DocView::ImageSubstitutor* pimgSubst = pthis->CreateImageSubstitutor();
                            if (pimgSubst)
                            {
                                pimgSubst->RemoveImageDesc(pimageDesc);
                                pthis->ForceCompleteReformat();
                                pthis->RemoveIdImageDescAssoc(idStr.ToCStr());
                                pthis->SetDirtyFlag();
                            }
                        }
                        else
                        {
                            Object* piobj = fn.Arg(1).ToObject(fn.Env);
                            if (piobj && piobj->GetObjectType() == Object_BitmapData)
                            {
                                BitmapData* pbmpData = static_cast<BitmapData*>(piobj);
                                ImageResource* pimgRes = pbmpData->GetImage();

                                Ptr<MovieDefImpl> md = pthis->GetResourceMovieDef();
                                Ptr<Render::Image> img;
                                if (pimgRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
                                    img = static_cast<Render::Image*>(pimgRes->GetImage());
                                else
                                {
                                    if (!md->GetImageCreator())
                                        LogDebugMessage(Log_Warning, "ImageCreator is null in UpdateImageSubstitution");
                                    else
                                    {
                                        ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(pthis));
                                        img = *md->GetImageCreator()->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimgRes->GetImage()));
                                    }
                                }
                                pimageDesc->pImage = img;
                                pthis->SetDirtyFlag();
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif //#ifdef GFX_AS2_ENABLE_BITMAPDATA

void AvmTextField::SetTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs == 1)
        {
            // my_textField.setTextFormat(textFormat:TextFormat)
            // Applies the properties of textFormat to all text in the text field.
            Object* pobjVal = fn.Arg(0).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                pthis->SetTextFormat(ptextFormatObj->mTextFormat);
                pthis->SetParagraphFormat(ptextFormatObj->mParagraphFormat);
                pthis->SetDirtyFlag();
            }
        }
        else if (fn.NArgs == 2)
        {
            // my_textField.setTextFormat(beginIndex:Number, textFormat:TextFormat)
            // Applies the properties of textFormat to the character at the beginIndex position.
            Object* pobjVal = fn.Arg(1).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                Number pos = fn.Arg(0).ToNumber(fn.Env);
                if (pos >= 0)
                {
                    unsigned upos = unsigned(pos);
                    pthis->SetTextFormat(ptextFormatObj->mTextFormat, upos, upos + 1);
                    pthis->SetParagraphFormat(ptextFormatObj->mParagraphFormat, upos, upos + 1);
                    pthis->SetDirtyFlag();
                }
            }
        }
        else if (fn.NArgs >= 3)
        {
            // my_textField.setTextFormat(beginIndex:Number, endIndex:Number, textFormat:TextFormat)
            // Applies the properties of the textFormat parameter to the span of text from the 
            // beginIndex position to the endIndex position.
            Object* pobjVal = fn.Arg(2).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                Number beginIndex = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env));
                Number endIndex   = Alg::Max((Number)0, fn.Arg(1).ToNumber(fn.Env));
                if (beginIndex <= endIndex)
                {
                    TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                    pthis->SetTextFormat(ptextFormatObj->mTextFormat, (unsigned)beginIndex, (unsigned)endIndex);
                    pthis->SetParagraphFormat(ptextFormatObj->mParagraphFormat, (unsigned)beginIndex, (unsigned)endIndex);
                    pthis->SetDirtyFlag();
                }
            }
        }
    }
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.setTextFormat is not supported due to SF_NO_FXPLAYER_AS_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::GetTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        UPInt beginIndex = 0, endIndex = SF_MAX_UPINT;
        if (fn.NArgs >= 1)
        {
            // beginIndex
            Number v = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env));
            if (v >= 0)
                beginIndex = (unsigned)v;
        }
        if (fn.NArgs >= 2)
        {
            // endIndex
            Number v = Alg::Max((Number)0, fn.Arg(1).ToNumber(fn.Env));
            if (v >= 0)
                endIndex = (unsigned)v;
        }
        else if (fn.NArgs >= 1)
        {
            Number v = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env) + 1);
            if (v >= 0)
                endIndex = (unsigned)v;
        }
        if (beginIndex <= endIndex)
        {
            Text::TextFormat textFmt(fn.Env->GetHeap());
            Text::ParagraphFormat paraFmt;
            pthis->GetTextAndParagraphFormat(&textFmt, &paraFmt, beginIndex, endIndex);

            Ptr<TextFormatObject> pasTextFmt = *SF_HEAP_NEW(fn.Env->GetHeap()) TextFormatObject(fn.Env);
            pasTextFmt->SetTextFormat(fn.Env->GetSC(), textFmt);
            pasTextFmt->SetParagraphFormat(fn.Env->GetSC(), paraFmt);
            fn.Result->SetAsObject(pasTextFmt);
        }
        else
            fn.Result->SetUndefined();
    }
    else
        fn.Result->SetUndefined();
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.getTextFormat is not supported due to SF_NO_FXPLAYER_AS_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::GetNewTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
//        AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        const Text::TextFormat* ptextFmt = pthis->GetDefaultTextFormat();
        const Text::ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();

        Ptr<TextFormatObject> pasTextFmt = *SF_HEAP_NEW(fn.Env->GetHeap()) TextFormatObject(fn.Env);
        if (ptextFmt)
            pasTextFmt->SetTextFormat(fn.Env->GetSC(), *ptextFmt);
        if (pparaFmt)
            pasTextFmt->SetParagraphFormat(fn.Env->GetSC(), *pparaFmt);
        fn.Result->SetAsObject(pasTextFmt);
    }
    else
        fn.Result->SetUndefined();
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.getNewTextFormat is not supported due to SF_NO_FXPLAYER_AS_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::SetNewTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            Object* pobjVal = fn.Arg(0).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);

                const Text::TextFormat* ptextFmt = pthis->GetDefaultTextFormat();
                const Text::ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();
                pthis->SetDefaultTextFormat(ptextFmt->Merge(ptextFormatObj->mTextFormat));
                pthis->SetDefaultParagraphFormat(pparaFmt->Merge(ptextFormatObj->mParagraphFormat));
            }
        }
    }
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.setNewTextFormat is not supported due to SF_NO_FXPLAYER_AS_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::ReplaceText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 3)
        {
            Number start = fn.Arg(0).ToNumber(fn.Env);
            Number end   = fn.Arg(1).ToNumber(fn.Env);
            ASString str   = fn.Arg(2).ToString(fn.Env);

            unsigned len = str.GetLength();
            unsigned startPos = unsigned(start);
            unsigned endPos   = unsigned(end);
            if (start >= 0 && end >= 0 && startPos <= endPos)
            {
                // replaceText does *NOT* use default text format, if replaced text
                // is located in the middle of the text; however, it uses the default
                // format, if the replacing text is at the very end of the text. This
                // is so, because Flash doesn't know what is the format at the cursor pos
                // if the cursor is behind the last char (caretIndex == length).
                // Our implementation knows always about formats, thus, we need artificially
                // replicate this behavior. Though, it still works differently from Flash,
                // since that default text format will be combined with current text format
                // at the cursor position. Not sure, we should worry about this.
                ConstPtr<Text::TextFormat> ptextFmt;
                ConstPtr<Text::ParagraphFormat> pparaFmt;
                UPInt prevLen = pthis->GetTextLength();
                UPInt newLen  = prevLen - (endPos - startPos) + len;
                if (startPos >= prevLen)
                {
                    // use default text format
                    ptextFmt = pthis->GetDefaultTextFormat();
                    pparaFmt = pthis->GetDefaultParagraphFormat();
                }
                else
                {
                    const Text::TextFormat* _ptextFmt;
                    const Text::ParagraphFormat* _pparaFmt;
                    pthis->GetTextAndParagraphFormat(&_ptextFmt, &_pparaFmt, startPos);
                    ptextFmt = _ptextFmt;
                    pparaFmt = _pparaFmt;
                }

                wchar_t buf[1024];
                if (len < sizeof(buf)/sizeof(buf[0]))
                {
                    UTF8Util::DecodeStringSafe(buf, 1024, str.ToCStr());
                    pthis->ReplaceText(buf, startPos, endPos);
                }
                else
                {
                    wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
                    UTF8Util::DecodeStringSafe(pbuf, len + 1, str.ToCStr());
                    pthis->ReplaceText(pbuf, startPos, endPos);
                    SF_FREE(pbuf);
                }
                if (pthis->HasEditorKit())
                {
                    // Replace text does not change the cursor position, unless
                    // this position doesn't exist anymore.

                    if (pthis->GetEditorKit()->GetCursorPos() > newLen)
                        pthis->GetEditorKit()->SetCursorPos(newLen, false);
                }
                if (pparaFmt)
                    pthis->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
                if (ptextFmt)
                {
                    pthis->SetTextFormat(*ptextFmt, startPos, startPos + len);
                }
                pthis->SetDirtyFlag();
            }
        }
    }
}

void AvmTextField::ReplaceSel(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1 && pthis->HasEditorKit())
        {
            ASString str = fn.Arg(0).ToString(fn.Env);

            ConstPtr<Text::TextFormat> ptextFmt = pthis->GetDefaultTextFormat();
            ConstPtr<Text::ParagraphFormat> pparaFmt = pthis->GetDefaultParagraphFormat();

            unsigned len = str.GetLength();
            UPInt startPos = pthis->GetEditorKit()->GetBeginSelection();
            UPInt endPos   = pthis->GetEditorKit()->GetEndSelection();
            wchar_t buf[1024];
            if (len < sizeof(buf)/sizeof(buf[0]))
            {
                UTF8Util::DecodeStringSafe(buf, 1024, str.ToCStr());
                pthis->ReplaceText(buf, startPos, endPos);
            }
            else
            {
                wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
                UTF8Util::DecodeStringSafe(pbuf, len + 1, str.ToCStr());
                pthis->ReplaceText(pbuf, startPos, endPos);
                SF_FREE(pbuf);
            }
            pthis->GetEditorKit()->SetCursorPos(startPos + len, false);
            if (pparaFmt)
                pthis->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
            if (ptextFmt)
                pthis->SetTextFormat(*ptextFmt, startPos, startPos + len);
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::RemoveTextField(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->GetDepth() < 16384)
        {
            pthis->LogScriptWarning("%s.removeMovieClip() failed - depth must be >= 0",
                pthis->GetName().ToCStr());
            return;
        }
        pthis->RemoveDisplayObject();
    }
}

#ifndef SF_NO_TEXTFIELD_EXTENSIONS
void AvmTextField::AppendText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            ASString str = fn.Arg(0).ToString(fn.Env);
            pthis->AppendText(str.ToCStr());
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::AppendHtml(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            ASString str = fn.Arg(0).ToString(fn.Env);
            Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(pthis));
            pthis->AppendHtml(str.ToCStr(), SF_MAX_UPINT, false, &imageInfoArray);
            if (imageInfoArray.GetSize() > 0)
            {
                pthis->ProcessImageTags(imageInfoArray);
            }
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::GetCharBoundaries(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            unsigned charIndex = (unsigned)fn.Arg(0).ToUInt32(fn.Env);
            RectF    charBounds;
            charBounds.Clear();
            if (pthis->GetCharBoundaries(&charBounds, charIndex))
            {

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);

                ASRect gasRect;
                gasRect.x1      = TwipsToPixels(Number(charBounds.x1));
                gasRect.y1      = TwipsToPixels(Number(charBounds.y1));
                gasRect.x2      = TwipsToPixels(Number(charBounds.x2));
                gasRect.y2      = TwipsToPixels(Number(charBounds.y2));

                prect->SetProperties(fn.Env, gasRect);

#else
                Ptr<Object> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

                ASStringContext *psc = fn.Env->GetSC();
                prect->SetConstMemberRaw(psc, "x", TwipsToPixels(Number(charBounds.x1)));
                prect->SetConstMemberRaw(psc, "y", TwipsToPixels(Number(charBounds.y1)));
                prect->SetConstMemberRaw(psc, "width", TwipsToPixels(Number(charBounds.x2)));
                prect->SetConstMemberRaw(psc, "height", TwipsToPixels(Number(charBounds.y2)));
#endif
                fn.Result->SetAsObject(prect.GetPtr());
            }
            else
                fn.Result->SetNull();
        }
    }
}

void AvmTextField::GetExactCharBoundaries(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            unsigned charIndex = (unsigned)fn.Arg(0).ToUInt32(fn.Env);
            RectF    charBounds;
            charBounds.Clear();
            if (pthis->GetExactCharBoundaries(&charBounds, charIndex))
            {

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);

                ASRect gasRect;
                gasRect.x1  = TwipsToPixels(Number(charBounds.x1));
                gasRect.y1  = TwipsToPixels(Number(charBounds.y1));
                gasRect.x2  = TwipsToPixels(Number(charBounds.x2));
                gasRect.y2  = TwipsToPixels(Number(charBounds.y2));
                prect->SetProperties(fn.Env, gasRect);

#else
                Ptr<Object> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

                ASStringContext *psc = fn.Env->GetSC();
                prect->SetConstMemberRaw(psc, "x", TwipsToPixels(Number(charBounds.x1)));
                prect->SetConstMemberRaw(psc, "y", TwipsToPixels(Number(charBounds.y1)));
                prect->SetConstMemberRaw(psc, "width", TwipsToPixels(Number(charBounds.x2)));
                prect->SetConstMemberRaw(psc, "height", TwipsToPixels(Number(charBounds.y2)));
#endif
                fn.Result->SetAsObject(prect.GetPtr());
            }
            else
                fn.Result->SetNull();
        }
    }
}

void AvmTextField::GetCharIndexAtPoint(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 2)
        {
            Number x = fn.Arg(0).ToNumber(fn.Env);
            Number y = fn.Arg(1).ToNumber(fn.Env);
            UPInt pos = pthis->GetCharIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
            if (!Alg::IsMax(pos))
                fn.Result->SetNumber(Number(pos));
            else
                fn.Result->SetNumber(-1);
        }
    }
}

void AvmTextField::GetLineIndexAtPoint(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 2)
        {
            Number x = fn.Arg(0).ToNumber(fn.Env);
            Number y = fn.Arg(1).ToNumber(fn.Env);
            unsigned  pos = pthis->GetLineIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
            if (!Alg::IsMax(pos))
                fn.Result->SetNumber(Number(pos));
            else
                fn.Result->SetNumber(-1);
        }
    }
}

void AvmTextField::GetLineOffset(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt off = pthis->GetLineOffset(unsigned(lineIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineLength(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt len = pthis->GetLineLength(unsigned(lineIndex));
                if (!Alg::IsMax(len))
                    fn.Result->SetNumber(Number(len));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineMetrics(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetUndefined();
            else
            {
                Text::DocView::LineMetrics metrics;
                bool rv = pthis->GetLineMetrics(unsigned(lineIndex), &metrics);
                if (rv)
                {
                    Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "ascent", Value(TwipsToPixels((Number)metrics.Ascent)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "descent", Value(TwipsToPixels((Number)metrics.Descent)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "width", Value(TwipsToPixels((Number)metrics.Width)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "height", Value(TwipsToPixels((Number)metrics.Height)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "leading", Value(TwipsToPixels((Number)metrics.Leading)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "x", Value(TwipsToPixels((Number)metrics.FirstCharXOff)));
                    fn.Result->SetAsObject(pobj);
                }
                else
                    fn.Result->SetUndefined();
            }
        }
    }
}

void AvmTextField::GetLineText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetUndefined();
            else
            {
                UPInt len = 0;
                const wchar_t* ptext = pthis->GetLineText(unsigned(lineIndex), &len);
                if (ptext)
                {
                    String str;
                    str.AppendString(ptext, (SPInt)len);
                    ASString asstr = fn.Env->CreateString(str);
                    fn.Result->SetString(asstr);
                }
                else
                    fn.Result->SetString(fn.Env->CreateConstString(""));
            }
        }
    }
}


void AvmTextField::GetFirstCharInParagraph(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int charIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (charIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt off = pthis->GetFirstCharInParagraph(unsigned(charIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineIndexOfChar(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int charIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (charIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                unsigned off = pthis->GetLineIndexOfChar(unsigned(charIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

// Extension method: copyToClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::CopyToClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool     richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->CopyToClipboard(startIndex, endIndex, richClipboard);
        }
    }
}

// Extension method: cutToClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::CutToClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool     richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->CutToClipboard(startIndex, endIndex, richClipboard);
            pthis->SetDirtyFlag();
        }
    }
}

// Extension method: pasteFromClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::PasteFromClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool     richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->PasteFromClipboard(startIndex, endIndex, richClipboard);
            pthis->SetDirtyFlag();
        }
    }
}
#endif //SF_NO_TEXTFIELD_EXTENSIONS

//////////////////////////////////////////
TextFieldObject::TextFieldObject(ASStringContext *psc, Object* pprototype)
: Object(psc)
{ 
    pIMECompositionStringStyles = NULL;
    Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
}

TextFieldObject::TextFieldObject(Environment* penv)
: Object(penv)
{ 
    pIMECompositionStringStyles = NULL;
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_TextField)); // this ctor is used for prototype obj only
}

TextFieldObject::TextFieldObject(GlobalContext* gCtxt, InteractiveObject* ptextfield) 
: Object(gCtxt->GetGC()), pTextField(ptextfield)
{
    AvmTextField* pavmTf = ToAvmTextField(ptextfield);
    ASStringContext* psc = pavmTf->GetASEnvironment()->GetSC();
    //Set__proto__(psc, gCtxt->GetPrototype(ASBuiltin_TextField));
    Set__proto__(psc, pavmTf->Get__proto__());
    pIMECompositionStringStyles = NULL;
}

TextFieldObject::~TextFieldObject()
{
    delete pIMECompositionStringStyles;
}

Text::IMEStyle*   TextFieldObject::GetIMECompositionStringStyles()
{
    if (pIMECompositionStringStyles)
        return pIMECompositionStringStyles;
    Object* pproto = Get__proto__();
    if (pproto && pproto->GetObjectType() == Object::Object_TextFieldASObject)
    {
        TextFieldObject* ptextproto = static_cast<TextFieldObject*>(pproto);
        return ptextproto->GetIMECompositionStringStyles();
    }
    return NULL;
}

void TextFieldObject::SetIMECompositionStringStyles(const Text::IMEStyle& imeStyles)
{
    if (!pIMECompositionStringStyles)
        pIMECompositionStringStyles = SF_HEAP_AUTO_NEW(this) Text::IMEStyle(imeStyles);
    else
        *pIMECompositionStringStyles = imeStyles;
}

//////
static const NameFunction GAS_TextFieldFunctionTable[] = 
{
    { "getDepth",           &AvmCharacter::CharacterGetDepth },
    { "getTextFormat",      &AvmTextField::GetTextFormat },
    { "getNewTextFormat",   &AvmTextField::GetNewTextFormat },
    { "setTextFormat",      &AvmTextField::SetTextFormat },
    { "setNewTextFormat",   &AvmTextField::SetNewTextFormat },
    { "replaceSel",         &AvmTextField::ReplaceSel },
    { "replaceText",        &AvmTextField::ReplaceText },
    { "removeTextField",    &AvmTextField::RemoveTextField },

    { 0, 0 }
};

#ifndef SF_NO_TEXTFIELD_EXTENSIONS
static const NameFunction GAS_TextFieldExtFunctionTable[] = 
{
    { "appendText",             &AvmTextField::AppendText },
    { "appendHtml",             &AvmTextField::AppendHtml },
    { "copyToClipboard",        &AvmTextField::CopyToClipboard },
    { "cutToClipboard",         &AvmTextField::CutToClipboard },
    { "getCharBoundaries",      &AvmTextField::GetCharBoundaries },
    { "getCharIndexAtPoint",    &AvmTextField::GetCharIndexAtPoint },
    { "getExactCharBoundaries", &AvmTextField::GetExactCharBoundaries },
    { "getFirstCharInParagraph",&AvmTextField::GetFirstCharInParagraph },
    { "getIMECompositionStringStyle",  &TextFieldProto::GetIMECompositionStringStyle },
    { "getLineIndexAtPoint",    &AvmTextField::GetLineIndexAtPoint },
    { "getLineIndexOfChar",     &AvmTextField::GetLineIndexOfChar },
    { "getLineOffset",          &AvmTextField::GetLineOffset },
    { "getLineMetrics",         &AvmTextField::GetLineMetrics },
    { "getLineText",            &AvmTextField::GetLineText },
    { "getLineLength",          &AvmTextField::GetLineLength },
    { "pasteFromClipboard",     &AvmTextField::PasteFromClipboard },
    { "setIMECompositionStringStyle",  &TextFieldProto::SetIMECompositionStringStyle },
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    { "setImageSubstitutions",  &AvmTextField::SetImageSubstitutions },
    { "updateImageSubstitution",&AvmTextField::UpdateImageSubstitution },
#endif

    { 0, 0 }
};
#endif //SF_NO_TEXTFIELD_EXTENSIONS

TextFieldProto::TextFieldProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor) :
Prototype<TextFieldObject>(psc, prototype, constructor)
{
    AsBroadcaster::InitializeProto(psc, this);
    InitFunctionMembers(psc, GAS_TextFieldFunctionTable);
    SetConstMemberRaw(psc, "scroll", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "hscroll", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "maxscroll", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "maxhscroll", Value::UNSET, PropFlags::PropFlag_DontDelete);

    SetConstMemberRaw(psc, "background", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "backgroundColor", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "border", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "borderColor", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "bottomScroll", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "mouseWheelEnabled", Value::UNSET, PropFlags::PropFlag_DontDelete);

    SetConstMemberRaw(psc, "antiAliasType", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "autoSize", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "condenseWhite", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "embedFonts", Value::UNSET, PropFlags::PropFlag_DontDelete);
    // there is no method to retrieve filters.
    //SetConstMemberRaw(psc, "filters", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "html", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "htmlText", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "length", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "maxChars", Value::UNSET, PropFlags::PropFlag_DontDelete);
    SetConstMemberRaw(psc, "multiline", Value::UNSET, PropFlags::PropFlag_DontDelete);    
    SetConstMemberRaw(psc, "password", Value::UNSET, PropFlags::PropFlag_DontDelete);      
    SetConstMemberRaw(psc, "restrict", Value::NULLTYPE, PropFlags::PropFlag_DontDelete);     
    SetConstMemberRaw(psc, "selectable", Value::UNSET, PropFlags::PropFlag_DontDelete);    
    SetConstMemberRaw(psc, "styleSheet", Value::UNSET, PropFlags::PropFlag_DontDelete);    
    // can't add tabEnabled here, since its presence will stop prototype chain lookup, even if
    // it is UNSET or "undefined".
    //SetConstMemberRaw(psc, "tabEnabled", Value::UNSET, PropFlags::PropFlag_DontDelete); 
    SetConstMemberRaw(psc, "tabIndex", Value::UNSET, PropFlags::PropFlag_DontDelete);    
    SetConstMemberRaw(psc, "text", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "textColor", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "textHeight", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "textWidth", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "type", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "variable", Value::UNSET, PropFlags::PropFlag_DontDelete);  
    SetConstMemberRaw(psc, "wordWrap", Value::UNSET, PropFlags::PropFlag_DontDelete);  


#ifndef SF_NO_TEXTFIELD_EXTENSIONS
    // init extensions
    InitFunctionMembers(psc, GAS_TextFieldExtFunctionTable);
#endif

#ifndef SF_NO_IME_SUPPORT
    // create default styles for IME
	SetIMECompositionStringStyles(Text::CompositionString::GetDefaultStyles());
#endif // SF_NO_IME_SUPPORT
}

static const NameFunction GAS_TextFieldStaticFunctionTable[] = 
{
    { "getFontList",  &TextFieldCtorFunction::GetFontList },
    { 0, 0 }
};

TextFieldCtorFunction::TextFieldCtorFunction(ASStringContext *psc)
: CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, GAS_TextFieldStaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
}

Object* TextFieldCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) TextFieldObject(penv);
}

void TextFieldCtorFunction::GlobalCtor(const FnCall& fn) 
{
    SF_UNUSED(fn);
    //fn.Result->SetAsObject(Ptr<Object>(*new TextFieldObject()));
}

FunctionRef TextFieldCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) TextFieldCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) TextFieldProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_TextField, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_TextField), Value(ctor));

#ifdef GFX_ENABLE_CSS
    pgc->AddBuiltinClassRegistry<ASBuiltin_StyleSheet, StyleSheetCtorFunction>(sc, ctor.GetObjectPtr()); 
#endif // SF_NO_CSS_SUPPORT

    return ctor;
}


void TextFieldCtorFunction::GetFontList(const FnCall& fn)
{
    MovieImpl* proot = fn.Env->GetMovieImpl();
    MovieDefImpl* pmoviedef = static_cast<MovieDefImpl*>(proot->GetMovieDef());

    // get fonts from ..
    StringHash<String> fontnames;
    // fontresource
    struct FontsVisitor : public MovieDef::ResourceVisitor 
    { 
        StringHash<String>*       pFontNames;

        FontsVisitor(StringHash<String>* pfontnames) : pFontNames(pfontnames) {}

        virtual void Visit(MovieDef*, Resource* presource, 
            ResourceId, const char*) 
        { 
            if (presource->GetResourceType() == Resource::RT_Font)
            {
                FontResource* pfontResource = static_cast<FontResource*>(presource);
                String fontname(pfontResource->GetName());
                pFontNames->Set(fontname, fontname);
            }
        }
    } fontsVisitor(&fontnames);
    pmoviedef->VisitResources(&fontsVisitor, MovieDef::ResVisit_Fonts);
    // fontlib
    FontLib* pfontlib = proot->GetFontLib();
    if (pfontlib)
    {
        pfontlib->LoadFontNames(fontnames);
    }
    // fontprovider
    FontProvider* pfontprovider = proot->GetFontProvider();
    if (pfontprovider)
    {
        pfontprovider->LoadFontNames(fontnames);
    }

    // returns an array of strings
    Ptr<ArrayObject> parr = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    for (StringHash<String>::ConstIterator iter = fontnames.Begin(); iter != fontnames.End(); ++iter)
    {
        Value name(fn.Env->CreateString(iter->First));
        parr->PushBack(name);
    }
    fn.Result->SetAsObject(parr);    
}


Text::HighlightInfo TextFieldProto::ParseStyle
(const FnCall& fn, unsigned paramIndex, const Text::HighlightInfo& initialHInfo)
{
    Text::HighlightInfo hinfo = initialHInfo;
    if (fn.NArgs >= 1)
    {
        Ptr<Object> pobj = fn.Arg(paramIndex).ToObject(fn.Env);
        if (pobj)
        {
            Value val;
            if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("textColor"), &val))
            {
                if (val.ToString(fn.Env) == "none")
                    hinfo.ClearTextColor();
                else
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        hinfo.SetTextColor(val.ToUInt32(fn.Env) | 0xFF000000u);
                }
            }
            if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("backgroundColor"), &val))
            {
                if (val.ToString(fn.Env) == "none")
                    hinfo.ClearBackgroundColor();
                else
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        hinfo.SetBackgroundColor(val.ToUInt32(fn.Env) | 0xFF000000u);
                }
            }
            if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("underlineColor"), &val))
            {
                if (val.ToString(fn.Env) == "none")
                    hinfo.ClearUnderlineColor();
                else
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        hinfo.SetUnderlineColor(val.ToUInt32(fn.Env) | 0xFF000000u);
                }
            }
            if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("underlineStyle"), &val))
            {
                ASString str = val.ToString(fn.Env);
                if (str == "dotted")
                    hinfo.SetDottedUnderline();
                else if (str == "single")
                    hinfo.SetSingleUnderline();
                else if (str == "thick")
                    hinfo.SetThickUnderline();
                else if (str == "ditheredSingle")
                    hinfo.SetDitheredSingleUnderline();
                else if (str == "ditheredThick")
                    hinfo.SetDitheredThickUnderline();
                else 
                    hinfo.ClearUnderlineStyle();
            }
        }
    }
    return hinfo;
}

void TextFieldProto::MakeStyle(const FnCall& fn, const Text::HighlightInfo& hinfo)
{
    Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
    if (hinfo.HasUnderlineStyle())
    {
        const char* pstyle;
        switch(hinfo.GetUnderlineStyle())
        {
        case Text::HighlightInfo::Underline_Single: pstyle = "single"; break;
        case Text::HighlightInfo::Underline_Thick:  pstyle = "thick"; break;
        case Text::HighlightInfo::Underline_Dotted: pstyle = "dotted"; break;
        case Text::HighlightInfo::Underline_DitheredSingle: pstyle = "ditheredSingle"; break;
        case Text::HighlightInfo::Underline_DitheredThick:  pstyle = "ditheredThick"; break;
        default: pstyle = NULL;
        }
        if (pstyle)
            pobj->SetConstMemberRaw(fn.Env->GetSC(), "underlineStyle", Value(fn.Env->CreateConstString(pstyle)));
    }
    if (hinfo.HasUnderlineColor())
    {
        Number c = (Number)(hinfo.GetUnderlineColor().ToColor32() & 0xFFFFFFu);
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "underlineColor", Value(c));
    }
    if (hinfo.HasBackgroundColor())
    {
        Number c = (Number)(hinfo.GetBackgroundColor().ToColor32() & 0xFFFFFFu);
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "backgroundColor", Value(c));
    }
    if (hinfo.HasTextColor())
    {
        Number c = (Number)(hinfo.GetTextColor().ToColor32() & 0xFFFFFFu);
        pobj->SetConstMemberRaw(fn.Env->GetSC(), "textColor", Value(c));
    }
    fn.Result->SetAsObject(pobj);
}

static Text::IMEStyle::Category GFx_StringToIMEStyleCategory(const ASString& categoryStr)
{
    Text::IMEStyle::Category cat = Text::IMEStyle::SC_MaxNum;
    if (categoryStr == "compositionSegment")
        cat = Text::IMEStyle::SC_CompositionSegment;
    else if (categoryStr == "clauseSegment")
        cat = Text::IMEStyle::SC_ClauseSegment;
    else if (categoryStr == "convertedSegment")
        cat = Text::IMEStyle::SC_ConvertedSegment;
    else if (categoryStr == "phraseLengthAdj")
        cat = Text::IMEStyle::SC_PhraseLengthAdj;
    else if (categoryStr == "lowConfSegment")
        cat = Text::IMEStyle::SC_LowConfSegment;
    return cat;
}

void TextFieldProto::SetIMECompositionStringStyle(const FnCall& fn)
{
    if (fn.ThisPtr)
    {
        Ptr<TextFieldObject> pasTextObj;
        if (fn.ThisPtr->GetObjectType() == Object_TextField)
        {
            AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
            pasTextObj = pthis->GetTextFieldASObject();
        }
        else if (fn.ThisPtr->GetObjectType() == Object_TextFieldASObject)
        {
            pasTextObj = static_cast<TextFieldObject*>(fn.ThisPtr);
        }
        if (pasTextObj)
        {
            if (fn.NArgs >= 1)
            {
                ASString categoryStr(fn.Arg(0).ToString(fn.Env));
                Text::IMEStyle::Category cat = GFx_StringToIMEStyleCategory(categoryStr);
                if (cat >= Text::IMEStyle::SC_MaxNum)
                    return;

                Text::IMEStyle* pimeStyles = pasTextObj->GetIMECompositionStringStyles();
                Text::IMEStyle imeStyles;
                if (pimeStyles)
                    imeStyles = *pimeStyles;
                imeStyles.SetElement(cat, ParseStyle(fn, 1, imeStyles.GetElement(cat)));
                pasTextObj->SetIMECompositionStringStyles(imeStyles);
            }
        }
    }
}

void TextFieldProto::GetIMECompositionStringStyle(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.ThisPtr)
    {
        Ptr<TextFieldObject> pasTextObj;
        if (fn.ThisPtr->GetObjectType() == Object_TextField)
        {
            AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
            pasTextObj = pthis->GetTextFieldASObject();
        }
        else if (fn.ThisPtr->GetObjectType() == Object_TextFieldASObject)
        {
            pasTextObj = static_cast<TextFieldObject*>(fn.ThisPtr);
        }
        if (pasTextObj)
        {
            Text::IMEStyle* pimeStyles = pasTextObj->GetIMECompositionStringStyles();
            if (pimeStyles)
            {
                ASString categoryStr(fn.Arg(0).ToString(fn.Env));
                Text::IMEStyle::Category cat = GFx_StringToIMEStyleCategory(categoryStr);
                if (cat >= Text::IMEStyle::SC_MaxNum)
                    return;
                const Text::HighlightInfo& hinfo = pimeStyles->GetElement(cat);
                MakeStyle(fn, hinfo);
            }
        }
    }
}

}}} //SF::GFx::AS2

