/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_TextFieldDef.h
Content     :   Text characters definitions implementation
Created     :   Nov, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_TEXTDEF_H
#define INC_SF_GFX_TEXTDEF_H

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_FontResource.h"

namespace Scaleform {
namespace GFx {

class DisplayObjectBase;

// A definition for a text display GFxCharacter, whose text can
// be changed at Runtime (by script or host).
struct TextFieldDef : public CharacterDef
{    
    ResourcePtr<FontResource> pFont;
    ResourceId              FontId;
    StringLH                FontClass;

    RectF                   TextRect;
    float                   TextHeight;

    Color                   ColorV;
    int                     MaxLength;

    float                   LeftMargin; // extra space between box border and text
    float                   RightMargin;
    float                   Indent;     // how much to indent the first line of multiline text
    float                   Leading;    // extra space between Lines (in addition to default font line spacing)
    StringLH                DefaultText;
    StringLH                VariableName;

    enum
    {
        Flags_WordWrap      = 0x1,
        Flags_Multiline     = 0x2,
        Flags_Password      = 0x4,  // show asterisks instead of actual characters
        Flags_ReadOnly      = 0x8,
        Flags_AutoSize      = 0x10, // resize our bound to fit the text
        Flags_Selectable    = 0x20,
        Flags_Border        = 0x40, // forces background and border to be drawn
        Flags_Html          = 0x80,
        Flags_UseDeviceFont = 0x100, // when false, use specified SWF internal font.Otherwise, device font will be used
        Flags_HasLayout     = 0x200, // Alignment, LeftMargin, RightMargin, Indent, Leading are explicitly loaded
        Flags_AAReadability = 0x400, // Anti-aliased for readability
        Flags_EmptyTextDef  = 0x800, // indicates def for createTextField
        Flags_WasStatic     = 0x1000
    };
    UInt16                  Flags;
    enum alignment
    {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT,
        ALIGN_CENTER,
        ALIGN_JUSTIFY,
    };
    alignment   Alignment;

    TextFieldDef();
    ~TextFieldDef();

    virtual CharacterDefType GetType() const { return TextField; }

    void SetWordWrap(bool v = true)     { (v) ? Flags |= Flags_WordWrap : Flags &= (~Flags_WordWrap); }
    void ClearWordWrap()                { SetWordWrap(false); }
    bool IsWordWrap() const             { return (Flags & Flags_WordWrap) != 0; }

    void SetMultiline(bool v = true)    { (v) ? Flags |= Flags_Multiline : Flags &= (~Flags_Multiline); }
    void ClearMultiline()               { SetMultiline(false); }
    bool IsMultiline() const            { return (Flags & Flags_Multiline) != 0; }

    void SetPassword(bool v = true)     { (v) ? Flags |= Flags_Password : Flags &= (~Flags_Password); }
    void ClearPassword()                { SetPassword(false); }
    bool IsPassword() const             { return (Flags & Flags_Password) != 0; }

    void SetReadOnly(bool v = true)     { (v) ? Flags |= Flags_ReadOnly : Flags &= (~Flags_ReadOnly); }
    void ClearReadOnly()                { SetReadOnly(false); }
    bool IsReadOnly() const             { return (Flags & Flags_ReadOnly) != 0; }

    void SetAutoSize(bool v = true)     { (v) ? Flags |= Flags_AutoSize : Flags &= (~Flags_AutoSize); }
    void ClearAutoSize()                { SetAutoSize(false); }
    bool IsAutoSize() const             { return (Flags & Flags_AutoSize) != 0; }

    void SetSelectable(bool v = true)   { (v) ? Flags |= Flags_Selectable : Flags &= (~Flags_Selectable); }
    void ClearSelectable()              { SetSelectable(false); }
    bool IsSelectable() const           { return (Flags & Flags_Selectable) != 0; }

    void SetBorder(bool v = true)       { (v) ? Flags |= Flags_Border : Flags &= (~Flags_Border); }
    void ClearBorder()                  { SetBorder(false); }
    bool IsBorder() const               { return (Flags & Flags_Border) != 0; }

    void SetHtml(bool v = true)         { (v) ? Flags |= Flags_Html : Flags &= (~Flags_Html); }
    void ClearHtml()                    { SetHtml(false); }
    bool IsHtml() const                 { return (Flags & Flags_Html) != 0; }

    void SetUseDeviceFont(bool v = true){ (v) ? Flags |= Flags_UseDeviceFont : Flags &= (~Flags_UseDeviceFont); }
    void ClearUseDeviceFont()           { SetUseDeviceFont(false); }
    bool DoesUseDeviceFont() const      { return (Flags & Flags_UseDeviceFont) != 0; }

