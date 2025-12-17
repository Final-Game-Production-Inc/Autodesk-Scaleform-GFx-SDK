/**************************************************************************

Filename    :   GFx_TextField.cpp
Content     :   Dynamic and input text field character implementation
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_TextField.h"
#include "Render/Text/Text_LineBuffer.h"
#include "GFx/Text/Text_StyleSheet.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Render/Render_TreeLog.h"
#include "GFx/GFx_ASUtils.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "Render/Render_TreeText.h"
#include "Kernel/SF_Range.h"

namespace Scaleform {
namespace GFx {


TextField::TextField(TextFieldDef* def, 
                     MovieDefImpl *pbindingDefImpl, 
                     ASMovieRootBase* pasRoot,
                     InteractiveObject* parent, 
                     ResourceId id)
:
    InteractiveObject(pbindingDefImpl, pasRoot, parent, id),
    pDef(def)
{
    SF_ASSERT(pDef);
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    pImageDescAssoc = NULL;
#endif
    if (pbindingDefImpl)
        pBinding = &pbindingDefImpl->GetResourceBinding();
    else
        pBinding = NULL;

    Flags = 0;
    FocusedControllerIdx = ~0u;

    //TextColor   = def->Color;
    Alignment   = def->Alignment;
    SetPassword(def->IsPassword());
    SetHtml(def->IsHtml());
    SetMouseWheelEnabled(true);
    pShadow     = 0;

    MovieImpl* proot = pasRoot->GetMovieImpl();

    Ptr<Text::Allocator> ptextAllocator = proot->GetTextAllocator();

    // First of all, search for a font manager associated with the specified MovieDefImpl
    // Second, if not found, try to get it from one of the parents.
    // Finally, get the first available one in proot and pray it will work (actually,
    // never should get that far).
    FontManager* fontMgr = proot->FindFontManager(pbindingDefImpl);
    if (!fontMgr && parent)
        fontMgr = parent->GetFontManager();
    if (!fontMgr)
        fontMgr = proot->FindFontManager();
    SF_ASSERT(fontMgr);

    pDocument = 
        *SF_HEAP_NEW(proot->GetMovieHeap()) Text::DocView(ptextAllocator, fontMgr, GetLog());
    pDocument->SetDocumentListener(&TextDocListener);
    pDocument->GetStyledText()->SetNewLine0D(); // Flash uses '\x0D' ('\r', #13) as a new-line char
    pDocument->EnableBidirectionalText();

    if (def->IsBorder())
    {
        SetBackgroundColor(Color(255, 255, 255, 255));
        SetBorderColor(Color(0, 0, 0, 255));
    }

    SetInitialFormatsAsDefault();

    if (def->MaxLength > 0)
        pDocument->SetMaxLength(UPInt(def->MaxLength));

    pDocument->SetViewRect(def->TextRect);
    if (def->DoesUseDeviceFont())
    {
        pDocument->SetUseDeviceFont();
        pDocument->SetAAForReadability();
    }
    if (def->IsMultiline())
        pDocument->SetMultiline();
    else
        pDocument->ClearMultiline();
    const bool  autoSizeByX = (def->IsAutoSize() && !(def->IsWordWrap() && def->IsMultiline()));
    const bool  autoSizeByY = (def->IsAutoSize() /*&& Multiline*/);
    if (autoSizeByX)
        pDocument->SetAutoSizeX();
    if (autoSizeByY)
        pDocument->SetAutoSizeY();
    if (def->IsWordWrap())
        pDocument->SetWordWrap();
    if (IsPassword())
        pDocument->SetPasswordMode();
    if (def->IsAAForReadability())
        pDocument->SetAAForReadability();
    if (!IsReadOnly() || def->IsSelectable())
    {
        CreateEditorKit();
    }
    SetNeedUpdateLayoutFlag();
#ifdef SF_AMP_SERVER
    if (pDef->IsAAForReadability())
    {
        AmpServer::GetInstance().IncrementFontOptRead();
    }
#endif
}

TextField::~TextField()
{
#ifdef SF_AMP_SERVER
    if (pDef->IsAAForReadability())
    {
        AmpServer::GetInstance().DecrementFontOptRead();
    }
#endif
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    ClearIdImageDescAssoc();
#endif
    pDocument->Close();
    delete pShadow;
}

bool TextField::IsSelectable() const         
{ 
    return (GetEditorKit()) ? 
        GetEditorKit()->IsSelectable() : GetTextFieldDef()->IsSelectable(); 
}

bool TextField::IsReadOnly() const 
{ 
    return (GetEditorKit()) ? 
        GetEditorKit()->IsReadOnly() : GetTextFieldDef()->IsReadOnly(); 
}

void TextField::SetFilters(const FilterSet* filters)
{    
    if (!filters)
        return;
    
    pFilters = const_cast<FilterSet*>(filters); // only for AddRef/Release!
    TextFilter tf;
    for (unsigned i = 0; i< filters->GetFilterCount(); i++)
    {
        const Filter* filter = filters->GetFilter(i);
        if ( filter )
            tf.LoadFilterDesc(filter);
    }

    SetTextFilters(tf);
}

void TextField::SetTextFilters(const TextFilter& f)
{ 
     if (pDocument)
     {
         if ( pDocument->SetFilters(f) )
            SetDirtyFlag();
     }
}

Double TextField::GetTextWidth() const
{
    return TwipsToPixels(pDocument->GetTextWidth());
}

Double TextField::GetTextHeight() const
{
    return TwipsToPixels(pDocument->GetTextHeight());
}

TextField::CSSHolderBase* TextField::GetCSSData() const 
{ 
#ifdef GFX_ENABLE_CSS
    return pCSSData.GetPtr(); 
#else
    return NULL;
#endif
}

void TextField::SetCSSData(TextField::CSSHolderBase* css)
{
#ifdef GFX_ENABLE_CSS
    pCSSData = css; 
#else
    SF_UNUSED(css);
#endif
}

void TextField::SetDirtyFlag()
{
    //MovieImpl* proot = GetMovieImpl();
    //if (proot)
    //    proot->SetDirtyFlag();
    static_cast<TreeText*>(GetRenderNode())->NotifyLayoutChanged();
}

void TextField::SetSelectable(bool v) 
{ 
#ifdef GFX_ENABLE_TEXT_INPUT
    SF_ASSERT(pDocument);
    Ptr<Text::EditorKit> peditor = GetEditorKit();
    if (v)
    {
        peditor = CreateEditorKit();
        peditor->SetSelectable();
    }
    else
    {
        if (peditor)
            peditor->ClearSelectable();
    }
#else
    SF_UNUSED(v);
#endif //SF_NO_TEXT_INPUT_SUPPORT
}

Ptr<Text::EditorKit> TextField::CreateEditorKit()
{
#ifdef GFX_ENABLE_TEXT_INPUT
    SF_ASSERT(pDocument);
    Ptr<Text::EditorKit> peditor = GetEditorKit();
    if (!peditor)
    {
        peditor = *SF_HEAP_AUTO_NEW(this) Text::EditorKit(pDocument);

        if (GetTextFieldDef()->IsReadOnly())
            peditor->SetReadOnly();
        if (GetTextFieldDef()->IsSelectable())
            peditor->SetSelectable();

        Ptr<Clipboard> pclipBoard = GetMovieImpl()->GetStateBagImpl()->GetClipboard();
        peditor->SetClipboard(pclipBoard);
        Ptr<TextKeyMap> pkeymap = GetMovieImpl()->GetStateBagImpl()->GetTextKeyMap();
        peditor->SetKeyMap(pkeymap);

        if (DoesUseRichClipboard())
            peditor->SetUseRichClipboard();
        else
            peditor->ClearUseRichClipboard();
    }
    return peditor;
#else
    return NULL;
#endif
}

void TextField::GetInitialFormats(Text::TextFormat* ptextFmt, Text::ParagraphFormat* pparaFmt)
{
    ptextFmt->InitByDefaultValues();
    pparaFmt->InitByDefaultValues();

    ResourceBindData fontData;
    bool lookForResource = true;

    // get font params and use them as initial text format params.
    if (GetTextFieldDef()->FontId.GetIdIndex())
    {
        // resolve font name, if possible
        fontData = pBinding->GetResourceData(GetTextFieldDef()->pFont);
    }
    else if (GetTextFieldDef()->FontClass.GetSize() > 0)
    {
        // If font class is used when we need to look through exports and imports
        // to resolve font name.
        Ptr<MovieDefImpl> md = GetResourceMovieDef();
        if (!GetMovieImpl()->FindExportedResource(md, &fontData, GetTextFieldDef()->FontClass))
        {
            // if font class not found as exported resource, try to use it as a font name;
            // this may happen when fontlib is in use.
            ptextFmt->SetFontName(GetTextFieldDef()->FontClass);
            lookForResource = false;
        }
    }
    if (lookForResource && (GetTextFieldDef()->FontId.GetIdIndex() || GetTextFieldDef()->FontClass.GetSize() > 0))
    {
        if (!fontData.pResource)
        {
            Ptr<Log> plog = GetLog();
            if (plog)
            {
                if (GetTextFieldDef()->FontId.GetIdIndex())
                    plog->LogError("Resource for font id = %d is not found in text field id = %d, def text = '%s'", 
                        GetTextFieldDef()->FontId.GetIdIndex(), GetId().GetIdIndex(), GetTextFieldDef()->DefaultText.ToCStr());
                else 
                    plog->LogError("Resource for font class = '%s' is not found in text field id = %d, def text = '%s'", 
                        GetTextFieldDef()->FontClass.ToCStr(), GetId().GetIdIndex(), GetTextFieldDef()->DefaultText.ToCStr());
            }
        }
        else if (fontData.pResource->GetResourceType() != Resource::RT_Font)
        {
            Ptr<Log> plog = GetLog();
            if (plog)
            {
                plog->LogError("Font id = %d is referring to non-font resource in text field id = %d, def text = '%s'", 
                    GetTextFieldDef()->FontId.GetIdIndex(), GetId().GetIdIndex(), GetTextFieldDef()->DefaultText.ToCStr());
            }
        }
        else
        {
            FontResource* pfontRes = (FontResource*)fontData.pResource.GetPtr();
            if (pfontRes)
            {
                Font* pfont = pfontRes->GetFont();
                ptextFmt->SetFontName(pfont->GetName());

                // Don't set bold and italic as part of initial format, if HTML is used.
                // Also, don't use font handle, since HTML should to set its own font by name.
                if (!GetTextFieldDef()->IsHtml() ||
                    GetTextFieldDef()->FontClass.GetSize() > 0)
                {
                    ptextFmt->SetBold(pfont->IsBold());
                    ptextFmt->SetItalic(pfont->IsItalic());
                    if (!GetTextFieldDef()->DoesUseDeviceFont() && pfont->IsResolved() )
                    {
                        Ptr<FontHandle> pfontHandle = 
                            *SF_NEW FontHandle(NULL, pfont, 0, 0, fontData.pBinding->GetOwnerDefImpl());
                        ptextFmt->SetFontHandle(pfontHandle);
                    }
                }
            }
        }
    }
    ptextFmt->SetFontSizeInFixp(unsigned(GetTextFieldDef()->TextHeight));
    ptextFmt->SetColor(GetTextFieldDef()->ColorV);

    Text::ParagraphFormat defaultParagraphFmt;
    switch (GetTextFieldDef()->Alignment)
    {
    case TextFieldDef::ALIGN_LEFT: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Left);
        break;
    case TextFieldDef::ALIGN_RIGHT: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Right);
        break;
    case TextFieldDef::ALIGN_CENTER: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Center);
        break;
    case TextFieldDef::ALIGN_JUSTIFY: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Justify);
        break;
    }
    if (GetTextFieldDef()->HasLayout())
    {
        pparaFmt->SetLeftMargin((unsigned)TwipsToPixels(GetTextFieldDef()->LeftMargin));
        pparaFmt->SetRightMargin((unsigned)TwipsToPixels(GetTextFieldDef()->RightMargin));
        pparaFmt->SetIndent((int)TwipsToPixels(GetTextFieldDef()->Indent));
        pparaFmt->SetLeading((int)TwipsToPixels(GetTextFieldDef()->Leading));
    }
}

