/**************************************************************************

Filename    :   Text_HTMLParser.cpp
Content     :   Styled text implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Kernel/SF_AutoPtr.h"
#include "Kernel/SF_Alg.h"
#include "Render/Text/Text_StyledText.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { 
namespace Render {
namespace Text {

//////////////////////////////////////////////////////////////////////////
// HTML parsing routine
//
enum HTMLElementsEnum
{
    GFxHTML_A,
    GFxHTML_B,
    GFxHTML_BR,
    GFxHTML_FONT,
    GFxHTML_I,
    GFxHTML_IMG,
    GFxHTML_LI,
    GFxHTML_P,
    GFxHTML_SPAN,
    GFxHTML_TAB,
    GFxHTML_TEXTFORMAT,
    GFxHTML_U,

    GFxHTML_ALIGN,
    GFxHTML_ALPHA,
    GFxHTML_BASELINE,
    GFxHTML_BLOCKINDENT,
    GFxHTML_CLASS,
    GFxHTML_COLOR,
    GFxHTML_FACE,
    GFxHTML_HEIGHT,
    GFxHTML_HREF,
    GFxHTML_HSPACE,
    GFxHTML_ID,
    GFxHTML_INDENT,
    GFxHTML_KERNING,
    GFxHTML_LEADING,
    GFxHTML_LEFTMARGIN,
    GFxHTML_LETTERSPACING,
    GFxHTML_RIGHTMARGIN,
    GFxHTML_SRC,
    GFxHTML_SIZE,
    GFxHTML_TARGET,
    GFxHTML_TABSTOPS,
    GFxHTML_VSPACE,
    GFxHTML_WIDTH,

    GFxHTML_LEFT,
    GFxHTML_RIGHT,
    GFxHTML_CENTER,
    GFxHTML_JUSTIFY
};

struct SGMLElementDesc
{
    const char*         ElemName;
    HTMLElementsEnum    ElemId;
    bool                ReqEndElement;
    bool                EmptyElement;

    template <class Char>
    struct Comparable
    {
        const Char* Str;
        UPInt       Size;

        Comparable(const Char* p, UPInt sz):Str(p), Size(sz) {}
    };

    template <class Char>
    struct Comparator
    {
        static int Compare(const SGMLElementDesc& p1, const Comparable<Char>& p2)
        {
            return -SGMLCharIter<Char>::StrCompare(p2.Str, p1.ElemName, p2.Size);
        }
        static int Less(const SGMLElementDesc& p1, const Comparable<Char>& p2)
        {
            return (-SGMLCharIter<Char>::StrCompare(p2.Str, p1.ElemName, p2.Size)) < 0;
        }
    };
    template <class Char>
    static const SGMLElementDesc* FindElem
        (const Char* lookForElemName, UPInt nameSize, const SGMLElementDesc* ptable, UPInt  tableSize)
    {
        Comparable<Char> e(lookForElemName, nameSize);
        UPInt i = Alg::LowerBoundSliced(ptable, 0, tableSize, e, Comparator<Char>::Less);
        if (i < tableSize && Comparator<Char>::Compare(ptable[i], e) == 0)
        {
            return &ptable[i];
        }
        return NULL;
    }
};

template <class Char>
struct SGMLStackElemDesc
{
    const Char*                 pElemName;
    UPInt                       ElemNameSize;
    const SGMLElementDesc*   pElemDesc;
    UPInt                       StartPos;
    TextFormat               TextFmt;
    ParagraphFormat      ParaFmt;

    // Default constructor is ONLY for in-place construction in array.
    SGMLStackElemDesc()
        : pElemName(NULL),ElemNameSize(0), pElemDesc(NULL), TextFmt(Memory::GetHeapByAddress(&pElemName))
    { }
    SGMLStackElemDesc(MemoryHeap* pheap, const Char* pname, UPInt sz, const SGMLElementDesc* pelemDesc, UPInt startPos)
        : pElemName(pname), ElemNameSize(sz), pElemDesc(pelemDesc), StartPos(startPos),
          TextFmt(pheap)
    { }
};

template <class Char>
bool StyledText::ParseHtmlImpl(const Char* phtml, 
                                  UPInt htmlSize, 
                                  StyledText::HTMLImageTagInfoArray* pimgInfoArr, 
                                  bool multiline, 
                                  bool condenseWhite,
                                  const StyleManagerBase* pstyleMgr,
                                  const TextFormat* defTxtFmt, const ParagraphFormat* defParaFmt)
{
    static const SGMLElementDesc elementsTable[] = 
    {  // must be sorted by name!
        { "A",      GFxHTML_A,      true, false },
        { "B",      GFxHTML_B,      true, false },
        { "BR",     GFxHTML_BR,     false, true },
        { "FONT",   GFxHTML_FONT,   true, false },
        { "I",      GFxHTML_I,      true, false },
        { "IMG",    GFxHTML_IMG,    false, true },
        { "LI",     GFxHTML_LI,     false, false },
        { "P",      GFxHTML_P,      false, false },
        { "SBR",    GFxHTML_BR,     false, true },
        { "SPAN",   GFxHTML_SPAN,   true,  false },
        { "TAB",    GFxHTML_TAB,     false, true },
        { "TEXTFORMAT", GFxHTML_TEXTFORMAT, true, false },
        { "U",      GFxHTML_U,      true, false }
    };

    static const SGMLElementDesc attributesTable[] = 
    {  // must be sorted by name!
        { "ALIGN",      GFxHTML_ALIGN },
        { "ALPHA",      GFxHTML_ALPHA },
        { "BASELINE",   GFxHTML_BASELINE },
        { "BLOCKINDENT",GFxHTML_BLOCKINDENT },
        { "CENTER",     GFxHTML_CENTER },
        { "CLASS",      GFxHTML_CLASS },
        { "COLOR",      GFxHTML_COLOR },
        { "FACE",       GFxHTML_FACE },
        { "HEIGHT",     GFxHTML_HEIGHT },
        { "HREF",       GFxHTML_HREF },
        { "HSPACE",     GFxHTML_HSPACE },
        { "ID",         GFxHTML_ID },
        { "INDENT",     GFxHTML_INDENT },
        { "JUSTIFY",    GFxHTML_JUSTIFY },
        { "KERNING",    GFxHTML_KERNING },
        { "LEADING",    GFxHTML_LEADING },
        { "LEFT",       GFxHTML_LEFT },
        { "LEFTMARGIN", GFxHTML_LEFTMARGIN },
        { "LETTERSPACING", GFxHTML_LETTERSPACING },
        { "RIGHT",      GFxHTML_RIGHT },
        { "RIGHTMARGIN", GFxHTML_RIGHTMARGIN },
        { "SIZE",       GFxHTML_SIZE },
        { "SRC",        GFxHTML_SRC },
        { "TABSTOPS",   GFxHTML_TABSTOPS },
        { "TARGET",     GFxHTML_TARGET },
        { "VSPACE",     GFxHTML_VSPACE },
        { "WIDTH",      GFxHTML_WIDTH }
    };

    if (htmlSize == 0)
    {
        EnsureTermNull();
        CheckIntegrity();
        return false;
    }

    MemoryHeap* pheap = Memory::GetHeapByAddress(this);

    ArrayDH<SGMLStackElemDesc<Char> > elementsStack(pheap);

    SGMLParser<Char> parser(pheap, phtml, htmlSize);
    if (condenseWhite)
        parser.SetCondenseWhite();

    int type;
    Paragraph* pcurPara = NULL;
    const SGMLElementDesc* plastElemDesc = NULL;
    int p_elem_is_opened = 0;

    TextFormat defaultTextFmt        = (defTxtFmt)  ? *defTxtFmt  : *GetDefaultTextFormat();
    ParagraphFormat defaultParaFmt   = (defParaFmt) ? *defParaFmt : *GetDefaultParagraphFormat();

    TextFormat lastFormat = defaultTextFmt;

    //clear URL for just inserted text
    // This is necessary to prevent URL-style propagation after
    // closing </A> tag. 
    defaultTextFmt.SetUrl("");

    while((type = parser.GetNext()) != SGMLPS_FINISHED && type != SGMLPS_ERROR)
    {
        switch(type)
        {
        case SGMLPS_START_ELEMENT:
            {
                const Char* elemName = NULL;
                UPInt elemLen;

                parser.ParseStartElement(&elemName, &elemLen);

                const SGMLElementDesc* pelemDesc = SGMLElementDesc::FindElem<Char>
                    (elemName, elemLen, elementsTable, sizeof(elementsTable)/sizeof(elementsTable[0]));
                plastElemDesc = pelemDesc;

                int prevStackIndex = int(elementsStack.GetSize()-1);

                if (pelemDesc && pelemDesc->EmptyElement)
                {
                    switch(pelemDesc->ElemId)
                    {
                    case GFxHTML_BR:
                        // new paragraph
                        if (multiline)
                            AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt);
                        break;
                    case GFxHTML_TAB:
                        AppendString("\t", 1);
                        break;
                    case GFxHTML_IMG:
                        {
                            if (pimgInfoArr)
                            {
                                const Char* pattrName;
                                UPInt      attrSz;
                                pimgInfoArr->PushBack(HTMLImageTagInfo(pheap));
                                HTMLImageTagInfo&  imgInfo = (*pimgInfoArr)[pimgInfoArr->GetSize() - 1];
                                pcurPara = GetLastParagraph();
                                if (pcurPara == NULL)
                                    pcurPara = AppendNewParagraph(&defaultParaFmt);
                                TextFormat imgCharFmt(defaultTextFmt);
                                imgInfo.ParaId = pcurPara->GetId();
                                while(parser.GetNextAttribute(&pattrName, &attrSz))
                                {
                                    const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                        (pattrName, attrSz, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                    if (pattrDesc)
                                    {
                                        const Char* pattrVal;
                                        UPInt      attrValSz;                                        
                                        if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                        {
                                            switch(pattrDesc->ElemId)
                                            {
                                            case GFxHTML_SRC:
                                                imgInfo.Url.AppendString(pattrVal, attrValSz);
                                                break;
                                            case GFxHTML_ID:
                                                imgInfo.Id.AppendString(pattrVal, attrValSz);
                                                break;
                                            case GFxHTML_ALIGN:
                                                pattrDesc = SGMLElementDesc::FindElem<Char>
                                                    (pattrVal, attrValSz, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                                if (pattrDesc)
                                                {
                                                    switch(pattrDesc->ElemId)
                                                    {
                                                    case GFxHTML_LEFT:
                                                        imgInfo.Alignment = HTMLImageTagInfo::Align_Left;
                                                        break;
                                                    case GFxHTML_RIGHT:
                                                        imgInfo.Alignment = HTMLImageTagInfo::Align_Right;
                                                        break;
                                                    case GFxHTML_BASELINE:
                                                        imgInfo.Alignment = HTMLImageTagInfo::Align_BaseLine;
                                                        break;
                                                    default: break;
                                                    }
                                                }
                                                break;
                                            case GFxHTML_WIDTH:
                                                {
                                                    int v;
                                                    if (parser.ParseInt(&v, pattrVal, attrValSz) && v >= 0)
                                                    {
                                                        imgInfo.Width = PixelsToFixp((unsigned)v);
                                                    }
                                                }
                                                break;
                                            case GFxHTML_HEIGHT:
                                                {
                                                    int v;
                                                    if (parser.ParseInt(&v, pattrVal, attrValSz) && v >= 0)
                                                    {
                                                        imgInfo.Height = PixelsToFixp((unsigned)v);
                                                    }
                                                }
                                                break;
                                            case GFxHTML_VSPACE:
                                                {
                                                    int v;
                                                    if (parser.ParseInt(&v, pattrVal, attrValSz))
                                                    {
                                                        imgInfo.VSpace = PixelsToFixp(v);
                                                    }
                                                }
                                                break;
                                            case GFxHTML_HSPACE:
                                                {
                                                    int v;
                                                    if (parser.ParseInt(&v, pattrVal, attrValSz))
                                                    {
                                                        imgInfo.HSpace = PixelsToFixp(v);
                                                    }
                                                }
                                                break;
                                            default: break;
                                            }
                                        }
                                    }
                                }
                                Ptr<HTMLImageTagDesc> ptextImageDesc = *SF_HEAP_AUTO_NEW(this) HTMLImageTagDesc;
                                imgCharFmt.SetImageDesc(ptextImageDesc);
                                ptextImageDesc->ScreenWidth     = (float)imgInfo.Width;
                                ptextImageDesc->ScreenHeight    = (float)imgInfo.Height;
                                ptextImageDesc->VSpace          = imgInfo.VSpace;
                                ptextImageDesc->HSpace          = imgInfo.HSpace;
                                ptextImageDesc->Url             = imgInfo.Url;
                                ptextImageDesc->Id              = imgInfo.Id;
                                ptextImageDesc->Alignment       = imgInfo.Alignment;
                                imgInfo.pTextImageDesc = ptextImageDesc;
                                TextFormat* ptextFmt;
                                const ParagraphFormat* pparaFmt;
                                if (prevStackIndex >= 0)
                                {
                                    ptextFmt = &elementsStack[prevStackIndex].TextFmt;
                                    pparaFmt = &elementsStack[prevStackIndex].ParaFmt;
                                }
                                else
                                {
                                    ptextFmt = &defaultTextFmt;
                                    pparaFmt = &defaultParaFmt;
                                }
                                if (ptextFmt->IsUrlSet())
                                    imgCharFmt.SetUrl(ptextFmt->GetUrl());
                                AppendString(" ", 1, &imgCharFmt, pparaFmt);
                                ptextFmt->SetImageDesc(NULL);  // make sure IMG is not duplicated for following content
                                lastFormat.SetImageDesc(NULL); // make sure IMG is not duplicated for <br>
                                SetDefaultTextFormat(*ptextFmt);
								defaultTextFmt = *ptextFmt;
                            }
                        }
                        break;
                    default: break;
                    }
                }
                else
                {
                    UPInt curPos = GetLength();
                    elementsStack.PushBack(SGMLStackElemDesc<Char>(pheap, elemName, elemLen, pelemDesc, 0));
                    SGMLStackElemDesc<Char>& stackElem = elementsStack[elementsStack.GetSize()-1];
                    stackElem.StartPos = curPos;
                    if (prevStackIndex >= 0)
                    {
                        stackElem.TextFmt = elementsStack[prevStackIndex].TextFmt;
                        stackElem.ParaFmt = elementsStack[prevStackIndex].ParaFmt;
                    }
                    else
                    {
                        stackElem.TextFmt = defaultTextFmt;
                        stackElem.ParaFmt = defaultParaFmt;
                    }

                    if (!pelemDesc)
                    {
                        #ifdef GFX_ENABLE_CSS
                        // apply CSS class for arbitrary tag
                        if (pstyleMgr)
                        {
                            const Style* pstyle =
                                pstyleMgr->GetStyle(StyleManagerBase::CSS_Tag, elemName, elemLen);
                            if (pstyle)
                            {
                                stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                            }
                        }
                        #endif //SF_NO_CSS_SUPPORT
                    }
                    else
                    {
                        switch(pelemDesc->ElemId)
                        {
                        case GFxHTML_A:
                            const Char* pattrName;
                            UPInt      attrSz;
                            while(parser.GetNextAttribute(&pattrName, &attrSz))
                            {
                                const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                    (pattrName, attrSz, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                if (pattrDesc)
                                {
                                    const Char* pattrVal;
                                    UPInt       attrValSz;                                        
                                    if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                    {
                                        switch(pattrDesc->ElemId)
                                        {
                                        case GFxHTML_HREF:
                                            stackElem.TextFmt.SetUrl(pattrVal, attrValSz);
                                            SetMayHaveUrl();
                                            break;
                                        default: break;
                                        }
                                    }
                                }
                            }
                            #ifdef GFX_ENABLE_CSS
                            if (pstyleMgr)
                            {
                                // apply CSS style, if any
                                const Style* pstyle = 
                                    pstyleMgr->GetStyle(StyleManagerBase::CSS_Tag, "a");
                                if (pstyle)
                                {
                                    //?stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                    stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                                }
                                if (stackElem.TextFmt.IsUrlSet())
                                {
                                    // get a:link CSS style since there is a HREF attr
                                    const Style* pstyleLink =
                                        pstyleMgr->GetStyle(StyleManagerBase::CSS_Tag, "a:link");
                                    if (pstyleLink)
                                    {
                                        //?stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyleLink->mParagraphFormat);
                                        stackElem.TextFmt = stackElem.TextFmt.Merge(pstyleLink->mTextFormat);
                                    }
                                }
                            }
                            #endif //GFC_NO_CSS_SUPPORT
                            break;
                        case GFxHTML_LI:
                        {
                            stackElem.ParaFmt.SetBullet();
                            pcurPara = GetLastParagraph();
                            if (pcurPara == NULL)
                                pcurPara = AppendNewParagraph(&stackElem.ParaFmt);
                            else if (pcurPara->GetLength() == 0)
                                pcurPara->SetFormat(GetAllocator(), stackElem.ParaFmt);
                            else
                            {
                                if (multiline)
                                    AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt); 
                                pcurPara = GetLastParagraph();
                                pcurPara->SetFormat(GetAllocator(), stackElem.ParaFmt);
                            }
                            #ifdef GFX_ENABLE_CSS
                            // apply CSS class (on tag basis)
                            if (pstyleMgr)
                            {
                                const Style* pstyle =
                                    pstyleMgr->GetStyle(StyleManagerBase::CSS_Tag, elemName, elemLen);
                                if (pstyle)
                                {
                                    stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                    stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                                    pcurPara->SetFormat(GetAllocator(), stackElem.ParaFmt);
                                }
                            }
                            #endif //SF_NO_CSS_SUPPORT
                            break;
                        }
                        case GFxHTML_P:
                        {
                            if ((pcurPara = GetLastParagraph()) == NULL)
                            {
                                // no paragraphs yet
                                pcurPara = AppendNewParagraph(&defaultParaFmt);
                            }
                            else if (p_elem_is_opened)
                            {
                                if (multiline)
                                    AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt);
                                pcurPara = GetLastParagraph();
                            }
                            #ifdef GFX_ENABLE_CSS
                            // apply CSS class (on tag basis)
                            if (pstyleMgr)
                            {
                                const Style* pstyle =
                                    pstyleMgr->GetStyle(StyleManagerBase::CSS_Tag, elemName, elemLen);
                                if (pstyle)
                                {
                                    stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                    stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                                }
                            }
                            #endif //SF_NO_CSS_SUPPORT

                            // new paragraph
                            const Char* pattrNameGraph = nullptr;
                            UPInt       attrSzGraph;
                            while(parser.GetNextAttribute(&pattrNameGraph, &attrSzGraph))
                            {
                                const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                    (pattrNameGraph, attrSzGraph, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                if (pattrDesc)
                                {
                                    const Char* pattrVal;
                                    UPInt       attrValSz;                                        
                                    if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                    {
                                        switch(pattrDesc->ElemId)
                                        {
                                        case GFxHTML_CLASS:
                                            {
                                                #ifdef GFX_ENABLE_CSS
                                                // apply CSS class
                                                if (pstyleMgr)
                                                {
                                                    const Style* pstyle =
                                                        pstyleMgr->GetStyle(StyleManagerBase::CSS_Class, pattrVal, attrValSz);
                                                    if (pstyle)
                                                    {
                                                        stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                                        stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                                                    }
                                                }
                                                #endif //SF_NO_CSS_SUPPORT
                                            }
                                            break;
                                        case GFxHTML_ALIGN:
                                            {
                                                pattrDesc = SGMLElementDesc::FindElem<Char>
                                                    (pattrVal, attrValSz, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                                if (pattrDesc)
                                                {
                                                    switch(pattrDesc->ElemId)
                                                    {
                                                    case GFxHTML_LEFT:
                                                        stackElem.ParaFmt.SetAlignment(ParagraphFormat::Align_Left);
                                                        break;
                                                    case GFxHTML_RIGHT:
                                                        stackElem.ParaFmt.SetAlignment(ParagraphFormat::Align_Right);
                                                        break;
                                                    case GFxHTML_CENTER:
                                                        stackElem.ParaFmt.SetAlignment(ParagraphFormat::Align_Center);
                                                        break;
                                                    case GFxHTML_JUSTIFY:
                                                        stackElem.ParaFmt.SetAlignment(ParagraphFormat::Align_Justify);
                                                        break;
                                                    default: break;
                                                    }
                                                }
                                            }
                                            break;
                                        default: break;
                                        }
                                    }
                                }
                            }
                            if (pcurPara->GetLength() == 0)
                                pcurPara->SetFormat(GetAllocator(), stackElem.ParaFmt);
                            //}
                            ++p_elem_is_opened;
                            break;
                        }
                        case GFxHTML_B:
                            stackElem.TextFmt.SetBold(true);
                            break;
                        case GFxHTML_I:
                            stackElem.TextFmt.SetItalic(true);
                            break;
                        case GFxHTML_U:
                            stackElem.TextFmt.SetUnderline(true);
                            break;
                        case GFxHTML_FONT:
                            {
                                const Char* pattrNameFont = nullptr;
                                UPInt       attrSzFont;
                                while(parser.GetNextAttribute(&pattrNameFont, &attrSzFont))
                                {
                                    const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                        (pattrNameFont, attrSzFont, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                    if (pattrDesc)
                                    {
                                        const Char* pattrVal;
                                        UPInt       attrValSz;                                        
                                        if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                        {
                                            switch(pattrDesc->ElemId)
                                            {
                                            case GFxHTML_COLOR:
                                                // parse color: #RRGGBB
                                                if (*pattrVal == '#')
                                                {
                                                    UInt32 color;
                                                    if (parser.ParseHexInt(&color, pattrVal+1, attrValSz-1))
                                                    {
                                                        stackElem.TextFmt.SetColor32(color);    
                                                    }
                                                }
                                                break;
                                            case GFxHTML_FACE:
                                                stackElem.TextFmt.SetFontList(pattrVal, attrValSz);
                                                break;
                                            case GFxHTML_SIZE:
                                                // parse size
                                                int sz;
                                                if (parser.ParseInt(&sz, pattrVal, attrValSz) && sz >= 0)
                                                {
                                                    stackElem.TextFmt.SetFontSize((float)sz);
                                                }
                                                break;
                                            case GFxHTML_KERNING:
                                                // parse kerning
                                                int kern;
                                                if (parser.ParseInt(&kern, pattrVal, attrValSz) && kern >= 0)
                                                {
                                                    if (kern)
                                                        stackElem.TextFmt.SetKerning();
                                                    else
                                                        stackElem.TextFmt.ClearKerning();
                                                }
                                                break;
                                            case GFxHTML_LETTERSPACING:
                                                // parse letterspacing
                                                float ls;
                                                if (parser.ParseFloat(&ls, pattrVal, attrValSz))
                                                {
                                                    stackElem.TextFmt.SetLetterSpacing(ls);
                                                }
                                                break;
                                                // extension
                                            case GFxHTML_ALPHA:
                                                // parse alpha: #AA
                                                if (*pattrVal == '#')
                                                {
                                                    UInt32 alpha;
                                                    if (parser.ParseHexInt(&alpha, pattrVal+1, attrValSz-1))
                                                    {
                                                        stackElem.TextFmt.SetAlpha(UInt8(alpha));    
                                                    }
                                                }
                                                break;
                                            default: break;
                                            }
                                        }
                                    }
                                }
                                lastFormat = stackElem.TextFmt;
                            }

                            break;
                        case GFxHTML_TEXTFORMAT:
                            {
                                const Char* pattrNameTF = nullptr;
                                UPInt       attrSzTF;
                                while(parser.GetNextAttribute(&pattrNameTF, &attrSzTF))
                                {
                                    const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                        (pattrNameTF, attrSzTF, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                    if (pattrDesc)
                                    {
                                        const Char* pattrVal;
                                        UPInt       attrValSz;                                        
                                        if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                        {
                                            switch(pattrDesc->ElemId)
                                            {
                                            case GFxHTML_INDENT:
                                                {
                                                    // parse indent
                                                    int indent;
                                                    if (parser.ParseInt(&indent, pattrVal, attrValSz))
                                                    {
                                                        stackElem.ParaFmt.SetIndent(indent);
                                                    }
                                                    break;
                                                }
                                            case GFxHTML_BLOCKINDENT:
                                                {
                                                    // parse indent
                                                    int indent;
                                                    if (parser.ParseInt(&indent, pattrVal, attrValSz) && indent >= 0)
                                                    {
                                                        stackElem.ParaFmt.SetBlockIndent((unsigned)indent);
                                                    }
                                                    break;
                                                }
                                            case GFxHTML_LEADING:
                                                {
                                                    // parse indent
                                                    int leading;
                                                    if (parser.ParseInt(&leading, pattrVal, attrValSz))
                                                    {
                                                        stackElem.ParaFmt.SetLeading(leading);
                                                    }
                                                    break;
                                                }
                                            case GFxHTML_LEFTMARGIN:
                                                {
                                                    // parse margin
                                                    int margin;
                                                    if (parser.ParseInt(&margin, pattrVal, attrValSz) && margin >= 0)
                                                    {
                                                        stackElem.ParaFmt.SetLeftMargin((unsigned)margin);
                                                    }
                                                    break;
                                                }
                                            case GFxHTML_RIGHTMARGIN:
                                                {
                                                    // parse margin
                                                    int margin;
                                                    if (parser.ParseInt(&margin, pattrVal, attrValSz) && margin >= 0)
                                                    {
                                                        stackElem.ParaFmt.SetRightMargin((unsigned)margin);
                                                    }
                                                    break;
                                                }
                                            case GFxHTML_TABSTOPS:
                                                {
                                                    // parse tabstops as [xx,xx,xx]
                                                    // calculate number of tabstops first
                                                    UPInt i = 0;
                                                    while(i < attrValSz && SGMLCharIter<Char>::IsSpace(pattrVal[i]))
                                                        ++i;
                                                    if (pattrVal[i] == '[')
                                                        ++i; // skip the [
                                                    unsigned n = 1;
                                                    UPInt j;
                                                    for (j = i; j < attrValSz && pattrVal[j] != ']'; ++j)
                                                    {
                                                        if (pattrVal[j] == ',')
                                                            ++n;
                                                    }
                                                    stackElem.ParaFmt.SetTabStopsNum(n);
                                                    unsigned idx = 0;
                                                    for (j = i; j < attrValSz && pattrVal[j] != ']'; )
                                                    {
                                                        unsigned v = 0;
                                                        for (; j < attrValSz && SGMLCharIter<Char>::IsDigit(pattrVal[j]); ++j)
                                                        {
                                                            v *= 10;
                                                            v += (pattrVal[j] - '0');
                                                        }
                                                        while(j < attrValSz && SGMLCharIter<Char>::IsSpace(pattrVal[j]))
                                                            ++j;
                                                        stackElem.ParaFmt.SetTabStopsElement(idx, v);
                                                        ++idx;
                                                        if (pattrVal[j] == ',')
                                                        {
                                                            ++j;
                                                            while(j < attrValSz && SGMLCharIter<Char>::IsSpace(pattrVal[j]))
                                                                ++j;
                                                        }
                                                    }
                                                }
                                                break;
                                            default: break;
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case GFxHTML_SPAN:
                            {
                                #ifdef GFX_ENABLE_CSS
                                const Char* pattrNameSPA = nullptr;
                                UPInt       attrSzSPA;
                                while(parser.GetNextAttribute(&pattrNameSPA, &attrSzSPA))
                                {
                                    const SGMLElementDesc* pattrDesc = SGMLElementDesc::FindElem<Char>
                                        (pattrNameSPA, attrSzSPA, attributesTable, sizeof(attributesTable)/sizeof(attributesTable[0]));
                                    if (pattrDesc)
                                    {
                                        const Char* pattrVal;
                                        UPInt       attrValSz;                                        
                                        if (parser.GetNextAttributeValue(&pattrVal, &attrValSz))
                                        {
                                            switch(pattrDesc->ElemId)
                                            {
                                                case GFxHTML_CLASS:
                                                    // apply CSS class
                                                    if (pstyleMgr)
                                                    {
                                                        const Style* pstyle =
                                                            pstyleMgr->GetStyle(StyleManagerBase::CSS_Class, pattrVal, attrValSz);
                                                        if (pstyle)
                                                        {
                                                            //?stackElem.ParaFmt = stackElem.ParaFmt.Merge(pstyle->mParagraphFormat);
                                                            stackElem.TextFmt = stackElem.TextFmt.Merge(pstyle->mTextFormat);
                                                            lastFormat = stackElem.TextFmt;
                                                        }
                                                    }
                                                    break;
                                                default: break;
                                            }
                                        }
                                    }
                                }
                                #endif //SF_NO_CSS_SUPPORT
                            }
                            break;
                        default: break;
                        }
                    }
                    lastFormat = stackElem.TextFmt;
                }
            }
            break;
        case SGMLPS_CONTENT:
            {
                const Char* content;
                UPInt  contentSize;
                if (parser.ParseContent(&content, &contentSize))
                {
                    if (contentSize > 0)
                    {
                        pcurPara = GetLastParagraph();
                        UPInt paraLen = (pcurPara) ? pcurPara->GetLength() : 0;
                        // do not add space at the beginning of paragraph, if condenseWhite
                        if (!condenseWhite || contentSize > 1 || content[0] != ' ' || paraLen > 0)
                        {
                            if (elementsStack.GetSize() > 0)
                            {
                                SGMLStackElemDesc<Char>& stackElem = elementsStack[elementsStack.GetSize()-1];
                                AppendString(content, contentSize, &stackElem.TextFmt, &stackElem.ParaFmt);
                                lastFormat = stackElem.TextFmt;
                            }
                            else
                            {
                                AppendString(content, contentSize, &defaultTextFmt, &defaultParaFmt);
                                lastFormat = defaultTextFmt; 
                            }
                        }
                    }
                }
            }
            break;
        case SGMLPS_END_ELEMENT:
        case SGMLPS_EMPTY_ELEMENT_FINISHED:
            {
                const Char* elemName = NULL;
                UPInt  elemLen       = 0;

                UPInt startPos;
                const SGMLStackElemDesc<Char>* pstackElem = NULL;
                if (type == SGMLPS_END_ELEMENT)
                {
                    if (elementsStack.GetSize() > 0)
                        pstackElem = &elementsStack[elementsStack.GetSize()-1];
                    parser.ParseEndElement(&elemName, &elemLen);
                }
                else
                {
                    // ignore end elements for BR & TAB
                    if (plastElemDesc)
                    {
                        if (plastElemDesc->EmptyElement)
                            break;
                    }
                    if (elementsStack.GetSize() > 0)
                    {
                        pstackElem = &elementsStack[elementsStack.GetSize()-1];
                        elemName = pstackElem->pElemName;
                        elemLen  = pstackElem->ElemNameSize;
                    }
                    else
                        break; // shouldn't get here at all.
                }
                if (pstackElem)
                {
                    // verify matching of end-element and start-element
                    if (parser.StrCompare(elemName, elemLen, pstackElem->pElemName, pstackElem->ElemNameSize) != 0)
                    {
                        // mismatch!
                        // just ignore such an element for now. Do we need to print
                        // kinda warning?
                        const SGMLElementDesc* pelemDesc = SGMLElementDesc::FindElem<Char>
                            (elemName, elemLen, elementsTable, sizeof(elementsTable)/sizeof(elementsTable[0]));
                        if (pelemDesc && p_elem_is_opened && (pelemDesc->ElemId == GFxHTML_P || pelemDesc->ElemId == GFxHTML_LI))
                        {
                            // looks like, if </p> end tag is used w/o closing all tags nested to it
                            // then it should close all opened tags till <p>
                            while(elementsStack.GetSize() > 0)
                            {
                                const SGMLStackElemDesc<Char>& stackElem = elementsStack[elementsStack.GetSize()-1];
                                if (stackElem.pElemDesc && pelemDesc->ElemId == stackElem.pElemDesc->ElemId)
                                    break;
                                elementsStack.PopBack();
                            }
                            if (elementsStack.GetSize() > 0)
                                pstackElem = &elementsStack[elementsStack.GetSize()-1];
                            else
                                break;
                        }
                        else
                            break;
                    }
                    bool isDisplaySet = pstackElem->ParaFmt.IsDisplaySet();
                    ParagraphFormat::DisplayType display = pstackElem->ParaFmt.GetDisplay();
                    plastElemDesc = pstackElem->pElemDesc;
                    startPos = pstackElem->StartPos;
                    elementsStack.PopBack();

                    if (plastElemDesc)
                    {
                        switch(plastElemDesc->ElemId)
                        {
                        case GFxHTML_P:
                            --p_elem_is_opened;
                            // Empty <P/> element doesn't produce a new paragraph in Flash
                            if (type == SGMLPS_EMPTY_ELEMENT_FINISHED)
                                break;
                        case GFxHTML_LI:
                            if (multiline)
                                AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt);
                            break;
                        default: break;
                        }
                    }
                    else
                    {
                        // arbitrary tags handling, if necessary
                        if (pstyleMgr)
                        {
                            // if CSS is used, check if we have block or none display
                            if (isDisplaySet)
                            {
                                switch(display)
                                {
                                case ParagraphFormat::Display_Block:
                                    // create a new paragraph
                                    if (multiline)
                                        AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt);
                                    break;
                                case ParagraphFormat::Display_None:
                                    {
                                    // create a new paragraph and remove the invisible text
                                    UPInt curPos = GetLength();
                                    if (multiline)
                                    {
                                        AppendString(NewLineStr(), 1, &lastFormat, &defaultParaFmt);
                                    }
                                    Remove(startPos, curPos - startPos);
                                    break;
                                    }
                                default:;
                                }
                            }

                        }
                    }
                }
                else
                {
                    // wrong end elem, ignore...
                    break;
                }
            }
            break;
        default: break;
        }
    }
    if (elementsStack.GetSize() > 0)
    {
        SGMLStackElemDesc<Char>& stackElem = elementsStack[elementsStack.GetSize()-1];
        SetDefaultTextFormat(stackElem.TextFmt);
        SetDefaultParagraphFormat(stackElem.ParaFmt);
        lastFormat = stackElem.TextFmt;
    }
    else
    {
        SetDefaultTextFormat(defaultTextFmt);
        SetDefaultParagraphFormat(defaultParaFmt);
    }
    EnsureTermNull();

    // Flash treats empty paragraphs at the end of the specific way. For example, if 
    // HTML code is as follows: 
    // "<p align="left"><font face="Arial" size="20" color="#ffffff" letterSpacing="5" kerning="1">Test 12345</font></p><p align="left"></p>"
    // then the last paragraph should be empty w/o any format data (<p align="left"></p>). Though,
    // Flash propagates the last known text format on the empty paragraphs. The code below does
    // this in GFx.
    Paragraph* ppara = GetLastParagraph();
    if (ppara && ppara->GetLength() == 0)
    {
        ParagraphsIterator paraIter = Paragraphs.Last();
        while(!paraIter.IsFinished())
        {
            Paragraph* pcurParaGraph = paraIter->GetPtr();
            if (pcurParaGraph->GetLength() <= 1)
            {
                pcurParaGraph->SetTextFormat(GetAllocator(), lastFormat, 0, SF_MAX_UPINT);
            }
            else
                break;
            --paraIter;
        }
    }

    CheckIntegrity();
    return true;
}

bool StyledText::ParseHtml(const char* phtml, UPInt  htmlSize, HTMLImageTagInfoArray* pimgInfoArr, 
               bool multiline, bool condenseWhite, const StyleManagerBase* pstyleMgr,
               const TextFormat* txtFmt, const ParagraphFormat* paraFmt)
{
// In order to reduce code size just decode the UTF-8 phtml into wchar_t and use wchar_t version
// of the ParseHtml. However, for best performance the char* version should be used.
//    return ParseHtmlImpl(phtml, htmlSize, pimgInfoArr, multiline, condenseWhite, pstyleMgr);
    wchar_t* pwbuf = (wchar_t*)SF_ALLOC((htmlSize + 1) * sizeof(wchar_t), StatRender_Text_Mem);
    htmlSize = UTF8Util::DecodeStringSafe(pwbuf, htmlSize + 1, phtml, htmlSize);
    bool rv = ParseHtmlImpl(pwbuf, htmlSize, pimgInfoArr, multiline, condenseWhite, pstyleMgr, txtFmt, paraFmt);
    SF_FREE(pwbuf);
    return rv;
}

bool StyledText::ParseHtml(const wchar_t* phtml, UPInt  htmlSize, HTMLImageTagInfoArray* pimgInfoArr, 
               bool multiline, bool condenseWhite, const StyleManagerBase* pstyleMgr,
               const TextFormat* txtFmt, const ParagraphFormat* paraFmt)
{
    return ParseHtmlImpl(phtml, htmlSize, pimgInfoArr, multiline, condenseWhite, pstyleMgr, txtFmt, paraFmt);
}
}}} // Scaleform::Render::Text