    void SetHasLayout(bool v = true)    { (v) ? Flags |= Flags_HasLayout : Flags &= (~Flags_HasLayout); }
    void ClearHasLayout()               { SetHasLayout(false); }
    bool HasLayout() const              { return (Flags & Flags_HasLayout) != 0; }

    void SetAAForReadability(bool v = true)  { (v) ? Flags |= Flags_AAReadability : Flags &= (~Flags_AAReadability); }
    void ClearAAForReadability()             { SetAAForReadability(false); }
    bool IsAAForReadability() const          { return (Flags & Flags_AAReadability) != 0; }

    void SetEmptyTextDef(bool v = true)  { (v) ? Flags |= Flags_EmptyTextDef : Flags &= (~Flags_EmptyTextDef); }
    void ClearEmptyTextDef()             { SetEmptyTextDef(false); }
    bool IsEmptyTextDef() const          { return (Flags & Flags_EmptyTextDef) != 0; }

    void SetWasStatic(bool v = true)     { (v) ? Flags |= Flags_WasStatic : Flags &= (~Flags_WasStatic); }
    void ClearWasStatic()                { SetWasStatic(false); }
    bool WasStatic() const               { return (Flags & Flags_WasStatic) != 0; }

//    GFxCharacter*   CreateCharacterInstance(InteractiveObject* parent, ResourceId id, MovieDefImpl *pbindingImpl);

    void InitEmptyTextDef();

    void Read(LoadProcess* p, TagType tagType);
    // *** Resource implementation

    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    unsigned GetResourceTypeCode() const     { return MakeTypeCode(RT_EditTextDef); }
};

// Helper struct.
class StaticTextRecord : public NewOverrideBase<StatMD_CharDefs_Mem>
{
public:
    class GlyphEntry
    {
    public:
        unsigned    GlyphIndex;         // Index of the glyph in font
        float   GlyphAdvance;       // Advance value from previous glyph

        GlyphEntry(unsigned index = (~0u), float advance = 0.0f)
        {
            GlyphIndex          = index;
            GlyphAdvance        = advance;
        }   
    };

    ArrayLH<GlyphEntry, StatMD_CharDefs_Mem>  Glyphs;
    ResourcePtr<FontResource>   pFont;

    Color                       ColorV;
    Render::PointF              Offset; 
    float                       TextHeight; 
    UInt16                      FontId;

    StaticTextRecord();

    void    Read(Stream* in, int glyphCount, int glyphBits, int advanceBits);

    // Obtains cumulative advance value.
    float   GetCumulativeAdvance() const;
};


// An array of text glyph records, used for convenience.
class StaticTextRecordList
{
public:

    ArrayLH<StaticTextRecord*, StatMD_CharDefs_Mem> Records;

    StaticTextRecordList() {}

    ~StaticTextRecordList()
    {
        Clear();
    }

    // Removes all records.
    void Clear();

    unsigned GetSize() const
    {
        return (unsigned)Records.GetSize();
    }

    // Add a new record to the list.
    StaticTextRecord* AddRecord();
};

class StaticTextDef : public CharacterDef
{
public:    
    RectF                   TextRect;
    Matrix2F                MatrixPriv;
    StaticTextRecordList    TextRecords;
    enum
    {
        Flags_AAReadability       = 0x01,  // Anti-aliased for readability
        Flags_HasInstances        = 0x02   // was the instance of this def already created
    };
    UByte                   Flags;

    StaticTextDef();

    virtual CharacterDefType GetType() const { return StaticText; }

    void SetAAForReadability(bool v = true)  { (v) ? Flags |= Flags_AAReadability : Flags &= (~Flags_AAReadability); }
    void ClearAAForReadability()             { SetAAForReadability(false); }
    bool IsAAForReadability() const          { return (Flags & Flags_AAReadability) != 0; }

    void SetHasInstances(bool v = true) { (v) ? Flags |= Flags_HasInstances : Flags &= (~Flags_HasInstances); }
    void ClearHasInstances()            { SetHasInstances(false); }
    bool HasInstances() const           { return (Flags & Flags_HasInstances) != 0; }

    void    Read(LoadProcess* p, TagType tagType);

    virtual RectF   GetBoundsLocal(float =0) const { return TextRect; }

    virtual bool    DefPointTestLocal(const Render::PointF &pt, bool testShape = 0, const DisplayObjectBase *pinst = 0) const;

    // *** Resource implementation

    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    virtual unsigned    GetResourceTypeCode() const     { return MakeTypeCode(RT_TextDef); }
};

}} // namespace Scaleform::GFx
#endif // INC_SF_GFX_TEXTDEF_H

