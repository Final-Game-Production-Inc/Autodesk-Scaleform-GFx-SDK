/**************************************************************************

Filename    :   Text_DocView.cpp
Content     :   Document-view implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Text_DocView.h"
#include "Text_Highlight.h"
#include "Text_WWHelper.h"
#include "Kernel/SF_Alg.h"
#include "Render/Render_Image.h"
#include "Render/Render_TextLayout.h"
#include "Kernel/SF_Log.h"

#define GFX_BULLET_CHAR_CODE        0x2022
#define GFX_BULLET_CHAR_ADVANCE     15
#define GFX_BULLET_PARA_INDENT      20
#define GFX_LINE_ALLOC_DELTA        100
#define GFX_TEXT_JUSTIFY_CORRECTION 30

#define GFX_MIN_LINE_HEIGHT_FOR_AUTOSIZE PixelsToFixp(6)

#ifdef SF_BUILD_DEBUG
//#define DEBUG_CURSOR_POS
//#define GFX_DEBUG_DISPLAY_INVISIBLE_CHARS
#endif //SF_BUILD_DEBUG

namespace Scaleform { 
namespace Render {
namespace Text {

//////////////////////////////////
// DocView
//
DocView::DocView(Allocator* pallocator, FontManagerBase* pfontMgr, Log* plog)
    : pFontManager(pfontMgr), pHighlight(NULL), pLog(plog), BorderColor(0), BackgroundColor(0)
{
    pDocument = *SF_HEAP_AUTO_NEW(this) DocumentText(this, pallocator);
    FormatCounter = 1;
    BeginSelection = EndSelection = SF_MAX_UPINT;
    SetAlignment(Align_Left);
    SetVAlignment(VAlign_None);
    SetTextAutoSize(TAS_None);
    Outline = 0;
    Flags = FlagsEx = RTFlags = 0;
    ViewRect.Clear();
    pImageSubstitutor = NULL;
    SetFontScaleFactor(1);
    MaxLength = 0;

    SetAutoFit(); // on by default
    SetDefaultShadow();
    TextWidth = TextHeight = 0;
}

DocView::~DocView()
{
    Close();
    ClearImageSubstitutor();
    delete pHighlight;
}


struct ImageSubstCmp
{
    struct Comparable
    {
        const wchar_t*  Str;
        UPInt           MaxSize;

        Comparable(const wchar_t* p, UPInt  sz):Str(p), MaxSize(sz) {}
    };
    static int StrCompare(const wchar_t* dst, UPInt  dstlen, const wchar_t* src, UPInt  srclen, bool insertion)
    {
        if (dstlen)
        {
            int f,l;
            int slen = (int)srclen, dlen = (int)dstlen;
            do {
                f = (int)(*(dst++));
                l = (int)(*(src++));
            } while (--dstlen && f && (f == l) && --srclen);

            if (f == l && srclen != 0 && (!insertion || dstlen != 0))
            {
                return dlen - slen;
            }
            return f - l;
        }
        else
            return 0-(int)srclen;
    }
    static int InsLess(const DocView::ImageSubstitutor::Element& p1, const Comparable& p2)
    {
        return (-StrCompare(p2.Str, p2.MaxSize, p1.SubString, p1.SubStringLen, true)) < 0;
    }
    static int Less(const DocView::ImageSubstitutor::Element& p1, const Comparable& p2)
    {
        return (-StrCompare(p2.Str, p2.MaxSize, p1.SubString, p1.SubStringLen, false)) < 0;
    }
    static int Equals(const DocView::ImageSubstitutor::Element& p1, const Comparable& p2)
    {
        return StrCompare(p2.Str, p2.MaxSize, p1.SubString, p1.SubStringLen, false) == 0;
    }
};

void DocView::ImageSubstitutor::AddImageDesc(const DocView::ImageSubstitutor::Element& elem)
{
    if (FindImageDesc(elem.SubString, elem.SubStringLen))
        return; // already added
    /*printf("----------\n");
    for(UPInt i = 0; i < Elements.GetSize(); ++i)
    printf("%S\n", Elements[i].SubString);
    printf("----------\n");*/

    ImageSubstCmp::Comparable e(elem.SubString, elem.SubStringLen);
    UPInt i = Alg::LowerBound(Elements, e, ImageSubstCmp::InsLess);
    Elements.InsertAt(i, elem);
}

ImageDesc* DocView::ImageSubstitutor::FindImageDesc(const wchar_t* pstr, UPInt  maxlen, UPInt * ptextLen)
{
    ImageSubstCmp::Comparable e(pstr, maxlen);
    UPInt i = Alg::LowerBound(Elements, e, ImageSubstCmp::Less);
    if (i < Elements.GetSize() && ImageSubstCmp::Equals(Elements[i], e))
    {
        if (ptextLen) *ptextLen = Elements[i].SubStringLen;
        return Elements[i].pImageDesc;
    }
    return NULL;
}

void DocView::ImageSubstitutor::RemoveImageDesc(ImageDesc* pimgDesc)
{
    for(UPInt i = 0; i < Elements.GetSize();)
    {
        if (Elements[i].pImageDesc == pimgDesc)
            Elements.RemoveAt(i);
        else
            ++i;
    }
}