void TextField::SetInitialFormatsAsDefault()
{
    Text::TextFormat          defaultTextFmt(Memory::GetHeapByAddress(this));
    Text::ParagraphFormat defaultParagraphFmt;

    const Text::TextFormat*          ptextFmt = pDocument->GetDefaultTextFormat();
    const Text::ParagraphFormat* pparaFmt = pDocument->GetDefaultParagraphFormat();

    if (!GetTextFieldDef()->IsEmptyTextDef())
    {
        GetInitialFormats(&defaultTextFmt, &defaultParagraphFmt);
        if (ptextFmt)
            defaultTextFmt      = ptextFmt->Merge(defaultTextFmt);
        if (pparaFmt)
            defaultParagraphFmt = pparaFmt->Merge(defaultParagraphFmt);
    }
    else
    {
        if (ptextFmt)
            defaultTextFmt      = *ptextFmt;
        if (pparaFmt)
            defaultParagraphFmt = *pparaFmt;

        Text::TextFormat          eTextFmt(Memory::GetHeapByAddress(this));
        Text::ParagraphFormat eParagraphFmt;
        eTextFmt.InitByDefaultValues();
        eParagraphFmt.InitByDefaultValues();
        defaultTextFmt      = eTextFmt.Merge(defaultTextFmt);
        defaultParagraphFmt = eParagraphFmt.Merge(defaultParagraphFmt);
    }
    SetDefaultTextFormat(defaultTextFmt);
    SetDefaultParagraphFormat(defaultParagraphFmt);
}

int TextField::CheckAdvanceStatus(bool playingNow)
{
    int rv = 0;
    bool advanceDisabled = IsAdvanceDisabled() || !CanAdvanceChar() || pDef->WasStatic();
    if (!advanceDisabled)
    {
        // Textfield is always in 'playing' status if it has a variable associated.
        bool playing = IsForceOneTimeAdvance() || IsForceAdvance();

        if (!playing)
        {
            // It is playing if there is a blinking cursor. It blinks only if 
            // the textfield is focused.
            // It is also playing if mouse is captured (for performing scrolling)
            MovieImpl* proot = GetMovieImpl();
            Text::EditorKit* peditorKit = GetEditorKit();
            if (IsVisibleFlagSet() && peditorKit && 
                ((peditorKit->HasCursor() && proot->IsFocused(this)) || peditorKit->IsMouseCaptured()))
                playing = true;
        }

        if (playing && !playingNow)
            rv = 1;
        else if (!playing && playingNow)
            rv = -1;
    }
    else if (playingNow)
        rv = -1;

#ifdef GFX_ENABLE_CSS
    if (rv != 1)
    {
        const Text::StyleManager* styleMgr = GetStyleSheet();
        if (styleMgr && (styleMgr->IsLoading() || styleMgr->IsLoadingFinished()))
        {
            // Need to keep AdvanceFrame going until style sheet loading is finished.
            rv = 1;
        }
    }
#endif
    return rv;
}

void TextField::SetStateChangeFlags(UInt8 flags) 
{ 
    InteractiveObject::SetStateChangeFlags(flags);
    SetForceOneTimeAdvance();
    SetReqPartialAdvanceFlag(); // enforce fastest call to AdvanceFrame
    if (!IsInPlayList())
        AddToPlayList();
    ModifyOptimizedPlayListLocal<TextField>();
}

// Override because pGeomData has topLeft offset for TextField
void TextField::UpdateTransform3D()
{
    Matrix3F rotXMat(Matrix3F::NoInit), transMat(Matrix3F::NoInit), scaleMat(Matrix3F::NoInit), rotYMat(Matrix3F::NoInit), rotZMat(Matrix3F::NoInit);

    Render::PointF topLeftOffset = pDocument->GetViewRect().TopLeft();
    transMat = Matrix3F::Translation((float)pGeomData->X - topLeftOffset.x, (float)pGeomData->Y - topLeftOffset.y,(float)pGeomData->Z);
    scaleMat = Matrix3F::Scaling((float)pGeomData->XScale/100.f, (float)pGeomData->YScale/100.f, (float)pGeomData->ZScale/100.f);

    // MOOSE TODO - collapse this into one matrix?
    rotXMat = pGeomData->XRotation != 0.0 ? Matrix3F::RotationX((float)SF_DEGTORAD(pGeomData->XRotation)) : Matrix3F::Identity; 
    rotYMat = pGeomData->YRotation != 0.0 ? Matrix3F::RotationY((float)SF_DEGTORAD(pGeomData->YRotation)) : Matrix3F::Identity; 
    rotZMat =  pGeomData->Rotation != 0.0 ? Matrix3F::RotationZ((float)SF_DEGTORAD(pGeomData->Rotation))  : Matrix3F::Identity; 

    // Apply transforms in SRT order
    Matrix3F m(transMat, Matrix3F(Matrix3F(rotZMat, rotYMat), Matrix3F(rotXMat, scaleMat)));   // operations happen right to left 

    if (m.IsValid())
        SetMatrix3D(m);
}

// Override AdvanceFrame so that variable dependencies can be updated.
void    TextField::AdvanceFrame(bool nextFrame, float framePos)
{
    UInt8 stateFlags        = GetStateChangeFlags();
    const UInt8 stateMask   = DisplayObjectBase::StateChanged_FontLib | DisplayObjectBase::StateChanged_FontMap |
                              DisplayObjectBase::StateChanged_FontProvider | DisplayObjectBase::StateChanged_Translator;
    bool textValueJustChanged = false;
    if (((stateFlags & stateMask) || GetMovieImpl()->AreRegisteredFontsChanged()) && pDocument)
    {
        ClearReqPartialAdvanceFlag(); // was enforced in SetStateChangeFlags
        pDocument->SetCompleteReformatReq();
        SetNeedUpdateLayoutFlag();
        if (stateFlags & DisplayObjectBase::StateChanged_Translator)
        {
            TextDocListener.TranslatorChanged();
            SetTextValue(OriginalTextValue.ToCStr(), IsOriginalHtml());
            textValueJustChanged = true;
        }
    }

    // use parent class' SetStateChangeFlags here to avoid extra calls to 
    // ModifyOptimizedPlayListLocal.
    InteractiveObject::SetStateChangeFlags(stateFlags & (~stateMask));

    if (IsForceOneTimeAdvance())
    {
        ClearForceOneTimeAdvance();
        ModifyOptimizedPlayListLocal<TextField>();
    }

#ifdef GFX_ENABLE_CSS
    // check if stylesheet was loading and loading is finished; re-apply 
    // the stylesheet if so.
    const Text::StyleManager* styleMgr = GetStyleSheet();
    if (styleMgr)
    {
        if (styleMgr->IsLoadingFinished())
        {
            styleMgr->SetReady();
            ModifyOptimizedPlayListLocal<TextField>();
            if (!textValueJustChanged) // avoid setting text if it was already just set above
            {
                SetNeedUpdateLayoutFlag();
                SetTextValue(OriginalTextValue.ToCStr(), IsOriginalHtml());
            }
        }
        else if (styleMgr->IsError())
        {
            styleMgr->SetReady();
            ModifyOptimizedPlayListLocal<TextField>();
        }
    }
#endif

    SF_UNUSED(framePos);
    if (nextFrame)
    {
        if (IsForceAdvance() && HasAvmObject())
            GetAvmTextField()->UpdateTextFromVariable();
        Flags |= Flags_NextFrame;
    }
    else
    {
        Flags &= ~Flags_NextFrame;
    }

    if (pDocument->HasEditorKit())
    {
        MovieImpl* proot = GetMovieImpl();
        Text::EditorKit* peditorKit = GetEditorKit();
        if (proot->IsFocused(this) || peditorKit->IsMouseCaptured())
        {
            Double timer = proot->GetTimeElapsed();
            peditorKit->Advance(timer);
            //if (peditorKit->HasCursor())
            //    proot->SetDirtyFlag(); // because cursor may blink
        }
    }
    if (IsNeedUpdateLayoutFlag())
        SetDirtyFlag();
}

bool TextField::HasStyleSheet() const 
{ 
    if (HasAvmObject())
        return GetAvmTextField()->HasStyleSheet();
    return false;
}

const Text::StyleManager* TextField::GetStyleSheet() const 
{ 
    if (HasAvmObject())
        return GetAvmTextField()->GetStyleSheet();
    return NULL;
}

bool TextField::IsTabable() const
{
    if (pDef->WasStatic())
        return false;
    //return GetVisible() && !IsReadOnly() && !IsTabEnabledFlagFalse();
    if (GetVisible() && !IsReadOnly())
    {
        if (!IsTabEnabledFlagDefined())
        {
            if (HasAvmObject())
                return GetAvmTextField()->IsTabable();
        }
        return !IsTabEnabledFlagFalse();
    }
    return false;
}

