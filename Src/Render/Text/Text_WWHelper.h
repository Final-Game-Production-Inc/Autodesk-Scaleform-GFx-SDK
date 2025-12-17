/**************************************************************************

PublicHeader:   GFx
Filename    :   Text_WWHelper.h
Content     :   Default implementation of Asian (Japanese, Korean, Chinese) word-wrapping
Created     :   7/22/2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_Text_WWHELPER_H
#define INC_Render_Text_WWHELPER_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Std.h"

namespace Scaleform { namespace Render { namespace Text {

// Word-wrapping helper class.
class WordWrapHelper
{
public:
    enum BreakInfoFlags
    {
        CBI_NonStartingChar    = 1,
        CBI_NonTerminatingChar = 2
    };
    enum WordWrappingTypes
    {
        WWT_Default       = 0,   // OnWordWrapping will not be invoked
        WWT_Asian         = 0x1, // mostly Chinese
        WWT_Prohibition   = 0x2, // Japanese prohibition rule
        WWT_NoHangulWrap  = 0x4, // Korean-specific rule
        WWT_Last          = WWT_NoHangulWrap,
        WWT_All           = (WWT_Asian|WWT_Prohibition|WWT_NoHangulWrap)
    };
    struct CharBreakInfo 
    { 
        wchar_t Char;
        UByte   Flags; // see BreakInfoFlags
    }; 
    static CharBreakInfo CharBreakInfoArray[];

    static bool    SF_STDCALL FindCharWithFlags(unsigned wwMode, wchar_t c, unsigned charBreakFlags);
    static bool    SF_STDCALL IsAsianChar(unsigned wwMode, wchar_t c)
    {
        if (wwMode & WWT_NoHangulWrap)
        {
            if ((c >= 0x1100 && c <= 0x11FF) ||   // Hangul Jamo
                (c >= 0x3130 && c <= 0x318F) ||   // Hangul Compatibility Jamo
                (c >= 0xAC00 && c <= 0xD7A3))     // Hangul Syllables
                return false;
        }

        return
            ((c >= 0x1100 && c <= 0x11FF) ||       // Hangul Jamo
            (c >= 0x3000 && c <= 0xD7AF) ||       // CJK / Hangul Syllables
            (c >= 0xF900 && c <= 0xFAFF) ||       // CJK Compatibility Ideographs
            (c >= 0xFF00 && c <= 0xFFDC));        // Halfwidth / Fullwidth
    }
    static bool SF_STDCALL IsNonStartingChar(unsigned wwMode, wchar_t c) 
    { 
        return IsWhiteSpaceChar(c) || FindCharWithFlags(wwMode, c, CBI_NonStartingChar); 
    }
    static bool SF_STDCALL IsNonTerminatingChar(unsigned wwMode, wchar_t c)    
    { 
        return FindCharWithFlags(wwMode, c, CBI_NonTerminatingChar); 
    }
    static bool SF_STDCALL IsWhiteSpaceChar(wchar_t c) 
    { 
        return SFiswspace(c) != 0;
    }
    static bool SF_STDCALL IsLineFeedChar(wchar_t c)   
    { 
        return c == L'\n'; 
    }
    static bool SF_STDCALL IsLineBreakOpportunityAt(unsigned wwMode, const wchar_t* pwstr, UPInt index)
    {
        if (index == 0) // no breaks at position 0, no empty line
            return false;

        return IsLineBreakOpportunityAt(wwMode, pwstr[index-1], pwstr[index]);
    }
    static bool SF_STDCALL IsLineBreakOpportunityAt(unsigned wwMode, wchar_t prevChar, wchar_t curChar)
    {
        if (prevChar == 0) // no breaks at position 0, no empty line
            return false;

        return 
            ((IsWhiteSpaceChar(prevChar) || IsAsianChar(wwMode, curChar) ||
            IsAsianChar(wwMode, prevChar) || prevChar == L'-')) &&
            !IsNonStartingChar(wwMode, curChar) && !IsNonTerminatingChar(wwMode, prevChar);
    }
    static UPInt SF_STDCALL FindNextNonWhiteSpace(const wchar_t* pwstr, UPInt pos, UPInt maxPos)
    {
        while (IsWhiteSpaceChar(pwstr[pos]))
            ++pos;
        if (pwstr && IsLineFeedChar(pwstr[pos]))
            ++pos;

        return (pos <= maxPos) ? pos : SF_MAX_UPINT;
    }
    static UPInt SF_STDCALL FindPrevNonWhiteSpace(const wchar_t* pwstr, UPInt pos)
    {
        SPInt p = (SPInt)pos;
        while (p >= 0 && (IsWhiteSpaceChar(pwstr[p]) || IsLineFeedChar(pwstr[p])))
            --p;
        return (p < 0) ? SF_MAX_UPINT : (UPInt)p;
    }
    static bool SF_STDCALL IsVowel(wchar_t c)
    {
        c = (wchar_t)SFtowlower(c);
        if (c == 'a' || c == 'e' || c == 'o' || c == 'u' || c == 'i')
            return true;
        return false;
    }
    // Finds word wrap position according to wwMode bitmask. Returns new word wrap position
    // in the line (pos 0 corresponds to lineStartPos); returns SF_MAX_UPINT if can't find the pos.
    static UPInt SF_STDCALL FindWordWrapPos
        (unsigned wwMode, UPInt wordWrapPos, const wchar_t* pparaText, UPInt paraLen, UPInt lineStartPos, UPInt lineLen);
};
}}} // Scaleform::Render::Text

#endif // INC_Render_Text_WWHELPER_H