void DocView::SetText(const char* putf8String, UPInt stringSize)
{
    pDocument->SetText(putf8String, stringSize);
    OnDocumentChanged(ViewNotify_SignificantTextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::SetText(const wchar_t* pstring, UPInt length)
{
    pDocument->SetText(pstring, length);
    OnDocumentChanged(ViewNotify_SignificantTextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::SetTextFormat(const TextFormat& fmt, UPInt startPos, UPInt endPos)
{
    RTFlags &= ~RTFlags_FontErrorDetected; //?
    pDocument->SetTextFormat(fmt, startPos, endPos);
    OnDocumentChanged(ViewNotify_FormatChange);
}

void DocView::SetParagraphFormat(const ParagraphFormat& fmt, UPInt startPos, UPInt endPos)
{
    pDocument->SetParagraphFormat(fmt, startPos, endPos);
    OnDocumentChanged(ViewNotify_FormatChange);
}

void DocView::DocumentText::OnParagraphRemoving(const Paragraph& para)
{
    pDocument->OnDocumentParagraphRemoving(para);
}

void DocView::DocumentText::OnTextInserting(UPInt startPos, UPInt length, const wchar_t* ptextInserting)
{
    SF_UNUSED3(startPos, length, ptextInserting);
    pDocument->OnDocumentChanged(ViewNotify_TextChange);
}

void DocView::DocumentText::OnTextInserting(UPInt startPos, UPInt length, const char* ptextInserting)
{
    SF_UNUSED3(startPos, length, ptextInserting);
    pDocument->OnDocumentChanged(ViewNotify_TextChange);
}

void DocView::DocumentText::OnTextRemoving(UPInt startPos, UPInt length)
{
    SF_UNUSED2(startPos, length);
    pDocument->OnDocumentChanged(ViewNotify_TextChange);
}

UPInt DocView::EditCommand(DocView::CommandType cmdId, const void* command)
{
#ifdef GFX_ENABLE_TEXT_INPUT
    UPInt res = 0;
    switch(cmdId)
    {
    case Cmd_InsertChar:
        {
            SF_ASSERT(command);
            const InsertCharCommand& cmd = *reinterpret_cast<const InsertCharCommand*>(command);
            // check for maxlength
            if (!HasMaxLength() || GetLength() + 1 <= GetMaxLength())
            {
                res = pDocument->InsertString(&cmd.CharCode, cmd.PosAt, 1);
                OnDocumentChanged(ViewNotify_TextChange);
            }
            break;
        }
    case Cmd_InsertPlainText:
        {
            SF_ASSERT(command);
            const InsertPlainTextCommand& cmd = *reinterpret_cast<const InsertPlainTextCommand*>(command);
            UPInt len    = cmd.Length;
            UPInt curLen = GetLength();
            // check for maxlength
            if (HasMaxLength() && curLen + len > GetMaxLength())
                len = GetMaxLength() - curLen;
            res = pDocument->InsertString(cmd.pStr, cmd.PosAt, len, 
                (!IsMultiline()) ? StyledText::NLP_IgnoreCRLF : StyledText::NLP_CompressCRLF);
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_InsertStyledText:
        {
            SF_ASSERT(command);
            const InsertStyledTextCommand& cmd = *reinterpret_cast<const InsertStyledTextCommand*>(command);
            SF_ASSERT(cmd.pText);

            UPInt len = SF_MAX_UPINT;
            // check for maxlength
            if (HasMaxLength())
            {
                UPInt curDstLen = GetLength();   
                UPInt curSrcLen = cmd.pText->GetLength();
                if (curDstLen + curSrcLen > GetMaxLength())
                    len = GetMaxLength() - curDstLen;
            }
            res = pDocument->InsertStyledText(*cmd.pText, cmd.PosAt, len);
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_ReplaceTextByChar:
        {
            SF_ASSERT(command);
            const ReplaceTextByCharCommand& cmd = *reinterpret_cast<const ReplaceTextByCharCommand*>(command);
            UPInt stPos, endPos;
            if (cmd.BeginPos > cmd.EndPos)
            {
                stPos = cmd.EndPos;
                endPos= cmd.BeginPos;
            }
            else
            {
                stPos = cmd.BeginPos;
                endPos= cmd.EndPos;
            }
            UPInt len = 1;                        
            // check for maxlength
            if (HasMaxLength())
            {
                UPInt curDstLen = GetLength();
                UPInt ep = Alg::Clamp(endPos, UPInt(0), curDstLen), 
                      sp = Alg::Clamp(stPos, UPInt(0), curDstLen);
                curDstLen -= (ep - sp);   
                if (curDstLen + 1 > GetMaxLength())
                    len = 0;
            }
            if (len > 0)
            {
                res = pDocument->InsertString(&cmd.CharCode, stPos, 1, (
                    !IsMultiline()) ? StyledText::NLP_IgnoreCRLF : StyledText::NLP_CompressCRLF);
                RemoveText(stPos + 1, endPos + 1);
                OnDocumentChanged(ViewNotify_TextChange);
            }
            break;
        }
    case Cmd_ReplaceTextByPlainText:
        {
            SF_ASSERT(command);
            const ReplaceTextByPlainTextCommand& cmd = *reinterpret_cast<const ReplaceTextByPlainTextCommand*>(command);
            UPInt stPos, endPos;
            if (cmd.BeginPos > cmd.EndPos)
            {
                stPos = cmd.EndPos;
                endPos= cmd.BeginPos;
            }
            else
            {
                stPos = cmd.BeginPos;
                endPos= cmd.EndPos;
            }
            UPInt len = cmd.Length;
            // check for maxlength
            if (HasMaxLength())
            {
                UPInt curDstLen = GetLength();
                UPInt ep = Alg::Clamp(endPos, UPInt(0), curDstLen), 
                      sp = Alg::Clamp(stPos, UPInt(0), curDstLen);
                curDstLen -= (ep - sp);   
                if (curDstLen + cmd.Length > GetMaxLength())
                    len = GetMaxLength() - curDstLen;
            }
            res = pDocument->InsertString(cmd.pStr, stPos, len, 
                (!IsMultiline()) ? StyledText::NLP_IgnoreCRLF : StyledText::NLP_CompressCRLF);
            RemoveText(stPos + res, endPos + res);
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_ReplaceTextByStyledText:
        {
            SF_ASSERT(command);
            const ReplaceTextByStyledTextCommand& cmd = *reinterpret_cast<const ReplaceTextByStyledTextCommand*>(command);
            UPInt stPos, endPos;
            if (cmd.BeginPos > cmd.EndPos)
            {
                stPos = cmd.EndPos;
                endPos= cmd.BeginPos;
            }
            else
            {
                stPos = cmd.BeginPos;
                endPos= cmd.EndPos;
            }
            UPInt len = SF_MAX_UPINT;
            // check for maxlength
            if (HasMaxLength())
            {
                UPInt curDstLen = GetLength();
                UPInt ep = Alg::Clamp(endPos, UPInt(0), curDstLen), 
                      sp = Alg::Clamp(stPos, UPInt(0), curDstLen);
                curDstLen -= (ep - sp);   
                if (curDstLen + cmd.pText->GetLength() > GetMaxLength())
                    len = GetMaxLength() - curDstLen;
            }
            res = pDocument->InsertStyledText(*cmd.pText, stPos, len);
            RemoveText(stPos + res, endPos + res);
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_BackspaceChar:
        {
            SF_ASSERT(command);
            res = 1;
            bool doNotify = false;
            const DeleteCharCommand& cmd = *reinterpret_cast<const DeleteCharCommand*>(command);
            // need to check a special case, if paragraph has indent or bullet and cursor
            // is at the beginning of para - remove these attributes first, before continuing 
            // removing chars
            UPInt indexInPara = 0;
            StyledText::ParagraphsIterator paraIter = 
                pDocument->GetParagraphByIndex(cmd.PosAt, &indexInPara);
            if (!paraIter.IsFinished() && indexInPara == 0)
            {
                Paragraph* ppara = *paraIter;
                SF_ASSERT(ppara);
                const ParagraphFormat* pparaFmt = ppara->GetFormat();
                if (pparaFmt)
                {
                    if (pparaFmt->IsBullet())
                    {
                        ParagraphFormat newFmt = *pparaFmt;
                        newFmt.SetBullet(false);
                        ppara->SetFormat(pDocument->GetAllocator(), newFmt);
                        doNotify = true;
                        res = 0;
                    }
                    else if (pparaFmt->GetIndent() != 0 || pparaFmt->GetBlockIndent() != 0)
                    {
                        ParagraphFormat newFmt = *pparaFmt;
                        newFmt.SetIndent(0);
                        newFmt.SetBlockIndent(0);
                        ppara->SetFormat(pDocument->GetAllocator(), newFmt);
                        doNotify = true;
                        res = 0;
                    }
                }
            }
            if (res == 1 && cmd.PosAt > 0) // res wasn't zeroed - means we need to remove char
            {
                RemoveText(cmd.PosAt - 1, cmd.PosAt);
            }
            else
                res = 0;
            if (doNotify)
                OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_DeleteChar:
        {
            SF_ASSERT(command);
            const DeleteCharCommand& cmd = *reinterpret_cast<const DeleteCharCommand*>(command);
            RemoveText(cmd.PosAt, cmd.PosAt + 1);
            res = 1;
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    case Cmd_DeleteText:
        {
            SF_ASSERT(command);
            const DeleteTextCommand& cmd = *reinterpret_cast<const DeleteTextCommand*>(command);
            UPInt stPos, endPos;
            if (cmd.BeginPos > cmd.EndPos)
            {
                stPos = cmd.EndPos;
                endPos= cmd.BeginPos;
            }
            else
            {
                stPos = cmd.BeginPos;
                endPos= cmd.EndPos;
            }
            //?SetDefaultTextAndParaFormat(stPos);
            RemoveText(stPos, endPos);
            res = endPos - stPos;
            OnDocumentChanged(ViewNotify_TextChange);
            break;
        }
    default: break;
    }
    return res;
#else
    SF_UNUSED2(cmdId, command);
    return 0;
#endif //SF_NO_TEXT_INPUT_SUPPORT
}

String DocView::GetText() const
{
    return pDocument->GetText();
}

String& DocView::GetText(String* retStr) const
{
    return pDocument->GetText(retStr);
}

String DocView::GetHtml() const
{
    return pDocument->GetHtml();
}

String& DocView::GetHtml(String* retStr) const
{
    return pDocument->GetHtml(retStr);
}

StringBuffer& DocView::GetHtml(StringBuffer* retStr) const
{
    return pDocument->GetHtml(retStr);
}

void DocView::ParseHtml(const char* putf8Str, UPInt utf8Len, bool condenseWhite, 
                               StyledText::HTMLImageTagInfoArray* pimgInfoArr,
                               const StyleManagerBase* pstyleMgr,
                               const TextFormat* txtFmt, const ParagraphFormat* paraFmt)
{
    pDocument->Clear();
    if (utf8Len == SF_MAX_UPINT)
        utf8Len = SFstrlen(putf8Str); // actually, this is the SIZE, not length. TODO
    pDocument->ParseHtml(putf8Str, utf8Len, pimgInfoArr, IsMultiline(), condenseWhite, pstyleMgr, txtFmt, paraFmt);
    OnDocumentChanged(ViewNotify_SignificantTextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::ParseHtml(const wchar_t* pwStr, UPInt strLen, bool condenseWhite, 
                               StyledText::HTMLImageTagInfoArray* pimgInfoArr,
                               const StyleManagerBase* pstyleMgr,
                               const TextFormat* txtFmt, const ParagraphFormat* paraFmt)
{
    pDocument->Clear();
    if (strLen == SF_MAX_UPINT)
        strLen = SFwcslen(pwStr); 
    pDocument->ParseHtml(pwStr, strLen, pimgInfoArr, IsMultiline(), condenseWhite, pstyleMgr, txtFmt, paraFmt);
    OnDocumentChanged(ViewNotify_SignificantTextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::AppendText(const wchar_t* pwStr, UPInt strLen)
{
    const TextFormat* ptxtfmt;
    const ParagraphFormat* pparafmt;
    UPInt len = pDocument->GetLength();
    if (len > 0 && pDocument->GetTextAndParagraphFormat(&ptxtfmt, &pparafmt, len - 1))
        pDocument->AppendString(pwStr, strLen, GetNewLinePolicy(), ptxtfmt, pparafmt);
    else
        pDocument->AppendString(pwStr, strLen, GetNewLinePolicy());
    OnDocumentChanged(ViewNotify_TextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::AppendText(const char* putf8Str, UPInt utf8Len)
{
    const TextFormat* ptxtfmt;
    const ParagraphFormat* pparafmt;
    UPInt len = pDocument->GetLength();
    if (len > 0 && pDocument->GetTextAndParagraphFormat(&ptxtfmt, &pparafmt, len - 1))
        pDocument->AppendString(putf8Str, utf8Len, GetNewLinePolicy(), ptxtfmt, pparafmt);
    else
        pDocument->AppendString(putf8Str, utf8Len, GetNewLinePolicy());
    OnDocumentChanged(ViewNotify_TextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::AppendHtml(const wchar_t* pwStr, UPInt strLen, bool condenseWhite, StyledText::HTMLImageTagInfoArray* pimgInfoArr)
{
    if (strLen == SF_MAX_UPINT)
        strLen = SFwcslen(pwStr); 
    pDocument->ParseHtml(pwStr, strLen, pimgInfoArr, IsMultiline(), condenseWhite);
    OnDocumentChanged(ViewNotify_TextChange | ViewNotify_ScrollingParamsChange);
}

void DocView::AppendHtml(const char* putf8Str, UPInt utf8Len, bool condenseWhite, StyledText::HTMLImageTagInfoArray* pimgInfoArr)
{
    if (utf8Len == SF_MAX_UPINT)
        utf8Len = SFstrlen(putf8Str); // actually, this is the SIZE, not length. TODO
    pDocument->ParseHtml(putf8Str, utf8Len, pimgInfoArr, IsMultiline(), condenseWhite);
    OnDocumentChanged(ViewNotify_TextChange | ViewNotify_ScrollingParamsChange);
}

UPInt DocView::InsertText(const wchar_t* pstr, UPInt startPos, UPInt strLen)
{
    return pDocument->InsertString(pstr, startPos, strLen);
}

UPInt DocView::ReplaceText(const wchar_t* pstr, UPInt startPos, UPInt endPos, UPInt strLen)
{
    RemoveText(startPos, endPos);
    UPInt len = pDocument->InsertString(pstr, startPos, strLen);
    //OnDocumentChanged(ViewNotify_TextChange);
    return len;
}

void DocView::Close()
{
    if (pDocument)
    {
        pDocument = NULL;

        SetDocumentListener(NULL);
        pEditorKit = NULL;
        mLineBuffer.ClearLines();
    }
}

bool DocView::ContainsNonLeftAlignment() const
{
    for (unsigned i = 0, n = pDocument->GetParagraphsCount(); i < n; ++i)
    {
        const Paragraph* ppara = pDocument->GetParagraph(i);
        SF_ASSERT(ppara);
        if (ppara->GetFormat()->GetAlignment() != ParagraphFormat::Align_Left)
            return true;
    }
    return false;
}

void DocView::SetViewRect(const RectF& rect, UseType ut)
{
    if (rect != ViewRect)
    {
        unsigned oldW = (unsigned)ViewRect.Width();
        unsigned oldH = (unsigned)ViewRect.Height();
        ViewRect = rect;
        RectF textRect = ViewRect;
        textRect.Contract(GFX_TEXT_GUTTER);
        mLineBuffer.SetVisibleRect(textRect);
        if (ut == UseExternally)
        {
            // if WordWrap is on and width has been changed then we need 
            // reformat. Otherwise, we just need to update the 
            // maxhscroll and maxvscroll values, unless we have set vertical
            // alignment, horizontal alignment other than "left", text auto size is on,
            // or one of the paragraph has alignment other than "left".
            unsigned newW = (unsigned)ViewRect.Width();
            unsigned newH = (unsigned)ViewRect.Height();
            if (GetTextAutoSize() != TAS_None ||

                // check width..
                (oldW != newW && (IsWordWrap() || 
                    // make sure the clipping is correct when reducing width and noclip flag is set
                    (newW < oldW && mLineBuffer.Geom.IsNoClipping()) || 
                    GetAlignment() != Align_Left || ContainsNonLeftAlignment())) ||

                // ... as well as check the height to make sure the correct clipping is used...
                (newH < oldH && mLineBuffer.Geom.IsNoClipping()) ||
                
                // check valign... 
                (GetVAlignment() != VAlign_None && GetVAlignment() != VAlign_Top)
                )
            {
                SetCompleteReformatReq();
            }
            else
            {
                ++FormatCounter; // invalidate cached values to force recalc
                unsigned maxHScroll = GetMaxHScroll();
                if(GetHScrollOffsetValue() > maxHScroll)
                {
                    SetHScrollOffset(maxHScroll);
                }
                unsigned maxVScroll = GetMaxVScroll();
                if(GetVScrollOffsetValue() > maxVScroll)
                {
                    SetVScrollOffset(maxVScroll);
                }
                mLineBuffer.InvalidateCache(); // need to force batches rebuild
            }
        }
    }
}

void DocView::OnDocumentChanged(unsigned notifyMask) 
{
    if (notifyMask & DocView::ViewNotify_SignificantMask)
        SetCompleteReformatReq();
    else
        SetReformatReq();
}

void DocView::OnDocumentParagraphRemoving(const Paragraph& para)
{
    // find lines for this paragraph in LineBuffer and mark them to be removed.
    LineBuffer::Iterator linesIt = mLineBuffer.Begin();
    for(bool found = false; !linesIt.IsFinished(); ++linesIt)
    {
        LineBuffer::Line& curLine = *linesIt;
        if (para.GetId() == curLine.GetParagraphId())
        {
            found = true;
            curLine.SetTextPos(~0u); // indicator of line to be removed
        }
        else if (found)
            break;
    }
    SetReformatReq();
}

FontHandle* DocView::FindFont(FindFontInfo* pfontInfo, bool quietMode)
{
    SF_ASSERT(pfontInfo);
    SF_ASSERT(pfontInfo->pCurrentFormat);

    // check if the font is same as in previous textFormat - don't need to check cache.
    if (!pfontInfo->pCurrentFont || !pfontInfo->pPrevFormat ||
        (!pfontInfo->pCurrentFormat->IsFontSame(*pfontInfo->pPrevFormat)))
    {
        // if font is not the same - check the cache first
        Ptr<FontHandle>* ppcurrentFont = (pfontInfo->pFontCache) ?
            pfontInfo->pFontCache->Get(pfontInfo->pCurrentFormat) : NULL; // no cache installed.
        if (ppcurrentFont)
            pfontInfo->pCurrentFont = *ppcurrentFont;
        else
        {
            if (pfontInfo->pCurrentFormat->IsFontHandleSet())
            {
                // font handle is set in TextFormat if font was referenced by id
                pfontInfo->pCurrentFont = pfontInfo->pCurrentFormat->GetFontHandle();
                //pfontInfo->pCurrentFont->FontScaleFactor = 1.3f;
            }
            else
            {
                // if still not found - resolve the font via FontManager.
                FontManagerBase* pfontMgr   = GetFontManager();
                SF_ASSERT(pfontMgr);

                // font name always should be set in merged text style
                if (pfontInfo->pCurrentFormat->IsFontListSet())
                {
                    pfontInfo->pCurrentFont = *pfontMgr->CreateFontHandle
                        (pfontInfo->pCurrentFormat->GetFontList(), 
                        pfontInfo->pCurrentFormat->IsBold(), 
                        pfontInfo->pCurrentFormat->IsItalic(),
                        DoesUseDeviceFont(), !pfontInfo->pCurrentFormat->IsSingleFontNameSet());

                    //pfontInfo->pCurrentFont->FontScaleFactor = 1.3f;
                }
                if (!pfontInfo->pCurrentFont)
                {
                    // font is not found! do default substitution here (TODO)
                    // if font is still not found, use a default font
                    if (!quietMode && !(RTFlags & RTFlags_FontErrorDetected) && pLog )
                    {
                        //collect information about font search path
                        FontManagerBase::FontSearchPathInfo searchInfo(1);
                        Ptr<FontHandle> h = *pfontMgr->CreateFontHandle(pfontInfo->pCurrentFormat->GetFontList(), 
                            pfontInfo->pCurrentFormat->IsBold(), 
                            pfontInfo->pCurrentFormat->IsItalic(),
                            DoesUseDeviceFont(), !pfontInfo->pCurrentFormat->IsSingleFontNameSet(), &searchInfo);
                        String charPath = GetDocumentListener()? GetDocumentListener()->GetCharacterPath(): String();
                        pLog->LogError("Missing font \"%s\" in \"%s\". Search log:\n%s", 
                            pfontInfo->pCurrentFormat->GetFontList().ToCStr(), charPath.ToCStr(), searchInfo.Info.ToCStr());
                        RTFlags |= RTFlags_FontErrorDetected;
                    }
                    pfontInfo->pCurrentFont = *pfontMgr->GetEmptyFont();
                    SF_ASSERT(pfontInfo->pCurrentFont);
                }
            }
            if (pfontInfo->pFontCache)
            {
                // add to cache
                pfontInfo->pFontCache->Set(pfontInfo->pCurrentFormat, pfontInfo->pCurrentFont);
            }
        }
    }
    pfontInfo->pPrevFormat = pfontInfo->pCurrentFormat;
    return pfontInfo->pCurrentFont;
}

#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
// This iterator iterates through specified 'text' of length 'len' in reverse order.
// Formatting info is retrieved from 'paragraph' using 'indexMap'. 'indexMap' is the 
// mapping between indices in 'text' and 'paragraph.GetText', i.e. 
// text[i] == paragraph->GetText()[indexMap[i]]. 
class RightToLeftIterator
{
    typedef Paragraph::CharacterInfo CharacterInfo;
public:
    RightToLeftIterator(): pParagraph(NULL), ParaText(NULL), IndexMap(NULL), 
        MirroredBits(NULL), ParaLen(0), Index(-1), VIndex(0)
    {
    }

    RightToLeftIterator(const Paragraph& paragraph, 
        const wchar_t* text, unsigned len, 
        const unsigned* indexMap, const bool* mirroredBits) : 
    pParagraph(&paragraph), ParaText(text), IndexMap(indexMap), 
        MirroredBits(mirroredBits), ParaLen(len), Index(int(len) - 1), VIndex(0)
    {
    }

    void Init(const Paragraph& paragraph, const wchar_t* text, unsigned len, 
        const unsigned* indexMap, const bool* mirroredBits)
    {
        pParagraph = &paragraph;
        ParaText   = text;
        ParaLen    = len;
        IndexMap   = indexMap;
        MirroredBits=mirroredBits;
        Index      = int(len) - 1;
        VIndex     = 0;
    }

    bool IsFinished() const { return Index < 0; }

    CharacterInfo&              operator*()
    {
        if (!IsFinished())
        {
            PlaceHolder.Character = ParaText[Index];
            PlaceHolder.Index     = VIndex; //IndexMap[Index];
            PlaceHolder.pFormat   = pParagraph->GetTextFormatPtr(IndexMap[Index]);
        }
        else
        {
            PlaceHolder.Character   = 0;
            PlaceHolder.Index       = 0;
            PlaceHolder.pFormat     = NULL;
        }
        return PlaceHolder; 
    }
    const CharacterInfo&        operator*() const
    {
        return const_cast<RightToLeftIterator*>(this)->operator*();
    }
    inline CharacterInfo*       operator->() { return &operator*(); }
    inline const CharacterInfo* operator->() const { return &operator*(); }

    void operator++()
    {
        if (!IsFinished())
        {
            --Index;
            ++VIndex;
        }
    }
    void operator++(int) { operator++(); }
    void operator+=(UPInt n)
    {
        if (!IsFinished())
        {
            Index -= int(n);
            VIndex += unsigned(n);
        }
    }

    const wchar_t* GetRemainingTextPtr(UPInt * plen) const
    {
        *plen = 0;
        return NULL; // @TODO? not supported yet (necessary for image substitution)
    }
    UPInt GetCurTextIndex() const { return Index; }
protected: //data
    const Paragraph*            pParagraph;
    const wchar_t*              ParaText;
    const unsigned*             IndexMap;
    const bool*                 MirroredBits;
    unsigned                    ParaLen;
    int                         Index;
    unsigned                    VIndex; // virtual index, [0..ParaLen-1] 
    CharacterInfo               PlaceHolder;
};
#endif // #ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT

class GFxLineCursor
{
public:
    LineBuffer::GlyphEntry*     pPrevGrec;                      // used for kerning
    Ptr<FontHandle>             pLastFont;                      // used for kerning and line width correction
    unsigned                    LastCharCode;                   // used for kerning

    unsigned                    LastGlyphIndex;                 // used to correct line width
    float                       LastAdvance;                    // used to correct line width
    int                         LastGlyphWidth;                 // used to correct line width
    int                         FirstGlyphWidth;                // used to correct line width in right-to-left text
    UInt32                      LastColor;

#ifndef SF_NO_IME_SUPPORT
    // composition string
    Ptr<CompositionStringBase>  pComposStr;
#endif
    UPInt                       ComposStrPosition;
    UPInt                       ComposStrLength;
    UPInt                       ComposStrCurPos;

    DocView*                    pDocView;
    const Paragraph*            pParagraph;
    int                         LineWidth;                      // accumulated line's width
    int                         LineWidthWithoutTrailingSpaces;
    unsigned                    LineLength;                     // accumulated line's length in chars
    float                       MaxFontAscent;
    float                       MaxFontDescent;
    float                       MaxFontLeading;
    Paragraph::CharactersIterator CharIter;
    Paragraph::CharacterInfo    CharInfoHolder;
    int                         Indent;
    int                         LeftMargin;
    int                         RightMargin;
    LineBuffer::GlyphInserter   GlyphIns;
    unsigned                    NumOfSpaces;
    unsigned                    NumOfTrailingSpaces;
    float                       FontScaleFactor;                // contains current font scale factor

    // used for bidirectional text. These are weak ptrs.
    // The owner of these is ParagraphFormatter.
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    RightToLeftIterator         RLCharIter;
    bool                        RightToLeft;
#endif

    bool                        LastKerning;
    bool                        LineHasNewLine;

    UPInt                       NumChars; // num chars processed

    GFxLineCursor(): pPrevGrec(NULL), pLastFont(NULL), LastCharCode(0), 
        LastGlyphIndex(0), LastAdvance(0), LastGlyphWidth(0), FirstGlyphWidth(0), LastColor(0), 
        ComposStrPosition(SF_MAX_UPINT), ComposStrLength(0),ComposStrCurPos(0),
        pDocView(NULL), pParagraph(NULL), 
        LineWidth(0), LineWidthWithoutTrailingSpaces(0), LineLength(0), 
        MaxFontAscent(0), MaxFontDescent(0), MaxFontLeading(0), Indent(0), 
        LeftMargin(0), RightMargin(0), NumOfSpaces(0), NumOfTrailingSpaces(0),
        FontScaleFactor(1.f), LastKerning(false), LineHasNewLine(false), NumChars(0)
    { 
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
        RightToLeft = false;
#endif
    }
    GFxLineCursor(DocView* pview, const Paragraph* ppara):
    pPrevGrec(NULL), pLastFont(NULL), LastCharCode(0), LastGlyphIndex(0), LastAdvance(0), 
        LastGlyphWidth(0), FirstGlyphWidth(0), LastColor(0), 
        ComposStrPosition(SF_MAX_UPINT), ComposStrLength(0), ComposStrCurPos(0),
        pDocView(pview), pParagraph(ppara), LineWidth(0), 
        LineWidthWithoutTrailingSpaces(0), LineLength(0),
        MaxFontAscent(0), MaxFontDescent(0), MaxFontLeading(0),
        CharIter(ppara->GetCharactersIterator(0)), Indent(0), LeftMargin(0), RightMargin(0), NumOfSpaces(0), 
        NumOfTrailingSpaces(0), FontScaleFactor(1.f), 
        LastKerning(false), LineHasNewLine(false), NumChars(0)
    { 
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
        RightToLeft = false;
#endif
    }
    ~GFxLineCursor()
    {
    }

#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    inline bool IsRightToLeft() const { return RightToLeft; }
#else
    inline bool IsRightToLeft() const { return false; }
#endif

    bool IsFinished() const 
    { 
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
        return (!RightToLeft) ? CharIter.IsFinished() : RLCharIter.IsFinished(); 
#else
        return CharIter.IsFinished();
#endif
    }

    void operator++()           { (*this) += 1; }
    void operator+=(unsigned n) 
    { 
#ifndef SF_NO_IME_SUPPORT
        if (pComposStr && pComposStr->GetLength() > 0)
        {
            UPInt newN = n;
            UPInt curTextPos = CharIter.GetCurTextIndex() + pParagraph->GetStartIndex();
            if (curTextPos     <= pComposStr->GetPosition() && 
                curTextPos + n >= pComposStr->GetPosition())
            {
                // position of composition string is in the range
                newN = Alg::PMin(pComposStr->GetPosition() - curTextPos, (UPInt)n);

                if (ComposStrCurPos + (n - newN) > pComposStr->GetLength())
                {
                    newN += ComposStrCurPos + (n - newN) - pComposStr->GetLength();
                    ComposStrCurPos = pComposStr->GetLength();
                    NumChars        += ComposStrCurPos;
                }
                else
                {
                    ComposStrCurPos += (n - newN);
                    NumChars        += (n - newN);
                }
                n = (unsigned)newN;
            }
        }
#endif //#ifdef SF_NO_IME_SUPPORT
        if (n > 0)
        {
            if (!IsRightToLeft())
                CharIter += n; 
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
            else 
                RLCharIter += n;
#endif

            NumChars += n;
        }
    }
    bool IsInsideComposStr() const
    {
#ifndef SF_NO_IME_SUPPORT
        if (pComposStr && pComposStr->GetLength() > 0)
        {
            UPInt curTextPos = CharIter->Index + pParagraph->GetStartIndex();
            if (curTextPos >= pComposStr->GetPosition())
            {
                if (ComposStrCurPos < pComposStr->GetLength())
                {
                    return true;
                }
            }
        }
#endif //#ifdef SF_NO_IME_SUPPORT
        return false;
    }
    const Paragraph::CharacterInfo& operator*() 
    { 
        CharInfoHolder.Index   = CharIter->Index;
#ifndef SF_NO_IME_SUPPORT
        if (pComposStr && pComposStr->GetLength() > 0)
        {
            UPInt curTextPos = CharInfoHolder.Index + pParagraph->GetStartIndex();
            if (curTextPos >= pComposStr->GetPosition())
            {
                if (curTextPos == pComposStr->GetPosition() && ComposStrCurPos < pComposStr->GetLength())
                {
                    CharInfoHolder.Index = CharIter->Index + ComposStrCurPos;
                    CharInfoHolder.Character = pComposStr->GetText()[ComposStrCurPos];
                    CharInfoHolder.pFormat = *pComposStr->GetAllocator()->AllocateTextFormat
                        (CharIter->pFormat->Merge(*pComposStr->GetTextFormat(ComposStrCurPos)));
                    return CharInfoHolder;
                }
                else
                {
                    CharInfoHolder.Index = CharIter->Index + pComposStr->GetLength();    
                }
            }
        }
#endif //#ifdef SF_NO_IME_SUPPORT

        if (!IsRightToLeft())
        {
            CharInfoHolder = *CharIter;
            if (pDocView->IsPasswordMode() && CharInfoHolder.Character != '\0')
                CharInfoHolder.Character = '*';
        }
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
        else
        {
            // bidirectional text substitute
            CharInfoHolder = *RLCharIter;
            if (pDocView->IsPasswordMode() && CharInfoHolder.Character != '\0')
                CharInfoHolder.Character = '*';
        }
#endif

        return CharInfoHolder;
    }

    float GetPrevGlyphAdvanceAdj(wchar_t c) const 
    {
        if (pPrevGrec)
        {
            SF_ASSERT(pLastFont);
            return pLastFont->GetFont()->GetKerningAdjustment(LastCharCode, c);
        }
        return 0;
    }
    void Reset()
    {
        pPrevGrec     = NULL;
        MaxFontAscent = MaxFontDescent = MaxFontLeading = 0;
        LastAdvance   = 0;
        LastKerning   = LineHasNewLine = false;
        LineWidthWithoutTrailingSpaces = 0;
        LineLength    = 0;
        NumOfSpaces   = NumOfTrailingSpaces = 0;
        NumChars      = 0;
        LastCharCode  = 0;
    }

    void TrackFontParams(Font* pfont, float scale)
    {
        float ascent = pfont->GetAscent();
        float descent = pfont->GetDescent();
        // if ascent is 0, then use 1024 instead.
        if (ascent == 0)
            ascent = 960.f;
        if (descent == 0)
            descent = 1024.f - 960.f;
        MaxFontAscent  = Alg::Max(MaxFontAscent,  ascent * scale);
        MaxFontDescent = Alg::Max(MaxFontDescent, descent * scale);
        MaxFontLeading = Alg::Max(MaxFontLeading, pfont->GetLeading() * scale);
    }

    const wchar_t* GetRemainingTextPtr(UPInt* remlen) const
    {
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
        if (IsRightToLeft())
            return RLCharIter.GetRemainingTextPtr(remlen);
#endif
        return CharIter.GetRemainingTextPtr(remlen);
    }
};

class ParagraphFormatter
{
    DocView*                        pDocView;
    const Paragraph*                pParagraph;
    const ParagraphFormat*          pParaFormat;
    LineBuffer::Line*               pTempLine;
    GFxLineCursor                   LineCursor;
    DocView::FontCache              FontCache;
    DocView::FindFontInfo           FindFontInfo;
    GFxLineCursor                   StartPoint, HalfPoint, WordWrapPoint;
    float                           TextRectWidth;

    unsigned                        TabStopsNum, TabStopsIndex;
    unsigned                        DeltaText;
    int                             Pass;
    bool                            HyphenationRequested;
    unsigned                        RequestedWordWrapPos;     // used only if Pass == 2
    bool                            HasLineFormatHandler;

    // custom wordwrapping
    wchar_t                         TextBufForCustomFormat[256];
    wchar_t*                        pTextBufForCustomFormat;
    UPInt                           TextBufLen;

    // used for bidirectional text. 
    unsigned                        NewLen;
    unsigned                        NewSize;
    wchar_t*                        NewParaText;
    unsigned*                       IndexMap;
    bool*                           MirroredBits;

    // local vars
    Font*                           pFont;
    Ptr<FontHandle>                 pFontHandle;
    float                           Scale;
    int                             GlyphIndex;
    float                           GlyphAdvance;
    int                             LastAdvance;
    int                             GlyphWidth;
    int                             AdjLineWidth;
    int                             NewLineWidth;
    float                           FontSize;
    bool                            isSpace;
    bool                            isNbsp;
public:
    // in-out values
    LineBuffer::Iterator*           pLinesIter;
    UByte                           TempLineBuff[1024];
    LineBuffer::Line*               pDynLine;
    int                             NextOffsetY;

    // in values
    int                             ParaYOffset;
    Log*                            pLog;

    // out values
    int                             ParaWidth;
    int                             ParaHeight;
    unsigned                        ParaLines;
    bool                            NeedRecenterLines;
    bool                            ForceVerticalCenterAutoSize;    // force vertical centered autoSize


    void FinalizeLine();
    void InitParagraph(const Paragraph& paragraph);
    void InitBidiText();
    void InitCustomWordWrapping();
    float GetActualFontSize();
    bool CheckWordWrap();
    bool HandleCustomWordWrap();
    void TreatBidirectionalTextInLine();
public:
    ParagraphFormatter(DocView*, Log* plog);
    ~ParagraphFormatter();
    void Format(const Paragraph& paragraph);
};

ParagraphFormatter::ParagraphFormatter(DocView* pdoc, Log* plog) 
 :  pDocView(pdoc), pParagraph(NULL), pParaFormat(NULL), pTempLine(NULL),
    FontCache(pdoc->GetHeap()), FindFontInfo(&FontCache),
    pDynLine(NULL)
{
    ParaYOffset        = 0;
    NextOffsetY        = 0;
    NeedRecenterLines  = false;
    ForceVerticalCenterAutoSize = false;
    pLog               = plog;

    NewSize = NewLen = 0;
    NewParaText = NULL;
    IndexMap    = NULL;
    MirroredBits= NULL;
}

ParagraphFormatter::~ParagraphFormatter()
{
    if (pDynLine)
        pDocView->mLineBuffer.LineAllocator.FreeLine(pDynLine);
    SF_FREE(NewParaText);
    SF_FREE(IndexMap);
    SF_FREE(MirroredBits);
}

void ParagraphFormatter::InitParagraph(const Paragraph& paragraph)
{
    pParagraph = &paragraph;
    pParaFormat = pParagraph->GetFormat();
    SF_ASSERT(pParagraph && pParaFormat);
    LineCursor = GFxLineCursor(pDocView, &paragraph);
    StartPoint = HalfPoint = WordWrapPoint = GFxLineCursor();
    LineCursor.FontScaleFactor = pDocView->GetFontScaleFactor();

    // Check, do we have a composition string somewhere in the paragraph
    if (pDocView->HasEditorKit() && pDocView->GetEditorKit()->HasCompositionString())
    {
#ifndef SF_NO_IME_SUPPORT
        LineCursor.pComposStr           = pDocView->GetEditorKit()->GetCompositionString();
        LineCursor.ComposStrPosition    = LineCursor.pComposStr->GetPosition();
        LineCursor.ComposStrLength      = LineCursor.pComposStr->GetLength();
#endif //#ifdef SF_NO_IME_SUPPORT
    }

    InitCustomWordWrapping();

    InitBidiText();

    // try to estimate how much memory would be necessary for temporary line
    // Assume, glyphCount <= paragraph.GetLength()
    //         fmtDataElemCount <= paragraph.GetLength()*2*paragraph.GetLength()
    UPInt maxGlyphCount = paragraph.GetSize() + LineCursor.ComposStrLength;
    if (pParaFormat->IsBullet())
        ++maxGlyphCount;
    unsigned linesz = LineBuffer::CalcLineSize
        ((unsigned)maxGlyphCount, (unsigned)maxGlyphCount*2, LineBuffer::Line32);
    if (linesz < sizeof(TempLineBuff))
    {
        pTempLine = ::new (TempLineBuff) LineBuffer::Line();
        //pTempLine = reinterpret_cast<LineBuffer::Line*>(TempLineBuff);
        pTempLine->SetMemSize(linesz);
    }
    else if (pDynLine)
    {
        if (linesz >= pDynLine->GetMemSize())
        {
            pDocView->mLineBuffer.LineAllocator.FreeLine(pDynLine);
            pDynLine = pDocView->mLineBuffer.LineAllocator.AllocLine
                (linesz + GFX_LINE_ALLOC_DELTA, LineBuffer::Line32);
        }
        pTempLine = pDynLine;
    }
    else
        pTempLine = pDynLine = pDocView->mLineBuffer.LineAllocator.AllocLine
        (linesz + GFX_LINE_ALLOC_DELTA, LineBuffer::Line32);
    pTempLine->InitLine32();
    pTempLine->ClearInitialized(); // to avoid tempLine->Release to be invoked
    pTempLine->SetNumGlyphs((unsigned)maxGlyphCount);

    SF_ASSERT(pTempLine);
    pTempLine->SetTextPos(unsigned(pDocView->TextPos2GlyphOffset(paragraph.GetStartIndex())));

    LineCursor.GlyphIns = LineBuffer::GlyphInserter
        (pTempLine->GetGlyphs(), pTempLine->GetNumGlyphs(), pTempLine->GetFormatData());

    ParaHeight              = ParaWidth = 0;
    ParaLines               = 0;

    DeltaText               = 1;
    Pass                    = 1;
    HyphenationRequested    = false;
    RequestedWordWrapPos    = 0;     // used only if Pass == 2
    isSpace                 = false;
    TextRectWidth           = pDocView->GetTextRect().Width();
    TabStopsNum = 0, TabStopsIndex = 0;
}

void ParagraphFormatter::InitBidiText()
{
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    if (pDocView->IsBidirectionalTextEnabled())
    {
        const UPInt len = pParagraph->GetLength();
        if (len > NewSize)
        {
            NewSize = unsigned(len);
            SF_FREE(NewParaText);
            SF_FREE(IndexMap);
            SF_FREE(MirroredBits);
            NewParaText = (wchar_t*)SF_HEAP_ALLOC(Memory::GetGlobalHeap(), sizeof(wchar_t)*len, StatRender_Text_Mem);
            IndexMap = (unsigned*)SF_HEAP_ALLOC(Memory::GetGlobalHeap(), sizeof(unsigned)*len, StatRender_Text_Mem);
            MirroredBits = (bool*)SF_HEAP_ALLOC(Memory::GetGlobalHeap(), sizeof(bool)*len, StatRender_Text_Mem);
        }
        NewLen = unsigned(len);
        
        memset(NewParaText, 0, sizeof(NewParaText[0])*len);
        memset(IndexMap, 0, sizeof(IndexMap[0])*len);
        memset(MirroredBits, 0, sizeof(MirroredBits[0])*len);

        LineCursor.RightToLeft = pDocView->pDocumentListener->View_PrepareBidiText(*pDocView, pParagraph->GetText(), 
            len, NewParaText, IndexMap, MirroredBits);
        if (LineCursor.RightToLeft)
        {
            LineCursor.RLCharIter.Init(*pParagraph, NewParaText, NewLen, IndexMap, MirroredBits);
            pDocView->SetBidirectionalTextRTFlag();
        }
        else
        {
            LineCursor.RightToLeft = false;
        }
    }
    else
#endif
        pDocView->ClearBidirectionalTextRTFlag();
}

void ParagraphFormatter::InitCustomWordWrapping()
{
    HasLineFormatHandler = (pDocView->pDocumentListener && pDocView->pDocumentListener->DoesHandleLineFormat());
    pTextBufForCustomFormat = NULL;

    // make a copy of paragraph text with insertion of composition string
    // to use it for custom formatting handler.
    if (HasLineFormatHandler)
    {
        // check if we can use buffer on stack. If not - allocate temporary one.
        UPInt reqBufSize = pParagraph->GetLength();
        bool composStrInsideThePara = 
            (LineCursor.ComposStrPosition >= pParagraph->GetStartIndex() && 
             LineCursor.ComposStrPosition <= pParagraph->GetStartIndex() + pParagraph->GetLength());
        if (composStrInsideThePara)
            reqBufSize = pParagraph->GetLength() + LineCursor.ComposStrLength;
        wchar_t* pbuf;
        if (reqBufSize < sizeof(TextBufForCustomFormat)/sizeof(TextBufForCustomFormat[0]))
            pbuf = TextBufForCustomFormat;
        else
            pbuf = pDocView->GetAllocator()->AllocText(reqBufSize + 1);

        // copy text of whole paragraph + composition string into the buffer
        const wchar_t* paraText = pParagraph->GetText();
        if (composStrInsideThePara && LineCursor.ComposStrLength > 0)
        {
#ifndef SF_NO_IME_SUPPORT
            UPInt composStrParaStartIdx = LineCursor.ComposStrPosition - pParagraph->GetStartIndex();
            SF_ASSERT((SPInt)composStrParaStartIdx >= 0);
            memcpy(pbuf, paraText, composStrParaStartIdx * sizeof(wchar_t));
            paraText += composStrParaStartIdx;
            memcpy(pbuf + composStrParaStartIdx, LineCursor.pComposStr->GetText(), 
                LineCursor.ComposStrLength * sizeof(wchar_t));
            memcpy(pbuf + composStrParaStartIdx + LineCursor.ComposStrLength, 
                paraText, (pParagraph->GetLength() - composStrParaStartIdx) * sizeof(wchar_t));
#endif
        }
        else
        {
            memcpy(pbuf, paraText, reqBufSize * sizeof(wchar_t));
        }
        pbuf[reqBufSize]        = 0;
        pTextBufForCustomFormat = pbuf;
        TextBufLen              = reqBufSize;
    }
}

float ParagraphFormatter::GetActualFontSize()
{
    float fontSize = FindFontInfo.pCurrentFormat->GetFontSize();
    if (FindFontInfo.pCurrentFont->GetFontScaleFactor() != 1.0f)
    {
        fontSize *= FindFontInfo.pCurrentFont->GetFontScaleFactor();
        //        if (!pDocView->IsMultiline())
        //            ForceVerticalCenterAutoSize = true;
    }
    if (pDocView->HasFontScaleFactor())
        fontSize *= pDocView->GetFontScaleFactor();
    return fontSize;
}

void ParagraphFormatter::TreatBidirectionalTextInLine()
{
#ifdef GFX_ENABLE_BIDIRECTIONAL_TEXT
    // we will recalc line width
    LineCursor.LineWidth = LineCursor.LineWidthWithoutTrailingSpaces = 0;

    unsigned glyphsCount         = LineCursor.GlyphIns.GetGlyphIndex();

    // reverse order of glyphs/format data in the target line
    struct FormatDataEntry_
    {
        LineBuffer::GlyphEntry  Glyph;
        Ptr<FontHandle>         pFont;
        Ptr<ImageDesc>          pImage;
        UInt32                  ColorV;
    };
    // allocate temporary storage for absolute format data for each glyph in the line
    FormatDataEntry_* absFmt = new FormatDataEntry_[glyphsCount];
    // limit scope of srcGI below.
    {
        LineCursor.LineWidth =  LineCursor.FirstGlyphWidth;
        LineBuffer::GlyphIterator srcGI(pTempLine->GetGlyphs(), glyphsCount, pTempLine->GetFormatData());
        unsigned i;
        for (i = 0; !srcGI.IsFinished(); ++i, ++srcGI)
        {
            absFmt[i].Glyph = srcGI.GetGlyph();
            absFmt[i].pFont = srcGI.GetFontHandle();
            absFmt[i].pImage= srcGI.GetImage();
            absFmt[i].ColorV= srcGI.GetColor().ToColor32();

            if (i != 0)
                LineCursor.LineWidth += absFmt[i].Glyph.GetAdvance();
        }
        SF_ASSERT(i == glyphsCount);
    }
    LineCursor.LineWidthWithoutTrailingSpaces = LineCursor.LineWidth;
    LineBuffer::ReleasePartOfLine(pTempLine->GetGlyphs(), glyphsCount, pTempLine->GetFormatData());

    LineBuffer::GlyphInserter dstGI(pTempLine->GetGlyphs(), glyphsCount, pTempLine->GetFormatData());

    FontHandle* prevFont = NULL;
    ImageDesc*  prevImg = NULL;
    UInt32      prevColor = 0;
    for (int s = int(glyphsCount) - 1; s >= 0; --s, ++dstGI)
    {
        LineBuffer::GlyphEntry& ge = dstGI.GetGlyph();
        ge = absFmt[s].Glyph;
        ge.ClearFmtData();
        if (absFmt[s].pImage && prevImg != absFmt[s].pImage)
        {
            dstGI.AddImage(absFmt[s].pImage);
            prevImg = absFmt[s].pImage;
        }
        else
        {
            if (absFmt[s].pFont && prevFont != absFmt[s].pFont)
            {
                dstGI.AddFont(absFmt[s].pFont);
                prevFont = absFmt[s].pFont;
            }
            if (prevColor != absFmt[s].ColorV)
            {
                dstGI.AddColor(Color(absFmt[s].ColorV));
                prevColor = absFmt[s].ColorV;
            }
        }
        absFmt[s].pFont = NULL;
        absFmt[s].pImage= NULL;
    }
    delete [] absFmt;
#endif
}

void ParagraphFormatter::FinalizeLine()
{
    int lastAdvance = Alg::IRound(LineCursor.LastAdvance);
    if (LineCursor.pPrevGrec)
    {
        LineCursor.pPrevGrec->SetAdvance(lastAdvance);
    }

    // correct the line width by removing the advance and adding the real width of the last char
    if (LineCursor.pLastFont)
        LineCursor.LineWidth += LineCursor.LastGlyphWidth;
    else
        LineCursor.LineWidth += lastAdvance;

    if (pParaFormat->IsRightAlignment() || pParaFormat->IsCenterAlignment())
    {
        // if text is right or center aligned and RightMargin is not zero - 
        // add the margin to line width.
        LineCursor.LineWidth                        += LineCursor.RightMargin;
        LineCursor.LineWidthWithoutTrailingSpaces   += LineCursor.RightMargin;
    }

    float fleading   = (pParaFormat->IsLeadingSet()) ? PixelsToFixp<float>(float(pParaFormat->GetLeading())) : LineCursor.MaxFontLeading; 
    int   lineHeight = Alg::IRound(LineCursor.MaxFontAscent + LineCursor.MaxFontDescent);
    int   leading = Alg::IRound(fleading);

    LineCursor.LineWidth                      = Alg::Max(0, LineCursor.LineWidth);
    LineCursor.LineWidthWithoutTrailingSpaces = Alg::Max(0, LineCursor.LineWidthWithoutTrailingSpaces);

    // reorder glyphs for right-to-left text, recalc format data, width, etc
    if (LineCursor.IsRightToLeft())
        TreatBidirectionalTextInLine();

    LineBuffer::Line* pcurLine;
    // so, here we need to allocate a "real" (non temporary) line and transfer
    // all data to it.
    // First, we need to decide, can we allocate Line8 or not...
    // The criteria for Line8 are as follows:
    // Dimensions = [-32768..32767] (i.e. -1638.4 .. 1638.35 pts), 
    // TextLen <= 255, GlyphsCount <= 255, Leading <= 255 (i.e. 12.75 pt), TextPos < 2^24
    unsigned glyphsCount         = LineCursor.GlyphIns.GetGlyphIndex();
    unsigned formatDataElemCount = LineCursor.GlyphIns.GetFormatDataElementsCount();
    if (pDocView->GetTextAutoSize() == DocView::TAS_None && // if auto text fit is on - always use long form
        LineCursor.LineLength <= 255 && glyphsCount <= 255 && (leading >= -128 && leading <= 127) &&
        (lineHeight >= 0 && lineHeight < 65536) && 
        (LineCursor.LineWidth >= 0 && LineCursor.LineWidth < 65536))
    {
        // convert line to short form
        pcurLine = pLinesIter->InsertNewLine(glyphsCount, formatDataElemCount, LineBuffer::Line8);
    }
    else
    {
        pcurLine = pLinesIter->InsertNewLine(glyphsCount, formatDataElemCount, LineBuffer::Line32);
    }
    pcurLine->SetParagraphId(LineCursor.pParagraph->GetId());
    pcurLine->SetParagraphModId(LineCursor.pParagraph->GetModCounter());
    
    // mark the line as right-to-left.
    if (LineCursor.IsRightToLeft())
        pcurLine->SetRightToLeft();

    // copy glyph and format data
    memcpy(pcurLine->GetGlyphs(), pTempLine->GetGlyphs(), 
        glyphsCount*sizeof(LineBuffer::GlyphEntry));
    memcpy(pcurLine->GetFormatData(), pTempLine->GetFormatData(), 
        formatDataElemCount*sizeof(LineBuffer::FormatDataEntry));

    pcurLine->SetTextPos(pTempLine->GetTextPos());
    pcurLine->SetTextLength(LineCursor.LineLength);

    pcurLine->SetBaseLineOffset(LineCursor.MaxFontAscent);

    // check for justify.
    // do not do justification for the last line in para, or for the empty one
    if (!LineCursor.LineHasNewLine && pDocView->IsWordWrap() && 
        LineCursor.NumOfSpaces - LineCursor.NumOfTrailingSpaces > 0 && pParaFormat->IsJustifyAlignment())
    {
        // do one more pass over the line and set appropriate advance values for spaces
        // to accomplish justification. 
        int deltaW = (int(TextRectWidth - GFX_TEXT_JUSTIFY_CORRECTION) - LineCursor.RightMargin) - 
                     (LineCursor.LineWidthWithoutTrailingSpaces + LineCursor.Indent + LineCursor.LeftMargin);
        if (deltaW > 0) //? could it be < 0?
        {
            int advDelta = deltaW / (LineCursor.NumOfSpaces - LineCursor.NumOfTrailingSpaces);

            LineBuffer::GlyphIterator git = pcurLine->Begin();
            for(; !git.IsFinished(); ++git)
            {
                LineBuffer::GlyphEntry& ge = git.GetGlyph();
                if (ge.IsSpaceChar())
                    ge.SetAdvance(ge.GetAdvance() + advDelta);
            }
            LineCursor.LineWidth += deltaW;
        }
    }

    // calculate the bounds of the line
    int lineOffsetX = LineCursor.Indent + LineCursor.LeftMargin;
    pcurLine->SetOffset(lineOffsetX, NextOffsetY);
    pcurLine->SetLeading(leading);

    ParagraphFormat::AlignType align = pParaFormat->GetAlignment();
    // inverse meaning of 'right' and 'left' alignment for RTL text
    //if (LineCursor.IsRightToLeft())
    //{
    //    // inverse left & right align
    //    if (align == ParagraphFormat::Align_Left)
    //        align = ParagraphFormat::Align_Right;
    //    else if (align == ParagraphFormat::Align_Right)
    //        align = ParagraphFormat::Align_Left;
    //}

    //switch(pParaFormat->GetAlignment())
    switch(align)
    {
    case ParagraphFormat::Align_Right:   
        pcurLine->SetAlignment(LineBuffer::Line::Align_Right); 
        pcurLine->SetDimensions(LineCursor.LineWidthWithoutTrailingSpaces, lineHeight);

        // align line accordingly to ViewRect
        if (LineCursor.IsRightToLeft())
            pcurLine->SetOffsetX(Alg::IRound(TextRectWidth) - LineCursor.LineWidthWithoutTrailingSpaces);
        else
            pcurLine->SetOffsetX(Alg::Max(0, Alg::IRound(TextRectWidth) - LineCursor.LineWidthWithoutTrailingSpaces));

        if (pDocView->IsAutoSizeX() || pDocView->GetTextAutoSize() != DocView::TAS_None)
        {
            NeedRecenterLines = true;
        }
        break;
    case ParagraphFormat::Align_Center:  
        {
            pcurLine->SetAlignment(LineBuffer::Line::Align_Center); 
            pcurLine->SetDimensions(LineCursor.LineWidthWithoutTrailingSpaces, lineHeight);

            // align line accordingly to ViewRect and LeftMargin. RightMargin is already counted
            // at the top of this func.
            float adjTextRect = TextRectWidth  - float(LineCursor.LeftMargin);
            pcurLine->SetOffsetX (Alg::Max(0, Alg::IRound(adjTextRect / 2 - 
                LineCursor.LineWidthWithoutTrailingSpaces / 2) + LineCursor.LeftMargin));

            if (pDocView->IsAutoSizeX() || pDocView->GetTextAutoSize() != DocView::TAS_None)
            {
                NeedRecenterLines = true;
            }
            break;
        }
    default:                                    
        pcurLine->SetAlignment(LineBuffer::Line::Align_Left); 
        pcurLine->SetDimensions(LineCursor.LineWidth, lineHeight);
        break;
    }

    ParaWidth    = Alg::Max(ParaWidth, lineOffsetX + LineCursor.LineWidth);
    ParaHeight   = NextOffsetY + lineHeight - ParaYOffset;
    NextOffsetY  += Alg::IRound(lineHeight + leading);
    LineCursor.Indent   = 0; // reset indent
    LineCursor.GlyphIns.Reset();
    LineCursor.LastColor = 0;
    LineCursor.pLastFont = NULL;
}

bool ParagraphFormatter::HandleCustomWordWrap()
{
    if (!LineCursor.pLastFont)
        return false;

    // if this is the first pass and document listener handles OnLineFormat
    // then invoke this handler and run the second path if necessary.
    if (Pass == 1 && HasLineFormatHandler)
    {
        float lwBeforeWW;
        Font* pfont;
        UPInt wordWrapPos;
        if (WordWrapPoint.IsFinished())
        {
            lwBeforeWW  = (float)LineCursor.LineWidth;
            pfont       = LineCursor.pLastFont->GetFont();
            wordWrapPos = LineCursor.NumChars; 
        }
        else
        {
            lwBeforeWW  = (float)WordWrapPoint.LineWidth;
            pfont       = WordWrapPoint.pLastFont->GetFont();
            wordWrapPos = WordWrapPoint.NumChars; 
        }
        // first of all we need to fill out LineFormatDesc structure.
        DocView::LineFormatDesc desc;
        desc.pParaText      = pTextBufForCustomFormat;
        desc.ParaTextLen    = TextBufLen;
        desc.LineStartPos   = pTempLine->GetTextPos() - pParagraph->GetStartIndex();
        desc.NumCharsInLine = LineCursor.NumChars;
        switch(pParaFormat->GetAlignment())
        {
        case ParagraphFormat::Align_Left:   
            desc.Alignment = DocView::LineFormatDesc::Align_Left;
            break;
        case ParagraphFormat::Align_Right:   
            desc.Alignment = DocView::LineFormatDesc::Align_Right;
            break;
        case ParagraphFormat::Align_Center:   
            desc.Alignment = DocView::LineFormatDesc::Align_Center;
            break;
        case ParagraphFormat::Align_Justify:   
            desc.Alignment = DocView::LineFormatDesc::Align_Justify;
            break;
        default:;
        }

        desc.VisibleRectWidth = (float)TextRectWidth;  // [in] width of client rect
        desc.CurrentLineWidth = (float)LineCursor.LineWidth;   // [in] current line width
        desc.LineWidthBeforeWordWrap = lwBeforeWW;// [in] line width till ProposedWordWrapPoint
        int dashGlyphIndex    = pfont->GetGlyphIndex('-');
        if (dashGlyphIndex > 0)
            desc.DashSymbolWidth  = pfont->GetGlyphWidth(dashGlyphIndex);                      // [in] may be used to calculate hyphenation

        desc.ProposedWordWrapPoint  = wordWrapPos;  // [in,out] text index of proposed word wrap pos
        float widths[256], *pwidths = widths;
        if (desc.NumCharsInLine + 1 > sizeof(widths)/sizeof(widths[0]))
        {
            pwidths = (float*)pDocView->GetAllocator()->AllocRaw(sizeof(float)*(desc.NumCharsInLine + 1));
        }
        unsigned i = 0;
        unsigned numGlyphs;
        LineBuffer::GlyphEntry* pglyphs = LineCursor.GlyphIns.GetReadyGlyphs(&numGlyphs);
        float deltaW = 0;
        // Skip leading zero-length chars and calc delta width
        for (; i < numGlyphs; ++i)
        {
            if (pglyphs[i].GetLength() == 0)
                deltaW += pglyphs[i].GetAdvance();
            else
                break;

        }
        desc.pWidths                    = pwidths;
        desc.VisibleRectWidth           -= deltaW;
        desc.CurrentLineWidth           -= deltaW;
        desc.LineWidthBeforeWordWrap    -= deltaW;
        // fill pwidths array with widths of every char
        float curW = 0;
        UPInt p;
        for (p = 0; i < numGlyphs; ++i)
        {
            pwidths[p] = curW;
            curW += (float)pglyphs[i].GetAdvance();
            unsigned l = pglyphs[i].GetLength();
            if (l > 1)
            {
                for (unsigned j = 0; j < l; ++j)
                {
                    pTextBufForCustomFormat[p + j] = GFX_NBSP_CHAR_CODE;
                    pwidths[p + j] = 0;
                }
            }
            p += l;
        }
        pwidths[p] = curW;
        desc.UseHyphenation = false;
        bool handlerSucceeded = pDocView->pDocumentListener->View_OnLineFormat(*pDocView, desc);
        if (handlerSucceeded)
        {
            //SF_ASSERT(desc.ProposedWordWrapPoint > pTempLine->GetTextPos());
            //SF_ASSERT(desc.ProposedWordWrapPoint < pTempLine->GetTextPos() + LineCursor.NumChars);

            HyphenationRequested = desc.UseHyphenation;

            // leave at least one char on the line...
            if (desc.ProposedWordWrapPoint == 0)
                desc.ProposedWordWrapPoint = 1;

            // if returned true then format was handled and need to update
            // format data according to desc data.
            if (desc.ProposedWordWrapPoint != wordWrapPos)
            {
                if (!WordWrapPoint.IsFinished() && desc.ProposedWordWrapPoint > WordWrapPoint.NumChars)
                {
                    LineCursor.GlyphIns.ResetTo(WordWrapPoint.GlyphIns);
                    LineCursor = WordWrapPoint;
                }
                else if (!HalfPoint.IsFinished() && desc.ProposedWordWrapPoint > HalfPoint.NumChars)
                {
                    LineCursor.GlyphIns.ResetTo(HalfPoint.GlyphIns);
                    LineCursor = HalfPoint;
                }
                else
                {
                    LineCursor.GlyphIns.ResetTo(StartPoint.GlyphIns);
                    LineCursor = StartPoint;
                }
                Pass = 2;
                RequestedWordWrapPos = (unsigned)desc.ProposedWordWrapPoint;
                DeltaText = 0; //don't increment
            }
        }
        if (pwidths != widths)
            pDocView->GetAllocator()->FreeRaw(pwidths);
        StartPoint = HalfPoint = GFxLineCursor();

        // if View_OnLineFormat returned true this means that word wrap position has been
        // moved and the second Pass is required. Unless, the new word wrap point is equal
        // to the proposed earlier.
        if (handlerSucceeded && Pass == 2)
            return true;
    }
    else if (Pass == 2)
    {
        WordWrapPoint = GFxLineCursor();
        Pass = 1;
    }

    // add hyphenation mark
    if (HyphenationRequested)
    {
        LineBuffer::GlyphEntry& grec = LineCursor.GlyphIns.GetGlyph();
        int lastAdvance = Alg::IRound(LineCursor.LastAdvance);
        if (LineCursor.pPrevGrec)
        {
            LineCursor.pPrevGrec->SetAdvance(lastAdvance);
        }
        int dashGlyphIndex = LineCursor.pLastFont->GetFont()->GetGlyphIndex('-');
        grec.SetIndex(dashGlyphIndex);
        grec.SetLength(0);
        grec.SetFontSize(FontSize);
        //grec.SetAdvance(LineCursor.pLastFont->GetAdvance(dashGlyphIndex))
        //grec.SetAdvance((int)(LineCursor.pLastFont->GetFont()->GetAdvance(dashGlyphIndex) * scale));
        LineCursor.LineWidth        = NewLineWidth;
        float dashGlyphAdvance = LineCursor.pLastFont->GetFont()->GetAdvance(dashGlyphIndex) * Scale;
        RectF dashGlyphBounds;
        int   dashGlyphWidth   = Alg::IRound(Alg::Max(
                pFont->GetGlyphBounds(dashGlyphIndex, &dashGlyphBounds).x2 * Scale + 20, dashGlyphAdvance));
        //LineCursor.LastGlyphWidth   = (int)(LineCursor.pLastFont->GetFont()->GetGlyphWidth(dashGlyphIndex) * Scale);
        LineCursor.LastAdvance      = (float)dashGlyphWidth;
        LineCursor.LastGlyphWidth   = dashGlyphWidth;
        LineCursor.LastGlyphIndex   = dashGlyphIndex;
        LineCursor.NumOfTrailingSpaces = 0;
        LineCursor.LineWidthWithoutTrailingSpaces = NewLineWidth + dashGlyphWidth;
        HyphenationRequested = false;
        LineCursor.pPrevGrec = &grec;
        ++LineCursor.GlyphIns;
    }
    return false;
}

bool ParagraphFormatter::CheckWordWrap()
{
    // Do word-wrapping?
    // use a real width of last glyph to test for intersection with viewrect
    if ((Pass == 1 && (pDocView->IsWordWrap() && !isSpace && 
        NewLineWidth + AdjLineWidth + LineCursor.Indent + LineCursor.LeftMargin > 
        TextRectWidth - LineCursor.RightMargin)) || 
        (Pass == 2 && LineCursor.NumChars == RequestedWordWrapPos))
    {
        if (HandleCustomWordWrap())
            return true;

        // create a new line by incrementing 'currentLineNum'
        NewLineWidth = 0;

        UPInt newTextPos;
        bool wordWrap = false;

        // if wordWrap - cut the last word
        if (!WordWrapPoint.IsFinished())
        {
            LineBuffer::GlyphInserter ins = WordWrapPoint.GlyphIns;
            LineCursor.GlyphIns.ResetTo(ins);

            LineCursor              = WordWrapPoint;
            isSpace                 = false;
            wordWrap                = true;
            DeltaText               = 0;
        }
        WordWrapPoint = GFxLineCursor();
        newTextPos = pParagraph->GetStartIndex() + (*LineCursor).Index;

        // mark the separator char at the end of the previous line
        if (LineCursor.pPrevGrec)
        {
            LineCursor.pPrevGrec->SetWordWrapSeparator();
        }
        LineCursor.LastAdvance = 0;
        LineCursor.LastGlyphWidth = 0;
        LineCursor.pPrevGrec = NULL;

        // finalize the line
        FinalizeLine();

        // reset values for the next line
        LineCursor.Reset();
        TabStopsIndex = 0; // reset tabStops index. Actually Flash 8 doesn't do this
        // Flash 8 applies tabStops only to the first line.
        // Flash 9 resets tabStops index for each line.

        SF_ASSERT(pTempLine);

        pTempLine->SetTextPos((unsigned)newTextPos);

        if (wordWrap)
        {
            LineCursor.LineWidth = 0;
            return true;
        }
    }
    return false;
}

// Format the paragraph into temporary line buffer
void ParagraphFormatter::Format(const Paragraph& paragraph)
{
    pDocView->ClearBidirectionalTextRTFlag();
    InitParagraph(paragraph);

    LineCursor.LeftMargin = 0;
    if (pParaFormat->IsBullet())
    {
        // add bullet symbol.
        LineBuffer::GlyphEntry& grec = LineCursor.GlyphIns.GetGlyph();

        const Paragraph::CharacterInfo& chInfo = *LineCursor;
        FindFontInfo.pCurrentFormat = chInfo.pFormat;
        if (FindFontInfo.pCurrentFormat)
        {
            // it is possible that there is no format info if paragraph
            // is empty.
            Ptr<FontHandle> pfontHandle = pDocView->FindFont(&FindFontInfo, false);
            SF_ASSERT(pfontHandle);

            int glyphIndex = pfontHandle->GetFont()->GetGlyphIndex(GFX_BULLET_CHAR_CODE);
            grec.SetIndex(glyphIndex);
            grec.SetAdvance(PixelsToFixp<int>(GFX_BULLET_CHAR_ADVANCE));
            float fontSize = GetActualFontSize()*2/3; 
            grec.SetFontSize(fontSize);
            grec.SetLength(0);

            UInt32 color = FindFontInfo.pCurrentFormat->GetColor32();
            LineCursor.GlyphIns.AddFont(pfontHandle);
            LineCursor.GlyphIns.AddColor(color);
            ++LineCursor.GlyphIns;

            LineCursor.pLastFont    = pfontHandle;
            LineCursor.LastColor    = color;
        }
        LineCursor.LeftMargin   = PixelsToFixp<int>(GFX_BULLET_PARA_INDENT) + PixelsToFixp<int>(GFX_BULLET_CHAR_ADVANCE);
        LineCursor.LineWidth    = LineCursor.LeftMargin;
        LineCursor.Indent       = -PixelsToFixp<int>(GFX_BULLET_CHAR_ADVANCE);
    }
    else
        LineCursor.Indent  = PixelsToFixp<int>(pParaFormat->GetIndent());
    LineCursor.LeftMargin  += PixelsToFixp<int>(pParaFormat->GetLeftMargin() + pParaFormat->GetBlockIndent());
    LineCursor.RightMargin = PixelsToFixp<int>(pParaFormat->GetRightMargin());
    const unsigned* const ptabStops = pParaFormat->GetTabStops(&TabStopsNum);

    Ptr<ImageDesc> pimgDesc;
    for(;!LineCursor.IsFinished(); LineCursor += DeltaText)
    {
        const Paragraph::CharacterInfo& chInfo = *LineCursor;

        if (Pass == 1 && pDocView->IsWordWrap() && !pimgDesc)
        {
            wchar_t lastChar = (wchar_t)LineCursor.LastCharCode;
            if (WordWrapHelper::IsLineBreakOpportunityAt(WordWrapHelper::WWT_All, lastChar, chInfo.Character))
            {
                WordWrapPoint = LineCursor;
            }
        }

        // save control points, if necessary
        if (Pass == 1 && HasLineFormatHandler)
        {
            float w = TextRectWidth;
            if (LineCursor.LineWidth == 0 && !StartPoint.pDocView)
                StartPoint = LineCursor;
            if (LineCursor.LineWidth > w/2) 
            {
                if (!HalfPoint.pDocView)  
                    HalfPoint = LineCursor;
            }
            SF_ASSERT(!StartPoint.IsFinished() || StartPoint.NumChars == 0);
        }

        DeltaText = 1;
        //SF_ASSERT(chInfo.Character != 6158); //@DBG

        pimgDesc = NULL;
        if (pDocView->pImageSubstitutor)
        {
            UPInt  remlen;
            const wchar_t* premainingStr = LineCursor.GetRemainingTextPtr(&remlen);
            UPInt  textLen;
            pimgDesc = pDocView->pImageSubstitutor->FindImageDesc(premainingStr, remlen, &textLen);
            if (pimgDesc)
            {
                DeltaText = (unsigned)textLen;
            }
        }

        if (!pimgDesc)
        {
            SF_ASSERT(chInfo.pFormat);
            if (chInfo.pFormat && chInfo.pFormat->IsImageDescSet() && chInfo.Character != 0)
            {
                pimgDesc = chInfo.pFormat->GetImageDesc();
            }
        }

        pFont = NULL;
        Scale = 1.f;
        FontSize = -1;
        if (!pimgDesc)
        {
            isNbsp  = (chInfo.Character == GFX_NBSP_CHAR_CODE);
            //isSpace = (chInfo.Character == 0 || (!isNbsp && G_iswspace(chInfo.Character) != 0));
            isSpace = (chInfo.Character == 0 || (!isNbsp && WordWrapHelper::IsWhiteSpaceChar(chInfo.Character)));

            if (FindFontInfo.pCurrentFont && FindFontInfo.pCurrentFormat == chInfo.pFormat) // is the format ptr same as previous one?
            {
                // if yes, just use FindFontInfo.CurrentFormat, since the default text format is same.
                pFontHandle = FindFontInfo.pCurrentFont;
            }
            else
            {
                FindFontInfo.pCurrentFormat = chInfo.pFormat;
                bool quietMode = (chInfo.Character == 0); // do not report "font not found" for '\0' char

                pFontHandle = pDocView->FindFont(&FindFontInfo, quietMode);
                SF_ASSERT(pFontHandle);
                if (!pFontHandle) 
                    continue;
            }
            pFont = pFontHandle->GetFont();
            FontSize = GetActualFontSize(); 
            Scale = PixelsToFixp(FontSize) / 1024.0f; // the EM square is 1024 x 1024   
            if (chInfo.Character == pDocView->NewLineChar() || chInfo.Character == '\0')
            {
                // ok, the end of the paragraph and the line. Add a fake glyph with 
                // half of space's width to the buffer
                GlyphIndex = pFont->GetGlyphIndex(' ');
#ifdef GFX_DEBUG_DISPLAY_INVISIBLE_CHARS
                if (chInfo.Character == '\0')
                    GlyphIndex = pFont->GetGlyphIndex(0xA4);
                else
                    GlyphIndex = pFont->GetGlyphIndex(0xB6);
#endif
                GlyphAdvance = pFont->GetAdvance(GlyphIndex) / 2 * Scale;
            }
            else if (chInfo.Character != '\t') // tab symbol does not need a glyph
            {
                GlyphIndex = pFont->GetGlyphIndex((UInt16)chInfo.Character);
                if (GlyphIndex == -1)
                {
                    if (isNbsp)
                    {
                        // &nbsp is not found; replace a glyph by the regular space's one.
                        GlyphIndex = pFont->GetGlyphIndex(' ');
                    }
                    if (GlyphIndex == -1)
                    {
                        // error -- missing glyph!
                        if (pLog && !(pDocView->RTFlags & DocView::RTFlags_FontErrorDetected))
                        {
                            FontManagerBase::FontSearchPathInfo searchInfo(1);
                            Ptr<FontHandle> h = *pDocView->GetFontManager()->
                                CreateFontHandle(FindFontInfo.pCurrentFormat->GetFontList(), 
                                FindFontInfo.pCurrentFormat->IsBold(), 
                                FindFontInfo.pCurrentFormat->IsItalic(),
                                pDocView->DoesUseDeviceFont(), !FindFontInfo.pCurrentFormat->IsSingleFontNameSet(), &searchInfo);

                            // theoretically, 'h' could be NULL here, so use pFont
                            String charPath   = pDocView->GetDocumentListener()? pDocView->GetDocumentListener()->GetCharacterPath(): String();
                            String charRanges = pFont->GetCharRanges();
                            unsigned glyphsCount   = pFont->GetGlyphShapeCount();
                            pLog->LogError("Missing \"%s\" glyph \'%c\' (0x%x) in \"%s\".\n"
                                "Font has %u glyphs, ranges %s.\nSearch log: \n%s",
                                FindFontInfo.pCurrentFormat->GetFontList().ToCStr(), (char)chInfo.Character, chInfo.Character, 
                                charPath.ToCStr(), glyphsCount,
                                charRanges.ToCStr(), searchInfo.Info.ToCStr());
                            pDocView->RTFlags |= DocView::RTFlags_FontErrorDetected;

                            // Drop through and use index == -1; this will display
                            // using the empty-box glyph
                        }
                    }
                }

                GlyphAdvance = pFont->GetAdvance(GlyphIndex) * Scale;

                // Set advance value for non-spacing marks (see 'Mn' category in Unicode std)
                // to zero, so it appears above the prev char. Mostly required for Arabic.
                if (SFisNonSpacingMark(chInfo.Character))
                {
                    GlyphAdvance = 0;
                }
            }
            else
            {
                // tab symbol case: glyph index and advance do not matter, an empty char
                // with calculated advance will be used instead.
                GlyphIndex   = -1;
                GlyphAdvance = 0;
            }

            // if kerning for previous glyph was on - modify its advance and the width of line
            if (LineCursor.pPrevGrec && !LineCursor.IsRightToLeft()) // no kerning support for right-to-left text
            {
                float prevGlyphAdvanceAdj = 0;
                if (LineCursor.LastKerning)
                    prevGlyphAdvanceAdj = LineCursor.GetPrevGlyphAdvanceAdj(chInfo.Character) * Scale;

                LastAdvance = Alg::IRound(LineCursor.LastAdvance + prevGlyphAdvanceAdj);
            }
            else
            {
                LastAdvance = Alg::IRound(LineCursor.LastAdvance);
            }
            GlyphWidth   = Alg::IRound(GlyphAdvance);
            //!AB: The code below is replaced by the code above, because using scaled right
            // boundary as a width of glyph may cause incorrect text positioning, especially
            // in the case of right-aligned text.
            //RectF glyphBounds;
            //GlyphWidth   = Alg::IRound(Alg::Max(
            //    pFont->GetGlyphBounds(GlyphIndex, &glyphBounds).x2 * Scale + 20, GlyphAdvance));

            // the space shouldn't be taken into account when checking the lineWidth for
            // intersection with viewrect. Though, the space's width still should be added
            // to the lineWidth, even if it is a "word-wrapping" point.
            AdjLineWidth = (isSpace) ? 0 : GlyphWidth;
        }
        else
        {
            FindFontInfo.pCurrentFormat = chInfo.pFormat;
            GlyphWidth      = Alg::IRound(pimgDesc->GetScreenWidth());
            GlyphAdvance    = GlyphWidth + 40.f;
            LastAdvance     = Alg::IRound(LineCursor.LastAdvance);
            AdjLineWidth    = GlyphWidth;
            GlyphIndex      = ~0u;
            isSpace         = false;
            isNbsp          = false;
        }

        NewLineWidth = LineCursor.LineWidth + LastAdvance;

        if (LineCursor.pPrevGrec)
        {
            LineCursor.pPrevGrec->SetAdvance(LastAdvance);
        }

        // Do word-wrapping?
        if (CheckWordWrap())
            continue;

        LineBuffer::GlyphEntry& grec = LineCursor.GlyphIns.GetGlyph();
        grec.SetIndex(GlyphIndex);
        if (LineCursor.IsInsideComposStr())
            grec.SetComposStrGlyph();
        // Advance will be set at the next iteration or in Finalize, taking
        // kerning into account.
        //grec.SetAdvance(GlyphAdvance);

        if (pimgDesc)
        {
            LineCursor.GlyphIns.AddImage(pimgDesc);
            grec.SetLength(DeltaText);
            LineCursor.LastKerning = false;

            Render::PointF p(0,0);
            p = pimgDesc->Matrix.Transform(p);
            LineCursor.MaxFontAscent  = Alg::Max(LineCursor.MaxFontAscent,  -p.y);
            LineCursor.MaxFontDescent = Alg::Max(LineCursor.MaxFontDescent, pimgDesc->GetScreenHeight() - -p.y);

            LineCursor.NumOfTrailingSpaces = 0;
            LineCursor.LineWidthWithoutTrailingSpaces = NewLineWidth + AdjLineWidth;

            if (FindFontInfo.pCurrentFormat->IsUrlSet())
                grec.SetUrl();
            else
                grec.ClearUrl();
        }
        else
        {
            UInt32 color  = FindFontInfo.pCurrentFormat->GetColor32();
            //float FontSize = FindFontInfo.pCurrentFormat->GetFontSize(LineCursor.FontScaleFactor);
            SF_ASSERT(FontSize >= 0);
            grec.SetFontSize(FontSize);

            if (pFontHandle != LineCursor.pLastFont)
                LineCursor.GlyphIns.AddFont(pFontHandle);
            if (color != LineCursor.LastColor)
                LineCursor.GlyphIns.AddColor(color);

            if (chInfo.Character == pDocView->NewLineChar() || chInfo.Character == '\0')
            {
                // special symbol at the end of paragraph. Set its length to 0.
                // it also shouldn't participate in textWidth's calculation.
                // but it should be taken into account when drawing background 
                // highlighting.
                if (chInfo.Character == '\0')
                    grec.SetLength(0);
                else
                    grec.SetLength(1); //?????????????? //!
                grec.SetInvisibleChar();
#ifdef GFX_DEBUG_DISPLAY_INVISIBLE_CHARS
                grec.ClearInvisibleChar();
#endif
                grec.SetNewLineChar();
                GlyphWidth = 0;
                LineCursor.LineHasNewLine = true;

                // gather dimensions for the line, ONLY if this is the first char
                if (chInfo.Index == 0)
                    LineCursor.TrackFontParams(pFont, Scale);

            }
            else
            {
                if (isSpace || isNbsp)
                {
                    grec.SetSpaceChar();
                    if (GlyphIndex < 0)
                        grec.SetInvisibleChar();
                    else
                    {
                        RectF bnd;
                        pFont->GetGlyphBounds(GlyphIndex, &bnd);
                        if (bnd.IsEmpty())
                            grec.SetInvisibleChar();
                    }
                    ++LineCursor.NumOfSpaces; // count nbsp as a space for "justify"
                    if (isSpace)
                        ++LineCursor.NumOfTrailingSpaces;
                    else
                    {
                        // non-breaking space: do not treat them as trailing spaces
                        LineCursor.NumOfTrailingSpaces = 0;
                        LineCursor.LineWidthWithoutTrailingSpaces = NewLineWidth + AdjLineWidth;
                    }
                }
                else
                {
                    LineCursor.NumOfTrailingSpaces = 0;
                    LineCursor.LineWidthWithoutTrailingSpaces = NewLineWidth + AdjLineWidth;
                }
                grec.SetLength(1);

                // gather dimensions for the line
                LineCursor.TrackFontParams(pFont, Scale);

            }
            if (FindFontInfo.pCurrentFormat->IsUnderline())
                grec.SetUnderline();
            else
                grec.ClearUnderline();
            if (FindFontInfo.pCurrentFormat->IsUrlSet())
                grec.SetUrl();
            else
                grec.ClearUrl();

            LineCursor.pLastFont        = pFontHandle;
            LineCursor.LastColor        = color;
            LineCursor.LastKerning      = FindFontInfo.pCurrentFormat->IsKerning();
        }

        // check for tabs
        if (chInfo.Character == '\t')
        {
            if (TabStopsIndex < TabStopsNum)
            {
                int nextTabStopPos = PixelsToFixp<int>(ptabStops[TabStopsIndex]);
                ++TabStopsIndex;
                if (nextTabStopPos > NewLineWidth)
                {
                    GlyphAdvance = float(nextTabStopPos - NewLineWidth);
                }
            }
            else
            {
                // Value of "tabStopSize" is kind of empirical, found by doing tests in
                // Flash. However, it is still not completely the same as in Flash. Looks like
                // Flash aligns tabstops using stage coordinates. We use local text coordinates.
                float tabStopSize = PixelsToFixp(static_cast<float>(floor((FontSize*2 + 8)/8)*8));
                float nextTabStopPos = static_cast<float>(floor((NewLineWidth + tabStopSize)/tabStopSize)*tabStopSize);
                GlyphAdvance = nextTabStopPos - NewLineWidth;
            }
        }

        LineCursor.LineLength += grec.GetLength();

        LineCursor.pPrevGrec        = &grec;
        LineCursor.LastCharCode     = chInfo.Character;
        LineCursor.LineWidth        = NewLineWidth;

        LineCursor.LastGlyphIndex   = GlyphIndex;
        LineCursor.LastAdvance      = GlyphAdvance;
        if (FindFontInfo.pCurrentFormat->IsLetterSpacingSet())
            LineCursor.LastAdvance  += PixelsToFixp(FindFontInfo.pCurrentFormat->GetLetterSpacing());
        LineCursor.LastGlyphWidth   = GlyphWidth;

        if (LineCursor.GlyphIns.GetGlyphIndex() == 0)
            LineCursor.FirstGlyphWidth   = GlyphWidth;

        ++LineCursor.GlyphIns;
    }

    if (pTempLine)
    {
        // finalize the last line
        FinalizeLine();
    }
    if (pTextBufForCustomFormat && pTextBufForCustomFormat != TextBufForCustomFormat)
        pDocView->GetAllocator()->FreeText(pTextBufForCustomFormat);
}

void DocView::Format()
{
    // Reset LineBuffer's VisibleRect to ViewRect
    RectF _textRect = ViewRect;
    _textRect.Contract(GFX_TEXT_GUTTER);
    mLineBuffer.SetVisibleRect(_textRect);

    StyledText* pstyledText = GetStyledText();
    if (!pstyledText)
        return;

    unsigned oldMaxHScroll = GetMaxHScrollValue();
    unsigned oldMaxVScroll = GetMaxVScrollValue();

    // if auto text fit feature is on, then we need to reformat everything
    // to be able to scale it correctly.
    if (GetTextAutoSize() != TAS_None)
        SetCompleteReformatReq();

    StyledText::ParagraphsIterator paraIter = pstyledText->GetParagraphIterator();
    LineBuffer::Iterator linesIt = mLineBuffer.Begin();

    ParagraphFormatter formatter(this, pLog);
    int textWidth = 0, textHeight = 0;

    while(!paraIter.IsFinished())
    {
        const Paragraph* ppara = *paraIter;
        SF_ASSERT(ppara);
        if (!linesIt.IsFinished())
        {
            LineBuffer::Line& curLine = *linesIt;

            if (!IsCompleteReformatReq() && 
                ppara->GetId() == curLine.GetParagraphId() && 
                ppara->GetModCounter() == curLine.GetParagraphModId())
            {
                // if paragraph is not changed, then just update yoffset & textpos of each line
                //UPInt pos = ppara->GetStartIndex();
                UPInt pos = TextPos2GlyphOffset(ppara->GetStartIndex());
                for(; !linesIt.IsFinished(); ++linesIt)
                {
                    LineBuffer::Line& curLineBuffer = *linesIt;
                    if (ppara->GetId() != curLineBuffer.GetParagraphId())
                        break;
                    curLineBuffer.SetTextPos((unsigned)pos);
                    int oldX = (int)curLineBuffer.GetOffsetX();
                    int oldY = (int)curLineBuffer.GetOffsetY();

                    pos += curLineBuffer.GetTextLength();
                    curLineBuffer.SetOffsetY(formatter.NextOffsetY);

                    textWidth  =  Alg::Max(textWidth, curLineBuffer.GetWidth());
                    // In Flash, the last empty line doesn't participate in textHeight,
                    // unless this is editable text.
                    if (ppara->GetLength() != 0 || (HasEditorKit() && !GetEditorKit()->IsReadOnly()))
                        textHeight =  int(formatter.NextOffsetY + curLineBuffer.GetHeight());

                    formatter.NextOffsetY += curLineBuffer.GetHeight() + curLineBuffer.GetLeading();

                    // recenter line, if necessary
                    if (IsAutoSizeX())
                    {
                        switch(curLineBuffer.GetAlignment())
                        {
                        case LineBuffer::Line::Align_Right:
                            curLineBuffer.SetOffsetX(Alg::Max(0, Alg::IRound(GetTextRect().Width()) - curLineBuffer.GetWidth()));

                            // still need to set NeedRecenterLines to true, in the case if view rect width will be changed
                            formatter.NeedRecenterLines = true; 
                            break;
                        case LineBuffer::Line::Align_Center:
                            curLineBuffer.SetOffsetX(Alg::Max(0, Alg::IRound(GetTextRect().Width() / 2) - curLineBuffer.GetWidth() / 2));

                            // still need to set NeedRecenterLines to true, in the case if view rect width will be changed
                            formatter.NeedRecenterLines = true;
                            break;

                        default: break;
                        }
                    }
                    if (oldX != (int)curLineBuffer.GetOffsetX() || oldY != (int)curLineBuffer.GetOffsetY())
                        mLineBuffer.InvalidateCache(); //??AB: should it happen unconditionally (this will cause batch rebuild)?
                }
                ++paraIter;
                continue;
            }
            else // para was changed, removed or inserted
            {
                bool paraWasRemoved = (curLine.GetTextPos() == ~0u);
                // do we need to remove lines for this paragraph?
                if (paraWasRemoved) // para was removed
                {
                    unsigned l = 0;
                    UInt32 paraId = curLine.GetParagraphId();
                    for(LineBuffer::Iterator it = linesIt; !it.IsFinished(); ++l, ++it)
                    {
                        LineBuffer::Line& curLineBuffer = *it;
                        if (curLineBuffer.GetParagraphId() != paraId)
                            break;
                    }
                    if (l > 0)
                    {
                        linesIt.Remove(l);
                        mLineBuffer.InvalidateCache(); // force batching to be rebuilt
                    }
                    // was the paragraph removed completely? don't do formatting then
                    continue;
                }
                else if ((ppara->GetId() == curLine.GetParagraphId() && ppara->GetModCounter() != curLine.GetParagraphModId())) // para was changed
                {
                    // remove all lines by one call
                    unsigned l = 0;
                    for(LineBuffer::Iterator it = linesIt; !it.IsFinished(); ++l, ++it)
                    {
                        LineBuffer::Line& curLineBuffer = *it;
                        if (curLineBuffer.GetParagraphId() != ppara->GetId())
                            break;
                    }
                    linesIt.Remove(l);
                    mLineBuffer.InvalidateCache(); // force batching to be rebuilt
                }
            }
        }
        // else, no lines to modify - just add formatting for the paragraph

        // format paragraph
        formatter.pLinesIter     = &linesIt;
        formatter.ParaYOffset    = formatter.NextOffsetY;
        formatter.Format(*ppara);
        mLineBuffer.InvalidateCache(); // force batching to be rebuilt

        textWidth  =  Alg::Max(textWidth, formatter.ParaWidth);

        // In Flash, the last empty line doesn't participate in textHeight,
        // unless this is editable text.
        if (ppara->GetLength() != 0 || (HasEditorKit() && !GetEditorKit()->IsReadOnly()))
            textHeight = int(formatter.ParaYOffset + formatter.ParaHeight);

        ++paraIter;
    }
    // now need to remove all remaining lines in line buffer
    if (!linesIt.IsFinished())
    {
        linesIt.Remove(mLineBuffer.GetSize() - linesIt.GetIndex());
        mLineBuffer.InvalidateCache();
    }

    // assign new values for TextWidth & TextHeight
    SF_ASSERT(textWidth >= 0 && textHeight >= 0);
    TextWidth = (UInt32)textWidth;
    TextHeight = (UInt32)textHeight;

    //    mLineBuffer.Dump();

    ++FormatCounter; // force textWidth, etc format dependent values to be recalculated

    ClearReformatReq();

    // handle autosize
    bool valignApplied = false;
    ViewVAlignment valign = GetVAlignment();
    if (IsAutoSizeX() || IsAutoSizeY() || formatter.ForceVerticalCenterAutoSize)
    {
        RectF newViewRect = ViewRect;
        if (IsAutoSizeX())
        {
            float newW = TextWidth + GFX_TEXT_GUTTER*2;

            switch(GetAlignment())
            {
            case Align_Center:
                newViewRect.x1 += newViewRect.Width()/2 - newW/2;
                break;
            case Align_Right:
                newViewRect.x1 = newViewRect.x2 - newW;
                break;

            default: break;
            }
            newViewRect.SetWidth(newW);
        }

        if (IsAutoSizeY() || formatter.ForceVerticalCenterAutoSize)
        {
            float newH = TextHeight + GFX_TEXT_GUTTER*2;
            if ((valign == VAlign_None && formatter.ForceVerticalCenterAutoSize) || valign == VAlign_Center)
            {
                newViewRect.y1 += newViewRect.Height()/2 - newH/2;
            }
            else if (valign == VAlign_Bottom)
            {
                newViewRect.y1 = newViewRect.y2 - newH;
            }
            newViewRect.SetHeight(newH);
            valignApplied = true;
        }

        if (newViewRect != ViewRect)
        {
            SetViewRect(newViewRect, UseInternally);
        }
    }

    // handle auto text fit here. Calculate the scale and recalculate 
    // all dimensions and advances accordingly.
    ViewTextAutoSize tas;
    if ((tas = GetTextAutoSize()) != TAS_None)
    { 
        RectF maxTextRect = ViewRect;
        maxTextRect.Contract(GFX_TEXT_GUTTER);

        if (!IsMultiline() && valign == VAlign_None)
        {
            // if valign is not set but textAutoSize is specified for single line edit - 
            // use "center" by default. This behavior might be overridden by explicit
            // setting of valign property.
            valign = VAlign_Center;
        }
        float scale = 1.f;
        float hscale = 1.f, vscale = 1.f;
        if (tas == TAS_Shrink)
        {
            // try to fit the text into the text rect
            if (textWidth > maxTextRect.Width())
            {
                hscale = maxTextRect.Width()/textWidth;
            }
            if (textHeight > maxTextRect.Height())
            {
                vscale = maxTextRect.Height()/textHeight;
            }
            scale = Alg::Min(vscale, hscale);
        }
        else if (tas == TAS_Fit)
        {
            // need either expand or shrink the text to fill the text rect
            hscale = vscale = SF_MAX_FLOAT;
            if (textWidth >= 20)
                hscale = maxTextRect.Width()/textWidth;
            if (textHeight >= 20)
                vscale = maxTextRect.Height()/textHeight;
            scale = Alg::Min(vscale, hscale);
            if (scale == SF_MAX_FLOAT)
                scale = 1.0f; // do not scale if both textWidth & textHeight == 0
        }
        if (scale != 1.f)
        {
            // check limits for scaling
            int minLineH = mLineBuffer.GetMinLineHeight();
            if (minLineH > 0)
            {
                float fmlh = float(minLineH);
                if (fmlh*scale < GFX_MIN_LINE_HEIGHT_FOR_AUTOSIZE)
                    scale = GFX_MIN_LINE_HEIGHT_FOR_AUTOSIZE/fmlh;
            }
            TextWidth  = (UInt32)(textWidth  * scale);
            TextHeight = (UInt32)(textHeight * scale);
            mLineBuffer.Scale(scale);
        }
    }

    // also, we need to correct vertical scrolling values, 
    // if they are out of range after re-formatting. Need to do it BEFORE
    // handling vertical alignment.
    bool needOnScroller = false;
    unsigned maxVScroll = GetMaxVScroll();
    if(GetVScrollOffsetValue() > maxVScroll)
    {
        SetVScrollOffset(maxVScroll);
    }
    else if (maxVScroll != oldMaxVScroll)
    {
        needOnScroller = true;   
    }

    // handle vertical alignment. If vertical auto size is not on, but
    // the vertical alignment is specified then align the line buffer INSIDE the
    // existing view rectangle w/o changing view rectangle (vertical auto size
    // changes both view and text rectangles)
    if (!valignApplied && valign != VAlign_None && !IsAutoSizeY())
    {
        RectF maxTextRect = ViewRect;
        maxTextRect.Contract(GFX_TEXT_GUTTER);

        RectF newTextRect = GetTextRect();
        float textHeightRect = float(TextHeight);
        if (textHeightRect < maxTextRect.Height())
        {
            switch(valign)
            {
            case VAlign_Top:
                newTextRect = maxTextRect;
                break;
            case VAlign_Center:
                newTextRect.y1 = maxTextRect.y1 + (maxTextRect.Height()/2 - textHeightRect/2);
                break;
            case VAlign_Bottom:
                newTextRect.y1 = maxTextRect.y2 - textHeightRect;
                break;
            default: break;
            }
            mLineBuffer.SetVisibleRect(newTextRect);
        }
        else
        {
            newTextRect = maxTextRect;
            if (valign == VAlign_Center || valign == VAlign_Bottom)
            {
                float visTextHeight = 0;
                LineBuffer::Iterator linesItBuffer = mLineBuffer.Begin() + GetBottomVScroll();
                for(int i = 0; !linesItBuffer.IsFinished(); --linesItBuffer, ++i)
                {
                    LineBuffer::Line& curLine = *linesItBuffer;
                    int lh = curLine.GetHeight();
                    if (i != 0) // do not use leading for the very last line
                        lh += curLine.GetLeading();
                    if (visTextHeight + float(lh) <= maxTextRect.Height())
                        visTextHeight += lh;
                    else 
                        break;
                }
                if (valign == VAlign_Bottom)
                    newTextRect.y1 = maxTextRect.y2 - visTextHeight;
                else // center
                    newTextRect.y1 = maxTextRect.y1 + (maxTextRect.Height()/2 - visTextHeight/2);
            }
            mLineBuffer.SetVisibleRect(newTextRect);
        }
    }
    // check if we need to recenter lines. Usually we need to recenter lines in two cases:
    // 1) Text has alignment "center" or "right" and autoSizeX is true;
    // 2) Text has alignment "center" or "right" and textAutoSize is used.
    if (formatter.NeedRecenterLines)
    {
        // do we need to recenter lines? This might be required if AutoSizeX is on
        // and the width of view rect was changed because of formatting.
        LineBuffer::Iterator linesItBuffer = mLineBuffer.Begin();
        for(; !linesItBuffer.IsFinished(); ++linesItBuffer)
        {
            LineBuffer::Line& curLine = *linesItBuffer;
            switch(curLine.GetAlignment())
            {
            case LineBuffer::Line::Align_Right:
                curLine.SetOffsetX(Alg::Max(0, Alg::IRound(GetTextRect().Width()) - curLine.GetWidth()));
                break;
            case LineBuffer::Line::Align_Center:
                curLine.SetOffsetX(Alg::Max(0, Alg::IRound(GetTextRect().Width() / 2) - curLine.GetWidth() / 2));
                break;

            default: break;
            }
        }
    }

    SF_ASSERT(!IsReformatReq());

    // also, we need to correct horizontal scrolling values, 
    // if they are out of range after re-formatting
    unsigned maxHScroll = GetMaxHScroll();
    if(GetHScrollOffsetValue() > maxHScroll)
    {
        SetHScrollOffset(maxHScroll);
    }
    else if (maxHScroll != oldMaxHScroll)
    {
        needOnScroller = true;   
    }
    if (pDocumentListener)
    {   
        if (needOnScroller && pDocumentListener->DoesHandleOnMaxScrollChanged())
            pDocumentListener->View_OnMaxScrollChanged(*this);
        if (pDocumentListener->DoesHandleOnViewChanged())
            pDocumentListener->View_OnChanged(*this);
    }

    // check if we can turn off clipping. This is necessary to avoid clipping of the 
    // right side of right-aligned italic text, for example.
    if (IsReadOnly() && 
        TextWidth <= ViewRect.Width() && TextHeight <= ViewRect.Height() &&
        GetHScrollOffsetValue() == 0)
    {
        mLineBuffer.Geom.SetNoClipping();
    }
    else
        mLineBuffer.Geom.ClearNoClipping();

    SF_ASSERT(!IsReformatReq());
    mLineBuffer.CheckIntegrity();

    if (pHighlight)
    {
        pHighlight->HighlightManager.UpdateGlyphIndices
            ((HasEditorKit()) ? GetEditorKit()->GetCompositionString() : NULL);
    }

    ClearBitmapFontFlag();
    if (mLineBuffer.Lines.GetSize() > 0 && mLineBuffer.Lines[0]->GetNumGlyphs() > 0) 
    { 
        // Check the very first glyph if it has a font. If it does - 
        // check if the font is "pixel aligned", meaning "bitmap font" is used.
        // Set the appropriate flag to TextParam.
        FontHandle* pfont;
        if ((pfont = mLineBuffer.FindFirstFontInfo()) != NULL)
        {
            if (pfont->GetFont()->IsPixelAligned()) 
            { 
                SetBitmapFontFlag();
            } 
        }
    } 
}

void DocView::CreateVisibleTextLayout(TextLayout::Builder& bld)
{
    if (!IsBidirectionalTextRTFlagSet() && pHighlight && !pHighlight->IsValid())
    {
        pHighlight->HighlightManager.UpdateGlyphIndices
            ((HasEditorKit()) ? GetEditorKit()->GetCompositionString() : NULL);
        pHighlight->Validate();
    }

    TextFieldParam param;
    LoadTextFieldParamFromTextFilter(param, Filter);

    if (IsAAForReadability())
    {
        param.TextParam.SetOptRead(true);
        param.ShadowParam.SetOptRead(true);
    }
    if (IsAutoFit())
    {
        param.TextParam.SetAutoFit(true);
        param.ShadowParam.SetAutoFit(true);
    }
    param.TextParam.SetBitmapFont(IsBitmapFontFlagSet());
    param.TextParam.SetFauxBold(GetFauxBold());
    param.TextParam.SetFauxItalic(GetFauxItalic());
    param.TextParam.SetForceVector(GetForceVector());
    param.TextParam.SetOutline((unsigned)GetOutline());

    mLineBuffer.CreateVisibleTextLayout(bld, 
        (pHighlight && !IsBidirectionalTextRTFlagSet()) ? &pHighlight->HighlightManager : NULL, 
        param);
    
    // need to add background
    if (Color(BorderColor).GetAlpha() > 0 || Color(BackgroundColor).GetAlpha() > 0)
    {
        bld.SetBackground(BackgroundColor, BorderColor);
    }

    // need to add cursor (if necessary)
    if (pEditorKit)
        pEditorKit->AddDrawCursorInfo(bld);
    bld.SetBounds(GetViewRect());
}

bool    DocView::SetHScrollOffset(unsigned hscroll)
{
    bool rv = false;
    unsigned maxHScroll = GetMaxHScroll();
    if (hscroll > maxHScroll)
        hscroll = maxHScroll;
    if (mLineBuffer.GetHScrollOffset() != hscroll)
    {
        mLineBuffer.SetHScrollOffset(hscroll);
        if (GetDocumentListener())
        {
            GetDocumentListener()->View_OnHScroll(*this, hscroll);
        }
        rv = true;
    }
    return rv;
}

bool    DocView::SetVScrollOffset(unsigned vscroll)
{
    bool rv = false;
    unsigned maxVScroll = GetMaxVScroll();
    if (vscroll > maxVScroll)
        vscroll = maxVScroll;
    if (mLineBuffer.GetFirstVisibleLineIndex() != vscroll)
    {
        mLineBuffer.SetFirstVisibleLine(vscroll);
        if (GetDocumentListener())
        {
            GetDocumentListener()->View_OnVScroll(*this, vscroll);
        }
        rv = true;
    }
    return rv;
}

unsigned    DocView::GetHScrollOffset()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    return GetHScrollOffsetValue();
}

unsigned    DocView::GetVScrollOffset()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    return GetVScrollOffsetValue();
}

float   DocView::GetTextWidth()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    if (TextWidth > 0)
        return float(TextWidth);
    return 0;
}
float   DocView::GetTextHeight()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    if (TextHeight > 0)
        return float(TextHeight);
    return 0;
}

unsigned   DocView::GetLinesCount()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    return mLineBuffer.GetSize();
}

unsigned DocView::GetMaxHScrollValue() const
{
    if (IsWordWrap())
        return 0;
    int editorDelta = (!IsReadOnly()) ? GFX_EDIT_HSCROLL_DELTA : 0;
    unsigned v = unsigned(Alg::Max(0.f, ((TextWidth > 0) ? float(TextWidth) : 0) - 
        GetTextRect().Width() + editorDelta));
    return v;
}

unsigned DocView::GetMaxVScrollValue() const
{
    return MaxVScroll.Value;
}

unsigned DocView::GetMaxHScroll()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    return GetMaxHScrollValue();
}

unsigned  DocView::GetMaxVScroll()
{
    // check if reformatting is pending. If yes - force reformat first
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
    }
    if (!MaxVScroll.IsValid(FormatCounter))
    {
        if (mLineBuffer.GetSize() == 0)
            MaxVScroll.SetValue(0, FormatCounter);
        else
        {
            unsigned l = 0;
            LineBuffer::Iterator linesIt = mLineBuffer.Last();
            SF_ASSERT(!linesIt.IsFinished());
            const RectF& textRect = GetTextRect();

            const LineBuffer::Line* pbottomLine = linesIt.GetPtr();
            if (IsReadOnly() && pbottomLine->GetTextLength() == 0) 
            {
                // Flash doesn't count the last empty line for Read-Only textfields.
                --linesIt;
                ++l;
            }
            if (!linesIt.IsFinished())
            {
                pbottomLine = linesIt.GetPtr(); // it might be changed above
                int   lineH = pbottomLine->GetHeight(); // + pbottomLine->GetNonNegLeading();
                float top = textRect.y1 + (pbottomLine->GetOffsetY() + lineH - textRect.y2);

                for(; !linesIt.IsFinished(); --linesIt, ++l)
                {
                    const LineBuffer::Line& line = *linesIt;

                    if (l > 0 && line.GetOffsetY() < top)
                        break;
                }
                // MaxVScroll is zero based.
                MaxVScroll.SetValue(mLineBuffer.GetSize() - l, FormatCounter);
            }
            else
                MaxVScroll.SetValue(0, FormatCounter);
        }
    }
    return MaxVScroll;
}

bool DocView::IsReadOnly() const 
{ 
    return !HasEditorKit() || GetEditorKit()->IsReadOnly(); 
}

void DocView::SetAutoSizeX()     
{ 
    if (!IsAutoSizeX())
    {
        Flags |= Flags_AutoSizeX; 
        SetCompleteReformatReq();
    }
}

void DocView::SetAutoSizeY()     
{ 
    if (!IsAutoSizeY())
    {
        Flags |= Flags_AutoSizeY; 
        SetCompleteReformatReq();
    }
}

void DocView::SetWordWrap()      
{ 
    if (!IsWordWrap())
    {
        Flags |= Flags_WordWrap; 
        SetCompleteReformatReq();
    }
}

void DocView::ClearWordWrap()    
{ 
    if (IsWordWrap())
    {
        Flags &= (~Flags_WordWrap); 
        SetCompleteReformatReq();
    }
}

bool DocView::ForceReformat()
{
    if (IsReformatReq())
    {
        Format();
        ClearReformatReq();
        return true;
    }
    return false;
}

const RectF& DocView::GetViewRect() 
{ 
    ForceReformat();
    return ViewRect; 
}

UPInt DocView::GetCharIndexAtPoint(float x, float y)
{
    ForceReformat();
    x -= mLineBuffer.Geom.VisibleRect.x1 - ViewRect.x1;
    y -= mLineBuffer.Geom.VisibleRect.y1 - ViewRect.y1;
    LineBuffer::Iterator it = mLineBuffer.FindLineAtYOffset(y + mLineBuffer.GetVScrollOffsetInFixp());
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;

        float lineOffX = (float)line.GetOffsetX();
        if (x >= lineOffX && x <= lineOffX + float(line.GetWidth()))
        {
            float xoffInLine = x - lineOffX;
            int   xoffset = 0;

            // now need to find the glyph entry. Do the exhaustive search for now
            LineBuffer::GlyphIterator git = line.Begin();
            unsigned i = 0;
            for(; !git.IsFinished(); ++git)
            {
                const LineBuffer::GlyphEntry& ge = git.GetGlyph();
                xoffset += ge.GetAdvance();

                if (xoffset > xoffInLine)
                    break;

                i += ge.GetLength();
            }

            return line.GetTextPos() + i;
        }
    }
    return SF_MAX_UPINT;
}

UPInt DocView::GetCursorPosInLine(unsigned lineIndex, float x)
{
    if (lineIndex >= mLineBuffer.GetSize()) 
        return SF_MAX_UPINT;
    ForceReformat();
    //?x -= mLineBuffer.Geom.VisibleRect.x1 - ViewRect.x1;
    return GetCursorPosInLineByOffset(lineIndex, x);
}

UPInt DocView::GetCursorPosInLineByOffset(unsigned lineIndex, float relativeOffsetX)
{
    if (lineIndex >= mLineBuffer.GetSize()) 
        return SF_MAX_UPINT;
    LineBuffer::Line& line = mLineBuffer[lineIndex];

    //?? if (relativeOffsetX >= line.Offset.x && relativeOffsetX <= line.Offset.x + float(line.Width()))
    {
        float xoffInLine = relativeOffsetX - line.GetOffsetX() + mLineBuffer.GetHScrollOffset();
        int   xoffset = 0;

        // now need to find the glyph entry. Do the exhaustive search for now
        LineBuffer::GlyphIterator git = line.Begin();
        unsigned i = 0;
        for(; !git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& ge = git.GetGlyph();
            unsigned advance = ge.GetAdvance();

            if (xoffset + advance > xoffInLine)
            {
                if (xoffInLine - xoffset > advance / 2)
                {
                    i += ge.GetLength();
                }
                break;
            }

            xoffset += advance;
            if (!ge.IsNewLineChar())
                i += ge.GetLength();
        }

        return line.GetTextPos() + i;
    }
}

// x, y - coord on the screen
UPInt DocView::GetCursorPosAtPoint(float x, float y)
{
    ForceReformat();
    x -= mLineBuffer.Geom.VisibleRect.x1 - ViewRect.x1;
    y -= mLineBuffer.Geom.VisibleRect.y1 - ViewRect.y1;
    LineBuffer::Iterator it = mLineBuffer.FindLineAtYOffset(y + mLineBuffer.GetVScrollOffsetInFixp());
    if (it.IsFinished())
    {
        if (y <= 0)
            it = mLineBuffer.Begin();
        else
            // if it.IsFinished, then try to get Iterator pointed to the last line
            it = mLineBuffer.Begin() + mLineBuffer.GetSize() - 1;
    }
    if (!it.IsFinished())
    {
        return GetCursorPosInLineByOffset(it.GetIndex(), x);
    }
    return (mLineBuffer.GetSize() == 0) ? 0 : SF_MAX_UPINT;
}

// x, y - coord on the screen
bool DocView::IsUrlAtPoint(float x, float y, Range* purlPosRange)
{
    ForceReformat();
    x -= mLineBuffer.Geom.VisibleRect.x1 - ViewRect.x1;
    y -= mLineBuffer.Geom.VisibleRect.y1 - ViewRect.y1;
    LineBuffer::Iterator it = mLineBuffer.FindLineAtYOffset(y + mLineBuffer.GetVScrollOffsetInFixp());
    bool rv = false;
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;

        float lineOffX = (float)line.GetOffsetX();
        if (x >= lineOffX && x <= lineOffX + float(line.GetWidth()))
        {
            float xoffInLine = x - lineOffX + mLineBuffer.GetHScrollOffset();
            int   xoffset = 0;

            // now need to find the glyph entry. Do the exhaustive search for now
            LineBuffer::GlyphIterator git = line.Begin();
            unsigned i = 0;
            for(; !git.IsFinished(); ++git)
            {
                const LineBuffer::GlyphEntry& ge = git.GetGlyph();
                unsigned advance = ge.GetAdvance();

                if (xoffset + advance > xoffInLine)
                {
                    rv = ge.IsUrl();
                    break;
                }

                xoffset += advance;
                i += ge.GetLength();
            }
            if (rv && purlPosRange)
            {
                purlPosRange->Index = 0;
                purlPosRange->Length = 0;
                // retrieve position range for the url
                // searching from the position (i + line.GetTextPos()) in both directions
                UPInt posInDoc = i + line.GetTextPos();
                UPInt indexInPara = 0;
                StyledText::ParagraphsIterator paraIter = pDocument->GetParagraphByIndex(posInDoc, &indexInPara);
                if (!paraIter.IsFinished())
                {
                    Paragraph* ppara = *paraIter;
                    SF_ASSERT(ppara);
                    TextFormat formatAtThePoint = ppara->GetTextFormat(indexInPara, indexInPara+1);
                    SF_ASSERT(formatAtThePoint.IsUrlSet());
                    Paragraph::FormatRunIterator itFR = ppara->GetIterator();
                    for (; !itFR.IsFinished(); ++itFR)
                    {
                        UPInt indexInDoc = (UPInt)itFR->Index + ppara->GetStartIndex();
                        if (itFR->pFormat->IsUrlSet() && itFR->pFormat->GetUrl() == formatAtThePoint.GetUrl())
                        {
                            if ((UPInt)purlPosRange->Index + purlPosRange->Length < indexInDoc)
                            {
                                if (indexInDoc > posInDoc)
                                    break;
                                purlPosRange->Index  = indexInDoc;
                                purlPosRange->Length = itFR->Length;
                            }
                            else
                            {
                                purlPosRange->Length += itFR->Length;
                            }
                        }
                        else
                        {
                            if (indexInDoc > posInDoc)
                                break;
                            purlPosRange->Index = 0;
                            purlPosRange->Length = 0;
                        }
                    }
                }
            }
        }
    }
    return rv;
}

void DocView::SetFontScaleFactor(float f) 
{ 
    if (f == 1) 
    {
        RTFlags &= ~RTFlags_HasFontScaleFactor;
        FontScaleFactor = PixelsToFixp(UInt16(1));
    }
    else
    {
        RTFlags |= RTFlags_HasFontScaleFactor;
        FontScaleFactor = (UInt16)PixelsToFixp(f); 
    }
}

// Returns the zero-based index value of the line at the point specified by the x  and y parameters.
unsigned DocView::GetLineIndexAtPoint(float x, float y)
{
    SF_UNUSED(x);
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.FindLineAtYOffset(y + mLineBuffer.GetVScrollOffsetInFixp());
    if (!it.IsFinished())
    {
        return it.GetIndex();
    }
    return ~0u;
}

// Returns the zero-based index value of the line containing the character specified by the indexOfChar parameter.
unsigned DocView::GetLineIndexOfChar(UPInt indexOfChar)
{
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.FindLineByTextPos(indexOfChar);
    if (!it.IsFinished())
    {
        return it.GetIndex();
    }
    return ~0u;
}

// Returns the character index of the first character in the line that the lineIndex parameter specifies.
UPInt DocView::GetLineOffset(unsigned lineIndex)
{
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.Begin() + lineIndex;
    if (!it.IsFinished())
    {
        return it->GetTextPos();
    }
    return SF_MAX_UPINT;
}

// Returns the number of characters in a specific text line.
UPInt DocView::GetLineLength(unsigned lineIndex, bool* phasNewLine)
{
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.Begin() + lineIndex;
    if (!it.IsFinished())
    {
        if (phasNewLine)
            *phasNewLine = it->HasNewLine();
        return it->GetTextLength();
    }
    if (phasNewLine)
        *phasNewLine = false;
    return SF_MAX_UPINT;
}

// Returns the text of the line specified by the lineIndex parameter.
const wchar_t* DocView::GetLineText(unsigned lineIndex, UPInt* plen)
{
    if (!plen)
        return NULL;
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.Begin() + lineIndex;
    if (!it.IsFinished())
    {
        unsigned pos = it->GetTextPos();
        UPInt indexInPara = 0;
        StyledText::ParagraphsIterator pit = pDocument->GetParagraphByIndex(pos, &indexInPara);
        if (!pit.IsFinished())
        {
            *plen = it->GetTextLength();
            return pit->GetPtr()->GetText() + indexInPara;
        }
    }
    return NULL;
}

// Returns the length of the paragraph containing the charIndex
SPInt DocView::GetParagraphLength(UPInt charIndex)
{
    ForceReformat();
    UPInt indexInPara = 0;
    StyledText::ParagraphsIterator pit = pDocument->GetParagraphByIndex(charIndex, &indexInPara);
    if (!pit.IsFinished())
    {
        return pit->GetPtr()->GetLength();
    }
    return -1;
}

// Returns metrics information about a given text line.
bool DocView::GetLineMetrics(unsigned lineIndex, DocView::LineMetrics* pmetrics)
{
    if (!pmetrics)
        return false;
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.Begin() + lineIndex;
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;
        pmetrics->Ascent        = (unsigned)line.GetAscent();
        pmetrics->Descent       = (unsigned)line.GetDescent();
        pmetrics->Width         = (unsigned)line.GetWidth();
        pmetrics->Height        = (unsigned)line.GetHeight();
        pmetrics->Leading       = line.GetLeading();
        pmetrics->FirstCharXOff = (int)line.GetOffsetX();
        return true;
    }
    return false;
}

// Returns a rectangle that is the bounding box of the character.
// This version returns the same rectangle as Flash 9 does, based on advance. For newline
// symbols it returns false.
bool DocView::GetCharBoundaries(RectF* pCharRect, UPInt indexOfChar)
{
    SF_ASSERT(pCharRect);
    if (!pCharRect || indexOfChar >= pDocument->GetLength()) return false;
    ForceReformat();

    LineBuffer::Iterator it = mLineBuffer.FindLineByTextPos(indexOfChar);
    bool rv = false;
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;
        UPInt indexInLine = indexOfChar - line.GetTextPos();
        LineBuffer::GlyphIterator git = line.Begin();
        int advance = line.GetOffsetX();
        for(unsigned i = 0; !git.IsFinished(); ++git, ++i)
        {
            const LineBuffer::GlyphEntry& glyph = git.GetGlyph();
            if (i == indexInLine)
            {
                if (glyph.IsNewLineChar()) // do not return rect for newline, use GetExactCharBoundaries
                    break;
                rv = true;

                pCharRect->Clear();
                pCharRect->x2 += float(glyph.GetAdvance());
                pCharRect->y1      = GFX_TEXT_GUTTER;
                pCharRect->y2   = float(line.GetHeight());
                (*pCharRect) += Render::PointF(float(advance) + GFX_TEXT_GUTTER, float(line.GetOffsetY()));
                break;
            }
            advance += glyph.GetAdvance();
        }
    }
    return rv;
}

// Returns a rectangle that is the bounding box of the character.
// This method returns more exact rectangle, based on glyph width. For
// newline symbols it returns true and a rectangle with half width of a space.
bool DocView::GetExactCharBoundaries(RectF* pCharRect, UPInt indexOfChar)
{
    SF_ASSERT(pCharRect);
    UPInt len = pDocument->GetLength();
    if (!pCharRect || indexOfChar > len) return false;
    ForceReformat();

    LineBuffer::Iterator it = mLineBuffer.FindLineByTextPos(indexOfChar);
    bool rv = false;
    if (!it.IsFinished())
    {
        LineBuffer::Line& line = *it;
        UPInt indexInLine = indexOfChar - line.GetTextPos();
        LineBuffer::GlyphIterator git = line.Begin();
        int advance = 0;
        for(unsigned i = 0; !git.IsFinished(); ++git, ++i)
        {
            const LineBuffer::GlyphEntry& glyph = git.GetGlyph();
            if (i == indexInLine)
            {
                if (!glyph.HasFmtImage())
                {
                    // boundary of the font glyph
                    rv = true;
                    Font* pfont = git.GetFont();
                    SF_ASSERT(pfont);
                    float scale = PixelsToFixp(glyph.GetFontSize()) / 1024.0f; // the EM square is 1024 x 1024   

                    // Flash 9 calculates boundary box using advance only
                    // Though, it is not precise, to get precise boundary
                    // need to use GetGlyphBounds instead
                    if (pfont->IsMissingGlyph(glyph.GetIndex()))
                    {
                        pCharRect->x1 = 0;
                        pCharRect->x2 = float(glyph.GetAdvance());
                    }
                    else
                        pfont->GetGlyphBounds(glyph.GetIndex(), pCharRect);
                    if (glyph.IsNewLineChar())
                        pCharRect->SetWidth(pCharRect->Width()/3);

                    float textHeight = (pfont->GetAscent() + pfont->GetDescent()) * scale;
                    pCharRect->x1   *= scale;
                    pCharRect->x2   *= scale;
                    pCharRect->y1   = line.GetBaseLineOffset() - pfont->GetAscent() * scale + GFX_TEXT_GUTTER;
                    pCharRect->y2   = pCharRect->y1 + textHeight;
                    (*pCharRect) += Render::PointF(float(advance) + GFX_TEXT_GUTTER, float(line.GetOffsetY()));
                }
                else
                {
                    // boundary of the embedded image
                    rv = true;
                    ImageDesc* pimg = git.GetImage();
                    SF_ASSERT(pimg);
                    pCharRect->x1 = -pimg->GetBaseLineX();
                    pCharRect->y1  = -pimg->GetBaseLineY();
                    pCharRect->x2  = pCharRect->x1 + pimg->GetScreenWidth();
                    pCharRect->y2 = pCharRect->y1  + pimg->GetScreenHeight();
                    (*pCharRect) += Render::PointF(float(advance) + GFX_TEXT_GUTTER, line.GetOffsetY() + GFX_TEXT_GUTTER);
                }
                break;
            }
            advance += glyph.GetAdvance();
        }
    }
    return rv;
}
UPInt DocView::GetFirstCharInParagraph(UPInt indexOfChar)
{
    UPInt indexInPara = SF_MAX_UPINT;
    StyledText::ParagraphsIterator pit = pDocument->GetParagraphByIndex(indexOfChar, &indexInPara);
    if (!pit.IsFinished())
    {
        return indexOfChar - indexInPara;
    }
    return SF_MAX_UPINT;
}

unsigned DocView::GetBottomVScroll()
{
    ForceReformat();
    LineBuffer::Iterator it = mLineBuffer.BeginVisible(-float(mLineBuffer.GetVScrollOffsetInFixp()));
    unsigned index = 0;
    for(; it.IsVisible(); ++it)
    {
        index = it.GetIndex();
    }
    return index;
}

bool DocView::SetBottomVScroll(unsigned newBottomMostLine)
{
    if (newBottomMostLine >= mLineBuffer.GetSize())
        newBottomMostLine = mLineBuffer.GetSize() - 1;
    SPInt l = newBottomMostLine;
    LineBuffer::Iterator linesIt = mLineBuffer.Begin() + newBottomMostLine;
    if (!linesIt.IsFinished())
    {
        const LineBuffer::Line& bottomLine = *linesIt;
        const RectF& textRect = GetTextRect();
        int lineH = bottomLine.GetHeight() + bottomLine.GetNonNegLeading();
        float top = textRect.y1 + (bottomLine.GetOffsetY() + lineH - textRect.y2);
        unsigned topMostLine = newBottomMostLine;
        for(; l >= 0 && !linesIt.IsFinished(); --linesIt)
        {
            const LineBuffer::Line& line = *linesIt;
            if (line.GetOffsetY() < top)
                break;

            topMostLine = (unsigned)l;
            --l;
        }
        return SetVScrollOffset(topMostLine);
    }
    return false;
}

Highlighter* DocView::CreateHighlighterManager() 
{
    if (!pHighlight)
        pHighlight = SF_HEAP_AUTO_NEW(this) HighlightDescLoc;
    return &pHighlight->HighlightManager;
}

bool DocView::AddHighlight(HighlightDesc* pdesc)
{
    if (!pHighlight)
        pHighlight = SF_HEAP_AUTO_NEW(this) HighlightDescLoc;
    return (pHighlight->HighlightManager.CreateNewHighlighter(pdesc) != NULL);
}

bool DocView::RemoveHighlight(int id)
{
    if (pHighlight)
        return pHighlight->HighlightManager.FreeHighlighter(id);
    return false;
}

void DocView::UpdateHighlight(const HighlightDesc& desc)
{
    SF_UNUSED(desc); //?
    if (!pHighlight || IsBidirectionalTextRTFlagSet())
        return;
    pHighlight->Invalidate();
}

HighlightDesc* DocView::GetSelectionHighlighterDesc() const
{
    if (!pHighlight || IsBidirectionalTextRTFlagSet())
        return NULL;
    HighlightDesc* pdesc = NULL;
    if ((pdesc = pHighlight->HighlightManager.GetHighlighterPtr(GFX_TOPMOST_HIGHLIGHTING_INDEX)) == NULL)
    {
        HighlightDesc desc;
        desc.StartPos = 0;
        desc.Length   = 0;
        desc.Id = GFX_TOPMOST_HIGHLIGHTING_INDEX;
        desc.Info.SetBackgroundColor(GFX_ACTIVE_SEL_BKCOLOR);
        desc.Info.SetTextColor(GFX_ACTIVE_SEL_TEXTCOLOR);
        pdesc = pHighlight->HighlightManager.CreateHighlighter(desc);
        SF_ASSERT(pdesc);
    }
    return pdesc;
}

HighlightDesc* DocView::GetSelectionHighlighterDesc()
{
    if (IsBidirectionalTextRTFlagSet()) return NULL;

    if (!pHighlight)
        pHighlight = SF_HEAP_AUTO_NEW(this) HighlightDescLoc;
    return const_cast<const DocView*>(this)->GetSelectionHighlighterDesc();
}

void DocView::SetSelection(UPInt startPos, UPInt endPos, bool highlightSelection)
{
    BeginSelection = startPos;
    EndSelection   = endPos;

    if (highlightSelection)
    {
        if (!pHighlight)
            pHighlight = SF_HEAP_AUTO_NEW(this) HighlightDescLoc;
        if (endPos < startPos)
        {
            UPInt t = endPos;
            endPos = startPos;
            startPos = t;
        }
        if (!IsBidirectionalTextRTFlagSet())
        {
            HighlightDesc* pdesc = GetSelectionHighlighterDesc();
            SF_ASSERT(pdesc);

            // UPInt oldLen = 1;
            UPInt newLen = endPos - startPos;
            if (pdesc->StartPos != startPos || pdesc->Length != newLen)
            {
                // oldLen = pdesc->Length;
                pdesc->StartPos = startPos;
                pdesc->Length   = newLen;

                pHighlight->Invalidate();
            }
        }
    }
}

/*void DocView::GetSelection(UPInt* pstartPos, UPInt* pendPos)
{
SF_ASSERT(pstartPos && pendPos);
*pstartPos = 0;
*pendPos   = 0;
if (!pHighlight)
return;
HighlightDesc* pdesc;
if ((pdesc = pHighlight->HighlightManager.GetHighlighterPtr(GFX_TOPMOST_HIGHLIGHTING_INDEX)) != NULL)
{
*pstartPos = pdesc->StartPos;
*pendPos   = pdesc->StartPos + pdesc->Length;
}
}*/

void DocView::SetSelectionBackgroundColor(UInt32 color)
{
    if (IsBidirectionalTextRTFlagSet()) return;

    HighlightDesc* pdesc = GetSelectionHighlighterDesc();
    SF_ASSERT(pdesc);
    if (pdesc->Info.GetBackgroundColor() != color)
    {
        pdesc->Info.SetBackgroundColor(color);
        pHighlight->Invalidate();
    }
}

void DocView::SetSelectionTextColor(UInt32 color)
{
    if (IsBidirectionalTextRTFlagSet()) return;

    HighlightDesc* pdesc = GetSelectionHighlighterDesc();
    SF_ASSERT(pdesc);
    if (pdesc->Info.GetTextColor() != color)
    {
        pdesc->Info.SetTextColor(color);
        pHighlight->Invalidate();
    }
}

UInt32 DocView::GetSelectionBackgroundColor() const
{
    HighlightDesc* pdesc = GetSelectionHighlighterDesc();
    if (pdesc)
        return pdesc->Info.GetBackgroundColor().ToColor32();
    return GFX_ACTIVE_SEL_BKCOLOR;
}

UInt32 DocView::GetSelectionTextColor() const
{
    HighlightDesc* pdesc = GetSelectionHighlighterDesc();
    if (pdesc)
        return pdesc->Info.GetBackgroundColor().ToColor32();
    return GFX_ACTIVE_SEL_TEXTCOLOR;
}

void DocView::SetDefaultTextAndParaFormat(UPInt cursorPos)
{
    const ParagraphFormat* pparaFmt = NULL;
    const TextFormat* ptextFmt = NULL;

    UPInt indexInPara = GetFirstCharInParagraph(cursorPos);
    if (indexInPara != SF_MAX_UPINT)
    {
        bool res;
        if (indexInPara != cursorPos)
            res = pDocument->GetTextAndParagraphFormat(&ptextFmt, &pparaFmt, cursorPos - 1);
        else
            res = pDocument->GetTextAndParagraphFormat(&ptextFmt, &pparaFmt, cursorPos);
        if (res)
        {
            SetDefaultParagraphFormat(pparaFmt);
            SetDefaultTextFormat(ptextFmt);
        }
    }
}

UPInt DocView::TextPos2GlyphOffset(UPInt textPos)
{
    if (pEditorKit)
        return pEditorKit->TextPos2GlyphOffset(textPos);
    return textPos;
}

UPInt DocView::TextPos2GlyphPos(UPInt textPos)
{
    if (pEditorKit)
        return pEditorKit->TextPos2GlyphPos(textPos);
    return textPos;
}

UPInt DocView::GlyphPos2TextPos(UPInt glyphPos)
{
    if (pEditorKit)
        return pEditorKit->GlyphPos2TextPos(glyphPos);
    return glyphPos;
}

}}} // Scaleform::Render::Text