// Set our text to the given string. pnewText can have HTML or
// regular formatting based on argument.
bool    TextField::SetTextValue(const char* pnewText, bool html, bool notifyVariable)
{
    // Check if we can avoid setting text (and reformatting / rebuilding layout) if:
    // 1) text that is being set is exactly the same as it was before;
    // 2) default text format was not changed (since default text format may cause
    // different formatting even if text is the same).
    // 3) replace/append functions were not called (they change the actual text without
    // modifying OriginalTextValue)
    if (OriginalTextValue == pnewText && !IsNeedUpdateLayoutFlag())
        return false;
    else
        ClearNeedUpdateLayoutFlag();

    // If CSS StyleSheet is set, then assignment to "text" works the same
    // way as to "htmlText".
    if (HasStyleSheet())
    {
        html = true;
        SetHtml(true);
    }

    OriginalTextValue = pnewText;
    SetOriginalHtml(html);
    // need to update the pnewText pointer, since assignment
    // to OriginalTextValue may cause its reallocation.
    pnewText = OriginalTextValue.ToCStr();

    bool doNotifyChanged = false;
    bool translated = false;
    if (!IsNoTranslate())
    {
        Ptr<Translator> ptrans = GetMovieImpl()->GetTranslator();
        if (ptrans)
        {
            class TranslateInfo : public Translator::TranslateInfo
            {
            public:
                WStringBuffer::Reserve<512> Res1, Res2;
                WStringBuffer ResultBuf;
                WStringBuffer KeyBuf;

                TranslateInfo(const char* instanceName):ResultBuf(Res1), KeyBuf(Res2)
                {
                    pResult       = &ResultBuf;
                    pInstanceName = instanceName;
                }

                void SetKey(const char* pkey)
                {
                    int nchars = (int)UTF8Util::GetLength(pkey);
                    KeyBuf.Resize(nchars + 1);
                    UTF8Util::DecodeStringSafe(KeyBuf.GetBuffer(), nchars + 1, pkey);
                    pKey = KeyBuf.GetBuffer();
                }
                void    SetKey() { pKey = KeyBuf.GetBuffer(); }

                bool    IsResultHtml() const { return (Flags & Flag_ResultHtml) != 0; }
                bool    IsTranslated() const { return (Flags & Flag_Translated) != 0; }

                void    SetKeyHtml() { Flags |= Flag_SourceHtml; }
            } translateInfo((!HasInstanceBasedName()) ? GetName().ToCStr() : "");

            if (!html || ptrans->CanReceiveHtml())
            {
                // convert html or plain text to unicode w/o parsing
                translateInfo.SetKey(pnewText);
                if (html)
                    translateInfo.SetKeyHtml();
                ptrans->Translate(&translateInfo);
            }
            else
            {
                // if html, but need to pass plain text
                pDocument->ParseHtml(pnewText, SF_MAX_UPINT, IsCondenseWhite(), NULL, GetStyleSheet(), NULL, NULL);
                pDocument->GetStyledText()->GetText(&translateInfo.KeyBuf);
                if (ptrans->NeedStripNewLines())
                    translateInfo.KeyBuf.StripTrailingNewLines();
                translateInfo.SetKey();
                ptrans->Translate(&translateInfo);
            }
            translated = translateInfo.IsTranslated();
            if (translated)
            {
                if (translateInfo.IsResultHtml())
                {
                    Text::TextFormat txtFmt(Memory::GetHeapByAddress(this));
                    Text::ParagraphFormat paraFmt;
                    GetInitialFormats(&txtFmt, &paraFmt);  

                    // Looks like Flash ignores default text format when html is set on
                    // timeline-created textfield; instead, the initial format is used.
                    if (!IsTimelineObjectFlagSet())
                    {
                        txtFmt = txtFmt.Merge(*GetDefaultTextFormat());
                        paraFmt = paraFmt.Merge(*GetDefaultParagraphFormat());
                    }

                    Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(this));
                    pDocument->ParseHtml(translateInfo.ResultBuf.ToWStr(), SF_MAX_UPINT, IsCondenseWhite(), 
                        &imageInfoArray, GetStyleSheet(), &txtFmt, &paraFmt);
                    if (imageInfoArray.GetSize() > 0)
                    {
                        ProcessImageTags(imageInfoArray);
                    }
                }
                else
                {
                    // Attempt to keep original formatting: get the format of the old text's first letter
                    // and apply to whole new text. This should prevent from losing formatting (bold, italic, etc)
                    // after translation.
                    const Text::TextFormat*         ptextFmt;
                    const Text::ParagraphFormat*    pparaFmt;
                    pDocument->GetTextAndParagraphFormat(&ptextFmt, &pparaFmt, 0);
                    pDocument->SetDefaultTextFormat(ptextFmt);
                    pDocument->SetDefaultParagraphFormat(pparaFmt);

                    pDocument->SetText(translateInfo.ResultBuf.ToWStr());
                }
                doNotifyChanged = true;
            }
        }
    }

    if (!translated)
    {
        if (html)
        {
            Text::TextFormat txtFmt(Memory::GetHeapByAddress(this));
            Text::ParagraphFormat paraFmt;
            GetInitialFormats(&txtFmt, &paraFmt);                

            // Looks like Flash ignores default text format when html is set on
            // timeline-created textfield; instead, the initial format is used.
            if (!IsTimelineObjectFlagSet())
            {
                txtFmt = txtFmt.Merge(*GetDefaultTextFormat());
                paraFmt = paraFmt.Merge(*GetDefaultParagraphFormat());
            }

            Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(this));
            pDocument->ParseHtml(pnewText, SF_MAX_UPINT, IsCondenseWhite(), &imageInfoArray, 
                GetStyleSheet(), &txtFmt, &paraFmt);
            if (imageInfoArray.GetSize() > 0)
            {
                ProcessImageTags(imageInfoArray);
            }
        }
        else
        {
            pDocument->SetText(pnewText);        
        }
    }

    if (pDocument->HasEditorKit())
    {
        if (!IsReadOnly()/* || IsSelectable()*/)
        {
            //!AB, this causes single line text scrolled to the end when text is assigned and
            // it is longer than the visible text field.
            // This is not happening in Flash, so at the moment just comment
            // this out.
            //if (!pDocument->IsMultiline())
            //    GetEditorKit()->SetCursorPos(pDocument->GetLength(), false);
            //else
            //    GetEditorKit()->SetCursorPos(0, false);
            UPInt docLen = pDocument->GetLength();
            if (GetEditorKit()->GetCursorPos() > docLen)
                GetEditorKit()->SetCursorPos(docLen, false);
        }
    }

    if (HasStyleSheet() && pDocument->MayHaveUrl())
    {
        // collect info about active zones for each URL
        CollectUrlZones();
    }
    else
    {
        // URLZones could already exist, need to flush them...
        ClearUrlZones();
    }

    // update variable, if necessary
    if (notifyVariable && HasAvmObject())
        GetAvmTextField()->UpdateVariable();

    //!AB MaxLength is available only for InputText; though, if
    // you set text by variable the maxlength is ignored.
    if (doNotifyChanged)
        NotifyChanged();

    SetNeedUpdateGeomData(); // it will call SetDirtyFlag.
    return true;
}

// Special method for complex objects API (see Value::ObjectInterface::SetDisplayInfo)
Render::PointF    TextField::TransformToTextRectSpace(const Value::DisplayInfo& info) const
{
    const Matrix& m = GetMatrix();
    Render::PointF     p(info.IsFlagSet(Value::DisplayInfo::V_x) ? (float)PixelsToTwips(info.GetX()) : pGeomData->X, 
        info.IsFlagSet(Value::DisplayInfo::V_y) ? (float)PixelsToTwips(info.GetY()) : pGeomData->Y);
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.x         -= r.x;
    p.y         -= r.y;
    p           = m.Transform(p);
    p.x         = TwipsToPixels(p.x);
    p.y         = TwipsToPixels(p.y);
    return p;
}


// Special code for complex objects API (see Value::ObjectInterface::SetPositionInfo)
void    TextField::GetPosition(Value::DisplayInfo* pinfo)
{
    GeomDataType geomData;
    UpdateAndGetGeomData(&geomData);
    Double x = TwipsToPixels(Double(geomData.X));
    Double y = TwipsToPixels(Double(geomData.Y));
    Double rotation = geomData.Rotation;
    Double xscale = geomData.XScale;
    Double yscale = geomData.YScale;
    Double alpha = GetCxform().M[0][3] * 100.F;
    bool visible = GetVisible();
    Double zscale = geomData.ZScale;
    Double z = geomData.Z;
    Double xrotation = geomData.XRotation;
    Double yrotation = geomData.YRotation;
    pinfo->Set(x, y, rotation, xscale, yscale, alpha, visible, z, xrotation, yrotation, zscale);
}

ASString    TextField::GetText(bool reqHtml) const 
{
    ASStringManager* pmgr = GetStringManager();
    if (reqHtml)
    {
        if (HasStyleSheet())
        {
            // return original html if CSS is set
            return pmgr->CreateString(OriginalTextValue);
        }
        else
        {
            // For non-HTML field, non-HTML text is returned.
            if (IsHtml())
                return pmgr->CreateString(pDocument->GetHtml());
            else
                return pmgr->CreateString(pDocument->GetText());
        }
    }
    else
    {                   
        return pmgr->CreateString(pDocument->GetText());
    }
}

ASString    TextField::GetHtml() const 
{
    ASStringManager* pmgr = GetStringManager();
    return pmgr->CreateString(pDocument->GetHtml());
}

// Return the topmost entity that the given point covers.  NULL if none.
// I.E. check against ourself.
DisplayObjectBase::TopMostResult TextField::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr)
{
    pdescr->pResult = NULL;
    if (pDef->WasStatic() || IsHitTestDisableFlagSet() || !GetVisible())
        return TopMost_FoundNothing;

    if (pdescr->pIgnoreMC == this)
        return TopMost_FoundNothing;

    Render::PointF localPt;
    TransformPointToLocal(&localPt, pt);

    DisplayObject* pmask = GetMask();  
    if (pmask)
    {
        if (pmask->IsUsedAsMask() && !pmask->IsUnloaded())
        {
            Render::PointF pp;
            if (!pmask->Is3D())
            {
                Matrix2F matrix;
                matrix.SetInverse(pmask->GetWorldMatrix());
                matrix *= GetWorldMatrix();
                pp = matrix.Transform(localPt);
            }
            else
            {
                Matrix3F view;
                Matrix4F proj; 
                Matrix3F world = pmask->GetWorldMatrix3D();
                Render::ScreenToWorld & stw = GetMovieImpl()->ScreenToWorld;

                if (pmask->GetProjectionMatrix3D(&proj))
                    stw.SetProjection(proj);

                if (pmask->GetViewMatrix3D(&view))
                    stw.SetView(view);

                stw.SetWorld(world);
                stw.GetWorldPoint(&pp);
            }

            if (!pmask->PointTestLocal(pp, HitTest_TestShape))
            {
                pdescr->pResult = NULL;
                return TopMost_FoundNothing;
            }
        }
    }

    if ((ClipDepth == 0) && PointTestLocal(localPt, HitTest_TestShape))
    {
        if (pdescr->TestAll || IsSelectable())
        {
            pdescr->pResult = this;
            return TopMost_Found;
        }
        else if (!IsSelectable() && IsHtml() && 
            pDocument->MayHaveUrl() && pDocument->IsUrlAtPoint(localPt.x, localPt.y))
        {
            // if not selectable, will need to check for url under the
            // mouse cursor. Return "this", if there is an url under 
            // the mouse cursor.
            pdescr->pResult = this;
            return TopMost_Found;
        }
        else
        {
            InteractiveObject* pparent = GetParent();
            while (pparent && pparent->IsSprite())
            {
                // Parent sprite would have to be in button mode to grab attention.
                Sprite * psprite = pparent->CharToSprite_Unsafe();
                if (pdescr->TestAll || psprite->ActsAsButton())
                {
                    // Check if sprite should be ignored
                    if (!pdescr->pIgnoreMC || (psprite != pdescr->pIgnoreMC))
                    {
                        pdescr->pResult = psprite;
                        return TopMost_Found;
                    }
                }
                pparent = pparent->GetParent ();
            }
        }
    }

    pdescr->LocalPt = localPt;
    pdescr->pResult = NULL;
    return TopMost_Continue;
}

unsigned    TextField::GetCursorType() const 
{
    if (IsHandCursor())
        return MouseCursorEvent::HAND;
    if (IsSelectable())
        return MouseCursorEvent::IBEAM;
    return InteractiveObject::GetCursorType();
}

bool TextField::IsUrlUnderMouseCursor(unsigned mouseIndex, Render::PointF* pPnt, Scaleform::Range* purlRangePos)
{
    // Local coord of mouse IN PIXELS.
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return false;
    SF_ASSERT(proot->GetMouseState(mouseIndex));
    Render::PointF a = proot->GetMouseState(mouseIndex)->GetLastPosition();

    Matrix  m = GetWorldMatrix();
    Render::PointF b;

    m.TransformByInverse(&b, a);
    if (pPnt) *pPnt = b;

    return pDocument->IsUrlAtPoint(b.x, b.y, purlRangePos);
}

bool TextField::IsUrlTheSame(unsigned mouseIndex, const Scaleform::Range& urlRange)
{
    bool rv = true;
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
        {
            if (pCSSData->UrlZones[i].Intersects(urlRange))
            {
                if (pCSSData->MouseState[mouseIndex].UrlZoneIndex != i + 1)
                {
                    rv = false;
                    break;
                }
            }
        }
    }
#else
    SF_UNUSED2(mouseIndex, urlRange);
#endif
    return rv;
}

