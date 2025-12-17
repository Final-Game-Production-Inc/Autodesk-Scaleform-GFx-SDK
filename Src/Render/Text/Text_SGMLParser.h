/**************************************************************************

PublicHeader:   Render
Filename    :   Text_SGMLParser.h
Content     :   SGML (markup language used for HTML) parser
Created     :   June, 2007
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_SGMLPARSER_H
#define INC_SF_GFX_SGMLPARSER_H

#include "Kernel/SF_Std.h"
#include "Kernel/SF_MemoryHeap.h"

namespace Scaleform { 
namespace Render {
namespace Text {

enum SGMLParserState
{
    SGMLPS_NONE,
    SGMLPS_ERROR,
    SGMLPS_START_ELEMENT,
    SGMLPS_CONTENT,
    SGMLPS_END_ELEMENT,
    SGMLPS_EMPTY_ELEMENT,
    SGMLPS_EMPTY_ELEMENT_NAME_END,
    SGMLPS_START_ELEMENT_ATTR,
    SGMLPS_START_ELEMENT_ATTR_VALUE,
    SGMLPS_START_ELEMENT_END_OF_ATTRS,

    SGMLPS_FINISHED = 0x8000,

    SGMLPS_EMPTY_ELEMENT_FINISHED = SGMLPS_EMPTY_ELEMENT | SGMLPS_FINISHED,
    SGMLPS_START_ELEMENT_FINISHED = SGMLPS_START_ELEMENT | SGMLPS_FINISHED,
    SGMLPS_END_ELEMENT_FINISHED = SGMLPS_END_ELEMENT | SGMLPS_FINISHED
};

template <typename Char>
class SGMLCharIter
{
    const Char*  pCurChar;
    const Char*  pNextChar;
    const Char*  pEnd;
    UInt32       CurChar;
    bool         DoContentParsing;

    UInt32 DecodeCurrentChar() { return 0; } // should be specialized
    UInt32 DecodeEscapedChar();
public:
    SGMLCharIter(const Char* p, UPInt sz): 
      pCurChar(p), pNextChar(p), pEnd(p + sz), CurChar(0), DoContentParsing(false) 
      { operator++(); }

    bool IsFinished() const { return pCurChar >= pEnd; }
    UInt32 operator*() const { return CurChar; }
    void operator++() 
    {
        pCurChar = pNextChar;
        if (IsEscapedChar())
            DecodeEscapedChar();
        else
            DecodeCurrentChar();
    }
    const Char* GetCurCharPtr() const { return pCurChar; }
    unsigned GetCurCharLength() const { return (unsigned)(pNextChar - pCurChar); }
    bool IsEscapedChar() const { return DoContentParsing && *pCurChar == '&'; }
    void StartContentParsing() 
    { 
        bool ov = DoContentParsing;
        DoContentParsing = true; 
        if (!ov && IsEscapedChar())
            DecodeEscapedChar();
    }
    void EndContentParsing() { DoContentParsing = false; }

    // helper funcs
    static int EncodeCharTo(UInt32 uniChar, Char* pdestBuf, UPInt destBufSz) { SF_UNUSED2(uniChar, pdestBuf); return 0; } // need to be specialized

    static bool IsSpace(UInt32 c)
    {
        return (SFiswspace((wchar_t)c) != 0);
    }
    static bool IsAlpha(UInt32 c)
    {
        return (SFiswalpha((wchar_t)c) != 0);
    }
    static bool IsAlphaNum(UInt32 c)
    {
        return (SFiswalnum((wchar_t)c) != 0);
    }
    static bool IsDigit(UInt32 c)
    {
        return (isdigit((int)c) != 0);
    }
    static bool IsXDigit(UInt32 c)
    {
        return (isxdigit((int)c) != 0);
    }
    static UInt32 ToLower(UInt32 c)
    {
        return SFtowlower((wchar_t)c);
    }
    static int StrCompare(const Char* wstr, const char* str, UPInt len);
    static int StrCompare(const Char* dst, UPInt dstlen, const Char* src, UPInt srclen);
};

template <class Char>
UInt32 SGMLCharIter<Char>::DecodeEscapedChar()
{
    if (pCurChar < pEnd)
    {
        pNextChar = pCurChar;
        if (*pNextChar == '&')
        {
            CurChar = *pNextChar;
            ++pNextChar;
            do 
            {
                if (pNextChar + 5 <= pEnd)
                {
                    if (StrCompare(pNextChar, "quot;", 5) == 0)
                    {
                        CurChar = '\"';
                        pNextChar += 5;
                        break;
                    }
                    else if (StrCompare(pNextChar, "apos;", 5) == 0)
                    {
                        CurChar = '\'';
                        pNextChar += 5;
                        break;
                    }
                    else if (StrCompare(pNextChar, "nbsp;", 5) == 0)
                    {
                        CurChar = 160;//'\xA0';
                        pNextChar += 5;
                        break;
                    }
                }
                if (pNextChar + 4 <= pEnd)
                {
                    if (StrCompare(pNextChar, "amp;", 4) == 0)
                    {
                        CurChar = '&';
                        pNextChar += 4;
                        break;
                    }
                }
                if (pNextChar + 3 <= pEnd)
                {
                    if (StrCompare(pNextChar, "lt;", 3) == 0)
                    {
                        CurChar = '<';
                        pNextChar += 3;
                        break;
                    }
                    if (StrCompare(pNextChar, "gt;", 3) == 0)
                    {
                        CurChar = '>';
                        pNextChar += 3;
                        break;
                    }
                }
                if (pNextChar + 2 <= pEnd)
                {
                    if (*pNextChar == '#')
                    {
                        const Char* psavedNext = pNextChar;
                        ++pNextChar;
                        UInt32 code = 0;
                        if (ToLower(*pNextChar == 'x'))
                        {
                            ++pNextChar;

                            // hexadecimal code of char
                            while(pNextChar < pEnd && *pNextChar != ';')
                            {
                                if (!IsXDigit(*pNextChar))
                                {
                                    code = ~0u;
                                    break; // wrong symbol, cancel
                                }
                                code <<= 4;
                                Char c = (Char)ToLower(*pNextChar); 
                                if (c >= '0' && c <= '9')
                                    code |= ((c - '0') & 0xF);
                                else if (c >= 'a' && c <= 'f')
                                    code |= (((c - 'a') & 0xF) + 10);
                                ++pNextChar;
                            }
                            if (code != ~0u && *pNextChar == ';') // skip the ';'
                                ++pNextChar;
                        }
                        else if (IsDigit(*pNextChar))
                        {
                            // decimal code
                            while(pNextChar < pEnd && *pNextChar != ';')
                            {
                                if (!IsDigit(*pNextChar))
                                {
                                    code = ~0u;
                                    break; // wrong symbol, cancel
                                }
                                code *= 10;
                                code += (*pNextChar - '0');
                                ++pNextChar;
                            }
                            if (code != ~0u && *pNextChar == ';') // skip the ';'
                                ++pNextChar;
                        }
                        else
                        {
                            // wrong value
                            code = ~0u;
                        }
                        if (code != ~0u)
                            CurChar = code;
                        else
                            pNextChar = psavedNext; // restore pNextChar
                    }
                    break;
                }
            } while(0);
        }
        return CurChar;
    }
    return 0;   
}

template <class Char>
int SGMLCharIter<Char>::StrCompare(const Char* wstr, const char* str, UPInt len) 
{
    if (len)
    {
        int f,l;
        SPInt slen = (SPInt)len;
        const char *s = str;
        do {
            f = (int)ToLower((UInt32)(*(wstr++)));
            l = (int)ToLower((UInt32)(*(str++)));
        } while (--len && f && (f == l) && *str != 0);

        if (f == l && (len != 0 || *str != 0))
        {
            f = (int)slen;
            l = (int)SFstrlen(s);
            return f - l;
        }

        return f - l;
    }
    else
        return (0-(int)SFstrlen(str));
}

template <class Char>
int SGMLCharIter<Char>::StrCompare(const Char* dst, UPInt dstlen, const Char* src, UPInt srclen) 
{
    if (dstlen)
    {
        int f,l;
        SPInt slen = (SPInt)srclen, dlen = (SPInt)dstlen;
        do {
            f = (int)ToLower((UInt32)(*(dst++)));
            l = (int)ToLower((UInt32)(*(src++)));
        } while (--dstlen && f && (f == l) && --srclen);

        if (f == l && (dstlen != 0 || srclen != 0))
        {
            return (int)(dlen - slen);
        }
        return f - l;
    }
    else
        return 0-(int)srclen;
}

template <>
inline UInt32 SGMLCharIter<wchar_t>::DecodeCurrentChar() 
{
    if (pCurChar < pEnd)
    {
        CurChar = *pCurChar;
        pNextChar = pCurChar + 1;
        return CurChar;
    }
    return 0; 
}

template <>
inline int SGMLCharIter<wchar_t>::EncodeCharTo(UInt32 uniChar, wchar_t* pdestBuf, UPInt destBufSz)
{
    SF_UNUSED(destBufSz);
    *pdestBuf = (wchar_t)uniChar;
    return 1;
}


template <>
inline UInt32 SGMLCharIter<char>::DecodeCurrentChar() 
{
    if (pCurChar < pEnd)
    {
        pNextChar = pCurChar;
        CurChar = UTF8Util::DecodeNextChar(&pNextChar);
        return CurChar;
    }
    return 0; 
}

template <>
inline int SGMLCharIter<char>::EncodeCharTo(UInt32 uniChar, char* pdestBuf, UPInt destBufSz)
{
    SPInt off = 0;
    UTF8Util::EncodeCharSafe(pdestBuf, destBufSz, &off, uniChar);
    return (int)off;
}


template <class Char>
class SGMLParser
{
public:
protected:
    MemoryHeap*         pHeap;
    int                 CurState;
    SGMLCharIter<Char>  Iter;
    Char*               pBuffer;
    UPInt               BufSize;
    UPInt               BufPos;
    bool                CondenseWhite;

    void SkipName();
    void SkipSpaces();
    void SkipComment();
    void SkipAttribute();
    void ParseName(const Char** ppname, UPInt* plen);
public:
    SGMLParser(MemoryHeap* pheap, const Char* phtml, UPInt htmlSize) :
      pHeap(pheap),
      CurState(SGMLPS_NONE), Iter(phtml, htmlSize), pBuffer(NULL), BufSize(0), 
      BufPos(0), CondenseWhite(false)
    { }
    ~SGMLParser()
    {
        SF_HEAP_FREE(pHeap, pBuffer);
    }

    // buffer ops
    void    ResetBuf() { BufPos = 0; }
    void    AppendToBuf(const Char*, UPInt);
    void    AppendCharToBuf(UInt32 uniChar);

    int     GetNext();
    bool    GetNextAttribute(const Char** ppattrName, UPInt* pattrNameSz);
    bool    GetNextAttributeValue(const Char** ppattrValue, UPInt* pattrValue);
    bool    ParseStartElement(const Char** ppelemName, UPInt* pelemLen);
    bool    ParseEndElement(const Char** ppelemName, UPInt* pelemLen);
    bool    ParseContent(const Char** ppContent, UPInt* pcontentSize);
    
    bool    HasAttributes() const { return CurState == SGMLPS_START_ELEMENT_ATTR; }
    void    SetCondenseWhite() { CondenseWhite = true; }

    // helper funcs
    static int StrCompare(const Char* wstr, const char* str, UPInt len)
    {
        return SGMLCharIter<Char>::StrCompare(wstr, str, len); 
    }
    static int StrCompare(const Char* pstr1, UPInt len1, const Char* pstr2, UPInt len2)
    {
        return SGMLCharIter<Char>::StrCompare(pstr1, len1, pstr2, len2); 
    }
    static bool ParseInt(int* pdestVal, const Char* pstr, UPInt len);
    static bool ParseHexInt(UInt32* pdestVal, const Char* pstr, UPInt len);
    static bool ParseFloat(float* pdestVal, const Char* pstr, UPInt len);
};

template <class Char>
void    SGMLParser<Char>::AppendToBuf(const Char* pstr, UPInt sz)
{
    if (BufPos + sz > BufSize)
    {
        BufSize += sz;
        if (!pBuffer)
            pBuffer = (Char*)SF_HEAP_ALLOC(pHeap, BufSize * sizeof(Char), StatRender_Text_Mem);
        else
            pBuffer = (Char*)SF_REALLOC(pBuffer, BufSize * sizeof(Char), StatRender_Text_Mem);
    }
    memcpy(pBuffer + BufPos, pstr, sz * sizeof(Char));
    BufPos += sz;
}

template <class Char>
void    SGMLParser<Char>::AppendCharToBuf(UInt32 uniChar)
{
    if (BufPos + 6 > BufSize) // 6 - max len of Unicode char
    {
        BufSize += 6; //? do we need some kind of macro here?
        if (!pBuffer)
            pBuffer = (Char*)SF_HEAP_ALLOC(pHeap, BufSize * sizeof(Char), StatRender_Text_Mem);
        else
            pBuffer = (Char*)SF_REALLOC(pBuffer, BufSize * sizeof(Char), StatRender_Text_Mem);
    }
    BufPos += Iter.EncodeCharTo(uniChar, pBuffer + BufPos, (BufSize - BufPos));
}

template <class Char>
int SGMLParser<Char>::GetNext()
{
    if (CurState == SGMLPS_ERROR)
        return SGMLPS_ERROR;

    if (!(CurState & SGMLPS_FINISHED))
    {
        // if there are unfinished states - skip them
        switch(CurState)
        {
        case SGMLPS_NONE:
            break;
        case SGMLPS_START_ELEMENT:
            SkipName();
        case SGMLPS_START_ELEMENT_ATTR:
        case SGMLPS_START_ELEMENT_ATTR_VALUE:
            while (CurState == SGMLPS_START_ELEMENT_ATTR || CurState == SGMLPS_START_ELEMENT_ATTR_VALUE)
            {
                SkipAttribute();
            }
        case SGMLPS_START_ELEMENT_END_OF_ATTRS: // handled in the next switch
            if (*Iter == '>')
            {
                ++Iter;
                CurState = SGMLPS_FINISHED | SGMLPS_START_ELEMENT;
            }
            break;
        case SGMLPS_CONTENT:
            // skip content, look for '<'
            while (!Iter.IsFinished() && *Iter != '<')
            {
                ++Iter;
            }
            CurState = SGMLPS_FINISHED | SGMLPS_CONTENT;
            break;
        case SGMLPS_END_ELEMENT:
            // skip end element name, look for '>'
            while (!Iter.IsFinished() && *Iter != '>')
            {
                ++Iter;
            }
            if (*Iter == '>')
            {
                ++Iter;
                CurState = SGMLPS_FINISHED | SGMLPS_END_ELEMENT;
            }
            else
                CurState = SGMLPS_ERROR;
            break;
        default: break;
        }
    }   

    if (CurState == SGMLPS_ERROR)
        return SGMLPS_ERROR;

    SGMLParserState newState = SGMLPS_NONE;
    while(!Iter.IsFinished() && newState == SGMLPS_NONE)
    {
        switch (*Iter)
        {
        case 0: newState = SGMLPS_ERROR; break;

        case '<':
            {
                ++Iter;
                if (*Iter == '!') // comment?
                {
                    SkipComment();
                }
                else if (*Iter == '/') // ending elem
                {
                    ++Iter;
                    newState = SGMLPS_END_ELEMENT;
                }
                else
                    newState = SGMLPS_START_ELEMENT;
                break;
            }
        case '/':
            {
                if (CurState == SGMLPS_START_ELEMENT_END_OF_ATTRS || 
                    CurState == SGMLPS_EMPTY_ELEMENT_NAME_END)
                {
                    ++Iter;

                    if (*Iter == '>')
                    {
                        ++Iter;
                        newState = SGMLPS_EMPTY_ELEMENT_FINISHED;
                    }
                    else
                    {
                        // no space is allowed between / and >. So HTML is wrong. What to do?
                        newState = SGMLPS_ERROR;
                    }
                }
                else 
                    newState = SGMLPS_CONTENT;
                break;
            }
        default:
            {
                newState = SGMLPS_CONTENT;
            }
            break;
        }
    }
    if (newState == SGMLPS_NONE)
        CurState = SGMLPS_FINISHED;
    else
        CurState = newState;
    return CurState;
}

template <class Char>
bool      SGMLParser<Char>::GetNextAttribute(const Char** ppattrName, UPInt* pattrNameSz)
{
    if (CurState == SGMLPS_ERROR)
        return false;

    if (CurState == SGMLPS_START_ELEMENT_ATTR_VALUE)
    {
        SkipAttribute();
    }
    bool rv = false;
    while(CurState == SGMLPS_START_ELEMENT_ATTR && !Iter.IsFinished())
    {
        SF_ASSERT(ppattrName && pattrNameSz);

        ParseName(ppattrName, pattrNameSz);

        SkipSpaces();
        if (!Iter.IsFinished())
        {
            if (*Iter == '=')
            {
                ++Iter;
                SkipSpaces();
                CurState = SGMLPS_START_ELEMENT_ATTR_VALUE;
                rv = true;
            }
            else
            {
                // wrong first char in attr name - skip it and its value
                SkipAttribute();
            }
        }
    }
    if (Iter.IsFinished())
        CurState = SGMLPS_ERROR;
    return rv;
}

template <class Char>
bool    SGMLParser<Char>::GetNextAttributeValue(const Char** ppattrValue, UPInt* pattrValueSz)
{
    bool rv = false;
    if (CurState == SGMLPS_START_ELEMENT_ATTR_VALUE)
    {
        SF_ASSERT(ppattrValue && pattrValueSz);

        UInt32 quote = *Iter;
        if (quote == '\"' || quote == '\'')
        {
            ++Iter;
            *ppattrValue = Iter.GetCurCharPtr();
            *pattrValueSz = 0;
            bool isInBuf = false;

            Iter.StartContentParsing();
            while(!Iter.IsFinished() && *Iter != quote)
            {
                if (Iter.IsEscapedChar())
                {
                    if (!isInBuf)
                    {
                        ResetBuf();
                        AppendToBuf(*ppattrValue, *pattrValueSz);
                        isInBuf = true;
                    }

                    AppendCharToBuf(*Iter);
                }
                else
                {
                    if (isInBuf)
                        AppendToBuf(Iter.GetCurCharPtr(), Iter.GetCurCharLength());
                    else
                        (*pattrValueSz) += Iter.GetCurCharLength();
                }
                ++Iter;

            }
            Iter.EndContentParsing();
            if (isInBuf)
            {
                *ppattrValue = pBuffer;
                *pattrValueSz = BufPos;
            }

            if (Iter.IsFinished())
            {
                CurState = SGMLPS_ERROR;
            }
            else
            {
                rv = true;
                ++Iter; // skip trailing quot
                SkipSpaces();
                if (*Iter == '>' || *Iter == '/')
                    CurState = SGMLPS_START_ELEMENT_END_OF_ATTRS;
                else
                    CurState = SGMLPS_START_ELEMENT_ATTR;
            }
        }
        else
        {
            CurState = SGMLPS_ERROR;
        }
    }
    if (Iter.IsFinished())
        CurState = SGMLPS_ERROR;
    return rv;
}

template <class Char>
void        SGMLParser<Char>::ParseName(const Char** ppname, UPInt* plen)
{
    SF_ASSERT(ppname && plen);
    *ppname = Iter.GetCurCharPtr();
    *plen = 0;
    bool isInBuf = false;

    // look for either > / or space
    while (!Iter.IsFinished() && 
        *Iter != '=' && *Iter != '>' && *Iter != '<' && *Iter != '>' && *Iter != '/' && !Iter.IsSpace(*Iter))
    {
        if (Iter.IsEscapedChar())
        {
            if (!isInBuf)
            {
                ResetBuf();
                AppendToBuf(*ppname, *plen);
                isInBuf = true;
            }

            AppendCharToBuf(*Iter);
        }
        else
        {
            if (isInBuf)
                AppendToBuf(Iter.GetCurCharPtr(), Iter.GetCurCharLength());
            else
                (*plen) += Iter.GetCurCharLength();
        }
        ++Iter;
    }
    if (isInBuf)
    {
        *ppname = pBuffer;
        *plen = BufPos;
    }
}

template <class Char>
bool        SGMLParser<Char>::ParseStartElement(const Char** ppelemName, UPInt* pelemLen)
{
    bool rv = false;
    if (CurState == SGMLPS_START_ELEMENT)
    {
        SF_ASSERT(ppelemName && pelemLen);

        ParseName(ppelemName, pelemLen);

        rv = true;
        if (*Iter == '>')
        {
            CurState = SGMLPS_START_ELEMENT_FINISHED;
            ++Iter;
        }
        else if (*Iter == '/')
        {
            // looks like this is an empty element, so set
            // the appropriate state
            CurState = SGMLPS_EMPTY_ELEMENT_NAME_END;
        }
        else
        {
            CurState = SGMLPS_START_ELEMENT_ATTR;
            SkipSpaces();
        }
    }
    return rv;
}

template <class Char>
bool        SGMLParser<Char>::ParseEndElement(const Char** ppelemName, UPInt* pelemLen)
{
    bool rv = false;
    if (CurState == SGMLPS_END_ELEMENT)
    {
        SF_ASSERT(ppelemName && pelemLen);

        ParseName(ppelemName, pelemLen);

        if (*Iter == '>')
        {
            CurState = SGMLPS_END_ELEMENT_FINISHED;
            ++Iter;
            rv = true;
        }
        else
        {
            CurState = SGMLPS_ERROR;
        }
    }
    return rv;
}
template <class Char>
bool        SGMLParser<Char>::ParseContent(const Char** ppContent, UPInt* pcontentSize)
{
    if (CurState == SGMLPS_CONTENT)
    {
        SF_ASSERT(ppContent && pcontentSize);
        *ppContent = Iter.GetCurCharPtr();
        *pcontentSize = 0;
        bool isInBuf = false;
        
        Iter.StartContentParsing();
        // look for either > / or space
        while (!Iter.IsFinished() && (*Iter != '<' || Iter.IsEscapedChar()))
        {
            if (Iter.IsSpace(*Iter) && CondenseWhite)
            {
                if (!isInBuf)
                {
                    ResetBuf();
                    AppendToBuf(*ppContent, *pcontentSize);
                    isInBuf = true;
                }
                AppendCharToBuf(' ');
                SkipSpaces();
                continue;
            }

            if (Iter.IsEscapedChar())
            {
                if (!isInBuf)
                {
                    ResetBuf();
                    AppendToBuf(*ppContent, *pcontentSize);
                    isInBuf = true;
                }

                AppendCharToBuf(*Iter);
            }
            else
            {
                if (isInBuf)
                    AppendToBuf(Iter.GetCurCharPtr(), Iter.GetCurCharLength());
                else
                    (*pcontentSize) += Iter.GetCurCharLength();
            }
            ++Iter;
        }
        if (isInBuf)
        {
            *ppContent = pBuffer;
            *pcontentSize = BufPos;
        }

        if (Iter.IsFinished() || (*Iter == '<' && !Iter.IsEscapedChar()))
        {
            CurState |= SGMLPS_FINISHED;
        }
        else 
            CurState = SGMLPS_ERROR;
        Iter.EndContentParsing();
    }
    return (CurState != SGMLPS_ERROR);
}

template <class Char>
void        SGMLParser<Char>::SkipName()
{
    if (CurState == SGMLPS_START_ELEMENT)
    {
        // look for either > / or space
        while (!Iter.IsFinished() && 
            *Iter != '=' && *Iter != '>' && *Iter != '<' && *Iter != '/' && !Iter.IsSpace(*Iter))
        {
            ++Iter;
        }
    }
}

template <class Char>
void        SGMLParser<Char>::SkipSpaces()
{
    while(!Iter.IsFinished() && Iter.IsSpace(*Iter))
    {
        ++Iter;
    }
}

template <class Char>
void        SGMLParser<Char>::SkipAttribute()
{
    if (CurState == SGMLPS_START_ELEMENT_ATTR)
    {
        SkipSpaces();
        // skip the name till the '='
        while (!Iter.IsFinished() && Iter.IsAlphaNum(*Iter) && *Iter != '=')
        {
            ++Iter;
        }
        if (Iter.IsFinished())
        {
            CurState = SGMLPS_ERROR;
            return;
        }
        if (*Iter == '=')
        {
            ++Iter;
            SkipSpaces();
            CurState = SGMLPS_START_ELEMENT_ATTR_VALUE; 
        }
        else if (*Iter == '/' || *Iter == '>')
            CurState = SGMLPS_START_ELEMENT_END_OF_ATTRS;
        else
            CurState = SGMLPS_ERROR;
    }
    if (CurState == SGMLPS_START_ELEMENT_ATTR_VALUE)
    {
        UInt32 quote = *Iter;
        if (quote == '\"' || quote == '\'')
        {
            do 
            {
                ++Iter;
            } while(!Iter.IsFinished() && *Iter != quote);
            if (Iter.IsFinished())
            {
                CurState = SGMLPS_ERROR;
                return;
            }
            ++Iter;
            SkipSpaces();
            if (*Iter == '>' || *Iter == '/')
                CurState = SGMLPS_START_ELEMENT_END_OF_ATTRS;
            else
                CurState = SGMLPS_START_ELEMENT_ATTR;
        }
        else
        {
            CurState = SGMLPS_ERROR;
            return;
        }
    }
}

template <class Char>
void        SGMLParser<Char>::SkipComment()
{
    int state = 0;
    while(!Iter.IsFinished() && *Iter != 0 && state != 3)
    {
        ++Iter;
        if (*Iter == '-')
        {
            if (state < 2)
                ++state;
        }
        else if (*Iter == '>')
        {
            if (state == 2)
                ++state;
        }
        else
            state = 0;
    }
    if (!Iter.IsFinished() && state == 3)
        ++Iter; // skip '>'
}

template <class Char>
bool SGMLParser<Char>::ParseInt(int* pdestVal, const Char* pstr, UPInt len)
{
    if (len == 0)
        return false;
    int v = 0, sign = 1;
    const Char* p = pstr;
    if (*p == '-')
    {
        sign = -1;
        ++p; --len;
    }
    else
    {
        if (*p == '+') { ++p; --len; }
    }
    for (UPInt i = 0; i < len; ++i, ++p)
    {
        if (!SGMLCharIter<Char>::IsDigit(*p))
            return false;
        v *= 10;
        v += (*p - '0');
    }
    *pdestVal = v * sign;
    return true;
}

template <class Char>
bool SGMLParser<Char>::ParseHexInt(UInt32* pdestVal, const Char* pstr, UPInt len)
{
    if (len == 0)
        return false;
    UInt32 v = 0;
    const Char* p = pstr;
    for (UPInt i = 0; i < len; ++i, ++p)
    {
        if (!SGMLCharIter<Char>::IsXDigit(*p))
            return false;
        v <<= 4;
        Char c = (Char)SGMLCharIter<Char>::ToLower(*p); 
        if (c >= '0' && c <= '9')
            v |= ((c - '0') & 0xF);
        else if (c >= 'a' && c <= 'f')
            v |= (((c - 'a') & 0xF) + 10);
    }
    *pdestVal = v;
    return true;
}

template <class Char>
bool SGMLParser<Char>::ParseFloat(float* pdestVal, const Char* pstr, UPInt len)
{
    if (len == 0)
        return false;
    Double v = 0, sign = 1;
    const Char* p = pstr;
    const Char* const pend = pstr + len;
    if (*p == '-')
    {
        sign = -1;
        ++p;
    }
    else
    {
        if (*p == '+') ++p;
    }
    // parse integer part
    for (; p < pend && *p != '.' && *p != ','; ++p)
    {
        if (!SGMLCharIter<Char>::IsDigit(*p))
            return false;
        v *= 10;
        v += (*p - '0');
    }
    if (p < pend && (*p == '.' || *p == ','))
    {
        ++p; // skip decimal point
        Double frac = 0;
        // parse fractional part
        for (; p < pend; ++p)
        {
            if (!SGMLCharIter<Char>::IsDigit(*p))
                return false;
            frac += (*p - '0');
            frac *= Double(0.1);
        }
        v += frac;
    }
    *pdestVal = (float)(v * sign);
    return true;
}
}}} // Scaleform::Render::Text

#endif // INC_SF_GFX_SGMLPARSER_H

