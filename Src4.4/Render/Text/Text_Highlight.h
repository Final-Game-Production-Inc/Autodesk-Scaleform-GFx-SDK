/**************************************************************************

PublicHeader:   Render
Filename    :   Text_Highlight.h
Content     :   Highlighting functionality for rich text.
Created     :   August 6, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_Render_TEXT_TEXTHIGHLIGHT_H
#define INC_Render_TEXT_TEXTHIGHLIGHT_H

//#include "GFx/GFx_Types.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Range.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Color.h"

#include "Render/Render_Stats.h"

namespace Scaleform { 

class String;

namespace Render {

class DrawingContext;

namespace Text {

class Highlighter;
class LineBuffer;
class CompositionStringBase;
class LineBuffer;

#define GFX_TOPMOST_HIGHLIGHTING_INDEX      INT_MAX
#define GFX_WIDECURSOR_HIGHLIGHTING_INDEX   (GFX_TOPMOST_HIGHLIGHTING_INDEX - 1)
#define GFX_COMPOSSTR_HIGHLIGHTING_INDEX    (GFX_TOPMOST_HIGHLIGHTING_INDEX - 7)

// A struct that holds rendering parameters of the selection.
struct HighlightInfo : public NewOverrideBase<StatRender_Text_Mem>
{
    enum UnderlineStyle
    {
        Underline_None   = 0,
        Underline_Single = 1,
        Underline_Thick  = 2,
        Underline_Dotted = 3,
        Underline_DottedThick    = 4,
        Underline_DitheredSingle = 5,
        Underline_DitheredThick  = 6
    };
    enum
    {
        Flag_UnderlineStyle     = 0x7, // mask
        Flag_Background         = 0x8,
        Flag_TextColor          = 0x10,
        Flag_UnderlineColor     = 0x20
    };
    Color       BackgroundColor;
    Color       TextColor;
    Color       UnderlineColor;
    UByte       Flags;

    HighlightInfo()                  { Reset(); }
    HighlightInfo(UnderlineStyle st) { Reset(); SetUnderlineStyle(st); }

    void Reset() { BackgroundColor = TextColor = UnderlineColor = 0; Flags = 0; }
    void Append(const HighlightInfo& mergee);
    void Prepend(const HighlightInfo& mergee);
    bool IsEqualWithFlags(const HighlightInfo& right, unsigned flags);

    void SetSingleUnderline()     { SetUnderlineStyle(Underline_Single); }
    bool IsSingleUnderline() const{ return GetUnderlineStyle() == Underline_Single; }

    void SetThickUnderline()     { SetUnderlineStyle(Underline_Thick); }
    bool IsThickUnderline() const{ return GetUnderlineStyle() == Underline_Thick; }

    void SetDottedUnderline()     { SetUnderlineStyle(Underline_Dotted); }
    bool IsDottedUnderline() const{ return GetUnderlineStyle() == Underline_Dotted; }

    void SetDottedThickUnderline()     { SetUnderlineStyle(Underline_DottedThick); }
    bool IsDottedThickUnderline() const{ return GetUnderlineStyle() == Underline_DottedThick; }

    void SetDitheredSingleUnderline()     { SetUnderlineStyle(Underline_DitheredSingle); }
    bool IsDitheredSingleUnderline() const{ return GetUnderlineStyle() == Underline_DitheredSingle; }

    void SetDitheredThickUnderline()     { SetUnderlineStyle(Underline_DitheredThick); }
    bool IsDitheredThickUnderline() const{ return GetUnderlineStyle() == Underline_DitheredThick; }

    UnderlineStyle GetUnderlineStyle() const { return UnderlineStyle(Flags & Flag_UnderlineStyle); }
    bool  HasUnderlineStyle() const { return GetUnderlineStyle() != 0; }
    void  ClearUnderlineStyle() { Flags &= ~Flag_UnderlineStyle; }
    void  SetUnderlineStyle(UnderlineStyle us) { ClearUnderlineStyle(); Flags |= (us & Flag_UnderlineStyle); }

    void SetBackgroundColor(const Color& backgr) { Flags |= Flag_Background; BackgroundColor = backgr; }
    Color GetBackgroundColor() const             { return (HasBackgroundColor()) ? BackgroundColor : Color(0, 0, 0, 0); }
    void ClearBackgroundColor()                   { Flags &= (~Flag_Background); }
    bool HasBackgroundColor() const               { return (Flags & Flag_Background) != 0; }

    void SetTextColor(const Color& color) { Flags |= Flag_TextColor; TextColor = color; }
    Color GetTextColor() const             { return (HasTextColor()) ? TextColor : Color(0, 0, 0, 0); }
    void ClearTextColor()                   { Flags &= (~Flag_TextColor); }
    bool HasTextColor() const               { return (Flags & Flag_TextColor) != 0; }

    void SetUnderlineColor(const Color& color) { Flags |= Flag_UnderlineColor; UnderlineColor = color; }
    Color GetUnderlineColor() const            { return (HasUnderlineColor()) ? UnderlineColor : Color(0, 0, 0, 0); }
    void ClearUnderlineColor()                  { Flags &= (~Flag_UnderlineColor); }
    bool HasUnderlineColor() const              { return (Flags & Flag_UnderlineColor) != 0; }

};

// A struct that holds positioning parameters of the selection.
struct HighlightDesc
{
    UPInt                   StartPos; // text position
    UPInt                   Length;   // length in chars
    SPInt                   Offset;   // offset from StartPos, in number of glyphs

    UPInt                   AdjStartPos; // adjusted on composition str starting text pos
    UPInt                   GlyphNum;   // pre-calculated number of glyphs

    unsigned                Id;       // id of desc
    HighlightInfo           Info;

    HighlightDesc():
        StartPos(SF_MAX_UPINT),Length(0),Offset(-1),AdjStartPos(0),GlyphNum(0),Id(0) {}
    HighlightDesc(SPInt startPos, UPInt len):
        StartPos(startPos), Length(len), Offset(-1), AdjStartPos(0),GlyphNum(0), Id(0) {}

    bool ContainsPos(UPInt pos) const 
    { 
        return (Length > 0) ? (pos >= StartPos && pos < StartPos + Length) : false; 
    }
    bool ContainsIndex(UPInt index) const 
    { 
        return (GlyphNum > 0) ? (index >= AdjStartPos && index < AdjStartPos + GlyphNum) : false; 
    }
    bool IsEmpty() const { return Length == 0; }
};

// An iterator class that iterates highlights position by position.
class HighlighterPosIterator
{
    const Highlighter*  pManager;
    UPInt               CurAdjStartPos;
    UPInt               NumGlyphs;
    HighlightDesc       CurDesc;

    void InitCurDesc();
public:
    HighlighterPosIterator() { CurAdjStartPos = NumGlyphs = 0; }
    HighlighterPosIterator(const Highlighter* pmanager, UPInt startPos, UPInt len = SF_MAX_UPINT);

    const HighlightDesc& operator*() const { return CurDesc; }
    void operator++(int);
    void operator++() { operator++(0); }
    void operator+=(UPInt p);

    bool IsFinished() const;
};

// An iterator class that iterates highlights by ranges.
class HighlighterRangeIterator
{
    const Highlighter*   pManager;
    UPInt                       CurTextPos;
    UPInt                       CurRangeIndex;
    HighlightDesc        CurDesc;
    unsigned                    Flags;

    void InitCurDesc();
public:
    // flags - Flag_<> from GFxTextHighlighInfo
    HighlighterRangeIterator(const Highlighter* pmanager, unsigned flags);
    HighlighterRangeIterator(const Highlighter* pmanager, UPInt startPos, unsigned flags);

    HighlightDesc operator*();
    void operator++(int);
    void operator++() { operator++(0); }

    bool IsFinished() const;
};

// This class provides functionality to manage, calculate and render the
// highlighting.
class Highlighter : public NewOverrideBase<StatRender_Text_Mem>
{
    friend class HighlighterRangeIterator;
    friend class HighlighterPosIterator;
protected:
    ArrayLH<HighlightDesc>  Highlighters;
    unsigned                        LastId;
    UPInt                           CorrectionPos;
    UPInt                           CorrectionLen;
    bool                            Valid;
    mutable SInt8                   HasUnderline; // cached flag, 0 - not set, 1 - true, false otherwise

public:
    Highlighter();

    bool IsEmpty() const { return Highlighters.GetSize() == 0; }

    // highlighter will be created with id first available
    HighlightDesc* CreateNewHighlighter(HighlightDesc* pdesc);
    
    // highlighter will be created with id specified in desc
    HighlightDesc* CreateHighlighter(const HighlightDesc& desc);
    
    // an empty highlighter will be created with id first available
    HighlightDesc* CreateEmptyHighlighter(unsigned *pid)
    {
        HighlightDesc empty;
        HighlightDesc* pdesc = CreateNewHighlighter(&empty);
        *pid = empty.Id;
        return pdesc;
    };

    HighlightDesc GetHighlighter(unsigned id) const;
    HighlightDesc* GetHighlighterPtr(unsigned id);
    bool SetHighlighter(unsigned id, const HighlightInfo& info);
    bool FreeHighlighter(unsigned id);

    HighlighterPosIterator   GetPosIterator(UPInt startPos = 0, UPInt len = SF_MAX_UPINT) const;

    // flags - Flag_<> from GFxTextHighlighInfo
    HighlighterRangeIterator GetRangeIterator(UPInt startPos = 0, unsigned flags = SF_MAX_UINT) const;

    void UpdateGlyphIndices(const CompositionStringBase* pcs);

    // valid/invalid flag control
    void Invalidate() { Valid = false; HasUnderline = 0; }
    void Validate()   { Valid = true; }
    bool IsValid() const { return Valid; }

    bool HasUnderlineHighlight() const;


//     void DrawBackground(DrawingContext& backgroundDrawing,
//                         LineBuffer& lineBuffer,
//                         const RectF& textRect,
//                         UPInt firstVisibleChar,
//                         float hScrollOffset,
//                         float vScrollOffset);

    void Add(HighlightDesc& merge);
    void Remove(const HighlightDesc& cut);

    void SetSelectColor(const Color& color);
    bool IsAnyCharSelected(UPInt selectStart, UPInt selectEnd);
};

}}} // Scaleform::Render::Text

#endif // INC_TEXT_GFXTEXTHIGHLIGHT_H
