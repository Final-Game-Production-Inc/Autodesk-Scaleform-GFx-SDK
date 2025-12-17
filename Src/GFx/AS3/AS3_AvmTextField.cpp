/**************************************************************************

Filename    :   AS3_AvmTextField.cpp
Content     :   Implementation of AS3 part of TextField character
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/AS3/AS3_AvmTextField.h"
#include "Render/Text/Text_DocView.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_TextEvent.h"
#include "GFx/AS3/Obj/GFx/AS3_Obj_Gfx_TextEventEx.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_BitmapData.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

AvmTextField::AvmTextField(TextField* ptf) 
:
    AvmInteractiveObj(ptf)
{
    ptf->SetHtml(true); // seems to me AS3 text fields are always html-enabled
    ptf->SetSelection(0, 0);
}

TextFieldDef* AvmTextField::GetTextFieldDef() const
{
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    return static_cast<TextField*>(pDispObj)->GetTextFieldDef();
}

bool AvmTextField::HasStyleSheet() const 
{ 
#ifdef GFX_ENABLE_CSS
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    const TextField::CSSHolderBase* pcss = GetTextField()->GetCSSData();
    return pcss && static_cast<const CSSHolder*>(pcss)->pASStyleSheet != NULL; 
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
    AvmInteractiveObj::OnEvent(event);
    if (event.Id == EventId::Event_Click)
    {
        TextField* ptf       = GetTextField();
        
        // PPS: If selection length > 1, then we are in the process of selecting text
        //      In this case, don't fire an event
        if (ptf->GetBeginIndex() != ptf->GetEndIndex())
            return false;

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

                            // url should represent link in event protocol:
                            // "event:eventString"
                            MovieImpl* proot = GetMovieImpl();
                            if (proot && String::CompareNoCase(url, "event:", sizeof("event:") - 1) == 0)
                            {
                                url += sizeof("event:") - 1;
                                
                                // fire the link event
                                const ASString& evtName = GetAS3Root()->GetStringManager()->CreateString("link");
                                Instances::fl_display::DisplayObject* as3obj = GetAS3Obj();
                                if (as3obj != NULL)
                                {
                                    if (as3obj->WillTrigger(evtName, false) || as3obj->WillTrigger(evtName, true))
                                    {
                                        SPtr<Instances::fl_events::TextEvent> evt;
                                        Value params[] = { Value(evtName), Value(true), Value(true) };
                                        ASVM* asvm = GetAS3Root()->GetAVM();
                                        asvm->ConstructInstance(evt, asvm->ExtensionsEnabled ? asvm->TextEventExClass : asvm->TextEventClass, 3, params);
                                        SF_ASSERT(evt);
                                        evt->Target = as3obj;
                                        evt->SetText(GetAS3Root()->GetStringManager()->CreateString(url));

                                        if (asvm->ExtensionsEnabled)
                                        {
                                            Instances::fl_gfx::TextEventEx* evtExt = static_cast<Instances::fl_gfx::TextEventEx*>(evt.GetPtr());
                                            evtExt->SetControllerIdx(event.ControllerIndex);
                                            evtExt->SetButtonIdx(event.ButtonId);
                                        }

                                        as3obj->Dispatch(evt, GetDispObj());
                                    }
                                }
                            }
                            //printf ("%s\n", url);
                            else
                            {
                                // PPS: If not event, then default functionality should be to let the OS try to open the url
                                UrlNavigator *urlnav = proot->GetUrlNavigator();
                                // PPS: Warning here may confuse customers.. leave it out for now
                                 if (!urlnav)
                                 {
//                                     GFx::LogState* plog = static_cast<ASVM&>(GetVM()).GetLogState();
//                                     if (plog) plog->LogWarning("navigateToURL failed! UrlNavigator state is not installed.");
                                     return true;
                                 }

                                urlnav->NavigateToUrl(ptextFmt->GetUrl());
                            }
                        }
                    }
                }
            }
        }

        return true;
    }
    return false; 
}

void AvmTextField::OnLinkEventEx(TextField::LinkEvent event, unsigned pos, unsigned controllerIndex)
{ 
    if (GetAVM()->ExtensionsEnabled)
    {
        TextField* ptf       = GetTextField();
        const Text::TextFormat* ptextFmt;
        if (ptf->GetTextAndParagraphFormat(&ptextFmt, NULL, pos))
        {
            if (ptextFmt->IsUrlSet())
            {
                const char* url = ptextFmt->GetUrl();

                const char* evtSyn = NULL;
                switch (event)
                {
                case TextField::Link_rollover:  evtSyn = "linkMouseOver";   break;
                case TextField::Link_rollout:   evtSyn = "linkMouseOut";    break;
                default: return;
                }

                // fire the link event
                const ASString& evtName = GetAS3Root()->GetStringManager()->CreateString(evtSyn);
                Instances::fl_display::DisplayObject* as3obj = GetAS3Obj();
                if (as3obj != NULL)
                {
                    if (as3obj->WillTrigger(evtName, false) || as3obj->WillTrigger(evtName, true))
                    {
                        SPtr<Instances::fl_gfx::TextEventEx> evtEx;
                        Value params[] = { Value(evtName), Value(true), Value(true) };
                        ASVM* asvm = GetAS3Root()->GetAVM();
                        asvm->ConstructInstance(evtEx, asvm->TextEventExClass, 3, params);
                        SF_ASSERT(evtEx);
                        evtEx->Target = as3obj;
                        evtEx->SetText(GetAS3Root()->GetStringManager()->CreateString(url));
                        evtEx->SetControllerIdx(controllerIndex);

                        as3obj->Dispatch(evtEx, GetDispObj());
                    }
                }
            }
        }
    }
}

bool AvmTextField::OnCharEvent(wchar_t wcharCode, unsigned controllerIdx)
{
    SF_ASSERT(wcharCode);
    return OnEditorInsertingText(GetTextField()->GetCaretIndex(), 1, &wcharCode, controllerIdx);
}

void AvmTextField::OnEventLoad()
{
    // should it be here still? For sprites it was moved in AS3Constructor
    GetAvmParent()->InsertChildToPlayList(GetIntObj());
    GetIntObj()->ModifyOptimizedPlayList();

    AvmInteractiveObj::OnEventLoad();
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

bool    AvmTextField::IsTabable() const
{
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) 
        return false;
    if (GetIntObj()->IsTabEnabledFlagDefined())
        return GetIntObj()->IsTabEnabledFlagTrue();
    else if (GetIntObj()->GetTabIndex() > 0)
        return true;
    return !GetTextField()->IsReadOnly();
}

void AvmTextField::NotifyChanged()
{
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_change);
    Instances::fl_display::DisplayObject* as3obj = GetAS3Obj();
    // check both capture and regular events, this event bubbles.
    if (as3obj && (as3obj->WillTrigger(evtName, false) || as3obj->WillTrigger(evtName, true)))
    {
        SPtr<Instances::fl_events::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
}

void AvmTextField::OnScroll()
{
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_scroll);
    Instances::fl_display::DisplayObject* as3obj = GetAS3Obj();
    // scroll event doesn't bubble
    if (as3obj && as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::fl_events::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
}

bool AvmTextField::OnEditorInsertingText(UPInt pos, UPInt len, const wchar_t* wstr, unsigned controllerIdx)
{
    SF_UNUSED3(pos, len, wstr);

    bool rv = true;

    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_textInput);
    Instances::fl_display::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj != NULL)
    {
        if (as3obj->WillTrigger(evtName, false) || as3obj->WillTrigger(evtName, true))
        {
            SPtr<Instances::fl_events::TextEvent> evt;
            Value params[] = { Value(evtName), Value(true), Value(true) };
            ASVM* asvm = GetAS3Root()->GetAVM();
            asvm->ConstructInstance(evt, asvm->ExtensionsEnabled ? asvm->TextEventExClass : asvm->TextEventClass, 3, params);
            SF_ASSERT(evt);
            evt->Target = as3obj;
            evt->SetText(wstr);

            if (asvm->ExtensionsEnabled)
            {
                Instances::fl_gfx::TextEventEx* evtExt = static_cast<Instances::fl_gfx::TextEventEx*>(evt.GetPtr());
                evtExt->SetControllerIdx(controllerIdx);
            }

            rv = as3obj->Dispatch(evt, GetDispObj());
        }
    }
    return rv;
}
bool AvmTextField::OnEditorRemovingText(UPInt pos, UPInt len, unsigned controllerIdx)
{
    SF_UNUSED3(pos, len, controllerIdx);
    return true;
}

void AvmTextField::ProceedImageSubstitution(VM& vm, int idx, const Value& ve)
{
    if (ve.IsObject())
    {
        TextField* ptextField = GetTextField();
        Instances::fl::Object* peobj = static_cast<Instances::fl::Object*>(ve.GetObject());
        SF_ASSERT(peobj);
        Value val;
        Text::DocView::ImageSubstitutor* pimgSubst = ptextField->CreateImageSubstitutor();
        if (!pimgSubst)
            return;
        Text::DocView::ImageSubstitutor::Element isElem;

        AS3::Multiname mn(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("subString"));
        if (peobj->GetProperty(mn, val))
        {
            val.ToStringValue(vm.GetStringManager()).DoNotCheck();
            ASString str = ASString(val.GetStringNode());
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

        AS3::Multiname mn1(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("image"));
        if (peobj->GetProperty(mn1, val))
        {
            if (vm.IsOfType(val, "flash.display.BitmapData", vm.GetCurrentAppDomain()))
            {
                Instances::fl_display::BitmapData* pbmpData = static_cast<Instances::fl_display::BitmapData*>(val.GetObject());
                ImageResource* pimgRes = pbmpData->GetImageResource();
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
        AS3::Multiname mn2(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("width"));
        if (peobj->GetProperty(mn2, val))
        {
            Value::Number v;
            val.Convert2Number(v).DoNotCheck();
            screenWidth = (float)PixelsToTwips(v);
        }
        AS3::Multiname mn3(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("height"));
        if (peobj->GetProperty(mn3, val))
        {
            Value::Number v;
            val.Convert2Number(v).DoNotCheck();
            screenHeight = (float)PixelsToTwips(v);
        }
        AS3::Multiname mn4(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("baseLineX"));
        if (peobj->GetProperty(mn4, val))
        {
            Value::Number v;
            val.Convert2Number(v).DoNotCheck();
            baseLineX = (float)PixelsToTwips(v);
        }
        AS3::Multiname mn5(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("baseLineY"));
        if (peobj->GetProperty(mn5, val))
        {
            Value::Number v;
            val.Convert2Number(v).DoNotCheck();
            baseLineY = (float)PixelsToTwips(v);
        }
        else
        {
            // if baseLineY is not specified, then use (origHeight - 1) pts instead
            baseLineY = origHeight - PixelsToTwips(1.0f);
        }
        AS3::Multiname mn6(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("id"));
        StringBuffer sb;
        if (peobj->GetProperty(mn6, val))
        {
            val.Convert2String(sb).DoNotCheck();
            idStr = sb.ToCStr();
        }

        isElem.pImageDesc = *SF_HEAP_AUTO_NEW(ptextField) Text::ImageDesc;
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

}}} // SF::GFx::AS3