void TextField::CollectUrlZones()
{
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        memset(pCSSData->MouseState, 0, sizeof(pCSSData->MouseState));
        pCSSData->UrlZones.RemoveAll();
        Text::StyledText* pstyledText = pDocument->GetStyledText();
        String currentUrl;
        UPInt startPos = 0, length = 0;
        unsigned n = pstyledText->GetParagraphsCount();
        for (unsigned i = 0; i < n; ++i)
        {
            const Text::Paragraph* ppara = pstyledText->GetParagraph(i);
            Text::Paragraph::FormatRunIterator it = ppara->GetIterator();
            for (; !it.IsFinished(); ++it)
            {
                UPInt indexInDoc = (UPInt)it->Index + ppara->GetStartIndex();
                if (it->pFormat->IsUrlSet())
                {
                    if (!currentUrl.IsEmpty())
                    {
                        if (indexInDoc != startPos + length || currentUrl != it->pFormat->GetUrl())
                        {
                            CSSHolderBase::UrlZone urlZone;
                            urlZone.SavedFmt = 
                                *pDocument->GetStyledText()->CopyStyledText(startPos, startPos + length);
                            pCSSData->UrlZones.InsertRange(startPos, length, urlZone);
                            currentUrl.Clear();
                        }
                        else
                        {
                            length += it->Length;
                        }
                    }
                    if (currentUrl.IsEmpty())
                    {
                        startPos = indexInDoc;
                        length   = it->Length;
                        currentUrl = it->pFormat->GetUrl();
                    }
                }
            }
        }
        if (!currentUrl.IsEmpty())
        {
            CSSHolderBase::UrlZone urlZone;
            urlZone.SavedFmt = 
                *pDocument->GetStyledText()->CopyStyledText(startPos, startPos + length);
            pCSSData->UrlZones.InsertRange(startPos, length, urlZone);
        }
    }
#endif // GFX_ENABLE_CSS
}

void TextField::ClearUrlZones()
{
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        memset(pCSSData->MouseState, 0, sizeof(pCSSData->MouseState));
        pCSSData->UrlZones.RemoveAll();
    }
#endif // GFX_ENABLE_CSS
}

void    TextField::PropagateMouseEvent(const EventId& id)
{
    MovieImpl* proot = GetMovieImpl();
    if (!proot || pDef->WasStatic())
        return;

    if (id.Id == EventId::Event_MouseMove)
    {
        // Implement mouse-drag.
        InteractiveObject::DoMouseDrag(id.MouseIndex);
    }

    Ptr<InteractiveObject> ptopMostChar = proot->GetMouseState(id.MouseIndex)->GetTopmostEntity();
    bool notifyChanged = false;

    // if ptopMostChar != thus - means that the cursor is out of the URL => restore URL fmt
    if ((ptopMostChar != this || id.Id == EventId::Event_RollOut) && 
        HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
    {
        notifyChanged = ChangeUrlFormat(Link_release, id.MouseIndex) || notifyChanged;
        notifyChanged = ChangeUrlFormat(Link_rollout, id.MouseIndex) || notifyChanged;
    }

    if (ptopMostChar == this ||
        (pDocument->HasEditorKit() && GetEditorKit()->IsMouseCaptured()))
    {
        if (!GetVisible())  
            return;
        switch(id.Id)
        {
        case EventId::Event_MouseDown:
            {
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Scaleform::Range urlRange;
                    if (IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange) && 
                        pCSSData && pCSSData->HasASStyleSheet() && (pms->GetButtonsState() & MouseState::MouseButton_Left))
                    {
                        // apply CSS style - a:active
                        notifyChanged = ChangeUrlFormat(Link_press, id.MouseIndex, &urlRange) || notifyChanged;
                    }
                }
#endif // GFX_ENABLE_CSS
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseDown(b.x, b.y, pms->GetButtonsState());

                    if (!IsInPlayList())
                        AddToPlayList();
                    ModifyOptimizedPlayListLocal<TextField>();
                }
            }
            break;
        case EventId::Event_MouseUp:
        case EventId::Event_ReleaseOutside:
            {
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Scaleform::Range urlRange;
                    if (IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange) && 
                        pCSSData && pCSSData->HasASStyleSheet() && !(pms->GetButtonsState() & MouseState::MouseButton_Left))
                    {
                        // apply CSS style - a:active
                        notifyChanged = ChangeUrlFormat(Link_release, id.MouseIndex, &urlRange) || notifyChanged;
                    }
                    notifyChanged = ChangeUrlFormat(Link_release, id.MouseIndex) || notifyChanged;
                }
#endif // GFX_ENABLE_CSS
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseUp(b.x, b.y, pms->GetButtonsState());

                    if (!IsInPlayList())
                        AddToPlayList();
                    ModifyOptimizedPlayListLocal<TextField>();
                }
            }
            break;
        case EventId::Event_MouseMove:
            {
                const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                SF_ASSERT(pms);
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseMove(b.x, b.y);
                }
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    Scaleform::Range urlRange;
                    bool urlIsUnderCursor = IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange);

                    // apply CSS style - either a:active or a:hover
                    if (urlIsUnderCursor)
                    {
                        if (!IsUrlTheSame(id.MouseIndex, urlRange))
                        {
                            notifyChanged = ChangeUrlFormat(Link_release, id.MouseIndex) || notifyChanged;
                            notifyChanged = ChangeUrlFormat(Link_rollout, id.MouseIndex) || notifyChanged;
                        }
                        notifyChanged = ChangeUrlFormat(
                            (pms->GetButtonsState() & MouseState::MouseButton_Left) ? Link_press : Link_rollover, 
                            id.MouseIndex, &urlRange) || notifyChanged;
                    }
                    else
                    {
                        notifyChanged = ChangeUrlFormat(Link_release, id.MouseIndex) || notifyChanged;
                        notifyChanged = ChangeUrlFormat(Link_rollout, id.MouseIndex) || notifyChanged;
                    }
                    SetHandCursor(urlIsUnderCursor);
                    proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                }
                else 
#endif // GFX_ENABLE_CSS
                {
                    if (IsHtml() && pDocument->MayHaveUrl())
                    {
                        SetHandCursor(IsUrlUnderMouseCursor(id.MouseIndex));
                        proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                    }
                    else if (IsHandCursor())
                    {
                        notifyChanged = ChangeUrlFormat(Link_rollout, id.MouseIndex) || notifyChanged;
                        ClearHandCursor();
                        proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                    }
                }
            }
            break;
        default: break;
        }
    }
    if (notifyChanged)
        NotifyChanged();
    InteractiveObject::PropagateMouseEvent(id);
}

// handle Event_MouseDown to grab focus
bool TextField::OnMouseEvent(const EventId& event)
{
    if (pDef->WasStatic())
        return false;
    PropagateMouseEvent(event);
    if (HasAvmObject())
        return GetAvmTextField()->OnMouseEvent(event);
    return false; 
}

bool TextField::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const
{
    if (pDef->WasStatic())
        return false;
    return (FocusedControllerIdx == ~0u || FocusedControllerIdx == controllerIdx) && 
        InteractiveObject::IsFocusAllowed(proot, controllerIdx);
}

bool TextField::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx)
{
    if (pDef->WasStatic())
        return false;
    return (FocusedControllerIdx == ~0u || FocusedControllerIdx == controllerIdx) && 
        InteractiveObject::IsFocusAllowed(proot, controllerIdx);
}


void TextField::OnFocus(FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
                        unsigned controllerIdx, FocusMovedType fmt)
{
    if (pDef->WasStatic())
        return;

    // manage the selection
    if (event == InteractiveObject::SetFocus)
    {
        if (IsSelectable() && !IsNoAutoSelection() && fmt == GFx_FocusMovedByKeyboard)
        {
            UPInt len = pDocument->GetLength();
            SetSelection(0, (SPInt)len);
        }
        FocusedControllerIdx = controllerIdx;
        if (pDocument->HasEditorKit())
        {
            GetEditorKit()->OnSetFocus();
            SetDirtyFlag();

            if (!IsReadOnly())
            {
                VirtualKeyboardInterface* vki = GetMovieImpl()->GetVirtualKeyboardInterface();
                if (vki)
                {
                    RectF worldRect = TwipsToPixels(GetBounds(GetWorldMatrix()));
                    vki->OnInputTextfieldFocusIn(IsMultiline(), worldRect);
                }
            }
        }
    }
    else if (event == InteractiveObject::KillFocus)
    {
        FocusedControllerIdx = ~0u;
        if (pDocument->HasEditorKit())
        {
            if (IsSelectable() && !DoesAlwaysShowSelection())
                GetEditorKit()->ClearSelection();

            GetEditorKit()->OnKillFocus();
            SetDirtyFlag();

            if (!IsReadOnly())
            {
                VirtualKeyboardInterface* vki = GetMovieImpl()->GetVirtualKeyboardInterface();
                if (vki)
                    vki->OnInputTextfieldFocusOut();
            }
        }
    }

    if (!IsReadOnly() || IsSelectable())
    {
        if (event == InteractiveObject::SetFocus)
        {
            ResetBlink(1, 1);
        }
        else
        {
            ResetBlink(0, 0);        
        }

        InteractiveObject::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);

        if (pDocument->HasEditorKit() && GetEditorKit()->HasCursor())
        {
            SetForceOneTimeAdvance();
            if (!IsInPlayList())
                AddToPlayList();
            ModifyOptimizedPlayListLocal<TextField>();
        }
    }
    else if (event == InteractiveObject::KillFocus || IsFocusEnabled(fmt)) 
    {
        InteractiveObject::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }
}

bool TextField::OnLosingKeyboardFocus(InteractiveObject*, unsigned , FocusMovedType fmt) 
{
    if (fmt == GFx_FocusMovedByMouse)
    {
        //!AB: Not sure, is this case still actual or not.
        // Though, this "if" may lead to incorrect behavior if the text field
        // hides itself in onPress handler: mouseCaptured flag won't be cleared
        // since no OnMouseUp will be received.
        //if (pDocument->HasEditorKit() && GetEditorKit()->IsMouseCaptured())
        //    return false; // prevent focus loss, if mouse is captured
    }
    return true;
}

RectF  TextField::GetBounds(const Matrix &t) const
{
    return t.EncloseTransform(pDocument->GetViewRect());
}

bool    TextField::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{
    if (IsHitTestDisableFlagSet())
        return false;
    if ((hitTestMask & HitTest_IgnoreInvisible) && !GetVisible())
        return false;
    // Flash only uses the bounding box for text - even if shape flag is used
    return pDocument->GetViewRect().Contains(pt);
}

void TextField::ResetBlink(bool state, bool blocked)
{
    if (pDocument->HasEditorKit())  
    {
        if (GetEditorKit()->HasCursor())
        {
            GetEditorKit()->ResetBlink(state, blocked);
            SetDirtyFlag();
        }
    }
}

void TextField::SetSelection(SPInt beginIndex, SPInt endIndex)
{
#ifdef GFX_ENABLE_TEXT_INPUT
    // Flash allows do selection in read-only non-selectable textfields.
    // To reproduce the same behavior we need to create EditorKit, even, 
    // though read-only non-selectable textfield doesn't need it.
    if (!pDocument->HasEditorKit())
        CreateEditorKit();
    if (pDocument->HasEditorKit())
    {
        beginIndex = Alg::Max((SPInt)0, beginIndex);
        endIndex   = Alg::Max((SPInt)0, endIndex);
        UPInt len = pDocument->GetLength();
        beginIndex = Alg::Min((SPInt)len, beginIndex);
        endIndex   = Alg::Min((SPInt)len, endIndex);
        GetEditorKit()->SetSelection((UPInt)beginIndex, (UPInt)endIndex);
        SetDirtyFlag();
    }
#else
    SF_UNUSED2(beginIndex, endIndex);
#endif //SF_NO_TEXT_INPUT_SUPPORT
}

SPInt TextField::GetCaretIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    if (!IsReadOnly() || IsSelectable())
    {
        if (pDocument->HasEditorKit())
        {
            return (SPInt)GetEditorKit()->GetCursorPos();
        }
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return -1;
}

UPInt TextField::GetBeginIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    //if ((!IsReadOnly() || IsSelectable()) && pDocument->HasEditorKit())
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetBeginSelection();
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return 0;
}

UPInt TextField::GetEndIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    //if ((!IsReadOnly() || IsSelectable()) && pDocument->HasEditorKit())
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetEndSelection();
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return 0;
}

