/**************************************************************************

Filename    :   Text_StyleSheet.cpp
Content     :   StyleSheet (Textfield.StyleSheet) object functionality
Created     :   May 20, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/Text/Text_StyleSheet.h"
#include "Render/Text/Text_SGMLParser.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { 
namespace GFx {
namespace Text {

#ifdef GFX_ENABLE_CSS

using Render::Text::ParagraphFormat;
using Render::Text::TextFormat;
using Render::Text::SGMLCharIter;
using Render::Text::SGMLParser;

// Enable verbose parsing trace
#ifdef SF_BUILD_DEBUG
//#define SF_CSS_VERBOSE_TRACE
#endif

//
// Tokens relevant to CSS1
//
template <typename T>
struct CSSToken
{
    enum TokenType
    {
        TT_Name,
        TT_QString,      // quoted string
        TT_WhiteSpace,
        TT_Number,
        TT_HexNumber,
        TT_Comma,
        TT_OpenBrace,
        TT_CloseBrace,
        TT_SemiColon,
        TT_Colon,
        TT_EOF,
        TT_Unknown,
    };

    TokenType       Type;
    const T*        pBase;
    UPInt           Length;

    CSSToken() : Type(TT_Unknown), pBase(0), Length(0) {}
    CSSToken(TokenType type) : Type(type), pBase(0), Length(0) {}
    CSSToken(TokenType type, const T* pbase, UPInt sz) 
        : Type(type), pBase(pbase), Length(sz) {}

    bool operator== (TokenType type)
    {
        return Type == type;
    }

    void operator+= (const CSSToken& tok)
    {
        SF_ASSERT( (pBase + Length) == tok.pBase ); // appending only works for contiguous tokens
        Length += tok.Length;
    }
};


//
// Tokenizer for CSS1 style sheets (supports only the Flash subset)
//
template <typename T>
class CSSTokenizer
{
public:
    CSSTokenizer(const T* buffer, UPInt len) 
        : pCurr(buffer), pEnd(buffer+len), CurrentToken(CSSToken<T>::TT_Unknown), 
          LineNo(1), ColNo(1), LineDev(0), ColDev(0) {}

    CSSToken<T>     GetNextToken();

    UPInt           GetLineNo()     { return LineNo; }
    UPInt           GetColNo()      { return ColNo; }

private:
    const T*        pCurr;
    const T*        pEnd;
    
    CSSToken<T>     CurrentToken;

    UPInt           LineNo;
    UPInt           ColNo;
    UPInt           LineDev;
    UPInt           ColDev;

    int             IsSpace(T c);
    int             IsDigit(T c);
    int             IsXDigit(T c);
    int             IsAlpha(T c);
    int             IsAlNum(T c);
    
};

template <> inline int  CSSTokenizer<char>::IsSpace(char c) { return isspace(c); }
template <> inline int  CSSTokenizer<wchar_t>::IsSpace(wchar_t c) { return SFiswspace(c); }

template <> inline int  CSSTokenizer<char>::IsDigit(char c) { return isdigit(c); }
template <> inline int  CSSTokenizer<wchar_t>::IsDigit(wchar_t c) { return SFiswdigit(c); }

template <> inline int  CSSTokenizer<char>::IsXDigit(char c) { return isxdigit(c); }
template <> inline int  CSSTokenizer<wchar_t>::IsXDigit(wchar_t c) { return SFiswxdigit(c); }

template <> inline int  CSSTokenizer<char>::IsAlpha(char c) { return isalpha(c); }
template <> inline int  CSSTokenizer<wchar_t>::IsAlpha(wchar_t c) { return SFiswalpha(c); }

template <> inline int  CSSTokenizer<char>::IsAlNum(char c) { return isalnum(c); }
template <> inline int  CSSTokenizer<wchar_t>::IsAlNum(wchar_t c) { return SFiswalnum(c); }


//
// Retrieve the next time from a CSS1 stream
//
template <typename T>
CSSToken<T>    CSSTokenizer<T>::GetNextToken()
{    
    const T*     pstr        = NULL;

    // (shortcut) bail if at end of file
    if (CurrentToken == CSSToken<T>::TT_EOF)
        return CSSToken<T>::TT_EOF;
    // set current token state
    pCurr += CurrentToken.Length;
    CurrentToken.pBase = pCurr;
    CurrentToken.Length = 1;
    // set current line/col state
    LineNo += LineDev;
    ColNo += ColDev;
    LineDev = 0;
    ColDev = 0;
    // init temp vars
    pstr = pCurr;

    // look at first char for context
    if (pstr == pEnd)
        // end of file reached
        CurrentToken = CSSToken<T>::TT_EOF;

    // whitespace
    else if ( IsSpace(*pstr) )
    {
        // looking for contiguous whitespace
        while (1)
        {
            if (*pstr == '\n') { LineDev++; ColDev=0; }
            pstr++; ColDev++;
            if ( pstr == pEnd || !IsSpace(*pstr) )
            {
                CurrentToken.Type = CSSToken<T>::TT_WhiteSpace;
                break;
            }
            CurrentToken.Length++;
        }
    }

    // comment
    else if (*pstr == '/')
    {
        CurrentToken.Type = CSSToken<T>::TT_WhiteSpace;
        pstr++; ColDev++;
        if (*pstr != '*')
        {
            // malformed comment; EOF
            pCurr = pEnd;
            CurrentToken.Type = CSSToken<T>::TT_EOF;
        }
        else
        {
            // consider comment as whitespace
            CurrentToken.Length++;
            CurrentToken.Type = CSSToken<T>::TT_WhiteSpace;
            bool closingComment = false;
            while (1)
            {
                if (*pstr == '\n') { LineDev++; ColDev=0; }
                pstr++; ColDev++;
                if ( pstr == pEnd )
                    break;
                else if (*pstr == '*')
                    closingComment = true;
                else if (closingComment && *pstr == '/')
                {
                    pstr++; ColDev++;
                    CurrentToken.Length++;
                    break;
                }
                else
                    closingComment = false;
                CurrentToken.Length++;
            }
        }
    }
    
    // name
    // PPS: (*pstr == '$') is a special case for font lib export names that begin with '$'
    else if (IsAlpha(*pstr) || (*pstr == '_') || (*pstr == '.') || (*pstr == '$'))
    {
        // looking for non-whitespace string
        while (1)
        {
            pstr++; ColDev++;
            if ( pstr == pEnd || !(IsAlNum(*pstr) || *pstr == '-' || *pstr == '_') )
            {
                CurrentToken.Type = CSSToken<T>::TT_Name;
                break;
            }
            CurrentToken.Length++;
        }
    }

    // quoted string
    else if (*pstr == '"')
    {
        while (1)
        {
            pstr++; ColDev++;
            if ( pstr == pEnd || *pstr == '"' )
            {
                CurrentToken.Type = CSSToken<T>::TT_QString;
                // eat the trailing quotes
                ColDev++;
                CurrentToken.Length++;
                break;
            }
            CurrentToken.Length++;
        }
    }

    // number
    else if (IsDigit(*pstr) || (*pstr == '-'))
    {
        // looking for digit or '.'
        while (1)
        {
            pstr++; ColDev++;
            if ( pstr == pEnd || !(IsDigit(*pstr) || *pstr == '.') )
            {
                CurrentToken.Type = CSSToken<T>::TT_Number;
                break;
            }
            CurrentToken.Length++;
        }
        if (*pstr == '.')
        {
            CurrentToken.Length++;
            pstr++; ColDev++;
            while (1)
            {
                pstr++; ColDev++;
                if ( pstr == pEnd || !IsDigit(*pstr) )
                    break;
                CurrentToken.Length++;
            }
        }
    }

    // hex number
    else if (*pstr == '#')
    {
        // looking for hex digits
        while (1)
        {
            pstr++; ColDev++;
            if ( pstr == pEnd || !IsXDigit(*pstr) )
            {
                CurrentToken.Type = CSSToken<T>::TT_HexNumber;
                break;
            }
            CurrentToken.Length++;
        }
    }

    // characters
    else
    {
        switch (*pstr)
        {
        case ',':
            CurrentToken.Type = CSSToken<T>::TT_Comma;
            break;
        case ';':
            CurrentToken.Type = CSSToken<T>::TT_SemiColon;
            break;
        case ':':
            CurrentToken.Type = CSSToken<T>::TT_Colon;
            break;
        case '{':
            CurrentToken.Type = CSSToken<T>::TT_OpenBrace;
            break;
        case '}':
            CurrentToken.Type = CSSToken<T>::TT_CloseBrace;
            break;
        default:
            CurrentToken.Type = CSSToken<T>::TT_Unknown;
        }
        ColDev++;
    }
    return CurrentToken;
}


//
// ********************************************************************
//


//
// CSS1 parse callback handler
//
template <typename T>
class CSSHandler
{
public:
    virtual ~CSSHandler() {}

    virtual void    OpenCSSSelectorBlock(const CSSToken<T>& name, void* pdata) = 0;
    virtual void    CloseCSSSelectorBlock(void* pdata) = 0;
    virtual void    PushCSSSelectorProperty(const CSSToken<T>& name, 
        const Array< CSSToken<T> >& value, 
        void* pdata) = 0;
};


//
// Parser for CSS1
//
template <typename T>
class CSSParser
{
private:
    enum ParseState
    {
        PS_ReadStyleName,
        PS_ReadPropertyName,
        PS_ReadPropertyValue,
        PS_Success,
        PS_Error,
    };

    CSSToken<T>             SelectorName;
    CSSToken<T>             PropertyName;
    Array< CSSToken<T> >   PropertyValue;
    
    bool                    bPseudoClass;
    bool                    bPropValSpool;
    UPInt                   PropValLinePos, PropValColPos;

public:
    CSSParser() : bPseudoClass(false), bPropValSpool(false) {}    

    bool Parse(const T* buffer, UPInt len, CSSHandler<T>* handler, void* pdata);
};


//
// Invokes callbacks for the appropriate CSS1 structures
//
template <typename T>
bool CSSParser<T>::Parse(const T* buffer, UPInt len, CSSHandler<T>* handler, void* pdata)
{
    bool                        done = false;
    CSSTokenizer<T>             tokens(buffer, len);
    CSSToken<T>                 token;
    ParseState                  state = PS_ReadStyleName;
    bool                        preserveToken = false;

#ifdef SF_CSS_VERBOSE_TRACE
    String                   tokenString;
    String                   gbuff;
    gbuff.AppendString(buffer, len);
    SF_DEBUG_MESSAGE(1, "---------------------------\n");
    SF_DEBUG_MESSAGE1(1, "Parsing:\n%s\n\n", gbuff.ToCStr());
#endif

    while ( !done )
    {
        // break on error
        if (state == PS_Error)
            break;
        // get token
        if (!preserveToken)
            token = tokens.GetNextToken();
        else
            preserveToken = false;
        // check for exit condition
        if (token == CSSToken<T>::TT_EOF)
            state = PS_Success;
        // perform state logic
        switch (state)
        {
        case PS_ReadStyleName:
            {
                switch (token.Type)
                {
                case CSSToken<T>::TT_WhiteSpace:   
                    if (bPseudoClass)
                    {
#ifdef SF_CSS_VERBOSE_TRACE
                        SF_DEBUG_MESSAGE(1, "- Error reading style name\n");
#endif
                        state = PS_Error;
                    }
                    break;

                case CSSToken<T>::TT_Name:
                    if (!bPseudoClass)
                    {
                        SelectorName = token;
                        // peek the next token
                        token = tokens.GetNextToken();
                        if (token == CSSToken<T>::TT_Colon)
                        {
                            // special case for pseudo-classes (eg: a:hover, etc)
                            SelectorName += token;
                            bPseudoClass = true;
                            break;
                        }
                        preserveToken = true;
                    }
                    else
                        SelectorName += token;
#ifdef SF_CSS_VERBOSE_TRACE
                    tokenString.Clear(); tokenString.AppendString(SelectorName.pBase, SelectorName.Length);
                    SF_DEBUG_MESSAGE3(1, "- Found selector: '%s' @ %d,%d\n", 
                        tokenString.ToCStr(), tokens.GetLineNo(), 
                        tokens.GetColNo());
#endif
                    handler->OpenCSSSelectorBlock(SelectorName, pdata);
                    // peek the next token
                    if (!preserveToken)
                        token = tokens.GetNextToken();
                    else
                        preserveToken = false;
                    if (token == CSSToken<T>::TT_WhiteSpace) token = tokens.GetNextToken();
                    switch (token.Type)
                    {
                    case CSSToken<T>::TT_Comma:    
                        state = PS_ReadStyleName;
                        bPseudoClass = false;
                        break;
                    case CSSToken<T>::TT_OpenBrace:
#ifdef SF_CSS_VERBOSE_TRACE
                        SF_DEBUG_MESSAGE(1, "- Beginning selector block\n");
#endif
                        state = PS_ReadPropertyName;                       
                        bPseudoClass = false;
                        break;
                    default:
#ifdef SF_CSS_VERBOSE_TRACE
                        SF_DEBUG_MESSAGE(1, "- Error reading style name\n");
#endif
                        state = PS_Error;
                    }
                    break;

                default:
#ifdef SF_CSS_VERBOSE_TRACE
                    SF_DEBUG_MESSAGE(1, "- Error reading style name\n");
#endif
                    state = PS_Error;
                }
                break;
            }

        case PS_ReadPropertyName:
            {
                switch (token.Type)
                {
                case CSSToken<T>::TT_WhiteSpace:   break;

                case CSSToken<T>::TT_CloseBrace:
#ifdef SF_CSS_VERBOSE_TRACE
                    SF_DEBUG_MESSAGE(1, "- Closing selector block\n");
#endif
                    handler->CloseCSSSelectorBlock(pdata);
                    state = PS_ReadStyleName;
                    break;

                case CSSToken<T>::TT_Name:
                    PropertyName = token;
#ifdef SF_CSS_VERBOSE_TRACE
                    tokenString.Clear(); tokenString.AppendString(token.pBase, token.Length);
                    SF_DEBUG_MESSAGE3(1, "- Found property name: '%s' @ %d,%d\n", 
                        tokenString.ToCStr(), tokens.GetLineNo(), tokens.GetColNo());
#endif
                    // peek the next token
                    token = tokens.GetNextToken();
                    // skip whitespace
                    if (token == CSSToken<T>::TT_WhiteSpace) token = tokens.GetNextToken();
                    if (token == CSSToken<T>::TT_Colon)
                        state = PS_ReadPropertyValue;
                    else
                    {
#ifdef SF_CSS_VERBOSE_TRACE
                        SF_DEBUG_MESSAGE(1, "- Error reading property name\n");
#endif
                        state = PS_Error;
                    }
                    break;

                default:
                    {
#ifdef SF_CSS_VERBOSE_TRACE
                        SF_DEBUG_MESSAGE(1, "- Error reading property name\n");
#endif
                        state = PS_Error;
                    }
                }
                break;
            }

        case PS_ReadPropertyValue:
            {
                switch (token.Type)
                {
                case CSSToken<T>::TT_SemiColon:
#ifdef SF_CSS_VERBOSE_TRACE
                    tokenString.Clear(); 
                    if (PropertyValue.GetSize() > 0)
                        tokenString.AppendString(PropertyValue[0].pBase, 
                            (PropertyValue[PropertyValue.GetSize()-1].pBase + 
                            PropertyValue[PropertyValue.GetSize()-1].Length)
                            - PropertyValue[0].pBase);
                    SF_DEBUG_MESSAGE3(1, "- Found property value: '%s' @ %d,%d\n", 
                        tokenString.ToCStr(), PropValLinePos, PropValColPos);
#endif
                    handler->PushCSSSelectorProperty(
                        PropertyName, 
                        PropertyValue,
                        pdata);

                    bPropValSpool = false;
                    state = PS_ReadPropertyName;
                    break;

                case CSSToken<T>::TT_CloseBrace:
#ifdef SF_CSS_VERBOSE_TRACE
                    tokenString.Clear(); 
                    if (PropertyValue.GetSize() > 0)
                        tokenString.AppendString(PropertyValue[0].pBase, 
                        (PropertyValue[PropertyValue.GetSize()-1].pBase + 
                        PropertyValue[PropertyValue.GetSize()-1].Length)
                        - PropertyValue[0].pBase);
                    SF_DEBUG_MESSAGE3(1, "- Found property value: '%s' @ %d,%d\n", 
                        tokenString.ToCStr(), PropValLinePos, PropValColPos);
                    SF_DEBUG_MESSAGE(1, "- Closing selector block");
#endif
                    handler->PushCSSSelectorProperty(
                        PropertyName,
                        PropertyValue,
                        pdata);                    
                    handler->CloseCSSSelectorBlock(pdata);
                    bPropValSpool = false;
                    state = PS_ReadStyleName;
                    break;

                default:
                    if (!bPropValSpool)
                    {
                        PropertyValue.Clear();
                        bPropValSpool = true;
                        // prune leading ws
                        if (token == CSSToken<T>::TT_WhiteSpace)
                            token = tokens.GetNextToken();
                        PropValLinePos = tokens.GetLineNo();
                        PropValColPos = tokens.GetColNo();
                    }
                    PropertyValue.PushBack(token);
                }
                break;                
            }
        default:
                done = true;
        }
    }
    if (state == PS_Success)
        return true;
    else
    {
#ifdef SF_CSS_VERBOSE_TRACE
        tokenString.Clear(); tokenString.AppendString(token.pBase, token.Length);
        SF_DEBUG_MESSAGE3(1, "   - Encountered '%s' @ %d,%d\n", tokenString.ToCStr(), 
            tokens.GetLineNo(), tokens.GetColNo());
#endif
        return false;
    }
}


//
// ********************************************************************
//

//
// CSS parser handler base
//
class TextStyleParserHandlerBase
{
protected:
    TextStyleParserHandlerBase(StyleManager* manager) : pManager(manager) 
    {
        SF_ASSERT(pManager);
    }

    Style* GetStyle(const StyleKey& key)
    {
        Style** ret = pManager->Styles.Get(key);
        return (ret) ? *ret : NULL;
    }

    void AddStyle(const StyleKey& key, Style* pstyle)
    {
        pManager->Styles.Add(key, pstyle);
    }


    const StyleKey&  GetTempStyleKey(StyleKey::KeyType t, const String& val)
    {
        return pManager->GetTempStyleKey(t, val);
    }

    StyleManager*        GetTextStyleManager() const { return pManager; }
    
private:
    StyleManager*        pManager;
};


//
// CSS parser handler implementation
//
template <typename T>
class TextStyleParserHandler : public CSSHandler<T>, public TextStyleParserHandlerBase
{
public:
    TextStyleParserHandler(StyleManager* manager) : TextStyleParserHandlerBase(manager) {}
    virtual ~TextStyleParserHandler() {}

    void OpenCSSSelectorBlock(const CSSToken<T>& name, void* pdata);
    void CloseCSSSelectorBlock(void* pdata);
    void PushCSSSelectorProperty(const CSSToken<T>& name, 
        const Array< CSSToken<T> >& value, 
        void* pdata);

private:
    void HandleColor            (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleDisplay          (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleFontFamily       (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleFontSize         (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleFontStyle        (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleFontWeight       (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleKerning          (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleLeading          (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleLetterSpacing    (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleMarginLeft       (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleMarginRight      (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleTextAlign        (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleTextDecoration   (const Array<Style*>& styles, const Array< CSSToken<T> >& value);
    void HandleTextIndent       (const Array<Style*>& styles, const Array< CSSToken<T> >& value);

    int     StrNCmpChar(const char* s1, const T* s2, UPInt size);
};

template <typename T> inline 
int     TextStyleParserHandler<T>::StrNCmpChar(const char* s1, const T* s2, UPInt size)
{
    return SGMLCharIter<T>::StrCompare(s2, s1, size);
}


//
// Open a selector block
//
template <typename T>
void TextStyleParserHandler<T>::OpenCSSSelectorBlock(const CSSToken<T>& name, void* pdata)
{
#ifdef SF_CSS_VERBOSE_TRACE
    String sname;
    sname.AppendString(name.pBase, name.Length);
    SF_DEBUG_MESSAGE1(1, "[CSS] Opening selector block: '%s'\n", sname.ToCStr());
#endif

    String keystr;
    // figure out key type
    StyleKey::KeyType type = StyleManager::CSS_Tag;
    if (name.Length > 0 && name.pBase[0] == '.')
    {
        type = StyleManager::CSS_Class;
        keystr.AppendString(name.pBase+1, name.Length-1);
    }
    else
        keystr.AppendString(name.pBase, name.Length);
    
    // does selector already exist?    
    const StyleKey& key = GetTempStyleKey(type, keystr);
    Style* pstyle = GetStyle(key);
    if (!pstyle)
    {
        MemoryHeap* pheap = Memory::GetHeapByAddress(GetTextStyleManager());
        pstyle = SF_HEAP_NEW(pheap) Style(pheap);
        AddStyle(key, pstyle);
    }
    else
    {
        // styles exists; clear all props
        pstyle->Reset();
    }
    // add to active selectors
    Array<Style*>* parr = (Array<Style*>*)pdata;
    SF_ASSERT(parr);
    parr->PushBack(pstyle);

}


//
// Close a selector block
//
template <typename T>
void TextStyleParserHandler<T>::CloseCSSSelectorBlock(void* pdata)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Closing selector block\n");
#endif

    // close active selectors
    Array<Style*>* parr = (Array<Style*>*)pdata;
    SF_ASSERT(parr);
    parr->Clear();
}


//
// Push a CSS1 property to the active selectors
//
template <typename T>
void TextStyleParserHandler<T>::PushCSSSelectorProperty(const CSSToken<T>& name, 
                                                        const Array< CSSToken<T> >& value, 
                                                        void* pdata)
{
    String sname;
    sname.AppendString(name.pBase, name.Length);
#ifdef SF_CSS_VERBOSE_TRACE
    String sval;
    if (value.GetSize() > 0)
        sval.AppendString(value[0].pBase, 
        (value[value.GetSize()-1].pBase + 
        value[value.GetSize()-1].Length)
        - value[0].pBase);
    SF_DEBUG_MESSAGE2(1, "[CSS] Found property: '%s' => '%s'\n", sname.ToCStr(), sval.ToCStr());
#endif

    Array<Style*>* parr = (Array<Style*>*)pdata;
    SF_ASSERT(parr);
    SF_ASSERT(parr->GetSize() > 0);

    // fail if no value tokens
    if (value.GetSize() < 1) return;

    if      (sname == "color")              HandleColor(*parr, value);
    else if (sname == "display")            HandleDisplay(*parr, value);
    else if (sname == "font-family")        HandleFontFamily(*parr, value);
    else if (sname == "font-size")          HandleFontSize(*parr, value);
    else if (sname == "font-style")         HandleFontStyle(*parr, value);
    else if (sname == "font-weight")        HandleFontWeight(*parr, value);
    else if (sname == "kerning")            HandleKerning(*parr, value);
    else if (sname == "leading")            HandleLeading(*parr, value);
    else if (sname == "letter-spacing")     HandleLetterSpacing(*parr, value);
    else if (sname == "margin-left")        HandleMarginLeft(*parr, value);
    else if (sname == "margin-right")       HandleMarginRight(*parr, value);
    else if (sname == "text-align")         HandleTextAlign(*parr, value);
    else if (sname == "text-decoration")    HandleTextDecoration(*parr, value);
    else if (sname == "text-indent")        HandleTextIndent(*parr, value);
#ifdef SF_CSS_VERBOSE_TRACE
    else
        SF_DEBUG_MESSAGE1(1, "[CSS] Unknown property name '%s'\n", sname.ToCStr());
#endif
}



//
// Handle a color property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleColor(const Array<Style*>& styles, 
                                            const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'color'\n");
#endif

    // Flash Doc: Only hexadecimal color values are supported. Named colors (such as 
    // blue) are not supported. Colors are written in the following format: #FF0000.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // fail if first value token is not a hex number
    if (value[idx].Type != CSSToken<T>::TT_HexNumber)
        return;
    // fail if hex number if not six hex chars
    if (value[idx].Length != 7)
        return;
    // convert hex number to RGB components
    UInt32 colval = 0;
    SGMLParser<T>::ParseHexInt(&colval, value[idx].pBase+1, value[idx].Length-1);
    // assign color value to all active selectors
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mTextFormat.SetColor32(colval);        
}


//
// Handle a display property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleDisplay(const Array<Style*>& styles, 
                                              const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'display'\n");
#endif
    SF_UNUSED(styles);

    // Flash Doc: Supported values are inline, block, and none.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    ParagraphFormat::DisplayType display = ParagraphFormat::Display_Inline;
    // check against supported values
    if ( !StrNCmpChar("inline", ch, (sz < 6) ? sz : 6) ) 
    {
        display = ParagraphFormat::Display_Inline;
    }
    else if ( !StrNCmpChar("block", ch, (sz < 5) ? sz : 5) )
    {
        display = ParagraphFormat::Display_Block;
    }
    else if ( !StrNCmpChar("none", ch, (sz < 4) ? sz : 4) )
    {
        display = ParagraphFormat::Display_None;
    }
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mParagraphFormat.SetDisplay(display);        
}


//
// Handle a font-family property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleFontFamily(const Array<Style*>& styles, 
                                                 const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'font-family'\n");
#endif
    SF_UNUSED2(styles, value);

    // Flash Doc: A comma-separated list of fonts to use, in descending order of 
    // desirability. Any font family name can be used. If you specify a generic font 
    // name, it is converted to an appropriate device font. The following font conversions 
    // are available: mono is converted to _typewriter, sans-serif is converted to _sans, 
    // and serif is converted to _serif.

    UPInt idx = 0;
    UPInt sz = value.GetSize();    
    StringBuffer fontstr;
    while ( idx < sz )
    {
        if (fontstr.GetSize() > 0)
            fontstr.AppendChar(',');
        // skip leading whitespace        
        if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
        {
            idx++;
            if (sz < (idx + 1)) return;
        }
        // peek name or string token
        if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
            return;
        // spool font name
        const T* tptr = value[idx].pBase;
        UPInt tsz = value[idx].Length;
        if (value[idx].Type == CSSToken<T>::TT_QString)
        {
            tptr++;
            tsz -= 2;
        }
        fontstr.AppendString(tptr, tsz);
        // peek comma or EOL
        while ( idx < sz && (value[idx++].Type != CSSToken<T>::TT_Comma) ) {}
    }
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mTextFormat.SetFontList(fontstr.ToCStr());
}


//
// Handle a font-size property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleFontSize(const Array<Style*>& styles, 
                                               const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'font-size'\n");
#endif

    // Flash Doc: Only the numeric part of the value is used. Units (px, pt) are not 
    // parsed; pixels and points are equivalent.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if font size is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mTextFormat.SetFontSize(num);
}


//
// Handle a font-style property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleFontStyle(const Array<Style*>& styles, 
                                                const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'font-style'\n");
#endif

    // Flash Doc: Recognized values are normal and italic.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    // check against supported values
    if ( !StrNCmpChar("normal", ch, (sz < 6) ? sz : 6) ) 
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetItalic(false);        
    }
    else if ( !StrNCmpChar("italic", ch, (sz < 6) ? sz : 6) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetItalic(true);        
    }
}


//
// Handle a font-weight property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleFontWeight(const Array<Style*>& styles, 
                                                 const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'font-weight'\n");
#endif

    // Flash Doc: Recognized values are normal and bold. 

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    // check against supported values
    if ( !StrNCmpChar("normal", ch, (sz < 6) ? sz : 6) ) 
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetBold(false);        
    }
    else if ( !StrNCmpChar("bold", ch, (sz < 4) ? sz : 4) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetBold(true);        
    }
}


//
// Handle a kerning property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleKerning(const Array<Style*>& styles, 
                                              const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'kerning'\n");
#endif

    // Flash Doc: Recognized values are true and false. Kerning is supported for 
    // embedded fonts only. Certain fonts, such as Courier New, do not support kerning. 
    // The kerning property is only supported in SWF files created in Windows, not in 
    // SWF files created on the Macintosh. However, these SWF files can be played in 
    // non-Windows versions of Flash Player and the kerning still applies.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    // check against supported values
    if ( !StrNCmpChar("true", ch, (sz < 4) ? sz : 4) ) 
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetKerning(true);
    }
    else if ( !StrNCmpChar("false", ch, (sz < 5) ? sz : 5) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetKerning(false);
    }
}


//
// Handle a letter-spacing property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleLeading(const Array<Style*>& styles, 
                                              const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'leading'\n");
#endif

    // Flash Doc: The amount of space that is uniformly distributed between lines. 
    // The value specifies the number of pixels that are added after each 
    // line. A negative value condenses the space between lines. Only the numeric 
    // part of the value is used. Units (px, pt) are not parsed; pixels and points are equivalent.

    // PPS: Negative values not supported because tokenizer does not handle negative numbers

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if leading is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mParagraphFormat.SetLeading((int)num);
}


//
// Handle a letter-spacing property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleLetterSpacing(const Array<Style*>& styles, 
                                                    const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'letter-spacing'\n");
#endif

    // Flash Doc: The amount of space that is uniformly distributed between characters. 
    // The value specifies the number of pixels that are added to the advance after each 
    // character. A negative value condenses the space between characters. Only the numeric 
    // part of the value is used. Units (px, pt) are not parsed; pixels and points are equivalent.

    // PPS: Negative values not supported because tokenizer does not handle negative numbers

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if font size is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mTextFormat.SetLetterSpacing(num);
}


//
// Handle a margin-left property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleMarginLeft(const Array<Style*>& styles, 
                                                 const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'margin-left'\n");
#endif

    // Flash Doc: Only the numeric part of the value is used. Units (px, pt) are not parsed; 
    // pixels and points are equivalent.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if font size is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mParagraphFormat.SetLeftMargin((unsigned)num);
}


//
// Handle a margin-right property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleMarginRight(const Array<Style*>& styles, 
                                                  const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'margin-right'\n");
#endif

    // Flash Doc: Only the numeric part of the value is used. Units (px, pt) are not parsed; 
    // pixels and points are equivalent.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if font size is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mParagraphFormat.SetRightMargin((unsigned)num);
}


//
// Handle a text-align property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleTextAlign(const Array<Style*>& styles, 
                                                const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'text-align'\n");
#endif

    // Flash Doc: Recognized values are left, center, right, and justify.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    // check against supported values
    if ( !StrNCmpChar("left", ch, (sz < 4) ? sz : 4) ) 
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mParagraphFormat.SetAlignment( ParagraphFormat::Align_Left );
    }
    else if ( !StrNCmpChar("center", ch, (sz < 6) ? sz : 6) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mParagraphFormat.SetAlignment( ParagraphFormat::Align_Center );
    }
    else if ( !StrNCmpChar("right", ch, (sz < 5) ? sz : 5) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mParagraphFormat.SetAlignment( ParagraphFormat::Align_Right );
    }
    else if ( !StrNCmpChar("justify", ch, (sz < 7) ? sz : 7) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mParagraphFormat.SetAlignment( ParagraphFormat::Align_Justify );
    }
}


//
// Handle a text-decoration property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleTextDecoration(const Array<Style*>& styles, 
                                                     const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'text-decoration'\n");
#endif

    // Flash Doc: Recognized values are none and underline.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract value for comparison
    if (value[idx].Type != CSSToken<T>::TT_Name && !(value[idx].Type == CSSToken<T>::TT_QString && value[idx].Length > 2))
        return;
    // setup comparison value
    const T* ch = value[idx].pBase;
    UPInt sz = value[idx].Length;
    if (value[idx].Type == CSSToken<T>::TT_QString)
    {
        ch++;
        sz -= 2;
    }
    // check against supported values
    if ( !StrNCmpChar("none", ch, (sz < 4) ? sz : 4) ) 
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetUnderline(false);        
    }
    else if ( !StrNCmpChar("underline", ch, (sz < 9) ? sz : 9) )
    {
        for (UPInt i=0; i < styles.GetSize(); i++)
            styles[i]->mTextFormat.SetUnderline(true);        
    }
}


//
// Handle a text-indent property value
//
template <typename T>
void TextStyleParserHandler<T>::HandleTextIndent(const Array<Style*>& styles, 
                                                 const Array< CSSToken<T> >& value)
{
#ifdef SF_CSS_VERBOSE_TRACE
    SF_DEBUG_MESSAGE(1, "[CSS] Handled property 'text-indent'\n");
#endif

    // Flash Doc: Only the numeric part of the value is used. Units (px, pt) are not 
    // parsed; pixels and points are equivalent.

    UPInt idx = 0;
    // skip leading whitespace
    if (value[idx].Type == CSSToken<T>::TT_WhiteSpace)
    {
        idx++;
        if (value.GetSize() < 2) return;
    }
    // extract numerical value
    if (value[idx].Type != CSSToken<T>::TT_Number)
        return;
    float num = 0;
    SGMLParser<T>::ParseFloat(&num, value[idx].pBase, value[idx].Length);
    // fail if font size is 0
    if (num == 0)
        return;
    // assign value to styles
    for (UPInt i=0; i < styles.GetSize(); i++)
        styles[i]->mParagraphFormat.SetIndent((int)num);
}


//
// ********************************************************************
//

//
// Equality operator for hash key
//
bool operator== (const StyleKey& key1, const StyleKey& key2)
{
    return (key1.Type == key2.Type) && (key1.Value == key2.Value);
}


//
// ********************************************************************
//


//
// Destructor
//
StyleManager::~StyleManager()
{
    ClearStyles();
}


//
// Parse CSS1 from a string buffer
//
template <typename Char>
bool StyleManager::ParseCSSImpl(const Char* buffer, UPInt len)
{
    CSSParser<Char> parser;
    // the following array is used to track application of styles to
    // multiple selectors
    Array<Style*> selectors;
    TextStyleParserHandler<Char> handler(this);
    return parser.Parse(buffer, len, &handler, &selectors);
}

bool StyleManager::ParseCSS(const char* buffer, UPInt len)
{
    // In order to reduce code size just decode the UTF-8 buffer into wchar_t and use wchar_t version
    // of ParseCSSImpl. However, for best performance the char* version should be used.
    //return ParseCSSImpl<char>(buffer, len);
    wchar_t* pwbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
    len = UTF8Util::DecodeStringSafe(pwbuf, len + 1, buffer, len);
    bool rv = ParseCSSImpl<wchar_t>(pwbuf, len);
    SF_FREE(pwbuf);
    return rv;
}

bool StyleManager::ParseCSS(const wchar_t* buffer, UPInt len)
{
    return ParseCSSImpl<wchar_t>(buffer, len);
}


// 
// Return style by name
//
const Style* StyleManager::GetStyle(StyleKey::KeyType type, const String& name) const
{
    Style* const* ret = Styles.Get(GetTempStyleKey(type, name));
    return (ret) ? *ret : NULL;
}

const Style* StyleManager::GetStyle(StyleKey::KeyType type, const char* name, UPInt len) const
{
    if (len == SF_MAX_UPINT)
        len = SFstrlen(name);
    String str(name, len);
    return GetStyle(type, str);
}
const Style* StyleManager::GetStyle(StyleKey::KeyType type, const wchar_t* name, UPInt len) const
{
    if (len == SF_MAX_UPINT)
        len = SFwcslen(name);
    String str;
    str.AppendString(name, len);
    return GetStyle(type, str);
}


// 
// Return the styles
//
const StyleHash& StyleManager::GetStyles() const
{
    return Styles;
}


//
// Add a style to the manager
//
void StyleManager::AddStyle(const StyleKey& key, Style* pstyle)
{
    Styles.Add(key, pstyle);
}

//
// Clear a specific style from the style manager
//
void StyleManager::ClearStyle(StyleKey::KeyType type, const String& name)
{
    const StyleKey & key = GetTempStyleKey(type, name);
    Style** style = Styles.Get(key);
    if (style)
    {
        delete *style;
        Styles.Remove(key);
    }
}

void StyleManager::ClearStyle(StyleKey::KeyType type, const char* name, UPInt len /* = SF_MAX_UPINT */)
{
    if (len == SF_MAX_UPINT)
        len = SFstrlen(name);
    String str(name, len);
    return ClearStyle(type, str);
}

//
// Clear all styles from the style manager
//
void StyleManager::ClearStyles()
{
    for (StyleHash::Iterator iter = Styles.Begin(); iter != Styles.End(); ++iter)
        delete iter->Second;
    Styles.Clear();    
}

#else

void GFxStyleSheet_DummyFunction() {}   // Exists to quelch compiler warning

#endif // SF_NO_CSS_SUPPORT

}}} // Scaleform::GFx::Text
