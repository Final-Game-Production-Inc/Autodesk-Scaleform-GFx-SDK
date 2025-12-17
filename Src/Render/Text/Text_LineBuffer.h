/**************************************************************************

PublicHeader:   Render
Filename    :   Text_LineBuffer.h
Content     :   Text line buffer
Created     :   May 31, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_TEXT_TEXTLINEBUFFER_H
#define INC_Render_TEXT_TEXTLINEBUFFER_H

#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Range.h"
#include "Render/Render_Font.h"
#include "Render/Render_GlyphParam.h"
#include "Render/Render_CxForm.h"
#include "Render/Render_TextLayout.h"
#include "Text_FontManager.h"
#include "Text_Core.h"
#include "Text_Highlight.h"
#include "Text_FilterDesc.h"

namespace Scaleform { 
namespace Render {
namespace Text {

using Render::TextLayout;
using Render::TextFieldParam;

#define GFX_TEXT_GUTTER         Render::Text::PixelsToFixp(2.f)

struct LineBufferGeometry
{
    typedef Matrix2F Matrix;

    LineBufferGeometry() : 
        FirstVisibleLinePos(0), 
        VisibleRect(0,0,0,0),
        HScrollOffset(0),
        Flags(0)
    {}

    unsigned    FirstVisibleLinePos;
    RectF       VisibleRect;
    unsigned    HScrollOffset;
    enum
    {
        Flag_InvalidCache       = 0x01,
        Flag_BatchHasUnderline  = 0x02,
        Flag_StaticText         = 0x04,
        Flag_Readability        = 0x08,
        Flag_CheckPreciseScale  = 0x10,
        Flag_NoClipping         = 0x20
    };
    UInt8    Flags;

    // helper methods
    void InvalidateCache()          { Flags |= Flag_InvalidCache; }
    void ValidateCache()            { Flags &= ~Flag_InvalidCache; }
    bool IsCacheInvalid() const     { return (Flags & Flag_InvalidCache) != 0; }

    void SetBatchHasUnderline()     { Flags |= Flag_BatchHasUnderline; }
    void ClearBatchHasUnderline()   { Flags &= ~Flag_BatchHasUnderline; }
    bool HasBatchUnderline() const  { return (Flags & Flag_BatchHasUnderline) != 0; }

    void SetStaticText()            { Flags |= Flag_StaticText; }
    bool IsStaticText() const       { return (Flags & Flag_StaticText) != 0; }

    void SetReadability()           { Flags |= Flag_Readability; }
    void ClearReadability()         { Flags &= ~Flag_Readability; }
    bool IsReadability() const      { return (Flags & Flag_Readability) != 0; }

    void SetCheckPreciseScale()     { Flags |= Flag_CheckPreciseScale; }
    void ClearCheckPreciseScale()   { Flags &= ~Flag_CheckPreciseScale; }
    bool NeedsCheckPreciseScale() const { return (Flags & Flag_CheckPreciseScale) != 0; }

    void SetNoClipping()            { Flags |= Flag_NoClipping; }
    void ClearNoClipping()          { Flags &= ~Flag_NoClipping; }
    bool IsNoClipping() const       { return (Flags & Flag_NoClipping) != 0; }
};


class LineBuffer
{
public:
    typedef Matrix2F                     Matrix;

    enum LineType
    {
        Line8,
        Line32
    };

    class GlyphEntry
    {
        // sizeof(GlyphEntry) == 8 bytes
    protected:
        UInt16  Index;
        UInt16  Advance;  // advance to the next glyph. Might be negative (in static text), see Flags_NegAdvance
        UInt16  LenAndFontSize; // len - how many chars this record represents (4 m.s.bits)

        enum
        {
            Mask_Length     = 0xF,
            Shift_Length    = 12,
            Mask_FontSize   = 0xFFF,
            Shift_FontSize  = 0,

            // 0x8000,
            Flags_NextFormat    = 0x4000,
            Flags_FmtHasFont    = 0x2000,
            Flags_FmtHasColor   = 0x1000,
            Flags_FmtHasImage   = 0x0800,

            Flags_Underline     = 0x0400,
            Flags_InvisibleChar = 0x0200,
            Flags_NewLineChar   = 0x0100,
            Flags_Url           = 0x0080,
            Flags_NegAdvance    = 0x0040, // used to indicate advance is negative
            // 0x0020,
            Flags_FractionalFontSize = 0x0010,
            Flags_WordWrapSeparator  = 0x0008,
            Flags_ComposStrGlyph = 0x0004,
            Flags_SpaceChar      = 0x0002
        };
        UInt16  Flags;

        void SetNegAdvance()      { Flags |= Flags_NegAdvance; }
        void ClearNegAdvance()    { Flags &= ~Flags_NegAdvance; }
        bool IsNegAdvance() const { return (Flags & Flags_NegAdvance) != 0; }
    public:
        GlyphEntry() { Init(); }

        void Init() { Index = 0; Advance = 0; LenAndFontSize = 0; Flags = 0; }
        void SetIndex(unsigned index)
        {
            SF_ASSERT(index < 65536 || index == ~0u);
            Index = (UInt16)index;
        }
        unsigned GetIndex() const { return (Index < 65535) ? Index : ~0u; }

        unsigned GetLength() const   { return (LenAndFontSize >> Shift_Length) & Mask_Length; }
        void SetLength(unsigned len) 
        { 
            SF_ASSERT(len <= Mask_Length); 
            LenAndFontSize = (UInt16)
                ((LenAndFontSize & (~(Mask_Length << Shift_Length))) | 
                 ((len & Mask_Length) << Shift_Length));
        }

        void SetFontSize(unsigned fs)
        {
            SF_ASSERT(fs <= Mask_FontSize);
            LenAndFontSize = (UInt16)
                ((LenAndFontSize & (~(Mask_FontSize << Shift_FontSize))) | 
                 ((fs & Mask_FontSize) << Shift_FontSize));
        }
        void SetFontSize(float fs) 
        {
            if (fs < 256.f)
            {
                unsigned fxpSize = (unsigned)(fs * 16.f);
                if (fxpSize & 0xF)
                {
                    Flags |= Flags_FractionalFontSize;
                    SetFontSize((unsigned)fxpSize);
                    return;
                }
            }
            Flags &= ~Flags_FractionalFontSize;
            SetFontSize((unsigned)fs); 
        } 
        void SetFontSizeInFixp(unsigned fs) 
        {
            if (fs < PixelsToFixp(256u))
            {
                unsigned fxpSize = (fs << 4) / 20;
                if (fxpSize & 0xF)
                {
                    Flags |= Flags_FractionalFontSize;
                    SetFontSize((unsigned)fxpSize);
                    return;
                }
            }
            Flags &= ~Flags_FractionalFontSize;
            SetFontSize((unsigned)fs); 
        }
        float GetFontSize() const 
        {
            unsigned fsz = (LenAndFontSize >> Shift_FontSize) & Mask_FontSize; 
            if (!(Flags & Flags_FractionalFontSize))
                return float(fsz); 
            return float(fsz)/16.f; // make fractional font size
        }

        void SetComposStrGlyph()       { Flags |= Flags_ComposStrGlyph; }
        void ClearComposStrGlyph()     { Flags &= ~Flags_ComposStrGlyph; }
        bool IsComposStrGlyph() const  { return (Flags & Flags_ComposStrGlyph) != 0; }

        void SetNextFormat()       { Flags |= Flags_NextFormat; }
        void ClearNextFormat()     { Flags &= ~Flags_NextFormat; }
        bool IsNextFormat() const  { return (Flags & Flags_NextFormat) != 0; }

        void SetFmtHasFont()       { Flags |= Flags_FmtHasFont; }
        void ClearFmtHasFont()     { Flags &= ~Flags_FmtHasFont; }
        bool HasFmtFont() const    { return (Flags & Flags_FmtHasFont) != 0; }

        void SetFmtHasColor()       { Flags |= Flags_FmtHasColor; }
        void ClearFmtHasColor()     { Flags &= ~Flags_FmtHasColor; }
        bool HasFmtColor() const    { return (Flags & Flags_FmtHasColor) != 0; }

        void SetFmtHasImage()       { Flags |= Flags_FmtHasImage; }
        void ClearFmtHasImage()     { Flags &= ~Flags_FmtHasImage; }
        bool HasFmtImage() const    { return (Flags & Flags_FmtHasImage) != 0; }

        void ClearFmtData() { Flags &= ~(Flags_NextFormat|Flags_FmtHasFont|Flags_FmtHasColor|Flags_FmtHasImage); }

        unsigned GetFormatDataElementsCount() const 
        { 
            return (HasFmtFont() ? 1 : 0) + (HasFmtColor() ? 1 : 0) + (HasFmtImage() ? 1 : 0); 
        }

        void SetUnderline()      { Flags |= Flags_Underline; }
        void ClearUnderline()    { Flags &= ~Flags_Underline; }
        bool IsUnderline() const { return (Flags & Flags_Underline) != 0; }

        void SetInvisibleChar()       { Flags |= Flags_InvisibleChar; }
        void ClearInvisibleChar()     { Flags &= ~Flags_InvisibleChar; }
        bool IsCharInvisible() const  { return (Flags & Flags_InvisibleChar) != 0; }

        void SetNewLineChar()       { Flags |= Flags_NewLineChar; }
        void ClearNewLineChar()     { Flags &= ~Flags_NewLineChar; }
        bool IsNewLineChar() const  { return (Flags & Flags_NewLineChar) != 0; }
        bool IsEOFChar() const      { return IsNewLineChar() && GetLength() == 0; }

        void SetUrl()      { Flags |= Flags_Url; }
        void ClearUrl()    { Flags &= ~Flags_Url; }
        bool IsUrl() const { return (Flags & Flags_Url) != 0; }

        void SetWordWrapSeparator()       { Flags |= Flags_WordWrapSeparator; }
        void ClearWordWrapSeparator()     { Flags &= ~Flags_WordWrapSeparator; }
        bool IsWordWrapSeparator() const  { return (Flags & Flags_WordWrapSeparator) != 0; }

        void SetSpaceChar()       { Flags |= Flags_SpaceChar; }
        void ClearSpaceChar()     { Flags &= ~Flags_SpaceChar; }
        bool IsSpaceChar() const  { return (Flags & Flags_SpaceChar) != 0; }

        // set advance value, in twips
        void SetAdvance(int v) 
        { 
            SF_ASSERT(v >= -65536 && v < 65536);
            if (v >= 0)
            {
                Advance = (UInt16)v;
                ClearNegAdvance();
            }
            else
            {
                Advance = (UInt16)Alg::Abs(v);
                SetNegAdvance();
            }
        }
        // returns advance, in twips
        int  GetAdvance() const { return (!IsNegAdvance()) ? Advance : -(int)(Advance); }

        void ClearFlags() { Flags = 0; }
    };

    union FormatDataEntry
    {
        FontHandle*             pFont;
        ImageDesc*              pImage;
        UInt32                  ColorV;
    };

    class GlyphIterator
    {
        GlyphEntry*             pGlyphs;
        GlyphEntry*             pEndGlyphs;
        FormatDataEntry*        pNextFormatData;
        HighlighterPosIterator  HighlighterIter;
        UInt32                  ColorV, OrigColor, UnderlineColor, SelectionColor;
        Ptr<FontHandle>         pFontHandle;
        Ptr<ImageDesc>          pImage;
        // a bit mask, see HighlightInfo::Flag_<>Underline:
        // Flag_SingleUnderline, Flag_DoubleUnderline, Flag_DottedUnderline
        HighlightInfo::UnderlineStyle UnderlineStyle; 
        unsigned                Delta;

        void UpdateDesc();
    public:
        GlyphIterator(void* pglyphs, unsigned glyphsCount, FormatDataEntry* pfmtData) : 
            ColorV(0), OrigColor(0), UnderlineColor(0), SelectionColor(0), UnderlineStyle(HighlightInfo::Underline_None),
            Delta(0)
        {
            pGlyphs         = reinterpret_cast<GlyphEntry*>(pglyphs);
            pEndGlyphs      = pGlyphs + glyphsCount;
            pNextFormatData = pfmtData;
            UpdateDesc();
        }
        GlyphIterator(void* pglyphs, unsigned glyphsCount, FormatDataEntry* pfmtData, 
                      const Highlighter& highlighter, UPInt lineStartPos) : 
            ColorV(0), OrigColor(0), UnderlineColor(0), SelectionColor(0), UnderlineStyle(HighlightInfo::Underline_None),
            Delta(0)
        {
            HighlighterIter = highlighter.GetPosIterator(lineStartPos);
            pGlyphs         = reinterpret_cast<GlyphEntry*>(pglyphs);
            pEndGlyphs      = pGlyphs + glyphsCount;
            pNextFormatData = pfmtData;
            UpdateDesc();
        }

        bool                IsFinished() const { return !pGlyphs || pGlyphs >= pEndGlyphs; }
        void                operator++();
        Font*               GetFont()  const { return (pFontHandle) ? pFontHandle->GetFont() : NULL; }
        FontHandle*         GetFontHandle()  const  { return pFontHandle; }
        bool                IsFauxBold() const      { return (pFontHandle) ? pFontHandle->IsFauxBold() : false; }
        bool                IsFauxItalic() const    { return (pFontHandle) ? pFontHandle->IsFauxItalic() : false; }
        bool                IsFauxBoldItalic() const{ return (pFontHandle) ? pFontHandle->IsFauxBoldItalic() : false; }
        unsigned            GetFauxFontStyle()      { return (pFontHandle) ? pFontHandle->GetFauxFontStyle() : 0;  }

        bool                IsAutoFitDisabled() const { return (pFontHandle) ? pFontHandle->IsAutoFitDisabled() : false; }

        Color               GetColor() const { return Color(ColorV); }
        GlyphEntry&         GetGlyph() const { return *pGlyphs; }
        ImageDesc*          GetImage() const { return pImage; }
        bool                HasImage() const { return !!pImage; }
        bool                IsUnderline() const { return UnderlineStyle != HighlightInfo::Underline_None; }
        bool                IsSingleUnderline() const 
            { return (UnderlineStyle == HighlightInfo::Underline_Single); }
        bool                IsThickUnderline() const 
            { return (UnderlineStyle == HighlightInfo::Underline_Thick); }
        bool                IsDottedUnderline() const 
            { return (UnderlineStyle == HighlightInfo::Underline_Dotted); }
        bool                IsDitheredSingleUnderline() const 
            { return (UnderlineStyle == HighlightInfo::Underline_DitheredSingle); }
        bool                IsDitheredThickUnderline() const 
            { return (UnderlineStyle == HighlightInfo::Underline_DitheredThick); }
        Color               GetUnderlineColor() const { return Color(UnderlineColor); }
        HighlightInfo::UnderlineStyle GetUnderlineStyle() const { return UnderlineStyle; }
        bool                IsSelected() const { return SelectionColor != 0; }
        Color               GetSelectionColor() const { return Color(SelectionColor); }
    };
    class GlyphInserter
    {
        GlyphEntry*         pGlyphs;
        FormatDataEntry*    pNextFormatData;
        unsigned            GlyphIndex;
        unsigned            GlyphsCount;
        unsigned            FormatDataIndex;
    public:
        GlyphInserter():pGlyphs(NULL), pNextFormatData(NULL), GlyphIndex(0),
            GlyphsCount(0), FormatDataIndex(0) {}
        GlyphInserter(GlyphEntry* pglyphs, unsigned glyphsCount, FormatDataEntry* pfmtData) :
            pGlyphs(reinterpret_cast<GlyphEntry*>(pglyphs)), pNextFormatData(pfmtData), 
            GlyphIndex(0), GlyphsCount(glyphsCount), FormatDataIndex(0) 
        {}
        GlyphInserter(const GlyphInserter& gi)
        {
            operator=(gi);
        }
        GlyphInserter& operator=(const GlyphInserter& gi)
        {
            pGlyphs         = gi.pGlyphs;
            pNextFormatData = gi.pNextFormatData;
            GlyphIndex      = gi.GlyphIndex;
            GlyphsCount     = gi.GlyphsCount;
            FormatDataIndex = gi.FormatDataIndex;
            return *this;
        }
        void Reset()
        {
            GlyphIndex = FormatDataIndex = 0;
        }
        bool IsFinished() const { return !pGlyphs || GlyphIndex >= GlyphsCount; }
        void operator++()
        {
            if (!IsFinished())
            {
                ++GlyphIndex;
            }
        }
        void operator++(int)
        {
            if (!IsFinished())
            {
                ++GlyphIndex;
            }
        }
        GlyphEntry& GetGlyph() 
        { 
            GlyphEntry& pg = pGlyphs[GlyphIndex];
            pg.Init();
            return pg; 
        }
        void AddFont(FontHandle* pfont)
        {
            pGlyphs[GlyphIndex].SetNextFormat();
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtFont());
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtColor()); // font should be set BEFORE color
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtImage()); // image cannot be set with font
            pGlyphs[GlyphIndex].SetFmtHasFont();
            pNextFormatData[FormatDataIndex++].pFont = pfont;
            pfont->AddRef();
        }
        void AddColor(Color color)
        {
            pGlyphs[GlyphIndex].SetNextFormat();
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtColor());
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtImage()); // image cannot be set with color
            pGlyphs[GlyphIndex].SetFmtHasColor();
            pNextFormatData[FormatDataIndex++].ColorV = color.ToColor32();
        }
        void AddImage(ImageDesc* pimage)
        {
            pGlyphs[GlyphIndex].SetNextFormat();
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtFont()); // image cannot be set with font
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtColor());// image cannot be set with color
            SF_ASSERT(!pGlyphs[GlyphIndex].HasFmtImage());
            pGlyphs[GlyphIndex].SetFmtHasImage();
            pNextFormatData[FormatDataIndex++].pImage = pimage;
            pimage->AddRef();
        }
        unsigned GetFormatDataElementsCount() const { return FormatDataIndex; }
        unsigned GetGlyphIndex() const { return GlyphIndex; }

        void ResetTo(const GlyphInserter& savedPos);

        GlyphEntry* GetReadyGlyphs(unsigned* pnumGlyphs)
        {
            *pnumGlyphs = GetGlyphIndex();
            return pGlyphs;
        }
    };
    struct LineData32 // 38 bytes
    {
        UInt32                  GlyphsCount;
        UInt32                  TextPos;            // index of the first char in the text
        SInt32                  OffsetX, OffsetY;   // offset of TopLeft corner, in twips
        UInt32                  Width, Height;      // dimensions, in twips; (W,H = 0) for static text;
        UInt32                  ParagraphId;
        UInt32                  TextLength; 
        UInt16                  ParagraphModId;
        UInt16                  BaseLineOffset;     // Y-offset from Bounds.y1 to baseline, in twips
        SInt16                  Leading;

        void Init()
        {
            GlyphsCount = 0;
            TextPos = 0;
            BaseLineOffset = 0;
            Leading = 0;
            OffsetX = OffsetY = 0;
            Width = Height = 0;
            TextLength = 0;
        }
        static unsigned GetLineStructSize() 
        { 
            struct _a { UInt32 b; UInt16 a; };
            return sizeof(LineData32) - (sizeof(_a) - (sizeof(UInt32) + sizeof(UInt16))); 
        }

        unsigned GetTextPos() const         { return TextPos; }
        void     SetTextPos(unsigned tp)    { TextPos = tp; }

        unsigned GetNumGlyphs() const       { return GlyphsCount; }
        void     SetNumGlyphs(unsigned tp)  { GlyphsCount = tp; }

        GlyphEntry* GetGlyphs() const { return reinterpret_cast<GlyphEntry*>(((UByte*)this) + GetLineStructSize()); }

        void SetTextLength(unsigned len) 
        { 
            TextLength = (UInt32)len;
        }
        unsigned GetTextLength() const { return TextLength; }

        void  SetBaseLineOffset(float baseLine) { BaseLineOffset = (UInt16)baseLine; }
        float GetBaseLineOffset() const         { return float(BaseLineOffset); }

        void  SetLeading(int leading) { SF_ASSERT(leading >= -32768 && leading < 32768); Leading = (SInt16)leading; }
        int   GetLeading() const       { return int(Leading); }
        int   GetNonNegLeading() const { return Leading > 0 ? int(Leading) : 0; }

        // set width & height, specified in twips
        void SetDimensions(int w, int h)
        {
            SF_ASSERT(w >= 0 && h >= 0 && w < INT_MAX && h < INT_MAX);
            Width = UInt32(w);
            Height = UInt32(h);
        }
        int  GetWidth() const  { return int(Width);  }
        int  GetHeight() const { return int(Height); }

        void    SetOffset(int x, int y) { OffsetX = x; OffsetY = y; }
        Point<int> GetOffset() const { return Point<int>(OffsetX, OffsetY); }
        int  GetOffsetX() const { return OffsetX; }
        int  GetOffsetY() const { return OffsetY; }
        void SetOffsetX(int x)  { OffsetX = x; }
        void SetOffsetY(int y)  { OffsetY = y; }

        unsigned GetParagraphId() const        { return ParagraphId; }
        void     SetParagraphId(unsigned id)   { ParagraphId = id; }
        UInt16   GetParagraphModId() const     { return ParagraphModId; }
        void     SetParagraphModId(UInt16 id)  { ParagraphModId = id; }
    };
    // 26 bytes data. Limitations:
    // Dimensions = [-32768..32767] (i.e. -1638.4 .. 1638.35 pts), 
    // TextLen <= 255, GlyphsCount <= 255, Leading <= 255 (i.e. 12.75 pt), TextPos < 2^24
    struct LineData8
    {
        UInt32  ParagraphId;
        UInt32  TextPosAndLength;   // index of the first char in the text / length (1 most signif. octet)
        SInt32  OffsetX, OffsetY;   // offset of TopLeft corner, in twips
        UInt16  Width, Height;      // dimensions, in twips; (W,H = 0) for static text;
        UInt16  ParagraphModId;
        UInt16  BaseLineOffset;     // Y-offset from Bounds.y1 to baseline, in twips
        UInt8   GlyphsCount;
        SInt8   Leading;
        enum
        {
            Mask_TextPos = 0xFFFFFFu,
            Shift_TextPos = 0,

            Mask_TextLength = 0xFF,
            Shift_TextLength = 24
        };

        void Init()
        {
            GlyphsCount = 0;
            TextPosAndLength = 0;
            BaseLineOffset = 0;
            Leading = 0;
            OffsetX = OffsetY = 0;
            Width = Height = 0;
        }
        static unsigned GetLineStructSize() 
        { 
            struct _a { UInt32 b; UInt16 a; };
            return sizeof(LineData8) - (sizeof(_a) - (sizeof(UInt32) + sizeof(UInt16))); 
        }

        unsigned GetTextPos() const 
        { 
            unsigned v = (TextPosAndLength >> Shift_TextPos) & Mask_TextPos;
            return (v == Mask_TextPos) ? ~0u : v; 
        }
        void SetTextPos(unsigned tp) 
        { 
            SF_ASSERT(tp <= Mask_TextPos || tp == ~0u);
            TextPosAndLength = 
                (TextPosAndLength & (~(Mask_TextPos << Shift_TextPos))) | 
                ((tp & Mask_TextPos) << Shift_TextPos);
        }

        unsigned GetNumGlyphs() const { return GlyphsCount; }
        void     SetNumGlyphs(unsigned tp) { SF_ASSERT(tp < 256); GlyphsCount = (UInt8)tp; }

        GlyphEntry* GetGlyphs() const { return reinterpret_cast<GlyphEntry*>(((UByte*)this) + GetLineStructSize()); }

        void SetTextLength(unsigned len) 
        { 
            SF_ASSERT(len < 256);
            TextPosAndLength = 
                (TextPosAndLength & (~(unsigned(Mask_TextLength) << Shift_TextLength))) | 
                ((len & Mask_TextLength) << Shift_TextLength);
        }
        unsigned GetTextLength() const { return (TextPosAndLength >> Shift_TextLength) & Mask_TextLength; }

        void  SetBaseLineOffset(float baseLine) { BaseLineOffset = (UInt16)baseLine; }
        float GetBaseLineOffset() const         { return float(BaseLineOffset); }

        void  SetLeading(int leading) { SF_ASSERT(leading >= -128 && leading < 128); Leading = (SInt8)leading; }
        int   GetLeading() const       { return int(Leading); }
        int   GetNonNegLeading() const { return Leading > 0 ? int(Leading) : 0; }

        // set width & height, specified in twips
        void SetDimensions(int w, int h)
        {
            SF_ASSERT(w >= 0 && h >= 0 && w < 65536 && h < 65536);
            Width = UInt16(w);
            Height = UInt16(h);
        }
        int  GetWidth() const  { return int(Width);  }
        int  GetHeight() const { return int(Height); }

        void SetOffset(int x, int y) 
        { 
            OffsetX = x; OffsetY = y; 
        }
        Point<int> GetOffset() const { return Point<int>(OffsetX, OffsetY); }
        int  GetOffsetX() const { return OffsetX; }
        int  GetOffsetY() const { return OffsetY; }
        void SetOffsetX(int x) { OffsetX = x; }
        void SetOffsetY(int y) { OffsetY = y; }

        unsigned GetParagraphId() const         { return ParagraphId; }
        void     SetParagraphId(unsigned id)    { ParagraphId = id; }
        UInt16   GetParagraphModId() const      { return ParagraphModId; }
        void     SetParagraphModId(UInt16 id)   { ParagraphModId = id; }
    };

    class Line
    {
        UInt32 MemSize;
        enum Flags
        {
            Flags_Data8         = 0x80000000u, // if set, Data8 is in use.
            Flags_Initialized   = 0x40000000u,
            Flags_RightToLeft   = 0x20000000u, // right-to-left line

            Mask_MemSize        = 0x07FFFFFFu,
            Shift_MemSize       = 0,

            Mask_Alignment      = 0x3,
            Shift_Alignment     = 27
        };
        union 
        {
            LineData32  Data32;
            LineData8   Data8;
        };

    public:
        enum Alignment
        {
            Align_Left   = 0,
            Align_Right  = 1,
            Align_Center = 2
        };
    public:
        Line():MemSize(0) {}

        void InitLine8()  { MemSize &= Mask_MemSize; MemSize |= Flags_Data8; MemSize |= Flags_Initialized;  Data8.Init(); }
        void InitLine32() { MemSize &= Mask_MemSize; MemSize |= Flags_Initialized; Data32.Init(); }
        void ClearInitialized()    { MemSize &= ~Flags_Initialized; }
        bool IsInitialized() const { return (MemSize & Flags_Initialized) != 0; }
        static unsigned GetLineStructSize(LineType lineType) 
        { 
            return (sizeof(UInt32) + ((lineType == Line8) ? LineData8::GetLineStructSize() : LineData32::GetLineStructSize())); 
        }
        void Release();

        inline bool IsData8() const  { return (MemSize & Flags_Data8) != 0; }
        inline bool IsData32() const { return !IsData8(); }

        void SetMemSize(unsigned len) 
        { 
            SF_ASSERT(len <= Mask_MemSize); 
            MemSize = 
                (MemSize & (~(Mask_MemSize << Shift_MemSize))) | 
                ((len & Mask_MemSize) << Shift_MemSize);
        }
        unsigned GetMemSize() const { return (MemSize >> Shift_MemSize) & Mask_MemSize; }

        void SetRightToLeft() { MemSize |= Flags_RightToLeft; }
        bool IsRightToLeft() const { return (MemSize & Flags_RightToLeft) != 0; }

        void SetAlignment(Alignment a)
        {
            MemSize = 
                (MemSize & (~(Mask_Alignment << Shift_Alignment))) | 
                ((a & Mask_Alignment) << Shift_Alignment);

        }
        Alignment GetAlignment() const { return (Alignment)((MemSize >> Shift_Alignment) & Mask_Alignment);}
        bool IsLeftAlignment() const   { return (GetAlignment() == Align_Left); }
        bool IsRightAlignment() const  { return (GetAlignment() == Align_Right); }
        bool IsCenterAlignment() const { return (GetAlignment() == Align_Center); }

        unsigned GetTextPos() const      { return (IsData8()) ? Data8.GetTextPos() : Data32.GetTextPos(); }
        void     SetTextPos(unsigned tp) { (IsData8()) ? Data8.SetTextPos(tp) : Data32.SetTextPos(tp); }

        unsigned GetNumGlyphs() const      { return (IsData8()) ? Data8.GetNumGlyphs() : Data32.GetNumGlyphs(); }
        void     SetNumGlyphs(unsigned tp) { (IsData8()) ? Data8.SetNumGlyphs(tp) : Data32.SetNumGlyphs(tp); }

        GlyphEntry* GetGlyphs() const { return (IsData8()) ? Data8.GetGlyphs() : Data32.GetGlyphs(); }
        FormatDataEntry* GetFormatData() const;

        void SetTextLength(unsigned len) { (IsData8()) ? Data8.SetTextLength(len) : Data32.SetTextLength(len); }
        unsigned GetTextLength() const   { return (IsData8()) ? Data8.GetTextLength() : Data32.GetTextLength(); }

        void  SetBaseLineOffset(float baseLine) { (IsData8()) ? Data8.SetBaseLineOffset(baseLine) : Data32.SetBaseLineOffset(baseLine); }
        float GetBaseLineOffset() const         { return (IsData8()) ? Data8.GetBaseLineOffset() : Data32.GetBaseLineOffset(); }

        float GetAscent() const  { return GetBaseLineOffset(); }
        float GetDescent() const { return GetHeight() - GetBaseLineOffset(); }

        void  SetLeading(int leading) { (IsData8()) ? Data8.SetLeading(leading) : Data32.SetLeading(leading); }
        int   GetLeading() const       { return (IsData8()) ? Data8.GetLeading() : Data32.GetLeading(); }
        int   GetNonNegLeading() const { return (IsData8()) ? Data8.GetNonNegLeading() : Data32.GetNonNegLeading(); }

        void SetDimensions(int w, int h)   { (IsData8()) ? Data8.SetDimensions(w, h) : Data32.SetDimensions(w, h); }
        int  GetWidth() const  { return (IsData8()) ? Data8.GetWidth() : Data32.GetWidth(); }
        int  GetHeight() const { return (IsData8()) ? Data8.GetHeight() : Data32.GetHeight(); }

        void SetOffset(int x, int y) { (IsData8()) ? Data8.SetOffset(x, y) : Data32.SetOffset(x, y); }
        Point<int> GetOffset() const { return (IsData8()) ? Data8.GetOffset() : Data32.GetOffset(); }
        int  GetOffsetX() const { return (IsData8()) ? Data8.GetOffsetX() : Data32.GetOffsetX(); }
        int  GetOffsetY() const { return (IsData8()) ? Data8.GetOffsetY() : Data32.GetOffsetY(); }
        void SetOffsetX(int x) { (IsData8()) ? Data8.SetOffsetX(x) : Data32.SetOffsetX(x); }
        void SetOffsetY(int y) { (IsData8()) ? Data8.SetOffsetY(y) : Data32.SetOffsetY(y); }

        unsigned GetParagraphId() const       { return (IsData8()) ? Data8.GetParagraphId() : Data32.GetParagraphId(); }
        UInt16   GetParagraphModId() const    { return (IsData8()) ? Data8.GetParagraphModId() : Data32.GetParagraphModId(); }
        void     SetParagraphId(unsigned id)  { (IsData8()) ? Data8.SetParagraphId(id) : Data32.SetParagraphId(id); }
        void     SetParagraphModId(UInt16 id) { (IsData8()) ? Data8.SetParagraphModId(id) : Data32.SetParagraphModId(id); }

        GlyphIterator Begin() const { return GlyphIterator(GetGlyphs(), GetNumGlyphs(), GetFormatData()); }
        GlyphIterator Begin(const Highlighter* phighlighter) const
        {
            if (phighlighter)
                return GlyphIterator(GetGlyphs(), GetNumGlyphs(), GetFormatData(), *phighlighter, GetTextPos()); 
            else
                return Begin();
        }

        bool HasNewLine() const;
    };

private:
    struct LineIndexComparator
    {
        static int Compare(const Line* p1, int index)
        {
            SF_ASSERT(p1);
            int si1 = p1->GetTextPos();
            if (index >= si1 && index < (int)(si1 + p1->GetTextLength()))
                return 0; 
            return si1 - index;
        }
        static int Less(const Line* p1, int index)
        {
            return Compare(p1, index) < 0;
        }
    };

    struct LineYOffsetComparator
    {
        static int Compare(const Line* p1, float yoffset)
        {
            SF_ASSERT(p1);
            float si1 = (float)p1->GetOffsetY();
            if (yoffset >= si1 && yoffset < si1 + p1->GetHeight() + p1->GetLeading())
                return 0; 
            return int(si1 - yoffset);
        }
        static int Less(const Line* p1, float yoffset)
        {
            return Compare(p1, yoffset) < 0;
        }
    };
public:
    static void ReleasePartOfLine(GlyphEntry* pglyphs, unsigned n, FormatDataEntry* pnextFormatData);

public:
    LineBuffer();
    ~LineBuffer();

    // Creates visible text layout
    void CreateVisibleTextLayout(TextLayout::Builder& bld, const Highlighter* phighlighter, const TextFieldParam& textFieldParam);

//     void    SetCursorPos(PointF position);
//     void    SetCursorColor(const Color&);
//     Color   GetCursorColor() const { return CursorColor; }

    // returns line, pos is the index of line
    Line& operator[](unsigned pos) 
    {
        return *Lines[pos];
    }
    const Line& operator[](unsigned pos) const
    {
        return *Lines[pos];
    }
    unsigned GetSize() const { return (unsigned)Lines.GetSize(); }

    void ResetCache(); 

    void ClearLines()
    {
        InvalidateCache();
        RemoveLines(0, (unsigned)Lines.GetSize());
    }

    friend class Iterator;
    friend class ConstIterator;

    class ConstIterator
    {
        friend class LineBuffer;

        const LineBuffer*   pLineBuffer;
        const Highlighter*  pHighlight;
        unsigned            CurrentPos;
        float               YOffset;    // negative, if vertically scrolled. Used for IsVisible.
        bool                StaticText; // for static text, all lines are visible

        ConstIterator(const LineBuffer& lb):
        pLineBuffer(&lb), CurrentPos(0), YOffset(0), StaticText(lb.IsStaticText()) {}
    public:
        ConstIterator():pLineBuffer(NULL), CurrentPos(0), YOffset(0), StaticText(0) {}
        ConstIterator(const ConstIterator& it):
        pLineBuffer(it.pLineBuffer), CurrentPos(it.CurrentPos), YOffset(it.YOffset), StaticText(it.StaticText) {}
        ConstIterator(const LineBuffer& lb, unsigned pos, float yoffset = 0):
        pLineBuffer(&lb), CurrentPos(pos), YOffset(yoffset), StaticText(lb.IsStaticText()) {}

        bool IsFinished() const 
        { 
            return !pLineBuffer || CurrentPos >= pLineBuffer->Lines.GetSize() || (int)CurrentPos < 0; 
        }
        bool IsVisible() const 
        { 
            if (!IsFinished())
            {
                if (StaticText) return true; // for static text we reckon all lines are visible.
                // otherwise, we need to sort all static text lines by Y
                return pLineBuffer->IsLineVisible(CurrentPos, YOffset);
            }
            return false;
        }

        ConstIterator& operator++() 
        { 
            SF_ASSERT(pLineBuffer);
            if (CurrentPos < pLineBuffer->Lines.GetSize())
                ++CurrentPos; 
            return *this; 
        }    
        ConstIterator operator++(int) 
        { 
            SF_ASSERT(pLineBuffer);
            ConstIterator it(*this); 
            if (CurrentPos < pLineBuffer->Lines.GetSize())
                ++CurrentPos; 
            return it; 
        }   
        ConstIterator& operator--() 
        { 
            SF_ASSERT(pLineBuffer);
            if ((int)CurrentPos >= 0)
                --CurrentPos; 
            return *this; 
        }    
        ConstIterator operator--(int) 
        { 
            SF_ASSERT(pLineBuffer);
            ConstIterator it(*this); 
            if ((int)CurrentPos >= 0)
                --CurrentPos; 
            return it; 
        }   
        ConstIterator operator+(unsigned delta) const
        {
            return ConstIterator(*pLineBuffer, CurrentPos + delta, YOffset);
        }
        ConstIterator operator-(unsigned delta) const
        {
            return ConstIterator(*pLineBuffer, CurrentPos - delta, YOffset);
        }
        const Line& operator*() const
        {
            SF_ASSERT(pLineBuffer);
            return (*pLineBuffer)[CurrentPos];
        }
        const Line* operator->() const
        {
            SF_ASSERT(pLineBuffer);
            return &(*pLineBuffer)[CurrentPos];
        }
        const Line* GetPtr() const
        {
            SF_ASSERT(pLineBuffer);
            return &(*pLineBuffer)[CurrentPos];
        }
        unsigned GetIndex() const { return ((int)CurrentPos >= 0) ? unsigned(CurrentPos) : ~0u; }

        void SetHighlighter(const Highlighter* ph) { pHighlight = ph; }
        const Highlighter* GetGighlighter() const  { return pHighlight; }
    };

    class Iterator
    {
        friend class LineBuffer;

        LineBuffer*         pLineBuffer;
        const Highlighter*  pHighlight;
        unsigned            CurrentPos;
        float               YOffset;    // negative, if vertically scrolled. Used for IsVisible.
        bool                StaticText; // for static text, all lines are visible

        Iterator(LineBuffer& lb):
            pLineBuffer(&lb), pHighlight(NULL), CurrentPos(0), YOffset(0), StaticText(lb.IsStaticText()) {}
        Iterator(LineBuffer& lb, unsigned pos, float yoffset = 0):
            pLineBuffer(&lb), pHighlight(NULL), CurrentPos(pos), YOffset(yoffset), StaticText(lb.IsStaticText()) {}
    public:
        Iterator():pLineBuffer(NULL), pHighlight(NULL), CurrentPos(0), YOffset(0), StaticText(false) {}
        Iterator(const Iterator& it):
            pLineBuffer(it.pLineBuffer), pHighlight(it.pHighlight), CurrentPos(it.CurrentPos), YOffset(it.YOffset), 
            StaticText(it.StaticText) {}

        bool IsFinished() const 
        { 
            return !pLineBuffer || CurrentPos >= pLineBuffer->Lines.GetSize() || (int)CurrentPos < 0; 
        }
        bool IsVisible() const 
        { 
            if (!IsFinished())
            {
                if (StaticText) return true; // for static text we reckon all lines are visible.
                                             // otherwise, we need to sort all static text lines by Y
                return pLineBuffer->IsLineVisible(CurrentPos, YOffset);
            }
            return false;
        }

        Iterator& operator++() 
        { 
            SF_ASSERT(pLineBuffer);
            if (CurrentPos < pLineBuffer->Lines.GetSize())
                ++CurrentPos; 
            return *this; 
        }    
        Iterator operator++(int) 
        { 
            SF_ASSERT(pLineBuffer);
            Iterator it(*this); 
            if (CurrentPos < pLineBuffer->Lines.GetSize())
                ++CurrentPos; 
            return it; 
        }   
        Iterator& operator--() 
        { 
            SF_ASSERT(pLineBuffer);
            if ((int)CurrentPos >= 0)
                --CurrentPos; 
            return *this; 
        }    
        Iterator operator--(int) 
        { 
            SF_ASSERT(pLineBuffer);
            Iterator it(*this); 
            if ((int)CurrentPos >= 0)
                --CurrentPos; 
            return it; 
        }   
        Line& operator*() const
        {
            SF_ASSERT(pLineBuffer);
            return (*pLineBuffer)[CurrentPos];
        }
        Line* operator->() const
        {
            SF_ASSERT(pLineBuffer);
            return &(*pLineBuffer)[CurrentPos];
        }
        Line* GetPtr() const
        {
            SF_ASSERT(pLineBuffer);
            return &(*pLineBuffer)[CurrentPos];
        }
        void Remove(unsigned num = 1)
        {
            if(!IsFinished())
            {
                pLineBuffer->RemoveLines(CurrentPos, num);
            }
        }
        Iterator operator+(unsigned delta) const
        {
            return Iterator(*pLineBuffer, CurrentPos + delta, YOffset);
        }
        Iterator operator-(unsigned delta) const
        {
            return Iterator(*pLineBuffer, CurrentPos - delta, YOffset);
        }
        Line* InsertNewLine(unsigned glyphCount, unsigned formatDataElementsCount, LineType lineType)
        {
            SF_ASSERT(pLineBuffer);
            unsigned pos = GetIndex();
            if (pos == ~0u)
                pos = (unsigned)pLineBuffer->Lines.GetSize();
            Line* pline = pLineBuffer->InsertNewLine(pos, glyphCount, formatDataElementsCount, lineType);
            ++CurrentPos;
            return pline;
        }
        unsigned GetIndex() const { return ((int)CurrentPos >= 0) ? unsigned(CurrentPos) : ~0u; }

        void     SetHighlighter(const Highlighter* ph) { pHighlight = ph; }
        const Highlighter* GetHighlighter() const  { return pHighlight; }

        LineBuffer::ConstIterator GetConstIterator() const;
    };

    Iterator Begin()                        { return Iterator(*this); }
    Iterator BeginVisible(float yoffset)    { return Iterator(*this, Geom.FirstVisibleLinePos, yoffset); }
    Iterator Last()                         { return Iterator(*this, (unsigned)Lines.GetSize() - 1); }
    Iterator FindLineByTextPos(UPInt textPos); 
    Iterator FindLineAtYOffset(float yoff);
    Iterator FindLineAtOffset(const Render::PointF& p);

    ConstIterator Begin() const             { return ConstIterator(*this); }
    ConstIterator BeginVisible(float yoffset) const { return ConstIterator(*this, Geom.FirstVisibleLinePos, yoffset); }
    ConstIterator Last() const              { return ConstIterator(*this, (unsigned)Lines.GetSize() - 1); }
    ConstIterator FindLineByTextPos(UPInt textPos) const
    { 
        Iterator iter = const_cast<LineBuffer*>(this)->FindLineByTextPos(textPos);
        return iter.GetConstIterator();
    }
    ConstIterator FindLineAtYOffset(float yoff) const
    { 
        return const_cast<LineBuffer*>(this)->FindLineAtYOffset(yoff).GetConstIterator();
    }

    void     SetVisibleRect(const RectF& bnd) { Geom.VisibleRect = bnd; }

    Line*           GetLine(unsigned lineIdx);
    const Line*     GetLine(unsigned lineIdx) const;
    Line*           InsertNewLine(unsigned lineIdx, unsigned glyphCount, unsigned formatDataElementsCount, LineType lineType);
    void            RemoveLines(unsigned lineIdx, unsigned num);
    static unsigned CalcLineSize(unsigned glyphCount, unsigned formatDataElementsCount, LineType lineType);

    // sets/gets the index of first visible line, 0-based
    void        SetFirstVisibleLine(unsigned line);
    unsigned    GetFirstVisibleLineIndex() const { return Geom.FirstVisibleLinePos; }

    // gets Y-offset of the first visible line, in twips, in line buffer's coord space (0-based)
    unsigned    GetVScrollOffsetInFixp() const;

    // gets/sets hscroll offset of the line buffer, in twips
    void        SetHScrollOffset(unsigned offset);
    unsigned    GetHScrollOffset() const { return Geom.HScrollOffset; }

    template <class Pnt, class Rect>
    Pnt BufferToView(const Rect& viewRect, const Pnt& p) const
    {
        Pnt dp(p);
        dp.x += Geom.VisibleRect.x1 - viewRect.x1;
        dp.y += Geom.VisibleRect.y1 - viewRect.y1;
        return dp;
    }
    template <class Pnt, class Rect>
    Pnt ViewToBuffer(const Rect& viewRect, const Pnt& p) const
    {
        Pnt dp(p);
        dp.x -= Geom.VisibleRect.x1 - viewRect.x1;
        dp.y -= Geom.VisibleRect.y1 - viewRect.y1;
        return dp;
    }

    void InvalidateCache()          { Geom.InvalidateCache(); }
    void ValidateCache()            { Geom.ValidateCache(); }
    bool IsCacheInvalid() const     { return Geom.IsCacheInvalid(); }

    void SetBatchHasUnderline()     { Geom.SetBatchHasUnderline(); }
    void ClearBatchHasUnderline()   { Geom.ClearBatchHasUnderline(); }
    bool HasBatchUnderline() const  { return Geom.HasBatchUnderline(); }

    void SetStaticText()            { Geom.SetStaticText(); }
    bool IsStaticText() const       { return Geom.IsStaticText(); }

    bool IsLineVisible(unsigned lineIndex) const
    {
        float yOffset = -float(GetVScrollOffsetInFixp());
        return IsLineVisible(lineIndex, yOffset);
    }
    bool IsLineVisible(unsigned lineIndex, float yOffset) const;
    bool IsPartiallyVisible(float yOffset) const;

    // scale the whole line buffer
    void Scale(float scaleFactor);
    // returns the minimal height of all lines in the buffer (in twips)
    int  GetMinLineHeight() const;

    FontHandle* FindFirstFontInfo() const;
#ifdef SF_BUILD_DEBUG
    void Dump() const;
    void CheckIntegrity() const;
#else
    void Dump() const {}
    void CheckIntegrity() const {}
#endif

public:
    ArrayLH<Line*>          Lines;

    LineBufferGeometry      Geom;
  
    class TextLineAllocator
    {
        //TODO: Array<Line*>  FreeLinesPool; ?
    public:
        Line* AllocLine(unsigned size, LineBuffer::LineType lineType);
        void FreeLine(Line* ptr);
    } LineAllocator;

    unsigned                LastHScrollOffset;
};

inline LineBuffer::ConstIterator LineBuffer::Iterator::GetConstIterator() const
{
    ConstIterator citer(*pLineBuffer, CurrentPos, YOffset);
    citer.SetHighlighter(pHighlight);
    return citer;
}

void LoadTextFieldParamFromTextFilter(TextFieldParam& params, const TextFilter& filter);

// recalculate the srcRect and the matrix to fit into 16-bit vertex format.
void RecalculateRectToFit16Bit(Matrix2F& matrix, const RectF& srcRect, RectF* pdestRect);

}}} // Scaleform::Render::Text

#endif //INC_TEXT_GFXTEXTLINEBUFFER_H