void TextField::OnEventLoad()
{
    // finalize the initialization. We need to initialize text here rather than in ctor
    // since the name of instance is not set yet in the ctor and setting text might cause
    // Translator to be invoked, which uses the instance name.
    if (pDef->DefaultText.GetLength() > 0)
    {
        SetTextValue(pDef->DefaultText.ToCStr(), IsHtml(), false);
    }
    else
    {
        // This assigns both Text and HtmlText vars.
        SetTextValue("", IsHtml(), false);
    }

//     if (HasAvmObject())
//         GetAvmTextField()->OnEventLoad();
    pDocument->Format();

    InteractiveObject::OnEventLoad();

    // invoked when textfield is being added into displaylist;
    // we need to modify GeomData to reflect correct X and Y coords.
    // It is incorrect to take X and Y coordinates directly from the matrix
    // as it usually happens for movieclip. The matrix of textfields 
    // contains some offset (usually, 2 pixels by both axis), because TextRect's
    // topleft corner is usually (-2, -2), not (0,0).
    // Thus, we need to recalculate X and Y received from the matrix with using 
    // TextRect topleft corner.
    if (!pGeomData)
    {
        GeomDataType geomData;
        UpdateAndGetGeomData(&geomData, true);
    }
}

void TextField::OnEventUnload()
{
    SetUnloading();
#ifndef SF_NO_IME_SUPPORT
    // text field is unloading, so we need to make sure IME was finalized
    // for this text field.
    MovieImpl* proot = GetMovieImpl();
    if (proot)
    {
        Ptr<IMEManagerBase> pIMEManager = proot->GetIMEManager();
        if (pIMEManager && pIMEManager->IsTextFieldFocused(this))
        {
            pIMEManager->DoFinalize();
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    SetDirtyFlag();
    InteractiveObject::OnEventUnload();
}

DisplayObjectBase::GeomDataType& TextField::UpdateAndGetGeomData(GeomDataType* pgeomData, bool force)
{
    SF_ASSERT(pgeomData);
    GetGeomData(*pgeomData);
    if (force || (Flags & Flags_NeedUpdateGeomData) != 0)
    {
        RectF rr   = pDocument->GetViewRect();
        Matrix m    = GetMatrix();
        Render::PointF p   = rr.TopLeft();

        // same as m.Transform(p), but with two exception:
        // all calculations are being done in doubles.
        Double x = Double(m.Sx()) * p.x + Double(m.Shx()) * p.y + Double(m.Tx());
        Double y = Double(m.Shy()) * p.x + Double(m.Sy()) * p.y + Double(m.Ty());

        pgeomData->X = Alg::IRound(x);
        pgeomData->Y = Alg::IRound(y);
        SetGeomData(*pgeomData);
        Flags &= ~Flags_NeedUpdateGeomData;
    }
    return *pgeomData;
}

#ifdef GFX_ENABLE_KEYBOARD
bool TextField::OnKeyEvent(const EventId& id, int* pkeyMask)
{
#ifdef SF_BUILD_DEBUG
    if (id.KeysState.IsScrollToggled())
    {
        switch(id.KeyCode)
        {
        case 19: // pause
            {
                pDocument->GetLineBuffer().Dump();
                break;
            }
        default: break;
        }
    }
#endif
    SF_UNUSED(pkeyMask);
    if (pDef->WasStatic())
        return false;

    MovieImpl* proot = GetMovieImpl();
    unsigned focusGroup     = proot->GetFocusGroupIndex(id.KeyboardIndex);
    if (!((*pkeyMask) & ((1 << focusGroup) & InteractiveObject::KeyMask_FocusedItemHandledMask)))
    {
        if (proot && proot->IsFocused(this, id.KeyboardIndex))
        {
            if (HasAvmObject())
                GetAvmTextField()->OnKeyEvent(id, pkeyMask);
            if (pDocument->HasEditorKit() && (!IsReadOnly() || IsSelectable()))
            {
                if (id.Id == EventId::Event_KeyDown)
                {
                    GetEditorKit()->OnKeyDown
                        ((int)id.KeyCode, id.KeysState.States);

                    // toggle overwrite/insert modes
                    if (id.KeyCode == Key::Insert)
                    {
                        SetOverwriteMode(!IsOverwriteMode());
                    }
                }
                else if (id.Id == EventId::Event_KeyUp)
                {
                    GetEditorKit()->OnKeyUp
                        ((int)id.KeyCode, id.KeysState.States);
                }
            }
            (*pkeyMask) |= ((1 << focusGroup) & InteractiveObject::KeyMask_FocusedItemHandledMask);
            return true;
        }
    }
    return false;
}

bool TextField::OnCharEvent(UInt32 wcharCode, unsigned controllerIdx)
{
    if (pDef->WasStatic())
        return false;

    // cursor positioning is available for selectable readonly textfields
    MovieImpl* proot = GetMovieImpl();
    if (proot && pDocument->HasEditorKit() && (!IsReadOnly() || IsSelectable()) && 
        FocusedControllerIdx == controllerIdx)
    {
        SF_ASSERT(HasAvmObject());
        if (GetAvmTextField()->OnCharEvent((wchar_t)wcharCode, controllerIdx))
            GetEditorKit()->OnChar(wcharCode);
    }
    return true;
}

void  TextField::SetOverwriteMode(bool overwMode)
{
    if (pDocument->HasEditorKit())
    {
        if (overwMode)
            GetEditorKit()->SetOverwriteMode();
        else
            GetEditorKit()->ClearOverwriteMode();
    }
}
bool TextField::IsOverwriteMode() const
{
    if (pDocument->HasEditorKit())
        return GetEditorKit()->IsOverwriteMode();
    return false;
}

void  TextField::SetWideCursor(bool wideCursor)
{
    if (pDocument->HasEditorKit())
    {
        if (wideCursor)
            GetEditorKit()->SetWideCursor();
        else
            GetEditorKit()->ClearWideCursor();
        SetDirtyFlag();
    }
}
#else
void TextField::SetOverwriteMode(bool) {}
void TextField::SetWideCursor(bool)    {}
bool TextField::IsOverwriteMode() const { return false; }
#endif //#ifdef GFX_ENABLE_KEYBOARD

// Special event handler; mouse wheel support
bool TextField::OnMouseWheelEvent(int mwDelta)
{ 
    SF_UNUSED(mwDelta); 
    // AS3 allows mouse wheel scroll on non-selectable textfields.
    // In AS2 mouseWheelEnabled should be set to false anyway.
    if (IsMouseWheelEnabled()/* && IsSelectable()*/)
    {
        int  vscroll = (int)pDocument->GetVScrollOffset();
        vscroll -= mwDelta;
        if (vscroll < 0)
            vscroll = 0;
        if (vscroll > (int)pDocument->GetMaxVScroll())
            vscroll = (int)pDocument->GetMaxVScroll();
        pDocument->SetVScrollOffset((unsigned)vscroll);
        SetDirtyFlag();
        return true;
    }
    return false; 
}

#ifdef GFX_AS2_ENABLE_BITMAPDATA
void TextField::AddIdImageDescAssoc(const char* idStr, Text::ImageDesc* pdesc)
{
    if (!pImageDescAssoc)
        pImageDescAssoc = SF_HEAP_AUTO_NEW(this) StringHashLH<Ptr<Text::ImageDesc> >;
    pImageDescAssoc->Set(idStr, pdesc);
}
void TextField::RemoveIdImageDescAssoc(const char* idStr)
{
    if (pImageDescAssoc)
    {
        pImageDescAssoc->Remove(idStr);
    }
}
void TextField::ClearIdImageDescAssoc()
{
    delete pImageDescAssoc;
    pImageDescAssoc = NULL;
}
#else //SF_NO_FXPLAYER_AS_BITMAPDATA
void TextField::AddIdImageDescAssoc(const char*, Text::ImageDesc*) {}
void TextField::RemoveIdImageDescAssoc(const char*) {}
void TextField::ClearIdImageDescAssoc() {}
#endif //SF_NO_FXPLAYER_AS_BITMAPDATA


#if !defined(SF_NO_FXPLAYER_AS_BITMAPDATA)
void TextField::ProcessImageTags(Text::StyledText::HTMLImageTagInfoArray& imageInfoArray)
{
    SF_UNUSED(imageInfoArray);

    UPInt n = imageInfoArray.GetSize();

    if (n > 0)
        SetDirtyFlag();

    for (UPInt i = 0; i < n; ++i)
    {
        Text::StyledText::HTMLImageTagInfo& imgTagInfo = imageInfoArray[i];
        SF_ASSERT(imgTagInfo.pTextImageDesc);
        // TODO: Url should be used to load image by the same way as loadMovie does.
        // At the moment there is no good way to get a callback when movie is loaded.
        // Therefore, at the moment, Url is used as export name only. "imgps://" also
        // may be used.

        ResourceBindData resBindData;
        Ptr<MovieDefImpl> md = GetResourceMovieDef();
        if (md) // can it be NULL?
        {
            // check for user protocol img:// and imgps://
            bool userImageProtocol = LoaderImpl::IsProtocolImage(imgTagInfo.Url);
            if (!userImageProtocol)
            {
                if (!GetMovieImpl()->FindExportedResource(md, &resBindData, imgTagInfo.Url))
                {
                    if (GetLog())
                        GetLog()->LogWarning
                        ("ProcessImageTags: can't find a resource for export name '%s'\n", imgTagInfo.Url.ToCStr());
                    continue;
                }
                SF_ASSERT(resBindData.pResource.GetPtr() != 0);
            }
            // Must check resource type, since users can theoretically pass other resource ids.
            if (userImageProtocol || resBindData.pResource->GetResourceType() == Resource::RT_Image)
            {
                Ptr<Render::Image> img;
                // bitmap
                if (!userImageProtocol)
                {
                    Ptr<ImageResource> pimgRes = GetMovieImpl()->GetImageResourceByLinkageId(GetResourceMovieDef(), imgTagInfo.Url);
                    if (!pimgRes)
                    {
                        if (GetLog())
                            GetLog()->LogWarning
                            ("ProcessImageTags: can't load the image '%s'\n", imgTagInfo.Url.ToCStr());
                        continue;
                    }
                    if (pimgRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
                        img = static_cast<Render::Image*>(pimgRes->GetImage());
                    else
                    {
                        if (!md->GetImageCreator())
                            LogDebugMessage(Log_Warning, "ImageCreator is null in ProcessImageTags");
                        else
                        {
                            ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, Memory::GetHeapByAddress(this));
                            img = *md->GetImageCreator()->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimgRes->GetImage()));
                        }
                    }
                }
                else
                {
                    MemoryHeap* pimageHeap = md->GetWeakLib()->GetImageHeap();
                    ImageCreateInfo cinfo(ImageCreateInfo::Create_FileImage, pimageHeap, 0);
                    cinfo.SetStates(md->GetLog(), static_cast<FileOpener*>(md->GetFileOpener().GetPtr()), md->GetImageFileHandlerRegistry());
                    cinfo.pMovie = GetMovieImpl();

                    ImageCreator*     pcreator = md->GetImageCreator();
                    if (pcreator)
                        img = *pcreator->LoadProtocolImage(cinfo, imgTagInfo.Url);
                    else
                        LogDebugMessage(Log_Warning, "Image resource creation failed - ImageCreator not installed");        
                }
                if (!img)
                {
                    LogDebugMessage(Log_Warning, "Image '%s' wasn't created in ProcessImageTags", imgTagInfo.Url.ToCStr());        
                    continue;
                }
                Render::ImageRect dimr = img->GetRect();
                imgTagInfo.pTextImageDesc->pImage = img;
                float origWidth = (float)dimr.Width(), origHeight = (float)dimr.Height();
                float screenWidth = 0, screenHeight = 0;
                if (imgTagInfo.Width)
                    screenWidth = float(imgTagInfo.Width);
                else
                    screenWidth  = (float)PixelsToTwips(dimr.Width());

                if (imgTagInfo.Height)
                    screenHeight = float(imgTagInfo.Height);
                else
                    screenHeight = (float)PixelsToTwips(dimr.Height());

                float baseLineY = PixelsToTwips(origHeight);
                baseLineY += imgTagInfo.VSpace;
                imgTagInfo.pTextImageDesc->ScreenWidth  = screenWidth;
                imgTagInfo.pTextImageDesc->ScreenHeight = screenHeight;
                imgTagInfo.pTextImageDesc->BaseLineY    = TwipsToPixels(baseLineY);
                imgTagInfo.pTextImageDesc->Matrix.AppendTranslation(0, -imgTagInfo.pTextImageDesc->BaseLineY);
                imgTagInfo.pTextImageDesc->Matrix.AppendScaling(screenWidth/origWidth, screenHeight/origHeight);

                pDocument->SetCompleteReformatReq();
            }
            else if (resBindData.pResource->GetResourceType() == Resource::RT_SpriteDef)
            {
                // sprite
                //SpriteDef* psprRes = (SpriteDef*)resBindData.pResource.GetPtr();
            }
        }
    }
}
#else
void TextField::ProcessImageTags(Text::StyledText::HTMLImageTagInfoArray&) 
{
    SF_DEBUG_WARNING(1, "<IMG> tags are not supported due to defined SF_NO_FXPLAYER_AS_BITMAPDATA.\n");
}
#endif //#if !defined(SF_NO_FXPLAYER_AS_BITMAPDATA)

