/**************************************************************************

Filename    :   Render_Font.cpp
Content     :   Abstract base class for system and built-in fonts.
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_Font.h"
#include "Render_GlyphParam.h"
#include "Render_BoundBox.h"
#include "Render_GlyphCache.h"

namespace Scaleform { namespace Render {




//------------------------------------------------------------------------
UInt16 Font::calcTopBound(UInt16 code)
{
    RectF bounds;

    int idx = GetGlyphIndex(code);
    if (idx != -1)
    {
        GetGlyphBounds(idx,&bounds);
        return UInt16(-bounds.y1);
    }
    return 0;
}



//------------------------------------------------------------------------
void Font::calcLowerUpperTop(GlyphCache* log)
{
    SInt16 lowerCaseTop = 0;
    SInt16 upperCaseTop = 0;
    if (LowerCaseTop == 0 && UpperCaseTop == 0)
    {
        const UByte upperCaseCandidates[] = "HEFTUVWXZ";
        const UByte lowerCaseCandidates[] = "zxvwy";
        
        const UByte* p;
        for (p = upperCaseCandidates; *p; ++p)
        {
            upperCaseTop = calcTopBound(*p);
            if (upperCaseTop)
                break;
        }

        if (upperCaseTop)
        {
            for (p = lowerCaseCandidates; *p; ++p)
            {
                lowerCaseTop = calcTopBound(*p);
                if (lowerCaseTop)
                    break;
            }
        }
    }
    if (lowerCaseTop && upperCaseTop)
    {
        LowerCaseTop = lowerCaseTop;
        UpperCaseTop = upperCaseTop;
    }
    else
    {
        if (log)
            log->LogWarning("Font '%s%s%s': No hinting chars "
                            "(any of 'HEFTUVWXZ' and 'zxvwy'). Auto-Hinting disabled.", 
                            GetName(), 
                            IsBold() ? " Bold" : "", 
                            IsItalic() ? " Italic" : "");
        LowerCaseTop = -1;
        UpperCaseTop = -1;
    }
}


//------------------------------------------------------------------------
bool Font::IsCJK(UInt16 code) const
{
    static const UInt16 ranges[] = 
    {
        0x1100, 0x11FF, // Hangul Jamo                          U+1100 - U+11FF

        0x2E80, 0x2FDF, // CJK Radicals Supplement,             U+2E80 - U+2EFF
                        // KangXi Radicals                      U+2F00 - U+2FDF

        0x2FF0, 0x4DB5, // Ideographic Description Characters   U+2FF0 - U+2FFF
                        // CJK Symbols and Punctuation          U+3000 - U+303F
                        // Hiragana                             U+3040 - U+309F
                        // Katakana                             U+30A0 - U+30FF
                        // Bopomofo                             U+3100 - U+312F
                        // Hangul Compatibility Jamo            U+3130 - U+318F
                        // Kanbun                               U+3190 - U+319F
                        // Bopomofo Extended                    U+31A0 - U+31BF
                        // CJK Strokes                          U+31C0 - U+31EF
                        // Katakana Phonetic Extensions         U+31F0 - U+31FF
                        // Enclosed CJK Letters and Months      U+3200 - U+32FF
                        // CJK Compatibility                    U+3300 - U+33FF
                        // CJK Unified Ideographs Extension A   U+3400 - U+4DB5

        0x4DC0, 0x9FBB, // Yijing Hexagram Symbols              U+4DC0 - U+4DFF
                        // CJK Unified Ideographs               U+4E00 - U+9FBB


        0xAC00, 0xD7A3, // Hangul Syllables                     U+AC00 - U+D7A3

        0xF900, 0xFAFF, // CJK Compatibility Ideographs         U+F900 - U+FAFF

        0xFF62, 0xFFDC, // Halfwidth and Fullwidth Forms        U+FF00 - U+FFEF (Ideographic part)

        0, 0
    };
    for (unsigned i = 0; ranges[i]; i += 2)
    {
        if (code >= ranges[i] && code <= ranges[i + 1])
            return true;
    }
    return false;
}


}} // Scaleform::Render
