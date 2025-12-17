/**************************************************************************

Filename    :   Text_WWHelper.cpp
Content     :   Default implementation of Asian (Japanese, Korean, Chinese) word-wrapping
Created     :   7/22/2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Text_WWHelper.h"

namespace Scaleform { 
namespace Render {
namespace Text {

// This table is a compilation from Unicode 5.1.0 standard 
// (http://www.unicode.org/reports/tr14/tr14-22.html)
WordWrapHelper::CharBreakInfo WordWrapHelper::CharBreakInfoArray[] = 
{  
    //!AB: these caused ww behavior very different from Flash. To avoid confusion -
    // commenting out.
//     {0x0021, CBI_NonStartingChar},                          // !    EX (XB)
//     {0x0022, CBI_NonStartingChar | CBI_NonTerminatingChar}, // "    QU (XB/XA)
//     {0x0024, CBI_NonTerminatingChar},                       // $    
//     {0x0025, CBI_NonStartingChar},                          // %    PO (XB)
//     {0x0027, CBI_NonStartingChar | CBI_NonTerminatingChar}, // '    QU (XB/XA)
//     {0x0028, CBI_NonTerminatingChar},                       // (    
//     {0x0029, CBI_NonStartingChar},                          // )           
//     {0x002E, CBI_NonStartingChar | CBI_NonTerminatingChar}, // .    IS (XB)
//     {0x002F, CBI_NonStartingChar | CBI_NonTerminatingChar}, // /    SY (A)
//     {0x003A, CBI_NonStartingChar},                          // :    IS (XB)
//     {0x003B, CBI_NonStartingChar},                          // ;    IS (XB)
//     {0x003F, CBI_NonStartingChar},                          // ?    EX (XB)
//     {0x005B, CBI_NonTerminatingChar},                       // [
//     {0x005C, CBI_NonTerminatingChar},                       // Reverse Solidus, PR (XA)                           
//     {0x005D, CBI_NonStartingChar},                          // ]
//     {0x007B, CBI_NonTerminatingChar},                       // {
//     {0x007D, CBI_NonStartingChar},                          // }
//     {0x00A2, CBI_NonStartingChar},                          // Cent Sign,       PO (XB)
//     {0x00A3, CBI_NonTerminatingChar},                       // Pound Sign
//     {0x00A5, CBI_NonTerminatingChar},                       // Yen Sign
//     {0x00A7, CBI_NonTerminatingChar},                       // Section Sign
//     {0x00B0, CBI_NonStartingChar},                          // Degree Sign,     PO (XB)
//     {0x00B1, CBI_NonTerminatingChar},                       // Plus-Minus,      PR (XA)                           
//     {0x00B7, CBI_NonStartingChar},                          // Middle Dot
    {0x00A0, CBI_NonStartingChar},                          // NBSP
    {0x058A, CBI_NonStartingChar},                          // Armenian Hyphen, BA (A)
    {0x2010, CBI_NonStartingChar},                          // Hyphen           BA (A)
    {0x2012, CBI_NonStartingChar},                          // Figure Dash      BA (A)
    {0x2013, CBI_NonStartingChar},                          // En Dash          BA (A)
    {0x2014, CBI_NonStartingChar},                          // Em Dash          B2 (B/A/XP)
    {0x2018, CBI_NonTerminatingChar},                       // Left Single Quotation Mark
    {0x2019, CBI_NonStartingChar},                          // Right Single Quotation Mark               
    {0x201C, CBI_NonTerminatingChar},                       // Left Double Quotation Mark
    {0x201D, CBI_NonStartingChar},                          // Right Double Quotation Mark               
    {0x2024, CBI_NonStartingChar},                          // One Dot Leader   IN (XP)
    {0x2025, CBI_NonStartingChar},                          // Two Dot Leader   IN (XP)
    {0x2026, CBI_NonStartingChar},                          // Horizontal Ellipsis, IN (XP)
    {0x2027, CBI_NonStartingChar},                          // Hyphenation Point,   BA (A)
    {0x2032, CBI_NonStartingChar},                          // Prime                PO (XB)
    {0x2033, CBI_NonStartingChar},                          // Double Prime         PO (XB)
    {0x2035, CBI_NonTerminatingChar},                       // Reversed Prime       PO (XB)
    {0x20A7, CBI_NonTerminatingChar},                       // Peseta Sign          PO (XB)
    {0x2103, CBI_NonStartingChar},                          // Degree Celsius Sign, PO (XB)
    {0x2109, CBI_NonStartingChar},                          // Degree Fahrenheit,   PO (XB)
    {0x2116, CBI_NonTerminatingChar},                       // Numero Sign,         PR (XA)                           
    {0x2212, CBI_NonTerminatingChar},                       // Minus Sign,          PR (XA)                           
    {0x2213, CBI_NonTerminatingChar},                       // Minus-or-plus Sign,  PR (XA)                           
    {0x2122, CBI_NonStartingChar},                          // Trade mark sign
    {0x2574, CBI_NonStartingChar},                          // Box Drawings Light Left
    {0x266F, CBI_NonTerminatingChar},                       // Music Sharp Sign (#)
    {0x3001, CBI_NonStartingChar},                          // Ideographic comma    CL (XB)
    {0x3002, CBI_NonStartingChar},                          // Full stop            CL (XB)
    {0x3005, CBI_NonStartingChar},                          // Ideographic Iteration Mark   NS (XB)
    {0x3008, CBI_NonTerminatingChar},                       // Left Angle Bracket
    {0x3009, CBI_NonStartingChar},                          // Right Angle Bracket
    {0x300A, CBI_NonTerminatingChar},                       // Left Double Angle Bracket
    {0x300B, CBI_NonStartingChar},                          // Right Double Angle Bracket 
    {0x300C, CBI_NonTerminatingChar},                       // Left Corner Bracket  
    {0x300D, CBI_NonStartingChar},                          // Right Corner Bracket 
    {0x300E, CBI_NonTerminatingChar},                       // Left White Corner Bracket
    {0x300F, CBI_NonStartingChar},                          // Right White Corner Bracket
    {0x3010, CBI_NonTerminatingChar},                       // Left Black Lenticular Bracket 
    {0x3011, CBI_NonStartingChar},                          // Right black Lenticular Bracket
    {0x3012, CBI_NonTerminatingChar},                       // Postal Mark
    {0x3014, CBI_NonTerminatingChar},                       // Left Tortoise Shell Bracket
    {0x3015, CBI_NonStartingChar},                          // Right Tortoise Shell Bracket
    {0x301C, CBI_NonStartingChar},                          // Wave Dash                    NS (XB)
    {0x301D, CBI_NonTerminatingChar},                       // Reversed Double Prime Quotation Mark     
    {0x301E, CBI_NonStartingChar},                          // Double Prime Quotation Mark     
    {0x301F, CBI_NonStartingChar},                          // Low Double Prime Quotation Mark
    // Japanese, Hiragana
    {0x3041, CBI_NonStartingChar},                          // Hiragana Letter Small A
    {0x3043, CBI_NonStartingChar},                          // Hiragana Letter Small I 
    {0x3045, CBI_NonStartingChar},                          // Hiragana Letter Small U
    {0x3047, CBI_NonStartingChar},                          // Hiragana Letter Small E
    {0x3049, CBI_NonStartingChar},                          // Hiragana Letter Small O 
    {0x3063, CBI_NonStartingChar},                          // Hiragana Letter Small Tu
    {0x3083, CBI_NonStartingChar},                          // Hiragana Letter Small Ya
    {0x3085, CBI_NonStartingChar},                          // Hiragana Letter Small Yu 
    {0x3087, CBI_NonStartingChar},                          // Hiragana Letter Small Yo   
    {0x308E, CBI_NonStartingChar},                          // Hiragana Letter Small Wa 
    // Japanese, Katakana/Hiragana
    {0x309B, CBI_NonStartingChar},                          // Katakana-Hiragana Voiced Sound Mark      NS (XB)
    {0x309C, CBI_NonStartingChar},                          // Katakana-Hiragana Semi-Voiced Sound Mark NS (XB)
    {0x309D, CBI_NonStartingChar},                          // Hiragana Iteration Mark                  NS (XB) 
    {0x309E, CBI_NonStartingChar},                          // Hiragana Voiced Iteration Mark           NS (XB) 
    // Japanese, Katakana
    {0x30A1, CBI_NonStartingChar},                          // Katakana Letter Small A                   
    {0x30A3, CBI_NonStartingChar},                          // Katakana Letter Small I
    {0x30A5, CBI_NonStartingChar},                          // Katakana Letter Small U
    {0x30A7, CBI_NonStartingChar},                          // Katakana Letter Small E 
    {0x30A9, CBI_NonStartingChar},                          // Katakana Letter Small O 
    {0x30C3, CBI_NonStartingChar},                          // Katakana Letter Small Tu
    {0x30E3, CBI_NonStartingChar},                          // Katakana Letter Small Ya                   
    {0x30E5, CBI_NonStartingChar},                          // Katakana Letter Small Yu                   
    {0x30E7, CBI_NonStartingChar},                          // Katakana Letter Small Yo                   
    {0x30EE, CBI_NonStartingChar},                          // Katakana Letter Small Wa                   
    {0x30F5, CBI_NonStartingChar},                          // Katakana Letter Small Ka                   
    {0x30F6, CBI_NonStartingChar},                          // Katakana Letter Small Ke                   
    {0x30FB, CBI_NonStartingChar},                          // katakana middle dot                        
    {0x30FC, CBI_NonStartingChar},                          // Katakana-Hiragana Prolonged Sound Mark     
    {0x30FD, CBI_NonStartingChar},                          // Katakana Iteration Mark                    
    {0x30FE, CBI_NonStartingChar},                          // Katakana Voiced Iteration Mark             
    {0xFDFC, CBI_NonStartingChar},                          // Rial Sign        PO (XB)
    {0xFE50, CBI_NonStartingChar},                          // Small Comma      CL (XB)
    {0xFE51, CBI_NonStartingChar},                          // Small Ideographic Comma
    {0xFE52, CBI_NonStartingChar},                          // Small Full Stop  CL (XB)
    {0xFE54, CBI_NonStartingChar},                          // Small Semicolon  NS (XB)                   
    {0xFE55, CBI_NonStartingChar},                          // Small Colon      NS (XB)                  
    {0xFE56, CBI_NonStartingChar},                          // Small Question Mark      EX (XB)                
    {0xFE57, CBI_NonStartingChar},                          // Small Exclamation Mark   EX (XB)          
    {0xFE59, CBI_NonTerminatingChar},                       // Small Left Parenthesis             
    {0xFE5A, CBI_NonStartingChar},                          // Small Right Parenthesis            
    {0xFE5B, CBI_NonTerminatingChar},                       // Small Left Curly Bracket           
    {0xFE5C, CBI_NonStartingChar},                          // Small Right Curly Bracket          
    {0xFE5D, CBI_NonTerminatingChar},                       // Small Left Tortoise Shell Bracket  
    {0xFE5E, CBI_NonStartingChar},                          // Small Right Tortoise Shell Bracket 
    {0xFE6A, CBI_NonStartingChar},                          // Small Percent Sign, PO (XB)
    {0xFF01, CBI_NonStartingChar},                          // Fullwidth Exclamation Mark       EX (XB)             
    {0xFF04, CBI_NonTerminatingChar},                       // Fullwidth Dollar Sign            PO (XB)      
    {0xFF05, CBI_NonStartingChar},                          // Fullwidth Percent Sign                 
    {0xFF08, CBI_NonTerminatingChar},                       // Fullwidth Left Parenthesis              
    {0xFF09, CBI_NonStartingChar},                          // Fullwidth Right Parenthesis             
    {0xFF0C, CBI_NonStartingChar},                          // Fullwidth Comma          CL (XB)
    {0xFF0E, CBI_NonStartingChar},                          // Fullwidth Full Stop      CL (XB)
    {0xFF1A, CBI_NonStartingChar},                          // Fullwidth Colon          NS (XB)       
    {0xFF1B, CBI_NonStartingChar},                          // Fullwidth Semicolon      NS (XB)       
    {0xFF1F, CBI_NonStartingChar},                          // Fullwidth Question Mark  EX (XB)
    {0xFF20, CBI_NonTerminatingChar},                       // Fullwidth Commercial At        
    {0xFF3B, CBI_NonTerminatingChar},                       // Fullwidth Left Square Bracket  
    {0xFF3D, CBI_NonStartingChar},                          // Fullwidth Right Square Bracket 
    {0xFF5B, CBI_NonTerminatingChar},                       // Fullwidth Left Curly Bracket   
    {0xFF5D, CBI_NonStartingChar},                          // Fullwidth Right Curly Bracket  
    {0xFF61, CBI_NonStartingChar},                          // Halfwidth Comma          CL (XB)
    {0xFF64, CBI_NonStartingChar},                          // Halfwidth Full Stop      CL (XB)
    {0xFFE0, CBI_NonStartingChar | CBI_NonTerminatingChar}, // Fullwidth Cent Sign      PO (XB)
    {0xFFE1, CBI_NonTerminatingChar},                       // Fullwidth Pound Sign 
    {0xFFE5, CBI_NonTerminatingChar},                       // Fullwidth Yen Sign  
    {0xFFE6, CBI_NonTerminatingChar}                        // Fullwidth Won Sign  
};   

bool WordWrapHelper::FindCharWithFlags(unsigned wwMode, wchar_t c, unsigned charBreakFlags)
{
    if (!(wwMode & WWT_Prohibition)) 
        return false;

    SPInt left   = 0;
    SPInt right  = sizeof(CharBreakInfoArray)/sizeof(CharBreakInfoArray[0]);
    SPInt middle = 0;

    while (left <= right)
    {
        middle = ((right-left)/2) + left;
        if (c == CharBreakInfoArray[middle].Char)
            return (CharBreakInfoArray[middle].Flags & charBreakFlags) != 0;

        if (c < CharBreakInfoArray[middle].Char)
            right = middle - 1;
        else
            left  = middle + 1;
    }
    return false;
}

UPInt WordWrapHelper::FindWordWrapPos
    (unsigned wwMode, UPInt wordWrapPos, const wchar_t* pparaText, UPInt paraLen, UPInt lineStartPos, UPInt lineLen)
{
    SF_ASSERT(wordWrapPos);
    if ((wwMode & (WWT_Asian | WWT_NoHangulWrap | WWT_Prohibition)) && lineLen > 0)
    {
        const wchar_t* pstr = pparaText + lineStartPos;
        UPInt remainingLen  = paraLen - lineStartPos;
        UPInt pos = lineLen - 1;
        for (; pos > 0; --pos)
        {
            if (WordWrapHelper::IsWhiteSpaceChar(pstr[pos]))
            {
                UPInt eolPos = WordWrapHelper::FindPrevNonWhiteSpace(pstr, pos);
                if (eolPos == SF_MAX_UPINT)
                {
                    pos = WordWrapHelper::FindNextNonWhiteSpace (pstr, pos + 1, remainingLen - 1);
                    break;
                }
                pos = eolPos + 1;
            }
            if (WordWrapHelper::IsLineBreakOpportunityAt(wwMode, pstr, pos))
                break;
        }            
        if (pos == 0)
        {
            // couldn't find any character to break the line
            wordWrapPos = lineLen - 1;
        }
        else
        {
            wordWrapPos = WordWrapHelper::FindNextNonWhiteSpace (pstr, pos, remainingLen - 1);
            if (wordWrapPos == SF_MAX_UPINT)
                wordWrapPos = lineLen - 1; //?
        }
        return wordWrapPos;
    }
    return SF_MAX_UPINT;
}

}}} // Scaleform::Render::Text