void TextField::ReplaceText(const wchar_t* ptext, UPInt beginPos, UPInt endPos, UPInt textLen)
{
    textLen = pDocument->ReplaceText(ptext, beginPos, endPos, textLen);
    if (pDocument->HasEditorKit())
    {
        // Replace text does not change the cursor position, unless
        // this position doesn't exist anymore.
        UPInt newLen = pDocument->GetLength();
        if (GetEditorKit()->GetCursorPos() > newLen)
            GetEditorKit()->SetCursorPos(newLen, false);
    }
    if (HasAvmObject())
        GetAvmTextField()->UpdateVariable();
    SetNeedUpdateLayoutFlag();
    NotifyChanged();
}

RectF TextField::GetCursorBounds(UPInt cursorPos, float* phscroll, float* pvscroll)
{
    if (pDocument->HasEditorKit())
    {
        Text::EditorKit* peditor = GetEditorKit();
        RectF newCursorRect;
        if (peditor->CalcAbsCursorRectOnScreen(cursorPos, &newCursorRect, NULL, NULL))
        {
            if (phscroll)
                *phscroll = float(pDocument->GetHScrollOffset());
            if (pvscroll)
                *pvscroll = float(pDocument->GetVScrollOffset());
            return newCursorRect;
        }
    }
    if (phscroll)
        *phscroll = 0;
    if (pvscroll)
        *pvscroll = 0;
    return RectF();
}


#ifndef SF_NO_IME_SUPPORT
void TextField::SetCandidateListFont(Sprite* psprite)
{
	const Text::ParagraphFormat*	pparaFormat;
	const Text::TextFormat*			ptextFormat;
	Ptr<FontHandle>					pcandFontHandle;
	pDocument->GetTextAndParagraphFormat(&ptextFormat, &pparaFormat, GetCursorPos());
	String fontList = ptextFormat->GetFontList();
	bool bold = ptextFormat->IsBold();
	bool italic = ptextFormat->IsItalic();
	bool device = false;
	unsigned matchFontFlags = ((bold) ? Font::FF_Bold : 0) |
		((italic) ? Font::FF_Italic : 0) |
		((device) ? Font::FF_DeviceFont : 0);
	DisplayObjectBase* pparent = GetParent();
	if (pparent)
	{
		FontManager* pfontManager = pparent->GetFontManager();
		Ptr<FontHandle> ptmp = *pfontManager->CreateFontHandle(fontList, matchFontFlags);
		
        if (ptmp)
        {
            pcandFontHandle = *SF_HEAP_NEW(psprite->GetMovieImpl()->GetMovieHeap()) 
                FontHandle(NULL, ptmp->GetFont(), GFX_CANDIDATELIST_FONTNAME, 0, 
                ((ptmp->pSourceMovieDef) ? ptmp->pSourceMovieDef : pfontManager->GetDefImpl()));
            psprite->SetIMECandidateListFont(pcandFontHandle);
        }
	}
}

FontResource* TextField::GetFontResource()
{
	if (!pDef->FontId.GetIdIndex())
        return NULL;
    // resolve font name, if possible
    ResourceBindData fontData = pBinding->GetResourceData(pDef->pFont);
    if (!fontData.pResource)
    {
        Ptr<Log> plog = GetLog();
        if (plog)
        {
            plog->LogError("Resource for font id = %d is not found in text field id = %d, def text = '%s'", 
                           pDef->FontId.GetIdIndex(), GetId().GetIdIndex(), pDef->DefaultText.ToCStr());
        }
    }
    else if (fontData.pResource->GetResourceType() != Resource::RT_Font)
    {
        Ptr<Log> plog = GetLog();
        if (plog)
        {
            plog->LogError("Font id = %d is referring to non-font resource in text field id = %d, def text = '%s'", 
                           pDef->FontId.GetIdIndex(), GetId().GetIdIndex(), pDef->DefaultText.ToCStr());
        }
    }
    else
    {
        FontResource* pfont = (FontResource*)fontData.pResource.GetPtr();
        return pfont;
    }
    return NULL;
}

void TextField::CreateCompositionString()
{
    if (pDocument->HasEditorKit())
    {
		Ptr<Text::CompositionString> cs = GetEditorKit()->CreateCompositionString();
        // need to grab styles for IME compos/clause/etc
/*       
		if (ASTextFieldObj)
        {
            Text::IMEStyle* pcsStyles = ASTextFieldObj->GetIMECompositionStringStyles();
            if (pcsStyles)
                cs->UseStyles(*pcsStyles);
        }
*/
        cs->SetText(L"");
        cs->SetPosition(GetEditorKit()->GetCursorPos());
        SetDirtyFlag();
    }
}

void TextField::ClearCompositionString()
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
            cs->SetText(L"");

        SetDirtyFlag();
    }
}

void TextField::ReleaseCompositionString()
{
    if (pDocument->HasEditorKit())
    {
        GetEditorKit()->ReleaseCompositionString();
        SetDirtyFlag();
    }
}

void TextField::CommitCompositionString(const wchar_t* pstr, UPInt len)
{
    if (pDocument->HasEditorKit())
    {
		Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            if (!pstr)
            {
                pstr = cs->GetText();
                len  = cs->GetLength();
            }
            else if (len == SF_MAX_UPINT)
            {
                len = SFwcslen(pstr);
            }
            // check for max length
            UPInt docLen = pDocument->GetLength();
            UPInt maxLen;
            if (pDocument->HasMaxLength())
                maxLen = pDocument->GetMaxLength();
            else
                maxLen = SF_MAX_UPINT;

            GetEditorKit()->SetCursorPos(cs->GetPosition(), false);
            for (UPInt i = 0; i < len && docLen < maxLen; ++i)
            {
                if (GetEditorKit()->OnChar(pstr[i]))
                    ++docLen;
            }

            cs->SetText(L"");
            cs->SetPosition(GetEditorKit()->GetCursorPos());
            if (HasAvmObject())
                GetAvmTextField()->UpdateVariable();
            NotifyChanged();
        }
    }
}
const wchar_t* TextField::GetCompositionString()
{
	Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
	if (cs)
	{
		return cs->GetText();
	}
	return NULL; 
}

void TextField::SetCompositionStringText(const wchar_t* pstr, UPInt len)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetText(pstr, len);
            SetDirtyFlag();
        }
    }
}

void TextField::SetCompositionStringPosition(UPInt pos)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetPosition(pos);
            SetDirtyFlag();
        }
    }
}

UPInt TextField::GetCompositionStringPosition() const
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            return cs->GetPosition();
        }
    }
    return SF_MAX_UPINT;
}

UPInt TextField::GetCompositionStringLength() const
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            return cs->GetLength();
        }
    }
    return 0;
}

void TextField::SetCursorInCompositionString(UPInt pos)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetCursorPosition(pos);
            SetDirtyFlag();
        }
    }
}

void TextField::HighlightCompositionStringText(UPInt posInCompStr, UPInt len, Text::IMEStyle::Category hcategory)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->HighlightText(posInCompStr, len, hcategory);
            SetDirtyFlag();
        }
    }
}
#endif //#ifdef SF_NO_IME_SUPPORT

UInt32 TextField::GetSelectedTextColor() const
{
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetActiveSelectionTextColor();
    }
    return GFX_ACTIVE_SEL_TEXTCOLOR;
}

UInt32 TextField::GetSelectedBackgroundColor() const
{
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetActiveSelectionBackgroundColor();
    }
    return GFX_ACTIVE_SEL_BKCOLOR;
}

void TextField::UpdateUrlStyles()
{
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
        {
            if (pCSSData->HasASStyleSheet())
            {
                const Text::Style* pAstyle = 
                    pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, "a");

                const Text::Style* pALinkStyle = 
                    pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, "a:link");

                Text::TextFormat fmt(Memory::GetHeapByAddress(this));
                if (pAstyle)
                    fmt = fmt.Merge(pAstyle->mTextFormat);
                if (pALinkStyle)
                    fmt = fmt.Merge(pALinkStyle->mTextFormat);

                // save current formatting data first
                UPInt stPos = (UPInt)pCSSData->UrlZones[i].FirstIndex(), endPos = (UPInt)pCSSData->UrlZones[i].NextIndex();

                pDocument->SetTextFormat(fmt, stPos, endPos);

                // Update saved fmt
                pCSSData->UrlZones[i].GetData().SavedFmt = 
                    *pDocument->GetStyledText()->CopyStyledText(stPos, endPos);
            }
        }
    }
#endif
}

bool TextField::ChangeUrlFormat(LinkEvent event, unsigned mouseIndex, const Scaleform::Range* purlRange)
{
#ifdef GFX_ENABLE_CSS
    if (!HasStyleSheet())
        return false;

    bool needsUpdate = false;
    const char* styleName = NULL;

    //CSSHolder::UrlZone* purlZone = NULL;
    RangeData<CSSHolderBase::UrlZone>* purlZone = NULL;
    switch(event)
    {
    case Link_press:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (pCSSData->MouseState[mouseIndex].HitBit)
                return needsUpdate; // this mouse already has hit the url
            pCSSData->MouseState[mouseIndex].HitBit = true;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().HitCount > 0)
            {
                ++purlZone->GetData().HitCount;
                return needsUpdate; // hit counter is not zero - do nothing
            }
        }
        else
        {
            for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
            {
                if (pCSSData->UrlZones[i].Intersects(*purlRange))
                {
                    purlZone = &pCSSData->UrlZones[i];
                    //purlZone->StartPos = (UPInt)pCSSData->UrlZones[i].Index;
                    //purlZone->EndPos = purlZone->StartPos + pCSSData->UrlZones[i].Length;
                    pCSSData->MouseState[mouseIndex].UrlZoneIndex = i + 1;
                    pCSSData->MouseState[mouseIndex].HitBit = true;
                    break;
                }
            }
        }
        if (purlZone)
        {
            if (purlZone->GetData().HitCount++ == 0)
            {
                styleName = "a:active";
            }
            else
                return needsUpdate;
        }
        break;
    case Link_release:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (!pCSSData->MouseState[mouseIndex].HitBit)
                return needsUpdate; // this mouse didn't hit the url
            pCSSData->MouseState[mouseIndex].HitBit = false;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            if (!pCSSData->MouseState[mouseIndex].OverBit) // if neither hit nor over left - clean up
                pCSSData->MouseState[mouseIndex].UrlZoneIndex = 0;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().HitCount > 0)
            {
                --purlZone->GetData().HitCount;
                if (purlZone->GetData().HitCount != 0)
                    return needsUpdate; // hit count is still bigger than zero, return
                if (purlZone->GetData().OverCount > 0)
                    styleName = "a:hover";
            }
        }
        break;
    case Link_rollover:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (pCSSData->MouseState[mouseIndex].OverBit)
                return needsUpdate; // this mouse already has rolled over the url
            pCSSData->MouseState[mouseIndex].OverBit = true;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().OverCount > 0)
            {
                ++purlZone->GetData().OverCount;

                if (HasAvmObject()) GetAvmTextField()->OnLinkEventEx(event, (unsigned)purlZone->FirstIndex(), mouseIndex);
                return needsUpdate; // over counter is not zero - do nothing
            }
        }
        else
        {
            for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
            {
                if (pCSSData->UrlZones[i].Intersects(*purlRange))
                {
                    purlZone = &pCSSData->UrlZones[i];
                    //purlZone->StartPos = (UPInt)pCSSData->UrlZones[i].Index;
                    //purlZone->EndPos = purlZone->StartPos + pCSSData->UrlZones[i].Length;
                    pCSSData->MouseState[mouseIndex].UrlZoneIndex = i + 1;
                    pCSSData->MouseState[mouseIndex].OverBit = true;

                    if (HasAvmObject()) GetAvmTextField()->OnLinkEventEx(event, (unsigned)purlZone->FirstIndex(), mouseIndex);
                    break;
                }
            }
        }
        if (purlZone)
        {
            if (purlZone->GetData().HitCount == 0 && purlZone->GetData().OverCount++ == 0)
            {
                styleName = "a:hover";
            }
            else
                return needsUpdate;
        }
        break;
    case Link_rollout:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (!pCSSData->MouseState[mouseIndex].OverBit)
                return needsUpdate; // this mouse didn't hit the url
            pCSSData->MouseState[mouseIndex].OverBit = false;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (!pCSSData->MouseState[mouseIndex].HitBit) // if neither hit nor over left - clean up
                pCSSData->MouseState[mouseIndex].UrlZoneIndex = 0;
            if (purlZone->GetData().OverCount > 0)
            {
                --purlZone->GetData().OverCount;

                if (HasAvmObject()) GetAvmTextField()->OnLinkEventEx(event, (unsigned)purlZone->FirstIndex(), mouseIndex);

                if (purlZone->GetData().OverCount != 0)
                    return needsUpdate; // over count is still bigger than zero, return
                if (purlZone->GetData().HitCount > 0)
                    styleName = "a:active";
            }
        }
        break;
    }

    if (purlZone && purlZone->GetData().SavedFmt)
    {
        // restore originally saved format
        UPInt stPos = (UPInt)purlZone->FirstIndex();
        UPInt enPos = (UPInt)purlZone->NextIndex();
        pDocument->RemoveText(stPos, enPos);
        pDocument->InsertStyledText(*purlZone->GetData().SavedFmt.GetPtr(), stPos);
        needsUpdate = true;
    }

    if (styleName)
    {
        // now need to apply a:hover style from CSS, combined with "a:link" and "a" ones
        const Text::Style* pstyle = 
            pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, styleName);
        if (pstyle && purlZone)
        {
            const Text::Style* pAstyle = 
                pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, "a");

            const Text::Style* pALinkStyle = 
                pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, "a:link");

            Text::TextFormat fmt(Memory::GetHeapByAddress(this));
            if (pAstyle)
                fmt = fmt.Merge(pAstyle->mTextFormat);
            if (pALinkStyle)
                fmt = fmt.Merge(pALinkStyle->mTextFormat);
            fmt = fmt.Merge(pstyle->mTextFormat);

            // save current formatting data first
            UPInt stPos = (UPInt)purlZone->FirstIndex(), endPos = (UPInt)purlZone->NextIndex();

            pDocument->SetTextFormat(fmt, stPos, endPos);
            needsUpdate = true;
        }
    }
    return needsUpdate;
#else
    SF_UNUSED3(event, mouseIndex, purlRange);
    return false;
#endif // GFX_ENABLE_CSS
}

void TextField::SetX(Double x)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    // and TransformToTextRectSpace() above.
    if (NumberUtil::IsNaN(x))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(x) || NumberUtil::IsPOSITIVE_INFINITY(x))
        x = 0;
    Matrix m    = GetMatrix();
    Double newX = PixelsToTwips(x);
    // We need to modify the coordinate being set. We need to
    // use TextRect.TopLeft point for this modification. 
    // First of all translate the coordinate to the TextRect coordinate
    // space, do the adjustment by the -TextRect.TopLeft() and translate
    // coordinate back to parent's coordinate space.
    Render::PointF     p((float)newX);
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.x         -= r.x;
    p           = m.Transform(p);
    
    DisplayObjectBase::SetX(TwipsToPixels(p.x));
    if (pGeomData)
        pGeomData->X = Alg::IRound(newX);
}

void TextField::SetY(Double y)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    // and TransformToTextRectSpace() above.
    if (NumberUtil::IsNaN(y))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(y) || NumberUtil::IsPOSITIVE_INFINITY(y))
        y = 0;
    Double newY = PixelsToTwips(y);
    Matrix m    = GetMatrix();
    // We need to modify the coordinate being set. We need to
    // use TextRect.TopLeft point for this modification. 
    // First of all translate the coordinate to the TextRect coordinate
    // space, do the adjustment by the -TextRect.TopLeft() and translate
    // coordinate back to parent's coordinate space.
    Render::PointF     p(0, (float)newY);
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.y         -= r.y;
    p           = m.Transform(p);
    DisplayObjectBase::SetY(TwipsToPixels(p.y));
    if (pGeomData)
        pGeomData->Y = Alg::IRound(newY);
}

Double TextField::GetX() const
{
    GeomDataType geomData;
    const_cast<TextField*>(this)->UpdateAndGetGeomData(&geomData);//? revise
    return TwipsToPixels(Double(geomData.X));
}

Double TextField::GetY() const
{
    GeomDataType geomData;
    const_cast<TextField*>(this)->UpdateAndGetGeomData(&geomData); //? revise
    return TwipsToPixels(Double(geomData.Y));
}

void TextField::SetWidth(Double width)
{
    float newWidth = float(PixelsToTwips(width));
    RectF viewRect = pDocument->GetViewRect();
    viewRect.x2 = viewRect.x1 + newWidth;
    pDocument->SetViewRect(viewRect);
    SetNeedUpdateGeomData();
}

void TextField::SetHeight(Double height)
{
    float newHeight = float(PixelsToTwips(height));
    RectF viewRect = pDocument->GetViewRect();
    viewRect.y2 = viewRect.y1 + newHeight;
    pDocument->SetViewRect(viewRect);
    SetNeedUpdateGeomData();
}

void TextField::SetXScale(Double xscale)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetXScale(xscale);
}

void TextField::SetYScale(Double yscale)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetYScale(yscale);
}

void TextField::SetRotation(Double rotation)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetRotation(rotation);
}

void TextField::SetBackground(bool b)
{
    Color c = GetBackgroundColor();
    if (b)
        c.SetAlpha(255);
    else
        c.SetAlpha(0);
    SetBackgroundColor(c);
    SetDirtyFlag();
}

void TextField::SetBackgroundColor(UInt32 rgb)
{
    pDocument->SetBackgroundColor((GetBackgroundColor32() & 0xFF000000u) | (rgb & 0xFFFFFFu));
    SetDirtyFlag();
}

void TextField::SetBackgroundColor(const Color& c)
{
    pDocument->SetBackgroundColor(c.ToColor32());
    SetDirtyFlag();
}

void TextField::SetBorder(bool b)
{
    Color c = GetBorderColor();
    if (b)
        c.SetAlpha(255);
    else
        c.SetAlpha(0);
    SetBorderColor(c);
    SetDirtyFlag();
}

void TextField::SetBorderColor(const Color& c)
{
    pDocument->SetBorderColor(c.ToColor32());
    SetDirtyFlag();
}

void TextField::SetBorderColor(UInt32 rgb)
{
    pDocument->SetBorderColor((GetBorderColor32() & 0xFF000000u) | (rgb & 0xFFFFFFu));
    SetDirtyFlag();
}

void TextField::SetText(const wchar_t* pnewText, bool reqHtml)
{
    bool currHtml = IsHtml();
    if (reqHtml && !currHtml)        { SetHtml(true); }
    else if (!reqHtml && currHtml)   { SetHtml(false); }

    enum { StackBuffSize = 512 };
    char* pbuff;
    static char stackBuff[StackBuffSize];
    UPInt charSz = SFwcslen(pnewText);

    const UPInt maxExpansion = (sizeof(wchar_t) <= 2) ? 3 : 6;
    UPInt outSz = charSz * maxExpansion + 1;
    pbuff = outSz <= StackBuffSize ? stackBuff : (char*)SF_HEAP_AUTO_ALLOC(this, outSz);

    UTF8Util::EncodeStringSafe(pbuff, outSz, pnewText);

    SetTextValue(pbuff, reqHtml);

    if (pbuff != stackBuff) SF_FREE(pbuff);
}

void TextField::SetTextColor(UInt32 rgb)
{
    // The arg is 0xRRGGBB format.
    Text::TextFormat colorTextFmt(Memory::GetHeapByAddress(this));
    colorTextFmt.SetColor32(rgb);

    pDocument->SetTextFormat(colorTextFmt);

    colorTextFmt = *pDocument->GetDefaultTextFormat();
    colorTextFmt.SetColor32(rgb);
    SetDefaultTextFormat(colorTextFmt);
    SetDirtyFlag();
}

Color TextField::GetTextColor() const
{
    const Text::TextFormat* ptf = pDocument->GetDefaultTextFormat();
    SF_ASSERT(ptf);
    return ptf->GetColor();
}

UInt32 TextField::GetTextColor32() const
{
    Color c = GetTextColor();
    return (((UInt32)c.GetRed()) << 16) | (((UInt32)c.GetGreen()) << 8) | ((UInt32)c.GetBlue());
}

bool TextField::SetShadowStyle(const char *pstr)
{
    if (!pShadow)
        if ((pShadow = SF_HEAP_AUTO_NEW(this) ShadowParams(GetStringManager())) == 0)
            return false;

    // The arg is 0xRRGGBB format.
    UInt32  rgb = pDocument->GetShadowColor();
    pShadow->ShadowColor.SetBlue((UByte)rgb);
    pShadow->ShadowColor.SetGreen((UByte)(rgb >> 8));
    pShadow->ShadowColor.SetRed((UByte)(rgb >> 16));

    // When using the old fasioned shadow we have to disable the new one.
    pDocument->DisableSoftShadow();

reset:
    pShadow->ShadowOffsets.Clear();
    pShadow->TextOffsets.Clear();

    // Parse shadow style text to generate the offset arrays.
    // Format is similar to "s{1,0.5}{-1,0.5}t{0,0}".
    const char *p = pstr;
    ArrayLH<Render::PointF> *offsets = 0;
    while (*p)
    {
        if (*p == 's' || *p == 'S')
        {
            offsets = &pShadow->ShadowOffsets;
            p++;
        }
        else if (*p == 't' || *p == 'T')
        {
            offsets = &pShadow->TextOffsets;
            p++;
        }
        else if (*p == '{' && offsets) // {x,y}
        {
            char pn[24];
            p++;
            const char *q = p;
            while (*q && *q != ',') q++;
            if (!*q || q-p > 23) { pstr = pShadow->ShadowStyleStr.ToCStr(); goto reset; }
            memcpy(pn,p,q-p);
            pn[q-p] = 0;
            float x = (float)(SFstrtod(pn, NULL) * 20.0);
            q++;
            p = q;

            while (*q && *q != '}') q++;
            if (!*q || q-p > 23) { pstr = pShadow->ShadowStyleStr.ToCStr(); goto reset; }
            memcpy(pn,p,q-p);
            pn[q-p] = 0;
            float y = (float)(SFstrtod(pn, NULL) * 20.0);
            q++;
            p = q;

            offsets->PushBack(Render::PointF(x,y));
        }
        else
        {
            pstr = pShadow->ShadowStyleStr.ToCStr();
            goto reset;
        }
    }

    if (*pstr)
        pShadow->ShadowStyleStr = GetStringManager()->CreateString(pstr);
    SetDirtyFlag();
    return true;
}

void TextField::SetShadowColor(UInt32 rgb)
{
    pDocument->SetShadowColor(rgb);
    if (pShadow)
    {
        // The arg is 0xRRGGBB format.
        UInt32 rgbShadow = pDocument->GetShadowColor();
        pShadow->ShadowColor.SetBlue((UByte)rgbShadow);
        pShadow->ShadowColor.SetGreen((UByte)(rgbShadow >> 8));
        pShadow->ShadowColor.SetRed((UByte)(rgbShadow >> 16));
    }
    SetDirtyFlag();
}

ASString TextField::GetShadowStyle() const
{
    return (pShadow ? pShadow->ShadowStyleStr : GetStringManager()->CreateEmptyString());
}


SPInt TextField::GetCursorPos() const
{
    return (HasEditorKit()) ? (SPInt)GetEditorKit()->GetCursorPos() : -1;
}

bool TextField::SetRestrict(const ASString& restrStr)
{
    if (!HasEditorKit())
        CreateEditorKit();
    return GetEditorKit()->SetRestrict(String(restrStr.ToCStr()));
}

const String* TextField::GetRestrict() const
{
    if (HasEditorKit())
        return GetEditorKit()->GetRestrict();
    return NULL;
}

void TextField::ClearRestrict()
{
    if (HasEditorKit())
        GetEditorKit()->ClearRestrict();
}

Ptr<Render::TreeNode> TextField::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeText> tshp = *context.CreateEntry<Render::TreeText>();
    tshp->Init(pDocument);
    return static_cast<Ptr<Render::TreeNode>>(tshp);
}

void TextField::NotifyChanged()
{
    if (HasAvmObject())
        GetAvmTextField()->NotifyChanged();
    SetDirtyFlag();
}

bool TextField::IsFocusEnabled(FocusMovedType fmt) const 
{ 
    if (HasAvmObject())
        return GetAvmTextField()->IsFocusEnabled(fmt);
    return false;
}

//////////////////////////////////////////////////////////////////////////
// TextDocumentListener
//
TextField* TextField::TextDocumentListener::GetTextField() const
{
    return (TextField*)(((UByte*)this) - 
        ((UByte*)&(((TextField*)this)->TextDocListener) - 
        (UByte*)((TextField*)this)) );
}
TextField::TextDocumentListener::TextDocumentListener() : 
DocumentListener(Mask_All & (~Mask_OnLineFormat)) 
{
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans)
    {
        if (ptrans->HandlesCustomWordWrapping())
        {
            HandlersMask |= Mask_OnLineFormat;
        }
        if (ptrans->HandlesBidirectionalText())
        {
            HandlersMask |= Mask_BidirectionalText;
        }
    }
}

void TextField::TextDocumentListener::TranslatorChanged()
{
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans && ptrans->HandlesCustomWordWrapping())
        HandlersMask |= Mask_OnLineFormat;
    else
        HandlersMask &= (~Mask_OnLineFormat);

    if (ptrans && ptrans->HandlesBidirectionalText())
        HandlersMask |= Mask_BidirectionalText;
    else
        HandlersMask &= (~Mask_BidirectionalText);
}

void TextField::TextDocumentListener::OnScroll(Text::DocView&)
{
    TextField* ptextField = GetTextField();
    if (ptextField->HasAvmObject())
        ptextField->GetAvmTextField()->OnScroll();
}

// events
void TextField::TextDocumentListener::View_OnHScroll(Text::DocView& view, unsigned newScroll)
{
    SF_UNUSED(newScroll);
    OnScroll(view);
}
void TextField::TextDocumentListener::View_OnVScroll(Text::DocView& view, unsigned newScroll)
{
    SF_UNUSED(newScroll);
    OnScroll(view);
}
void TextField::TextDocumentListener::View_OnMaxScrollChanged(Text::DocView& view)
{
    OnScroll(view);
}
bool TextField::TextDocumentListener::View_OnLineFormat(Text::DocView&, Text::DocView::LineFormatDesc& desc) 
{ 
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans)
    {
        Translator::LineFormatDesc tdesc;
        tdesc.CurrentLineWidth = TwipsToPixels(desc.CurrentLineWidth);
        tdesc.DashSymbolWidth  = TwipsToPixels(desc.DashSymbolWidth);
        tdesc.LineStartPos     = desc.LineStartPos;
        tdesc.LineWidthBeforeWordWrap = TwipsToPixels(desc.LineWidthBeforeWordWrap);
        tdesc.NumCharsInLine   = desc.NumCharsInLine;
        tdesc.pWidths          = desc.pWidths;
        tdesc.pParaText        = desc.pParaText;
        tdesc.ParaTextLen      = desc.ParaTextLen;
        tdesc.ProposedWordWrapPoint = desc.ProposedWordWrapPoint;
        tdesc.VisibleRectWidth = TwipsToPixels(desc.VisibleRectWidth);
        tdesc.UseHyphenation   = desc.UseHyphenation;
        tdesc.Alignment        = desc.Alignment;
        // it is ok to modify values in the desc.pCharWidths since
        // they are not used by further formatting logic.
        for (UPInt i = 0; i < tdesc.NumCharsInLine; ++i)
            desc.pWidths[i] = TwipsToPixels(desc.pWidths[i]);
        if (ptrans->OnWordWrapping(&tdesc))
        {
            desc.ProposedWordWrapPoint = tdesc.ProposedWordWrapPoint;
            desc.UseHyphenation        = tdesc.UseHyphenation;
            return true;
        }
    }
    return false; 
}

// A callback that is called on bidirectional text enabled textfields. This method
// receives original text ('text'/'textLen' parameters) and pre-allocated buffers: 
//   'newText'  - new re-ordered text should be put there, do not overrun the length (the length is the same as 'textLen')
//   'indexMap' - a buffer for an array of unsigned integers, that should be filled with new indices of each char. For
//                example, if original char at index 0 was relocated to index 10, then indexMap[0] = 10.
//   'mirrorBits'- a buffer for an array of bools (length = 'textLen'), where index is an index of char in 'newText' and the value (true/false)
//                indicates necessity of the glyph mirroring (for example, for integral sign).
// Should return 'true' if method was successful and the core should use contents of the buffers; false, if no changes
// to the textfield should be applied.
bool    TextField::TextDocumentListener::View_PrepareBidiText(Text::DocView&, const wchar_t* paraText, UPInt textLen, 
                                        wchar_t* newParaText, unsigned* indexMap, bool* mirroredBits)
{
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans && ptrans->HandlesBidirectionalText())
    {
        return ptrans->OnBidirectionalText(paraText, textLen, newParaText, indexMap, mirroredBits);
    }
    return false;
}

void    TextField::TextDocumentListener::View_OnChanged(Text::DocView&)    
{
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag();
}

// editor events
void    TextField::TextDocumentListener::Editor_OnChanged(Text::EditorKitBase& editor) 
{ 
    SF_UNUSED(editor); 
    TextField* peditChar = GetTextField();
    peditChar->pDocument->GetText(&peditChar->OriginalTextValue);
    if (peditChar->HasAvmObject())
        peditChar->GetAvmTextField()->UpdateVariable();
    peditChar->NotifyChanged();
}

void    TextField::TextDocumentListener::Editor_OnCursorMoved(Text::EditorKitBase& editor)
{
    SF_UNUSED(editor); 
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag();
}

void    TextField::TextDocumentListener::Editor_OnCursorBlink(Text::EditorKitBase& editor, bool cursorState)
{
    SF_UNUSED(editor);
    SF_UNUSED(cursorState);
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag(); //? will we blink cursor more gracefully?
}

bool TextField::TextDocumentListener::Editor_OnInsertingText(Text::EditorKitBase&, UPInt pos, UPInt len, const wchar_t* wstr)
{
    TextField* peditChar = GetTextField();
    if (peditChar->HasAvmObject())
        return peditChar->GetAvmTextField()->OnEditorInsertingText(pos, len, wstr);
    return true;
}
bool TextField::TextDocumentListener::Editor_OnRemovingText(Text::EditorKitBase&, UPInt pos, UPInt len)
{
    TextField* peditChar = GetTextField();
    if (peditChar->HasAvmObject())
        return peditChar->GetAvmTextField()->OnEditorRemovingText(pos, len);
    return true;
}

String TextField::TextDocumentListener::GetCharacterPath()
{
    String ret;
    GetTextField()->GetAbsolutePath(&ret);
    return ret;
}

static unsigned TextFilterFlags2Mode(UInt8 flags)
{
    unsigned m = 0;
    if (flags & Render::TreeText::FilterFlag_KnockOut)
        m |= Render::BlurFilterParams::Mode_Knockout;
    if (flags & Render::TreeText::FilterFlag_HideObject)
        m |= Render::BlurFilterParams::Mode_HideObject;
    if (flags & Render::TreeText::FilterFlag_FineBlur)
        m |= Render::BlurFilterParams::Mode_Highlight; //????
    return m;
}
const Render::FilterSet* TextField::GetFilters() const
{
    if (GetRenderNode() && !pFilters)
    {
        Render::TreeText* textNode = static_cast<Render::TreeText*>(GetRenderNode());
        Render::TreeText::Filter filters[3];
        UPInt n = textNode->GetFilters(filters, sizeof(filters)/sizeof(filters[0]));
        if (n > 0)
        {
            pFilters = *SF_NEW Render::FilterSet;
            for (UPInt i = 0; i < n; ++i)
            {
                const Render::TreeText::Filter& textFilter = filters[i];
                switch(textFilter.Type)
                {
                case Render::TreeText::Filter_Blur:
                    {
                        Ptr<Render::BlurFilter> f = *SF_NEW Render::BlurFilter(
                            textFilter.Blur.BlurX, textFilter.Blur.BlurY);
                        f->GetParams().Strength = textFilter.Blur.Strength/100.f;
                        pFilters->AddFilter(f);
                    }
                    break;
                case Render::TreeText::Filter_DropShadow:
                    {
                        Ptr<Render::ShadowFilter> f = *SF_NEW Render::ShadowFilter(
                            float(SF_DEGTORAD(textFilter.DropShadow.Angle)), textFilter.DropShadow.Distance,
                            textFilter.DropShadow.BlurX, textFilter.DropShadow.BlurY);
                        f->GetParams().Strength  = textFilter.DropShadow.Strength/100.f;
                        f->GetParams().Colors[0] = textFilter.DropShadow.Color;
                        f->GetParams().Mode = TextFilterFlags2Mode(textFilter.DropShadow.Flags);
                        pFilters->AddFilter(f);
                    }
                    break;
                case Render::TreeText::Filter_Glow:
                    {
                        Ptr<Render::GlowFilter> f = *SF_NEW Render::GlowFilter(
                            textFilter.Glow.BlurX, textFilter.Glow.BlurY);
                        f->GetParams().Strength  = textFilter.Glow.Strength/100.f;
                        f->GetParams().Colors[0] = textFilter.Glow.Color;
                        f->GetParams().Mode = TextFilterFlags2Mode(textFilter.Glow.Flags);
                        pFilters->AddFilter(f);
                    }
                    break;
                default:;
                }
            }
        }
        else
            pFilters = NULL;
        
    }
    return pFilters;
}

}} // namespace Scaleform::GFx
